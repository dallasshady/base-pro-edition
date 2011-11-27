
#include "headers.h"
#include "cave.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "script.h"
#include "equip.h"
#include "ostankino.h"
#include "kvly.h"

/**
 * clearance functions
 */

bool weatherClearanceKVLY(WeatherType weatherType)
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

bool windClearanceKVLY(Vector3f dir, float ambient, float blast)
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

bool equipCallback_KVLY(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

bool equipCallback_KVLY_Wings(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // equip best wings
    if( !equipBestWingsuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

/**
 * freejump
 */

void castingCallback_KVLY_Underground(Actor* parent)
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
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 1.0f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * interval jump
 */

namespace script { class KVLY_IntervalJumps : public Script
{
private:
    NPCL _npcs;
public:
    // script core
    KVLY_IntervalJumps(Jumper* player, NPCL& npcs) : Script( player )
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

void castingCallback_KVLY_IntervalJumps(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/ij01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/ij02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/ij03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // load fourth ghost & cast NPC
    CatToy* catToy04 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/ij04.cattoy" ); assert( catToy04 );
    NPC* ghost04 = ::castGhostNPC( mission, exitPoint, catToy04, npcs ); assert( ghost04 );

    // cast script
    NPCL ghosts;
    ghosts.push_back( ghost01 );
    ghosts.push_back( ghost02 );
    ghosts.push_back( ghost03 );
    ghosts.push_back( ghost04 );
    new script::KVLY_IntervalJumps( mission->getPlayer(), ghosts );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 1.5f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_extinct.ogg" );
}

/**
 * interval jump
 */

namespace script { class KVLY_3way : public Script
{
public:
    // script core
    KVLY_3way(Jumper* player, CatToy* leaderCatToy) : Script( player )
    {
        // create routine
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

void castingCallback_KVLY_3way(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/3way01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/3way02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // cast script
    new script::KVLY_3way( mission->getPlayer(), catToy01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 1.75f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_extinct.ogg" );
}

/**
 * interval jump
 */

namespace script { class KVLY_6way : public Script
{
public:
    // script core
    KVLY_6way(Jumper* player, CatToy* leaderCatToy) : Script( player )
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

void castingCallback_KVLY_6way(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/6way01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/6way02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/6way03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // load fourth ghost & cast NPC
    CatToy* catToy04 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/6way04.cattoy" ); assert( catToy04 );
    NPC* ghost04 = ::castGhostNPC( mission, exitPoint, catToy04, npcs ); assert( ghost04 );

    // load fifth ghost & cast NPC
    CatToy* catToy05 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/6way05.cattoy" ); assert( catToy05 );
    NPC* ghost05 = ::castGhostNPC( mission, exitPoint, catToy05, npcs ); assert( ghost05 );

    // cast script
    new script::KVLY_6way( mission->getPlayer(), catToy01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 2.0f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_extinct.ogg" );
}

/**
 * 6-way with wingsuits
 */

namespace script { class KVLY_6wayWings : public Script
{
public:
    // script core
    KVLY_6wayWings(Jumper* player, CatToy* leaderCatToy) : Script( player )
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

void castingCallback_KVLY_6wayWings(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/6wings01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/6wings02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/6wings03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // load fourth ghost & cast NPC
    CatToy* catToy04 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/6wings04.cattoy" ); assert( catToy04 );
    NPC* ghost04 = ::castGhostNPC( mission, exitPoint, catToy04, npcs ); assert( ghost04 );

    // load fifth ghost & cast NPC
    CatToy* catToy05 = CatToy::loadGhostCatToy( "./usr/cattoys/tvboogie/6wings05.cattoy" ); assert( catToy05 );
    NPC* ghost05 = ::castGhostNPC( mission, exitPoint, catToy05, npcs ); assert( ghost05 );

    // cast script
    new script::KVLY_6wayWings( mission->getPlayer(), catToy01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 2.0f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_extinct.ogg" );
}