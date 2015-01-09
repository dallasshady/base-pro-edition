
#include "headers.h"
#include "jumper.h"
#include "imath.h"
#include "../common/istring.h"
#include "pose.h"


/**
 * process constants
 */

const float trackingBlendTime         = 0.7f;
const float steeringBackBoneBendLimit = 20.0f;
const float legPitchTime              = 0.85f;
const float legPitchBendLimit         = 20.0f;

/**
 * related animations
 */

static engine::AnimSequence trackForwardSequence = 
{
    FRAMETIME(335), 
    FRAMETIME(450), 
    engine::ltPeriodic, 
    FRAMETIME(335) 
};

static engine::AnimSequence trackForwardSequenceWings = 
{
    FRAMETIME(1985), 
    FRAMETIME(2004), 
    engine::ltPeriodic, 
    FRAMETIME(1985) 
};

static engine::AnimSequence trackDownSequence =
{
    FRAMETIME(473), 
    FRAMETIME(490), 
    engine::ltPeriodic, 
    FRAMETIME(473) 
};

static engine::AnimSequence trackDownSequenceWings =
{
    FRAMETIME(2005), 
    FRAMETIME(2025), 
    engine::ltPeriodic, 
    FRAMETIME(2005) 
};

static engine::AnimSequence trackLeftSequence =
{
    FRAMETIME(512), 
    FRAMETIME(526), 
    engine::ltPeriodic, 
    FRAMETIME(512)     
};

static engine::AnimSequence trackLeftSequenceWings =
{
    FRAMETIME(2049), 
    FRAMETIME(2071), 
    engine::ltPeriodic, 
    FRAMETIME(2049)     
};

static engine::AnimSequence trackRightSequence =
{
    FRAMETIME(451), 
    FRAMETIME(472), 
    engine::ltPeriodic, 
    FRAMETIME(451) 
};

static engine::AnimSequence trackRightSequenceWings =
{
    FRAMETIME(2026), 
    FRAMETIME(2048), 
    engine::ltPeriodic, 
    FRAMETIME(2026) 
};

/**
 * class implementation
 */

Jumper::Tracking::Tracking(Jumper* jumper, NxActor* phActor, MatrixConversion* mc) :
    JumperAction( jumper )
{
	SpinalCord *spine = _jumper->getSpinalCord();

	// *** ARCH POSE ***
	Pose ArchPose;
	float arch_pose_area = 1.3f;
	ArchPose.L = 0.2f;
	// head
	ArchPose.airfoils[POSEANIM_JOINT_HEAD]		= NxVec3(0.0f, 0.0f, 1.0f);	
	ArchPose.airfoils_area[POSEANIM_JOINT_HEAD] = arch_pose_area*0.4f;
	// torso (stabilizer)
	ArchPose.airfoils[POSEANIM_JOINT_TORSO]		= NxVec3(0.0f, 1.5f, 0.0f);
	ArchPose.airfoils_area[POSEANIM_JOINT_HEAD] = arch_pose_area*0.15f;
	// arms
	ArchPose.airfoils[POSEANIM_JOINT_LARM]		= NxVec3(-2.22777f, 0.52505f, 10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_HEAD] = arch_pose_area*0.15f;
	ArchPose.airfoils[POSEANIM_JOINT_RARM]		= NxVec3( 2.22777f, 0.52505f, 10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_HEAD] = arch_pose_area*0.15f;
	// legs
	ArchPose.airfoils[POSEANIM_JOINT_LLEG]		= NxVec3(-2.22777f, 0.52505f,-10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_HEAD] = arch_pose_area*0.15f;
	ArchPose.airfoils[POSEANIM_JOINT_RLEG]		= NxVec3( 2.22777f, 0.52505f,-10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_HEAD] = arch_pose_area*0.15f;
		// anims
		// turns
	PoseAnim *turnsanim = new PoseAnim();
	turnsanim->next = NULL;
	turnsanim->jointangles[POSEANIM_JOINT_LARM] = 20.0f;
	turnsanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,0.0f,1.0f);
	turnsanim->jointangles[POSEANIM_JOINT_RARM] = -20.0f;
	turnsanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,0.0f,1.0f);
	turnsanim->var = new float*[1];
	turnsanim->var[0] = &spine->left;



    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.5f;
    _endOfAction = false;
    _phActor = phActor;
    _matrixConversion = mc;
    _steering = 0.0f;
    _tracking = 0.0f;
    _legPitch = 0.0f;
	_leveling = 0.0f;

	bool useWingsuit = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit;
	if (useWingsuit) {
		_phActor->setAngularDamping(5.0f);
	}
    if( jumper->_player )
    {
        Gameplay::iGameplay->resetActionChannels( ::iaZoomIn );
    }

    engine::IAnimationController* animCtrl = _clump->getAnimationController();

	// create airfoils
	Airfoil **airfoils = new Airfoil*[5];
	float area = 1.3f;
	float Cd = 0.4f * database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->mTracking;
	if (useWingsuit) {
		area = 1.9f;
	}
	airfoils[0] = new Airfoil(_phActor, area*0.40f, Cd, NxVec3(0,0,0));		// chest (stabilizer)
	airfoils[1] = new Airfoil(_phActor, area*0.15f, Cd, NxVec3(0,0,0));		// left arm
	airfoils[2] = new Airfoil(_phActor, area*0.15f, Cd, NxVec3(0,0,0));		// right arm
	airfoils[3] = new Airfoil(_phActor, area*0.15f, Cd, NxVec3(0,0,0));		// left leg
	airfoils[4] = new Airfoil(_phActor, area*0.15f, Cd, NxVec3(0,0,0));		// right leg
	// positions are set in UpdatePhysics()

	for (int i = 0; i < 5; ++i) {
		airfoils[i]->setAOAadd(90.0f);
	}
	_jumper->clearAirfoils();
	_jumper->addAirfoils(5, airfoils);

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    // setup animation cycles
    if( useWingsuit )
    {
        animCtrl->setTrackAnimation( 0, &trackForwardSequenceWings );
        animCtrl->setTrackAnimation( 1, &trackDownSequenceWings );
        animCtrl->setTrackAnimation( 2, &trackLeftSequenceWings );
    }
    else
    {
        animCtrl->setTrackAnimation( 0, &trackForwardSequence );
        animCtrl->setTrackAnimation( 1, &trackDownSequence );
        animCtrl->setTrackAnimation( 2, &trackLeftSequence );
    }
    animCtrl->setTrackActivity( 0, true );
    animCtrl->setTrackActivity( 1, true );
    animCtrl->setTrackActivity( 2, true );
    animCtrl->setTrackSpeed( 0, 1.0f );
    animCtrl->setTrackWeight( 0, 1.0f );
    animCtrl->setTrackSpeed( 1, 1.0f );
    animCtrl->setTrackWeight( 1, 0.0f );
    animCtrl->setTrackSpeed( 2, 1.0f );
    animCtrl->setTrackWeight( 2, 0.0f );
    animCtrl->resetTrackTime( 0 );
    animCtrl->resetTrackTime( 1 );
    animCtrl->resetTrackTime( 2 );
    updatePhysics();
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

