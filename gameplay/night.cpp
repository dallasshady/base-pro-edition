
#include "headers.h"
#include "night.h"
#include "geoscape.h"
#include "gameplay.h"

Night::Night(Career* career, bool firstNight) : Event( career, 3 )
{
    if( firstNight )
    {
        _timeTo = 0;
        _duration = HOURS_TO_MINUTES(6);
    }
    else
    {
        _timeTo = HOURS_TO_MINUTES(12);
        _duration = HOURS_TO_MINUTES(12);
    }
    _timeTo += _duration;

    // current time
    DateTime currentTime( career->getVirtues()->evolution.time );

    // correct lateness
    if( currentTime.hour == 6 )
    {
        _timeTo -= float( currentTime.minute );
        _duration -= float( currentTime.minute );
    }
    if( currentTime.hour == 5 )
    {
        _timeTo += 60 - float( currentTime.minute );
        _duration += 60 - float( currentTime.minute );
    }
}

Night::Night(Career* career, TiXmlElement* element) : Event( career, 3, element )
{
}

Night::~Night()
{
}

const char* Night::getClassName(void)
{
    return NIGHT_CLASS_NAME;
}

void Night::onSaveToXml(TiXmlElement* element)
{
}

void Night::onBeginEvent(Geoscape* geoscape)
{
    geoscape->addHistory( 
        Gameplay::iLanguage->getUnicodeString(434), 
        Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ) 
    );
}

void Night::onEndEvent(Geoscape* geoscape)
{
    geoscape->addHistory( 
        Gameplay::iLanguage->getUnicodeString(435), 
        Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ) 
    );
    _career->addEvent( new Night( _career, false ) );
}

void Night::onUpdateEvent(Geoscape* geoscape, float dt)
{
}

void Night::enumerateLocationIds(std::vector<unsigned int>& ids)
{
    ids.clear();
}