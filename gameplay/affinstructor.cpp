
#include "headers.h"
#include "affinstructor.h"
#include "gameplay.h"
#include "hud.h"
#include "../common/istring.h"

using namespace instructor;

/**
 * abstract static line instructor
 */

StaticLineInstructor::Jump::Jump(Jumper* jumper) :
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

void StaticLineInstructor::Jump::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );

    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaPhase )->getTrigger() )
    {
        setLock( false );
    }
}

StaticLineInstructor::DontBeAfraid::DontBeAfraid(Jumper* jumper, float duration) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(268), duration )
{
}


StaticLineInstructor::AgainstTheWind::AgainstTheWind(Jumper* jumper, Vector3f wind, const wchar_t* orderText, float orderAltitude) :
    Instructor::Notification( jumper, L"", 0.1f )
{
    setLock( true );
    _wind = wind;
    _orderText = orderText;
    _orderAltitude = orderAltitude;
}

void StaticLineInstructor::AgainstTheWind::onUpdateOrder(float dt)
{
    // base behaviour
    Notification::onUpdateOrder( dt );

    // check dot product
    if( isLocked() )
    {
        Vector3f jumperAt = getJumper()->getClump()->getFrame()->getAt();
        jumperAt[1] = 0;
        jumperAt.normalize();
        float dotProduct = Vector3f::dot( jumperAt, _wind );
        float vel = getJumper()->getVel().length() * 0.01f;
        if( dotProduct > 0.77 )
        {
            setMessage( wstrformat( Gameplay::iLanguage->getUnicodeString(278), _orderText.c_str(), vel ).c_str() );
        }
        else if( dotProduct > -0.77 )
        {
            setMessage( wstrformat( Gameplay::iLanguage->getUnicodeString(277), _orderText.c_str(), vel ).c_str() );
        }
        else
        {
            setMessage( wstrformat( Gameplay::iLanguage->getUnicodeString(271), _orderText.c_str(), vel ).c_str() );
        }

        // end of order - by altitude
        Vector3f jumperPos = getJumper()->getClump()->getFrame()->getPos();
        if( jumperPos[1] < _orderAltitude ) setLock( false );
    }
}


StaticLineInstructor::Brake::Brake(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(280), 
            Gameplay::iGameplay->getActionChannel( ::iaLeft )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaRight )->getInputActionHint()
        ).c_str(),
        10.0f
    )
{}

StaticLineInstructor::StaticLineInstructor(Jumper* player)  : Instructor( player )
{
    _isAcquired = false;
    _score = 0;
}

StaticLineInstructor::~StaticLineInstructor()
{
    // mission isn't aborted?
    if( _player->isOverActivity() )
    {
        Virtues* virtues = getScene()->getCareer()->getVirtues();
        assert( _finite( getGoalScore() ) );
        virtues->evolution.score += getGoalScore();
    }
}

void StaticLineInstructor::onUpdateActivity(float dt)
{
    // base behaviour
    Instructor::onUpdateActivity( dt );

    // simulate static line
    if( _jumper->getPhase() == ::jpFreeFalling )
    {
        if( !_jumper->getPilotchuteSimulator()->isPulled() &&
            !_jumper->getFreefallActor()->isSleeping() )
        {
            // pilotchute pull frame
            Vector3f pp = Jumper::getLineRigJoint( _jumper->getClump() )->getPos();
            Vector3f py = _jumper->getVel(); py.normalize();
            Vector3f px; px.cross( py, Vector3f(0,1,0) ); px.normalize();
            Vector3f pz; py.cross( px, py ); pz.normalize();
            pp += py * 100.0f;

            // connect pilot chute
            _jumper->getPilotchuteSimulator()->connect( 
                _jumper->getFreefallActor(), 
                Jumper::getBackBone( _jumper->getClump() ), 
                _jumper->getLocalPilotAnchor() 
            );

            // pull pilotchute
            _jumper->getPilotchuteSimulator()->pull( Matrix4f(
                px[0], px[1], px[2], 0.0f,
                py[0], py[1], py[2], 0.0f,
                pz[0], pz[1], pz[2], 0.0f,
                pp[0], pp[1], pp[2], 1.0f
            ) );
            _jumper->getPilotchuteSimulator()->updateActivity( 0.0f );

            // and drop
            _jumper->getPilotchuteSimulator()->drop( NxVec3( 0,0,0 ) );
            _jumper->getPilotchuteSimulator()->setInflation( 0.5f );
        }
    }

    // capture score
    if( !_isAcquired )
    {
        if( _player->getVirtues()->evolution.health == 0 ) _score = 0;
        else if( _player->isLanding() ) _score = 5.0f;
        else if( _player->isBadLanding() ) _score = 0.0f;
        else _score = 0.0f;
    }
}

