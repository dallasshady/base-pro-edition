
#include "headers.h"
#include "animobject.h"

/**
 * class implementation
 */
   
AnimatedObject::AnimatedObject(Actor* parent, AnimatedObjectDesc* desc) : Actor( parent )
{
    assert( desc->source );
    assert( desc->map );
    assert( desc->animDistance > 0 );
    assert( desc->animSpeed > 0 );

    _name = "AnimatedObject";
    _desc = *desc;

    engine::IClump* clone;
    callback::AtomicL atomics;
    _desc.map->forAllAtomics( callback::enumerateAtomics, &atomics );
    for( callback::AtomicI atomicI=atomics.begin(); atomicI!=atomics.end(); atomicI++ )
    {
        clone = _desc.source->clone( "AnimatedObjectClone" );
        clone->getFrame()->setMatrix( (*atomicI)->getFrame()->getLTM() );
        clone->getAnimationController()->setTrackSpeed( 0, _desc.animSpeed );
        _scene->getStage()->add( clone );
        _objects.push_back( clone );
        (*atomicI)->setFlags( 0 );
    }
}

AnimatedObject::~AnimatedObject()
{
    for( callback::ClumpI clumpI=_objects.begin(); clumpI!=_objects.end(); clumpI++ )
    {
        _scene->getStage()->remove( (*clumpI) );
        (*clumpI)->release();
    }
}

/**
 * actor abstracts
 */

void AnimatedObject::onUpdateActivity(float dt)
{
    Actor* cameraActor = _scene->getCamera();
    if( !cameraActor ) return;

    Matrix4f pose = cameraActor->getPose();
    Vector3f cameraPos( pose[3][0], pose[3][1], pose[3][2] );

    Vector3f objectPos;
    for( callback::ClumpI clumpI=_objects.begin(); clumpI!=_objects.end(); clumpI++ )
    {
        objectPos = (*clumpI)->getFrame()->getPos();
        if( (objectPos-cameraPos).length() < _desc.animDistance )
        {
            (*clumpI)->getAnimationController()->advance( dt );
        }
    }
}