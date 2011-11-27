/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description light sources
 *
 * @author bad3p
 */

#ifndef LIGHT_IMPLEMENTATION_INCLUDED
#define LIGHT_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "frame.h"

class Light : public engine::ILight,
              virtual public Updatable
{
private:
    struct Chunk
    {
        auid         id;
        unsigned int lightset;
        D3DLIGHT9    lightProps;
        auid         frameId;
    };
private:
    friend class BSP;
private:
    typedef std::list<void*> voidL;
    typedef voidL::iterator voidI;
private:
    unsigned int _lightset;
    D3DLIGHT9    _lightProps;
    Frame*       _frame;
    Sphere       _lightSphere;
    void*        _bsp;
    voidL        _sectors;
protected:
    // Updatable
    virtual void onUpdate(void);
protected:
    // callbacks
    void affectBSPSector(void* sector);
public:
    // class implementation
    Light(D3DLIGHT9* lightProps);
    Light(engine::LightType lightType);
    virtual ~Light();
    // ILight
    virtual void __stdcall release(void);
    virtual engine::IFrame* __stdcall getFrame(void);
    virtual void __stdcall setFrame(engine::IFrame* frame);
    virtual engine::LightType __stdcall getType(void);    
    virtual unsigned int __stdcall getLightset(void);
    virtual void __stdcall setLightset(unsigned int value);
    virtual Vector4f __stdcall getDiffuseColor(void);
    virtual void __stdcall setDiffuseColor(Vector4f value);
    virtual Vector4f __stdcall getSpecularColor(void);
    virtual void __stdcall setSpecularColor(Vector4f value);
    virtual float __stdcall getRange(void);
    virtual void __stdcall setRange(float value);
    virtual Vector3f __stdcall getAttenuation(void);
    virtual void __stdcall setAttenuation(const Vector3f& value);
    virtual float __stdcall getTheta(void);
    virtual void __stdcall setTheta(float theta);
    virtual float __stdcall getPhi(void);
    virtual void __stdcall setPhi(float phi);
public:
    inline unsigned int lightset(void) { return _lightset; }
    inline D3DCOLORVALUE* diffuse(void) { return &_lightProps.Diffuse; }
    inline Vector position(void) { return static_cast<Vector>( _lightProps.Position ); }
    inline Vector direction(void) { return static_cast<Vector>( _lightProps.Direction ); }
    inline Frame* frame(void) { return _frame; }
public:
    // module local
    Light* clone(void);
    void apply(DWORD lightIndex);
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
};

#endif