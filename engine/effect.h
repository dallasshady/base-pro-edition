/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description effect implementation
 *
 * @author bad3p
 */

#ifndef EFFECT_IMPLEMENTATION_INCLUDED
#define EFFECT_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "texture.h"
#include "shader.h"
#include "geometry.h"
#include "mesh.h"

/**
 * IEffect implementation
 */

class Effect : public engine::IEffect
{
protected:
    typedef std::list<Effect*> EffectL;
    typedef EffectL::iterator EffectI;
protected:
    typedef std::pair<std::string,ID3DXEffect*> HLSL;
    typedef std::map<std::string,ID3DXEffect*> HLSLMap;
    typedef HLSLMap::iterator HLSLI;
protected:
    static EffectL         _animatedEffects; // global list of automatically updated effects
    static IParamPack*     _hlslConfig;      // HLSL-effect configuratioon file
    static HLSLMap         _hlslMap;         // global map of HLSL-effects
    static StaticLostable* _hlslLostable;
private:
    struct Chunk
    {
        auid id;
        char name[engine::maxNameLength];
        bool hasArguments;
    };
protected:
    friend class Geometry;
protected:
    // support of cooperative work
    static void onLostDevice(void);
    static void onResetDevice(void);
protected:
    // Effect protected behaviour
    virtual int getBufferSize(void) = 0;
    virtual void* getBuffer(void) = 0;
    virtual void onUpdate(float dt) {}
    virtual void onSetEffect(Geometry* geometry) {}
    virtual void onChangeArgument(int argId) {}
public:
    // Effect : module local
    virtual void render(Mesh* mesh, int subsetId, Shader* shader) = 0;
public:
    // class implementation
    virtual ~Effect();
    // IEffect
    virtual void __stdcall release(void) { delete this; }
    // serialization
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
public:
    // Effect manipulation
    static int getNumEffects(void);
    static const char* getEffectName(int effectId);
    static Effect* create(const char* effectName);    
    static void update(float dt);
    // system routine
    static void init(void);
    static void term(void);
};

/**
 * grass shading
 */

#define GRASS_MAX_NUM_LIGHTS 2

class FxGrass : public Effect
{
private:
    static ID3DXEffect*    _grassEffect;
    static Quartector      _lightPosition[GRASS_MAX_NUM_LIGHTS];
    static D3DCOLORVALUE   _lightDiffuseColor[GRASS_MAX_NUM_LIGHTS];
    static D3DCOLORVALUE   _lightSpecularColor[GRASS_MAX_NUM_LIGHTS];
    static StaticLostable* _grassLostable;
private:
    struct Arguments
    {
        unsigned int numLayers;  // number of layers rendered to reach volume effect
        float        height;     // grass height
    };
private:
    Arguments _arguments;
private:
    // support of cooperative work
    static void onLostDevice(void);
    static void onResetDevice(void);
protected:
    // protected Effect implemetation
    virtual int getBufferSize(void) { return sizeof(Arguments); }
    virtual void* getBuffer(void) { return &_arguments; }
public:
    // public Effect implemetation
    virtual void render(Mesh* mesh, int subsetId, Shader* shader);
public:
    // class implementation
    FxGrass();
    // IEffect implemetation
    virtual const char* __stdcall getName(void);
    virtual int __stdcall getNumArguments(void);
    virtual const char* __stdcall getArgumentName(int argId);    
    virtual Variant __stdcall getArgument(int argId);
    virtual void __stdcall setArgument(int argId, const Variant& value);
    virtual bool __stdcall isCompatible(engine::IShader* shader);
    virtual bool __stdcall isCompatible(engine::IGeometry* geometry);
public:
    // effect resource management
    static void releaseResources(void);
public:
    // public effect name
    static const char* effectName;
};

/**
 * water surface
 */

#define WATER_MAX_NUM_LIGHTS        2
#define WATER_NUM_ANIMATION_SAMPLES 16

class FxWater : public Effect
{
private:
    static ID3DXEffect*    _effect;
    static StaticLostable* _effectLostable;
private:
    struct Arguments
    {
        float   reflectivity;
        Flector uvOffset1;
        Flector uvVelocity1;
        Flector uvOffset2;
        Flector uvVelocity2;
    };
private:
    Arguments _arguments;
private:
    static void onLostDevice(void);
    static void onResetDevice(void);
protected:
    // protected Effect implemetation
    virtual int getBufferSize(void) { return sizeof(Arguments); }
    virtual void* getBuffer(void) { return &_arguments; }
    virtual void onUpdate(float dt);
public:
    // public Effect implemetation
    virtual void render(Mesh* mesh, int subsetId, Shader* shader);
public:
    // class implementation
    FxWater();
    // IEffect implemetation
    virtual const char* __stdcall getName(void);
    virtual int __stdcall getNumArguments(void);
    virtual const char* __stdcall getArgumentName(int argId);    
    virtual Variant __stdcall getArgument(int argId);
    virtual void __stdcall setArgument(int argId, const Variant& value);
    virtual bool __stdcall isCompatible(engine::IShader* shader);
    virtual bool __stdcall isCompatible(engine::IGeometry* geometry);
public:
    // effect resource management
    static void releaseResources(void);
public:
    // public effect name
    static const char* effectName;
};

/**
 * texture blender (fixed function)
 */

#define BLENDER_MAX_NUM_LIGHTS 2

