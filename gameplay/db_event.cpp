
#include "headers.h"
#include "database.h"

using namespace database;

static EventInfo events[] = 
{
    /* 00 */ { 76, 77, "ev_workweek", "./res/gui/textures/ev_workweek.dds" },
    /* 01 */ { 93, 94, "ev_system", "./res/gui/textures/ev_system.dds" },
    /* 02 */ { 95, 96, "ev_travel", "./res/gui/textures/ev_travel.dds" },
    /* 03 */ { 110, 111, "ev_night", "./res/gui/textures/ev_night.dds" },
    /* 04 */ { 251, 252, "ev_boogie", "./res/gui/textures/ev_boogie.dds" },
    /* 05 */ { 520, 521, "ev_festival", "./res/gui/textures/ev_festival.dds" },
    /* 06 */ { 567, 568, "ev_climbing", "./res/gui/textures/ev_climbing.dds" },
    /* 07 */ { 637, 638, "ev_smokeball", "./res/gui/textures/ev_smokeball.dds" },
    /* 08 */ { 824, 825, "ev_community", "./res/gui/textures/ev_community.dds" },
    { 0, 0, NULL, NULL }
};

unsigned int EventInfo::getNumRecords(void)
{
    unsigned int result = 0;
    unsigned int i = 0;
    while( events[i].shortId != 0 ) i++, result++;
    return result;
}

EventInfo* EventInfo::getRecord(unsigned int id)
{
    assert( id >= 0 && id < getNumRecords() );
    return events + id;
}