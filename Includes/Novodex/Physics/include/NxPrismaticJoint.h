#ifndef NX_PHYSICS_NXPRISMATICJOINT
#define NX_PHYSICS_NXPRISMATICJOINT
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxJoint.h"

class NxPrismaticJointDesc;

/**
 A prismatic joint permits relative translational movement between two bodies along
 an axis, but no relative rotational movement.
*/
class NxPrismaticJoint: public NxJoint
	{
	public:
	/**
	use this for changing a significant number of joint parameters at once.
	Use the set() methods for changing only a single property at once.
	*/
	virtual void loadFromDesc(const NxPrismaticJointDesc&) = 0;

	/**
	writes all of the object's attributes to the desc struct  
	*/
	virtual void saveToDesc(NxPrismaticJointDesc&) = 0;
	};
#endif
