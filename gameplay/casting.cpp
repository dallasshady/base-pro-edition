
#include "headers.h"
#include "casting.h"
#include "scene.h"
#include "gameplay.h"
#include "sound.h"
#include "traffic.h"
#include "animobject.h"
#include "crowd.h"
#include "airplane.h"
#include "forest.h"
#include "cutscene.h"

/**
 * LOD calculator for forest batches
 */

float getTreeLODDistance(unsigned int lodId, float forestFactor)
{
    assert( lodId <= 4 );
    assert( forestFactor > 0 && forestFactor <= 1 );

    switch( lodId )
    {
    case 0: return 2500.0f + 2500.0f * forestFactor;   // 2500  to 5000
    case 1: return 5000.0f + 5000.0f * forestFactor;   // 5000  to 10000
    case 2: return 7500.0f + 12500.0f * forestFactor;  // 7500  to 20000
    case 3: return 25000.0f + 75000.0f * forestFactor; // 25000 to 100000
    case 4: return 50000.0f + 150000.0f * forestFactor; // 50000 to 200000
    }
    return 0;
}

/**
 * wind casting
 */

static bool isAspenValue(float value, float minValue, float maxValue)
{
    return value > minValue + 0.25f * ( maxValue - minValue );
}

static bool isGustyValue(float value, float minValue, float maxValue)
{
    return value > minValue + 0.5f * ( maxValue - minValue );
}

static void castingCallbackOrdinaryWind(Actor* parent)
{
    // retrieve wind    
    float windAmbient = parent->getScene()->getLocation()->getWindAmbient();
    float windBlast   = parent->getScene()->getLocation()->getWindBlast();

    // calm?
    if( windAmbient >= database::windCalmAmbientMin && windAmbient <= database::windCalmAmbientMax )
    {
        // quiet monotone wind sound
        AmbientSoundDesc desc;
        desc.processingMode = apmAltitude;
        desc.position       = Vector3f( 0,0,0 );
        desc.maxValue       = 20000.0f;
        desc.maxValueGain   = 0.125f;
        desc.minValue       = 0;
        desc.minValueGain   = 0.0625f;
        new AmbientSound( parent, "./res/sounds/wind/monotone_wind.ogg", &desc );
    }
    // light wind?
    else if( windAmbient >= database::windLightAmbientMin && windAmbient <= database::windLightAmbientMax )
    {
        // wind is aspen?
        if( isAspenValue( windBlast, database::windLightBlastMin, database::windLightBlastMax ) )
        {
            AmbientSoundDesc desc;
            desc.processingMode = apmAltitude;
            desc.position       = Vector3f( 0,0,0 );
            desc.maxValue       = 20000.0f;
            desc.maxValueGain   = 0.25f;
            desc.minValue       = 0;
            desc.minValueGain   = 0.125f;
            new AmbientSound( parent, "./res/sounds/wind/aspen_wind.ogg", &desc );
        }
        // wind is gusty?
        else if( isGustyValue( windBlast, database::windLightBlastMin, database::windLightBlastMax ) )
        {
            AmbientSoundDesc desc;
            desc.processingMode = apmAltitude;
            desc.position       = Vector3f( 0,0,0 );
            desc.maxValue       = 20000.0f;
            desc.maxValueGain   = 0.25f;
            desc.minValue       = 0;
            desc.minValueGain   = 0.125f;
            new AmbientSound( parent, "./res/sounds/wind/gusty_wind.ogg", &desc );
        }
        // wind is monotone
        else
        {
            AmbientSoundDesc desc;
            desc.processingMode = apmAltitude;
            desc.position       = Vector3f( 0,0,0 );
            desc.maxValue       = 20000.0f;
            desc.maxValueGain   = 0.25f;
            desc.minValue       = 0;
            desc.minValueGain   = 0.125f;
            new AmbientSound( parent, "./res/sounds/wind/monotone_wind.ogg", &desc );
        }
    }
    // medium wind?
    else if( windAmbient >= database::windMediumAmbientMin && windAmbient <= database::windMediumAmbientMax )
    {
        // wind is aspen?
        if( isAspenValue( windBlast, database::windLightBlastMin, database::windLightBlastMax ) )
        {
            AmbientSoundDesc desc;
            desc.processingMode = apmAltitude;
            desc.position       = Vector3f( 0,0,0 );
            desc.maxValue       = 20000.0f;
            desc.maxValueGain   = 0.25f;
            desc.minValue       = 0;
            desc.minValueGain   = 0.125f;
            new AmbientSound( parent, "./res/sounds/wind/aspen_wind.ogg", &desc );
        }
        // wind is gusty?
        else if( isGustyValue( windBlast, database::windLightBlastMin, database::windLightBlastMax ) )
        {
            AmbientSoundDesc desc;
            desc.processingMode = apmAltitude;
            desc.position       = Vector3f( 0,0,0 );
            desc.maxValue       = 20000.0f;
            desc.maxValueGain   = 0.25f;
            desc.minValue       = 0;
            desc.minValueGain   = 0.125f;
            new AmbientSound( parent, "./res/sounds/wind/gusty_wind.ogg", &desc );
        }
        // wind is monotone
        else
        {
            AmbientSoundDesc desc;
            desc.processingMode = apmAltitude;
            desc.position       = Vector3f( 0,0,0 );
            desc.maxValue       = 20000.0f;
            desc.maxValueGain   = 0.25f;
            desc.minValue       = 0;
            desc.minValueGain   = 0.125f;
            new AmbientSound( parent, "./res/sounds/wind/monotone_wind.ogg", &desc );
        }
    }
    // strong wind?
    else
    {
        // lous storm sound
        AmbientSoundDesc desc;
        desc.processingMode = apmAltitude;
        desc.position       = Vector3f( 0,0,0 );
        desc.maxValue       = 20000.0f;
        desc.maxValueGain   = 0.25f;
        desc.minValue       = 0;
        desc.minValueGain   = 0.125f;
        new AmbientSound( parent, "./res/sounds/wind/storm_wind.ogg", &desc );
    }
}

