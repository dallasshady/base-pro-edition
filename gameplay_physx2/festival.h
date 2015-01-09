
#ifndef BASE_FESTIVAL_EVENT_INCLUDED
#define BASE_FESTIVAL_EVENT_INCLUDED

#include "headers.h"
#include "career.h"

/**
 * salary event
 */

#define FESTIVAL_CLASS_NAME "FE3715A200"

class Festival : public Event
{
private:
    struct PrivateData
    {
        unsigned int locationId; // location where's Festival are come about
    }
    _privateData;
public:
    Festival(Career* career, unsigned int locationId, float timeToBegin, float duration);
    Festival(Career* career, TiXmlElement* element);
    virtual ~Festival();
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