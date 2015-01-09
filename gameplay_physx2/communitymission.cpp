
#include "headers.h"
#include "communitymission.h"
#include "airplane.h"
#include "hud.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "ostankino.h"
#include "dropzone.h"
#include "equip.h"
#include "script.h"
#include "unicode.h"

/**
 * XML parsing helpers
 */

static int xmlInt(TiXmlNode* node, const char* attributeName)
{
    assert( node->Type() == TiXmlNode::ELEMENT );
    const char* value = static_cast<TiXmlElement*>( node )->Attribute( attributeName );
    
    if( value )
    {
        return atoi( value );
    }
    else
    {
        getCore()->logMessage( "[me:error] can't find integer value (%s), return 0", attributeName );
        return 0;
    }
}

static float xmlFloat(TiXmlNode* node, const char* attributeName)
{
    assert( node->Type() == TiXmlNode::ELEMENT );
    const char* value = static_cast<TiXmlElement*>( node )->Attribute( attributeName );
    
    if( value )
    {
        return float( atof( value ) );
    }
    else
    {
        getCore()->logMessage( "[me:error] can't find floating point value (%s), return 0.0", attributeName );
        return 0;
    }
}

static Vector3f xmlVector3f(TiXmlNode* node, const char* attributeName)
{
    assert( node->Type() == TiXmlNode::ELEMENT );
    const char* value = static_cast<TiXmlElement*>( node )->Attribute( attributeName );
    
    if( !value )
    {
        getCore()->logMessage( "[me:error] can't find 3D-vector value (%s), return 0.0;0.0;0.0", attributeName );
        return Vector3f(0,0,0);
    }
    
    StringL xyz;
    strseparate( value, ",", xyz );

    Vector3f result( 0,0,0 );
    unsigned int i = 0;
    for( StringI stringI = xyz.begin(); stringI != xyz.end(); stringI++, i++ )
    {
    	result[i] = float( atof( stringI->c_str() ) );
    }

    return result;
}

static Vector4f xmlVector4f(TiXmlNode* node, const char* attributeName)
{
    assert( node->Type() == TiXmlNode::ELEMENT );
    const char* value = static_cast<TiXmlElement*>( node )->Attribute( attributeName );
    assert( value );
    
    StringL xyzw;
    strseparate( value, ",", xyzw );

    Vector4f result( 0,0,0,0 );
    unsigned int i = 0;
    for( StringI stringI = xyzw.begin(); stringI != xyzw.end(); stringI++, i++ )
    {
    	result[i] = float( atof( stringI->c_str() ) );
    }

    return result;
}

/**
 * community clearance functions
 */

