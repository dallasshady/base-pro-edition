#ifndef NX_PHYSICS_NXP
#define NX_PHYSICS_NXP
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

//this header should be included first thing in all headers in physics/include
#ifndef NXP_DLL_EXPORT
	#if defined NX_PHYSICS_DLL

		#define NXP_DLL_EXPORT __declspec(dllexport)
		//new: default foundation to static lib:
		#define NXF_DLL_EXPORT //__declspec(dllimport)

	#elif defined NX_PHYSICS_STATICLIB

		#define NXP_DLL_EXPORT
		#define NXF_DLL_EXPORT

	#elif defined NX_USE_SDK_DLLS

		#define NXP_DLL_EXPORT __declspec(dllimport)
		//new: default foundation to static lib:
		#define NXF_DLL_EXPORT //__declspec(dllimport)

	#elif defined NX_USE_SDK_STATICLIBS

		#define NXP_DLL_EXPORT
		#define NXF_DLL_EXPORT

	#else

		//#error Please define either NX_USE_SDK_DLLS or NX_USE_SDK_STATICLIBS in your project settings depending on the kind of libraries you use!
		//new: default foundation to static lib:
		#define NXP_DLL_EXPORT __declspec(dllimport)
		#define NXF_DLL_EXPORT //__declspec(dllimport)
  			
	#endif
#endif

#include "Nxf.h"
#include "NxVec3.h"
#include "NxQuat.h"
#include "NxMat33.h"
#include "NxMat34.h"

#include "NxVersionNumber.h"
/**
Pass the constant NX_PHYSICS_SDK_VERSION to the NxCreatePhysicsSDK function. 
This is to ensure that the application is using the same header version as the
library was built with.
*/

#define NX_PHYSICS_SDK_VERSION ((   NX_SDK_VERSION_MAJOR   <<24)+(NX_SDK_VERSION_MINOR    <<16)+(NX_SDK_VERSION_BUGFIX    <<8) + 0)
//2.1.1 Automatic scheme via VersionNumber.h on July 9, 2004.
//2.1.0 (new scheme: major.minor.build.configCode) on May 12, 2004.  ConfigCode can be i.e. 32 vs. 64 bit.
//2.3 on Friday April 2, 2004, starting ag. changes.
//2.2 on Friday Feb 13, 2004
//2.1 on Jan 20, 2004

/*
Note: users can't change these defines as it would need the libraries to be recompiled!

AM: PLEASE MAKE SURE TO HAVE AN 'NX_' PREFIX ON ALL NEW DEFINES YOU ADD HERE!!!!!

*/

#define NX_USE_ADAPTIVE_FORCE
#define NX_SUPPORT_NEW_FILTERING

//#define NX_SUPPORT_MESH_SCALE		// Experimental mesh scale support

#ifdef NX_DISABLE_FLUIDS
	#define NX_USE_FLUID_API  0
	#define NX_USE_SDK_FLUIDS 0
#else
	// If we are exposing the Fluid API.
	#define NX_USE_FLUID_API 1

	// If we are compiling in support for Fluid.
	// We check to see if this is already defined
	// by the Project or Makefile.
	#ifndef NX_USE_SDK_FLUIDS
		#define NX_USE_SDK_FLUIDS 1
	#endif
#endif /* NX_DISABLE_FLUIDS */

// a bunch of simple defines used in several places:

typedef NxU16 NxActorGroup;
typedef NxU16 NxCollisionGroup;		// Must be < 32
typedef NxU16 NxMaterialIndex;
typedef NxU32 NxTriangleID;

#if NX_USE_FLUID_API
	typedef NxU16 NxFluidGroup;
#endif

////// moved enums here that are used in Core so we don't have to include headers such as NxBoxShape in core!!


