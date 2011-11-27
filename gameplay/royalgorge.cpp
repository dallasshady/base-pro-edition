
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

bool weatherClearanceRoyalGorge(WeatherType weatherType)
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

bool windClearanceRoyalGorge(Vector3f dir, float ambient, float blast)
{
    // wind doesn't matters
    return true;
}

/**
 * equipment callback
 */

bool equipCallback_RoyalGorge(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider up and 38' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 4;

    return true;
}

/**
 * go fall games : interval jumping
 */

namespace script { class GoFall_IntervalJumping_Script : public Script
{
private:
    NPCL _npcs;
public:
    // script core
    GoFall_IntervalJumping_Script(Jumper* player, NPCL& npcs) : Script( player )
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

void castingCallback_GoFall_IntervalJumping(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/gofall/ij01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/gofall/ij02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load third ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/gofall/ij03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // cast script
    NPCL ghosts;
    ghosts.push_back( ghost01 );
    ghosts.push_back( ghost02 );
    ghosts.push_back( ghost03 );
    // ghosts.push_back( ghost01 );    
    new script::GoFall_IntervalJumping_Script( mission->getPlayer(), ghosts );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 0.75f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}

/**
 * bridge proxy
 */

const float captureTimeout = 0.125f;

class GoalBridgeProxy : public Goal
{
private:
    bool                      _captureIsActive;
    Vector3f                  _prevPos;
    engine::IAtomic*          _overBridgeTrigger;
    engine::IAtomic*          _underBridgeTrigger;
    bool                      _overBridge;
    bool                      _underBridge;    
    Sensor*                   _sensor;
    float                     _captureTimeout;
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void)
    {
        return Gameplay::iLanguage->getUnicodeString(601);
    }
    virtual const wchar_t* getGoalValue(void)
    {
        if( _overBridge )
        {
            return Gameplay::iLanguage->getUnicodeString(602);
        }
        else if( _underBridge )
        {
            return Gameplay::iLanguage->getUnicodeString(603);
        }
        else
        {
            return Gameplay::iLanguage->getUnicodeString(604);
        }
    }
    virtual float getGoalScore(void)
    {
        if( _overBridge )
        {
            return 1.25f * _player->getVirtues()->getMaximalBonusScore();
        }
        else if( _underBridge )
        {
            return 0.75f * _player->getVirtues()->getMaximalBonusScore();
        }
        else
        {
            return 0;
        }
    }
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt)
    {
        // base behaviour
        Goal::onUpdateActivity( dt );

        // perform constant time step behaviour
        _captureTimeout -= dt;
        if( _captureTimeout < 0 )
        {
            _captureTimeout = captureTimeout;
            if( !_captureIsActive )
            {
                if( _player->getPhase() != ::jpRoaming )
                {
                    _captureIsActive = true;
                    _prevPos = _player->getClump()->getFrame()->getPos();
                }
            }
            else if( _player->getPhase() == ::jpFreeFalling ||
                     _player->getCanopySimulator()->getInflation() < 0.25f )
            {
                Vector3f currPos = _player->getClump()->getFrame()->getPos();
                if( !_overBridge )
                {
                    _sensor->sense( _prevPos, currPos - _prevPos, _overBridgeTrigger );
                    if( _sensor->getNumIntersections() ) _overBridge = true;
                    _sensor->sense( currPos, _prevPos - currPos, _overBridgeTrigger );
                    if( _sensor->getNumIntersections() ) _overBridge = true;
                }
                if( !_underBridge )
                {
                    _sensor->sense( _prevPos, currPos - _prevPos, _underBridgeTrigger );
                    if( _sensor->getNumIntersections() ) _underBridge = true;
                    _sensor->sense( currPos, _prevPos - currPos, _underBridgeTrigger );
                    if( _sensor->getNumIntersections() ) _underBridge = true;
                }
                _prevPos = currPos;
            }
        }
    }
public:
    // class implementation
    GoalBridgeProxy(Jumper* player, engine::IAtomic* overBridgeTrigger, engine::IAtomic* underBridgeTrigger) : Goal( player )
    {
        _overBridgeTrigger = overBridgeTrigger;
        _underBridgeTrigger = underBridgeTrigger;
        _overBridge = false;
        _underBridge = false;
        _captureIsActive = false;
        _sensor = new Sensor;
        _captureTimeout = captureTimeout;
    }
    virtual ~GoalBridgeProxy()
    {
        // mission isn't aborted?
        if( _player->isOverActivity() )
        {
            Virtues* virtues = getScene()->getCareer()->getVirtues();
            assert( _finite( getGoalScore() ) );
            virtues->evolution.score += getGoalScore();
        }
        delete _sensor;
    }
};

