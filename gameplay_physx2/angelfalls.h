
#ifndef ANGEL_FALLS_MISSION_ACTORS_INCLUDED
#define ANGEL_FALLS_MISSION_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "npc.h"
#include "goal.h"

/**
 * clearance functions
 */

bool weatherClearanceAngelFalls(WeatherType weatherType);
bool windClearanceAngelFalls(Vector3f dir, float ambient, float blast);

/**
 * angel falls freejumping
 */

bool equipCallback_AngelFalls(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_AngelFalls_Wings(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

void castingCallback_AngelFallsFreeJump(Actor* parent);

/**
 * angel falls boogie
 */

void castingCallback_AngelFallsIntervalJump(Actor* parent);
void castingCallback_AngelFallsNoWings(Actor* parent);
void castingCallback_AngelFallsCloseAndCloser(Actor* parent);
void castingCallback_AngelFalls6way(Actor* parent);

#endif