void Jumper::Tracking::updateBlending(float dt)
{
    // blend animations
    float weight0, weight1, weight2, correct01;
    weight2 = fabs( _steering );
    weight2 *= 0.5f;	weight2 *= 0.0f;
    weight1 = _tracking;
    weight0 = 1.0f - weight1;
    correct01 = ( weight0 + weight1 ) * ( 1.0f - weight2 );
    weight0 *= correct01;
    weight1 *= correct01;

    engine::IAnimationController* animCtrl = _clump->getAnimationController();
    animCtrl->setTrackWeight( 0, weight0 );
    animCtrl->setTrackWeight( 1, weight1 );
    animCtrl->setTrackWeight( 2, weight2 );   


	return;
	// turn animations
    if( _steering < 0 && animCtrl->getTrackAnimation( 2 ) != &trackLeftSequence &&
                         animCtrl->getTrackAnimation( 2 ) != &trackLeftSequenceWings )
    {
        if( database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit )
        {
            animCtrl->setTrackAnimation( 2, &trackLeftSequenceWings );
        }
        else
        {
            animCtrl->setTrackAnimation( 2, &trackLeftSequence );
        }
        animCtrl->resetTrackTime( 2 );
    }
    if( _steering > 0 && animCtrl->getTrackAnimation( 2 ) != &trackRightSequence )
    {
        if( database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit )
        {
            animCtrl->setTrackAnimation( 2, &trackRightSequenceWings );
        }
        else
        {
            animCtrl->setTrackAnimation( 2, &trackRightSequence );
        }
        animCtrl->resetTrackTime( 2 );
    }
}

