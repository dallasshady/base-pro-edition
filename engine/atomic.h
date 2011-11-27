/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description atomics
 *
 * @author bad3p
 */

#ifndef ATOMIC_IMPLEMENTATION_INCLUDED
#define ATOMIC_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "frame.h"
#include "geometry.h"
#include "shadows.h"

/**
 * IAtomic implementation
 */

class Atomic : public engine::IAtomic,
               virtual public Updatable 
{
private:
    struct Chunk
    {
        auid id;
        auid geometryId;
        auid frameId;
    };
private:
    friend struct Frame;
    friend class  BSP;
    friend class  BSPSector;
    friend class  RayIntersection;
    friend class  SphereIntersection;
private:
    typedef std::list<void*> voidL;
    typedef voidL::iterator voidI;
private:
    unsigned int _flags;
    unsigned int _renderFrameId;
    Frame*       _frame;
    Geometry*    _geometry;
    Sphere       _boundingSphere; // this is actual bounding sphere of atomic
    Sphere       _synchroSphere;  // this is bounding sphere used for placement within BSP
    void*        _bsp;
    voidL        _sectors;
    int          _numBoneMatrices;
    D3DXMATRIX** _boneMatrices;
    Texture*     _lightmap;
private:
    // runtime only
    bool                    _lodEnabled;
    float                   _lodMaxDistance;
    float                   _lodMinDistance;
    engine::IAtomicCallBack _renderCallback;
    void*                   _renderCallbackData;
    float                   _distanceUT;
    float                   _radiusUT;
    Vector                  _contourDirection;
protected:
    // Updatable
    virtual void onUpdate(void);
protected:
    // hidden behaviour
    void collideBSPSector(void* sector);
public:
    // class implementation
    Atomic();
    virtual ~Atomic();
public:
    // IAtomic
    virtual void __stdcall release(void);
    virtual unsigned int __stdcall getFlags(void);
    virtual void __stdcall setFlags(unsigned int flags);
    virtual engine::IFrame* __stdcall getFrame(void);
    virtual void __stdcall setFrame(engine::IFrame* frame);
    virtual engine::IGeometry* __stdcall getGeometry(void);
    virtual void __stdcall setGeometry(engine::IGeometry* geometry);
    virtual void __stdcall setRenderCallback(engine::IAtomicCallBack callBack, void* data);
    virtual void __stdcall setUpdateTreshold(float distance, float radius);
    virtual Vector3f __stdcall getAABBInf(void);
    virtual Vector3f __stdcall getAABBSup(void);
    virtual engine::ITexture* __stdcall getLightMap(void);
    virtual void __stdcall setLightMap(engine::ITexture* lightMap);
    virtual Vector3f __stdcall getContourDirection(void);
    virtual void __stdcall setContourDirection(const Vector3f& value);
public:
    // module locals : inlines
    inline unsigned int flags(void) { return _flags; }
    inline Frame*  frame(void) { return _frame; }
    inline Geometry* geometry(void) { return _geometry; }
    inline Sphere* getBoundingSphere(void) { return &_boundingSphere; }
    inline void* getBSP(void) { return _bsp; }
    inline bool isLODEnabled(void) { return _lodEnabled; }
    inline float getLODMaxDistance(void) { return _lodMaxDistance; }
    inline float getLODMinDistance(void) { return _lodMinDistance; }
    inline D3DXMATRIX** getBoneMatrices(void) { return _boneMatrices; }
    inline Texture* lightmap(void) { return _lightmap; }
    inline Vector* cinematicContourDirection(void) { return &_contourDirection; }    
public:
    // module locals
    Atomic* clone(void);
    void render(void);
    void renderDepthMap(void);
    void renderShadowVolume(ShadowVolume* shadowVolume, float depth, Vector* lightPos, Vector* lightDir);
    void setLOD(float maxDistance, float minDistance);
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
public:
    // atomic, being rendered now
    static Atomic* currentAtomic;
};

typedef Atomic* (*AtomicRayIntersectionCallback)(Line* ray, Atomic* atomic);

#endif
