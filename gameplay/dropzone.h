
#ifndef DROPZONE_MISSION_ACTORS_INCLUDED
#define DROPZONE_MISSION_ACTORS_INCLUDED

#include "headers.h"
#include "mission.h"
#include "jumper.h"
#include "goal.h"

/**
 * dropzone clearance functions
 */

bool weatherClearanceDropzone(WeatherType weatherType);
bool windClearanceDropzone(Vector3f dir, float ambient, float blast);

/**
 * virtual freefall missions : startup callbacks
 */

Virtues::Equipment selectSkydivingEquipment(Career* career);

void castingCallback_VFF_TheBeginning(Actor* parent);
void castingCallback_VFF_TheManeuvering01(Actor* parent);
void castingCallback_VFF_TheManeuvering02(Actor* parent);
void castingCallback_VFF_HopAndPop(Actor* parent);
void castingCallback_VFF_Freefall(Actor* parent);
void castingCallback_VFF_Tracking(Actor* parent);
void castingCallback_VFF_Gadgets(Actor* parent);
void castingCallback_VFF_Linetwists(Actor* parent);

/**
 * BASE VFF : startup callbacks
 */

Virtues::Equipment selectBASEEquipment(Career* career, float windAmbient, float windBlast);

void castingCallback_BASEVFF_PCA(Actor* parent);
void castingCallback_BASEVFF_Freefall(Actor* parent);
void castingCallback_BASEVFF_LandingAccuracy(Actor* parent);
void castingCallback_BASEVFF_Lineovers(Actor* parent);

/**
 * freejumps from dropzone
 */

void castSkydivers(Mission* mission, Airplane* airplane, unsigned int numSkydivers);
void castFreejump(Actor* parent, float altitude, int pose);

void castingCallback_Freejump_800(Actor* parent);
void castingCallback_Freejump_1500(Actor* parent);
void castingCallback_Freejump_3000(Actor* parent);
void castingCallback_Freejump_4000(Actor* parent);
void castingCallback_Freejump_8000(Actor* parent);
void castingCallback_Freejump_4000_Mass(Actor *parent);
void castingCallback_Freejump_Multi(Actor* parent);
void castingCallback_Freejump_Headdown(Actor* parent);
void castingCallback_Freejump_Sitfly(Actor* parent);
void castingCallback_Freejump_Backfly(Actor* parent);

bool equipCallback_Freejump(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_FreejumpCommonSuit(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_FreejumpWingsuit(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

/**
 * skybase from dropzone
 */

void castSkybaserForGhost(Mission* mission, Airplane* airplane, CatToy* ghost);
void castSkybaseJump(Actor* parent, float altitude, CatToy* ghost);

void castingCallback_Skybase_250(Actor* parent);
void castingCallback_Skybase_500(Actor* parent);

bool equipCallback_Skybase_250(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);
bool equipCallback_Skybase_500(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

/**
 * smokeball events
 */ 

void castingCallback_CloudyRodeo_RGB(Actor* parent);
void castingCallback_CloudyRodeo_SlalomRGB(Actor* parent);
void castingCallback_CloudyRodeo_ExtremeRGB(Actor* parent);

bool equipCallback_CloudyRodeo_ExtremeRGB(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

#endif