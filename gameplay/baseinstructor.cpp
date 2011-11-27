
#include "headers.h"
#include "baseinstructor.h"
#include "gameplay.h"
#include "../common/istring.h"

using namespace instructor;

/**
 * abstract BASE jumping instructor
 */

BASEInstructor::JumpBeforeTheWind::JumpBeforeTheWind(Jumper* jumper, Vector3f wind) :
    Instructor::Notification( jumper, L"", 0.5f )
{
    _wind = wind;
    setLock( true );
}

void BASEInstructor::JumpBeforeTheWind::onUpdateOrder(float dt)
{
    // base behaviour
    Notification::onUpdateOrder( dt );

    if( isLocked() )
    {
        bool phaseTrigger = Gameplay::iGameplay->getActionChannel( ::iaPhase )->getTrigger();

        // determine condition: "jumper is turned before the wind" 
        Vector3f jumperAt = getJumper()->getClump()->getFrame()->getAt();
        jumperAt[1] = 0;
        jumperAt.normalize();
        float dotProduct = Vector3f::dot( jumperAt, _wind );
        if( dotProduct > 0 )
        {
            setMessage( Gameplay::iLanguage->getUnicodeString(467) );
            getJumper()->enablePhase( false );
        }
        else if( dotProduct < 0 && dotProduct > -0.77 )
        {
            const wchar_t* phaseHint = Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint();
            setMessage( wstrformat( Gameplay::iLanguage->getUnicodeString(468), phaseHint ).c_str() );
            getJumper()->enablePhase( true );
            if( phaseTrigger ) setLock( false );
        }
        else
        {
            const wchar_t* phaseHint = Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint();
            setMessage( wstrformat( Gameplay::iLanguage->getUnicodeString(469), phaseHint ).c_str() );
            getJumper()->enablePhase( true );
            if( phaseTrigger ) setLock( false );
        }
    }
}

BASEInstructor::BASEInstructor(Jumper* player)  : Instructor( player )
{
    _isAcquired = false;
    _score = 0;
}

BASEInstructor::~BASEInstructor()
{
    // mission isn't aborted?
    if( _player->isOverActivity() )
    {
        Virtues* virtues = getScene()->getCareer()->getVirtues();
        assert( _finite( getGoalScore() ) );
        virtues->evolution.score += getGoalScore();
    }
}

void BASEInstructor::onUpdateActivity(float dt)
{
    // base behaviour
    Instructor::onUpdateActivity( dt );

    // capture score
    if( !_isAcquired )
    {
        if( _player->getVirtues()->evolution.health == 0 ) _score = 0;
        else if( _player->isLanding() ) _score = 10.0f;
        else if( _player->isBadLanding() ) _score = 0.0f;
        else _score = 0.0f;
    }
}

const wchar_t* BASEInstructor::getGoalValue(void)
{
    if( _jumper->getVirtues()->evolution.health == 0 )
    {
        return Gameplay::iLanguage->getUnicodeString(281);
    }
    else
    {
        if( _jumper->isLanding() )
        {
            return Gameplay::iLanguage->getUnicodeString(283);
        }
        else
        {
            return Gameplay::iLanguage->getUnicodeString(282);
        }
    }
}

float BASEInstructor::getGoalScore(void)
{
    return _score;
}

/**
 * BASE jumping instructor ¹1
 */

BASEInstructor01::Invitation01::Invitation01(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(470), 10 )
{
}

BASEInstructor01::Invitation02::Invitation02(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(471), 10 )
{
}

BASEInstructor01::BASEInstructor01(Jumper* player) : BASEInstructor( player )
{
    // disable phase change
    player->enablePhase( false );

    // create order
    _order = new Invitation01( player );
}

