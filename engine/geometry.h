/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description geometry etc
 *
 * @author bad3p
 */

#ifndef GEOMETRY_IMPLEMENTATION_INCLUDED
#define GEOMETRY_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "shader.h"
#include "mesh.h"
#include "hash.h"

/**
 * hash support
 */

const double fuzzyEpsilon = 0.0000001;

__forceinline bool fuzzyEq(Vector* v1, Vector* v2)
{
    return fabs( D3DXVec3LengthSq( v1 ) - D3DXVec3LengthSq( v2 ) ) < fuzzyEpsilon;
}

__forceinline unsigned int hashCode(Vector* vector)
{
   unsigned int xhash = (*(int*)(void*)(&vector->x));
   unsigned int yhash = (*(int*)(void*)(&vector->y));
   unsigned int zhash = (*(int*)(void*)(&vector->z));
   return xhash + (yhash * 37) + (zhash * 101);
}

/**
 * geometry triangle
 */

struct Triangle
{
public:
    WORD  vertexId[3];
    DWORD shaderId;
public:
    inline void set(WORD v0, WORD v1, WORD v2, DWORD sh)
    {
        vertexId[0] = v0, vertexId[1] = v1, vertexId[2] = v2;
        shaderId = sh;
    }
};

/**
 * geometry edge
 */

struct Edge
{
public:
    int  triangleId[2];
    WORD vertexId[2];
};

class EdgeHash : public Hashable
{
public:        
    WORD    indices[2];
    Vector* vertices;
    unsigned int hash;
public:
    EdgeHash();
    EdgeHash(int idx0, int idx1, Vector* v);    
public:
    virtual unsigned int hashCode() const { return hash; }
    bool operator==(const EdgeHash& rhs);    
};

/**
 * octal tree, used to space partitioning for collision detection
 */

class Geometry;
class OcTreeSector;

#define OCTREESECTOR_CAPACITY 16    /* limit of triangles per octree sector */
#define OCTREESECTOR_SIZE     25.0f /* limit of octree sector bounding box edge size */

typedef OcTreeSector* OcTreeSectorRayIntersectionCallBack(Line* ray, OcTreeSector* sector);

class OcTreeSector
{
private:
    friend class RayIntersection;
    friend class SphereIntersection;
private:
    struct Chunk
    {
        auid id;
        auid parentId;
        auid geometryId;
        AABB boundingBox;
        int  numTriangles;
    };
private:
    OcTreeSector*    _parent;
    AABB             _boundingBox;
    std::vector<int> _triangles;
    OcTreeSector*    _subtree[8];
    Geometry*        _geometry;    
public:
    OcTreeSector(Chunk* chunk, IResource* resource, AssetObjectM& assetObjects);
    OcTreeSector(OcTreeSector* parent, AABB boundingBox, Geometry* geometry);
    virtual ~OcTreeSector();
public:
    inline Geometry* getGeometry(void) { return _geometry; }
    inline int getNumTriangles(void) { return _triangles.size(); }
    inline int getTriangleId(int idx) { assert( idx>=0 && idx<int(_triangles.size()) ); return _triangles[idx]; }
public:
    int getOcTreeSize(void);
    bool checkConsistency(void);
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
    OcTreeSector* forAllIntersections(Line* ray, OcTreeSectorRayIntersectionCallBack* callBack);
};

/**
 * IGeometry implementation
 */

