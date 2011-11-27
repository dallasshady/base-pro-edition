
#ifndef INTERNAL_DATABASE_INCLUDED
#define INTERNAL_DATABASE_INCLUDED

#include "../shared/engine.h"
#include "../shared/audio.h"
#include "../shared/gui.h"

class Actor;
class Career;

/**
 * (outside database) - types of weather
 */

enum WeatherType
{
    wtSunny,
    wtVariable,
    wtCloudy,
    wtLightRain,
    wtHardRain,
    wtThunder,
    wtDatabaseEnding // used for database enumeration of weather options
};

namespace database
{

/**
 * wind constants
 */

const float windCalmAmbientMin = 0.0f;
const float windCalmAmbientMax = 0.5f;
const float windCalmBlastMin = 0.5f;
const float windCalmBlastMax = 1.0f;

const float windLightAmbientMin = 1.0f;
const float windLightAmbientMax = 3.0f;
const float windLightBlastMin = 3.0f;
const float windLightBlastMax = 6.0f;

const float windMediumAmbientMin = 3.0f;
const float windMediumAmbientMax = 6.0f;
const float windMediumBlastMin   = 6.0f;
const float windMediumBlastMax   = 9.0f;

const float windStrongAmbientMin = 6.0f;
const float windStrongAmbientMax = 9.0f;
const float windStrongBlastMin   = 9.0f;
const float windStrongBlastMax   = 12.0f;

/**
 * textures
 */

struct GearTexture
{
public:
    const char* textureName;
    const char* resourceName;
public:
    static unsigned int getNumRecords(void);
    static GearTexture* getRecord(unsigned int id);
};

/**
 * color constatns (language resources)
 */

#define CLID_DARK_BLUE    156
#define CLID_PURPLE       157
#define CLID_KHAKI        158
#define CLID_LILAC        159
#define CLID_YELLOW       160
#define CLID_MAGENTA      161
#define CLID_LIGHT_BLUE   162
#define CLID_LIGHT_GREEN  163
#define CLID_ORANGE       164
#define CLID_WHITE        165
#define CLID_CRIMSON      166
#define CLID_LIGHT_VIOLET 167
#define CLID_VIOLET       168
#define CLID_DARK_GREEN   169
#define CLID_BLACK        230

/**
 * manufacturer constatns (language resources)
 */

#define MFRID_D3    171
#define MFRID_D4    863
#define MFRID_UTURN 735

/**
 * face database
 */

struct Face
{
public:
    unsigned int descriptionId; // id of description text in language module
    unsigned int modelId;       // 3d-model identifier, see Jumper::getHead()
    const char*  iconName;      // texture for gui usage (preloaded!)
    const char*  resourceName;  // path to resource, contained this texture
    const char*  textureName;   // texture for head & hands    
public:
    static unsigned int getNumRecords(void);
    static Face* getRecord(unsigned int id);
};

/**
 * (gear) helmet database
 */

struct Helmet
{
public:
    float        cost;             // cost for fine model
    float        Kda;              // coefficient of damage absorbtion
    float        Kage;             // coefficient of ageing (damage - to - state reduction)
    unsigned int modelId;          // 3d model identifier
    unsigned int nameId;           // (language id) model name
    unsigned int descriptionId;    // (language id) model description
    unsigned int colorId;          // (language id) color name
    unsigned int manufacturerId;   // (language id) manufacturer name
    unsigned int textureId;        // GearTexture record id    
public:
    static unsigned int getNumRecords(void);
    static Helmet* getRecord(unsigned int id);
};

/**
 * (gear) suit database
 */

struct Suit
{
public:
    bool         trade;               // suit is tradeable
    float        cost;                // suit cost
    bool         wingsuit;            // suit have wings
    unsigned int modelId;             // model identified, see Jumper::getBody()
    unsigned int nameId;              // (language id) model name    
    unsigned int descriptionId;       // (language id) model description
    unsigned int colorId;             // (language id) color name
    unsigned int manufacturerId;      // (language id) manufacturer name
    unsigned int textureId;           // GearTexture record id
    float        kAlign;              // horizontal velosity aligment coefficient
    float        mHeadDown;           // multiplier for air resistance in headdown pose 
    float        mFrog;               // multiplier for air resistance in frog pose 
    float        mTracking;           // multiplier for air resistance in tracking pose 
    float        mFrogGlideRatio;     // multiplier for glide ratio in frog pose
    float        mTrackingGlideRatio; // multiplier for glide ratio in tracking pose
    float        mFrogGlideForce;     // multiplier for glide force in frog pose
    float        mTrackingGlideForce; // multiplier for glide force in tracking pose
    float        Kage;                // coefficient of ageing (damage - to - state reduction)
public:
    static unsigned int getNumRecords(void);    
    static Suit* getRecord(unsigned int id);
};

/**
 * (gear) AAD database
 */
struct AAD {
public:
	float altitude;			// activation altitude
	float speed;			// activation speed
	unsigned int descriptionId;  // (language id) 
public:
    static unsigned int getNumRecords(void);
    static AAD* getRecord(unsigned int id);
};
/**
 * (gear) rig database
 */
struct Rig
{
public:
    bool         trade;          // rig is tradeable
    float        cost;           // cost of the rig
    bool         skydiving;      // true if rig for skydiving only
    unsigned int nameId;         // (language id) model name
    unsigned int descriptionId;  // (language id) model description
    unsigned int colorId;        // (language id) color name
    unsigned int manufacturerId; // (language id) manufacturer name
    unsigned int textureId;      // texture name
    float        Kab;            // coefficient of overburden absorbtion 
    float        Kminoverb;      // minimal ageing overburden
    float        Kminage;        // miniam ageing (per second)
    float        Kmaxoverb;      // maximal ageing overburden
    float        Kmaxage;        // maximal ageing (per second)    
public:
    static unsigned int getNumRecords(void);
    static Rig* getRecord(unsigned int id);
};

/**
 * (gear) canopies & pilotchutes
 */

/**
 * PAB type gives the description for general behaviour
 * there are certain controller scheme for each PAB type (see CanopySimulator)
 */

enum PABType
{
    pabtLeftFlap,     // left flap bone
    pabtRightFlap,    // right flap bone
    pabtFrontSection, // front semi-section bone
    pabtRearSection   // rear semi-section bone
};

enum LineOverSide
{
    losIndifferent, // doesn't affected fy lineover effect
    losLeft,        // left lineover effect
    losRight        // right lineover effect
};

/**
 * PAB descriptor
 */

struct PABDesc
{
public:        
    const char*  pabName;   // bone name
    PABType      pabType;   // type (see before)
    Vector3f     pabAxis;   // rotation axis
    float        pabLimit;  // rotation limit (degrees)
    float        pabSpeed;  // rotation speed (degrees)    
    const char*  cstName;   // constrained bone name (in condition of same axis!)
    float        cstWeight; // weight of affect of constrained bone animation
    LineOverSide loSide;    // side of lineover effect
    float        loWeight;  // weight of lineover effect (0..1)
};

class RiserScheme
{
public:
    enum RiserType
    {
        rtOuterFrontLeft,
        rtOuterRearLeft,
        rtOuterFrontRight,
        rtOuterRearRight,
        rtInnerFrontLeft,
        rtInnerRearLeft,
        rtInnerFrontRight,
        rtInnerRearRight,
        rtBrakeLeft,
        rtBrakeRight
    };
public:
    // number of cords per riser (4 risers gives 4x cords)
    virtual unsigned int getNumCords(void) = 0;
    virtual unsigned int getNumBrakes(void) = 0;
    // joint (frame) name, actual for a canopy 3d-model
    virtual const char* getJointName(RiserType riserType, unsigned int cordId) = 0;
    virtual const char* getLeftLineoverJointName(void) = 0;
    virtual const char* getRightLineoverJointName(void) = 0;
    // PABs
    virtual unsigned int getNumPABs(void) = 0;
    virtual PABDesc* getPAB(unsigned int pabId) = 0;
};

struct Pilotchute
{
public:
    float size;   // pilotchute size (in inches)
    float mass;   // pilotchute mass    
    float scale;  // pilotchute model scale
    float Vrec;   // velocity, recommended for dropping
    float Talign; // align torque coefficient
    float Tgyres; // gyration resistance coefficient
    float Fair;   // air resistance coefficient
};

struct Canopy
{
public:
    bool         trade;          // flag for trading activity
    bool         skydiving;      // flag for skydiving gear
    gui::Rect    color;          // color preview
    unsigned int textureId;      // texture id, @see GearTexture database
    float        cost;           // cost
    const char*  templateName;   // canopy 3d model template name    
public:
    unsigned int nameId;         // (language id) model name
    unsigned int descriptionId;  // (language id) model description
    unsigned int manufacturerId; // (language id) manufacturer name
    float        mass;           // canopy mass
    float        scale;          // canopy model scale
    float        square;         // canopy square
    float        aspect;         // canopy aspect    
    float        frontCord;      // front cord length
    float        rearCord;       // rear cord length
    float        centralCord;    // central cord length (auxially joint)
    float        cascade;        // distance from riser to cascade point
    RiserScheme* riserScheme;    // canopy riser scheme    
public:
    float        Kyair;    // top air resistance coefficient,  
    float        Kzair;    // front air resistance coefficient
    float        Kxair;    // side air resistance coefficient
    float        AAdeep;   // attack angle modifier for deep brakes
    float        Klifts;   // lift force for shallow brakes
    float        Kliftd;   // lift force for deep brakes
    float        Kdrags;   // drag force for shallow brakes
    float        Kdragd;   // drag force for deep brakes
    float        Kbraking; // (control) braking force coefficient 
    float        Kturn;    // (control) turn force coefficient 
    float        Kdampmin; // minimal angular damping (velocity magn. is 0.0f)
    float        Kdampmax; // maximal angular damping (velocity magn. is Vdampmax)
    float        Vdampmax; // reference velocity for angular damping
public:
    float        SRDminvel; // slider removed/down: minimal reference velocity
    float        SRDmink;   // slider removed/down: minimal opening koeff
    float        SRDmaxvel; // slider removed/down: maximal reference velocity;
    float        SRDmaxk;   // slider removed/down: maximal opening koeff
public:
    float        SUminvel; // slider up: minimal reference velocity
    float        SUmink;   // slider up: minimal opening koeff
    float        SUmaxvel; // slider up: maximal reference velocity;
    float        SUmaxk;   // slider up: maximal opening koeff
public:
    unsigned int Cnum;     // number of supported collapse areas
    float        Cradius;  // maximal radius of collapse area
    float        Cpower;   // reference power of collapse (linearly actual for Cradius)
    float        Cminvel;  // minimal reference velocity for Cradius
    float        Cmaxvel;  // maximal reference velocity for Cradius
    float        Crestore; // restoration speed (speed of collapse radius decreasing)
    float        Cresvel;  // reference velocity for Crestore
public:
    float        Kminoverb;  // minimal overburden
    float        Kmindamage; // minimal damage
    float        Kmaxoverb;  // maximal overburden
    float        Kmaxdamage; // maximal damage
    float        Kminfrict;  // minimal friction force
    float        Kminrip;    // minimal ripping damage
    float        Kmaxfrict;  // maximal friction force
    float        Kmaxrip;    // minimal ripping damage
public:
    unsigned int numPilots; // number of avaiable pilotchute options
    Pilotchute*  pilots;    // available pilotchute options
public:
    static unsigned int getNumRecords(void);
	static unsigned int getReserveNumRecords(void);
    static Canopy* getRecord(unsigned int id);
	static Canopy* getReserveRecord(unsigned int id);
    static float getOptimalCanopySquare(float weight, float wind);
    static float getWingLoading(unsigned int id, float weight);
	static float getReserveWingLoading(unsigned int id, float weight);
};

/**
 * event database
 */

struct EventInfo
{
public:
    unsigned int shortId;       // id of short description text
    unsigned int descriptionId; // id of full description text
    const char*  textureName;   // button texture name
    const char*  resourceName;  // splash image texture resource
public:
    static unsigned int getNumRecords(void);
    static EventInfo* getRecord(unsigned int id);
};

/**
 * location database
 */

typedef void (*CastingCallback)(Actor* parent);

struct LocationInfo
{
public:
    bool          accessible;  // location is accessible
    unsigned int  nameId;      // id of text, naming the location
    int           worldX;      // "longitude"
    int           worldY;      // "lattitude"
    const char*   gameData;    // name of entry in game data pool (serialization)    
    const char*   thumbnail;   // thumbnail resource (NULL if no)
    bool          wind;        // wind flag (false for indoor & cave locations)
    float         stayFee;     // "stay-in-location" fee (collected by Divine event)
    unsigned int  boogieId;    // (language) id of boogie event (0 for no boogies)
    unsigned int  festivalId;  // (language) id of festival event (0 for no festivals)
    unsigned int  climbingId;  // (language) id of climbing event (0 for no climbings)
    unsigned int  smokeballId; // (language) id of smokeball event (0 for no smokeballs)
    unsigned int  communityId; // (language) id of user community event (0 for no events)
public:
    struct PhysicsLimits
    {
    public:
        unsigned int numActors;
        unsigned int numBodies;
        unsigned int numStaticShapes;
        unsigned int numDynamicShapes;
        unsigned int numJoints;
    }
    physicsLimits;
public:
    struct RegularAsset
    {
    public:
        const char* resource;
        float       zNear;
        float       zFar;
    }
    stage, extras;
public:
    struct AssetInfo
    {
    public:
        const char* name;
        const char* resource;
    }* localAssets;
public:
    const char** localTextures;
public:
    struct AfterFx
    {
    public:
        float brightPass;
        float bloom;
    }
    afterFx;
public:
    struct Grass
    {
    public:
        engine::GrassScheme* scheme;
        const char*          textureName;
        const char*          textureResource;
        const char*          cache;
        const char*          templ;
        float                fadeStart;
        float                fadeEnd;
    }
    grass;
public:
    struct ExitPoint
    {
    public:
        unsigned int nameId;
        const char*  extras;
        float        delay;  // reference delay (in seconds)
    }
    *exitPoints;
public:
    CastingCallback castingCallback;
public:
    struct Footsteps
    {
    public:
        float refdist;
        float maxdist;
        float rolloff;
        float walkPitch;
        float backPitch;
        float turnPitch;
        const char* walkSound;
        const char* turnSound;
    } 
    footsteps;
public:
    struct Weather
    {
    public:
        WeatherType     weather;      // weather option
        engine::FogType fogType;      // fog type for this weather (Exp or Exp2 only)
        float           fogDensity;   // fog density
        Vector4f        fogColor;     // fog color
        float           sunMute;      // sun light muting factor (0..1)
        float           ambientMute;  // ambient light muting factor (0..1)
        RegularAsset    panorama;     // subj
    }
    *weathers;
public:
    struct Reverberation
    {
        float inGain;
        float reverbMixDB;
        float reverbTime;
        float hfTimeRatio;
    }
    *reverberation;
public:
    static unsigned int getNumRecords(void);
    static LocationInfo* getRecord(unsigned int id);
};

/**
 * mission database
 */

struct MissionInfo;

typedef bool (*WeatherClearance)(WeatherType weatherType);
typedef bool (*WindClearance)(Vector3f dir, float ambient, float blast);
typedef bool (*EquipCallback)(Career* career, float windAmbient, float windBlast, database::MissionInfo* missionInfo);

#define AIRPLANE_EXIT 0xFFFFFFFF /* jump off from airplane instead of exit point */

enum MissionFlag
{
    mfForcedEquipment = 0x00000001, // mission provides equipment for player by itself
    mfEnumSize32bits  = 0xFFFFFFFF
};

struct MissionInfo
{
public:
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
    TiXmlNode*       missionNode;      // mission node (user missions only)
};

enum TournamentType
{
    tfArbitrary, // tournament is arbitrary (can be played when no events)
    tfBoogie,    // boogie tournament (can be played with boogie event)
    tfFestival,  // festival tournament (can be played with festival event)
    tfClimbing,  // climbing tournament (can be played with climbing event)
    tfSmokeball, // smokeball tournament (can be played with smokeball event)
    tfUser       // user-maked tournament
};

struct TournamentInfo
{
public:
    unsigned int   nameId;     // (language) tournament name id
    unsigned int   briefId;    // (language) tournament briefing text id
    unsigned int   locationId; // tournament location
    TournamentType type;       // tournament flags
    const char*    gameData;   // name of gamedata record
    const char*    thumbnail;  // thumbnail resource
    MissionInfo*   missions;   // tournament missions
public:
    unsigned int getNumMissions(void);
    unsigned int getMinimalRank(void);
public:
    static unsigned int getNumRecords(void);
    static unsigned int getDemoTournament(void);
    static TournamentInfo* getRecord(unsigned int id);
    static void addRecord(TournamentInfo* tournamentInfo);
};

/**
 * npc database
 */

struct NPCInfo
{
public:
    unsigned int nameId;     // (language) npc name
    float        level;      // npc level (0..1), match the rank as 1:10
    float        height;     // corresponding virtue: height
    float        weight;     // corresponding virtue: weight
    unsigned int face;       // corresponding virtue: face id
    float        perception; // corresponding skill: perception
    float        tracking;   // corresponding skill: tracking
    float        endurance;  // corresponding skill: endurance
    float        rigging;    // corresponding skill: rigging
public:
    static unsigned int getNumRecords(void);
    static NPCInfo* getRecord(unsigned int id);
    static unsigned int getLicensedCharacterId(void);
    static unsigned int getRandomNonLicensedCharacter(float level, float epsilon);
    static void select(float level, float epsilon, bool licensed, std::vector<unsigned int>& result);
    static void addRecord(NPCInfo* npc);
};

}

#endif