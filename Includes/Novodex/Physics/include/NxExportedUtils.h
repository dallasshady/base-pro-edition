#ifndef NX_PHYSICS_NXLEGACYEXPORTS
#define NX_PHYSICS_NXLEGACYEXPORTS
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

struct NxIntegrals;
/*
These are functions that used to be exported for the user from the foundation DLL, which doesn't exist anymore.
They are kept here and exported from the physics SDK DLL for backwards compatibility.
*/

NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxBoxContainsPoint(const NxBox& box, const NxVec3& p);
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxCreateBox(NxBox& box, const NxBounds3& aabb, const NxMat34& mat);
NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxComputeBoxPlanes(const NxBox& box, NxPlane* planes);
NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxComputeBoxPoints(const NxBox& box, NxVec3* pts);
NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxComputeBoxVertexNormals(const NxBox& box, NxVec3* pts);
NX_C_EXPORT NXP_DLL_EXPORT const NxU32* NX_CALL_CONV NxGetBoxEdges();
NX_C_EXPORT NXP_DLL_EXPORT const NxI32* NX_CALL_CONV NxGetBoxEdgesAxes();
NX_C_EXPORT NXP_DLL_EXPORT const NxU32* NX_CALL_CONV NxGetBoxTriangles();
NX_C_EXPORT NXP_DLL_EXPORT const NxVec3* NX_CALL_CONV NxGetBoxLocalEdgeNormals();
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxComputeBoxWorldEdgeNormal(const NxBox& box, NxU32 edge_index, NxVec3& world_normal);
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxComputeCapsuleAroundBox(const NxBox& box, NxCapsule& capsule);
NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxIsBoxAInsideBoxB(const NxBox& a, const NxBox& b);

NX_C_EXPORT NXP_DLL_EXPORT const NxU32* NX_CALL_CONV NxGetBoxQuads();
NX_C_EXPORT NXP_DLL_EXPORT const NxU32* NX_CALL_CONV NxBoxVertexToQuad(NxU32 vertexIndex);

NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxComputeBoxAroundCapsule(const NxCapsule& capsule, NxBox& box);

NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxSetFPUPrecision24();
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxSetFPUPrecision53();
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxSetFPUPrecision64();

NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxSetFPURoundingChop();
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxSetFPURoundingUp();
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxSetFPURoundingDown();
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxSetFPURoundingNear();

NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxSetFPUExceptions(bool);

NX_C_EXPORT NXP_DLL_EXPORT int NX_CALL_CONV NxIntChop(const NxF32& f);
NX_C_EXPORT NXP_DLL_EXPORT int NX_CALL_CONV NxIntFloor(const NxF32& f);
NX_C_EXPORT NXP_DLL_EXPORT int NX_CALL_CONV NxIntCeil(const NxF32& f);

NX_C_EXPORT NXP_DLL_EXPORT NxF32 NX_CALL_CONV NxComputeDistanceSquared(const NxRay& ray, const NxVec3& point, NxF32* t);
NX_C_EXPORT NXP_DLL_EXPORT NxF32 NX_CALL_CONV NxComputeSquareDistance(const NxSegment& seg, const NxVec3& point, NxF32* t);

NX_C_EXPORT NXP_DLL_EXPORT NxBSphereMethod NX_CALL_CONV NxComputeSphere(NxSphere& sphere, unsigned nb_verts, const NxVec3* verts);
NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxFastComputeSphere(NxSphere& sphere, unsigned nb_verts, const NxVec3* verts);
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxMergeSpheres(NxSphere& merged, const NxSphere& sphere0, const NxSphere& sphere1);

NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxNormalToTangents(const NxVec3 & n, NxVec3 & t1, NxVec3 & t2);

/**
Rotates a 3x3 symmetric inertia tensor I into a space R where it can be represented with the diagonal matrix D.
I = R * D * R'
Returns false on failure. 
*/
NX_C_EXPORT NXP_DLL_EXPORT bool NX_CALL_CONV NxDiagonalizeInertiaTensor(const NxMat33 & denseInertia, NxVec3 & diagonalInertia, NxMat33 & rotation);
/**
computes rotation matrix M so that:

M * x = b

x and b are unit vectors.
*/
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxFindRotationMatrix(const NxVec3 & x, const NxVec3 & b, NxMat33 & M);

/**
computes bounds of an array of vertices
*/
NX_C_EXPORT NXP_DLL_EXPORT void NX_CALL_CONV NxComputeBounds(NxVec3& min, NxVec3& max, NxU32 nbVerts, const NxVec3* verts);

/**
computes bounds of an array of vertices
*/
NX_INLINE void NxComputeBounds(NxBounds3& bounds, NxU32 nbVerts, const NxVec3* verts)
	{
	NxVec3 min, max;
	NxComputeBounds(min, max, nbVerts, verts);
	bounds.set(min, max);
	}

/**
computes CRC of input buffer
*/
NX_C_EXPORT NXP_DLL_EXPORT NxU32 NX_CALL_CONV NxCrc32(const void* buffer, NxU32 nbBytes);

#endif