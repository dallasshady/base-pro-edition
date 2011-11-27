
#include "headers.h"
#include "database.h"
#include "casting.h"
#include "version.h"

using namespace database;

/**
 * sub-database : lists of local assets
 */

static LocationInfo::AssetInfo noLocalAssets[] = 
{
    { NULL, NULL }
};

static LocationInfo::AssetInfo dropzoneAssets[] = 
{
    { "CrowdMale01", "./res/x/crowdmale01.x" },
    { "DropzoneCutscene", "./res/dropzone/cutscene.x" },
    { "RoadTraffic", "./res/dropzone/roadtraffic.x" },
    { "Airplane01", "./res/dropzone/airplane01.x" },
    { "Helicopter01", "./res/x/helicopter01.x" },
    { "Airplane02", "./res/x/cessna_static.x" },
    { NULL, NULL }
};

static LocationInfo::AssetInfo ostankinoAssets[] = 
{
    { "CrowdMale01", "./res/x/crowdmale01.x" },
    { "TrafficTrain01", "./res/ostankino/traffic_train.x" },
    { "TrafficBus01", "./res/ostankino/traffic_bus.x" },
    { "TrafficCars01", "./res/ostankino/traffic_cars.x" },
    { "OstankinoCutscene", "./res/ostankino/cutscene.x" },
    { "Helicopter01", "./res/x/helicopter01.x" },
    { "Airplane02", "./res/x/cessna_static.x" },
    { NULL, NULL }
};

static LocationInfo::AssetInfo caveAssets[] = 
{
    { "Bush01", "./res/cave/bush01.x" },
    { "Bush02", "./res/cave/bush02.x" },
    { "Tree01", "./res/cave/tree01.x" },
    { "Tree02", "./res/cave/tree02.x" },
    { "Tree03", "./res/cave/tree03.x" },
    { "Tree04", "./res/cave/tree04.x" },
    { "Tree05", "./res/cave/tree05.x" },
    { "Tree06", "./res/cave/tree06.x" },
    { "Birds", "./res/cave/birds.x" },
    { "Helicopter01", "./res/x/helicopter01.x" },
    { "CaveCutscene", "./res/cave/cutscene.x" },
    { "Airplane02", "./res/x/cessna_static.x" },
    { NULL, NULL }
};

static LocationInfo::AssetInfo trollveggenAssets[] = 
{   
    { "Helicopter01", "./res/x/helicopter01.x" },
    { "TrafficCars01", "./res/trollveggen/traffic_car.x" },
    { "TrafficTrain01", "./res/trollveggen/traffic_train.x" },
    { "TrollveggenCutscene", "./res/trollveggen/cutscene.x" },
    { "Airplane02", "./res/x/cessna_static.x" },
    { NULL, NULL }
};

static LocationInfo::AssetInfo royalGorgeBridgeAssets[] = 
{
    { "CrowdMale01", "./res/x/crowdmale01.x" },
    { "Helicopter01", "./res/x/helicopter01.x" },
    { "RoyalGorgeBridgeCutscene", "./res/royalgorge/cutscene.x" },
    { "Airplane02", "./res/x/cessna_static.x" },
    { NULL, NULL }
};

static LocationInfo::AssetInfo kvlyAssets[] = 
{
    { "KVLYCutscene", "./res/dropzone/cutscene.x" },
    { "Helicopter01", "./res/x/helicopter01.x" },
    { "Airplane02", "./res/x/cessna_static.x" },
    { NULL, NULL }
};

static LocationInfo::AssetInfo kjeragAssets[] = 
{   
    { "Helicopter01", "./res/x/helicopter01.x" },
    { "KjeragCutscene", "./res/kjerag/cutscene.x" },
    { "Airplane02", "./res/x/cessna_static.x" },
    { NULL, NULL }
};

static LocationInfo::AssetInfo angelFallsAssets[] = 
{   
    { "Helicopter01", "./res/x/helicopter01.x" },
    { "AngelFallsCutscene", "./res/angelfalls/cutscene.x" },
    { "Airplane02", "./res/x/cessna_static.x" },
    { NULL, NULL }
};

static LocationInfo::AssetInfo elCapitanAssets[] = 
{   
    { "Helicopter01", "./res/x/helicopter01.x" },
    { "AngelFallsCutscene", "./res/angelfalls/cutscene.x" },
    { "Airplane02", "./res/x/cessna_static.x" },
    { NULL, NULL }
};

/**
 * sub-database : lists of local textures
 */

