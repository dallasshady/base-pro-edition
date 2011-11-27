
#ifndef SCENE_ACTIVITY_INCLUDED
#define SCENE_ACTIVITY_INCLUDED

#include "headers.h"
#include "activity.h"
#include "career.h"
#include "sensor.h"
#include "gameplay.h"
#include "geoscape.h"
#include "database.h"
#include "render.h"

class Scene;
class Actor;

/**
 * events are happen in the scene bacause of activity of actors
 */

#define EVENT_RESERVED_RANGE_BEGIN 0x0000
#define EVENT_RESERVED_RANGE_END   0x0DFF

#define EVENT_SCENE_REVERBERATION_IS_CHANGED 0x0E00

#define EVENT_CAMERA_IS_ACTUAL 0x0F00

#define EVENT_JUMPER_BEGIN_WALKFWD     0x1000
#define EVENT_JUMPER_END_WALKFWD       0x1001
#define EVENT_JUMPER_BEGIN_WALKBCK     0x1002
#define EVENT_JUMPER_END_WALKBCK       0x1003
#define EVENT_JUMPER_BEGIN_TURN        0x1004
#define EVENT_JUMPER_END_TURN          0x1005
#define EVENT_JUMPER_PITCHSHIFT        0x1006
#define EVENT_JUMPER_FIRSTPERSON       0x1007
#define EVENT_JUMPER_THIRDPERSON       0x1008
#define EVENT_JUMPER_BEGIN_FREEFALL    0x1009
#define EVENT_JUMPER_END_FREEFALL      0x100A
#define EVENT_JUMPER_FREEFALL_VELOCITY 0x100B
#define EVENT_JUMPER_FREEFALL_MODIFIER 0x100C
#define EVENT_JUMPER_ENUMERATE         0x100D

#define EVENT_CANOPY_OPEN      0x1100
#define EVENT_CANOPY_VELOCITY  0x1101
#define EVENT_CANOPY_COLLAPSED 0x1102

#define EVENT_DELAY_START 0x1200
#define EVENT_DELAY_STOP  0x1201

#define EVENT_GOAL_ENUMERATE 0x1300

#define EVENT_HUD_HIDE 0x1400

#define EVENT_EASY_CRY   0x1500
#define EVENT_MEDIUM_CRY 0x1501
#define EVENT_HARD_CRY   0x1502

#define EVENT_FOREST_ENUMERATE 0x1600

/**
 * actors are active (and interactive) objects inside the scene
 */

const Matrix4f defaultPose( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 );
const Vector3f defaultVel( 0,0,0 );

class Actor;

typedef std::vector<Actor*> ActorV;
typedef ActorV::iterator ActorI;

class Actor
{   
protected:
    std::string _name;     // actor name    
    Scene*      _scene;    // scene
    Actor*      _parent;   // parent actor
    ActorV      _children; // team of children actors
public:
    // actor abstracts
    virtual void onUpdateActivity(float dt) {}
    virtual void onUpdatePhysics(void) {}
    virtual void onContact(NxContactPair &pair, NxU32 events) {}
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData) {}
    virtual Matrix4f getPose(void) { return defaultPose; }
    virtual Vector3f getVel(void) { return defaultVel; }
public:
    // class implementation
    Actor(Scene* scene);
    Actor(Actor* parent);
    virtual ~Actor();
public:
    // specific behaviour
    void happen(Actor* initiator, unsigned int eventId, void* eventData = NULL);
    void updateActivity(float dt);
    void updatePhysics(void);
public:
    // actor common : inlines
    inline Scene* getScene(void) { return _scene; }
    inline Actor* getParent(void) { return _parent; }
    inline const char* getName(void) { return _name.c_str(); }
};

/**
 * mode is stackable actor
 */

class Mode : public Actor
{
public:
    // mode abstracts
    virtual void onSuspend(void) = 0;
    virtual void onResume(void) = 0;
    virtual bool endOfMode(void) = 0;
public:
    Mode(Scene* scene);
    virtual ~Mode();
};

/**
 * scene is action that can interact with player
 */

