
#include "headers.h"
#include "jumper.h"
#include "imath.h"

Jumper::SideStepJump::SideStepJump(Jumper* jumper, NxActor* phActor, MatrixConversion* mc, engine::AnimSequence* sequence, float animSpeed, float criticalPeriod) :
    Jumper::JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.2f;
    _phActor = phActor;
    _matrixConversion = mc;
    _criticalPeriod = criticalPeriod;
    _endOfAction = false;

    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    // setup animation track 1
    animCtrl->setTrackAnimation( 0, sequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, animSpeed );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );

    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
    _clump->getFrame()->getLTM();

    // obtain current pelvis LTM
    Matrix4f pelvisLTM = Jumper::getPelvisFrame( _jumper->getClump() )->getLTM();
    Matrix4f chestLTM  = Jumper::getChestFrame( _jumper->getClump() )->getLTM();

    // begin to track position & angle
    _prevPelvisPos.set( 
        0.5f * ( pelvisLTM[3][0] + chestLTM[3][0] ), 
        0.5f * ( pelvisLTM[3][1] + chestLTM[3][1] ), 
        0.5f * ( pelvisLTM[3][2] + chestLTM[3][2] )
    );
    _prevPelvisUp.set( pelvisLTM[1][0], pelvisLTM[1][1], pelvisLTM[1][2] );
    _prevPelvisAt.set( pelvisLTM[2][0], pelvisLTM[2][1], pelvisLTM[2][2] );
    _prevPelvisUp.normalize();
    _prevPelvisAt.normalize();
}

Jumper::SideStepJump::~SideStepJump()
{
    if( _endOfAction ) 
    {
        update( 0.0f );

        // pelvis LTM
        Matrix4f currentPelvisLTM = Jumper::getPelvisFrame( _jumper->getClump() )->getLTM();

        // chest LTM
        Matrix4f currentChestLTM = Jumper::getChestFrame( _jumper->getClump() )->getLTM();

        // place clump
        Matrix4f poseByPelvis = _jumper->getPelvisToClumpConversion()->convert( currentPelvisLTM );
        Matrix4f poseByChest  = _jumper->getChestToClumpConversion()->convert( currentChestLTM );
        _jumper->getClump()->getFrame()->setMatrix( poseByChest );
        _jumper->getClump()->getFrame()->getLTM();

        // place actor
        Matrix4f sampleLTM = Jumper::getCollisionFF( _clump )->getFrame()->getLTM();
        _phActor->setGlobalPose( wrap( sampleLTM ) );

        // wake up actor
        _phActor->wakeUp();

        // determine facial inclination angle
  //      Vector3f charAt = _clump->getFrame()->getAt(); charAt.normalize();
  //      Vector3f charRight = _clump->getFrame()->getRight(); charRight.normalize();
  //      float fiAngle = ::calcAngle( charAt, Vector3f(0,1,0), charRight );

		//_clump->getFrame()->rotate(Vector3f(0,1,0), -fiAngle);
    }
}

void Jumper::SideStepJump::update(float dt)
{
    updateAnimation( dt );
    _clump->getFrame()->getLTM();

    if( dt > 0 )
    {
        // obtain current pelvis LTM
        //Matrix4f pelvisLTM = Jumper::getPelvisFrame( _jumper->getClump() )->getLTM();
        //Matrix4f chestLTM  = Jumper::getChestFrame( _jumper->getClump() )->getLTM();

        // update physics actor properties
        //Vector3f currPelvisPos( 
        //    0.5f * ( pelvisLTM[3][0] + chestLTM[3][0] ), 
        //    0.5f * ( pelvisLTM[3][1] + chestLTM[3][1] ), 
        //    0.5f * ( pelvisLTM[3][2] + chestLTM[3][2] )
        //);
        //Vector3f currPelvisUp( pelvisLTM[1][0], pelvisLTM[1][1], pelvisLTM[1][2] );
        //Vector3f currPelvisAt( pelvisLTM[2][0], pelvisLTM[2][1], pelvisLTM[2][2] );
        //Vector3f currPelvisRight( pelvisLTM[0][0], pelvisLTM[0][1], pelvisLTM[0][2] );
        //currPelvisRight.normalize();
        //currPelvisUp.normalize();
        //currPelvisAt.normalize();

        // linear velocity
        //NxVec3 vel = wrap( ( currPelvisPos - _prevPelvisPos ) * 1.0f / dt );
        //_phActor->setLinearVelocity( vel );
        //_jumper->initOverburdenCalculator( vel );
    
        // angular velocity
        //float angle = ::calcAngle( currPelvisUp, _prevPelvisUp, currPelvisAt );   
		//Vector3f zero;
		//zero[0] = zero[1] = zero[2] = 0;
  //      _phActor->setAngularVelocity( wrap( zero ) );

  //      _prevPelvisPos = currPelvisPos;
  //      _prevPelvisUp = currPelvisUp;
  //      _prevPelvisAt = currPelvisAt;

        if( _actionTime - _blendTime > _criticalPeriod ) _endOfAction = true;
    }
}

void Jumper::SideStepJump::updatePhysics(void)
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
	float res = ( exp( pow( ( velocityV.magnitude() / Vt ), 1.4f ) )  - 1.0f ) / 1.718f;
    NxVec3 Far = NxVec3(0,1,0) * res * _phActor->getMass() * 9.8f;

    // control torque
    NxVec3 Tctrl( 0,0,0 );
    Tctrl += NxVec3(1,0,0) * -_jumper->getVirtues()->getSteerPitch() * _jumper->getSpinalCord()->down;
    Tctrl += NxVec3(1,0,0) * _jumper->getVirtues()->getSteerPitch() * _jumper->getSpinalCord()->up;
    Tctrl *= It;

    // finalize motion equation    
    _phActor->addForce( Far );
    _phActor->addLocalTorque( Tctrl );
}