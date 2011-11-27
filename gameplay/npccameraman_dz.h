
#ifndef NPC_CAMERAMANDZ_PROGRAM_INCLUDED
#define NPC_CAMERAMANDZ_PROGRAM_INCLUDED

#include "npc.h"

/**
 * npc program : cameraman behaviour
 */

class NPCCameraman_DZ : public NPCProgram
{
protected:
    // abstraction layer
    virtual void onUpdate(float dt);
    virtual bool isEndOfProgram(void);
    virtual void onEndOfSubProgram(NPCProgram* subProgram);
public:
    NPCCameraman_DZ(NPC* npc);
    virtual ~NPCCameraman_DZ();
};

#endif