#define FRAMETIME(F) ( float(F)*3.0f/100.0f )

const float simulationStepTime = 1.0f/100.0f;
const float maxHoldingTime = HOURS_TO_MINUTES(12);

class Scene : public Activity,
              virtual public NxUserContactReport,
              virtual public NxUserTriggerReport,
              virtual public RenderSource
{
protected:
    typedef std::pair<database::LocationInfo::ExitPoint*,Enclosure*> EnclosureT;
    typedef std::map<database::LocationInfo::ExitPoint*,Enclosure*> EnclosureM;
    typedef EnclosureM::iterator EnclosureI;
    typedef std::list<engine::IAsset*> AssetL;
    typedef AssetL::iterator AssetI;
    typedef std::list<engine::ITexture*> TextureL;
    typedef TextureL::iterator TextureI;
    typedef std::list<engine::IParticleSystem*> ParticleSystemL;
    typedef ParticleSystemL::iterator ParticleSystemI;
    typedef std::list<engine::IRendering*> SmokeTrailL;
    typedef SmokeTrailL::iterator SmokeTrailI;
protected:
    bool                _isLoaded;
    bool                _endOfActivity;
    Career*             _career;
    Location*           _location;    
    float               _holdingTime;
    float               _passedTime;
    engine::IAsset*     _panoramaAsset; // panorama
    engine::IAsset*     _stageAsset;    // stage 
    engine::IAsset*     _extrasAsset;   // extras (game data)
    AssetL              _localAssets;   // other local assets (for separate objects)
    TextureL            _localTextures;
    engine::IBSP*       _panorama;
    engine::IBSP*       _stage;
    engine::ITexture*   _grassTexture;
    engine::IRendering* _grass;
    engine::ITexture*   _rainTexture;
    engine::IRendering* _rain;
    engine::IAtomic*    _collisionGeometry;    
    float               _panoramaNearClip;
    float               _panoramaFarClip;
    float               _stageNearClip;
    float               _stageFarClip;
    float               _brightPass;
    float               _bloom;
    float               _timeSpeed;
    float               _timeSpeedMultiplier;
    float               _switchHUDTimeout;  // timeout for switch HUD action
    bool                _isHUDEnabled;      // HUD flag
    float               _windTime;          // wind simulation time
    Actor*              _scenery;           // scenery co-ordinator, it holds common actors
    Actor*              _camera;            // current camera actor
    Matrix4f            _lastCameraPose;    // subj
    Mode*               _modeQuery;         // mode query (avaiable once per act cycle)
    std::stack<Mode*>   _modes;             // stack of modes
    EnclosureM          _enclosures;        // various enclosures inside scene
    ParticleSystemL     _particleSystems;   // particle emitters
    SmokeTrailL         _smokeTrails;
    Sensor*             _clipRay;
private:
    database::LocationInfo*                _locationInfo;    // subj.
    database::LocationInfo::Weather*       _locationWeather; // graphics weather options;
    database::LocationInfo::Reverberation* _reverberation;   // current reverberation
private:
    gui::IGuiWindow*    _loadingWindow;
private:    
    NxBounds3           _phSceneBounds;     // NX scene bounds
    NxSceneLimits       _phSceneLimits;     // NX scene limits
    NxSceneDesc         _phSceneDesc;       // NX scene descriptor
    NxScene*            _phScene;           // NX scene
    NxVec3*             _phTerrainVerts;
    NxU32*              _phTerrainTriangles;
    NxMaterialIndex*    _phTerrainMaterials;
    NxTriangleMeshDesc      _phTerrainDesc;      // NX terrain descriptor
    NxTriangleMeshShapeDesc _phTerrainShapeDesc; // NX terrain shape descrpitor
    NxActorDesc             _phTerrainActorDesc; // NX descriptor for terrain actor
    NxActor*                _phTerrain;          // NX terrain
    NxTriangleMeshShape*    _phTerrainShape;     // NX terrain shape
    NxMaterial*             _phFleshMaterial;       // generic flesh material
    NxMaterial*             _phMovingFleshMaterial; // moving flesh material (like feets motion)
    NxMaterial*             _phClothMaterial;       // cloth material
public:
    // engine progress callback
    static void progressCallback(const wchar_t* description, float progress, void* userData);
    static void postRenderCallback(void* data);
private:
    // decomposition of class behaviour
    void load(void);
    void initializePhysics(void);
public:
    // physics handlers
    virtual void onContactNotify(NxContactPair &pair, NxU32 events);
    virtual void onTrigger(NxShape& triggerShape, NxShape& otherShape, NxTriggerFlag status);
public:
    // class implementation
    Scene(Career* career, Location* location, float holdingTime);
    virtual ~Scene();
    // Activity
    virtual void updateActivity(float dt);
    virtual bool endOfActivity(void);
    virtual void onBecomeActive(void);
    virtual void onBecomeInactive(void);
    // RenderSource
    virtual Vector4f getClearColor(void);
    virtual float getBlur(void);
    virtual float getBrightPass(void);
    virtual float getBloom(void);
    virtual unsigned int getNumPasses(void);
    virtual float getPassNearClip(unsigned int passId);
    virtual float getPassFarClip(unsigned int passId);
    virtual unsigned int getPassClearFlag(unsigned int passId);
    virtual void renderPass(unsigned int passId);
    virtual void renderLensFlares(void);
    // class complicated behaviour
    void endOfScene(void);
    float getHoldingTime(void);
    float getPassedTime(void);
    void passHoldingTime(float time);
    void queryMode(Mode* mode);
    void setCamera(Actor* camera);
    engine::IClump* getExtras(const char* extrasName);
    unsigned int getNumExitPoints(void);
    Enclosure* getExitPointEnclosure(unsigned int exitId);
    const wchar_t* getExitPointName(unsigned int exitId);
    engine::IClump* findClump(const char* clumpName);
    float getTimeSpeed(void);
    void setTimeSpeed(float value);
    void setTimeSpeedMultiplier(float value);
    audio::ISound* createWalkSound(void);
    audio::ISound* createTurnSound(void);
    float getWalkPitch(void);
    float getBackPitch(void);
    float getTurnPitch(void);
    NxVec3 getWindAtPoint(const NxVec3& point);
    void addParticleSystem(engine::IParticleSystem* psys);
    void removeParticleSystem(engine::IParticleSystem* psys);
    void addSmokeTrail(engine::IRendering* smokeTrail);
    void removeSmokeTrail(engine::IRendering* smokeTrail);
    bool clipCameraRay(const Vector3f& targetPos, const Vector3f& cameraPos, float& clipDistance);
public:
    // inlinez
    inline Career* getCareer(void) { return _career; }
    inline Location* getLocation(void) { return _location; }
    inline Actor* getCamera(void) { return _camera; }
    inline Actor* getScenery(void) { return _scenery; }
    inline Mode* getTopMode(void) { return _modes.size() ? _modes.top() : NULL; }
    inline engine::IBSP* getPanorama(void) { return _panorama; }
    inline engine::IBSP* getStage(void) { return _stage; }
    inline engine::IRendering* getGrass(void) { return _grass; }    
    inline NxScene* getPhScene(void) { return _phScene; }
    inline NxActor* getPhTerrain(void) { return _phTerrain; }
    inline NxTriangleMeshShape* getPhTerrainShape(void) { return _phTerrainShape; }
    inline NxMaterial* getPhFleshMaterial(void) { return _phFleshMaterial; }
    inline NxMaterial* getPhMovingFleshMaterial(void) { return _phMovingFleshMaterial; }
    inline NxMaterial* getPhClothMaterial(void) { return _phClothMaterial; }
    inline bool isHUDEnabled(void) { return _isHUDEnabled; }
    inline database::LocationInfo* getLocationInfo(void) { return _locationInfo; }
    inline database::LocationInfo::Weather* getLocationWeather(void) { return _locationWeather; }
    inline database::LocationInfo::Reverberation* getReverberation(void) { return _reverberation; }
};

#endif