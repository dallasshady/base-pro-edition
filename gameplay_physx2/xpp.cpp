
#include "headers.h"
#include "xpp.h"
#include "callback.h"

static struct XppEntry
{
public:
    const char*    clumpName;
    xpp::xppMethod method;
}
xppEntries[] =
{
    { "Bush01", xpp::preprocessBush },
    { "Bush02", xpp::preprocessBush },
    { "Tree01", xpp::preprocessTree },
    { "Tree02", xpp::preprocessTree },
    { "Tree03", xpp::preprocessTree },
    { "Tree04", xpp::preprocessTree },
    { "Tree05", xpp::preprocessTree },
    { "Tree06", xpp::preprocessTree },
    { "CrowdMale01", xpp::preprocessCrowdMale01 },
    { "OstankinoCutscene", xpp::preprocessOstankinoCutscene },
    { "BaseJumper01", xpp::preprocessBaseJumper01 },
    { "Pilot02", xpp::preprocessPilotchute },
    { "Canopy02", xpp::preprocessCanopy02 },
    { "Slider", xpp::preprocessSlider },
    { "TrafficBus01", xpp::preprocessTraffic },
    { "TrafficCars01", xpp::preprocessTraffic },
    { "RoadTraffic", xpp::preprocessTraffic },
    { "Airplane01", xpp::preprocessAirplane },
    { "Airplane02", xpp::preprocessAirplane },
    { "Helicopter01", xpp::preprocessHelicopter },
    { NULL, NULL }
};

void xpp::preprocessXAsset(engine::IAsset* asset)
{
    // enumerate clumps
    callback::ClumpL clumps;
    asset->forAllClumps( callback::enumerateClumps, &clumps );

    // for each clump
    for( callback::ClumpI clumpI = clumps.begin(); clumpI != clumps.end(); clumpI++ )
    {
        // search for corresponding xpp-entry
        XppEntry* entry = xppEntries;
        while( entry->clumpName )
        {
            if( strcmp( (*clumpI)->getName(), entry->clumpName ) == 0 )
            {
                entry->method( (*clumpI) );
                break;
            }
            // next entry
            entry++;
        }
    }
}