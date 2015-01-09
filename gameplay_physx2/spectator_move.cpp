
#include "headers.h"
#include "crowd.h"
#include "imath.h"

/**
 * animation sequences
 */

static engine::AnimSequence walkSequence = 
{ 
    FRAMETIME(60), 
    FRAMETIME(107), 
    engine::ltPeriodic, 
    FRAMETIME(84) 
};

static engine::AnimSequence runSequence = 
{ 
    FRAMETIME(253), 
    FRAMETIME(300), 
    engine::ltPeriodic, 
    FRAMETIME(281) 
};

/**
 * action properties
 */

const float walkVelocity = 150.0f;
const float runVelocity = 500.0f;
const float movePrecision = 100.0f;

/**
 * action
 */

Spectator::Move::Move(engine::IClump* clump, Enclosure* enclosure, const Vector3f& pos, bool fast) :
    Character::Action( clump )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.1f;
    _endOfAction = false;
    _enclosure = enclosure;
    _fast = fast;
    _pos = pos;

    // use animation controller
    engine::IAnimationController* controller = _clump->getAnimationController();

    // capture blend source
    controller->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( controller->getTrackAnimation( i ) ) controller->setTrackActivity( i, false );
    }

    if( _fast )
    {
        controller->setTrackAnimation( 0, &runSequence );
        controller->setTrackSpeed( 0, 1.0f );
    }
    else
    {
        controller->setTrackAnimation( 0, &walkSequence );
        controller->setTrackSpeed( 0, 0.75f );
    }
    controller->setTrackWeight( 0, 1.0f );
    controller->setTrackActivity( 0, true );
    controller->resetTrackTime( 0 );

    // capture blend destination
    controller->advance( 0.0f );
    controller->captureBlendDst();
    controller->blend( 0.0f );
}

void Spectator::Move::update(float dt)
{   
    updateAnimation( dt );

    // leave blending phase
    if( _actionTime < _blendTime ) return;

    // determine distance to desired position
    Vector3f distance = _pos - _clump->getFrame()->getPos();
    if( distance.length() < movePrecision )
    {
        _endOfAction = true;
        return;
    }

    // calculate motion velocity
    float vel;
    if( _fast )
    {
        if( _actionTime - _blendTime < FRAMETIME(281)-FRAMETIME(253) )
        {
            vel = runVelocity * _actionTime/(FRAMETIME(281)-FRAMETIME(253));
        }
        else
        {
            vel = runVelocity;
        }
    }
    else
    {
        if( _actionTime - _blendTime < FRAMETIME(84)-FRAMETIME(60) )
        {
            vel = walkVelocity * _actionTime / (FRAMETIME(84)-FRAMETIME(60));
        }
        else
        {
            vel = walkVelocity;
        }
    }

    // spectator properties
    float width  = 25;
    float height = 180;

    // retrieve current clump position, and raise to the height of spectator
    Vector3f pos = _clump->getFrame()->getPos();
    pos += Vector3f( 0,1,0 ) * height;

    // direction vector
    Vector3f dir = _clump->getFrame()->getAt() * vel * dt;
    dir += Vector3f( 0,-900,0 ) * dt;

    // move it along direction of clump
    pos = _enclosure->move( pos, dir, width, height );

    // lower position to the ground
    pos -= Vector3f( 0,1,0 ) * height;

    // determine actual motion distance at this act
    Vector3f AMD = pos - _clump->getFrame()->getPos();
    if( AMD.length() < ( _clump->getFrame()->getAt() * vel * dt * 0.75f ).length() )
    {
        _endOfAction = true;
    }

    // setup position for clump 
    _clump->getFrame()->setPos( pos );

    // determine direction to destination point
    dir = _pos - pos;
    dir[1] = 0.0f; dir.normalize();

    // rotate clump if it is needed
    float angle = calcAngle( dir, _clump->getFrame()->getAt(), Vector3f( 0,1,0 ) );
    if( fabs( angle ) > 1.0f )
    {
        _clump->getFrame()->rotateRelative( Vector3f(0,1,0), angle );
    }
}