/**
 * rain casting
 */

static void castingCallbackOrdinaryRain(Actor* parent)
{
    // rain ambient processor
    AmbientSoundDesc desc;
    desc.processingMode = apmAltitude;
    desc.position       = Vector3f( 0,0,0 );
    desc.maxValue       = 10000.0f;
    desc.maxValueGain   = 0.0625f;
    desc.minValue       = 0;
    desc.minValueGain   = 0.25f;

    // by weather
    switch( parent->getScene()->getLocation()->getWeather() )
    {
    case ::wtLightRain:
        new AmbientSound( parent, "./res/sounds/rain/light.ogg", &desc );
        break;
    case ::wtHardRain:
        new AmbientSound( parent, "./res/sounds/rain/hard.ogg", &desc );
        break;
    case ::wtThunder:
        new AmbientSound( parent, "./res/sounds/rain/storm.ogg", &desc );
        break;
    }
}

/**
 * helper for traffic casting 
 */

static void castingCallbackTraffic(Actor* parent, engine::IClump* source, unsigned int density)
{
    assert( source );

    for( unsigned int i=0; i<density; i++ )
    {
        TrafficDesc trafficDesc;
        trafficDesc.source     = source;
        trafficDesc.animSpeed  = 0.25f;
        trafficDesc.animOffset = ( float( i ) / float( density ) );
        new Traffic( parent, &trafficDesc );
    }
}

/**
 * casting for ostankino
 */

