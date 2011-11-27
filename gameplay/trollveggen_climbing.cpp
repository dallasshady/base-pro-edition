
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
 * common equip callback
 */

bool equipCallback_TrollClimbing(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // equip best wingsuit if available
    equipBestWingsuit( career, windAmbient, windBlast );

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

/**
 * common script
 */

namespace script { class TrollClimbing_Script : public Script
{
private:
    NPC* _npc;
private:
    // routines
    class Intro01 : public Message
    {
    public:
        Intro01(Jumper* jumper) : Message( jumper, Gameplay::iLanguage->getUnicodeString(575), 10.0f ) {}
    };
    class Intro02 : public Message
    {
    public:
        Intro02(Jumper* jumper, NPC* npc) : 
            Message(
                jumper,
                wstrformat( Gameplay::iLanguage->getUnicodeString(552), npc->getNPCName() ).c_str(), 
                10.0f 
            ) 
        {}
    };
public:
    // script core
    TrollClimbing_Script(Jumper* player, NPC* npc) : Script( player )
    {
        _npc = npc;

        // create routine
        _routine = new Intro01( player );
    }
    virtual void onUpdateActivity(float dt)
    {
        // base behaviour
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {
            if( routineIs( Intro01 ) )
            {
                delete _routine;
                _routine = new Intro02( _jumper, _npc );
            }
            else if( routineIs( Intro02 ) )
            {
                delete _routine;
                _routine = new Script::Countdown( _jumper, _npc->getCatToy() );
            }
            else
            {       
                delete _routine;
                _routine = NULL;
            }
        }
    }
}; }

/**
 * base climbing - trollveggen route
 */

static float climbingRewardRates[11] = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 250.0f, 325.0f, 400.0f, 475.0f, 550.0f, 625.0f
};

void castingCallback_TrollClimbing_TrollveggenRoute(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // load ghost
    CatToy* catToy = CatToy::loadGhostCatToy( "./usr/cattoys/trollclimbing/2way01.cattoy" ); assert( catToy );

    // cast LICENSED_CHAR as ghost NPC
    unsigned int npcId = database::NPCInfo::getLicensedCharacterId();
    if( mission->getScene()->getCareer()->getLicensedFlag() ) 
    {
        npcId = database::NPCInfo::getRandomNonLicensedCharacter( 
            mission->getScene()->getCareer()->getVirtues()->getSkillLevel(), 0.25f
        );
    }
    NPC* joeBlack = new NPC( mission, npcId, NULL, exitPoint, catToy );
    joeBlack->getJumper()->setSignatureType( stBrief );
    joeBlack->setProgram( new NPCCameraman( joeBlack ) );    

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );

    // cast NPC's smokejet
    new SmokeJet( joeBlack->getJumper(), Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );

    // cast script
    new script::TrollClimbing_Script( mission->getPlayer(), joeBlack );

    // footage goal descriptor
    GoalFootageDescriptor footageDesc;
    footageDesc.bad        = GoalFootageDescriptor::RangeMark( 200.0f, 0.0f );
    footageDesc.poor       = GoalFootageDescriptor::RangeMark( 100.0f, 0.01f );
    footageDesc.tolerable  = GoalFootageDescriptor::RangeMark( 50.0f, 0.05f );
    footageDesc.good       = GoalFootageDescriptor::RangeMark( 25.0f, 0.125f );
    footageDesc.fine       = GoalFootageDescriptor::RangeMark( 0.0f, 0.25f );
    footageDesc.timeFactor = 1.0f;

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalFootage( mission->getPlayer(), joeBlack, &footageDesc );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.0f );

    // reward goal
    /*
    bool reward = ( getCore()->getRandToolkit()->getUniform(0.0f,1.0f) < 0.75f );
    if( reward ) 
    {
        GoalRewardDescriptor rewardDescriptor( climbingRewardRates );
        new GoalReward( mission->getPlayer(), &rewardDescriptor );
    }*/

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_tired_apart.ogg" );
}

/**
 * base climbing - trollryggen route
 */

