
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
#include "instructor.h"
#include "acroinstructor.h"

/**
 * lesson: jump from run
 */

namespace instructor { class JumpFromRunInstructor : public AcrobaticsInstructor
{
public:
    // script routines
    class Disqualification : public Notification
    {
    public:
        Disqualification(Jumper* jumper) : Notification( jumper, L"", 1.0f ) 
        {
            setLock( true );
        }
        virtual void onUpdateOrder(float dt)
        {
            Notification::onUpdateOrder( dt );

            // retrieve suit info
            database::Suit* suit = database::Suit::getRecord( getJumper()->getVirtues()->equipment.suit.id );
            if( !suit->wingsuit )
            {
                setLock( false );
            }
            else
            {
                setMessage( Gameplay::iLanguage->getUnicodeString(643) );
            }
        }
    };
    class Invitation01 : public Notification
    {
    public:
        Invitation01(Jumper* jumper) : 
            Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(665), 10 )
        {}
    };
    class Invitation02 : public Notification
    {
    public:
        Invitation02(Jumper* jumper) : 
            Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(667), 10 )
        {}
    };
    class Invitation03 : public Notification
    {
    public:
        Invitation03(Jumper* jumper) : 
            Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(668), 10 )
        {}
    };
    class Invitation04 : public Notification
    {
    public:
        Invitation04(Jumper* jumper) : 
            Instructor::Notification( 
                jumper, 
                wstrformat( 
                    Gameplay::iLanguage->getUnicodeString(669),
                    Gameplay::iGameplay->getActionChannel( ::iaForward )->getInputActionHint(),
                    Gameplay::iGameplay->getActionChannel( ::iaModifier )->getInputActionHint(),
                    Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint()
                ).c_str(),
                10 
            )
        {}
    };
public:
    JumpFromRunInstructor(Jumper* player) : AcrobaticsInstructor( player, ::acroJumpFromRun )
    {
        // disable phase change
        player->enablePhase( false );
        // create order
        _order = new Disqualification( player );
    }
public:
    virtual void onUpdateActivity(float dt)
    {
        // base behaviour
        AcrobaticsInstructor::onUpdateActivity( dt );

        // end of order
        if( _order && _order->isExecuted() )
        {
            if( orderIs(Disqualification) )
            {
                delete _order;
                _order = new Invitation01( _jumper );
            }
            else if( orderIs(Invitation01) )
            {
                delete _order;
                _order = new Invitation02( _jumper );
            }
            else if( orderIs(Invitation02) )
            {
                delete _order;
                _order = new Invitation03( _jumper );
            }
            else if( orderIs(Invitation03) )
            {
                delete _order;
                _order = new Invitation04( _jumper );
                _player->enablePhase( true );
            }
            else
            {
                delete _order;
                _order = NULL;
            }
        }
    }
}; }

void castingCallback_TrollField_JumpFromRun(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast instructor
    new instructor::JumpFromRunInstructor( mission->getPlayer() );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    
    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

bool equipCallback_TrollField(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // equip best suit
    if( !equipBestSuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}