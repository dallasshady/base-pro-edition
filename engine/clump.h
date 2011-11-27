/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description clumps
 *
 * @author bad3p
 */

#ifndef CLUMP_IMPLEMENTATION_INCLUDED
#define CLUMP_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "frame.h"
#include "atomic.h"
#include "light.h"
#include "animation.h"

/**
 * IAtomic implementation
 */

class Clump : public engine::IClump
{
private:
    struct Chunk
    {
        auid id;
        char name[engine::maxNameLength];
        int  numFrames;
        int  numGeometries;
        int  numAtomics;
        int  numLights;        
    };
private:
    friend class BinaryAsset;
    friend class BSP;
private:
    typedef std::list<Atomic*> AtomicL;
    typedef AtomicL::iterator AtomicI;
    typedef std::list<Light*> LightL;
    typedef LightL::iterator LightI;
private:
    std::string          _name;
    Frame*               _frame;
    AtomicL              _atomics;
    LightL               _lights;
    void*                _bsp;
    AnimationController* _animController;
    bool                 _hasLODs;
private:
    static engine::IFrame* collectFrameCB(engine::IFrame* frame, void* data);
    void cloneFrameHierarchy(Frame* frame, Clump* clone);
    void cloneAttachedObjects(Clump* clone);
public:
    // class implementation
    Clump(const char* clumpName);
    virtual ~Clump();
public:
    // IClump
    virtual const char* __stdcall getName(void);
    virtual void __stdcall setName(const char* name);
    virtual engine::IBSP* __stdcall getBSP(void);
    virtual void __stdcall release(void);
    virtual engine::IFrame* __stdcall getFrame(void);
    virtual void __stdcall setFrame(engine::IFrame* frame);
    virtual void __stdcall add(engine::IAtomic* atomic);
    virtual void __stdcall remove(engine::IAtomic* atomic);
    virtual void __stdcall add(engine::ILight* light);
    virtual void __stdcall remove(engine::ILight* light);
    virtual void __stdcall forAllAtomics(engine::IAtomicCallBack callBack, void* data);
    virtual void __stdcall forAllLights(engine::ILightCallBack callBack, void* data);
    virtual void __stdcall render(void);
    virtual engine::IClump* __stdcall clone(const char* cloneName);
    virtual void __stdcall setLOD(engine::IAtomic* atomic, float maxDistance, float minDistance);
    // IClump : animation support
    virtual engine::IAnimationController* __stdcall getAnimationController(void) { return _animController; }
public:
    inline Atomic* getAtomic(Frame* frame)
    {
        for( AtomicI atomicI = _atomics.begin(); atomicI != _atomics.end(); atomicI++ )
        {
            if( (*atomicI)->frame() == frame ) return (*atomicI);
        }
        return NULL;
    }
	inline Frame* frame(void) { return _frame; }
    inline AnimationController* getAnimController(void) { return _animController; }
    inline void setAnimation(AnimationSet* animationSet) 
    { 
        _animController = new AnimationController( _frame, animationSet, _frame );
    }
public:
    // module locals
    void updateLODs(void);
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
};

#endif