void casting::castingCallbackOstankino(Actor* parent)
{
    // cast wind by ordinary method
    castingCallbackOrdinaryWind( parent );

    // cast rain by ordinary method
    castingCallbackOrdinaryRain( parent );

    // city ambient
    AmbientSoundDesc desc;
    desc.processingMode = apmAltitude;
    desc.position       = Vector3f( 0,0,0 );
    desc.maxValue       = 30000.0f;
    desc.maxValueGain   = 0.01f;
    desc.minValue       = 0;
    desc.minValueGain   = 0.25f;
    new AmbientSound( parent, "./res/sounds/ostankino/city.ogg", &desc );

    // crowd ambient
    desc.processingMode = apmDistance;
    desc.position       = Vector3f( 22457, 422, -1366 );
    desc.maxValue       = 15000.0f;
    desc.maxValueGain   = 0.01f;
    desc.minValue       = 0;
    desc.minValueGain   = 0.25f;
    new AmbientSound( parent, "./res/sounds/ostankino/crowd.ogg", &desc );

    // traffic density
    double trafficDensity;
    TiXmlElement* details = Gameplay::iGameplay->getConfigElement( "details" );
    details->Attribute( "traffic", &trafficDensity );
    // 100% road traffic
    unsigned int defaultTrainDensity = 4;
    unsigned int defaultBusDensity   = 4;
    unsigned int defaultCarDensity   = 8;
    // configured road traffic
    unsigned int trainDensity = unsigned int( double( defaultTrainDensity ) * trafficDensity );
    unsigned int busDensity   = unsigned int( double( defaultBusDensity ) * trafficDensity );
    unsigned int carDensity   = unsigned int( double( defaultCarDensity ) * trafficDensity );
    if( trainDensity )
    {
        castingCallbackTraffic( parent, parent->getScene()->findClump( "TrafficTrain01" ), trainDensity );
    }
    if( busDensity )
    {
        castingCallbackTraffic( parent, parent->getScene()->findClump( "TrafficBus01" ), busDensity );
    }
    if( carDensity )
    {
        castingCallbackTraffic( parent, parent->getScene()->findClump( "TrafficCars01" ), carDensity );
    }

    // crowd density
    double crowdDensity;
    details->Attribute( "crowd", &crowdDensity );

    // crowd
    CrowdDesc crowdDesc;
    crowdDesc.extras = parent->getScene()->getExtras( "CrowdEnclosure01" );
    crowdDesc.numActors = unsigned int( 64 * crowdDensity );
    crowdDesc.numWalkingActors = 8;
    new Crowd( parent, &crowdDesc );

    double forestFactor;    
    details->Attribute( "forest", &forestFactor );
      
    if( forestFactor > 0.0f )
    {
        callback::AtomicL forestSurfaces;

        // obtain forest surface 1
        engine::IClump* forestClump01 = parent->getScene()->getExtras( "ForestSurface01" ); assert( forestClump01 );        
        forestClump01->forAllAtomics( callback::enumerateAtomics, &forestSurfaces ); assert( forestSurfaces.size() );
        engine::IAtomic* forestSurface01 = (*forestSurfaces.begin());

        // obtain forest surface 2
        forestSurfaces.clear();
        engine::IClump* forestClump02 = parent->getScene()->getExtras( "ForestSurface02" ); assert( forestClump02 );
        forestClump02->forAllAtomics( callback::enumerateAtomics, &forestSurfaces ); assert( forestSurfaces.size() );
        engine::IAtomic* forestSurface02 = (*forestSurfaces.begin());

        // forest01::tree10
        ForestDesc forestDesc1;
        forestDesc1.cache     = "./usr/cache/ostankino_forest01_tree10";
        forestDesc1.assetName = "./res/speedtrees/tree10.ba";
        forestDesc1.density   = 0.015f;
        forestDesc1.minScale  = 1.0f;
        forestDesc1.maxScale  = 1.25f;
        forestDesc1.collScale = 0.33f;
        forestDesc1.entangleFactor = 0.66f;
        forestDesc1.minImpulseFactor = 0.50f;
        forestDesc1.maxImpulseFactor = 0.75f;
        forestDesc1.ripFactor = 7.5f;
        forestDesc1.damageFactor = 1750.0f;        
        forestDesc1.surface        = forestSurface01;
        forestDesc1.bspLeafSize    = 128;
        forestDesc1.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc1.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc1.lodDistance[2] = getTreeLODDistance( 2, float(forestFactor) );
        forestDesc1.lodDistance[3] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc1 );

        // forest02::tree01
        ForestDesc forestDesc2;
        forestDesc2.cache     = "./usr/cache/ostankino_forest02_tree01";
        forestDesc2.assetName = "./res/speedtrees/tree01.ba";
        forestDesc2.density   = 0.0025f;
        forestDesc2.minScale  = 1.25f;
        forestDesc2.maxScale  = 1.75f;
        forestDesc2.collScale = 0.5f;
        forestDesc2.entangleFactor = 0.86f;
        forestDesc2.minImpulseFactor = 0.66f;
        forestDesc2.maxImpulseFactor = 0.99f;
        forestDesc2.ripFactor = 5.5f;
        forestDesc2.damageFactor = 1550.0f;        
        forestDesc2.surface        = forestSurface02;
        forestDesc2.bspLeafSize    = 128;
        forestDesc2.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc2.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc2.lodDistance[2] = getTreeLODDistance( 2, float(forestFactor) );
        forestDesc2.lodDistance[3] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc2 );

        // forest02::tree02
        ForestDesc forestDesc3;
        forestDesc3.cache     = "./usr/cache/ostankino_forest02_tree02";
        forestDesc3.assetName = "./res/speedtrees/tree02.ba";
        forestDesc3.density   = 0.0155f;
        forestDesc3.minScale  = 1.5f;
        forestDesc3.maxScale  = 2.0f;
        forestDesc3.collScale = 0.5f;
        forestDesc3.entangleFactor = 0.70f;
        forestDesc3.minImpulseFactor = 0.5f;
        forestDesc3.maxImpulseFactor = 0.7f;
        forestDesc3.ripFactor = 6.6f;
        forestDesc3.damageFactor = 1333.0f;        
        forestDesc3.surface        = forestSurface02;
        forestDesc3.bspLeafSize    = 128;
        forestDesc3.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc3.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc3.lodDistance[2] = getTreeLODDistance( 2, float(forestFactor) );
        forestDesc3.lodDistance[3] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc3 );
    }

    // cutscene mode
    CutsceneDesc cutsceneDesc;
    cutsceneDesc.source = parent->getScene()->findClump( "OstankinoCutscene" ); assert( cutsceneDesc.source );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1), FRAMETIME(100) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(102), FRAMETIME(199) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(201), FRAMETIME(381) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(384), FRAMETIME(440) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(480), FRAMETIME(549) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(554), FRAMETIME(647) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(653), FRAMETIME(746) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(753), FRAMETIME(850) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(852), FRAMETIME(979) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(981), FRAMETIME(1099) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1104), FRAMETIME(1200) ) );
    cutsceneDesc.speed = 0.75f;
    new Cutscene( parent->getScene(), &cutsceneDesc );
}

/**
 * casting for cave of swallows
 */

static void castingCallbackCaveAnimatedObject(Actor* parent, engine::IClump* source, engine::IClump* map)
{
    AnimatedObjectDesc animObjectDesc;
    animObjectDesc.source       = source;
    animObjectDesc.map          = map;
    animObjectDesc.animDistance = 3000.0f;
    animObjectDesc.animSpeed    = 0.25f;
    new AnimatedObject( parent, &animObjectDesc );
}