void castingCallback_GoFall_BridgeProxy(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter object (cuted, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    //airplaneDesc.exitPointFrames.push_back( "HelicopterExit02" );
    //airplaneDesc.exitPointFrames.push_back( "HelicopterExit03" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 0.0f;
    airplaneDesc.lastAltitude  = 0.0f;
    airplaneDesc.loweringSpeed = 0.0f;
    airplaneDesc.initOffset.set( -109319, 77500, 169459 );
    airplaneDesc.initDirection.set( -0.8f, 0.0f, -0.6f );
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

    // search for triggers
    engine::IClump* triggers = mission->getScene()->getExtras( "Triggers" ); assert( triggers );
    engine::IAtomic* overBridgeTrigger = Gameplay::iEngine->getAtomic( triggers, Gameplay::iEngine->findFrame( triggers->getFrame(), "TriggerOverBridge" ) );
    engine::IAtomic* underBridgeTrigger = Gameplay::iEngine->getAtomic( triggers, Gameplay::iEngine->findFrame( triggers->getFrame(), "TriggerUnderBridge" ) );

    // create bridge proxy goal
    new GoalBridgeProxy( mission->getPlayer(), overBridgeTrigger, underBridgeTrigger );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}

bool equipCallback_GoFall_BridgeProxy(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
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

/**
 * go fall games : deployment order, 3-way
 */

namespace script { class GoFall_Leader4way_Script : public Script
{
public:
    // script core
    GoFall_Leader4way_Script(Jumper* player, NPC* npc) : Script( player )
    {
        // create routine
        _routine = new Script::Countdown( player, npc->getCatToy() );
        //_routine = new Script::Timer( player );
        //_routine = new Script::SeeYou( player, _npcs );
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

void castingCallback_GoFall_Leader4way(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/gofall/do01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/gofall/do02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load second ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/gofall/do03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // cast generic goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // cast delay accuracy goal
    GoalDelayAccuracyDescriptor desc;
    desc.minDelay = 3.5f;
    desc.minScore = -1.0f;
    desc.maxDelay = 4.0f;
    desc.maxScore = 2.0f;
    desc.interpolationSmoothing = 5.0f;
    new GoalDelayAccuracy( mission->getPlayer(), &desc );

    // cast script
    new script::GoFall_Leader4way_Script( mission->getPlayer(), ghost01 );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}

bool equipCallback_GoFall_Leader4way(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider up and 40' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 3;

    return true;
}

/**
 * last one 4-way
 */

namespace script { class GoFall_LastOne4way_Script : public Script
{
public:
    // script core
    GoFall_LastOne4way_Script(Jumper* player, NPC* npc) : Script( player )
    {
        // create routine
        _routine = new Script::Countdown( player, npc->getCatToy() );
        //_routine = new Script::Timer( player );
        //_routine = new Script::SeeYou( player, _npcs );
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

void castingCallback_GoFall_LastOne4way(Actor* parent)
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
    CatToy* catToy01 = CatToy::loadGhostCatToy( "./usr/cattoys/gofall/do01.cattoy" ); assert( catToy01 );
    NPC* ghost01 = ::castGhostNPC( mission, exitPoint, catToy01, npcs ); assert( ghost01 );

    // load second ghost & cast NPC
    CatToy* catToy02 = CatToy::loadGhostCatToy( "./usr/cattoys/gofall/do02.cattoy" ); assert( catToy02 );
    NPC* ghost02 = ::castGhostNPC( mission, exitPoint, catToy02, npcs ); assert( ghost02 );

    // load second ghost & cast NPC
    CatToy* catToy03 = CatToy::loadGhostCatToy( "./usr/cattoys/gofall/do03.cattoy" ); assert( catToy03 );
    NPC* ghost03 = ::castGhostNPC( mission, exitPoint, catToy03, npcs ); assert( ghost03 );

    // cast generic goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // cast delay accuracy goal
    GoalDelayAccuracyDescriptor desc;
    desc.minDelay = 0.45f;
    desc.minScore = 2.0f;
    desc.maxDelay = 1.0f;
    desc.maxScore = -1.0f;
    desc.interpolationSmoothing = 5.0f;
    new GoalDelayAccuracy( mission->getPlayer(), &desc );

    // cast script
    new script::GoFall_LastOne4way_Script( mission->getPlayer(), ghost01 );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}

bool equipCallback_GoFall_LastOne4way(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider down and 45' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soDown;
    career->getVirtues()->equipment.pilotchute   = 1;

    return true;
}

/**
 * go fall games : opening mission
 */

class GoalBridgeLanding : public Goal
{
private:
    float _playerAltitude;
private:
    // goal results
    //  - 0 is landed outside bridge
    //  - 1 is bad landing upon the  bridge
    //  - 2 is landing upon the  bridge
    unsigned int getGoalResult(void)
    {
        if( _player->isOverActivity() )
        {            
            float epsilon = fabs( _playerAltitude - 29803 );
            if( epsilon < 25.0f )
            {
                if( _player->isLanding() )
                {
                    return 2;
                }
                else if( _player->isBadLanding() )
                {
                    return 1;
                }
            }
        }
        return 0;
    }
protected:
    // Goal abstracts
    virtual const wchar_t* getGoalName(void)
    {
        return Gameplay::iLanguage->getUnicodeString(601);
    }
    virtual const wchar_t* getGoalValue(void)
    {
        switch( getGoalResult() )
        {
        case 0: return Gameplay::iLanguage->getUnicodeString(618);
        case 1: return Gameplay::iLanguage->getUnicodeString(617);
        case 2: return Gameplay::iLanguage->getUnicodeString(616);
        default: return L"";
        }
    }
    virtual float getGoalScore(void)
    {
        switch( getGoalResult() )
        {
        case 0: return 0;
        case 1: return _player->getVirtues()->getMaximalBonusScore() * 0.25f;
        case 2: return _player->getVirtues()->getMaximalBonusScore();
        default: return 0;
        }
    }
public:
    // Actor abstracts
    virtual void onUpdateActivity(float dt)
    {
        // base behaviour
        Goal::onUpdateActivity( dt );

        // detect altitude
        _playerAltitude = this->_player->getClump()->getFrame()->getPos()[1];
    }
public:
    // class implementation
    GoalBridgeLanding(Jumper* player) : Goal( player )
    {
        _playerAltitude = 0;        
    }
    virtual ~GoalBridgeLanding()
    {
        // mission isn't aborted?
        if( _player->isOverActivity() )
        {
            Virtues* virtues = getScene()->getCareer()->getVirtues();
            assert( _finite( getGoalScore() ) );
            virtues->evolution.score += getGoalScore();
        }
    }
};

void castingCallback_GoFall_Opening(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter object (cuted, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    //airplaneDesc.exitPointFrames.push_back( "HelicopterExit02" );
    //airplaneDesc.exitPointFrames.push_back( "HelicopterExit03" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 0.0f;
    airplaneDesc.lastAltitude  = 0.0f;
    airplaneDesc.loweringSpeed = 0.0f;
    airplaneDesc.initOffset.set( -78267, 55000, 134315 );
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
    new GoalBridgeLanding( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}

/**
 * royal gorge bridge : extreme tracking
 */

void castingCallback_GoFall_ExtremeTracking(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast helicopter object (cuted, single exit)
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
    airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
    //airplaneDesc.exitPointFrames.push_back( "HelicopterExit02" );
    //airplaneDesc.exitPointFrames.push_back( "HelicopterExit03" );
    airplaneDesc.animationSpeed = 0.75f;
    airplaneDesc.initAltitude  = 0.0f;
    airplaneDesc.lastAltitude  = 0.0f;
    airplaneDesc.loweringSpeed = 0.0f;
    airplaneDesc.initOffset.set( -83207, 60000, 119758 );
    airplaneDesc.initDirection.set( 0.3f, 0.0f, -0.3f );
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
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, getCore()->getRandToolkit()->getUniform( 0.75f, 1.0f ) );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}

/**
 * go fall games : flip competition
 */

void castingCallback_GoFall_FlipCompetition(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast generic goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // cast flip count goal
    new GoalFlipCount( mission->getPlayer(), mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}

/**
 * royal gorge bridge : underground jump
 */

void castingCallback_RoyalGorgeUnderground(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast generic goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 1.0f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_delinquent.ogg" );
}
