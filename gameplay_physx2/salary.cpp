
#include "headers.h"
#include "salary.h"
#include "crypt.h"
#include "geoscape.h"
#include "gameplay.h"

RegularWork::RegularWork(Career* career, bool firstSalary) : Event( career, 0 ) 
{    
    if( firstSalary )    
    {
        // salary are happen each monday in 8:00
        // it is also represents the beginning of a new workweek
        _timeTo = 0;
    }
    else
    {
        _timeTo = DAYS_TO_MINUTES(2);
    }
    _duration = DAYS_TO_MINUTES(5);
    _timeTo += _duration;
    _privateData.workTime = 0.0f;
    _privateData.missingTime = 0.0f;
}

RegularWork::RegularWork(Career* career, TiXmlElement* element) : Event( career, 0, element )
{
    int cs;
    element->Attribute( "private_cs", &cs );
    std::string data = element->Attribute( "private_data" );
    ::decrypt( &_privateData, sizeof(PrivateData), data, career->getName() );
    if( cs != ::checksum( &_privateData, sizeof(PrivateData) ) )
    {
        throw ccor::Exception( "User database entry corrupted: \"%s\"! Cheating not allowed!", _career->getName() );
    }
}

RegularWork::~RegularWork()
{
}

void RegularWork::onSaveToXml(TiXmlElement* element)
{
    int cs = ::checksum( &_privateData, sizeof(PrivateData) );
    std::string data;
    ::encrypt( data, &_privateData, sizeof(PrivateData), _career->getName() );
    element->SetAttribute( "private_cs", cs );
    element->SetAttribute( "private_data", data.c_str() );
}

const char* RegularWork::getClassName(void)
{
    return REGULAR_WORK_CLASS_NAME;
}

void RegularWork::onBeginEvent(Geoscape* geoscape)
{
    geoscape->addHistory( 
        Gameplay::iLanguage->getUnicodeString(109), 
        Vector4f( 0.25f, 1.0f, 0.25f, 1.0f ) 
    );
}

void RegularWork::onEndEvent(Geoscape* geoscape)
{   
    float workDays    = _privateData.workTime / ( 24.0f * 60.0f );
    float missingDays = _privateData.missingTime / ( 24.0f * 60.0f );

    // update salary
    float salary = _career->getVirtues()->getSalary();
    float income = workDays * salary / 5.0f;
    _career->getVirtues()->evolution.funds += income;
    geoscape->addHistory( wstrformat( Gameplay::iLanguage->getUnicodeString(290), income ).c_str(), Vector4f( 0.25,1,0.25,1 ) );

    // update promotion
    unsigned int currentPromotion = unsigned int( _career->getVirtues()->getPromotion() );
    _career->getVirtues()->evolution.workTime += workDays;
    _career->getVirtues()->evolution.workTime -= missingDays * 3;
    if( _career->getVirtues()->evolution.workTime < 0 )
    {
        _career->getVirtues()->evolution.workTime = 0;
    }
    unsigned int newPromotion = unsigned int( _career->getVirtues()->getPromotion() );
    if( newPromotion > currentPromotion )
    {
        geoscape->addHistory( Gameplay::iLanguage->getUnicodeString(291), Vector4f( 0.25,1,0.25,1 ) );
    }
    else if( newPromotion < currentPromotion )
    {
        geoscape->addHistory( Gameplay::iLanguage->getUnicodeString(292), Vector4f( 1,0.25,0.25,1 ) );
    }

    _career->addEvent( new RegularWork( _career, false ) );
}

void RegularWork::onUpdateEvent(Geoscape* geoscape, float dt)
{    
    if( ( _flags & ::efActive ) )
    {
        // check player in home location
        if( geoscape->getHomeLocation()->getPlayer() )
        {
            _privateData.workTime += dt;
        }
        else
        {
            _privateData.missingTime += dt;
        }
    }
}

void RegularWork::enumerateLocationIds(std::vector<unsigned int>& ids)
{
    ids.clear();   
    ids.push_back( 0 );
}