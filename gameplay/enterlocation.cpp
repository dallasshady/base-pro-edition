
#include "headers.h"
#include "careercourse.h"
#include "gameplay.h"
#include "database.h"
#include "scene.h"
#include "../common/istring.h"

/**
 * class implementation
 */

EnterLocationDialog::EnterLocationDialog(Geoscape* geoscape, Location* location)
{
    _geoscape   = geoscape;
    _location   = location;
    _isComplete = false;

    // dialog window
    _window = Gameplay::iGui->createWindow( "EnterLocationDialog" ); assert( _window );
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );

    // location info
    database::LocationInfo* locationInfo = database::LocationInfo::getRecord( _location->getDatabaseId() );

    // setup location name
    gui::IGuiPanel* panel = _window->getPanel()->find( "LocationName" );
    assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(locationInfo->nameId) );

    // setup weather forecast
    panel = _window->getPanel()->find( "WeatherForecast" );
    assert( panel && panel->getStaticText() );
    const wchar_t* format = Gameplay::iLanguage->getUnicodeString(108);
    std::wstring text = wstrformat( 
        format, 
        _location->getWeatherForecast(),
        _location->getWindForecast(), 
        int( _location->getWindAmbient() ), 
        int( _location->getWindBlast() ) 
    );
    panel->getStaticText()->setText( text.c_str() );
}

EnterLocationDialog::~EnterLocationDialog()
{
    _window->getPanel()->release();
}

/**
 * CareerDialog implementation
 */

bool EnterLocationDialog::isComplete(void)
{
    return _isComplete;
}

void EnterLocationDialog::onGuiMessage(gui::Message* message)
{
    if( message->event == gui::onButtonClick )        
    {
        if( strcmp( message->origin->getName(), "Enter" ) == 0 )
        {
            database::LocationInfo* locationInfo = database::LocationInfo::getRecord( _location->getDatabaseId() );
            if( locationInfo->accessible )
            {
                // determine holding time in scene
                DateTime dateTime = _geoscape->getDateTime();
                float currentDayTime = HOURS_TO_MINUTES( float( dateTime.hour ) ) + float( dateTime.minute );
				float leftDayTime;
				// set time left to jump to max if were in freemode
				if ( Gameplay::iGameplay->_freeModeIsEnabled )
				{
					leftDayTime = 720.0f;
				}
				else
				{
					leftDayTime = maxHoldingTime - currentDayTime + HOURS_TO_MINUTES( 6 );
				}
                //float leftDayTime = maxHoldingTime - currentDayTime + HOURS_TO_MINUTES( 6 );
                if( leftDayTime > maxHoldingTime )
                {
                    leftDayTime = maxHoldingTime;
                }
                _geoscape->requestActivity( new Scene( _geoscape->getCareer(), _location, leftDayTime ) );
            }
            _isComplete = true;
        }
        else if( strcmp( message->origin->getName(), "Leave" ) == 0 )
        {
            _isComplete = true;
        }
    }
}