const wchar_t* StaticLineInstructor::getGoalValue(void)
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

float StaticLineInstructor::getGoalScore(void)
{
    return _score;
}

/**
 * static line instructor ¹1
 */

StaticLineInstructor01::Invitation::Invitation(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(237), 10 )
{
}

StaticLineInstructor01::CameraControl::CameraControl(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(239), 
            Gameplay::iGameplay->getActionChannel( ::iaHeadLeft )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaHeadUp )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaZoomIn )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaZoomOut )->getInputActionHint()
        ).c_str(),
        10
    )
{
}

StaticLineInstructor01::DiscoverTheWind::DiscoverTheWind(Jumper* jumper) :
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(270), 10.0f )
{
}

StaticLineInstructor01::StaticLineInstructor01(Jumper* player) : StaticLineInstructor( player )
{
    // disable phase change
    player->enablePhase( false );

    // create order
    _order = new Invitation( player );    
}

void StaticLineInstructor01::onUpdateActivity(float dt)
{
    // base behaviour
    StaticLineInstructor::onUpdateActivity( dt );

    // end of order
    if( _order && _order->isExecuted() )
    {
        if( orderIs(Invitation) )
        {
            delete _order;
            _order = new CameraControl( _jumper );
        }
        else if( orderIs(CameraControl) )
        {
            delete _order;
            _order = new Jump( _jumper );
        }
        else if( orderIs(Jump) )
        {
            delete _order;
            _order = new DontBeAfraid( _jumper, 3.0f );
        }
        else if( orderIs(DontBeAfraid) )
        {
            delete _order;
            _order = new DiscoverTheWind( _jumper );
        }
        else if( orderIs(DiscoverTheWind) )
        {
            delete _order;
            _order = new AgainstTheWind( _jumper, _scene->getLocation()->getWindDirection(), L"", 2500.0f );
        }
        else if( orderIs(AgainstTheWind) )
        {
            delete _order;
            _order = new Brake( _jumper );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }
}

/**
 * static line instructor ¹2
 */

StaticLineInstructor02::Invitation::Invitation(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(205), 10 )
{
}

StaticLineInstructor02::FirstPersonMode::FirstPersonMode(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(206), 
            Gameplay::iGameplay->getActionChannel( ::iaCameraMode0 )->getInputActionHint()
        ).c_str(),
        2.0f
    )
{
    setLock( true );
}

void StaticLineInstructor02::FirstPersonMode::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );

    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaCameraMode0 )->getTrigger() )
    {
        setLock( false );
    }
}

StaticLineInstructor02::ThirdPersonMode::ThirdPersonMode(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(207), 
            Gameplay::iGameplay->getActionChannel( ::iaCameraMode1 )->getInputActionHint()
        ).c_str(),
        2.0f
    )
{
    setLock( true );
}

void StaticLineInstructor02::ThirdPersonMode::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );

    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaCameraMode1 )->getTrigger() )
    {
        setLock( false );
    }
}

StaticLineInstructor02::CanopyControl::CanopyControl(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(395), 
            Gameplay::iGameplay->getActionChannel( ::iaLeft )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaRight )->getInputActionHint()
        ).c_str(),
        10.0f
    )
{
}

