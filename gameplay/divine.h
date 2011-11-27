
#ifndef DIVINE_EVENT_INCLUDED
#define DIVINE_EVENT_INCLUDED

#include "headers.h"
#include "career.h"

/**
 * salary event
 */

#define DIVINE_CLASS_NAME "D151733537"

class Divine : public Event
{
private:
    void castBoogieEvent(void);
    void castFestivalEvent(void);
    void castClimbingEvent(void);
    void castSmokeballEvent(void);
    void castCommunityEvent(void);
public:
    Divine(Career* career);
    Divine(Career* career, TiXmlElement* element);
    virtual ~Divine();
public:
    virtual const char* getClassName(void);
    virtual void onBeginEvent(Geoscape* geoscape);
    virtual void onEndEvent(Geoscape* geoscape);
    virtual void onUpdateEvent(Geoscape* geoscape, float dt);
    virtual void enumerateLocationIds(std::vector<unsigned int>& ids);
};

#endif