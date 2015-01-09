
#include "headers.h"
#include "jumper.h"
#include "imath.h"

/**
 * jumper turn action properties
 */

const float turnBlendTime = 0.1f;
const float turnVelocity = 120.0f;

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
 * turn action for jumper
 */

Jumper::Turn::Turn(Jumper* jumper, float side) : JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = turnBlendTime;
    _endOfAction = false;
    _side = side;

    // animation controller
    engine::IAnimationController* controller = _clump->getAnimationController();

    // capture blend source
    controller->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( controller->getTrackAnimation( i ) ) controller->setTrackActivity( i, false );
    }

    // choose animation cycle
    if( sgn( side ) < 0 )
    {
        controller->setTrackAnimation( 0, &turnLeftSequence );
    }
    else
    {
        controller->setTrackAnimation( 0, &turnRightSequence );
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

Jumper::Turn::~Turn()
{
    //_sound->stop();
}

void Jumper::Turn::update(float dt)
{
    updateAnimation( dt );

    // leave if turn is starting
    if( _actionTime - _blendTime < FRAMETIME(143) - FRAMETIME(135) ) return;

    // evaluate angle to turn
    float amplitude = _jumper->getSpinalCord()->right + _jumper->getSpinalCord()->left;
    float angleToTurn = -sgn( _side ) * turnVelocity * dt * amplitude;
    _clump->getAnimationController()->setTrackSpeed( 0, amplitude );

    // rotate frame
    _clump->getFrame()->rotateRelative( Vector3f(0,1,0), angleToTurn );

    // check turn status
    if( sgn( _jumper->getSpinalCord()->right - _jumper->getSpinalCord()->left ) != sgn( _side ) ) 
    {
        _endOfAction = true;
    }
}