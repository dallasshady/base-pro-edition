
#include "headers.h"
#include "hud.h"

/**
 * actor abstracts
 */

void Timer::onUpdateActivity(float dt)
{
    if( _active ) _time += dt;

    // decompose time
    int mm = int( _time / 60 );
    int ss = int( _time - mm * 60 );
    int ms = int( 1000 * ( _time - ss - mm * 60 ) );

    // minutes
    int units = mm;

    // digits pickup
    int digit = units % 10;
    setIndicator( _window->getPanel()->find( "MM0" ), digit );
    
    units = units / 10;
    digit = units % 10;
    setIndicator( _window->getPanel()->find( "MM1" ), digit );

    // seconds
    units = ss;

    // digits pickup
    digit = units % 10;
    setIndicator( _window->getPanel()->find( "SS0" ), digit );
    
    units = units / 10;
    digit = units % 10;
    setIndicator( _window->getPanel()->find( "SS1" ), digit );

    // milliseconds
    units = ms;

    // digits pickup
    digit = units % 10;
    setIndicator( _window->getPanel()->find( "MSS0" ), digit );
    
    units = units / 10;
    digit = units % 10;
    setIndicator( _window->getPanel()->find( "MSS1" ), digit );

    units = units / 10;
    digit = units % 10;
    setIndicator( _window->getPanel()->find( "MSS2" ), digit );
}

void Timer::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    // start/stop event
    if( initiator == _parent )
    {
        if( eventId == _startEvent )
        {
            _active = true;
        }
        else if( eventId == _stopEvent )
        {
            _active = false;
        }
    }
    // HUD hide event
    if( eventId == EVENT_HUD_HIDE )
    {
        if( _window->getPanel()->getParent() )
        {
            _window->getPanel()->getParent()->removePanel( _window->getPanel() );
        }
    }
}

/**
 * class implementation
 */

Timer::Timer(Actor* parent, unsigned int startEvent, unsigned int stopEvent, const wchar_t* caption) : 
    HUD( parent )
{
    assert( caption );

    _window = Gameplay::iGui->createWindow( "Timer" ); assert( _window );
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atBottom, 0, gui::atRight, 0 );
    
    gui::IGuiPanel* panel = _window->getPanel()->find( "Caption" );
    assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( caption );

    _time = 0;
    _active = false;
    _startEvent = startEvent;
    _stopEvent  = stopEvent;
}

Timer::~Timer()
{
    Gameplay::iGui->getDesktop()->removePanel( _window->getPanel() );
    _window->getPanel()->release();
}