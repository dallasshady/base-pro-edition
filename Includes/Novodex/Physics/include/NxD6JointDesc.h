#ifndef NX_PHYSICS_NXD6JOINTDESC
#define NX_PHYSICS_NXD6JOINTDESC

#include "Nxp.h"
#include "NxJointDesc.h"
#include "NxJointLimitSoftPairDesc.h"
#include "NxBitfield.h"
#include "NxMotorDesc.h"
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/





///////////////////////////////////////////////////////////


class NxD6JointDesc : public NxJointDesc
{
public:

	NxD6JointMotion			xMotion, yMotion, zMotion; //!< Define the linear degrees of freedom
	NxD6JointMotion			swing1Motion, swing2Motion, twistMotion; //!< Define the angular degrees of freedom

	NxJointLimitSoftDesc		linearLimit; //!< If some linear DOF are limited, linearLimit defines the characteristics of these limits
	NxJointLimitSoftDesc		swing1Limit; //!< If swing1Motion is NX_D6JOINT_MOTION_LIMITED, swing1Limit defines the characteristics of the limit
	NxJointLimitSoftDesc		swing2Limit; //!< If swing2Motion is NX_D6JOINT_MOTION_LIMITED, swing2Limit defines the characteristics of the limit
	NxJointLimitSoftPairDesc	twistLimit; //!< If twistMotion is NX_D6JOINT_MOTION_LIMITED, twistLimit defines the characteristics of the limit

	/* drive */
	NxJointDriveDesc		xDrive, yDrive, zDrive; //!< Drive the three linear DOF

	NxJointDriveDesc		swingDrive, twistDrive;  //!< These drives are used if the flag NX_D6JOINT_SLERP_DRIVE is not set
	NxJointDriveDesc		slerpDrive;				 //!< This drive is used if the flag NX_D6JOINT_SLERP_DRIVE is set

	NxVec3					drivePosition; //!< If the type of xDrive (yDrive,zDrive) is NX_D6JOINT_DRIVE_POSITION, drivePosition defines the goal position
	NxQuat					driveOrientation; //!< If the type of swing1Limit (swing2Limit,twistLimit) is NX_D6JOINT_DRIVE_POSITION, driveOrientation defines the goal orientation

	NxVec3					driveLinearVelocity; //!< If the type of xDrive (yDrive,zDrive) is NX_D6JOINT_DRIVE_VELOCITY, driveLinearVelocity defines the goal linear velocity
	NxVec3					driveAngularVelocity; //!< If the type of swing1Limit (swing2Limit,twistLimit) is NX_D6JOINT_DRIVE_VELOCITY, driveAngularVelocity defines the goal angular velocity

	NxJointProjectionMode	projectionMode;	//!< If projectionMode is NX_JPM_NONE, projection is disabled. If NX_JPM_POINT_MINDIST, bodies are projected to limits leaving an linear error of projectionDistance and an angular error of projectionAngle
	NxReal					projectionDistance;	
	NxReal					projectionAngle;	

	NxReal					gearRatio;	//!< when the flag NX_D6JOINT_GEAR_ENABLED is set, the angular velocity of the second actor is driven towards the angular velocity of the first actor times gearRatio (both w.r.t. their primary axis)

	NxU32 flags;                      //!< This is a combination of the bits defined by ::NxD6JointFlag 

	/**
	constructor sets to default.
	*/

	NX_INLINE			NxD6JointDesc();

	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE	void	setToDefault();

	/**

	returns true if the current settings are valid

	*/

	NX_INLINE	bool	isValid() const;
};

NxD6JointDesc::NxD6JointDesc() : NxJointDesc(NX_JOINT_D6)
{
	setToDefault();
}

void NxD6JointDesc::setToDefault() 
{
	NxJointDesc::setToDefault();

	xMotion = NX_D6JOINT_MOTION_FREE;
	yMotion = NX_D6JOINT_MOTION_FREE;
	zMotion = NX_D6JOINT_MOTION_FREE;
	twistMotion = NX_D6JOINT_MOTION_FREE;
	swing1Motion = NX_D6JOINT_MOTION_FREE;
	swing2Motion = NX_D6JOINT_MOTION_FREE;

	drivePosition.set(0,0,0);
	driveOrientation.id();

	driveLinearVelocity.set(0,0,0);
	driveAngularVelocity.set(0,0,0);

	projectionDistance = 0.1f;
	projectionAngle = 0.0872f;	//about 5 degrees in radians.

	flags = 0;
	gearRatio = 1.0f;
}

bool NxD6JointDesc::isValid() const
{
	if (flags & NX_D6JOINT_SLERP_DRIVE) {	// only possible with all angular DOF available
		if (swing1Motion == NX_D6JOINT_MOTION_LOCKED ||
			swing1Motion == NX_D6JOINT_MOTION_LOCKED ||
			twistMotion == NX_D6JOINT_MOTION_LOCKED) return false;
	}

	// swing limits are symmetric, thus their range is 0..180 degrees
	if (swing1Motion == NX_D6JOINT_MOTION_LIMITED) {
		if (swing1Limit.value < 0.0f) return false;
		if (swing1Limit.value > NxPi) return false;
	}
	if (swing2Motion == NX_D6JOINT_MOTION_LIMITED) {
		if (swing2Limit.value < 0.0f) return false;
		if (swing2Limit.value > NxPi) return false;
	}

	// twist limits are asymmetric with -180 <= low < high <= 180 degrees
	if (twistMotion == NX_D6JOINT_MOTION_LIMITED) {
		if (twistLimit.low.value < -NxPi) return false;
		if (twistLimit.high.value > NxPi) return false;
		if (twistLimit.low.value > twistLimit.high.value) return false;
	}

	// in angular limited-free mode, only -90..90 swings are possible
	if (swing1Motion == NX_D6JOINT_MOTION_LIMITED && swing2Motion == NX_D6JOINT_MOTION_FREE) 
		if (swing1Limit.value > NxHalfPi) return false; 
	if (swing2Motion == NX_D6JOINT_MOTION_LIMITED && swing1Motion == NX_D6JOINT_MOTION_FREE) 
		if (swing2Limit.value > NxHalfPi) return false; 

	if (flags & NX_D6JOINT_GEAR_ENABLED)		// gear only with twist motion enabled
		if (twistMotion == NX_D6JOINT_MOTION_LOCKED) return false;

	return NxJointDesc::isValid();
}

#endif