void Jumper::Tracking::updateProceduralAnimation(float dt)
{
	SpinalCord *spine = _jumper->getSpinalCord();
	bool useWingsuit = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit;

    float pitchAngle = legPitchBendLimit * _legPitch * (1.0f - _tracking*0.6f);
	if (pitchAngle == 0.0f) {
		pitchAngle = 0.001f;
	}
	float steeringAngle = steeringBackBoneBendLimit * _steering;

	// ANIMATIONS
	// turns
	_jumper->animBodyPart("joint8", Vector3f(0.0f,0.0f,1.0f), -steeringAngle);	// right arm
	_jumper->animBodyPart("joint32", Vector3f(0.0f,0.0f,1.0f), steeringAngle);	// left arm

	// levelling
	if (_leveling) {
		float max_arm_angle = 30.0f;
		float max_leg_angle = 30.0f;
		if (_tracking) {
			max_arm_angle = -28.0f;
			max_leg_angle = 28.0f;
		}
		_jumper->animBodyPart("joint8", Vector3f(1.0f,0.0f,0.0f), -max_arm_angle * _leveling);		// right arm
		_jumper->animBodyPart("joint32", Vector3f(1.0f,0.0f,0.0f), -max_arm_angle * _leveling);		// left arm
		_jumper->animBodyPart("Hip_Joint", Vector3f(0.0f,0.0f,1.0f), -max_leg_angle * _leveling);	// left leg
		_jumper->animBodyPart("Hip_Joint1", Vector3f(0.0f,0.0f,1.0f), -max_leg_angle * _leveling);	// right leg
	}

	// front pitch
	if (!spine->modifier && spine->leftWarp && spine->rightWarp) {
		float pitch = spine->leftWarp;
		if (spine->rightWarp < pitch) {
			pitch = spine->rightWarp;
		}

		// headdown

		// normal pitch
		//_jumper->animBodyPart("Poyasnitsa", Vector3f(0.0f,0.0f,1.0f), 30.0f * pitch);	// torso
		//_jumper->animBodyPart("joint8", Vector3f(1.0f,0.0f,0.0f), -70.0f * pitch);
		//_jumper->animBodyPart("joint32", Vector3f(1.0f,0.0f,0.0f), -70.0f * pitch);
		//_jumper->animBodyPart("joint8", Vector3f(0.0f,0.0f,1.0f), 40.0f * pitch);
		//_jumper->animBodyPart("joint32", Vector3f(0.0f,0.0f,1.0f), 40.0f * pitch);

	// back pitch
	} else if (!spine->modifier && spine->leftRearRiser && spine->rightRearRiser) {
		float pitch = spine->leftRearRiser;
		if (spine->rightRearRiser < pitch) {
			pitch = spine->rightRearRiser;
		}
		_jumper->animBodyPart("joint8", Vector3f(1.0f,0.0f,0.0f), -70.0f * pitch);	// right arm
		_jumper->animBodyPart("joint32", Vector3f(1.0f,0.0f,0.0f), -70.0f * pitch);	// left arm
		_jumper->animBodyPart("joint8", Vector3f(0.0f,0.0f,1.0f), 40.0f * pitch);	// right arm
		_jumper->animBodyPart("joint32", Vector3f(0.0f,0.0f,1.0f), 40.0f * pitch);	// left arm

	// left roll
	} else if (!useWingsuit && !spine->modifier && spine->leftWarp) {
		_jumper->animBodyPart("joint32", Vector3f(0.0f,1.0f,0.0f), -40.0f * spine->leftWarp);// left arm
		_jumper->animBodyPart("joint8", Vector3f(0.0f,1.0f,0.0f), 50.0f * spine->leftWarp);	// right arm

		_jumper->animBodyPart("Hip_Joint", Vector3f(0.0f,1.0f,0.0f), -22.0f * spine->leftWarp);	// left leg
		_jumper->animBodyPart("Hip_Joint1", Vector3f(0.0f,1.0f,0.0f), 22.0f * spine->leftWarp); // right leg

		_jumper->animBodyPart("Poyasnitsa", Vector3f(0.0f,0.0f,1.0f), 15.0f * spine->leftWarp);	// torso
	// right roll
	} else if (!useWingsuit && !spine->modifier && spine->rightWarp) {
		_jumper->animBodyPart("joint32", Vector3f(0.0f,1.0f,0.0f), 50.0f * spine->rightWarp);// left arm
		_jumper->animBodyPart("joint8", Vector3f(0.0f,1.0f,0.0f), -40.0f * spine->rightWarp);	// right arm

		_jumper->animBodyPart("Hip_Joint", Vector3f(0.0f,1.0f,0.0f), -22.0f * spine->rightWarp);	// left leg
		_jumper->animBodyPart("Hip_Joint1", Vector3f(0.0f,1.0f,0.0f), 22.0f * spine->rightWarp); // right leg

		_jumper->animBodyPart("Poyasnitsa", Vector3f(0.0f,0.0f,1.0f), 15.0f * spine->rightWarp);	// torso
	}

	// forward/backward
	if (_legPitch && (useWingsuit && _tracking || !useWingsuit ) ) {
		float max_arm_angle = 40.0f;
		if (_tracking) max_arm_angle = 20.0f;
		_jumper->animBodyPart("joint8", Vector3f(0.0f,1.0f,0.0f), -max_arm_angle * _legPitch);	// right arm
		_jumper->animBodyPart("joint32", Vector3f(0.0f,1.0f,0.0f), -max_arm_angle * _legPitch); // left arm

		const float ymod = _tracking > 0.0f? -1.0f : 1.0f;
		_jumper->animBodyPart("Hip_Joint", Vector3f( 0.0f, 0.3f * ymod, 0.7f ), pitchAngle);	// left leg
		_jumper->animBodyPart("Hip_Joint1", Vector3f( 0.0f, -0.3f * ymod, 0.7f ), pitchAngle);	// right leg
	}


	
/*
    // left leg    
    engine::IFrame* leftLegBone = Jumper::getLeftLegBone( _clump );    
    matrix = leftLegBone->getMatrix();
    axis = Vector3f( matrix[2][0], matrix[2][1], matrix[2][2] );
	axis = Vector3f( 0.0f, 0.3f * ymod, 0.7f );
    axis.normalize();
    matrix = Gameplay::iEngine->rotateMatrix( matrix, axis, pitchAngle );
    leftLegBone->setMatrix( matrix );

    // right leg
    engine::IFrame* rightLegBone = Jumper::getRightLegBone( _clump );
    matrix = rightLegBone->getMatrix();
    //axis = Vector3f( matrix[2][0], matrix[2][1], matrix[2][0] );
	axis = Vector3f( 0.0f, -0.3f * ymod, 0.7f );
    axis.normalize();
    matrix = Gameplay::iEngine->rotateMatrix( matrix, axis, pitchAngle );
    rightLegBone->setMatrix( matrix );*/
}

