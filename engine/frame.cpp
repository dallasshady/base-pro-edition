
#include "headers.h"
#include "frame.h"
#include "atomic.h"
#include "asset.h"

/**
 * creation routine
 */

engine::IFrame* Engine::createFrame(const char* frameName)
{
    return new Frame( frameName );
}

/**
 * class implementation
 */

unsigned int Frame::_numDirtyFrames = 0;
Frame**      Frame::_dirtyFrames = NULL;

Frame::Frame(const char* frameName)
{
    if( !frameName ) frameName = "";
    Name = new char[ strlen(frameName) + 1 ];
    strcpy( Name, frameName );
    D3DXMatrixIdentity( &TransformationMatrix );
    D3DXMatrixIdentity( &LTM );
    pParentFrame    = NULL;
    pAttachedObject = NULL;
    _dirty = false;
    pFrameFirstChild = pFrameSibling = NULL;
    pMeshContainer = NULL;
}

Frame::~Frame()
{    
    if( isDirty() ) synchronizeSafe();
    setParent( NULL );   
    while( pFrameFirstChild ) delete static_cast<Frame*>( pFrameFirstChild );
    delete[] Name;
}

/**
 * IFrame
 */

void Frame::release(void)
{
    delete this;
}

const char* Frame::getName(void)
{
    return Name;
}

Matrix4f Frame::getMatrix(void)
{
    return wrap( TransformationMatrix );
}

Matrix4f Frame::getLTM(void)
{
    if( isDirty() ) synchronizeSafe();
    return wrap( LTM );
}

Vector3f Frame::getRight(void)
{
    if( isDirty() ) synchronizeSafe();
    return wrap( dxRight( &LTM ) );
}

Vector3f Frame::getUp(void)
{
    if( isDirty() ) synchronizeSafe();
    return wrap( dxUp( &LTM ) );
}

Vector3f Frame::getAt(void)
{
    if( isDirty() ) synchronizeSafe();
    return wrap( dxAt( &LTM ) );
}

Vector3f Frame::getPos(void)
{
    if( isDirty() ) synchronizeSafe();
    return wrap( dxPos( &LTM ) );
}

void Frame::setPos(const Vector3f& pos)
{
    TransformationMatrix._41 = pos[0];
    TransformationMatrix._42 = pos[1];
    TransformationMatrix._43 = pos[2];
    TransformationMatrix._44 = 1.0f;
    dirty();
}

void Frame::setMatrix(const Matrix4f& matrix)
{
    TransformationMatrix = wrap( matrix );
    dirty();
}

void Frame::rotateRelative(const Vector3f& axis, float angle)
{
    Vector pos = dxPos( &TransformationMatrix );
    TransformationMatrix._41 = 
    TransformationMatrix._42 = 
    TransformationMatrix._43 = 0.0f;
    TransformationMatrix._44 = 1.0f;
    dxRotate( &TransformationMatrix, &wrap( axis ), angle );
    TransformationMatrix._41 = pos.x,
    TransformationMatrix._42 = pos.y,
    TransformationMatrix._43 = pos.z,
    TransformationMatrix._44 = 1.0f;
    dirty();
}

void Frame::rotate(const Vector3f& axis, float angle)
{
    dxRotate( &TransformationMatrix, &wrap( axis ), angle );
    dirty();
}

void Frame::translate(const Vector3f& translation)
{
    dxTranslate( &TransformationMatrix, &wrap( translation ) );
    dirty();
}

void Frame::setLTM(const Matrix4f& matrix)
{
    LTM = wrap( matrix );
    _dirty = false;
}

engine::IFrame* Frame::getParent(void)
{
    return pParentFrame;
}

void Frame::setParent(engine::IFrame* frame)
{
    // destroy previous relationship
    if( pParentFrame )
    {
        Frame* prevSibling = NULL;
        Frame* child = static_cast<Frame*>( pParentFrame->pFrameFirstChild );
        while( child )
        {
            // frame is found?
            if( child == this )
            {
                // break child chain, remove this frame from chain and tail the chain again
                if( prevSibling ) prevSibling->pFrameSibling = pFrameSibling;
                else pParentFrame->pFrameFirstChild = pFrameSibling;
                pFrameSibling = NULL;
                break;
            }
            // move to next child
            prevSibling = child;
            child = static_cast<Frame*>( child->pFrameSibling );
        }
    }

    // build new relationship
    pParentFrame = dynamic_cast<Frame*>( frame );
    if( pParentFrame ) 
    {
        pFrameSibling = pParentFrame->pFrameFirstChild;
        pParentFrame->pFrameFirstChild = this;        
    }
}

