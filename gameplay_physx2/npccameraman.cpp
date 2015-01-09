
#include "headers.h"
#include "npccameraman.h"
#include "npcfollow.h"
#include "npcfreefall.h"
#include "npcflight.h"

/**
 * class implementation
 */

NPCCameraman::NPCCameraman(NPC* npc) : NPCProgram( npc )
{
}

NPCCameraman::~NPCCameraman()
{
}

/**
 * abstraction layer
 */

void NPCCameraman::onUpdate(float dt)
{
    // follow cat toy untile it is roaming
    if( getNPC()->getCatToy()->getPhase() == ::jpRoaming )
    {
        call( new NPCFollow( getNPC(), 100.0f ) );
    }
    // frefall for cat toy until it is in freefall
    else if( getNPC()->getCatToy()->getPhase() == ::jpFreeFalling )
    {
        call( new NPCFreefall( getNPC() ) );
    }
    // flight for cat toy
    else 
    {
        bool canopyIsSkydiving = database::Canopy::getRecord( getNPC()->getJumper()->getVirtues()->equipment.canopy.id )->skydiving;
        if( canopyIsSkydiving )
        {
            call( new NPCFlight( getNPC(), 75.0f, 50.0f ) );
        }
        else
        {
            call( new NPCFlight( getNPC(), 25.0f, 7.5f ) );
        }
    }
}

bool NPCCameraman::isEndOfProgram(void)
{
    return false;
}

void NPCCameraman::onEndOfSubProgram(NPCProgram* subProgram)
{
}