StaticLineInstructor02::StaticLineInstructor02(Jumper* player) : StaticLineInstructor( player )
{
    // disable phase change
    player->enablePhase( false );

    // create order
    _order = new Invitation( player );
}

void StaticLineInstructor02::onUpdateActivity(float dt)
{
    // base behaviour
    StaticLineInstructor::onUpdateActivity( dt );

    // end of order
    if( _order && _order->isExecuted() )
    {
        if( orderIs(Invitation) )
        {
            delete _order;
            _order = new FirstPersonMode( _jumper );
        }
        else if( orderIs(FirstPersonMode) )
        {
            delete _order;
            _order = new ThirdPersonMode( _jumper );
        }
        else if( orderIs(ThirdPersonMode) )
        {
            delete _order;
            _order = new Jump( _jumper );
        }
        else if( orderIs(Jump) )
        {
            delete _order;
            _order = new DontBeAfraid( _jumper, 3.0f );
        }
        else if( orderIs(DontBeAfraid) )
        {
            delete _order;
            _order = new CanopyControl( _jumper );
        }
        else if( orderIs(CanopyControl) )
        {
            delete _order;
            _order = new AgainstTheWind( _jumper, _scene->getLocation()->getWindDirection(), Gameplay::iLanguage->getUnicodeString(396), 2500.0f );
        }
        else if( orderIs(AgainstTheWind) )
        {
            delete _order;
            _order = new Brake( _jumper );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }
}

/**
 * static line instructor ¹3
 */

StaticLineInstructor03::QuickLeft::QuickLeft(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(210), 
            Gameplay::iGameplay->getActionChannel( ::iaLeft )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaLeftWarp )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void StaticLineInstructor03::QuickLeft::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );

    if( isLocked() && 
        Gameplay::iGameplay->getActionChannel( ::iaLeft )->getTrigger() &&
        Gameplay::iGameplay->getActionChannel( ::iaLeftWarp )->getTrigger() )
    {
        setLock( false );
    }
}

StaticLineInstructor03::QuickRight::QuickRight(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(273), 
            Gameplay::iGameplay->getActionChannel( ::iaRight )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaRightWarp )->getInputActionHint()
        ).c_str(),
        5.0f
    )
{
}

StaticLineInstructor03::OtherCombos::OtherCombos(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(274), 
            Gameplay::iGameplay->getActionChannel( ::iaLeft )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaRightWarp )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaRight )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaLeftWarp )->getInputActionHint()
        ).c_str(),
        10.0f
    )
{
}

StaticLineInstructor03::BowCanopy::BowCanopy(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(397), 
            Gameplay::iGameplay->getActionChannel( ::iaLeftWarp )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaRightWarp )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void StaticLineInstructor03::BowCanopy::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );

    if( isLocked() && 
        Gameplay::iGameplay->getActionChannel( ::iaLeftWarp )->getTrigger() &&
        Gameplay::iGameplay->getActionChannel( ::iaRightWarp )->getTrigger() )
    {
        setLock( false );
    }
}

StaticLineInstructor03::StaticLineInstructor03(Jumper* player) : StaticLineInstructor( player )
{
    // disable phase change
    player->enablePhase( false );

    // create order
    _order = new Jump( player );
}