void BASEInstructor01::onUpdateActivity(float dt)
{
    // base behaviour
    BASEInstructor::onUpdateActivity( dt );

    // end of order
    if( _order && _order->isExecuted() )
    {
        if( orderIs(Invitation01) )
        {
            delete _order;
            _order = new Invitation02( _jumper );
        }
        else if( orderIs(Invitation02) )
        {
            delete _order;
            _order = new JumpBeforeTheWind( _jumper, _scene->getLocation()->getWindDirection() );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }
}

/**
 * BASE jumping instructor ¹2
 */

BASEInstructor02::Invitation::Invitation(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(474), 10 )
{
}

BASEInstructor02::Pull::Pull(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(401), 
            Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint()
        ).c_str(),
        0.25f
    )
{
    setLock( true );
    _timeSpeed = 1.0f;
}

BASEInstructor02::Pull::~Pull()
{
    getJumper()->getScene()->setTimeSpeed( 1.0f );
}

void BASEInstructor02::Pull::onUpdateOrder(float dt)
{
    // inherited behaviour
    Notification::onUpdateOrder( dt );

    float minTimeSpeed = 0.0625f;

    if( isLocked() && !getJumper()->getFreefallActor()->isSleeping() )
    {
        // update time speed
        _timeSpeed -= 2 * _timeSpeed * dt;
        if( _timeSpeed < minTimeSpeed ) _timeSpeed = minTimeSpeed;

        // obtain jumper's altitude
        float altitude = getJumper()->getClump()->getFrame()->getPos()[1];

        // check input action, or altitude
        if( Gameplay::iGameplay->getActionChannel( ::iaPhase )->getTrigger() ||
            altitude < 10000.0f )
        {
            setLock( false );
        }
    }
    else
    {
        // update time speed
        _timeSpeed += 2 * _timeSpeed * dt;
        if( _timeSpeed > 1.0f ) _timeSpeed = 1.0f;
    }

    getJumper()->getScene()->setTimeSpeed( _timeSpeed );
}

bool BASEInstructor02::Pull::isExecuted(void)
{
    return ( !isLocked() && _timeSpeed == 1.0f );
}

BASEInstructor02::BASEInstructor02(Jumper* player) : BASEInstructor( player )
{
    // disable phase change
    player->enablePhase( false );

    // create order
    _order = new Invitation( player );
}

void BASEInstructor02::onUpdateActivity(float dt)
{
    // base behaviour
    BASEInstructor::onUpdateActivity( dt );

    // end of order
    if( _order && _order->isExecuted() )
    {
        if( orderIs(Invitation) )
        {
            delete _order;
            _order = new JumpBeforeTheWind( _jumper, _scene->getLocation()->getWindDirection() );
        }
        else if( orderIs(JumpBeforeTheWind) )
        {
            delete _order;
            _order = new Pull( _jumper );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }
}

/**
 * BASE jumping instructor ¹3
 */

BASEInstructor03::Invitation::Invitation(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(477), 10 )
{
}

BASEInstructor03::BASEInstructor03(Jumper* player) : BASEInstructor( player )    
{
    // disable phase change
    player->enablePhase( false );

    // create order
    _order = new Invitation( player );

    // create "landing accuracy" goal
    Vector3f pos;
    switch( getCore()->getRandToolkit()->getUniformInt() % 3 )
    {
    case 0:
        pos.set( 55811, 5, 49582 );
        break;
    case 1:
        pos.set( 42015, 5, 57016 );
        break;
    case 2:
        pos.set( 46383, 5, 52490 );
        break;
    }

    _goalLandingAccuracy = new GoalLandingAccuracy( player, pos, 25.0f );
}

void BASEInstructor03::onUpdateActivity(float dt)
{
    // base behaviur
    BASEInstructor::onUpdateActivity( dt );

    // end of order
    if( _order && _order->isExecuted() )
    {
        if( orderIs(Invitation) )
        {
            delete _order;
            _order = new JumpBeforeTheWind( 
                BASEInstructor::_jumper, 
                BASEInstructor::_scene->getLocation()->getWindDirection() 
            );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }
}

const wchar_t* BASEInstructor03::getGoalValue(void)
{
    float landingAccuracy = _goalLandingAccuracy->getLandingAccuracy();
    if( landingAccuracy > 0.66f )
    {
        return Gameplay::iLanguage->getUnicodeString(481);
    }
    else if( landingAccuracy > 0.33f )
    {
        return Gameplay::iLanguage->getUnicodeString(482);
    }
    else if( landingAccuracy > 0 )
    {
        return Gameplay::iLanguage->getUnicodeString(483);
    }
    else
    {
        if( _jumper->getVirtues()->evolution.health == 0 )
        {
            return Gameplay::iLanguage->getUnicodeString(281);
        }
        else
        {
            return Gameplay::iLanguage->getUnicodeString(282);
        }
    }
}

/**
 * BASE jumping instructor ¹3
 */

BASEInstructor04::Invitation01::Invitation01(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(509), 10 )
{
}

BASEInstructor04::Invitation02::Invitation02(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(510), 10 )
{
}

BASEInstructor04::Jump::Jump(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(240), 
            Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint()
        ).c_str(),
        0.5f
    )
{
    setLock( true );
    jumper->enablePhase( true );
}

void BASEInstructor04::Jump::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaPhase )->getTrigger() )
    {
        setLock( false );
    }
}

