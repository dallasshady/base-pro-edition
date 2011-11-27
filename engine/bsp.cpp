
#include "headers.h"
#include "bsp.h"
#include "asset.h"
#include "wire.h"
#include "collision.h"
#include "camera.h"
#include "gui.h"

BSP*       BSP::currentBSP = NULL;
BSPSector* BSPSector::currentSector = NULL;

void BSPSector::subdivide(BSPSector* sector)
{
    AABB aabbLeft, aabbRight; 
    sector->getBoundingBox()->divide( aabbLeft, aabbRight, AABB::maxPlane );

    sector->_leftSubset  = new BSPSector( sector->_bsp, sector, aabbLeft, NULL );
    sector->_rightSubset = new BSPSector( sector->_bsp, sector, aabbRight, NULL );

    if( sector->getSectorLevel() < 10 )
    {
        subdivide( sector->_leftSubset );
        subdivide( sector->_rightSubset );
    }
}

engine::IBSP* Engine::createBSP(const char* bspName, const Vector3f& boxInf, const Vector3f& boxSup)
{
    AABB boundingBox( wrap( boxInf ), wrap( boxSup ) );

    BSP* bsp = new BSP( bspName, boundingBox, 0 );
    BSPSector* rootSector = new BSPSector( bsp, NULL, boundingBox, NULL );
    //BSPSector::subdivide( rootSector );
    return bsp;
}

/**
 * bsp sector
 */

BSPSector::BSPSector(BSP* bsp, BSPSector* parent, AABB boundingBox, Geometry* geometry)
{
    _leftSubset = _rightSubset = NULL; 
    _bsp = bsp, _parent = parent;
    if( _parent ) 
    {
        if( _parent->_leftSubset == NULL ) _parent->_leftSubset = this;
        else _parent->_rightSubset = this;
    }
    else
    {
        assert( _bsp->_root == NULL );
        _bsp->_root = this;
    }
    _boundingBox = boundingBox;
    _geometry = geometry;
    if( _geometry ) _geometry->_numReferences++;
    _lightmap = NULL;
}

BSPSector::~BSPSector()
{
    assert( _atomicsInSector.size() == 0 );
    assert( _lightsInSector.size() == 0 );

    if( _leftSubset ) delete _leftSubset;
    if( _rightSubset ) delete _rightSubset;

    if( _geometry ) _geometry->release();

    if( _lightmap ) 
    {
        _lightmap->release();
    }
}

BSPSector* BSPSector::getSectorAroundPoint(const Vector& point)
{
    if( _boundingBox.isInside( point ) )
    {
        // no branching from this sector?
        if( !_leftSubset ) return this;
        // pass branching...
        BSPSector* result = _leftSubset->getSectorAroundPoint( point );
        if( !result ) return _rightSubset->getSectorAroundPoint( point );
        return result;
    }
    return NULL;
}

int BSPSector::getNumSubsetSectors(void)
{
    if( _leftSubset ) return 1 + _leftSubset->getNumSubsetSectors() + _rightSubset->getNumSubsetSectors();
    return 1;
}

int BSPSector::getNumLeafSectors(void)
{
    if( !_leftSubset ) return 1;
    return _leftSubset->getNumLeafSectors() + _rightSubset->getNumLeafSectors();
}

int BSPSector::getSectorLevel(void)
{
    if( _parent ) return _parent->getSectorLevel() + 1; else return 0;    
}

void BSPSector::illuminate(unsigned int lightset)
{
    int lightIndex = 0;
    for( LightI lightI = _lightsInSector.begin();
                lightI != _lightsInSector.end();
                lightI++ )
    {
        if( (*lightI)->lightset() == lightset &&
            (*lightI)->getType() != engine::ltAmbient )
        {            
            _dxCR( dxLightEnable( lightIndex, TRUE ) );
            (*lightI)->apply( lightIndex );

            if( lightIndex++ > 7 ) break;
        }
    }
    for( int i=lightIndex; i<8; i++ )
    {
        _dxCR( dxLightEnable( i, FALSE ) );
    }
}

void BSPSector::render(void)
{
    currentSector = this;
    Engine::statistics.bspRendered++;

    illuminate( 0 );

    if( _geometry ) 
    {
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
        _geometry->render();        
    }

    for( AtomicI atomicI = _atomicsInSector.begin();
                 atomicI != _atomicsInSector.end();
                 atomicI++ )
    {
        if( (*atomicI)->flags() & engine::afRender ) (*atomicI)->render();
    }
    currentSector = NULL;
}

