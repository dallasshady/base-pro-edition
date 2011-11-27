
#ifndef TROLLVEGGEN_MISSION_ACTORS_INCLUDED
#define TROLLVEGGEN_MISSION_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "npc.h"
#include "goal.h"

/**
 * clearance functions
 */

bool weatherClearanceTrollveggen(WeatherType weatherType);
bool windClearanceTrollveggen(Vector3f dir, float ambient, float blast);

/**
 * freejumping
 */

void castingCallback_TrollryggenFreejumping(Actor* parent);
void castingCallback_TrollveggenFreejumping(Actor* parent);
void castingCallback_TrollFreejumping(Actor* parent);

/**
 * "troll boogie" missions
 */

void castingCallback_TrollBoogie_TrollryggenIntervalJumping(Actor* parent);
void castingCallback_TrollBoogie_Trollryggen3way(Actor* parent);
void castingCallback_TrollBoogie_Trollveggen3way(Actor* parent);
void castingCallback_TrollBoogie_TrollProximity(Actor* parent);
void castingCallback_TrollBoogie_TrollryggenProximity(Actor* parent);
void castingCallback_TrollBoogie_HeliBASE(Actor* parent);

bool equipCallback_TrollBoogie(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

/**
 * "troll climbing" missions
 */

void castingCallback_TrollClimbing_TrollveggenRoute(Actor* parent);
void castingCallback_TrollClimbing_TrollryggenRoute(Actor* parent);
void castingCallback_TrollClimbing_TrollRoute(Actor* parent);

bool equipCallback_TrollClimbing(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

/**
 * "troll smokeball" mission
 */

void castingCallback_TrollSmokeball_RGB_Tracking(Actor* parent);
void castingCallback_TrollSmokeball_RGB_Slalom(Actor* parent);

/**
 * "troll field program" mission
 */

bool equipCallback_TrollField(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

void castingCallback_TrollField_JumpFromRun(Actor* parent);

#endif