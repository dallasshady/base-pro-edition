/*
[] Collect a single instance of all data that's being used in different methods every frame and
   cache it object wide, so that different methods don't have to retrieve it many times

*/

#include "headers.h"
#include "jumper.h"
#include "imath.h"
#include "../common/istring.h"

// local includes
#include "jumper_poses_init.h"
#include "jumper_poses_init_wings.h"
#include "jumper_machine.h"

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
    FRAMETIME(336),		// FRAMETIME(450),
    engine::ltNone, // engine::ltPeriodic, 
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

static engine::AnimSequence sitFlyingSequence = 
{ 
    FRAMETIME(1430), 
    FRAMETIME(1466),	// FRAMETIME(1466)
	engine::ltPeriodic,	// engine::ltPeriodic
    FRAMETIME(1430)
};

static engine::AnimSequence frontFlipSequence = 
{ 
    FRAMETIME(491), 
    FRAMETIME(511),  
    engine::ltPeriodic,
    FRAMETIME(491)
};
/**
 * class implementation
 */


Jumper::Tracking::Tracking(Jumper* jumper, NxActor* phActor, MatrixConversion* mc) :
    JumperAction( jumper )
{
    // set action properties
    _actionTime = 0.0f;
    _blendTime = 0.5f;
    _endOfAction = false;
    _phActor = phActor;
    _matrixConversion = mc;
    _wing_area = 0.0f;
    _tracking = 0.0f;
    _leg_pitch = 0.0f;
	_leveling = 0.0f;
	_carving_forward = _carving_sideways = 0.0f;
	_leveling_higher = 0.0f;	// slower
	_leveling_lower = 0.0f;		// faster
	_transfer_pose = false;
	_default_pose = jumper->getDefaultPose();
	_current_pose = jumper->getDefaultPose();
	this->useWingsuit = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit;
	_machine_state_previous = _machine_state_pre_previous = _machine_state = MACHINE_STATIC_ARCH;
	_fresh_state = true;


	// set poses
	if (!this->useWingsuit) {
		this->initPoses();
	} else {
		this->initPosesWings();
	}

	// get altitude
	float altitude = 0.0f;
	if (_phActor->isSleeping() == false) {
		NxVec3 pos = _phActor->getGlobalPosition();
		altitude = pos.y;
	}

	//// go arch
	if (_default_pose == POSE_ARCH || altitude > 0.0f && altitude < 2000.0f) {
		currentPose = ArchPose;
		_current_pose = POSE_ARCH;
	// go other pose
	} else {
		//  set default pose
		if (_current_pose == POSE_HEADDOWN) {
			currentPose = HeaddownPose;
		} else if (_current_pose == POSE_SITFLY) {
			currentPose = SitflyPose;
		} else if (_current_pose == POSE_BACKFLY) {
			currentPose = BackflyPose;
		}
	}

	// switch pose by reaction
	//NxVec3 motionDir = _phActor->getLinearVelocity();
	//motionDir.normalize();
	//NxVec3 canopyDown  = _phActor->getGlobalPose().M.getColumn(2);
	//canopyDown.normalize();
	//float reaction = canopyDown.dot( motionDir );
	//if (reaction >= 0.75f && reaction < 1.75f && _legPitch != 1) {
	//	// go headdown
	//	currentPose.morphToPose(&HeaddownPose, 1.0f/0.6f);
	//	_leveling_lower = _leveling_higher = _leveling = 0.0f;
	//	_legPitch = 1;
	//} else if (reaction >= -1.75f && reaction <= -0.75f &&_legPitch != 2) {
	//	// go sitfly
	//	currentPose.morphToPose(&SitflyPose, 1.0f/0.6f);
	//	_leveling_lower = _leveling_higher = _leveling = 0.0f;
	//	_legPitch = 2;
	//}

	if (this->useWingsuit) {
		_phActor->setAngularDamping(5.0f);
	}
    if( jumper->_player )
    {
        Gameplay::iGameplay->resetActionChannels( ::iaZoomIn );
    }

    engine::IAnimationController* animCtrl = _clump->getAnimationController();

    // capture blend source
    animCtrl->captureBlendSrc();

    // reset animation mixer
    for( unsigned int i=0; i<engine::maxAnimationTracks; i++ )
    {
        if( animCtrl->getTrackAnimation( i ) ) animCtrl->setTrackActivity( i, false );
    }

    // setup animation cycles
    if( this->useWingsuit )
    {
        animCtrl->setTrackAnimation( ANIM_TRACK_ARCH, &trackForwardSequenceWings );
        animCtrl->setTrackAnimation( ANIM_TRACK_TRACKING, &trackDownSequenceWings );
        animCtrl->setTrackAnimation( ANIM_TRACK_SITFLY, &sitFlyingSequence );	// STUB: no sitfly with wings!!
		animCtrl->setTrackAnimation( ANIM_TRACK_FLIP, &frontFlipSequence );
    } else {
        animCtrl->setTrackAnimation( ANIM_TRACK_ARCH, &trackForwardSequence );
        animCtrl->setTrackAnimation( ANIM_TRACK_TRACKING, &trackDownSequence );
        animCtrl->setTrackAnimation( ANIM_TRACK_SITFLY, &sitFlyingSequence );
		animCtrl->setTrackAnimation( ANIM_TRACK_FLIP, &frontFlipSequence );
    }

	// setup animation track parameters
	for (int i = 0; i < ANIM_TRACK_ENUM_END; ++i) {
		animCtrl->setTrackActivity(i, true);
		animCtrl->resetTrackTime(i);
	}
    animCtrl->setTrackSpeed( ANIM_TRACK_ARCH, 1.0f );
    animCtrl->setTrackSpeed( ANIM_TRACK_TRACKING, 0.4f );
    animCtrl->setTrackSpeed( ANIM_TRACK_SITFLY, 0.4f );
	animCtrl->setTrackSpeed( ANIM_TRACK_FLIP, 0.5f );
    animCtrl->setTrackWeight( ANIM_TRACK_ARCH, 1.0f );
    animCtrl->setTrackWeight( ANIM_TRACK_TRACKING, 0.0f );
    animCtrl->setTrackWeight( ANIM_TRACK_SITFLY, 0.0f );
	animCtrl->setTrackWeight( ANIM_TRACK_FLIP, 0.0f );

	// reset
    updatePhysics();
    animCtrl->advance( 0.0f );

    // capture blend destination
    animCtrl->captureBlendDst();
    animCtrl->blend( 0.0f );
}

