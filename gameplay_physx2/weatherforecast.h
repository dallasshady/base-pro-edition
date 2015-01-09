
#ifndef WEATHER_FORECAST_EVENT_INCLUDED
#define WEATHER_FORECAST_EVENT_INCLUDED

#include "headers.h"
#include "career.h"

/**
 * salary event
 */

#define WEATHER_FORECAST_CLASS_NAME "89FEFFFEFF"

class WeatherForecast : public Event
{
public:
    WeatherForecast(Career* career);
    WeatherForecast(Career* career, TiXmlElement* element);
    virtual ~WeatherForecast();
public:
    virtual const char* getClassName(void);
    virtual void onBeginEvent(Geoscape* geoscape);
    virtual void onEndEvent(Geoscape* geoscape);
    virtual void onUpdateEvent(Geoscape* geoscape, float dt);
    virtual void enumerateLocationIds(std::vector<unsigned int>& ids);
};

#endif