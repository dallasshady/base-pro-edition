
#include "headers.h"
#include "clump.h"
#include "bsp.h"
#include "asset.h"
#include "camera.h"

/**
 * creation routine
 */

engine::IClump* Engine::createClump(const char* clumpName)
{
    return new Clump( clumpName );
}

/**
 * class implementation
 */

Clump::Clump(const char* clumpName)
{
    _name           = clumpName;
    _frame          = NULL;
    _bsp            = NULL;
    _animController = NULL;
    _hasLODs        = false;
}

Clump::~Clump()
{
    if( _bsp )
    {
        reinterpret_cast<BSP*>( _bsp )->remove( this );
    }

    // first, release atomics
    while( _atomics.size() )
    {
        Atomic* atomic = *_atomics.begin();
        _atomics.erase( _atomics.begin() );
        atomic->release();
    }

    // second, release lights
    while( _lights.size() )
    {
        Light* light = *_lights.begin();
        _lights.erase( _lights.begin() );
        light->release();
    }

    // third, release animation controller
    if( _animController ) delete _animController;

    // last, release frame hierarchy
    if( _frame ) _frame->release();
}

/**
 * IClump
 */

const char* Clump::getName(void)
{
    return _name.c_str();
}

void Clump::setName(const char* name)
{
    assert( name );
    _name = name;
}

engine::IBSP* Clump::getBSP(void)
{
    return reinterpret_cast<BSP*>( _bsp );
}

void Clump::release(void)
{
    delete this;
}

engine::IFrame* Clump::getFrame(void)
{
    return _frame;
}

void Clump::setFrame(engine::IFrame* frame)
{
    _frame = dynamic_cast<Frame*>( frame );
}

void Clump::add(engine::IAtomic* atomic)
{
    Atomic* a = dynamic_cast<Atomic*>( atomic );
    if( a ) _atomics.push_back( a );
}

void Clump::remove(engine::IAtomic* atomic)
{
    Atomic* a = dynamic_cast<Atomic*>( atomic );
    for( AtomicI atomicI = _atomics.begin();
                 atomicI != _atomics.end();
                 atomicI++ )
    {
        if( *atomicI == a )
        {
            _atomics.erase( atomicI );
            break;
        }
    }
}

void Clump::add(engine::ILight* light)
{
    Light* l = dynamic_cast<Light*>( light );
    if( l ) _lights.push_back( l );
}

void Clump::remove(engine::ILight* light)
{
    Light* l = dynamic_cast<Light*>( light );
    for( LightI lightI = _lights.begin(); lightI != _lights.end(); lightI++ )
    {
        if( *lightI == l )
        {
            _lights.erase( lightI );
            break;
        }
    }
}

void Clump::forAllAtomics(engine::IAtomicCallBack callBack, void* data)
{
    for( AtomicI atomicI = _atomics.begin(); atomicI != _atomics.end(); atomicI++ )
    {
        if( !callBack( *atomicI, data ) ) break;
    }
}

void Clump::forAllLights(engine::ILightCallBack callBack, void* data)
{
    for( LightI lightI = _lights.begin(); lightI != _lights.end(); lightI++ )
    {
        if( !callBack( *lightI, data ) ) break;
    }
}

void Clump::render(void)
{
    for( AtomicI atomicI = _atomics.begin();
                 atomicI != _atomics.end();
                 atomicI++ )
    {
        (*atomicI)->render();
    }
}

engine::IClump* Clump::clone(const char* cloneName)
{
    Clump* result = new Clump( cloneName );
    
    // process hierarchy, clone frames, atomics and light sources
    cloneFrameHierarchy( _frame, result );
    
    // clone attached objects
    cloneAttachedObjects( result );
    
    // setup animation
    if( _animController ) result->setAnimation( _animController->getAnimationSet() );

    // end of cloning
    return result;
}

void Clump::cloneFrameHierarchy(Frame* frame, Clump* clone)
{
    Frame* clonedFrame = new Frame( frame->getName() );
    clonedFrame->setMatrix( frame->getMatrix() );
    
    if( frame->pParentFrame == NULL ) 
    {
        clone->setFrame( clonedFrame );
    }
    else
    {
        Frame* parentFrame = clone->frame()->findFrame( frame->pParentFrame->Name );
        assert( parentFrame );
        clonedFrame->setParent( parentFrame );
    }

    if( frame->pFrameSibling ) 
    {
        cloneFrameHierarchy( static_cast<Frame*>( frame->pFrameSibling ), clone );
    }
    if( frame->pFrameFirstChild )
    {
        cloneFrameHierarchy( static_cast<Frame*>( frame->pFrameFirstChild ), clone );
    }
}

void Clump::cloneAttachedObjects(Clump* clone)
{
    Light*  clonedLight;
    Atomic* clonedAtomic;
    Frame*  originalFrame;
    Frame*  clonedFrame;
    for( AtomicI atomicI = _atomics.begin(); atomicI != _atomics.end(); atomicI++ )
    {
        originalFrame = (*atomicI)->frame(); assert( originalFrame );
        clonedFrame = clone->frame()->findFrame( originalFrame->Name ); assert( clonedFrame );
        clonedAtomic = (*atomicI)->clone();
        clonedAtomic->setFrame( clonedFrame );
        clone->add( clonedAtomic );
    }
    for( LightI lightI = _lights.begin(); lightI != _lights.end(); lightI++ )
    {
        originalFrame = (*lightI)->frame(); assert( originalFrame );
        clonedFrame = clone->frame()->findFrame( originalFrame->Name ); assert( clonedFrame );
        clonedLight = (*lightI)->clone();
        clonedLight->setFrame( clonedFrame );
        clone->add( clonedLight );
    }
}

