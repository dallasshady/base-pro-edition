
#include "headers.h"
#include "database.h"

using namespace database;

// default canopy pack
#define REGULAR_TRADE true

// atari edition canopy pack
/*#ifdef GAMEPLAY_EDITION_ATARI
    #define UTURN_TRADE true
#else
    #define UTURN_TRADE false
#endif
*/
#define UTURN_TRADE true

#define MODELID_PSYCHONAUT_210 182
#define MODELID_PSYCHONAUT_221 183
#define MODELID_PSYCHONAUT_232 74
#define MODELID_PSYCHONAUT_243 78
#define MODELID_PSYCHONAUT_254 79
#define MODELID_PSYCHONAUT_265 80
#define MODELID_PSYCHONAUT_276 81
#define MODELID_PSYCHONAUT_287 82
#define MODELID_PSYCHONAUT_298 83
#define MODELID_PSYCHONAUT_309 84

#define MODELID_JAHNCLE_150 855
#define MODELID_JAHNCLE_190 86
#define MODELID_JAHNCLE_210 85
#define MODELID_JAHNCLE_240 87
#define MODELID_JAHNCLE_270 218
#define MODELID_JAHNCLE_290 228

#define MODELID_HAIBANE_71 854
#define MODELID_HAIBANE_100 853
#define MODELID_HAIBANE_130 377
#define MODELID_HAIBANE_160 379
#define MODELID_HAIBANE_190 380
#define MODELID_HAIBANE_220 381
#define MODELID_HAIBANE_250 382

#define MODELID_FB_254 722
#define MODELID_FB_265 723
#define MODELID_FB_240 761

#define MODELID_MORPHEUS_210 725
#define MODELID_MORPHEUS_221 726
#define MODELID_MORPHEUS_232 727
#define MODELID_MORPHEUS_243 728
#define MODELID_MORPHEUS_254 729
#define MODELID_MORPHEUS_265 730
#define MODELID_MORPHEUS_276 731
#define MODELID_MORPHEUS_287 732
#define MODELID_MORPHEUS_298 733

#define MODELID_OBSESSION_210 736
#define MODELID_OBSESSION_240 737
#define MODELID_OBSESSION_270 738
#define MODELID_OBSESSION_290 739

#define MODELID_GFORCE_220 741
#define MODELID_GFORCE_250 742

#define MODELID_INFINITY_160 744
#define MODELID_INFINITY_190 745

#define MODELID_PETRA_69 915
#define MODELID_EFREET_49 919

#define DESCRIPTIONID_PSYCHONAUT 345
#define DESCRIPTIONID_JAHNCLE    346
#define DESCRIPTIONID_HAIBANE    378
#define DESCRIPTIONID_FB         724
#define DESCRIPTIONID_MORPHEUS   734
#define DESCRIPTIONID_OBSESSION  740
#define DESCRIPTIONID_GFORCE     743
#define DESCRIPTIONID_INFINITY   746
#define DESCRIPTIONID_PETRA   915
#define DESCRIPTIONID_EFREET   919

/**
 * riser scheme 02 - common skydiving canopy (9 sections)
 */

const unsigned int riserScheme02NumPABs = 32;

