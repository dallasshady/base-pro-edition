#ifndef NX_COLLISION_NXCONVEXSHAPEDESC
#define NX_COLLISION_NXCONVEXSHAPEDESC
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "NxShapeDesc.h"
#include "NxConvexShape.h"

class NxConvexMesh;

/**
Descriptor class for NxConvexShape.
*/
class NxConvexShapeDesc : public NxShapeDesc
	{
	public:
	NxConvexMesh*	meshData;	//!< References the triangle mesh that we want to instance.
	NxU32			meshFlags;	//!< Combination of ::NxMeshShapeFlag
#ifdef NX_SUPPORT_CONVEX_SCALE
	NxReal			scale;
#endif
	/**
	constructor sets to default.
	*/
	NX_INLINE					NxConvexShapeDesc();	
	/**
	(re)sets the structure to the default.	
	*/
	NX_INLINE virtual	void	setToDefault();
	/**
	returns true if the current settings are valid
	*/
	NX_INLINE virtual	bool	isValid() const;
	};

NX_INLINE NxConvexShapeDesc::NxConvexShapeDesc() : NxShapeDesc(NX_SHAPE_CONVEX)	//constructor sets to default
	{
	setToDefault();
	}

NX_INLINE void NxConvexShapeDesc::setToDefault()
	{
	NxShapeDesc::setToDefault();
	meshData	= NULL;
	meshFlags	= 0;
#ifdef NX_SUPPORT_CONVEX_SCALE
	scale		= 1.0f;
#endif
	}

NX_INLINE bool NxConvexShapeDesc::isValid() const
	{
	if(!meshData)	return false;
#ifdef NX_SUPPORT_CONVEX_SCALE
	if(scale<=0.0f)	return false;
#endif
	return NxShapeDesc::isValid();
	}

#endif