enum NxShapeType
	{
	NX_SHAPE_PLANE,		//!< A physical plane
	NX_SHAPE_SPHERE,	//!< A physical sphere
	NX_SHAPE_BOX,		//!< A physical box (OBB)
	NX_SHAPE_CAPSULE,	//!< A physical capsule (LSS)
	NX_SHAPE_CONVEX,	//!< A physical convex mesh
	NX_SHAPE_MESH,		//!< A physical mesh
	NX_SHAPE_COMPOUND,	//!< internal use only!
	NX_SHAPE_COUNT,

	NX_SHAPE_FORCE_DWORD = 0x7fffffff
	};

enum NxMeshShapeFlag
	{
	/** 
	Select between "normal" or "smooth" sphere-mesh contact generation routines.
	The 'normal' algorithm assumes that the mesh is composed from flat triangles. 
	When a ball rolls along the mesh surface, it will experience small, sudden changes 
	in velocity as it rolls from one triangle to the next. The smooth algorithm, on the other hand, 
	assumes that the triangles are just an approximation of a surface that is smooth.  
	It uses the Gouraud algorithm to smooth the triangles' vertex normals (which in this 
	case are particularly important). This way the rolling sphere's velocity will change 
	smoothly over time, instead of suddenly. We recommend this algorithm for simulating car wheels on a terrain.
	*/
	NX_MESH_SMOOTH_SPHERE_COLLISIONS	= (1<<0),		
	};

enum NxCapsuleShapeFlag
	{
	/*
	If this flag is set, the capsule shape represents a moving sphere, moving along the ray defined by the capsule's positive Y axis.  
	Currently this behavior is only implemented for points (zero radius spheres).
	*/
	NX_SWEPT_SHAPE	= (1<<0)
	};

/**
Parameter to addForce*() calls, determines the exact operation that is carried out.
*/
enum NxForceMode
	{
	NX_FORCE,                   //!< parameter has unit of mass * distance/ time^2, i.e. a force
	NX_IMPULSE,                 //!< parameter has unit of mass * distance /time
	NX_VELOCITY_CHANGE,			//!< parameter has unit of distance / time, i.e. the effect is mass independent: a velocity change.
	NX_SMOOTH_IMPULSE,          //!< same as NX_IMPULSE but the effect is applied over all substeps.  Use this for motion controllers that repeatedly apply an impulse.
	NX_SMOOTH_VELOCITY_CHANGE,	//!< same as NX_VELOCITY_CHANGE but the effect is applied over all substeps.  Use this for motion controllers that repeatedly apply an impulse.
	NX_ACCELERATION				//!< parameter has unit of distance/ time^2, i.e. an acceleration.  It gets treated just like a force except the mass is not divided out before integration.
	};

#define NX_NUM_SLEEP_FRAMES (20.0f*0.02f)		//This is a 'legacy' setup that works out to 20 when using the standard time step.
//#define NX_NUM_SLEEP_FRAMES (999999999.0f)


enum NxBodyFlag
	{
	NX_BF_DISABLE_GRAVITY	= (1<<0),	//!< set if gravity should not be applied on this body
	
	/**	
	Enable/disable freezing for this body/actor. A frozen actor becomes temporarily static.
	Note: this is an experimental feature which doesn't always work on actors which have joints 
	connected to them.
	*/
	NX_BF_FROZEN_POS_X		= (1<<1),
	NX_BF_FROZEN_POS_Y		= (1<<2),
	NX_BF_FROZEN_POS_Z		= (1<<3),
	NX_BF_FROZEN_ROT_X		= (1<<4),
	NX_BF_FROZEN_ROT_Y		= (1<<5),
	NX_BF_FROZEN_ROT_Z		= (1<<6),
	NX_BF_FROZEN_POS		= NX_BF_FROZEN_POS_X|NX_BF_FROZEN_POS_Y|NX_BF_FROZEN_POS_Z,
	NX_BF_FROZEN_ROT		= NX_BF_FROZEN_ROT_X|NX_BF_FROZEN_ROT_Y|NX_BF_FROZEN_ROT_Z,
	NX_BF_FROZEN			= NX_BF_FROZEN_POS|NX_BF_FROZEN_ROT,


	/**
	Enables kinematic mode for the actor.  Kinematic actors are special dynamic actors that are not 
	influenced by forces (such as gravity), and have no momentum.  They appear to have infinite
	mass and can be moved around the world using the moveGlobal*() methods.  They will push 
	regular dynamic actors out of the way.
	
	Currently they will not collide with static or other kinematic objects.  This will change in a later version.
	Note that if a dynamic actor is squished between a kinematic and a static or two kinematics, then it will
	have no choice but to get pressed into one of them.  Later we will make it possible to have the kinematic
	motion be blocked in this case.

	Kinematic actors are great for moving platforms or characters, where direct motion control is desired.

	You can not connect Reduced joints to kinematic actors.  Lagrange joints work ok if the platform
	is moving with a relatively low, uniform velocity.
	*/
	NX_BF_KINEMATIC			= (1<<7),
	NX_BF_VISUALIZATION		= (1<<8),		//!< Enable debug renderer for this body
	};

