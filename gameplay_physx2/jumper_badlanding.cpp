
#include "headers.h"
#include "jumper.h"
#include "imath.h"

/**
 * related animations
 */

static engine::AnimSequence crashSequence =
{
    FRAMETIME(1114),
    FRAMETIME(1158),
    engine::ltPeriodic,
    FRAMETIME(1114)
};

/**
 * class implementation
 */

Jumper::BadLanding::BadLanding(Jumper* jumper, NxActor* phFlight, MatrixConversion* mcFlight) :
    JumperAction( jumper )
{
	getCore()->logMessage("BAD LANDING");
	if (jumper->getCanopySimulator()->isCutAway) return;
	getCore()->logMessage("BAD LANDING1");
    // set action properties    
    _actionTime = 0.0f;
    _blendTime = 0.2f;
    _endOfAction = false;
    _phActor = phFlight;
    _matrixConversion = mcFlight;
    _jumper->getCanopySimulator()->enableWind( false );
    _Kfr = 0.5f;
	getCore()->logMessage("BAD LANDING2");
    // set fixed flesh material for this actor
    assert( _phActor->getNbShapes() );
    NxShape** shapes = _phActor->getShapes();
    shapes[0]->setMaterial( jumper->getScene()->getPhFleshMaterial()->getMaterialIndex() );
	getCore()->logMessage("BAD LANDING3");
    _phActor->setAngularDamping( 5.0f );
    _phActor->setLinearDamping( 0.1f );
	getCore()->logMessage("BAD LANDING4");
    // animation controller
    engine::IAnimationController* animCtrl = _clump->getAnimationController();
	getCore()->logMessage("BAD LANDING5");
    // remove procedural influence
    animCtrl->advance( 0.0f );
    _clump->getFrame()->getLTM();

    // capture blend source
    animCtrl->captureBlendSrc();
	getCore()->logMessage("BAD LANDING6");
    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }
	getCore()->logMessage("BAD LANDING7");
    // setup animation
    animCtrl->setTrackAnimation( 0, &crashSequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, 1.0f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->advance( 0.0f );
	getCore()->logMessage("BAD LANDING8");
    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
	getCore()->logMessage("BAD LANDING9");
}

void Jumper::BadLanding::update(float dt)
{
    updateAnimation( dt );

    float velocity = _phActor->getLinearVelocity().magnitude();

    // animation speed is linear function of motion velocity
    float animSpeed = velocity / 6.0f;
    if( animSpeed > 1.0f ) animSpeed = 1.0f;
    if( animSpeed < 0.1f ) animSpeed = 0.1f;
    _clump->getAnimationController()->setTrackSpeed( 0, animSpeed );

    // synchronize physics & render
    _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );
    _clump->getFrame()->getLTM();
}

void Jumper::BadLanding::updatePhysics(void)
{
    // update character physics
    Vector3f charAt = _clump->getFrame()->getAt(); charAt.normalize();
    Vector3f worldUp = Vector3f(0,1,0);
    Vector3f axis = _clump->getFrame()->getUp(); axis.normalize();
    float angle = ::calcAngle( worldUp, charAt, axis ); 

    // align torque, relative to world up direction
    float Kalign = 0.0125f;
    NxVec3 Talign( axis[0], axis[1], axis[2] );
    Talign = Talign * sqr( angle ) * sgn( angle ) * Kalign;

    // friction force
    /*
    NxVec3 vel = _phActor->getLinearVelocity();
    if( vel.magnitude() > 0.25f )
    {
        _Kfr += 0.5f * ::simulationStepTime;
    }
    NxVec3 Ffr = vel * -_Kfr * _phActor->getMass();
    */

    // finalize motion equation
    //_phActor->addForce( Ffr );
    _phActor->addTorque( Talign );
}