void BSPSector::renderDepthMap(void)
{
    currentSector = this;
    Engine::statistics.bspRendered++;

    // render sector geometry
    if( _geometry ) 
    {
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
        _geometry->renderDepthMap();
    }

    // render sector atomics (alpha-sorting is not significant now!)
    for( AtomicI atomicI = _atomicsInSector.begin();
                 atomicI != _atomicsInSector.end();
                 atomicI++ )
    {
        if( (*atomicI)->flags() & engine::afRender ) (*atomicI)->renderDepthMap();
    }

    currentSector = NULL;
}

void BSPSector::renderShadowVolume(ShadowVolume* shadowVolume, float depth, Vector* lightPos, Vector* lightDir)
{
    currentSector = this;

    // render shadow volume of sector geometry
    /* if( _geometry ) 
    {
        shadowVolume->renderShadowVolume( _geometry, NULL, depth, lightPos, lightDir );
    }*/

    // render sector atomics (alpha-sorting is not significant now!)
    for( AtomicI atomicI = _atomicsInSector.begin();
                 atomicI != _atomicsInSector.end();
                 atomicI++ )
    {
        if( (*atomicI)->flags() & engine::afRender &&
            (*atomicI)->flags() & engine::afCastShadow )
        {
            (*atomicI)->renderShadowVolume(
                shadowVolume, depth, lightPos, lightDir
            );
        }
    }

    currentSector = NULL;
}

void BSPSector::write(IResource* resource)
{
    ChunkHeader sectorHeader( BA_SECTOR, sizeof( Chunk ) );
    sectorHeader.write( resource );

    Chunk chunk;
    chunk.id          = (auid)(this);
    chunk.bspId       = (auid)(_bsp);
    chunk.parentId    = (auid)(_parent);
    chunk.hasGeometry = ( _geometry != NULL );
    chunk.boundingBox = _boundingBox;

    // write sector chunk
    fwrite( &chunk, sizeof(Chunk), 1, resource->getFile() );

    // write geometry
    if( _geometry ) _geometry->write( resource );

    // write subsets
    if( _leftSubset )
    {
        _leftSubset->write( resource );
        _rightSubset->write( resource );
    }
}

AssetObjectT BSPSector::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader sectorHeader( resource );
    if( sectorHeader.type != BA_SECTOR ) throw Exception( "Unexpected chunk type" );
    if( sectorHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, sizeof(Chunk), 1, resource->getFile() );
    
    AssetObjectI assetObjectI = assetObjects.find( chunk.bspId );
    assert( assetObjectI != assetObjects.end() );
    BSP* bsp = reinterpret_cast<BSP*>( assetObjectI->second );

    BSPSector* parentSector = NULL;
    assetObjectI = assetObjects.find( chunk.parentId );
    if( assetObjectI != assetObjects.end() ) parentSector = reinterpret_cast<BSPSector*>( assetObjectI->second );

    Geometry* geometry = NULL;
    if( chunk.hasGeometry )
    {
        AssetObjectT assetObjectT = Geometry::read( resource, assetObjects );
        geometry = reinterpret_cast<Geometry*>( assetObjectT.second );
        geometry->setShaders( bsp->getShaders() );
        geometry->instance();
        assetObjects.insert( assetObjectT );
    }

    BSPSector* sector = new BSPSector( bsp, parentSector, chunk.boundingBox, geometry );

    return AssetObjectT( chunk.id, sector );
}

/**
 * IBSPSector
 */

bool BSPSector::isLeaf(void)
{
    return ( _leftSubset == NULL );
}

engine::IGeometry* BSPSector::getGeometry(void) 
{ 
    return _geometry; 
}

engine::IBSP* BSPSector::getBSP(void) 
{ 
    return _bsp; 
}

engine::ITexture* BSPSector::getLightMap(void)
{
    return _lightmap;
}

void BSPSector::setLightMap(engine::ITexture* lightMap)
{
    if( _lightmap )
    {
        _lightmap->release();
        _lightmap = NULL;
    }

    _lightmap = dynamic_cast<Texture*>( lightMap );
    if( _lightmap ) _lightmap->addReference();
}

/**
 * bsp
 */

engine::IAtomic* BSP::setAtomicWorldCB(engine::IAtomic* atomic, void* data)
{
    Atomic* a = dynamic_cast<Atomic*>( atomic );
    a->_bsp = data;
    a->onUpdate();
    return atomic;
}

