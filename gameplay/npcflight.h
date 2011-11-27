
#ifndef NPC_FLIGHT_PROGRAM_INCLUDED
#define NPC_FLIGHT_PROGRAM_INCLUDED

#include "npc.h"

/**
 * npc program : freefall and track to desired position / character
 */

class NPCFlight : public NPCProgram
{
protected:
    float _steeringAltitude;
    float _landingAltitude;
    float _steeringRange;
protected:
    // abstraction layer
    virtual void onUpdate(float dt);
    virtual bool isEndOfProgram(void);
    virtual void onEndOfSubProgram(NPCProgram* subProgram);
public:
    // class implementation
    NPCFlight(NPC* npc, float steeringAltitude, float landingAltitude);
    virtual ~NPCFlight();
};

#endif