static PABDesc riserScheme02PABs[riserScheme02NumPABs] =
{
 /* 01 lbr1 */ { "Bone39", pabtLeftFlap,     Vector3f(0,1,0),  45.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* 02 lbr2 */ { "Bone57", pabtLeftFlap,     Vector3f(0,1,0),  40.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* 03 lbr3 */ { "Bone63", pabtLeftFlap,     Vector3f(0,1,0),  35.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* 04 lbr4 */ { "Bone60", pabtLeftFlap,     Vector3f(0,1,0),  30.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* 05 rbr1 */ { "Bone49", pabtRightFlap,    Vector3f(0,1,0),  45.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* 06 rbr2 */ { "Bone19", pabtRightFlap,    Vector3f(0,1,0),  40.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* 07 rbr3 */ { "Bone22", pabtRightFlap,    Vector3f(0,1,0),  35.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* 08 rbr4 */ { "Bone16", pabtRightFlap,    Vector3f(0,1,0),  30.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* 09 rls1 */ { "Bone38", pabtRearSection,  Vector3f(-1,0,0), 60.0f, 60.0f, "Bone39", 0.5f, losLeft, 1.0f },
 /* 10 rls2 */ { "Bone56", pabtRearSection,  Vector3f(-1,0,0), 60.0f, 60.0f, "Bone57", 0.5f, losLeft, 0.75f },
 /* 11 rls3 */ { "Bone62", pabtRearSection,  Vector3f(-1,0,0), 60.0f, 60.0f, "Bone63", 0.5f, losLeft, 0.5f },
 /* 12 rls4 */ { "Bone59", pabtRearSection,  Vector3f(-1,0,0), 60.0f, 60.0f, "Bone60", 0.5f, losLeft, 0.25f },
 /* 13 rls5 */ { "Bone51", pabtRearSection,  Vector3f(-1,0,0), 60.0f, 60.0f, NULL,     0.0f, losLeft, 0.125f },
 /* 14 rrs1 */ { "Bone48", pabtRearSection,  Vector3f(1,0,0),  60.0f, 60.0f, "Bone49", 0.5f, losRight, 1.0f },
 /* 15 rrs2 */ { "Bone18", pabtRearSection,  Vector3f(1,0,0),  60.0f, 60.0f, "Bone19", 0.5f, losRight, 0.75f },
 /* 16 rrs3 */ { "Bone21", pabtRearSection,  Vector3f(1,0,0),  60.0f, 60.0f, "Bone22", 0.5f, losRight, 0.5f },
 /* 17 rrs4 */ { "Bone15", pabtRearSection,  Vector3f(1,0,0),  60.0f, 60.0f, "Bone16", 0.5f, losRight, 0.25f },
 /* 18 rrs5 */ { "Bone24", pabtRearSection,  Vector3f(1,0,0),  60.0f, 60.0f, NULL,     0.0f, losRight, 0.125f },
 /* 19 fls1 */ { "Bone42", pabtFrontSection, Vector3f(1,0,0),  90.0f, 60.0f, NULL,     0.0f, losLeft, 1.0f },
 /* 20 fls2 */ { "Bone35", pabtFrontSection, Vector3f(1,0,0),  90.0f, 60.0f, NULL,     0.0f, losLeft, 0.75f },
 /* 21 fls3 */ { "Bone33", pabtFrontSection, Vector3f(1,0,0),  90.0f, 60.0f, NULL,     0.0f, losLeft, 0.5f },
 /* 22 fls4 */ { "Bone29", pabtFrontSection, Vector3f(1,0,0),  90.0f, 60.0f, NULL,     0.0f, losLeft, 0.25f },
 /* 23 fls5 */ { "Bone31", pabtFrontSection, Vector3f(1,0,0),  90.0f, 60.0f, NULL,     0.0f, losLeft, 0.125f },
 /* 24 frs1 */ { "Bone45", pabtFrontSection, Vector3f(-1,0,0), 90.0f, 60.0f, NULL,     0.0f, losRight, 1.0f },
 /* 25 frs2 */ { "Bone04", pabtFrontSection, Vector3f(-1,0,0), 90.0f, 60.0f, NULL,     0.0f, losRight, 0.75f },
 /* 26 frs3 */ { "Bone06", pabtFrontSection, Vector3f(-1,0,0), 90.0f, 60.0f, NULL,     0.0f, losRight, 0.5f },
 /* 27 frs4 */ { "Bone08", pabtFrontSection, Vector3f(-1,0,0), 90.0f, 60.0f, NULL,     0.0f, losRight, 0.25f },
 /* 28 frs5 */ { "Bone10", pabtFrontSection, Vector3f(-1,0,0), 90.0f, 60.0f, NULL,     0.0f, losRight, 0.125f },
 /* 29 flsX */ { "Bone41", pabtRearSection,  Vector3f(0,1,0),  60.0f, 60.0f, NULL,     0.0f, losLeft, 0.5f },
 /* 30 rlsX */ { "Bone37", pabtRearSection,  Vector3f(0,1,0),  60.0f, 60.0f, NULL,     0.0f, losLeft, 0.5f },
 /* 31 frsX */ { "Bone44", pabtRearSection,  Vector3f(0,1,0),  60.0f, 60.0f, NULL,     0.0f, losRight, 0.5f },
 /* 32 rrsX */ { "Bone47", pabtRearSection,  Vector3f(0,1,0),  60.0f, 60.0f, NULL,     0.0f, losRight, 0.5f }
};

static class RiserScheme02 : public RiserScheme
{
public:
    virtual unsigned int getNumCords(void)
    {
        return 5;
    }
    virtual unsigned int getNumBrakes(void)
    {
        return 4;
    }
    virtual const char* getJointName(RiserScheme::RiserType riserType, unsigned int cordId)
    {        
        switch( riserType )
        {
        case rtOuterFrontLeft:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone43";
            if( cordId == 1 ) return "Bone36";
            if( cordId == 2 ) return "Bone34";
            if( cordId == 3 ) return "Bone30";
            if( cordId == 4 ) return "Bone32";
            break;
        case rtOuterRearLeft:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone39";
            if( cordId == 1 ) return "Bone57";
            if( cordId == 2 ) return "Bone63";
            if( cordId == 3 ) return "Bone60";
            if( cordId == 4 ) return "Bone52";
            break;
        case rtOuterFrontRight:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone46";
            if( cordId == 1 ) return "Bone05";
            if( cordId == 2 ) return "Bone07";
            if( cordId == 3 ) return "Bone09";
            if( cordId == 4 ) return "Bone11";
            break;
        case rtOuterRearRight:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone49";
            if( cordId == 1 ) return "Bone19";
            if( cordId == 2 ) return "Bone22";
            if( cordId == 3 ) return "Bone16";
            if( cordId == 4 ) return "Bone25";
            break;
        case rtInnerFrontLeft:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone42";
            if( cordId == 1 ) return "Bone35";
            if( cordId == 2 ) return "Bone33";
            if( cordId == 3 ) return "Bone29";
            if( cordId == 4 ) return "Bone31";
        case rtInnerRearLeft:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone38";
            if( cordId == 1 ) return "Bone56";
            if( cordId == 2 ) return "Bone62";
            if( cordId == 3 ) return "Bone59";
            if( cordId == 4 ) return "Bone51";
        case rtInnerFrontRight:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone45";
            if( cordId == 1 ) return "Bone04";
            if( cordId == 2 ) return "Bone06";
            if( cordId == 3 ) return "Bone08";
            if( cordId == 4 ) return "Bone10";
        case rtInnerRearRight:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone48";
            if( cordId == 1 ) return "Bone18";
            if( cordId == 2 ) return "Bone21";
            if( cordId == 3 ) return "Bone15";
            if( cordId == 4 ) return "Bone24";
        case rtBrakeLeft:
            assert( cordId >= 0 && cordId < getNumBrakes() );
            if( cordId == 0 ) return "Bone40";
            if( cordId == 1 ) return "Bone58";
            if( cordId == 2 ) return "Bone64";
            if( cordId == 3 ) return "Bone61";
        case rtBrakeRight:
            assert( cordId >= 0 && cordId < getNumBrakes() );
            if( cordId == 0 ) return "Bone50";
            if( cordId == 1 ) return "Bone20";
            if( cordId == 2 ) return "Bone23";
            if( cordId == 3 ) return "Bone17";
        }
        return NULL;
    }
    virtual const char* getLeftLineoverJointName(void)
    {
        return "Bone55";
    }
    virtual const char* getRightLineoverJointName(void)
    {
        return "Bone14";
    }
    virtual unsigned int getNumPABs(void)
    {
        return riserScheme02NumPABs;
    }
    virtual PABDesc* getPAB(unsigned int pabId)
    {
        assert( pabId>=0 && pabId<riserScheme02NumPABs );
        return riserScheme02PABs+pabId;
    }
} riserScheme02;

/**
 * riser scheme 01 - common BASE canopy (7 sections)
 */

const unsigned int riserScheme01NumPABs = 26;

static PABDesc riserScheme01PABs[riserScheme01NumPABs] =
{
 /* lbr1 */ { "Bone57", pabtLeftFlap,     Vector3f(0,1,0),  45.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* lbr2 */ { "Bone63", pabtLeftFlap,     Vector3f(0,1,0),  35.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* lbr3 */ { "Bone60", pabtLeftFlap,     Vector3f(0,1,0),  30.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* rbr1 */ { "Bone19", pabtRightFlap,    Vector3f(0,1,0),  45.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* rbr2 */ { "Bone22", pabtRightFlap,    Vector3f(0,1,0),  35.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* rbr3 */ { "Bone16", pabtRightFlap,    Vector3f(0,1,0),  30.0f, 90.0f, NULL,     0.0f, losIndifferent, 0.0f },
 /* rls1 */ { "Bone56", pabtRearSection,  Vector3f(-1,0,0), 60.0f, 60.0f, "Bone57", 0.5f, losLeft, 1.0f },
 /* rls2 */ { "Bone62", pabtRearSection,  Vector3f(-1,0,0), 60.0f, 60.0f, "Bone63", 0.5f, losLeft, 0.75f },
 /* rls3 */ { "Bone59", pabtRearSection,  Vector3f(-1,0,0), 60.0f, 60.0f, "Bone60", 0.5f, losLeft, 0.5f },
 /* rls4 */ { "Bone51", pabtRearSection,  Vector3f(-1,0,0),  60.0f, 60.0f, NULL,    0.0f, losLeft, 0.25f },
 /* rrs1 */ { "Bone18", pabtRearSection,  Vector3f(1,0,0),  60.0f, 60.0f, "Bone19", 0.5f, losRight, 1.0f },
 /* rrs2 */ { "Bone21", pabtRearSection,  Vector3f(1,0,0),  60.0f, 60.0f, "Bone22", 0.5f, losRight, 0.75f },
 /* rrs3 */ { "Bone15", pabtRearSection,  Vector3f(1,0,0),  60.0f, 60.0f, "Bone16", 0.5f, losRight, 0.5f },
 /* rrs4 */ { "Bone24", pabtRearSection,  Vector3f(1,0,0),  60.0f, 60.0f, NULL,     0.0f, losRight, 0.25f },
 /* fls1 */ { "Bone35", pabtFrontSection, Vector3f(1,0,0),  90.0f, 60.0f, NULL,     0.0f, losLeft, 0.75f },
 /* fls2 */ { "Bone33", pabtFrontSection, Vector3f(1,0,0),  90.0f, 60.0f, NULL,     0.0f, losLeft, 0.5f },
 /* fls3 */ { "Bone29", pabtFrontSection, Vector3f(1,0,0),  90.0f, 60.0f, NULL,     0.0f, losLeft, 0.25f },
 /* fls4 */ { "Bone31", pabtFrontSection, Vector3f(1,0,0),  90.0f, 60.0f, NULL,     0.0f, losLeft, 0.125f },
 /* frs1 */ { "Bone04", pabtFrontSection, Vector3f(-1,0,0), 90.0f, 60.0f, NULL,     0.0f, losRight, 0.75f },
 /* frs2 */ { "Bone06", pabtFrontSection, Vector3f(-1,0,0), 90.0f, 60.0f, NULL,     0.0f, losRight, 0.5f },
 /* frs3 */ { "Bone08", pabtFrontSection, Vector3f(-1,0,0), 90.0f, 60.0f, NULL,     0.0f, losRight, 0.25f },
 /* frs4 */ { "Bone10", pabtFrontSection, Vector3f(-1,0,0), 90.0f, 60.0f, NULL,     0.0f, losRight, 0.125f },
 /* flsX */ { "Bone28", pabtRearSection,  Vector3f(0,1,0),  60.0f, 60.0f, NULL,     0.0f, losLeft, 0.5f },
 /* rlsX */ { "Bone55", pabtRearSection,  Vector3f(0,1,0),  60.0f, 60.0f, NULL,     0.0f, losLeft, 0.5f },
 /* frsX */ { "Bone03", pabtRearSection,  Vector3f(0,1,0),  60.0f, 60.0f, NULL,     0.0f, losRight, 0.5f },
 /* rrsX */ { "Bone14", pabtRearSection,  Vector3f(0,1,0),  60.0f, 60.0f, NULL,     0.0f, losRight, 0.5f }
};

static class RiserScheme01 : public RiserScheme
{
public:
    virtual unsigned int getNumCords(void)
    {
        return 4;
    }
    virtual unsigned int getNumBrakes(void)
    {
        return 3;
    }
    virtual const char* getJointName(RiserScheme::RiserType riserType, unsigned int cordId)
    {        
        switch( riserType )
        {
        case rtOuterFrontLeft:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone36";
            if( cordId == 1 ) return "Bone34";
            if( cordId == 2 ) return "Bone30";
            if( cordId == 3 ) return "Bone32";
            break;
        case rtOuterRearLeft:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone57";
            if( cordId == 1 ) return "Bone63";
            if( cordId == 2 ) return "Bone60";
            if( cordId == 3 ) return "Bone52";
            break;
        case rtOuterFrontRight:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone11";
            if( cordId == 1 ) return "Bone09";
            if( cordId == 2 ) return "Bone07";
            if( cordId == 3 ) return "Bone05";            
            break;
        case rtOuterRearRight:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone25";
            if( cordId == 1 ) return "Bone16";
            if( cordId == 2 ) return "Bone22";
            if( cordId == 3 ) return "Bone19";
            break;
        case rtInnerFrontLeft:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone35";
            if( cordId == 1 ) return "Bone33";
            if( cordId == 2 ) return "Bone29";
            if( cordId == 3 ) return "Bone31";
        case rtInnerRearLeft:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone56";
            if( cordId == 1 ) return "Bone62";
            if( cordId == 2 ) return "Bone59";
            if( cordId == 3 ) return "Bone51";
        case rtInnerFrontRight:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone10";
            if( cordId == 1 ) return "Bone08";
            if( cordId == 2 ) return "Bone06";
            if( cordId == 3 ) return "Bone04";
        case rtInnerRearRight:
            assert( cordId >= 0 && cordId < getNumCords() );
            if( cordId == 0 ) return "Bone24";
            if( cordId == 1 ) return "Bone15";
            if( cordId == 2 ) return "Bone21";
            if( cordId == 3 ) return "Bone18";
        case rtBrakeLeft:
            assert( cordId >= 0 && cordId < getNumBrakes() );
            if( cordId == 0 ) return "Bone58";
            if( cordId == 1 ) return "Bone64";
            if( cordId == 2 ) return "Bone61";
        case rtBrakeRight:
            assert( cordId >= 0 && cordId < getNumBrakes() );
            if( cordId == 0 ) return "Bone20";
            if( cordId == 1 ) return "Bone23";
            if( cordId == 2 ) return "Bone17";
        }
        return NULL;
    }
    virtual const char* getLeftLineoverJointName(void)
    {
        return "Bone62";
    }
    virtual const char* getRightLineoverJointName(void)
    {
        return "Bone21";
    }
    virtual unsigned int getNumPABs(void)
    {
        return riserScheme01NumPABs;
    }
    virtual PABDesc* getPAB(unsigned int pabId)
    {
        assert( pabId>=0 && pabId<riserScheme01NumPABs );
        return riserScheme01PABs+pabId;
    }
} riserScheme01;

/**
 * main database
 */

// color previews (uv coords)
#define COLOR_DISABLED         gui::Rect( 0,0,0,0 ), 0
#define COLOR_P_01_01          gui::Rect( 16,0,31,31 ), 49
#define COLOR_P_01_02          gui::Rect( 32,0,47,31 ), 50
#define COLOR_P_01_03          gui::Rect( 48,0,63,31 ), 51
#define COLOR_P_01_05          gui::Rect( 64,0,79,31 ), 52
#define COLOR_P_01_06          gui::Rect( 80,0,95,31 ), 53
#define COLOR_P_01_07          gui::Rect( 96,0,111,31 ), 54
#define COLOR_P_onecolor_01_08 gui::Rect( 112,0,127,31 ), 55
#define COLOR_P_onecolor_01_10 gui::Rect( 128,0,143,31 ), 56
#define COLOR_P_onecolor_01_11 gui::Rect( 144,0,159,31 ), 57
#define COLOR_P_onecolor_01_13 gui::Rect( 160,0,175,31 ), 58
#define COLOR_P_onecolor_01_16 gui::Rect( 176,0,191,31 ), 59
#define COLOR_P_onecolor_01_18 gui::Rect( 192,0,207,31 ), 60
#define COLOR_P_onecolor_01_20 gui::Rect( 208,0,223,31 ), 61
#define COLOR_P_onecolor_01_21 gui::Rect( 224,0,239,31 ), 62
#define COLOR_P_onecolor_01_22 gui::Rect( 240,0,255,31 ), 63
#define COLOR_Morpheus_P01_02  gui::Rect( 256,0,271,31 ), 65
#define COLOR_Obsession_03     gui::Rect( 272,0,287,31 ), 66
#define COLOR_GForce_sky_01    gui::Rect( 288,0,303,31 ), 67 
#define COLOR_Infinity_sky_01  gui::Rect( 304,0,319,31 ), 68
#define COLOR_FBRedBull        gui::Rect( 320,0,335,31 ), 69
#define COLOR_Main_petra_69    gui::Rect( 352,0,367,31 ), 72

// inflation dynamics
// SRDminvel, SRDmink, SRDmaxvel, SRDmaxk, SUminvel, SUmink, SUmaxvel, SUmaxk
#define INFL_PSYCHONAUT 2.5f, 0.0f, 20.0f, 0.5f, 0.0f, 0.125f, 40.0f, 0.05f
#define INFL_JAHNCLE    5.0f, 0.0f, 20.0f, 1.0f, 0.0f, 0.250f, 70.0f, 0.0625f
#define INFL_HAIBANE    0.0f, 0.0f,  1.0f, 0.0f, 0.0f, 0.125f, 40.0f, 0.0375f

// collapse dynamics
// Cnum, Cradius, Cpower, Cminvel, Cmaxvel, Crestore, Cresvel

#define COLLAPSE_PSYCHONAUT 4, 2.5f, 1.0f/2.0f, 0.25f, 12.0f, 0.5f, 9.0f
#define COLLAPSE_JAHNCLE    4, 2.5f, 1.0f/2.0f, 0.25f, 12.0f, 0.5f, 9.0f
#define COLLAPSE_HAIBANE    4, 2.5f, 1.0f/2.0f, 0.25f, 12.0f, 0.5f, 9.0f

// ageing dynamics
// Kminoverb, Kmindamage, Kmaxoverb, Kmaxdamage, Kminfrict, Kminrip, Kmaxfrict, Kmaxrip

#define AGEING_PSYCHONAUT 3.0f, 0.0f, 12.0f, 1.0f/120.f, 100.0f, 0.0f, 1000.0f, 1.0f/60.0f
#define AGEING_JAHNCLE    3.0f, 0.0f, 12.0f, 1.0f/140.f, 100.0f, 0.0f, 1200.0f, 1.0f/60.0f
#define AGEING_HAIBANE    3.0f, 0.0f, 12.0f, 1.0f/120.f, 100.0f, 0.0f,  850.0f, 1.0f/60.0f

// travel of attack angle 
#define AA_PSYCHONAUT 7.5f
#define AA_JAHNCLE    12.5f
#define AA_HAIBANE    10.0f

/**
 * pilotchute options
 */

const unsigned int numBasePilots = 6;

static database::Pilotchute basePilots[numBasePilots] = 
{
    //        size, mass, scale, Vrec, Talign, Tgyres, Fair
    /* 0 */ { 48, 0.36f, 1.2192f, 8.26f, 2.0f, 0.5f, 4.0f },
    /* 1 */ { 45, 0.30f, 1.1430f, 17.23f, 1.9f,  0.48f, 3.9f },
    /* 2 */ { 42, 0.28f, 1.0688f, 25.24f, 1.8f,  0.46f, 3.8f },
    /* 3 */ { 40, 0.26f, 1.0160f, 32.11f, 1.7f,  0.44f, 3.7f },
    /* 4 */ { 38, 0.25f, 0.9652f, 42.24f, 1.6f,  0.42f, 3.6f },
    /* 5 */ { 36, 0.24f, 0.9144f, 49.98f, 1.5f,  0.40f, 3.5f }
};

const unsigned int numSkydivingPilots = 2;

static database::Pilotchute skydivingPilots[numSkydivingPilots] = 
{
          // size, mass, scale, Vrec, Talign, Tgyres, Fair
    ///* 0 */ { 72, 0.48f, 1.8f, 39.98f, 1.5f,  0.40f, 8.0f },
    /* 0 */ { 36, 0.24f, 0.9144f, 49.98f, 1.5f,  0.40f, 4.0f },
    /* 1 */ { 32, 0.21f, 0.8f, 56.0f, 1.3f, 0.36f, 3.8f }
};

/**
 * properties
 */
//									nameId			   descriptionId		  manufId   mass   scale   area    aspect ftCord rearCord ctrCord cascade				Kyair   Kzair   Kxair  AAdeep          Klifts  Kliftd   Kdrags Kdragd Kbraking   Kturn  Kdampmin max    Vdampmax
#define PROPS_PSYCHONAUT_210 MODELID_PSYCHONAUT_210, DESCRIPTIONID_PSYCHONAUT, MFRID_D3, 2.5f, 4.417f, 210.0f, 2.0f, 3.010f, 3.220f, 3.220f, 2.5f, &riserScheme01, 35.000f, 2.00f, 10.00f, AA_PSYCHONAUT, 12.500f, 25.000f, 1.000f, 6.00f, 0.1750f, 0.750f, 6.00f, 12.00f, 20.00f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_PSYCHONAUT_221 MODELID_PSYCHONAUT_221, DESCRIPTIONID_PSYCHONAUT, MFRID_D3, 2.6f, 4.531f, 221.0f, 2.0f, 3.090f, 3.310f, 3.310f, 2.5f, &riserScheme01, 36.800f, 2.10f, 10.52f, AA_PSYCHONAUT, 13.150f, 26.300f, 1.050f, 6.31f, 0.1840f, 0.790f, 6.31f, 12.62f, 21.04f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_PSYCHONAUT_232 MODELID_PSYCHONAUT_232, DESCRIPTIONID_PSYCHONAUT, MFRID_D3, 2.7f, 4.642f, 232.0f, 2.0f, 3.160f, 3.390f, 3.390f, 2.5f, &riserScheme01, 38.600f, 2.20f, 11.04f, AA_PSYCHONAUT, 13.800f, 27.610f, 1.100f, 6.62f, 0.1930f, 0.828f, 6.62f, 13.25f, 22.09f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_PSYCHONAUT_243 MODELID_PSYCHONAUT_243, DESCRIPTIONID_PSYCHONAUT, MFRID_D3, 2.8f, 4.751f, 243.0f, 2.0f, 3.240f, 3.470f, 3.470f, 2.5f, &riserScheme01, 40.500f, 2.31f, 11.57f, AA_PSYCHONAUT, 14.460f, 28.920f, 1.157f, 6.94f, 0.2020f, 0.867f, 6.94f, 13.88f, 23.14f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_PSYCHONAUT_254 MODELID_PSYCHONAUT_254, DESCRIPTIONID_PSYCHONAUT, MFRID_D3, 2.9f, 4.857f, 254.0f, 2.0f, 3.310f, 3.550f, 3.550f, 2.5f, &riserScheme01, 42.310f, 2.42f, 12.10f, AA_PSYCHONAUT, 15.110f, 30.220f, 1.210f, 7.25f, 0.2110f, 0.900f, 7.25f, 14.51f, 24.20f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_PSYCHONAUT_265 MODELID_PSYCHONAUT_265, DESCRIPTIONID_PSYCHONAUT, MFRID_D3, 3.0f, 4.961f, 265.0f, 2.0f, 3.380f, 3.620f, 3.620f, 2.5f, &riserScheme01, 46.375f, 2.65f, 13.25f, AA_PSYCHONAUT, 16.560f, 33.125f, 1.325f, 7.95f, 0.2310f, 0.993f, 7.95f, 15.90f, 26.50f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_PSYCHONAUT_276 MODELID_PSYCHONAUT_276, DESCRIPTIONID_PSYCHONAUT, MFRID_D3, 3.1f, 5.063f, 276.0f, 2.0f, 3.450f, 3.700f, 3.700f, 2.5f, &riserScheme01, 48.300f, 2.76f, 13.80f, AA_PSYCHONAUT, 17.250f, 34.500f, 1.379f, 8.28f, 0.2415f, 1.035f, 8.28f, 16.56f, 27.60f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_PSYCHONAUT_287 MODELID_PSYCHONAUT_287, DESCRIPTIONID_PSYCHONAUT, MFRID_D3, 3.2f, 5.163f, 287.0f, 2.0f, 3.520f, 3.775f, 3.775f, 2.5f, &riserScheme01, 50.220f, 2.86f, 14.35f, AA_PSYCHONAUT, 17.940f, 35.870f, 1.434f, 8.60f, 0.2510f, 1.076f, 8.60f, 17.22f, 28.70f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_PSYCHONAUT_298 MODELID_PSYCHONAUT_298, DESCRIPTIONID_PSYCHONAUT, MFRID_D3, 3.3f, 5.261f, 298.0f, 2.0f, 3.588f, 3.845f, 3.845f, 2.5f, &riserScheme01, 52.150f, 2.98f, 14.90f, AA_PSYCHONAUT, 18.625f, 37.250f, 1.489f, 8.94f, 0.2600f, 1.117f, 8.94f, 17.88f, 29.80f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_PSYCHONAUT_309 MODELID_PSYCHONAUT_309, DESCRIPTIONID_PSYCHONAUT, MFRID_D3, 3.4f, 5.358f, 309.0f, 2.0f, 3.655f, 3.916f, 3.916f, 2.5f, &riserScheme01, 54.070f, 3.09f, 15.45f, AA_PSYCHONAUT, 18.540f, 38.620f, 1.545f, 9.26f, 0.2700f, 1.158f, 9.26f, 18.54f, 30.90f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots


#define PROPS_JAHNCLE_190 MODELID_JAHNCLE_190, DESCRIPTIONID_JAHNCLE, MFRID_D3, 2.25f, 4.201f, 190.0f, 2.0f, 2.755f, 3.087f, 3.087f, 2.25f, &riserScheme01, 31.66f, 1.81f,  9.04f, AA_JAHNCLE, 11.31f, 22.62f, 0.904f, 5.43f, 0.158f, 0.680f, 5.43f, 10.85f, 18.10f, INFL_JAHNCLE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numBasePilots, basePilots
#define PROPS_JAHNCLE_210 MODELID_JAHNCLE_210, DESCRIPTIONID_JAHNCLE, MFRID_D3, 2.50f, 4.417f, 210.0f, 2.0f, 2.900f, 3.250f, 3.250f, 2.25f, &riserScheme01, 35.00f, 2.00f, 10.00f, AA_JAHNCLE, 12.50f, 25.00f, 1.000f, 6.00f, 0.175f, 0.750f, 6.00f, 12.00f, 20.00f, INFL_JAHNCLE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numBasePilots, basePilots
#define PROPS_JAHNCLE_240 MODELID_JAHNCLE_240, DESCRIPTIONID_JAHNCLE, MFRID_D3, 2.75f, 4.722f, 240.0f, 2.0f, 3.097f, 3.470f, 3.470f, 2.25f, &riserScheme01, 40.00f, 2.28f, 11.42f, AA_JAHNCLE, 14.28f, 28.57f, 1.142f, 6.86f, 0.200f, 0.857f, 6.86f, 13.71f, 22.85f, INFL_JAHNCLE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numBasePilots, basePilots
#define PROPS_JAHNCLE_270 MODELID_JAHNCLE_270, DESCRIPTIONID_JAHNCLE, MFRID_D3, 3.00f, 5.008f, 270.0f, 2.0f, 3.285f, 3.680f, 3.680f, 2.25f, &riserScheme01, 45.00f, 2.57f, 12.85f, AA_JAHNCLE, 16.07f, 32.14f, 1.285f, 7.71f, 0.225f, 0.964f, 7.71f, 15.43f, 25.70f, INFL_JAHNCLE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numBasePilots, basePilots
#define PROPS_JAHNCLE_290 MODELID_JAHNCLE_290, DESCRIPTIONID_JAHNCLE, MFRID_D3, 3.25f, 5.190f, 290.0f, 2.0f, 3.400f, 3.814f, 3.814f, 2.25f, &riserScheme01, 48.33f, 2.76f, 13.80f, AA_JAHNCLE, 17.26f, 34.52f, 1.380f, 8.28f, 0.242f, 1.035f, 8.28f, 16.57f, 27.61f, INFL_JAHNCLE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numBasePilots, basePilots

#define PROPS_JAHNCLE_150 MODELID_JAHNCLE_150, DESCRIPTIONID_JAHNCLE, MFRID_D3, 2.30f, 3.300f, 150.0f, 2.5f,  2.20f, 2.400f,  2.300f,   2.25f, &riserScheme01,29.00f, 1.20f,  8.50f, AA_JAHNCLE, 11.00f, 21.00f, 0.880f, 5.30f, 0.145f,   0.400f,  5.10f, 10.00f, 17.00f,INFL_JAHNCLE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numBasePilots, basePilots
//						  nameId			   descriptionId		  manufId   mass   scale   area    aspect ftCord rearCord ctrCord cascade				  Kyair   Kzair  Kxair  AAdeep      Klifts  Kliftd  Kdrags  Kdragd Kbraking  Kturn  Kdampmin max    Vdampmax
// 2.93
//#define PROPS_PETRA_69	  MODELID_PETRA_69,    DESCRIPTIONID_PETRA,   MFRID_D3, 1.70f, 2.022f*1.7f, 69.0f,  1.4607f, 2.4f, 2.500f,  2.4f, 2.25f, &riserScheme02, 9.50f, 0.070f, 11.0f, AA_HAIBANE,  7.50f, 11.00f, 0.050f, 0.09f, 0.0600f, 0.15100f, 25.20f, 20.00f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots
#define PROPS_PETRA_69	  MODELID_PETRA_69,     DESCRIPTIONID_PETRA, MFRID_D3,  1.70f, 2.022f, 69.0f,  1.4607f, 1.095f, 1.370f, 1.370f, 1.2f, &riserScheme02, 9.00f, 0.100f, 11.0f, AA_HAIBANE,  7.50f, 11.00f, 0.050f, 0.09f, 0.0500f, 0.15100f, 9.20f, 20.00f,  14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots
#define PROPS_EFREET_49	  MODELID_EFREET_49,    DESCRIPTIONID_EFREET,MFRID_D3,  0.98f, 1.900f, 49.0f,  2.7000f, 0.800f, 1.200f, 1.200f, 1.0f, &riserScheme01, 1.00f, 0.040f, 10.0f,  10.0f,      9.50f, 12.00f, 0.150f, 0.25f, 0.1000f, 0.15000f, 25.00f, 50.00f, 16.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots

#define PROPS_HAIBANE_71  MODELID_HAIBANE_71,  DESCRIPTIONID_HAIBANE, MFRID_D3, 1.30f, 2.122f, 71.0f,  2.45f, 1.295f, 1.670f, 1.670f, 1.0f, &riserScheme02, 13.00f, 0.270f, 16.0f, AA_HAIBANE,  9.70f, 13.20f, 0.110f, 0.44f, 0.0350f, 0.07500f, 1.30f, 4.50f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots
#define PROPS_HAIBANE_100 MODELID_HAIBANE_100, DESCRIPTIONID_HAIBANE, MFRID_D3, 1.70f, 2.988f, 100.0f, 2.5f,  1.850f, 2.400f, 2.400f, 1.2f, &riserScheme02, 14.00f, 0.380f, 26.0f, AA_HAIBANE, 11.50f, 15.00f, 0.150f, 0.59f, 0.0490f, 0.11500f, 1.10f, 4.00f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots
#define PROPS_HAIBANE_130 MODELID_HAIBANE_130, DESCRIPTIONID_HAIBANE, MFRID_D3, 2.50f, 3.885f, 130.0f, 2.5f,  2.650f, 2.900f, 2.900f, 1.5f, &riserScheme02, 15.00f, 0.500f, 30.0f, AA_HAIBANE, 12.00f, 15.50f, 0.350f, 0.60f, 0.0500f, 0.12500f, 1.20f, 4.50f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots
#define PROPS_HAIBANE_160 MODELID_HAIBANE_160, DESCRIPTIONID_HAIBANE, MFRID_D3, 2.75f, 4.310f, 160.0f, 2.5f,  2.940f, 3.220f, 3.220f, 1.5f, &riserScheme02, 16.65f, 0.555f, 33.3f, AA_HAIBANE, 13.00f, 17.00f, 0.400f, 0.65f, 0.0555f, 0.13875f, 1.30f, 5.00f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots
#define PROPS_HAIBANE_190 MODELID_HAIBANE_190, DESCRIPTIONID_HAIBANE, MFRID_D3, 3.00f, 4.697f, 190.0f, 2.5f,  3.200f, 3.510f, 3.510f, 1.5f, &riserScheme02, 18.15f, 0.605f, 36.3f, AA_HAIBANE, 14.00f, 18.50f, 0.450f, 0.70f, 0.0605f, 0.15100f, 1.40f, 5.50f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots
#define PROPS_HAIBANE_220 MODELID_HAIBANE_220, DESCRIPTIONID_HAIBANE, MFRID_D3, 3.25f, 5.054f, 220.0f, 2.5f,  3.447f, 3.774f, 3.774f, 1.5f, &riserScheme02, 19.50f, 0.650f, 39.0f, AA_HAIBANE, 14.50f, 19.00f, 0.475f, 0.72f, 0.0650f, 0.16250f, 1.45f, 5.75f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots
#define PROPS_HAIBANE_250 MODELID_HAIBANE_250, DESCRIPTIONID_HAIBANE, MFRID_D3, 3.50f, 5.388f, 250.0f, 2.5f,  3.670f, 3.850f, 3.850f, 1.5f, &riserScheme02, 20.80f, 0.690f, 41.6f, AA_HAIBANE, 15.00f, 20.00f, 0.500f, 0.75f, 0.0690f, 0.17300f, 1.50f, 6.00f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots

#define PROPS_FB_254 MODELID_FB_254, DESCRIPTIONID_FB, MFRID_D3, 2.9f, 4.857f, 254.0f, 2.0f, 3.310f, 3.550f, 3.550f, 2.5f, &riserScheme01, 42.310f, 2.42f, 12.10f, AA_PSYCHONAUT, 15.110f, 30.220f, 1.210f, 7.25f, 0.2110f, 0.900f, 7.25f, 14.51f, 24.20f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_FB_265 MODELID_FB_265, DESCRIPTIONID_FB, MFRID_D3, 3.0f, 4.961f, 265.0f, 2.0f, 3.380f, 3.620f, 3.620f, 2.5f, &riserScheme01, 46.375f, 2.65f, 13.25f, AA_PSYCHONAUT, 16.560f, 33.125f, 1.325f, 7.95f, 0.2310f, 0.993f, 7.95f, 15.90f, 26.50f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_FB_240 MODELID_FB_240, DESCRIPTIONID_FB, MFRID_D3, 2.75f, 4.722f, 240.0f, 2.0f, 3.097f, 3.470f, 3.470f, 2.25f, &riserScheme01, 40.00f, 2.28f, 11.42f, AA_JAHNCLE, 14.28f, 28.57f, 1.142f, 6.86f, 0.200f, 0.857f, 6.86f, 13.71f, 22.85f, INFL_HAIBANE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numSkydivingPilots, skydivingPilots

#define PROPS_MORPHEUS_210 MODELID_MORPHEUS_210, DESCRIPTIONID_MORPHEUS, MFRID_UTURN, 2.5f, 4.417f, 210.0f, 2.0f, 3.010f, 3.220f, 3.220f, 2.5f, &riserScheme01, 35.000f, 2.00f, 10.00f, AA_PSYCHONAUT, 12.500f, 25.000f, 1.000f, 6.00f, 0.1750f, 0.750f, 6.00f, 12.00f, 20.00f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_MORPHEUS_221 MODELID_MORPHEUS_221, DESCRIPTIONID_MORPHEUS, MFRID_UTURN, 2.6f, 4.531f, 221.0f, 2.0f, 3.090f, 3.310f, 3.310f, 2.5f, &riserScheme01, 36.800f, 2.10f, 10.52f, AA_PSYCHONAUT, 13.150f, 26.300f, 1.050f, 6.31f, 0.1840f, 0.790f, 6.31f, 12.62f, 21.04f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_MORPHEUS_232 MODELID_MORPHEUS_232, DESCRIPTIONID_MORPHEUS, MFRID_UTURN, 2.7f, 4.642f, 232.0f, 2.0f, 3.160f, 3.390f, 3.390f, 2.5f, &riserScheme01, 38.600f, 2.20f, 11.04f, AA_PSYCHONAUT, 13.800f, 27.610f, 1.100f, 6.62f, 0.1930f, 0.828f, 6.62f, 13.25f, 22.09f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_MORPHEUS_243 MODELID_MORPHEUS_243, DESCRIPTIONID_MORPHEUS, MFRID_UTURN, 2.8f, 4.751f, 243.0f, 2.0f, 3.240f, 3.470f, 3.470f, 2.5f, &riserScheme01, 40.500f, 2.31f, 11.57f, AA_PSYCHONAUT, 14.460f, 28.920f, 1.157f, 6.94f, 0.2020f, 0.867f, 6.94f, 13.88f, 23.14f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_MORPHEUS_254 MODELID_MORPHEUS_254, DESCRIPTIONID_MORPHEUS, MFRID_UTURN, 2.9f, 4.857f, 254.0f, 2.0f, 3.310f, 3.550f, 3.550f, 2.5f, &riserScheme01, 42.310f, 2.42f, 12.10f, AA_PSYCHONAUT, 15.110f, 30.220f, 1.210f, 7.25f, 0.2110f, 0.900f, 7.25f, 14.51f, 24.20f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_MORPHEUS_265 MODELID_MORPHEUS_265, DESCRIPTIONID_MORPHEUS, MFRID_UTURN, 3.0f, 4.961f, 265.0f, 2.0f, 3.380f, 3.620f, 3.620f, 2.5f, &riserScheme01, 46.375f, 2.65f, 13.25f, AA_PSYCHONAUT, 16.560f, 33.125f, 1.325f, 7.95f, 0.2310f, 0.993f, 7.95f, 15.90f, 26.50f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_MORPHEUS_276 MODELID_MORPHEUS_276, DESCRIPTIONID_MORPHEUS, MFRID_UTURN, 3.1f, 5.063f, 276.0f, 2.0f, 3.450f, 3.700f, 3.700f, 2.5f, &riserScheme01, 48.300f, 2.76f, 13.80f, AA_PSYCHONAUT, 17.250f, 34.500f, 1.379f, 8.28f, 0.2415f, 1.035f, 8.28f, 16.56f, 27.60f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_MORPHEUS_287 MODELID_MORPHEUS_287, DESCRIPTIONID_MORPHEUS, MFRID_UTURN, 3.2f, 5.163f, 287.0f, 2.0f, 3.520f, 3.775f, 3.775f, 2.5f, &riserScheme01, 50.220f, 2.86f, 14.35f, AA_PSYCHONAUT, 17.940f, 35.870f, 1.434f, 8.60f, 0.2510f, 1.076f, 8.60f, 17.22f, 28.70f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots
#define PROPS_MORPHEUS_298 MODELID_MORPHEUS_298, DESCRIPTIONID_MORPHEUS, MFRID_UTURN, 3.3f, 5.261f, 298.0f, 2.0f, 3.588f, 3.845f, 3.845f, 2.5f, &riserScheme01, 52.150f, 2.98f, 14.90f, AA_PSYCHONAUT, 18.625f, 37.250f, 1.489f, 8.94f, 0.2600f, 1.117f, 8.94f, 17.88f, 29.80f, INFL_PSYCHONAUT, COLLAPSE_PSYCHONAUT, AGEING_PSYCHONAUT, numBasePilots, basePilots

#define PROPS_OBSESSION_210 MODELID_OBSESSION_210, DESCRIPTIONID_OBSESSION, MFRID_UTURN, 2.50f, 4.417f, 210.0f, 2.0f, 2.900f, 3.250f, 3.250f, 2.25f, &riserScheme01, 35.00f, 2.00f, 10.00f, AA_JAHNCLE, 12.50f, 25.00f, 1.000f, 6.00f, 0.175f, 0.750f, 6.00f, 12.00f, 20.00f, INFL_JAHNCLE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numBasePilots, basePilots
#define PROPS_OBSESSION_240 MODELID_OBSESSION_240, DESCRIPTIONID_OBSESSION, MFRID_UTURN, 2.75f, 4.722f, 240.0f, 2.0f, 3.097f, 3.470f, 3.470f, 2.25f, &riserScheme01, 40.00f, 2.28f, 11.42f, AA_JAHNCLE, 14.28f, 28.57f, 1.142f, 6.86f, 0.200f, 0.857f, 6.86f, 13.71f, 22.85f, INFL_JAHNCLE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numBasePilots, basePilots
#define PROPS_OBSESSION_270 MODELID_OBSESSION_270, DESCRIPTIONID_OBSESSION, MFRID_UTURN, 3.00f, 5.008f, 270.0f, 2.0f, 3.285f, 3.680f, 3.680f, 2.25f, &riserScheme01, 45.00f, 2.57f, 12.85f, AA_JAHNCLE, 16.07f, 32.14f, 1.285f, 7.71f, 0.225f, 0.964f, 7.71f, 15.43f, 25.70f, INFL_JAHNCLE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numBasePilots, basePilots
#define PROPS_OBSESSION_290 MODELID_OBSESSION_290, DESCRIPTIONID_OBSESSION, MFRID_UTURN, 3.25f, 5.190f, 290.0f, 2.0f, 3.400f, 3.814f, 3.814f, 2.25f, &riserScheme01, 48.33f, 2.76f, 13.80f, AA_JAHNCLE, 17.26f, 34.52f, 1.380f, 8.28f, 0.242f, 1.035f, 8.28f, 16.57f, 27.61f, INFL_JAHNCLE, COLLAPSE_JAHNCLE, AGEING_JAHNCLE, numBasePilots, basePilots

#define PROPS_GFORCE_220 MODELID_GFORCE_220, DESCRIPTIONID_GFORCE, MFRID_UTURN, 3.25f, 5.054f, 220.0f, 2.5f, 3.447f, 3.774f, 3.774f, 1.5f, &riserScheme02, 19.50f, 0.650f, 39.0f, AA_HAIBANE, 14.50f, 19.00f, 0.475f, 0.72f, 0.0650f, 0.16250f, 1.45f, 5.75f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots
#define PROPS_GFORCE_250 MODELID_GFORCE_250, DESCRIPTIONID_GFORCE, MFRID_UTURN, 3.50f, 5.388f, 250.0f, 2.5f, 3.670f, 3.850f, 3.850f, 1.5f, &riserScheme02, 20.80f, 0.690f, 41.6f, AA_HAIBANE, 15.00f, 20.00f, 0.500f, 0.75f, 0.0690f, 0.17300f, 1.50f, 6.00f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots

#define PROPS_INFINITY_160 MODELID_INFINITY_160, DESCRIPTIONID_INFINITY, MFRID_UTURN, 2.75f, 4.310f, 160.0f, 2.5f, 2.940f, 3.220f, 3.220f, 1.5f, &riserScheme02, 16.65f, 0.555f, 33.3f, AA_HAIBANE, 13.00f, 17.00f, 0.400f, 0.65f, 0.0555f, 0.13875f, 1.30f, 5.00f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots
#define PROPS_INFINITY_190 MODELID_INFINITY_190, DESCRIPTIONID_INFINITY, MFRID_UTURN, 3.00f, 4.697f, 190.0f, 2.5f, 3.200f, 3.510f, 3.510f, 1.5f, &riserScheme02, 18.15f, 0.605f, 36.3f, AA_HAIBANE, 14.00f, 18.50f, 0.450f, 0.70f, 0.0605f, 0.15100f, 1.40f, 5.50f, 14.00f, INFL_HAIBANE, COLLAPSE_HAIBANE, AGEING_HAIBANE, numSkydivingPilots, skydivingPilots

static Canopy canopies[] = 
{
    // skydiving, color, cost, templateName, props_macro

    /* 000 */ { REGULAR_TRADE, false, COLOR_P_01_01, 1050.0f, "Canopy01", PROPS_PSYCHONAUT_210 },
    /* 001 */ { REGULAR_TRADE, false, COLOR_P_01_01, 1100.0f, "Canopy01", PROPS_PSYCHONAUT_221 },
    /* 002 */ { REGULAR_TRADE, false, COLOR_P_01_01, 1150.0f, "Canopy01", PROPS_PSYCHONAUT_232 },
    /* 003 */ { REGULAR_TRADE, false, COLOR_P_01_01, 1225.0f, "Canopy01", PROPS_PSYCHONAUT_243 },
    /* 004 */ { REGULAR_TRADE, false, COLOR_P_01_01, 1255.0f, "Canopy01", PROPS_PSYCHONAUT_254 },
    /* 005 */ { REGULAR_TRADE, false, COLOR_P_01_01, 1325.0f, "Canopy01", PROPS_PSYCHONAUT_265 },
    /* 006 */ { REGULAR_TRADE, false, COLOR_P_01_01, 1425.0f, "Canopy01", PROPS_PSYCHONAUT_276 },
    /* 007 */ { REGULAR_TRADE, false, COLOR_P_01_01, 1455.0f, "Canopy01", PROPS_PSYCHONAUT_287 },
    /* 008 */ { REGULAR_TRADE, false, COLOR_P_01_01, 1475.0f, "Canopy01", PROPS_PSYCHONAUT_298 },
    /* 009 */ { REGULAR_TRADE, false, COLOR_P_01_01, 1515.0f, "Canopy01", PROPS_PSYCHONAUT_309 },

    /* 010 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_20, 1300.0f, "Canopy02", PROPS_JAHNCLE_210 },
    /* 011 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_20, 1200.0f, "Canopy02", PROPS_JAHNCLE_190 },
    /* 012 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_20, 1400.0f, "Canopy02", PROPS_JAHNCLE_240 },
    /* 013 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_20, 1500.0f, "Canopy02", PROPS_JAHNCLE_270 },
    /* 014 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_20, 1600.0f, "Canopy02", PROPS_JAHNCLE_290 },
    
    /* 015 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_21,  950.0f, "Canopy03", PROPS_HAIBANE_130 },	
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_21, 1100.0f, "Canopy03", PROPS_HAIBANE_160 },
    /* 017 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_21, 1250.0f, "Canopy03", PROPS_HAIBANE_190 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_21, 1400.0f, "Canopy03", PROPS_HAIBANE_220 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_21, 1550.0f, "Canopy03", PROPS_HAIBANE_250 },

    /* 020 */ { REGULAR_TRADE, false, COLOR_P_01_02, 1050.0f, "Canopy01", PROPS_PSYCHONAUT_210 },
			  { REGULAR_TRADE, false, COLOR_P_01_02, 1100.0f, "Canopy01", PROPS_PSYCHONAUT_221 },
			  { REGULAR_TRADE, false, COLOR_P_01_02, 1150.0f, "Canopy01", PROPS_PSYCHONAUT_232 },
    /* 023 */ { REGULAR_TRADE, false, COLOR_P_01_02, 1225.0f, "Canopy01", PROPS_PSYCHONAUT_243 },
			  { REGULAR_TRADE, false, COLOR_P_01_02, 1255.0f, "Canopy01", PROPS_PSYCHONAUT_254 },
			  { REGULAR_TRADE, false, COLOR_P_01_02, 1325.0f, "Canopy01", PROPS_PSYCHONAUT_265 },
    /* 026 */ { REGULAR_TRADE, false, COLOR_P_01_02, 1425.0f, "Canopy01", PROPS_PSYCHONAUT_276 },
			  { REGULAR_TRADE, false, COLOR_P_01_02, 1455.0f, "Canopy01", PROPS_PSYCHONAUT_287 },
			  { REGULAR_TRADE, false, COLOR_P_01_02, 1475.0f, "Canopy01", PROPS_PSYCHONAUT_298 },
    /* 029 */ { REGULAR_TRADE, false, COLOR_P_01_02, 1515.0f, "Canopy01", PROPS_PSYCHONAUT_309 },

    /* 030 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_22, 1300.0f, "Canopy02", PROPS_JAHNCLE_210 },
			  { REGULAR_TRADE, false, COLOR_P_onecolor_01_22, 1200.0f, "Canopy02", PROPS_JAHNCLE_190 },
			  { REGULAR_TRADE, false, COLOR_P_onecolor_01_22, 1400.0f, "Canopy02", PROPS_JAHNCLE_240 },
			  { REGULAR_TRADE, false, COLOR_P_onecolor_01_22, 1500.0f, "Canopy02", PROPS_JAHNCLE_270 },
			  { REGULAR_TRADE, false, COLOR_P_onecolor_01_22, 1600.0f, "Canopy02", PROPS_JAHNCLE_290 },

    /* 035 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_08,  950.0f, "Canopy03", PROPS_HAIBANE_130 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_08, 1100.0f, "Canopy03", PROPS_HAIBANE_160 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_08, 1250.0f, "Canopy03", PROPS_HAIBANE_190 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_08, 1400.0f, "Canopy03", PROPS_HAIBANE_220 },
    /* 039 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_08, 1550.0f, "Canopy03", PROPS_HAIBANE_250 },

    /* 040 */ { REGULAR_TRADE, false, COLOR_P_01_03, 1050.0f, "Canopy01", PROPS_PSYCHONAUT_210 },
    /* 041 */ { REGULAR_TRADE, false, COLOR_P_01_03, 1100.0f, "Canopy01", PROPS_PSYCHONAUT_221 },
    /* 042 */ { REGULAR_TRADE, false, COLOR_P_01_03, 1150.0f, "Canopy01", PROPS_PSYCHONAUT_232 },
    /* 043 */ { REGULAR_TRADE, false, COLOR_P_01_03, 1225.0f, "Canopy01", PROPS_PSYCHONAUT_243 },
    /* 044 */ { REGULAR_TRADE, false, COLOR_P_01_03, 1255.0f, "Canopy01", PROPS_PSYCHONAUT_254 },
    /* 045 */ { REGULAR_TRADE, false, COLOR_P_01_03, 1325.0f, "Canopy01", PROPS_PSYCHONAUT_265 },
    /* 046 */ { REGULAR_TRADE, false, COLOR_P_01_03, 1425.0f, "Canopy01", PROPS_PSYCHONAUT_276 },
    /* 047 */ { REGULAR_TRADE, false, COLOR_P_01_03, 1455.0f, "Canopy01", PROPS_PSYCHONAUT_287 },
    /* 048 */ { REGULAR_TRADE, false, COLOR_P_01_03, 1475.0f, "Canopy01", PROPS_PSYCHONAUT_298 },
    /* 049 */ { REGULAR_TRADE, false, COLOR_P_01_03, 1515.0f, "Canopy01", PROPS_PSYCHONAUT_309 },

    /* 050 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_18, 1300.0f, "Canopy02", PROPS_JAHNCLE_210 },
    /* 051 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_18, 1200.0f, "Canopy02", PROPS_JAHNCLE_190 },
    /* 052 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_18, 1400.0f, "Canopy02", PROPS_JAHNCLE_240 },
    /* 053 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_18, 1500.0f, "Canopy02", PROPS_JAHNCLE_270 },
    /* 054 */ { REGULAR_TRADE, false, COLOR_P_onecolor_01_18, 1600.0f, "Canopy02", PROPS_JAHNCLE_290 },

    /* 055 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13,  950.0f, "Canopy03", PROPS_HAIBANE_130 },
    /* 056 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13, 1100.0f, "Canopy03", PROPS_HAIBANE_160 },
    /* 057 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13, 1250.0f, "Canopy03", PROPS_HAIBANE_190 },
    /* 058 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13, 1400.0f, "Canopy03", PROPS_HAIBANE_220 },
    /* 059 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13, 1550.0f, "Canopy03", PROPS_HAIBANE_250 },

    /* 060 */ { REGULAR_TRADE, false, COLOR_P_01_05, 1050.0f, "Canopy01", PROPS_PSYCHONAUT_210 },
    /* 061 */ { REGULAR_TRADE, false, COLOR_P_01_05, 1100.0f, "Canopy01", PROPS_PSYCHONAUT_221 },
    /* 062 */ { REGULAR_TRADE, false, COLOR_P_01_05, 1150.0f, "Canopy01", PROPS_PSYCHONAUT_232 },
    /* 063 */ { REGULAR_TRADE, false, COLOR_P_01_05, 1225.0f, "Canopy01", PROPS_PSYCHONAUT_243 },
    /* 064 */ { REGULAR_TRADE, false, COLOR_P_01_05, 1255.0f, "Canopy01", PROPS_PSYCHONAUT_254 },
    /* 065 */ { REGULAR_TRADE, false, COLOR_P_01_05, 1325.0f, "Canopy01", PROPS_PSYCHONAUT_265 },
    /* 066 */ { REGULAR_TRADE, false, COLOR_P_01_05, 1425.0f, "Canopy01", PROPS_PSYCHONAUT_276 },
    /* 067 */ { REGULAR_TRADE, false, COLOR_P_01_05, 1455.0f, "Canopy01", PROPS_PSYCHONAUT_287 },
    /* 068 */ { REGULAR_TRADE, false, COLOR_P_01_05, 1475.0f, "Canopy01", PROPS_PSYCHONAUT_298 },
    /* 069 */ { REGULAR_TRADE, false, COLOR_P_01_05, 1515.0f, "Canopy01", PROPS_PSYCHONAUT_309 },

    /* 070 */ { REGULAR_TRADE, false, COLOR_P_01_06, 1050.0f, "Canopy01", PROPS_PSYCHONAUT_210 },
    /* 071 */ { REGULAR_TRADE, false, COLOR_P_01_06, 1100.0f, "Canopy01", PROPS_PSYCHONAUT_221 },
    /* 072 */ { REGULAR_TRADE, false, COLOR_P_01_06, 1150.0f, "Canopy01", PROPS_PSYCHONAUT_232 },
    /* 073 */ { REGULAR_TRADE, false, COLOR_P_01_06, 1225.0f, "Canopy01", PROPS_PSYCHONAUT_243 },
    /* 074 */ { REGULAR_TRADE, false, COLOR_P_01_06, 1255.0f, "Canopy01", PROPS_PSYCHONAUT_254 },
    /* 075 */ { REGULAR_TRADE, false, COLOR_P_01_06, 1325.0f, "Canopy01", PROPS_PSYCHONAUT_265 },
    /* 076 */ { REGULAR_TRADE, false, COLOR_P_01_06, 1425.0f, "Canopy01", PROPS_PSYCHONAUT_276 },
    /* 077 */ { REGULAR_TRADE, false, COLOR_P_01_06, 1455.0f, "Canopy01", PROPS_PSYCHONAUT_287 },
    /* 078 */ { REGULAR_TRADE, false, COLOR_P_01_06, 1475.0f, "Canopy01", PROPS_PSYCHONAUT_298 },
    /* 079 */ { REGULAR_TRADE, false, COLOR_P_01_06, 1515.0f, "Canopy01", PROPS_PSYCHONAUT_309 },

    /* 080 */ { REGULAR_TRADE, false, COLOR_P_01_07, 1050.0f, "Canopy01", PROPS_PSYCHONAUT_210 },
    /* 081 */ { REGULAR_TRADE, false, COLOR_P_01_07, 1100.0f, "Canopy01", PROPS_PSYCHONAUT_221 },
    /* 082 */ { REGULAR_TRADE, false, COLOR_P_01_07, 1150.0f, "Canopy01", PROPS_PSYCHONAUT_232 },
    /* 083 */ { REGULAR_TRADE, false, COLOR_P_01_07, 1225.0f, "Canopy01", PROPS_PSYCHONAUT_243 },
    /* 084 */ { REGULAR_TRADE, false, COLOR_P_01_07, 1255.0f, "Canopy01", PROPS_PSYCHONAUT_254 },
    /* 085 */ { REGULAR_TRADE, false, COLOR_P_01_07, 1325.0f, "Canopy01", PROPS_PSYCHONAUT_265 },
    /* 086 */ { REGULAR_TRADE, false, COLOR_P_01_07, 1425.0f, "Canopy01", PROPS_PSYCHONAUT_276 },
    /* 087 */ { REGULAR_TRADE, false, COLOR_P_01_07, 1455.0f, "Canopy01", PROPS_PSYCHONAUT_287 },
    /* 088 */ { REGULAR_TRADE, false, COLOR_P_01_07, 1475.0f, "Canopy01", PROPS_PSYCHONAUT_298 },
    /* 089 */ { REGULAR_TRADE, false, COLOR_P_01_07, 1515.0f, "Canopy01", PROPS_PSYCHONAUT_309 },

    /* 090 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_10,  950.0f, "Canopy03", PROPS_HAIBANE_130 },
    /* 091 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_10, 1100.0f, "Canopy03", PROPS_HAIBANE_160 },
    /* 092 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_10, 1250.0f, "Canopy03", PROPS_HAIBANE_190 },
    /* 093 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_10, 1400.0f, "Canopy03", PROPS_HAIBANE_220 },
    /* 094 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_10, 1550.0f, "Canopy03", PROPS_HAIBANE_250 },

    /* 095 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_11,  950.0f, "Canopy03", PROPS_HAIBANE_130 },
    /* 096 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_11, 1100.0f, "Canopy03", PROPS_HAIBANE_160 },
    /* 097 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_11, 1250.0f, "Canopy03", PROPS_HAIBANE_190 },
    /* 098 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_11, 1400.0f, "Canopy03", PROPS_HAIBANE_220 },
    /* 099 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_11, 1550.0f, "Canopy03", PROPS_HAIBANE_250 },

    /* 100 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13,  950.0f, "Canopy03", PROPS_HAIBANE_130 },
    /* 101 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13, 1100.0f, "Canopy03", PROPS_HAIBANE_160 },
    /* 102 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13, 1250.0f, "Canopy03", PROPS_HAIBANE_190 },
    /* 103 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13, 1400.0f, "Canopy03", PROPS_HAIBANE_220 },
    /* 104 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13, 1550.0f, "Canopy03", PROPS_HAIBANE_250 },

    /* 105 */ { REGULAR_TRADE, false, COLOR_FBRedBull, 0.0f, "Canopy06", PROPS_FB_254 },
    /* 106 */ { REGULAR_TRADE, false, COLOR_FBRedBull, 0.0f, "Canopy06", PROPS_FB_265 },    

    /* 107 */ { UTURN_TRADE, false, COLOR_Morpheus_P01_02, 1000.0f, "Canopy04", PROPS_MORPHEUS_210 },
    /* 108 */ { UTURN_TRADE, false, COLOR_Morpheus_P01_02, 1050.0f, "Canopy04", PROPS_MORPHEUS_221 },
    /* 109 */ { UTURN_TRADE, false, COLOR_Morpheus_P01_02, 1100.0f, "Canopy04", PROPS_MORPHEUS_232 },
    /* 110 */ { UTURN_TRADE, false, COLOR_Morpheus_P01_02, 1150.0f, "Canopy04", PROPS_MORPHEUS_243 },
    /* 111 */ { UTURN_TRADE, false, COLOR_Morpheus_P01_02, 1200.0f, "Canopy04", PROPS_MORPHEUS_254 },
    /* 112 */ { UTURN_TRADE, false, COLOR_Morpheus_P01_02, 1250.0f, "Canopy04", PROPS_MORPHEUS_265 },
    /* 113 */ { UTURN_TRADE, false, COLOR_Morpheus_P01_02, 1300.0f, "Canopy04", PROPS_MORPHEUS_276 },
    /* 114 */ { UTURN_TRADE, false, COLOR_Morpheus_P01_02, 1350.0f, "Canopy04", PROPS_MORPHEUS_287 },
    /* 115 */ { UTURN_TRADE, false, COLOR_Morpheus_P01_02, 1400.0f, "Canopy04", PROPS_MORPHEUS_298 },

    /* 116 */ { UTURN_TRADE, false, COLOR_Obsession_03, 1200.0f, "Canopy04", PROPS_OBSESSION_210 },
    /* 117 */ { UTURN_TRADE, false, COLOR_Obsession_03, 1350.0f, "Canopy04", PROPS_OBSESSION_240 },
    /* 118 */ { UTURN_TRADE, false, COLOR_Obsession_03, 1500.0f, "Canopy04", PROPS_OBSESSION_270 },
    /* 119 */ { UTURN_TRADE, false, COLOR_Obsession_03, 1650.0f, "Canopy04", PROPS_OBSESSION_290 },

    /* 120 */ { UTURN_TRADE, true, COLOR_GForce_sky_01, 1300.0f, "Canopy05", PROPS_GFORCE_220 },
    /* 121 */ { UTURN_TRADE, true, COLOR_GForce_sky_01, 1500.0f, "Canopy05", PROPS_GFORCE_250 },

    /* 122 */ { UTURN_TRADE, true, COLOR_Infinity_sky_01, 700.0f, "Canopy05", PROPS_INFINITY_160 },
    /* 123 */ { UTURN_TRADE, true, COLOR_Infinity_sky_01, 900.0f, "Canopy05", PROPS_INFINITY_190 },

    /* 124 */ { true , true, COLOR_FBRedBull, 0.0f, "Canopy06", PROPS_FB_240 },

	/* 125 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_21,  750.0f, "Canopy03", PROPS_HAIBANE_71 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_21,  850.0f, "Canopy03", PROPS_HAIBANE_100 },
			  { REGULAR_TRADE, false, COLOR_P_onecolor_01_20, 1666.0f, "Canopy02", PROPS_JAHNCLE_150 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_08,  750.0f, "Canopy03", PROPS_HAIBANE_71 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_13,  750.0f, "Canopy03", PROPS_HAIBANE_71 },
	/* 130 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13,  850.0f, "Canopy03", PROPS_HAIBANE_100 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_10,  750.0f, "Canopy03", PROPS_HAIBANE_71 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_10,  850.0f, "Canopy03", PROPS_HAIBANE_100 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_11,  750.0f, "Canopy03", PROPS_HAIBANE_71 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_11,  850.0f, "Canopy03", PROPS_HAIBANE_100 },
	/* 135 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_13,  750.0f, "Canopy03", PROPS_HAIBANE_71 },
			  { REGULAR_TRADE, true, COLOR_P_onecolor_01_13,  850.0f, "Canopy03", PROPS_HAIBANE_100 },

	/* 137 */ { REGULAR_TRADE, true, COLOR_Main_petra_69,     699.0f, "Canopy03", PROPS_PETRA_69 },
	/* 138 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_08, 2000.0f, "Canopy03", PROPS_EFREET_49 },

    { 0, 0, COLOR_DISABLED, 0.0f, 0 }
};

unsigned int Canopy::getNumRecords(void)
{
    unsigned int result = 0;
    unsigned int i = 0;
    while( canopies[i].nameId != 0 ) i++, result++;
    return result;
}

Canopy* Canopy::getRecord(unsigned int id)
{
    assert( id >= 0 && id < getNumRecords() );
	Canopy *canopy = canopies + id;
	
    return canopy;
}

float Canopy::getOptimalCanopySquare(float weight, float wind)
{
    // optimal square for calm weather
    float optimalSquare = 100.0f + weight / 0.4535f;

    // wind correction   
    if( wind >= 9.0f ) optimalSquare -= 20.0f;
    else if( wind >= 6.0f ) optimalSquare -= 15.0f;
    else if( wind >= 3.0f ) optimalSquare -= 10.0f;

    return optimalSquare;
}

float Canopy::getWingLoading(unsigned int id, float weight)
{
    assert( id >= 0 && id < getNumRecords() );
    return ( weight / 0.45f ) / getRecord( id )->square;
}