void casting::castingCallbackCaveOfSwallows(Actor* parent)
{
    // traffic density
    double trafficDensity;
    TiXmlElement* details = Gameplay::iGameplay->getConfigElement( "details" );
    details->Attribute( "traffic", &trafficDensity );

    // cave outdoor ambient
    AmbientSoundDesc desc;
    desc.processingMode = apmAltitude;
    desc.position       = Vector3f( 0,0,0 );
    desc.maxValue       = 400.0f;
    desc.maxValueGain   = 0.5f;
    desc.minValue       = -13000.0f;
    desc.minValueGain   = 0.01f;
    new AmbientSound( parent, "./res/sounds/cave/outdoor.ogg", &desc );

    // cave indoor ambient
    desc.processingMode = apmAltitude;
    desc.position       = Vector3f( 0,0,0 );
    desc.maxValue       = -5000.0f;
    desc.maxValueGain   = 0.01f;
    desc.minValue       = -20000.0f;
    desc.minValueGain   = 0.5f;
    new AmbientSound( parent, "./res/sounds/cave/indoor.ogg", &desc );

    // cave swallows
    desc.processingMode = apmDistance;
    desc.position       = Vector3f( -111.0f ,-2954.0f, -140.0f );
    desc.maxValue       = 10000.0f;
    desc.maxValueGain   = 0.01f;
    desc.minValue       = 2500.0f;
    desc.minValueGain   = 0.5f;
    new AmbientSound( parent, "./res/sounds/cave/swallows.ogg", &desc );

    // needful things
    callback::Locator locator;

    // animated objects
    castingCallbackCaveAnimatedObject( parent, parent->getScene()->findClump( "Bush01" ), locator.locate( parent->getScene()->getStage(), "Bushes01" ) );
    castingCallbackCaveAnimatedObject( parent, parent->getScene()->findClump( "Bush02" ), locator.locate( parent->getScene()->getStage(), "Bushes02" ) );
    castingCallbackCaveAnimatedObject( parent, parent->getScene()->findClump( "Tree01" ), locator.locate( parent->getScene()->getStage(), "Trees01" ) );
    castingCallbackCaveAnimatedObject( parent, parent->getScene()->findClump( "Tree02" ), locator.locate( parent->getScene()->getStage(), "Trees02" ) );
    castingCallbackCaveAnimatedObject( parent, parent->getScene()->findClump( "Tree03" ), locator.locate( parent->getScene()->getStage(), "Trees03" ) );
    castingCallbackCaveAnimatedObject( parent, parent->getScene()->findClump( "Tree04" ), locator.locate( parent->getScene()->getStage(), "Trees04" ) );
    castingCallbackCaveAnimatedObject( parent, parent->getScene()->findClump( "Tree05" ), locator.locate( parent->getScene()->getStage(), "Trees05" ) );
    castingCallbackCaveAnimatedObject( parent, parent->getScene()->findClump( "Tree06" ), locator.locate( parent->getScene()->getStage(), "Trees06" ) );

    // traffic objects
    unsigned int birdDensity = unsigned int( 20 * trafficDensity );
    castingCallbackTraffic( parent, parent->getScene()->findClump( "Birds" ), birdDensity );

    // cutscene mode
    CutsceneDesc cutsceneDesc;
    cutsceneDesc.source = parent->getScene()->findClump( "CaveCutscene" ); assert( cutsceneDesc.source );    
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1), FRAMETIME(140) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(143), FRAMETIME(408) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(416), FRAMETIME(527) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(535), FRAMETIME(696) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(702), FRAMETIME(896) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(901), FRAMETIME(1200) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1207), FRAMETIME(1500) ) );
    cutsceneDesc.speed = 0.5f;
    new Cutscene( parent->getScene(), &cutsceneDesc );
}

/**
 * casting for trollveggen
 */

void casting::castingCallbackTrollveggen(Actor* parent)
{
    // cast wind by ordinary method
    castingCallbackOrdinaryWind( parent );

    // cast rain by ordinary method
    castingCallbackOrdinaryRain( parent );

    // traffic density
    double trafficDensity;
    double forestFactor;
    TiXmlElement* details = Gameplay::iGameplay->getConfigElement( "details" );
    details->Attribute( "traffic", &trafficDensity );
    details->Attribute( "forest", &forestFactor );
    // 100% road traffic
    unsigned int defaultTrainDensity = 1;
    unsigned int defaultCarDensity   = 16;
    // configured road traffic
    unsigned int trainDensity = unsigned int( double( defaultTrainDensity ) * trafficDensity );
    unsigned int carDensity   = unsigned int( double( defaultCarDensity ) * trafficDensity );
    if( trainDensity )
    {
        castingCallbackTraffic( parent, parent->getScene()->findClump( "TrafficTrain01" ), trainDensity );
    }    
    if( carDensity )
    {
        castingCallbackTraffic( parent, parent->getScene()->findClump( "TrafficCars01" ), carDensity );
    }
      
    if( forestFactor > 0.0f )
    {
        // obtain forest surface 1
        engine::IClump* forestClump = parent->getScene()->getExtras( "ForestSurface01" ); assert( forestClump );
        callback::AtomicL forestSurfaces;    
        forestClump->forAllAtomics( callback::enumerateAtomics, &forestSurfaces ); assert( forestSurfaces.size() );
        engine::IAtomic* forestSurface01 = (*forestSurfaces.begin());

        // forest01::tree01
        ForestDesc forestDesc1;
        forestDesc1.cache     = "./usr/cache/trollveggen_forest01_tree11";
        forestDesc1.assetName = "./res/speedtrees/tree11.ba";
        forestDesc1.density   = 0.0125f;
        forestDesc1.minScale  = 1.05f;
        forestDesc1.maxScale  = 1.75f;
        forestDesc1.collScale = 0.5f;
        forestDesc1.entangleFactor = 0.55f;
        forestDesc1.minImpulseFactor = 0.40f;
        forestDesc1.maxImpulseFactor = 0.80f;
        forestDesc1.ripFactor = 8.0f;
        forestDesc1.damageFactor = 1640.0f;   
        forestDesc1.layers         = forestClump;
        forestDesc1.surface        = forestSurface01;
        forestDesc1.bspLeafSize    = 128;
        forestDesc1.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc1.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc1.lodDistance[2] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc1 );
    }

    // cutscene mode
    CutsceneDesc cutsceneDesc;
    cutsceneDesc.source = parent->getScene()->findClump( "TrollveggenCutscene" ); assert( cutsceneDesc.source );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(95), FRAMETIME(167) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(178), FRAMETIME(284) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(289), FRAMETIME(385) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(399), FRAMETIME(536) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(550), FRAMETIME(676) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(740), FRAMETIME(800) ) );
    cutsceneDesc.speed = 0.25f;
    new Cutscene( parent->getScene(), &cutsceneDesc );
}