void Jumper::Tracking::updateBlending(float dt)
{
	engine::IAnimationController* animCtrl = _clump->getAnimationController();

	const int ANIMS = ANIM_TRACK_ENUM_END;
	float *anim_weights = new float[ANIMS];
	for (int i = 0; i < ANIMS; ++i) anim_weights[i] = 0.0f;

	// default pose
	anim_weights[currentPose.animTrack] = currentPose.animTrackWeight;

	// set weights	(anim with highest weight wins)
	PoseAnim *d = &currentPose.anims;
	while (d != NULL) {
		float weight = d->animTrackWeight * d->value;
		if (d->animTrack != -1 && weight > anim_weights[d->animTrack]) {
			anim_weights[d->animTrack] = weight;
		}

		d = d->next;
	}

	// add weight from morph
	if (currentPose.targetPose != NULL) {
		anim_weights[currentPose.targetPose->animTrack] = currentPose.target_prog;
	}

	// set default pose weight
	int current = currentPose.animTrack;
	// set other weights
	for (int i = 0; i < ANIMS; ++i) {
		if (i != current) {
			anim_weights[current] -= anim_weights[i];
		}
	}

	// set weights
	for (int i = 0; i < ANIMS; ++i) {
		animCtrl->setTrackWeight(i, anim_weights[i]);
	}

	// debug
	//if (this->_jumper->_debug_window) {
	//	gui::IGuiPanel* panel = this->_jumper->_debug_window->getPanel()->find( "Message" );
	//	assert( panel && panel->getStaticText() );
	//	panel->getStaticText()->setText( wstrformat(L"weights: %2.2f %2.2f %2.2f", anim_weights[0], anim_weights[1], anim_weights[2]).c_str() );
	//}
}

