
#include "headers.h"
#include "hud.h"

const float acAMTimeout  = 1.0f;
const float acXWATimeout = 0.25f;

/**
 * actor abstracts
 */

void Altimeter::onUpdateActivity(float dt)
{
    _timeout -= dt;
    if( _timeout < 0 ) _timeout = 0;

    if( _timeout > 0 )
    {
        // reset HUD
        resetIndicator( _window->getPanel()->find( "Digit5" ) );
        resetIndicator( _window->getPanel()->find( "Digit4" ) );
        resetIndicator( _window->getPanel()->find( "Digit3" ) );
        resetIndicator( _window->getPanel()->find( "Digit2" ) );
        resetIndicator( _window->getPanel()->find( "Digit1" ) );
        resetIndicator( _window->getPanel()->find( "Digit0" ) );
        resetIndicator( _window->getPanel()->find( "AuDigit5" ) );
        resetIndicator( _window->getPanel()->find( "AuDigit4" ) );
        resetIndicator( _window->getPanel()->find( "AuDigit3" ) );
        resetIndicator( _window->getPanel()->find( "AuDigit2" ) );
        resetIndicator( _window->getPanel()->find( "AuDigit1" ) );
        resetIndicator( _window->getPanel()->find( "AuDigit0" ) );
    }
    else
    {
        // retrieve parent actor altitude
        Matrix4f parentPose = _parent->getPose();
        float altitude = parentPose[3][1];

        // draw signum digit
        setSignumIndicator( _window->getPanel()->find( "Digit5" ), altitude );        

        // absolute units conversion
		float meters = fabs( altitude ) * 0.01f ;
		int units = int( meters );
		if ( Gameplay::iGameplay->_feetModeIsEnabled )
		{
			float feet = fabs( altitude ) * 0.01f * 3.2808399f;
			units = int( feet );
		}
        // digits pickup
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

        units = units / 10;
        digit = units % 10;
        setIndicator( _window->getPanel()->find( "Digit4" ), digit );

        // update audible altimeter
        gui::IGuiPanel* auCaption = _window->getPanel()->find( "AuCaption" );
        assert( auCaption && auCaption->getStaticText() );
        if( _state->mode )
        {        
            auCaption->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(217) );
        }
        else
        {
            auCaption->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(216) );
        }      

	    // update altimeter units
  //      gui::IGuiPanel* auUnits = _window->getPanel()->find( "AuUnits" );
  //      assert( auUnits && auUnits->getStaticText() );
		//auUnits->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(213) );

		// audible altimeter signum indicator
        setSignumIndicator( _window->getPanel()->find( "AuDigit5" ), _state->altitude );

        // absolute units conversion
        meters = fabs( _state->altitude ) * 0.01f;
        units = int( meters );

        // digits pickup
        digit = units % 10;
        setIndicator( _window->getPanel()->find( "AuDigit0" ), digit );
    
        units = units / 10;
        digit = units % 10;
        setIndicator( _window->getPanel()->find( "AuDigit1" ), digit );

        units = units / 10;
        digit = units % 10;
        setIndicator( _window->getPanel()->find( "AuDigit2" ), digit );

        units = units / 10;
        digit = units % 10;
        setIndicator( _window->getPanel()->find( "AuDigit3" ), digit );

        units = units / 10;
        digit = units % 10;
        setIndicator( _window->getPanel()->find( "AuDigit4" ), digit );

        // helpers act
        _acAMTimeout  -= dt;
        _acIWATimeout -= dt;
        _acDWATimeout -= dt;
        if( _acAMTimeout < 0 ) _acAMTimeout = 0;
        if( _acIWATimeout < 0 ) _acIWATimeout = 0;
        if( _acDWATimeout < 0 ) _acDWATimeout = 0;

        // altimeter control
        ActionChannel* acAM  = Gameplay::iGameplay->getActionChannel( ::iaAltimeterMode );
        ActionChannel* acIWA = Gameplay::iGameplay->getActionChannel( ::iaIncreaseWarningAltitude );
        ActionChannel* acDWA = Gameplay::iGameplay->getActionChannel( ::iaDecreaseWarningAltitude );
        assert( acAM );
        assert( acIWA );
        assert( acDWA );

        if( !acAM->getTrigger() ) _acAMTimeout = 0;
        if( !acIWA->getTrigger() ) _acIWATimeout = 0;
        if( !acDWA->getTrigger() ) _acDWATimeout = 0;

        if( acAM->getTrigger() && _acAMTimeout == 0 )
        {
            _acAMTimeout = acAMTimeout;
            if( _state->mode )
            {
                _toneSound->release();
                _toneSound = NULL;
            }
            _state->mode = !_state->mode;
            if( _state->mode )
            {
                _toneSound = Gameplay::iAudio->createStaticSound( "./res/sounds/hud/aualt0.ogg" );
                _toneSound->setLoop( true );
            }
        }
        if( acIWA->getTrigger() && _acIWATimeout == 0 )
        {
            _acIWATimeout = acXWATimeout;
            _state->altitude += 1000;
        }
        if( acDWA->getTrigger() && _acDWATimeout == 0 )
        {
            _acDWATimeout = acXWATimeout;
            _state->altitude -= 1000;
        }

        // play tone sound
        if( altitude < _state->altitude && _toneSound )            
        {
            if( !_toneSound->isPlaying() ) _toneSound->play();
        }
        else if( _toneSound )
        {
            if( _toneSound->isPlaying() ) _toneSound->stop();
        }
    }
}