/**
 * casting for royal gorge bridge
 */

void casting::castingCallbackRoyalGorgeBridge(Actor* parent)
{
    double trafficDensity, crowdDensity;
    TiXmlElement* details = Gameplay::iGameplay->getConfigElement( "details" );
    details->Attribute( "traffic", &trafficDensity );   
    details->Attribute( "crowd", &crowdDensity );

    // cast wind by ordinary method
    castingCallbackOrdinaryWind( parent );

    // cast rain by ordinary method
    castingCallbackOrdinaryRain( parent );

    // crowd ambient
    AmbientSoundDesc desc;
    desc.processingMode = apmDistance;
    desc.position       = Vector3f( -76163, 30020, 126908 );
    desc.maxValue       = 15000.0f;
    desc.maxValueGain   = 0.01f;
    desc.minValue       = 0;
    desc.minValueGain   = 0.25f;
    new AmbientSound( parent, "./res/sounds/ostankino/crowd.ogg", &desc );

    if( crowdDensity )
    {
        // crowd #1
        CrowdDesc crowdDesc;
        crowdDesc.extras = parent->getScene()->getExtras( "CrowdEnclosure01" );
        crowdDesc.numActors = unsigned int( 32 * crowdDensity );
        crowdDesc.numWalkingActors = 6;
        new Crowd( parent, &crowdDesc );

        // crowd #2
        crowdDesc.extras = parent->getScene()->getExtras( "CrowdEnclosure02" );
        crowdDesc.numActors = unsigned int( 8 * crowdDensity );
        crowdDesc.numWalkingActors = 2;
        new Crowd( parent, &crowdDesc );

        // crowd #3
        crowdDesc.extras = parent->getScene()->getExtras( "CrowdEnclosure03" );
        crowdDesc.numActors = unsigned int( 8 * crowdDensity );
        crowdDesc.numWalkingActors = 2;
        new Crowd( parent, &crowdDesc );

        // crowd #4
        crowdDesc.extras = parent->getScene()->getExtras( "CrowdEnclosure04" );
        crowdDesc.numActors = unsigned int( 8 * crowdDensity );
        crowdDesc.numWalkingActors = 2;
        new Crowd( parent, &crowdDesc );
    }

    // cutscene mode
    CutsceneDesc cutsceneDesc;
    cutsceneDesc.source = parent->getScene()->findClump( "RoyalGorgeBridgeCutscene" ); assert( cutsceneDesc.source );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1), FRAMETIME(99) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(105), FRAMETIME(201) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(205), FRAMETIME(329) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(337), FRAMETIME(477) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(581), FRAMETIME(602) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(605), FRAMETIME(677) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(685), FRAMETIME(796) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(804), FRAMETIME(905) ) );
    cutsceneDesc.speed = 0.75f;
    new Cutscene( parent->getScene(), &cutsceneDesc );
}

/**
 * casting for dropzone
 */

