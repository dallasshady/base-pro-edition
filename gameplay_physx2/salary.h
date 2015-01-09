
#ifndef REGULAR_WORK_EVENT_INCLUDED
#define REGULAR_WORK_EVENT_INCLUDED

#include "headers.h"
#include "career.h"

/**
 * salary event
 */

#define REGULAR_WORK_CLASS_NAME "6F0EF9A356"

class RegularWork : public Event
{
private:
    struct PrivateData
    {
        float workTime;    // time, spended on work during this event
        float missingTime; // time, for works are missed during this event
    }
    _privateData;
public:
    RegularWork(Career* career, bool firstSalary);
    RegularWork(Career* career, TiXmlElement* element);
    virtual ~RegularWork();
public:
    virtual void onSaveToXml(TiXmlElement* element);
    virtual const char* getClassName(void);
    virtual void onBeginEvent(Geoscape* geoscape);
    virtual void onEndEvent(Geoscape* geoscape);
    virtual void onUpdateEvent(Geoscape* geoscape, float dt);
    virtual void enumerateLocationIds(std::vector<unsigned int>& ids);
};

#endif