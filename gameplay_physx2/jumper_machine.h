#ifndef JUPMER_MACHINE_H
#define JUPMER_MACHINE_H

// poses
enum {
	POSE_ARCH,
	POSE_HEADDOWN,
	POSE_BACKFLY,
	POSE_SITFLY,
	POSE_ENUM_END,		// end of regular poses

	POSE_FLIP,
	POSE_ENUM_END_ALL	// end of all poses including temporary poses
};

// machine states
enum {
	// meta states (states that happen only a frame and switches to another state which happens in the same frame)
	MACHINE_META_POSITION,

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
	MACHINE_TEMP_RIGHTFLIP,
};

// actions
enum {
	MACHINE_ACTION_PITCH_DOWN,
	MACHINE_ACTION_PITCH_UP,
	MACHINE_ACTION_PITCH_LEFT,
	MACHINE_ACTION_PITCH_RIGHT,
	MACHINE_ACTION_TRACK,
	MACHINE_ACTION_TORQUE_FRONT,
	MACHINE_ACTION_TORQUE_BACK,
	MACHINE_ACTION_TORQUE_LEFT,
	MACHINE_ACTION_TORQUE_RIGHT,

	MACHINE_ACTION_ENUM_END
};

#define MACHINE_ACTIONS MACHINE_ACTION_ENUM_END 	// number of possible action

// sets actions to true which are initiated
void Jumper::Tracking::machineActions(bool actions[], SpinalCord *spine) {
	// reset actions
	//for (int i = 0; i < MACHINE_ACTIONS; ++i) {
	//	actions[i] = false;
	//}
	
	// set actions
	actions[MACHINE_ACTION_PITCH_DOWN]	= !spine->trigger_modifier
										&& spine->trigger_leftWarp
										&& spine->trigger_rightWarp
										&& !spine->trigger_leftRearRiser
										&& !spine->trigger_rightRearRiser
										&& !spine->trigger_up
										&& !spine->trigger_down;
	actions[MACHINE_ACTION_PITCH_UP]	= !spine->trigger_modifier
										&& spine->trigger_leftRearRiser
										&& spine->trigger_rightRearRiser
										&& !spine->trigger_leftWarp
										&& !spine->trigger_rightWarp
										&& !spine->trigger_up
										&& !spine->trigger_down;
	actions[MACHINE_ACTION_PITCH_LEFT]	= !spine->trigger_modifier
										&& spine->trigger_leftWarp
										&& spine->trigger_leftRearRiser
										&& !spine->trigger_rightWarp
										&& ! spine->trigger_rightRearRiser;
	actions[MACHINE_ACTION_PITCH_RIGHT]	= !spine->trigger_modifier
										&& spine->trigger_rightWarp
										&& spine->trigger_rightRearRiser
										&& !spine->trigger_leftWarp
										&& ! spine->trigger_leftRearRiser;
	
	actions[MACHINE_ACTION_TRACK]		= spine->trigger_modifier;

	actions[MACHINE_ACTION_TORQUE_FRONT]= !spine->trigger_modifier
										&& spine->trigger_up;
	actions[MACHINE_ACTION_TORQUE_BACK]= !spine->trigger_modifier
										&& spine->trigger_down;
	actions[MACHINE_ACTION_TORQUE_LEFT]= !spine->trigger_modifier
										&& spine->trigger_left;
	actions[MACHINE_ACTION_TORQUE_RIGHT]= !spine->trigger_modifier
										&& spine->trigger_right;
}

