#ifndef NX_COLLISION_NXCONVEXSHAPE
#define NX_COLLISION_NXCONVEXSHAPE
/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/
#include "Nxp.h"
#include "NxShape.h"

class NxConvexShapeDesc;

class NxConvexShape: public NxShape
	{
	public:
	virtual	void	saveToDesc(NxConvexShapeDesc&)	const = 0;

	/**
	Retrieves the convex mesh data associated with this instance.
	*/
	virtual	NxConvexMesh&		getConvexMesh() = 0;
	};
#endif