bool weatherClearanceCommunity(WeatherType weatherType)
{
    // can't jump during bad weather
    if( weatherType == wtLightRain ||
        weatherType == wtHardRain ||
        weatherType == wtThunder )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool windClearanceCommunity(Vector3f dir, float ambient, float blast)
{
    // can't jump during strong wind
    if( 0.5f * ( ambient + blast ) > 9 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

/**
 * community mission equip callback
 */

bool equipCallback_CommunityMission(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // pass mission XML, search for equipment node
    TiXmlNode* child = missionInfo->missionNode->FirstChild();
    if( child ) do 
    {
        // equipment node?
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "equipment" ) == 0 )
        {
            // select canopy
            if( xmlInt( child, "skydiving" ) )
            {
                equipBestSkydivingEquipment( career, windAmbient, windBlast );
            }
            else
            {
                equipBestBASEEquipment( career, windAmbient, windBlast );
            }

            // select suit
            if( xmlInt( child, "wingsuit" ) )
            {
                equipBestWingsuit( career, windAmbient, windBlast );
            }
            else
            {
                equipBestSuit( career, windAmbient, windBlast );
            }

            // select slider
            if( strcmp( static_cast<TiXmlElement*>(child)->Attribute( "slider" ), "up" ) == 0 )
            {
                career->getVirtues()->equipment.sliderOption = ::soUp;
            }
            else if( strcmp( static_cast<TiXmlElement*>(child)->Attribute( "slider" ), "down" ) == 0 )
            {
                career->getVirtues()->equipment.sliderOption = ::soDown;
            }
            else
            {
                career->getVirtues()->equipment.sliderOption = ::soRemoved;
            }

            // select pilotchute
            career->getVirtues()->equipment.pilotchute = xmlInt( child, "pc" );
            if( xmlInt( child, "skydiving" ) && career->getVirtues()->equipment.pilotchute > 1 )
            {
                getCore()->logMessage( "[me:error] max pc index for skydiving canopy is 1" );
                career->getVirtues()->equipment.pilotchute = 1;
            }
            else if( career->getVirtues()->equipment.pilotchute > 5 )
            {
                getCore()->logMessage( "[me:error] max pc index for skydiving canopy is 5" );
                career->getVirtues()->equipment.pilotchute = 5;
            }

            // its over
            break;
        }
        // next mission node
        child = child->NextSibling();
    }
    while( child != NULL );

    return true;
}

/**
 * community mission script
 */

typedef std::pair<std::string,NPC*> NPCP;
typedef std::map<std::string,NPC*> NPCM;

namespace script { class CommunityScript : public Script
{
private:
    NPCL _npcs;
public:
    // script core
    CommunityScript(Jumper* player, NPCL& npcs, NPCM& npcm, TiXmlNode* scriptNode) : Script( player )
    {
        _npcs = npcs;
        _routine = NULL;
        
        std::string routineName = static_cast<TiXmlElement*>(scriptNode)->Attribute( "routine" );
        if( strcmp( routineName.c_str(), "timer" ) == 0 )
        {
            _routine = new Script::Timer( player );
        }
        else if( strcmp( routineName.c_str(), "seeyou" ) == 0 )
        {
            _routine = new Script::SeeYou( player, _npcs );
        }
        else if( strcmp( routineName.c_str(), "countdown" ) == 0 )
        {
            NPCM::iterator npcI = npcm.find( static_cast<TiXmlElement*>(scriptNode)->Attribute( "npc" ) );
            if( npcI != npcm.end() )
            {
                _routine = new Script::Countdown( player, npcI->second->getCatToy() );
            }
            else
            {
                getCore()->logMessage( "[me:error] unknown NPC name \"%s\"", static_cast<TiXmlElement*>(scriptNode)->Attribute( "npc" ) );
            }
        }
        else
        {
            getCore()->logMessage( "[me:error] unknown routine \"%s\"", routineName.c_str() );
        }

        // create routine
        //_routine = new Script::Countdown( player, npc->getCatToy() );
        //_routine = new Script::SeeYou( player, _npcs );
    }
    virtual void onUpdateActivity(float dt)
    {
        // base behaviour
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {            
            delete _routine;
            _routine = NULL;
        }
    }
}; }

/**
 * community mission startup callback
 */

static Gear generatePrize(const char* prizeType)
{
    // choose prize
    Gear result;
    result.state = 1.0f;

    // number of attempts to generate tradeable gear
    unsigned int numOfAttempts = 0;
    unsigned int typeId;
    
    do
    {
        // determine gear type id
        if( strcmp( prizeType, "any" ) == 0 )
        {
            typeId = getCore()->getRandToolkit()->getUniformInt() % 4;
        }
        else if( strcmp( prizeType, "helmet" ) == 0 )
        {
            typeId = 0;
        }
        else if( strcmp( prizeType, "suit" ) == 0 )
        {
            typeId = 1;
        }
        else if( strcmp( prizeType, "rig" ) == 0 )
        {
            typeId = 2;
        }
        else if( strcmp( prizeType, "canopy" ) == 0 )
        {
            typeId = 3;
        }
        else
        {
            getCore()->logMessage( "[me:error] unidentified type of prize \"%s\", forced any type", prizeType );
            typeId = getCore()->getRandToolkit()->getUniformInt() % 4;
        }

        switch( typeId )
        {
        case 0: 
            result.type = gtHelmet;
            result.id   = getCore()->getRandToolkit()->getUniformInt() % database::Helmet::getNumRecords();
            assert( result.id < database::Helmet::getNumRecords() );
            break;
        case 1: 
            result.type = gtSuit; 
            result.id   = getCore()->getRandToolkit()->getUniformInt() % database::Suit::getNumRecords();
            assert( result.id < database::Suit::getNumRecords() );
            break;
        case 2: 
            result.type = gtRig; 
            result.id   = getCore()->getRandToolkit()->getUniformInt() % database::Rig::getNumRecords();
            assert( result.id < database::Rig::getNumRecords() );
            break;
        case 3: 
            result.type = gtCanopy; 
            result.id   = getCore()->getRandToolkit()->getUniformInt() % database::Canopy::getNumRecords();
            assert( result.id < database::Canopy::getNumRecords() );
            break;
        default:
            assert( !"shouldn't be here!" );
        }
        numOfAttempts++;
        if( numOfAttempts >= 100 ) break;
    }
    while( !result.isTradeable() );

    return result;
}

void castingCallback_CommunityMission(Actor* parent)
{
    // retrieve mission
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );

    // retrieve mission info
    database::MissionInfo* missionInfo = mission->getMissionInfo();
    assert( missionInfo->missionNode );

    bool playerIsPresent = false;
    NPCM npcm;

    // pass mission XML
    TiXmlNode* child = missionInfo->missionNode->FirstChild();
    if( child ) do 
    {
        // exit node?
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "exit" ) == 0 )
        {
            // parsing objects
            AirplaneDesc airplaneDesc;
            Airplane*    airplane = NULL;
            Enclosure*   fixedExit = NULL;

            // helicopter exit?
            if( strcmp( static_cast<TiXmlElement*>(child)->Attribute( "type" ), "helicopter" ) == 0 )
            {
                // cast helicopter object
                airplaneDesc.templateClump = parent->getScene()->findClump( "Helicopter01" ); assert( airplaneDesc.templateClump );
                airplaneDesc.propellerFrame = "PropellerSound";
                airplaneDesc.propellerSound = "./res/sounds/aircrafts/helicopter.ogg";
                airplaneDesc.exitPointFrames.push_back( "HelicopterExit01" );
                airplaneDesc.exitPointFrames.push_back( "HelicopterExit02" );
                airplaneDesc.exitPointFrames.push_back( "HelicopterExit03" );
                airplaneDesc.animationSpeed = 0.75f;
                airplaneDesc.initAltitude  = 0.0f;
                airplaneDesc.lastAltitude  = 0.0f;
                airplaneDesc.loweringSpeed = 0.0f;
                airplaneDesc.initOffset = xmlVector3f( child, "pos" ) * 100;
                airplaneDesc.initDirection = xmlVector3f( child, "dir" );
                airplaneDesc.initDirection.normalize();
				airplaneDesc.fixedWing = false;
                airplane = new Airplane( mission, &airplaneDesc );
            }
            else if( strcmp( static_cast<TiXmlElement*>(child)->Attribute( "type" ), "airplane" ) == 0 )
            {
                // create descriptor
                AirplaneDesc airplaneDesc;
                airplaneDesc.templateClump = parent->getScene()->findClump( "Airplane02" ); assert( airplaneDesc.templateClump );
                airplaneDesc.propellerFrame = "PropellerSound";
                airplaneDesc.propellerSound = "./res/sounds/aircrafts/airplane.ogg";
                airplaneDesc.exitPointFrames.push_back( "LeftDoor01" );
                airplaneDesc.exitPointFrames.push_back( "RightDoor01" );
                airplaneDesc.exitPointFrames.push_back( "RightDoor02" );
                airplaneDesc.cameraFrames.push_back( "Cesna_Cam" );
                airplaneDesc.cameraFrames.push_back( "Cesna_Cam1" );
                airplaneDesc.cameraFrames.push_back( "Cesna_Cam2" );
                airplaneDesc.animationSpeed = 0.5f;
                airplaneDesc.initAltitude  = 0;
                airplaneDesc.lastAltitude  = 0.0f;
                airplaneDesc.loweringSpeed = 0.0f;
				airplaneDesc.fixedWing = true;
                const char* fileName = static_cast<TiXmlElement*>(child)->Attribute( "waypoints" );
                if( fileName )
                {
                    TiXmlDocument* waypointDoc = new TiXmlDocument( fileName );
                    waypointDoc->LoadFile();

                    // collect waypoints
                    TiXmlNode* wp = waypointDoc->FirstChild();
                    if( wp ) do
                    {
                        // waypoint node?
                        if( wp->Type() == TiXmlNode::ELEMENT && strcmp( wp->Value(), "waypoint" ) == 0 )
                        {
                            AirplaneDesc::WayPoint wayPoint(
                                xmlVector3f( wp, "pos" ) * 100.0f,
                                xmlVector3f( wp, "dir" ),
                                xmlFloat( wp, "vel" ) * 100.0f
                            );
                            airplaneDesc.waypoints.push_back( wayPoint );
                        }
                        // next node
                        wp = wp->NextSibling();
                    }
                    while( wp != NULL );
                }
                else
                {
                    getCore()->logMessage( "Airplane waypoints are not specified!" );
                }

                // cast airplane object
                airplane = new Airplane( mission, &airplaneDesc );
            }
            else if( strcmp( static_cast<TiXmlElement*>(child)->Attribute( "type" ), "fixed" ) == 0 )
            {
                fixedExit = mission->getScene()->getExitPointEnclosure( xmlInt( child, "id" ) );
            }
            else
            {
                // log error
                getCore()->logMessage( "[me:error] unknown type of exit point (%s)", static_cast<TiXmlElement*>(child)->Attribute( "type" ) );
            }

            if( airplane || fixedExit )
            {
                // enumerate jumpers for this exit
                TiXmlNode* sub = child->FirstChild();
                if( sub ) do 
                {
                    // jumper node?
                    if( sub->Type() == TiXmlNode::ELEMENT && strcmp( sub->Value(), "jumper" ) == 0 )
                    {
                        std::string jumperName = static_cast<TiXmlElement*>(sub)->Attribute( "name" );
                        // player jumper?
                        if( strcmp( jumperName.c_str(), "player" ) == 0 )
                        {
                            if( playerIsPresent )
                            {
                                getCore()->logMessage( "[me:error] player already present!" );
                            }
                            else
                            {
                                // cast player
                                Jumper* player = NULL;
                                if( airplane )
                                {
                                    // cast player on airplane
                                    player = new Jumper( mission, airplane, NULL, NULL, NULL, NULL );
                                }
                                else if( fixedExit )
                                {
                                    // cast player on fixed exit point
                                    player = new Jumper( mission, NULL, fixedExit, NULL, NULL, NULL );
                                }
                                assert( player );
                                mission->setPlayer( player );

                                // setup full signature for player
                                player->setSignatureType( stFull );

                                // need smokejet?
                                if( static_cast<TiXmlElement*>(sub)->Attribute( "smokejet" ) )
                                {
                                    // cast smokejet
                                    new SmokeJet( player, xmlVector4f( sub, "smokejet" ), ::sjmLeft );
                                }

                                // create default mission goals
                                new GoalStateOfHealth( mission->getPlayer() );
                                new GoalStateOfGear( mission->getPlayer() );
                                new GoalLanding( mission->getPlayer() );
                                new GoalExperience( mission->getPlayer() );
								new GoalFreeFallTime( mission->getPlayer() );
								new GoalOpening( mission->getPlayer() );
								new GoalCanopyTime( mission->getPlayer() );

                                // raise flag
                                playerIsPresent = true;
                            }
                        }
                        // npc jumper?
                        else
                        {                            
                            // create cattoy
                            CatToy* catToy = NULL;
                            if( !static_cast<TiXmlElement*>(sub)->Attribute( "cattoy" ) )
                            {
                                getCore()->logMessage( "[me:error] cattoy for npc %s is not specified!", jumperName.c_str() );
                            }
                            std::string cattoyName = static_cast<TiXmlElement*>(sub)->Attribute( "cattoy" );
                            NPCM::iterator npcI = npcm.find( cattoyName );
                            if( strcmp( cattoyName.c_str(), "player" ) == 0 )
                            {
                                if( mission->getPlayer() )
                                {
                                    catToy = CatToy::wrap( mission->getPlayer() );
                                }
                                else
                                {
                                    getCore()->logMessage( "[me:error] failed to create player cattoy, player doesn't exists" );
                                }
                            }
                            else if( npcI != npcm.end() )
                            {
                                catToy = CatToy::wrap( npcI->second->getJumper() );
                            }
                            else
                            {
                                catToy = CatToy::loadGhostCatToy( cattoyName.c_str() );
                                if( !catToy )
                                {
                                    getCore()->logMessage( "[me:error] failed to load ghost cattoy \"%s\"", cattoyName.c_str() );
                                }
                            }

                            // find NPC
                            unsigned int recordId = 0;
                            unsigned int npcId = database::NPCInfo::getNumRecords();
                            std::string  npcName;
                            while( recordId < database::NPCInfo::getNumRecords() )
                            {
                                npcName = unicodeToAsciiz( Gameplay::iLanguage->getUnicodeString( database::NPCInfo::getRecord( recordId )->nameId ) );
                                if( strcmp( npcName.c_str(), jumperName.c_str() ) == 0 )
                                {
                                    npcId = recordId;
                                    break;
                                }
                                recordId++;
                            }
                            if( npcId == database::NPCInfo::getNumRecords() )
                            {
                                getCore()->logMessage( "[me:error] can't find NPC \"%s\"", jumperName.c_str() );
                            }
                            else
                            {
                                // create NPC jumper
                                NPC* npc = NULL;
                                if( airplane )
                                {
                                    // create NPC
                                    npc = new NPC( mission, npcId, airplane, NULL, catToy );                                    
                                    // setup cameraman behaviour
                                    npc->setProgram( new NPCCameraman( npc ) );
                                    // setup brief signature
                                    npc->getJumper()->setSignatureType( stBrief );
                                }
                                else if( fixedExit )
                                {
                                    npc = ::castGhostNPC( mission, fixedExit, catToy, npcId );
                                }

                                assert( npc );

                                // need smokejet?
                                if( static_cast<TiXmlElement*>(sub)->Attribute( "smokejet" ) )
                                {
                                    // cast smokejet
                                    new SmokeJet( npc->getJumper(), xmlVector4f( sub, "smokejet" ), ::sjmLeft );
                                }
                                
                                // add NPC to jumper list
                                npcm.insert( NPCP( jumperName, npc ) );
                            }
                        }
                    }
                    sub = sub->NextSibling();
                }
                while( sub != NULL );
            }
            // no exit point?
            else
            {
                // log error
                getCore()->logMessage( "[me:error] no exit point for mission!" );
            }
        }
        // script node?
        else if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "script" ) == 0 )
        {
            // build list of npcs
            NPCL npcs;
            for( NPCM::iterator npcI = npcm.begin(); npcI != npcm.end(); npcI++ )
            {
                npcs.push_back( npcI->second );
            }

            // create script
            new script::CommunityScript( mission->getPlayer(), npcs, npcm, child );
        }
        // soundtrack node?
        else if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "soundtrack" ) == 0 )
        {
            Gameplay::iGameplay->playSoundtrack(
                static_cast<TiXmlElement*>( child )->Attribute( "file" )
            );
        }
        // goal node?
        else if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), "goal" ) == 0 )
        {
            const char* goalType = static_cast<TiXmlElement*>( child )->Attribute( "type" );
            if( goalType )
            {
                float scale = xmlFloat( child, "scale" );
                if( scale > 30 )
                {
                    getCore()->logMessage( "[me:warning] landing zone size is out of limits!" );
                    scale = 30;
                }
                if( strcmp( goalType, "landing_accuracy" ) == 0 )
                {
                    new GoalLandingAccuracy( 
                        mission->getPlayer(),
                        xmlVector3f( child, "pos" ) * 100,
                        scale
                    );
                }
                else if( strcmp( goalType, "hard_landing" ) == 0 )
                {
                    const char* prize = static_cast<TiXmlElement*>( child )->Attribute( "prize" );
                    if( !prize )
                    {
                        getCore()->logMessage( "[me:warning] prize type is not specified, forced any type" );
                        prize = "any";
                    }
                    float scale = xmlFloat( child, "scale" );
                    if( scale > 15 )
                    {
                        getCore()->logMessage( "[me:warning] landing zone size is out of limits!" );
                        scale = 15;
                    }

                    new GoalHardLanding(
                        mission->getPlayer(),
                        xmlVector3f( child, "pos" ) * 100,
                        scale,
                        generatePrize( prize )
                    );
                }
                else if( strcmp( goalType, "smokeball" ) == 0 )
                {
                    std::vector<SmokeBallDesc> smokeballs;

                    // enumerate smokeballs
                    TiXmlNode* sub = child->FirstChild();
                    if( sub ) do 
                    {
                        // jumper node?
                        if( sub->Type() == TiXmlNode::ELEMENT && strcmp( sub->Value(), "smokeball" ) == 0 )
                        {
                            SmokeBallDesc smokeBallDesc;
                            smokeBallDesc.radius = xmlFloat( sub, "radius" ) * 100;
                            smokeBallDesc.numParticles = xmlInt( sub, "pnum" );
                            smokeBallDesc.particleRadius = xmlFloat( sub, "pscale" ) * 100;
                            smokeBallDesc.particleMass = 0.25f;
                            smokeBallDesc.color = xmlVector4f( sub, "color" );
                            smokeBallDesc.center = xmlVector3f( sub, "pos" ) * 100;
                            smokeballs.push_back( smokeBallDesc );
                        }
                        sub = sub->NextSibling();
                    }
                    while( sub != NULL );

                    // create goal
                    if( smokeballs.size() )
                    {
                        new GoalSmokeball( 
                            mission->getPlayer(), 
                            smokeballs,
                            mission->getPlayer()->getVirtues()->getMaximalBonusScore() 
                        );
                    }
                    else
                    {
                        getCore()->logMessage( "[mr:error] no smokeballs specified!" );
                    }
                }
                else
                {
                    getCore()->logMessage( "[me:error] goal type \"%s\" is unknown", goalType );
                }
            }
            else
            {
                getCore()->logMessage( "[me:error] goal type is not specified!" );
            }
        }

        // next mission node
        child = child->NextSibling();
    }
    while( child != NULL );
}