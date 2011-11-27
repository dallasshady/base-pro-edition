#ifndef NX_PHYSICS_NXSLIDINGJOINT
#define NX_PHYSICS_NXSLIDINGJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

#include "NxJoint.h"
class NxCylindricalJointDesc;

/**
 A sliding joint permits relative translational movement between two bodies along
 an axis, and also relative rotation along the axis.
*/
class NxCylindricalJoint : public NxJoint
	{
	public:
	/**
	use this for changing a significant number of joint parameters at once.
	Use the set() methods for changing only a single property at once.
	*/
	virtual void loadFromDesc(const NxCylindricalJointDesc&) = 0;

	/**
	writes all of the object's attributes to the desc struct  
	*/
	virtual void saveToDesc(NxCylindricalJointDesc&) = 0;
	};
#endif
