
#include "headers.h"
#include "dropzone.h"
#include "airplane.h"
#include "hud.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "baseinstructor.h"
#include "landingaccuracy.h"
#include "version.h"

/**
 * common methods
 */

Virtues::Equipment selectBASEEquipment(Career* career, float windAmbient, float windBlast)
{
    Virtues* virtues = career->getVirtues();

    Virtues::Equipment equipment;

    // orange Syndrome,
    equipment.helmet = Gear( ::gtHelmet, 8 );
    // orange Solifuge Altitude
    equipment.suit = Gear( ::gtSuit, 8 );
    // white Vector Velcro
    equipment.rig = Gear( ::gtRig, 9 );

    #ifdef GAMEPLAY_EDITION_ATARI
        if( career->getLicensedFlag() )
        {
            // FB jumpsuit
            equipment.suit = Gear( ::gtSuit, 28 );
            // FB BASE gear
            equipment.rig = Gear( ::gtRig, 22 );
        }
    #endif

    // choose optimal canopy square    
    float optimalSquare = database::Canopy::getOptimalCanopySquare( 
        virtues->appearance.weight,
        0.5f * ( windAmbient + windBlast )
    );

    // selection source (all purple Psychonauts)
    unsigned int numVariants = 10;
    unsigned int variants[] = { 0,1,2,3,4,5,6,7,8,9 };
    unsigned int variantId = 0;
    float epsilon = fabs( database::Canopy::getRecord( variants[variantId] )->square - optimalSquare );
    for( unsigned int i=1; i<numVariants; i++ )
    {
        float extraEpsilon = fabs( database::Canopy::getRecord( variants[i] )->square - optimalSquare );
        if( extraEpsilon < epsilon )
        {
            epsilon = extraEpsilon;
            variantId = i;
        }
    }

    // select canopy
    equipment.canopy = Gear( ::gtCanopy, variants[variantId] );

    #ifdef GAMEPLAY_EDITION_ATARI
        if( career->getLicensedFlag() )
        {
            if( ( 0.5f * ( windAmbient + windBlast ) ) > 6.0f )
            {
                equipment.canopy = Gear( ::gtCanopy, 105 );
            }
            else
            {
                equipment.canopy = Gear( ::gtCanopy, 106 );
            }
        }
    #endif

    // equipment options
    equipment.malfunctions = false;  // no malfunctions
    equipment.experience   = false;  // no experience
    equipment.pilotchute   = 0;      // 48' pilotchute
    equipment.sliderOption = ::soRemoved; // slider removed

    // result
    return equipment;
}

void castingCallback_BASEVFF_PCA(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // build forced equipment
    Virtues::Equipment equipment = selectBASEEquipment( 
        mission->getScene()->getCareer(),
        mission->getScene()->getLocation()->getWindAmbient(),
        mission->getScene()->getLocation()->getWindBlast()
    );

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast LICENSED_CHAR as assist base jumper
    unsigned int npcId = database::NPCInfo::getLicensedCharacterId();
    if( mission->getScene()->getCareer()->getLicensedFlag() ) 
    {
        npcId = database::NPCInfo::getRandomNonLicensedCharacter( 
            mission->getScene()->getCareer()->getVirtues()->getSkillLevel(), 0.25f
        );
    }
    NPC* joeBlack = new NPC( mission, npcId, NULL, exitPoint, CatToy::wrap( mission->getPlayer() ) );

    joeBlack->setProgram( new NPCAssist( joeBlack, mission->getPlayer() ) );

    // setup brief signature for LICENSED_CHAR
    joeBlack->getJumper()->setSignatureType( stBrief );

    // cast instructor
    new instructor::BASEInstructor01( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

void castingCallback_BASEVFF_Freefall(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // build forced equipment
    Virtues::Equipment equipment = selectBASEEquipment( 
        mission->getScene()->getCareer(),
        mission->getScene()->getLocation()->getWindAmbient(),
        mission->getScene()->getLocation()->getWindBlast()
    );

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast instructor
    new instructor::BASEInstructor02( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );

    // play no music for this mission
    Gameplay::iGameplay->stopSoundtrack();
}

void castingCallback_BASEVFF_LandingAccuracy(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // build forced equipment
    Virtues::Equipment equipment = selectBASEEquipment( 
        mission->getScene()->getCareer(),
        mission->getScene()->getLocation()->getWindAmbient(),
        mission->getScene()->getLocation()->getWindBlast()
    );

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast LICENSED_CHAR as assist base jumper
    unsigned int npcId = database::NPCInfo::getLicensedCharacterId();
    if( mission->getScene()->getCareer()->getLicensedFlag() ) 
    {
        npcId = database::NPCInfo::getRandomNonLicensedCharacter( 
            mission->getScene()->getCareer()->getVirtues()->getSkillLevel(), 0.25f
        );
    }
    NPC* joeBlack = new NPC( mission, npcId, NULL, exitPoint, CatToy::wrap( mission->getPlayer() ) );
    joeBlack->setProgram( new NPCAssist( joeBlack, mission->getPlayer() ) );

    // setup brief signature for LICENSED_CHAR
    joeBlack->getJumper()->setSignatureType( stBrief );

    // cast instructor
    new instructor::BASEInstructor03( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

void castingCallback_BASEVFF_Lineovers(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // cast helicopter (cut'ed version, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 30000.0f;
    airplaneDesc.lastAltitude  = 500.0f;
    airplaneDesc.loweringSpeed = 500.0f;
	airplaneDesc.fixedWing = false;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // build forced equipment
    Virtues::Equipment equipment = selectBASEEquipment( 
        mission->getScene()->getCareer(),
        mission->getScene()->getLocation()->getWindAmbient(),
        mission->getScene()->getLocation()->getWindBlast()
    );

    // enable malfuctions and select 42` pilotchute
    equipment.malfunctions = true;
    equipment.pilotchute = 2;

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast instructor
    new instructor::BASEInstructor04( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}