engine::IAtomic* BSP::removeAtomicCB(engine::IAtomic* atomic, void* data)
{
    Atomic*    a      = dynamic_cast<Atomic*>( atomic );
    BSP*       bsp    = reinterpret_cast<BSP*>( a->_bsp );
    
    BSPSector* sector;
    while( a->_sectors.size() )
    {
        sector = reinterpret_cast<BSPSector*>( *a->_sectors.begin() );
        BSPSector::AtomicI atomicI = sector->_atomicsInSector.find( a );
        if( atomicI != sector->_atomicsInSector.end() ) 
        {
            sector->_atomicsInSector.erase( atomicI );
        }
        a->_sectors.erase( a->_sectors.begin() );
    }
    a->_bsp = NULL;
    return atomic;
}

engine::ILight* BSP::setLightWorldCB(engine::ILight* light, void* data)
{
    Light* l = dynamic_cast<Light*>( light );
    l->_bsp = data;
    l->_sectors.clear();
    l->onUpdate();
    if( l->getType() == engine::ltAmbient ) reinterpret_cast<BSP*>( data )->_ambients.push_back( l );
    return light;
}

engine::ILight* BSP::removeLightCB(engine::ILight* light, void* data)
{
    Light* l = dynamic_cast<Light*>( light );

    BSPSector* sector;    
    BSPSector::LightI lightI;
    for( Light::voidI sectorI = l->_sectors.begin();
                      sectorI != l->_sectors.end();
                      sectorI++ )
    {
        sector = reinterpret_cast<BSPSector*>( *sectorI );
        for( lightI = sector->_lightsInSector.begin();
             lightI != sector->_lightsInSector.end();
             lightI++ )
        {
            if( *lightI == l )
            {
                sector->_lightsInSector.erase( lightI );
                break;
            }
        }
    }
    l->_sectors.clear();
    l->_bsp = NULL;

    return light;
}

BSPSector* BSP::sectorCallBack(BSPSector* sector, engine::IBSPSectorCallBack callBack, void* data)
{
    if( sector->_leftSubset )
    {
        if( !sectorCallBack( sector->_leftSubset, callBack, data ) ) return NULL;
        return sectorCallBack( sector->_rightSubset, callBack, data );
    }
    return ( callBack( sector, data ) ? sector : NULL );
}

BSPSector* BSP::sectorRender(BSPSector* sector)
{
    if( intersectAABBFrustum( &sector->_boundingBox, Camera::frustrum ) )
    {
        if( sector->_leftSubset )
        {
            sector->_lsc = sector->_leftSubset->getBoundingBox()->inf + 0.5f * ( sector->_leftSubset->getBoundingBox()->sup - sector->_leftSubset->getBoundingBox()->inf );
            sector->_rsc = sector->_rightSubset->getBoundingBox()->inf + 0.5f * ( sector->_rightSubset->getBoundingBox()->sup - sector->_rightSubset->getBoundingBox()->inf );
            sector->_lsd = Camera::eyePos - sector->_lsc;
            sector->_rsd = Camera::eyePos - sector->_rsd;

            if( D3DXVec3LengthSq( &sector->_lsd ) < D3DXVec3LengthSq( &sector->_rsd ) )
            {
                sectorRender( sector->_leftSubset );
                sectorRender( sector->_rightSubset );
            }
            else
            {
                sectorRender( sector->_rightSubset );
                sectorRender( sector->_leftSubset );
            }
        }
        else
        {
            sector->render();
            if( Engine::instance->getRenderMode() & engine::rmBSPAABB )
            {
                if( sector->_atomicsInSector.size() )
                {
                    dxRenderAABB( sector->getBoundingBox(), &green, NULL );
                }
                else
                {
                    dxRenderAABB( sector->getBoundingBox(), &yellow, NULL );
                }
            }
        }
    }
    return sector;
}

