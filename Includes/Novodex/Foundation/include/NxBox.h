#ifndef NX_FOUNDATION_NXBOX
#define NX_FOUNDATION_NXBOX
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nx.h"
#include "NxVec3.h"
#include "NxMat33.h"
#include "NxMat34.h"

class NxCapsule;
class NxPlane;
class NxBox;
class NxBounds3;

class NxBox
	{
	public:
	/**
	Constructor
	*/
	NX_INLINE NxBox()
		{
		}

	/**
	Constructor
	*/
	NX_INLINE NxBox(const NxVec3& _center, const NxVec3& _extents, const NxMat33& _rot) : center(_center), extents(_extents), rot(_rot)
		{
		}

	/**
	Destructor
	*/
	NX_INLINE ~NxBox()
		{
		}

	/**
	 Setups an empty box.
	 */
	NX_INLINE void setEmpty()
		{
		center.zero();
		extents.set(NX_MIN_REAL, NX_MIN_REAL, NX_MIN_REAL);
		rot.id();
		}
#ifdef FOUNDATION_EXPORTS

	/**
	 Tests if a point is contained within the box
	 \param		p	[in] the world point to test
	 \return	true if inside the box
	 */
	NX_INLINE bool containsPoint(const NxVec3& p) const
		{
		return NxBoxContainsPoint(*this, p);
		}

	/**
	 Builds a box from AABB and a world transform.
	 \param		aabb	[in] the aabb
	 \param		mat		[in] the world transform
	 */
	NX_INLINE void create(const NxBounds3& aabb, const NxMat34& mat)
		{
		NxCreateBox(*this, aabb, mat);
		}
#endif
	/**
	 Recomputes the box after an arbitrary transform by a 4x4 matrix.
	 \param		mtx		[in] the transform matrix
	 \param		obb		[out] the transformed OBB
	 */
	NX_INLINE void rotate(const NxMat34& mtx, NxBox& obb) const
		{
		// The extents remain constant
		obb.extents = extents;
		// The center gets x-formed
//		obb.center = mCenter * mtx;
//		TransformPoint(center, obb.center, mtx);
		mtx.M.multiplyByTranspose(center, obb.center);
		obb.center += mtx.t;
		// Combine rotations
//		obb.rot = mRot * Matrix3x3(mtx);
		obb.rot.multiply(rot, mtx.M);	// ### check order
//		obb.rot.multiply(mtx.M, rot);	// ### check order
		}

	/**
	 Checks the box is valid.
	 \return	true if the box is valid
	 */
	NX_INLINE bool isValid() const
		{
		// Consistency condition for (Center, Extents) boxes: Extents >= 0.0f
		if(extents.x < 0.0f)	return false;
		if(extents.y < 0.0f)	return false;
		if(extents.z < 0.0f)	return false;
		return true;
		}
#ifdef FOUNDATION_EXPORTS

	/**
	 Computes the obb planes.
	 \param		planes	[out] 6 box planes
	 \return	true if success
	 */
	NX_INLINE bool computePlanes(NxPlane* planes) const
		{
		return NxComputeBoxPlanes(*this, planes);
		}

	/**
	 Computes the obb points.
	 \param		pts	[out] 8 box points
	 \return	true if success
	 */
	NX_INLINE bool computePoints(NxVec3* pts) const
		{
		return NxComputeBoxPoints(*this, pts);
		}

	/**
	 Computes vertex normals.
	 \param		pts	[out] 8 box points
	 \return	true if success
	 */
	NX_INLINE bool computeVertexNormals(NxVec3* pts) const
		{
		return NxComputeBoxVertexNormals(*this, pts);
		}

	/**
	 Returns edges.
	 \return	24 indices (12 edges) indexing the list returned by ComputePoints()
	 */
	NX_INLINE const NxU32* getEdges() const
		{
		return NxGetBoxEdges();
		}

	NX_INLINE const NxI32* getEdgesAxes() const
		{
		return NxGetBoxEdgesAxes();
		}

	/**
	 *	Returns triangles.
	 *	\return		36 indices (12 triangles) indexing the list returned by ComputePoints()
	 */
	NX_INLINE const NxU32* getTriangles() const
		{
		return NxGetBoxTriangles();
		}

	/**
	 Returns local edge normals.
	 \return	edge normals in local space
	 */
	NX_INLINE const NxVec3* getLocalEdgeNormals() const
		{
		return NxGetBoxLocalEdgeNormals();
		}

	/**
	 Returns world edge normal
	 \param		edge_index		[in] 0 <= edge index < 12
	 \param		world_normal	[out] edge normal in world space
	 */
	NX_INLINE void computeWorldEdgeNormal(NxU32 edge_index, NxVec3& world_normal) const
		{
		NxComputeBoxWorldEdgeNormal(*this, edge_index, world_normal);
		}

	/**
	 Computes a capsule surrounding the box.
	 \param		capsule	[out] the capsule
	 */
	NX_INLINE void computeCapsule(NxCapsule& capsule) const
		{
		NxComputeCapsuleAroundBox(*this, capsule);
		}

	/**
	 Checks the box is inside another box
	 \param		box		[in] the other box
	 \return	TRUE if we're inside the other box
	 */
	NX_INLINE bool isInside(const NxBox& box) const
		{
		return NxIsBoxAInsideBoxB(*this, box);
		}
#endif
	// Accessors

	NX_INLINE const NxVec3& GetCenter() const
		{
		return center;
		}

	NX_INLINE const NxVec3& GetExtents() const
		{
		return extents;
		}

	NX_INLINE const NxMat33& GetRot() const
		{
		return rot;
		}

/*	NX_INLINE	void GetRotatedExtents(NxMat33& extents) const
		{
		extents = mRot;
		extents.Scale(mExtents);
		}*/

	NxVec3	center;
	NxVec3	extents;
	NxMat33	rot;
	};

#endif
