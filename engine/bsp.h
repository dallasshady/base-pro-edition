/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description bsp implementation
 *
 * @author bad3p
 */

#ifndef BSP_IMPLEMENTATION_INCLUDED
#define BSP_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "atomic.h"
#include "light.h"
#include "clump.h"
#include "shader.h"
#include "alphasort.h"
#include "rendering.h"
#include "psys.h"
#include "batch.h"
#include "shadows.h"

/**
 * IBSPSector
 */

class BSPSector : public engine::IBSPSector
{
private:
    struct Chunk
    {
        auid id;
        auid bspId;
        auid parentId;
        AABB boundingBox;
        bool hasGeometry;
    };
private:
    friend class Atomic;
    friend class Light;
    friend class BSP;
    friend class RayIntersection;
private:
    typedef std::set<Atomic*> AtomicS;
    typedef AtomicS::iterator AtomicI;
    typedef std::set<Light*> LightS;
    typedef LightS::iterator LightI;
private:
    AABB         _boundingBox;
    BSP*         _bsp;
    BSPSector*   _parent;
    BSPSector*   _leftSubset;
    BSPSector*   _rightSubset;
    AtomicS      _atomicsInSector;
    LightS       _lightsInSector;
    Geometry*    _geometry;
    Texture*     _lightmap;
    Vector       _lsc, _rsc, _lsd, _rsd; // temporary data
public:
    // class implementation
    BSPSector(BSP* bsp, BSPSector* parent, AABB boundingBox, Geometry* geometry);
    virtual ~BSPSector();
public:
    // IWorldSector
    virtual bool __stdcall isLeaf(void);
    virtual engine::IGeometry* __stdcall getGeometry(void);
    virtual engine::IBSP* __stdcall getBSP(void);
    virtual engine::ITexture* __stdcall getLightMap(void);
    virtual void __stdcall setLightMap(engine::ITexture* lightMap);
public:
    // module local : inlines
    inline AABB* getBoundingBox(void) { return &_boundingBox; }
    inline Geometry* geometry(void) { return _geometry; }
    inline BSP* bsp(void) { return _bsp; }
    inline Texture* lightmap(void) { return _lightmap; }
public:
    // module local
    BSPSector* getSectorAroundPoint(const Vector& point);
    int getNumSubsetSectors(void);
    int getNumLeafSectors(void);
    int getSectorLevel(void);
    void illuminate(unsigned int lightset);
    void render(void);
    void renderDepthMap(void);
    void renderShadowVolume(ShadowVolume* shadowVolume, float depth, Vector* lightPos, Vector* lightDir);
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
    bool forAllAtomicIntersections(Line* ray, AtomicRayIntersectionCallback callBack);
public:
    static void subdivide(BSPSector* sector);
public:
    static BSPSector* currentSector;
};

/**
 * IBSP
 */

