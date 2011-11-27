
#ifndef ROYALGORGE_MISSION_ACTORS_INCLUDED
#define ROYALGORGE_MISSION_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "goal.h"

/**
 * dropzone clearance functions
 */

bool weatherClearanceRoyalGorge(WeatherType weatherType);
bool windClearanceRoyalGorge(Vector3f dir, float ambient, float blast);

/**
 * go fall games
 */

void castingCallback_GoFall_Opening(Actor* parent);
void castingCallback_GoFall_IntervalJumping(Actor* parent);
void castingCallback_GoFall_Leader4way(Actor* parent);
void castingCallback_GoFall_LastOne4way(Actor* parent);
void castingCallback_GoFall_BridgeProxy(Actor* parent);
void castingCallback_GoFall_ExtremeTracking(Actor* parent);
void castingCallback_GoFall_FlipCompetition(Actor* parent);

bool equipCallback_RoyalGorge(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_GoFall_Leader4way(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_GoFall_LastOne4way(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_GoFall_BridgeProxy(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

/**
 * underground activity
 */

void castingCallback_RoyalGorgeUnderground(Actor* parent);

/**
 * smokeball missions
 */

void castingCallback_GoFall_RGB_Tracking(Actor* parent);
void castingCallback_GoFall_RGB_Extreme(Actor* parent);

/**
 * field training
 */

bool equipCallback_RoyalGorge_FieldTraining(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

void castingCallback_GoFallField_Frontflip(Actor* parent);
void castingCallback_GoFallField_Backflip(Actor* parent);
void castingCallback_GoFallField_BackflipBackforward(Actor* parent);

#endif