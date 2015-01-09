
#include "headers.h"
#include "npcfollow.h"
#include "npcmove.h"

NPCFollow::NPCFollow(NPC* npc, float distance) : NPCProgram( npc )
{
    _lastTargetPose = npc->getCatToy()->getCurrentPose();
    _distance = distance;
}

NPCFollow::~NPCFollow()
{
}

/**
 * abstraction layer
 */

void NPCFollow::onUpdate(float dt)
{
    // reset spinal cord
    SpinalCord* spinalCord = getNPC()->getSpinalCord();
    spinalCord->reset();

    // last target pose
    _lastTargetPose = getNPC()->getCatToy()->getCurrentPose();
    // extract target position
    Vector3f targetPos( _lastTargetPose[3][0], _lastTargetPose[3][1], _lastTargetPose[3][2] );
    targetPos += Vector3f( 0, jumperRoamingSphereSize, 0 );
    // NPC pose
    Matrix4f npcPose = getNPC()->getJumper()->getPose();
    // extract NPC position
    Vector3f npcPos( npcPose[3][0], npcPose[3][1], npcPose[3][2] );
    npcPos += Vector3f( 0, jumperRoamingSphereSize, 0 );

    // distance to target
    Vector3f distance = targetPos - npcPos;

    // flat distance
    Vector3f flatDistance = distance; flatDistance[1] = 0.0f;

    // distance too large?
    if( flatDistance.length() > _distance )
    {
        // call move program
        call( new NPCMove( getNPC(), targetPos, _distance ) );
    }
    else
    {
        // jumper absolute orientation
        Vector3f jumperAt = getNPC()->getJumper()->getClump()->getFrame()->getAt();
        jumperAt.normalize();

        // direction to target
        Vector3f targetDir = flatDistance;
        targetDir.normalize();

        // angle to target
        Vector3f atH = jumperAt; atH[1] = 0; atH.normalize();
        Vector3f dirH = targetDir; dirH[1] = 0; dirH.normalize();
        float targetAngle = ::calcAngle( dirH, atH, Vector3f( 0,1,0 ) );

        // turn jumper by AI algo
        if( fabs( targetAngle ) > 15.0f )
        {
            float aiRotationVel = 180.0f;
            float aiRotationAngle = sgn( targetAngle ) * aiRotationVel * dt;
            if( fabs( aiRotationAngle ) > fabs( targetAngle ) ) aiRotationAngle = targetAngle;
            getNPC()->getJumper()->getClump()->getFrame()->rotateRelative( Vector3f( 0,1,0 ), aiRotationAngle );
        }
    }
}

bool NPCFollow::isEndOfProgram(void)
{
    return ( getNPC()->getCatToy()->getPhase() != ::jpRoaming );
}

void NPCFollow::onEndOfSubProgram(NPCProgram* subProgram)
{
    // if jumper is stuck
    if( getNPC()->getJumper()->isStuck() )
    {
        // retrieve current jumper position    
        Vector3f jumperPos = getNPC()->getJumper()->getClump()->getFrame()->getPos();
        jumperPos += Vector3f( 0, jumperRoamingSphereSize, 0 );

        // search for nearest pathpoint
        bool isFound = false;
        Vector3f imPos;
        float imDist;
        unsigned int imId;
        for( unsigned int i=0; i<getNPC()->getEnclosure()->getNumMarkers(); i++ )
        {
            if( getNPC()->getEnclosure()->getMarkerFlags( i ) & mtPath )
            {
                Vector3f attemptPos = getNPC()->getEnclosure()->getMarkerFrame( i )->getPos();
                float attemptDist = ( attemptPos - jumperPos ).length();
                if( !isFound || attemptDist < imDist )
                {
                    imPos = attemptPos;
                    imDist = attemptDist;
                    imId = i;
                    isFound = true;
                }
            }
        }

        // if pathpoint is found
        if( isFound )
        {
            // move to pathpoint
            call( new NPCMove( getNPC(), imPos ) );
        }
    }
}