enum NxShapeFlag
	{
	NX_TRIGGER_ON_ENTER				= (1<<0),	//!< Trigger callback will be called when a shape enters the trigger volume.
	NX_TRIGGER_ON_LEAVE				= (1<<1),	//!< Trigger callback will be called after a shape leaves the trigger volume.
	NX_TRIGGER_ON_STAY				= (1<<2),	//!< Trigger callback will be called while a shape is intersecting the trigger volume.
	NX_TRIGGER_ENABLE				= NX_TRIGGER_ON_ENTER|NX_TRIGGER_ON_LEAVE|NX_TRIGGER_ON_STAY,

	NX_SF_VISUALIZATION				= (1<<3),	//!< Enable debug renderer for this shape
	NX_SF_DISABLE_COLLISION			= (1<<4),	//!< Disable collision detection for this shape (counterpart of NX_AF_DISABLE_COLLISION)
												//!< IMPORTANT: this is only used for compound objects! Use NX_AF_DISABLE_COLLISION otherwise.
	NX_SF_FEATURE_INDICES			= (1<<5),	//!< Enable feature indices in contact stream.
	NX_SF_DISABLE_RAYCASTING		= (1<<6),	//!< Disable raycasting for this shape
	NX_SF_POINT_CONTACT_FORCE		= (1<<7),	//!< Enable contact force reporting per contact point in contact stream (otherwise we only report force per actor pair)

#if NX_USE_FLUID_API
	NX_SF_FLUID_DRAIN				= (1<<8),	//!< Not available in the current release. Sets the shape to be a fluid drain.
	NX_SF_FLUID_DRAIN_INVERT		= (1<<9),	//!< Not available in the current release. Invert the domain of the fluid drain.
	NX_SF_FLUID_DISABLE_COLLISION	= (1<<10),	//!< Not available in the current release. Disable collision with fluids.
	NX_SF_FLUID_ACTOR_REACTION		= (1<<11),	//!< Not available in the current release. Enables the reaction of the shapes actor on fluid collision.
#endif
	NX_SF_DISABLE_RESPONSE			= (1<<12),	//!< Disable collision response for this shape (counterpart of NX_AF_DISABLE_RESPONSE)
												//!< IMPORTANT: this is only used for compound objects! Use NX_AF_DISABLE_RESPONSE otherwise.
	};

typedef NxShapeFlag	NxTriggerFlag;		//!< For compatibility with previous SDK versions before 2.1.1

enum NxHeightFieldAxis
	{
	NX_X				= 0,
	NX_Y				= 1,
	NX_Z				= 2,
	NX_NOT_HEIGHTFIELD	= 0xff
	};

typedef NxVec3 NxPoint;
struct NxTriangle32
	{
	NX_INLINE	NxTriangle32()							{}
	NX_INLINE	NxTriangle32(NxU32 a, NxU32 b, NxU32 c) { v[0] = a; v[1] = b; v[2] = c; }

	NxU32 v[3];	//vertex indices
	};

