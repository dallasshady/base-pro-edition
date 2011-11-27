
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
#include "acroinstructor.h"

/**
 * common equip callback
 */

bool equipCallback_RoyalGorge_FieldTraining(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
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
 * front flip lesson
 */

namespace instructor { class FrontFlipInstructor : public AcrobaticsInstructor
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
            Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(678), 10 )
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
            Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(673), 10 )
        {}
    };
    class PerformFrontflip : public Notification
    {
    public:
        PerformFrontflip(Jumper* jumper) : 
            Instructor::Notification( 
                jumper, 
                wstrformat(
                    Gameplay::iLanguage->getUnicodeString(679),
                    Gameplay::iGameplay->getActionChannel( ::iaForward )->getInputActionHint(),
                    Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint()                    
                ).c_str(),
                1 
            )
        {
            setLock( true );
            jumper->enablePhase( true );
        }
    public:
        virtual void onUpdateOrder(float dt)
        {
            Notification::onUpdateOrder( dt );

            if( getJumper()->getPhase() == ::jpFreeFalling )
            {
                setLock( false );
            }
        }
    };
    class Invitation04 : public Notification
    {
    public:
        Invitation04(Jumper* jumper) : 
            Instructor::Notification( 
                jumper, 
                wstrformat(
                    Gameplay::iLanguage->getUnicodeString(675),
                    Gameplay::iGameplay->getActionChannel( ::iaModifier )->getInputActionHint(),
                    Gameplay::iGameplay->getActionChannel( ::iaBackward )->getInputActionHint()
                ).c_str(),
                10 
            )
        {}
    };
public:
    FrontFlipInstructor(Jumper* player) : AcrobaticsInstructor( player, ::acroFrontFlip )
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
                _order = new PerformFrontflip( _jumper );
            }
            else if( orderIs(PerformFrontflip) )
            {
                delete _order;
                _order = new Invitation04( _jumper );
            }
            else
            {
                delete _order;
                _order = NULL;
            }
        }
    }
}; }

void castingCallback_GoFallField_Frontflip(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast instructor
    new instructor::FrontFlipInstructor( mission->getPlayer() );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

/**
 * back flip lesson
 */

namespace instructor { class BackFlipInstructor : public AcrobaticsInstructor
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
            Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(672), 10 )
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
            Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(673), 10 )
        {}
    };
    class PerformBackflip : public Notification
    {
    public:
        PerformBackflip(Jumper* jumper) : 
            Instructor::Notification( 
                jumper, 
                wstrformat(
                    Gameplay::iLanguage->getUnicodeString(674),
                    Gameplay::iGameplay->getActionChannel( ::iaBackward )->getInputActionHint(),
                    Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint()                    
                ).c_str(),
                1 
            )
        {
            setLock( true );
            jumper->enablePhase( true );
        }
    public:
        virtual void onUpdateOrder(float dt)
        {
            Notification::onUpdateOrder( dt );

            if( getJumper()->getPhase() == ::jpFreeFalling )
            {
                setLock( false );
            }
        }
    };
    class Invitation04 : public Notification
    {
    public:
        Invitation04(Jumper* jumper) : 
            Instructor::Notification( 
                jumper, 
                wstrformat(
                    Gameplay::iLanguage->getUnicodeString(675),
                    Gameplay::iGameplay->getActionChannel( ::iaModifier )->getInputActionHint(),
                    Gameplay::iGameplay->getActionChannel( ::iaForward )->getInputActionHint()
                ).c_str(),
                10 
            )
        {}
    };
public:
    BackFlipInstructor(Jumper* player) : AcrobaticsInstructor( player, ::acroFrontBackFlip )
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
                _order = new PerformBackflip( _jumper );
            }
            else if( orderIs(PerformBackflip) )
            {
                delete _order;
                _order = new Invitation04( _jumper );
            }
            else
            {
                delete _order;
                _order = NULL;
            }
        }
    }
}; }

void castingCallback_GoFallField_Backflip(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast instructor
    new instructor::BackFlipInstructor( mission->getPlayer() );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}

/**
 * back flip back forward lesson
 */

namespace instructor { class BackFlipBackForwardInstructor : public AcrobaticsInstructor
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
            Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(682), 10 )
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
            Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(683), 10 )
        {}
    };
    class PerformBackFlipBackForward : public Notification
    {
    public:
        PerformBackFlipBackForward(Jumper* jumper) : 
            Instructor::Notification( 
                jumper, 
                wstrformat(
                    Gameplay::iLanguage->getUnicodeString(684),
                    Gameplay::iGameplay->getActionChannel( ::iaBackward )->getInputActionHint(),
                    Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint()                    
                ).c_str(),
                1 
            )
        {
            setLock( true );
            jumper->enablePhase( true );
        }
    public:
        virtual void onUpdateOrder(float dt)
        {
            Notification::onUpdateOrder( dt );

            if( getJumper()->getPhase() == ::jpFreeFalling )
            {
                setLock( false );
            }
        }
    };
    class Invitation04 : public Notification
    {
    public:
        Invitation04(Jumper* jumper) : 
            Instructor::Notification( 
                jumper, 
                wstrformat(
                    Gameplay::iLanguage->getUnicodeString(675),
                    Gameplay::iGameplay->getActionChannel( ::iaModifier )->getInputActionHint(),
                    Gameplay::iGameplay->getActionChannel( ::iaForward )->getInputActionHint()
                ).c_str(),
                10 
            )
        {}
    };
public:
    BackFlipBackForwardInstructor(Jumper* player) : AcrobaticsInstructor( player, ::acroBackFlip )
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
                _order = new PerformBackFlipBackForward( _jumper );
            }
            else if( orderIs(PerformBackFlipBackForward) )
            {
                delete _order;
                _order = new Invitation04( _jumper );
            }
            else
            {
                delete _order;
                _order = NULL;
            }
        }
    }
}; }

void castingCallback_GoFallField_BackflipBackforward(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast instructor
    new instructor::BackFlipBackForwardInstructor( mission->getPlayer() );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_action.ogg" );
}