static const char* dropzoneTextures[] =
{
    "./res/x/textures/crowdmale01.dds",
    "./res/x/textures/crowdmale02.dds",
    "./res/x/textures/crowdmale03.dds",
    "./res/x/textures/crowdmale04.dds",
    "./res/x/textures/crowdmale05.dds",
    "./res/x/textures/crowdmale06.dds",
    NULL
};

static const char* ostankinoTextures[] =
{
    "./res/x/textures/crowdmale01.dds",
    "./res/x/textures/crowdmale02.dds",
    "./res/x/textures/crowdmale03.dds",
    "./res/x/textures/crowdmale04.dds",
    "./res/x/textures/crowdmale05.dds",
    "./res/x/textures/crowdmale06.dds",
    NULL
};

static const char* royalGorgeBridgeTextures[] =
{
    "./res/x/textures/crowdmale01.dds",
    "./res/x/textures/crowdmale02.dds",
    "./res/x/textures/crowdmale03.dds",
    "./res/x/textures/crowdmale04.dds",
    "./res/x/textures/crowdmale05.dds",
    "./res/x/textures/crowdmale06.dds",
    NULL
};

/**
 * sub-database : lists of exit points
 */

static LocationInfo::ExitPoint noExitPoints[] = 
{
    { 0, NULL, 0 }
};

static LocationInfo::ExitPoint dropzoneExitPoints[] = 
{
    { 279, "Enclosure01", 1.0f },
    { 0, NULL }
};

static LocationInfo::ExitPoint ostankinoExitPoints[] = 
{
    { 122, "Enclosure01", 6.0f },
    { 123, "Enclosure02", 7.0f },
    { 124, "Enclosure03", 8.0f },
    { 238, "Enclosure04", 1.0f },
    { 0, NULL }
};

static LocationInfo::ExitPoint caveExitPoints[] = 
{
    { 126, "Enclosure01", 6.0f }, 
    { 0, NULL }
};

static LocationInfo::ExitPoint trollveggenExitPoints[] = 
{
    { 235, "Enclosure01", 15.0f },
    { 233, "Enclosure02", 15.0f },
    { 269, "Enclosure03", 15.0f },
    { 272, "Enclosure04", 10.0f },
    { 275, "Enclosure05", 10.0f }, 
    { 276, "Enclosure06", 10.0f }, 
    { 0, NULL }
};

static LocationInfo::ExitPoint royalGorgeBridgeExitPoints[] = 
{
    { 229, "Enclosure01", 4.0f },
    { 0, NULL }
};

static LocationInfo::ExitPoint kvlyExitPoints[] = 
{
    { 769, "Enclosure01", 8.0f },
    { 0, NULL }
};

static LocationInfo::ExitPoint kjeragExitPoints[] = 
{
    { 785, "Enclosure01", 8.0f },
    { 790, "Enclosure02", 8.0f },
    { 793, "Enclosure04", 8.0f },
    { 796, "Enclosure03", 8.0f },
    { 0, NULL }
};

static LocationInfo::ExitPoint angelFallsExitPoints[] = 
{
    { 826, "Enclosure01", 8.0f },
    { 827, "Enclosure02", 8.0f },
    { 0, NULL }
};

static LocationInfo::ExitPoint elCapitanExitPoints[] = 
{
    { 846, "Enclosure01", 8.0f },
    { 0, NULL }
};

/**
 * sub-database : lists of grass schemes
 */

