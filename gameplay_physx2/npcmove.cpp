
#include "headers.h"
#include "npcmove.h"
#include "../common/istring.h"

/**
 * class implementation
 */

NPCMove::NPCMove(NPC* npc, unsigned int markerId, float precision) : NPCProgram( npc )
{    
    _precision = ( precision < ::jumperRoamingSphereSize ) ? ::jumperRoamingSphereSize : precision;
    _position = getNPC()->getEnclosure()->getMarkerFrame( markerId )->getPos();
}

NPCMove::NPCMove(NPC* npc, Vector3f position, float precision) : NPCProgram( npc )
{
    _precision = ( precision < ::jumperRoamingSphereSize ) ? ::jumperRoamingSphereSize : precision;
    _position = position;
}

NPCMove::~NPCMove()
{
}

/**
 * abstraction layer
 */

static Vector3f _tpos( 0,0,0 );

void NPCMove::onUpdate(float dt)
{
    // reset spinal cord
    SpinalCord* spinalCord = getNPC()->getSpinalCord();
    spinalCord->reset();

    // jumper absolute orientation
    Vector3f jumperAt = getNPC()->getJumper()->getClump()->getFrame()->getAt();
    jumperAt.normalize();

    // retrieve current jumper position    
    Vector3f jumperPos = getNPC()->getJumper()->getClump()->getFrame()->getPos();
    jumperPos += Vector3f( 0, jumperRoamingSphereSize, 0 );

    // retrieve target marker position
    Vector3f targetPos = _position;
    _tpos = targetPos;

    // distance to target position
    float targetDist = ( targetPos - jumperPos ).length();

    // direction to target
    Vector3f targetDir = targetPos - jumperPos; 
    targetDir.normalize();

    // angle to target
    jumperAt[1] = 0; jumperAt.normalize();
    targetDir[1] = 0; targetDir.normalize();
    float targetAngle = ::calcAngle( targetDir, jumperAt, Vector3f( 0,1,0 ) );

    // move jumper by spinal cord
    if( fabs( targetAngle ) < 30.0f ) spinalCord->up = 1.0f;

    // turn jumper by AI algo
    float aiRotationVel = 180.0f;
    float aiRotationAngle = sgn( targetAngle ) * aiRotationVel * dt;
    if( fabs( aiRotationAngle ) > fabs( targetAngle ) ) aiRotationAngle = targetAngle;
    getNPC()->getJumper()->getClump()->getFrame()->rotateRelative( Vector3f( 0,1,0 ), aiRotationAngle );
}

bool NPCMove::isEndOfProgram()
{
    // stuck state
    if( getNPC()->getJumper()->isStuck() ) return true;

    // retrieve current jumper position    
    Vector3f jumperPos = getNPC()->getJumper()->getClump()->getFrame()->getPos();
    jumperPos += Vector3f( 0, jumperRoamingSphereSize, 0 );
    // retrieve target marker position
    Vector3f targetPos = _position;
    // distance to target position
    float targetDist = ( targetPos - jumperPos ).length();

    // traverse target marker
    return ( targetDist <= _precision );
}

void NPCMove::renderTargetPosition(void)
{
    Gameplay::iEngine->renderSphere( _tpos, ::jumperRoamingSphereSize, Vector4f( 1,0,0,1 ) );
}