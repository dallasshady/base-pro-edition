#ifndef NX_SCENE_STATS
#define NX_SCENE_STATS

/*----------------------------------------------------------------------------*\
|
|						Public Interface to NovodeX Technology
|
|							     www.novodex.com
|
\*----------------------------------------------------------------------------*/

#include "Nxp.h"

class NxSceneStats
	{
	public:
	//collisions:
	NxU32   numContacts;
	NxU32   maxContacts;
	NxU32	numPairs;
	NxU32	maxPairs;
	//sleep:
	NxU32   numDynamicActorsInAwakeGroups;
	NxU32   maxDynamicActorsInAwakeGroups;
	//solver:
	NxU32	numAxisConstraints;
	NxU32	maxAxisConstraints;
	NxU32	numSolverBodies;
	NxU32	maxSolverBodies;
	//scene:
	NxU32   numActors;
	NxU32	maxActors;
	NxU32   numDynamicActors;
	NxU32	maxDynamicActors;
	NxU32   numStaticShapes;
	NxU32	maxStaticShapes;
	NxU32	numDynamicShapes;
	NxU32	maxDynamicShapes;
	NxU32	numJoints;
	NxU32	maxJoints;


	NxSceneStats()
		{
		reset();
		}

	void reset()
		{
		numContacts = 0;
		maxContacts = 0;
		numPairs = 0;
		maxPairs = 0;
		numDynamicActorsInAwakeGroups = 0;
		maxDynamicActorsInAwakeGroups = 0;
		numAxisConstraints = 0;
		maxAxisConstraints = 0;
		numSolverBodies = 0;
		maxSolverBodies = 0;
		numActors = 0;
		maxActors = 0;
		numDynamicActors = 0;
		maxDynamicActors = 0;
		numStaticShapes = 0;
		maxStaticShapes = 0;
		numDynamicShapes = 0;
		maxDynamicShapes = 0;
		numJoints = 0;
		maxJoints = 0;
		}
	};

#endif
