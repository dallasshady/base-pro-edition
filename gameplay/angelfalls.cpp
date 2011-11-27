
#include "headers.h"
#include "cave.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "script.h"
#include "equip.h"
#include "ostankino.h"
#include "angelfalls.h"
#include "gameplay.h"

/**
 * clearance functions
 */

bool weatherClearanceAngelFalls(WeatherType weatherType)
{
    // can't jump during bad weather
    if( weatherType == wtLightRain || weatherType == wtHardRain || weatherType == wtThunder )
    {
        return false;
    }
    else
    {
        return true;
    }

}

bool windClearanceAngelFalls(Vector3f dir, float ambient, float blast)
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

bool equipCallback_AngelFalls(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;
    if( !equipBestSuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

bool equipCallback_AngelFalls_Wings(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
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

void castingCallback_AngelFallsFreeJump(Actor* parent)
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
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}

/**
 * angel falls boogie : interval jump
 */

namespace script { class AngelFalls_IntervalJump : public Script
{
private:
    NPCL _npcs;
public:
    // script core
    AngelFalls_IntervalJump(Jumper* player, NPCL& npcs) : Script( player )
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

void castingCallback_AngelFallsIntervalJump(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/ij01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/ij02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/ij03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // load fourth ghost & cast NPC
    CatToy* catToy04 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/ij04.cattoy" ); assert( catToy04 );
    NPC* ghost04 = ::castGhostNPC( mission, exitPoint, catToy04, npcs ); assert( ghost04 );

    // cast script
    NPCL ghosts;
    ghosts.push_back( ghost01 );
    ghosts.push_back( ghost02 );
    ghosts.push_back( ghost03 );
    ghosts.push_back( ghost04 );
    new script::AngelFalls_IntervalJump( mission->getPlayer(), ghosts );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.25f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}

/**
 * angel falls boogie : no wings
 */

namespace script { class AngelFalls_NoWings : public Script
{
private:
    Jumper* _player;
    CatToy* _leaderCatToy;
private:
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
    AngelFalls_NoWings(Jumper* player, CatToy* leaderCatToy) : Script( player )
    {
        _player = player;
        _leaderCatToy = leaderCatToy;

        // create routine
        // _routine = new Script::Timer( player );        
        _routine = new Disqualification( player );
    }
    virtual void onUpdateActivity(float dt)
    {
        // base behaviour
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {            
            if( routineIs(Disqualification) )
            {
                delete _routine;
                _routine = new Script::Countdown( _player, _leaderCatToy );
            }
            else
            {
                delete _routine;
                _routine = NULL;
            }
        }
    }
}; }

void castingCallback_AngelFallsNoWings(Actor* parent)
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
    selectNPCs( mission->getPlayer(), 0.25f, npcs );

    // load first ghost & cast NPC
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/nowings01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/nowings02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // in this mission can participate third character. sometimes.
    if( getCore()->getRandToolkit()->getUniform(0,1) > 0.33f )
    {
        CatToy* catToy03 = NULL;
        if( getCore()->getRandToolkit()->getUniform(0,1) > 0.5f )
        {
            catToy03 = CatToy::wrap( ghost01->getJumper() );
        }
        else
        {
            catToy03 = CatToy::wrap( ghost02->getJumper() );
        }
        NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );
    }

    // cast script
    new script::AngelFalls_NoWings( mission->getPlayer(), catToy02 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 2.5f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_i_degrade.ogg" );
}

/**
 * angel falls boogie : close and closer
 */

namespace script { class AngelFalls_CloseAndCloser : public Script
{
public:
    // script core
    AngelFalls_CloseAndCloser(Jumper* player, CatToy* leaderCatToy) : Script( player )
    {
        // create routine
        // _routine = new Script::Timer( player );
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

void castingCallback_AngelFallsCloseAndCloser(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 0.25f, 0.25f, 1.0f ), ::sjmLeft );

    // select NPCs
    std::vector<unsigned int> npcs;
    selectNPCs( mission->getPlayer(), 0.25f, npcs );

    // load first ghost & cast NPC
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/close01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/close02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // cast NPC smokejets
    new SmokeJet( ghost01->getJumper(), Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );
    new SmokeJet( ghost02->getJumper(), Vector4f( 0.25f, 0.25f, 1.0f, 1.0f ), ::sjmLeft );

    // cast script
    new script::AngelFalls_CloseAndCloser( mission->getPlayer(), catToy01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.75f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_tired_apart.ogg" );
}

/**
 * angel boogie : 6-way
 */

namespace script { class AngelFalls_6way : public Script
{
public:
    // script core
    AngelFalls_6way(Jumper* player, CatToy* leaderCatToy) : Script( player )
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

void castingCallback_AngelFalls6way(Actor* parent)
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
    selectNPCs( mission->getPlayer(), 0.5f, npcs );

    // load first ghost & cast NPC
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/6way01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/6way02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/6way03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // load fourth ghost & cast NPC
    CatToy* catToy04 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/6way04.cattoy" ); assert( catToy04 );
    NPC* ghost04 = ::castGhostNPC( mission, exitPoint, catToy04, npcs ); assert( ghost04 );

    // load fifth ghost & cast NPC
    CatToy* catToy05 = CatToy::loadGhostCatToy( "./usr/cattoys/angelfalls/6way05.cattoy" ); assert( catToy05 );
    NPC* ghost05 = ::castGhostNPC( mission, exitPoint, catToy05, npcs ); assert( ghost05 );

    // cast script
    new script::AngelFalls_6way( mission->getPlayer(), catToy01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.33f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}