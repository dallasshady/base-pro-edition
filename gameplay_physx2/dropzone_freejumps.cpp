
#include "headers.h"
#include "dropzone.h"
#include "airplane.h"
#include "hud.h"
#include "npc.h"
#include "npccameraman.h"
#include "npccameraman_dz.h"
#include "affinstructor.h"
#include "smokejet.h"
#include "equip.h"

/**
 * common methods
 */

void castSkydivers(Mission* mission, Airplane* airplane, unsigned int numSkydivers, bool npc_programs = true)
{
    assert( mission->getPlayer() );

    // detect player level
    float playerLevel = 0.25f * (
        mission->getPlayer()->getVirtues()->getEnduranceSkill() + 
        mission->getPlayer()->getVirtues()->getPerceptionSkill() +
        mission->getPlayer()->getVirtues()->getRiggingSkill() +
        mission->getPlayer()->getVirtues()->getTrackingSkill()
    );

    // detect player is a LICENSED_CHAR
    bool playerIsLicensed = mission->getScene()->getCareer()->getLicensedFlag();

    // build list of same level npc    
    std::vector<unsigned int> npcs;
    database::NPCInfo::select( playerLevel, 0.126f, !playerIsLicensed, npcs );
	
    for( unsigned i = 0; i < numSkydivers; ++i )
    {
        if( npcs.size() )
        {
            // select NPC
            unsigned int index = getCore()->getRandToolkit()->getUniformInt() % npcs.size();
            unsigned int npcId = npcs[index];
			npcs.erase( npcs.begin() + index );
            // create NPC
            NPC* npc = new NPC( mission, npcId, airplane, NULL, CatToy::wrap( mission->getPlayer() ), mission->getPlayer(), true );
			npc->getJumper()->getFreefallActor()->setMass(mission->getPlayer()->getVirtues()->appearance.weight);

            // setup cameraman behaviour
			if (npc_programs) npc->setProgram( new NPCCameraman_DZ( npc ) );
            // setup brief signature
            npc->getJumper()->setSignatureType( stBrief );
			npc->getJumper()->setDefaultPose(mission->getPlayer()->getDefaultPose());
        }
    }
}

NPC *castTandemStudent(Mission* mission, Airplane* airplane)
{
    assert( mission->getPlayer() );

    // detect player level
    float playerLevel = 0.25f * (
        mission->getPlayer()->getVirtues()->getEnduranceSkill() + 
        mission->getPlayer()->getVirtues()->getPerceptionSkill() +
        mission->getPlayer()->getVirtues()->getRiggingSkill() +
        mission->getPlayer()->getVirtues()->getTrackingSkill()
    );

    // detect player is a LICENSED_CHAR
    bool playerIsLicensed = mission->getScene()->getCareer()->getLicensedFlag();

    // build list of same level npc    
    std::vector<unsigned int> npcs;
    database::NPCInfo::select( playerLevel, 0.126f, !playerIsLicensed, npcs );
	
    if( npcs.size() ) {
        // select NPC
        unsigned int index = getCore()->getRandToolkit()->getUniformInt() % npcs.size();
        unsigned int npcId = npcs[index];
		npcs.erase( npcs.begin() + index );
        // create NPC
        NPC* npc = new NPC( mission, npcId, airplane, NULL, CatToy::wrap( mission->getPlayer() ), mission->getPlayer(), true );
		npc->getJumper()->getFreefallActor()->setMass(mission->getPlayer()->getVirtues()->appearance.weight);

        // setup student behaviour
		//npc->setProgram( new NPCTandemStudent( npc ) );
        // setup brief signature
        npc->getJumper()->setSignatureType( stBrief );
		npc->getJumper()->setDefaultPose(mission->getPlayer()->getDefaultPose());
		return npc;
    }

	return NULL;
}

