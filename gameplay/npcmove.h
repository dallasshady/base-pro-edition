
#ifndef NPC_MOVE_PROGRAM_INCLUDED
#define NPC_MOVE_PROGRAM_INCLUDED

#include "npc.h"

/**
 * npc program : move to position
 */

class NPCMove : public NPCProgram
{
private:
    Vector3f _position;
    float    _precision;
protected:
    // abstraction layer
    virtual void onUpdate(float dt);
    virtual bool isEndOfProgram(void);
public:
    // class implementation
    NPCMove(NPC* npc, unsigned int markerId, float precision = ::jumperRoamingSphereSize);
    NPCMove(NPC* npc, Vector3f position, float precision = ::jumperRoamingSphereSize);
    virtual ~NPCMove();
public:
    static void renderTargetPosition(void);
};

#endif