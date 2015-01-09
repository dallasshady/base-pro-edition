
#ifndef KJERAG_MISSION_ACTORS_INCLUDED
#define KJERAG_MISSION_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "npc.h"
#include "goal.h"

/**
 * clearance functions
 */

bool weatherClearanceKjerag(WeatherType weatherType);
bool windClearanceKjerag(Vector3f dir, float ambient, float blast);

/**
 * Kjerag freejumping
 */

bool equipCallback_Kjerag(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_Kjerag_Wings(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

void castingCallback_Kjerag_Freejump(Actor* parent);

/**
 * Kjerag Boogie
 */

void castingCallback_KjeragBoogie_IntervalJumps(Actor* parent);
void castingCallback_KjeragBoogie_6wayFromEP2(Actor* parent);
void castingCallback_KjeragBoogie_6wayFromTW(Actor* parent);
void castingCallback_KjeragBoogie_3wingsFromEP1(Actor* parent);
void castingCallback_KjeragBoogie_3wingsFromEP3(Actor* parent);
void castingCallback_KjeragBoogie_Proxi(Actor* parent);
void castingCallback_KjeragBoogie_HeliBASE(Actor* parent);
void castingCallback_KjeragBoogie_HeliBASE2(Actor* parent);
void castingCallback_KjeragBoogie_HeliBASE3(Actor* parent);

/**
 * Kjerag Smokeball
 */

void castingCallback_KjeragSmokeball_RGBTracking(Actor* parent);
void castingCallback_KjeragSmokeball_RGBSlalom01(Actor* parent);
void castingCallback_KjeragSmokeball_RGBZigzag(Actor* parent);

#endif