void Jumper::Tracking::updateProceduralAnimation(float dt)
{
	// CONTROLS
	SpinalCord *spine = _jumper->getSpinalCord();
	// levelling
	if (spine->wlo || spine->hook) {
		if (spine->wlo) {
			_leveling -= 0.5f * dt;
			if (_leveling < -1.0f) _leveling = -1.0f;
		} else if (spine->hook) {
			_leveling += 0.5f * dt;
			if (_leveling > 1.0f) _leveling = 1.0f;
		}
		if (_leveling > 0.0f) {
			_leveling_higher = _leveling;
			_leveling_lower = 0.0f;
		} else if (_leveling < 0.0f) {
			_leveling_higher = 0.0f;
			_leveling_lower = -_leveling;
		} else {
			_leveling_lower = _leveling_higher = 0.0f;
		}
	}
	// pitch control
	if (spine->up || spine->down || (!spine->modifier && _leg_pitch > 0.0f)) {
		if (spine->down || (!spine->modifier && _leg_pitch > 0.0f)) {
			_leg_pitch -= 0.5f * dt;
			if (_leg_pitch < -1.0f) _leg_pitch = -1.0f;
		} else if (spine->up) {
			_leg_pitch += 0.5f * dt;
			if (_leg_pitch > 1.0f) _leg_pitch = 1.0f;
		}
	}
	// limit pitch
	//if (_leg_pitch < 0.0f) _leg_pitch = 0.0f;

	// carving control
	if ((spine->up || spine->down) && spine->modifier) {
		if (spine->down) {
			_carving_forward -= 0.5f * dt;
			if (_carving_forward < -1.0f) _carving_forward = -1.0f;
		} else if (spine->up) {
			_carving_forward += 0.5f * dt;
			if (_carving_forward > 1.0f) _carving_forward = 1.0f;
		}
	}
	if ((spine->left || spine->right) && spine->modifier) {
		if (spine->left) {
			_carving_sideways -= 0.45f * dt;
			if (_carving_sideways < -1.0f) _carving_sideways = -1.0f;
		} else if (spine->right) {
			_carving_sideways += 0.4f * dt;
			if (_carving_sideways > 1.0f) _carving_sideways = 1.0f;
		}
	}

	// wing area control
	if (spine->leftWarp || spine->rightWarp) {
		if (spine->leftWarp) {
			_wing_area -= 0.75f * dt;
			if (_wing_area < -1.0f) _wing_area = -1.0f;
		} else if (spine->rightWarp) {
			_wing_area += 0.75f * dt;
			if (_wing_area > 1.0f) _wing_area = 1.0f;
		}
	}

	// only decrease wing area 
	if (_wing_area > 0.0f) _wing_area = 0.0f;

	// control safeguard
	if (!this->useWingsuit && spine->modifier > 0.0f) {
		spine->up = spine->down = 0.0f;
	}

	if ((machine_state() <= MACHINE_STATIC_SITFLY || machine_state() == MACHINE_TEMP_TRACKING) && spine->trigger_modifier) {
		this->_tracking += 1.4f * dt;
		if (this->_tracking > 1.0f) this->_tracking = 1.0f;
	} else {
		this->_tracking -= 3.5f * dt;
		if (this->_tracking < 0.0f) this->_tracking = 0.0f;
	}
	

	// ANIMATIONS
	currentPose.controlPose(dt);
	if (currentPose.targetPose != NULL && currentPose.target_prog >= 1.0f) {		// end of morph
		currentPose = *currentPose.targetPose;
		currentPose.target_prog = 0.0f;
		currentPose.target_rate = 0.0f;
		currentPose.targetPose = NULL;
		currentPose.restPose = NULL;
		currentPose.controlPose(0);
	}

	// rotate joints
	for (int i = 0; i < POSEANIM_JOINTS_ALL; ++i) {
		_jumper->animBodyPart(currentPose.jointnames[i], wrap(currentPose.jointaxis[i]), currentPose.jointangles[i]);
	}

	// traverse anims
	PoseAnim *d = &currentPose.anims;
	while (d != NULL) {
		// look for pose transfer
		//if (currentPose.targetPose == NULL && d->prev_value > d->value) {
		//	NxVec3 motionDir = _phActor->getLinearVelocity();
		//	motionDir.normalize();
		//	NxVec3 canopyDown  = _phActor->getGlobalPose().M.getColumn(2);
		//	canopyDown.normalize();
		//	float reaction = canopyDown.dot( motionDir );

		//	if (_legPitch != 0 && reaction > -0.25f && reaction < 0.25f) {
		//		currentPose.morphToPose(&ArchPose, 1.0f/0.5f);
		//		_legPitch = 0;
		//	} else if (_legPitch != 1 && reaction > 0.75f && reaction < 1.25f) {
		//		currentPose.morphToPose(&HeaddownPose, 1.0f/0.5f);
		//		_legPitch = 1;
		//	}
		//}
		// blocks
		if (d->only_if_not_zero != NULL) {
			if (*d->only_if_not_zero == 0.0f) {
				d = d->next;
				continue;
			}
		}
		if (d->only_if_zero != NULL) {
			if (*d->only_if_zero != 0.0f) {
				d = d->next;
				continue;
			}
		}


		float value = d->value;
		// control!
		if (value != 0.0f) {
			for (int i = 0; i < POSEANIM_JOINTS_ALL; ++i) {
				// rotate joints
				this->_jumper->animBodyPart(currentPose.jointnames[i], wrap(d->jointaxis[i]), d->jointangles[i] * value);
			}
		}

		d = d->next;
	}


	machine();


	// altitude [m]
	//NxVec3 pos = _phActor->getGlobalPosition();
	//NxVec3 velocity = _phActor->getLinearVelocity();
	//float altitude = pos.y;
	//const float AirDensity = altitude <= 10000.0f ? (1.196f - 0.0000826f * altitude) : (0.27f);
	
	// tunnel
	//velocity.y = -56.0f;
	
	// add wind
	//NxVec3 wind = _jumper->getScene()->getWindAtPoint( _phActor->getGlobalPose().t );
	//velocity += wind;

	// speed (m/s) squared
	//float Vsq = velocity.magnitudeSquared();


	// drag coeff
	//float Cd = 0.4f;	// altitude suit = 0.4f
	//const float mTracking = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->mTracking;
	//Cd *= mTracking;

	// Drag force vector
	//NxVec3 VecFd = -velocity;
	//VecFd.normalize();

	// add drag force
	//float velo_avg = 0.0f;
	//for (int i = 0; i < POSEANIM_JOINTS; ++i) {
	//	velo_avg += _phActor->getPointVelocity(this->currentPose.airfoils_now[i]).magnitude();
	//}
	//velo_avg /= (float)POSEANIM_JOINTS;

	//for (int i = 0; i < POSEANIM_JOINTS; ++i) {
	//	float Fd = 0.5f * AirDensity * Vsq * Cd * this->currentPose.airfoils_area_now[i];

	//	// move body part if drag is high
	//	if (i == POSEANIM_JOINT_LLEG || i == POSEANIM_JOINT_RLEG) {
	//		float value = -100.0f * (1.0f - _phActor->getPointVelocity(this->currentPose.airfoils_now[i]).magnitude() / velo_avg);
	//		getCore()->logMessage("velo: %2.2f", value);
	//		if (value > 5.0f) {
	//			value = 5.0f;
	//		} else if (value < -5.0f) {
	//			value = -5.0f;
	//		}
	//		body_part_deflection[i] += value;
	//		this->_jumper->animBodyPart(currentPose.jointnames[i], Vector3f(0.0f,0.0f,1.0f), body_part_deflection[i]);
	//	}
	//}
}



