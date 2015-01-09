#ifndef JUMPER_POSES_INIT_H
#define JUMPER_POSES_INIT_H

// animation tracks
enum {
	ANIM_TRACK_NO_CHANGE = -1,
	ANIM_TRACK_ARCH,
	ANIM_TRACK_TRACKING,
	ANIM_TRACK_SITFLY,
	ANIM_TRACK_FLIP,

	ANIM_TRACK_ENUM_END
};

void Jumper::Tracking::initPoses() {
	SpinalCord *spine = _jumper->getSpinalCord();
	const float mTracking = database::Suit::getRecord( _jumper->getVirtues()->equipment.suit.id )->mTracking;
	NxVec3 tmp = NxVec3(0,0,0);	// temp vector

	// *** ARCH POSE ***
	// float arch_pose_area = 0.75f;
	float arch_pose_area = 1.3f;
	ArchPose.L = 0.2f;
	if (mTracking > 1.0f) {	// thermal
		arch_pose_area = 1.5f;
		ArchPose.L = 0.29f; 
	}
	ArchPose.reaction = 0.0f;
	ArchPose.animTrack = ANIM_TRACK_ARCH;
	// head
	ArchPose.airfoils[POSEANIM_JOINT_HEAD]		= NxVec3(0.0f, 0.0f, 1.0f);	
	ArchPose.airfoils_area[POSEANIM_JOINT_HEAD] = arch_pose_area*0.0f;
	// torso (stabilizer)
	ArchPose.airfoils[POSEANIM_JOINT_TORSO]		= NxVec3(0.0f, 1.5f, 0.0f);
	ArchPose.airfoils_area[POSEANIM_JOINT_TORSO] = arch_pose_area*0.4f;
	// arms
	ArchPose.airfoils[POSEANIM_JOINT_LARM]		= NxVec3(-2.22777f, 0.52505f, 10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_LARM] = arch_pose_area*0.15f;
	ArchPose.airfoils[POSEANIM_JOINT_RARM]		= NxVec3( 2.22777f, 0.52505f, 10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_RARM] = arch_pose_area*0.15f;
	// legs
	ArchPose.airfoils[POSEANIM_JOINT_LLEG]		= NxVec3(-2.22777f, 0.52505f,-10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_LLEG] = arch_pose_area*0.15f;
	ArchPose.airfoils[POSEANIM_JOINT_RLEG]		= NxVec3( 2.22777f, 0.52505f,-10.00000f);
	ArchPose.airfoils_area[POSEANIM_JOINT_RLEG] = arch_pose_area*0.15f;

		// anims
	PoseAnim *APturnsanim = new PoseAnim();
	PoseAnim *APtrackinganim = new PoseAnim();
	PoseAnim *APtrackingpitchanim = new PoseAnim();
	PoseAnim *APfwdanim = new PoseAnim();
	PoseAnim *APbackanim = new PoseAnim();
	PoseAnim *APleveling = new PoseAnim();
	PoseAnim *APtrackingleveling = new PoseAnim();
	PoseAnim *APpitchfwd = new PoseAnim();
	PoseAnim *APpitchback = new PoseAnim();
	PoseAnim *AProllleftanim = new PoseAnim();
	PoseAnim *AProllrightanim = new PoseAnim();

		// turns
	APturnsanim->jointangles[POSEANIM_JOINT_LARM] = 20.0f;
	APturnsanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,0.0f,1.0f);
	APturnsanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,0.0f);
	APturnsanim->jointangles[POSEANIM_JOINT_RARM] = -20.0f;
	APturnsanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,0.0f,1.0f);
	APturnsanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,-1.0f,0.0f);
	APturnsanim->torque = NxVec3(0, 100.0f, 0);
	APturnsanim->var_c = 2;
	APturnsanim->var_op = POSEANIM_OP_SUB;
	APturnsanim->var[0] = &spine->left;
	APturnsanim->var[1] = &spine->right;
	APturnsanim->next = APtrackinganim;

		// tracking
	APtrackinganim->var_c = 1;
	APtrackinganim->var_op = POSEANIM_OP_SUM;
	APtrackinganim->var[0] = &spine->modifier;
	APtrackinganim->airfoils[POSEANIM_JOINT_LARM] = NxVec3( 0.0f,0.0f,-2.5f);
	APtrackinganim->airfoils[POSEANIM_JOINT_RARM] = NxVec3( 0.0f,0.0f,-2.5f);
	APtrackinganim->airfoils[POSEANIM_JOINT_LLEG] = NxVec3( 0.0f,0.0f,-0.1f);
	APtrackinganim->airfoils[POSEANIM_JOINT_RLEG] = NxVec3( 0.0f,0.0f,-0.1f);
	//APtrackinganim->airfoils[POSEANIM_JOINT_TORSO] = NxVec3(0.0f, -1.5f, -1.0f);
	//if (useWingsuit) {	
	//	APtrackinganim->dL = 0.0f;	// wingsuit
	//} else if (mTracking > 1.0) {
	//	APtrackinganim->dL = 0.0f;	// tracksuit
	//} else {
	//	APtrackinganim->dL = 0.0f;	// altitude suit
	//}
	APtrackinganim->animTrack = ANIM_TRACK_TRACKING;
	APtrackinganim->next = APtrackingpitchanim;

		// tracking pitch
	APtrackingpitchanim->var_c = 1;
	APtrackingpitchanim->var_op = POSEANIM_OP_MIN;
	APtrackingpitchanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3( 0.0f,0.0f,-2.5f);
	APtrackingpitchanim->airfoils[POSEANIM_JOINT_RARM] = NxVec3( 0.0f,0.0f,-2.5f);
	APtrackingpitchanim->airfoils[POSEANIM_JOINT_LLEG] = NxVec3( 0.0f,0.0f,-0.1f);
	APtrackingpitchanim->airfoils[POSEANIM_JOINT_RLEG] = NxVec3( 0.0f,0.0f,-0.1f);
	APtrackingpitchanim->jointangles[POSEANIM_JOINT_TORSO] = 15.0f;
	APtrackingpitchanim->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	APtrackingpitchanim->var[0] = &this->_leg_pitch;
	APtrackingpitchanim->only_if_not_zero = &spine->modifier;
	APtrackingpitchanim->next = APfwdanim;

		// forward/backward
	APfwdanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,0.0f,-0.2f);
	APfwdanim->airfoils[POSEANIM_JOINT_RARM] = NxVec3(0.0f,0.0f,-0.2f);
	APfwdanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,0.0f);
	APfwdanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,1.0f,0.0f);
	APfwdanim->jointangles[POSEANIM_JOINT_LARM] = 35.0f;
	APfwdanim->jointangles[POSEANIM_JOINT_RARM] = 35.0f;
	APfwdanim->var_c = 2;
	APfwdanim->var_op = POSEANIM_OP_SUB;
	APfwdanim->var[0] = &spine->up;
	APfwdanim->var[1] = &spine->down;
	APfwdanim->next = APleveling;
	APfwdanim->force = NxVec3(0.0f, 0.0f, 100.0f);
		// levelling
			// arms
	APleveling->airfoils[POSEANIM_JOINT_LARM] = NxVec3(-0.1f,-0.4f, 0.1f);
	APleveling->airfoils[POSEANIM_JOINT_RARM] = NxVec3( 0.1f,-0.4f, 0.1f);
	APleveling->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(1.0f,0.0f,0.0f);
	APleveling->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(1.0f,0.0f,0.0f);
	APleveling->jointangles[POSEANIM_JOINT_LARM] = -30.0f;
	APleveling->jointangles[POSEANIM_JOINT_RARM] = -30.0f;
	APleveling->airfoils_area[POSEANIM_JOINT_LARM] = 0.2f * 0.15f;
	APleveling->airfoils_area[POSEANIM_JOINT_RARM] = 0.2f * 0.15f;
			// legs
	APleveling->airfoils[POSEANIM_JOINT_LLEG] = NxVec3(-0.1f,-0.4f, 0.1f);
	APleveling->airfoils[POSEANIM_JOINT_RLEG] = NxVec3( 0.1f,-0.4f, 0.1f);
	APleveling->airfoils_area[POSEANIM_JOINT_LLEG] = 0.2f * 0.15f;
	APleveling->airfoils_area[POSEANIM_JOINT_RLEG] = 0.2f * 0.15f;
	APleveling->jointaxis[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,0.0f,1.0f);
	APleveling->jointaxis[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,0.0f,1.0f);
	APleveling->jointangles[POSEANIM_JOINT_LLEG] = -30.0f;
	APleveling->jointangles[POSEANIM_JOINT_RLEG] = -30.0f;

			// torso
	APleveling->airfoils[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,-0.1f, 0.0f);
	APleveling->airfoils_area[POSEANIM_JOINT_TORSO] = 0.2f * 0.4f;
			// params
	APleveling->var_c = 1;
	APleveling->var_op = POSEANIM_OP_MIN;
	APleveling->var[0] = &this->_leveling;
	APleveling->only_if_zero = &spine->modifier;
	APleveling->next = APtrackingleveling;
		
		// tracking levelling
	APtrackingleveling->var_c = 1;
	APtrackingleveling->var_op = POSEANIM_OP_MIN;
	APtrackingleveling->var[0] = &this->_leveling;
	APtrackingleveling->only_if_not_zero = &spine->modifier;
		// arms
	APtrackingleveling->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,1.0f);
	APtrackingleveling->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,1.0f,1.0f);
	APtrackingleveling->jointangles[POSEANIM_JOINT_LARM] = 25.0f;
	APtrackingleveling->jointangles[POSEANIM_JOINT_RARM] = 17.0f;
	APtrackingleveling->next = AProllleftanim;
	APtrackingleveling->airfoils_area[POSEANIM_JOINT_TORSO] = 0.2f * 0.4f;

		// roll left
			// arms
	AProllleftanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,0.0f);
	AProllleftanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,1.0f,0.0f);
	AProllleftanim->jointangles[POSEANIM_JOINT_LARM] = -40.0f;
	AProllleftanim->jointangles[POSEANIM_JOINT_RARM] = 50.0f;
	AProllleftanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(2.3f, 0.0f, 0.0f);
	AProllleftanim->airfoils[POSEANIM_JOINT_RARM] = NxVec3(2.0f, 0.0f, 0.0f);
			// legs
	AProllleftanim->jointaxis[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,1.0f,0.0f);
	AProllleftanim->jointaxis[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,1.0f,0.0f);
	AProllleftanim->jointangles[POSEANIM_JOINT_LLEG] = -22.0f;
	AProllleftanim->jointangles[POSEANIM_JOINT_RLEG] = 22.0f;
			// torso
	AProllleftanim->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	AProllleftanim->jointangles[POSEANIM_JOINT_TORSO] = 8.0f;
			// forces
	AProllleftanim->torque = NxVec3(0.0f, 0.0f, -900.0f);
	AProllleftanim->targetPose = &BackflyPose;

			// params
	AProllleftanim->var_c = 2;
	AProllleftanim->var_op = POSEANIM_OP_MIN;
	AProllleftanim->var[0] = &spine->leftWarp;
	AProllleftanim->var[1] = &spine->leftRearRiser;
	AProllleftanim->next = AProllrightanim;
		// roll right
			// arms
	AProllrightanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,0.0f);
	AProllrightanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,1.0f,0.0f);
	AProllrightanim->jointangles[POSEANIM_JOINT_LARM] = 50.0f;
	AProllrightanim->jointangles[POSEANIM_JOINT_RARM] = -40.0f;
	AProllrightanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(2.0f, 0.0f, 0.0f);
	AProllrightanim->airfoils[POSEANIM_JOINT_RARM] = NxVec3(2.3f, 0.0f, 0.0f);
			// legs
	AProllrightanim->jointaxis[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,1.0f,0.0f);
	AProllrightanim->jointaxis[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,1.0f,0.0f);
	AProllrightanim->jointangles[POSEANIM_JOINT_LLEG] = -22.0f;
	AProllrightanim->jointangles[POSEANIM_JOINT_RLEG] = 22.0f;
			// torso
	AProllrightanim->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	AProllrightanim->jointangles[POSEANIM_JOINT_TORSO] = 8.0f;
			// forces
	AProllrightanim->torque = NxVec3(0.0f, 0.0f, 900.0f);
	//AProllrightanim->targetPose = &BackflyPose;

			// params
	AProllrightanim->var_c = 2;
	AProllrightanim->var_op = POSEANIM_OP_MIN;
	AProllrightanim->var[0] = &spine->rightWarp;
	AProllrightanim->var[1] = &spine->rightRearRiser;
	AProllrightanim->next = APpitchfwd;

		// pitch forward
			// torso
	APpitchfwd->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	APpitchfwd->jointangles[POSEANIM_JOINT_TORSO] = 30.0f;
	APpitchfwd->airfoils[POSEANIM_JOINT_TORSO]  = NxVec3(0.0f, 0.0f, -3.0f);
			// params	
	APpitchfwd->var_c = 2;
	APpitchfwd->var_op = POSEANIM_OP_MIN;
	APpitchfwd->var[0] = &spine->leftWarp;
	APpitchfwd->var[1] = &spine->rightWarp;
	APpitchfwd->next = APpitchback;
			// params
	APpitchfwd->dReaction = -0.7f;

		// pitch backward
			// torso
	APpitchback->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	APpitchback->jointangles[POSEANIM_JOINT_TORSO] = 30.0f;
	APpitchback->airfoils[POSEANIM_JOINT_TORSO]  = NxVec3(0.0f, 0.0f, 3.0f);
			// params	
	APpitchback->var_c = 2;
	APpitchback->var_op = POSEANIM_OP_MIN;
	APpitchback->var[0] = &spine->leftRearRiser;
	APpitchback->var[1] = &spine->rightRearRiser;
	APpitchback->next = NULL;
			// params
	APpitchback->dReaction = 0.7f;

	ArchPose.anims = *APturnsanim;



	// *** HEADDOWN POSE ***
	float headdown_pose_area = arch_pose_area * 0.91f;
	HeaddownPose.L = 0.2f;
	HeaddownPose.reaction = -1.0f;
	HeaddownPose.animTrack = ANIM_TRACK_TRACKING;
	// head
	HeaddownPose.airfoils[POSEANIM_JOINT_HEAD]		= NxVec3(0.0f, 0.0f, 0.0f);	
	HeaddownPose.airfoils_area[POSEANIM_JOINT_HEAD] = headdown_pose_area*0.0f;
	// torso (stabilizer)
	HeaddownPose.airfoils[POSEANIM_JOINT_TORSO]		= NxVec3(0.0f, 0.0f, 0.6f);
	HeaddownPose.airfoils_area[POSEANIM_JOINT_TORSO] = headdown_pose_area*0.4f;
	// arms
	HeaddownPose.airfoils[POSEANIM_JOINT_LARM]		= NxVec3(-0.22777f, 0.0f, -2.00000f);
	HeaddownPose.airfoils_area[POSEANIM_JOINT_LARM] = headdown_pose_area*0.15f;
	HeaddownPose.airfoils[POSEANIM_JOINT_RARM]		= NxVec3( 0.22777f, 0.0f, -2.00000f);
	HeaddownPose.airfoils_area[POSEANIM_JOINT_RARM] = headdown_pose_area*0.15f;
	// legs
	HeaddownPose.airfoils[POSEANIM_JOINT_LLEG]		= NxVec3(-0.22777f, 0.0f,-6.00000f);
	HeaddownPose.jointaxis[POSEANIM_JOINT_LLEG]		= NxVec3(0.0f,0.0f,1.0f);
	HeaddownPose.jointangles[POSEANIM_JOINT_LLEG]	= -22.0f;
	HeaddownPose.airfoils_area[POSEANIM_JOINT_LLEG] = headdown_pose_area*0.15f;
	HeaddownPose.airfoils[POSEANIM_JOINT_RLEG]		= NxVec3( 0.22777f, 0.0f,-6.00000f);
	HeaddownPose.airfoils_area[POSEANIM_JOINT_RLEG] = headdown_pose_area*0.15f;
	HeaddownPose.jointaxis[POSEANIM_JOINT_RLEG]		= NxVec3(0.0f,0.0f,1.0f);
	HeaddownPose.jointangles[POSEANIM_JOINT_RLEG]	= -22.0f;
	// head
	HeaddownPose.jointaxis[POSEANIM_JOINT_HEAD]		= NxVec3(0.0f,0.0f,0.1f);
	HeaddownPose.jointangles[POSEANIM_JOINT_HEAD]	= 15.0f;
	// arms
	HeaddownPose.jointangles[POSEANIM_JOINT_LARM]	= 20.0f;
	HeaddownPose.jointaxis[POSEANIM_JOINT_LARM]		= NxVec3(0.0f,0.5f,0.5f);
	HeaddownPose.jointangles[POSEANIM_JOINT_RARM]	= 20.0f;
	HeaddownPose.jointaxis[POSEANIM_JOINT_RARM]		= NxVec3(0.0f,0.0f,1.0f);
	
		// anims
	PoseAnim *HDturnsanim = new PoseAnim();
	PoseAnim *HDtrackkanim = new PoseAnim();
	PoseAnim *HDlevelinganim = new PoseAnim();
	PoseAnim *HDcarvinganim = new PoseAnim();
	PoseAnim *HDcarvingSidewaysanim = new PoseAnim();
	PoseAnim *HDcarvingForwardanim = new PoseAnim();

		// turns
	HDturnsanim->jointangles[POSEANIM_JOINT_LARM] = 20.0f;
	HDturnsanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,0.0f,1.0f);
	HDturnsanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,0.0f);
	HDturnsanim->jointangles[POSEANIM_JOINT_RARM] = -20.0f;
	HDturnsanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,0.0f,1.0f);
	HDturnsanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,-1.0f,0.0f);
	HDturnsanim->torque = NxVec3(0, 0, 40.0f);
	HDturnsanim->var_c = 2;
	HDturnsanim->var_op = POSEANIM_OP_SUB;
	HDturnsanim->var[0] = &spine->left;
	HDturnsanim->var[1] = &spine->right;
	HDturnsanim->only_if_zero = &spine->modifier;
	HDturnsanim->next = HDtrackkanim;
		// forward/backward
	HDtrackkanim->jointangles[POSEANIM_JOINT_LLEG]	= 15.0f;
	HDtrackkanim->jointaxis[POSEANIM_JOINT_LLEG]	= NxVec3(0.0f,0.0f,1.0f);
	HDtrackkanim->airfoils[POSEANIM_JOINT_LLEG]		= NxVec3(0.0f,-0.9f,0.0f);
	HDtrackkanim->jointangles[POSEANIM_JOINT_RLEG]	= 15.0f;
	HDtrackkanim->jointaxis[POSEANIM_JOINT_RLEG]	= NxVec3(0.0f,0.0f,1.0f);
	HDtrackkanim->airfoils[POSEANIM_JOINT_RLEG]		= NxVec3(0.0f,-0.9f,0.0f);
	HDtrackkanim->airfoils[POSEANIM_JOINT_TORSO]	= NxVec3(0.0f,0.6f,0.0f);
	HDtrackkanim->var_c = 2;
	HDtrackkanim->var_op = POSEANIM_OP_SUB;
	HDtrackkanim->var[0] = &spine->up;
	HDtrackkanim->var[1] = &spine->down;
	HDtrackkanim->dL = 0.0f;
	HDtrackkanim->next = HDcarvingSidewaysanim;
	HDtrackkanim->force = NxVec3(0.0f, -100.0f, 0.0f);
		// carving
	HDcarvinganim->jointangles[POSEANIM_JOINT_LLEG]	= 30.0f;
	HDcarvinganim->jointaxis[POSEANIM_JOINT_LLEG]	= NxVec3(0.0f,0.0f,1.0f);
	HDcarvinganim->airfoils[POSEANIM_JOINT_LLEG]	= NxVec3(0.0f,-0.9f,0.0f);
	HDcarvinganim->jointangles[POSEANIM_JOINT_RLEG]	= 30.0f;
	HDcarvinganim->jointaxis[POSEANIM_JOINT_RLEG]	= NxVec3(0.0f,0.0f,1.0f);
	HDcarvinganim->airfoils[POSEANIM_JOINT_RLEG]	= NxVec3(0.0f,-0.9f,0.0f);
	HDcarvinganim->airfoils[POSEANIM_JOINT_TORSO]	= NxVec3(0.0f,0.6f,0.0f);
	HDcarvinganim->var_c = 1;
	HDcarvinganim->var_op = POSEANIM_OP_SUB;
	HDcarvinganim->var[0] = &spine->modifier;
	HDcarvinganim->dL = 0.0f;
	HDcarvinganim->next = HDcarvingSidewaysanim;
		// carving sideways
	HDcarvingSidewaysanim->jointangles[POSEANIM_JOINT_LARM] = 20.0f;
	HDcarvingSidewaysanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,0.0f,1.0f);
	HDcarvingSidewaysanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,0.0f);
	HDcarvingSidewaysanim->jointangles[POSEANIM_JOINT_RARM] = -20.0f;
	HDcarvingSidewaysanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,0.0f,1.0f);
	HDcarvingSidewaysanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,-1.0f,0.0f);
	HDcarvingSidewaysanim->torque = NxVec3(0, 0, 40.0f);
	HDcarvingSidewaysanim->force = NxVec3(800.0f, 0.0f, 0.0f);
	HDcarvingSidewaysanim->var_c = 1;
	HDcarvingSidewaysanim->var_op = POSEANIM_OP_SUM;
	HDcarvingSidewaysanim->var[0] = &this->_carving_sideways;
	HDcarvingSidewaysanim->only_if_not_zero = &spine->modifier;
	HDcarvingSidewaysanim->next = HDcarvingForwardanim;
		// carving forward
	HDcarvingForwardanim->jointangles[POSEANIM_JOINT_LKNEE] = 20.0f;
	HDcarvingForwardanim->jointaxis[POSEANIM_JOINT_LKNEE] = NxVec3(0.0f,0.0f,1.0f);
	HDcarvingForwardanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,0.0f);
	HDcarvingForwardanim->jointangles[POSEANIM_JOINT_RKNEE] = -20.0f;
	HDcarvingForwardanim->jointaxis[POSEANIM_JOINT_RKNEE] = NxVec3(0.0f,0.0f,1.0f);
	HDcarvingForwardanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,-1.0f,0.0f);
	HDcarvingForwardanim->force = NxVec3(600.0f, 0.0f, 0.0f);
	HDcarvingForwardanim->torque = NxVec3(0, 0, -40.0f);
	HDcarvingForwardanim->var_c = 1;
	HDcarvingForwardanim->var_op = POSEANIM_OP_SUM;
	HDcarvingForwardanim->var[0] = &this->_carving_forward;
	HDcarvingForwardanim->only_if_not_zero = &spine->modifier;
	HDcarvingForwardanim->next = HDlevelinganim;

		// leveling
	HDlevelinganim->airfoils_area[POSEANIM_JOINT_TORSO] = headdown_pose_area*0.3f;
	HDlevelinganim->var_c = 1;
	HDlevelinganim->var[0] = &this->_leveling;
	HDlevelinganim->next = NULL;
		// arms
	HDlevelinganim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,1.0f);
	HDlevelinganim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,1.0f,1.0f);
	HDlevelinganim->jointangles[POSEANIM_JOINT_LARM] = -20.0f;
	HDlevelinganim->jointangles[POSEANIM_JOINT_RARM] = -12.0f;
	HDlevelinganim->jointaxis[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,1.0f,0.0f);
	HDlevelinganim->jointaxis[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,1.0f,0.0f);
	HDlevelinganim->jointangles[POSEANIM_JOINT_LLEG] = 12.0f;
	HDlevelinganim->jointangles[POSEANIM_JOINT_RLEG] = -7.0f;

	HeaddownPose.anims = *HDturnsanim;


	// *** SITFLY POSE ***
	float sitfly_pose_area = arch_pose_area*0.91f;
	SitflyPose.L = 0.3f;
	SitflyPose.reaction = 1.0f;
	SitflyPose.animTrack = ANIM_TRACK_SITFLY;

	// head
	SitflyPose.airfoils[POSEANIM_JOINT_HEAD]		= NxVec3(0.0f, 0.0f, 0.0f);	
	SitflyPose.airfoils_area[POSEANIM_JOINT_HEAD] = sitfly_pose_area*0.0f;
	// torso (stabilizer)
	SitflyPose.airfoils[POSEANIM_JOINT_TORSO]		= NxVec3(0.0f, 0.0f, 2.0f);
	SitflyPose.airfoils_area[POSEANIM_JOINT_TORSO]  = sitfly_pose_area*0.4f;
	SitflyPose.jointaxis[POSEANIM_JOINT_TORSO]      = NxVec3(0.0f,0.0f,1.0f);
	SitflyPose.jointangles[POSEANIM_JOINT_TORSO]    = -23.0f;
	// arms
	SitflyPose.airfoils[POSEANIM_JOINT_LARM]		= NxVec3(-0.22777f, 0.0f, 10.00000f);
	SitflyPose.airfoils_area[POSEANIM_JOINT_LARM] = sitfly_pose_area*0.15f;
	SitflyPose.airfoils[POSEANIM_JOINT_RARM]		= NxVec3( 0.22777f, 0.0f, 10.00000f);
	SitflyPose.airfoils_area[POSEANIM_JOINT_RARM] = sitfly_pose_area*0.15f;
	// legs
	SitflyPose.airfoils[POSEANIM_JOINT_LLEG]		= NxVec3(-0.22777f, 0.0f,-10.00000f);
	//SitflyPose.jointaxis[POSEANIM_JOINT_LLEG]		= NxVec3(0.0f,0.0f,1.0f);
	//SitflyPose.jointangles[POSEANIM_JOINT_LLEG]	= -22.0f;
	SitflyPose.airfoils_area[POSEANIM_JOINT_LLEG] = sitfly_pose_area*0.15f;
	SitflyPose.airfoils[POSEANIM_JOINT_RLEG]		= NxVec3( 0.22777f, 0.0f,-10.00000f);
	SitflyPose.airfoils_area[POSEANIM_JOINT_RLEG] = sitfly_pose_area*0.15f;

		// anims
	PoseAnim *SFturnsanim = new PoseAnim();
	PoseAnim *SFtrackinganim = new PoseAnim();
	PoseAnim *SFfwdanim = new PoseAnim();
	PoseAnim *SFbackanim = new PoseAnim();
	PoseAnim *SFleveling_lower = new PoseAnim();
	PoseAnim *SFleveling_higher = new PoseAnim();
	PoseAnim *SFpitchfwd = new PoseAnim();
	PoseAnim *SFpitchback = new PoseAnim();

		// turns
			// legs
	SFturnsanim->airfoils[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,0.0f,-0.13f);
	SFturnsanim->airfoils[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,0.0f,-0.13f);
	SFturnsanim->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	SFturnsanim->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	SFturnsanim->jointangles[POSEANIM_JOINT_TORSO] = -73.0f;
	SFturnsanim->jointangles[POSEANIM_JOINT_TORSO] = 13.0f;
		// turns
	SFturnsanim->jointangles[POSEANIM_JOINT_LARM] = 13.0f;
	SFturnsanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,0.0f,1.0f);
	SFturnsanim->jointangles[POSEANIM_JOINT_RARM] = -13.0f;
	SFturnsanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,0.0f,1.0f);
			// forces
	SFturnsanim->torque = NxVec3(0, 0, 40.0f);
			// params
	SFturnsanim->var_c = 2;
	SFturnsanim->var_op = POSEANIM_OP_SUB;
	SFturnsanim->var[0] = &spine->left;
	SFturnsanim->var[1] = &spine->right;
	SFturnsanim->next = SFfwdanim;

	SitflyPose.anims = *SFturnsanim;

		// forward/backward
			// arms
	SFfwdanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,0.0f,1.0f);
	SFfwdanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,0.0f,1.0f);
	SFfwdanim->jointangles[POSEANIM_JOINT_LARM] = 13.0f;
	SFfwdanim->jointangles[POSEANIM_JOINT_RARM] = 13.0f;
			// legs
	SFfwdanim->airfoils[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,0.1f,0.0f);
	SFfwdanim->airfoils[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,0.1f,0.0f);
	SFfwdanim->jointaxis[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,0.0f,1.0f);
	SFfwdanim->jointaxis[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,0.0f,1.0f);
	SFfwdanim->jointangles[POSEANIM_JOINT_LLEG] = 23.0f;
	SFfwdanim->jointangles[POSEANIM_JOINT_RLEG] = 23.0f;
			// torso
	SFfwdanim->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	SFfwdanim->jointangles[POSEANIM_JOINT_TORSO] = -10.0f;
	SFfwdanim->airfoils[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.08f,0.0f);
			// forces
	SFfwdanim->force = NxVec3(0.0f, -140.0f, 0.0f);

	SFfwdanim->var_c = 2;
	SFfwdanim->var_op = POSEANIM_OP_SUB;
	SFfwdanim->var[0] = &spine->up;
	SFfwdanim->var[1] = &spine->down;
	SFfwdanim->next = SFleveling_lower;

		// levelling lower
			// arms
	SFleveling_lower->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(1.0f,-1.0f,0.0f);
	SFleveling_lower->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(1.0f,-1.0f,0.0f);
	SFleveling_lower->jointangles[POSEANIM_JOINT_LARM] = 55.0f;
	SFleveling_lower->jointangles[POSEANIM_JOINT_RARM] = 55.0f;
			// legs
	tmp = NxVec3(-1.0f,0.0f,1.0f);
	tmp.normalize();
	SFleveling_lower->jointaxis[POSEANIM_JOINT_LLEG] = tmp;
	tmp = NxVec3(1.2f,0.0f,1.0f);
	tmp.normalize();
	SFleveling_lower->jointaxis[POSEANIM_JOINT_RLEG] = tmp;
	SFleveling_lower->jointangles[POSEANIM_JOINT_LLEG] = 48.0f;
	SFleveling_lower->jointangles[POSEANIM_JOINT_RLEG] = 48.0f;

			// torso
	SFleveling_lower->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	SFleveling_lower->jointangles[POSEANIM_JOINT_TORSO] = 23.0f;
	SFleveling_lower->airfoils_area[POSEANIM_JOINT_TORSO] = -0.15f * 0.4f;
			// params
	SFleveling_lower->var_c = 1;
	SFleveling_lower->var_op = POSEANIM_OP_SUM;
	SFleveling_lower->var[0] = &this->_leveling_lower;
	SFleveling_lower->next = SFleveling_higher;


		// levelling higher
			// arms
	SFleveling_higher->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,0.0f,-1.0f);
	SFleveling_higher->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,0.0f,-1.0f);
	SFleveling_higher->jointangles[POSEANIM_JOINT_LARM] = -55.0f;
	SFleveling_higher->jointangles[POSEANIM_JOINT_RARM] = -55.0f;
			// legs
	tmp = NxVec3(-1.0f,0.0f,1.0f);
	tmp.normalize();
	SFleveling_higher->jointaxis[POSEANIM_JOINT_LLEG] = tmp;
	tmp = NxVec3(1.2f,0.0f,1.0f);
	tmp.normalize();
	SFleveling_higher->jointaxis[POSEANIM_JOINT_RLEG] = tmp;
	SFleveling_higher->jointangles[POSEANIM_JOINT_LLEG] = -15.0f;
	SFleveling_higher->jointangles[POSEANIM_JOINT_RLEG] = -15.0f;

			// torso
	SFleveling_higher->airfoils_area[POSEANIM_JOINT_TORSO] = 0.15f * 0.4f;
	SFleveling_higher->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	SFleveling_higher->jointangles[POSEANIM_JOINT_TORSO] = -6.0f;

			// params
	SFleveling_higher->var_c = 1;
	SFleveling_higher->var_op = POSEANIM_OP_SUM;
	SFleveling_higher->var[0] = &this->_leveling_higher;
	SFleveling_higher->next = NULL;




	// *** BACKFLY POSE ***
	float backfly_pose_area = arch_pose_area*0.99f;
	BackflyPose.L = 0.2f;
	if (useWingsuit) {		// wingsuit
		backfly_pose_area = 1.3f;
		BackflyPose.L = 0.3f;
	} else if (mTracking > 1.0f) {	// thermal
		//backfly_pose_area = arch_pose_area*0.99f;		// arch_pose_area increases already 
		BackflyPose.L = 0.29f; 
	}
	BackflyPose.reaction = 0.0f;
	BackflyPose.animTrack = ANIM_TRACK_SITFLY;
	// head
	BackflyPose.airfoils[POSEANIM_JOINT_HEAD]		= NxVec3(0.0f, 0.0f, 1.0f);	
	BackflyPose.airfoils_area[POSEANIM_JOINT_HEAD] = backfly_pose_area*0.0f;
	// torso (stabilizer)
	BackflyPose.airfoils[POSEANIM_JOINT_TORSO]		= NxVec3(0.0f, -0.5f, 0.0f);
	BackflyPose.airfoils_area[POSEANIM_JOINT_TORSO] = backfly_pose_area*0.4f;
	BackflyPose.jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f, 0.0f, 1.0f);
	BackflyPose.jointangles[POSEANIM_JOINT_TORSO] = -15.0f;
	// arms
	BackflyPose.airfoils[POSEANIM_JOINT_LARM]		= NxVec3(-2.22777f, -0.52505f, 10.00000f);
	BackflyPose.airfoils_area[POSEANIM_JOINT_LARM] = backfly_pose_area*0.15f;
	BackflyPose.airfoils[POSEANIM_JOINT_RARM]		= NxVec3( 2.22777f, -0.52505f, 10.00000f);
	BackflyPose.airfoils_area[POSEANIM_JOINT_RARM] = backfly_pose_area*0.15f;
	tmp = NxVec3(0.0f, 0.5f, -0.7f);
	tmp.normalize();
	BackflyPose.jointangles[POSEANIM_JOINT_LARM] = -75.0f;
	BackflyPose.jointaxis[POSEANIM_JOINT_LARM] = tmp;
	BackflyPose.jointangles[POSEANIM_JOINT_RARM] = -75.0f;
	BackflyPose.jointaxis[POSEANIM_JOINT_RARM] = tmp;
	// legs
	BackflyPose.airfoils[POSEANIM_JOINT_LLEG]		= NxVec3(-2.22777f, -0.52505f,-10.00000f);
	BackflyPose.airfoils_area[POSEANIM_JOINT_LLEG] = backfly_pose_area*0.15f;
	BackflyPose.airfoils[POSEANIM_JOINT_RLEG]		= NxVec3( 2.22777f, -0.52505f,-10.00000f);
	BackflyPose.airfoils_area[POSEANIM_JOINT_RLEG] = backfly_pose_area*0.15f;

		// anims
	PoseAnim *BFturnsanim = new PoseAnim();
	PoseAnim *BFtrackinganim = new PoseAnim();
	PoseAnim *BFfwdanim = new PoseAnim();
	PoseAnim *BFbackanim = new PoseAnim();
	PoseAnim *BFleveling = new PoseAnim();
	PoseAnim *BFpitchfwd = new PoseAnim();
	PoseAnim *BFpitchback = new PoseAnim();
	PoseAnim *BFrollrightanim = new PoseAnim();
	PoseAnim *BFrollleftanim = new PoseAnim();

		// turns
			// legs
	BFturnsanim->airfoils[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,0.0f,-0.13f);
	BFturnsanim->airfoils[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,0.0f,-0.13f);
	BFturnsanim->jointaxis[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,0.0f,1.0f);
	BFturnsanim->jointaxis[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,0.0f,1.0f);
	BFturnsanim->jointangles[POSEANIM_JOINT_LLEG] = 13.0f;
	BFturnsanim->jointangles[POSEANIM_JOINT_RLEG] = -13.0f;
			// forces
	BFturnsanim->torque = NxVec3(0, -100.0f, 0);
			// params
	BFturnsanim->var_c = 2;
	BFturnsanim->var_op = POSEANIM_OP_SUB;
	BFturnsanim->var[0] = &spine->left;
	BFturnsanim->var[1] = &spine->right;
	BFturnsanim->next = BFtrackinganim;

		// tracking
	BFtrackinganim->airfoils[POSEANIM_JOINT_LARM] = NxVec3( 0.0f,0.0f,-1.5f);
	BFtrackinganim->airfoils[POSEANIM_JOINT_RARM] = NxVec3( 0.0f,0.0f,-1.5f);
	BFtrackinganim->airfoils[POSEANIM_JOINT_LLEG] = NxVec3( 0.0f,0.0f,-0.1f);
	BFtrackinganim->airfoils[POSEANIM_JOINT_RLEG] = NxVec3( 0.0f,0.0f,-0.1f);
	BFtrackinganim->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f, 0.0f, 1.0f);
	BFtrackinganim->jointangles[POSEANIM_JOINT_TORSO] = 45.0f;
	tmp = NxVec3(0.0f, 0.5f, -0.7f);
	tmp.normalize();
	BFtrackinganim->jointangles[POSEANIM_JOINT_LARM] = 55.0f;
	BFtrackinganim->jointaxis[POSEANIM_JOINT_LARM] = tmp;
	BFtrackinganim->jointangles[POSEANIM_JOINT_RARM] = 55.0f;
	BFtrackinganim->jointaxis[POSEANIM_JOINT_RARM] = tmp;
		// params
	BFtrackinganim->var_c = 1;
	BFtrackinganim->var_op = POSEANIM_OP_SUM;
	BFtrackinganim->var[0] = &spine->modifier;
	BFtrackinganim->animTrack = ANIM_TRACK_TRACKING;
	BFtrackinganim->animTrackWeight = 1.0f;
	if (useWingsuit) {	
		//BFtrackinganim->dL = -0.58f;	// wingsuit
	} else if (mTracking > 1.0) {
		//BFtrackinganim->dL = -0.58f;	// tracksuit
	} else {
		//BFtrackinganim->dL = -0.4f;	// altitude suit
	}
	BFtrackinganim->next = BFfwdanim;

		// forward/backward
			// legs
	BFfwdanim->airfoils[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,0.0f,-0.2f);
	BFfwdanim->airfoils[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,0.0f,-0.2f);
	BFfwdanim->jointaxis[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,0.0f,1.0f);
	BFfwdanim->jointaxis[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,0.0f,1.0f);
	BFfwdanim->jointangles[POSEANIM_JOINT_LLEG] = 30.0f;
	BFfwdanim->jointangles[POSEANIM_JOINT_RLEG] = 30.0f;
			// knees
	BFfwdanim->jointaxis[POSEANIM_JOINT_LKNEE] = NxVec3(0.0f,0.0f,1.0f);
	BFfwdanim->jointaxis[POSEANIM_JOINT_RKNEE] = NxVec3(0.0f,0.0f,1.0f);
	BFfwdanim->jointangles[POSEANIM_JOINT_LKNEE] = 5.0f;
	BFfwdanim->jointangles[POSEANIM_JOINT_RKNEE] = 5.0f;
			// arms
	BFfwdanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,0.0f,-0.2f);
	BFfwdanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(0.0f,0.0f,-0.2f);
	BFfwdanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,0.0f);
	BFfwdanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,1.0f,0.0f);
	BFfwdanim->jointangles[POSEANIM_JOINT_LARM] = 30.0f;
	BFfwdanim->jointangles[POSEANIM_JOINT_RARM] = 30.0f;

	BFfwdanim->var_c = 2;
	BFfwdanim->var_op = POSEANIM_OP_SUB;
	BFfwdanim->var[0] = &spine->up;
	BFfwdanim->var[1] = &spine->down;
	BFfwdanim->next = BFleveling;
	BFfwdanim->force = NxVec3(0.0f, 0.0f, 100.0f);

		// levelling
			// arms
	BFleveling->airfoils[POSEANIM_JOINT_LARM] = NxVec3(-0.1f,-0.4f, 0.1f);
	BFleveling->airfoils[POSEANIM_JOINT_RARM] = NxVec3( 0.1f,-0.4f, 0.1f);
	BFleveling->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(1.0f,0.0f,0.0f);
	BFleveling->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(1.0f,0.0f,0.0f);
	BFleveling->jointangles[POSEANIM_JOINT_LARM] = 20.0f;
	BFleveling->jointangles[POSEANIM_JOINT_RARM] = 20.0f;
	BFleveling->airfoils_area[POSEANIM_JOINT_LARM] = 0.2f * 0.15f;
	BFleveling->airfoils_area[POSEANIM_JOINT_RARM] = 0.2f * 0.15f;
			// legs
	BFleveling->airfoils[POSEANIM_JOINT_LLEG] = NxVec3(-0.1f,-0.4f,-0.1f);
	BFleveling->airfoils[POSEANIM_JOINT_RLEG] = NxVec3( 0.1f,-0.4f,-0.1f);
	BFleveling->airfoils_area[POSEANIM_JOINT_LLEG] = 0.2f * 0.15f;
	BFleveling->airfoils_area[POSEANIM_JOINT_RLEG] = 0.2f * 0.15f;
	BFleveling->jointaxis[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,0.0f,1.0f);
	BFleveling->jointaxis[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,0.0f,1.0f);
	BFleveling->jointangles[POSEANIM_JOINT_LLEG] = 30.0f;
	BFleveling->jointangles[POSEANIM_JOINT_RLEG] = 30.0f;
			// torso
	BFleveling->airfoils[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.2f, 0.0f);
	BFleveling->airfoils_area[POSEANIM_JOINT_TORSO] = 0.2f * 0.4f;
			// params
	BFleveling->var_c = 1;
	BFleveling->var_op = POSEANIM_OP_SUM;
	BFleveling->var[0] = &this->_leveling;
	BFleveling->next = BFrollleftanim;

	// roll left
			// arms
	BFrollleftanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,0.0f);
	BFrollleftanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,1.0f,0.0f);
	BFrollleftanim->jointangles[POSEANIM_JOINT_LARM] = 50.0f;
	BFrollleftanim->jointangles[POSEANIM_JOINT_RARM] = -40.0f;
	BFrollleftanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(2.3f, 0.0f, 0.0f);
	BFrollleftanim->airfoils[POSEANIM_JOINT_RARM] = NxVec3(2.0f, 0.0f, 0.0f);
			// legs
	BFrollleftanim->jointaxis[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,1.0f,0.0f);
	BFrollleftanim->jointaxis[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,1.0f,0.0f);
	BFrollleftanim->jointangles[POSEANIM_JOINT_LLEG] = -22.0f;
	BFrollleftanim->jointangles[POSEANIM_JOINT_RLEG] = 22.0f;
			// torso
	BFrollleftanim->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	BFrollleftanim->jointangles[POSEANIM_JOINT_TORSO] = 8.0f;
			// forces
	BFrollleftanim->torque = NxVec3(0.0f, 0.0f, -900.0f);
	BFrollleftanim->targetPose = &BackflyPose;

			// params
	BFrollleftanim->var_c = 2;
	BFrollleftanim->var_op = POSEANIM_OP_MIN;
	BFrollleftanim->var[0] = &spine->leftWarp;
	BFrollleftanim->var[1] = &spine->leftRearRiser;
	BFrollleftanim->next = BFrollrightanim;
		// roll right
			// arms
	BFrollrightanim->jointaxis[POSEANIM_JOINT_LARM] = NxVec3(0.0f,1.0f,0.0f);
	BFrollrightanim->jointaxis[POSEANIM_JOINT_RARM] = NxVec3(0.0f,1.0f,0.0f);
	BFrollrightanim->jointangles[POSEANIM_JOINT_LARM] = 50.0f;
	BFrollrightanim->jointangles[POSEANIM_JOINT_RARM] = -40.0f;
	BFrollrightanim->airfoils[POSEANIM_JOINT_LARM] = NxVec3(2.0f, 0.0f, 0.0f);
	BFrollrightanim->airfoils[POSEANIM_JOINT_RARM] = NxVec3(2.3f, 0.0f, 0.0f);
			// legs
	BFrollrightanim->jointaxis[POSEANIM_JOINT_LLEG] = NxVec3(0.0f,1.0f,0.0f);
	BFrollrightanim->jointaxis[POSEANIM_JOINT_RLEG] = NxVec3(0.0f,1.0f,0.0f);
	BFrollrightanim->jointangles[POSEANIM_JOINT_LLEG] = -22.0f;
	BFrollrightanim->jointangles[POSEANIM_JOINT_RLEG] = 22.0f;
			// torso
	BFrollrightanim->jointaxis[POSEANIM_JOINT_TORSO] = NxVec3(0.0f,0.0f,1.0f);
	BFrollrightanim->jointangles[POSEANIM_JOINT_TORSO] = 8.0f;
			// forces
	BFrollrightanim->torque = NxVec3(0.0f, 0.0f, 900.0f);

			// params
	BFrollrightanim->var_c = 2;
	BFrollrightanim->var_op = POSEANIM_OP_MIN;
	BFrollrightanim->var[0] = &spine->rightWarp;
	BFrollrightanim->var[1] = &spine->rightRearRiser;
	AProllrightanim->next = NULL;
	
	BackflyPose.anims = *BFturnsanim;


	// ****** FLIP POSES ******
	// *** FRONT FLIP POSE ***
	float flip_pose_area = 0.7f;
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
	FrontFlipPose.airfoils[POSEANIM_JOINT_LARM] = NxVec3( 0.0f,  0.5f, 0.5f);
	FrontFlipPose.airfoils_area[POSEANIM_JOINT_RARM] = flip_pose_area * 0.15f;
	FrontFlipPose.airfoils[POSEANIM_JOINT_RARM] = NxVec3( 0.0f,  0.5f,-0.5f);
		// legs
	FrontFlipPose.airfoils_area[POSEANIM_JOINT_LLEG] = flip_pose_area * 0.15f;
	FrontFlipPose.airfoils[POSEANIM_JOINT_LLEG] = NxVec3( 0.0f,-0.5f, 0.5f);
	FrontFlipPose.airfoils_area[POSEANIM_JOINT_RLEG] = flip_pose_area * 0.15f;
	FrontFlipPose.airfoils[POSEANIM_JOINT_RLEG] = NxVec3( 0.0f,-0.5f,-0.5f);

	// reset poses
	ArchPose.resetPose(0);
	HeaddownPose.resetPose(0);
	SitflyPose.resetPose(0);
	BackflyPose.resetPose(0);

	FrontFlipPose.resetPose(0);
}

#endif