void StaticLineInstructor03::onUpdateActivity(float dt)
{
    // base behaviour
    StaticLineInstructor::onUpdateActivity( dt );

    // end of order
    if( _order && _order->isExecuted() )
    {
        if( orderIs(Jump) )
        {
            delete _order;
            _order = new DontBeAfraid( _jumper, 10.0f );
        }    
        else if( orderIs(DontBeAfraid) )
        {
            delete _order;
            _order = new QuickLeft( _jumper );
        }
        else if( orderIs(QuickLeft) )
        {
            delete _order;
            _order = new QuickRight( _jumper );
        }
        else if( orderIs(QuickRight) )
        {
            delete _order;
            _order = new OtherCombos( _jumper );
        }
        else if( orderIs(OtherCombos) )
        {
            delete _order;
            _order = new BowCanopy( _jumper );
        }
        else if( orderIs(BowCanopy) )
        {
            delete _order;
            _order = new AgainstTheWind( _jumper, _scene->getLocation()->getWindDirection(), Gameplay::iLanguage->getUnicodeString(396), 2500.0f );
        }
        else if( orderIs(AgainstTheWind) )
        {
            delete _order;
            _order = new Brake( _jumper );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }
}

/**
 * hop and pop instructor
 */

HopAndPopInstructor::Invitation::Invitation(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(400), 10 )
{
}

HopAndPopInstructor::Pull::Pull(Jumper* jumper) : 
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(401), 
            Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
    _timeSpeed = 1.0f;
}

HopAndPopInstructor::Pull::~Pull()
{
    getJumper()->getScene()->setTimeSpeed( 1.0f );
}

void HopAndPopInstructor::Pull::onUpdateOrder(float dt)
{
    // inherited behaviour
    Notification::onUpdateOrder( dt );

    float minTimeSpeed = 0.25f;

    if( isLocked() && !getJumper()->getFreefallActor()->isSleeping() )
    {
        // update time speed
        _timeSpeed -= 2 * _timeSpeed * dt;
        if( _timeSpeed < minTimeSpeed ) _timeSpeed = minTimeSpeed;

        // obtain jumper's altitude
        float altitude = getJumper()->getClump()->getFrame()->getPos()[1];

        // check input action, or altitude
        if( Gameplay::iGameplay->getActionChannel( ::iaPhase )->getTrigger() ||
            altitude < 20000.0f )
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

bool HopAndPopInstructor::Pull::isExecuted(void)
{
    return ( !isLocked() && _timeSpeed == 1.0f );
}

HopAndPopInstructor::HopAndPopInstructor(Jumper* player) : StaticLineInstructor( player )
{
    // disable phase change
    player->enablePhase( false );

    // create order
    _order = new Invitation( player );
}

void HopAndPopInstructor::onUpdateActivity(float dt)
{
    // super-base behaviour
    Instructor::onUpdateActivity( dt );

    // end of order
    if( _order && _order->isExecuted() )
    {
        if( orderIs(Invitation) )
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
            _order = new AgainstTheWind( _jumper, _scene->getLocation()->getWindDirection(), Gameplay::iLanguage->getUnicodeString(396), 2500.0f );
        }
        else if( orderIs(AgainstTheWind) )
        {
            delete _order;
            _order = new Brake( _jumper );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }
}

/**
 * abstract freefall instructor
 */

FreefallInstructor::Pull::Pull(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(412),
            Gameplay::iGameplay->getActionChannel( ::iaPhase )->getInputActionHint()
        ).c_str(),
        1.0f
    )
{
    setLock( true );
    jumper->enablePhase( true );
}

void FreefallInstructor::Pull::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaPhase )->getTrigger() )
    {
        setLock( false );
    }
}

FreefallInstructor::FreefallInstructor(Jumper* player) : StaticLineInstructor( player )
{
}

void FreefallInstructor::onUpdateActivity(float dt)
{
    // super-base behaviour (no static-line simulation)
    Instructor::onUpdateActivity( dt );
}

/**
 * freefall instructor ¹1
 */

FreefallInstructor01::Invitation::Invitation(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(404), 10 )
{}

FreefallInstructor01::FixCamera::FixCamera(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(405), 
            Gameplay::iGameplay->getActionChannel( ::iaCameraMode1 )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void FreefallInstructor01::FixCamera::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaCameraMode1 )->getTrigger() )
    {
        setLock( false );
    }
}

FreefallInstructor01::ZoomCamera::ZoomCamera(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(406), 
            Gameplay::iGameplay->getActionChannel( ::iaZoomIn )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void FreefallInstructor01::ZoomCamera::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaZoomIn )->getTrigger() )
    {
        setLock( false );
    }
}