void Jumper::Tracking::update(float dt)
{
	updateAnimation( dt );

	// leveling control
	if (_jumper->getSpinalCord()->wlo) {
		_leveling -= 0.5f * dt;
		if (_leveling < -1.0f) _leveling = -1.0f;
	} else if (_jumper->getSpinalCord()->hook) {
		_leveling += 0.5f * dt;
		if (_leveling > 1.0f) _leveling = 1.0f;
	} else if (_jumper->getSpinalCord()->modifier) {
		//_leveling = 0.0f;
	}

    // synchronize physics & render
    _clump->getFrame()->setMatrix( _matrixConversion->convert( wrap( _phActor->getGlobalPose() ) ) );    
    updateProceduralAnimation( dt );
    _clump->getFrame()->getLTM();

    // update controls
    if( _actionTime > _blendTime ) 
    {
        updateBlending( dt );
    }
    else if( _jumper->_player )
    {
        Gameplay::iGameplay->resetActionChannels( ::iaZoomIn );
    }
}

/**
 * physics controller
 */

static float getAirResistancePower(float i)
{
    return ( exp( pow( i, 1.4f ) )  - 1.0f ) / 1.718f;
}

static float getGlidePower(float angle)
{
    float i = cos( angle * 3.1415926f / 180.0f );
    return ( pow( cos( i ), 7.0f ) * pow( i, 8.0f ) ) / 0.01544f;
}

