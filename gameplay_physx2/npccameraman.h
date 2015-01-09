
#ifndef NPC_CAMERAMAN_PROGRAM_INCLUDED
#define NPC_CAMERAMAN_PROGRAM_INCLUDED

#include "npc.h"

/**
 * npc program : cameraman behaviour
 */

class NPCCameraman : public NPCProgram
{
protected:
    // abstraction layer
    virtual void onUpdate(float dt);
    virtual bool isEndOfProgram(void);
    virtual void onEndOfSubProgram(NPCProgram* subProgram);
public:
    NPCCameraman(NPC* npc);
    virtual ~NPCCameraman();
};

#endif