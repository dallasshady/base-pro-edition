
#ifndef BASE_COMMUNITY_EVENT_INCLUDED
#define BASE_COMMUNITY_EVENT_INCLUDED

#include "headers.h"
#include "career.h"

/**
 * community event
 */

#define COMMUNITY_CLASS_NAME "C099781713"

class CommunityEvent : public Event
{
private:
    struct PrivateData
    {
        unsigned int locationId; // location where's community event are come about
    }
    _privateData;
public:
    CommunityEvent(Career* career, unsigned int locationId, float timeToBegin, float duration);
    CommunityEvent(Career* career, TiXmlElement* element);
    virtual ~CommunityEvent();
public:
    virtual void onSaveToXml(TiXmlElement* element);
    virtual const char* getClassName(void);
    virtual void onBeginEvent(Geoscape* geoscape);
    virtual void onEndEvent(Geoscape* geoscape);
    virtual void onUpdateEvent(Geoscape* geoscape, float dt);
    virtual void enumerateLocationIds(std::vector<unsigned int>& ids);
public:
    // inlinez
    inline unsigned int getLocationId(void) { return _privateData.locationId; }
};

#endif