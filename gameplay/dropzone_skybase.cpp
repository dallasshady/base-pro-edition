
#include "headers.h"
#include "dropzone.h"
#include "airplane.h"
#include "hud.h"
#include "npc.h"
#include "npccameraman.h"
#include "affinstructor.h"
#include "smokejet.h"
#include "equip.h"

/**
 * common methods
 */

void castSkybaserForGhost(Mission* mission, Airplane* airplane, CatToy* ghost)
{
    assert( ghost );

    // detect cattoy level
    float cattoyLevel = 0.25f * (
        ghost->getVirtues()->getEnduranceSkill() + 
        ghost->getVirtues()->getPerceptionSkill() +
        ghost->getVirtues()->getRiggingSkill() +
        ghost->getVirtues()->getTrackingSkill()
    );

    // detect player is a LICENSED_CHAR
    bool playerIsLicensed = mission->getScene()->getCareer()->getLicensedFlag();

    // build list of same level npc    
    std::vector<unsigned int> npcs;
    database::NPCInfo::select( cattoyLevel, 0.126f, !playerIsLicensed, npcs );

    // select NPC
    unsigned int index = getCore()->getRandToolkit()->getUniformInt() % npcs.size();
    unsigned int npcId = npcs[index];
    
    // create NPC
    NPC* npc = new NPC( mission, npcId, airplane, NULL, ghost );

    // devote ghost
    npc->devoteCattoy();

    // setup cameraman behaviour
    npc->setProgram( new NPCCameraman( npc ) );

    // setup brief signature
    npc->getJumper()->setSignatureType( stBrief );
}

void castSkybaseJump(Actor* parent, float altitude, CatToy* ghost)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter object
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit02" );
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit03" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = altitude;
    airplaneDesc.lastAltitude  = altitude;
    airplaneDesc.loweringSpeed = 0.0f;
	airplaneDesc.fixedWing = false;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // cast player on airplane
	mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );
	

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast NPC
    if( ghost != NULL ) castSkybaserForGhost( mission, airplane, ghost );

    // cast goals
    new GoalLanding( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

/**
 * mission startups
 */

void castingCallback_Skybase_250(Actor* parent)
{
    // select ghost
    CatToy* ghost = NULL;
    switch( getCore()->getRandToolkit()->getUniformInt() % 2 )
    {
    case 0:
        ghost = CatToy::loadGhostCatToy( "./usr/cattoys/skybase/h250_r10_sd_2d5.cattoy" );
        break;
    case 1:
        ghost = CatToy::loadGhostCatToy( "./usr/cattoys/skybase/h250_r5_sd_3d0.cattoy" );
        break;
    }

    castSkybaseJump( parent, 25000.0f, ghost );
}

bool equipCallback_Skybase_250(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider down and 40' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soDown;
    career->getVirtues()->equipment.pilotchute   = 3;

    return true;
}

void castingCallback_Skybase_500(Actor* parent)
{
    // select ghost
    CatToy* ghost = NULL;
    switch( getCore()->getRandToolkit()->getUniformInt() % 2 )
    {
    case 0:
        ghost = CatToy::loadGhostCatToy( "./usr/cattoys/skybase/h500_r5_su_7d2.cattoy" );
        break;
    case 1:
        ghost = CatToy::loadGhostCatToy( "./usr/cattoys/skybase/h500_r10_su_8d3.cattoy" );
        break;
    }

    castSkybaseJump( parent, 50000.0f, ghost );
}

bool equipCallback_Skybase_500(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider up and 38' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 4;

    return true;
}