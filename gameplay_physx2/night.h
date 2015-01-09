
#ifndef NIGHT_EVENT_INCLUDED
#define NIGHT_EVENT_INCLUDED

#include "headers.h"
#include "career.h"

/**
 * night event
 */

#define NIGHT_CLASS_NAME "21C8700000"

class Night : public Event
{
public:
    Night(Career* career, bool firstNight);
    Night(Career* career, TiXmlElement* element);
    virtual ~Night();
public:
    virtual void onSaveToXml(TiXmlElement* element);
    virtual const char* getClassName(void);
    virtual void onBeginEvent(Geoscape* geoscape);
    virtual void onEndEvent(Geoscape* geoscape);
    virtual void onUpdateEvent(Geoscape* geoscape, float dt);
    virtual void enumerateLocationIds(std::vector<unsigned int>& ids);
};

#endif