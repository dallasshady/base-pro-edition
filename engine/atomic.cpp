
#include "headers.h"
#include "atomic.h"
#include "bsp.h"
#include "asset.h"
#include "wire.h"
#include "collision.h"
#include "camera.h"

/**
 * creation routine
 */

engine::IAtomic* Engine::createAtomic(void)
{
    return new Atomic;
}

/**
 * partial functions
 */

BSP* getAtomicBSP(Atomic* atomic)
{
    return reinterpret_cast<BSP*>( atomic->getBSP() );
}

/**
 * class implementation
 */

Atomic* Atomic::currentAtomic = NULL;
    
Atomic::Atomic()
{
    _flags = engine::afRender | engine::afCollision;
    _renderFrameId = 0;
    _frame = NULL;
    _geometry = NULL;
    _bsp = NULL;
    _boneMatrices = NULL;
    _lightmap = NULL;

    // initialize runtime members
    _lodEnabled = false;
    _lodMaxDistance = _lodMinDistance = 0.0f;
    _renderCallback = NULL;
    _renderCallbackData = NULL;
    _distanceUT = 5000.0f;
    _radiusUT = 0.0f;
    _contourDirection.x = _contourDirection.y = _contourDirection.z = 0.0f;
}

Atomic::~Atomic()
{
    setFrame( NULL );
    if( _geometry ) _geometry->release();
    if( _lightmap ) _lightmap->release();
}

void Atomic::release(void)
{
    delete this;
}

unsigned int Atomic::getFlags(void)
{
    return _flags;
}

void Atomic::setFlags(unsigned int flags)
{
    _flags = flags;
    if( _frame ) onUpdate();
}

engine::IFrame* Atomic::getFrame(void)
{
    return _frame;
}

void Atomic::setFrame(engine::IFrame* frame)
{
    if( _frame ) _frame->pAttachedObject = NULL;
    _frame = dynamic_cast<Frame*>( frame );
    if( _frame )
    {
        assert( _frame->pAttachedObject == NULL );
        _frame->pAttachedObject = this;
        onUpdate();
    }

    // update bone matrices
    if( _boneMatrices ) delete[] _boneMatrices;
    _boneMatrices = NULL;
    if( _frame && _geometry && _geometry->mesh() && _geometry->mesh()->pSkinInfo )
    {
        _boneMatrices = _geometry->mesh()->assembleBoneMatrices( _frame->getRoot() );
    }
}

engine::IGeometry* Atomic::getGeometry(void)
{
    return _geometry;
}

void Atomic::setGeometry(engine::IGeometry* geometry)
{
    if( _geometry ) _geometry->release();
    _geometry = dynamic_cast<Geometry*>( geometry );
    if( _geometry ) _geometry->_numReferences++;

    // update bone matrices
    if( _boneMatrices ) delete[] _boneMatrices;
    _boneMatrices = NULL;
    if( _frame && _geometry && _geometry->mesh() && _geometry->mesh()->pSkinInfo )
    {
        _boneMatrices = _geometry->mesh()->assembleBoneMatrices( _frame->getRoot() );
    }
}

void Atomic::setRenderCallback(engine::IAtomicCallBack callBack, void* data)
{
    _renderCallback = callBack;
    _renderCallbackData = data;   
}

void Atomic::setUpdateTreshold(float distance, float radius)
{
    _distanceUT = distance, _radiusUT = radius;
}

Atomic* Atomic::clone(void)
{
    Atomic* result = new Atomic;
    result->setGeometry( _geometry );
    result->setFlags( _flags );
    return result;
}

Vector3f Atomic::getAABBInf(void)
{
    // calculate AABB in world space
    AABB worldAABB;
    worldAABB.calculate( geometry()->getBoundingBox(), &_frame->LTM );
    // return result
    return wrap( worldAABB.inf );
}

Vector3f Atomic::getAABBSup(void)
{
    // calculate AABB in world space
    AABB worldAABB;
    worldAABB.calculate( geometry()->getBoundingBox(), &_frame->LTM );
    // return result
    return wrap( worldAABB.sup );
}

engine::ITexture* Atomic::getLightMap(void)
{
    return _lightmap;
}

void Atomic::setLightMap(engine::ITexture* lightMap)
{
    if( _lightmap )
    {
        _lightmap->release();
        _lightmap = NULL;
    }

    _lightmap = dynamic_cast<Texture*>( lightMap );
    _lightmap->addReference();
}

Vector3f Atomic::getContourDirection(void)
{
    return wrap( _contourDirection );
}

