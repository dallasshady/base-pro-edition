
#include "headers.h"
#include "missionbrowser.h"
#include "imath.h"
#include "database.h"
#include "equip.h"
#include "mission.h"
#include "version.h"

/**
 * class implementation
 */

MissionSource::MissionSource(unsigned int tournamentId)
{
    // store tournament
    _tournamentId = tournamentId;
}

MissionSource::~MissionSource()
{
}

/**
 * BrowserSource
 */

const char* MissionSource::getDefaultThumbnail(void)
{
    return database::TournamentInfo::getRecord( _tournamentId )->thumbnail;
}

unsigned int MissionSource::getNumItems(void)
{
    return database::TournamentInfo::getRecord( _tournamentId )->getNumMissions() + 1;
}

const wchar_t* MissionSource::getName(unsigned int itemId)
{
    assert( itemId < getNumItems() );
    if( itemId )
    {
        return Gameplay::iLanguage->getUnicodeString(database::TournamentInfo::getRecord( _tournamentId )->missions[itemId-1].nameId);
    }
    else
    {
        return L"...";
    }
}

const wchar_t* MissionSource::getBriefing(unsigned int itemId)
{
    assert( itemId < getNumItems() );
    if( itemId )
    {
        return Gameplay::iLanguage->getUnicodeString(database::TournamentInfo::getRecord( _tournamentId )->missions[itemId-1].briefId);
    }
    else
    {
        return L"";
    }
}

const char* MissionSource::getThumbnail(unsigned int itemId)
{
    assert( itemId < getNumItems() );
    if( itemId )
    {
        return database::TournamentInfo::getRecord( _tournamentId )->missions[itemId-1].thumbnail;
    }
    else
    {
        return getDefaultThumbnail();
    }
}

float MissionSource::getDuration(unsigned int itemId)
{
    assert( itemId < getNumItems() );
    if( itemId )
    {
        return database::TournamentInfo::getRecord( _tournamentId )->missions[itemId-1].missionTime;
    }
    else
    {
        return 0.0f;
    }
}

bool MissionSource::getWalkthroughFlag(Career* career, unsigned int itemId)
{
    assert( itemId < getNumItems() );
    if( itemId )
    {
        return career->getMissionWalkthroughFlag( _tournamentId, itemId-1 );
    }
    else
    {
        return false;
    }
}

bool MissionSource::getWeatherClearance(unsigned int itemId, WeatherType weatherType)
{
    assert( itemId < getNumItems() );
    if( itemId )
    {
        return database::TournamentInfo::getRecord( _tournamentId )->missions[itemId-1].weatherClearance( weatherType );
    }
    else
    {
        return true;
    }
}

bool MissionSource::getWindClearance(unsigned int itemId, Vector3f dir, float ambient, float blast)
{
    assert( itemId < getNumItems() );
    if( itemId )
    {
        return database::TournamentInfo::getRecord( _tournamentId )->missions[itemId-1].windClearance( dir, ambient, blast );
    }
    else
    {
        return true;
    }
}

bool MissionSource::equip(unsigned int itemId, Career* career, float ambient, float blast)
{
    assert( itemId < getNumItems() );
    if( itemId )
    {
        database::MissionInfo* missionInfo = database::TournamentInfo::getRecord( _tournamentId )->missions + itemId - 1;
        if( missionInfo->equipCallback == NULL )
        {
            return true;
        }
        else
        {
            return missionInfo->equipCallback( career, ambient, blast, missionInfo );
        }
    }
    else
    {
        return true;
    }
}

bool MissionSource::browseBack(unsigned int itemId)
{
    return ( itemId == 0 );
}

BrowserSource* MissionSource::browse(unsigned int itemId, Scene* scene, MissionBrowser* browser)
{
    assert( itemId < getNumItems() );

    if( itemId )
    {
        database::MissionInfo* missionInfo = database::TournamentInfo::getRecord( _tournamentId )->missions + itemId - 1;

        // define minimal rank to play this mission
        unsigned int missionRank = missionInfo->rank;

        // define weather rank modifier
        unsigned int weatherRank = 0;        
        if( scene->getLocation()->getWeather() == ::wtLightRain )
        {
            weatherRank = 1;
        }
        else if( scene->getLocation()->getWeather() == ::wtHardRain )
        {
            weatherRank = 2;
        }
        else if( scene->getLocation()->getWeather() == ::wtThunder )
        {
            weatherRank = 3;
        }

        // define wind rank modifier
        unsigned int windRank = 0;
        if( !missionInfo->windClearance( scene->getLocation()->getWindDirection(), scene->getLocation()->getWindAmbient(),  scene->getLocation()->getWindBlast() ) )
        {
            windRank = 1;
        }

        // modify mission rank
        missionRank = missionRank + weatherRank + windRank;
        
        // check mission rank
        bool rankIsEnough = scene->getCareer()->getVirtues()->getRank() >= missionRank;
        #ifdef GAMEPLAY_DEVELOPER_EDITION
            rankIsEnough = true;
        #endif
		// if freejumping mode is enabled we don't care about the players rank
		if ( Gameplay::iGameplay->_freeModeIsEnabled )
		{
			rankIsEnough = true;
		}
        if( rankIsEnough )
        {
            // check mission time
            bool timeIsEnough = scene->getHoldingTime() >= missionInfo->missionTime;
            #ifdef GAMEPLAY_DEVELOPER_EDITION
                timeIsEnough = true;
            #endif
			// if freejumping mode is enabled we don't care about the time
			if ( Gameplay::iGameplay->_freeModeIsEnabled )
			{
				timeIsEnough = true;
			}
            if( timeIsEnough )
            {
				// if freejumping mode is enabled we don't care about passing the time to the engine
				if ( !Gameplay::iGameplay->_freeModeIsEnabled )
				{
					// start mission
					#ifndef GAMEPLAY_DEVELOPER_EDITION
						#ifndef GAMEPLAY_DEMOVERSION
							scene->passHoldingTime( missionInfo->missionTime );
						#endif
					#endif
				}
                // equip is needed?
                if( missionInfo->flags & database::mfForcedEquipment )
                {
                    new Mission( scene, missionInfo, _tournamentId, ( itemId - 1 ) );                        
                }
                else
                {
                    new Equip( scene, missionInfo, _tournamentId, ( itemId - 1 ) );
                }
            }
            else
            {
                browser->setFootnote( Gameplay::iLanguage->getUnicodeString(595), 3.0f );
            }
        }
        else
        {
            browser->setFootnote( wstrformat( Gameplay::iLanguage->getUnicodeString(636), missionRank, weatherRank, windRank ).c_str(), 3.0f );
        }
    }

    return NULL;
}