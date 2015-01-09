
#include "headers.h"
#include "jumper.h"
#include "imath.h"

/**
 * action properties
 */

const float walkBackVelocity = 55.0f;
const float walkTurnVelocity = 60;

/**
 * "walk backward" animation sequences
 */

static engine::AnimSequence walkBackwardSequence = 
{
    FRAMETIME(551), 
    FRAMETIME(589), 
    engine::ltPeriodic, 
    FRAMETIME(566) 
};

/**
 * class implementation
 */

Jumper::WalkBackward::WalkBackward(Jumper* jumper) : JumperAction( jumper )
{
    // set action properties
    _enclosure = jumper->getEnclosure();
    _actionTime = 0.0f;
    _blendTime = 0.1f;
    _endOfAction = false;    

    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // capture blend source
    animCtrl->captureBlendSrc();

    // setup animation cycle
    animCtrl->setTrackAnimation( 0, &walkBackwardSequence );
    if( animCtrl->getTrackAnimation( 1 ) ) 
    {
        animCtrl->setTrackActivity( 1, false );
    }
    if( animCtrl->getTrackAnimation( 2 ) )
    {
        animCtrl->setTrackActivity( 2, false );
    }
    animCtrl->setTrackActivity( 0, true );    
    animCtrl->setTrackSpeed( 0, 0.75f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

Jumper::WalkBackward::~WalkBackward()
{
    //if( _sound->isPlaying() ) _sound->stop();    
}

void Jumper::WalkBackward::update(float dt)
{
    _jumper->_isStuck = false;

    updateAnimation( dt );

    // pass blending phase
    if( _actionTime < _blendTime ) return;

    // calculate motion velocity
    Vector3f velocity = _clump->getFrame()->getAt();
    if( _actionTime - _blendTime < FRAMETIME(566)-FRAMETIME(551) )
    {
         velocity *= walkBackVelocity * _actionTime / (FRAMETIME(566)-FRAMETIME(551));
    }
    else
    {
        velocity *= walkBackVelocity;
    }

    // move
    Vector3f direction = velocity * dt * -1;
    Vector3f pos = _clump->getFrame()->getPos() + Vector3f( 0, jumperRoamingSphereSize, 0 );
    Vector3f lastPos = pos;
    pos = _enclosure->move( pos, direction, jumperRoamingSphereSize );
    Vector3f actualDistance = _enclosure->getActualDistance();

    // slowly move down until collision
    direction.set( 0,-1,0 );
    unsigned int limit = 200;
    do
    {
        pos = _enclosure->move( pos, direction, jumperRoamingSphereSize );
        limit--;
    }
    while( limit > 0 && _enclosure->getLastNumIntersectedTriangles() == 0 );
    if( limit == 0 )
    {
        pos = lastPos;
        actualDistance.set( 0,0,0 );
    }

    pos += Vector3f( 0, -jumperRoamingSphereSize, 0 );
    _clump->getFrame()->setPos( pos );
    
    engine::IAnimationController* animCtrl = _clump->getAnimationController(); 

    // check actual velocity of roaming engine
    float pitchShiftFactor = 1.0f;
    if( velocity.length() > 0 && dt > 0 )
    {
        float braking = actualDistance.length() / (velocity * dt).length();
        animCtrl->setTrackSpeed( 0, braking * 0.75f );
        if( braking < 0.5f ) 
        {
            _endOfAction = true;
            _jumper->_isStuck = true;
        }
        pitchShiftFactor *= braking;
    }

    // update sound properties
    _jumper->happen( _jumper, EVENT_JUMPER_PITCHSHIFT, &pitchShiftFactor );

    // rotate character
    if( !_endOfAction )
    {
        float turnSgn = _jumper->getSpinalCord()->right - _jumper->getSpinalCord()->left;
        float angleToTurn = -walkTurnVelocity * dt * turnSgn;
        _clump->getFrame()->rotateRelative( Vector3f( 0,1,0 ), angleToTurn );
    }
}