void Atomic::setContourDirection(const Vector3f& value)
{
    _contourDirection.x = value[0];
    _contourDirection.y = value[1];
    _contourDirection.z = value[2];
}

/**
 * module locals
 */

void Atomic::render(void)
{
    if( _bsp )
    {
        if( reinterpret_cast<BSP*>(_bsp)->getRenderFrameId() == _renderFrameId ) return;
        _renderFrameId = reinterpret_cast<BSP*>(_bsp)->getRenderFrameId();
    }
    if( _frame && _geometry )
    {

        Engine::statistics.atomicsRendered++;       
        currentAtomic = this;

        if( _renderCallback ) _renderCallback( this, _renderCallbackData );

        // setup world transformation
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &_frame->LTM ) );        
        
        // setup bone matrices
        if( _boneMatrices ) Mesh::pBoneMatrices = _boneMatrices;

        // is atomic a shadow caster?
        if( _flags & engine::afCastShadow )
        {
            // setup stencil buffer to avoid self-shadowing
            /*
            dxSetRenderState( D3DRS_STENCILENABLE, TRUE );
            dxSetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );
            dxSetRenderState( D3DRS_STENCILREF, 0x1 );
            dxSetRenderState( D3DRS_STENCILMASK, 0xffffffff );
            dxSetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
            dxSetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR );
            dxSetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCR );
            dxSetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
            */

            // render geometry
            _geometry->render();

            // disable stencil buffer
            // dxSetRenderState( D3DRS_STENCILENABLE, FALSE );
        }
        else
        {
            // render geometry
            _geometry->render();
        }

        // render hint
        if( Engine::instance->getRenderMode() & engine::rmAtomicSphere )
        {
            //dxRenderAABB( geometry()->getBoundingBox(), &white, &frame()->LTM );
            dxRenderSphere( &_boundingSphere, &gray, NULL );
        }

        currentAtomic = NULL;
    }
}

void Atomic::renderDepthMap(void)
{
    if( _bsp )
    {
        if( reinterpret_cast<BSP*>(_bsp)->getRenderFrameId() == _renderFrameId ) return;
        _renderFrameId = reinterpret_cast<BSP*>(_bsp)->getRenderFrameId();
    }
    if( _frame && _geometry )
    {
        Engine::statistics.atomicsRendered++;
        currentAtomic = this;

        // setup world transformation
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &_frame->LTM ) );

        // setup bone matrices
        if( _boneMatrices ) Mesh::pBoneMatrices = _boneMatrices;

        // render geometry ib to the depth map
        _geometry->renderDepthMap();

        currentAtomic = NULL;
    }
}

void Atomic::renderShadowVolume(ShadowVolume* shadowVolume, float depth, Vector* lightPos, Vector* lightDir)
{
    if( _bsp )
    {
        if( reinterpret_cast<BSP*>(_bsp)->getRenderFrameId() == _renderFrameId ) return;
        _renderFrameId = reinterpret_cast<BSP*>(_bsp)->getRenderFrameId();
    }
    if( _frame && _geometry )
    {
        currentAtomic = this;

        // setup bone matrices
        if( _boneMatrices ) 
        {
            Mesh::pBoneMatrices = _boneMatrices; 

            //shadowVolume->renderShadowCaster( _geometry, NULL );
            shadowVolume->renderShadowVolume(
                _geometry, NULL, depth,  lightPos,  lightDir
            );
        }
        else
        {
            // render shadow volume            
            shadowVolume->renderShadowVolume(
                _geometry, &_frame->LTM, depth,  lightPos,  lightDir
            );
        }

        currentAtomic = NULL;
    }
}

void Atomic::setLOD(float maxDistance, float minDistance)
{
    _lodEnabled = true;
    _lodMaxDistance = maxDistance;
    _lodMinDistance = minDistance;
}

void Atomic::write(IResource* resource)
{
    ChunkHeader atomicHeader( BA_ATOMIC, sizeof( Chunk ) );
    atomicHeader.write( resource );

    Chunk chunk;
    chunk.id = (auid)( this );
    chunk.frameId = (auid)( _frame );
    chunk.geometryId = (auid)( _geometry );
    fwrite( &chunk, atomicHeader.size, 1, resource->getFile() );
}

