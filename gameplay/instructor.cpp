
#include "headers.h"
#include "instructor.h"
#include "imath.h"

/**
 * abstract order
 */

Instructor::Order::Order(Jumper* jumper)
{
    // memorize jumper
    _jumper = jumper;

    // create window
    _window = Gameplay::iGui->createWindow( "Instructor" ); assert( _window );
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );
}

Instructor::Order::~Order()
{
    _window->getPanel()->release();
}

void Instructor::Order::setMessage(const wchar_t* message)
{
    // setup message
    gui::IGuiPanel* panel = _window->getPanel()->find( "Message" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( message );
    panel = _window->getPanel()->find( "MessageShadow" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( message );
}

/**
 * notification order
 */

Instructor::Notification::Notification(Jumper* jumper, const wchar_t* message, float delay) :
    Order( jumper )
{
    _lock = false;
    _timeLeft = delay;
    setMessage( message );
}

/**
 * notification order
 */

float Instructor::Notification::getMaximalScore(void)
{
    return 0;
}

float Instructor::Notification::getAchievedScore(void)
{
    return 0;
}

void Instructor::Notification::onUpdateOrder(float dt)
{
    if( !_lock ) _timeLeft -= dt;
}

bool Instructor::Notification::isExecuted(void)
{
    return _timeLeft <= 0;
}

/**
 * Instructor
 */

Instructor::Instructor(Jumper* jumper) : Goal( jumper )
{
    _jumper = jumper;
    _order  = NULL;
}
    
Instructor::~Instructor()
{
    // delete order
    if( _order ) delete _order;
}

void Instructor::onUpdateActivity(float dt)
{    
    // update current order
    if( _order ) _order->onUpdateOrder( dt );

    // base behaviour
    Goal::onUpdateActivity( dt );
}

void Instructor::onUpdatePhysics()
{
    // base behaviour
    Goal::onUpdatePhysics();
}

const wchar_t* Instructor::getGoalName(void)
{
    return Gameplay::iLanguage->getUnicodeString( 236 );
}

void Instructor::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    // base behaviour
    Goal::onEvent( initiator, eventId, eventData );
}