class BSP : public engine::IBSP
{
private:
    friend class BSPSector;
private:
    // additional alpha storage
    typedef std::vector<AlphaGeometry> AlphaGeometryV;
    typedef AlphaGeometryV::iterator AlphaGeometryI;
private:
    // alpha rendering
    void renderAlphaGeometry(void);
private:
    // world chunk
    struct Chunk
    {
        auid       id;
        char       name[engine::maxNameLength];
        AABB       boundingBox;
        int        numShaders;        
        int        numSectors;
        D3DFOGMODE fogMode;
        float      fogStart;
        float      fogEnd;
        float      fogDensity;
        Color      fogColor;
    };
private:
    typedef std::list<Clump*> ClumpL;
    typedef ClumpL::iterator ClumpI;
    typedef std::list<Light*> LightL;
    typedef LightL::iterator LightI;
    typedef std::list<Rendering*> RenderingL;
    typedef RenderingL::iterator RenderingI;
    typedef std::list<ParticleSystem*> ParticleSystemL;
    typedef ParticleSystemL::iterator ParticleSystemI;
    typedef std::list<Batch*> BatchL;
    typedef BatchL::iterator BatchI;
private:
    std::string               _name;
    AABB                      _boundingBox;
    BSPSector*                _root;
    ClumpL                    _clumps;
    LightL                    _lights;
    LightL                    _ambients;
    RenderingL                _renderings;
    ParticleSystemL           _particleSystems;
    BatchL                    _batches;
    int                       _numShaders;
    Shader**                  _shaders;
    D3DFOGMODE                _fogMode;
    float                     _fogStart;
    float                     _fogEnd;
    float                     _fogDensity;
    Color                     _fogColor;
    Vector                    _shadowCastDirection;
    Color                     _shadowCastColor;
    float                     _shadowCastDepth;
    unsigned int              _renderFrameId;
    AlphaSorting              _sortedAlpha;
    AlphaGeometryV            _unsortedAlpha;
    ShadowVolume*             _shadowVolume;
    engine::BSPRenderCallback _postRenderCallback;
    void*                     _postRenderCallbackData;
private:
    // internals
    static engine::IAtomic* setAtomicWorldCB(engine::IAtomic* atomic, void* data);
    static engine::IAtomic* removeAtomicCB(engine::IAtomic* atomic, void* data);
    static engine::ILight* setLightWorldCB(engine::ILight* light, void* data);
    static engine::ILight* removeLightCB(engine::ILight* light, void* data);
    static BSPSector* sectorCallBack(BSPSector* sector, engine::IBSPSectorCallBack callBack, void* data);
    static BSPSector* sectorRender(BSPSector* sector);
    static BSPSector* sectorRenderDepthMap(BSPSector* sector);
    static BSPSector* sectorRenderShadowVolume(BSPSector* sector, ShadowVolume* shadowVolume, float depth, Vector* lightPos, Vector* lightDir);
    static engine::ILight* renderLensFlaresCB(engine::ILight* light, void* data);
    static engine::ILight* findShadowCastLightCB(engine::ILight* light, void* data);
    static engine::IClump* findShadowCastLightCB(engine::IClump* clump, void* data);
    static void renderLensFlares(Light* light);
public:
    // class implementation
    BSP(const char* bspName, AABB boundingBox, int numShaders);
    virtual ~BSP();
public:
    // IBSP
    virtual const char* __stdcall getName(void);
    virtual void __stdcall release(void);
    virtual void __stdcall add(engine::IClump* clump);
    virtual void __stdcall add(engine::ILight* light);
    virtual void __stdcall add(engine::IRendering* rendering);
    virtual void __stdcall add(engine::IParticleSystem* particleSystem);
    virtual void __stdcall add(engine::IBatch* batch);
    virtual void __stdcall remove(engine::IClump* clump);
    virtual void __stdcall remove(engine::ILight* light);
    virtual void __stdcall remove(engine::IRendering* rendering);
    virtual void __stdcall remove(engine::IParticleSystem* particleSystem);
    virtual void __stdcall remove(engine::IBatch* batch);
    virtual void __stdcall forAllSectors(engine::IBSPSectorCallBack callBack, void* data);
    virtual void __stdcall forAllLights(engine::ILightCallBack callBack, void* data);
    virtual void __stdcall forAllClumps(engine::IClumpCallBack callBack, void* data);
    virtual void __stdcall render(void);
    virtual void __stdcall renderLensFlares(void);
    virtual void __stdcall renderDepthMap(void);
    virtual Vector3f __stdcall getAABBInf(void);
    virtual Vector3f __stdcall getAABBSup(void);
    // IBSP : fog controls
    virtual engine::FogType __stdcall getFogType(void);
    virtual void __stdcall setFogType(engine::FogType fogType);
    virtual float __stdcall getFogStart(void);
    virtual void __stdcall setFogStart(float fogStart);
    virtual float __stdcall getFogEnd(void);
    virtual void __stdcall setFogEnd(float fogEnd);
    virtual float __stdcall getFogDensity(void);
    virtual void __stdcall setFogDensity(float fogDensity);
    virtual Vector4f __stdcall getFogColor(void);
    virtual void __stdcall setFogColor(const Vector4f& fogColor);
    // IBSP : shadow control
    virtual void __stdcall setShadowCastDirection(const Vector3f& dir);
    virtual void __stdcall setShadowCastColor(const Vector4f& color);
    virtual void __stdcall setShadowCastDepth(float depth);
    // IBSP : miscellaneous
    virtual Vector4f __stdcall getAmbient(unsigned int lightset = 0);
    virtual void __stdcall setPostRenderCallback(engine::BSPRenderCallback callback, void* data);
public:
    // module locals : inlines
    inline unsigned int getRenderFrameId(void) { return _renderFrameId; }
    inline BSPSector* getRoot(void) { return _root; }
    inline BSPSector* getSectorAroundPoint(const Vector& point) { return _root->getSectorAroundPoint( point ); }
    inline int getNumShaders(void) { return _numShaders; }
    inline Shader** getShaders(void) { return _shaders; }
    inline Shader* getShader(int id) { return _shaders[id]; }
public:
    // module locals
    void setShader(int id, Shader* shader);
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
    void forAllAtomicIntersections(Line* ray, AtomicRayIntersectionCallback callBack);
    void calculateGlobalAmbient(unsigned int lightset);
public:
    // module locals: alpha-sorting support
    void addAlphaGeometry(Atomic* atomic, unsigned int subsetId);
    void addAlphaGeometry(BSPSector* sector, unsigned int subsetId);
public:
    static BSP* currentBSP;
};

BSP* getAtomicBSP(Atomic* atomic);

#endif
