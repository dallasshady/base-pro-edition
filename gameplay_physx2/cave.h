
#ifndef CAVE_MISSION_ACTORS_INCLUDED
#define CAVE_MISSION_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "npc.h"
#include "goal.h"

/**
 * clearance functions
 */

bool weatherClearanceCave(WeatherType weatherType);
bool windClearanceCave(Vector3f dir, float ambient, float blast);

/**
 * "cave base" missions
 */

void castingCallback_CaveBase_IntervalJumping(Actor* parent);
void castingCallback_CaveBase_2way(Actor* parent);
void castingCallback_CaveBase_3way(Actor* parent);

bool equipCallback_CaveBase(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

/** 
 * cave freejumping
 */

void castingCallback_CaveBase_Freejumping(Actor* parent);

#endif