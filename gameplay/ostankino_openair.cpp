
#include "headers.h"
#include "ostankino.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "script.h"
#include "smokejet.h"
#include "equip.h"

/**
 * clearance functions
 */

bool weatherClearanceOpenAir(WeatherType weatherType)
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

bool windClearanceOpenAir(Vector3f dir, float ambient, float blast)
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
 * equip callbacks
 */

bool equipCallback_OpenAir_CommonMission(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

bool equipCallback_OpenAir_WingsuitMission(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // equip best wingsuit
    if( !equipBestWingsuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

bool equipCallback_OpenAir_HardLanding(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider down and 42' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soDown;
    career->getVirtues()->equipment.pilotchute   = 2;

    return true;
}

/**
 * common methods
 */

NPC* castCameramanNPC(Mission* mission, Enclosure* enclosure, Jumper* player, std::vector<unsigned int>& npcs)
{
    assert( npcs.size() );

    if( npcs.size() )
    {
        // select NPC
        unsigned int index = getCore()->getRandToolkit()->getUniformInt() % npcs.size();
        unsigned int npcId = npcs[index];
        npcs.erase( &npcs[index] );
        // create NPC
        NPC* npc = new NPC( mission, npcId, NULL, enclosure, CatToy::wrap( player ) );
        // setup cameraman behaviour
        npc->setProgram( new NPCCameraman( npc ) );
        // setup brief signature
        npc->getJumper()->setSignatureType( stBrief );
        return npc;
    }

    return NULL;
}

NPC* castGhostNPC(Mission* mission, Enclosure* enclosure, CatToy* ghost, unsigned int npcId)
{
    // create NPC
    NPC* npc = new NPC( mission, npcId, NULL, enclosure, ghost );
    // setup cameraman behaviour
    npc->setProgram( new NPCCameraman( npc ) );
    // setup brief signature
    npc->getJumper()->setSignatureType( stBrief );
    return npc;
}

NPC* castGhostNPC(Mission* mission, Enclosure* enclosure, CatToy* ghost, std::vector<unsigned int>& npcs)
{
    assert( ghost );
    assert( npcs.size() );

    if( npcs.size() )
    {
        // select NPC
        unsigned int index = getCore()->getRandToolkit()->getUniformInt() % npcs.size();
        unsigned int npcId = npcs[index];
        npcs.erase( &npcs[index] );
        // cast NPC
        return castGhostNPC( mission, enclosure, ghost, npcId );
    }

    return NULL;
}

/**
 * opening startup
 */

namespace script { class OpenAir_Opening_Script : public Script
{
private:
    CatToy* _countDownCattoy;
public:
    // script routines
    class Intro01 : public Message
    {
    public:
        Intro01(Jumper* jumper) : Message( jumper, Gameplay::iLanguage->getUnicodeString(526), 10.0f ) {}
    };
    class Intro02 : public Message
    {
    public:
        Intro02(Jumper* jumper) : Message( jumper, Gameplay::iLanguage->getUnicodeString(527), 10.0f ) {}
    };
public:
    // script core
    OpenAir_Opening_Script(Jumper* player, CatToy* countDownCattoy) : Script( player )
    {
        assert( countDownCattoy );
        _countDownCattoy = countDownCattoy;

        // create routine
        _routine = new Intro01( player );
    }
    virtual void onUpdateActivity(float dt)
    {
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {
            if( routineIs(Intro01) )
            {
                delete _routine;
                _routine = new Intro02( _jumper );
            }
            else if( routineIs(Intro02) )
            {
                delete _routine;
                _routine = new Countdown( _jumper, _countDownCattoy );
            }            
            else
            {
                delete _routine;
                _routine = NULL;
            }
        }
    }
}; }

void castingCallback_OpenAir_Opening(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // technical platform
    Enclosure* technicalPlatform = parent->getScene()->getExitPointEnclosure( 0 );

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ), sjmRight );

    // select NPCs
    std::vector<unsigned int> npcs;
    selectNPCs( mission->getPlayer(), 0.26f, npcs );

    // cast cameraman NPC for player
    NPC* cameraman = castCameramanNPC( mission, exitPoint, mission->getPlayer(), npcs );
    assert( cameraman );
    if( cameraman )
    {
        new SmokeJet( cameraman->getJumper(), Vector4f( 0.25f, 0.25f, 1.0f, 1.0f ), sjmRight );
    }

    // load ghost
    CatToy* catToy = CatToy::loadGhostCatToy( "./usr/cattoys/openair/opening01.cattoy" ); assert( catToy );

    // cast ghost NPC at technical platform
    NPC* ghost = ::castGhostNPC( mission, technicalPlatform, catToy, npcs );
    assert( ghost );
    if( ghost )
    {
        new SmokeJet( ghost->getJumper(), Vector4f( 1.0f, 0.25f, 0.25f, 1.0f ), sjmRight );
    }

    // cast ghost's cameraman
    NPC* ghostCameraman = ::castCameramanNPC( mission, technicalPlatform, ghost->getJumper(), npcs );
    assert( ghostCameraman );
    if( ghostCameraman )
    {
        new SmokeJet( ghostCameraman->getJumper(), Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ), sjmRight );
    }

    // cast script
    new script::OpenAir_Opening_Script( mission->getPlayer(), catToy );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.0f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}

