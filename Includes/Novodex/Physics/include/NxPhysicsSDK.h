#ifndef NX_PHYSICS_NX_PHYSICS_SDK
#define NX_PHYSICS_NX_PHYSICS_SDK
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxUserAllocator.h"

class NxScene;
class NxSceneDesc;
class NxUserDebugRenderer;
class NxTriangleMesh;
class NxTriangleMeshDesc;
class NxConvexMesh;
class NxConvexMeshDesc;
class NxUserOutputStream;
class NxUserAllocator;
class NxActor;
class NxJoint;
class NxStream;
class NxFoundationSDK;
class NxDebugRenderable;

enum NxCookingValue
	{
	/**
	Version numbers follow this format:

		Version = 16bit|16bit

	The high part is increased each time the format changes so much that
	pre-cooked files become incompatible with the system (and hence must
	be re-cooked)

	The low part is increased each time the format changes but the code
	can still read old files. You don't need to re-cook the data in that
	case, unless you want to make sure cooked files are optimal.
	*/
	NX_COOKING_CONVEX_VERSION_PC,
	NX_COOKING_MESH_VERSION_PC,
	NX_COOKING_CONVEX_VERSION_XENON,
	NX_COOKING_MESH_VERSION_XENON,
	};

/**
Reads an internal value.
*/
NX_C_EXPORT NXP_DLL_EXPORT NxU32 NxGetValue(NxCookingValue);



/**
	Abstract singleton factory class used for instancing objects in the Physics SDK. You can get an instance of this
	class by calling NxCreatePhysicsSDK().
*/