typedef NxU32 NxSubmeshIndex;
enum NxInternalFormat
	{
	NX_FORMAT_NODATA,		//!< No data available
	NX_FORMAT_FLOAT,		//!< Data is in floating-point format
	NX_FORMAT_BYTE,			//!< Data is in byte format (8 bit)
	NX_FORMAT_SHORT,		//!< Data is in short format (16 bit)
	NX_FORMAT_INT,			//!< Data is in int format (32 bit)
	};

enum NxInternalArray
	{
	NX_ARRAY_TRIANGLES,		//!< Array of triangles (index buffer). One triangle = 3 vertex references in returned format.
	NX_ARRAY_VERTICES,		//!< Array of vertices (vertex buffer). One vertex = 3 coordinates in returned format.
	NX_ARRAY_NORMALS,		//!< Array of vertex normals. One normal = 3 coordinates in returned format.
	NX_ARRAY_HULL_VERTICES,	//!< Array of hull vertices. One vertex = 3 coordinates in returned format.
	NX_ARRAY_HULL_POLYGONS,	//!< Array of hull polygons
	};

enum NxJointType
	{
	NX_JOINT_PRISMATIC,			//!< Permits a single translational degree of freedom.
	NX_JOINT_REVOLUTE,			//!< Also known as a hinge joint, permits one rotational degree of freedom.
	NX_JOINT_CYLINDRICAL,		//!< Formerly known as a sliding joint, permits one translational and one rotational degree of freedom.
	NX_JOINT_SPHERICAL,			//!< Also known as a ball or ball and socket joint.
	NX_JOINT_POINT_ON_LINE,		//!< A point on one actor is constrained to stay on a line on another.
	NX_JOINT_POINT_IN_PLANE,	//!< A point on one actor is constrained to stay on a plane on another.
	NX_JOINT_DISTANCE,			//!< A point on one actor maintains a certain distance range to another point on another actor.
	NX_JOINT_PULLEY,			//!< A pulley joint.
	NX_JOINT_FIXED,				//!< A "fixed" connection.
	NX_JOINT_D6,				//!< A 6 degree of freedom joint

	NX_JOINT_COUNT,				//!< Just to track the number of available enum values. Not a joint type.
	NX_JOINT_FORCE_DWORD = 0x7fffffff
	};

enum NxJointState
	{
	NX_JS_UNBOUND,
	NX_JS_SIMULATING,
	NX_JS_BROKEN
	};

enum NxJointFlag
    {
    NX_JF_COLLISION_ENABLED	= (1<<0),	//!< Raised if collision detection should be enabled between the jointed parts.
    NX_JF_VISUALIZATION		= (1<<1),	//!< Enable debug renderer for this joint
    };

enum NxJointProjectionMode
	{
	NX_JPM_NONE  = 0,				//!< don't project this joint
	NX_JPM_POINT_MINDIST = 1,		//!< this is the only projection method right now 
	//there are expected to be more modes later
	};

enum NxRevoluteJointFlag
	{
	NX_RJF_LIMIT_ENABLED = 1 << 0,			//!< true if the limit is enabled
	NX_RJF_MOTOR_ENABLED = 1 << 1,			//!< true if the motor is enabled
	NX_RJF_SPRING_ENABLED = 1 << 2,			//!< true if the spring is enabled.  The spring will only take effect if the motor is disabled.
	};

enum NxPulleyJointFlag
	{
	NX_PJF_IS_RIGID = 1 << 0,	//!< true if the joint also maintains a minimum distance, not just a maximum.
	NX_PJF_MOTOR_ENABLED = 1 << 1			//!< true if the motor is enabled
	};


enum NxDistanceJointFlag
	{
	NX_DJF_MAX_DISTANCE_ENABLED = 1 << 0,	//!< true if the joint enforces the maximum separate distance.
	NX_DJF_MIN_DISTANCE_ENABLED = 1 << 1,	//!< true if the joint enforces the minimum separate distance.
	NX_DJF_SPRING_ENABLED		= 1 << 2,	//!< true if the spring is enabled
	};

