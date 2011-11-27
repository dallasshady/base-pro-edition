#ifndef NX_COLLISION_NXTRIANGLEMESHSHAPE
#define NX_COLLISION_NXTRIANGLEMESHSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"
#include "NxShape.h"

class NxTriangle;
class NxTriangleMeshShapeDesc;
class NxTriangleMesh;


/**
This class is a shape instance of a triangle mesh object of type NxTriangleMesh.

Each shape is owned by an actor that it is attached to.

An instance can be created by calling the createShape() method of the NxActor object
that should own it, with a NxTriangleMeshShapeDesc object as the parameter, or by adding the 
shape descriptor into the NxActorDesc class before creating the actor.

The shape is deleted by calling NxActor::releaseShape() on the owning actor.
*/

class NxTriangleMeshShape: public NxShape
	{
	public:
	/*
	Saves the state of the shape object to a descriptor.
	*/
	virtual	void				saveToDesc(NxTriangleMeshShapeDesc&)	const = 0;

	/**
	Retrieves the triangle mesh data associated with this instance.
	*/
	virtual	NxTriangleMesh&		getTriangleMesh() = 0;

	/**
	Retrieves triangle data from a triangle ID.
	*/
	virtual	NxU32				getTriangle(NxTriangle& worldTri, NxTriangle* edgeTri, NxU32* flags, NxTriangleID triangleIndex) const	= 0;

	/**
	Finds triangles touching input bounds.
	*/
	virtual	bool				overlapAABBTriangles(const NxBounds3 bounds, NxU32 flags, NxU32& nb, const NxU32*& indices)	const	= 0;
	};
#endif