static engine::GrassSpecie caveSpecies[] = 
{
    { "Salvia Divinorum", { Vector2f(0.25f,0.25f), Vector2f(0.25f,0.0f), Vector2f(0.0f,0.0f), Vector2f(0.0f,0.25f) }, Vector2f(25.0f,25.0f), Vector2f(5.0f,5.0f), 0.2f },
    { "Calea Zakatetichi", { Vector2f(0.5f,0.25f), Vector2f(0.5f,0.0f), Vector2f(0.25f,0.0f), Vector2f(0.25f,0.25f) }, Vector2f(35.0f,35.0f), Vector2f(7.5f,7.5f), 0.2f },
    { "Actaea Spicata", { Vector2f(0.75f,0.25f), Vector2f(0.75f,0.0f), Vector2f(0.5f,0.0f), Vector2f(0.5f,0.25f) }, Vector2f(25.0f,25.0f), Vector2f(2.5f,2.5f), 5.0f },
    { "Ranunculus Acris", { Vector2f(0.125f,0.75f), Vector2f(0.125f,0.25f), Vector2f(0.0f,0.25f), Vector2f(0.0f,0.75f) }, Vector2f(25.0f,95.0f), Vector2f(10.0f,25.0f), 0.1f },
    { "Calea Zinkwichi", { Vector2f(0.375f,0.75f), Vector2f(0.375f,0.25f), Vector2f(0.125f,0.25f), Vector2f(0.125f,0.75f) }, Vector2f(50.0f,75.0f), Vector2f(5.0f,7.0f), 0.1f },
    { "Cannabis Ruderalis", { Vector2f(0.625f,0.75f), Vector2f(0.625f,0.25f), Vector2f(0.375f,0.25f), Vector2f(0.375f,0.75f) }, Vector2f(50.0f,75.0f), Vector2f(5.0f,7.0f), 0.4f },
    { "Caltha Palustris", { Vector2f(0.875f,0.75f), Vector2f(0.875f,0.25f), Vector2f(0.625f,0.25f), Vector2f(0.625f,0.75f) }, Vector2f(20.0f,40.0f), Vector2f(8.0f,16.0f), 1.0f },
    { "Anemone Nemorosa", { Vector2f(0.25f,1.0f), Vector2f(0.25f,0.75f), Vector2f(0.0f,0.75f), Vector2f(0.0f,1.0f) }, Vector2f(40.0f,20.0f), Vector2f(2.0f,2.0f), 0.3f },
    { NULL, { Vector2f(0,0), Vector2f(0,0), Vector2f(0,0), Vector2f(0,0) }, Vector2f(0,0), Vector2f(0,0), 0 }
};

static engine::GrassScheme caveGrass =
{
    1000.0f,
    Vector4f( 1.0f, 1.0f, 1.0f, 1.0f ),
    caveSpecies
};

static engine::GrassSpecie kvlySpecies[] = 
{
    { "Zea mays", { Vector2f(1.0f,1.0f), Vector2f(1.0f,0.0f), Vector2f(0.0f,0.0f), Vector2f(0.0f,1.0f) }, Vector2f(75.0f,150.0f), Vector2f(12.5f,25.0f), 1.0f },
    { NULL, { Vector2f(0,0), Vector2f(0,0), Vector2f(0,0), Vector2f(0,0) }, Vector2f(0,0), Vector2f(0,0), 0 }
};

static engine::GrassScheme kvlyGrass =
{
    2000.0f,
    Vector4f( 0.75f, 0.75f, 0.75f, 1.0f ),
    kvlySpecies
};

/**
 * weather database
 */

static LocationInfo::Weather dropzoneWeatherOptions[] = 
{
    { ::wtSunny,     engine::fogExp2, 0.00000175f, Vector4f( 0.84f,0.86f,0.88f,1.0f ), 1.0f,  1.0f,  { "./res/dropzone/clearday.ba", 1000, 2500000 } },
    { ::wtVariable,  engine::fogExp2, 0.0000025f,  Vector4f( 0.84f,0.85f,0.88f,1.0f ), 0.95f, 0.95f, { "./res/dropzone/cloudyday.ba", 1000, 2500000 } },
    { ::wtCloudy,    engine::fogExp,  0.000005f,   Vector4f( 0.84f,0.85f,0.88f,1.0f ), 0.85f, 0.9f,  { "./res/dropzone/cloudyday.ba", 1000, 2500000 } },
    { ::wtLightRain, engine::fogExp2, 0.00001f,   Vector4f( 0.65f,0.66f,0.68f,1.0f ), 0.75f, 0.85f, { "./res/dropzone/rainyday.ba", 1000, 2500000 } },
    { ::wtHardRain,  engine::fogExp2, 0.000025f,   Vector4f( 0.65f,0.66f,0.68f,1.0f ), 0.75f, 0.85f, { "./res/dropzone/rainyday.ba", 1000, 2500000 } },
    { ::wtThunder,   engine::fogExp2, 0.00005f,    Vector4f( 0.65f,0.66f,0.68f,1.0f ), 0.65f, 0.80f, { "./res/dropzone/rainyday.ba", 1000, 2500000 } },
    { ::wtDatabaseEnding, engine::fogNone, 0.0f, Vector4f( 0,0,0,0 ), 0.0f, 0.0f, { NULL, 0, 0 } }
};

