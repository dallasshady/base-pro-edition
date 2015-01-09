
#include "headers.h"
#include "trollveggen.h"
#include "npc.h"
#include "npccameraman.h"
#include "npcassist.h"
#include "landingaccuracy.h"
#include "script.h"
#include "smokejet.h"
#include "ostankino.h"
#include "equip.h"

void castingCallback_TrollryggenFreejumping(Actor* parent)
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
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString( 533 ), btUnderground, 0.75f );
	new GoalFreeFallTime( mission->getPlayer() );
	new GoalCanopyTime( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_requiem.ogg" );
}

void castingCallback_TrollveggenFreejumping(Actor* parent)
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
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString( 533 ), btUnderground, 0.875f );
	new GoalFreeFallTime( mission->getPlayer() );
	new GoalCanopyTime( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_requiem.ogg" );
}

void castingCallback_TrollFreejumping(Actor* parent)
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
    new GoalBonus( mission->getPlayer(), Gameplay::iLanguage->getUnicodeString( 533 ), btUnderground, 1.0f );
	new GoalFreeFallTime( mission->getPlayer() );
	new GoalCanopyTime( mission->getPlayer() );

    // play original music for this mission
    Gameplay::iGameplay->playSoundtrack( "./res/sounds/music/dirty_moleculas_requiem.ogg" );
}