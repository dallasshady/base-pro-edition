
#ifndef NPC_ASSIST_PROGRAM_INCLUDED
#define NPC_ASSIST_PROGRAM_INCLUDED

#include "npc.h"

/**
 * npc program : assist
 */

class NPCAssist : public NPCProgram
{
protected:
    Jumper* _ward;
protected:
    // abstraction layer
    virtual void onUpdate(float dt);
    virtual bool isEndOfProgram(void);
    virtual void onEndOfSubProgram(NPCProgram* subProgram);
public:
    NPCAssist(NPC* npc, Jumper* ward);
    virtual ~NPCAssist();
};

#endif