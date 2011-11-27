
#include "headers.h"
#include "database.h"

using namespace database;

static Face faces[] = 
{
    /* 00 */ { 348, 0, "face0101", "./res/x/textures/character0101.dds", "character0101" },
    /* 01 */ { 349, 0, "face0102", "./res/x/textures/character0102.dds", "character0102" },
    /* 02 */ { 350, 0, "face0103", "./res/x/textures/character0103.dds", "character0103" },
    /* 03 */ { 351, 0, "face0104", "./res/x/textures/character0104.dds", "character0104" },
    /* 04 */ { 352, 0, "face0105", "./res/x/textures/character0105.dds", "character0105" },
    /* 05 */ { 353, 0, "face0106", "./res/x/textures/character0106.dds", "character0106" },
    /* 06 */ { 354, 0, "face0107", "./res/x/textures/character0107.dds", "character0107" },
    /* 07 */ { 355, 1, "face0201", "./res/x/textures/character0201.dds", "character0201" },
    /* 08 */ { 356, 1, "face0202", "./res/x/textures/character0202.dds", "character0202" },
    /* 09 */ { 357, 2, "face0301", "./res/x/textures/character0301.dds", "character0301" },
    /* 10 */ { 358, 2, "face0302", "./res/x/textures/character0302.dds", "character0302" },
    /* 11 */ { 359, 2, "face0303", "./res/x/textures/character0303.dds", "character0303" },
    /* 12 */ { 360, 3, "face0401", "./res/x/textures/character0401.dds", "character0401" },
    /* 13 */ { 361, 3, "face0406", "./res/x/textures/character0406.dds", "character0406" },
    { 0, 0, NULL, NULL, NULL }
};

unsigned int Face::getNumRecords(void)
{
    unsigned int result = 0;
    unsigned int i = 0;
    while( faces[i].descriptionId != 0 ) i++, result++;
    return result;
}

Face* Face::getRecord(unsigned int id)
{
    assert( id < getNumRecords() );
    return faces+id;
}