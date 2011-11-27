#ifndef NX_PHYSICS_NXPULLEYJOINT
#define NX_PHYSICS_NXPULLEYJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxJoint.h"

class NxPulleyJointDesc;

/**
 A pulley joint.
*/
class NxPulleyJoint: public NxJoint
	{
	public:
	/**
	use this for changing a significant number of joint parameters at once.
	Use the set() methods for changing only a single property at once.
	*/
	virtual void loadFromDesc(const NxPulleyJointDesc&) = 0;

	/**
	writes all of the object's attributes to the desc struct  
	*/
	virtual void saveToDesc(NxPulleyJointDesc&) = 0;

	/**
	sets motor parameters for the joint. For a positive velTarget, the motor pulls the 
	first body towards its pulley, for a negative velTarget, the motor pulls the
	second body towards its pulley.
	
	velTarget - the relative velocity the motor is trying to achieve. The motor will only be able
				to reach this velocity if the maxForce is sufficiently large. If the joint is 
				moving faster than this velocity, the motor will actually try to brake. If you set this
				to infinity then the motor will keep speeding up, unless there is some sort of resistance
				on the attached bodies. 
	maxForce -  the maximum force the motor can exert. Zero disables the motor.
				Default is 0, should be >= 0. Setting this to a very large value if velTarget is also 
				very large may not be a good idea.
	freeSpin -  if this flag is set, and if the joint is moving faster than velTarget, then neither
				braking nor additional acceleration will result.
				default: false.
	This automatically enables the motor.
	*/
	virtual void setMotor(const NxMotorDesc &) = 0;
	/**
	reads back the motor parameters. Returns true if it is enabled.
	*/
	virtual bool getMotor(NxMotorDesc &) = 0;

	/**
	sets the flags.	This is a combination of the ::NxPulleyJointFlag bits.
	*/
	virtual void setFlags(NxU32 flags) = 0;

	/**
	returns the current flag settings.
	*/
	virtual NxU32 getFlags() = 0;

	};
#endif
