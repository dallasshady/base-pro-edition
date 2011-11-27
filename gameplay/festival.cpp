
#include "headers.h"
#include "festival.h"
#include "crypt.h"
#include "geoscape.h"
#include "database.h"
#include "gameplay.h"

Festival::Festival(Career* career, unsigned int locationId, float timeToBegin, float duration) : Event( career, 5 )
{
    _privateData.locationId = locationId;
    _timeTo = timeToBegin;
    _duration = duration;
    _timeTo += _duration;

    // overwrite event caption
    unsigned int festivalId = database::LocationInfo::getRecord( locationId )->festivalId;
    assert( festivalId );
    setEventCaption( Gameplay::iLanguage->getUnicodeString(festivalId) );
}

Festival::Festival(Career* career, TiXmlElement* element) : Event( career, 5, element )
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
    unsigned int festivalId = database::LocationInfo::getRecord( _privateData.locationId )->festivalId;
    assert( festivalId );
    setEventCaption( Gameplay::iLanguage->getUnicodeString(festivalId) );
}

Festival::~Festival()
{
}

const char* Festival::getClassName(void)
{
    return FESTIVAL_CLASS_NAME;
}

void Festival::onBeginEvent(Geoscape* geoscape)
{
}

void Festival::onEndEvent(Geoscape* geoscape)
{
}

void Festival::onUpdateEvent(Geoscape* geoscape, float dt)
{
}

void Festival::onSaveToXml(TiXmlElement* element)
{
    int cs = ::checksum( &_privateData, sizeof(PrivateData) );
    std::string data;
    ::encrypt( data, &_privateData, sizeof(PrivateData), _career->getName() );
    element->SetAttribute( "private_cs", cs );
    element->SetAttribute( "private_data", data.c_str() );
}

void Festival::enumerateLocationIds(std::vector<unsigned int>& ids)
{
    ids.clear();
    ids.push_back( _privateData.locationId );
}