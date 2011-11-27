
#ifndef OSTANKINO_MISSION_ACTORS_INCLUDED
#define OSTANKINO_MISSION_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "npc.h"
#include "goal.h"

/**
 * "Attic Underground"
 */

// clearance functions
bool weatherClearanceAU(WeatherType weatherType);
bool windClearanceAU(Vector3f dir, float ambient, float blast);

// common methods
void selectNPCs(Jumper* player, float skillDispersion, std::vector<unsigned int>& npcs);
void selectNPCs(Virtues* virtues, float skillDispersion, std::vector<unsigned int>& npcs);
NPC* castAssistNPC(Mission* mission, Enclosure* enclosure, Jumper* player, std::vector<unsigned int>& npcs);
NPC* castCameramanNPC(Mission* mission, Enclosure* enclosure, Jumper* player, std::vector<unsigned int>& npcs);
NPC* castGhostNPC(Mission* mission, Enclosure* enclosure, CatToy* ghost, unsigned int npcId);
NPC* castGhostNPC(Mission* mission, Enclosure* enclosure, CatToy* ghost, std::vector<unsigned int>& npcs);

// startups
void castingCallback_AU_PCA(Actor* parent);

bool equipCallback_AU_PCA(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

/**
 * "Open Air BASE"
 */

// clearance functions
bool weatherClearanceOpenAir(WeatherType weatherType);
bool windClearanceOpenAir(Vector3f dir, float ambient, float blast);

// festival startups
void castingCallback_OpenAir_Opening(Actor* parent);
void castingCallback_OpenAir_LandingAccuracy(Actor* parent);
void castingCallback_OpenAir_TrackingPerformance(Actor* parent);
void castingCallback_OpenAir_SpiralFreefall(Actor* parent);
void castingCallback_OpenAir_PikeOfThrills(Actor* parent);
void castingCallback_OpenAir_HardLanding(Actor* parent);
void castingCallback_OpenAir_Cameraman(Actor* parent);
void castingCallback_OpenAir_6way(Actor* parent);
void castingCallback_OpenAir_FlipCount(Actor* parent);

// smokeball startups
void castingCallback_OpenAir_RGB_Zigzag(Actor* parent);
void castingCallback_OpenAir_RGB_Spiral(Actor* parent);
void castingCallback_OpenAir_RGB_Extreme(Actor* parent);

// autoequip
bool equipCallback_OpenAir_CommonMission(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_OpenAir_WingsuitMission(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_OpenAir_HardLanding(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_OpenAir_FlipCount(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

#endif