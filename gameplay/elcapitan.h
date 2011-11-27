
#ifndef EL_CAPITAN_MISSION_ACTORS_INCLUDED
#define EL_CAPITAN_MISSION_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "npc.h"
#include "goal.h"

/**
 * clearance functions
 */

bool weatherClearanceElCapitan(WeatherType weatherType);
bool windClearanceElCapitan(Vector3f dir, float ambient, float blast);

/**
 * angel falls freejumping
 */

bool equipCallback_ElCapitan(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_ElCapitan_Wings(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

void castingCallback_ElCapitanFreeJump(Actor* parent);

/**
 * angel falls boogie
 */

/*
void castingCallback_AngelFallsIntervalJump(Actor* parent);
void castingCallback_AngelFallsNoWings(Actor* parent);
void castingCallback_AngelFallsCloseAndCloser(Actor* parent);
void castingCallback_AngelFalls6way(Actor* parent);
*/

#endif