class Geometry : public engine::IGeometry
{
private:
    struct Chunk
    {
        auid id;
        char name[engine::maxNameLength];
        int  numVertices;
        int  numTriangles;
        int  numUVSets;
        int  numShaders;
        int  numPrelights;
        int  numOcTreeSectors;
        bool sharedShaders;
        bool hasEffect;
        bool hasSkin;
    };
private:
    friend class Atomic;
    friend class Batch;
    friend class BSPSector;
private:
    int                _numReferences;
    std::string        _name;
    Sphere             _boundingSphere;
    AABB               _boundingBox;
    int                _numVertices;
    int                _numTriangles;
    int                _numUVSets;
    int                _numShaders;
    int                _numPrelights;
    bool               _sharedShaders;
    Vector*            _vertices;
    Vector*            _skinnedVertices; // for software skinning
    Vector*            _normals;
    Flector*           _uvs[engine::maxTextureLayers];
    Color*             _prelights[engine::maxPrelightLayers];
    Triangle*          _triangles;
    Shader**           _shaders;
    D3DVERTEXELEMENT9* _vertexDeclaration;
    OcTreeSector*      _ocTreeRoot;
    Mesh*              _mesh;
    void*              _effect;
    std::vector<Edge>  _edges; // computational structure
private:
    static OcTreeSector* onOcTreeSectorRayIntersection(Line* ray, OcTreeSector* sector);
private:
    void captureMeshData(bool captureShaders);
    void addEdge(Table<EdgeHash,int>& edgeTable, std::vector<Edge>& edgeVector, int v0, int v1, int face);
public:
    // class implementation    
    Geometry(
        int  numVertices,
        int  numTriangles,
        int  numUVSets,
        int  numShaders,
        int  numPrelights,
        bool sharedShaders,
        const char* geometryName
    );
    Geometry(
        Mesh* mesh,
        const char* geometryName
    );
    virtual ~Geometry();
public:
    // IGeometry
    virtual int __stdcall getNumReferences(void);
    virtual void __stdcall release(void);
    virtual const char* __stdcall getName(void);
    virtual Vector3f __stdcall getAABBInf(void);
    virtual Vector3f __stdcall getAABBSup(void);
    virtual int __stdcall getNumShaders(void);
    virtual engine::IShader* __stdcall getShader(int shaderId);
    virtual void __stdcall setShader(int shaderId, engine::IShader* shader);
    virtual engine::Mesh* __stdcall createMesh(void);
    virtual engine::Mesh* __stdcall getMeshInfo(void);
    virtual void __stdcall setMesh(engine::Mesh* mesh);
    virtual void __stdcall forcePrelight(const Vector4f& color);
    virtual engine::IEffect* __stdcall getEffect(void);
    virtual void __stdcall setEffect(engine::IEffect* effect);
    virtual bool __stdcall isBorderEdge(int triangleId, int edgeId);
    virtual int __stdcall getNumFaces(void);
    virtual void __stdcall getFace(int faceId, Vector3f& v0, Vector3f& v1, Vector3f& v2, engine::IShader** shader);
    virtual void __stdcall generateSkinTangents(void);
public:
    // module locals : inlines
    inline int getNumVertices(void) { return _numVertices; }
    inline int getNumTriangles(void) { return _numTriangles; }
    inline int getNumUVSets(void) { return _numUVSets; }
    inline int getNumPrelights(void) { return _numPrelights; }
    inline int getNumEdges(void) { return _edges.size(); }
    inline Vector* getVertices(void) { return _vertices; }    
    inline Vector* getNormals(void) { return _normals; }
    inline Flector* getUVSet(int id) { assert( id>=0 && id<_numUVSets ); return _uvs[id]; }
    inline Color* getPrelights(int id) { assert( id>=0 && id<_numPrelights ); return _prelights[id]; }
    inline Triangle* getTriangles(void) { return _triangles; }
    inline AABB* getBoundingBox(void) { return &_boundingBox; }
    inline Sphere* getBoundingSphere(void) { return &_boundingSphere; }
    inline OcTreeSector* getOcTreeRoot(void) { return _ocTreeRoot; }
    inline Shader* shader(int id) { assert( id>=0 && id<_numShaders ); return _shaders[id]; }
    inline Mesh* mesh(void) { return _mesh; }
    Vector* getSkinnedVertices(void);
    Edge* getEdges(void);
public:
    // module locals
    void setShaders(Shader** shaders);
    void setShader(int id, Shader* shader);
    void instance(void);
    void render(void);
    void renderDepthMap(void);
    void renderAlphaGeometry(unsigned int subsetId);
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
};

#endif
