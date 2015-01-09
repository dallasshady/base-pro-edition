
#ifndef BASE_BOOGIE_EVENT_INCLUDED
#define BASE_BOOGIE_EVENT_INCLUDED

#include "headers.h"
#include "career.h"

/**
 * salary event
 */

#define BOOGIE_CLASS_NAME "B00613BA23"

class Boogie : public Event
{
private:
    struct PrivateData
    {
        unsigned int locationId; // location where's Boogie are come about
    }
    _privateData;
public:
    Boogie(Career* career, unsigned int locationId, float timeToBegin, float duration);
    Boogie(Career* career, TiXmlElement* element);
    virtual ~Boogie();
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