enum NxD6JointMotion
	{
	NX_D6JOINT_MOTION_LOCKED,
	NX_D6JOINT_MOTION_LIMITED,
	NX_D6JOINT_MOTION_FREE
	};


enum NxD6JointLockFlags
	{
	NX_D6JOINT_LOCK_X			= 1<<0,
	NX_D6JOINT_LOCK_Y			= 1<<1,
	NX_D6JOINT_LOCK_Z			= 1<<2,
	NX_D6JOINT_LOCK_LINEAR		= 7,

	NX_D6JOINT_LOCK_TWIST		= 1<<3,
	NX_D6JOINT_LOCK_SWING1		= 1<<4,
	NX_D6JOINT_LOCK_SWING2		= 1<<5,
	NX_D6JOINT_LOCK_ANGULAR		= 7<<3,

	};


enum NxD6JointLimitFlags
	{
	NX_D6JOINT_LIMIT_TWIST	= 1<<0,
	NX_D6JOINT_LIMIT_SWING	= 1<<1,
	NX_D6JOINT_LIMIT_LINEAR	= 1<<2
	};


enum NxD6JointDriveType
	{
	NX_D6JOINT_DRIVE_POSITION	= 1<<0,
	NX_D6JOINT_DRIVE_VELOCITY	= 1<<1
	};

enum NxD6JointFlag
	{
	NX_D6JOINT_SLERP_DRIVE = 1<<0,
	NX_D6JOINT_GEAR_ENABLED = 1<<1
	};

enum NxActorFlag
	{
	NX_AF_DISABLE_COLLISION			= (1<<0),	//!< Enable/disable collision detection
	NX_AF_DISABLE_RESPONSE			= (1<<1),	//!< Enable/disable collision response (reports contacts but don't use them)
	
#if NX_USE_FLUID_API
	NX_AF_FLUID_DISABLE_COLLISION	= (1<<2),	//!< Not available in the current release. Disable collision with fluids
	NX_AF_FLUID_ACTOR_REACTION		= (1<<3),	//!< Not available in the current release. Enable the reaction on fluid collision
#endif
	};

enum NxSphericalJointFlag
	{
	NX_SJF_TWIST_LIMIT_ENABLED = 1 << 0,//!< true if the twist limit is enabled
	NX_SJF_SWING_LIMIT_ENABLED = 1 << 1,//!< true if the swing limit is enabled
	NX_SJF_TWIST_SPRING_ENABLED= 1 << 2,//!< true if the twist spring is enabled
	NX_SJF_SWING_SPRING_ENABLED= 1 << 3,//!< true if the swing spring is enabled
	NX_SJF_JOINT_SPRING_ENABLED= 1 << 4,//!< true if the joint spring is enabled
	};

enum NxQueryFlags
	{
	NX_QUERY_WORLD_SPACE	= (1<<0),	// world-space parameter, else object space
	NX_QUERY_FIRST_CONTACT	= (1<<1),	// returns first contact only, else returns all contacts
	};

enum NxTriangleFlags
	{
	// Must be the 3 first ones to be indexed by (flags & (1<<edge_index))
	NXTF_ACTIVE_EDGE01	= (1<<0),
	NXTF_ACTIVE_EDGE12	= (1<<1),
	NXTF_ACTIVE_EDGE20	= (1<<2),
	NXTF_DOUBLE_SIDED	= (1<<3),
	};