BSP::BSP(const char* bspName, AABB boundingBox, int numShaders)
{
    _name = bspName;
    _boundingBox = boundingBox;
    _numShaders  = numShaders;
    if( _numShaders ) 
    {
        _shaders = new Shader*[_numShaders];
        memset( _shaders, 0, sizeof(Shader*)*_numShaders );
    }
    else 
    {
        _shaders = NULL;
    }
    _root = NULL;

    _renderFrameId = 0;

    _fogMode    = D3DFOG_NONE;
    _fogStart   = 300000;
    _fogEnd     = 500000;
    _fogDensity = 0.000005f;
    _fogColor   = D3DCOLOR_RGBA( 255, 255, 255, 255 );

    _shadowCastDirection = Vector( 0,-1,0 );
    _shadowCastColor     = D3DCOLOR_RGBA( 0, 0, 0, 96 );
    _shadowCastDepth     = 100000.0f;

    // create shadow volume pipeline
    _shadowVolume = new ShadowVolume( 4096 );

    _postRenderCallback = NULL;
    _postRenderCallbackData = NULL;
}

BSP::~BSP()
{
    while( _renderings.size() )
    {
        Rendering* rendering = *_renderings.begin();
        remove( rendering );
        rendering->release();
    }
    while( _particleSystems.size() )
    {
        ParticleSystem* pSys = *_particleSystems.begin();
        remove( pSys );
        pSys->release();
    }
    while( _clumps.size() )
    {
        Clump* clump = *_clumps.begin();
        remove( clump );
        clump->release();
    }
    while( _lights.size() )
    {
        Light* light = *_lights.begin();
        remove( light );
        light->release();
    }
    delete _root;
    for( int i=0; i<_numShaders; i++ ) _shaders[i]->release();
    delete _shaders;
    delete _shadowVolume;
}

const char* BSP::getName(void)
{
    return _name.c_str();
}

void BSP::release(void)
{
    delete this;
}

void BSP::add(engine::IClump* clump)
{
    Clump* c = dynamic_cast<Clump*>( clump ); assert( c );
    _clumps.push_back( c );
    c->forAllAtomics( setAtomicWorldCB, this );
    c->forAllLights( setLightWorldCB, this );
    c->_bsp = this;
}

void BSP::add(engine::ILight* light)
{
    Light* l = dynamic_cast<Light*>( light ); assert( l );
    _lights.push_back( l );
    if( l->getType() == engine::ltAmbient ) _ambients.push_back( l );
    l->_bsp = this;
    l->onUpdate();
}

void BSP::add(engine::IRendering* rendering)
{
    Rendering* r = dynamic_cast<Rendering*>( rendering ); assert( r );
    _renderings.push_back( r );
}

void BSP::add(engine::IParticleSystem* particleSystem)
{
    ParticleSystem* pSys = dynamic_cast<ParticleSystem*>( particleSystem ); assert( pSys );
    _particleSystems.push_back( pSys );
}

void BSP::add(engine::IBatch* batch)
{
    Batch* b = dynamic_cast<Batch*>( batch ); assert( b );
    _batches.push_back( b ); 
}

void BSP::remove(engine::IClump* clump)
{
    Clump* c = dynamic_cast<Clump*>( clump );
    c->forAllAtomics( removeAtomicCB, this );
    c->forAllLights( removeLightCB, this );
    for( ClumpI clumpI = _clumps.begin(); clumpI != _clumps.end(); clumpI++ )
    {
        if( *clumpI == c )
        {
            _clumps.erase( clumpI );
            break;
        }
    }
    c->_bsp = NULL;
}

void BSP::remove(engine::ILight* light)
{
    Light* l = dynamic_cast<Light*>( light );
    for( LightI lightI = _lights.begin(); lightI != _lights.end(); lightI++ )
    {
        if( *lightI == l )
        {
            removeLightCB( light, this );
            _lights.erase( lightI );
            break;
        }
    }
}

void BSP::remove(engine::IRendering* rendering)
{
    Rendering* r = dynamic_cast<Rendering*>( rendering ); assert( r );

    for( RenderingI renderingI = _renderings.begin(); 
                    renderingI != _renderings.end(); 
                    renderingI++ )
    {
        if( *renderingI == r )
        {
            _renderings.erase( renderingI );
            break;
        }
    }
}

void BSP::remove(engine::IParticleSystem* particleSystem)
{
    ParticleSystem* pSys = dynamic_cast<ParticleSystem*>( particleSystem ); assert( pSys );

    for( ParticleSystemI pSysI = _particleSystems.begin(); 
                         pSysI != _particleSystems.end(); 
                         pSysI++ )
    {
        if( *pSysI == pSys )
        {
            _particleSystems.erase( pSysI );
            break;
        }
    }
}

void BSP::remove(engine::IBatch* batch)
{
    Batch* b = dynamic_cast<Batch*>( batch ); assert( b );

    for( BatchI batchI = _batches.begin(); 
                batchI != _batches.end(); 
                batchI++ )
    {
        if( *batchI == b )
        {
            _batches.erase( batchI );
            break;
        }
    }
}

