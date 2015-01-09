
#ifndef TRAVEL_EVENT_INCLUDED
#define TRAVEL_EVENT_INCLUDED

#include "headers.h"
#include "career.h"

/**
 * travel event
 */

#define TRAVEL_CLASS_NAME "AEAFA00767"

class Travel : public Event
{
private:
    struct PrivateData
    {
        unsigned int departureId; // location of departure 
        unsigned int arrivatId;   // location of arrival 
    }
    _privateData;
public:
    Travel(Career* career, unsigned int departureId, unsigned int arrivalId, float time);
    Travel(Career* career, TiXmlElement* element);
    virtual ~Travel();
public:
    virtual void onSaveToXml(TiXmlElement* element);
    virtual const char* getClassName(void);
    virtual void onBeginEvent(Geoscape* geoscape);
    virtual void onEndEvent(Geoscape* geoscape);
    virtual void onUpdateEvent(Geoscape* geoscape, float dt);
    virtual void enumerateLocationIds(std::vector<unsigned int>& ids);
};

#endif