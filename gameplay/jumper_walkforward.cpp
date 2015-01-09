
#include "headers.h"
#include "jumper.h"
#include "imath.h"

/**
 * action properties
 */

const float walkForwardBlendTime = 0.1f;
const float walkForwardVelocity  = 150.0f;
const float walkVelocity = 125.0f;
const float runVelocity = 300.0f;
const float walkTurnVelocity = 60;

/**
 * "walk forward" animation sequences
 */

static engine::AnimSequence walkForwardSequence = 
{ 
    FRAMETIME(60), 
    FRAMETIME(107), 
    engine::ltPeriodic, 
    FRAMETIME(84) 
};

static engine::AnimSequence runForwardSequence = 
{ 
    FRAMETIME(253), 
    FRAMETIME(300), 
    engine::ltPeriodic, 
    FRAMETIME(281) 
};
/**
 * class implementation
 */

Jumper::WalkForward::WalkForward(Jumper* jumper) : JumperAction( jumper )
{
    // set action properties
    _enclosure = jumper->getEnclosure();
    _actionTime = 0.0f;
    _blendTime = walkForwardBlendTime;
    _endOfAction = false;
    _velocity.set( 0,0,0 );
    _running = false;

    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    // setup animation cycle
    animCtrl->setTrackAnimation( 0, &walkForwardSequence );
    animCtrl->setTrackAnimation( 1, &runForwardSequence );    
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackActivity( 1, true );
    animCtrl->setTrackSpeed( 0, 0.75f );
    animCtrl->setTrackSpeed( 1, 1.0f );
    if( _jumper->getSpinalCord()->modifier /*&& 
        !database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit*/ )
    {
        animCtrl->setTrackWeight( 0, 0.0f );
        animCtrl->setTrackWeight( 1, 1.0f );
    }
    else
    {
        animCtrl->setTrackWeight( 0, 1.0f );
        animCtrl->setTrackWeight( 1, 0.0f );
    }
    animCtrl->resetTrackTime( 0 );
    animCtrl->resetTrackTime( 1 );
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

Jumper::WalkForward::~WalkForward()
{
}

/**
 * Action implementation
 */

void Jumper::WalkForward::update(float dt)
{
    _jumper->_isStuck = false;

    updateAnimation( dt );

    // pass blending phase
    if( _actionTime < _blendTime ) return;

    // calculate motion velocity
    float walkVel;
    float runVel;
    if( _actionTime - _blendTime < FRAMETIME(84)-FRAMETIME(60) )
    {
         walkVel = walkVelocity * _actionTime / (FRAMETIME(84)-FRAMETIME(60));
    }
    else
    {
        walkVel = walkVelocity;
    }
    if( _actionTime - _blendTime < FRAMETIME(281)-FRAMETIME(253) )
    {
        runVel = runVelocity * _actionTime/(FRAMETIME(281)-FRAMETIME(253));
    }
    else
    {
        runVel = runVelocity;
    }

    engine::IAnimationController* animCtrl = _clump->getAnimationController(); 

    // jumper can run
    _running = _jumper->getSpinalCord()->modifier > 0.0f; /*&& !database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit*/;

    // speedup & final velocity
    _velocity = _clump->getFrame()->getAt();
    float pitchShiftFactor = 1.0f;
    if( _running && animCtrl->getTrackWeight( 0 ) == 0 )
    {
        _velocity *= runVel;
        pitchShiftFactor = 2.0f;
    }
    if( _running && animCtrl->getTrackWeight( 0 ) > 0 )
    {
        float weight = animCtrl->getTrackWeight( 0 ) - dt;
        if( weight < 0 ) weight = 0;
        animCtrl->setTrackWeight( 0, weight );
        animCtrl->setTrackWeight( 1, 1-weight );
        pitchShiftFactor += 1-weight;
        _velocity *= ( walkVel + ( runVel - walkVel ) * ( 1-weight ) );
    }
    if( !_running && animCtrl->getTrackWeight( 1 ) == 0 )
    {
        _velocity *= walkVel;
    }
    if( !_running && animCtrl->getTrackWeight( 1 ) > 0 )
    {
        float weight = animCtrl->getTrackWeight( 1 ) - dt;
        if( weight < 0 ) weight = 0;
        animCtrl->setTrackWeight( 0, 1-weight );
        animCtrl->setTrackWeight( 1, weight );
        pitchShiftFactor += weight;
        _velocity *= ( runVel + ( walkVel - runVel ) * ( 1-weight ) );
    }

    // move along direction
    Vector3f direction = _velocity * dt;
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
    while( limit>0 && _enclosure->getLastNumIntersectedTriangles() == 0 );
    if( limit == 0 )
    {
        pos = lastPos;
        actualDistance.set( 0,0,0 );
    }

    pos += Vector3f( 0, -jumperRoamingSphereSize, 0 );
    _clump->getFrame()->setPos( pos );

    // check actual velocity of roaming engine
    if( _velocity.length() > 0 && dt > 0 )
    {
        float braking = actualDistance.length() / ( _velocity * dt ).length();
        animCtrl->setTrackSpeed( 0, braking * 0.75f );
        animCtrl->setTrackSpeed( 1, braking );
        pitchShiftFactor *= braking;
        if( braking < 0.25f ) 
        {
            _endOfAction = true;
            _jumper->_isStuck = true;
        }
    }

    // modify pitch shift factor
    _jumper->happen( _jumper, EVENT_JUMPER_PITCHSHIFT, &pitchShiftFactor );

    // rotate character
    if( !_endOfAction )
    {
        ActionChannel* left = Gameplay::iGameplay->getActionChannel( iaLeft );
        ActionChannel* right = Gameplay::iGameplay->getActionChannel( iaRight );
        float turn = -left->getAmplitude() + right->getAmplitude();
        float angleToTurn = -walkTurnVelocity * dt * turn;
        _clump->getFrame()->rotateRelative( Vector3f( 0,1,0 ), angleToTurn );
    }
}

float Jumper::WalkForward::getVelocity(void)
{
    return _velocity.length();
}