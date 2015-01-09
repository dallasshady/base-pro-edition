
#include "headers.h"
#include "career.h"
#include "crypt.h"
#include "database.h"
#include "gameplay.h"
#include "salary.h"
#include "weatherforecast.h"
#include "travel.h"
#include "night.h"
#include "divine.h"
#include "boogie.h"
#include "festival.h"
#include "climbing.h"
#include "smokeevent.h"
#include "community.h"
#include "../common/istring.h"

/**
 * class implementation
 */

Event::Event(Career* career, unsigned int databaseId)
{
    _career     = career;
    _databaseId = databaseId;
    _flags      = 0;
    
    initializeGui();
}

Event::Event(Career* career, unsigned int databaseId, TiXmlElement* element)
{
    _career     = career;
    _databaseId = databaseId;

    initializeGui();
    
    int cs;
    std::string data;

    // read duration
    data = element->Attribute( "duration_data" );
    element->Attribute( "duration_cs", &cs );
    ::decrypt( &_duration, sizeof(unsigned int), data, _career->getName() );
    if( cs != ::checksum( &_duration, sizeof(unsigned int) ) )
    {
        throw ccor::Exception( "User database entry corrupted: \"%s\"! Cheating not allowed!", _career->getName() );
    }

    // read time to
    data = element->Attribute( "timeto_data" );
    element->Attribute( "timeto_cs", &cs );
    ::decrypt( &_timeTo, sizeof(unsigned int), data, _career->getName() );
    if( cs != ::checksum( &_timeTo, sizeof(unsigned int) ) )
    {
        throw ccor::Exception( "User database entry corrupted: \"%s\"! Cheating not allowed!", _career->getName() );
    }

    // read flags
    data = element->Attribute( "flags_data" );
    element->Attribute( "flags_cs", &cs );
    ::decrypt( &_flags, sizeof(unsigned int), data, _career->getName() );
    if( cs != ::checksum( &_flags, sizeof(unsigned int) ) )
    {
        throw ccor::Exception( "User database entry corrupted: \"%s\"! Cheating not allowed!", _career->getName() );
    }
}

Event::~Event()
{
    _window->getPanel()->release();
    _buttonTexture->release();
}

/**
 * protected behaviour
 */

void Event::setEventCaption(const wchar_t* caption)
{
    // setup static text caption
    gui::IGuiPanel* panel = _window->getPanel()->find( "EventDescription" );
    assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( caption );
    panel = _window->getPanel()->find( "EventDescriptionShadow" );
    assert( panel && panel->getStaticText() );
    panel->getStaticText()->setText( caption );
}

void Event::initializeGui(void)
{
    // event info
    database::EventInfo* eventInfo = database::EventInfo::getRecord( _databaseId );

    // create textures
    _buttonTexture = Gameplay::iEngine->getTexture( eventInfo->textureName );
    if( !_buttonTexture ) _buttonTexture = Gameplay::iEngine->createTexture( eventInfo->resourceName );
    assert( _buttonTexture );
    _buttonTexture->addReference();    

    // create window
    _window = Gameplay::iGui->createWindow( "CareerEvent" );
    _window->getPanel()->setName( strformat( "EventWindow_%X", (unsigned int)(this) ).c_str() );

    // setup button texture
    gui::IGuiPanel* eventImage = _window->getPanel()->find( "EventImage" );
    assert( eventImage && eventImage->getButton() );
    eventImage->setTexture( _buttonTexture );
    eventImage->setTextureRect( gui::Rect( 0,0,_buttonTexture->getWidth(),_buttonTexture->getHeight() ) );

    // setup shadow texture
    gui::IGuiPanel* eventImageShadow = _window->getPanel()->find( "EventImageShadow" );
    assert( eventImageShadow && eventImageShadow->getWindow() );
    eventImageShadow->setTexture( _buttonTexture );
    eventImageShadow->setTextureRect( gui::Rect( 0,0,_buttonTexture->getWidth(),_buttonTexture->getHeight() ) );

    // setup default caption for this event
    // this may be changed in code of derived constructors
    setEventCaption( Gameplay::iLanguage->getUnicodeString( eventInfo->shortId ) );
}

/**
 * serialization
 */

void Event::saveToXml(TiXmlNode* node)
{
    TiXmlElement element( "event" );
    element.SetAttribute( "class", getClassName() );

    int cs;
    std::string data;

    // write duration
    cs = ::checksum( &_duration, sizeof(unsigned int) );
    ::encrypt( data, &_duration, sizeof(unsigned int), _career->getName() );
    element.SetAttribute( "duration_cs", cs );
    element.SetAttribute( "duration_data", data.c_str() );

    // write time to
    cs = ::checksum( &_timeTo, sizeof(unsigned int) );
    ::encrypt( data, &_timeTo, sizeof(unsigned int), _career->getName() );
    element.SetAttribute( "timeto_cs", cs );
    element.SetAttribute( "timeto_data", data.c_str() );

    // write flags
    cs = ::checksum( &_flags, sizeof(unsigned int) );
    ::encrypt( data, &_flags, sizeof(unsigned int), _career->getName() );
    element.SetAttribute( "flags_cs", cs );
    element.SetAttribute( "flags_data", data.c_str() );

    // write event specific
    onSaveToXml( &element );

    // finalize
    node->InsertEndChild( element );
}

Event* Event::createFromXml(Career* career, TiXmlElement* element)
{
    assert( strcmp( element->Value(), "event" ) == 0 );

    if( strcmp( element->Attribute( "class" ), REGULAR_WORK_CLASS_NAME ) == 0 )
    {
        return new RegularWork( career, element );
    }
    else if( strcmp( element->Attribute( "class" ), WEATHER_FORECAST_CLASS_NAME ) == 0 )
    {
        return new WeatherForecast( career, element );
    }
    else if( strcmp( element->Attribute( "class" ), DIVINE_CLASS_NAME ) == 0 )
    {
        return new Divine( career, element );
    }
    else if( strcmp( element->Attribute( "class" ), TRAVEL_CLASS_NAME ) == 0 )
    {
        return new Travel( career, element );
    }
    else if( strcmp( element->Attribute( "class" ), NIGHT_CLASS_NAME ) == 0 )
    {
        return new Night( career, element );
    }
    else if( strcmp( element->Attribute( "class" ), BOOGIE_CLASS_NAME ) == 0 )
    {
        return new Boogie( career, element );
    }
    else if( strcmp( element->Attribute( "class" ), FESTIVAL_CLASS_NAME ) == 0 )
    {
        return new Festival( career, element );
    }
    else if( strcmp( element->Attribute( "class" ), CLIMBING_CLASS_NAME ) == 0 )
    {
        return new Climbing( career, element );
    }
    else if( strcmp( element->Attribute( "class" ), SMOKEBALL_CLASS_NAME ) == 0 )
    {
        return new SmokeballEvent( career, element );
    }
    else if( strcmp( element->Attribute( "class" ), COMMUNITY_CLASS_NAME ) == 0 )
    {
        return new CommunityEvent( career, element );
    }
    else
    {
        return NULL;
    }
}

/**
 * dumbs
 */

void Event::onSaveToXml(TiXmlElement* element)
{
}