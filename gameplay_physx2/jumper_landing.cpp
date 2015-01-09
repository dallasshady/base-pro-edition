
#include "headers.h"
#include "jumper.h"
#include "imath.h"

/**
 * related animations
 */

static engine::AnimSequence landingSequence =
{
    FRAMETIME(911),
    FRAMETIME(961),
    engine::ltNone,
    0.0f
};

static engine::AnimSequence canopySuppressionSequence =
{
    FRAMETIME(1160),
    FRAMETIME(1207),
    engine::ltNone,
    0.0f
};

/**
 * class implementation
 */

Jumper::Landing::Landing(Jumper* jumper, NxActor* phFlight, MatrixConversion* mcFlight) :
    JumperAction( jumper )
{
	if (jumper->getCanopySimulator()->isCutAway) return;

    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.2f;
    _endOfAction = false;
    _phActor = phFlight;
    _matrixConversion = mcFlight;
    _Kfr = 0.5f;

    _phActor->setAngularDamping( 2.0f );

    // animation controller
    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // remove procedural influence
    animCtrl->advance( 0.0f );
    _clump->getFrame()->getLTM();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    // setup animation
    animCtrl->setTrackAnimation( 0, &landingSequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, 0.55f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

void Jumper::Landing::update(float dt)
{
    updateAnimation( dt );

    // animation speed is linear function of motion velocity
    if( _clump->getAnimationController()->getTrackAnimation( 0 ) != &canopySuppressionSequence )
    {
        float animSpeed = _phActor->getLinearVelocity().magnitude() / 8.0f;
        if( animSpeed < 0.1f ) animSpeed = 0.1f;
        if( animSpeed > 1.0f ) animSpeed = 1.0f;
        _clump->getAnimationController()->setTrackSpeed( 0, animSpeed );
    }
    else
    {
        _clump->getAnimationController()->setTrackSpeed( 0, 1.0f );
    }

    // synchronize physics & render
    _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );
    _clump->getFrame()->getLTM();

    // check if character is stopped
    if( _clump->getAnimationController()->getTrackAnimation( 0 ) != &canopySuppressionSequence &&
        ( _clump->getAnimationController()->isEndOfAnimation( 0 ) || _phActor->getLinearVelocity().magnitude() < 0.2f ) )
    {
        startCanopySuppression();
    }
}

void Jumper::Landing::updatePhysics(void)
{
    // local coordinate system of base jumper
    NxMat34 actorPose = _phActor->getGlobalPose();
    NxVec3 x = actorPose.M.getColumn(0);
    NxVec3 y = actorPose.M.getColumn(1);
    NxVec3 z = actorPose.M.getColumn(2);

    // update character physics for landing action
    // determine angle btw. character vertical && world up vector
    Vector3f charUp = wrap( y );
    charUp.normalize();
    Vector3f worldUp = Vector3f( 0,1,0 );
    Vector3f axis;
    axis.cross( worldUp, charUp );
    axis.normalize();
    float angle = ::calcAngle( worldUp, charUp, axis );

    // when character stoop treshold angle it will stumbles
    if( fabs( angle ) > 20.0f ) this->_endOfAction = true;

    // align torque, relative to world up direction
    float Kalign = 1.0f;
    NxVec3 Talign( axis[0], axis[1], axis[2] );
    Talign = Talign * sqr( angle ) * sgn( angle ) * Kalign;

    // align torque, relative to canopy
    float Kcanopy = 0.0125f;
    NxVec3 Tcanopy( 0,0,0 );
    if( &canopySuppressionSequence == _clump->getAnimationController()->getTrackAnimation( 0 ) )
    {
        Vector3f charAt = _clump->getFrame()->getAt();
        charAt.normalize();
        Vector3f canopyPos = _jumper->getCanopySimulator()->getClump()->getFrame()->getPos();
        Vector3f charPos = _clump->getFrame()->getPos();
        Vector3f dirToCanopy = canopyPos - charPos;
        dirToCanopy.normalize();
        charAt[1] = 0; charAt.normalize();
        dirToCanopy[1] = 0; dirToCanopy.normalize();
        angle = ::calcAngle( charAt, dirToCanopy, worldUp );
        Tcanopy.set( 0,1,0 );
        Tcanopy = Tcanopy * sqr( angle ) * sgn( angle ) * -Kcanopy;
    }

    // friction force
    NxVec3 vel = _phActor->getLinearVelocity();
    engine::IAnimationController* animCtrl = _clump->getAnimationController();
    if( animCtrl->getTrackAnimation( 0 ) == &canopySuppressionSequence )
    {
        if( vel.magnitude() > 0.25f )
        {
            _Kfr += 5.0f * ::simulationStepTime;
        }
    }
    NxVec3 Ffr = vel * -_Kfr * _phActor->getMass();

    // finalize motion equation        
    _phActor->addForce( Ffr );
    _phActor->addTorque( Talign + Tcanopy );
}

void Jumper::Landing::startCanopySuppression(void)
{
    _actionTime = 0.0f;

    // change material
    assert( _phActor->getNbShapes() );
    _phActor->getShapes()[0]->setMaterial( 
        _jumper->getScene()->getPhFleshMaterial()->getMaterialIndex() 
    );

    _phActor->setAngularDamping( 3.0f );

    // animation controller
    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // remove procedural influence
    animCtrl->advance( 0.0f );
    _clump->getFrame()->getLTM();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    // setup animation
    animCtrl->setTrackAnimation( 0, &canopySuppressionSequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, 0.75f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );

    // disable wind
    _jumper->getCanopySimulator()->enableWind( false );
}

float Jumper::Landing::getDamageFactor(void)
{
    return 0.125f;
}