
#ifndef SMOKEBALL_EVENT_INCLUDED
#define SMOKEBALL_EVENT_INCLUDED

#include "headers.h"
#include "career.h"

/**
 * smokeball
 */

#define SMOKEBALL_CLASS_NAME "2604EBA770"

class SmokeballEvent : public Event
{
private:
    struct PrivateData
    {
        unsigned int locationId; // location where's Festival are come about
    }
    _privateData;
public:
    SmokeballEvent(Career* career, unsigned int locationId, float timeToBegin, float duration);
    SmokeballEvent(Career* career, TiXmlElement* element);
    virtual ~SmokeballEvent();
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