
#include "headers.h"
#include "gameplay.h"
#include "crypt.h"
#include "scene.h"
#include "version.h"
#include "unicode.h"
#include "communitymission.h"

database::MissionInfo nullTerminator = { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL };

/**
 * user community events globals
 */

void Gameplay::generateUserCommunityEvents(void)
{    
    WIN32_FIND_DATA fileFindData;
    HANDLE fileFindHandle;
    std::string filePath;

    // enumerate and load XML files in "./usr/missions" folder
    fileFindHandle = FindFirstFile( "./usr/missions/*.xml", &fileFindData );
    if( fileFindHandle != INVALID_HANDLE_VALUE )
    {
        do
        {
            filePath = "./usr/missions/";
            filePath += fileFindData.cFileName;
            TiXmlDocument* xmlDocument = new TiXmlDocument( filePath.c_str() );
            xmlDocument->LoadFile();
            _userTournaments.push_back( xmlDocument );
        }
        while( FindNextFile( fileFindHandle, &fileFindData ) );

        // close enumeration
        FindClose( fileFindHandle );
    }

    // generate tournaments    
    database::NPCInfo        npcInfo;
    database::TournamentInfo tournamentInfo;    
    std::string*             pstr;
    int                      ivalue;
    double                   fvalue;
    for( unsigned int i=0; i<_userTournaments.size(); i++ )
    {
        // enumerate tournaments
        TiXmlNode* child = _userTournaments[i]->FirstChild();
        if( child ) do 
        {
            // NPC node?
            if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "npc" ) == 0 )
            {
                // npc name
                npcInfo.nameId = Gameplay::iLanguage->addUnicodeString(
                    asciizToUnicode( static_cast<TiXmlElement*>( child )->Attribute( "name" ) ).c_str()
                );
                // npc height
                static_cast<TiXmlElement*>( child )->Attribute( "height", &fvalue );
                if( fvalue < 170.0f ) fvalue = 170.0f;
                if( fvalue > 190.0f ) fvalue = 190.0f;
                npcInfo.height = float( fvalue );
                // npc weight
                static_cast<TiXmlElement*>( child )->Attribute( "weight", &fvalue );
                if( fvalue < 50.0f ) fvalue = 50.0f;
                if( fvalue > 110.0f ) fvalue = 110.0f;
                npcInfo.weight = float( fvalue );
                // npc face
                static_cast<TiXmlElement*>( child )->Attribute( "face", &ivalue );
                if( ivalue < 0 ) ivalue = 0;
                if( ivalue >= int( database::Face::getNumRecords() ) ) ivalue = ( database::Face::getNumRecords() - 1 );
                npcInfo.face = ivalue;
                // npc perception skill
                static_cast<TiXmlElement*>( child )->Attribute( "perception", &fvalue );
                if( fvalue < 0.0f ) fvalue = 0.0f;
                if( fvalue > 1.0f ) fvalue = 1.0f;
                npcInfo.perception = float( fvalue );
                // npc tracking skill
                static_cast<TiXmlElement*>( child )->Attribute( "tracking", &fvalue );
                if( fvalue < 0.0f ) fvalue = 0.0f;
                if( fvalue > 1.0f ) fvalue = 1.0f;
                npcInfo.tracking = float( fvalue );
                // npc endurance skill
                static_cast<TiXmlElement*>( child )->Attribute( "endurance", &fvalue );
                if( fvalue < 0.0f ) fvalue = 0.0f;
                if( fvalue > 1.0f ) fvalue = 1.0f;
                npcInfo.endurance = float( fvalue );
                // npc rigging skill
                static_cast<TiXmlElement*>( child )->Attribute( "rigging", &fvalue );
                if( fvalue < 0.0f ) fvalue = 0.0f;
                if( fvalue > 1.0f ) fvalue = 1.0f;
                npcInfo.rigging = float( fvalue );
                // npc overall level
                npcInfo.level = 0.25f * ( 
                    npcInfo.perception + 
                    npcInfo.endurance +
                    npcInfo.tracking +
                    npcInfo.rigging
                );
                // register NPC
                database::NPCInfo::addRecord( &npcInfo );
            }
            // event node?
            else if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "pack" ) == 0 )
            {                
                // tournament name
                tournamentInfo.nameId = Gameplay::iLanguage->addUnicodeString(
                    asciizToUnicode( static_cast<TiXmlElement*>( child )->Attribute( "name" ) ).c_str()
                );
                // tournament briefing
                tournamentInfo.briefId = Gameplay::iLanguage->addUnicodeString(
                    asciizToUnicode( static_cast<TiXmlElement*>( child )->Attribute( "brief" ) ).c_str()
                );
                // tournament location
                static_cast<TiXmlElement*>( child )->Attribute( "locationId", &ivalue );
                tournamentInfo.locationId = ivalue;
                // gamedata identifier
                pstr = new std::string;
                *pstr = static_cast<TiXmlElement*>( child )->Attribute( "gameData" );
                _userStrings.push_back( pstr );
                tournamentInfo.gameData = pstr->c_str();
                // thumbnail
                pstr = new std::string;
                *pstr = static_cast<TiXmlElement*>( child )->Attribute( "thumbnail" );
                _userStrings.push_back( pstr );
                tournamentInfo.thumbnail = pstr->c_str();
                // tournament type is user-maked
                tournamentInfo.type = database::tfUser;
                // generate missions for the tournament
                generateMissions( static_cast<TiXmlElement*>(child), &tournamentInfo );
                unsigned int numMissions = tournamentInfo.getNumMissions();
                // update list of tournaments
                database::TournamentInfo::addRecord( &tournamentInfo );
            }
            child = child->NextSibling();
        }
        while( child != NULL );
    }
}