void Frame::forAllChildren(engine::IFrameCallBack callBack, void* data)
{
    Frame* child = static_cast<Frame*>( pFrameFirstChild );
    while( child )
    {
        if( !callBack( child, data ) ) break;
        child = static_cast<Frame*>( child->pFrameSibling );
    }
}

/**
 * module locals
 */

Frame* Frame::getRoot(void)
{
    if( pParentFrame == NULL ) return this;
    return pParentFrame->getRoot();
}

void Frame::dirty(void)
{
    if( !_dirty )
    {
        if( _numDirtyFrames == engine::maxDirtyFrames )
        {
            synchronizeSafe();
        }
        else
        {
            _dirty = true;
            _dirtyFrames[_numDirtyFrames] = this;
            _numDirtyFrames++;
        }
    }
}

void Frame::synchronizeSafe(void)
{
    if( _dirty ) for( unsigned int i=0; i<Frame::_numDirtyFrames; i++ )
    {
        if( _dirtyFrames[i] == this )
        {
            _dirtyFrames[i] = NULL;
            break;
        }
    }

    _dirty = false;

    // check parent frame is dirty
    if( pParentFrame && pParentFrame->isDirtyHierarchy() )
    {
        getRoot()->synchronizeSafe();
        return;
    }

    // synchronize this frame
    if( !pParentFrame ) LTM = TransformationMatrix;
    else dxMultiply( &LTM, &TransformationMatrix, &pParentFrame->LTM );

    // synchronize its children
    Frame* child = static_cast<Frame*>( pFrameFirstChild );
    while( child )
    {
        child->synchronizeSafe();
        child = static_cast<Frame*>( child->pFrameSibling );
    }
    // update attached atomic
    if( pAttachedObject ) pAttachedObject->onUpdate();
}

void Frame::synchronizeFast(void)
{
    _dirty = false;

    // synchronize this frame
    if( !pParentFrame ) LTM = TransformationMatrix; 
    else dxMultiply( &LTM, &TransformationMatrix, &pParentFrame->LTM );
	
    // synchronize its children
    Frame* child = static_cast<Frame*>( pFrameFirstChild );
    while( child )
    {
        child->synchronizeFast();
        child = static_cast<Frame*>( child->pFrameSibling );
    }
    // update attached object
    if( pAttachedObject ) pAttachedObject->onUpdate();
}

void Frame::synchronizeAll(void)
{
    for( unsigned int i=0; i<_numDirtyFrames; i++ )
    {
        if( _dirtyFrames[i] ) _dirtyFrames[i]->synchronizeFast();
    }
    _numDirtyFrames = 0;
}

void Frame::init(void)
{
    _dirtyFrames = new Frame*[engine::maxDirtyFrames];
}

void Frame::term(void)
{
    assert( _dirtyFrames != NULL );
    delete[] _dirtyFrames;
}

/**
 * serialization
 */

void Frame::write(IResource* resource)
{
    ChunkHeader frameHeader( BA_FRAME, sizeof( Chunk ) );
    frameHeader.write( resource );

    Chunk chunk;    
    memset( chunk.name, 0, engine::maxNameLength );
    if( strlen( Name ) > engine::maxNameLength )
    {
        strncpy( chunk.name, Name, engine::maxNameLength - 1 );
    }
    else
    {
        strcpy( chunk.name, Name );
    }
    chunk.id = (auid)( this );
    chunk.matrix = TransformationMatrix;
    chunk.parent = (auid)( pParentFrame );

    fwrite( &chunk, sizeof( chunk ), 1, resource->getFile() );
}

AssetObjectT Frame::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader frameHeader( resource );
    if( frameHeader.type != BA_FRAME ) throw Exception( "Unexpected chunk type" );
    if( frameHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, sizeof(chunk), 1, resource->getFile() );

    Frame* frame = new Frame( chunk.name );
    frame->TransformationMatrix = chunk.matrix;
    AssetObjectI assetObjectI = assetObjects.find( chunk.parent );
    if( assetObjectI != assetObjects.end() )
    {
        frame->setParent( reinterpret_cast<Frame*>( assetObjectI->second ) );
    }

    return AssetObjectT( chunk.id, frame );
}