
#include "headers.h"
#include "missionbrowser.h"
#include "imath.h"
#include "database.h"
#include "equip.h"
#include "mission.h"
#include "boogie.h"
#include "festival.h"
#include "climbing.h"
#include "smokeevent.h"
#include "community.h"
#include "version.h"

/**
 * class implementation
 */

TournamentSource::TournamentSource(unsigned int locationId, Career* career)
{
    // store location
    _locationId = locationId;

    // build list
    unsigned int numTournaments = database::TournamentInfo::getNumRecords();    
    for( unsigned int i=0; i<numTournaments; i++ )
    {
        database::TournamentInfo* tournamentInfo = database::TournamentInfo::getRecord( i );
        // filter tournaments from this location
        if( tournamentInfo->locationId == locationId )
        {
			if ( Gameplay::iGameplay->_freeModeIsEnabled )
			{
				_tournaments.push_back( i );
			}
            // boogie tournament?
            else if( tournamentInfo->type == database::tfBoogie )
            {
                // check there is a boogie event in location
                for( unsigned int j=0; j<career->getNumEvents(); j++ )
                {
                    // boogie event?
                    Boogie* boogie = dynamic_cast<Boogie*>( career->getEvent( j ) );
                    if( boogie && boogie->getLocationId() == locationId && boogie->getFlags() & efActive )
                    {
                        _tournaments.push_back( i );
                    }
                }
            }
            // festival tournament?
            else if( tournamentInfo->type == database::tfFestival )
            {
                // check there is a festival event in location
                for( unsigned int j=0; j<career->getNumEvents(); j++ )
                {
                    // festival event?
                    Festival* festival = dynamic_cast<Festival*>( career->getEvent( j ) );
                    if( festival && festival->getLocationId() == locationId && festival->getFlags() && efActive )
                    {
                        _tournaments.push_back( i );
                    }
                }
            }
            // climbing tournament?
            else if( tournamentInfo->type == database::tfClimbing )
            {
                // check there is a climbing event in location
                for( unsigned int j=0; j<career->getNumEvents(); j++ )
                {
                    // climbing event?
                    Climbing* climbing = dynamic_cast<Climbing*>( career->getEvent( j ) );
                    if( climbing && climbing->getLocationId() == locationId && climbing->getFlags() && efActive )
                    {
                        _tournaments.push_back( i );
                    }
                }
            }
            // smokeball tournament?
            else if( tournamentInfo->type == database::tfSmokeball )
            {
                // check there is a smokeball event in location
                for( unsigned int j=0; j<career->getNumEvents(); j++ )
                {
                    // climbing event?
                    SmokeballEvent* smokeballEvent = dynamic_cast<SmokeballEvent*>( career->getEvent( j ) );
                    if( smokeballEvent && smokeballEvent->getLocationId() == locationId && smokeballEvent->getFlags() && efActive )
                    {
                        _tournaments.push_back( i );
                    }
                }
            }
            // community tournament?
            else if( tournamentInfo->type == database::tfUser )
            {
                /*
                // check there is a community event in location
                for( unsigned int j=0; j<career->getNumEvents(); j++ )
                {
                    // climbing event?
                    CommunityEvent* communityEvent = dynamic_cast<CommunityEvent*>( career->getEvent( j ) );
                    if( communityEvent && communityEvent->getLocationId() == locationId && communityEvent->getFlags() && efActive )
                    {
                        _tournaments.push_back( i );
                    }
                }
                */

                // always
                _tournaments.push_back( i );
            }
            // arbitrary tournament?
            else if( tournamentInfo->type == database::tfArbitrary )
            {
                bool eventInLocation = false;
                // check there is an event in location
                for( unsigned int j=0; j<career->getNumEvents(); j++ )
                {
                    // check for boogie event
                    Boogie* boogie = dynamic_cast<Boogie*>( career->getEvent( j ) );
                    if( boogie && boogie->getLocationId() == locationId && boogie->getFlags() & efActive )
                    {
                        eventInLocation = true;
                        break;
                    }
                    // check for festival event
                    Festival* festival = dynamic_cast<Festival*>( career->getEvent( j ) );
                    if( festival && festival->getLocationId() == locationId && festival->getFlags() && efActive )
                    {
                        eventInLocation = true;
                        break;
                    }
                    // check for climbing event
                    Climbing* climbing = dynamic_cast<Climbing*>( career->getEvent( j ) );
                    if( climbing && climbing->getLocationId() == locationId && climbing->getFlags() && efActive )
                    {
                        eventInLocation = true;
                        break;
                    }
                    // check for smokeball event
                    SmokeballEvent* smokeballEvent = dynamic_cast<SmokeballEvent*>( career->getEvent( j ) );
                    if( smokeballEvent && smokeballEvent->getLocationId() == locationId && smokeballEvent->getFlags() && efActive )
                    {
                        eventInLocation = true;
                        break;
                    }
                    // check for community event
                    CommunityEvent* communityEvent = dynamic_cast<CommunityEvent*>( career->getEvent( j ) );
                    if( communityEvent && communityEvent->getLocationId() == locationId && communityEvent->getFlags() && efActive )
                    {
                        eventInLocation = true;
                        break;
                    }
                }
                // arbitrary tournament are enabled:
                //  - if there is no other events in location
                //  - if this is not a demo tournament
                if( !eventInLocation && i != database::TournamentInfo::getDemoTournament() )
                {
                     _tournaments.push_back( i );
                }
            }
        }
    }

    #ifdef GAMEPLAY_DEMOVERSION
        _tournaments.clear();
        _tournaments.push_back( database::TournamentInfo::getDemoTournament() );
    #endif
}

TournamentSource::~TournamentSource()
{
}

/**
 * BrowserSource
 */

const char* TournamentSource::getDefaultThumbnail(void)
{
    return database::LocationInfo::getRecord( _locationId )->thumbnail;
}

unsigned int TournamentSource::getNumItems(void)
{
    return _tournaments.size();
}

const wchar_t* TournamentSource::getName(unsigned int itemId)
{
    return Gameplay::iLanguage->getUnicodeString( database::TournamentInfo::getRecord( _tournaments[itemId] )->nameId );
}

const wchar_t* TournamentSource::getBriefing(unsigned int itemId)
{
    return Gameplay::iLanguage->getUnicodeString( database::TournamentInfo::getRecord( _tournaments[itemId] )->briefId );
}

const char* TournamentSource::getThumbnail(unsigned int itemId)
{
    assert( itemId < _tournaments.size() );
    return database::TournamentInfo::getRecord( _tournaments[itemId] )->thumbnail;
}

float TournamentSource::getDuration(unsigned int itemId)
{
    return 0.0f;
}

bool TournamentSource::getWalkthroughFlag(Career* career, unsigned int itemId)
{
    return false;
}

bool TournamentSource::getWeatherClearance(unsigned int itemId, WeatherType weatherType)
{
    return true;
}

bool TournamentSource::getWindClearance(unsigned int itemId, Vector3f dir, float ambient, float blast)
{
    return true;
}

bool TournamentSource::equip(unsigned int itemId, Career* career, float ambient, float blast)
{
    return true;
}

bool TournamentSource::browseBack(unsigned int itemId)
{
    return false;
}

BrowserSource* TournamentSource::browse(unsigned int itemId, Scene* scene, MissionBrowser* browser)
{
    return new MissionSource( _tournaments[itemId] );
}