FreefallInstructor01::ThirdPersonMode::ThirdPersonMode(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(407), 
            Gameplay::iGameplay->getActionChannel( ::iaCameraMode1 )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void FreefallInstructor01::ThirdPersonMode::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaCameraMode1 )->getTrigger() )
    {
        setLock( false );
    }
}

FreefallInstructor01::TurnLeft::TurnLeft(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(408), 
            Gameplay::iGameplay->getActionChannel( ::iaLeft )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void FreefallInstructor01::TurnLeft::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaLeft )->getTrigger() )
    {
        setLock( false );
    }
}

FreefallInstructor01::TurnRight::TurnRight(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(409), 
            Gameplay::iGameplay->getActionChannel( ::iaRight )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void FreefallInstructor01::TurnRight::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaRight )->getTrigger() )
    {
        setLock( false );
    }
}

FreefallInstructor01::TurnHeadUpAndHeadDown::TurnHeadUpAndHeadDown(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(410), 
            Gameplay::iGameplay->getActionChannel( ::iaForward )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaBackward )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void FreefallInstructor01::TurnHeadUpAndHeadDown::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && 
        ( Gameplay::iGameplay->getActionChannel( ::iaForward )->getTrigger() ||
          Gameplay::iGameplay->getActionChannel( ::iaBackward )->getTrigger() ) )
    {
        setLock( false );
    }
}

FreefallInstructor01::AlignAndStabilize::AlignAndStabilize(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        Gameplay::iLanguage->getUnicodeString(411),
        2.0f
    )
{
    setLock( true );
}

void FreefallInstructor01::AlignAndStabilize::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() )
    {
        // check pose alignment
        Vector3f jumperUp = getJumper()->getClump()->getFrame()->getUp();
        jumperUp.normalize();
        if( Vector3f::dot( jumperUp, Vector3f(0,1,0) ) > 0.9f ) 
        {
            setLock( false );
        }
        else
        {
            setLock( true );
        }
    }
}

FreefallInstructor01::TimeCompression::TimeCompression(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(413), 
            Gameplay::iGameplay->getActionChannel( ::iaAccelerateFlightTime )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaDecelerateFlightTime )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void FreefallInstructor01::TimeCompression::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() && 
        ( Gameplay::iGameplay->getActionChannel( ::iaAccelerateFlightTime )->getTrigger() ||
          Gameplay::iGameplay->getActionChannel( ::iaDecelerateFlightTime )->getTrigger() ) )
    {
        setLock( false );
    }
}

FreefallInstructor01::LoseHeight::LoseHeight(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        Gameplay::iLanguage->getUnicodeString(414),
        0.5f
    )
{
    setLock( true );
}

void FreefallInstructor01::LoseHeight::onUpdateOrder(float dt)
{
    Notification::onUpdateOrder( dt );
    if( isLocked() )
    {
        Vector3f jumperPos = getJumper()->getClump()->getFrame()->getPos();
        if( jumperPos[1] < 20000.0f )
        {
            setLock( false );
        }        
    }
}

FreefallInstructor01::FreefallInstructor01(Jumper* player) : FreefallInstructor( player )
{
    player->enablePhase( false );

    // create order
    _order = new Invitation( player );
}

