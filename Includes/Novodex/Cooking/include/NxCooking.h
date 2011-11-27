#ifndef NX_COOKING_H
#define NX_COOKING_H
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxc.h"
class NxUserAllocator;
class NxUserOutputStream;
class NxTriangleMeshDesc;
class NxConvexMeshDesc;
class NxStream;
//#ifdef NX_SUPPORT_INTERNAL_FACES
	class NxVec3;
	class NxPlane;
	class NxConvexMeshDesc2;
//#endif

	enum NxPlatform
		{
		PLATFORM_PC,
		PLATFORM_XENON,
		};

	struct NXC_DLL_EXPORT NxCookingParams
		{
		NxPlatform	targetPlatform;			//!< Target platform
		float		skinWidth;				//!< Skin width for convexes
		bool		hintCollisionSpeed;		//!< Favorize speed or memory for collision structures
		};

	/**
	Sets cooking parameters
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NxSetCookingParams(const NxCookingParams& params);
	/**
	Gets cooking parameters
	*/
	NX_C_EXPORT NXC_DLL_EXPORT const NxCookingParams& NxGetCookingParams();
	/**
	Checks endianness is the same between cooking & target platforms
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NxPlatformMismatch();

	/**
	Initializes cooking. This must be called at least once, before any cooking method is called (else cooking fails).
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NxInitCooking(NxUserAllocator* allocator = NULL, NxUserOutputStream* outputStream = NULL);
	/**
	Closes cooking. This must be called at the end of your app, to release cooking-related data.
	*/
	NX_C_EXPORT NXC_DLL_EXPORT void NxCloseCooking();

	/**
	Cooks a triangle mesh. The results are written to the stream.
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NxCookTriangleMesh(const NxTriangleMeshDesc& desc, NxStream& stream);
	/**
	Cooks a convex mesh. The results are written to the stream.
	*/
	NX_C_EXPORT NXC_DLL_EXPORT bool NxCookConvexMesh(const NxConvexMeshDesc& desc, NxStream& stream);
//#ifdef NX_SUPPORT_INTERNAL_FACES
	NX_C_EXPORT NXC_DLL_EXPORT bool NxCookConvexMesh2(const NxConvexMeshDesc2& desc, NxStream& stream);
	NX_C_EXPORT NXC_DLL_EXPORT bool NxSliceConvex(const NxConvexMeshDesc& desc, const NxPlane& localPlane, NxConvexMeshDesc2& positivePart, NxConvexMeshDesc2& negativePart, NxVec3& offsetP, NxVec3& offsetN);
	NX_C_EXPORT NXC_DLL_EXPORT void NxReleaseSlicingData();
//#endif

#endif
