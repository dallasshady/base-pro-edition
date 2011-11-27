
#include "headers.h"
#include "jumper.h"
#include "imath.h"

static engine::AnimSequence sitFlyingSequence = 
{ 
    FRAMETIME(1430), 
    FRAMETIME(1466),  
    engine::ltPeriodic,
    FRAMETIME(1430)
};

/**
 * class implementation
 */

Jumper::SitFlying::SitFlying(Jumper* jumper, NxActor* phActor, MatrixConversion* mc) :
    Jumper::JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.33f;
    _phActor = phActor;
    _matrixConversion = mc;
    _endOfAction = false;

    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    // setup animation cycle
    animCtrl->setTrackAnimation( 0, &sitFlyingSequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, 0.75f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );    
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );


}

Jumper::SitFlying::~SitFlying()
{
}

/**
 * Action
 */

void Jumper::SitFlying::update(float dt)
{
    updateAnimation( dt );

    // synchronize physics & render
    _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );
    _clump->getFrame()->getLTM();
}

static float getAirResistancePower(float i)
{
    return ( exp( pow( i, 1.4f ) )  - 1.0f ) / 1.718f;
}

void Jumper::SitFlying::updatePhysics(void)
{
    // velocity of base jumper's body
    NxVec3 velocity = _phActor->getLinearVelocity();

    // vectical velocity of base jumper's body
    NxVec3 velocityV( 0, velocity.y, 0 );

    // air resistance coefficient
    float AR = 1.125f * _jumper->getVirtues()->getFrogAirResistance();

    // terminal velocity
    float Vt = sqrt( 9.8f * _phActor->getMass() / AR );
    float It = velocityV.magnitude() / Vt;

    // air resistance force
    NxVec3 Far = NxVec3(0,1,0) * getAirResistancePower( velocityV.magnitude() / Vt ) * _phActor->getMass() * 9.8f;

    // control torque
    NxVec3 Tctrl( 0,0,0 );
    Tctrl += NxVec3(1,0,0) * -_jumper->getVirtues()->getSteerPitch() * _jumper->getSpinalCord()->down;
    Tctrl += NxVec3(1,0,0) * _jumper->getVirtues()->getSteerPitch() * _jumper->getSpinalCord()->up;
    Tctrl *= It;

    // finalize motion equation    
    _phActor->addForce( Far );
    _phActor->addLocalTorque( Tctrl );
}