
#include "headers.h"
#include "cave.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "script.h"
#include "ostankino.h"
#include "equip.h"

/**
 * clearance functions
 */

bool weatherClearanceCave(WeatherType weatherType)
{
    // no restrictions
    return true;
}

bool windClearanceCave(Vector3f dir, float ambient, float blast)
{
    // no restrictions
    return true;
}

/**
 * common equip callback
 */

bool equipCallback_CaveBase(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, 0, 0 ) ) return false;

    // set slider up and 38' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 4;

    return true;
}

/**
 * interval jumping
 */

namespace script { class CaveBase_IntervalJumping_Script : public Script
{
private:
    NPCL _npcs;
public:
    // script core
    CaveBase_IntervalJumping_Script(Jumper* player, NPCL& npcs) : Script( player )
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

void castingCallback_CaveBase_IntervalJumping(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/cave/ij01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/cave/ij02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/cave/ij03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // load fourth ghost & cast NPC
    CatToy* catToy04 = CatToy::loadGhostCatToy( "./usr/cattoys/cave/ij04.cattoy" ); assert( catToy04 );
    NPC* ghost04 = ::castGhostNPC( mission, exitPoint, catToy04, npcs ); assert( ghost04 );

    // cast script
    NPCL ghosts;
    ghosts.push_back( ghost01 );
    ghosts.push_back( ghost02 );
    ghosts.push_back( ghost03 );
    ghosts.push_back( ghost04 );
    new script::CaveBase_IntervalJumping_Script( mission->getPlayer(), ghosts );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 0.85f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_human_seed.ogg" );
}

/**
 * 2-way
 */

namespace script { class CaveBase_2way_Script : public Script
{
private:
    NPC* _npc;
public:
    // script core
    CaveBase_2way_Script(Jumper* player, NPC* npc) : Script( player )
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

void castingCallback_CaveBase_2way(Actor* parent)
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

    // load ghost
    CatToy* catToy01 = NULL;
    switch( getCore()->getRandToolkit()->getUniformInt() % 2 )
    {
    case 0:
        catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/cave/2way01.cattoy" ); 
        break;
    case 1:
        catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/cave/2way02.cattoy" ); 
        break;
    };
    assert( catToy01 );

    // cast ghost NPC
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // cast script
    new script::CaveBase_2way_Script( mission->getPlayer(), ghost01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.125f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_human_seed.ogg" );
}

/**
 * 3-way
 */

namespace script { class CaveBase_3way_Script : public Script
{
private:
    NPC* _npc;
public:
    // script core
    CaveBase_3way_Script(Jumper* player, NPC* npc) : Script( player )
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

void castingCallback_CaveBase_3way(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/cave/3way01.cattoy" ); assert( catToy01 );    
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/cave/3way02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // cast script
    new script::CaveBase_3way_Script( mission->getPlayer(), ghost01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.25f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_human_seed.ogg" );
}

/**
 * cave freejumping
 */

void castingCallback_CaveBase_Freejumping(Actor* parent)
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
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_human_seed.ogg" );
}