void Gameplay::generateMissions(TiXmlElement* node, database::TournamentInfo* tournamentInfo)
{
    int ivalue;

    // calculate number of missions
    unsigned int numMissions = 0;
    TiXmlNode* child = node->FirstChild();
    if( child ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "mission" ) == 0 )
        {
            numMissions++;            
        }
        child = child->NextSibling();
    }
    while( child != NULL );

    // allocate space for missions
    database::MissionInfo* missions = new database::MissionInfo[numMissions+1];

    // write NULL-terminator
    missions[numMissions] = nullTerminator;

    // read missions
    unsigned int index = 0;
    std::string* pstr;
    child = node->FirstChild();
    if( child ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "mission" ) == 0 )
        {
            // mission rank
            static_cast<TiXmlElement*>( child )->Attribute( "rank", &ivalue );
            missions[index].rank = ivalue;
            // mission name
            missions[index].nameId = Gameplay::iLanguage->addUnicodeString(
                asciizToUnicode( static_cast<TiXmlElement*>( child )->Attribute( "name" ) ).c_str()
            );
            // mission briefing
            missions[index].briefId = Gameplay::iLanguage->addUnicodeString(
                asciizToUnicode( static_cast<TiXmlElement*>( child )->Attribute( "brief" ) ).c_str()
            );
            // mission exit point
            missions[index].exitPointId = AIRPLANE_EXIT;
            // mission time
            static_cast<TiXmlElement*>( child )->Attribute( "missionTime", &ivalue );
            if( ivalue < 1 ) ivalue = 1;
            missions[index].missionTime = float( ivalue ) * 60;
            // mission thumbnail
            pstr = new std::string;
            *pstr = static_cast<TiXmlElement*>( child )->Attribute( "thumbnail" );
            _userStrings.push_back( pstr );
            missions[index].thumbnail = pstr->c_str();
            // mission flags 
            missions[index].flags = 0;
            // mission node
            missions[index].missionNode = child;
            // mission callbacks
            missions[index].weatherClearance = weatherClearanceCommunity;
            missions[index].windClearance = windClearanceCommunity;
            missions[index].castingCallback = castingCallback_CommunityMission;
            missions[index].equipCallback = equipCallback_CommunityMission;
            // next mission
            index++;
        }
        child = child->NextSibling();
    }
    while( child != NULL );

    // cache missions
    _userMissions.push_back( missions );

    // update tournament
    tournamentInfo->missions = missions;
}

void Gameplay::cleanupUserCommunityEvents(void)
{
    // delete XML documents
    for( unsigned int i=0; i<_userTournaments.size(); i++ )
    {
        delete _userTournaments[i];
    }
    // delete string cache
    for( i=0; i<_userStrings.size(); i++ )
    {
        delete _userStrings[i];
    }
    // delete mission cache
    for( i=0; i<_userMissions.size(); i++ )
    {
        delete[] _userMissions[i];
    }
}