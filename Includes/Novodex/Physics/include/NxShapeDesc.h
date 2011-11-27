#ifndef NX_COLLISION_NXSHAPEDESC
#define NX_COLLISION_NXSHAPEDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "NxShape.h"

/**
Descriptor for NxShape class. Used for saving and loading the shape state.
See the derived classes for the different shape types.
*/
class NxShapeDesc
	{
	protected:
	const NxShapeType		type;			//!< The type of the shape (see NxShape.h). This gets set by the derived class' ctor, the user should not have to change it.
	public:
	NxMat34					localPose;		//!< The pose of the shape in the coordinate frame of the owning actor.
	NxU32					shapeFlags;		//!< A combination of ::NxShapeFlag values.
	NxCollisionGroup		group;			//!< See the documentation for NxShape::setGroup().
	NxMaterialIndex			materialIndex;	//!< The material index of the shape. See NxPhysicsSDK::addMaterial().
	NxReal					skinWidth;		/*!< The shape has been radially inflated by this much over the graphical representation.
Two shapes will interpenetrate by the sum of their skin widths; this means that their graphical representations should just touch.
The default skin width is the negative global parameter ::NX_MIN_SEPARATION_FOR_PENALTY.  This is used if skinWidth is -1.  A sum skin width
of zero for two bodies is not permitted because it is unstable.  If you simulation jitters because resting bodies occasionally lose contact, increase the sizes of your
collision volumes and the skin width.
Units: distance. 
*/
	void*					userData;		//!< Will be copied to NxShape::userData.
	const char*				name;			//!< Possible debug name. The string is not copied by the SDK, only the pointer is stored.
#ifdef NX_SUPPORT_NEW_FILTERING
	NxGroupsMask			groupsMask;		//!< Groups bitmask for collision filtering
#endif
	NX_INLINE virtual		~NxShapeDesc();
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE virtual	bool isValid() const;

	NX_INLINE			NxShapeType	getType()	const	{ return type; }

	protected:
	/**
	constructor sets to default.
	*/
	NX_INLINE				NxShapeDesc(NxShapeType);
	};

NX_INLINE NxShapeDesc::NxShapeDesc(NxShapeType t) : type(t)
	{
	setToDefault();
	}

NX_INLINE NxShapeDesc::~NxShapeDesc()
	{
	}

NX_INLINE void NxShapeDesc::setToDefault()
	{
	localPose.id();
	shapeFlags			= NX_SF_VISUALIZATION;
	group				= 0;
	materialIndex		= 0;
	skinWidth			= -1.0f;
	userData			= NULL;
	name				= NULL;
#ifdef NX_SUPPORT_NEW_FILTERING
	groupsMask.bits0	= 0;
	groupsMask.bits1	= 0;
	groupsMask.bits2	= 0;
	groupsMask.bits3	= 0;
#endif
	}

NX_INLINE bool NxShapeDesc::isValid() const
	{
	if(!localPose.isFinite())
		return false;
	if(group>=32)
		return false;	// We only support 32 different groups
	if(shapeFlags&0xffff0000)
		return false;	// Only 16-bit flags supported here
	if(type >= NX_SHAPE_COUNT)
		return false;
	if(materialIndex==0xffff)
		return false;	// 0xffff is reserved for internal usage
	if (skinWidth != -1 && skinWidth < 0)
		return false;
	return true;
	}


#endif
