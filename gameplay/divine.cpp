
#include "headers.h"
#include "geoscape.h"
#include "divine.h"
#include "boogie.h"
#include "festival.h"
#include "climbing.h"
#include "smokeevent.h"
#include "community.h"
#include "database.h"
#include "travel.h"
#include "gameplay.h"

Divine::Divine(Career* career) : Event( career, 1 ) 
{
    // divine event occurs each day
    _timeTo = DAYS_TO_MINUTES(1);
    _duration = DAYS_TO_MINUTES(1);
    _flags = _flags | efSystem;
}

Divine::Divine(Career* career, TiXmlElement* element) : Event( career, 1, element )
{
}

Divine::~Divine()
{
}

const char* Divine::getClassName(void)
{
    return DIVINE_CLASS_NAME;
}

void Divine::onBeginEvent(Geoscape* geoscape)
{
    castBoogieEvent();
    castFestivalEvent();
    castClimbingEvent();
    castSmokeballEvent();
    castCommunityEvent();

    // if player isn't in home location and isn't travelling
    if( geoscape->getPlayerLocation() && geoscape->getPlayerLocation()->getDatabaseId() != 0 )
    {
        // location info
        database::LocationInfo* info = database::LocationInfo::getRecord( geoscape->getPlayerLocation()->getDatabaseId() );

        // home location travelling cost
        Vector2f currentCoords = geoscape->getPlayerLocation()->getLocationCoords();
        Vector2f targetCoords = geoscape->getHomeLocation()->getLocationCoords();
        float targetDistance = ( targetCoords - currentCoords ).length();
        targetDistance = targetDistance * 40075.696f / geoscape->getWindow()->getPanel()->getRect().getWidth();
        float referenceCost = 0.05f;
        float targetCost = targetDistance * referenceCost;
        float referenceTime = 0.1f;
        float targetTime = targetDistance * referenceTime;

        // check we have enought money to stay in location
        if( targetCost + info->stayFee < _career->getVirtues()->evolution.funds )
        {
            // pay fee
            _career->getVirtues()->evolution.funds -= info->stayFee;
        }
        else
        {
            // travel to home location
            // remove player from location
            unsigned int departureId = geoscape->getPlayerLocation()->getDatabaseId();
            geoscape->getPlayerLocation()->setPlayer( false );
            // decrease funds
            _career->getVirtues()->evolution.funds -= targetCost;
            // add event
            _career->addEvent( new Travel( _career, departureId, 0, targetTime ) );
            // message
            geoscape->addHistory( Gameplay::iLanguage->getUnicodeString(293), Vector4f( 1,0.25,0.25,1 ) );
        }
    }

    // simulate market dynamics
    unsigned int maxTransactions = 10;
    // simulate buying
    unsigned int numTransactions = getCore()->getRandToolkit()->getUniformInt() % maxTransactions;
    while( numTransactions && geoscape->getMarketSize() )
    {
        // buy random gear
        geoscape->removeGearFromMarket( 
            getCore()->getRandToolkit()->getUniformInt() % geoscape->getMarketSize()
        );
        numTransactions--;
    }
    // simulate selling
    numTransactions = getCore()->getRandToolkit()->getUniformInt() % maxTransactions;
    while( numTransactions )
    {
        // sell random gear
        Gear gear = geoscape->generateGear( getCore()->getRandToolkit()->getUniform( 0, 1 ) > 0.75f );
        if( gear.isTradeable() ) geoscape->addGearToMarket( gear );
        numTransactions--;
    }
}

void Divine::onEndEvent(Geoscape* geoscape)
{
    _career->addEvent( new Divine( _career ) );
}

void Divine::onUpdateEvent(Geoscape* geoscape, float dt)
{
}

void Divine::enumerateLocationIds(std::vector<unsigned int>& ids)
{
    ids.clear();
    for( unsigned int i=0; i<database::LocationInfo::getNumRecords(); i++ )
    {
        ids.push_back( i );
    }
}

/**
 * private behaviour
 */

const float boogieEventProbability = 1.0f/7.0f;

