
#include "headers.h"
#include "trollveggen.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "script.h"
#include "smokejet.h"
#include "ostankino.h"
#include "equip.h"

/**
 * clearance functions
 */

bool weatherClearanceTrollveggen(WeatherType weatherType)
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

bool windClearanceTrollveggen(Vector3f dir, float ambient, float blast)
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
 * common equip callback
 */

bool equipCallback_TrollBoogie(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

/**
 * interval jumping from trollryggen
 */

namespace script { class TrollBoogie_Trollryggen_IntervalJumping_Script : public Script
{
private:
    NPCL _npcs;
public:
    // script core
    TrollBoogie_Trollryggen_IntervalJumping_Script(Jumper* player, NPCL& npcs) : Script( player )
    {
        _npcs = npcs;

        // create routine
        //_routine = new Script::Countdown( player, npc->getCatToy() );
        //_routine = new Script::Timer( player );
        _routine = new Script::SeeYou( player, _npcs );
    }
    virtual void onUpdateActivity(float dt)
    {
        // base behaviour
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {            
            delete _routine;
            _routine = NULL;
        }
    }
}; }

static float intervalRewardRates[11] = {
    0.0f, 25.0f, 50.0f, 75.0f, 100.0f, 125.0f, 150.0f, 175.0f, 200.0f, 250.0f, 300.0f
};

void castingCallback_TrollBoogie_TrollryggenIntervalJumping(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // select NPCs
    std::vector<unsigned int> npcs;
    selectNPCs( mission->getPlayer(), 0.51f, npcs );

    // load first ghost & cast NPC
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/trollboogie/ij01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/trollboogie/ij02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/trollboogie/ij03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // load fourth ghost & cast NPC
    CatToy* catToy04 = CatToy::loadGhostCatToy( "./usr/cattoys/trollboogie/ij04.cattoy" ); assert( catToy04 );
    NPC* ghost04 = ::castGhostNPC( mission, exitPoint, catToy04, npcs ); assert( ghost04 );

    // cast script
    NPCL ghosts;
    ghosts.push_back( ghost01 );
    ghosts.push_back( ghost02 );
    ghosts.push_back( ghost03 );
    ghosts.push_back( ghost04 );
    new script::TrollBoogie_Trollryggen_IntervalJumping_Script( mission->getPlayer(), ghosts );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.0f );

    // reward goal    
    /*
    bool reward = ( getCore()->getRandToolkit()->getUniform(0.0f,1.0f) < 0.33f );
    if( reward ) 
    {
        GoalRewardDescriptor rewardDescriptor( intervalRewardRates );
        new GoalReward( mission->getPlayer(), &rewardDescriptor );
    }*/

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * 3-way from Trollryggen
 */

namespace script { class TrollBoogie_Trollryggen_3way_Script : public Script
{
private:
    NPC* _npc;
public:
    // script core
    TrollBoogie_Trollryggen_3way_Script(Jumper* player, NPC* npc) : Script( player )
    {
        _npc = npc;

        // create routine
        _routine = new Script::Countdown( player, npc->getCatToy() );
    }
    virtual void onUpdateActivity(float dt)
    {
        // base behaviour
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {            
            delete _routine;
            _routine = NULL;
        }
    }
}; }

void castingCallback_TrollBoogie_Trollryggen3way(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // select NPCs
    std::vector<unsigned int> npcs;
    selectNPCs( mission->getPlayer(), 0.26f, npcs );

    // load first ghost
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/trollboogie/3way01.cattoy" ); assert( catToy01 );

    // cast first ghost NPC
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/trollboogie/3way02.cattoy" ); assert( catToy02 );

    // cast first ghost NPC
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // cast script
    new script::TrollBoogie_Trollryggen_3way_Script( mission->getPlayer(), ghost01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 0.75f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * 3-way from Trollveggen
 */

namespace script { class TrollBoogie_Trollveggen_3way_Script : public Script
{
private:
    NPC* _npc;
public:
    // script core
    TrollBoogie_Trollveggen_3way_Script(Jumper* player, NPC* npc) : Script( player )
    {
        _npc = npc;

        // create routine
        _routine = new Script::Countdown( player, npc->getCatToy() );
    }
    virtual void onUpdateActivity(float dt)
    {
        // base behaviour
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {            
            delete _routine;
            _routine = NULL;
        }
    }
}; }

void castingCallback_TrollBoogie_Trollveggen3way(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // select NPCs
    std::vector<unsigned int> npcs;
    selectNPCs( mission->getPlayer(), 0.26f, npcs );

    // load first ghost
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/trollboogie/3way03.cattoy" ); assert( catToy01 );

    // cast first ghost NPC
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/trollboogie/3way04.cattoy" ); assert( catToy02 );

    // cast second ghost NPC
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // cast script
    new script::TrollBoogie_Trollveggen_3way_Script( mission->getPlayer(), ghost01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.125f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * proximity missions in trollveggen
 */

namespace script { class TrollBoogie_ProximityMission : public Script
{
public:
    // routines
    class Intro : public Message
    {
    public:
        Intro(Jumper* jumper) : Message( jumper, Gameplay::iLanguage->getUnicodeString(579), 10.0f ) {}
    };
public:
    // script core
    TrollBoogie_ProximityMission(Jumper* player) : Script( player )
    {
        // create routine
        _routine = new Intro( player );
    }
    virtual void onUpdateActivity(float dt)
    {
        // base behaviour
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {            
            delete _routine;
            _routine = NULL;
        }
    }
}; }

static float proximityRewardRates[11] = {
    0.0f, 0.0f, 0.0f, 100.0f, 150.0f, 200.0f, 250.0f, 300.0f, 350.0f, 400.0f, 450.0f
};

void castingCallback_TrollBoogie_Proximity(Mission* mission, unsigned int exitId)
{
    // exit point
    Enclosure* exitPoint = mission->getScene()->getExitPointEnclosure( exitId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 0.25f, 0.25f, 1.0f ), ::sjmLeft );

    // cast script
    new script::TrollBoogie_ProximityMission( mission->getPlayer() );

    // descriptor of proximity goal 
    GoalProximityDescriptor proximityDesc;
    proximityDesc.range0 = GoalProximityDescriptor::RangeMark( 50.0f, 0.05f );
    proximityDesc.range1 = GoalProximityDescriptor::RangeMark( 25.0f, 0.1f );
    proximityDesc.range2 = GoalProximityDescriptor::RangeMark( 10.0f, 0.25f );
    proximityDesc.range3 = GoalProximityDescriptor::RangeMark( 5.0f, 0.5f );    

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalProximity( mission->getPlayer(), &proximityDesc );

    // reward goal    
    /*
    bool reward = ( getCore()->getRandToolkit()->getUniform(0.0f,1.0f) < 0.5f );
    if( reward ) 
    {
        GoalRewardDescriptor rewardDescriptor( proximityRewardRates );
        new GoalReward( mission->getPlayer(), &rewardDescriptor );
    }*/
    
    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

void castingCallback_TrollBoogie_TrollProximity(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );
    castingCallback_TrollBoogie_Proximity( mission, mission->getMissionInfo()->exitPointId );
}

void castingCallback_TrollBoogie_TrollryggenProximity(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );
    castingCallback_TrollBoogie_Proximity( mission, mission->getMissionInfo()->exitPointId );
}

/**
 * heli-BASE mission
 */

void castingCallback_TrollBoogie_HeliBASE(Actor* parent)
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
    airplaneDesc.initAltitude  = 0.0f;
    airplaneDesc.lastAltitude  = 0.0f;
    airplaneDesc.loweringSpeed = 0.0f;
    airplaneDesc.initOffset.set( 50997, 152465, -16082 );
    airplaneDesc.initDirection.set( -0.9f, 0.0f, 0.4f );
    airplaneDesc.initDirection.normalize();
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.25f );

    // play original music for this mission    
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}