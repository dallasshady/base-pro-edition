
#ifndef GLOW_RENDERING_INCLUDED
#define GLOW_RENDERING_INCLUDED

#include "headers.h"
#include "../shared/engine.h"
#include "fundamentals.h"
#include "rendering.h"
#include "psys.h"
#include "bsp.h"
#include "light.h"

/**
 * glow rendering
 */

class Glow : public Rendering
{
private:
    struct GlowParticle
    {
    public:
        Light*   light;     // driven light source
        bool     state;     // particle state (in/out of frustrum)
        Vector4f color;     // particle color
    public:
        GlowParticle(Light* l) : light(l), state(false), color(0,0,0,0) {}
    };
private:
    typedef std::vector<GlowParticle> GlowParticles;
private:
    static engine::ILight* collectLights(engine::ILight* light, void* data);
    static engine::IClump* collectLights(engine::IClump* clump, void* data);
private:    
    BSP*            _bsp;             // driven BSP scene    
    GlowParticles   _glowParticles;   // lights from BSP scene
    Shader*         _shader;          // shading technique
    ParticleSystem* _particleSystem;  // driven particles    
private:
    // setProperty(...) - rendering properties 
    float    _dt;              // ("dt") - update time step 
    Vector4f _fadeSpeed;       // ("fadeSpeed") - color fading speed
    float    _minSizeDistance; // ("minSizeDistance") - minimal sizing distance
    Vector2f _minSize;         // ("minSize") - particle size at minimal distance
    float    _maxSizeDistance; // ("maxSizeDistance") - maximal sizing distance
    Vector2f _maxSize;         // ("maxSize") - particle size at maximal distance
public:
    // class implementation
    Glow(BSP* bsp, const char* glowTextureResource);
    virtual ~Glow();
    // IRendering
    virtual void __stdcall release(void);
    virtual void __stdcall setProperty(const char* propertyName, float value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector2f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector3f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector4f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Matrix4f& value);
    // Rendering
    virtual void render(void);
};

#endif