static LocationInfo::Weather ostankinoWeatherOptions[] = 
{
    { ::wtSunny,     engine::fogExp2, 0.0000035f, Vector4f( 0.84f,0.86f,0.88f,1.0f ), 1.0f,  1.0f,  { "./res/ostankino/clearday.ba", 1000, 1500000 } },
    { ::wtVariable,  engine::fogExp2, 0.000005f,  Vector4f( 0.65f,0.66f,0.67f,1.0f ), 0.95f, 0.95f, { "./res/ostankino/cloudyday.ba", 1000, 1500000 } },
    { ::wtCloudy,    engine::fogExp,  0.00001f,   Vector4f( 0.65f,0.66f,0.67f,1.0f ), 0.85f, 0.9f,  { "./res/ostankino/cloudyday.ba", 1000, 1500000 } },
    { ::wtLightRain, engine::fogExp2, 0.00002f,   Vector4f( 0.65f,0.66f,0.68f,1.0f ), 0.75f, 0.85f, { "./res/ostankino/rainyday.ba", 1000, 1500000 } },
    { ::wtHardRain,  engine::fogExp2, 0.00005f,   Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/ostankino/stormday.ba", 1000, 1500000 } },
    { ::wtThunder,   engine::fogExp2, 0.0001f,    Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.65f, 0.80f, { "./res/ostankino/stormday.ba", 1000, 1500000 } },
    { ::wtDatabaseEnding, engine::fogNone, 0.0f, Vector4f( 0,0,0,0 ), 0.0f, 0.0f, { NULL, 0, 0 } }
};

static LocationInfo::Weather trollveggenWeatherOptions[] = 
{
    { ::wtSunny,     engine::fogExp2, 0.000001f,   Vector4f( 0.84f,0.86f,0.88f,1.0f ), 1.0f,  1.0f,  { "./res/trollveggen/clearday.ba", 1000, 2000000 } },
    { ::wtVariable,  engine::fogExp2, 0.0000015f,  Vector4f( 0.72f,0.75f,0.78f,1.0f ), 0.95f, 0.95f, { "./res/trollveggen/cloudyday.ba", 1000, 2000000 } },
    { ::wtCloudy,    engine::fogExp,  0.00000225f, Vector4f( 0.72f,0.75f,0.78f,1.0f ), 0.85f, 0.9f,  { "./res/trollveggen/cloudyday.ba", 1000, 2000000 } },
    { ::wtLightRain, engine::fogExp2, 0.0000035f,  Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/trollveggen/rainyday.ba", 1000, 2000000 } },
    { ::wtHardRain,  engine::fogExp2, 0.000005f,   Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/trollveggen/rainyday.ba", 1000, 2000000 } },
    { ::wtThunder,   engine::fogExp2, 0.0000075f,  Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.65f, 0.80f, { "./res/trollveggen/rainyday.ba", 1000, 2000000 } },
    { ::wtDatabaseEnding, engine::fogNone, 0.0f, Vector4f( 0,0,0,0 ), 0.0f, 0.0f, { NULL, 0, 0 } }
};

static LocationInfo::Weather caveWeatherOptions[] = 
{
    { ::wtSunny,     engine::fogExp2, 0.0000025f, Vector4f( 0.51f,0.59f,0.65f,1.0f ), 1.0f,  1.0f,  { "./res/cave/clearday.ba", 1000, 2000000 } },
    { ::wtVariable,  engine::fogExp2, 0.0000045f, Vector4f( 0.51f,0.59f,0.65f,1.0f ), 0.95f, 0.95f, { "./res/cave/cloudyday.ba", 1000, 2000000 } },
    { ::wtCloudy,    engine::fogExp,  0.00001f,   Vector4f( 0.51f,0.59f,0.65f,1.0f ), 0.85f, 0.9f,  { "./res/cave/cloudyday.ba", 1000, 2000000 } },
    { ::wtLightRain, engine::fogExp2, 0.0001f,    Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/cave/rainyday.ba", 1000, 2000000 } },
    { ::wtHardRain,  engine::fogExp2, 0.0002f,    Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/cave/rainyday.ba", 1000, 2000000 } },
    { ::wtThunder,   engine::fogExp2, 0.0002f,    Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.65f, 0.80f, { "./res/cave/rainyday.ba", 1000, 2000000 } },
    { ::wtDatabaseEnding, engine::fogNone, 0.0f, Vector4f( 0,0,0,0 ), 0.0f, 0.0f, { NULL, 0, 0 } }
};

static LocationInfo::Weather bridgeWeatherOptions[] = 
{
    { ::wtSunny,     engine::fogExp2, 0.000002f,  Vector4f( 0.84f,0.86f,0.88f,1.0f ), 1.0f,  1.0f,  { "./res/royalgorge/clearday.ba", 1000, 2000000 } },
    { ::wtVariable,  engine::fogExp2, 0.0000025f, Vector4f( 0.65f,0.66f,0.67f,1.0f ), 0.95f, 0.95f, { "./res/royalgorge/cloudyday.ba", 1000, 2000000 } },
    { ::wtCloudy,    engine::fogExp2, 0.000003f,  Vector4f( 0.65f,0.66f,0.67f,1.0f ), 0.85f, 0.9f,  { "./res/royalgorge/cloudyday.ba", 1000, 2000000 } },
    { ::wtLightRain, engine::fogExp2, 0.000005f,  Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/royalgorge/rainyday.ba", 1000, 2000000 } },
    { ::wtHardRain,  engine::fogExp2, 0.0000075f, Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/royalgorge/rainyday.ba", 1000, 2000000 } },
    { ::wtThunder,   engine::fogExp2, 0.00001f,   Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.65f, 0.80f, { "./res/royalgorge/rainyday.ba", 1000, 2000000 } },
    { ::wtDatabaseEnding, engine::fogNone, 0.0f, Vector4f( 0,0,0,0 ), 0.0f, 0.0f, { NULL, 0, 0 } }
};

static LocationInfo::Weather kvlyWeatherOptions[] = 
{
    { ::wtSunny,     engine::fogExp2, 0.000002f,  Vector4f( 0.84f,0.86f,0.88f,1.0f ), 1.0f,  1.0f,  { "./res/royalgorge/clearday.ba", 1000, 2000000 } },
    { ::wtVariable,  engine::fogExp2, 0.0000025f, Vector4f( 0.65f,0.66f,0.67f,1.0f ), 0.95f, 0.95f, { "./res/royalgorge/cloudyday.ba", 1000, 2000000 } },
    { ::wtCloudy,    engine::fogExp2, 0.000003f,  Vector4f( 0.65f,0.66f,0.67f,1.0f ), 0.85f, 0.9f,  { "./res/royalgorge/cloudyday.ba", 1000, 2000000 } },
    { ::wtLightRain, engine::fogExp2, 0.000005f,  Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/royalgorge/rainyday.ba", 1000, 2000000 } },
    { ::wtHardRain,  engine::fogExp2, 0.0000075f, Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/royalgorge/rainyday.ba", 1000, 2000000 } },
    { ::wtThunder,   engine::fogExp2, 0.00001f,   Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.65f, 0.80f, { "./res/royalgorge/rainyday.ba", 1000, 2000000 } },
    { ::wtDatabaseEnding, engine::fogNone, 0.0f, Vector4f( 0,0,0,0 ), 0.0f, 0.0f, { NULL, 0, 0 } }
};

static LocationInfo::Weather kjeragWeatherOptions[] = 
{
    { ::wtSunny,     engine::fogExp2, 0.000001f,   Vector4f( 0.84f,0.86f,0.88f,1.0f ), 1.0f,  1.0f,  { "./res/trollveggen/clearday.ba", 1000, 2000000 } },
    { ::wtVariable,  engine::fogExp2, 0.0000015f,  Vector4f( 0.72f,0.75f,0.78f,1.0f ), 0.95f, 0.95f, { "./res/trollveggen/cloudyday.ba", 1000, 2000000 } },
    { ::wtCloudy,    engine::fogExp,  0.00000225f, Vector4f( 0.72f,0.75f,0.78f,1.0f ), 0.85f, 0.9f,  { "./res/trollveggen/cloudyday.ba", 1000, 2000000 } },
    { ::wtLightRain, engine::fogExp2, 0.0000035f,  Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/trollveggen/rainyday.ba", 1000, 2000000 } },
    { ::wtHardRain,  engine::fogExp2, 0.000005f,   Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/trollveggen/rainyday.ba", 1000, 2000000 } },
    { ::wtThunder,   engine::fogExp2, 0.0000075f,  Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.65f, 0.80f, { "./res/trollveggen/rainyday.ba", 1000, 2000000 } },
    { ::wtDatabaseEnding, engine::fogNone, 0.0f, Vector4f( 0,0,0,0 ), 0.0f, 0.0f, { NULL, 0, 0 } }
};

static LocationInfo::Weather angelFallsWeatherOptions[] = 
{
    { ::wtSunny,     engine::fogExp2, 0.00000035f, Vector4f( 0.51f,0.59f,0.65f,1.0f ), 1.0f,  1.0f,  { "./res/angelfalls/clearday.ba", 1000, 3000000 } },
    { ::wtVariable,  engine::fogExp2, 0.00000075f, Vector4f( 0.51f,0.59f,0.65f,1.0f ), 0.95f, 0.95f, { "./res/angelfalls/variableday.ba", 1000, 3000000 } },
    { ::wtCloudy,    engine::fogExp2, 0.00000125f,   Vector4f( 0.51f,0.59f,0.65f,1.0f ), 0.85f, 0.9f,  { "./res/angelfalls/cloudyday.ba", 1000, 3000000 } },
    { ::wtLightRain, engine::fogExp,  0.0000125f,    Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/angelfalls/rainyday.ba", 1000, 3000000 } },
    { ::wtHardRain,  engine::fogExp,  0.000025f,    Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/angelfalls/rainyday.ba", 1000, 3000000 } },
    { ::wtThunder,   engine::fogExp,  0.00005f,    Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.65f, 0.80f, { "./res/angelfalls/rainyday.ba", 1000, 3000000 } },
    { ::wtDatabaseEnding, engine::fogNone, 0.0f, Vector4f( 0,0,0,0 ), 0.0f, 0.0f, { NULL, 0, 0 } }
};

static LocationInfo::Weather elCapitanWeatherOptions[] = 
{
    { ::wtSunny,     engine::fogExp2, 0.00000035f, Vector4f( 0.51f,0.59f,0.65f,1.0f ), 1.0f,  1.0f,  { "./res/angelfalls/clearday.ba", 1000, 3000000 } },
    { ::wtVariable,  engine::fogExp2, 0.00000075f, Vector4f( 0.51f,0.59f,0.65f,1.0f ), 0.95f, 0.95f, { "./res/angelfalls/variableday.ba", 1000, 3000000 } },
    { ::wtCloudy,    engine::fogExp2, 0.00000125f,   Vector4f( 0.51f,0.59f,0.65f,1.0f ), 0.85f, 0.9f,  { "./res/angelfalls/cloudyday.ba", 1000, 3000000 } },
    { ::wtLightRain, engine::fogExp,  0.0000125f,    Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/angelfalls/rainyday.ba", 1000, 3000000 } },
    { ::wtHardRain,  engine::fogExp,  0.000025f,    Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.75f, 0.85f, { "./res/angelfalls/rainyday.ba", 1000, 3000000 } },
    { ::wtThunder,   engine::fogExp,  0.00005f,    Vector4f( 0.37f,0.40f,0.45f,1.0f ), 0.65f, 0.80f, { "./res/angelfalls/rainyday.ba", 1000, 3000000 } },
    { ::wtDatabaseEnding, engine::fogNone, 0.0f, Vector4f( 0,0,0,0 ), 0.0f, 0.0f, { NULL, 0, 0 } }
};

/**
 * reverberation datatbase
 */

LocationInfo::Reverberation dropzoneReverberation = { 1.0f, 0.33f, 0.125f, 0.25f };
LocationInfo::Reverberation moscowReverberation = { 1.0f, 0.5f, 0.33f, 0.66f };
LocationInfo::Reverberation caveReverberation = { 1.0f, 0.9f, 1.5f, 0.75f };
LocationInfo::Reverberation trollveggenReverberation = { 1.0f, 0.419f, 1.740f, 0.158f };
LocationInfo::Reverberation bridgeReverberation = { 1.0f, 0.5f, 1.0f, 0.9f };
LocationInfo::Reverberation kvlyReverberation = { 1.0f, 0.5f, 1.0f, 0.9f };

/**
 * main location database
 */

static LocationInfo locations[] = 
{
    // 0 - home dropzone
    { 
        true, 88, 0, 0, "LGD00", "./res/thumbnails/000.dds", true,
        0.0f, // no stay-in-location fee
        0, // no boogies
        0, // no festivals
        0, // no climbings
        639, // smokeball event
        824, // community event
        { 1000,1000,1000,1000,1000 },        
        { "./res/dropzone/dropzone.ba", 10, 1100000 },
        { "./res/dropzone/gamedata.ba", 0, 0 },
        dropzoneAssets,
        dropzoneTextures,
        { 0.03f, 0.73f },
        { NULL, NULL, NULL, NULL, NULL, 0.0f, 0.0f },
        dropzoneExitPoints,
        casting::castingCallbackDropzone,
        { 100.0f, 20000.0f, 1.0f, 1.25f, 0.9f, 1.0f, "./res/sounds/footsteps/metal/walk.ogg", "./res/sounds/footsteps/metal/turn.ogg" },
        dropzoneWeatherOptions,
        &dropzoneReverberation
    },
    // 1 - ostankino tv tower
    { 
        true, 89, 1243, 179, "LGD01", "./res/thumbnails/100.dds", true,
        20.0f,
        0, // no boogies
        522, // festival event name
        0, // no climbings
        0, // no smokeballs
        824, // community event
        { 1000,1000,1000,1000,1000 },        
        { "./res/ostankino/ostankino.ba", 10, 1000000 },
        { "./res/ostankino/gamedata.ba", 0, 0 },
        ostankinoAssets,
        ostankinoTextures,
        { 0.03f, 0.73f },
        { NULL, NULL, NULL, NULL, NULL, 0.0f, 0.0f },
        ostankinoExitPoints,
        casting::castingCallbackOstankino,
        { 100.0f, 20000.0f, 1.0f, 1.25f, 0.9f, 1.0f, "./res/sounds/footsteps/metal/walk.ogg", "./res/sounds/footsteps/metal/turn.ogg" },
        ostankinoWeatherOptions,
        &moscowReverberation
    },
    // 2 - cave of swallows
    { 
        true, 90, 460, 394, "LGD02", "./res/thumbnails/400.dds", false,
        15.0f,
        254, // boogie event
        0, // no festivals
        0, // no climbings
        0, // no smokeballs
        824, // community event
        { 1000,1000,1000,1000,1000 },
        { "./res/cave/caveofswallows.ba", 10, 1000000 },
        { "./res/cave/gamedata.ba", 0, 0 },
        caveAssets,
        NULL,
        { 0.025f, 0.65f },
        { &caveGrass, "mexicanvegetation", "./res/particles/mexicanvegetation.dds", "./usr/cache/caveofswallows.grass", "GrassSurface", 2000, 2500 },
        caveExitPoints,
        casting::castingCallbackCaveOfSwallows,
        { 100.0f, 20000.0f, 1.0f, 1.1f, 0.9f, 1.5f, "./res/sounds/footsteps/rock/walk.ogg", "./res/sounds/footsteps/rock/turn.ogg" },
        caveWeatherOptions,
        &caveReverberation
    },
    // 3 - trollveggen
    { 
        true, 91, 1047, 153, "LGD03", "./res/thumbnails/300.dds", true,
        30.0f,
        253, // boogie event
        0,   // no festivals
        567, // climbing event
        0,   // no smokeballs
        824, // community event
        { 1000,1000,1000,1000,1000 },
        { "./res/trollveggen/trollveggen.ba", 10, 1000000 },
        { "./res/trollveggen/gamedata.ba", 0, 0 },
        trollveggenAssets,
        NULL,
        { 0.03f, 0.63f },
        { NULL, NULL, NULL, NULL, NULL, 0.0f, 0.0f },
        trollveggenExitPoints,
        casting::castingCallbackTrollveggen,
        { 100.0f, 20000.0f, 1.0f, 1.1f, 0.9f, 1.5f, "./res/sounds/footsteps/rock/walk.ogg", "./res/sounds/footsteps/rock/turn.ogg" },
        trollveggenWeatherOptions,
        &trollveggenReverberation
    },
    // 4 - royal gorge bridge
    { 
        true, 92, 398, 303, "LGD04", "./res/thumbnails/500.dds", false,
        25.0f,
        0,   // no boogies
        596, // festival event
        0,   // no climbings
        0,   // no smokeballs
        824, // community event
        { 1000,1000,1000,1000,1000 },
        { "./res/royalgorge/royalgorgebridge.ba", 10, 1000000 },
        { "./res/royalgorge/gamedata.ba", 0, 0 },
        royalGorgeBridgeAssets,
        royalGorgeBridgeTextures,
        { 0.0275f, 0.8f },
        { NULL, NULL, NULL, NULL, NULL, 0.0f, 0.0f },
        royalGorgeBridgeExitPoints,
        casting::castingCallbackRoyalGorgeBridge,
        { 100.0f, 20000.0f, 1.0f, 1.25f, 0.9f, 1.0f, "./res/sounds/footsteps/metal/walk.ogg", "./res/sounds/footsteps/metal/turn.ogg" },
        bridgeWeatherOptions,
        &bridgeReverberation
    },
    // 5 - kvly-tv mast
    { 
        #ifdef ADDON_WEB_KK
            true, 
        #else
            false,
        #endif
        768, 464, 256, "LGD05", "./res/thumbnails/600.dds", true,
        25.0f,
        773, // boogie event
        0, // no festival
        0, // no climbings
        0, // no smokeballs
        824, // community event
        { 1000,1000,1000,1000,1000 },
        { "./res/kvly/kvly.ba", 10, 1000000 },
        { "./res/kvly/gamedata.ba", 0, 0 },
        kvlyAssets,
        NULL, // no extra textures
        { 0.0275f, 0.8f },
        { &kvlyGrass, "corn", "./res/particles/corn.dds", "./usr/cache/kvly.grass", "GrassSurface", 7500, 10000 },
        kvlyExitPoints,
        casting::castingCallbackKVLY,
        { 100.0f, 20000.0f, 1.0f, 1.25f, 0.9f, 1.0f, "./res/sounds/footsteps/metal/walk.ogg", "./res/sounds/footsteps/metal/turn.ogg" },
        kvlyWeatherOptions,
        &kvlyReverberation
    },
    // 6 - kjerag
    { 
        #ifdef ADDON_WEB_KK
            true, 
        #else
            false,
        #endif
        784, 1094, 185, "LGD06", "./res/thumbnails/700.dds", true,
        25.0f,
        799, // boogie event
        0, // no festivals
        0, // no climbings
        0, // no smokeballs
        824, // community event
        { 1000,1000,1000,1000,1000 },
        { "./res/kjerag/kjerag.ba", 10, 1000000 },
        { "./res/kjerag/gamedata.ba", 0, 0 },
        kjeragAssets,
        NULL,
        { 0.02825f, 0.675f },
        { NULL, NULL, NULL, NULL, NULL, 0.0f, 0.0f },
        kjeragExitPoints,
        casting::castingCallbackKjerag,
        { 100.0f, 20000.0f, 1.0f, 1.1f, 0.9f, 1.5f, "./res/sounds/footsteps/rock/walk.ogg", "./res/sounds/footsteps/rock/turn.ogg" },
        kjeragWeatherOptions,
        &trollveggenReverberation
    },
    // 7 - angel falls
    { 
        #ifdef ADDON_WEB_KK
            true, 
        #else
            false,
        #endif
        832, 328*2, 241*2, "LGD07", "./res/thumbnails/800.dds", true,
        25.0f,
        842, // angel falls boogie
        0, // no festivals
        0, // no climbings
        0, // no smokeballs
        824, // community event
        { 1000,1000,1000,1000,1000 },
        { "./res/angelfalls/angelfalls.ba", 10, 1500000 },
        { "./res/angelfalls/gamedata.ba", 0, 0 },
        angelFallsAssets,
        NULL,
        { 0.025f, 0.5f },
        { NULL, NULL, NULL, NULL, NULL, 0.0f, 0.0f },
        angelFallsExitPoints,
        casting::castingCallbackAngelFalls,
        { 100.0f, 20000.0f, 1.0f, 1.1f, 0.9f, 1.5f, "./res/sounds/footsteps/rock/walk.ogg", "./res/sounds/footsteps/rock/turn.ogg" },
        angelFallsWeatherOptions,
        &trollveggenReverberation
    },
    // 8 - el capitan
    { 
        #ifdef ADDON_WEB_EL_CAPITAN
            true, 
        #else
            false,
        #endif
        846, 340, 301, "LGD08", "./res/thumbnails/900.dds", true,
        25.0f,
        0, // el capitan boogie
        0, // no festivals
        0, // no climbings
        0, // no smokeballs
        824, // community event
        { 1000,1000,1000,1000,1000 },
        { "./res/elcapitan/elcapitan.ba", 10, 1500000 },
        { "./res/elcapitan/gamedata.ba", 0, 0 },
        elCapitanAssets,
        NULL,
        { 0.025f, 0.5f },
        { NULL, NULL, NULL, NULL, NULL, 0.0f, 0.0f },
        elCapitanExitPoints,
        casting::castingCallbackElCapitan,
        { 100.0f, 20000.0f, 1.0f, 1.1f, 0.9f, 1.5f, "./res/sounds/footsteps/rock/walk.ogg", "./res/sounds/footsteps/rock/turn.ogg" },
        elCapitanWeatherOptions,
        &trollveggenReverberation
    },
    { 0, 0, 0 }
};

unsigned int LocationInfo::getNumRecords(void)
{
    unsigned int result = 0;
    unsigned int i = 0;
    while( locations[i].nameId != 0 ) i++, result++;
    return result;
}

LocationInfo* LocationInfo::getRecord(unsigned int id)
{
    assert( id >= 0 && id < getNumRecords() );
    return locations + id;
}