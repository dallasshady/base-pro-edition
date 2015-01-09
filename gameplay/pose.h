#ifndef POSE_INCLUDED
#define POSE_INCLUDED

#include "imath.h"

#define POSEANIM_JOINTS	6		// number of joints (for physics)
#define POSEANIM_JOINTS_ALL	10	// number of joints (for physics + only visual (knees, etc))
#define POSEANIM_VARS	16		// number of PoseAnim vars

enum {
	POSEANIM_OP_MAX,	// variable of highest value is used
	POSEANIM_OP_MIN,	// variable of lowest value is used
	POSEANIM_OP_AVG,	// average value of all variables if used
	POSEANIM_OP_SUM,	// sum of all variables is used as a value
	POSEANIM_OP_SUB		// variables are subtracted from one another: value = var[0] - var[1] - .. - var[POSEANIM_VARS-1]
};
enum {
	POSEANIM_JOINT_HEAD = 0,
	POSEANIM_JOINT_TORSO = 1,
	POSEANIM_JOINT_LARM = 2,
	POSEANIM_JOINT_RARM = 3,
	POSEANIM_JOINT_LLEG = 4,
	POSEANIM_JOINT_RLEG = 5,
	POSEANIM_JOINT_LKNEE = 6,	// not for physics, only visual	(POSEANIM_JOINTS will not reach it)
	POSEANIM_JOINT_RKNEE = 7,	// not for physics, only visual (POSEANIM_JOINTS will not reach it)
	POSEANIM_JOINT_LSHOU = 8,	// not for physics, only visual	(POSEANIM_JOINTS will not reach it)
	POSEANIM_JOINT_RSHOU = 9	// not for physics, only visual (POSEANIM_JOINTS will not reach it)
};

class Pose;

class PoseAnim {
public:
	int var_op;					// variable operator POSEANIM_OP_XXX
	int var_c;					// counters

	float *only_if_zero;		// if not NULL, animation works only if this value is 0
	float *only_if_not_zero;	// if not NULL, animation works only if this value is NOT 0

	float *var[POSEANIM_VARS];	// variable(s) to control the effect of animation. Range: [0;1]
	float value;				// cached value of vars 
	float prev_value;			// cached value of vars from previous frame
	float dA;					// change in surface area m^2 (unused)
	float dL;					// change in lift coef
	float dReaction;			// change in reaction

	PxVec3 airfoils[POSEANIM_JOINTS];			// CHANGE in airfoil (drag element) positions [m]
	float airfoils_area[POSEANIM_JOINTS];		// CHANGE in airfoil (drag element) area [m^2]
	PxVec3 jointaxis[POSEANIM_JOINTS_ALL];		// CHANGE in mesh joint rotation axes
	float  jointangles[POSEANIM_JOINTS_ALL];	// CHANGE in mesh joint rotation angles

	PxVec3 torque;				// added torque
	PxVec3 force;				// added force

	PoseAnim *next;

	// morph to this pose if value == 1
	Pose *targetPose; 

	int animTrack;
	float animTrackWeight;

	PoseAnim();
	~PoseAnim();
};

class Pose {
public:
	float A;							// surface area [m^2] (unused)
	float L;							// lift coefficient at rest
	float L_now;						// lift coefficient after animations

	PxVec3 airfoils[POSEANIM_JOINTS];			// airfoil (drag element) positions at rest [m]
	float airfoils_area[POSEANIM_JOINTS];		// airfoil (drag element) area at rest [m^2]
	PxVec3 jointaxis[POSEANIM_JOINTS_ALL];		// mesh joint rotation axes
	float  jointangles[POSEANIM_JOINTS_ALL];	// mesh joint rotation angles

	PxVec3 airfoils_now[POSEANIM_JOINTS];		// airfoil (drag element) positions based on current animation [m]
	float airfoils_area_now[POSEANIM_JOINTS];	// airfoil (drag element) area positions based on current animation [m^2]

	PxVec3 torque;						// added torque
	PxVec3 force;						// added force
	float reaction;
	float reaction_now;

	static const char *jointnames[];	// 

	int animTrack;
	float animTrackWeight;

	PoseAnim anims;

	// pose with no animation alterations
	Pose *restPose;

	// if not null, this instance will animate itself towards this set pose on every controlPose();
	Pose *targetPose;
	float target_rate;
	float target_prog;
	void morphToPose(Pose *targetPose, float rate, bool force = false);

	bool animBodyPart(const char *frame_title);

	// methods
	// takes all anims into account and according to their variables, interpolates 
	// pose parameters. Also, if targetPose != NULL morphs towards targetPose
	void controlPose(double dt);

	// set current pose variables to rest pose
	void resetPose(double dt);

	// interface methods
	
	// constructors
	Pose::Pose();
};

#endif