void Altimeter::onEvent(Actor* initiator, unsigned int eventId, void* eventData)
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
   
Altimeter::Altimeter(Actor* parent) : HUD( parent )
{
    // window
    _window = Gameplay::iGui->createWindow( "Altimeter" ); assert( _window );
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atTop, 0, gui::atLeft, 0 );

    // timeout
    _timeout = getCore()->getRandToolkit()->getUniform( 1, 3 );

    // retrieve altimeter state
    _gameData = _scene->getCareer()->getGameData( "ALTST" );
    if( _gameData == NULL )
    {
        _gameData = new GameData( sizeof( AltimeterState ) );
        _state = reinterpret_cast<AltimeterState*>( _gameData->getData() );
        _state->mode     = false;
        _state->altitude = 20000.0f;
        _scene->getCareer()->addGameData( "ALTST", _gameData );
    }
    else
    {
        _state = reinterpret_cast<AltimeterState*>( _gameData->getData() );
    }

    // reset altimeter control helpers
    _acAMTimeout  = 0;
    _acIWATimeout = 0;
    _acDWATimeout = 0;

    // create sound
    if( _state->mode )
    {    
        _toneSound = Gameplay::iAudio->createStaticSound( "./res/sounds/hud/aualt0.ogg" );    
        _toneSound->setLoop( true );
    }
    else
    {
        _toneSound = NULL;
    }
}

Altimeter::~Altimeter()
{
    _state->mode = false;
    if( _toneSound ) _toneSound->release();
    Gameplay::iGui->getDesktop()->removePanel( _window->getPanel() );
    _window->getPanel()->release();
}

void Altimeter::setAudibleAltimeter(Career* career, bool mode, float altitude)
{
    // retrieve altimeter state
    AltimeterState* state = NULL;
    GameData* gameData = career->getGameData( "ALTST" );    
    if( gameData == NULL )
    {
        gameData = new GameData( sizeof( AltimeterState ) );
        state = reinterpret_cast<AltimeterState*>( gameData->getData() );
        career->addGameData( "ALTST", gameData );
    }
    else
    {
        state = reinterpret_cast<AltimeterState*>( gameData->getData() );
    }

    // setup altimeter
    state->mode = mode;
    state->altitude = altitude;
}

bool Altimeter::getAAMode(Career* career)
{
    // retrieve altimeter state
    AltimeterState* state = NULL;
    GameData* gameData = career->getGameData( "ALTST" );    
    if( gameData == NULL )
    {
        gameData = new GameData( sizeof( AltimeterState ) );
        state = reinterpret_cast<AltimeterState*>( gameData->getData() );
        state->mode = false;
        state->altitude = 0.0f;
        career->addGameData( "ALTST", gameData );
    }
    else
    {
        state = reinterpret_cast<AltimeterState*>( gameData->getData() );
    }

    return state->mode;
}

float Altimeter::getAAAltitude(Career* career)
{
    // retrieve altimeter state
    AltimeterState* state = NULL;
    GameData* gameData = career->getGameData( "ALTST" );    
    if( gameData == NULL )
    {
        gameData = new GameData( sizeof( AltimeterState ) );
        state = reinterpret_cast<AltimeterState*>( gameData->getData() );
        state->mode = false;
        state->altitude = 0.0f;
        career->addGameData( "ALTST", gameData );
    }
    else
    {
        state = reinterpret_cast<AltimeterState*>( gameData->getData() );
    }

    return state->altitude;
}