void BSP::forAllSectors(engine::IBSPSectorCallBack callBack, void* data)
{
    sectorCallBack( _root, callBack, data );
}

void BSP::forAllLights(engine::ILightCallBack callBack, void* data)
{
    for( LightI lightI = _lights.begin(); lightI != _lights.end(); lightI++ )
    {
        if( !callBack( *lightI, data ) ) return;
    }
}

void BSP::forAllClumps(engine::IClumpCallBack callBack, void* data)
{
    for( ClumpI clumpI = _clumps.begin(); clumpI != _clumps.end(); clumpI++ )
    {
        if( !callBack( *clumpI, data ) ) return;
    }
}

Vector4f BSP::getAmbient(unsigned int lightset)
{
    // calculate world global ambient color
    D3DCOLORVALUE result;
    dxSetColorValue( &result, 0, 0, 0, 1 );
    for( LightI ambientI = _ambients.begin();
                ambientI != _ambients.end();
                ambientI++ )
    {
        if( (*ambientI)->lightset() == lightset )
        {
            dxAddColorValue( &result, (*ambientI)->diffuse() );
            dxSaturateColorValue( &result );
        }
    }
    return Vector4f( result.r, result.g, result.b, result.a );
}

void BSP::calculateGlobalAmbient(unsigned int lightset)
{
    // calculate world global ambient color for default lightset
    dxSetColorValue( Shader::globalAmbient(), 0, 0, 0, 1 );
    for( LightI ambientI = _ambients.begin();
                ambientI != _ambients.end();
                ambientI++ )
    {
        if( (*ambientI)->lightset() == lightset )
        {
            dxAddColorValue( Shader::globalAmbient(), (*ambientI)->diffuse() );
            dxSaturateColorValue( Shader::globalAmbient() );
        }
    }
}

static void renderFrameHierarchy(Frame* frame)
{
    if( frame->pParentFrame )
    {
        Line line;
        Color color = D3DCOLOR_RGBA( 255, 255, 255, 255 );
        line.start = dxPos( &frame->LTM );
        line.end   = dxPos( &frame->pParentFrame->LTM );
        dxRenderLine( &line, &color, NULL );
    }
    if( frame->pFrameSibling ) 
    {
        renderFrameHierarchy( static_cast<Frame*>( frame->pFrameSibling ) );
    }
    if( frame->pFrameFirstChild ) 
    {
        renderFrameHierarchy( static_cast<Frame*>( frame->pFrameFirstChild ) );
    }
}

// render state corrector data
static gui::Rect rscRect( 0,0,7,7 );
static gui::Rect rscTextureRect( 0,0,7,7 );
static Vector4f  rscColor( 1,1,1,1 );

BSPSector* BSP::sectorRenderShadowVolume(
    BSPSector* sector, 
    ShadowVolume* shadowVolume,
    float depth,
    Vector* lightPos,
    Vector* lightDir)
{
    // pass BSP sub-tree
    if( sector->_leftSubset )
    {
        sectorRenderShadowVolume( sector->_leftSubset, shadowVolume, depth, lightPos, lightDir );
        sectorRenderShadowVolume( sector->_rightSubset, shadowVolume, depth, lightPos, lightDir );
    }

    // leaf sector?
    if( sector->geometry() )
    {
        // render shadow volume
        sector->renderShadowVolume( shadowVolume, depth, lightPos, lightDir );
    }

    // next
    return sector;
}

engine::ILight* BSP::findShadowCastLightCB(engine::ILight* light, void* data)
{
    Light** result = reinterpret_cast<Light**>( data );

    if( light->getType() == engine::ltPoint )
    {
        *result = dynamic_cast<Light*>( light );
        return NULL;
    }
    else
    {
        return light;
    }
}

engine::IClump* BSP::findShadowCastLightCB(engine::IClump* clump, void* data)
{
    clump->forAllLights( findShadowCastLightCB, data );
    return clump;
}

