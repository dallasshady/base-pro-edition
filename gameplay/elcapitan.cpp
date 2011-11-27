
#include "headers.h"
#include "cave.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "script.h"
#include "equip.h"
#include "ostankino.h"
#include "angelfalls.h"

/**
 * clearance functions
 */

bool weatherClearanceElCapitan(WeatherType weatherType)
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

bool windClearanceElCapitan(Vector3f dir, float ambient, float blast)
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
 * equipment
 */

bool equipCallback_ElCapitan(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;
    if( !equipBestSuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

bool equipCallback_ElCapitan_Wings(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo)
{
    // equip best rig and canopy
    if( !equipBestBASEEquipment( career, windAmbient, windBlast ) ) return false;
    if( !equipBestWingsuit( career, windAmbient, windBlast ) ) return false;

    // set slider up and 36' pilotchute 
    career->getVirtues()->equipment.sliderOption = ::soUp;
    career->getVirtues()->equipment.pilotchute   = 5;

    return true;
}

/**
 * freejumps
 */

void castingCallback_ElCapitanFreeJump(Actor* parent)
{
    Mission* mission = dynamic_cast<Mission*>( parent ); assert( mission );    

    // exit point
    Enclosure* exitPoint = parent->getScene()->getExitPointEnclosure( mission->getMissionInfo()->exitPointId );

    // cast player on exit point
    mission->setPlayer( new Jumper( mission, NULL, exitPoint, NULL, NULL, NULL ) );

    // setup full signature for player
    mission->getPlayer()->setSignatureType( stFull );

    // cast goals
    new GoalStateOfHealth( mission->getPlayer() );
    new GoalStateOfGear( mission->getPlayer() );
    new GoalLanding( mission->getPlayer() );
    new GoalExperience( mission->getPlayer() );
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString(533), btProgressive, 1.0f );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_retry.ogg" );
}