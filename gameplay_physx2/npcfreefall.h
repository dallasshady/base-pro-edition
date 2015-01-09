
#ifndef NPC_FREEFALL_PROGRAM_INCLUDED
#define NPC_FREEFALL_PROGRAM_INCLUDED

#include "npc.h"

/**
 * npc program : freefall and track to desired position / character
 */

class NPCFreefall : public NPCProgram
{
private:    
    Vector3f _targetPos;
    bool     _positionIsSucceed;
    bool     _directionIsSucceed;
    Sensor   _sensor;
protected:
    // abstraction layer
    virtual void onUpdate(float dt);
    virtual bool isEndOfProgram(void);
    virtual void onEndOfSubProgram(NPCProgram* subProgram);
public:
    // class implementation
    NPCFreefall(NPC* npc);
    virtual ~NPCFreefall();
};

#endif