/**
 * landing accuracy
 */

void castingCallback_OpenAir_LandingAccuracy(Actor* parent)
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
    new GoalLandingAccuracy( mission->getPlayer(), Vector3f( 18984,10,-4156 ), 25.0f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}

/**
 * tracking performance
 */

namespace script { class OpenAir_TrackingPerformance_Script : public Script
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
    OpenAir_TrackingPerformance_Script(Jumper* player) : Script( player )
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

void castingCallback_OpenAir_TrackingPerformance(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );

    // cast script
    new script::OpenAir_TrackingPerformance_Script( mission->getPlayer() );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalTrackingPerformance( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}

/**
 * spiral freefall
 */

namespace script { class OpenAir_SpiralFreefall_Script : public OpenAir_TrackingPerformance_Script
{
public:
    // script core
    OpenAir_SpiralFreefall_Script(Jumper* player) : OpenAir_TrackingPerformance_Script( player )
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

void castingCallback_OpenAir_SpiralFreefall(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 1.0f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );

    // cast script
    new script::OpenAir_TrackingPerformance_Script( mission->getPlayer() );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalSpiral( mission->getPlayer(), Vector3f( 0.0f, 0.0f, -1464 ) );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}

/**
 * pike of thrills
 */

void castingCallback_OpenAir_PikeOfThrills(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast player smokejet
    new SmokeJet( mission->getPlayer(), Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ), ::sjmLeft );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 2.0f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}

/**
 * hard landing
 */

void castingCallback_OpenAir_HardLanding(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // choose prize
    Gear prize;
    prize.state = 1.0f;
    unsigned int numOfAttempts = 0;
    do
    {
        switch( getCore()->getRandToolkit()->getUniformInt() % 4 )
        {
        case 0: 
            prize.type = gtHelmet;
            prize.id   = getCore()->getRandToolkit()->getUniformInt() % database::Helmet::getNumRecords();
            assert( prize.id < database::Helmet::getNumRecords() );
            break;
        case 1: 
            prize.type = gtSuit; 
            prize.id   = getCore()->getRandToolkit()->getUniformInt() % database::Suit::getNumRecords();
            assert( prize.id < database::Suit::getNumRecords() );
            break;
        case 2: 
            prize.type = gtRig; 
            prize.id   = getCore()->getRandToolkit()->getUniformInt() % database::Rig::getNumRecords();
            assert( prize.id < database::Rig::getNumRecords() );
            break;
        case 3: 
            prize.type = gtCanopy; 
            prize.id   = getCore()->getRandToolkit()->getUniformInt() % database::Canopy::getNumRecords();
            assert( prize.id < database::Canopy::getNumRecords() );
            break;
        default:
            assert( !"shouldn't be here!" );
        }
        numOfAttempts++;
        if( numOfAttempts >= 100 ) break;
    }
    while( !prize.isTradeable() );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // cast hard landing goal
    switch( getCore()->getRandToolkit()->getUniformInt() % 4 )
    {
    case 0:
        new GoalHardLanding( mission->getPlayer(), Vector3f( 54729.2f, 12.5f, 710.7f ), 25, prize );
        break;
    case 1:
        new GoalHardLanding( mission->getPlayer(), Vector3f( -20161.7f, 5270.0f, 14244.7f ), 15, prize );
        break;
    case 2:
        new GoalHardLanding( mission->getPlayer(), Vector3f( -33753.8f, 9175.5f, -41883.1f ), 25, prize );
        break;
    case 3:
        new GoalHardLanding( mission->getPlayer(), Vector3f( 29874.7f, 1350.1f, 15392.7f ), 25, prize );
        break;
    }

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}

/**
 * cameraman mission
 */

namespace script { class OpenAir_Cameraman_Script : public Script
{
private:
    NPC* _npc;
private:
    // routines
    class Intro : public Message
    {
    public:
        Intro(Jumper* jumper, NPC* npc) : 
            Message( 
                jumper, 
                wstrformat( Gameplay::iLanguage->getUnicodeString(552), npc->getNPCName() ).c_str(),
                10.0f 
            ) 
        {}
    };    
public:
    // script core
    OpenAir_Cameraman_Script(Jumper* player, NPC* npc) : Script( player )
    {
        _npc = npc;

        // create routine
        _routine = new Intro( player, npc );
    }
    virtual void onUpdateActivity(float dt)
    {
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {
            if( routineIs(Intro) )
            {
                delete _routine;
                _routine = new Countdown( _jumper, _npc->getCatToy() );
            }
            else
            {
                delete _routine;
                _routine = NULL;
            }
        }
    }
}; }

void castingCallback_OpenAir_Cameraman(Actor* parent)
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
    CatToy* catToy = CatToy::loadGhostCatToy( "./usr/cattoys/openair/cameraman01.cattoy" ); assert( catToy );