void Jumper::Tracking::update(float dt)
{
	updateAnimation( dt );

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

void Jumper::Tracking::updatePhysics(void)
{
	SpinalCord *spine = _jumper->getSpinalCord();
	/// PHYSICS
	// altitude [m]
	NxVec3 pos = _phActor->getGlobalPosition();
	NxVec3 velocity = _phActor->getLinearVelocity();
	float altitude = pos.y;
	
	this->useWingsuit = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->wingsuit;

	// fake forces
	//currentPose.controlPose(_jumper->getDeltaTime());	// remove for optimization sake (test effects first)
	// torque
	if (!currentPose.torque.isZero()) {
		_phActor->addLocalTorque(currentPose.torque);
	}
	// forces
	if (!currentPose.force.isZero()) {
		_phActor->addLocalForce(currentPose.force);
	}

	//const float roll_power = 650.0f;
	//const float front_roll_power = 500.0f;
	//const float back_roll_power = 430.0f;
	//if (_jumper->getSpinalCord()->leftWarp && _jumper->getSpinalCord()->leftRearRiser) {
	//	if (_current_pose == POSE_ARCH || _current_pose == POSE_BACKFLY) {
	//		_phActor->addLocalTorque(NxVec3(0.0f, 0.0f, -900.0f));
	//	} else {
	//		_phActor->addLocalTorque(NxVec3(0.0f, 900.0f, 0.0f));
	//	}
	//} else 	if (_jumper->getSpinalCord()->rightWarp && _jumper->getSpinalCord()->rightRearRiser) {
	//	if (_current_pose == POSE_ARCH || _current_pose == POSE_BACKFLY) {
	//		_phActor->addLocalTorque(NxVec3(0.0f, 0.0f, 900.0f));
	//	} else {
	//		_phActor->addLocalTorque(NxVec3(0.0f, -900.0f, 0.0f));
	//	}
	//}
	//if (!spine->modifier && spine->leftWarp && spine->rightWarp) {	// front roll
	//	float turnx = front_roll_power;
	//	_phActor->addLocalTorque(NxVec3(turnx, 0.0f, 0.0f));

	//} else if (!spine->modifier && spine->leftRearRiser && spine->rightRearRiser) {	// back roll
	//	float turnx = -back_roll_power;
	//	_phActor->addLocalTorque(NxVec3(turnx, 0.0f, 0.0f));
	//}

	// air density: converted to linear from barometric equation [0:10] km altitude
	// http://www.denysschen.com/catalogue/density.aspx
	const float AirDensityOld = altitude <= 10000.0f ? (1.196f - 0.0000826f * altitude) : (0.27f);
	
	//int b = 0;
	//float alt_layers[] = {0.0f, 11000.0f, 20000.0f, 32000.0f, 47000.0f, 51000.0f, 71000.0f, -1.0f};
	//while (altitude > alt_layers[b] && alt_layers[b] > 0.0f) ++b;

	//float mass_density[] =    { 1.2250f, 0.36391f, 0.08803f, 0.01322f, 0.00143f, 0.00086f, 0.000064f };
	//float standard_temp[] =   {  288.15f,  216.65f,  216.65f, 228.65f, 270.65f, 270.65f, 214.65f};
	//float lapse_rate[] =	  { -0.0065f, 0.0f, 0.001f, 0.0028f, 0.0f, -0.0028f, -0.002f };

	//float AirDensity = 0.0f;
	//if (lapse_rate[b] != 0.0f) {
	//	AirDensity = mass_density[b] * ((standard_temp[b] + (lapse_rate[b] * (altitude - alt_layers[b]))) / standard_temp[b]);
	//	const float power = (-9.80665f*0.0289644f) / (8.31432f* lapse_rate[b]);
	//	AirDensity = powf(AirDensity, power);
	//} else {
	//	AirDensity = mass_density[b] * expf( (-9.80665f*0.0289644f*(altitude - alt_layers[b])) / (8.31432f*standard_temp[b]) );
	//}

	// tunnel
	//velocity.y = -56.0f;
	
	// add wind
	const NxVec3 wind = _jumper->getScene()->getWindAtPoint( _phActor->getGlobalPose().t );
	velocity += wind;

	// speed (m/s) squared
	const float Vsq = velocity.magnitudeSquared();

	// get relative angle
	float angle = 0.0f;

	//velocity = _phActor->getLinearVelocity();
	NxVec3 jumperAt = wrap(_jumper->getClump()->getFrame()->getAt());
	NxVec3 jumperUp = wrap(_jumper->getClump()->getFrame()->getUp());

	NxMat34 globalPose = _phActor->getGlobalPose();
    NxVec3 x = globalPose.M.getColumn(0);
    NxVec3 y = globalPose.M.getColumn(2);
    NxVec3 z = -globalPose.M.getColumn(1);

	velocity.normalize();
    angle = ::calcAngle( z, velocity, x );


	// DRAG
	// Fd = cosf(aoa*piover180)^2*(1-Fdmax)+Fdmin
	// Fd = cos(x*(pi/180))^2*0.6+0.2
	// Fl = powf(sinf(angle*piover180*4.0f),2)*(Flmax-Flmin)+Flmin
	// Fl = sin(x*2*(pi/180))^2 * (0.8-0.0)+0.0
	const float Fdmax = 1.0f;
	const float Fdmin = 0.5f;
	// LIFT
	const float Flmax = 0.8f;
	const float Flmin = 0.0f;

	// drag fraction
	const float piover180 = 0.0174532925f;
	const float Dfrac = powf(cosf(angle*piover180),2)*(Fdmax-Fdmin)+Fdmin;

	// lift fraction
	const float Lfrac = (powf(sinf(angle*piover180*2.0f),2)*(Flmax-Flmin)+Flmin) * 0.36f;

	// total area
	float total_area = 0.0f;
	for (int i = 0; i < POSEANIM_JOINTS; ++i) {
		total_area += this->currentPose.airfoils_area_now[i];
	}

	if (_jumper->_debug_window) {
		gui::IGuiPanel* panel = _jumper->_debug_window->getPanel()->find( "Message" );
		assert( panel && panel->getStaticText() );
		//panel->getStaticText()->setText( wstrformat(L"aoa: %2.2f; dragf: %2.2f; liftf: %2.2f; pitch: %2.2f", angle, Dfrac, Lfrac, _leg_pitch).c_str() );
		
		//panel->getStaticText()->setText( wstrformat(L"Pitch: %2.2f\nLeveling: %2.2f", -this->_leg_pitch, this->_leveling).c_str() );
		
		//panel->getStaticText()->setText( wstrformat(L"D1: %2.3f\nD2: %2.3f\nA: %2.3f", AirDensity, AirDensityOld, total_area).c_str() );
		//panel->getStaticText()->setText( wstrformat(L"L: %2.2f\nR: %2.2f\nU: %2.2f\n", 
		//	Gameplay::iGameplay->getActionChannel( iaLeftJoy )->getAmplitude(),
		//	Gameplay::iGameplay->getActionChannel( iaRightJoy )->getAmplitude(),
		//	Gameplay::iGameplay->getActionChannel( iaBackwardJoy )->getAmplitude()
			//Gameplay::iGameplay->getActionChannel( iaBackwardJoy )->getAmplitude()
			//).c_str() );
		;
	}

	// drag coeff
	float Cd = 0.4f;	// altitude suit = 0.4f
	const float mTracking = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->mTracking;
	Cd *= mTracking;

	// Drag force vector
	NxVec3 VecFd = -velocity;
	VecFd.normalize();

	// add drag force
	float Fd_total = 0.0f;
	for (int i = 0; i < POSEANIM_JOINTS; ++i) {
		float Fd = 0.5f * AirDensityOld * Vsq * Cd * this->currentPose.airfoils_area_now[i];
		Fd_total += Fd;
		NxVec3 Drag = Fd*VecFd * (1.0f - this->currentPose.L_now) * Dfrac;

		// torso position is middle between all other airfoils
		//if (i == POSEANIM_JOINT_TORSO) {
		//	NxVec3 torso = NxVec3(0,0,0);
		//	for (int a = 0; a < POSEANIM_JOINTS; ++a) {
		//		if (a != POSEANIM_JOINT_TORSO) {
		//			torso += this->currentPose.airfoils_now[a];
		//		}
		//	}
		//	torso /= (float)(POSEANIM_JOINTS-1);
		//	_phActor->addForceAtLocalPos(Drag, torso);
		//} else {
			_phActor->addForceAtLocalPos(Drag, this->currentPose.airfoils_now[i]);
		//}
	}

	// convert some drag to lift
	// reaction
	NxVec3 motionDir = velocity;
	motionDir.normalize();
	NxVec3 canopyDown  = _phActor->getGlobalPose().M.getColumn(2);
	canopyDown.normalize();
	float reaction = canopyDown.dot( motionDir );
	
	reaction += currentPose.reaction_now;



	//_phActor->addLocalForce(NxVec3(0,0,1) * Fd_total * Lfrac);

	NxVec3 Flift = NxVec3(0,0,0);
	if (reaction != 0.0f) {
		//Flift = NxVec3(0,1,0) * Fd_total * reaction * this->currentPose.L_now * 1.8f;
		
		Flift = NxVec3(0,0,1) * Fd_total * Lfrac * (1.0f + this->currentPose.L_now);
		if (this->useWingsuit) {
			Flift += NxVec3(0,1,0) * Fd_total * Lfrac * 0.7f;
			Flift += NxVec3(0,0,1) * Fd_total * Lfrac * (1.0f + this->currentPose.L_now) * 0.13f;
		}
		// add lift
		_phActor->addLocalForce(Flift);
	}

	// wind tunnel
	//float tunnel_force = 0.5f * AirDensity * Vsq * Cd * Fd_total;
}

void Jumper::Tracking::changePose(int pose, float rate, bool force) {
	//getCore()->logMessage("changePose1(%2d, %2.2f)", pose, rate);

	// bail if already morphing, trying to morph to NULL or trying to morph at zero rate
	// or trying to change to the same pose as current
	if ((!force && currentPose.targetPose != NULL) || rate <= 0.0f || pose == _current_pose) return;
	
	//getCore()->logMessage("changePose2(%2d, %2.2f)", pose, rate);

	Pose *targetPose = NULL;
	if (pose == POSE_ARCH) {
		targetPose = &ArchPose;
	} else if (pose == POSE_HEADDOWN) {
		targetPose = &HeaddownPose;
	} else if (pose == POSE_BACKFLY) {
		targetPose = &BackflyPose;
	} else if (pose == POSE_SITFLY) {
		targetPose = &SitflyPose;
	} else if (pose == POSE_FLIP) {
		targetPose = &FrontFlipPose;
	}


	// invalid pose id
	if (targetPose == NULL) return;
	//getCore()->logMessage("changePose3(%2d, %2.2f)", pose, rate);

	// change
	currentPose.morphToPose(targetPose, 1.0f/0.6f, force);
	_leveling_lower = _leveling_higher = _leveling = 0.0f;
	_current_pose = pose;


	// send pose
	if (_jumper->isPlayer()) {
		NetworkData *packet = new NetworkData;
		packet->timestamp = Gameplay::iGameplay->getGlobalTime();
		packet->receiver_type = NET_REC_JUMPER;
		packet->receiver_id = 1;
		packet->data_type = 6;	// jumper pose
		packet->data_length = sizeof _current_pose;
		packet->data = new int;
		memcpy(packet->data, &_current_pose, sizeof _current_pose);
		//_jumper->network->sendPacket(packet);
	}
}