void casting::castingCallbackDropzone(Actor* parent)
{
    double trafficDensity, crowdDensity, forestFactor;
    TiXmlElement* details = Gameplay::iGameplay->getConfigElement( "details" );
    details->Attribute( "traffic", &trafficDensity );   
    details->Attribute( "crowd", &crowdDensity );
    details->Attribute( "forest", &forestFactor );

    // cast wind by ordinary method
    castingCallbackOrdinaryWind( parent );

    // cast rain by ordinary method
    castingCallbackOrdinaryRain( parent );

    if( forestFactor > 0 )
    {       
        // obtain forest surfaces
        engine::IClump* forestClump = parent->getScene()->getExtras( "ForestSurface01" ); assert( forestClump );
        callback::AtomicL forestSurfaces;    
        forestClump->forAllAtomics( callback::enumerateAtomics, &forestSurfaces ); assert( forestSurfaces.size() );    
        engine::IAtomic* forestSurface = (*forestSurfaces.begin());

        // forest01::tree01
        ForestDesc forestDesc;
        forestDesc.cache     = "./usr/cache/dropzone_forest01_tree01";    
        forestDesc.assetName = "./res/speedtrees/tree01.ba";
        forestDesc.density   = 0.0045f;
        forestDesc.minScale  = 1.0f;
        forestDesc.maxScale  = 1.45f;
        forestDesc.collScale = 0.5f;
        forestDesc.entangleFactor = 0.5f;
        forestDesc.minImpulseFactor = 0.25f;
        forestDesc.maxImpulseFactor = 0.5f;
        forestDesc.ripFactor = 5.0f;
        forestDesc.damageFactor = 1500.0f;
        forestDesc.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc.lodDistance[2] = getTreeLODDistance( 2, float(forestFactor) );
        forestDesc.lodDistance[3] = getTreeLODDistance( 3, float(forestFactor) );
        forestDesc.surface        = forestSurface;
        forestDesc.bspLeafSize    = 128;
        new Forest( parent, &forestDesc );    

        // forest01::tree02
        forestDesc.cache     = "./usr/cache/dropzone_forest01_tree02";
        forestDesc.assetName = "./res/speedtrees/tree02.ba";
        forestDesc.density   = 0.0085f;
        forestDesc.minScale  = 1.0f;
        forestDesc.maxScale  = 1.75f;
        forestDesc.collScale = 0.5f;
        forestDesc.entangleFactor = 0.5f;
        forestDesc.minImpulseFactor = 0.25f;
        forestDesc.maxImpulseFactor = 0.5f;
        forestDesc.ripFactor = 5.0f;
        forestDesc.damageFactor = 1500.0f;
        forestDesc.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc.lodDistance[2] = getTreeLODDistance( 3, float(forestFactor) );
        forestDesc.surface        = forestSurface;
        forestDesc.bspLeafSize    = 128;
        new Forest( parent, &forestDesc );
    }
    
    if( crowdDensity )
    {
        // crowd #1
        CrowdDesc crowdDesc;
        crowdDesc.extras = parent->getScene()->getExtras( "CrowdEnclosure01" );
        crowdDesc.numActors = unsigned int( 32 * crowdDensity );
        crowdDesc.numWalkingActors = 6;
        new Crowd( parent, &crowdDesc );

        // crowd #2
        crowdDesc.extras = parent->getScene()->getExtras( "CrowdEnclosure02" );
        crowdDesc.numActors = unsigned int( 32 * crowdDensity );
        crowdDesc.numWalkingActors = 6;
        new Crowd( parent, &crowdDesc );
    }

    // traffic objects
    if( trafficDensity )
    {
        unsigned int carsDensity = unsigned int( 16 * trafficDensity );    
        castingCallbackTraffic( parent, parent->getScene()->findClump( "RoadTraffic" ), carsDensity );
    }

    // cutscene mode
    CutsceneDesc cutsceneDesc;
    cutsceneDesc.source = parent->getScene()->findClump( "DropzoneCutscene" ); assert( cutsceneDesc.source );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1), FRAMETIME(98) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(104), FRAMETIME(264) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(272), FRAMETIME(405) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(412), FRAMETIME(538) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(542), FRAMETIME(636) ) );
    cutsceneDesc.speed = 0.5f;
    new Cutscene( parent->getScene(), &cutsceneDesc );
}

/**
 * casting for KVLY-TV mast
 */

void casting::castingCallbackKVLY(Actor* parent)
{
    double forestFactor;
    double trafficDensity, crowdDensity;
    TiXmlElement* details = Gameplay::iGameplay->getConfigElement( "details" );
    details->Attribute( "traffic", &trafficDensity );   
    details->Attribute( "crowd", &crowdDensity );
    details->Attribute( "forest", &forestFactor );

    // cast wind by ordinary method
    castingCallbackOrdinaryWind( parent );

    // cast rain by ordinary method
    castingCallbackOrdinaryRain( parent );
      
    if( forestFactor > 0.0f )
    {
        // obtain forest surface 1
        engine::IClump* forestClump = parent->getScene()->getExtras( "ForestSurface01" ); assert( forestClump );
        callback::AtomicL forestSurfaces;    
        forestClump->forAllAtomics( callback::enumerateAtomics, &forestSurfaces ); assert( forestSurfaces.size() );
        engine::IAtomic* forestSurface01 = (*forestSurfaces.begin());

        // forest01::tree01
        ForestDesc forestDesc1;
        forestDesc1.cache     = "./usr/cache/kvly_forest01_tree01";
        forestDesc1.assetName = "./res/speedtrees/tree01.ba";
        forestDesc1.density   = 0.005f;
        forestDesc1.minScale  = 1.0f;
        forestDesc1.maxScale  = 1.66f;
        forestDesc1.collScale = 0.5f;
        forestDesc1.entangleFactor = 0.75f;
        forestDesc1.minImpulseFactor = 0.30f;
        forestDesc1.maxImpulseFactor = 0.60f;
        forestDesc1.ripFactor = 5.0f;
        forestDesc1.damageFactor = 1500.0f;        
        forestDesc1.surface        = forestSurface01;
        forestDesc1.bspLeafSize    = 128;
        forestDesc1.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc1.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc1.lodDistance[2] = getTreeLODDistance( 2, float(forestFactor) );
        forestDesc1.lodDistance[3] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc1 );

        // forest01::tree02   
        ForestDesc forestDesc2;
        forestDesc2.cache     = "./usr/cache/kvly_forest01_tree02";
        forestDesc2.assetName = "./res/speedtrees/tree02.ba";
        forestDesc2.density   = 0.015f;
        forestDesc2.minScale  = 1.0f;
        forestDesc2.maxScale  = 1.45f;
        forestDesc2.collScale = 0.5f;
        forestDesc2.entangleFactor = 0.55f;
        forestDesc2.minImpulseFactor = 0.33f;
        forestDesc2.maxImpulseFactor = 0.66f;
        forestDesc2.ripFactor = 5.5f;
        forestDesc2.damageFactor = 1750.0f;        
        forestDesc2.surface        = forestSurface01;
        forestDesc2.bspLeafSize    = 128;
        forestDesc2.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc2.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc2.lodDistance[2] = getTreeLODDistance( 2, float(forestFactor) );
        forestDesc2.lodDistance[3] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc2 );
    }

    // cutscene mode
    CutsceneDesc cutsceneDesc;
    cutsceneDesc.source = parent->getScene()->findClump( "KVLYCutscene" ); assert( cutsceneDesc.source );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1), FRAMETIME(98) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(104), FRAMETIME(264) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(272), FRAMETIME(405) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(412), FRAMETIME(538) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(542), FRAMETIME(636) ) );
    cutsceneDesc.speed = 0.5f;
    new Cutscene( parent->getScene(), &cutsceneDesc );
}

