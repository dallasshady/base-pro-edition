
#include "headers.h"
#include "database.h"
#include "casting.h"
#include "arbitrary.h"
#include "dropzone.h"
#include "ostankino.h"
#include "trollveggen.h"
#include "cave.h"
#include "royalgorge.h"
#include "kvly.h"
#include "kjerag.h"
#include "angelfalls.h"
#include "elcapitan.h"

using namespace database;

/**
 * mission groups
 */

static MissionInfo demoMissions[] =
{
    { 0, 524, 525, 0, 1,  60.0f, "./res/thumbnails/121.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_Opening, equipCallback_OpenAir_CommonMission, NULL },
    { 0, 534, 535, 0, 1,  60.0f, "./res/thumbnails/122.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_LandingAccuracy, equipCallback_OpenAir_CommonMission, NULL },
    { 0, 536, 537, 0, 1,  60.0f, "./res/thumbnails/123.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_TrackingPerformance, equipCallback_OpenAir_WingsuitMission, NULL },
    { 0, 543, 544, 0, 0,  90.0f, "./res/thumbnails/124.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_SpiralFreefall, equipCallback_OpenAir_WingsuitMission, NULL },
    { 0, 545, 546, 0, 2, 180.0f, "./res/thumbnails/125.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_PikeOfThrills, equipCallback_OpenAir_CommonMission, NULL },
    { 0, 547, 548, 0, 1,  60.0f, "./res/thumbnails/126.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_HardLanding, equipCallback_OpenAir_HardLanding, NULL },
    { 0, 550, 551, 0, 0,  90.0f, "./res/thumbnails/127.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_Cameraman, equipCallback_OpenAir_CommonMission, NULL },
    { 0, 559, 560, 0, 0,  90.0f, "./res/thumbnails/128.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_6way, equipCallback_OpenAir_CommonMission, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo vffMissions[] =
{
    { 0, 146, 147, mfForcedEquipment, AIRPLANE_EXIT, 2*60, "./res/thumbnails/011.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_VFF_TheBeginning, NULL, NULL },
    { 0, 203, 204, mfForcedEquipment, AIRPLANE_EXIT, 2*60, "./res/thumbnails/012.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_VFF_TheManeuvering01, NULL, NULL },
    { 0, 208, 209, mfForcedEquipment, AIRPLANE_EXIT, 2*60, "./res/thumbnails/013.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_VFF_TheManeuvering02, NULL, NULL },
    { 0, 398, 399, mfForcedEquipment, AIRPLANE_EXIT, 2*60, "./res/thumbnails/014.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_VFF_HopAndPop, NULL, NULL },
    { 0, 402, 403, mfForcedEquipment, AIRPLANE_EXIT, 2*60, "./res/thumbnails/015.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_VFF_Freefall, NULL, NULL },
    { 0, 415, 416, mfForcedEquipment, AIRPLANE_EXIT, 2*60, "./res/thumbnails/016.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_VFF_Tracking, NULL, NULL },
    { 0, 428, 429, mfForcedEquipment, AIRPLANE_EXIT, 2*60, "./res/thumbnails/017.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_VFF_Gadgets, NULL, NULL },
    { 0, 440, 441, mfForcedEquipment, AIRPLANE_EXIT, 2*60, "./res/thumbnails/018.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_VFF_Linetwists, NULL, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo baseVffMissions[] =
{
    { 0, 465, 466, mfForcedEquipment, 0, 2*60, "./res/thumbnails/021.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_BASEVFF_PCA, NULL, NULL },
    { 0, 472, 473, mfForcedEquipment, 0, 2*60, "./res/thumbnails/022.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_BASEVFF_Freefall, NULL, NULL },
    { 0, 475, 476, mfForcedEquipment, 0, 2*60, "./res/thumbnails/023.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_BASEVFF_LandingAccuracy, NULL, NULL },
    { 0, 507, 508, mfForcedEquipment, 0, 2*60, "./res/thumbnails/024.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_BASEVFF_Lineovers, NULL, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo freeJumpingMissions[] = 
{
    { 2, 487, 488, 0, 0, 35.0f, "./res/thumbnails/031.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Freejump_800, equipCallback_Freejump, NULL },
    { 1, 491, 492, 0, 0, 40.0f, "./res/thumbnails/032.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Freejump_1500, equipCallback_Freejump, NULL },
    { 0, 495, 496, 0, 0, 50.0f, "./res/thumbnails/033.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Freejump_3000, equipCallback_Freejump, NULL },
    { 0, 497, 498, 0, 0, 60.0f, "./res/thumbnails/034.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Freejump_4000, equipCallback_Freejump, NULL },
	// CLEAN VERSION
	//{ 0, 333, 498, 0, 0, 1.0f, "./res/thumbnails/034.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Freejump_Multi, equipCallback_Freejump, NULL },
	//{ 4, 499, 500, 0, 0, 60.0f, "./res/thumbnails/headdown.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Freejump_Headdown, equipCallback_Freejump, NULL },
	//{ 4, 907, 908, 0, 0, 60.0f, "./res/thumbnails/sitfly.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Freejump_Sitfly, equipCallback_Freejump, NULL },
	//{ 4, 909, 910, 0, 0, 60.0f, "./res/thumbnails/backfly.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Freejump_Backfly, equipCallback_Freejump, NULL },
    
	{ 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo skybaseMissions[] = 
{
    { 1, 503, 504, 0, 0, 60.0f, "./res/thumbnails/041.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Skybase_250, equipCallback_Skybase_250, NULL },
    { 1, 505, 506, 0, 0, 90.0f, "./res/thumbnails/042.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Skybase_500, equipCallback_Skybase_500, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo cloudyRodeoMissions[] = 
{
    { 1, 641, 642, 0, AIRPLANE_EXIT, 2*60, "./res/thumbnails/051.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_CloudyRodeo_RGB, equipCallback_FreejumpCommonSuit, NULL },
    { 1, 644, 645, 0, AIRPLANE_EXIT, 2*60, "./res/thumbnails/052.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_CloudyRodeo_SlalomRGB, equipCallback_FreejumpWingsuit, NULL },
    { 3, 646, 647, 0, AIRPLANE_EXIT, 2*60, "./res/thumbnails/053.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_CloudyRodeo_ExtremeRGB, equipCallback_CloudyRodeo_ExtremeRGB, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo moscowUndergroundMissions[] = 
{
    { 4, 516, 517, 0, 3, 60, "./res/thumbnails/111.dds", weatherClearanceAU, windClearanceAU, castingCallback_AU_PCA, equipCallback_AU_PCA, NULL },
	{ 4, 900, 901, 0, 3, 60, "./res/thumbnails/112.dds", weatherClearanceAU, windClearanceAU, castingCallback_AU_PCIH, equipCallback_AU_PCIH, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo openAirMissions[] = 
{
    { 2, 524, 525, 0, 1,  60.0f, "./res/thumbnails/121.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_Opening, equipCallback_OpenAir_CommonMission, NULL },
    { 1, 534, 535, 0, 1,  60.0f, "./res/thumbnails/122.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_LandingAccuracy, equipCallback_OpenAir_CommonMission, NULL },
    { 2, 536, 537, 0, 1,  60.0f, "./res/thumbnails/123.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_TrackingPerformance, equipCallback_OpenAir_WingsuitMission, NULL },
    { 3, 543, 544, 0, 0,  90.0f, "./res/thumbnails/124.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_SpiralFreefall, equipCallback_OpenAir_WingsuitMission, NULL },
    { 2, 545, 546, 0, 2, 180.0f, "./res/thumbnails/125.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_PikeOfThrills, equipCallback_OpenAir_CommonMission, NULL },
    { 1, 547, 548, 0, 1,  60.0f, "./res/thumbnails/126.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_HardLanding, equipCallback_OpenAir_HardLanding, NULL },
    { 1, 550, 551, 0, 0,  90.0f, "./res/thumbnails/127.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_Cameraman, equipCallback_OpenAir_CommonMission, NULL },
    { 1, 559, 560, 0, 0,  90.0f, "./res/thumbnails/128.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_6way, equipCallback_OpenAir_CommonMission, NULL },
    { 1, 685, 688, 0, 1,  60.0f, "./res/thumbnails/129.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_FlipCount, equipCallback_OpenAir_FlipCount, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo openAirSmokeballMissions[] = 
{
    { 2, 652, 653, 0, 2, 180.0f, "./res/thumbnails/131.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_RGB_Zigzag, equipCallback_OpenAir_WingsuitMission, NULL },
    { 2, 654, 655, 0, 2, 180.0f, "./res/thumbnails/132.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_RGB_Spiral, equipCallback_OpenAir_WingsuitMission, NULL },
    { 2, 646, 656, 0, 2, 180.0f, "./res/thumbnails/133.dds", weatherClearanceOpenAir, windClearanceOpenAir, castingCallback_OpenAir_RGB_Extreme, equipCallback_OpenAir_WingsuitMission, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo trollFreejumpingMissions[] =
{
    { 0, 625, 626, 0, 0, 5*60.0f, "./res/thumbnails/331.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollryggenFreejumping, equipCallback_TrollBoogie, NULL },
    { 1, 627, 628, 0, 1, 6*60.0f, "./res/thumbnails/332.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollveggenFreejumping, equipCallback_TrollBoogie, NULL },
    { 2, 629, 630, 0, 2, 7*60.0f, "./res/thumbnails/333.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollFreejumping, equipCallback_TrollBoogie, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo trollFieldTrainingMissions[] = 
{
    { 1, 663, 664, 0, 0, 5*60.0f, "./res/thumbnails/351.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollField_JumpFromRun, equipCallback_TrollField, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo trollBoogieMissions[] =
{
    { 1, 592, 593, 0, 0, 5*60.0f, "./res/thumbnails/316.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollBoogie_TrollryggenIntervalJumping, equipCallback_TrollBoogie, NULL },
    { 2, 563, 564, 0, 0, 5*60.0f, "./res/thumbnails/311.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollBoogie_Trollryggen3way, equipCallback_TrollBoogie, NULL },
    { 2, 565, 566, 0, 1, 6*60.0f, "./res/thumbnails/312.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollBoogie_Trollveggen3way, equipCallback_TrollBoogie, NULL },
    { 3, 580, 581, 0, 0, 5*60.0f, "./res/thumbnails/314.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollBoogie_TrollryggenProximity, equipCallback_TrollBoogie, NULL },
    { 3, 576, 577, 0, 2, 7*60.0f, "./res/thumbnails/313.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollBoogie_TrollProximity, equipCallback_TrollBoogie, NULL },
    { 1, 582, 583, 0, AIRPLANE_EXIT, 2*60.0f, "./res/thumbnails/315.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollBoogie_HeliBASE, equipCallback_TrollBoogie, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo trollClimbingMissions[] =
{
    { 5, 569, 570, 0, 3, 7*60.0f, "./res/thumbnails/321.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollClimbing_TrollveggenRoute, equipCallback_TrollClimbing, NULL },
    { 5, 571, 572, 0, 4, 8*60.0f, "./res/thumbnails/322.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollClimbing_TrollryggenRoute, equipCallback_TrollClimbing, NULL },
    { 5, 573, 574, 0, 5, 9*60.0f, "./res/thumbnails/323.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollClimbing_TrollRoute, equipCallback_TrollClimbing, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo trollSmokeballMissions[] = 
{
    { 3, 652, 657, 0, 0, 5*60.0f, "./res/thumbnails/341.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollSmokeball_RGB_Tracking, equipCallback_TrollClimbing, NULL },
    { 3, 644, 658, 0, 5, 9*60.0f, "./res/thumbnails/342.dds", weatherClearanceTrollveggen, windClearanceTrollveggen, castingCallback_TrollSmokeball_RGB_Slalom, equipCallback_TrollClimbing, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo caveFreejumpMissions[] =
{
    { 0, 621, 623, 0, 0, 4*60.0f, "./res/thumbnails/421.dds", weatherClearanceCave, windClearanceCave, castingCallback_CaveBase_Freejumping, equipCallback_CaveBase, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo caveBaseMissions[] =
{
    { 2, 589, 590, 0, 0, 2*60.0f, "./res/thumbnails/413.dds", weatherClearanceCave, windClearanceCave, castingCallback_CaveBase_IntervalJumping, equipCallback_CaveBase, NULL },
    { 3, 585, 586, 0, 0, 2*60.0f, "./res/thumbnails/411.dds", weatherClearanceCave, windClearanceCave, castingCallback_CaveBase_2way, equipCallback_CaveBase, NULL },
    { 3, 587, 588, 0, 0, 2*60.0f, "./res/thumbnails/412.dds", weatherClearanceCave, windClearanceCave, castingCallback_CaveBase_3way, equipCallback_CaveBase, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo royalGorgeUndergroundMissions[] = 
{
    { 0, 612, 613, 0, 0, 60.0f, "./res/thumbnails/521.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_RoyalGorgeUnderground, equipCallback_RoyalGorge, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo goFallFieldTrainingMissions[] = 
{
    { 1, 676, 677, 0, 0, 4*60.0f, "./res/thumbnails/541.dds", weatherClearanceAU, windClearanceAU, castingCallback_GoFallField_Frontflip, equipCallback_RoyalGorge_FieldTraining, NULL },
    { 2, 670, 671, 0, 0, 4*60.0f, "./res/thumbnails/542.dds", weatherClearanceCave, windClearanceCave, castingCallback_GoFallField_Backflip, equipCallback_RoyalGorge_FieldTraining, NULL },
    { 3, 680, 681, 0, 0, 4*60.0f, "./res/thumbnails/543.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_GoFallField_BackflipBackforward, equipCallback_RoyalGorge_FieldTraining, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo goFallMissions[] =
{
    { 2, 524, 615, 0, AIRPLANE_EXIT, 2*60.0f, "./res/thumbnails/514.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_GoFall_Opening, equipCallback_RoyalGorge, NULL },
    { 1, 589, 598, 0, 0, 60.0f, "./res/thumbnails/511.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_GoFall_IntervalJumping, equipCallback_RoyalGorge, NULL },
    { 2, 605, 606, 0, 0, 60.0f, "./res/thumbnails/513.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_GoFall_Leader4way, equipCallback_GoFall_Leader4way, NULL },
    { 1, 609, 610, 0, 0, 60.0f, "./res/thumbnails/513.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_GoFall_LastOne4way, equipCallback_GoFall_LastOne4way, NULL },
    { 1, 685, 686, 0, 0, 60.0f, "./res/thumbnails/516.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_GoFall_FlipCompetition, equipCallback_RoyalGorge_FieldTraining, NULL },
    { 3, 599, 600, 0, AIRPLANE_EXIT, 2*60.0f, "./res/thumbnails/512.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_GoFall_BridgeProxy, equipCallback_GoFall_BridgeProxy, NULL },
    { 3, 619, 620, 0, AIRPLANE_EXIT, 2*60.0f, "./res/thumbnails/515.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_GoFall_ExtremeTracking, equipCallback_RoyalGorge, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo goFallSmokeballMissions[] =
{
    { 2, 641, 659, 0, AIRPLANE_EXIT, 2*60.0f, "./res/thumbnails/531.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_GoFall_RGB_Tracking, equipCallback_GoFall_BridgeProxy, NULL },
    { 2, 646, 660, 0, AIRPLANE_EXIT, 2*60.0f, "./res/thumbnails/532.dds", weatherClearanceRoyalGorge, windClearanceRoyalGorge, castingCallback_GoFall_RGB_Extreme, equipCallback_GoFall_BridgeProxy, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo kvlyUndergroundMissions[] =
{
    { 0, 612, 772, 0, 0, 30.0f, "./res/thumbnails/611.dds", weatherClearanceKVLY, windClearanceKVLY, castingCallback_KVLY_Underground, equipCallback_KVLY, NULL },
	{ 1, 491, 492, 0, 0, 40.0f, "./res/thumbnails/032.dds", weatherClearanceKVLY, windClearanceKVLY, castingCallback_Freejump_4000, equipCallback_Freejump, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo tvBoogieMissions[] =
{
    { 1, 589, 775, 0, 0, 4*60.0f, "./res/thumbnails/621.dds", weatherClearanceKVLY, windClearanceKVLY, castingCallback_KVLY_IntervalJumps, equipCallback_KVLY, NULL },
    { 2, 776, 777, 0, 0, 4*60.0f, "./res/thumbnails/622.dds", weatherClearanceKVLY, windClearanceKVLY, castingCallback_KVLY_3way, equipCallback_KVLY, NULL },
    { 3, 778, 779, 0, 0, 4*60.0f, "./res/thumbnails/623.dds", weatherClearanceKVLY, windClearanceKVLY, castingCallback_KVLY_6way, equipCallback_KVLY, NULL },
    { 3, 780, 781, 0, 0, 4*60.0f, "./res/thumbnails/624.dds", weatherClearanceKVLY, windClearanceKVLY, castingCallback_KVLY_6wayWings, equipCallback_KVLY_Wings, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo kjeragFreejumpingMissions[] = 
{
    { 0, 788, 789, 0, 0, 2.0f*60.0f, "./res/thumbnails/711.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_Kjerag_Freejump, equipCallback_Kjerag, NULL },
    { 0, 791, 792, 0, 1, 2.5f*60.0f, "./res/thumbnails/712.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_Kjerag_Freejump, equipCallback_Kjerag_Wings, NULL },
    { 0, 794, 795, 0, 2, 3.0f*60.0f, "./res/thumbnails/713.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_Kjerag_Freejump, equipCallback_Kjerag, NULL },
    { 0, 797, 798, 0, 3, 3.5f*60.0f, "./res/thumbnails/714.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_Kjerag_Freejump, equipCallback_Kjerag_Wings, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo kjeragBoogieMissions[] =
{
    { 1, 801, 802, 0, 2, 3.0f*60.0f, "./res/thumbnails/721.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragBoogie_IntervalJumps, equipCallback_Kjerag, NULL },
    { 1, 803, 804, 0, 2, 3.0f*60.0f, "./res/thumbnails/722.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragBoogie_6wayFromEP2, equipCallback_Kjerag, NULL },
    { 2, 805, 806, 0, 0, 2.0f*60.0f, "./res/thumbnails/723.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragBoogie_6wayFromTW, equipCallback_Kjerag, NULL },
    { 2, 807, 808, 0, 1, 2.5f*60.0f, "./res/thumbnails/724.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragBoogie_3wingsFromEP1, equipCallback_Kjerag_Wings, NULL },
    { 2, 809, 810, 0, 3, 3.5f*60.0f, "./res/thumbnails/725.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragBoogie_3wingsFromEP3, equipCallback_Kjerag_Wings, NULL },
    { 3, 811, 812, 0, 1, 2.5f*60.0f, "./res/thumbnails/726.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragBoogie_Proxi, equipCallback_Kjerag_Wings, NULL },
    { 3, 813, 814, 0, 3, 3.5f*60.0f, "./res/thumbnails/727.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragBoogie_Proxi, equipCallback_Kjerag_Wings, NULL },
    { 3, 817, 818, 0, AIRPLANE_EXIT, 1.5f*60.0f, "./res/thumbnails/729.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragBoogie_HeliBASE, equipCallback_Kjerag, NULL },
    { 4, 815, 816, 0, AIRPLANE_EXIT, 1.5f*60.0f, "./res/thumbnails/728.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragBoogie_HeliBASE2, equipCallback_Kjerag_Wings, NULL },
    { 4, 819, 820, 0, AIRPLANE_EXIT, 1.5f*60.0f, "./res/thumbnails/72a.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragBoogie_HeliBASE3, equipCallback_Kjerag, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo kjeragSmokeballMissions[] = 
{
    { 1, 641, 821, 0, 2, 3.0f*60.0f, "./res/thumbnails/731.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragSmokeball_RGBTracking, equipCallback_Kjerag, NULL },
    { 2, 644, 822, 0, 0, 2.0f*60.0f, "./res/thumbnails/732.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragSmokeball_RGBSlalom01, equipCallback_Kjerag_Wings, NULL },
    { 3, 652, 823, AIRPLANE_EXIT, 0, 1.5f*60.0f, "./res/thumbnails/733.dds", weatherClearanceKjerag, windClearanceKjerag, castingCallback_KjeragSmokeball_RGBZigzag, equipCallback_Kjerag_Wings, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo angelFallsFreeJumps[] = 
{
    { 0, 830, 831, 0, 0, 3.0f*60.0f, "./res/thumbnails/811.dds", weatherClearanceAngelFalls, windClearanceAngelFalls, castingCallback_AngelFallsFreeJump, equipCallback_AngelFalls, NULL },
    { 0, 833, 834, 0, 1, 2.5f*60.0f, "./res/thumbnails/812.dds", weatherClearanceAngelFalls, windClearanceAngelFalls, castingCallback_AngelFallsFreeJump, equipCallback_AngelFalls, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo angelFallsBoogie[] = 
{
    { 1, 589, 835, 0, 0, 3.0f*60.0f, "./res/thumbnails/821.dds", weatherClearanceAngelFalls, windClearanceAngelFalls, castingCallback_AngelFallsIntervalJump, equipCallback_AngelFalls, NULL },
    { 2, 838, 839, 0, 1, 2.5f*60.0f, "./res/thumbnails/822.dds", weatherClearanceAngelFalls, windClearanceAngelFalls, castingCallback_AngelFallsCloseAndCloser, equipCallback_AngelFalls_Wings, NULL },
    { 2, 840, 841, 0, 1, 2.5f*60.0f, "./res/thumbnails/823.dds", weatherClearanceAngelFalls, windClearanceAngelFalls, castingCallback_AngelFalls6way, equipCallback_AngelFalls, NULL },
    { 3, 836, 837, 0, 0, 3.0f*60.0f, "./res/thumbnails/824.dds", weatherClearanceAngelFalls, windClearanceAngelFalls, castingCallback_AngelFallsNoWings, equipCallback_AngelFalls, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo elCapitanFreeJumps[] =
{
    { 0, 849, 850, 0, 0, 4.0f*60.0f, "./res/thumbnails/911.dds", weatherClearanceElCapitan, windClearanceElCapitan, castingCallback_ElCapitanFreeJump, equipCallback_ElCapitan, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

/*
    unsigned int     rank;             // minimal rank, necessible to achieve this mission
    unsigned int     nameId;           // (language) mission name id
    unsigned int     briefId;          // (language) mission briefing text id
    unsigned int     flags;            // mission flags
    unsigned int     exitPointId;      // mission exit point (or AIRPLANE_EXIT)

    float            missionTime;      // career time taken when playing this mission
    const char*      thumbnail;        // thumbnail resource    
    WeatherClearance weatherClearance; // function checks mission clearance by weather
    WindClearance    windClearance;    // function checks mission clearance by wind
    CastingCallback  castingCallback;  // mission casting procedure
    EquipCallback    equipCallback;    // mission equip procedure
    TiXmlNode*       missionNode;      // mission node (user missions only)*/

static MissionInfo burjTowerFreeJumps[] =
{
    { 0, 612, 772, 0, 0, 4*60.0f, "./res/thumbnails/611.dds", weatherClearanceKVLY, windClearanceKVLY, castingCallback_KVLY_Underground, equipCallback_KVLY, NULL },
	{ 1, 491, 492, 0, 0, 40.0f, "./res/thumbnails/032.dds", weatherClearanceDropzone, windClearanceDropzone, castingCallback_Freejump_8000, equipCallback_Freejump, NULL },
    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};

static MissionInfo mnemonicFreeJumps[] =
{
    { 0, 747, 748, 0, AIRPLANE_EXIT, 240.0f, "./res/thumbnails/611.dds", weatherClearanceKVLY, windClearanceKVLY, castingCallback_KVLY_Underground, equipCallback_KVLY, NULL },
	{ 0, 747, 748, 0, 12, 240.0f, "./res/thumbnails/611.dds", weatherClearanceKVLY, windClearanceKVLY, castingCallback_KVLY_Underground, equipCallback_KVLY, NULL },
	{ 0, 747, 748, 0, 13, 240.0f, "./res/thumbnails/611.dds", weatherClearanceKVLY, windClearanceKVLY, castingCallback_KVLY_Underground, equipCallback_KVLY, NULL },

    { 0, 0, 0, 0, 0, 0.0f, NULL, NULL, NULL, NULL }
};
/**
 * tournament database
 */

static TournamentInfo tournaments[] =
{
    { 649, 650, 1, tfArbitrary, NULL,  "./res/thumbnails/120.dds", demoMissions },
    { 125, 145, 0, tfArbitrary, "T01", "./res/thumbnails/010.dds", vffMissions },
    { 463, 464, 0, tfArbitrary, "T02", "./res/thumbnails/020.dds", baseVffMissions },
    { 485, 486, 0, tfArbitrary, "T03", "./res/thumbnails/030.dds", freeJumpingMissions },
    { 501, 502, 0, tfArbitrary, "T04", "./res/thumbnails/040.dds", skybaseMissions },
    { 639, 640, 0, tfSmokeball, "T05", "./res/thumbnails/050.dds", cloudyRodeoMissions },
    { 514, 515, 1, tfArbitrary, "T06", "./res/thumbnails/110.dds", moscowUndergroundMissions },
    { 522, 523, 1, tfFestival,  "T07", "./res/thumbnails/120.dds", openAirMissions },
    { 637, 638, 1, tfFestival,  "T08", "./res/thumbnails/130.dds", openAirSmokeballMissions },
    {  91, 624, 3, tfArbitrary, "T09", "./res/thumbnails/330.dds", trollFreejumpingMissions },
    { 661, 662, 3, tfBoogie,    "T10", "./res/thumbnails/350.dds", trollFieldTrainingMissions }, 
    { 253, 562, 3, tfBoogie,    "T11", "./res/thumbnails/310.dds", trollBoogieMissions },
    { 567, 568, 3, tfClimbing,  "T12", "./res/thumbnails/320.dds", trollClimbingMissions },
    { 637, 638, 3, tfBoogie,    "T13", "./res/thumbnails/340.dds", trollSmokeballMissions },
    {  90, 622, 2, tfArbitrary, "T14", "./res/thumbnails/420.dds", caveFreejumpMissions },
    { 254, 584, 2, tfBoogie,    "T15", "./res/thumbnails/410.dds", caveBaseMissions },
    {  92, 611, 4, tfArbitrary, "T16", "./res/thumbnails/520.dds", royalGorgeUndergroundMissions },
    { 661, 662, 4, tfFestival,  "T17", "./res/thumbnails/540.dds", goFallFieldTrainingMissions },
    { 596, 597, 4, tfFestival,  "T18", "./res/thumbnails/510.dds", goFallMissions },
    { 637, 638, 4, tfFestival,  "T19", "./res/thumbnails/530.dds", goFallSmokeballMissions },
    { 770, 771, 5, tfArbitrary, "T20", "./res/thumbnails/610.dds", kvlyUndergroundMissions },
    { 773, 774, 5, tfBoogie,    "T21", "./res/thumbnails/620.dds", tvBoogieMissions },
    { 786, 787, 6, tfArbitrary, "T22", "./res/thumbnails/710.dds", kjeragFreejumpingMissions },
    { 799, 800, 6, tfBoogie,    "T23", "./res/thumbnails/720.dds", kjeragBoogieMissions },
    { 637, 638, 6, tfBoogie,    "T24", "./res/thumbnails/730.dds", kjeragSmokeballMissions },
    { 828, 829, 7, tfArbitrary, "T25", "./res/thumbnails/810.dds", angelFallsFreeJumps },
    { 842, 829, 7, tfBoogie,    "T26", "./res/thumbnails/820.dds", angelFallsBoogie },
    { 847, 848, 8, tfArbitrary, "T27", "./res/thumbnails/910.dds", elCapitanFreeJumps },
	{ 847, 848, 9, tfArbitrary, "T28", "./res/thumbnails/910.dds", burjTowerFreeJumps },
	{ 769, 746, 10, tfArbitrary, "T29", "./res/thumbnails/610.dds", mnemonicFreeJumps },
	{ 773, 774, 10, tfBoogie,    "T30", "./res/thumbnails/620.dds", tvBoogieMissions },
	
    { 0, 0, 0, tfArbitrary, NULL, NULL, NULL }
};

/**
 * expandable list of tournaments
 */

static bool                        exTournamentsIsActual = false;
static std::vector<TournamentInfo> exTournaments;

static void buildExTournaments(void)
{
    if( !exTournamentsIsActual )
    {
        assert( exTournaments.size() == 0 );

        unsigned int i = 0;
        while( tournaments[i].nameId != 0 )
        {
            exTournaments.push_back( tournaments[i] );
            i++;
        }

        exTournamentsIsActual = true;
    }
}

/**
 * database access
 */

unsigned int TournamentInfo::getNumMissions(void)
{
    if( missions == NULL ) return 0;

    unsigned int result = 0;
    unsigned int i = 0;
    while( missions[i].nameId != 0 ) i++, result++;
    return result;
}

unsigned int TournamentInfo::getMinimalRank(void)
{
    unsigned int minimalRank = 10;
    unsigned int numMissions = getNumMissions();
    for( unsigned int i=0; i<numMissions; i++ )
    {
        if( missions[i].rank < minimalRank )
        {
            minimalRank = missions[i].rank;
        }
    }
    return minimalRank;
}

unsigned int TournamentInfo::getNumRecords(void)
{
    buildExTournaments();
    return exTournaments.size();
}

TournamentInfo* TournamentInfo::getRecord(unsigned int id)
{
    assert( id >= 0 && id < getNumRecords() );

    buildExTournaments();
    return &exTournaments[id];
}

unsigned int TournamentInfo::getDemoTournament(void)
{
    return 0;
}

void TournamentInfo::addRecord(TournamentInfo* tournamentInfo)
{
    buildExTournaments();
    exTournaments.push_back( *tournamentInfo );
}