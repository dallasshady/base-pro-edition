
#include "headers.h"
#include "smokeevent.h"
#include "crypt.h"
#include "geoscape.h"
#include "database.h"
#include "gameplay.h"

SmokeballEvent::SmokeballEvent(Career* career, unsigned int locationId, float timeToBegin, float duration) : Event( career, 7 )
{
    _privateData.locationId = locationId;
    _timeTo = timeToBegin;
    _duration = duration;
    _timeTo += _duration;

    // overwrite event caption
    unsigned int smokeballId = database::LocationInfo::getRecord( locationId )->smokeballId;
    assert( smokeballId );
    setEventCaption( Gameplay::iLanguage->getUnicodeString( smokeballId ) );
}

SmokeballEvent::SmokeballEvent(Career* career, TiXmlElement* element) : Event( career, 7, element )
{
    int cs;
    element->Attribute( "private_cs", &cs );
    std::string data = element->Attribute( "private_data" );
    ::decrypt( &_privateData, sizeof(PrivateData), data, career->getName() );
    if( cs != ::checksum( &_privateData, sizeof(PrivateData) ) )
    {
        throw ccor::Exception( "User database entry corrupted: \"%s\"! Cheating not allowed!", _career->getName() );
    }

    // overwrite event caption
    unsigned int smokeballId = database::LocationInfo::getRecord( _privateData.locationId )->smokeballId;
    assert( smokeballId );
    setEventCaption( Gameplay::iLanguage->getUnicodeString( smokeballId ) );
}

SmokeballEvent::~SmokeballEvent()
{
}

const char* SmokeballEvent::getClassName(void)
{
    return SMOKEBALL_CLASS_NAME;
}

void SmokeballEvent::onBeginEvent(Geoscape* geoscape)
{
}

void SmokeballEvent::onEndEvent(Geoscape* geoscape)
{
}

void SmokeballEvent::onUpdateEvent(Geoscape* geoscape, float dt)
{
}

void SmokeballEvent::onSaveToXml(TiXmlElement* element)
{
    int cs = ::checksum( &_privateData, sizeof(PrivateData) );
    std::string data;
    ::encrypt( data, &_privateData, sizeof(PrivateData), _career->getName() );
    element->SetAttribute( "private_cs", cs );
    element->SetAttribute( "private_data", data.c_str() );
}

void SmokeballEvent::enumerateLocationIds(std::vector<unsigned int>& ids)
{
    ids.clear();
    ids.push_back( _privateData.locationId );
}