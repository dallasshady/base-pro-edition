
#include "headers.h"
#include "frame.h"
#include "atomic.h"
#include "asset.h"

/**
 * creation routine
 */

engine::IFrameSnapshot* Engine::createFrameSnapshot(engine::IFrame* rootFrame)
{
    return new FrameSnapshot( rootFrame );
}

/**
 * class implementation
 */

engine::IFrame* FrameSnapshot::makeFrameSnapshotCB(engine::IFrame* frame, void* data)
{
    Frame* f = dynamic_cast<Frame*>( frame );
    Snapshot snapshot;

    if( f->isDirty() ) f->synchronizeSafe();

    snapshot.scale = Vector( 
        D3DXVec3Length( &dxRight( &f->TransformationMatrix ) ),
        D3DXVec3Length( &dxUp( &f->TransformationMatrix ) ),
        D3DXVec3Length( &dxAt( &f->TransformationMatrix ) )
    );
    snapshot.translation = dxPos( &f->TransformationMatrix );
    
    Matrix pr = f->TransformationMatrix;
    pr._11 /= snapshot.scale.x, pr._12 /= snapshot.scale.x, pr._13 /= snapshot.scale.x, pr._14 = 0.0f;
    pr._21 /= snapshot.scale.y, pr._22 /= snapshot.scale.y, pr._23 /= snapshot.scale.y, pr._24 = 0.0f;
    pr._31 /= snapshot.scale.z, pr._32 /= snapshot.scale.z, pr._33 /= snapshot.scale.z, pr._34 = 0.0f;
    pr._41 = 0.0f, pr._42 = 0.0f, pr._43 = 0.0f, pr._44 = 1.0f;
    D3DXQuaternionRotationMatrix(
        &snapshot.rotation,
        &pr
    );
    D3DXQuaternionNormalize( &snapshot.rotation, &snapshot.rotation );
    
    reinterpret_cast<FrameSnapshot*>( data )->_snapshotM.insert( SnapshotT( f, snapshot ) );

    frame->forAllChildren( makeFrameSnapshotCB, data );

    return frame;
}

FrameSnapshot::FrameSnapshot(engine::IFrame* frame)
{
    makeFrameSnapshotCB( frame, this );
}

FrameSnapshot::~FrameSnapshot()
{
}

void FrameSnapshot::release(void)
{
    delete this;
}

void FrameSnapshot::blend(engine::IFrameSnapshot* destState, float interpolator)
{
    Snapshot  blendshot;
    Matrix    r,s,rs,t;
    SnapshotI srcI, dstI;
    FrameSnapshot* dest = dynamic_cast<FrameSnapshot*>( destState ); 
    
    assert( dest );
    assert( dest->_snapshotM.size() == _snapshotM.size() );
    assert( dest->_snapshotM.begin()->first == _snapshotM.begin()->first );

    // trim interpolator range
    if( interpolator < 0.0f ) interpolator = 0.0f;
    if( interpolator > 1.0f ) interpolator = 1.0f;

    // blend frames
    srcI = _snapshotM.begin();
    dstI = dest->_snapshotM.begin();
    while( srcI != _snapshotM.end() )
    {
        D3DXVec3Lerp( &blendshot.scale, &srcI->second.scale, &dstI->second.scale, interpolator );        
        D3DXVec3Lerp( &blendshot.translation, &srcI->second.translation, &dstI->second.translation, interpolator );        
        D3DXQuaternionSlerp( &blendshot.rotation, &srcI->second.rotation, &dstI->second.rotation, interpolator );
        
        D3DXMatrixRotationQuaternion( &r, &blendshot.rotation );
        D3DXMatrixScaling( &s, blendshot.scale.x, blendshot.scale.y, blendshot.scale.z );
        D3DXMatrixTranslation( &t, blendshot.translation.x, blendshot.translation.y, blendshot.translation.z );
        dxMultiply( &rs, &s, &r );
        dxMultiply( &srcI->first->TransformationMatrix, &rs, &t );

        srcI->first->dirty();

        srcI++, dstI++;
        assert( srcI->first == dstI->first );
    }
}