// THE machine
void Jumper::Tracking::machine() {
	SpinalCord *spine = _jumper->getSpinalCord();

	// machine action for this frame
	bool actions[MACHINE_ACTIONS];
	machineActions(actions, spine);

	// CONDITIONS
	// torque
	const NxMat34 freeFallPose = _jumper->getFreefallActor()->getGlobalPose();
    const NxVec3 spin_axis_x = freeFallPose.M.getColumn(0);
	const NxVec3 spin_axis_z = freeFallPose.M.getColumn(2);
	const float SPIN_X = spin_axis_x.dot( _jumper->getFreefallActor()->getAngularVelocity() );
	const float SPIN_Z = spin_axis_z.dot( _jumper->getFreefallActor()->getAngularVelocity() );
	const float spin_for_fb_flip = 2.5f;	// minimum spin for a front/back flip
	const float spin_for_lr_flip = 3.8f;	// minimum spin for a left/right flip

	// get relative angle
	float angle = 0.0f;

	NxVec3 velocity = _phActor->getLinearVelocity();
	NxVec3 jumperAt = wrap(_jumper->getClump()->getFrame()->getAt());
	NxVec3 jumperUp = wrap(_jumper->getClump()->getFrame()->getUp());

	NxMat34 globalPose = _phActor->getGlobalPose();
    NxVec3 x = globalPose.M.getColumn(0);
    NxVec3 y = globalPose.M.getColumn(2);
    NxVec3 z = -globalPose.M.getColumn(1);

	velocity.normalize();
    angle = ::calcAngle( z, velocity, x );

	//// META STATES
	// pose choose
	if (this->machine_state() == MACHINE_META_POSITION) {
		int pose = POSE_ARCH;

		// switch by angle to relative wind
		// -45..45..135..180..-180..-135..-45
		// AAAAAAAHHHHHBBBBB  BBBBBBBBBBSSSSS
		if (this->machine_previuos_state() != MACHINE_TEMP_PITCHED_DOWN &&
			this->machine_previuos_state() != MACHINE_TEMP_PITCHED_UP &&
			this->machine_previuos_state() != MACHINE_TEMP_PITCHED_LEFT &&
			this->machine_previuos_state() != MACHINE_TEMP_PITCHED_RIGHT) 
		{

			if (angle >= -45.0f && angle < 45.0f) {
				pose = POSE_ARCH;
			} else if (angle >= 45.0f && angle < 135.0f) {
				pose = POSE_HEADDOWN;
			} else if (angle >= 135 || angle < -135) {
				pose = POSE_BACKFLY;
			} else if (angle >= -135 && angle < -45) {
				pose = POSE_SITFLY;
			}
		// switch after pitching
		} else {
			pose = _current_pose;
			if (this->machine_previuos_state() == MACHINE_TEMP_PITCHED_DOWN) {
				++pose;
			} else if (this->machine_previuos_state() == MACHINE_TEMP_PITCHED_UP) {
				--pose;
			} else if (this->machine_previuos_state() == MACHINE_TEMP_PITCHED_LEFT ||
					   this->machine_previuos_state() == MACHINE_TEMP_PITCHED_RIGHT) {
				pose += 2;
			}

			if (pose >= POSE_ENUM_END) {
				pose -= POSE_ENUM_END;
			} else if (pose < 0) {
				pose += POSE_ENUM_END;
			}
		}

		// change pose
		// disable non-wingsuit poses
		if (this->useWingsuit && (pose == POSE_SITFLY || pose == POSE_HEADDOWN)) {
			pose = POSE_ARCH;
		}

		// do not go to temporary poses
		changePose(pose, 1.0f/0.5f);
		if ( _current_pose != pose ) {
			changePose(pose, 1.0f/0.5f, true);
		}

		// set machine state
		// go to tracking pose instead of arch if modifier
		if (actions[MACHINE_ACTION_TRACK] && pose == POSE_ARCH) {
			machine_state(MACHINE_TEMP_TRACKING);
		} else {
			machine_state(MACHINE_STATIC_ARCH);
		}
	}

	// STATIC ARCH
	if (this->machine_state() == MACHINE_STATIC_ARCH) {
		if (_fresh_state) {
			_fresh_state = false;
		}
		assert( _current_pose < POSE_ENUM_END );
		
		if (actions[MACHINE_ACTION_TORQUE_FRONT]) {
			_phActor->addLocalTorque(NxVec3(130.0f, 0.0f, 0.0f));
		}

		// pitch down
		if (actions[MACHINE_ACTION_PITCH_DOWN]) {
			machine_state(MACHINE_TEMP_PITCHED_DOWN);
		// pitch up
		} else if (actions[MACHINE_ACTION_PITCH_UP]) {
			machine_state(MACHINE_TEMP_PITCHED_UP);
		// pitch left
		} else if (actions[MACHINE_ACTION_PITCH_LEFT]) {
			machine_state(MACHINE_TEMP_PITCHED_LEFT);
		// pitch right
		} else if (actions[MACHINE_ACTION_PITCH_RIGHT]) {
			machine_state(MACHINE_TEMP_PITCHED_RIGHT);
		// belly tracking
		} else if (actions[MACHINE_ACTION_TRACK]) {
			machine_state(MACHINE_TEMP_TRACKING);
		// front torque
		} else if (actions[MACHINE_ACTION_TORQUE_FRONT] && SPIN_X >= spin_for_fb_flip) {
			machine_state(MACHINE_TEMP_FRONTFLIP);
		// back torque
		} else if (actions[MACHINE_ACTION_TORQUE_BACK] && SPIN_X <= -spin_for_fb_flip) {
			machine_state(MACHINE_TEMP_BACKFLIP);
		// left torque
		} else if (_current_pose != POSE_HEADDOWN &&  _current_pose != POSE_SITFLY && actions[MACHINE_ACTION_TORQUE_LEFT] && SPIN_Z >= spin_for_lr_flip) {
			machine_state(MACHINE_TEMP_LEFTFLIP);
		// right torque
		} else if (_current_pose != POSE_HEADDOWN &&  _current_pose != POSE_SITFLY && actions[MACHINE_ACTION_TORQUE_RIGHT] && SPIN_Z <= -spin_for_lr_flip) {
			machine_state(MACHINE_TEMP_RIGHTFLIP);
		}
	}

	// STATIC HEADDOWN
	else if (this->machine_state() == MACHINE_STATIC_HEADDOWN) {

	}

	// STATIC BACKFLY
	else if (this->machine_state() == MACHINE_STATIC_BACKFLY) {

	}

	// STATIC SITFLY
	else if (this->machine_state() == MACHINE_STATIC_SITFLY) {

	}

	// TEMP PITCHED DOWN
	else if (this->machine_state() == MACHINE_TEMP_PITCHED_DOWN) {
		if (!actions[MACHINE_ACTION_PITCH_DOWN]) {
			machine_state(MACHINE_META_POSITION);
		}
	}

	// TEMP PITCHED UP
	else if (this->machine_state() == MACHINE_TEMP_PITCHED_UP) {
		if (!actions[MACHINE_ACTION_PITCH_UP]) {
			machine_state(MACHINE_META_POSITION);
		}
	}

	// TEMP PITCHED LEFT
	else if (this->machine_state() == MACHINE_TEMP_PITCHED_LEFT) {
		if (!actions[MACHINE_ACTION_PITCH_LEFT]) {
			machine_state(MACHINE_META_POSITION);
		}
	}

	// TEMP PICHED RIGHT
	else if (this->machine_state() == MACHINE_TEMP_PITCHED_RIGHT) {
		if (!actions[MACHINE_ACTION_PITCH_RIGHT]) {
			machine_state(MACHINE_META_POSITION);
		}
	}

	// TEMP TRACKING
	else if (this->machine_state() == MACHINE_TEMP_TRACKING) {
		if (!actions[MACHINE_ACTION_TRACK]) {
			machine_state(MACHINE_STATIC_ARCH);
		}
	}

	// TEMP BACKLFY TRACKING
	else if (this->machine_state() == MACHINE_TEMP_BACKFLY_TRACKING) {
		if (!actions[MACHINE_ACTION_TRACK]) {
			machine_state(MACHINE_STATIC_BACKFLY);
		}
	}

	// TEMP FRONT FLIP
	else if (this->machine_state() == MACHINE_TEMP_FRONTFLIP) {
		if (_fresh_state) {
			changePose(POSE_FLIP, 1.0f/0.5f, true);
			_fresh_state = false;
		}
		if (actions[MACHINE_ACTION_TRACK] || SPIN_X < spin_for_fb_flip && !actions[MACHINE_ACTION_TORQUE_FRONT]) {
			machine_state(MACHINE_META_POSITION);
		} else {
			if (actions[MACHINE_ACTION_TORQUE_FRONT]) {
				float torque = 5000.0f;
				if (_current_pose == POSE_HEADDOWN) {
					torque = 10000.0f;
				}
				_phActor->addLocalTorque(NxVec3(torque, 0.0f, 0.0f));
			}
		}
	}

	// TEMP BACK FLIP
	else if (this->machine_state() == MACHINE_TEMP_BACKFLIP) {
		if (_fresh_state) {
			changePose(POSE_FLIP, 1.0f/0.5f, true);
			_fresh_state = false;
		}
		if (actions[MACHINE_ACTION_TRACK] || SPIN_X > -spin_for_fb_flip && !actions[MACHINE_ACTION_TORQUE_BACK]) {
			machine_state(MACHINE_META_POSITION);
		} else {
			if (actions[MACHINE_ACTION_TORQUE_BACK]) {
				float torque = -5000.0f;
				if (_current_pose == POSE_HEADDOWN) {
					torque = -10000.0f;
				}
				_phActor->addLocalTorque(NxVec3(torque, 0.0f, 0.0f));
			}
		}
	}

	// TEMP LEFT FLIP
	else if (this->machine_state() == MACHINE_TEMP_LEFTFLIP) {
		if (_fresh_state) {
			changePose(POSE_FLIP, 1.0f/0.5f, true);
			_fresh_state = false;
		}
		if (actions[MACHINE_ACTION_TRACK] || SPIN_Z > -spin_for_lr_flip && !actions[MACHINE_ACTION_TORQUE_LEFT]) {
			machine_state(MACHINE_META_POSITION);
		}
	}

	// TEMP RIGHT FLIP
	else if (this->machine_state() == MACHINE_TEMP_RIGHTFLIP) {
		if (_fresh_state) {
			changePose(POSE_FLIP, 1.0f/0.5f, true);
			_fresh_state = false;
		}
		if (actions[MACHINE_ACTION_TRACK] || SPIN_Z < spin_for_lr_flip && !actions[MACHINE_ACTION_TORQUE_RIGHT]) {
			machine_state(MACHINE_META_POSITION);
		}
	}

}