void FreefallInstructor01::onUpdateActivity(float dt)
{
    // base behaviour
    FreefallInstructor::onUpdateActivity( dt );

    // end of order
    if( _order && _order->isExecuted() )
    {
        if( orderIs(Invitation) )
        {
            delete _order;
            _order = new FixCamera( _jumper );
        }
        else if( orderIs(FixCamera) )
        {
            delete _order;
            _order = new ZoomCamera( _jumper );
        }
        else if( orderIs(ZoomCamera) )
        {
            delete _order;
            _order = new ThirdPersonMode( _jumper );
        }
        else if( orderIs(ThirdPersonMode) )
        {
            delete _order;
            _order = new Jump( _jumper );
        }
        else if( orderIs(Jump) )
        {
            delete _order;
            _order = new TurnLeft( _jumper );
            _jumper->enablePhase( false );
        }
        else if( orderIs(TurnLeft) )
        {
            delete _order;
            _order = new TurnRight( _jumper );
        }
        else if( orderIs(TurnRight) )
        {
            delete _order;
            _order = new TurnHeadUpAndHeadDown( _jumper );
        }
        else if( orderIs(TurnHeadUpAndHeadDown) )
        {
            delete _order;
            _order = new AlignAndStabilize( _jumper );
        }
        else if( orderIs(AlignAndStabilize) )
        {
            delete _order;
            _order = new Pull( _jumper );
        }
        else if( orderIs(Pull) )
        {
            delete _order;
            _order = new TimeCompression( _jumper );
        }
        else if( orderIs(TimeCompression) )
        {
            delete _order;
            _order = new LoseHeight( _jumper );
        }
        else if( orderIs(LoseHeight) )
        {
            delete _order;
            _order = new AgainstTheWind( _jumper, _scene->getLocation()->getWindDirection(), L"", 2500.0f );
        }
        else if( orderIs(AgainstTheWind) )
        {
            delete _order;
            _order = new Brake( _jumper );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }

    // check altitude
    if( _jumper->getPhase() != ::jpFlight )
    {
        Vector3f jumperPos = _jumper->getClump()->getFrame()->getPos();
        if( jumperPos[1] < 70000.0f && !orderIs(Pull) )
        {
            delete _order;
            _order = new Pull( _jumper );
        }
    }
    else
    {
        Vector3f jumperPos = _jumper->getClump()->getFrame()->getPos();
        // check altitude, disable time acceleration if altitude is less than 200m
        if( jumperPos[1] < 20000.0f )
        {
            _scene->setTimeSpeedMultiplier( 1.0f );
        }
    }
}

/**
 * freefall instructor ¹2
 */

FreefallInstructor02::Invitation01::Invitation01(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(417), 10 )
{
}

FreefallInstructor02::Invitation02::Invitation02(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(418), 10 )
{
}

FreefallInstructor02::AssumeTrackingPose::AssumeTrackingPose(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(419), 
            Gameplay::iGameplay->getActionChannel( ::iaModifier )->getInputActionHint()
        ).c_str(),
        1.0f
    )
{
    setLock( true );
}

void FreefallInstructor02::AssumeTrackingPose::onUpdateOrder(float dt)
{
    // base behaviour
    Notification::onUpdateOrder( dt );

    if( isLocked() && Gameplay::iGameplay->getActionChannel( ::iaModifier )->getTrigger() )
    {
        setLock( false );
    }
}

FreefallInstructor02::InclinePose::InclinePose(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(426), 
            Gameplay::iGameplay->getActionChannel( ::iaForward )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaBackward )->getInputActionHint()
        ).c_str(),
        1.0f
    )
{
    setLock( true );
}

void FreefallInstructor02::InclinePose::onUpdateOrder(float dt)
{
    // base behaviour
    Notification::onUpdateOrder( dt );

    if( isLocked() && 
        ( Gameplay::iGameplay->getActionChannel( ::iaForward )->getTrigger() ||
          Gameplay::iGameplay->getActionChannel( ::iaBackward )->getTrigger() ) )
    {
        setLock( false );
    }
}

FreefallInstructor02::SteerPose::SteerPose(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(427), 
            Gameplay::iGameplay->getActionChannel( ::iaLeft )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaRight )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void FreefallInstructor02::SteerPose::onUpdateOrder(float dt)
{
    // base behaviour
    Notification::onUpdateOrder( dt );

    if( isLocked() && 
        ( Gameplay::iGameplay->getActionChannel( ::iaLeft )->getTrigger() ||
          Gameplay::iGameplay->getActionChannel( ::iaRight )->getTrigger() ) )
    {
        setLock( false );
    }
}

