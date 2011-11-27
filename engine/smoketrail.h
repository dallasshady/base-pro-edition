
#ifndef SMOKETRAIL_RENDERING_INCLUDED
#define SMOKETRAIL_RENDERING_INCLUDED

#include "headers.h"
#include "../shared/engine.h"
#include "tinyxml.h"
#include "fundamentals.h"
#include "texture.h"
#include "shader.h"
#include "../common/istring.h"
#include "rendering.h"

/**
 * smoketrail rendering
 */

class SmokeTrail : public Rendering,
                   virtual public Lostable
{
private:
    struct TrailParticle
    {
    public:
        Vector position;  // current particle position
        Vector velocity;  // current particle velocity
        float  lifeTime;  // increasing life time
        float  sizeTime;  // it's taken from LERP value of emission descriptor
        float  startSize; // it's taken from LERP value of emission descriptor
        float  endSize;   // it's taken from LERP value of emission descriptor
        unsigned char colorR; // particle color (taken from current ambient)
        unsigned char colorG; // particle color (taken from current ambient)
        unsigned char colorB; // particle color (taken from current ambient)
    public:
        TrailParticle(Vector p, Vector v, float lt, float st, float ss, float es, unsigned char r, unsigned char g, unsigned char b) :
            position( p ),
            velocity( v ),
            lifeTime( lt ),
            sizeTime( st ),
            startSize( ss ),
            endSize( es ),
            colorR( r ),
            colorG( g ),
            colorB( b )
        {}
    };
    typedef std::list<TrailParticle> TrailParticles;
    typedef TrailParticles::iterator TrailParticleI;
private:
    /**
     * vertex structure
     */
    struct ParticleVertex
    {
        Vector  pos;   // position
        Color   color; // color
        Flector uv;    // uv
    };
private:
    Shader*                  _shader;    
    engine::SmokeTrailScheme _scheme;            // scheme descriptor
    unsigned char            _ambientR;          // ambient color
    unsigned char            _ambientG;          // ambient color
    unsigned char            _ambientB;          // ambient color
    Flector                  _uvs[4];            // UV's wrapped from scheme descriptor
    TrailParticles           _particles;         // list of particles
    bool                     _enabled;           // if true, emission will take place
    Vector                   _emissionPoint;     // current emission point
    Vector                   _emissionDirection; // current emission velocity vector
    Vector                   _windVelocity;      // current wind velocity    
    Vector                   _emitterVelocity;   // emitter velocity
private:
    IDirect3DVertexBuffer9* _vertexBuffer; // vertex buffer
    IDirect3DIndexBuffer9*  _indexBuffer;  // index buffer
private:
    void update(float dt);
public:
    // class implementation
    SmokeTrail(engine::IShader* shader, engine::SmokeTrailScheme* scheme);
    virtual ~SmokeTrail();
    // IRendering
    virtual void __stdcall release(void);    
    virtual void __stdcall setProperty(const char* propertyName, float value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector2f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector3f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector4f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Matrix4f& value);
    // Rendering
    virtual void render(void);
    // Lostable
    virtual void onLostDevice(void);
    virtual void onResetDevice(void);
};

#endif