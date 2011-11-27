#ifndef NX_PHYSICS_NXBODYDESC
#define NX_PHYSICS_NXBODYDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

/**
Descriptor for the optional rigid body dynamic state of NxActor.
*/
class NxBodyDesc
	{
	public:
	NxMat34		massLocalPose;		  //!< position and orientation of the center of mass
	NxVec3		massSpaceInertia;	  //!< Diagonal mass space inertia tensor in body space (all zeros to let the system compute it)
	NxReal		mass;				  //!< Mass of body
	NxVec3		linearVelocity;		  //!< Initial linear velocity
	NxVec3		angularVelocity;	  //!< Initial angular velocity
	NxReal		wakeUpCounter;		  //!< Initial wake-up counter
	NxReal		linearDamping;		  //!< Linear damping
	NxReal		angularDamping;		  //!< Angular damping
	NxReal		maxAngularVelocity;	  //!< Max. allowed angular velocity (negative values to use default)
	NxU32		flags;				  //!< Combination of body flags

	NxReal		sleepLinearVelocity;  //!< maximum linear velocity at which body can go to sleep. If negative, the global default will be used.
	NxReal		sleepAngularVelocity; //!< maximum angular velocity at which body can go to sleep. If negative, the global default will be used.

	NxU32		solverIterationCount; //!< solver accuracy setting when dealing with this body.
	/**
	constructor sets to default, mass == 0 (an immediate call to isValid() 
	will return false). 
	*/
	NX_INLINE NxBodyDesc();	
	/**
	(re)sets the structure to the default, mass == 0 (an immediate call to isValid() 
	will return false). 	
	*/
	NX_INLINE void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE bool isValid() const;
	};

NX_INLINE NxBodyDesc::NxBodyDesc()	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxBodyDesc::setToDefault()
	{
	massLocalPose			.id();
	massSpaceInertia		.zero();
	linearVelocity			.zero();	//setNotUsed();	//when doing a loadFromDesc, the user expects to set the complete state, so this is not OK.
	angularVelocity			.zero();	//setNotUsed();
	wakeUpCounter			= 20.0f*0.02f;
	mass					= 0.0f;
	linearDamping			= 0.0f;
	angularDamping			= 0.05f;
	maxAngularVelocity		= -1.0f;
	flags					= NX_BF_VISUALIZATION;
	sleepLinearVelocity		= -1.0;
	sleepAngularVelocity	= -1.0;

	solverIterationCount    = 4;
	}

NX_INLINE bool NxBodyDesc::isValid() const
	{
	if(mass<0.0f)		//no negative masses plz.
		return false;
	if(!massLocalPose.isFinite())
		return false;
	return true;
	}

#endif
