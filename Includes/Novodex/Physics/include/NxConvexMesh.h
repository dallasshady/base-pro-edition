#ifndef NX_COLLISION_NXCONVEXMESH
#define NX_COLLISION_NXCONVEXMESH
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nx.h"
#include "NxTriangleMesh.h"

class NxConvexMeshShape;
class NxSimpleTriangleMesh;
class NxConvexMeshDesc;
class NxStream;
#ifdef NX_SUPPORT_INTERNAL_FACES
class NxConvexMeshDesc2;
#endif

/**

A convex mesh, internally represented as a list of convex polygons. The number
of polygons is limited to 256.

To avoid duplicating data when you have several instances of a particular 
mesh positioned differently, you do not use this class to represent a 
convex object directly. Instead, you create an instance of this mesh via
the NxConvexMeshShape class.

To create an instance of this class call NxPhysicsSDK::createConvexMesh(),
and NxPhysicsSDK::releaseConvexMesh() to delete it. This is only possible
once you have released all of its NxConvexMeshShape instances.

*/

class NxConvexMesh
	{
	public:

#ifdef NX_COOKING
	/**
	Sets the triangle mesh data. Note that the mesh data will be copied, then cleaned.

	The internal data structures will be created when this call is made. If you change the triangle mesh
	data, you need to make this call again to recompute the data structures.

	See the documentation of NxTriangleMeshDesc for specifics about the parameters.
	*/
	virtual	bool				loadFromDesc(const NxConvexMeshDesc&) = 0;
	#ifdef NX_SUPPORT_INTERNAL_FACES
	virtual	bool				loadFromDesc(const NxConvexMeshDesc2&) = 0;
	#endif
#endif
	virtual	bool				saveToDesc(NxConvexMeshDesc&)	const	= 0;

	/**
	Gets the number of internal submeshes for this mesh.
	*/
	virtual NxU32				getSubmeshCount()							const	= 0;

	/**
	For a given submesh, retrieves the number of elements of a given internal array.
	*/
	virtual NxU32				getCount(NxSubmeshIndex, NxInternalArray)	const	= 0;

	/**
	For a given submesh, retrieves the format of a given internal array.
	*/
	virtual NxInternalFormat	getFormat(NxSubmeshIndex, NxInternalArray)	const	= 0;

	/**
	For a given submesh, retrieves the base pointer of a given internal array.
	*/
	virtual const void*			getBase(NxSubmeshIndex, NxInternalArray)	const	= 0;

	/**
	For a given submesh, retrieves the stride value of a given internal array.
	The stride value is always a number of bytes. You have to skip this number of bytes
	to go from one element to the other in an array, starting from the base.
	*/
	virtual NxU32				getStride(NxSubmeshIndex, NxInternalArray)	const	= 0;

	virtual	bool				load(const NxStream& stream)		= 0;
#ifdef NX_COOKING
	virtual	bool				save(NxStream& stream)		const	= 0;
#endif
	};
#endif
