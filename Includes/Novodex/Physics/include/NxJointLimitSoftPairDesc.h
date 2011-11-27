#ifndef NX_PHYSICS_NXJOINTLIMITSOFTPAIRDESC
#define NX_PHYSICS_NXJOINTLIMITSOFTPAIRDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "NxJointLimitSoftDesc.h"

/**
Describes a pair of joint limits
*/
class NxJointLimitSoftPairDesc
	{
	public:
	NxJointLimitSoftDesc low;		//!< The low limit (smaller value)
	NxJointLimitSoftDesc high;		//!< the high limit (larger value)

	NX_INLINE NxJointLimitSoftPairDesc();
	NX_INLINE void setToDefault();
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxJointLimitSoftPairDesc::NxJointLimitSoftPairDesc()
	{
	setToDefault();
	}

NX_INLINE void NxJointLimitSoftPairDesc::setToDefault()
	{
	//nothing
	}

NX_INLINE bool NxJointLimitSoftPairDesc::isValid() const
	{
	return (low.isValid() && high.isValid() && low.value <= high.value);
	}

#endif
