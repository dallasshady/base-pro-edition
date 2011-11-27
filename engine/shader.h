/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description shaders etc.
 *
 * @author bad3p
 */

#ifndef SHADER_IMPLEMENTATION_INCLUDED
#define SHADER_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "texture.h"


static inline engine::BlendingMode wrap(D3DBLEND value)
{
    switch( value )
    {
    case D3DBLEND_ZERO: return engine::bmZero;
    case D3DBLEND_ONE: return engine::bmOne;
    case D3DBLEND_SRCCOLOR: return engine::bmSrcColor;
    case D3DBLEND_INVSRCCOLOR: return engine::bmInvSrcColor;
    case D3DBLEND_SRCALPHA: return engine::bmSrcAlpha;
    case D3DBLEND_INVSRCALPHA: return engine::bmInvSrcAlpha;
    case D3DBLEND_DESTALPHA: return engine::bmDestAlpha;
    case D3DBLEND_INVDESTALPHA: return engine::bmInvDestAlpha;
    case D3DBLEND_DESTCOLOR: return engine::bmDestColor;
    case D3DBLEND_INVDESTCOLOR: return engine::bmInvDestColor;
    case D3DBLEND_SRCALPHASAT: return engine::bmSrcAlphaSat;
    default:
        assert( !"shouldn't be here!" );
        return engine::bmZero;
    }
}

static inline D3DBLEND wrap(engine::BlendingMode value)
{
    switch( value )
    {
    case engine::bmZero: return D3DBLEND_ZERO;
    case engine::bmOne: return D3DBLEND_ONE;
    case engine::bmSrcColor: return D3DBLEND_SRCCOLOR;
    case engine::bmInvSrcColor: return D3DBLEND_INVSRCCOLOR;
    case engine::bmSrcAlpha: return D3DBLEND_SRCALPHA;
    case engine::bmInvSrcAlpha: return D3DBLEND_INVSRCALPHA;
    case engine::bmDestAlpha: return D3DBLEND_DESTALPHA;
    case engine::bmInvDestAlpha: return D3DBLEND_INVDESTALPHA;
    case engine::bmDestColor: return D3DBLEND_DESTCOLOR;
    case engine::bmInvDestColor: return D3DBLEND_INVDESTCOLOR;
    case engine::bmSrcAlphaSat: return D3DBLEND_SRCALPHASAT;
    default:
        assert( !"shouldn't be here!" );
        return D3DBLEND_ZERO;
    }
}

static inline engine::BlendingOperation wrap(D3DBLENDOP value)
{
    switch( value )
    {
    case D3DBLENDOP_ADD: return engine::bpAdd;
    case D3DBLENDOP_SUBTRACT: return engine::bpSubtract;
    case D3DBLENDOP_REVSUBTRACT: return engine::bpRevSubtract;
    case D3DBLENDOP_MIN: return engine::bpMin;
    case D3DBLENDOP_MAX: return engine::bpMax;
    default:
        assert( !"shouldn't be here!" );
        return engine::bpAdd;
    }
}

static inline D3DBLENDOP wrap(engine::BlendingOperation value)
{
    switch( value )
    {
    case engine::bpAdd: return D3DBLENDOP_ADD;
    case engine::bpSubtract: return D3DBLENDOP_SUBTRACT;
    case engine::bpRevSubtract: return D3DBLENDOP_REVSUBTRACT;
    case engine::bpMin: return D3DBLENDOP_MIN;
    case engine::bpMax: return D3DBLENDOP_MAX;
    default:
        assert( !"shouldn't be here!" );
        return D3DBLENDOP_ADD;
    }
}

/**
 * IShader implementation
 */

