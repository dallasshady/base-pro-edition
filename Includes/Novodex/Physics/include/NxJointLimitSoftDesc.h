#ifndef NX_PHYSICS_NXJOINTLIMITSOFTDESC
#define NX_PHYSICS_NXJOINTLIMITSOFTDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

/**
Describes a joint limit.
*/
class NxJointLimitSoftDesc
	{
	public:
	NxReal value;		//!< the angle / position beyond which the limit is active. Which side the limit restricts depends on whether this is a high or low limit.
	NxReal restitution;	//!< limit bounce
	NxReal spring;		//!< if greater than zero, the limit is soft, i.e. a spring pulls the joint back to the limit
	NxReal damping;		//!< if spring is greater than zero, this is the damping of the spring

	NX_INLINE NxJointLimitSoftDesc();
	NX_INLINE void setToDefault();
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxJointLimitSoftDesc::NxJointLimitSoftDesc()
	{
	setToDefault();
	}

NX_INLINE void NxJointLimitSoftDesc::setToDefault()
	{
	value = 0;
	restitution = 0;
	spring = 0;
	damping = 0;
	}

NX_INLINE bool NxJointLimitSoftDesc::isValid() const
	{
	return (restitution >= 0 && restitution <= 1);
	}

#endif
