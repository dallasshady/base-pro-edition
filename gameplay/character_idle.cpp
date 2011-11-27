
#include "headers.h"
#include "character.h"

Character::Idle::Idle(engine::IClump* clump, engine::AnimSequence* sequence, float blendTime, float animSpeed) :
    Character::Action( clump )
{
    _blendTime = blendTime;

    engine::IAnimationController* controller = _clump->getAnimationController();

    // capture blend source
    controller->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( controller->getTrackAnimation( i ) ) controller->setTrackActivity( i, false );
    }

    // setup animation sequence
    controller->setTrackAnimation( 0, sequence );
    controller->setTrackSpeed( 0, animSpeed );
    controller->setTrackWeight( 0, 1.0f );    
    controller->setTrackActivity( 0, true );
    controller->resetTrackTime( 0 );

    // update animation
    controller->advance( 0.0f );

    // capture blend destination
    controller->captureBlendDst();
    controller->blend( 0.0f );
}

void Character::Idle::update(float dt)
{
    updateAnimation( dt );
}