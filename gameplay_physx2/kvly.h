
#ifndef KVLY_MISSION_ACTORS_INCLUDED
#define KVLY_MISSION_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "npc.h"
#include "goal.h"

/**
 * clearance functions
 */

bool weatherClearanceKVLY(WeatherType weatherType);
bool windClearanceKVLY(Vector3f dir, float ambient, float blast);

/**
 * KVLY freejumping
 */

bool equipCallback_KVLY(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_KVLY_Wings(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
void castingCallback_KVLY_Underground(Actor* parent);

/**
 * TV Boogie
 */

void castingCallback_KVLY_IntervalJumps(Actor* parent);
void castingCallback_KVLY_3way(Actor* parent);
void castingCallback_KVLY_6way(Actor* parent);
void castingCallback_KVLY_6wayWings(Actor* parent);

#endif