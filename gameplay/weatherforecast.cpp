
#include "headers.h"
#include "geoscape.h"
#include "weatherforecast.h"
#include "database.h"

WeatherForecast::WeatherForecast(Career* career) : Event( career, 1 ) 
{
    _timeTo = DAYS_TO_MINUTES(1);
    _duration = DAYS_TO_MINUTES(1);
    _flags = _flags | efSystem;
}

WeatherForecast::WeatherForecast(Career* career, TiXmlElement* element) : Event( career, 1, element )
{
}

WeatherForecast::~WeatherForecast()
{
}

const char* WeatherForecast::getClassName(void)
{
    return WEATHER_FORECAST_CLASS_NAME;
}

void WeatherForecast::onBeginEvent(Geoscape* geoscape)
{
    for( unsigned int i=0; i<geoscape->getNumLocations(); i++ )
    {        
        Location* location = geoscape->getLocation( i );
        float p = getCore()->getRandToolkit()->getUniform( 0, 1 );
        switch( location->getWeather() )
        {
        case wtSunny:
            // 50% for variable
            // 25% for cloudy
            if( p < 0.5 ) location->setWeather( wtVariable );
            else if( p < 0.75f ) location->setWeather( wtCloudy );
            break;
        case wtVariable:
            // 50% for cloudy
            // 25% for sunny
            // 15% for light rain
            if( p < 0.5 ) location->setWeather( wtCloudy );
            else if( p < 0.75f ) location->setWeather( wtSunny );
            else if( p < 0.90f ) location->setWeather( wtLightRain );
            break;
        case wtCloudy:
            // 25% for light rain
            // 25% for hard rain
            // 10% for thunder
            // 30% for variable
            if( p < 0.25 ) location->setWeather( wtLightRain );
            else if( p < 0.5f ) location->setWeather( wtHardRain );
            else if( p < 0.6f ) location->setWeather( wtThunder );
            else if( p < 0.9f ) location->setWeather( wtVariable );
            break;
        case wtLightRain:
            // 45% for cloudy
            // 45% for variable
            if( p < 0.45 ) location->setWeather( wtCloudy );
            else if( p < 0.9f ) location->setWeather( wtVariable );
            break;
        case wtHardRain:
            // 45% for cloudy
            // 45% for variable
            if( p < 0.45 ) location->setWeather( wtCloudy );
            else if( p < 0.9f ) location->setWeather( wtVariable );
            break;
        case wtThunder:
            // 45% for cloudy
            // 45% for variable
            if( p < 0.45 ) location->setWeather( wtCloudy );
            else if( p < 0.9f ) location->setWeather( wtVariable );
            break;
        }

        // setup wind
        p = getCore()->getRandToolkit()->getUniform( 0, 1 );
        Vector3f dir( 
            getCore()->getRandToolkit()->getUniform( -1, 1 ),
            0,
            getCore()->getRandToolkit()->getUniform( -1, 1 )
        );
        dir.normalize();
        float calmAmbient   = getCore()->getRandToolkit()->getUniform( database::windCalmAmbientMin, database::windCalmAmbientMax );
        float calmBlast     = getCore()->getRandToolkit()->getUniform( database::windCalmBlastMin, database::windCalmBlastMax );
        float lightAmbient  = getCore()->getRandToolkit()->getUniform( database::windLightAmbientMin, database::windLightAmbientMax );
        float lightBlast    = getCore()->getRandToolkit()->getUniform( database::windLightBlastMin, database::windLightBlastMax );
        float mediumAmbient = getCore()->getRandToolkit()->getUniform( database::windMediumAmbientMin, database::windMediumAmbientMax );
        float mediumBlast   = getCore()->getRandToolkit()->getUniform( database::windMediumBlastMin, database::windMediumBlastMax );
        float strongAmbient = getCore()->getRandToolkit()->getUniform( database::windStrongAmbientMin, database::windStrongAmbientMax );
        float strongBlast   = getCore()->getRandToolkit()->getUniform( database::windStrongBlastMin, database::windStrongBlastMax );
        switch( location->getWeather() )
        {
        case wtSunny:
            // 40% calm
            // 40% light wind
            // 10% medium wind
            // 10% strong wind
            if( p < 0.4 ) location->setWind( dir, calmAmbient, calmBlast );
            else if( p < 0.8 ) location->setWind( dir, lightAmbient, lightBlast );
            else if( p < 0.9 ) location->setWind( dir, mediumAmbient, mediumBlast );
            else location->setWind( dir, strongAmbient, strongBlast );
            break;
        case wtVariable:
            // 10% calm
            // 40% light wind
            // 40% medium wind
            // 10% strong wind
            if( p < 0.1 ) location->setWind( dir, calmAmbient, calmBlast );
            else if( p < 0.5 ) location->setWind( dir, lightAmbient, lightBlast );
            else if( p < 0.9 ) location->setWind( dir, mediumAmbient, mediumBlast );
            else location->setWind( dir, strongAmbient, strongBlast );
            break;
        case wtCloudy:
            // 10% calm
            // 30% light wind
            // 30% medium wind
            // 30% strong wind
            if( p < 0.1 ) location->setWind( dir, calmAmbient, calmBlast );
            else if( p < 0.4 ) location->setWind( dir, lightAmbient, lightBlast );
            else if( p < 0.7 ) location->setWind( dir, mediumAmbient, mediumBlast );
            else location->setWind( dir, strongAmbient, strongBlast );
            break;
        case wtLightRain:
            // 10% calm
            // 30% light wind
            // 30% medium wind
            // 30% strong wind
            if( p < 0.1 ) location->setWind( dir, calmAmbient, calmBlast );
            else if( p < 0.4 ) location->setWind( dir, lightAmbient, lightBlast );
            else if( p < 0.7 ) location->setWind( dir, mediumAmbient, mediumBlast );
            else location->setWind( dir, strongAmbient, strongBlast );
            break;
        case wtHardRain:            
            // 10% calm
            // 10% light wind
            // 40% medium wind
            // 40% strong wind
            if( p < 0.1 ) location->setWind( dir, calmAmbient, calmBlast );
            else if( p < 0.2 ) location->setWind( dir, lightAmbient, lightBlast );
            else if( p < 0.6 ) location->setWind( dir, mediumAmbient, mediumBlast );
            else location->setWind( dir, strongAmbient, strongBlast );
            break;
        case wtThunder:
            // 10% calm
            // 10% light wind
            // 20% medium wind
            // 60% strong wind
            if( p < 0.1 ) location->setWind( dir, calmAmbient, calmBlast );
            else if( p < 0.2 ) location->setWind( dir, lightAmbient, lightBlast );
            else if( p < 0.4 ) location->setWind( dir, mediumAmbient, mediumBlast );
            else location->setWind( dir, strongAmbient, strongBlast );
            break;
        }
    }
}

void WeatherForecast::onEndEvent(Geoscape* geoscape)
{
    _career->addEvent( new WeatherForecast( _career ) );
}

void WeatherForecast::onUpdateEvent(Geoscape* geoscape, float dt)
{
}

void WeatherForecast::enumerateLocationIds(std::vector<unsigned int>& ids)
{
    ids.clear();
    for( unsigned int i=0; i<database::LocationInfo::getNumRecords(); i++ )
    {
        ids.push_back( i );
    }
}