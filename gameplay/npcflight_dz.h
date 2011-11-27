
#ifndef NPC_FLIGHTDZ_PROGRAM_INCLUDED
#define NPC_FLIGHTDZ_PROGRAM_INCLUDED

#include "npc.h"

/**
 * npc program : freefall and track to desired position / character
 */

class NPCFlight_DZ : public NPCProgram
{
protected:
    float _steeringAltitude;
    float _landingAltitude;
    float _steeringRange;
	Vector3f _groundTarget;
	bool _groundTargetEnabled;
protected:
    // abstraction layer
    virtual void onUpdate(float dt);
    virtual bool isEndOfProgram(void);
    virtual void onEndOfSubProgram(NPCProgram* subProgram);
public:
    // class implementation
    NPCFlight_DZ(NPC* npc, float steeringAltitude, float landingAltitude);
    virtual ~NPCFlight_DZ();
};

#endif