void Divine::castBoogieEvent(void)
{
    float p = getCore()->getRandToolkit()->getUniform( 0, 1 );
    if( p < boogieEventProbability )
    {
        // enumerate locations able for boogie
        unsigned int numLocations = database::LocationInfo::getNumRecords();
        std::vector<unsigned int> locations;
        for( unsigned int i=0; i<numLocations; i++ )
        {
            if( database::LocationInfo::getRecord(i)->accessible && 
                database::LocationInfo::getRecord(i)->boogieId )
            {
                // check boogie is available by character rank
                bool isAvailable = false;
                for( unsigned int j=0; j<database::TournamentInfo::getNumRecords(); j++ )
                {
                    if( database::TournamentInfo::getRecord( j )->locationId == i &&
                        database::TournamentInfo::getRecord( j )->type == database::tfBoogie && 
                        database::TournamentInfo::getRecord( j )->getMinimalRank() <= _career->getVirtues()->getRank() )
                    {
                        isAvailable = true;
                    }
                }
                // add pretender
                if( isAvailable ) locations.push_back( i );
            }
        }

        if( locations.size() )
        {
            // choose random location        
            unsigned int index = abs( getCore()->getRandToolkit()->getUniformInt() );
            index = index % locations.size();
            unsigned int locationId = locations[index];

            // check there is no same event in this location
            bool eventIsPresent = false;
            for( i=0; i<_career->getNumEvents(); i++ )
            {
                Boogie* boogie = dynamic_cast<Boogie*>( _career->getEvent(i) );
                if( boogie && boogie->getLocationId() == locationId )
                {
                    eventIsPresent = true;
                    break;
                }
            }

            if( !eventIsPresent )
            {
                // random time to start
                unsigned int daysToStart  = unsigned int( getCore()->getRandToolkit()->getUniform( 1, 7 ) );
                unsigned int daysDuration = unsigned int( getCore()->getRandToolkit()->getUniform( 1, 3 ) );
                // new event
                _career->addEvent( new Boogie( 
                    _career, 
                    locationId, 
                    24.0f * 60.0f * daysToStart, 
                    24.0f * 60.0f * daysDuration
                ) );
            }
        }
    }
}

const float festivalEventProbability = 1.0f/7.0f;

void Divine::castFestivalEvent(void)
{
    float p = getCore()->getRandToolkit()->getUniform( 0, 1 );
    if( p < festivalEventProbability )
    {
        // enumerate locations able for festivals
        unsigned int numLocations = database::LocationInfo::getNumRecords();
        std::vector<unsigned int> locations;
        for( unsigned int i=0; i<numLocations; i++ )
        {
            if( database::LocationInfo::getRecord(i)->accessible && 
                database::LocationInfo::getRecord(i)->festivalId )
            {
                // check festival is available by character rank
                bool isAvailable = false;
                for( unsigned int j=0; j<database::TournamentInfo::getNumRecords(); j++ )
                {
                    if( database::TournamentInfo::getRecord( j )->locationId == i &&
                        database::TournamentInfo::getRecord( j )->type == database::tfFestival && 
                        database::TournamentInfo::getRecord( j )->getMinimalRank() <= _career->getVirtues()->getRank() )
                    {
                        isAvailable = true;
                    }
                }
                // add pretender
                if( isAvailable ) locations.push_back( i );
            }
        }

        if( locations.size() )
        {
            // choose random location        
            unsigned int index = abs( getCore()->getRandToolkit()->getUniformInt() );
            index = index % locations.size();
            unsigned int locationId = locations[index];

            // check there is no same event in this location
            bool eventIsPresent = false;
            for( i=0; i<_career->getNumEvents(); i++ )
            {
                Festival* festival = dynamic_cast<Festival*>( _career->getEvent(i) );
                if( festival && festival->getLocationId() == locationId )
                {
                    eventIsPresent = true;
                    break;
                }
            }
    
            if( !eventIsPresent )
            {
                // random time to start
                unsigned int daysToStart  = unsigned int( getCore()->getRandToolkit()->getUniform( 1, 7 ) );
                unsigned int daysDuration = unsigned int( getCore()->getRandToolkit()->getUniform( 1, 3 ) );
                // new event
                _career->addEvent( new Festival( 
                    _career, 
                    locationId, 
                    24.0f * 60.0f * daysToStart, 
                    24.0f * 60.0f * daysDuration
                ) );
            }
        }
    }
}

const float climbingEventProbability = 1.0f/7.0f;

void Divine::castClimbingEvent(void)
{
    float p = getCore()->getRandToolkit()->getUniform( 0, 1 );
    if( p < climbingEventProbability )
    {
        // enumerate locations able for climbings
        unsigned int numLocations = database::LocationInfo::getNumRecords();
        std::vector<unsigned int> locations;
        for( unsigned int i=0; i<numLocations; i++ )
        {
            if( database::LocationInfo::getRecord(i)->accessible && 
                database::LocationInfo::getRecord(i)->climbingId )
            {
                // check climbing is available by character rank
                bool isAvailable = false;
                for( unsigned int j=0; j<database::TournamentInfo::getNumRecords(); j++ )
                {
                    if( database::TournamentInfo::getRecord( j )->locationId == i &&
                        database::TournamentInfo::getRecord( j )->type == database::tfClimbing && 
                        database::TournamentInfo::getRecord( j )->getMinimalRank() <= _career->getVirtues()->getRank() )
                    {
                        isAvailable = true;
                    }
                }
                // add pretender
                if( isAvailable ) locations.push_back( i );
            }
        }

        if( locations.size() )
        {
            // choose random location        
            unsigned int index = abs( getCore()->getRandToolkit()->getUniformInt() );
            index = index % locations.size();
            unsigned int locationId = locations[index];

            // check there is no same event in this location
            bool eventIsPresent = false;
            for( i=0; i<_career->getNumEvents(); i++ )
            {
                Climbing* climbing = dynamic_cast<Climbing*>( _career->getEvent(i) );
                if( climbing && climbing->getLocationId() == locationId )
                {
                    eventIsPresent = true;
                    break;
                }
            }

            if( !eventIsPresent )
            {
                // random time to start
                unsigned int daysToStart  = unsigned int( getCore()->getRandToolkit()->getUniform( 1, 7 ) );
                unsigned int daysDuration = unsigned int( getCore()->getRandToolkit()->getUniform( 1, 3 ) );
                // new event
                _career->addEvent( new Climbing( 
                    _career, 
                    locationId, 
                    24.0f * 60.0f * daysToStart, 
                    24.0f * 60.0f * daysDuration
                ) );
            }
        }
    }
}

