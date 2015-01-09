
#include "headers.h"
#include "npcassist.h"
#include "npcfollow.h"
#include "npccameraman.h"

/**
 * class implementation
 */

NPCAssist::NPCAssist(NPC* npc, Jumper* ward) : NPCProgram( npc )
{
    _ward = ward;
}

NPCAssist::~NPCAssist()
{    
}

/**
 * abstraction layer
 */

void NPCAssist::onUpdate(float dt)
{
    // follow cat toy until it is roaming
    if( getNPC()->getCatToy()->getPhase() == ::jpRoaming )
    {
        call( new NPCFollow( getNPC(), 75.0f ) );
    }
    // pull and drop ward's pilotchute
    else if( _ward && !_ward->getFreefallActor()->isSleeping() )
    {
        // pilotchute pull frame
        Vector3f pp = Jumper::getLineHandJoint( getNPC()->getJumper()->getClump() )->getPos();
        Vector3f py = _ward->getClump()->getFrame()->getPos() - getNPC()->getJumper()->getClump()->getFrame()->getPos(); py.normalize();
        Vector3f px; px.cross( py, Vector3f(0,1,0) ); px.normalize();
        Vector3f pz; py.cross( px, py ); pz.normalize();

        // check wind
        float windSpeed = 0.5f * ( getNPC()->getScene()->getLocation()->getWindAmbient() +
                                   getNPC()->getScene()->getLocation()->getWindBlast() );
        Vector3f wardAt = _ward->getClump()->getFrame()->getAt();
        wardAt.normalize();
        if( windSpeed < 2.0f || Vector3f::dot( wardAt, getNPC()->getScene()->getLocation()->getWindDirection() ) < 0 )
        {
            // connect pilot chute
            _ward->getPilotchuteSimulator()->connect( 
                _ward->getFreefallActor(), 
                Jumper::getBackBone( _ward->getClump() ), 
                _ward->getLocalPilotAnchor() 
            );

            // pull pilotchute
            _ward->getPilotchuteSimulator()->pull( Matrix4f(
                px[0], px[1], px[2], 0.0f,
                py[0], py[1], py[2], 0.0f,
                pz[0], pz[1], pz[2], 0.0f,
                pp[0], pp[1], pp[2], 1.0f
            ) );
            _ward->getPilotchuteSimulator()->updateActivity( 0.0f );

            // and drop
            _ward->getPilotchuteSimulator()->drop( NxVec3( 0,0,0 ) );
            _ward->getPilotchuteSimulator()->setInflation( 0.25f );
        }

        // and no longer track the ward happiness :)
        _ward = NULL;
    }
}

bool NPCAssist::isEndOfProgram(void)
{
    return false;
}

void NPCAssist::onEndOfSubProgram(NPCProgram* subProgram)
{
}