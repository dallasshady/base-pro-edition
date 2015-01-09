
#ifndef BASE_CLIMBING_EVENT_INCLUDED
#define BASE_CLIMBING_EVENT_INCLUDED

#include "headers.h"
#include "career.h"

/**
 * salary event
 */

#define CLIMBING_CLASS_NAME "C718B16300"

class Climbing : public Event
{
private:
    struct PrivateData
    {
        unsigned int locationId; // location where's Festival are come about
    }
    _privateData;
public:
    Climbing(Career* career, unsigned int locationId, float timeToBegin, float duration);
    Climbing(Career* career, TiXmlElement* element);
    virtual ~Climbing();
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