const float smokeballEventProbability = 1.0f/7.0f;

void Divine::castSmokeballEvent(void)
{
    float p = getCore()->getRandToolkit()->getUniform( 0, 1 );
    if( p < smokeballEventProbability )
    {
        // enumerate locations able for smokeball
        unsigned int numLocations = database::LocationInfo::getNumRecords();
        std::vector<unsigned int> locations;
        for( unsigned int i=0; i<numLocations; i++ )
        {
            if( database::LocationInfo::getRecord(i)->accessible && 
                database::LocationInfo::getRecord(i)->smokeballId )
            {
                // check smokeball is available by character rank
                bool isAvailable = false;
                for( unsigned int j=0; j<database::TournamentInfo::getNumRecords(); j++ )
                {
                    database::TournamentInfo* tournamentInfo = database::TournamentInfo::getRecord( j );
                    if( tournamentInfo->locationId == i )
                    {
                        if( tournamentInfo->type == database::tfSmokeball && 
                            tournamentInfo->getMinimalRank() <= _career->getVirtues()->getRank() )
                        {
                            isAvailable = true;
                        }
                    }
                }
                // add pretender
                if( isAvailable ) locations.push_back( i );
            }
        }

        if( locations.size() )
        {
            // choose random location        
            unsigned int index = abs( getCore()->getRandToolkit()->getUniformInt() );
            index = index % locations.size();
            unsigned int locationId = locations[index];

            // check there is no same event in this location
            bool eventIsPresent = false;
            for( i=0; i<_career->getNumEvents(); i++ )
            {
                SmokeballEvent* smokeballEvent = dynamic_cast<SmokeballEvent*>( _career->getEvent(i) );
                if( smokeballEvent && smokeballEvent->getLocationId() == locationId )
                {
                    eventIsPresent = true;
                    break;
                }
            }

            if( !eventIsPresent )
            {
                // random time to start
                unsigned int daysToStart  = unsigned int( getCore()->getRandToolkit()->getUniform( 1, 7 ) );
                unsigned int daysDuration = unsigned int( getCore()->getRandToolkit()->getUniform( 1, 3 ) );
                // new event
                _career->addEvent( new SmokeballEvent( 
                    _career, 
                    locationId, 
                    24.0f * 60.0f * daysToStart, 
                    24.0f * 60.0f * daysDuration
                ) );
            }
        }
    }
}

const float communityEventProbability = 1.0f/7.0f;

void Divine::castCommunityEvent(void)
{
    return;
    
    /*
    float p = getCore()->getRandToolkit()->getUniform( 0, 1 );
    if( p < communityEventProbability )
    {
        // enumerate locations able for community
        unsigned int numLocations = database::LocationInfo::getNumRecords();
        std::vector<unsigned int> locations;
        for( unsigned int i=0; i<numLocations; i++ )
        {
            if( database::LocationInfo::getRecord(i)->accessible && 
                database::LocationInfo::getRecord(i)->communityId )
            {
                // check community is available by character rank
                bool isAvailable = false;
                for( unsigned int j=0; j<database::TournamentInfo::getNumRecords(); j++ )
                {
                    database::TournamentInfo* tournamentInfo = database::TournamentInfo::getRecord( j );
                    if( tournamentInfo->locationId == i )
                    {
                        if( tournamentInfo->type == database::tfUser && 
                            tournamentInfo->getMinimalRank() <= _career->getVirtues()->getRank() )
                        {
                            isAvailable = true;
                        }
                    }
                }
                // add pretender
                if( isAvailable ) locations.push_back( i );
            }
        }

        if( locations.size() )
        {
            // choose random location        
            unsigned int index = abs( getCore()->getRandToolkit()->getUniformInt() );
            index = index % locations.size();
            unsigned int locationId = locations[index];

            // check there is no same event in this location
            bool eventIsPresent = false;
            for( i=0; i<_career->getNumEvents(); i++ )
            {
                CommunityEvent* communityEvent = dynamic_cast<CommunityEvent*>( _career->getEvent(i) );
                if( communityEvent && communityEvent->getLocationId() == locationId )
                {
                    eventIsPresent = true;
                    break;
                }
            }

            if( !eventIsPresent )
            {
                // random time to start
                unsigned int daysToStart  = unsigned int( getCore()->getRandToolkit()->getUniform( 1, 7 ) );
                unsigned int daysDuration = unsigned int( getCore()->getRandToolkit()->getUniform( 1, 3 ) );
                // new event
                _career->addEvent( new CommunityEvent( 
                    _career, 
                    locationId, 
                    24.0f * 60.0f * daysToStart, 
                    24.0f * 60.0f * daysDuration
                ) );
            }
        }
    }
    */
}