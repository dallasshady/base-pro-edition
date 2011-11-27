
#include "headers.h"
#include "royalgorge.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "script.h"
#include "smokejet.h"
#include "ostankino.h"
#include "equip.h"

void castingCallback_GoFall_RGB_Tracking(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter object (cuted, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 0.0f;
    airplaneDesc.lastAltitude  = 0.0f;
    airplaneDesc.loweringSpeed = 0.0f;
    airplaneDesc.initOffset.set( -114576, 42500, 123456 );
    airplaneDesc.initDirection.set( 0.3f, 0.0f, -0.4f );
    airplaneDesc.initDirection.normalize();
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // create common goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 500.0f;
    smokeBallDesc.numParticles = 128;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 500.0f;

    std::vector<SmokeBallDesc> smokeballs;

    // green smokeball
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -109522.9f, 27738.2f, 127991.1f );
    smokeballs.push_back( smokeBallDesc );

    // blue smokeball
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( -104472.4f, 21041.7f, 132413.2f );
    smokeballs.push_back( smokeBallDesc );

    // red smokeball
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -98341.5f, 15293.5f, 137133.8f );
    smokeballs.push_back( smokeBallDesc );

    // smokeball goal
    new GoalSmokeball( mission->getPlayer(), smokeballs, 1.5f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}

void castingCallback_GoFall_RGB_Extreme(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter object (cuted, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 0.0f;
    airplaneDesc.lastAltitude  = 0.0f;
    airplaneDesc.loweringSpeed = 0.0f;
    airplaneDesc.initOffset.set( -111711, 52500, 170776 );
    airplaneDesc.initDirection.set( -0.7f, 0.0f, -0.7f );
    airplaneDesc.initDirection.normalize();
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // create common goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 750.0f;
    smokeBallDesc.numParticles = 128;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 500.0f;

    std::vector<SmokeBallDesc> smokeballs;

    // green smokeball
    smokeballs.clear();
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -94004, 13320, 147413 );
    smokeballs.push_back( smokeBallDesc );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 2.0f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // blue smokeball
    smokeballs.clear();
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( -95321, 16064, 145988 );
    smokeballs.push_back( smokeBallDesc );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 2.5f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // red smokeball
    smokeballs.clear();
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -92426, 10738, 149185 );
    smokeballs.push_back( smokeBallDesc );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 3.0f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}