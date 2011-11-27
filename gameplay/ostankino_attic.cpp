
#include "headers.h"
#include "ostankino.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "script.h"
#include "equip.h"
#include "version.h"

/**
 * clearance functions
 */

bool weatherClearanceAU(WeatherType weatherType)
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

bool windClearanceAU(Vector3f dir, float ambient, float blast)
{
    Vector3f westWind( -1.0f, 0.0f, 0.0f );

    // first, check wind direction
    if( Vector3f::dot( dir, westWind ) > 0.77f )
    {
        return false;
    }

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
 * common methods
 */

void selectNPCs(Jumper* player, float skillDispersion, std::vector<unsigned int>& npcs)
{
    npcs.clear();

    // detect player level
    float playerLevel = 0.25f * (
        player->getVirtues()->getEnduranceSkill() + 
        player->getVirtues()->getPerceptionSkill() +
        player->getVirtues()->getRiggingSkill() +
        player->getVirtues()->getTrackingSkill()
    );

    // detect player is a LICENSED_CHAR
    bool playerIsLicensed = player->isPlayer() && player->getScene()->getCareer()->getLicensedFlag();

    // build list of NPCs
    database::NPCInfo::select( playerLevel, skillDispersion, !playerIsLicensed, npcs );
}

void selectNPCs(Virtues* virtues, float skillDispersion, std::vector<unsigned int>& npcs)
{
    npcs.clear();

    // detect player level
    float playerLevel = 0.25f * (
        virtues->getEnduranceSkill() + 
        virtues->getPerceptionSkill() +
        virtues->getRiggingSkill() +
        virtues->getTrackingSkill()
    );

    // build list of NPCs
    database::NPCInfo::select( playerLevel, skillDispersion, !false, npcs );
}

NPC* castAssistNPC(Mission* mission, Enclosure* enclosure, Jumper* player, std::vector<unsigned int>& npcs)
{
    assert( npcs.size() );

    if( npcs.size() )
    {
        // select NPC
        unsigned int index = getCore()->getRandToolkit()->getUniformInt() % npcs.size();
        unsigned int npcId = npcs[index];
        npcs.erase( &npcs[index] );
        // create NPC
        NPC* npc = new NPC( mission, npcId, NULL, enclosure, CatToy::wrap( mission->getPlayer() ) );
        // setup assistant behaviour
        npc->setProgram( new NPCAssist( npc, player ) );
        // setup brief signature
        npc->getJumper()->setSignatureType( stBrief );
        return npc;
    }

    return NULL;
}

/**
 * startups
 */

namespace script { class AU_PCA_Script : public Script
{
private:
    NPC* _assistant;
public:
    /**
     * script core
     */
    AU_PCA_Script(Jumper* player, NPC* assistant) : Script( player )
    {
        assert( assistant );
        _assistant = assistant;

        // disable phase change
        player->enablePhase( false );

        // create routine
        _routine = new Assist( player, _assistant, player->getScene()->getLocation()->getWindDirection() );
    }
    virtual ~AU_PCA_Script()
    {
    }
    virtual void onUpdateActivity(float dt)
    {
        Script::onUpdateActivity( dt );

        if( _routine && _routine->isExecuted() )
        {
            delete _routine;
            _routine = NULL;
        }
    }
}; }

void castingCallback_AU_PCA(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // select NPCs
    std::vector<unsigned int> npcs;
    selectNPCs( mission->getPlayer(), 0.126f, npcs );

    // cast assist NPC for player
    NPC* assistant = castAssistNPC( mission, exitPoint, mission->getPlayer(), npcs );
    assert( assistant );
    if( assistant ) 
    {
        // cast script
        new script::AU_PCA_Script( mission->getPlayer(), assistant );
    }

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btUnderground, 1.5f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_delinquent.ogg" );
}

bool equipCallback_AU_PCA(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;

    // set slider down and 48' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soDown;
    career->getVirtues()->equipment.pilotchute   = 0;

    return true;
}