FreefallInstructor02::FreefallInstructor02(Jumper* player) : FreefallInstructor( player )
{
    player->enablePhase( false );

    // create order
    _order = new Invitation01( player );
}

void FreefallInstructor02::onUpdateActivity(float dt)
{
    // base behaviour
    FreefallInstructor::onUpdateActivity( dt );

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
            _order = new AssumeTrackingPose( _jumper );
            _jumper->enablePhase( false );
        }
        else if( orderIs(AssumeTrackingPose) )
        {
            delete _order;
            _order = new InclinePose( _jumper );
        }
        else if( orderIs(InclinePose) )
        {
            delete _order;
            _order = new SteerPose( _jumper );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }

    // check altitude
    if( _jumper->getPhase() != ::jpFlight )
    {
        Vector3f jumperPos = _jumper->getClump()->getFrame()->getPos();
        if( jumperPos[1] < 70000.0f && !orderIs(Pull) )
        {
            delete _order;
            _order = new Pull( _jumper );
        }
    }
    else
    {
        Vector3f jumperPos = _jumper->getClump()->getFrame()->getPos();
        // check altitude, disable time acceleration if altitude is less than 200m
        if( jumperPos[1] < 20000.0f )
        {
            _scene->setTimeSpeedMultiplier( 1.0f );
        }
    }
}

/**
 * freefall instructor ¹3
 */

FreefallInstructor03::Invitation01::Invitation01(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(430), 10 )
{
}

FreefallInstructor03::Invitation02::Invitation02(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(431), 10 )
{
}

FreefallInstructor03::SetupAltimeter::SetupAltimeter(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(432), 
            Gameplay::iGameplay->getActionChannel( ::iaDecreaseWarningAltitude )->getInputActionHint(),
            Gameplay::iGameplay->getActionChannel( ::iaIncreaseWarningAltitude )->getInputActionHint()
        ).c_str(),
        1.0f
    )
{
    setLock( true );
}

void FreefallInstructor03::SetupAltimeter::onUpdateOrder(float dt)
{
    // base behaviour
    Notification::onUpdateOrder( dt );

    if( Altimeter::getAAAltitude( getJumper()->getScene()->getCareer() ) == 70000.0f )
    {
        setLock( false );
    }
}

FreefallInstructor03::TurnOnAltimeter::TurnOnAltimeter(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(433), 
            Gameplay::iGameplay->getActionChannel( ::iaAltimeterMode )->getInputActionHint()
        ).c_str(),
        1.0f
    )
{
    setLock( true );
}

void FreefallInstructor03::TurnOnAltimeter::onUpdateOrder(float dt)
{
    // base behaviour
    Notification::onUpdateOrder( dt );

    if( Altimeter::getAAMode( getJumper()->getScene()->getCareer() ) )
    {
        setLock( false );
    }
}

FreefallInstructor03::TurnOffAltimeter::TurnOffAltimeter(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(436), 
            Gameplay::iGameplay->getActionChannel( ::iaAltimeterMode )->getInputActionHint()
        ).c_str(),
        1.0f
    )
{
    setLock( true );
}

void FreefallInstructor03::TurnOffAltimeter::onUpdateOrder(float dt)
{
    // base behaviour
    Notification::onUpdateOrder( dt );

    if( !Altimeter::getAAMode( getJumper()->getScene()->getCareer() ) )
    {
        setLock( false );
    }
}

FreefallInstructor03::ExamineVariometer::ExamineVariometer(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(437), 10 )
{
}

FreefallInstructor03::ExamineHUD::ExamineHUD(Jumper* jumper) : 
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(438), 
            Gameplay::iGameplay->getActionChannel( ::iaSwitchHUDMode )->getInputActionHint()
        ).c_str(),
        10.0f
    )
{
}

FreefallInstructor03::FreefallInstructor03(Jumper* player) : FreefallInstructor( player )
{
    player->enablePhase( false );

    // create order
    _order = new Invitation01( player );
}