void Jumper::Tracking::updatePhysics(void)
{
    // update controls
    if( _actionTime > _blendTime )
    {
        // update tracking 
        if( !_jumper->getSpinalCord()->modifier )
        {   
            if( _tracking < simulationStepTime / trackingBlendTime ) _tracking = 0;
            else _tracking -= simulationStepTime / trackingBlendTime;
        }
        // update tracking 
        else
        {
            _tracking += simulationStepTime / trackingBlendTime;
            if( _tracking > 1.0f ) _tracking = 1.0f;
        }

        // update steering
        _steering = 0;
        _steering += _jumper->getSpinalCord()->left;
        _steering -= _jumper->getSpinalCord()->right;
        if( _steering == 0.0f )
        {
            _clump->getAnimationController()->resetTrackTime( 2 );
        }
    }

    // leg pitch
    if( _jumper->getSpinalCord()->up != 0.0f ) _legPitch -= simulationStepTime / legPitchTime;
    if( _jumper->getSpinalCord()->down != 0.0f ) _legPitch += simulationStepTime / legPitchTime;


    if( !_jumper->getSpinalCord()->modifier && _jumper->getSpinalCord()->up == 0.0f && _jumper->getSpinalCord()->down == 0.0f )
    {
        if( _legPitch < 0 )
        {
            _legPitch += simulationStepTime / legPitchTime;
            if( _legPitch > 0 ) _legPitch = 0.0f;
        }
        if( _legPitch > 0 )
        {
            _legPitch -= simulationStepTime / legPitchTime;
            if( _legPitch < 0 ) _legPitch = 0.0f;
        }
    }
    if( _legPitch < -1 ) _legPitch = -1;
    if( _legPitch > 1 ) _legPitch = 1;

    // velocity of base jumper's body
    NxVec3 velocity = _phActor->getLinearVelocity();

    // horizontal velocity of base jumper's body
    NxVec3 velocityH( velocity.x, 0, velocity.z );

    // vectical velocity of base jumper's body
    NxVec3 velocityV( 0, velocity.y, 0 );

    // local coordinate system of base jumper
    NxMat34 freeFallPose = _phActor->getGlobalPose();
    NxVec3 x = freeFallPose.M.getColumn(0);
    NxVec3 y = freeFallPose.M.getColumn(1);
    NxVec3 z = freeFallPose.M.getColumn(2);

    // air resistance coefficients
    float ARfrog = _jumper->getVirtues()->getFrogAirResistance();
    float ARtrac = _jumper->getVirtues()->getTrackingAirResistance();
    float ARhead = _jumper->getVirtues()->getHeadfirstAirResistance();
    float ARlift = 1.2f;

    // retrieve final resistance coefficient
    float AR = ARfrog * ( 1.0f - _tracking ) + ARtrac * _tracking;
    float headfirstFactor = z.dot( NxVec3(0,-1,0) );
    if( headfirstFactor < 0 ) headfirstFactor = 0.0f;
    AR = AR * ( 1.0f - headfirstFactor ) + ARhead * headfirstFactor;

    // angle of attack
    // ( negative angle of attack affects force direction )
    NxVec3 hz = z; hz.y = 0; hz.normalize();
    float aa = ::calcAngle( z, hz, x );       

    // terminal velocity
    float Vt = sqrt( 9.8f * _phActor->getMass() / AR );
    float It = velocityV.magnitude() / Vt;

    // air resistance force
    NxVec3 Far = NxVec3(0,1,0) * getAirResistancePower( velocityV.magnitude() / Vt ) * _phActor->getMass() * 9.8f;

	 
	// vel
	velocity.normalize();
	velocity *= getAirResistancePower( velocityV.magnitude() / Vt ) * _phActor->getMass() * 9.81f;

	// left hand
    //engine::IFrame* leftHand = Jumper::getLeftRing( _clump )->getFrame();   
    //Matrix4f matrixLH = leftHand->getMatrix();
    NxVec3 posLH = NxVec3( -2.22777f, 0.52505f, 10.00000f );		// copied and mirrored from legs
	
	if( _tracking == 0.0f ) {
		//posLH.z -= 0.2f * (_jumper->getSpinalCord()->up - _jumper->getSpinalCord()->down);
	} else {
		posLH.z -= 0.5f * _tracking;
	}

	// right hand
    //engine::IFrame* rightHand = Jumper::getRightRing( _clump )->getFrame();    
    //Matrix4f matrixRH = rightHand->getMatrix();
    NxVec3 posRH = NxVec3(  2.22777f, 0.52505f, 10.00000f );		// copied and mirrored from legs
	if( _tracking == 0.0f ) {
		//posRH.z -= 0.2f * (_jumper->getSpinalCord()->up - _jumper->getSpinalCord()->down);
	} else {
		posRH.z -= 0.5f * _tracking;
	}

    // left leg    
    engine::IFrame* leftLegBone = Jumper::getLeftLegBone( _clump );    
    Matrix4f matrixLL = leftLegBone->getMatrix();
    //NxVec3 posLL = NxVec3( matrixLL[3][0], -matrixLL[3][1], -matrixLL[3][2] );
	NxVec3 posLL = NxVec3( -2.22777f, 0.52505f, -10.00000f );
	if( _tracking == 0.0f ) {
		posLL.z -= 0.2f * (_jumper->getSpinalCord()->up - _jumper->getSpinalCord()->down);
	} else {
		posLL.z -= 0.1f * _tracking;
	}

	// right leg
    engine::IFrame* rightLegBone = Jumper::getRightLegBone( _clump );    
    Matrix4f matrixRL = leftLegBone->getMatrix();
    //NxVec3 posRL = NxVec3( -matrixLL[3][0], -matrixLL[3][1], -matrixLL[3][2] );
	NxVec3 posRL = NxVec3( 2.22777f, 0.52505f, -10.00000f );
	if( _tracking == 0.0f ) {
		posRL.z -= 0.2f * (_jumper->getSpinalCord()->up - _jumper->getSpinalCord()->down);
	} else {
		posRL.z -= 0.1f * _tracking;
	}

	// stabilizer, move back (and hands too) for wings
	NxVec3 stabilizer = NxVec3(0,1.5f,0);
	bool useWingsuit = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit;

	float fwdback = (_jumper->getSpinalCord()->up - _jumper->getSpinalCord()->down);
	if (!useWingsuit && !_tracking) {
		_phActor->addLocalForce(NxVec3(0.0f, 0.0f, fwdback * 100.0f));
	}

	if (useWingsuit) {
		stabilizer.y -= 0.6f;
		stabilizer.z -= 2.0f;
		posLH.z -= 1.0f;
		posRH.z -= 1.0f;
	}

	SpinalCord *spine = _jumper->getSpinalCord();


	// set airfoil positions
	//_jumper->getAirfoil(0)->setPosition(stabilizer);
	//_jumper->getAirfoil(1)->setPosition(posLH);	
	//_jumper->getAirfoil(2)->setPosition(posRH);
	//_jumper->getAirfoil(3)->setPosition(posLL);
	//_jumper->getAirfoil(4)->setPosition(posRL);

	// recalculate area
	float area = 1.3f;
	//float Cd = 0.4f * database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->mTracking;
	if (useWingsuit) {
		area = 1.6f;
	}

	//_jumper->getAirfoil(0)->setWingArea(area*(0.40f + _tracking*0.05f));
	//_jumper->getAirfoil(3)->setWingArea(area*(0.15f + _tracking*0.01f));
	//_jumper->getAirfoil(4)->setWingArea(area*(0.15f + _tracking*0.01f));

	//int i = 0;
	//Airfoil *airfoil = _jumper->getAirfoil(0);
	//Vector3f charAt = _clump->getFrame()->getAt(); charAt.normalize();
	//while (airfoil != NULL) {
	//	airfoil->calcForce(wrap(charAt));
	//	++i;
	//	airfoil = _jumper->getAirfoil(i);
	//}


	// ROLL
	const float roll_power = 650.0f;
	const float front_roll_power = 500.0f;
	const float back_roll_power = 430.0f;
	if (!spine->modifier && spine->leftWarp && spine->rightWarp) {	// front roll
		float turnx = front_roll_power;
		_phActor->addLocalTorque(NxVec3(turnx, 0.0f, 0.0f));
		
		posLH.z -= spine->leftWarp * 2.0f;
		posRH.z -= spine->rightWarp * 2.0f;
		posLL.z -= spine->leftWarp * 0.5f;
		posRL.z -= spine->rightWarp * 0.5f;

	} else if (!spine->modifier && spine->leftRearRiser && spine->rightRearRiser) {	// back roll
		float turnx = -back_roll_power;
		_phActor->addLocalTorque(NxVec3(turnx, 0.0f, 0.0f));
		
		posLH.z += spine->leftRearRiser * 2.0f;
		posRH.z += spine->rightRearRiser * 2.0f;
		posLL.z -= spine->leftRearRiser * 0.5f;
		posRL.z -= spine->rightRearRiser * 0.5f;

	} else if (spine->leftWarp) {										// roll left
		float turnz = -roll_power*spine->leftWarp;
		if (_tracking) {
			turnz *= 0.1f;
		}
		_phActor->addLocalTorque(NxVec3(0.0f, 0.0f, turnz));

		posLH.x *= 1.0f - spine->leftWarp;
		posRH.x *= 1.0f - spine->leftWarp;
		posLL.x *= 1.0f - spine->leftWarp;
		posRL.x *= 1.0f - spine->leftWarp;

	} else if (spine->rightWarp) {										// roll right
		float turnz = roll_power*spine->rightWarp;
		if (_tracking) {
			turnz *= 0.1f;
		}
		_phActor->addLocalTorque(NxVec3(0.0f, 0.0f, turnz));

		posLH.x *= 1.0f - spine->rightWarp;
		posRH.x *= 1.0f - spine->rightWarp;
		posLL.x *= 1.0f - spine->rightWarp;
		posRL.x *= 1.0f - spine->rightWarp;
	}

	// TURNS
	const float turn_power = 100.0f;
	if (spine->left) {
		float randx = 0.0f;//getCore()->getRandToolkit()->getUniform(-turn_power, turn_power)*spine->left;
		float randz = 0.0f;//getCore()->getRandToolkit()->getUniform(-turn_power, turn_power)*spine->left;
		if (useWingsuit) {
			randz = -70.0f*spine->left;
		}
		float realy = turn_power*spine->left;
		if (_tracking) {
			realy *= 0.9f;
		}
		_phActor->addLocalTorque(NxVec3(randx, realy, randz));
	} else if (spine->right) {
		float randx = 0.0f;//getCore()->getRandToolkit()->getUniform(-100.0f, 100.0f)*spine->right;
		float randz = 0.0f;//getCore()->getRandToolkit()->getUniform(-100.0f, 100.0f)*spine->right;
		if (useWingsuit) {
			randz = 70.0f*spine->right;
		}
		float realy = -turn_power*spine->right;
		if (_tracking) {
			realy *= 0.9f;
		}
		_phActor->addLocalTorque(NxVec3(randx, realy, randz));
	}


	/// PHYSICS
	// altitude (m)
	NxVec3 pos = _phActor->getGlobalPosition();
	float altitude = pos.y;

	// air density: converted to linear from barometric equation [0:10] km altitude
	// http://www.denysschen.com/catalogue/density.aspx
	const float AirDensity = altitude <= 10000.0f ? (1.196f - 0.0000826f * altitude) : (0.27f);
	// speed (m/s) squared
	float Vsq = _phActor->getLinearVelocity().magnitudeSquared();

	// drag coeff
	float Cd = 0.4f;	// altitude suit = 0.4f
	const float mTracking = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->mTracking;
	Cd *= mTracking;
	// reference area
	float A = 1.3f;

	float L = 0.2f + 0.6f * _tracking;			// max lift factor (altitude suit)
	if (useWingsuit) {		// wingsuit
		A = 0.6f;
		L = 0.29f + 0.4f * _tracking;
	} else if (mTracking > 1.0f) {	// thermal
		A = 1.4f;
		L = 0.29f + 0.6f * _tracking; 
	}
	A += _leveling * 0.2f;

	// Drag
	float Fd = 0.5f * AirDensity * Vsq * Cd * A ;	
	NxVec3 VecFd = velocity; VecFd.normalize();
	VecFd *= Fd;		// Drag force vector


	if (useWingsuit) {
		stabilizer.y -= 0.1f;
		stabilizer.z -= 0.1f;
	}
	// structural changes
	// tracking
	if (spine->modifier) {
		if (useWingsuit) {
			posLL.y += 0.8f * _legPitch;
			posLL.z += 0.8f * _legPitch;
			posRL.y += 0.8f * _legPitch;
			posRL.z += 0.8f * _legPitch;
			posLH.z -= 2.5f * _tracking;
			posRH.z -= 2.5f * _tracking;
		} else {
			posLL.y += 0.7f * _legPitch;
			posLL.z += 0.7f * _legPitch;
			posRL.y += 0.7f * _legPitch;
			posRL.z += 0.7f * _legPitch;
			posLH.z -= 2.0f * _tracking;
			posRH.z -= 2.0f * _tracking;
		}
	}

	// convert some drag to lift
	NxVec3 motionDir = _phActor->getLinearVelocity(); motionDir.normalize();
	NxVec3 canopyDown  = _phActor->getGlobalPose().M.getColumn(2); canopyDown.normalize();
	float reaction = canopyDown.dot( motionDir );
	/*if (reaction > 0.5f) {
		reaction = 1.0f - reaction;
	} else if (reaction < -0.5f) {
		reaction = -1.0f - reaction;
	}*/
	//reaction *= 2.0f;

	// add drag
	//  reaction
	NxVec3 Flift = NxVec3(0,0,0);
	if (reaction != 0.0f) {
		Flift = NxVec3(0,1,0) * Fd * reaction * L * 1.8f;
		//float Flift_scalar = Fd * reaction * L * 3.55f;
		//if (useWingsuit) {
		//	Flift_scalar = Fd * reaction * L * 0.0f;
		//}

		//Vector3f charAt = _clump->getFrame()->getAt();
		//charAt.normalize();

		//NxVec3 Flift = VecFd;
		//Flift = Flift.cross(wrap(charAt));
		//Flift = Flift.cross(VecFd);
		//Flift.normalize();
		//Flift *= Flift_scalar;

		// take away drag that went to lift
		VecFd *= (1.0f - L);
	}

	_phActor->addForceAtLocalPos(-VecFd*0.40f, stabilizer);	// stabilizer
	_phActor->addForceAtLocalPos(-VecFd*0.15f, posLL);
	_phActor->addForceAtLocalPos(-VecFd*0.15f, posRL);
	_phActor->addForceAtLocalPos(-VecFd*0.15f, posLH);
	_phActor->addForceAtLocalPos(-VecFd*0.15f, posRH);
	
	// wing drag
	if (useWingsuit && false) {
		posLL.x = 0.0f;
		_phActor->addForceAtLocalPos(-VecFd*0.4f*_tracking, posLL);
		_phActor->addForceAtLocalPos(-VecFd*0.1f*_tracking, posLH);
		_phActor->addForceAtLocalPos(-VecFd*0.1f*_tracking, posRH);
	}

	// add lift
	_phActor->addLocalForce(Flift);

	/////////////////////////////
	return;

    // steering coefficients
 //   float Kslide = _jumper->getVirtues()->getSteerSlide();
 //   float Kroll  = _jumper->getVirtues()->getSteerRoll();
 //   float Kalign = _jumper->getVirtues()->getSteerAlign();

 //   // control torque
 //   NxVec3 Tctrl( 0,0,0 );

 //   float leftRate  = ( _jumper->getSpinalCord()->leftWarp > 0.0f ) ? 5.0f : 1.0f;
 //   float rightRate = ( _jumper->getSpinalCord()->rightWarp > 0.0f ) ? 5.0f : 1.0f;

 //   Tctrl += NxVec3(1,0,0) * -_jumper->getVirtues()->getSteerPitch() * _jumper->getSpinalCord()->down;
 //   Tctrl += NxVec3(1,0,0) * _jumper->getVirtues()->getSteerPitch() * _jumper->getSpinalCord()->up;
 //   Tctrl += NxVec3(0,0,1) * leftRate * -_jumper->getVirtues()->getSteerTurn() * _jumper->getSpinalCord()->left;
 //   Tctrl += NxVec3(0,0,1) * rightRate * _jumper->getVirtues()->getSteerTurn() * _jumper->getSpinalCord()->right;
 //   Tctrl *= It;

 //   // calculate steering torque
 //   NxVec3 signum = z; signum.y = 0.0f; signum.normalize();
 //   NxVec3 vertical(0,1,0);
 //   float steerAngle = calcAngle( y, vertical, signum );
 //   float inclinationAngle = fabs( calcAngle( z, signum ) );
 //   if( steerAngle > 0 ) inclinationAngle *= -1;
 //   steerAngle += inclinationAngle;
 //   NxVec3 Tsteer = ( y * sqrt( velocity.magnitude() * Kslide ) * -steerAngle )+
 //                   ( vertical * sqrt( velocity.magnitude() * Kroll ) * -steerAngle ) +
 //                   ( z * sqrt( velocity.magnitude() * Kalign ) * -steerAngle );

	//Tsteer.y *= 2.0f;
 //   Tsteer *= getAirResistancePower( It );

 //   // gyration resistance
 //   float GRfrog = 20.0f;
 //   float GRtrac = 40.0f;
 //   float GR = GRfrog * ( 1.0f - _tracking ) + GRtrac * _tracking;
 //   float GRV = velocity.magnitude() / 25.0f;
 //   if( GRV > 1.0f ) GRV = 1.0f;
 //   GR *= GRV;
 //   NxVec3 Tr = _phActor->getAngularVelocity() * -GR;

 //   // glide properties
 //   float glideRatio = _jumper->getVirtues()->getFrogGlideRatio() * ( 1.0f - _tracking ) + 
 //                      _jumper->getVirtues()->getTrackingGlideRatio() * ( _tracking );
 //   float glideCoeff = _jumper->getVirtues()->getFrogGlideCoefficient() * ( 1.0f - _tracking ) + 
 //                      _jumper->getVirtues()->getTrackingGlideCoefficient() * _tracking;

 //   // double component glide force vector
 //   NxVec3 g = y + z; g.y = 0.0f; g.normalize();

 //   // horizontal motion vector
 //   NxVec3 h = velocityH; h.normalize();

 //   // magnitude of terminal glide velocity
 //   float Vtgl = Vt * glideRatio;

 //   // glide force
 //   NxVec3 Fg = g * sgn(aa) * getGlidePower( fabs( aa ) ) * glideCoeff * getAirResistancePower( It );

 //   // backtracking keeps 1/4 perfomance from forward tracking
 //   if( aa < 0 ) Fg *= 0.25f;

 //   // glide resistance force
 //   NxVec3 Fgr = -h * getAirResistancePower( velocityH.magnitude() / Vtgl ) * glideCoeff;
 //   
 //   // finalize motion equation    
 //   _phActor->addForce( Far + Fg + Fgr );
 //   _phActor->addTorque( Tr * It + Tsteer );
 //   _phActor->addLocalTorque( Tctrl );



	//return;
 //   // fake physics : horizontal velocity aligment
 //   float kHAlign = _tracking * ::simulationStepTime * _jumper->getVirtues()->getHorizontalAligment();
 //   NxVec3 lVel = _phActor->getLinearVelocity();
 //   NxVec3 vVel( 0,1,0 );
 //   vVel *= vVel.dot( lVel );
 //   NxVec3 hVel = lVel - vVel;
 //   float hVelMagnitude = hVel.magnitude();
 //   hVel.normalize();
 //   hVel = hVel + g * kHAlign;
 //   hVel.normalize();
 //   hVel *= hVelMagnitude;
 //   _phActor->setLinearVelocity( hVel + vVel );
}