void BSP::render(void)
{
    currentBSP = this;

    // reset shader buffering
    Shader::_lastShader = NULL;
    _renderFrameId++;
    Engine::statistics.bspTotal += _root->getNumLeafSectors();

    // update LODs
    for( ClumpI clumpI = _clumps.begin(); clumpI != _clumps.end(); clumpI++ )
    {
        (*clumpI)->updateLODs();
    }

    calculateGlobalAmbient( 0 );

    // setup fog
    if( _fogMode != D3DFOG_NONE )
    {
        dxSetRenderState( D3DRS_FOGENABLE, TRUE );
        dxSetRenderState( D3DRS_FOGCOLOR, _fogColor );
        dxSetRenderState( D3DRS_FOGTABLEMODE, _fogMode );
        switch( _fogMode )
        {
        case D3DFOG_LINEAR:
            dxSetRenderState( D3DRS_FOGSTART, *(DWORD*)(&_fogStart) );
            dxSetRenderState( D3DRS_FOGEND,   *(DWORD*)(&_fogEnd) );
            break;
        case D3DFOG_EXP:
        case D3DFOG_EXP2:
            dxSetRenderState( D3DRS_FOGDENSITY, *(DWORD*)(&_fogDensity) );
            break;
        }
    }

    // render all opaque geometry
    sectorRender( _root );

    // render all batched geometries
    for( BatchI batchI = _batches.begin(); batchI != _batches.end(); batchI++ )
    {
        (*batchI)->render();
    }

    // render all transparent geometry
    renderAlphaGeometry();

    {
        // find light for shadow casting
        Light* caster = NULL;
        forAllClumps( findShadowCastLightCB, &caster );

        // build shadow casting data
        Vector lightPosData;
        if( caster )
        {
            lightPosData.x = caster->frame()->LTM._41;
            lightPosData.y = caster->frame()->LTM._42;
            lightPosData.z = caster->frame()->LTM._43;
        }
        Vector* lightPos = caster ? &lightPosData : NULL;
        Vector* lightDir = caster ? NULL : &_shadowCastDirection;

        // fill stencil buffer with shadow data
        _renderFrameId++;
        sectorRenderShadowVolume( _root, _shadowVolume, _shadowCastDepth, lightPos, lightDir );
    }

    // render shadow mask
    _shadowVolume->renderShadow( &_shadowCastColor );

    for( RenderingI renderingI = _renderings.begin();
                    renderingI != _renderings.end();
                    renderingI++ )
    {
        (*renderingI)->render();
    }

    for( ParticleSystemI pSysI = _particleSystems.begin();
                         pSysI != _particleSystems.end();
                         pSysI++ )
    {
        (*pSysI)->render();
    }

    if( _postRenderCallback ) _postRenderCallback( _postRenderCallbackData );

    // bugfix (render state corrector)
    dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    Gui::instance->renderRect( 
        rscRect,
        Engine::instance->alphaBlackTexture,
        rscTextureRect,
        rscColor
    );
    dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE );

    // reset fog
    if( _fogMode != D3DFOG_NONE )
    {
        dxSetRenderState( D3DRS_FOGENABLE, FALSE );        
    }

    // reset global ambient
    dxSetColorValue( Shader::globalAmbient(), 0, 0, 0, 1 );

    // frame hierarchy
    if( Engine::instance->getRenderMode() & engine::rmFrameHierarchy )
    {
        dxSetRenderState( D3DRS_ZENABLE, FALSE );
        for( ClumpI clumpI = _clumps.begin(); clumpI != _clumps.end(); clumpI++ )
        {
            renderFrameHierarchy( (*clumpI)->frame() );
        }
        dxSetRenderState( D3DRS_ZENABLE, TRUE );
    }

    //fclose( _renderReport );
    //_renderReport = NULL;
}

/**
 * IBSP : fog controls
 */

engine::FogType BSP::getFogType(void)
{
    switch( _fogMode )
    {
    case D3DFOG_NONE: return engine::fogNone;
    case D3DFOG_EXP: return engine::fogExp;
    case D3DFOG_EXP2: return engine::fogExp2;
    case D3DFOG_LINEAR: return engine::fogLinear;
    default:
        assert( !"shouldn't be here!" );
        return engine::fogNone;
    }
}

void BSP::setFogType(engine::FogType fogType)
{
    switch( fogType )
    {
    case engine::fogNone: 
        _fogMode = D3DFOG_NONE;
        break;
    case engine::fogExp:
        _fogMode = D3DFOG_EXP;
        break;
    case engine::fogExp2: 
        _fogMode = D3DFOG_EXP2;
        break;
    case engine::fogLinear: 
        _fogMode = D3DFOG_LINEAR;
        break;
    default:
        assert( !"shouldn't be here!" );
        _fogMode = D3DFOG_NONE;
    }
}