    // cast ghost NPC
    NPC* ghost = ::castGhostNPC( mission, exitPoint, catToy, npcs ); assert( ghost );

    // cast script
    new script::OpenAir_Cameraman_Script( mission->getPlayer(), ghost );

    // footage goal descriptor
    GoalFootageDescriptor footageDesc;
    footageDesc.bad        = GoalFootageDescriptor::RangeMark( 50.0f, 0.0f );
    footageDesc.poor       = GoalFootageDescriptor::RangeMark( 25.0f, 0.01f );
    footageDesc.tolerable  = GoalFootageDescriptor::RangeMark( 12.5f, 0.05f );
    footageDesc.good       = GoalFootageDescriptor::RangeMark( 6.25f, 0.125f );
    footageDesc.fine       = GoalFootageDescriptor::RangeMark( 0.0f,  0.5f );
    footageDesc.timeFactor = 1.0f;

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalFootage( mission->getPlayer(), ghost, &footageDesc );

    // play no music for this mission
    Gameplay::iGameplay->stopSoundtrack();
}

/**
 * 6-way mission
 */

namespace script { class OpenAir_6way_Script : public Script
{
private:
    NPC* _npc;
private:
    // routines
    class Intro : public Message
    {
    public:
        Intro(Jumper* jumper) : 
            Message( 
                jumper, 
                Gameplay::iLanguage->getUnicodeString(561),
                10.0f 
            ) 
        {}
    };    
public:
    // script core
    OpenAir_6way_Script(Jumper* player, NPC* npc) : Script( player )
    {
        _npc = npc;

        // create routine
        _routine = new Intro( player );
    }
    virtual void onUpdateActivity(float dt)
    {
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {
            if( routineIs(Intro) )
            {
                delete _routine;
                _routine = new Countdown( _jumper, _npc->getCatToy() );
            }
            else
            {
                delete _routine;
                _routine = NULL;
            }
        }
    }
}; }

