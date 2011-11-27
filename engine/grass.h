
#ifndef GRASS_RENDERING_INCLUDED
#define GRASS_RENDERING_INCLUDED

#include "headers.h"
#include "../shared/engine.h"
#include "tinyxml.h"
#include "fundamentals.h"
#include "texture.h"
#include "shader.h"
#include "../common/istring.h"
#include "rendering.h"

/**
 * grass particle
 */

struct GrassParticle
{
public:
    float   alpha;    // for internal usage
    float   distance; // for internal usage
    Matrix  matrix;
    Flector uv[4];    
};

typedef std::vector<GrassParticle> GrassParticles;
typedef GrassParticles::iterator GrassParticleI;

/**
 * grass cluster
 */

struct GrassCluster
{
public:
    Sphere         boundingSphere;
    unsigned int   numParticles;
    GrassParticle* particles;
public:
    GrassCluster()
    {
        numParticles = 0;
        particles = NULL;
        boundingSphere.center.x = boundingSphere.center.y =  boundingSphere.center.z = 0;
        boundingSphere.radius = 0;
    }
    GrassCluster(unsigned int np)
    {
        numParticles = np;
        particles = new GrassParticle[numParticles];
        boundingSphere.center.x = boundingSphere.center.y =  boundingSphere.center.z = 0;
        boundingSphere.radius = 0;
    }
    virtual ~GrassCluster()
    {
        if( particles ) delete[] particles;
    }
public:
    void updateBoundingSphere(void);
    void push_back(GrassParticle* particle);
};

typedef std::vector<GrassCluster*> GrassClusters;
typedef GrassClusters::iterator  GrassClusterI;

/**
 * grass rendering
 */

class Grass : public Rendering,
              virtual public Lostable
{
private:
    GrassClusters _clusters;  // managed clusters
    float         _fadeStart; // grass particles at this distance begins to fade-out
    float         _fadeEnd;   // grass particles at this distance are disappears
    unsigned char _ambientR;  // ambient color
    unsigned char _ambientG;  // ambient color
    unsigned char _ambientB;  // ambient color
private:
    Shader*                 _shader;       // shading technique
    IDirect3DVertexBuffer9* _vertexBuffer; // vertex buffer
    IDirect3DIndexBuffer9*  _indexBuffer;  // index buffer
private:
    unsigned char   _nestSize[256];  // record contains corresponding nest size
    GrassParticle*  _nest[256][256]; // 256 nests for particle vertices
    unsigned int    _numItems;       // number of items to sort
    GrassParticle*  _items[65535];   // items to sort
private:
    void generateSpecie(engine::GrassSpecie* specie, engine::IAtomic* templateAtomic, GrassParticles& solidStorage);
    void regroupCluster(GrassParticles& solidStorage, float clusterSize);
    void renderBuffers(unsigned int numPassParticles);
public:
    // class implementation
    Grass(const char* resourcePath, engine::IAtomic* templateAtomic, engine::ITexture* texture, engine::GrassScheme* grassScheme, float fadeStart, float fadeEnd);
    virtual ~Grass();
    // IRendering
    virtual void __stdcall release(void);
    virtual void __stdcall setProperty(const char* propertyName, float value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector3f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector2f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Vector4f& value);
    virtual void __stdcall setProperty(const char* propertyName, const Matrix4f& value);
    // Lostable
    virtual void onLostDevice(void);
    virtual void onResetDevice(void);
    // Rendering
    void render(void);
};

/**
 * auxially methods
 */

inline float getUniform(float a, float b)
{
    return a + ( b - a ) * float( rand() ) / RAND_MAX;
}

#endif