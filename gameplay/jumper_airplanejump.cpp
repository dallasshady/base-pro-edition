
#include "headers.h"
#include "jumper.h"
#include "imath.h"

static engine::AnimSequence airplaneJumpSequence =
{
    FRAMETIME(1669),
//    FRAMETIME(1683),
	FRAMETIME(1678),
    engine::ltNone,
    0.0f
};

static engine::AnimSequence airplaneJumpSequenceWings =
{
    FRAMETIME(2104),
    FRAMETIME(2130),
    engine::ltNone,
    0.0f
};

/**
 * class implementation
 */

const float trackSpeed = 0.75f;
const float trackInvSpeed = 1.0f / trackSpeed;

Jumper::AirplaneJump::AirplaneJump(Jumper* jumper, NxActor* actor, MatrixConversion* mc) :
    JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.1f;
    _endOfAction = false;
    _phActor = actor;
    _matrixConversion = mc;

    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    bool useWingsuit = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit;

    // setup animation cycle
    if( useWingsuit )
    {
        animCtrl->setTrackAnimation( 0, &airplaneJumpSequenceWings );
    }
    else
    {
        animCtrl->setTrackAnimation( 0, &airplaneJumpSequence );
    }
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, trackSpeed );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );    
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

Jumper::AirplaneJump::~AirplaneJump()
{
}
 
void Jumper::AirplaneJump::update(float dt)
{
    updateAnimation( dt );

    bool useWingsuit = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit;
    float phaseTime = useWingsuit ? trackInvSpeed * ( FRAMETIME(2113) - FRAMETIME(2104) ) : trackInvSpeed * ( FRAMETIME(1673) - FRAMETIME(1669) );

    if( _actionTime > _blendTime + phaseTime )
    {
        if( _phActor->isSleeping() )
        {
            // place jumper to airplane exit
            Matrix4f clumpLTM = _clump->getFrame()->getLTM();
            Vector3f clumpScale = calcScale( clumpLTM );
            Matrix4f exitLTM = _jumper->getAirplaneExit()->getLTM();
            orthoNormalize( exitLTM );
            exitLTM[0][0] *= clumpScale[0], exitLTM[0][1] *= clumpScale[0], exitLTM[0][2] *= clumpScale[0];
            exitLTM[1][0] *= clumpScale[1], exitLTM[1][1] *= clumpScale[1], exitLTM[1][2] *= clumpScale[1];
            exitLTM[2][0] *= clumpScale[2], exitLTM[2][1] *= clumpScale[2], exitLTM[2][2] *= clumpScale[2];
            _clump->getFrame()->setMatrix( exitLTM );
            _clump->getFrame()->getLTM();

            Matrix4f sampleLTM = Jumper::getCollisionFF( _clump )->getFrame()->getLTM();
            _phActor->setGlobalPose( wrap( sampleLTM ) );
            _phActor->wakeUp();
            NxVec3 velH = wrap( _clump->getFrame()->getAt() );
            velH.normalize();
            velH *= 3.0f;
            NxVec3 velV = wrap( _clump->getFrame()->getUp() );
            velV.normalize();
            velV *= 0.25f;
			
            NxVec3 velA = wrap( _jumper->getAirplane()->getVel() );
            _phActor->setLinearVelocity( velH + velV + velA );
            _jumper->initOverburdenCalculator( velH + velV + velA );

			if (_jumper->getSpinalCord()->left) {
				_phActor->addLocalTorque(NxVec3(0,5700.0f,0));
				//_phActor->setAngularDamping(2.0f);
			} else if (_jumper->getSpinalCord()->right) {
				_phActor->addLocalTorque(NxVec3(0,-5700.0f,0));
				//_phActor->setAngularDamping(2.0f);
			}

			if (_jumper->getSpinalCord()->up) {		// headdown exit
				_phActor->addLocalTorque(NxVec3(5700.0f,0,0));
			} else if (_jumper->getSpinalCord()->down) {	// sitfly exit
				_phActor->addLocalTorque(NxVec3(-8700.0f,0,0));
				_phActor->addLocalForce(NxVec3(0,0,10000.0f));
			}
			_phActor->setAngularDamping(2.0f);
        }
        else
        {
			if (_jumper->getSpinalCord()->left) {
				_phActor->addLocalTorque(NxVec3(0,2000.0f*dt,0));
			} else if (_jumper->getSpinalCord()->right) {
				_phActor->addLocalTorque(NxVec3(0,-2000.0f*dt,0));
			}
            _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );
        }
    }
    else
    {
        // place jumper to airplane exit
        Matrix4f clumpLTM = _clump->getFrame()->getLTM();
        Vector3f clumpScale = calcScale( clumpLTM );

        Matrix4f exitLTM = _jumper->getAirplaneExit()->getLTM();
        orthoNormalize( exitLTM );
        exitLTM[0][0] *= clumpScale[0], exitLTM[0][1] *= clumpScale[0], exitLTM[0][2] *= clumpScale[0];
        exitLTM[1][0] *= clumpScale[1], exitLTM[1][1] *= clumpScale[1], exitLTM[1][2] *= clumpScale[1];
        exitLTM[2][0] *= clumpScale[2], exitLTM[2][1] *= clumpScale[2], exitLTM[2][2] *= clumpScale[2];

        _clump->getFrame()->setMatrix( exitLTM );
    }

	if( _clump->getAnimationController()->isEndOfAnimation( 0 ) || (_jumper->getSpinalCord()->modifier && _jumper->isPlayer() &&   _actionTime > _blendTime + phaseTime ))
    {
        _endOfAction = true;
    }


}