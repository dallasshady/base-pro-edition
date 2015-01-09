#ifndef JUMPER_POSES_INIT_WINGS_H
#define JUMPER_POSES_INIT_WINGS_H


void Jumper::Tracking::initPosesWings() {
	SpinalCord *spine = _jumper->getSpinalCord();
	const float mTracking = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->mTracking;
	PxVec3 tmp = PxVec3(0,0,0);	// temp vector

	// *** ARCH POSE ***
	float arch_pose_area = 1.3f;
	ArchPose.L = 0.35f;

	//arch_pose_area = 0.39f;
	//ArchPose.L = 0.35f;

	ArchPose.reaction = 0.0f;
	ArchPose.animTrack = ANIM_TRACK_ARCH;
	// head
	ArchPose.airfoils[POSEANIM_JOINT_HEAD]		= PxVec3(0.0f, 0.0f, 1.0f);	
	ArchPose.airfoils_area[POSEANIM_JOINT_HEAD] = arch_pose_area*0.0f;
	// torso (stabilizer)
	ArchPose.airfoils[POSEANIM_JOINT_TORSO]		= PxVec3(0.0f, 1.5f, 0.0f);
	ArchPose.airfoils_area[POSEANIM_JOINT_TORSO] = arch_pose_area*0.4f;
	// arms
	ArchPose.airfoils[POSEANIM_JOINT_LARM]		= PxVec3(-2.22777f, 0.52505f, 10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_LARM] = arch_pose_area*0.15f;
	ArchPose.jointaxis[POSEANIM_JOINT_LARM]		= PxVec3(0.0f, 1.0f, 1.0f);
	ArchPose.jointangles[POSEANIM_JOINT_LARM]	= 25.0f;
	ArchPose.airfoils[POSEANIM_JOINT_RARM]		= PxVec3( 2.22777f, 0.52505f, 10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_RARM] = arch_pose_area*0.15f;
	ArchPose.jointaxis[POSEANIM_JOINT_RARM]		= PxVec3(0.0f, 0.0f, 1.0f);
	ArchPose.jointangles[POSEANIM_JOINT_RARM]	= 25.0f;
	// legs
	ArchPose.airfoils[POSEANIM_JOINT_LLEG]		= PxVec3(-2.22777f, 0.52505f,-10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_LLEG] = arch_pose_area*0.15f;
	ArchPose.airfoils[POSEANIM_JOINT_RLEG]		= PxVec3( 2.22777f, 0.52505f,-10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_RLEG] = arch_pose_area*0.15f;
	ArchPose.jointaxis[POSEANIM_JOINT_LLEG]		= PxVec3(0.0f,0.0f,1.0f);
	ArchPose.jointangles[POSEANIM_JOINT_LLEG]	= -12.0f;
	ArchPose.jointaxis[POSEANIM_JOINT_RLEG]		= PxVec3(0.0f,0.0f,1.0f);
	ArchPose.jointangles[POSEANIM_JOINT_RLEG]	= -12.0f;

		// anims
	PoseAnim *APturnsanim = new PoseAnim();
	PoseAnim *APtrackinganim = new PoseAnim();
	PoseAnim *APspdbreakanim = new PoseAnim();
	PoseAnim *APbackanim = new PoseAnim();
	PoseAnim *APleveling_higher = new PoseAnim();
	PoseAnim *APwingareaanim = new PoseAnim();
	PoseAnim *APpitchfwd = new PoseAnim();
	PoseAnim *APpitchback = new PoseAnim();
	PoseAnim *AProllleftanim = new PoseAnim();
	PoseAnim *AProllrightanim = new PoseAnim();
	PoseAnim *APtrackingpitchanim = new PoseAnim();

		// turns
			// arms
	//APturnsanim->airfoils[POSEANIM_JOINT_LARM] = PxVec3(0.0f,1.0f,0.0f);
	//APturnsanim->airfoils[POSEANIM_JOINT_LARM] = PxVec3(0.0f,-1.0f,0.0f);
	//APturnsanim->jointangles[POSEANIM_JOINT_TORSO] = 20.0f;
	//APturnsanim->jointaxis[POSEANIM_JOINT_TORSO] = PxVec3(1.0f,0.0f,0.0f);

	APturnsanim->jointaxis[POSEANIM_JOINT_LARM] = PxVec3(0.0f,0.0f,1.0f);
	APturnsanim->jointaxis[POSEANIM_JOINT_RARM] = PxVec3(0.0f,0.0f,1.0f);
	APturnsanim->jointangles[POSEANIM_JOINT_LARM] = 20.0f;
	APturnsanim->jointangles[POSEANIM_JOINT_RARM] = -20.0f;

			// forces
	APturnsanim->torque = PxVec3(0, 230.0f, 0);
			// params
	APturnsanim->var_c = 2;
	APturnsanim->var_op = POSEANIM_OP_SUB;
	APturnsanim->var[0] = &spine->left;
	APturnsanim->var[1] = &spine->right;
	APturnsanim->next = APspdbreakanim;

		// speed braking
			// leg wing
	APspdbreakanim->jointaxis[POSEANIM_JOINT_LLEG]	= PxVec3(0.0f,0.0f,1.0f);
	APspdbreakanim->jointangles[POSEANIM_JOINT_LLEG]	= 20.0f;
	APspdbreakanim->jointaxis[POSEANIM_JOINT_RLEG]	= PxVec3(0.0f,0.0f,1.0f);
	APspdbreakanim->jointangles[POSEANIM_JOINT_RLEG]	= 20.0f;
	//APspdbreakanim->airfoils_area[POSEANIM_JOINT_TORSO] = 0.08f;
	APspdbreakanim->force = PxVec3(0.0f, 0.0f, -80.0f);

			// params
	APspdbreakanim->var_c = 1;
	APspdbreakanim->var_op = POSEANIM_OP_SUM;
	APspdbreakanim->var[0] = &spine->rearBrake;
	APspdbreakanim->next = APleveling_higher;

		// levelling
			// arms
	//APleveling_higher->jointaxis[POSEANIM_JOINT_LARM] = PxVec3(0.0f,0.0f,1.0f);
	//APleveling_higher->jointaxis[POSEANIM_JOINT_RARM] = PxVec3(0.0f,0.0f,1.0f);
	//APleveling_higher->jointangles[POSEANIM_JOINT_LARM] = 20.0f;
	//APleveling_higher->jointangles[POSEANIM_JOINT_RARM] = 20.0f;
			// torso
	//APleveling_higher->airfoils[POSEANIM_JOINT_TORSO] = PxVec3(0.0f,-1.0f, 0.0f);
	//APleveling_higher->airfoils_area[POSEANIM_JOINT_TORSO] = 0.2f * 0.4f;
			// forces
	APleveling_higher->force = PxVec3(0.0f, 80.0f, 0.0f);
			// params
	APleveling_higher->var_c = 2;
	APleveling_higher->var_op = POSEANIM_OP_SUB;
	APleveling_higher->var[0] = &spine->hook;
	APleveling_higher->var[1] = &spine->wlo;
	APleveling_higher->next = APtrackinganim;

		// tracking
	APtrackinganim->var_c = 1;
	APtrackinganim->var_op = POSEANIM_OP_SUM;
	APtrackinganim->var[0] = &this->_tracking;
	APtrackinganim->airfoils[POSEANIM_JOINT_LARM] = PxVec3( 0.0f,0.0f,-2.5f);
	APtrackinganim->airfoils[POSEANIM_JOINT_RARM] = PxVec3( 0.0f,0.0f,-2.5f);
	APtrackinganim->airfoils[POSEANIM_JOINT_LLEG] = PxVec3( 0.0f,0.0f,-0.5f);
	APtrackinganim->airfoils[POSEANIM_JOINT_RLEG] = PxVec3( 0.0f,0.0f,-0.5f);
	APtrackinganim->airfoils[POSEANIM_JOINT_TORSO] = PxVec3(0.0f, 0.4f, -1.8f);

	//APtrackinganim->dL = 0.45f;	// wingsuit

	APtrackinganim->animTrack = ANIM_TRACK_TRACKING;
	APtrackinganim->next = AProllleftanim;


		
		// roll left
			// arms
	AProllleftanim->jointaxis[POSEANIM_JOINT_LARM] = PxVec3(0.0f,1.0f,0.0f);
	AProllleftanim->jointaxis[POSEANIM_JOINT_RARM] = PxVec3(0.0f,1.0f,0.0f);
	//AProllleftanim->jointangles[POSEANIM_JOINT_LARM] = 40.0f;
	AProllleftanim->jointangles[POSEANIM_JOINT_LARM] = 30.0f;
	AProllleftanim->airfoils[POSEANIM_JOINT_LARM] = PxVec3(2.3f, 0.0f, 0.0f);
	AProllleftanim->airfoils[POSEANIM_JOINT_RARM] = PxVec3(2.0f, 0.0f, 0.0f);
			// legs
	AProllleftanim->jointaxis[POSEANIM_JOINT_LLEG] = PxVec3(0.0f,1.0f,0.0f);
	AProllleftanim->jointaxis[POSEANIM_JOINT_RLEG] = PxVec3(0.0f,1.0f,0.0f);
	AProllleftanim->jointangles[POSEANIM_JOINT_LLEG] = -22.0f;
	AProllleftanim->jointangles[POSEANIM_JOINT_RLEG] = 22.0f;
			// torso
	AProllleftanim->jointaxis[POSEANIM_JOINT_TORSO] = PxVec3(0.0f,0.0f,1.0f);
	AProllleftanim->jointangles[POSEANIM_JOINT_TORSO] = 8.0f;
			// forces
	AProllleftanim->torque = PxVec3(0.0f, 0.0f, -900.0f);
			// params
	AProllleftanim->var_c = 2;
	AProllleftanim->var_op = POSEANIM_OP_MIN;
	AProllleftanim->var[0] = &spine->leftWarp;
	AProllleftanim->var[1] = &spine->leftRearRiser;
	AProllleftanim->next = AProllrightanim;
	AProllleftanim->dL = -0.1f;

		// roll right
			// arms
	AProllrightanim->jointaxis[POSEANIM_JOINT_LARM] = PxVec3(0.0f,1.0f,0.0f);
	AProllrightanim->jointaxis[POSEANIM_JOINT_RARM] = PxVec3(0.0f,1.0f,0.0f);
	AProllrightanim->jointangles[POSEANIM_JOINT_RARM] = 30.0f;
	AProllrightanim->airfoils[POSEANIM_JOINT_LARM] = PxVec3(2.3f, 0.0f, 0.0f);
	AProllrightanim->airfoils[POSEANIM_JOINT_RARM] = PxVec3(2.0f, 0.0f, 0.0f);
			// legs
	AProllrightanim->jointaxis[POSEANIM_JOINT_LLEG] = PxVec3(0.0f,1.0f,0.0f);
	AProllrightanim->jointaxis[POSEANIM_JOINT_RLEG] = PxVec3(0.0f,1.0f,0.0f);
	AProllrightanim->jointangles[POSEANIM_JOINT_LLEG] = -22.0f;
	AProllrightanim->jointangles[POSEANIM_JOINT_RLEG] = 22.0f;
			// torso
	AProllrightanim->jointaxis[POSEANIM_JOINT_TORSO] = PxVec3(0.0f,0.0f,1.0f);
	AProllrightanim->jointangles[POSEANIM_JOINT_TORSO] = 8.0f;
			// forces
	AProllrightanim->torque = PxVec3(0.0f, 0.0f, 900.0f);
			// params
	AProllrightanim->var_c = 2;
	AProllrightanim->var_op = POSEANIM_OP_MIN;
	AProllrightanim->var[0] = &spine->rightWarp;
	AProllrightanim->var[1] = &spine->rightRearRiser;
	AProllrightanim->next = APtrackingpitchanim;
	AProllrightanim->dL = -0.1f;

		// tracking pitch
	APtrackingpitchanim->var_c = 1;
	APtrackingpitchanim->var_op = POSEANIM_OP_MIN;
	APtrackingpitchanim->airfoils[POSEANIM_JOINT_LARM] = PxVec3( 0.0f,0.0f,-3.5f);
	APtrackingpitchanim->airfoils[POSEANIM_JOINT_RARM] = PxVec3( 0.0f,0.0f,-3.5f);
		APtrackingpitchanim->airfoils[POSEANIM_JOINT_LARM] = PxVec3( 0.0f,0.0f,-4.5f);
		APtrackingpitchanim->airfoils[POSEANIM_JOINT_RARM] = PxVec3( 0.0f,0.0f,-4.5f);
	APtrackingpitchanim->airfoils[POSEANIM_JOINT_LLEG] = PxVec3( 0.0f,0.0f,-0.1f);
	APtrackingpitchanim->airfoils[POSEANIM_JOINT_RLEG] = PxVec3( 0.0f,0.0f,-0.1f);
		APtrackingpitchanim->airfoils[POSEANIM_JOINT_LLEG] = PxVec3( 0.0f,0.0f,-0.5f);
		APtrackingpitchanim->airfoils[POSEANIM_JOINT_RLEG] = PxVec3( 0.0f,0.0f,-0.5f);
	APtrackingpitchanim->jointangles[POSEANIM_JOINT_TORSO] = 15.0f;
	APtrackingpitchanim->jointaxis[POSEANIM_JOINT_TORSO] = PxVec3(0.0f,0.0f,1.0f);

	APtrackingpitchanim->jointaxis[POSEANIM_JOINT_LARM] = PxVec3(0.0f,0.0f,1.0f);
	APtrackingpitchanim->jointaxis[POSEANIM_JOINT_RARM] = PxVec3(0.0f,0.0f,1.0f);
	APtrackingpitchanim->jointangles[POSEANIM_JOINT_LARM] = -20.0f;
	APtrackingpitchanim->jointangles[POSEANIM_JOINT_RARM] = -20.0f;

	APtrackingpitchanim->var[0] = &this->_leg_pitch;
	APtrackingpitchanim->only_if_not_zero = &spine->modifier;
	APtrackingpitchanim->next = APpitchfwd;
	APtrackingpitchanim->dL = 0.2f;
		// pitch forward
			// torso
	APpitchfwd->airfoils[POSEANIM_JOINT_LARM] = PxVec3( 0.0f,0.0f,-6.5f);
	APpitchfwd->airfoils[POSEANIM_JOINT_RARM] = PxVec3( 0.0f,0.0f,-6.5f);
	APpitchfwd->airfoils[POSEANIM_JOINT_LLEG] = PxVec3( 0.0f,0.0f,-1.7f);
	APpitchfwd->airfoils[POSEANIM_JOINT_RLEG] = PxVec3( 0.0f,0.0f,-1.7f);
	APpitchfwd->jointangles[POSEANIM_JOINT_TORSO] = 15.0f;
	APpitchfwd->jointaxis[POSEANIM_JOINT_TORSO] = PxVec3(0.0f,0.0f,1.0f);
			// forces
	APpitchfwd->torque = PxVec3(1300.0f, 0.0f, 0.0f);
			// params
	APpitchfwd->var_c = 2;
	APpitchfwd->var_op = POSEANIM_OP_MIN;
	APpitchfwd->var[0] = &spine->leftWarp;
	APpitchfwd->var[1] = &spine->rightWarp;
	APpitchfwd->next = APwingareaanim;

		// wing area (only negative is used!)
			// arms
	APwingareaanim->jointaxis[POSEANIM_JOINT_LARM] = PxVec3(0.0f, 1.0f, 0.0f);
	APwingareaanim->jointangles[POSEANIM_JOINT_LARM] = -13.0f;
	APwingareaanim->airfoils_area[POSEANIM_JOINT_LARM] = arch_pose_area * 0.01f;
	APwingareaanim->jointaxis[POSEANIM_JOINT_RARM] = PxVec3(0.0f, 1.0f, 0.0f);
	APwingareaanim->jointangles[POSEANIM_JOINT_RARM] = -13.0f;
	APwingareaanim->airfoils_area[POSEANIM_JOINT_RARM] = arch_pose_area * 0.01f;
	//		// legs
	APwingareaanim->jointaxis[POSEANIM_JOINT_LLEG] = PxVec3(0.0f, 1.0f, 0.0f);
	APwingareaanim->jointangles[POSEANIM_JOINT_LLEG] = 6.0f;
	APwingareaanim->airfoils_area[POSEANIM_JOINT_LLEG] = arch_pose_area * 0.01f;
	APwingareaanim->jointaxis[POSEANIM_JOINT_RLEG] = PxVec3(0.0f, 1.0f, 0.0f);
	APwingareaanim->jointangles[POSEANIM_JOINT_RLEG] = -6.0f;
	APwingareaanim->airfoils_area[POSEANIM_JOINT_RLEG] = arch_pose_area * 0.01f;
			// params	
	APwingareaanim->var_c = 1;
	APwingareaanim->var_op = POSEANIM_OP_SUM;
	APwingareaanim->var[0] = &this->_wing_area;
	APwingareaanim->next = APpitchback;
	//APwingareaanim->dReaction = 0.2f;

		// pitch backward
			// torso
	//APpitchback->airfoils[POSEANIM_JOINT_LARM] = PxVec3( 0.0f,0.0f, 1.5f);
	//APpitchback->airfoils[POSEANIM_JOINT_RARM] = PxVec3( 0.0f,0.0f, 1.5f);
	//APpitchback->airfoils[POSEANIM_JOINT_LLEG] = PxVec3( 0.0f,0.0f, 0.09f);
	//APpitchback->airfoils[POSEANIM_JOINT_RLEG] = PxVec3( 0.0f,0.0f, 0.09f);
			// params	
	APpitchback->var_c = 1;
	APpitchback->var_op = POSEANIM_OP_SUM;
	APpitchback->var[0] = &this->_leg_pitch;
	APpitchback->next = NULL;

	ArchPose.anims = *APturnsanim;


	// ****** FLIP POSES ******
	// *** FRONT FLIP POSE ***
	float flip_pose_area = 0.5f;
	FrontFlipPose.L = 0.0f;
	if (useWingsuit) {		// wingsuit
		flip_pose_area += 0.3f;
		FrontFlipPose.L = 0.1f;
	} else if (mTracking > 1.0f) {	// thermal
		flip_pose_area += 0.1f; 
	}
	FrontFlipPose.reaction = 0.0f;
	FrontFlipPose.animTrack = ANIM_TRACK_FLIP;
	FrontFlipPose.animTrackWeight = 1.0f;
		// torso
	FrontFlipPose.airfoils_area[POSEANIM_JOINT_TORSO] = flip_pose_area * 0.4f;
		// arms
	FrontFlipPose.airfoils_area[POSEANIM_JOINT_LARM] = flip_pose_area * 0.15f;
	FrontFlipPose.airfoils[POSEANIM_JOINT_LARM] = PxVec3( 0.0f,  0.5f, 0.5f);
	FrontFlipPose.airfoils_area[POSEANIM_JOINT_RARM] = flip_pose_area * 0.15f;
	FrontFlipPose.airfoils[POSEANIM_JOINT_RARM] = PxVec3( 0.0f,  0.5f,-0.5f);
		// legs
	FrontFlipPose.airfoils_area[POSEANIM_JOINT_LLEG] = flip_pose_area * 0.15f;
	FrontFlipPose.airfoils[POSEANIM_JOINT_LLEG] = PxVec3( 0.0f,-0.5f, 0.5f);
	FrontFlipPose.airfoils_area[POSEANIM_JOINT_RLEG] = flip_pose_area * 0.15f;
	FrontFlipPose.airfoils[POSEANIM_JOINT_RLEG] = PxVec3( 0.0f,-0.5f,-0.5f);


	// reset poses
	ArchPose.resetPose(0);
	HeaddownPose.resetPose(0);
	SitflyPose.resetPose(0);
	BackflyPose.resetPose(0);

	FrontFlipPose.resetPose(0);
}

#endif