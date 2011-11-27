
#include "headers.h"
#include "crowd.h"
#include "imath.h"

/**
 * spectatior turn action properties
 */

const float turnBlendTime = 0.5f;
const float turnVelocity = 90.0f;

/**
 * turn animation sequences
 */

static engine::AnimSequence turnLeftSequence = 
{ 
    FRAMETIME(135), 
    FRAMETIME(168), 
    engine::ltPeriodic, 
    FRAMETIME(144) 
};

static engine::AnimSequence turnRightSequence = 
{ 
    FRAMETIME(180), 
    FRAMETIME(213), 
    engine::ltPeriodic, 
    FRAMETIME(189) 
};

/**
 * turn action for spectator
 */

Spectator::Turn::Turn(engine::IClump* clump, Vector3f dir) :
    Character::Action( clump )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = turnBlendTime;
    _endOfAction = false;
    _dir = dir;

    // animation controller
    engine::IAnimationController* controller = _clump->getAnimationController();

    // capture blend source
    controller->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( controller->getTrackAnimation( i ) ) controller->setTrackActivity( i, false );
    }

    // calculate angle between current direction & desired direction
    float angle = calcAngle( _dir, _clump->getFrame()->getAt(), Vector3f(0,1,0) );

    // choose animation cycle
    if( sgn( angle ) < 0 )
    {
        controller->setTrackAnimation( 0, &turnRightSequence );
    }
    else
    {
        controller->setTrackAnimation( 0, &turnLeftSequence );
    }

    // setup animation mixing    
    controller->setTrackSpeed( 0, 1.0f );
    controller->setTrackWeight( 0, 1.0f );
    controller->resetTrackTime( 0 );
    controller->setTrackActivity( 0, true );

    // capture blend destination
    controller->advance( 0.0f );
    controller->captureBlendDst();
    controller->blend( 0.0f );
}

void Spectator::Turn::update(float dt)
{
    updateAnimation( dt );

    // leave if turn is starting
    if( _actionTime - _blendTime < FRAMETIME(143) - FRAMETIME(135) ) return;

    // evaluate end of action
    float angle = calcAngle( _dir, _clump->getFrame()->getAt(), Vector3f( 0,1,0 ) );
    if( fabs( angle ) < 1.0f ) 
    {
        _endOfAction = true;
        return;
    }

    // evaluate angle to turn
    float angleToTurn = sgn( angle ) * turnVelocity * dt;
    if( fabs( angle ) < fabs( angleToTurn ) ) angleToTurn = angle;

    // rotate frame
    _clump->getFrame()->rotateRelative( Vector3f(0,1,0), angleToTurn );
}