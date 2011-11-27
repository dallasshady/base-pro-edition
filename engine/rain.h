
#ifndef RAIN_RENDERING_INCLUDED
#define RAIN_RENDERING_INCLUDED

#include "headers.h"
#include "../shared/engine.h"
#include "tinyxml.h"
#include "fundamentals.h"
#include "texture.h"
#include "shader.h"
#include "../common/istring.h"
#include "rendering.h"

/**
 * rain particle
 */

struct RainParticle
{
public:
    Vector  pos; // particle position
    Vector  vel; // particle velocity
};

/**
 * rain rendering
 */

class Rain : public Rendering,
             virtual public Lostable
{
private:
    typedef std::list<Rain*> RainL;
    typedef RainL::iterator RainI;
private:
    static RainL _rainL;
private:
    Color                   _ambient;      // particle's ambient color
    Shader*                 _shader;       // shading technique
    IDirect3DVertexBuffer9* _vertexBuffer; // vertex buffer
    IDirect3DIndexBuffer9*  _indexBuffer;  // index buffer
private:
    float         _emissionSphere; // actual sphere of rain emission
    unsigned int  _numParticles;   // number of particles
    RainParticle* _particles;      // particle array
    Vector        _centerOffset;   // offset of emission volume (actual for current step)
    Vector        _centerVelocity; // velocity of emission volume (actual for current step)
    unsigned int  _useEdgeOffset;  // internal, means that particle will be spawn at edge
private:    
    Vector        _propCenter;       // property: center of emission sphere
    Vector        _propVelocity;     // property: initial particle velocity
    float         _propNBias;        // property: initial bias for normal of velocity (angles)
    float         _propTimeSpeed;    // property: speed of simulation time (multiplier)
private:
    void onUpdate(float dt);
public:
    // class implementation
    Rain(unsigned int maxParticles, float emissionSphere, engine::ITexture* texture, Vector4f ambient);
    virtual ~Rain();
    // IRendering
    virtual void __stdcall release(void);    
    virtual void __stdcall setProperty(const char* propertyName, float value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector3f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Matrix4f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector2f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector4f& value);    
    // Lostable
    virtual void onLostDevice(void);
    virtual void onResetDevice(void);
    // Rendering
    void render(void);
public:
    // rain activity
    static void update(float dt);
};

#endif