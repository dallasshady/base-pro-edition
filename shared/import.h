/**
 * This source code is a part of AqWARium game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description Import interfaces
 *
 * @author bad3p
 */

#ifndef IMPORT_INTERFACES_INCLUDED
#define IMPORT_INTERFACES_INCLUDED

#include "ccor.h"
#include "matrix.h"

namespace import
{

/**
 * import unique identifier
 */

typedef int iuid;

/**
 * interface for import stream
 */

enum ImportType
{
    itNULL,        // no data to import
    itTexture,     // texture data
    itMaterial,    // material data
    itFrame,       // transformation frame data
    itGeometry,    // geometry data
    itAtomic,      // atomic data
    itClump,       // clump data
    itWorldSector, // world sector data
    itWorld,       // world data
    itLight,       // light data
};

enum ImportAddressMode
{
    iamWrap,
    iamMirror,
    iamClamp,
    iamBorder
};

enum ImportFilterMode
{
    ifmNearest,
    ifmLinear,
    ifmMipNearest,
    ifmMipLinear,
    ifmLinearMipNearest,
    ifmLinearMipLinear
};

struct ImportTexture
{
public:
    iuid              id;
    char*             name;
    int               width;
    int               height;
    int               depth;
    unsigned char*    pixels;
    int               stride;
    ImportAddressMode addressMode;
    ImportFilterMode  filterMode;
public:
    ImportTexture()
    {
        id = 0;
        name = NULL;
        width = height = depth = 0;
        pixels = NULL;
        stride = 0;
    }
};

struct ImportMaterial
{
public:
    enum BlendType
    {
        btOver,
        btAdd,
        btModulate,
        btBlendTextureAlpha
    };
public:
    iuid      id;
    char*     name;
    Vector4f  color;
    iuid      textureId;
    iuid      dualPassTextureId;
    BlendType dualPassBlendType;
public:
    ImportMaterial()
    {
        id = 0;
        name = NULL;
        color.set( 0,0,0,0 );
        textureId = 0;
        dualPassTextureId = 0;
    }
};

struct ImportFrame
{
public:
    iuid     id;
    char*    name;
    iuid     parentId;
    Matrix4f modeling;
public:
    ImportFrame()
    {
        id = 0;
        name = NULL;
        parentId = 0;
        modeling.set( 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0 );
    }
};

struct Triangle
{
public:
    int vertexId[3];
    int materialId;
public:
    Triangle()
    {
        vertexId[0] = vertexId[1] = vertexId[2] = materialId = 0;
    }
};

struct ImportGeometry
{
public:
    iuid        id;
    char*       name;
    int         numVertices;
    int         numTriangles;
    int         numUVs;
    int         numMaterials;
    Vector3f*   vertices;
    Vector3f*   normals;
    Vector2f*   uvs[8];
    Vector4f*   prelights;
    Triangle*   triangles;
    iuid*       materials;
public:
    ImportGeometry()
    {
        id = 0;
        name = NULL;
        numVertices = numTriangles = numUVs = numMaterials = 0;
        vertices = normals = NULL;
        uvs[0] = uvs[1] = uvs[2] = uvs[3] = uvs[4] = uvs[5] = uvs[6] = uvs[7] = NULL;
        prelights = NULL;
        triangles = NULL;
        materials = NULL;
    }
};

struct ImportAtomic
{
public:
    iuid id;
    iuid geometryId;
    iuid frameId;
    iuid lightmapId;
public:
    ImportAtomic()
    {
        id = geometryId = frameId = 0;
    }
};

struct ImportClump
{
public:
    iuid  id;
    char* name;
    int   numAtomics;
    iuid* atomics;
    int   numLights;
    iuid* lights;
    iuid  frameId; 
public:
    ImportClump()
    {
        id = 0;
        name = NULL;
        numAtomics = 0;
        numLights = 0;
        atomics = NULL;
        lights = NULL;
    }
};

struct ImportWorldSector
{
public:
    iuid       id;
    iuid       parentId;
    iuid       worldId;
    int        numVertices;
    int        numTriangles;
    int        numUVs;
    Vector3f*  vertices;
    Vector3f*  normals;
    Vector2f*  uvs[8];
    Vector4f*  prelights;
    Triangle*  triangles;
    Vector3f   aabbInf;
    Vector3f   aabbSup;
    iuid       lightmapId;
public:
    ImportWorldSector()
    {
        id = parentId = 0;
        numVertices = numTriangles = numUVs = 0;
        vertices = normals = NULL;
        uvs[0] = uvs[1] = uvs[2] = uvs[3] = uvs[4] = uvs[5] = uvs[6] = uvs[7] = NULL;
        prelights = NULL;
        triangles = NULL;
        aabbInf = aabbSup = Vector3f( 0,0,0 );
    }
};

struct ImportWorld
{
public:
    iuid     id;
    char*    name;
    int      numMaterials;
    iuid*    materials;
    Vector3f aabbInf;
    Vector3f aabbSup;
public:
    ImportWorld()
    {
        id = 0, name = NULL, numMaterials = 0, materials = NULL;
        aabbInf = aabbSup = Vector3f( 0,0,0 );
    }
};

struct ImportLight
{
public:
    enum LightType
    {
        ltDirectional,
        ltAmbient,
        ltPoint,
        ltSpot
    };
public:
    iuid      id;
    iuid      frameId;
    LightType type;
    float     radius;
    Vector4f  color;
    float     coneAngle;
};

class IImportStream
{
public:
    /**
     * importing routine,
     */
    virtual ImportType __stdcall getType(void) = 0;
    virtual ImportTexture* __stdcall importTexture(void) = 0;
    virtual ImportMaterial* __stdcall importMaterial(void) = 0;
    virtual ImportFrame* __stdcall importFrame(void) = 0;
    virtual ImportGeometry* __stdcall importGeometry(void) = 0;
    virtual ImportAtomic* __stdcall importAtomic(void) = 0;
    virtual ImportClump* __stdcall importClump(void) = 0;
    virtual ImportWorldSector* __stdcall importWorldSector(void) = 0;
    virtual ImportWorld* __stdcall importWorld(void) = 0;
    virtual ImportLight* __stdcall importLight(void) = 0;
};

/**
 * interface for import entity
 */

typedef void (*LightMapProgressCallback)(float progress, void* userData);

class IImport : public ccor::IBase 
{
public:
    DECLARE_INTERFACE_ID(0x20002);
public:
	/**
	 * generate handness matrix
	 */
	virtual Matrix4f __stdcall generateHandnessMatrix(const Vector3f& translate, const Vector3f& axis, float angle) = 0;
public:
    /**
     * process import from rws resource
     */
    virtual IImportStream* __stdcall importRws(const char* resourceName) = 0;    
public:
    /**
     * generates normal map from first resource and put it to second resource
     */
    virtual void __stdcall createNormalMap(
        const char* resourceName1,
        const char* resourceName2,
        float bumpiness
    ) = 0;
    /**
     * generates normal map by input pixels (pixels are in Import format)
     */
    virtual void __stdcall createNormalMap(
        int               Width,
        int               Height,
        int               Depth,
        unsigned char*    Pixels,
        int               stride,
        bool              clamp,
        float             bumpiness
    ) = 0;
public:
    virtual void __stdcall calculateLightMaps(
        const char* resourceName, 
        LightMapProgressCallback callback,
        void* userData
    ) = 0;
public:
    /**
     * release routine
     */
    virtual void __stdcall release(ImportTexture* importData) = 0;
    virtual void __stdcall release(ImportMaterial* importData) = 0;
    virtual void __stdcall release(ImportFrame* importData) = 0;
    virtual void __stdcall release(ImportGeometry* importData) = 0;
    virtual void __stdcall release(ImportAtomic* importData) = 0;
    virtual void __stdcall release(ImportClump* importData) = 0;
    virtual void __stdcall release(ImportWorldSector* importData) = 0;
    virtual void __stdcall release(ImportWorld* importData) = 0;
    virtual void __stdcall release(ImportLight* importData) = 0;
    virtual void __stdcall release(IImportStream* importStream) = 0;    
};

}

#endif
