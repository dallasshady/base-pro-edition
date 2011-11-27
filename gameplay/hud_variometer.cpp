
#include "headers.h"
#include "hud.h"

/**
 * actor abstracts
 */

void Variometer::onUpdateActivity(float dt)
{
    _timeout -= dt;
    if( _timeout < 0 ) _timeout = 0;

    if( _timeout > 0 )
    {
		// vertical
        resetIndicator( _window->getPanel()->find( "Digit4" ) );
        resetIndicator( _window->getPanel()->find( "Digit3" ) );
        resetIndicator( _window->getPanel()->find( "Digit2" ) );
        resetIndicator( _window->getPanel()->find( "Digit1" ) );
        resetIndicator( _window->getPanel()->find( "Digit0" ) );
		// horizontal
        resetIndicator( _window->getPanel()->find( "Digit14" ) );
        resetIndicator( _window->getPanel()->find( "Digit13" ) );
        resetIndicator( _window->getPanel()->find( "Digit12" ) );
        resetIndicator( _window->getPanel()->find( "Digit11" ) );
        resetIndicator( _window->getPanel()->find( "Digit10" ) );
		// glide ratio
        resetIndicator( _window->getPanel()->find( "Digit24" ) );
        resetIndicator( _window->getPanel()->find( "Digit23" ) );
        resetIndicator( _window->getPanel()->find( "Digit22" ) );
        resetIndicator( _window->getPanel()->find( "Digit21" ) );
        resetIndicator( _window->getPanel()->find( "Digit20" ) );
    }
    else
    {
        // retrieve parent actor fall rate
        Vector3f parentVel = _parent->getVel();
        float fall = parentVel[1];

        // retrieve parent horizontal speed
		Vector3f vel = _parent->getVel();
		vel[1] = 0;
		float vel_horizontal = vel.length();

		// retrieve glide ratio
		float glide_ratio = 0.0f;
		if (fabs(fall) > 0.001f && vel_horizontal > 0.001f) {
			glide_ratio = vel_horizontal / fall;
		}

        // draw signum digit
        setSignumIndicator( _window->getPanel()->find( "Digit4" ), fall );    
		setSignumIndicator( _window->getPanel()->find( "Digit14" ), 1 );   // absolute value - always positive
		setSignumIndicator( _window->getPanel()->find( "Digit24" ), 1 );   // absolute value - always positive

        // absolute units conversion (vertical)
        float meters = fabs( fall ) * 0.1f;
        int units = int( meters );
		// Are we using feet units?
		if ( Gameplay::iGameplay->_feetModeIsEnabled )
		{
			float feet = fabs( fall ) * 0.1f * 3.2808399f;
			units = int( feet );
		}

        // absolute units conversion (horizontal)
        float metersH = fabs( vel_horizontal ) * 0.1f;
        int unitsH = int( metersH );
		// Are we using feet units?
		if ( Gameplay::iGameplay->_feetModeIsEnabled )
		{
			float feetH = fabs( vel_horizontal ) * 0.1f * 3.2808399f;
			unitsH = int( feetH );
		}

        // not needed: absolute units conversion (glide ratio)
        float metersG = fabs( glide_ratio ) * 100;
        int unitsG = int( metersG );

        // digits pickup (vertical)
        int digit = units % 10;
        setIndicator( _window->getPanel()->find( "Digit0" ), digit );
    
        units = units / 10;
        digit = units % 10;
        setIndicator( _window->getPanel()->find( "Digit1" ), digit );

        units = units / 10;
        digit = units % 10;
        setIndicator( _window->getPanel()->find( "Digit2" ), digit );

        units = units / 10;
        digit = units % 10;
        setIndicator( _window->getPanel()->find( "Digit3" ), digit );

        // digits pickup (horizontal)
        int digitH = unitsH % 10;
        setIndicator( _window->getPanel()->find( "Digit10" ), digitH );
    
        unitsH = unitsH / 10;
        digitH = unitsH % 10;
        setIndicator( _window->getPanel()->find( "Digit11" ), digitH );

        unitsH = unitsH / 10;
        digitH = unitsH % 10;
        setIndicator( _window->getPanel()->find( "Digit12" ), digitH );

        unitsH = unitsH / 10;
        digitH = unitsH % 10;
        setIndicator( _window->getPanel()->find( "Digit13" ), digitH );

        // digits pickup (glide ratio)
        int digitG = unitsG % 10;
        setIndicator( _window->getPanel()->find( "Digit20" ), digitG );
    
        unitsG = unitsG / 10;
        digitG = unitsG % 10;
        setIndicator( _window->getPanel()->find( "Digit21" ), digitG );

        unitsG = unitsG / 10;
        digitG = unitsG % 10;
        setIndicator( _window->getPanel()->find( "Digit22" ), digitG );

        unitsG = unitsG / 10;
        digitG = unitsG % 10;
        setIndicator( _window->getPanel()->find( "Digit23" ), digitG );
    }
}

void Variometer::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
{
    if( eventId == EVENT_HUD_HIDE )
    {
        if( _window->getPanel()->getParent() )
        {
            _window->getPanel()->getParent()->removePanel( _window->getPanel() );
        }
    }
}

/**
 * class implemetation
 */
   
Variometer::Variometer(Actor* parent) : HUD( parent )
{
    // window
    _window = Gameplay::iGui->createWindow( "Variometer" ); assert( _window );
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atTop, 0, gui::atRight, 0 );

    // timeout
    _timeout = getCore()->getRandToolkit()->getUniform( 1, 3 );
}

Variometer::~Variometer()
{
    Gameplay::iGui->getDesktop()->removePanel( _window->getPanel() );
    _window->getPanel()->release();
}