/**
Parameters enums to be used as the 1st arg to setParameter or getParameter.
*/
enum NxParameter
	{
	/** RigidBody-related parameters  */
	NX_PENALTY_FORCE,					//!< DEPRECATED! Do not use!
	NX_SKIN_WIDTH,						//!< Default value for ::NxShapeDesc::skinWidth, see for more info.  (range: [0, inf]) Default: 0.05, Unit: distance.

	NX_DEFAULT_SLEEP_LIN_VEL_SQUARED,	//!< The default linear velocity, squared, below which objects start going to sleep. (range: [0, inf]) Default: (0.15*0.15)
	NX_DEFAULT_SLEEP_ANG_VEL_SQUARED,	//!< The default angular velocity, squared, below which objects start going to sleep. (range: [0, inf]) Default: (0.14*0.14)

	NX_BOUNCE_TRESHOLD,					//!< A contact with a relative velocity below this will not bounce.	(range: [-inf, 0]) Default: -2

	NX_DYN_FRICT_SCALING,				//!< This lets the user scale the magnitude of the dynamic friction applied to all objects.	(range: [0, inf]) Default: 1
	NX_STA_FRICT_SCALING,				//!< This lets the user scale the magnitude of the static friction applied to all objects.	(range: [0, inf]) Default: 1

	NX_MAX_ANGULAR_VELOCITY,			//!< See the comment for NxBody::setMaxAngularVelocity() for details.	Default: 7

	/** Collision-related parameters:  */

	NX_CONTINUOUS_CD,					//!< Enable/disable continuous collision detection (0.0f to disable)
	
	/**
	The below settings permit the debug visualization of various simulation properties. 
	The setting is either zero, in which case the property is not drawn. Otherwise it is a scaling factor
	that determines the size of the visualization widgets.

	Only bodies and joints for which visualization is turned on using setFlag(VISUALIZE) are visualized.
	Contacts are visualized if they involve a body which is being visualized.
	Default is 0.

	Notes:
	- to see any visualization, you have to set NX_VISUALIZATION_SCALE to nonzero first.
	- the scale factor has been introduced because it's difficult (if not impossible) to come up with a
	good scale for 3D vectors. Normals are normalized and their length is always 1. But it doesn't mean
	we should render a line of length 1. Depending on your objects/scene, this might be completely invisible
	or extremely huge. That's why the scale factor is here, to let you tune the length until it's ok in
	your scene.
	- however, things like collision shapes aren't ambiguous. They are clearly defined for example by the
	triangles & polygons themselves, and there's no point in scaling that. So the visualization widgets
	are only scaled when it makes sense.
	*/
	NX_VISUALIZATION_SCALE,			//!< This overall visualization scale gets multiplied with the individual scales. Setting to zero turns ignores all visualizations. Default is 0.

	NX_VISUALIZE_WORLD_AXES,
	NX_VISUALIZE_BODY_AXES,
	NX_VISUALIZE_BODY_MASS_AXES,
	NX_VISUALIZE_BODY_LIN_VELOCITY,
	NX_VISUALIZE_BODY_ANG_VELOCITY,
	NX_VISUALIZE_BODY_LIN_MOMENTUM,
	NX_VISUALIZE_BODY_ANG_MOMENTUM,
	NX_VISUALIZE_BODY_LIN_ACCEL,
	NX_VISUALIZE_BODY_ANG_ACCEL,
	NX_VISUALIZE_BODY_LIN_FORCE,
	NX_VISUALIZE_BODY_ANG_FORCE,
	NX_VISUALIZE_BODY_REDUCED,
	NX_VISUALIZE_BODY_JOINT_GROUPS,
	NX_VISUALIZE_BODY_CONTACT_LIST,
	NX_VISUALIZE_BODY_JOINT_LIST,
	NX_VISUALIZE_BODY_DAMPING,
	NX_VISUALIZE_BODY_SLEEP,

	NX_VISUALIZE_JOINT_LOCAL_AXES,
	NX_VISUALIZE_JOINT_WORLD_AXES,
	NX_VISUALIZE_JOINT_LIMITS,
	NX_VISUALIZE_JOINT_ERROR,
	NX_VISUALIZE_JOINT_FORCE,
	NX_VISUALIZE_JOINT_REDUCED,

	NX_VISUALIZE_CONTACT_POINT,
	NX_VISUALIZE_CONTACT_NORMAL,
	NX_VISUALIZE_CONTACT_ERROR,
	NX_VISUALIZE_CONTACT_FORCE,

	NX_VISUALIZE_ACTOR_AXES,

	NX_VISUALIZE_COLLISION_AABBS,		//!< Visualize bounds (last updated AABBs in world space) Note that they might not be up to
										//!< date when you visualize them (i.e. they might be away from the object's current position)
										//!< Please use NX_VISUALIZE_COLLISION_SAP to visualize current bounds.
	NX_VISUALIZE_COLLISION_SHAPES,		//!< Shape visualization
	NX_VISUALIZE_COLLISION_AXES,		//!< Shape axis visualization
	NX_VISUALIZE_COLLISION_COMPOUNDS,	//!< Compound visualization (compound AABBs in world space)
	NX_VISUALIZE_COLLISION_VNORMALS,	//!< Mesh & convex vertex normals
	NX_VISUALIZE_COLLISION_FNORMALS,	//!< Mesh & convex face normals
	NX_VISUALIZE_COLLISION_SPHERES,		//!< Bounding spheres

	NX_VISUALIZE_COLLISION_SAP,			//!< SAP structures (current AABBs in world space)
	NX_VISUALIZE_COLLISION_STATIC,		//!< Static pruning structures (last updated AABBs in world space)
	NX_VISUALIZE_COLLISION_DYNAMIC,		//!< Dynamic pruning structures (last updated AABBs in world space)
	NX_VISUALIZE_COLLISION_FREE,		//!< "Free" pruning structures (last updated AABBs in world space)

#if NX_USE_FLUID_API
	NX_VISUALIZE_FLUID_EMITTERS,		//!< Not available in the current release. Emitter visualization.
	NX_VISUALIZE_FLUID_POSITION,		//!< Not available in the current release. Particle position visualization.
	NX_VISUALIZE_FLUID_VELOCITY,		//!< Not available in the current release. Particle velocity visualization.
	NX_VISUALIZE_FLUID_KERNEL_RADIUS,	//!< Not available in the current release. Particle kernel radius visualization.
	NX_VISUALIZE_FLUID_BOUNDS,			//!< Not available in the current release. Fluid AABB visualization.
#endif

	NX_ADAPTIVE_FORCE,
	NX_PARAMS_NUM_VALUES,				//!< This is not a parameter, it just records the current number of parameters.

	NX_MIN_SEPARATION_FOR_PENALTY,		//!< Deprecated! Use SKIN_WIDTH instead.  The minimum contact separation value in order to apply a penalty force. (range: [-inf, 0) ) I.e. This must be negative!!  Default: -0.05
										// Note: its OK for this one to be > than NX_PARAMS_NUM_VALUES because internally it uses the slot of SKIN_WIDTH.

	};

#ifdef NX_SUPPORT_NEW_FILTERING
	/**
	128-bit mask used for collision filtering.

	The collision filtering equation for 2 shapes S0 and S1 is:

	(G0 op0 K0) op2 (G1 op1 K1) == b

	with

	G0 = NxGroupsMask for shape S0. See ::setGroupsMask
	G1 = NxGroupsMask for shape S1. See ::setGroupsMask
	K0 = filtering constant 0. See ::setFilterConstant0
	K1 = filtering constant 1. See ::setFilterConstant1
	b = filtering boolean. See ::setFilterBool
	op0, op1, op2 = filtering operations. See ::setFilterOps

	If the filtering equation is true, collision detection is enabled.
	*/
	class NxGroupsMask
		{
		public:
		NX_INLINE	NxGroupsMask()	{}
		NX_INLINE	~NxGroupsMask()	{}

		NxU32		bits0, bits1, bits2, bits3;
		};

	/**
	Collision filtering operations.
	*/
	enum NxFilterOp
		{
		NX_FILTEROP_AND,
		NX_FILTEROP_OR,
		NX_FILTEROP_XOR,
		NX_FILTEROP_NAND,
		NX_FILTEROP_NOR,
		NX_FILTEROP_NXOR,
		};
#endif


#endif