class FxBlender : public Effect
{
private:
    struct Arguments
    {
        float detailsLayerOffset;
        float detailsUVMultiplier;
    };
private:
    static ID3DXEffect*    _effect;
    static StaticLostable* _effectLostable;
    static Quartector      _lightPosition[GRASS_MAX_NUM_LIGHTS];
    static D3DCOLORVALUE   _lightDiffuseColor[GRASS_MAX_NUM_LIGHTS];
    static D3DCOLORVALUE   _lightSpecularColor[GRASS_MAX_NUM_LIGHTS];
private:
    Arguments _arguments;
private:
    static void onLostDevice(void);
    static void onResetDevice(void);
protected:
    // protected Effect implemetation
    virtual int getBufferSize(void) { return sizeof(Arguments); }
    virtual void* getBuffer(void) { return &_arguments; }    
public:
    // public Effect implemetation
    virtual void render(Mesh* mesh, int subsetId, Shader* shader);
public:
    // class implementation
    FxBlender();
    // IEffect implemetation
    virtual const char* __stdcall getName(void);
    virtual int __stdcall getNumArguments(void);
    virtual const char* __stdcall getArgumentName(int argId);    
    virtual Variant __stdcall getArgument(int argId);
    virtual void __stdcall setArgument(int argId, const Variant& value);
    virtual bool __stdcall isCompatible(engine::IShader* shader);
    virtual bool __stdcall isCompatible(engine::IGeometry* geometry);
public:
    // effect resource management
    static void releaseResources(void);
public:
    // public effect name
    static const char* effectName;
};

/**
 * waterfall effect
 */

#define WATERFALL_MAX_NUM_LIGHTS 2

class FxWaterfall : public Effect
{
private:
    struct Arguments
    {
        Flector uvOffset;
        Flector uvVelocity;
    };
private:
    static ID3DXEffect*    _effect;
    static StaticLostable* _effectLostable;
    static Quartector      _lightPosition[WATERFALL_MAX_NUM_LIGHTS];
    static D3DCOLORVALUE   _lightDiffuseColor[WATERFALL_MAX_NUM_LIGHTS];
    static D3DCOLORVALUE   _lightSpecularColor[WATERFALL_MAX_NUM_LIGHTS];
private:
    Arguments _arguments;
private:
    static void onLostDevice(void);
    static void onResetDevice(void);
protected:
    // protected Effect implemetation
    virtual int getBufferSize(void) { return sizeof(Arguments); }
    virtual void* getBuffer(void) { return &_arguments; }
    virtual void onUpdate(float dt);
public:
    // public Effect implemetation
    virtual void render(Mesh* mesh, int subsetId, Shader* shader);
public:
    // class implementation
    FxWaterfall();
    // IEffect implemetation
    virtual const char* __stdcall getName(void);
    virtual int __stdcall getNumArguments(void);
    virtual const char* __stdcall getArgumentName(int argId);    
    virtual Variant __stdcall getArgument(int argId);
    virtual void __stdcall setArgument(int argId, const Variant& value);
    virtual bool __stdcall isCompatible(engine::IShader* shader);
    virtual bool __stdcall isCompatible(engine::IGeometry* geometry);
public:
    // effect resource management
    static void releaseResources(void);
public:
    // public effect name
    static const char* effectName;
};

/**
 * external effect
 */

class EffectHLSL : public Effect
{
protected:
    struct Argument
    {
    public:
        VariantType  type;
        std::string  name;
        unsigned int size;
        unsigned int offset;
    public:
        Argument(const char* descriptor);
    };
    typedef std::vector<Argument> ArgumentV;
    typedef ArgumentV::iterator ArgumentI;
protected:
    struct Flags
    {
        unsigned int camera         : 1; // setup camera attributes
        unsigned int lightPalette   : 4; // setup light sources
        unsigned int material       : 1; // setup material attributes
        unsigned int world          : 1; // setup world transformation
        unsigned int iWorld         : 1; // setup inversion of world transformation
        unsigned int worldViewProj  : 1; // setup world/view/projection transformation
        unsigned int baseTexture    : 1; // setup base texture
        unsigned int normalMap      : 1; // setup normal map
        unsigned int environmentMap : 1; // setup environment map
    };
protected:
    Flags        _flags;      // effect flags
    ID3DXEffect* _effect;     // effect code
    std::string  _name;       // effect name
    ArgumentV    _arguments;  // effect arguments
    unsigned int _bufferSize; // size of argument buffer
    char*        _buffer;     // argument buffer
protected:
    // Effect protected behaviour
    virtual int getBufferSize(void) { return _bufferSize; }
    virtual void* getBuffer(void) { return _buffer; }
    virtual void onUpdate(float dt) {}
    virtual void onSetEffect(Geometry* geometry) {}
    virtual void onChangeArgument(int argId) {}
public:
    // Effect : module local
    virtual void render(Mesh* mesh, int subsetId, Shader* shader);
public:
    // class implementation
    EffectHLSL(const char* fxName);
    virtual ~EffectHLSL();
public:
    // IEffect implemetation
    virtual const char* __stdcall getName(void);
    virtual int __stdcall getNumArguments(void);
    virtual const char* __stdcall getArgumentName(int argId);    
    virtual Variant __stdcall getArgument(int argId);
    virtual void __stdcall setArgument(int argId, const Variant& value);
    virtual bool __stdcall isCompatible(engine::IShader* shader);
    virtual bool __stdcall isCompatible(engine::IGeometry* geometry);
};

#endif