void castMultijump(Actor* parent, float altitude )
{
	/// TANDEM TEST MISSION
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );
	
    // setup audible altimeter
    Altimeter::setAudibleAltimeter( mission->getScene()->getCareer(), true, 70000.0f );
	char *cessnaExitFrames[3] = {"LeftDoor01", "RightDoor01", "RightDoor02"};


	// cast airplane object
	AirplaneDesc airplaneDesc;
	airplaneDesc.templateClump = parent->getScene()->findClump( "Airplane02" ); assert( airplaneDesc.templateClump );
	airplaneDesc.propellerFrame = "PropellerSound";
	airplaneDesc.propellerSound = "./res/sounds/aircrafts/airplane.ogg";
	// set number of exits
	airplaneDesc.exitPointFrames.push_back( cessnaExitFrames[0] );
	airplaneDesc.exitPointFrames.push_back( cessnaExitFrames[0] );

	airplaneDesc.cameraFrames.push_back( "Cesna_Cam" );
	airplaneDesc.cameraFrames.push_back( "Cesna_Cam1" );
	airplaneDesc.cameraFrames.push_back( "Cesna_Cam2" );
	airplaneDesc.animationSpeed = 0.68f;		// 0.5 ~ 30m/s (~59knots)
	airplaneDesc.initAltitude  = altitude;
	airplaneDesc.initOffset = Vector3f(0, 0, 0);
	float offset = 90.0f + (altitude - 400000.0f) / 10000.0f;
	if (altitude <= 100000.0f) {
		offset = 0;
	}
	airplaneDesc.initOffset += wrap(mission->getScene()->getWindAtPoint(NxVec3(0,0,0))) * offset;
	airplaneDesc.lastAltitude  = 5000.0f;
	airplaneDesc.loweringSpeed = 500.0f;
	airplaneDesc.fixedWing = true;
	Airplane* airplane = new Airplane( mission, &airplaneDesc );


	// cast player on airplane			
	mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

	// load ghost
	//CatToy* catToy = CatToy::loadGhostCatToy( "./usr/cattoys/skybase/h500_r5_su_7d2.cattoy" );
    //CatToy* catToy = CatToy::loadGhostCatToy( "./usr/cattoys/ghost.cattoy" );
	//assert( catToy );
	//castSkybaserForGhost(mission, airplane, catToy);

	// setup full signature for player
	mission->getPlayer()->setSignatureType( stFull );
	mission->getPlayer()->InDropzone(true);
	mission->getPlayer()->setDefaultPose(0);
	
	// cast NPCs
	//castTandemInstructor( mission, airplane );
	//mission->getPlayer()->setTandemStudent( castTandemStudent( mission, airplane )->getJumper() );

    // cast goals
    new GoalDropzone( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
	new GoalOpening( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
	new GoalFreeFallTime( mission->getPlayer() );
	new GoalCanopyTime( mission->getPlayer() );
	new GoalJumps( mission->getPlayer() );
	

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}


void castFreejump(Actor* parent, float altitude, int pose = 0)
{


	/// TEST MISSION
	if (altitude == 700000.0f) {
		altitude =  100000.0f;
		
		Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );
		
		// setup audible altimeter
		Altimeter::setAudibleAltimeter( mission->getScene()->getCareer(), false, 70000.0f );
		
		char *cessnaExitFrames[3] = {"LeftDoor01", "RightDoor01", "RightDoor02"};

		// cast airplane object
		AirplaneDesc airplaneDesc;
		airplaneDesc.templateClump = parent->getScene()->findClump( "Airplane01" ); assert( airplaneDesc.templateClump );
		airplaneDesc.propellerFrame = "PropellerSound";
		airplaneDesc.propellerSound = "./res/sounds/aircrafts/airplane.ogg";
		// set number of exits
		for (int load = 0; load < 6; ++load) {
			airplaneDesc.exitPointFrames.push_back( cessnaExitFrames[load%3] );
		}
		airplaneDesc.cameraFrames.push_back( "Cesna_Cam" );
		airplaneDesc.cameraFrames.push_back( "Cesna_Cam1" );
		airplaneDesc.cameraFrames.push_back( "Cesna_Cam2" );
		airplaneDesc.animationSpeed = 0.68f;		// 0.5 ~ 30m/s (~59knots)
		airplaneDesc.initAltitude  = altitude;
		airplaneDesc.initOffset = Vector3f(0, 0, 0);
		float offset = 90.0f + (altitude - 400000.0f) / 10000.0f;
		if (altitude <= 100000.0f) {
			offset = 0;
		}
		airplaneDesc.initOffset += wrap(mission->getScene()->getWindAtPoint(NxVec3(0,0,0))) * offset;
		airplaneDesc.lastAltitude  = 5000.0f;
		airplaneDesc.loweringSpeed = 500.0f;
		airplaneDesc.fixedWing = true;
		Airplane* airplane = new Airplane( mission, &airplaneDesc );

		// exit point
		Enclosure* exitPoint = new Enclosure(airplaneDesc.templateClump, 0.0f);

		
		// cast player on airplane			
		mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

		// setup full signature for player
		mission->getPlayer()->setSignatureType( stFull );
		mission->getPlayer()->InDropzone(true);

		// cast goals
		new GoalDropzone( mission->getPlayer() );
		new GoalOpening( mission->getPlayer() );
		new GoalLanding( mission->getPlayer() );
		new GoalStateOfHealth( mission->getPlayer() );
		new GoalStateOfGear( mission->getPlayer() );
		new GoalExperience( mission->getPlayer() );
		new GoalFreeFallTime( mission->getPlayer() );
		new GoalCanopyTime( mission->getPlayer() );
		new GoalJumps( mission->getPlayer() );

		// play original music for this mission
		Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
		return;
	}


	/// NORMAL MISSION
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // setup audible altimeter
    Altimeter::setAudibleAltimeter( mission->getScene()->getCareer(), true, 70000.0f );
	char *cessnaExitFrames[3] = {"LeftDoor01", "RightDoor01", "RightDoor02"};

	/// MASS JUMP
	int planes = 1;
	int divers_in_plane = 6;
	if (altitude == 430000.0f) {
		planes = 2;
	}
	for (int a = 0; a < planes; ++a) {
		// cast airplane object
		AirplaneDesc airplaneDesc;
		airplaneDesc.templateClump = parent->getScene()->findClump( "Airplane01" ); assert( airplaneDesc.templateClump );
		airplaneDesc.propellerFrame = "PropellerSound";
		airplaneDesc.propellerSound = "./res/sounds/aircrafts/airplane.ogg";
		// set number of exits
		for (int load = 0; load < divers_in_plane; ++load) {
			airplaneDesc.exitPointFrames.push_back( cessnaExitFrames[load%3] );
		}
		airplaneDesc.cameraFrames.push_back( "Cesna_Cam" );
		airplaneDesc.cameraFrames.push_back( "Cesna_Cam1" );
		airplaneDesc.cameraFrames.push_back( "Cesna_Cam2" );
		airplaneDesc.animationSpeed = 0.68f;		// 0.5 ~ 30m/s (~59knots)
		airplaneDesc.initAltitude  = altitude - a*1000.0f;
		airplaneDesc.initOffset = Vector3f(a*1300.0f, 0, 0);
		float offset = (altitude - 400000.0f) / 10000.0f;
		if (altitude <= 100000.0f) {
			offset = 0;
		}
		airplaneDesc.initOffset += wrap(mission->getScene()->getWindAtPoint(NxVec3(0,0,0))) * offset;
		airplaneDesc.lastAltitude  = 5000.0f;
		airplaneDesc.loweringSpeed = 500.0f;
		airplaneDesc.fixedWing = true;
		Airplane* airplane = new Airplane( mission, &airplaneDesc );

		if (a == 0) { 
			// cast player on airplane			
			mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );
			if (pose == 1) {
				// cast player smokejet
				new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 0.25f, 0.25f, 1.0f ), ::sjmLeft );
			}

			// setup full signature for player
			mission->getPlayer()->setSignatureType( stFull );
			mission->getPlayer()->InDropzone(true);
			mission->getPlayer()->setDefaultPose(pose);
			
			// cast NPCs
			castSkydivers( mission, airplane, airplaneDesc.exitPointFrames.size()-1, true );
		} else {
			castSkydivers( mission, airplane, airplaneDesc.exitPointFrames.size(), true );
		}
	}
    // cast goals
    new GoalDropzone( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
	new GoalOpening( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
	new GoalFreeFallTime( mission->getPlayer() );
	new GoalCanopyTime( mission->getPlayer() );
	new GoalJumps( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

/**
 * startup callbacks
 */

void castingCallback_Freejump_800(Actor* parent)
{
    castFreejump( parent, 80000.0f );
}

void castingCallback_Freejump_1500(Actor* parent)
{
    castFreejump( parent, 150000.0f );
}

void castingCallback_Freejump_3000(Actor* parent)
{
    castFreejump( parent, 300000.0f );
}

void castingCallback_Freejump_4000(Actor* parent)
{
    castFreejump( parent, 400000.0f );
}

void castingCallback_Freejump_4000_Mass(Actor* parent)
{
    castFreejump( parent, 430000.0f );
}
void castingCallback_Freejump_8000(Actor* parent)
{
    castFreejump( parent, 800000.0f, 1 );
}
void castingCallback_Freejump_Headdown(Actor* parent)
{
    castFreejump( parent, 400000.0f, 1 );
}
void castingCallback_Freejump_Sitfly(Actor* parent)
{
    castFreejump( parent, 400000.0f, 2 );
}
void castingCallback_Freejump_Backfly(Actor* parent)
{
    castFreejump( parent, 400000.0f, 3 );
}

void castingCallback_Freejump_Multi(Actor* parent)
{
    castMultijump( parent, 400000.0f );
}
/**
 * equip callback
 */

bool equipCallback_Freejump(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestSkydivingEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 0;

    return true;
}

bool equipCallback_FreejumpCommonSuit(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestSkydivingEquipment( career, windAmbient, windBlast ) ) return false;

    // equip best common suit
    if( !equipBestSuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 0;

    return true;
}

bool equipCallback_FreejumpWingsuit(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestSkydivingEquipment( career, windAmbient, windBlast ) ) return false;

    // equip best wingsuit
    if( !equipBestWingsuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 0;

    return true;
}