
#include "headers.h"
#include "careercourse.h"
#include "gameplay.h"
#include "travel.h"
#include "database.h"
#include "../common/istring.h"

/**
 * class implementation
 */

TravelDialog::TravelDialog(Geoscape* geoscape, Location* location, float travelCost, float travelTime, float returnCost, float returnTime)
{
    assert( location );
    _isComplete = false;
    _geoscape = geoscape;
    _location = location;
    _travelCost = travelCost;
    _travelTime = travelTime;
    _returnCost = returnCost;
    _returnTime = returnTime;    
    _totalCost  = travelCost + returnCost;
    _window = Gameplay::iGui->createWindow( "TravelDialog" ); assert( _window );

    // align window
    Gameplay::iGui->getDesktop()->insertPanel( _window->getPanel() );
    _window->align( gui::atCenter, 0, gui::atCenter, 0 );

    // location info
    database::LocationInfo* locationInfo = database::LocationInfo::getRecord( _location->getDatabaseId() );
    const wchar_t* locationName = Gameplay::iLanguage->getUnicodeString(locationInfo->nameId);
    const wchar_t* timeFormat = Gameplay::iLanguage->getUnicodeString(103);
    const wchar_t* costFormat = Gameplay::iLanguage->getUnicodeString(104);

    // travel times
    DateTime travelDT( _travelTime ); travelDT.day--;
    DateTime returnDT( _returnTime ); returnDT.day--;
    
    // setup text
    gui::IGuiPanel* panel;
    panel = _window->getPanel()->find( "TravelDestination" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( locationName );
    panel = _window->getPanel()->find( "TravelTime" ); assert( panel && panel->getStaticText() );    
    panel->getStaticText()->setText( wstrformat( timeFormat, travelDT.day, travelDT.hour, travelDT.minute ).c_str() );
    panel = _window->getPanel()->find( "TravelCost" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( costFormat, _travelCost ).c_str() );
    panel = _window->getPanel()->find( "ReturnTime" ); assert( panel && panel->getStaticText() );    
    panel->getStaticText()->setText( wstrformat( timeFormat, returnDT.day, returnDT.hour, returnDT.minute ).c_str() );
    panel = _window->getPanel()->find( "ReturnCost" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( costFormat, _returnCost ).c_str() );
    panel = _window->getPanel()->find( "TotalCost" ); assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( wstrformat( costFormat, _totalCost ).c_str() );

    // setup text highlight
    if( _totalCost < _geoscape->getCareer()->getVirtues()->evolution.funds )
    {
        panel->getStaticText()->setTextColor( Vector4f( 0.0f, 0.5f, 0.0f, 1.0f ) );
    }
    else
    {
        panel->getStaticText()->setTextColor( Vector4f( 0.5f, 0.0f, 0.0f, 1.0f ) );
    }
}

TravelDialog::~TravelDialog()
{
    _window->getPanel()->release();
}

/**
 * CareerDialog implementation
 */

bool TravelDialog::isComplete(void)
{
    return _isComplete;
}

void TravelDialog::onGuiMessage(gui::Message* message)
{
    if( message->event == gui::onButtonClick )        
    {
        if( strcmp( message->origin->getName(), "Travel" ) == 0 )
        {
            if( _totalCost < _geoscape->getCareer()->getVirtues()->evolution.funds )
            {
                // remove player from location
                unsigned int departureId = _geoscape->getPlayerLocation()->getDatabaseId();
                _geoscape->getPlayerLocation()->setPlayer( false );
                // decrease funds
                _geoscape->getCareer()->getVirtues()->evolution.funds -= _travelCost;
                // add event
                _geoscape->getCareer()->addEvent( new Travel( _geoscape->getCareer(), departureId,  _location->getDatabaseId(), _travelTime ) );
                _isComplete = true;
            }
            else
            {
                gui::IGuiPanel* panel = _window->getPanel()->find( "Total" ); assert( panel && panel->getStaticText() );
                panel->getStaticText()->setText( Gameplay::iLanguage->getUnicodeString(105) );
                panel->getStaticText()->setTextColor( Vector4f( 0.5f, 0.0f, 0.0f, 1.0f ) );
            }
        }
        else if( strcmp( message->origin->getName(), "Cancel" ) == 0 )
        {
            _isComplete = true;
        }
    }
}