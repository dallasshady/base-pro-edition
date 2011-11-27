
#include "headers.h"
#include "cave.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "script.h"
#include "equip.h"
#include "ostankino.h"
#include "kjerag.h"

/**
 * clearance functions
 */

bool weatherClearanceKjerag(WeatherType weatherType)
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

bool windClearanceKjerag(Vector3f dir, float ambient, float blast)
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
 * equipment
 */

bool equipCallback_Kjerag(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;
    if( !equipBestSuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

bool equipCallback_Kjerag_Wings(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;
    if( !equipBestWingsuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

/**
 * freejumps
 */

void castingCallback_Kjerag_Freejump(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.0f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * Kjerag Boogie : Interval Jumps
 */

namespace script { class Kjerag_IntervalJumps : public Script
{
private:
    NPCL _npcs;
public:
    // script core
    Kjerag_IntervalJumps(Jumper* player, NPCL& npcs) : Script( player )
    {
        // create routine
        _npcs = npcs;
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

void castingCallback_KjeragBoogie_IntervalJumps(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/ij01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/ij02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/ij03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // load fourth ghost & cast NPC
    CatToy* catToy04 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/ij04.cattoy" ); assert( catToy04 );
    NPC* ghost04 = ::castGhostNPC( mission, exitPoint, catToy04, npcs ); assert( ghost04 );

    // load fifth ghost & cast NPC
    CatToy* catToy05 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/ij05.cattoy" ); assert( catToy05 );
    NPC* ghost05 = ::castGhostNPC( mission, exitPoint, catToy05, npcs ); assert( ghost05 );

    // cast script
    NPCL ghosts;
    ghosts.push_back( ghost01 );
    ghosts.push_back( ghost02 );
    ghosts.push_back( ghost03 );
    ghosts.push_back( ghost04 );
    ghosts.push_back( ghost05 );
    new script::Kjerag_IntervalJumps( mission->getPlayer(), ghosts );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 1.25f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * Kjerag Boogie : 6-way from Exit Point ¹2
 */

namespace script { class Kjerag_6wayFromEP2 : public Script
{
public:
    // script core
    Kjerag_6wayFromEP2(Jumper* player, CatToy* leaderCatToy) : Script( player )
    {
        // create routine
        //_routine = new Script::Timer( player );
        _routine = new Script::Countdown( player, leaderCatToy );
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

void castingCallback_KjeragBoogie_6wayFromEP2(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/6way2ep01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/6way2ep02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/6way2ep03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // load fourth ghost & cast NPC
    CatToy* catToy04 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/6way2ep04.cattoy" ); assert( catToy04 );
    NPC* ghost04 = ::castGhostNPC( mission, exitPoint, catToy04, npcs ); assert( ghost04 );

    // load fifth ghost & cast NPC
    CatToy* catToy05 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/6way2ep05.cattoy" ); assert( catToy05 );
    NPC* ghost05 = ::castGhostNPC( mission, exitPoint, catToy05, npcs ); assert( ghost05 );

    // cast script
    new script::Kjerag_6wayFromEP2( mission->getPlayer(), catToy01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 1.375f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * Kjerag Boogie : 6-way from Terminal Wall
 */

namespace script { class Kjerag_6wayFromTW : public Script
{
public:
    // script core
    Kjerag_6wayFromTW(Jumper* player, CatToy* leaderCatToy) : Script( player )
    {
        // create routine
        //_routine = new Script::Timer( player );
        _routine = new Script::Countdown( player, leaderCatToy );
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

void castingCallback_KjeragBoogie_6wayFromTW(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/6waytw01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/6waytw02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/6waytw03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // load fourth ghost & cast NPC
    CatToy* catToy04 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/6waytw04.cattoy" ); assert( catToy04 );
    NPC* ghost04 = ::castGhostNPC( mission, exitPoint, catToy04, npcs ); assert( ghost04 );

    // load fifth ghost & cast NPC
    CatToy* catToy05 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/6waytw05.cattoy" ); assert( catToy05 );
    NPC* ghost05 = ::castGhostNPC( mission, exitPoint, catToy05, npcs ); assert( ghost05 );

    // cast script
    new script::Kjerag_6wayFromTW( mission->getPlayer(), catToy01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 1.625f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * 3-way with wingsuits from EP1
 */

namespace script { class Kjerag_3wingsFromEP1 : public Script
{
public:
    // script core
    Kjerag_3wingsFromEP1(Jumper* player, CatToy* leaderCatToy) : Script( player )
    {
        // create routine
        //_routine = new Script::Timer( player );
        _routine = new Script::Countdown( player, leaderCatToy );
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

void castingCallback_KjeragBoogie_3wingsFromEP1(Actor* parent)
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

    // load first ghost & cast NPC
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/3wings1ep01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/3wings1ep02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // cast script
    new script::Kjerag_3wingsFromEP1( mission->getPlayer(), catToy01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 1.75f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * 3-way with wingsuits from EP1
 */

namespace script { class Kjerag_3wingsFromEP3 : public Script
{
public:
    // script core
    Kjerag_3wingsFromEP3(Jumper* player, CatToy* leaderCatToy) : Script( player )
    {
        // create routine
        _routine = new Script::Timer( player );
        //_routine = new Script::Countdown( player, leaderCatToy );
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

void castingCallback_KjeragBoogie_3wingsFromEP3(Actor* parent)
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

    // load first ghost & cast NPC
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/3wings3ep01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/kjerag/3wings3ep02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // cast script
    new script::Kjerag_3wingsFromEP3( mission->getPlayer(), catToy01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 1.75f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * proximity jump from exit point ¹1
 */

namespace script { class Kjerag_Proxi : public Script
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
    Kjerag_Proxi(Jumper* player) : Script( player )
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

void castingCallback_KjeragBoogie_Proxi(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // exit point
    Enclosure* exitPoint = mission->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 0.25f, 0.25f, 1.0f ), ::sjmLeft );

    // cast script
    new script::Kjerag_Proxi( mission->getPlayer() );

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
    
    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * heli-BASE mission
 */

void castingCallback_KjeragBoogie_HeliBASE(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter object
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 0.0f;
    airplaneDesc.lastAltitude  = 0.0f;
    airplaneDesc.loweringSpeed = 0.0f;    
    airplaneDesc.initOffset.set( -306085, 99844, 275535 );
    airplaneDesc.initDirection.set( -0.9f, 0.0f, -0.3f );
    airplaneDesc.initDirection.normalize();
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );
    new SmokeJet( mission->getPlayer(), Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ), ::sjmRight );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.75f );

    // play original music for this mission    
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * heli-BASE mission ¹2
 */

void castingCallback_KjeragBoogie_HeliBASE2(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter object
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 0.0f;
    airplaneDesc.lastAltitude  = 0.0f;
    airplaneDesc.loweringSpeed = 0.0f;
    airplaneDesc.initOffset.set( -224714, 110000, 345413 );
    airplaneDesc.initDirection.set( -0.9f, 0.0f, 0.3f );
    airplaneDesc.initDirection.normalize();
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );
    new SmokeJet( mission->getPlayer(), Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ), ::sjmRight );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 2.0f );

    // play original music for this mission    
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * heli-BASE ¹3
 */

void castingCallback_KjeragBoogie_HeliBASE3(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter object
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 0.0f;
    airplaneDesc.lastAltitude  = 0.0f;
    airplaneDesc.loweringSpeed = 0.0f;
    airplaneDesc.initOffset.set( -159876, 116000, 196040 );
    airplaneDesc.initDirection.set( -0.8f, 0.0f, 0.4f );
    airplaneDesc.initDirection.normalize();
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );
    new SmokeJet( mission->getPlayer(), Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ), ::sjmRight );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 2.0f );

    // play original music for this mission    
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * Kjerag Smokeball Missions 
 */

/**
 * RGB-Tracking
 */

namespace script { class Kjerag_RGBTracking : public Script
{
public:
    // script routines
    class Disqualification : public Message
    {
    public:
        Disqualification(Jumper* jumper) : Message( jumper, L"", 1.0f ) 
        {
            jumper->enablePhase( false );
            setLock( true );
        }
        virtual void onUpdateRoutine(float dt)
        {
            Script::Message::onUpdateRoutine( dt );

            // retrieve suit info
            database::Suit* suit = database::Suit::getRecord( getJumper()->getVirtues()->equipment.suit.id );
            if( !suit->wingsuit )
            {
                setLock( false );
                getJumper()->enablePhase( true );
            }
            else
            {
                setMessage( Gameplay::iLanguage->getUnicodeString(643) );
            }
        }
    };
public:
    // script core
    Kjerag_RGBTracking(Jumper* player) : Script( player )
    {
        // create routine
        _routine = new Disqualification( player );
    }
    virtual void onUpdateActivity(float dt)
    {
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {
            delete _routine;
            _routine = NULL;
        }
    }
}; }

void castingCallback_KjeragSmokeball_RGBTracking(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // exit point
    Enclosure* exitPoint = mission->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    std::vector<SmokeBallDesc> smokeballs;

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 2000.0f;
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 1050.0f;

    // cast blue smokeball 
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( -31304.8, 25398.3, 80255.8 );
    smokeballs.push_back( smokeBallDesc );

    // cast green smokeball 
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -9925.1, 48511.3, 96634.1 );
    smokeballs.push_back( smokeBallDesc );

    // cast red smokeball 
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( 5990.9, 74315.5, 107900.6 );
    smokeballs.push_back( smokeBallDesc );

    // cast script
    new script::Kjerag_RGBTracking( mission->getPlayer() );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 0.75f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );
    
    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_extinct.ogg" );
}

/**
 * RGB-Slalom ¹1
 */

namespace script { class Kjerag_RGBSlalom : public Script
{
public:
    // script routines
    class Disqualification : public Message
    {
    public:
        Disqualification(Jumper* jumper) : Message( jumper, L"", 1.0f ) 
        {
            jumper->enablePhase( false );
            setLock( true );
        }
        virtual void onUpdateRoutine(float dt)
        {
            Script::Message::onUpdateRoutine( dt );

            // retrieve suit info
            database::Suit* suit = database::Suit::getRecord( getJumper()->getVirtues()->equipment.suit.id );
            if( suit->wingsuit )
            {
                setLock( false );
                getJumper()->enablePhase( true );
            }
            else
            {
                setMessage( Gameplay::iLanguage->getUnicodeString(538) );
            }
        }
    };
public:
    // script core
    Kjerag_RGBSlalom(Jumper* player) : Script( player )
    {
        // create routine
        _routine = new Disqualification( player );
    }
    virtual void onUpdateActivity(float dt)
    {
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {
            delete _routine;
            _routine = NULL;
        }
    }
}; }

void castingCallback_KjeragSmokeball_RGBSlalom01(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // exit point
    Enclosure* exitPoint = mission->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    std::vector<SmokeBallDesc> smokeballs;

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 2000.0f;
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 1050.0f;

    // cast blue smokeball 
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( 402966.3, 17037.2, -84710.0 );
    smokeballs.push_back( smokeBallDesc );

    // cast green smokeball 
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( 413899.6, 50471.1, -33701.9 );
    smokeballs.push_back( smokeBallDesc );

    // cast red smokeball 
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( 401765.3, 71406.1, -21030.7 );
    smokeballs.push_back( smokeBallDesc );

    // cast script
    new script::Kjerag_RGBSlalom( mission->getPlayer() );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 0.875f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );
    
    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_extinct.ogg" );
}

/**
 * RGB-Zigzag
 */

void castingCallback_KjeragSmokeball_RGBZigzag(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter object
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 0.0f;
    airplaneDesc.lastAltitude  = 0.0f;
    airplaneDesc.loweringSpeed = 0.0f;
    airplaneDesc.initOffset.set( -66266, 95000, -237612 );
    airplaneDesc.initDirection.set( 1.0, 0.0, 0.0 );
    airplaneDesc.initDirection.normalize();
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    std::vector<SmokeBallDesc> smokeballs;

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 2000.0f;
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 1050.0f;

    // cast blue smokeball 
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( -39150.6, 24209.9, -140857.4 );
    smokeballs.push_back( smokeBallDesc );

    // cast green smokeball 
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -59454.6, 46528.6, -185446.0 );
    smokeballs.push_back( smokeBallDesc );

    // cast red smokeball 
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -64046.3, 62328.7, -212124.9 );
    smokeballs.push_back( smokeBallDesc );

    // cast script
    new script::Kjerag_RGBSlalom( mission->getPlayer() );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 1.0f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );
    
    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_extinct.ogg" );
}