AssetObjectT Atomic::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader atomicHeader( resource );
    if( atomicHeader.type != BA_ATOMIC ) throw Exception( "Incompatible binary asset format" );
    if( atomicHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, atomicHeader.size, 1, resource->getFile() );

    Atomic* atomic = new Atomic;

    // setup atomic geometry    
    AssetObjectI assetObjectI = assetObjects.find( chunk.geometryId );
    if( assetObjectI != assetObjects.end() )
    {
        atomic->setGeometry( reinterpret_cast<Geometry*>( assetObjectI->second ) );
    }

    // setup atomic frame
    assetObjectI = assetObjects.find( chunk.frameId );
    if( assetObjectI != assetObjects.end() )
    {
        atomic->setFrame( reinterpret_cast<Frame*>( assetObjectI->second ) );
    }

    return AssetObjectT( chunk.id, atomic );
}

/**
 * Updatable behaviour
 */

static Vector _wTemp;
static float  _wDistance;
static float  _wRadius;

void Atomic::onUpdate(void)
{
    // update bounding volume
    if( _geometry )
    {
        // non-skinned for all
        //_boundingSphere = *_geometry->getBoundingSphere();
        //_boundingSphere.center += dxPos( &_frame->LTM );

        if( strcmp( _geometry->getName(), "Mask_meshShape" ) == 0 )
        {
            int iiii=0;
        }
        
        // non-skinned?        
        if( _boneMatrices == NULL )
        {
            // obtain LTM of atomic
            Matrix* ltm = &_frame->LTM;
            // transform geometry bounding sphere with LTM
            D3DXVec3TransformCoord(
                &_boundingSphere.center, 
                &_geometry->getBoundingSphere()->center, 
                ltm
            );
            Vector x( ltm->_11, ltm->_12, ltm->_13 );
            Vector y( ltm->_21, ltm->_22, ltm->_23 );
            Vector z( ltm->_31, ltm->_32, ltm->_33 );
            float xScaleSq = D3DXVec3LengthSq( &x );
            float yScaleSq = D3DXVec3LengthSq( &y );
            float zScaleSq = D3DXVec3LengthSq( &z );
            float maxScale = xScaleSq > yScaleSq ? xScaleSq : yScaleSq;
            maxScale = maxScale > zScaleSq ? maxScale : zScaleSq;
            maxScale = sqrt( maxScale );
            _boundingSphere.radius = _geometry->getBoundingSphere()->radius * maxScale;
        }
        // skinned?
        else
        {
            _boundingSphere.calculate( 
                _geometry->mesh()->pSkinInfo->GetNumBones(),
                _boneMatrices
            );
        }
        
    }

    // determine weight of difference btw. bounding sphere and synchro sphere
    D3DXVec3Subtract( &_wTemp, &_boundingSphere.center, &_synchroSphere.center );
    _wDistance = D3DXVec3Length( &_wTemp );
    _wRadius   = fabs( _boundingSphere.radius - _synchroSphere.radius );

    // update atomic placement in world sectors
    if( ( _wDistance > _distanceUT ) || ( _wRadius > _radiusUT ) )
    {        
        if( _bsp )
        {        
            // update atomic location in bsp-tree
            if( _flags & engine::afCollision || _flags & engine::afRender )
            {
                // affect world sectors
                BSPSector::AtomicI atomicI;
                for( voidI sectorI = _sectors.begin(); 
                           sectorI != _sectors.end(); 
                           sectorI++ )
                {
                    atomicI = reinterpret_cast<BSPSector*>( *sectorI )->_atomicsInSector.find( this );
                    assert( atomicI != reinterpret_cast<BSPSector*>( *sectorI )->_atomicsInSector.end() );
                    reinterpret_cast<BSPSector*>( *sectorI )->_atomicsInSector.erase( atomicI );
                }
                _sectors.clear();
                collideBSPSector( reinterpret_cast<BSP*>( _bsp )->getRoot() );
            }

            // store synchro-sphere
            _synchroSphere = _boundingSphere;
        }
    }
}

void Atomic::collideBSPSector(void* sector)
{
    // is atomic sphere intersects the bounding box of world sector?
    if( intersectionSphereAABB( 
            &_boundingSphere, 
            &reinterpret_cast<BSPSector*>( sector )->_boundingBox 
      ) )
    {
        // is this a leaf sector?
        if( reinterpret_cast<BSPSector*>( sector )->_leftSubset == NULL ) 
        {
            reinterpret_cast<BSPSector*>( sector )->_atomicsInSector.insert( this );
            _sectors.push_back( sector );
        }
        else
        {
            collideBSPSector( reinterpret_cast<BSPSector*>( sector )->_leftSubset );
            collideBSPSector( reinterpret_cast<BSPSector*>( sector )->_rightSubset );
        }
    }
}