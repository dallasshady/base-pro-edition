
#include "headers.h"
#include "climbing.h"
#include "crypt.h"
#include "geoscape.h"
#include "database.h"
#include "gameplay.h"

Climbing::Climbing(Career* career, unsigned int locationId, float timeToBegin, float duration) : Event( career, 6 )
{
    _privateData.locationId = locationId;
    _timeTo = timeToBegin;
    _duration = duration;
    _timeTo += _duration;

    // overwrite event caption
    unsigned int climbingId = database::LocationInfo::getRecord( locationId )->climbingId;
    assert( climbingId );
    setEventCaption( Gameplay::iLanguage->getUnicodeString(climbingId) );
}

Climbing::Climbing(Career* career, TiXmlElement* element) : Event( career, 6, element )
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
    unsigned int climbingId = database::LocationInfo::getRecord( _privateData.locationId )->climbingId;
    assert( climbingId );
    setEventCaption( Gameplay::iLanguage->getUnicodeString(climbingId) );
}

Climbing::~Climbing()
{
}

const char* Climbing::getClassName(void)
{
    return CLIMBING_CLASS_NAME;
}

void Climbing::onBeginEvent(Geoscape* geoscape)
{
}

void Climbing::onEndEvent(Geoscape* geoscape)
{
}

void Climbing::onUpdateEvent(Geoscape* geoscape, float dt)
{
}

void Climbing::onSaveToXml(TiXmlElement* element)
{
    int cs = ::checksum( &_privateData, sizeof(PrivateData) );
    std::string data;
    ::encrypt( data, &_privateData, sizeof(PrivateData), _career->getName() );
    element->SetAttribute( "private_cs", cs );
    element->SetAttribute( "private_data", data.c_str() );
}

void Climbing::enumerateLocationIds(std::vector<unsigned int>& ids)
{
    ids.clear();
    ids.push_back( _privateData.locationId );
}