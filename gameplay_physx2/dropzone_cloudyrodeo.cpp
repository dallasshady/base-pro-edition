
#include "headers.h"
#include "dropzone.h"
#include "airplane.h"
#include "hud.h"
#include "smokeball.h"
#include "script.h"
#include "equip.h"

/**
 * RGB tracking mission
 */

namespace script { class Dropzone_CloudyRodeo_RGB : public Script
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
    Dropzone_CloudyRodeo_RGB(Jumper* player) : Script( player )
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

void castingCallback_CloudyRodeo_RGB(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // setup audible altimeter
    Altimeter::setAudibleAltimeter( mission->getScene()->getCareer(), true, 70000.0f );

    // define mission epicenter
    const Vector3f epicenterRGB( -19348.0f, 0.0f, 18259.0f );

    // define spatial dispersion of smokeballs
    float sbDisp = 20000.0f;

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
    airplaneDesc.initOffset = epicenterRGB + Vector3f( 0, 400000, 0 );
    airplaneDesc.initDirection.set( 
        getCore()->getRandToolkit()->getUniform( -1, 1 ),
        0.0f,
        getCore()->getRandToolkit()->getUniform( -1, 1 )
    );
    airplaneDesc.initDirection.normalize();
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    std::vector<SmokeBallDesc> smokeballs;

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 3000.0f;    
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 1050.0f;

    // cast green smokeball 
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center = epicenterRGB + Vector3f(
        getCore()->getRandToolkit()->getUniform( -sbDisp, sbDisp ),
        300000.0f, 
        getCore()->getRandToolkit()->getUniform( -sbDisp, sbDisp ) 
    );
    smokeballs.push_back( smokeBallDesc );

    // cast blue smokeball 
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center = epicenterRGB + Vector3f(
        getCore()->getRandToolkit()->getUniform( -sbDisp, sbDisp ),
        200000.0f, 
        getCore()->getRandToolkit()->getUniform( -sbDisp, sbDisp ) 
    );
    smokeballs.push_back( smokeBallDesc );

    // cast red smokeball 
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center = epicenterRGB + Vector3f(
        getCore()->getRandToolkit()->getUniform( -sbDisp, sbDisp ),
        100000.0f, 
        getCore()->getRandToolkit()->getUniform( -sbDisp, sbDisp ) 
    );
    smokeballs.push_back( smokeBallDesc );

    // cast goals
    new GoalDropzone( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 0.5f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );
	new GoalFreeFallTime( mission->getPlayer() );
	new GoalCanopyTime( mission->getPlayer() );
	new GoalOpening( mission->getPlayer() );

    // cast script
    new script::Dropzone_CloudyRodeo_RGB( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_requiem.ogg" );
}

/**
 * RGB slalom mission
 */

namespace script { class Dropzone_CloudyRodeo_SlalomRGB : public Script
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
    Dropzone_CloudyRodeo_SlalomRGB(Jumper* player) : Script( player )
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

void castingCallback_CloudyRodeo_SlalomRGB(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // setup audible altimeter
    Altimeter::setAudibleAltimeter( mission->getScene()->getCareer(), true, 70000.0f );

    // define mission epicenter
    const Vector3f epicenterRGB( -19348.0f, 0.0f, 18259.0f );

    // define slalom direction
    Vector3f slalomDir = Vector3f(
        getCore()->getRandToolkit()->getUniform( -1, 1 ),
        0,
        getCore()->getRandToolkit()->getUniform( -1, 1 )
    );
    slalomDir.normalize();

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
    airplaneDesc.initOffset = epicenterRGB + Vector3f( 0, 400000, 0 );
    airplaneDesc.initDirection.set( 
        getCore()->getRandToolkit()->getUniform( -1, 1 ),
        0.0f,
        getCore()->getRandToolkit()->getUniform( -1, 1 )
    );
    airplaneDesc.initDirection.normalize();
    Airplane* airplane = new Airplane( mission, &airplaneDesc );

    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    std::vector<SmokeBallDesc> smokeballs;

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 3000.0f;    
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 1050.0f;

    // cast green smokeball 
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center = epicenterRGB + slalomDir * 50000.0f + Vector3f( 0, 300000.0f, 0 );
    smokeballs.push_back( smokeBallDesc );

    // cast blue smokeball 
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center = epicenterRGB - slalomDir * 45000.0f + Vector3f( 0, 200000.0f, 0 );
    smokeballs.push_back( smokeBallDesc );

    // cast red smokeball 
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center = epicenterRGB + slalomDir * 40000.0f + Vector3f( 0, 100000.0f, 0 );
    smokeballs.push_back( smokeBallDesc );

    // cast goals
    new GoalDropzone( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 0.75f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );
	new GoalFreeFallTime( mission->getPlayer() );
	new GoalOpening( mission->getPlayer() );
	new GoalCanopyTime( mission->getPlayer() );

    // cast script
    new script::Dropzone_CloudyRodeo_SlalomRGB( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_requiem.ogg" );
}

/**
 * RGB extreme mission
 */

namespace script { class Dropzone_CloudyRodeo_ExtremeRGB : public Script
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
            database::Canopy* canopy = database::Canopy::getRecord( getJumper()->getVirtues()->equipment.canopy.id );
            if( suit->wingsuit && !canopy->skydiving )
            {
                setLock( false );
                getJumper()->enablePhase( true );
            }
            else
            {
                setMessage( Gameplay::iLanguage->getUnicodeString(648) );
            }
        }
    };
public:
    // script core
    Dropzone_CloudyRodeo_ExtremeRGB(Jumper* player) : Script( player )
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

void castingCallback_CloudyRodeo_ExtremeRGB(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // cast airplane object
    AirplaneDesc airplaneDesc;
    airplaneDesc.templateClump = parent->getScene()->findClump( "Airplane01" ); assert( airplaneDesc.templateClump );
    airplaneDesc.propellerFrame = "PropellerSound";
    airplaneDesc.propellerSound = "./res/sounds/aircrafts/airplane.ogg";
    airplaneDesc.exitPointFrames.push_back( "LeftDoor01" );
    airplaneDesc.exitPointFrames.push_back( "RightDoor01" );
    airplaneDesc.exitPointFrames.push_back( "RightDoor02" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam1" );
    airplaneDesc.cameraFrames.push_back( "Cesna_Cam2" );
    airplaneDesc.animationSpeed = 0.5f;
    airplaneDesc.initAltitude  = 200000;
    airplaneDesc.lastAltitude  = 5000.0f;
    airplaneDesc.loweringSpeed = 500.0f;
	airplaneDesc.fixedWing = true;
    Airplane* airplane = new Airplane( mission, &airplaneDesc );
   
    // cast player on airplane
    mission->setPlayer( new Jumper( mission, airplane, NULL, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    std::vector<SmokeBallDesc> smokeballs;

    // define smokeball properties
    SmokeBallDesc smokeBallDesc;    
    smokeBallDesc.radius = 3000.0f;    
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 1050.0f;

    // cast goals
    new GoalLanding( mission->getPlayer() );
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
	new GoalOpening( mission->getPlayer() );
	new GoalFreeFallTime( mission->getPlayer() );
	new GoalCanopyTime( mission->getPlayer() );

    // cast green smokeball 
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( 49380,12932,53780 );
    smokeballs.clear();
    smokeballs.push_back( smokeBallDesc );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 1.5f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // cast blue smokeball 
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( 63847,17567,103629 );
    smokeballs.clear();
    smokeballs.push_back( smokeBallDesc );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 2.0f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // cast red smokeball 
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( 65144,13634,97431 );
    smokeballs.clear();
    smokeballs.push_back( smokeBallDesc );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 3.0f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // cast script
    new script::Dropzone_CloudyRodeo_ExtremeRGB( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_requiem.ogg" );
}

bool equipCallback_CloudyRodeo_ExtremeRGB(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
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