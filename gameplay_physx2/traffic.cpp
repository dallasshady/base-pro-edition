
#include "headers.h"
#include "traffic.h"

/**
 * actor abstracts
 */

void Traffic::onUpdateActivity(float dt)
{
    _clump->getAnimationController()->advance( dt );    
    if( _clump->getAnimationController()->isEndOfAnimation( 0 ) )
    {
        _clump->getAnimationController()->resetTrackTime( 0 );
    }
}

Traffic::Traffic(Actor* parent, TrafficDesc* desc) : Actor( parent )
{
    assert( desc );
    assert( desc->source );

    _desc = *desc;
    
    // clone
    _clump = _desc.source->clone( "TrafficObject" ); assert( _clump );

    // setup animation
    engine::AnimSequence* animSequence = _clump->getAnimationController()->getDefaultAnimation();
    animSequence->loopType = engine::ltNone;
    animSequence->loopStartTime = 0.0f;    
    _clump->getAnimationController()->resetTrackTime( 0 );
    _clump->getAnimationController()->advance( ( animSequence->endTime - animSequence->startTime ) * float( _desc.animOffset ) );
    _clump->getAnimationController()->setTrackSpeed( 0, _desc.animSpeed );

    // add to stage
    _scene->getStage()->add( _clump );
}

Traffic::~Traffic()
{
    if( _clump )
    {
        _scene->getStage()->remove( _clump );
        _clump->release();
    }
}