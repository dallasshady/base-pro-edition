
#include "headers.h"
#include "database.h"

using namespace database;

// default canopy pack
#define REGULAR_TRADE true

#define MODELID_A1_193 891
#define MODELID_A1_218 859
#define MODELID_A1_235 860
#define MODELID_A1_253 861

#define DESCRIPTIONID_A1 862

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

// inflation dynamics
//				SRDminvel, SRDmink, SRDmaxvel, SRDmaxk, SUminvel, SUmink, SUmaxvel, SUmaxk

#define INFL_A1 5.0f,      0.0f,    20.0f,     1.0f,    0.0f,     0.250f, 50.0f,    0.0625f

// collapse dynamics
//							Cnum, Cradius, Cpower,   Cminvel, Cmaxvel, Crestore, Cresvel

#define COLLAPSE_A1			   4,    2.5f,  0.5f,	   0.25f,  12.0f,      0.5f, 9.0f

// ageing dynamics
// Kminoverb, Kmindamage, Kmaxoverb, Kmaxdamage, Kminfrict, Kminrip, Kmaxfrict, Kmaxrip

#define AGEING_A1    3.0f, 0.0f, 12.0f, 1.0f/140.f, 100.0f, 0.0f, 1200.0f, 1.0f/60.0f

// travel of attack angle 
#define AA_A1    14.5f

/**
 * pilotchute options
 */

const unsigned int numReservePilots = 1;

static database::Pilotchute reservePilots[numReservePilots] = 
{
    //      size,  mass,   scale,   Vrec, Talign, Tgyres, Fair
    /* 0 */ { 38, 0.19f, 0.9652f, 42.24f, 1.6f,  0.42f, 2.0f }
};

/**
 * properties
 */

//						  nameId			   descriptionId		  manufId   mass   scale        area  aspect ftCord rearCord ctrCord cascade				   Kyair     Kzair     Kxair AAdeep          Klifts   Kliftd  Kdrags  Kdragd Kbraking  Kturn  Kdampmin max    Vdampmax
#define PROPS_A1_193	MODELID_A1_193,			DESCRIPTIONID_A1,	  MFRID_D4, 1.679f, 3.878f, 193.000f, 2.100f, 2.54537f, 2.800f, 2.800f, 1.500f, &riserScheme01,  11.478f, 0.940f, 25.582f, AA_A1 - 2.0f,        8.590f, 19.623f, 0.785f, 5.196f, 0.054f, 0.006f, 3.480f, 3.969f,  8.365f, INFL_A1, COLLAPSE_A1, AGEING_A1, numReservePilots, reservePilots
#define PROPS_A1_218	MODELID_A1_218,			DESCRIPTIONID_A1,	  MFRID_D4, 2.201f, 4.381f, 218.000f, 2.100f, 3.09700f, 3.400f, 3.400f, 1.500f, &riserScheme01,  15.047f, 1.233f, 33.535f, AA_A1,		11.261f, 25.723f, 1.028f, 6.811f, 0.120f, 0.008f, 4.562f, 5.202f, 10.965f, INFL_A1, COLLAPSE_A1, AGEING_A1, numReservePilots, reservePilots
#define PROPS_A1_235	MODELID_A1_235,			DESCRIPTIONID_A1,     MFRID_D4, 2.550f, 4.722f, 235.000f, 2.100f, 3.09700f, 3.400f, 3.400f, 1.500f, &riserScheme01,  17.460f, 1.430f, 38.919f, AA_A1,		13.069f, 29.853f, 1.194f, 7.905f, 0.139f, 0.009f, 5.294f, 6.038f, 12.725f, INFL_A1, COLLAPSE_A1, AGEING_A1, numReservePilots, reservePilots
#define PROPS_A1_253	MODELID_A1_253,			DESCRIPTIONID_A1,     MFRID_D4, 2.750f, 5.084f, 253.000f, 2.100f, 3.09700f, 3.400f, 3.400f, 1.500f, &riserScheme01,  18.800f, 1.540f, 41.900f, AA_A1,		14.070f, 32.140f, 1.285f, 8.510f, 0.150f, 0.010f, 5.700f, 6.500f, 13.700f, INFL_A1, COLLAPSE_A1, AGEING_A1, numReservePilots, reservePilots
// angle = 1.581 (for 218-253 sizes)
static Canopy reserves[] = 
{
    // skydiving, color, cost, templateName, props_macro

    /* 000 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_10, 1495.0f, "Canopy04", PROPS_A1_253 },
	/* 001 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_10, 1495.0f, "Canopy04", PROPS_A1_235 },
    /* 002 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_10, 1495.0f, "Canopy04", PROPS_A1_218 },
	/* 003 */ { REGULAR_TRADE, true, COLOR_P_onecolor_01_10, 1495.0f, "Canopy04", PROPS_A1_193 },

    { 0, 0, COLOR_DISABLED, 0.0f, 0 }
};

unsigned int Canopy::getReserveNumRecords(void)
{
    unsigned int result = 0;
    unsigned int i = 0;
    while( reserves[i].nameId != 0 ) i++, result++;
    return result;
}

Canopy* Canopy::getReserveRecord(unsigned int id)
{
    assert( id >= 0 && id < getReserveNumRecords() );
    return reserves + id;
}

float Canopy::getReserveWingLoading(unsigned int id, float weight)
{
    assert( id >= 0 && id < getReserveNumRecords() );
    return ( weight / 0.45f ) / getReserveRecord( id )->square;
}