void castingCallback_OpenAir_6way(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // observation platform
    Enclosure* observationPlatform = parent->getScene()->getExitPointEnclosure( 1 );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // select NPCs
    std::vector<unsigned int> npcs;
    selectNPCs( mission->getPlayer(), 0.51f, npcs );

    // load first ghost
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/openair/6way01.cattoy" ); assert( catToy01 );

    // cast ghost NPC
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );    

    // cast cameraman NPC for ghost
    NPC* ghost01cameraman = ::castCameramanNPC( mission, exitPoint, ghost01->getJumper(), npcs ); assert( ghost01cameraman );

    // load second ghost
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/openair/6way02.cattoy" ); assert( catToy02 );

    // cast second ghost NPC
    NPC* ghost02 = ::castGhostNPC( mission, observationPlatform, catToy02, npcs ); assert( ghost02 );

    // cast cameraman NPC for second ghost
    NPC* ghost02cameraman01 = ::castCameramanNPC( mission, observationPlatform, ghost02->getJumper(), npcs ); assert( ghost02cameraman01 );

    // cast cameraman NPC for second ghost's cameraman ;)
    NPC* ghost02cameraman02 = ::castCameramanNPC( mission, observationPlatform, ghost02cameraman01->getJumper(), npcs ); assert( ghost02cameraman02 );

    // cast script
    new script::OpenAir_6way_Script( mission->getPlayer(), ghost01 );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.0f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}

/**
 * flip count mission
 */

void castingCallback_OpenAir_FlipCount(Actor* parent)
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
    new GoalFlipCount( mission->getPlayer(), mission->getPlayer()->getVirtues()->getMaximalBonusScore() * 0.75f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}

bool equipCallback_OpenAir_FlipCount(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // equip best suit
    if( !equipBestSuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 38' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 4;

    return true;
}

/**
 * RGB-zigzag mission
 */

namespace script { class OpenAir_RGB_Script : public Script
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
    OpenAir_RGB_Script(Jumper* player) : Script( player )
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

void castingCallback_OpenAir_RGB_Zigzag(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // smokeballs
    std::vector<SmokeBallDesc> smokeballs;

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 1500.0f;
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 750.0f;

    // green smokeball 
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( 2332.6f, 41989.6f, -4030.4f );
    smokeballs.push_back( smokeBallDesc );

    // blue smokeball 
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( 11476.3f, 28769.4f, -9162.0f );
    smokeballs.push_back( smokeBallDesc );

    // cast red smokeball 
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( 26914.6f, 15546.4f, -2507.3f );
    smokeballs.push_back( smokeBallDesc );

    // cast goals
    new GoalLanding( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 0.75f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // cast script
    new script::OpenAir_RGB_Script( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}

/**
 * RGB-spiral mission
 */

void castingCallback_OpenAir_RGB_Spiral(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // smokeballs
    std::vector<SmokeBallDesc> smokeballs;

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 1500.0f;
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 750.0f;

    // green smokeball 
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -621.8f, 42222.8f, -3112.1f );
    smokeballs.push_back( smokeBallDesc );

    // blue smokeball 
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( 3121.9f, 27289.0f, -2318.4f );
    smokeballs.push_back( smokeBallDesc );

    // cast red smokeball 
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -141.1f, 16183.9f, 4816.4f );
    smokeballs.push_back( smokeBallDesc );

    // cast goals
    new GoalLanding( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 1.0f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // cast script
    new script::OpenAir_RGB_Script( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}

/**
 * RGB-extreme mission
 */

void castingCallback_OpenAir_RGB_Extreme(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast goals
    new GoalLanding( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 1500.0f;
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 750.0f;

    std::vector<SmokeBallDesc> smokeballs;

    // cast red smokeball
    smokeballs.clear();
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( 46684.0f, 9756.8f, -10375.8f );
    smokeballs.push_back( smokeBallDesc );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 3.0f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // cast blue smokeball
    smokeballs.clear();
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( -22803.3f, 12154.4f, -37947.5f );
    smokeballs.push_back( smokeBallDesc );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 2.5f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // cast green smokeball
    smokeballs.clear();
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -19535.5f, 14932.5f, 14405.6f );
    smokeballs.push_back( smokeBallDesc );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 1.5f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // cast script
    new script::OpenAir_RGB_Script( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/lot of lie.ogg" );
}