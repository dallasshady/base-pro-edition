
#include "headers.h"
#include "jumper.h"
#include "imath.h"
#include "hud.h"
/**
 * related animations
 */

static engine::AnimSequence openingSequence = 
{
    FRAMETIME(1038),	//FRAMETIME(1038),
    FRAMETIME(1112),
    engine::ltNone, 
    0.0f
};

/**
 * action
 */

Jumper::CanopyOpening::CanopyOpening(Jumper* jumper, PxRigidDynamic* phFreeFall, PxRigidDynamic* phFlight, MatrixConversion* mcFlight, PilotchuteSimulator* pc, CanopySimulator* c, PxVec3 fla, PxVec3 fra, PxVec3 rla, PxVec3 rra) :
    JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.2f;
    _endOfAction = false;
    _phActor = phFlight;
    _matrixConversion = mcFlight;
    _pilotchute = pc;
    _canopy = c;
    _frontLeftAnchor  = fla;
    _frontRightAnchor = fra;
    _rearLeftAnchor   = rla;
    _rearRightAnchor  = rra;
    _initialLD = phFlight->getLinearDamping();

    // activate jumper body simulator
    Matrix4f sampleLTM = Jumper::getCollisionFC( _clump )->getFrame()->getLTM();
    phFlight->setGlobalPose(PxTransform(wrap( sampleLTM )));
    phFlight->wakeUp();
    phFlight->setLinearVelocity( phFreeFall->getLinearVelocity() );
    phFlight->setAngularVelocity( phFreeFall->getAngularVelocity() );    

    // connect & open canopy    
    _canopy->connect(
        _phActor, 
        _frontLeftAnchor,
        _frontRightAnchor,
        _rearLeftAnchor,
        _rearRightAnchor,
        Jumper::getFrontLeftRiser( _clump ),
        Jumper::getFrontRightRiser( _clump ),
        Jumper::getRearLeftRiser( _clump ),
        Jumper::getRearRightRiser( _clump )
    );

	// age
	if (_jumper->getCanopyReserveSimulator() && _jumper->getCanopyReserveSimulator() == _canopy) {
		++jumper->getVirtues()->equipment.reserve.age;
	} else {
		++jumper->getVirtues()->equipment.canopy.age;
	}

    // retrieve pilotchute velocity
    float pcVel = pc->getPhActor()->getLinearVelocity().magnitude();
	float anglVel = phFreeFall->getAngularVelocity().magnitude();

    // retrieve pilotchute reference velocity
	database::Canopy* canopyInfo = _canopy->getGearRecord();
	database::Pilotchute* pcInfo;
	if (_jumper->getCanopyReserveSimulator() == _canopy) {
		pcInfo = canopyInfo->pilots;
	} else {
		assert( canopyInfo->numPilots > _jumper->getVirtues()->equipment.pilotchute );
		pcInfo = canopyInfo->pilots + _jumper->getVirtues()->equipment.pilotchute;
	}

    // probability of lineover
    float lineoverProb  = _jumper->getVirtues()->getLineoverProbability( pcVel / pcInfo->Vrec );
	// no slider has not smaller probability
	if (_jumper->getVirtues()->equipment.sliderOption != soRemoved) {
		lineoverProb *= 0.3f;
	}
	// reserves have 70% less lineover probability
	if (_jumper->getCanopyReserveSimulator() == _canopy) {
		lineoverProb *= 0.3f;
	}
    bool  leftLineover  = ( getCore()->getRandToolkit()->getUniform( 0, 1 ) < lineoverProb );
    bool  rightLineover = !leftLineover && ( getCore()->getRandToolkit()->getUniform( 0, 1 ) < lineoverProb );
    float leftLOW  = leftLineover ? getCore()->getRandToolkit()->getUniform( 0.5, 1.0f ) : 0;
    float rightLOW = rightLineover ? getCore()->getRandToolkit()->getUniform( 0.5, 1.0f ) : 0;

    // probability of linetwists
	// add probability if spinning
	//if( _jumper->isPlayer() ) getCore()->logMessage( "angular velocity component on deployment: %2.10f", anglVel * 0.13f );
    float linetwistsProb = _jumper->getVirtues()->getLinetwistsProbability( pcVel ) + anglVel * 0.13f;
    float linetwistsDice = getCore()->getRandToolkit()->getUniform( 0.0f, 1.0f );
	
	// probability of linetwists because of incorrect body position
	PxVec3 motionDir = _jumper->getFreefallActor()->getLinearVelocity(); motionDir.normalize();
	PxVec3 canopyDown  = _jumper->getFreefallActor()->getGlobalPose().q.getBasisVector2(); canopyDown.normalize();
    float relativity = 1.0f + fabs(canopyDown.dot( motionDir ));
	linetwistsProb *= relativity;

	// reserves have 50% less linetwist probability
	if (_jumper->getCanopyReserveSimulator() == _canopy) {
		linetwistsProb *= 0.5f;
	}
	
	// base canopies gave 40% less linetwist probability
	else if (!canopyInfo->skydiving) {
		linetwistsProb *= 0.4f;
	}

    bool  linetwists = ( linetwistsDice <= linetwistsProb );
	//if( _jumper->isPlayer() ) {
		//getCore()->logMessage( "linetwists prob: %3.5f", linetwistsProb );
		//getCore()->logMessage( "linetwists dice: %3.5f", linetwistsDice );
	//} else {
		//getCore()->logMessage( "linetwists prob BOT: %3.5f", linetwistsProb );
		//getCore()->logMessage( "linetwists dice BOT: %3.5f", linetwistsDice );
	//}

    // generate linetwists (positive is righttwist, negative is lefttwist)
	// the smaller the canopy, the heavier the linetwist
	float sq = canopyInfo->square * 2.0f;
	// 300 canopy: 120; 840
	// 200 canopy: 320; 1040
	// 100 canopy: 520; 1240
    float linetwistsAngle = getCore()->getRandToolkit()->getUniform( 720 - sq, 1540 - sq );
	if (linetwistsAngle > 1080.0f) {
		linetwistsAngle = 1440.0f;
	} else if (linetwistsAngle < 320.0f) {
		linetwistsAngle = 180.0f;
	}

    float sign = getCore()->getRandToolkit()->getUniform( -1, 1 );
    sign = sign < 0.0f ? -1.0f : ( sign > 0.0f ? 1.0f : 0.0f );
    linetwistsAngle *= sign;

    if( !linetwists ) linetwistsAngle = 0.0f;

	// test, force linetwist
	//if (!_jumper->isPlayer() && _jumper->getCanopyReserveSimulator() != _canopy) {
	//	linetwists = true;
	//	linetwistsAngle = 1440.0f;
	//}

    // offheading : canopy turns by specified angle
    float minTurn = 30.0f; // rigging skill = 0.0 
    float maxTurn = 10.0f;  // rigging skill = 1.0 
    float rigging = _jumper->getVirtues()->getRiggingSkill(); assert( rigging >= 0 && rigging <= 1 );
    float turn    = minTurn * ( 1 - rigging ) + maxTurn * rigging;
    float angle = getCore()->getRandToolkit()->getUniform( -turn, turn );
    //if( _jumper->isPlayer() ) getCore()->logMessage( "additional turn (offheading): %2.1f", angle );
    //if( !_jumper->isPlayer() ) angle = 0;


    Vector3f sampleX( sampleLTM[0][0], sampleLTM[0][1], sampleLTM[0][2] );
    Vector3f sampleY( sampleLTM[1][0], sampleLTM[1][1], sampleLTM[1][2] );
    Vector3f sampleZ( sampleLTM[2][0], sampleLTM[2][1], sampleLTM[2][2] );
    Vector3f sampleP( sampleLTM[3][0], sampleLTM[3][1], sampleLTM[3][2] );
    // orient canopy towards jumper velocity
    sampleZ = wrap( phFlight->getLinearVelocity() );
    sampleZ *= -1;
    sampleZ.normalize();
    sampleY = _jumper->getClump()->getFrame()->getAt() * -1;
    sampleX.cross( sampleY, sampleZ );
    sampleX.normalize();
    sampleY.cross( sampleZ, sampleX );
    sampleY.normalize();
    sampleLTM.set( 
        sampleX[0], sampleX[1], sampleX[2], 0.0f,
        sampleY[0], sampleY[1], sampleY[2], 0.0f,
        sampleZ[0], sampleZ[1], sampleZ[2], 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    // turn canopy by random angle

    sampleLTM = Gameplay::iEngine->rotateMatrix( sampleLTM, sampleZ, angle );
    // move clump behind jumper
    sampleP -= sampleZ * 80.0f;

    sampleLTM[3][0] = sampleP[0];
    sampleLTM[3][1] = sampleP[1];
    sampleLTM[3][2] = sampleP[2];

    _canopy->open(PxTransform(wrap(sampleLTM)), _phActor->getLinearVelocity(), leftLOW, rightLOW, linetwistsAngle );

    // reconnect pilotchute to canopy
    _canopy->getClump()->getFrame()->getLTM();
    _pilotchute->connect( 
        _canopy->getPxRigidDynamic(), 
        CanopySimulator::getPilotCordJoint( _canopy->getClump() ),
        _canopy->getPilotAnchor()
    );
	//_pilotchute->setFreebag(_canopy == _jumper->getCanopyReserveSimulator()); set in jumper constructor and jumper::fireReserveCanopy()

    // put to sleep freefall simulator
    phFreeFall->putToSleep();
    
	//PHYSX3
	//phFreeFall->raiseActorFlag( NX_AF_DISABLE_COLLISION );

    // show risers
    Jumper::getRisers( _clump )->setFlags( engine::afRender );

    // animation controller
    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    // setup animation
    animCtrl->setTrackAnimation( 0, &openingSequence );
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackSpeed( 0, 0.75f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

Jumper::CanopyOpening::~CanopyOpening()
{
    _phActor->setLinearDamping( _initialLD );
}

static float getAirResistancePower(float i)
{
    return ( exp( pow( i, 1.4f ) )  - 1.0f ) / 1.718f;
}

void Jumper::CanopyOpening::update(float dt)
{
    if( _canopy->getInflation() > 0 )
    {
        updateAnimation( dt );
    }

    if( _clump->getAnimationController()->isEndOfAnimation( 0 ) ) _endOfAction = true;

    // synchronize physics & render
    _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );
    _clump->getFrame()->getLTM();
}

void Jumper::CanopyOpening::updatePhysics(void)
{
    // velocity of base jumper's body
    PxVec3 velocity = _phActor->getLinearVelocity();

    // local coordinate system of base jumper
	PxTransform pose = _phActor->getGlobalPose();
	PxVec3 x = pose.q.getBasisVector0();
    PxVec3 y = pose.q.getBasisVector1();
    PxVec3 z = pose.q.getBasisVector2();

    // air resistance force
    float AR = _jumper->getVirtues()->getTrackingAirResistance();

    // terminal velocity
    float Vt = sqrt( 9.8f * _phActor->getMass() / AR );
    float It = velocity.magnitude() / Vt;

    // air resistance force
    PxVec3 Far = PxVec3(0,1,0) * getAirResistancePower( velocity.magnitude() / Vt ) * _phActor->getMass() * 9.8f;

    // finalize motion equation    
    _phActor->addForce( Far );

    // linear damping is function of jumper velocity    
    // this is prevents calculation errors due to high speed rates
    float minVel     = 50.0f;
    float minDamping = _initialLD;
    float maxVel     = 70.0f;
    float maxDamping = 2.5f;
    float factor = ( velocity.magnitude() - minVel ) / ( maxVel - minVel );
    factor = factor < 0 ? 0 : ( factor > 1 ? 1 : factor );
    float damping = minDamping * ( factor - 1 ) + maxDamping * ( factor );
    _phActor->setLinearDamping( damping );

    // shallow brake setting
    _canopy->setLeftDeep( 0.0f );
    _canopy->setRightDeep( 0.0f );

}

bool Jumper::CanopyOpening::isCriticalAnimationRange(void)
{
    return ( _actionTime - _blendTime ) < ( FRAMETIME(1062) - FRAMETIME(1038) );
}