
#include "headers.h"
#include "community.h"
#include "crypt.h"
#include "geoscape.h"
#include "database.h"
#include "gameplay.h"

CommunityEvent::CommunityEvent(Career* career, unsigned int locationId, float timeToBegin, float duration) : Event( career, 8 )
{
    _privateData.locationId = locationId;
    _timeTo = timeToBegin;
    _duration = duration;
    _timeTo += _duration;

    // overwrite event caption
    unsigned int communityId = database::LocationInfo::getRecord( locationId )->communityId;
    assert( communityId );
    setEventCaption( Gameplay::iLanguage->getUnicodeString(communityId) );
}

CommunityEvent::CommunityEvent(Career* career, TiXmlElement* element) : Event( career, 8, element )
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
    unsigned int communityId = database::LocationInfo::getRecord( _privateData.locationId )->communityId;
    assert( communityId );
    setEventCaption( Gameplay::iLanguage->getUnicodeString(communityId) );
}

CommunityEvent::~CommunityEvent()
{
}

const char* CommunityEvent::getClassName(void)
{
    return COMMUNITY_CLASS_NAME;
}

void CommunityEvent::onBeginEvent(Geoscape* geoscape)
{
}

void CommunityEvent::onEndEvent(Geoscape* geoscape)
{
}

void CommunityEvent::onUpdateEvent(Geoscape* geoscape, float dt)
{
}

void CommunityEvent::onSaveToXml(TiXmlElement* element)
{
    int cs = ::checksum( &_privateData, sizeof(PrivateData) );
    std::string data;
    ::encrypt( data, &_privateData, sizeof(PrivateData), _career->getName() );
    element->SetAttribute( "private_cs", cs );
    element->SetAttribute( "private_data", data.c_str() );
}

void CommunityEvent::enumerateLocationIds(std::vector<unsigned int>& ids)
{
    ids.clear();
    ids.push_back( _privateData.locationId );
}