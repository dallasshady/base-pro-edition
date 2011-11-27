/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2007-2008
 *
 * @description batch of geometries
 *
 * @author bad3p
 */

#ifndef BATCH_IMPLEMENTATION_INCLUDED
#define BATCH_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "geometry.h"

/**
 * batch with no certain implementation
 */

class Batch : public engine::IBatch
{
protected:
    // level of details
    struct Lod
    {
    public:
        Geometry*     lodGeometry;   // the geometry of LOD
        unsigned int  lodSize;       // current number of instances in this LOD
        float         lodSqDistance; // squared distance of LOD
        Matrix*       lodMatrices;   // instance matricex
        Quartector*   lodColors;     // instance colors
    public:
        Lod() : lodGeometry(NULL), lodSize(0), lodSqDistance(0), lodMatrices(NULL) {}
    };
protected:
    // batch sector
    struct Sector
    {
    public:
        typedef std::set<unsigned int> DynamicIndices;
        typedef DynamicIndices::iterator DynamicIndex;
    public:
        AABB          boundingBox;    // sector bounds
        Sphere        boundingSphere; // alternative bounds
        Sector*       left;           // left subset
        Sector*       right;          // right subset
        unsigned int  numIndices;     // number of instances in sector
        unsigned int* indices;        // indices of instances
    public:
        Sector(IResource* resource);
        Sector(unsigned int leafSize, Geometry* geometryLod0, Matrix* instances, AABB* aabb, DynamicIndices& parentIndices, float treeProgress=0.0f, float sectorProgress=1.0f);
        ~Sector();
    public:
        void render(float maxDistance);
        void write(IResource* resource);
        unsigned int getNumInstancesInHierarchy(void);
        void forAllInstancesInAABB(Geometry* geometryLod0, Matrix* instances, AABB* aabb, engine::IBatchCallback callback, void* data);
    public:        
        static Sector* createTree(unsigned int leafSize, Geometry* geometryLod0, Matrix* instances, unsigned int numInstances);
    };
protected:
    unsigned int         _batchSize;                  // number of instances
    engine::BatchScheme  _batchScheme;                // rendering scheme
    Lod                  _lods[engine::maxBatchLods]; // lods
    Matrix*              _matrices;                   // instance transformations
    Quartector*          _colors;                     // instance colors
    Sector*              _rootSector;                 // speedup structure
protected:
    static ID3DXEffect*    _effect;
    static StaticLostable* _effectLostable;
    static unsigned int    _numGlobalReferences;
private:
    static void onLostDeviceStatic(void);
    static void onResetDeviceStatic(void);
protected:
    // LOD builders
    void updateLODs(void);
    void updateLODs(Sector* sector);
public:
    // class implementation
    Batch(unsigned int batchSize, engine::BatchScheme* batchScheme);
    virtual ~Batch();
public:
    // IBatch
    virtual void __stdcall release(void);
    virtual unsigned int __stdcall getBatchSize(void);
    virtual unsigned int __stdcall getLodSize(unsigned int lodId);
    virtual engine::BatchScheme* __stdcall getBatchScheme(void);
    virtual Matrix4f __stdcall getMatrix(unsigned int batchId);
    virtual void __stdcall setMatrix(unsigned int batchId, const Matrix4f& matrix);
    // IBatch : spatial optimization
    virtual void __stdcall createBatchTree(unsigned int leafSize, const char* resourceName);
    virtual void __stdcall forAllInstancesInAABB(Vector3f aabbInf, Vector3f aabbSup, engine::IBatchCallback callback, void* data);
public:
    // module locals : inlines
    inline Geometry* geometry(unsigned int gId) { assert( gId<_batchScheme.numLods ); return _lods[gId].lodGeometry; }
    inline Matrix* matrices(void) { return _matrices; }
public:
    // local virtuals
    virtual void render(void) = 0;
};

/**
 * shader batch
 */

class ShaderBatch : public Batch,
                    virtual public Lostable
{
public:
    // format of the vertices for shader instancing
    struct InstanceVertex
    {
        Vector  pos;      // untransformed position
        Vector  normal;   // untransformed normal
        Flector uv;       // texture coordinates 
        float   instance; // instance id
    };    
private:
    IDirect3DVertexDeclaration9* _vertexDeclaration;
    IDirect3DVertexBuffer9*      _vbModel[engine::maxBatchLods];
    IDirect3DIndexBuffer9*       _ibModel[engine::maxBatchLods];
private:
    // speed-up scheme rendering
    void renderNoLODs(void);
    void renderLODs(void);    
public:
    // class implementation
    ShaderBatch(unsigned int batchSize, engine::BatchScheme* batchScheme);
    virtual ~ShaderBatch();
public:
    // local virtuals
    virtual void render(void);
public:
    // Lostable
    virtual void onLostDevice(void);
    virtual void onResetDevice(void);
};

/**
 * hardware batch
 */

class HardwareBatch : public Batch,
                      virtual public Lostable
{
public:
    // format of the vertices for hardware instancing
    struct InstanceVertex
    {
        Vector  pos;    // untransformed position
        Vector  normal; // untransformed normal
        Flector uv;     // texture coordinates
    };
    // format of the instance data for hardware instancing
    struct Instance
    {
        Matrix     matrix;
        Quartector color;
    };
private:
    IDirect3DVertexDeclaration9* _vertexDeclaration; // shared vertex declaration    
    IDirect3DVertexBuffer9*      _vbModel; // shared vertex buffer
    IDirect3DIndexBuffer9*       _ibModel; // shared index buffer    
    unsigned int                 _startVertex[engine::maxBatchLods]; // start vertex of LOD
    unsigned int                 _startIndex[engine::maxBatchLods]; // start index of LOD    
    unsigned int                 _vbInstanceSize; // size of instance buffer
    IDirect3DVertexBuffer9*      _vbInstance;     // instance buffer
private:
    // speed-up scheme rendering
    void renderNoLODs(void);
    void renderLODs(void);    
public:
    // class implementation
    HardwareBatch(unsigned int batchSize, engine::BatchScheme* batchScheme);
    virtual ~HardwareBatch();
public:
    // module locals
    virtual void render(void);
public:
    // Lostable
    virtual void onLostDevice(void);
    virtual void onResetDevice(void);
};

#endif
