
/**
 * actor of uniform (mono-specie) forest 
 * it is need to use several forest actors in order to create non-uniform forest
 */

#ifndef FOREST_IMPLEMENTATION_INCLUDED
#define FOREST_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "scene.h"
#include "../shared/audio.h"

struct ForestDesc
{
public:
    std::string      cache;            // path to cache file
    engine::IClump*  layers;           // additional LOD layers (optional)
    engine::IAtomic* surface;          // surface
    std::string      assetName;        // path to asset contained 3d-models
    float            minScale;         // minimal scale of trees
    float            maxScale;         // maximal scale of trees\    
    float            collScale;        // scale of collision geometry (AABB)
    float            density;          // density of forest
    float            entangleFactor;   // min canopy volume caused entangle
    float            minImpulseFactor; // min impulse factor for jumper body
    float            maxImpulseFactor; // max impulse factor for jumper body
    float            ripFactor;        // ripping force acting on canopy
    float            damageFactor;     // damage acting on jumpers
    unsigned int     bspLeafSize;      // spatial sectorization limit
    float            lodDistance[engine::maxBatchLods]; // lod distances    
public:
    ForestDesc()
    {
        cache = "";
        layers  = NULL;
        surface = NULL;
        assetName = "";
        minScale = 0;
        maxScale = 0;
        collScale = 0;
        density = 0;
        entangleFactor = 0;
        minImpulseFactor = 0;
        maxImpulseFactor = 0;
        ripFactor = 0;
        damageFactor = 0;
        bspLeafSize = 0;
        for( unsigned int i=0; i<engine::maxBatchLods; i++ )
        {
            lodDistance[i] = 0;
        }
    }
};

class Forest : public Actor
{
private:
    ForestDesc            _desc;             // descriptor
    engine::IAsset*       _asset;            // loaded asset        
    engine::BatchScheme   _trunkScheme;      // batch scheme of tree trunk
    engine::BatchScheme   _canopyScheme;     // batch scheme of tree canopy    
    std::vector<Matrix4f> _treeMatrix;       // forest solution
    engine::IBatch*       _trunkBatch;       // trunk batch renderer
    engine::IBatch*       _canopyBatch;      // canopy batch renderer
    audio::ISound*        _rustleSound;      // sound of rustle
    audio::ISound*        _squeakSound;      // sound of wook squeak
    std::vector<NxBox>    _debugBoxes;
private:
    // collision with jumpers
    Actor*            _currentJumper;
    engine::IAtomic*  _currentJumperCollision;
    NxActor*          _currentJumperActor;
    NxBox             _jumperOBB;
    bool              _jumperCanopyIsOpened;
    Actor*            _currentCanopy;
    database::Canopy* _currentCanopyInfo;
    engine::IAtomic*  _currentCanopyCollision;
    NxActor*          _currentCanopyActor;
    NxBox             _canopyOBB;
private:
    static unsigned int getClumpLodId(engine::IClump* clump);
    static unsigned int onCollideJumper(unsigned int id, Matrix4f* matrix, void* data);
    static unsigned int onCollideCanopy(unsigned int id, Matrix4f* matrix, void* data);
private:
    float getDistanceToNearestTree(const Vector3f& pos);
private:
    // sounds
    void playRustleSound(const Vector3f& pos);
    void playSqueakSound(const Vector3f& pos);
public:
    // actor abstracts
    virtual void onEvent(Actor* initiator, unsigned int eventId, void* eventData);
    virtual void onUpdateActivity(float dt);
    virtual void onUpdatePhysics(void);
    virtual Matrix4f getPose(void) { return Matrix4f( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 ); }
public:
    // class implementation
    Forest(Actor* parent, ForestDesc* desc);
    virtual ~Forest();
public:
    void simulateInteraction(Actor* actor);
    void debugRender(void);
};

#endif