float BSP::getFogStart(void)
{
    return _fogStart;
}

void BSP::setFogStart(float fogStart)
{
    _fogStart = fogStart;
}

float BSP::getFogEnd(void)
{
    return _fogEnd;
}

void BSP::setFogEnd(float fogEnd)
{
    _fogEnd = fogEnd;
}

float BSP::getFogDensity(void)
{
    return _fogDensity;
}

void BSP::setFogDensity(float fogDensity)
{
    _fogDensity = fogDensity;
}

Vector4f BSP::getFogColor(void)
{
    return wrap( _fogColor );
}

void BSP::setFogColor(const Vector4f& fogColor)
{
    _fogColor = wrap( fogColor );
}

/**
 * locals
 */

void BSP::setShadowCastDirection(const Vector3f& dir)
{
    _shadowCastDirection = wrap( dir );    
}

void BSP::setShadowCastColor(const Vector4f& color)
{
    _shadowCastColor = wrap( color );    
}

void BSP::setShadowCastDepth(float depth)
{
    _shadowCastDepth = depth;
}

/**
 * locals
 */

void BSP::setShader(int id, Shader* shader)
{
    assert( id>=0 && id<_numShaders );
    if( _shaders[id] ) _shaders[id]->release();
    _shaders[id] = shader;
    if( _shaders[id] ) _shaders[id]->_numReferences++;
}

void BSP::write(IResource* resource)
{
    ChunkHeader bspHeader( BA_BSP, sizeof( Chunk ) );
    bspHeader.write( resource );

    Chunk chunk;
    memset( chunk.name, 0, engine::maxNameLength );
    if( _name.length() > engine::maxNameLength )
    {
        strncpy( chunk.name, _name.c_str(), engine::maxNameLength - 1 );
    }
    else
    {
        strcpy( chunk.name, _name.c_str() );
    }

    chunk.id = (auid)( this );
    chunk.boundingBox = _boundingBox;
    chunk.numSectors = _root->getNumSubsetSectors();
    chunk.numShaders = _numShaders;
    chunk.fogMode    = _fogMode;
    chunk.fogStart   = _fogStart;
    chunk.fogEnd     = _fogEnd;
    chunk.fogDensity = _fogDensity;
    chunk.fogColor   = _fogColor;

    // write BSP chunk
    fwrite( &chunk, sizeof(Chunk), 1, resource->getFile() );

    // write shaders
    for( int i=0; i<_numShaders; i++ ) _shaders[i]->write( resource );
    
    // write sectors
    _root->write( resource );
}

AssetObjectT BSP::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader bspHeader( resource );
    if( bspHeader.type != BA_BSP ) throw Exception( "Unexpected chunk type" );
    if( bspHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, sizeof(Chunk), 1, resource->getFile() );

    BSP* bsp = new BSP( chunk.name, chunk.boundingBox, chunk.numShaders );

    bsp->_fogMode    = chunk.fogMode;
    bsp->_fogStart   = chunk.fogStart;
    bsp->_fogEnd     = chunk.fogEnd;
    bsp->_fogDensity = chunk.fogDensity;
    bsp->_fogColor   = chunk.fogColor;

    // read shaders
    for( int i=0; i<chunk.numShaders; i++ )
    {
        AssetObjectT assetObjectT = Shader::read( resource, assetObjects );
        bsp->setShader( i, reinterpret_cast<Shader*>( assetObjectT.second ) );
        assetObjects.insert( assetObjectT );
    }

    assetObjects.insert( AssetObjectT( chunk.id, bsp ) );

    // read sectors
    for( i=0; i<chunk.numSectors; i++ )
    {
        assetObjects.insert( BSPSector::read( resource, assetObjects ) );
    }

    return AssetObjectT( AssetObjectT( chunk.id, bsp ) );
}

/**
 * collision handling
 */

bool BSPSector::forAllAtomicIntersections(Line* ray, AtomicRayIntersectionCallback callBack)
{
    if( !intersectionRayAABB( ray, &_boundingBox ) ) return true;

    if( _leftSubset )
    {
        if( _leftSubset->forAllAtomicIntersections( ray, callBack ) == NULL ) return false;
        return NULL != _rightSubset->forAllAtomicIntersections( ray, callBack );
    }
    else
    {
        for( AtomicI atomicI = _atomicsInSector.begin();
                     atomicI != _atomicsInSector.end();
                     atomicI++ )
        {
            float d;
            if( intersectionRaySphere( ray, &(*atomicI)->_boundingSphere, &d ) )
            {
                if( callBack( ray, *atomicI ) == NULL ) return false;
            }
        }
    }
    return true;
}

