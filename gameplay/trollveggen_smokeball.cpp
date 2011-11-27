
#include "headers.h"
#include "trollveggen.h"
#include "script.h"
#include "ostankino.h"

namespace script { class TrollSmokeball_RGB_Script : public Script
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
                setMessage( Gameplay::iLanguage->getUnicodeString( 538 ) );
            }
        }
    };
public:
    // script core
    TrollSmokeball_RGB_Script(Jumper* player) : Script( player )
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

/**
 * RGB tracking
 */

void castingCallback_TrollSmokeball_RGB_Tracking(Actor* parent)
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
    smokeBallDesc.radius = 2000.0f;
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 875.0f;

    // green smokeball 
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( 23205.9f, 141698.1f, 21659.0f );
    smokeballs.push_back( smokeBallDesc );

    // blue smokeball 
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( -25721.1f, 96578.7f, 27820.2f );
    smokeballs.push_back( smokeBallDesc );

    // cast red smokeball 
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -66936.4f, 65688.4f, -15831.8f );
    smokeballs.push_back( smokeBallDesc );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 1.5f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // cast script
    new script::TrollSmokeball_RGB_Script( mission->getPlayer() );
    
    // play original music for this mission    
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}

/**
 * RGB tracking
 */

void castingCallback_TrollSmokeball_RGB_Slalom(Actor* parent)
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
    smokeBallDesc.radius = 2000.0f;
    smokeBallDesc.numParticles = 256;
    smokeBallDesc.particleMass = 0.25f;
    smokeBallDesc.particleRadius = 875.0f;

    // green smokeball 
    smokeBallDesc.color.set( 0.25f, 1.0f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -80633.6f, 118241.0f, 21474.7f );
    smokeballs.push_back( smokeBallDesc );

    // blue smokeball 
    smokeBallDesc.color.set( 0.25f, 0.25f, 1.0f, 0.25f );
    smokeBallDesc.center.set( -103734.6f, 77817.5f, -6367.1f );
    smokeballs.push_back( smokeBallDesc );

    // cast red smokeball 
    smokeBallDesc.color.set( 1.0f, 0.25f, 0.25f, 0.25f );
    smokeBallDesc.center.set( -91338.1f, 47141.2f, -33289.1f );
    smokeballs.push_back( smokeBallDesc );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalSmokeball( mission->getPlayer(), smokeballs, 1.5f * mission->getPlayer()->getVirtues()->getMaximalBonusScore() );

    // cast script
    new script::TrollSmokeball_RGB_Script( mission->getPlayer() );
    
    // play original music for this mission    
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_insectosound.ogg" );
}