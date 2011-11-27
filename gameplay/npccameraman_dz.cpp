
#include "headers.h"
#include "npccameraman_dz.h"
#include "npcfollow.h"
#include "npcfreefall_dz.h"
#include "npcflight_dz.h"

/**
 * class implementation
 */

NPCCameraman_DZ::NPCCameraman_DZ(NPC* npc) : NPCProgram( npc )
{
}

NPCCameraman_DZ::~NPCCameraman_DZ()
{
}

/**
 * abstraction layer
 */

void NPCCameraman_DZ::onUpdate(float dt)
{
    // follow cat toy untile it is roaming
    if( false && getNPC()->getJumper()->getPhase() == ::jpRoaming )
    {
        call( new NPCFollow( getNPC(), 100.0f ) );
    }
    // frefall for cat toy until it is in freefall
    else if( getNPC()->getJumper()->getPhase() == ::jpRoaming || getNPC()->getJumper()->getPhase() == ::jpFreeFalling )
    {
        call( new NPCFreefall_DZ( getNPC() ) );
    }
    // flight for cat toy
    else 
    {
        bool canopyIsSkydiving = database::Canopy::getRecord( getNPC()->getJumper()->getVirtues()->equipment.canopy.id )->skydiving;
        if( canopyIsSkydiving )
        {
            call( new NPCFlight_DZ( getNPC(), 75.0f, 50.0f ) );
        }
        else
        {
            call( new NPCFlight_DZ( getNPC(), 25.0f, 7.5f ) );
        }
    }
}

bool NPCCameraman_DZ::isEndOfProgram(void)
{
    return false;
}

void NPCCameraman_DZ::onEndOfSubProgram(NPCProgram* subProgram)
{
}