void Clump::setLOD(engine::IAtomic* atomic, float maxDistance, float minDistance)
{
    Atomic* a = dynamic_cast<Atomic*>( atomic ); assert( a );

    // check clump has this atomic
    bool itHasAtomic = false;
    for( AtomicI atomicI = _atomics.begin(); atomicI != _atomics.end(); atomicI++ )
    {
        if( (*atomicI) == a ) 
        {
            itHasAtomic = true;
            break;
        }
    }
    assert( itHasAtomic );
    if( !itHasAtomic ) return;

    // setup lod distances
    a->setLOD( maxDistance, minDistance );

    // mark clump as LOD manager
    _hasLODs = true;
}

/**
 * serialization
 */

engine::IFrame* Clump::collectFrameCB(engine::IFrame* frame, void* data)
{
    reinterpret_cast<std::list<Frame*>*>( data )->push_back( dynamic_cast<Frame*>( frame ) );
    frame->forAllChildren( collectFrameCB, data );
    return frame;
}

void Clump::write(IResource* resource)
{
    ChunkHeader clumpHeader( BA_CLUMP, sizeof( Chunk ) );
    clumpHeader.write( resource );

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
    chunk.numAtomics = _atomics.size();
    chunk.numLights = _lights.size();

    // collect frames
    std::list<Frame*> frames;
    if( _frame ) collectFrameCB( _frame, &frames );
    chunk.numFrames = frames.size();

    // collect geometries
    std::list<Geometry*>           geometries;
    std::list<Geometry*>::iterator geometryI;
    for( Clump::AtomicI atomicI = _atomics.begin();
                        atomicI != _atomics.end();
                        atomicI++ )
    {
        Geometry* geometry = dynamic_cast<Geometry*>( (*atomicI)->getGeometry() );
        bool alreadyInList = false;
        for( geometryI = geometries.begin(); geometryI != geometries.end(); geometryI++ )
        {
            if( (*geometryI) == geometry )
            {
                alreadyInList = true;
                break;
            }
        }
        if( !alreadyInList ) geometries.push_back( geometry );
    }
    chunk.numGeometries = geometries.size();

    // write clump chunk
    fwrite( &chunk, sizeof(Chunk), 1, resource->getFile() );

    // write frames
    for( std::list<Frame*>::iterator frameI = frames.begin();
                                     frameI != frames.end();
                                     frameI++ )
    {
        (*frameI)->write( resource );
    }    

    // write geometries
    for( geometryI = geometries.begin(); geometryI != geometries.end(); geometryI++ )
    {
        (*geometryI)->write( resource );
    }

    // write atomics
    for( atomicI = _atomics.begin(); atomicI != _atomics.end(); atomicI++ )
    {
        (*atomicI)->write( resource );
    }

    // write lights
    for( LightI lightI = _lights.begin(); lightI != _lights.end(); lightI++ )
    {
        (*lightI)->write( resource );
    }
}

AssetObjectT Clump::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader clumpHeader( resource );
    if( clumpHeader.type != BA_CLUMP ) throw Exception( "Unexpected chunk type" );
    if( clumpHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, sizeof(chunk), 1, resource->getFile() );

    Clump* clump = new Clump( chunk.name );

    // read frames
    for( int i=0; i<chunk.numFrames; i++ )
    {
        AssetObjectT assetObjectT = Frame::read( resource, assetObjects );
        if( i == 0 ) 
        {
            clump->setFrame( reinterpret_cast<Frame*>( assetObjectT.second ) );
            reinterpret_cast<Frame*>( assetObjectT.second )->dirty();
        }
        assetObjects.insert( assetObjectT );
    }    

    // read geometries
    for( i=0; i<chunk.numGeometries; i++ )
    {
        AssetObjectT assetObjectT = Geometry::read( resource, assetObjects );
        reinterpret_cast<Geometry*>( assetObjectT.second )->instance();
        assetObjects.insert( assetObjectT );
    }

    // read atomics
    for( i=0; i<chunk.numAtomics; i++ )
    {
        AssetObjectT assetObjectT = Atomic::read( resource, assetObjects );
        assetObjects.insert( assetObjectT );
        clump->add( reinterpret_cast<Atomic*>( assetObjectT.second ) );
    }

    // read lights
    for( i=0; i<chunk.numLights; i++ )
    {
        AssetObjectT assetObjectT = Light::read( resource, assetObjects );
        assetObjects.insert( assetObjectT );
        clump->add( reinterpret_cast<Light*>( assetObjectT.second ) );
    }

    return AssetObjectT( chunk.id, clump );
}

static Vector _clumpPos;
static Vector _distance;
static float  _length;

void Clump::updateLODs(void)
{
    if( !_hasLODs ) return;

    // use clump root frame as measurement point
    _clumpPos.x = _frame->LTM._41,
    _clumpPos.y = _frame->LTM._42,
    _clumpPos.z = _frame->LTM._43;
    
    // measure distance between clump & eye
    D3DXVec3Subtract( &_distance, &Camera::eyePos, &_clumpPos );
    _length = D3DXVec3Length( &_distance );

    // process atomics
    unsigned int flags;
    for( AtomicI atomicI = _atomics.begin(); atomicI != _atomics.end(); atomicI++ )
    {
        if( (*atomicI)->isLODEnabled() )
        {
            flags = (*atomicI)->flags();
            if( _length <= (*atomicI)->getLODMaxDistance() &&
                _length >= (*atomicI)->getLODMinDistance() )
            {
                flags = flags | engine::afRender;
            }
            else 
            {
                flags = flags & ~engine::afRender;
            }
            if( (*atomicI)->flags() != flags ) (*atomicI)->setFlags( flags );
        }
    }
}