
#include "headers.h"
#include "travel.h"
#include "crypt.h"
#include "geoscape.h"
#include "gameplay.h"

Travel::Travel(Career* career, unsigned int departureId, unsigned int arrivalId, float time) : Event( career, 2 )
{
    _privateData.departureId = departureId;
    _privateData.arrivatId   = arrivalId;
    _timeTo = 0;
    _duration = time;
    _timeTo += _duration;
}

Travel::Travel(Career* career, TiXmlElement* element) : Event( career, 2, element )
{
    int cs;
    element->Attribute( "private_cs", &cs );
    std::string data = element->Attribute( "private_data" );
    ::decrypt( &_privateData, sizeof(PrivateData), data, career->getName() );
    if( cs != ::checksum( &_privateData, sizeof(PrivateData) ) )
    {
        throw ccor::Exception( "User database entry corrupted: \"%s\"! Cheating not allowed!", _career->getName() );
    }
}

Travel::~Travel()
{
}

const char* Travel::getClassName(void)
{
    return TRAVEL_CLASS_NAME;
}

void Travel::onBeginEvent(Geoscape* geoscape)
{
}

void Travel::onEndEvent(Geoscape* geoscape)
{
    assert( geoscape->getPlayerLocation() == NULL );
    Location* location = NULL;
    for( unsigned int i=0; i<geoscape->getNumLocations(); i++ )
    {
        if( geoscape->getLocation( i )->getDatabaseId() == _privateData.arrivatId )
        {
            location = geoscape->getLocation( i );
            break;
        }
    }
    assert( location );
    location->setPlayer( true );

    database::LocationInfo* locationInfo = database::LocationInfo::getRecord( location->getDatabaseId() );

    geoscape->addHistory( 
        wstrformat(
            Gameplay::iLanguage->getUnicodeString(67), 
            Gameplay::iLanguage->getUnicodeString(locationInfo->nameId)
        ).c_str(),
        Vector4f( 0.25f, 1.0f, 0.25f, 1.0f )
    );
}

void Travel::onUpdateEvent(Geoscape* geoscape, float dt)
{
}

void Travel::onSaveToXml(TiXmlElement* element)
{
    int cs = ::checksum( &_privateData, sizeof(PrivateData) );
    std::string data;
    ::encrypt( data, &_privateData, sizeof(PrivateData), _career->getName() );
    element->SetAttribute( "private_cs", cs );
    element->SetAttribute( "private_data", data.c_str() );
}

void Travel::enumerateLocationIds(std::vector<unsigned int>& ids)
{
    ids.clear();
    ids.push_back( _privateData.departureId );
    ids.push_back( _privateData.arrivatId );
}