class NxPhysicsSDK
	{
	protected:
			NxPhysicsSDK()	{}
	virtual	~NxPhysicsSDK()	{}

	public:

	/**
	Destroys the instance it is called on.

	Use this release method to destroy an instance of this class. Be sure
	to not keep a reference to this object after calling release.
	*/
	virtual	void release() = 0;

	/**
	Function that lets you set global simulation parameters.
	Returns false if the value passed is out of range for usage specified by the enum.
	*/
	virtual bool setParameter(NxParameter paramEnum, NxReal paramValue) = 0;

	/**
	Function that lets you query global simulation parameters.
	*/
	virtual NxReal getParameter(NxParameter paramEnum) const = 0;

	/**
	Creates a scene. The scene can then create its contained entities.
	*/
	virtual NxScene* createScene(const NxSceneDesc&) = 0;

	/**
	Deletes the instance passed.
	Be sure	to not keep a reference to this object after calling release.
	*/
	virtual void releaseScene(NxScene&) = 0;

	/**
	Gets number of created scenes.
	*/
	virtual NxU32 getNbScenes()			const	= 0;

	/**
	Retrieves pointer to created scenes.
	*/
	virtual NxScene* getScene(NxU32)			= 0;

#ifdef NX_COOKING
	/**
	Creates a triangle mesh object. This can then be instanced into TriangleMeshShape objects.
	*/
	virtual NxTriangleMesh* createTriangleMesh(const NxTriangleMeshDesc&) = 0;
#endif
	/**
	Creates a triangle mesh object. This can then be instanced into TriangleMeshShape objects.
	*/
	virtual NxTriangleMesh* createTriangleMesh(const NxStream&) = 0;

	/**
	Destroys the instance passed.
	Be sure	to not keep a reference to this object after calling release.
	Do not release the triangle mesh before all its instances are released first!
	*/
	virtual	void	releaseTriangleMesh(NxTriangleMesh&) = 0;

#ifdef NX_COOKING
	/**
	Creates a convex mesh object. This can then be instanced into ConvexShape objects.
	*/
	virtual NxConvexMesh* createConvexMesh(const NxConvexMeshDesc&) = 0;
#endif
	/**
	Creates a convex mesh object. This can then be instanced into ConvexShape objects.
	*/
	virtual NxConvexMesh* createConvexMesh(const NxStream&) = 0;

	/**
	Destroys the instance passed.
	Be sure	to not keep a reference to this object after calling release.
	Do not release the convex mesh before all its instances are released first!
	*/
	virtual	void	releaseConvexMesh(NxConvexMesh&) = 0;

	/**
	It is possible to assign each shape to a collision groups using NxShape::setGroup().
	With this method one can set whether collisions should be detected between shapes 
	belonging to a given pair of groups. Initially all pairs are enabled.

	Collision detection between two shapes a and b occurs if: 
	getGroupCollisionFlag(a->getGroup(), b->getGroup()) && isEnabledPair(a,b) is true.

	Fluids can be assigned to collision groups as well.

	NxCollisionGroup is an integer between 0 and 31.
	*/
	virtual void setGroupCollisionFlag(NxCollisionGroup group1, NxCollisionGroup group2, bool enable) = 0;

	/**
	This reads the value set with the above call.
	NxCollisionGroup is an integer between 0 and 31.
	*/
	virtual bool getGroupCollisionFlag(NxCollisionGroup group1, NxCollisionGroup group2) const = 0;

	/**
	It is possible to assign each actor to a group using NxActor::setGroup(). This is a different
	set of groups from the shape groups despite the similar name. Here more up to 0xffff different groups are permitted,
	With this method one can set contact reporting flags between actors belonging to a pair of groups.

	The following flags are permitted:

	NX_NOTIFY_ON_START_TOUCH
	NX_NOTIFY_ON_END_TOUCH	
	NX_NOTIFY_ON_TOUCH		
	NX_NOTIFY_ON_IMPACT		
	NX_NOTIFY_ON_ROLL		
	NX_NOTIFY_ON_SLIDE	

	See ::NxContactPairFlag.

	Note that finer grain control of pairwise flags is possible using the functions
	NxScene::setShapePairFlags() and NxScene::setActorPairFlags().
	*/
	virtual void setActorGroupPairFlags(NxActorGroup group1, NxActorGroup group2, NxU32 flags) = 0;

	/**
	This reads the value set with the above call.
	*/
	virtual NxU32 getActorGroupPairFlags(NxActorGroup group1, NxActorGroup group2) const = 0;


#if NX_USE_FLUID_API

	/**
	Not available in the current release.
	Fluids can be assigned to fluid groups by using NxFluid::setFluidGroup(). This method sets
	fluid actor group pairs to configure collision and collision reports between 
	fluids and actors using the flags:

	NX_IGNORE_PAIR
	NX_NOTIFY_ON_COLLISION

	See ::NxFluidContactFlag.
	*/
	virtual void setFluidGroupPairFlags(NxActorGroup actorGroup, NxFluidGroup fluidGroup, NxU32 flags) = 0;

	/**
	Not available in the current release.
	This reads the value set with the above call.
	*/
	virtual NxU32 getFluidGroupPairFlags(NxActorGroup actorGroup, NxFluidGroup fluidGroup) const = 0;

#endif

	/**
	Renders visualization graphics to the passed rendering device.
	*/
	virtual void visualize(const NxUserDebugRenderer&) = 0;

	/**
	retrieves the FoundationSDK instance.
	*/
	virtual NxFoundationSDK& getFoundationSDK() const = 0;

	/**
	retrieves the debug renderable
	*/
	virtual NxDebugRenderable*	getDebugRenderable() const = 0;

#ifdef NX_SUPPORT_NEW_FILTERING
	/**
	Setups filtering operations. See comments for ::NxGroupsMask
	*/
	virtual	void	setFilterOps(NxFilterOp op0, NxFilterOp op1, NxFilterOp op2)	= 0;

	/**
	Setups filtering's boolean value. See comments for ::NxGroupsMask
	*/
	virtual	void	setFilterBool(bool flag)										= 0;

	/**
	Setups filtering's K0 value. See comments for ::NxGroupsMask
	*/
	virtual	void	setFilterConstant0(const NxGroupsMask& mask)					= 0;

	/**
	Setups filtering's K1 value. See comments for ::NxGroupsMask
	*/
	virtual	void	setFilterConstant1(const NxGroupsMask& mask)					= 0;
#endif
	};

/**
Creates an instance of this class. May not be a class member to avoid name mangling.
Pass the constant NX_PHYSICS_SDK_VERSION as the argument.
Because the class is a singleton class, multiple calls return the same object.
*/
NX_C_EXPORT NXP_DLL_EXPORT NxPhysicsSDK* NX_CALL_CONV NxCreatePhysicsSDK(NxU32 sdkVersion, NxUserAllocator* allocator = NULL, NxUserOutputStream* outputStream = NULL);

/**
Retrieves the Physics SDK allocator. Used by NxAllocateable's inlines and other macros below.
*/
NX_C_EXPORT NXP_DLL_EXPORT NxUserAllocator* NxGetPhysicsSDKAllocator();

/**
Retrieves the Foundation SDK after it has been created.
*/
NX_C_EXPORT NXP_DLL_EXPORT NxFoundationSDK* NxGetFoundationSDK();

/**
Retrieves the Physics SDK after it has been created.
*/
NX_C_EXPORT NXP_DLL_EXPORT NxPhysicsSDK* NxGetPhysicsSDK();

#endif