void Jumper::Tracking::machine_state(int state) {
	if (state == this->_machine_state) return;

	_fresh_state = true;

	// meta states don't count as previous (they are meta, duh!)
	if (this->_machine_state_previous != MACHINE_META_POSITION)	this->_machine_state_pre_previous = this->_machine_state_previous;
	if (this->_machine_state != MACHINE_META_POSITION)	this->_machine_state_previous = this->_machine_state;

	this->_machine_state = state;


	const char states[][30] = {"MACHINE_META_POSITION",
							"MACHINE_STATIC_ARCH",
							"MACHINE_STATIC_HEADDOWN",
							"MACHINE_STATIC_BACKFLY",
							"MACHINE_STATIC_SITFLY",
							"MACHINE_TEMP_PITCHED_DOWN",
							"MACHINE_TEMP_PITCHED_UP",
							"MACHINE_TEMP_PITCHED_LEFT",
							"MACHINE_TEMP_PITCHED_RIGHT",
							"MACHINE_TEMP_TRACKING",
							"MACHINE_TEMP_BACKFLY_TRACKING",
							"MACHINE_TEMP_FRONTFLIP",
							"MACHINE_TEMP_BACKFLIP",
							"MACHINE_TEMP_LEFTFLIP",
							"MACHINE_TEMP_RIGHTFLIP"};
	//if (_jumper->isPlayer()) {
	//	getCore()->logMessage("pose: %d; state:\n%s\n%s\n%s\n", _current_pose, states[_machine_state_pre_previous], states[_machine_state_previous], states[_machine_state]);
	//}
	//if (_jumper->_debug_window) {
	//	gui::IGuiPanel* panel = _jumper->_debug_window->getPanel()->find( "Message" );
	//	assert( panel && panel->getStaticText() );
	//	//panel->getStaticText()->setText( wstrformat(L"TORQUE: %2.2f %2.2f", SPIN_X, SPIN_Z).c_str() );
	//	panel->getStaticText()->setText( wstrformat(L"pose: %d; state:\n%s\n%s\n%s\n", _current_pose, states[_machine_state_pre_previous], states[_machine_state_previous], states[_machine_state]).c_str() );
	//	//panel->getStaticText()->setText( wstrformat(L"angle: %2.2f", angle).c_str() );
	//}
}

int Jumper::Tracking::machine_state() {
	return _machine_state;
}
int Jumper::Tracking::machine_previuos_state() {
	return _machine_state_previous;
}
int Jumper::Tracking::machine_pre_previuos_state() {
	return _machine_state_pre_previous;
}
#endif