BASEInstructor04::Pull::Pull(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(412), 
            Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void BASEInstructor04::Pull::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaPhase )->getTrigger() )
    {
        setLock( false );
    }
}

BASEInstructor04::FightLineover::FightLineover(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(511), 
            Gameplay::iGameplay->getActionChannel( ::iaWLO)->getInputActionHint()
        ).c_str(),
        0.5f
    )
{
    setLock( true );
}

void BASEInstructor04::FightLineover::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && 
        getJumper()->getCanopySimulator()->getLeftLOW() == 0.0f && 
        getJumper()->getCanopySimulator()->getRightLOW() == 0.0f )
    {
        setLock( false );
    }
}

BASEInstructor04::AboutHookKnife::AboutHookKnife(Jumper* jumper) : 
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(512), 
            Gameplay::iGameplay->getActionChannel( ::iaHook )->getInputActionHint()
        ).c_str(),
        10.0f
    )
{
}

BASEInstructor04::Farewell::Farewell(Jumper* jumper) : 
    Instructor::Notification( 
        jumper,
        Gameplay::iLanguage->getUnicodeString(513),
        10.0f
    )
{
}

BASEInstructor04::BASEInstructor04(Jumper* player) : BASEInstructor( player )    
{
    // disable phase change
    player->enablePhase( false );

    // create order
    _order = new Invitation01( player );

    // reset internal behaviour trigger
    _lineoverIsApplied = false;
}

void BASEInstructor04::onUpdateActivity(float dt)
{
    // base behaviur
    BASEInstructor::onUpdateActivity( dt );

    // end of order
    if( _order && _order->isExecuted() )
    {
        if( orderIs(Invitation01) )
        {
            delete _order;
            _order = new Invitation02( _jumper );
        }
        else if( orderIs(Invitation02) )
        {
            delete _order;
            _order = new Jump( _jumper );
        }
        else if( orderIs(Jump) )
        {
            delete _order;
            _order = new Pull( _jumper );
        }
        else if( orderIs(Pull) )
        {
            delete _order;
            _order = new FightLineover( _jumper );
        }
        else if( orderIs(FightLineover) )
        {
            delete _order;
            _order = new AboutHookKnife( _jumper );
        }
        else if( orderIs(AboutHookKnife) )
        {
            delete _order;
            _order = new Farewell( _jumper );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }

    // apply lineover
    if( _jumper->getPhase() == ::jpFlight && !_lineoverIsApplied )
    {        
        if( _jumper->getCanopySimulator()->isOpened() )
        {
            _lineoverIsApplied = true;
            _jumper->getCanopySimulator()->setLineover( 1.0f, 0.0f, true );
            _jumper->getCanopySimulator()->setLinetwists( 0.0f );
        }
    }
}