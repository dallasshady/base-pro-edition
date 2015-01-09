#ifndef JUPMER_MACHINE_H
#define JUPMER_MACHINE_H

// poses
enum {
	POSE_ARCH,
	POSE_HEADDOWN,
	POSE_BACKFLY,
	POSE_SITFLY,

	POSE_ENUM_END
};

// machine states
enum {
	// static states (just falling)
	MACHINE_STATIC_ARCH,
	MACHINE_STATIC_HEADDOWN,
	MACHINE_STATIC_BACKFLY,
	MACHINE_STATIC_SITFLY,

	// temporary states (tracking, backfly tracking, pitch up/down/left/right, flipping front/back/left/right
	MACHINE_TEMP_PITCHED_DOWN,
	MACHINE_TEMP_PITCHED_UP,
	MACHINE_TEMP_PITCHED_LEFT,
	MACHINE_TEMP_PITCHED_RIGHT,
	MACHINE_TEMP_TRACKING,
	MACHINE_TEMP_BACKFLY_TRACKING,
	MACHINE_TEMP_FRONTFLIP,
	MACHINE_TEMP_BACKFLIP,
	MACHINE_TEMP_LEFTFLIP,
	MACHINE_TEMP_RIGHTFLIP
};

void Jumper::Tracking::machine() {
	SpinalCord *spine = _jumper->getSpinalCord();

	NxVec3 motionDir = _phActor->getLinearVelocity();
	motionDir.normalize();
	NxVec3 canopyDown  = _phActor->getGlobalPose().M.getColumn(2);
	canopyDown.normalize();
	float reaction = canopyDown.dot( motionDir );// + currentPose.reaction;

	Vector3f charAt = _clump->getFrame()->getAt(); 
	charAt.normalize();
	if (this->_jumper->_debug_window) {
		gui::IGuiPanel* panel = this->_jumper->_debug_window->getPanel()->find( "Message" );
		assert( panel && panel->getStaticText() );
		panel->getStaticText()->setText( wstrformat(L"reaction: %2.2f (%2.2f %2.2f %2.2f)", reaction, charAt[0], charAt[1], charAt[2]).c_str() );
	}

	if (currentPose.targetPose != NULL) return;

	if (useWingsuit) return;



	// falling


	// tracking

	// choosing

	// roll
	// flipping (barrel rolling)
	



	return;
	// switch poses
	// arch
	if (_current_pose == POSE_ARCH) {
		// go headdown
		if (_jumper->getSpinalCord()->leftWarp && _jumper->getSpinalCord()->rightWarp) {
			currentPose.morphToPose(&HeaddownPose, 1.0f/1.2f);
			_leveling_lower = _leveling_higher = _leveling = 0.0f;
			_current_pose = POSE_HEADDOWN;
		// go backfly
		} else if (_jumper->getSpinalCord()->leftWarp && _jumper->getSpinalCord()->leftRearRiser) {
			currentPose.morphToPose(&BackflyPose, 1.0f/0.6f);
			_leveling_lower = _leveling_higher = _leveling = 0.0f;
			_current_pose = POSE_BACKFLY;
		// go sitfly
		} else if (_jumper->getSpinalCord()->leftRearRiser && _jumper->getSpinalCord()->rightRearRiser) {
			currentPose.morphToPose(&SitflyPose, 1.0f/0.8f);
			_leveling_lower = _leveling_higher = _leveling = 0.0f;
			_current_pose = POSE_SITFLY;
		}
	// headdown
	} else if (_current_pose == POSE_HEADDOWN) {
		// go backfly
		if (_jumper->getSpinalCord()->leftWarp && _jumper->getSpinalCord()->rightWarp) {
			currentPose.morphToPose(&BackflyPose, 1.0f/0.8f);
			_leveling_lower = _leveling_higher = _leveling = 0.0f;
			_current_pose = POSE_BACKFLY;
		// go arch
		} else if (_jumper->getSpinalCord()->leftRearRiser && _jumper->getSpinalCord()->rightRearRiser) {
			currentPose.morphToPose(&ArchPose, 1.0f/1.05f);
			_leveling_lower = _leveling_higher = _leveling = 0.0f;
			_current_pose = POSE_ARCH;
		}
	// sitfly
	} else if (_current_pose == POSE_SITFLY) {
		// go arch
		if (_jumper->getSpinalCord()->leftWarp && _jumper->getSpinalCord()->rightWarp) {
			currentPose.morphToPose(&ArchPose, 1.0f/1.05f);
			_leveling_lower = _leveling_higher = _leveling = 0.0f;
			_current_pose = POSE_ARCH;
		// go backfly
		} else if (_jumper->getSpinalCord()->leftRearRiser && _jumper->getSpinalCord()->rightRearRiser) {
			currentPose.morphToPose(&BackflyPose, 1.0f/0.7f);
			_leveling_lower = _leveling_higher = _leveling = 0.0f;
			_current_pose = POSE_BACKFLY;
		}
	// backfly
	} else if (_current_pose == POSE_BACKFLY) {
		// go sitfly
		if (_jumper->getSpinalCord()->leftWarp && _jumper->getSpinalCord()->rightWarp) {
			currentPose.morphToPose(&SitflyPose, 1.0f/0.8f);
			_leveling_lower = _leveling_higher = _leveling = 0.0f;
			_current_pose = POSE_SITFLY;
		// go arch
		} else if (_jumper->getSpinalCord()->leftWarp && _jumper->getSpinalCord()->leftRearRiser) {
			currentPose.morphToPose(&ArchPose, 1.0f/0.6f);
			_leveling_lower = _leveling_higher = _leveling = 0.0f;
			_current_pose = POSE_ARCH;
		// go headdown
		} else if (_jumper->getSpinalCord()->leftRearRiser && _jumper->getSpinalCord()->rightRearRiser) {
			currentPose.morphToPose(&HeaddownPose, 1.0f/1.2f);
			_leveling_lower = _leveling_higher = _leveling = 0.0f;
			_current_pose = POSE_HEADDOWN;
		}
	}
}

#endif