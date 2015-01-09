
#ifndef NPC_FOLLOW_PROGRAM_INCLUDED
#define NPC_FOLLOW_PROGRAM_INCLUDED

#include "npc.h"

/**
 * npc program : follow the character
 */

class NPCFollow : public NPCProgram
{
private:
    Matrix4f _lastTargetPose;
    float    _distance;
protected:
    // abstraction layer
    virtual void onUpdate(float dt);
    virtual bool isEndOfProgram(void);
    virtual void onEndOfSubProgram(NPCProgram* subProgram);
public:
    NPCFollow(NPC* npc, float distance);
    virtual ~NPCFollow();
};

#endif