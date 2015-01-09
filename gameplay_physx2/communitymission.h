
#ifndef COMMUNITY_MISSION_ACTORS_INCLUDED
#define COMMUNITY_MISSION_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "goal.h"

/**
 * community clearance functions
 */

bool weatherClearanceCommunity(WeatherType weatherType);
bool windClearanceCommunity(Vector3f dir, float ambient, float blast);

/**
 * community mission startup callback
 */

bool equipCallback_CommunityMission(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
void castingCallback_CommunityMission(Actor* parent);

#endif