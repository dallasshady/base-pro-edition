#ifndef NX_COLLISION_NXSHAPE
#define NX_COLLISION_NXSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nxp.h"
#include "NxPhysicsSDK.h"

class NxBounds3;
class NxBoxShape;
class NxBoxShapeOffCenter;
class NxPlaneShape;
class NxSphereShape;
class NxCapsuleShape;
class NxCollisionSpace;
class NxConvexShape;
class NxTriangleMeshShape;
class NxActor;
class NxRay;
class NxSphere;
struct NxRaycastHit;

/**
Abstract base class for the various collision shapes.
An instance of a subclass can be created by calling the createShape() method of the NxActor class,
or by adding the shape descriptors into the NxActorDesc class before creating the actor.

Note: in order to avoid a naming conflict, downcast operators are isTYPE(), while up casts are getTYPE().
*/
class NxShape
	{
	protected:
	NX_INLINE							NxShape() : userData(NULL), appData(NULL)
													{}
	virtual								~NxShape()	{}

	public:

	/**
	Retrieves the actor which this shape is associated with.
	*/
	virtual		NxActor&				getActor() const = 0;

	/**
	Sets which collision group this shape is part of. Default group is 0. Maximum possible group is 31.
	Collision groups are sets of shapes which may or may not be set
	to collision detect with each other, by NxPhysicsSDK::setGroupCollisionFlag()
	NxCollisionGroup is an integer between 0 and 31.
	*/
	virtual		void					setGroup(NxCollisionGroup) = 0;

	/**
	Retrieves the value set with the above call.
	NxCollisionGroup is an integer between 0 and 31.
	*/
	virtual		NxCollisionGroup		getGroup() const = 0;

	/**
	Returns a world space AABB enclosing this shape.
	*/
	virtual		void					getWorldBounds(NxBounds3& dest) const = 0;	

	/**
	The shape may be turned into a trigger by setting one or more of the
	above TriggerFlag-s to true. A trigger shape will not collide
	with other shapes. Instead, if a shape enters the trigger's volume, 
	a trigger event will be sent to the user via the NxUserTriggerReport::onTrigger method.
	You can set a NxUserTriggerReport object with NxScene::setUserTriggerReport().

	Since version 2.1.1 this is also used to setup generic (non-trigger) flags.
	*/
	virtual		void					setFlag(NxShapeFlag flag, bool value) = 0;

	/**
	Retrieves a flag.
	*/
	virtual		NX_BOOL					getFlag(NxShapeFlag flag) const = 0;

	/**
	The setLocal*() methods set the pose of the shape in actor space, i.e. relative
	to the actor they are owned by.
	This transformation is identity by default.
	*/
	virtual		void					setLocalPose(const NxMat34&)			= 0;

	/**
	The setLocal*() methods set the pose of the shape in actor space, i.e. relative
	to the actor they are owned by.
	This transformation is identity by default.
	*/
	virtual		void					setLocalPosition(const NxVec3&)			= 0;

	/**
	The setLocal*() methods set the pose of the shape in actor space, i.e. relative
	to the actor they are owned by.
	This transformation is identity by default.
	*/
	virtual		void					setLocalOrientation(const NxMat33&)		= 0;

	/**
	The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative
	to the actor they are owned by.
	This transformation is identity by default.
	*/
	virtual		NxMat34					getLocalPose()					const	= 0;

	/**
	The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative
	to the actor they are owned by.
	This transformation is identity by default.
	*/
	virtual		NxVec3					getLocalPosition()				const	= 0;

	/**
	The getLocal*() methods retrieve the pose of the shape in actor space, i.e. relative
	to the actor they are owned by.
	This transformation is identity by default.
	*/
	virtual		NxMat33					getLocalOrientation()			const	= 0;

	/**
	The setGlobal() calls are convenience methods which transform the passed parameter
	into the current local space of the actor and then call setLocalPose().
	*/
	virtual		void					setGlobalPose(const NxMat34&)			= 0;

	/**
	The setGlobal() calls are convenience methods which transform the passed parameter
	into the current local space of the actor and then call setLocalPose().
	*/
	virtual		void					setGlobalPosition(const NxVec3&)		= 0;

	/**
	The setGlobal() calls are convenience methods which transform the passed parameter
	into the current local space of the actor and then call setLocalPose().
	*/
	virtual		void					setGlobalOrientation(const NxMat33&)	= 0;

	/**
	The getGlobal*() methods retrieve the shape's current world space pose. This is 
	the local pose multiplied by the actor's current global pose.
	*/
	virtual		NxMat34					getGlobalPose()					const	= 0;

	/**
	The getGlobal*() methods retrieve the shape's current world space pose. This is 
	the local pose multiplied by the actor's current global pose.
	*/
	virtual		NxVec3					getGlobalPosition()				const	= 0;

	/**
	The getGlobal*() methods retrieve the shape's current world space pose. This is 
	the local pose multiplied by the actor's current global pose.
	*/
	virtual		NxMat33					getGlobalOrientation()			const	= 0;

	/**
	Assigns a material index to the shape. The material index can be retrieved by calling
	NxMaterial::getMaterialIndex().
	If the material index is invalid, it will still be recorded, but 
	the default material (at index 0) will effectively be used for simulation.
	*/
	virtual		void					setMaterial(NxMaterialIndex)	= 0;

	/**
	Retrieves the material index currently assigned to the shape.
	*/
	virtual		NxMaterialIndex			getMaterial() const				= 0;

	/**
	Sets the skin width.  See NxShapeDesc::skinWidth.
	*/
	virtual		void					setSkinWidth(NxReal skinWidth)	= 0;
	/**
	Retrieves the skin width.  See NxShapeDesc::skinWidth.
	*/
	virtual		NxReal					getSkinWidth() const	= 0;

	/**
	returns the type of shape.
	*/
	virtual		NxShapeType				getType() const = 0;

	/**
	Type casting operator. The result may be cast to the desired subclass type.
	*/
	NX_INLINE	void*					is(NxShapeType type) { return (type == getType()) ? (void*)this : NULL;		}

	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE	NxPlaneShape*			isPlane()			{ return (NxPlaneShape*)		is(NX_SHAPE_PLANE);		}
	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE	NxSphereShape*			isSphere()			{ return (NxSphereShape*)		is(NX_SHAPE_SPHERE);	}
	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE	NxBoxShape*				isBox()				{ return (NxBoxShape*)			is(NX_SHAPE_BOX);		}
	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE	NxCapsuleShape*			isCapsule()			{ return (NxCapsuleShape*)		is(NX_SHAPE_CAPSULE);	}
	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE	NxConvexShape*			isConvexMesh()		{ return (NxConvexShape*)		is(NX_SHAPE_CONVEX);	}
	/**
	attempts to perform an downcast to the type returned. Returns 0 if this object is not of the appropriate type.
	*/
	NX_INLINE	NxTriangleMeshShape*	isTriangleMesh()	{ return (NxTriangleMeshShape*)	is(NX_SHAPE_MESH);		}

	/**
	Sets a name string for the object that can be retrieved with getName().  This is for debugging and is not used
	by the SDK.  The string is not copied by the SDK, only the pointer is stored.
	*/
	virtual		void					setName(const char*)		= 0;

	/**
	retrieves the name string set with setName().
	*/
	virtual		const char*				getName()			const	= 0;

	/*
	casts a world-space ray against the shape.
	maxDist is the maximum allowed distance for the ray. You can use this for segment queries.
	hintFlags is a combination of ::NxRaycastBit flags.
	firstHit is a hint saying you're only interested in a boolean answer.

	Note: Make certain that the direction vector of NxRay is normalized.
	*/
	virtual		bool					raycast(const NxRay& worldRay, NxReal maxDist, NxU32 hintFlags, NxRaycastHit& hit, bool firstHit)	const = 0;

	/*
	Checks whether the shape overlaps a world-space sphere or not.
	*/
	virtual		bool					checkOverlapSphere(const NxSphere& worldSphere)														const = 0;

	/*
	Checks whether the shape overlaps a world-space AABB or not.
	*/
	virtual		bool					checkOverlapAABB(const NxBounds3& worldBounds)														const = 0;

#ifdef NX_SUPPORT_NEW_FILTERING
	/**
	Sets 128-bit mask used for collision filtering. See comments for ::NxGroupsMask
	*/
	virtual		void					setGroupsMask(const NxGroupsMask& mask)	= 0;

	/**
	Sets 128-bit mask used for collision filtering. See comments for ::NxGroupsMask
	*/
	virtual		const NxGroupsMask&		getGroupsMask()	const = 0;
#endif

				void*					userData;	//!< user can assign this to whatever, usually to create a 1:1 relationship with a user object.
				void*					appData;
	};
#endif
