
#ifndef NPC_FREEFALLDZ_PROGRAM_INCLUDED
#define NPC_FREEFALLDZ_PROGRAM_INCLUDED

#include "npc.h"

/**
 * npc program : freefall and track to desired position / character
 */

class NPCFreefall_DZ : public NPCProgram
{
private:    
    Vector3f _targetPos;
    bool     _positionIsSucceed;
    bool     _directionIsSucceed;
    Sensor   _sensor;
	float	 _timeUntilJump;
protected:
    // abstraction layer
    virtual void onUpdate(float dt);
    virtual bool isEndOfProgram(void);
    virtual void onEndOfSubProgram(NPCProgram* subProgram);
public:
    // class implementation
    NPCFreefall_DZ(NPC* npc);
    virtual ~NPCFreefall_DZ();
};

#endif