
#include "headers.h"
#include "jumper.h"
#include "imath.h"

Jumper::FlipJump::FlipJump(Jumper* jumper, PxRigidDynamic* phActor, MatrixConversion* mc, engine::AnimSequence* sequence, float animSpeed, float criticalPeriod) :
    Jumper::JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.2f;
    _phActor = phActor;
    _matrixConversion = mc;
    _criticalPeriod = criticalPeriod;
    _endOfAction = false;
	_allowModifier = !_jumper->getSpinalCord()->modifier;

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

Jumper::FlipJump::~FlipJump()
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
        _phActor->setGlobalPose(PxTransform(wrap( sampleLTM )));

        // wake up actor
        _phActor->wakeUp();
    }
}

void Jumper::FlipJump::update(float dt)
{
    updateAnimation( dt );
    _clump->getFrame()->getLTM();
	if (!_jumper->getSpinalCord()->modifier) {
		_allowModifier = true;
	}

    if( dt > 0 )
    {
        // obtain current pelvis LTM
        Matrix4f pelvisLTM = Jumper::getPelvisFrame( _jumper->getClump() )->getLTM();
        Matrix4f chestLTM  = Jumper::getChestFrame( _jumper->getClump() )->getLTM();

        // update physics actor properties
        Vector3f currPelvisPos( 
            0.5f * ( pelvisLTM[3][0] + chestLTM[3][0] ), 
            0.5f * ( pelvisLTM[3][1] + chestLTM[3][1] ), 
            0.5f * ( pelvisLTM[3][2] + chestLTM[3][2] )
        );
        Vector3f currPelvisUp( pelvisLTM[1][0], pelvisLTM[1][1], pelvisLTM[1][2] );
        Vector3f currPelvisAt( pelvisLTM[2][0], pelvisLTM[2][1], pelvisLTM[2][2] );
        Vector3f currPelvisRight( pelvisLTM[0][0], pelvisLTM[0][1], pelvisLTM[0][2] );
        currPelvisRight.normalize();
        currPelvisUp.normalize();
        currPelvisAt.normalize();

        // linear velocity
		if (_jumper->getAirplane() == NULL) {
			PxVec3 vel = wrap( ( currPelvisPos - _prevPelvisPos ) * 1.0f / dt );
			_phActor->setLinearVelocity( vel );
			_jumper->initOverburdenCalculator( vel );
		// linear velocity from aircraft
		} else {
			PxVec3 velH = wrap( _clump->getFrame()->getAt() );
			velH.normalize();
			velH *= 3.0f;
			PxVec3 velV = wrap( _clump->getFrame()->getUp() );
			velV.normalize();
			velV *= 0.25f;
			PxVec3 velA = wrap( _jumper->getAirplane()->getVel() );
			_phActor->setLinearVelocity( velH + velV + velA );
			_jumper->initOverburdenCalculator( velH + velV + velA );
		}
        // angular velocity
        float angle = ::calcAngle( currPelvisUp, _prevPelvisUp, currPelvisAt );    
        _phActor->setAngularVelocity( wrap( currPelvisAt * ( angle / dt ) ) );

        _prevPelvisPos = currPelvisPos;
        _prevPelvisUp = currPelvisUp;
        _prevPelvisAt = currPelvisAt;

		if( _actionTime - _blendTime > _criticalPeriod || (_actionTime - _blendTime >= _criticalPeriod*0.75f && _allowModifier && _jumper->getSpinalCord()->modifier) ) _endOfAction = true;
    }
}

void Jumper::FlipJump::updatePhysics(void)
{
}