void BSP::forAllAtomicIntersections(Line* ray, AtomicRayIntersectionCallback callBack)
{
    _root->forAllAtomicIntersections( ray, callBack );
}

/**
 * alpha-sorting support
 */

static Vector _objectPos;
static Vector _distanceV;
static float  _distance;
static bool   _nearest;

void BSP::addAlphaGeometry(Atomic* atomic, unsigned int subsetId)
{
    // calculate sorting key
    _objectPos = atomic->getBoundingSphere()->center;
    D3DXVec3Subtract( &_distanceV, &_objectPos, &Camera::eyePos );
    _distance = 0.005f * D3DXVec3Length( &_distanceV );

    // retrieve sector
    BSPSector* firstSector = NULL;
    if( atomic->_sectors.size() ) 
    {
        firstSector = reinterpret_cast<BSPSector*>( *atomic->_sectors.begin() );
    }

    // choose alpha pool
    if( _distance <= 255 )
    {
        _sortedAlpha.add( &AlphaGeometry( 
            _nearest,
            unsigned char( _distance ),
            atomic->frame(), 
            atomic->geometry(),
            firstSector,
            atomic->getBoneMatrices(),
            subsetId 
        ) );
    }
    else
    {
        _unsortedAlpha.push_back( AlphaGeometry( 
            _nearest,
            unsigned char( _distance ),
            atomic->frame(), 
            atomic->geometry(),
            firstSector,
            atomic->getBoneMatrices(),
            subsetId 
        ) );
    }
}

void BSP::addAlphaGeometry(BSPSector* sector, unsigned int subsetId)
{
    // calculate sorting key
    _objectPos = sector->getBoundingBox()->inf + 0.5f * ( sector->getBoundingBox()->sup - sector->getBoundingBox()->inf );
    D3DXVec3Subtract( &_distanceV, &_objectPos, &Camera::eyePos );
    _distance = 0.005f * D3DXVec3Length( &_distanceV );

    // choose alpha pool
    if( _distance <= 255 )
    {
        _sortedAlpha.add( &AlphaGeometry( 
            false,
            unsigned char( _distance ),
            NULL, 
            sector->geometry(),
            sector,
            NULL,
            subsetId 
        ) );
    }
    else
    {
        _unsortedAlpha.push_back( AlphaGeometry( 
            false,
            unsigned char( _distance ),
            NULL, 
            sector->geometry(),
            sector,
            NULL,
            subsetId 
        ) );
    }
}

void BSP::renderAlphaGeometry(void)
{
    if( ( _sortedAlpha.size() + _unsortedAlpha.size() ) == 0 ) return;

    BSPSector* lastLightingSource = NULL;

    // render unsorted alpha-objects
    AlphaGeometryI alphaGeometryI;
    for( alphaGeometryI = _unsortedAlpha.begin();
         alphaGeometryI != _unsortedAlpha.end();
         alphaGeometryI++ )
    {
        // setup world transformation
        if( alphaGeometryI->frame )
        {
            _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &alphaGeometryI->frame->LTM ) );
        }
        else
        {
            _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
        }

        if( lastLightingSource != alphaGeometryI->sector )
        {
            assert( alphaGeometryI->sector );
            lastLightingSource = alphaGeometryI->sector;
            lastLightingSource->illuminate( 0 );
        }
        
        // render subset
        if( alphaGeometryI->boneMatrices )
        {
            Mesh::pBoneMatrices = alphaGeometryI->boneMatrices;
        }
        alphaGeometryI->geometry->renderAlphaGeometry( alphaGeometryI->subsetId );
    }

    Engine::statistics.alphaObjectsRendered += _sortedAlpha.size();

    // sort & render nearest alpha objects
    _sortedAlpha.render();

    _sortedAlpha.clear();
    _unsortedAlpha.clear();
}

Vector3f BSP::getAABBInf(void)
{
    AABB* aabb = _root->getBoundingBox();
    return wrap( aabb->inf );
}

Vector3f BSP::getAABBSup(void)
{
    AABB* aabb = _root->getBoundingBox();
    return wrap( aabb->sup );
}

void BSP::setPostRenderCallback(engine::BSPRenderCallback callback, void* data)
{
    _postRenderCallback = callback;
    _postRenderCallbackData = data;
}