class Shader : public engine::IShader
{
private:
    struct Chunk
    {
        auid                      id;
        char                      name[engine::maxNameLength];
        int                       numLayers;
        auid                      layerTexture[engine::maxTextureLayers];
        engine::BlendingType      layerBlending[engine::maxTextureLayers];
        int                       layerUV[engine::maxTextureLayers];
        Color                     layerConstant[engine::maxTextureLayers];
        D3DMATERIAL9              materialColor;
        auid                      normalMap;
        int                       normalMapUV;
        auid                      environmentMap;
        unsigned int              flags;
        unsigned int              lightset;
        D3DBLEND                  srcBlend;
        D3DBLEND                  dstBlend;
        D3DBLENDOP                blendOp;
        D3DCMPFUNC                alphaTestFunction;
        DWORD                     alphaTestRef;
        bool                      hasEffect;
    };
private:
    friend class Rain;
    friend class Grass;
    friend class ParticleTank;
    friend class Emitter;
    friend class Geometry;
    friend class BSP;
private:
    int                   _numReferences;
    std::string           _name;
    int                   _numLayers;
    Texture*              _layerTexture[engine::maxTextureLayers];
    engine::BlendingType  _layerBlending[engine::maxTextureLayers];
    int                   _layerUV[engine::maxTextureLayers];
    Color                 _layerConstant[engine::maxTextureLayers];
    D3DMATERIAL9          _materialColor;
    Texture*              _normalMap;
    int                   _normalMapUV;
    Texture*              _environmentMap;
    unsigned int          _flags;
    unsigned int          _lightset;
    D3DBLEND              _srcBlend;
    D3DBLEND              _dstBlend;
    D3DBLENDOP            _blendOp;
    D3DCMPFUNC            _alphaTestFunction;
    DWORD                 _alphaTestRef;
    void*                 _effect;
public:
    // non-serializable properties
    D3DCOLORVALUE _hemisphere[2];     // hemisphere ambient color
    D3DCOLORVALUE _illuminationColor; // (cinematic) illumination color
    D3DCOLORVALUE _contourColor;      // (cinematic) contour color
private:
    static Shader*       _lastShader;
    static D3DCOLORVALUE _globalAmbient;
public:
    // class implementation
    Shader(int numLayers, const char* shaderName);
    virtual ~Shader();
public:
    // IShader
    virtual void __stdcall addReference(void);
    virtual int __stdcall getNumReferences(void);
    virtual void __stdcall release(void);
    virtual const char* __stdcall getName(void);
    virtual int __stdcall getNumLayers(void);
    virtual void __stdcall setNumLayers(int numLayers, engine::ITexture* defaultTexture);
    virtual engine::BlendingType __stdcall getLayerBlending(int layerId);
    virtual void __stdcall setLayerBlending(int layerId, engine::BlendingType blending);
    virtual engine::ITexture* __stdcall getLayerTexture(int layerId);
    virtual void __stdcall setLayerTexture(int layerId, engine::ITexture* texture);
    virtual int __stdcall getLayerUV(int layerId);
    virtual void __stdcall setLayerUV(int layerId, int uvId);
    virtual Vector4f __stdcall getLayerConstant(int layerId);
    virtual void __stdcall setLayerConstant(int layerId, const Vector4f& layerConstant);
    virtual Vector4f __stdcall getDiffuseColor(void);
    virtual void __stdcall setDiffuseColor(const Vector4f& value);
    virtual Vector4f __stdcall getSpecularColor(void);
    virtual void __stdcall setSpecularColor(const Vector4f& value);
    virtual float __stdcall getSpecularPower(void);
    virtual void __stdcall setSpecularPower(float value);
    virtual engine::ITexture* __stdcall getNormalMap(void);
    virtual void __stdcall setNormalMap(engine::ITexture* texture);
    virtual int __stdcall getNormalMapUV(void);
    virtual void __stdcall setNormalMapUV(int normalMapUV);
    virtual engine::ITexture* __stdcall getEnvironmentMap(void);
    virtual void __stdcall setEnvironmentMap(engine::ITexture* texture);
    // flags & etc.
    virtual unsigned int __stdcall getFlags(void);
    virtual void __stdcall setFlags(unsigned int flags);
    virtual engine::BlendingMode __stdcall getSrcBlend(void);
    virtual void __stdcall setSrcBlend(engine::BlendingMode blendMode);
    virtual engine::BlendingMode __stdcall getDestBlend(void);
    virtual void __stdcall setDestBlend(engine::BlendingMode blendMode);
    virtual engine::BlendingOperation __stdcall getBlendOp(void);
    virtual void __stdcall setBlendOp(engine::BlendingOperation blendOp);
    virtual engine::CompareFunction __stdcall getAlphaTestFunction(void);
    virtual void __stdcall setAlphaTestFunction(engine::CompareFunction function);
    virtual unsigned char __stdcall getAlphaTestRef(void);
    virtual void __stdcall setAlphaTestRef(unsigned char alphaRef);
    virtual unsigned int __stdcall getLightset(void);
    virtual void __stdcall setLightset(unsigned int value);
    // effects
    virtual engine::IEffect* __stdcall getEffect(void);
    virtual bool __stdcall setEffect(engine::IEffect* effect);
    // hemisphere ambient
    virtual Vector4f __stdcall getUpperHemisphere(void);
    virtual Vector4f __stdcall getLowerHemisphere(void);
    virtual void __stdcall setHemisphere(Vector4f upperColor, Vector4f lowerColor);
    // cinematic lighting
    virtual Vector4f __stdcall getIlluminationColor(void);
    virtual Vector4f __stdcall getContourColor(void);
    virtual void __stdcall setIlluminationColor(const Vector4f& value);
    virtual void __stdcall setContourColor(const Vector4f& value);
public:
    inline Texture* layerTexture(unsigned int i) { return _layerTexture[i]; }
    inline Texture* normalMap(void) { return _normalMap; }
    inline Texture* environmentMap(void) { return _environmentMap; }
    inline bool isInvisible(void) { return ( _alphaTestFunction == D3DCMP_NEVER ); }
    inline D3DMATERIAL9* getMaterial(void) { return &_materialColor; }
    inline void* effect() { return _effect; }
    inline D3DCOLORVALUE* hemisphere() { return _hemisphere; } 
    inline D3DCOLORVALUE* cinematicIlluminationColor(void) { return &_illuminationColor; }
    inline D3DCOLORVALUE* cinematicContourColor(void) { return &_contourColor; }
public:
    inline static D3DCOLORVALUE* globalAmbient(void) { return &_globalAmbient; }
public:
    // module locals
    void apply(void);
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
};

#endif
