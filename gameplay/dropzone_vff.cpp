
#include "headers.h"
#include "dropzone.h"
#include "airplane.h"
#include "hud.h"
#include "npc.h"
#include "npccameraman.h"
#include "affinstructor.h"
#include "smokejet.h"
#include "version.h"

/**
 * database support : clearance functions
 */

bool weatherClearanceDropzone(WeatherType weatherType)
{
    // can't jump during bad weather
    if( weatherType == wtLightRain ||
        weatherType == wtHardRain ||
        weatherType == wtThunder )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool windClearanceDropzone(Vector3f dir, float ambient, float blast)
{
    // can't jump during strong wind
    if( 0.5f * ( ambient + blast ) > 9 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
 * common methods
 */

Virtues::Equipment selectSkydivingEquipment(Career* career)
{
    Virtues* virtues = career->getVirtues();

    Virtues::Equipment result;
    
    // orange Syndrome,
    result.helmet = Gear( ::gtHelmet, 8 );
    // orange Solifuge Altitude
    result.suit = Gear( ::gtSuit, 8 );
    // white Harpy
    result.rig = Gear( ::gtRig, 17 );

    // Felix Baumgartner?
    #ifdef GAMEPLAY_EDITION_ATARI
        if( career->getLicensedFlag() )
        {
            // FB suit
            result.suit = Gear( ::gtSuit, 28 );
            // FB skydiving rig
            result.rig = Gear( ::gtRig, 23 );
        }
    #endif

    // white Haibane
    if( virtues->appearance.weight > 75 )
    {
        // 250sqf
        result.canopy = Gear( ::gtCanopy, 19 );
    }
    else
    {
        // 220sqf
        result.canopy = Gear( ::gtCanopy, 18 );
    }
    // Felix Baumgartner?
    #ifdef GAMEPLAY_EDITION_ATARI
        if( career->getLicensedFlag() )
        {
            // FB skydiving canopy
            result.canopy = Gear( ::gtCanopy, 124 );
        }
    #endif

    // equipment options
    result.malfunctions = false;  // no malfunctions
    result.experience   = false;  // no experience
    result.pilotchute   = 0;      // 32' pilotchute
    result.sliderOption = ::soUp; // slider up

    return result;
}

/**
 * casting callbacks
 */

void castingCallback_VFF_TheBeginning(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // wind direction
    Vector3f wind = mission->getScene()->getLocation()->getWindDirection();

    // cast helicopter (cut'ed version, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initOffset.set( 0,0,0 );
    airplaneDesc.initDirection.cross( wind, Vector3f( 0,1,0 ) );
    airplaneDesc.initDirection.normalize();
    airplaneDesc.initDirection *= -1;
    airplaneDesc.initAltitude  = 20000.0f;
    airplaneDesc.lastAltitude  = 500.0f;
    airplaneDesc.loweringSpeed = 500.0f;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    Virtues::Equipment equipment = selectSkydivingEquipment( mission->getScene()->getCareer() );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast instructor
    new instructor::StaticLineInstructor01( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    
    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

void castingCallback_VFF_TheManeuvering01(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // wind direction
    Vector3f wind = mission->getScene()->getLocation()->getWindDirection();

    // cast helicopter (cut'ed version, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initOffset.set( 0,0,0 );
    airplaneDesc.initDirection.cross( wind, Vector3f( 0,1,0 ) );
    airplaneDesc.initDirection.normalize();
    airplaneDesc.initDirection *= 1;
    airplaneDesc.initAltitude  = 30000.0f;
    airplaneDesc.lastAltitude  = 500.0f;
    airplaneDesc.loweringSpeed = 500.0f;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    Virtues::Equipment equipment = selectSkydivingEquipment( mission->getScene()->getCareer() );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast instructor
    new instructor::StaticLineInstructor02( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    
    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

void castingCallback_VFF_TheManeuvering02(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter (cut'ed version, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 50000.0f;
    airplaneDesc.lastAltitude  = 800.0f;
    airplaneDesc.loweringSpeed = 700.0f;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    Virtues::Equipment equipment = selectSkydivingEquipment( mission->getScene()->getCareer() );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast instructor
    new instructor::StaticLineInstructor03( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    
    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

void castingCallback_VFF_HopAndPop(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter (cut'ed version, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 40000.0f;
    airplaneDesc.lastAltitude  = 400.0f;
    airplaneDesc.loweringSpeed = 400.0f;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    Virtues::Equipment equipment = selectSkydivingEquipment( mission->getScene()->getCareer() );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast instructor
    new instructor::HopAndPopInstructor( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    
    // no music for this mission
    Gameplay::iGameplay->stopSoundtrack();
}

void castingCallback_VFF_Freefall(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast airplane object (cut'ed, only right door exits)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Airplane01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/airplane.ogg";
    airplaneDesc.exitPointFrames.push_back( "RightDoor01" );
    airplaneDesc.exitPointFrames.push_back( "RightDoor02" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam1" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam2" );
    airplaneDesc.animationSpeed = 0.5f;
    airplaneDesc.initAltitude  = 300000.0f;
    airplaneDesc.lastAltitude  = 5000.0f;
    airplaneDesc.loweringSpeed = 500.0f;
	airplaneDesc.fixedWing = true;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    Virtues::Equipment equipment = selectSkydivingEquipment( mission->getScene()->getCareer() );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast instructor
    new instructor::FreefallInstructor01( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

void castingCallback_VFF_Tracking(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast airplane object (cut'ed, only right door exits)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Airplane01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/airplane.ogg";
    airplaneDesc.exitPointFrames.push_back( "RightDoor01" );
    airplaneDesc.exitPointFrames.push_back( "RightDoor02" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam1" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam2" );
    airplaneDesc.animationSpeed = 0.5f;
    airplaneDesc.initAltitude  = 300000.0f;
    airplaneDesc.lastAltitude  = 5000.0f;
    airplaneDesc.loweringSpeed = 500.0f;
	airplaneDesc.fixedWing = true;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    Virtues::Equipment equipment = selectSkydivingEquipment( mission->getScene()->getCareer() );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ), sjmRight );

    // cast instructor
    new instructor::FreefallInstructor02( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

void castingCallback_VFF_Gadgets(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // setup audible altimeter
    Altimeter::setAudibleAltimeter( mission->getScene()->getCareer(), false, 60000.0f );

    // cast airplane object (cut'ed, only right door exits)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Airplane01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/airplane.ogg";
    airplaneDesc.exitPointFrames.push_back( "RightDoor01" );
    airplaneDesc.exitPointFrames.push_back( "RightDoor02" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam1" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam2" );
    airplaneDesc.animationSpeed = 0.5f;
    airplaneDesc.initAltitude  = 100000.0f;
    airplaneDesc.lastAltitude  = 5000.0f;
    airplaneDesc.loweringSpeed = 500.0f;
	airplaneDesc.fixedWing = true;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    Virtues::Equipment equipment = selectSkydivingEquipment( mission->getScene()->getCareer() );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast instructor
    new instructor::FreefallInstructor03( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

void castingCallback_VFF_Linetwists(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter (cut'ed version, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initDirection.normalize();
    airplaneDesc.initDirection *= -1;
    airplaneDesc.initAltitude  = 30000.0f;
    airplaneDesc.lastAltitude  = 500.0f;
    airplaneDesc.loweringSpeed = 500.0f;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    Virtues::Equipment equipment = selectSkydivingEquipment( mission->getScene()->getCareer() );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, &equipment ) );

    // setup brief signature for player
    mission->getPlayer()->setSignatureType( stBrief );

    // cast instructor
    new instructor::LinetwistsInstructor( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}