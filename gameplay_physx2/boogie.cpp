
#include "headers.h"
#include "boogie.h"
#include "crypt.h"
#include "geoscape.h"
#include "database.h"
#include "gameplay.h"

Boogie::Boogie(Career* career, unsigned int locationId, float timeToBegin, float duration) : Event( career, 4 )
{
    _privateData.locationId = locationId;
    _timeTo = timeToBegin;
    _duration = duration;
    _timeTo += _duration;

    // overwrite event caption
    unsigned int boogieId = database::LocationInfo::getRecord( locationId )->boogieId;
    assert( boogieId );
    setEventCaption( Gameplay::iLanguage->getUnicodeString(boogieId) );
}

Boogie::Boogie(Career* career, TiXmlElement* element) : Event( career, 4, element )
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
    unsigned int boogieId = database::LocationInfo::getRecord( _privateData.locationId )->boogieId;
    assert( boogieId );
    setEventCaption( Gameplay::iLanguage->getUnicodeString(boogieId) );
}

Boogie::~Boogie()
{
}

const char* Boogie::getClassName(void)
{
    return BOOGIE_CLASS_NAME;
}

void Boogie::onBeginEvent(Geoscape* geoscape)
{
}

void Boogie::onEndEvent(Geoscape* geoscape)
{
}

void Boogie::onUpdateEvent(Geoscape* geoscape, float dt)
{
}

void Boogie::onSaveToXml(TiXmlElement* element)
{
    int cs = ::checksum( &_privateData, sizeof(PrivateData) );
    std::string data;
    ::encrypt( data, &_privateData, sizeof(PrivateData), _career->getName() );
    element->SetAttribute( "private_cs", cs );
    element->SetAttribute( "private_data", data.c_str() );
}

void Boogie::enumerateLocationIds(std::vector<unsigned int>& ids)
{
    ids.clear();
    ids.push_back( _privateData.locationId );
}