void castingCallback_TrollClimbing_TrollryggenRoute(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // load ghost
    CatToy* catToy = CatToy::loadGhostCatToy( "./usr/cattoys/trollclimbing/2way02.cattoy" ); assert( catToy );

    // cast LICENSED_CHAR as ghost NPC
    unsigned int npcId = database::NPCInfo::getLicensedCharacterId();
    if( mission->getScene()->getCareer()->getLicensedFlag() ) 
    {
        npcId = database::NPCInfo::getRandomNonLicensedCharacter( 
            mission->getScene()->getCareer()->getVirtues()->getSkillLevel(), 0.25f
        );
    }
    NPC* joeBlack = new NPC( mission, npcId, NULL, exitPoint, catToy );
    joeBlack->getJumper()->setSignatureType( stBrief );
    joeBlack->setProgram( new NPCCameraman( joeBlack ) );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );

    // cast NPC's smokejet
    new SmokeJet( joeBlack->getJumper(), Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );

    // cast script
    new script::TrollClimbing_Script( mission->getPlayer(), joeBlack );

    // footage goal descriptor
    GoalFootageDescriptor footageDesc;
    footageDesc.bad        = GoalFootageDescriptor::RangeMark( 300.0f, 0.0f );
    footageDesc.poor       = GoalFootageDescriptor::RangeMark( 170.0f, 0.01f );
    footageDesc.tolerable  = GoalFootageDescriptor::RangeMark( 75.0f, 0.05f );
    footageDesc.good       = GoalFootageDescriptor::RangeMark( 37.5f, 0.125f );
    footageDesc.fine       = GoalFootageDescriptor::RangeMark( 0.0f, 0.25f );
    footageDesc.timeFactor = 1.0f;

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalFootage( mission->getPlayer(), joeBlack, &footageDesc );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.0f );

    // reward goal
    /*
    bool reward = ( getCore()->getRandToolkit()->getUniform(0.0f,1.0f) < 0.75f );
    if( reward ) 
    {
        GoalRewardDescriptor rewardDescriptor( climbingRewardRates );
        new GoalReward( mission->getPlayer(), &rewardDescriptor );
    }*/

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_tired_apart.ogg" );
}


/**
 * 2-way from Troll
 */

void castingCallback_TrollClimbing_TrollRoute(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // load ghost
    CatToy* catToy = CatToy::loadGhostCatToy( "./usr/cattoys/trollclimbing/2way03.cattoy" ); assert( catToy );

    // cast LICENSED_CHAR as ghost NPC
    unsigned int npcId = database::NPCInfo::getLicensedCharacterId();
    if( mission->getScene()->getCareer()->getLicensedFlag() ) 
    {
        npcId = database::NPCInfo::getRandomNonLicensedCharacter( 
            mission->getScene()->getCareer()->getVirtues()->getSkillLevel(), 0.25f
        );
    }
    NPC* joeBlack = new NPC( mission, npcId, NULL, exitPoint, catToy );
    joeBlack->getJumper()->setSignatureType( stBrief );
    joeBlack->setProgram( new NPCCameraman( joeBlack ) );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );

    // cast NPC's smokejet
    new SmokeJet( joeBlack->getJumper(), Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );

    // cast script
    new script::TrollClimbing_Script( mission->getPlayer(), joeBlack );

    // footage goal descriptor
    GoalFootageDescriptor footageDesc;
    footageDesc.bad        = GoalFootageDescriptor::RangeMark( 300.0f, 0.0f );
    footageDesc.poor       = GoalFootageDescriptor::RangeMark( 170.0f, 0.01f );
    footageDesc.tolerable  = GoalFootageDescriptor::RangeMark( 75.0f, 0.05f );
    footageDesc.good       = GoalFootageDescriptor::RangeMark( 37.5f, 0.125f );
    footageDesc.fine       = GoalFootageDescriptor::RangeMark( 0.0f, 0.25f );
    footageDesc.timeFactor = 1.0f;

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalFootage( mission->getPlayer(), joeBlack, &footageDesc );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.0f );

    // reward goal
    /*
    bool reward = ( getCore()->getRandToolkit()->getUniform(0.0f,1.0f) < 0.75f );
    if( reward ) 
    {
        GoalRewardDescriptor rewardDescriptor( climbingRewardRates );
        new GoalReward( mission->getPlayer(), &rewardDescriptor );
    }*/

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_tired_apart.ogg" );
}