/**
 * casting for Kjerag
 */

void casting::castingCallbackKjerag(Actor* parent)
{
    double forestFactor;
    double trafficDensity, crowdDensity;
    TiXmlElement* details = Gameplay::iGameplay->getConfigElement( "details" );
    details->Attribute( "traffic", &trafficDensity );   
    details->Attribute( "crowd", &crowdDensity );
    details->Attribute( "forest", &forestFactor );

    // cast wind by ordinary method
    castingCallbackOrdinaryWind( parent );

    // cast rain by ordinary method
    castingCallbackOrdinaryRain( parent );   

    // cast wind by ordinary method
    castingCallbackOrdinaryWind( parent );

    // cast rain by ordinary method
    castingCallbackOrdinaryRain( parent );
      
    if( forestFactor > 0.0f )
    {
        // obtain forest surface 1
        engine::IClump* forestClump = parent->getScene()->getExtras( "ForestSurface01" ); assert( forestClump );
        callback::AtomicL forestSurfaces;    
        forestClump->forAllAtomics( callback::enumerateAtomics, &forestSurfaces ); assert( forestSurfaces.size() );
        engine::IAtomic* forestSurface01 = (*forestSurfaces.begin());

        // forest01::tree01
        ForestDesc forestDesc1;
        forestDesc1.cache     = "./usr/cache/kjerag_forest01_tree11";
        forestDesc1.assetName = "./res/speedtrees/tree11.ba";
        forestDesc1.density   = 0.0075f;
        forestDesc1.minScale  = 1.25f;
        forestDesc1.maxScale  = 1.85f;
        forestDesc1.collScale = 0.5f;
        forestDesc1.entangleFactor = 0.66f;
        forestDesc1.minImpulseFactor = 0.50f;
        forestDesc1.maxImpulseFactor = 0.75f;
        forestDesc1.ripFactor = 7.5f;
        forestDesc1.damageFactor = 1750.0f;        
        forestDesc1.surface        = forestSurface01;
        forestDesc1.bspLeafSize    = 128;
        forestDesc1.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc1.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc1.lodDistance[2] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc1 );
    }

    // cutscene mode
    CutsceneDesc cutsceneDesc;
    cutsceneDesc.source = parent->getScene()->findClump( "KjeragCutscene" ); assert( cutsceneDesc.source );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1), FRAMETIME(100) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(110), FRAMETIME(250) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(265), FRAMETIME(430) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(453), FRAMETIME(540) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(558), FRAMETIME(660) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(691), FRAMETIME(759) ) );
    cutsceneDesc.speed = 0.5f;
    new Cutscene( parent->getScene(), &cutsceneDesc );
}

/**
 * casting for Angel Falls
 */

void casting::castingCallbackAngelFalls(Actor* parent)
{
    // cast wind by ordinary method
    castingCallbackOrdinaryWind( parent );

    // cast rain by ordinary method
    castingCallbackOrdinaryRain( parent );

    // forest config
    double forestFactor;
    TiXmlElement* details = Gameplay::iGameplay->getConfigElement( "details" );
    details->Attribute( "forest", &forestFactor );
    
    if( forestFactor > 0.0f )
    {
        // obtain forest surface 1
        engine::IClump* forestClump = parent->getScene()->getExtras( "ForestSurface01" ); assert( forestClump );
        callback::AtomicL forestSurfaces;    
        forestClump->forAllAtomics( callback::enumerateAtomics, &forestSurfaces ); assert( forestSurfaces.size() );
        engine::IAtomic* forestSurface01 = (*forestSurfaces.begin());

        // forest01::tree01
        ForestDesc forestDesc1;
        forestDesc1.cache     = "./usr/cache/angelfalls_forest01_tree01";
        forestDesc1.assetName = "./res/speedtrees/tree01.ba";
        forestDesc1.density   = 0.0025f;
        forestDesc1.minScale  = 1.0f;
        forestDesc1.maxScale  = 1.66f;
        forestDesc1.collScale = 0.5f;
        forestDesc1.entangleFactor = 0.75f;
        forestDesc1.minImpulseFactor = 0.30f;
        forestDesc1.maxImpulseFactor = 0.60f;
        forestDesc1.ripFactor = 5.0f;
        forestDesc1.damageFactor = 1500.0f;        
        forestDesc1.surface        = forestSurface01;
        forestDesc1.bspLeafSize    = 128;
        forestDesc1.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc1.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc1.lodDistance[2] = getTreeLODDistance( 2, float(forestFactor) );
        forestDesc1.lodDistance[3] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc1 );

        // forest01::tree02   
        ForestDesc forestDesc2;
        forestDesc2.cache     = "./usr/cache/angelfalls_forest01_tree02";
        forestDesc2.assetName = "./res/speedtrees/tree02.ba";
        forestDesc2.density   = 0.0075f;
        forestDesc2.minScale  = 1.0f;
        forestDesc2.maxScale  = 1.45f;
        forestDesc2.collScale = 0.5f;
        forestDesc2.entangleFactor = 0.55f;
        forestDesc2.minImpulseFactor = 0.33f;
        forestDesc2.maxImpulseFactor = 0.66f;
        forestDesc2.ripFactor = 5.5f;
        forestDesc2.damageFactor = 1750.0f;        
        forestDesc2.surface        = forestSurface01;
        forestDesc2.bspLeafSize    = 128;
        forestDesc2.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc2.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc2.lodDistance[2] = getTreeLODDistance( 2, float(forestFactor) );
        forestDesc2.lodDistance[3] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc2 );
    }

    // cutscene mode
    CutsceneDesc cutsceneDesc;
    cutsceneDesc.source = parent->getScene()->findClump( "AngelFallsCutscene" ); assert( cutsceneDesc.source );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1), FRAMETIME(180) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(182), FRAMETIME(300) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(302), FRAMETIME(559) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(561), FRAMETIME(659) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(661), FRAMETIME(800) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(802), FRAMETIME(1000) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1002), FRAMETIME(1100) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1102), FRAMETIME(1200) ) );
    cutsceneDesc.speed = 0.5f;
    new Cutscene( parent->getScene(), &cutsceneDesc );
}

