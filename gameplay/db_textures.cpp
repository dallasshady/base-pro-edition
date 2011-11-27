
#include "headers.h"
#include "database.h"

using namespace database;

static GearTexture textures[] =
{
    /* 000 */ { "cloth01", "./res/x/textures/cloth01.dds" },
    /* 001 */ { "cloth02", "./res/x/textures/cloth02.dds" },
    /* 002 */ { "cloth03", "./res/x/textures/cloth03.dds" },
    /* 003 */ { "cloth04", "./res/x/textures/cloth04.dds" },
    /* 004 */ { "cloth05", "./res/x/textures/cloth05.dds" },
    /* 005 */ { "cloth06", "./res/x/textures/cloth06.dds" },
    /* 006 */ { "cloth07", "./res/x/textures/cloth07.dds" },
    /* 007 */ { "cloth08", "./res/x/textures/cloth08.dds" },
    /* 008 */ { "cloth09", "./res/x/textures/cloth09.dds" },
    /* 009 */ { "cloth10", "./res/x/textures/cloth10.dds" },
    /* 010 */ { "cloth11", "./res/x/textures/cloth11.dds" },
    /* 011 */ { "cloth12", "./res/x/textures/cloth12.dds" },
    /* 012 */ { "cloth13", "./res/x/textures/cloth13.dds" },
    /* 013 */ { "cloth14", "./res/x/textures/cloth14.dds" },
    /* 014 */ { "cloth15", "./res/x/textures/cloth15.dds" },
    /* 015 */ { "cloth16", "./res/x/textures/cloth16.dds" },
    /* 016 */ { "cloth17", "./res/x/textures/cloth17.dds" },
    /* 017 */ { "falco01", "./res/x/textures/falco01.dds" },
    /* 018 */ { "helmet0201", "./res/x/textures/helmet0201.dds" },
    /* 019 */ { "helmet0202", "./res/x/textures/helmet0202.dds" },
    /* 020 */ { "helmet0203", "./res/x/textures/helmet0203.dds" },
    /* 021 */ { "helmet0207", "./res/x/textures/helmet0207.dds" },
    /* 022 */ { "helmet0208", "./res/x/textures/helmet0208.dds" },
    /* 023 */ { "helmet0209", "./res/x/textures/helmet0209.dds" },
    /* 024 */ { "helmet0210", "./res/x/textures/helmet0210.dds" },
    /* 025 */ { "helmet0211", "./res/x/textures/helmet0211.dds" },
    /* 026 */ { "helmet0214", "./res/x/textures/helmet0214.dds" },
    /* 027 */ { "helmet0303", "./res/x/textures/helmet0303.dds" },
    /* 028 */ { "helmet0304", "./res/x/textures/helmet0304.dds" },
    /* 029 */ { "helmet0305", "./res/x/textures/helmet0305.dds" },
    /* 030 */ { "helmet0306", "./res/x/textures/helmet0306.dds" },
    /* 031 */ { "helmet0307", "./res/x/textures/helmet0307.dds" },
    /* 032 */ { "helmet0309", "./res/x/textures/helmet0309.dds" },
    /* 033 */ { "helmet0311", "./res/x/textures/helmet0311.dds" },
    /* 034 */ { "helmet0312", "./res/x/textures/helmet0312.dds" },
    /* 035 */ { "helmet0313", "./res/x/textures/helmet0313.dds" },
    /* 036 */ { "falco02", "./res/x/textures/falco02.dds" },
    /* 037 */ { "falco03", "./res/x/textures/falco03.dds" },
    /* 038 */ { "falco04", "./res/x/textures/falco04.dds" },
    /* 039 */ { "falco05", "./res/x/textures/falco05.dds" },
    /* 040 */ { "falco06", "./res/x/textures/falco06.dds" },
    /* 041 */ { "falco07", "./res/x/textures/falco07.dds" },
    /* 042 */ { "falco08", "./res/x/textures/falco08.dds" },
    /* 043 */ { "falco09", "./res/x/textures/falco09.dds" },
    /* 044 */ { "falco10", "./res/x/textures/falco10.dds" },
    /* 045 */ { "harpy01", "./res/x/textures/harpy01.dds" },
    /* 046 */ { "harpy02", "./res/x/textures/harpy02.dds" },
    /* 047 */ { "harpy03", "./res/x/textures/harpy03.dds" },
    /* 048 */ { "harpy04", "./res/x/textures/harpy04.dds" },
    /* 049 */ { "P_01_01", "./res/x/textures/P_01_01.dds" },
    /* 050 */ { "P_01_02", "./res/x/textures/P_01_02.dds" },
    /* 051 */ { "P_01_03", "./res/x/textures/P_01_03.dds" },
    /* 052 */ { "P_01_05", "./res/x/textures/P_01_05.dds" },
    /* 053 */ { "P_01_06", "./res/x/textures/P_01_06.dds" },
    /* 054 */ { "P_01_07", "./res/x/textures/P_01_07.dds" },
    /* 055 */ { "P_onecolor_01_08", "./res/x/textures/P_onecolor_01_08.dds" },
    /* 056 */ { "P_onecolor_01_10", "./res/x/textures/P_onecolor_01_10.dds" },
    /* 057 */ { "P_onecolor_01_11", "./res/x/textures/P_onecolor_01_11.dds" },
    /* 058 */ { "P_onecolor_01_13", "./res/x/textures/P_onecolor_01_13.dds" },
    /* 059 */ { "P_onecolor_01_16", "./res/x/textures/P_onecolor_01_16.dds" },
    /* 060 */ { "P_onecolor_01_18", "./res/x/textures/P_onecolor_01_18.dds" },
    /* 061 */ { "P_onecolor_01_20", "./res/x/textures/P_onecolor_01_20.dds" },
    /* 062 */ { "P_onecolor_01_21", "./res/x/textures/P_onecolor_01_21.dds" },
    /* 063 */ { "P_onecolor_01_22", "./res/x/textures/P_onecolor_01_22.dds" },
    /* 064 */ { "cloth18", "./res/x/textures/cloth18.dds" },
    /* 065 */ { "Morpheus_P01_02", "./res/x/textures/Morpheus_P01_02.dds" },
    /* 066 */ { "Obsession_03", "./res/x/textures/Obsession_03.dds" },
    /* 067 */ { "GForce_sky_01", "./res/x/textures/GForce_sky_01.dds" },
    /* 068 */ { "Infinity_sky_01", "./res/x/textures/Infinity_sky_01.dds" },
    /* 069 */ { "fbredbull", "./res/x/textures/fbredbull.dds" },
    /* 070 */ { "FelixCloth01_10", "./res/x/textures/FelixCloth01_10.dds" },
    { NULL, NULL }
};

unsigned int GearTexture::getNumRecords(void)
{
    unsigned int result = 0;
    unsigned int i = 0;
    while( textures[i].resourceName != 0 ) i++, result++;
    return result;
}

GearTexture* GearTexture::getRecord(unsigned int id)
{
    assert( id >= 0 && id < getNumRecords() );
    return textures + id;
}