void FreefallInstructor03::onUpdateActivity(float dt)
{
    // base behaviour
    FreefallInstructor::onUpdateActivity( dt );

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
            _order = new SetupAltimeter( _jumper );
        }
        else if( orderIs(SetupAltimeter) )
        {
            delete _order;
            _order = new TurnOnAltimeter( _jumper );
        }
        else if( orderIs(TurnOnAltimeter) )
        {
            delete _order;
            _order = new Jump( _jumper );
        }
        else if( orderIs(Pull) )
        {
            delete _order;
            _order = new TurnOffAltimeter( _jumper );
        }
        else if( orderIs(TurnOffAltimeter) )
        {
            delete _order;
            _order = new ExamineVariometer( _jumper );
        }
        else if( orderIs(ExamineVariometer) )
        {
            delete _order;
            _order = new ExamineHUD( _jumper );
        }
        else
        {
            delete _order;
            _order = NULL;
        }
    }

    // check altitude
    if( _jumper->getPhase() != ::jpFlight )
    {
        Vector3f jumperPos = _jumper->getClump()->getFrame()->getPos();
        if( jumperPos[1] < 70000.0f && !orderIs(Pull) )
        {
            delete _order;
            _order = new Pull( _jumper );
        }
    }
    else
    {
        Vector3f jumperPos = _jumper->getClump()->getFrame()->getPos();
        // check altitude, disable time acceleration if altitude is less than 200m
        if( jumperPos[1] < 20000.0f )
        {
            _scene->setTimeSpeedMultiplier( 1.0f );
        }
    }
}

/**
 * linetwists instructor
 */

LinetwistsInstructor::Invitation01::Invitation01(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(442), 10 )
{
}

LinetwistsInstructor::Invitation02::Invitation02(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(443), 10 )
{
}

LinetwistsInstructor::FightLinetwists::FightLinetwists(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(444), 
            Gameplay::iGameplay->getActionChannel( ::iaLeftWarp )->getInputActionHint()
        ).c_str(),
        3.0f
    )
{
    setLock( true );
}

void LinetwistsInstructor::FightLinetwists::onUpdateOrder(float dt)
{
    // base behaviour
    Notification::onUpdateOrder( dt );

    if( getJumper()->getCanopySimulator()->getLinetwists() == 0.0f )
    {
        setLock( false );
    }
    else
    {
        setLock( true );
    }
}

LinetwistsInstructor::OtherLinetwists::OtherLinetwists(Jumper* jumper) :
    Instructor::Notification( 
        jumper,
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(445), 
            Gameplay::iGameplay->getActionChannel( ::iaRightWarp )->getInputActionHint()
        ).c_str(),
        10.0f
    )
{
}

LinetwistsInstructor::Farewell::Farewell(Jumper* jumper) : 
    Instructor::Notification( jumper, Gameplay::iLanguage->getUnicodeString(439), 10 )
{
}

LinetwistsInstructor::LinetwistsInstructor(Jumper* player) : StaticLineInstructor( player )
{
    // set switch
    _lineoverIsApplied = false;

    // disable phase change
    player->enablePhase( false );

    // create order
    _order = new Invitation01( player );
}

void LinetwistsInstructor::onUpdateActivity(float dt)
{
    // base behaviour
    StaticLineInstructor::onUpdateActivity( dt );

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
            _order = new FightLinetwists( _jumper );
        }
        else if( orderIs(FightLinetwists) )
        {
            delete _order;
            _order = new OtherLinetwists( _jumper );
        }
        else if( orderIs(OtherLinetwists) )
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

    // apply linetwists
    if( _jumper->getPhase() == ::jpFlight && !_lineoverIsApplied )
    {        
        if( _jumper->getCanopySimulator()->isOpened() )
        {
            _lineoverIsApplied = true;
            _jumper->getCanopySimulator()->setLinetwists( 360 );
        }
    }
}