/**
 * casting for El Capitan
 */

void casting::castingCallbackElCapitan(Actor* parent)
{
    // cast wind by ordinary method
    castingCallbackOrdinaryWind( parent );

    // cast rain by ordinary method
    castingCallbackOrdinaryRain( parent );

    // forest config
    double forestFactor;
    TiXmlElement* details = Gameplay::iGameplay->getConfigElement( "details" );
    details->Attribute( "forest", &forestFactor );
    
    if( forestFactor > 0.0f )
    {
        // obtain forest surface 1
        engine::IClump* forestClump = parent->getScene()->getExtras( "ForestSurface01" ); assert( forestClump );
        callback::AtomicL forestSurfaces;    
        forestClump->forAllAtomics( callback::enumerateAtomics, &forestSurfaces ); assert( forestSurfaces.size() );
        engine::IAtomic* forestSurface01 = (*forestSurfaces.begin());

        // forest01::tree01
        ForestDesc forestDesc1;
        forestDesc1.cache     = "./usr/cache/elcapitan_forest01_tree01";
        forestDesc1.assetName = "./res/speedtrees/tree01.ba";
        forestDesc1.density   = 0.0025f;
        forestDesc1.minScale  = 1.0f;
        forestDesc1.maxScale  = 1.66f;
        forestDesc1.collScale = 0.5f;
        forestDesc1.entangleFactor = 0.75f;
        forestDesc1.minImpulseFactor = 0.30f;
        forestDesc1.maxImpulseFactor = 0.60f;
        forestDesc1.ripFactor = 5.0f;
        forestDesc1.damageFactor = 1500.0f;        
        forestDesc1.surface        = forestSurface01;
        forestDesc1.bspLeafSize    = 128;
        forestDesc1.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc1.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc1.lodDistance[2] = getTreeLODDistance( 2, float(forestFactor) );
        forestDesc1.lodDistance[3] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc1 );

        // forest01::tree02   
        ForestDesc forestDesc2;
        forestDesc2.cache     = "./usr/cache/elcapitan_forest01_tree02";
        forestDesc2.assetName = "./res/speedtrees/tree02.ba";
        forestDesc2.density   = 0.0075f;
        forestDesc2.minScale  = 1.0f;
        forestDesc2.maxScale  = 1.45f;
        forestDesc2.collScale = 0.5f;
        forestDesc2.entangleFactor = 0.55f;
        forestDesc2.minImpulseFactor = 0.33f;
        forestDesc2.maxImpulseFactor = 0.66f;
        forestDesc2.ripFactor = 5.5f;
        forestDesc2.damageFactor = 1750.0f;        
        forestDesc2.surface        = forestSurface01;
        forestDesc2.bspLeafSize    = 128;
        forestDesc2.lodDistance[0] = getTreeLODDistance( 0, float(forestFactor) );
        forestDesc2.lodDistance[1] = getTreeLODDistance( 1, float(forestFactor) );
        forestDesc2.lodDistance[2] = getTreeLODDistance( 2, float(forestFactor) );
        forestDesc2.lodDistance[3] = getTreeLODDistance( 3, float(forestFactor) );
        new Forest( parent, &forestDesc2 );
    }

    // cutscene mode
    CutsceneDesc cutsceneDesc;
    cutsceneDesc.source = parent->getScene()->findClump( "AngelFallsCutscene" ); assert( cutsceneDesc.source );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1), FRAMETIME(180) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(182), FRAMETIME(300) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(302), FRAMETIME(559) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(561), FRAMETIME(659) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(661), FRAMETIME(800) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(802), FRAMETIME(1000) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1002), FRAMETIME(1100) ) );
    cutsceneDesc.tracks.push_back( CutsceneTrack( FRAMETIME(1102), FRAMETIME(1200) ) );
    cutsceneDesc.speed = 0.5f;
    new Cutscene( parent->getScene(), &cutsceneDesc );
}