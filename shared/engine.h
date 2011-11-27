/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description Engine interfaces
 *
 * @author bad3p
 */

#ifndef ENGINE_INTERFACES_INCLUDED
#define ENGINE_INTERFACES_INCLUDED

#include "ccor.h"
#include "matrix.h"
#include "vector.h"

/**
 * graphics engine
 */

namespace engine 
{

using namespace ccor;

const maxNameLength = 256;

/**
 * common definitions
 */

enum CompareFunction
{
    cfNever,        // accept new value if it is less than current value
    cfEqual,        // test will always fail
    cfLess,         // accept new value if it is equal to the current
    cfLessEqual,    // accept new value if it is less or equal to the current
    cfGreater,      // accept new value if it is greater than current
    cfNotEqual,     // accept new value if it isn't equal to the current
    cfGreaterEqual, // accept new value if it is greater or equal to the current
    cfAlways        // test will always success
};

/**
 * texture management routine
 */

enum AddressType
{
    atWrap,   // tile the texture at every integer junction
    atMirror, // atWrap + texture is flipped at every integer junction
    atClamp,  // uv's outside the range [0,1] are set to the texture color at 0 or 1, respectively
    atBorder, // texture coordinates outside the range [0,1] are set to the border color
};

enum FilterType
{
    ftNone,        // disable mipmapping
    ftPoint,       // point sampled
    ftLinear,      // bilinear interpolation
    ftAnisotropic, // anisotropic distortion compensation
    ftPyramidal,   // a 4-sample tent filter 
    ftGaussian     // a 4-sample Gaussian filter
};

enum CubemapFace
{
    cfPositiveX = 0,
    cfNegativeX = 1,
    cfPositiveY = 2,
    cfNegativeY = 3,
    cfPositiveZ = 4,
    cfNegativeZ = 5
};

class ITexture : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30000);
public:
    /**
     * system management routine
     */
    virtual void __stdcall addReference(void) = 0;
    virtual int __stdcall getNumReferences(void) = 0;
    virtual void __stdcall release(void) = 0;
    virtual void __stdcall save(const char* resourceName) = 0;
public:
    /**
     * texture properties
     */
    virtual const char* __stdcall getName(void) = 0;
    virtual int __stdcall getWidth(void) = 0;
    virtual int __stdcall getHeight(void) = 0;
public:
    /**
     * texture controls
     */
    virtual AddressType __stdcall getAddressTypeU(void) = 0;
    virtual void __stdcall setAddressTypeU(AddressType addressType) = 0;
    virtual AddressType __stdcall getAddressTypeV(void) = 0;
    virtual void __stdcall setAddressTypeV(AddressType addressType) = 0;
    virtual Vector4f __stdcall getBorderColor(void) = 0;
    virtual void __stdcall setBorderColor(const Vector4f& borderColor) = 0;
    virtual FilterType __stdcall getMagFilter(void) = 0;
    virtual void __stdcall setMagFilter(FilterType filterType) = 0;
    virtual FilterType __stdcall getMinFilter(void) = 0;
    virtual void __stdcall setMinFilter(FilterType filterType) = 0;
    virtual FilterType __stdcall getMipFilter(void) = 0;
    virtual void __stdcall setMipFilter(FilterType filterType) = 0;
    virtual int __stdcall getMaxAnisotropy(void) = 0;
    virtual void __stdcall setMaxAnisotropy(int maxAnisotropy) = 0;
    virtual int __stdcall getMaxMipLevel(void) = 0;
    virtual void __stdcall setMaxMipLevel(int maxMipLevel) = 0;
    virtual float __stdcall getMipmapLODBias(void) = 0;
    virtual void __stdcall setMipmapLODBias(float lodBias) = 0;
};

/**
 * effect system
 * 
 * engine effect is a combination of fixed function pipeline, and hardware 
 * vertex, and pixel shaders, used to vary common rendering of scene, or certain 
 * scene geometry, or certain scene shader
 */

class IGeometry;
class IShader;

class IEffect : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30010);
public:
    virtual const char* __stdcall getName(void) = 0;
    virtual int __stdcall getNumArguments(void) = 0;
    virtual const char* __stdcall getArgumentName(int argId) = 0;
    virtual Variant __stdcall getArgument(int argId) = 0;
    virtual void __stdcall setArgument(int argId, const Variant& value) = 0;
    virtual void __stdcall release(void) = 0;
    virtual bool __stdcall isCompatible(IShader* shader) = 0;
    virtual bool __stdcall isCompatible(IGeometry* geometry) = 0;
};

/**
 * shader system
 */

const int maxTextureLayers = 4;
const int maxPrelightLayers = 1;

// layer blending type
enum BlendingType
{
    btOver,              // L2
    btAdd,               // L1 + L2
    btSub,               // L1 - L2
    btAddSigned,         // L1 + L2 - 0.5
    btAddSmooth,         // L1 + L2 - L1 * L2
    btModulate,          // L1 * L2
    btModulate2x,        // L1 * L2 * 2
    btModulate4x,        // L1 * L2 * 4
    btBlendTextureAlpha, // L1 * (L2alpha) + L2 * (1-L2alpha)
    btLerp               // L1 * L2 + (1 - L1) * LerpFactor (single stage allowable)
};

// framebuffer blending mode
enum BlendingMode
{ 
    bmZero,
    bmOne,
    bmSrcColor,
    bmInvSrcColor,
    bmSrcAlpha,
    bmInvSrcAlpha,
    bmDestAlpha,
    bmInvDestAlpha,
    bmDestColor,
    bmInvDestColor,
    bmSrcAlphaSat
};

// framebuffer blending operation
enum BlendingOperation
{
    bpAdd,
    bpSubtract,
    bpRevSubtract,
    bpMin,
    bpMax
};

// shader flags
enum ShaderFlag
{
    sfCulling           = 1,   // backface culling is enabled
    sfCollision         = 2,   // collision detection is enabled
    sfAlphaBlending     = 4,   // alpha-blending is enabled
    sfAlphaTesting      = 8,   // alpha-testing is enabled
    sfHemisphere        = 16,  // hemisphere ambient is enabled for shading technique
    sfCaching           = 32,  // shader can use caching routine
    sfLighting          = 64,  // shader should use lighting (FFP only)
    sfCinematicLighting = 128  // cinematic lighting is enabled for shading technique
};

class IShader : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30003);
public:
    /**
     * system management routine
     */
    virtual void __stdcall addReference(void) = 0;
    virtual int __stdcall getNumReferences(void) = 0;
    virtual void __stdcall release(void) = 0;    
public:
    /**
     * shader properties
     */
    virtual const char* __stdcall getName(void) = 0;
public:
    /**
     * shader texture layers manipulation
     */
    virtual int __stdcall getNumLayers(void) = 0;
    virtual void __stdcall setNumLayers(int numLayers, ITexture* defaultTexture) = 0;
    virtual BlendingType __stdcall getLayerBlending(int layerId) = 0;
    virtual void __stdcall setLayerBlending(int layerId, BlendingType blending) = 0;
    virtual ITexture* __stdcall getLayerTexture(int layerId) = 0;
    virtual void __stdcall setLayerTexture(int layerId, ITexture* texture) = 0;
    virtual int __stdcall getLayerUV(int layerId) = 0;
    virtual void __stdcall setLayerUV(int layerId, int uvId) = 0;
    virtual Vector4f __stdcall getLayerConstant(int layerId) = 0;
    virtual void __stdcall setLayerConstant(int layerId, const Vector4f& layerConstant) = 0;
    virtual Vector4f __stdcall getDiffuseColor(void) = 0;
    virtual void __stdcall setDiffuseColor(const Vector4f& value) = 0;
    virtual Vector4f __stdcall getSpecularColor(void) = 0;
    virtual void __stdcall setSpecularColor(const Vector4f& value) = 0;
    virtual float __stdcall getSpecularPower(void) = 0;
    virtual void __stdcall setSpecularPower(float value) = 0;
    virtual ITexture* __stdcall getNormalMap(void) = 0;
    virtual void __stdcall setNormalMap(ITexture* texture) = 0;
    virtual int __stdcall getNormalMapUV(void) = 0;
    virtual void __stdcall setNormalMapUV(int normalMapUV) = 0;
    virtual ITexture* __stdcall getEnvironmentMap(void) = 0;
    virtual void __stdcall setEnvironmentMap(ITexture* texture) = 0;    
    /**
     * flags & flags options
     */
    virtual unsigned int __stdcall getFlags(void) = 0;
    virtual void __stdcall setFlags(unsigned int flags) = 0;
    virtual BlendingMode __stdcall getSrcBlend(void) = 0;
    virtual void __stdcall setSrcBlend(BlendingMode blendMode) = 0;
    virtual BlendingMode __stdcall getDestBlend(void) = 0;
    virtual void __stdcall setDestBlend(BlendingMode blendMode) = 0;
    virtual BlendingOperation __stdcall getBlendOp(void) = 0;
    virtual void __stdcall setBlendOp(BlendingOperation blendOp) = 0;
    virtual CompareFunction __stdcall getAlphaTestFunction(void) = 0;
    virtual void __stdcall setAlphaTestFunction(CompareFunction function) = 0;
    virtual unsigned char __stdcall getAlphaTestRef(void) = 0;
    virtual void __stdcall setAlphaTestRef(unsigned char alphaRef) = 0;
    virtual unsigned int __stdcall getLightset(void) = 0;
    virtual void __stdcall setLightset(unsigned int value) = 0;
    /**
     * effect
     */
    virtual IEffect* __stdcall getEffect(void) = 0;
    virtual bool __stdcall setEffect(IEffect* effect) = 0;
    /**
     * hemisphere ambient
     */
    virtual Vector4f __stdcall getUpperHemisphere(void) = 0;
    virtual Vector4f __stdcall getLowerHemisphere(void) = 0;
    virtual void __stdcall setHemisphere(Vector4f upperColor, Vector4f lowerColor) = 0;
    /**
     * cinematic lighting
     *
     * this lighting is consisnts of two directional light sources:
     * - illumination light
     *   - do soften a shadows of main light source.
     *   - the direction of illumination light is equals to direction of camera's eye 
     * - contour light 
     *   - do draw a character's outline
     *   - the direction of contour light is customizable
     *
     * see also IAtomic::setContourDirection()
     */
    virtual Vector4f __stdcall getIlluminationColor(void) = 0;
    virtual Vector4f __stdcall getContourColor(void) = 0;
    virtual void __stdcall setIlluminationColor(const Vector4f& value) = 0;
    virtual void __stdcall setContourColor(const Vector4f& value) = 0;
};

/**
 * transformation frame
 */

const  unsigned int maxDirtyFrames = 4096;

class IFrame;

typedef IFrame* (*IFrameCallBack)(IFrame* frame, void* data);

class IFrame : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30002);
public:
    /**
     * system management routine
     */
    virtual void __stdcall release(void) = 0;
public:
    /**
     * frame properties
     */
    virtual const char* __stdcall getName(void) = 0;
    virtual Matrix4f __stdcall getMatrix(void) = 0;
    virtual Matrix4f __stdcall getLTM(void) = 0;
    virtual Vector3f __stdcall getRight(void) = 0;
    virtual Vector3f __stdcall getUp(void) = 0;
    virtual Vector3f __stdcall getAt(void) = 0;
    virtual Vector3f __stdcall getPos(void) = 0;
public:
    /**
     * frame manipulation
     */
    virtual void __stdcall setPos(const Vector3f& pos) = 0;
    virtual void __stdcall setMatrix(const Matrix4f& matrix) = 0;
    virtual void __stdcall rotate(const Vector3f& axis, float angle) = 0;
    virtual void __stdcall rotateRelative(const Vector3f& axis, float angle) = 0;
    virtual void __stdcall translate(const Vector3f& translation) = 0;    
    virtual void __stdcall setLTM(const Matrix4f& matrix) = 0;
public:
    /**
     * frame hierarchy
     */
    virtual IFrame* __stdcall getParent(void) = 0;
    virtual void __stdcall setParent(IFrame* frame) = 0;
    virtual void __stdcall forAllChildren(IFrameCallBack callBack, void* data) = 0;
};

/**
 * geometry
 */

struct Mesh
{
public:
    struct Triangle
    {
        unsigned int vertexId[3];
        unsigned int shaderId;
    };
public:
    unsigned int numVertices;
    unsigned int numTriangles;
    unsigned int numUVs;
    Vector3f*    vertices;
    Triangle*    triangles;
    Vector2f*    uvs[maxTextureLayers];
public:
    Mesh() : numVertices(0), numTriangles(0), numUVs(NULL), vertices(NULL), triangles(NULL) 
    {
        for( unsigned int i=0; i<maxTextureLayers; i++ )
        {
            uvs[i] = NULL;
        }
    }
};

class IGeometry : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30004);
public:
    /**
     * system management routine
     */
    virtual int __stdcall getNumReferences(void) = 0;
    virtual void __stdcall release(void) = 0;
public:
    /**
     * geometry properties
     */
    virtual const char* __stdcall getName(void) = 0;
    virtual Vector3f __stdcall getAABBInf(void) = 0;
    virtual Vector3f __stdcall getAABBSup(void) = 0;
    virtual int __stdcall getNumShaders(void) = 0;
    virtual IShader* __stdcall getShader(int shaderId) = 0;
    virtual void __stdcall setShader(int shaderId, engine::IShader* shader) = 0;
    virtual Mesh* __stdcall getMeshInfo(void) = 0;
    virtual Mesh* __stdcall createMesh(void) = 0;
    virtual void __stdcall setMesh(Mesh* mesh) = 0;
    virtual void __stdcall forcePrelight(const Vector4f& color) = 0;
    virtual int __stdcall getNumFaces(void) = 0;
    virtual void __stdcall getFace(int faceId, Vector3f& v0, Vector3f& v1, Vector3f& v2, IShader** shader) = 0;
    virtual void __stdcall generateSkinTangents(void) = 0;
public:
    /**
     * geometry effects
     */
    virtual IEffect* __stdcall getEffect(void) = 0;
    virtual void __stdcall setEffect(IEffect* effect) = 0;
public:
    /**
     * determine if triangle edge is border edge
     * edge vertices are:
     *  ( edgeId == 0 ) : ( 0,1 )
     *  ( edgeId == 1 ) : ( 0,2 )
     *  ( edgeId == 2 ) : ( 1,2 )
     */
    virtual bool __stdcall isBorderEdge(int triangleId, int edgeId) = 0;
};

/**
 * arbitrary-driven particle system
 */

struct Particle
{
public:
    bool     visible;   // particle is visible
    Vector3f position;  // worldspace position of a particle
    Vector3f direction; // worldspace direction of a particle
    float    rotation;  // particle rotation
    Vector2f size;      // particle size
    Vector4f color;     // particle color
    Vector2f uv[4];     // particle uv's
};

class IParticleSystem : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x3001e);
public:
    /**
     * system management routine
     */
    virtual void __stdcall release(void) = 0;
public:
    /**
     * emitter control
     */
    virtual unsigned int __stdcall getNumParticles(void) = 0;
    virtual Particle* __stdcall getParticles(void) = 0;
    virtual Vector4f __stdcall getAmbient(void) = 0;
    virtual void __stdcall setAmbient(Vector4f color) = 0;    
};

/**
 * atomic (transformed geometry)
 */

enum AtomicFlags
{
    afRender      = 0x00000001l, // atomic will be rendered (visible)
    afCollision   = 0x00000002l, // atomic will be collided
    afCastShadow  = 0x00000004l, // atomic will cast stencil shadow
};

class IAtomic;

typedef IAtomic* (*IAtomicCallBack)(IAtomic* atomic, void* data);

class IAtomic : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30006);
public:
    /**
     * system management routine
     */
    virtual void __stdcall release(void) = 0;
public:
    /**
     * atomic properties
     */
    virtual unsigned int __stdcall getFlags(void) = 0;
    virtual void __stdcall setFlags(unsigned int flags) = 0;
    virtual IFrame* __stdcall getFrame(void) = 0;
    virtual void __stdcall setFrame(IFrame* frame) = 0;
    virtual IGeometry* __stdcall getGeometry(void) = 0;
    virtual void __stdcall setGeometry(IGeometry* geometry) = 0;
    virtual Vector3f __stdcall getAABBInf(void) = 0;
    virtual Vector3f __stdcall getAABBSup(void) = 0;
    virtual void __stdcall setUpdateTreshold(float distance, float radius) = 0;
    virtual ITexture* __stdcall getLightMap(void) = 0;
    virtual void __stdcall setLightMap(ITexture* lightMap) = 0;
    virtual Vector3f __stdcall getContourDirection(void) = 0;
    virtual void __stdcall setContourDirection(const Vector3f& value) = 0;
public:
    /**
     * rendering variation
     */
    virtual void __stdcall setRenderCallback(IAtomicCallBack callBack, void* data) = 0;
};

/**
 * batch (a large set of the identical and transformed geometries)
 */

const unsigned int maxBatchLods = 5;

enum BatchFlags
{
    bfCastShadow = 0x00000001l, // batch will cast stencil shadow
};

struct BatchScheme
{
public:
    unsigned int flags;                     // @see BatchFlags
    unsigned int numLods;                   // number of LODs
    IGeometry*   lodGeometry[maxBatchLods]; // geometry of LODs
    float        lodDistance[maxBatchLods]; // maximal distance of LOD    
public:
    BatchScheme() : numLods(0), flags(0)
    {
        for( unsigned int i=0; i<maxBatchLods; i++ )
        {
            lodGeometry[i] = NULL;
            lodDistance[i] = 0.0f;
        }
    }
public:
    // scheme validation check
    bool isValid(void)
    {
        for( unsigned int i=0; i<numLods; i++ )
        {
            if( lodGeometry[i] == NULL ) return false;
            if( i > 0 && lodDistance[i] < lodDistance[i-1] ) return false;
        }
        return true;
    }
};

typedef unsigned int (*IBatchCallback)(unsigned int id, Matrix4f* matrix, void* data);

class IBatch : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x0);
public:
    /**
     * system management routine
     */
    virtual void __stdcall release(void) = 0;
public:
    /**
     * batch properties
     */
    virtual unsigned int __stdcall getBatchSize(void) = 0;
    virtual unsigned int __stdcall getLodSize(unsigned int lodId) = 0;
    virtual BatchScheme* __stdcall getBatchScheme(void) = 0;
    virtual Matrix4f __stdcall getMatrix(unsigned int batchId) = 0;
    virtual void __stdcall setMatrix(unsigned int batchId, const Matrix4f& matrix) = 0;    
public:
    /**
     * spatial optimization
     */
    virtual void __stdcall createBatchTree(unsigned int leafSize, const char* resourceName) = 0;
    virtual void __stdcall forAllInstancesInAABB(Vector3f aabbInf, Vector3f aabbSup, IBatchCallback callback, void* data) = 0;
};

/**
 * light source
 */

enum LightType
{
    ltAmbient,
    ltPoint,
    ltSpot,
    ltDirectional
};

class ILight : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30009);
public:
    /**
     * system management routine
     */
    virtual void __stdcall release(void) = 0;
    /**
     * light position & orientation
     */
    virtual IFrame* __stdcall getFrame(void) = 0;
    virtual void __stdcall setFrame(IFrame* frame) = 0;
    /**
     * light poroperties
     */
    virtual LightType __stdcall getType(void) = 0;    
    virtual unsigned int __stdcall getLightset(void) = 0;
    virtual void __stdcall setLightset(unsigned int value) = 0;
    virtual Vector4f __stdcall getDiffuseColor(void) = 0;
    virtual void __stdcall setDiffuseColor(Vector4f value) = 0;
    virtual Vector4f __stdcall getSpecularColor(void) = 0;
    virtual void __stdcall setSpecularColor(Vector4f value) = 0;
    virtual float __stdcall getRange(void) = 0;
    virtual void __stdcall setRange(float value) = 0;
    virtual Vector3f __stdcall getAttenuation(void) = 0;
    virtual void __stdcall setAttenuation(const Vector3f& value) = 0;
    virtual float __stdcall getTheta(void) = 0;
    virtual void __stdcall setTheta(float theta) = 0;
    virtual float __stdcall getPhi(void) = 0;
    virtual void __stdcall setPhi(float phi) = 0;
};

typedef ILight* (*ILightCallBack)(ILight* light, void* data);

/**
 * animation controller
 */

const unsigned int maxAnimationTracks = 8;

enum LoopType
{
    ltNone,     // no loop
    ltMirror,   // mirror loop ( 0,1,2,1,0,1,2,1,0,... )
    ltPeriodic, // periodic loop ( 0,1,2,0,1,2,0,1,2,...)
};

struct AnimSequence
{
public:
    float    startTime;     // animation start time
    float    endTime;       // animation end time
    LoopType loopType;      // type of loop
    float    loopStartTime; // loop is starts from this time, end of loop is end of animation
};

struct WeightSet
{
public:
    float        weight[maxAnimationTracks]; // personal track weight for each animation (bone)
    unsigned int userData;                   // optional user data
};

typedef WeightSet* (*WeightSetCallBack)(const char* animationName, WeightSet* weightSet, void* data);

class IAnimationController : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30014);
public:
    /**
     * animation properties
     */
    virtual AnimSequence* __stdcall getDefaultAnimation(void) = 0;
public:
    /**
     * animation track controller
     */
    virtual bool __stdcall isBeginOfAnimation(unsigned int trackId) = 0;
    virtual bool __stdcall isEndOfAnimation(unsigned int trackId) = 0;
    virtual void __stdcall resetTrackTime(unsigned int trackId) = 0;
    virtual bool __stdcall getTrackActivity(unsigned int trackId) = 0;
    virtual void __stdcall setTrackActivity(unsigned int trackId, bool flag) = 0;
    virtual float __stdcall getTrackSpeed(unsigned int trackId) = 0;
    virtual void __stdcall setTrackSpeed(unsigned int trackId, float speed) = 0;
    virtual float __stdcall getTrackWeight(unsigned int trackId) = 0;
    virtual void __stdcall setTrackWeight(unsigned int trackId, float weight) = 0;
    virtual AnimSequence* __stdcall getTrackAnimation(unsigned int trackId) = 0;
    virtual void __stdcall setTrackAnimation(unsigned int trackId, AnimSequence* sequence) = 0;
    virtual void __stdcall copyTrack(unsigned int srcTrackId, unsigned int dstTrackId) = 0;
    virtual void __stdcall advance(float dt) = 0;
public:
    /**
     * animation blender
     */
    virtual void __stdcall captureBlendSrc(void) = 0;
    virtual void __stdcall captureBlendDst(void) = 0;
    virtual void __stdcall blend(float interpolator) = 0;
public:
    /**
     * weight set manipulation
     */
    virtual unsigned int __stdcall getNumAnimations(void) = 0;
    virtual void __stdcall createWeightSet(const char* weightSetName) = 0;
    virtual void __stdcall enableWeightSet(const char* weightSetName) = 0;
    virtual void __stdcall disableWeightSet(void) = 0;    
    virtual WeightSet* __stdcall getWeightSet(const char* weightSetName) = 0;
    virtual WeightSet* __stdcall getAnimationWeightSet(const char* weightSetName, const char* animationName) = 0;    
    virtual void __stdcall forAllAnimationWeightSets(const char* weightSetName, WeightSetCallBack callBack, void* data) = 0;
};

/**
 * clump (group of lights and atomics)
 */

class IBSP;

class IClump : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30007);
public:
    /**
     * system management routine
     */
    virtual void __stdcall release(void) = 0;
public:
    /**
     * clump manipulation
     */
    virtual const char* __stdcall getName(void) = 0;
    virtual void __stdcall setName(const char* name) = 0;
    virtual IBSP* __stdcall getBSP(void) = 0;
    virtual IFrame* __stdcall getFrame(void) = 0;
    virtual void __stdcall setFrame(IFrame* frame) = 0;
    virtual void __stdcall add(IAtomic* atomic) = 0;
    virtual void __stdcall add(ILight* light) = 0;
    virtual void __stdcall remove(IAtomic* atomic) = 0;
    virtual void __stdcall remove(ILight* light) = 0;
    virtual void __stdcall forAllAtomics(IAtomicCallBack callBack, void* data) = 0;    
    virtual void __stdcall forAllLights(ILightCallBack callBack, void* data) = 0;    
    virtual void __stdcall render(void) = 0;
    virtual IClump* __stdcall clone(const char* cloneName) = 0;
    virtual void __stdcall setLOD(IAtomic* atomic, float maxDistance, float minDistance) = 0;
public:
    /**
     * animation methods
     */
    virtual IAnimationController* __stdcall getAnimationController(void) = 0;
};

typedef IClump* (*IClumpCallBack)(IClump* clump, void* data);

/**
 * fog
 */

enum FogType
{
    fogNone,
    fogExp,
    fogExp2,
    fogLinear
};

/**
 * camera
 */

enum ClearMode
{
    cmClearColor   = 0x00000001l, /* clear color buffer */
    cmClearDepth   = 0x00000002l, /* clear depth buffer */
    cmClearStencil = 0x00000004l  /* clear stencil planes */
};

class ICamera : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30005);
public:
    /**
     * system management routine
     */
    virtual void __stdcall release(void) = 0;
public:
    /**
     * camera properties
     */
    virtual IFrame* __stdcall getFrame(void) = 0;
    virtual void __stdcall setFrame(IFrame* frame) = 0;
    virtual float __stdcall getNearClipPlane(void) = 0;
    virtual void __stdcall setNearClipPlane(float nearClip) = 0;
    virtual float __stdcall getFarClipPlane(void) = 0;
    virtual void __stdcall setFarClipPlane(float farClip) = 0;
    virtual float __stdcall getFOV(void) = 0;
    virtual void __stdcall setFOV(float fov) = 0;
public:
    /**
     * rendering controls
     */
    virtual void __stdcall beginScene(unsigned int clearMode, const Vector4f& clearColor) = 0;
    virtual void __stdcall endScene(void) = 0;
public:
    /**
     * render specified raster within specified rectangular area
     */
    virtual void __stdcall renderTexture(engine::ITexture* texture) = 0;
	virtual void __stdcall renderTextureAdditive(engine::ITexture* texture) = 0;
public:
    /**
     * raycasting: unproject specified ray in to the world space
     */
    virtual void __stdcall buildPickRay(float x, float y, Vector3f& start, Vector3f& dir) = 0;
    virtual Vector3f __stdcall projectPosition(const Vector3f& position) = 0;
};

/**
 * post-effect for camera
 */

enum PostEffectType
{                   
    pfxNone,       // [   ] no effect, just render-to-texture implementation
    pfxMotionBlur, // [W  ] simple motion blur
    pfxDOF,        // [WT ] depth-of-field (@see for IBSP::renderDepthMap() method)
    pfxBloom       // [W V] bright pass (W gives luminance) then bloom (V gives bloom value)
                   // -----------------------------------
                   // [W  ] - means that effect is uses weight argument
                   // [ T ] - means that effect is uses texture argument
                   // [  V] - means that effect is uses vector argument
};

class ICameraEffect : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30016);
public:
    /**
     * @return affected camera
     */
    virtual ICamera* __stdcall getCamera(void) = 0;
public:
    /**
     * controls the current effect
     */
    virtual PostEffectType __stdcall getPfx(void) = 0;
    virtual void __stdcall setPfx(PostEffectType pfxType) = 0;
    virtual unsigned int __stdcall getQuality(void) = 0 ;
    virtual void __stdcall setQuality(unsigned int quality) = 0 ;
public:
    /**
     * controls the weight of effect
     */
    virtual float __stdcall getWeight(void) = 0;
    virtual void __stdcall setWeight(float value) = 0;
    /**
     * controls the vector argument of effect
     */
    virtual Vector4f __stdcall getVector(void) = 0;
    virtual void __stdcall setVector(const Vector4f& value) = 0;
public:
    /**
     * controls the texture argument for effect
     */
    virtual ITexture* __stdcall getTexture(void) = 0;
    virtual void __stdcall setTexture(ITexture* texture) = 0;
public:
    /**
     * apply current effect to rendered image
     * invoke this method after the general beginScene/endScene sequences
     */
    virtual void __stdcall applyEffect(void) = 0;
};

/**
 * abstract rendering
 */

class IRendering
{
public:
    DECLARE_INTERFACE_ID(0x3001c);
public:
    virtual void __stdcall release(void) = 0;
    virtual void __stdcall setProperty(const char* propertyName, float value) = 0;
    virtual void __stdcall setProperty(const char* propertyName, const Vector2f& value) = 0;
    virtual void __stdcall setProperty(const char* propertyName, const Vector3f& value) = 0;
    virtual void __stdcall setProperty(const char* propertyName, const Vector4f& value) = 0;
    virtual void __stdcall setProperty(const char* propertyName, const Matrix4f& value) = 0;   
};

/**
 * BSP (binary space partition) is a complex of methods, used to manipulate the
 * large portions of static geometry; 
 *
 * these interfaces also used to generalise scenery management
 */

class IBSP;

class IBSPSector : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x3000d);
public:
    virtual bool __stdcall isLeaf(void) = 0;
    virtual IGeometry* __stdcall getGeometry(void) = 0;
    virtual IBSP* __stdcall getBSP(void) = 0;
    virtual ITexture* __stdcall getLightMap(void) = 0;
    virtual void __stdcall setLightMap(ITexture* lightMap) = 0;
};

typedef IBSPSector* (*IBSPSectorCallBack)(IBSPSector* clump, void* data);
typedef void (*BSPRenderCallback)(void* data);

class IBSP : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x3000e);
public:
    /**
     * common methods
     */
    virtual const char* __stdcall getName(void) = 0;
    virtual void __stdcall release(void) = 0;
    virtual void __stdcall add(IClump* clump) = 0;
    virtual void __stdcall add(ILight* light) = 0;
    virtual void __stdcall add(IRendering* rendering) = 0;
    virtual void __stdcall add(IParticleSystem* particleSystem) = 0;
    virtual void __stdcall add(IBatch* batch) = 0;
    virtual void __stdcall remove(IClump* clump) = 0;
    virtual void __stdcall remove(ILight* light) = 0;
    virtual void __stdcall remove(IRendering* rendering) = 0;
    virtual void __stdcall remove(IParticleSystem* particleSystem) = 0;
    virtual void __stdcall remove(IBatch* batch) = 0;
    virtual void __stdcall forAllSectors(IBSPSectorCallBack callBack, void* data) = 0;    
    virtual void __stdcall forAllLights(ILightCallBack callBack, void* data) = 0;
    virtual void __stdcall forAllClumps(IClumpCallBack callBack, void* data) = 0;
    virtual void __stdcall render(void) = 0;
    virtual void __stdcall renderLensFlares(void) = 0;
    virtual void __stdcall renderDepthMap(void) = 0;
    virtual Vector3f __stdcall getAABBInf(void) = 0;
    virtual Vector3f __stdcall getAABBSup(void) = 0;
    /**
     * fog control
     */
    virtual FogType __stdcall getFogType(void) = 0;
    virtual void __stdcall setFogType(FogType fogType) = 0;
    virtual float __stdcall getFogStart(void) = 0;
    virtual void __stdcall setFogStart(float fogStart) = 0;
    virtual float __stdcall getFogEnd(void) = 0;
    virtual void __stdcall setFogEnd(float fogEnd) = 0;
    virtual float __stdcall getFogDensity(void) = 0;
    virtual void __stdcall setFogDensity(float fogDensity) = 0;
    virtual Vector4f __stdcall getFogColor(void) = 0;
    virtual void __stdcall setFogColor(const Vector4f& fogColor) = 0;
    /**
     * shadow control
     */
    virtual void __stdcall setShadowCastDirection(const Vector3f& dir) = 0;
    virtual void __stdcall setShadowCastColor(const Vector4f& color) = 0;
    virtual void __stdcall setShadowCastDepth(float depth) = 0;
    /**
     * miscellaneous
     */
    virtual Vector4f __stdcall getAmbient(unsigned int lightset = 0) = 0;
    virtual void __stdcall setPostRenderCallback(BSPRenderCallback callback, void* data) = 0;
};

typedef IBSP* (*IBSPCallBack)(IBSP* bsp, void* data);

/**
 * intersection queries
 */

struct BoundingBox
{
public:
    Vector3f sup;    // AABB supremum point
    Vector3f inf;    // AABB infinum point
    Matrix4f matrix; // OBB transformation
};

struct CollisionTriangle
{
public:
    // triangle info
    int      triangleId;
    Vector3f normal;
    Vector3f vertices[3];    
    IShader* shader;
public:
    // collision extension
    Vector3f collisionPoint;
    float    distance;
};

typedef CollisionTriangle* (*CollisionCallBack)(
    CollisionTriangle* collTriangle, // collision triangle data
    engine::IBSPSector* sector,      // defines collision sector
    engine::IAtomic* atomic,         // defines collision atomic
    void* data                       // optional user data
);

class IRayIntersection
{
public:
    DECLARE_INTERFACE_ID(0x30012);
public:
    virtual void __stdcall release(void) = 0;
    virtual void __stdcall setRay(const Vector3f& start, const Vector3f& direction) = 0;    
    virtual void __stdcall intersect(IBSP* bsp, CollisionCallBack callBack, void* data) = 0;
    virtual void __stdcall intersect(IAtomic* atomic, CollisionCallBack callBack, void* data) = 0;
};

class ISphereIntersection
{
public:
    DECLARE_INTERFACE_ID(0x30013);
public:
    virtual void __stdcall release(void) = 0;
    virtual void __stdcall setSphere(const Vector3f& center, float radius) = 0;
    virtual void __stdcall intersect(IAtomic* atomic, CollisionCallBack callBack, void* data) = 0;
    virtual bool __stdcall intersect(const Vector3f& v0, const Vector3f& v1, const Vector3f& v2, CollisionTriangle* collisionTriangle) = 0;
};

/**
 * assets are designed to support serialization of engine objects
 */

enum AssetType
{
    atImport, // asset is used to manage import resource
    atBinary, // asset is used to manage D3 binary resource
    atXFile   // asset is used to manage x-file resource
};

class IAsset : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x3000c);
public:
    virtual void __stdcall clear(void) = 0;
    virtual bool __stdcall isEmpty(void) = 0;
    virtual void __stdcall release(void) = 0;
    virtual void __stdcall forAllClumps(IClumpCallBack callBack, void* data) = 0;
    virtual void __stdcall forAllBSPs(IBSPCallBack callBack, void* data) = 0;
    virtual void __stdcall seize(IAsset* victimAsset) = 0;
    virtual void __stdcall add(engine::IClump* clump) = 0;
    virtual void __stdcall remove(engine::IClump* clump) = 0;
    virtual void __stdcall serialize(void) = 0;
};

/**
 * asynchronous loader
 */

class ILoader : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30017);
public:
    /**
     * release loader
     */
    virtual void __stdcall release(void) = 0;
    /**
     * @return progress of loading, ranged from 0 to 1 (fully completed)
     */
    virtual float __stdcall getProgress(void) = 0;
    /**
     * @return loaded asset
     */
    virtual IAsset* __stdcall getAsset(void) = 0;
    /**
     * suspends loader
     */
    virtual void __stdcall suspend(void) = 0;
    virtual void __stdcall resume(void) = 0;
};

/**
 * grass scheme
 */

struct GrassSpecie
{
public:
    const char* name;     // specie name (note NULL means end of species array!)
    Vector2f    uv[4];    // uv, referring to scheme texture
    Vector2f    size;     // particle size
    Vector2f    sizeBias; // particle size bias
    float       density;  // particle generation density (per square meter)
};

struct GrassScheme
{
public:
    float        clisterSize; // radius of particle's clustering spheres
    Vector4f     ambient;     // particle ambient color
    GrassSpecie* species;     // array grass species
};

/**
 * smoke trail scheme
 */

struct SmokeTrailScheme
{
public:
    // note:
    // fission distance determines general emitter behaviour:
    // when distance between last emitted point and emission
    // epicenter will be greater than fission distance,
    // a new point will be emitted
    //
    // time worksheet:
    // |------------------lifetime---------------------->|
    // |---sizetime-->|-------------------|---fadetime-->|
public:    
    struct LERPValue
    {
    public:
        float minFactor; // minimal referring factor
        float minValue;  // minimal referring value
        float maxFactor; // maximal referring factor
        float maxValue;  // maximal referring value
    public:
        LERPValue() : minFactor(0), minValue(0), maxFactor(1), maxValue(1) {}
        LERPValue( float minf, float minv, float maxf, float maxv ) :
            minFactor(minf), minValue(minv), maxFactor(maxf), maxValue(maxv) {}
    public:
        inline float getValue(float factor)
        {
            float interpolator = ( factor - minFactor ) / ( maxFactor - minFactor );
            return minValue * ( 1 - interpolator ) + maxValue * interpolator;
        }
        inline float getSaturatedValue(float factor)
        {
            float interpolator = ( factor - minFactor ) / ( maxFactor - minFactor );
            interpolator = interpolator < 0 ? 0 : ( interpolator > 1 ? 1 : interpolator );
            return minValue * ( 1 - interpolator ) + maxValue * interpolator;
        }
    };
public:
    unsigned int numParticles;  // number of particles
    LERPValue    fissionLERP;   // determ. dependence of fission caused by velocity
    float        damping;       // damping coefficient of moving particle
    float        heat;          // heat coefficient of moving particle
    float        lifeTime;      // life time of particle
    float        fadeTime;      // fade time of particle  (less or equal to life time)
    LERPValue    sizeTimeLERP;  // determ. dependence of size time caused by velocity
    LERPValue    startSizeLERP; // determ. dependence of start size caused by velocity
    LERPValue    endSizeLERP;   // determ. dependence of end size caused by velocity
    Vector2f     uv[4];         // uv, referring to the particle
    Vector4f     ambient;       // ambient color
public:
    SmokeTrailScheme()
    {
        numParticles = 0;
        damping      = 0.0f;
        heat         = 0.0f;
        lifeTime     = 1.0f;
        fadeTime     = 1.0f;
        uv[0]        = Vector2f( 0,0 );
        uv[1]        = Vector2f( 0,0 );
        uv[2]        = Vector2f( 0,0 );
        uv[3]        = Vector2f( 0,0 );
        ambient      = Vector4f( 1,1,1,1 );
    }
};

/**
 * progress callback
 */

typedef void (*ProgressCallBack)(const wchar_t* description, float progress, void* userData);

/**
 * graphics engine
 */

enum RenderState
{
    rsZEnable = 7,       // bool, enables/disables z-buffer test
    rsZWriteEnable = 14  // bool, enables/disables z-buffer write
};

enum RenderMode
{
    rmBSPAABB        = 0x00000001l, // render bounding volume (AABB) for BSP sectors
    rmAtomicSphere   = 0x00000002l, // render bounding volume (sphere) for atomics
    rmFrameHierarchy = 0x00000004l  // render frame hierarchy wires
};

enum DeviceState
{
    dsOk,
    dsLost,
    dsNotReset,
    dsDriverInternalError
};

struct RenderStatictics
{
public:
    unsigned int bspTotal;             // total bsp sectors;
    unsigned int bspRendered;          // actually rendered bsp sectors;
    unsigned int atomicsRendered;      // actually rendered atomics;
    unsigned int alphaObjectsRendered; // actually rendered alpha-objects
    unsigned int shaderCacheHits;      // caching hits for shaders
};

class IEngine : public ccor::IBase
{
public:
    DECLARE_INTERFACE_ID(0x30001);
public:
    /**
     * system probes & routines
     */
    virtual unsigned int __stdcall getRenderMode(void) = 0;
    virtual void __stdcall setRenderMode(unsigned int renderMode) = 0;
    virtual RenderStatictics* __stdcall getStatictics(void) = 0;
    virtual Vector3f __stdcall getScreenSize(void) = 0;
    virtual void __stdcall setProgressCallback(ProgressCallBack callback, void* userData) = 0;
    virtual DeviceState __stdcall handleCooperativeLevel(void) = 0;
    virtual bool __stdcall canRestoreDevice(void) = 0;
    /**
     * rendering
     */    
    virtual ICamera* __stdcall getDefaultCamera(void) = 0;
	virtual void __stdcall present(void) = 0;
    virtual void __stdcall setRenderState(RenderState renderState, unsigned int value) = 0;
    /**
     * interfaces creation routine
     */
    virtual ITexture* __stdcall createRenderTarget(int width, int height, int depth, const char* textureName) = 0;
    virtual ITexture* __stdcall createCubeRenderTarget(int size, int depth, const char* textureName) = 0;
    virtual ITexture* __stdcall createTexture(const char* resourcePath) = 0;
    virtual ITexture* __stdcall createDUDVFromNormalMap(ITexture* normalMap, const char* dudvName) = 0;
    virtual IShader* __stdcall createShader(int numLayers, const char* shaderName) = 0;
    virtual IFrame* __stdcall createFrame(const char* frameName) = 0;
    virtual IGeometry* __stdcall createGeometry(int numVertices, int numTriangles, int numUVSets, int numShaders, int numPrelights, bool sharedShaders, const char* geometryName) = 0;
    virtual ICamera* __stdcall createCamera(unsigned int width, unsigned int height) = 0;
    virtual ICameraEffect* __stdcall createCameraEffect(ITexture* renderTarget) = 0;
    virtual ILight* __stdcall createLight(LightType lightType) = 0;
    virtual IAtomic* __stdcall createAtomic(void) = 0;
    virtual IBatch* __stdcall createBatch(unsigned int batchSize, BatchScheme* batchScheme) = 0;
    virtual IClump* __stdcall createClump(const char* clumpName) = 0;
    virtual IBSP* __stdcall createBSP(const char* bspName, const Vector3f& boxInf, const Vector3f& boxSup) = 0;
    virtual IEffect* __stdcall createEffect(const char* effectName) = 0;
    virtual IRayIntersection* __stdcall createRayIntersection(void) = 0;
    virtual ISphereIntersection* __stdcall createSphereIntersection(void) = 0;    
    virtual IAsset* __stdcall createAsset(AssetType assetType, const char* resourcePath) = 0;
    virtual ILoader* __stdcall createLoader(AssetType assetType, const char* resourcePath) = 0;
    virtual IParticleSystem* __stdcall createParticleSystem(unsigned int numParticles, engine::IShader* shader, float alphaSortDepth) = 0;
    virtual IRendering* __stdcall createGrass(const char* resourcePath, IAtomic* templateAtomic, ITexture* texture, GrassScheme* grassScheme, float fadeStart, float fadeEnd) = 0;
    virtual IRendering* __stdcall createRain(unsigned int maxParticles, float emissionSphere, ITexture* texture, Vector4f ambient) = 0;
    virtual IRendering* __stdcall createSmokeTrail(IShader* shader, SmokeTrailScheme* scheme) = 0;
    virtual IRendering* __stdcall createGlow(IBSP* bsp, const char* textureResource) = 0;
    /**
     * environment map generation
     */
    virtual void __stdcall beginEnvironmentMap(
        ITexture*       envMap,
        CubemapFace     face,
        Matrix4f        cameraMatrix,
        unsigned int    clearMode, 
        const Vector4f& clearColor
    ) = 0;
    virtual void __stdcall endEnvironmentMap(void) = 0;    
    /**
     * utilites     
     */
    virtual IFrame* __stdcall findFrame(IFrame* root, const char* frameName) = 0;
    virtual IAtomic* __stdcall getAtomic(IClump* clump, IFrame* frame) = 0;
    virtual Mesh* __stdcall createMesh(unsigned int numVertices, unsigned int numTriangles, unsigned int numUVs) = 0;
    virtual void __stdcall releaseMesh(Mesh* mesh) = 0;
    virtual bool __stdcall intersectOBB(const BoundingBox& obb1, const BoundingBox& obb2) = 0;
    /**
     * texture list enumeration
     */
    virtual int __stdcall getNumTextures(void) = 0;
    virtual ITexture* __stdcall getTexture(int id) = 0;
    virtual ITexture* __stdcall getTexture(const char* textureName) = 0;    
    /**
     * effect enumeration
     */
    virtual int __stdcall getNumEffects(void) = 0;
    virtual const char* __stdcall getEffectName(int effectId) = 0;
    virtual bool __stdcall isPfxSupported(PostEffectType pfxType) = 0;
    /**
     * transformation routine
     */
    virtual Matrix4f __stdcall rotateMatrix(const Matrix4f& matrix, const Vector3f& axis, float angle) = 0;
    virtual Matrix4f __stdcall translateMatrix(const Matrix4f& matrix, const Vector3f& vector) = 0;    
    virtual Matrix4f __stdcall transformMatrix(const Matrix4f& matrix, const Matrix4f& transformation) = 0;
    virtual Matrix4f __stdcall invertMatrix(const Matrix4f& matrix) = 0;
    virtual Vector3f __stdcall transformCoord(const Vector3f& coord, const Matrix4f& transformation) = 0;
    virtual Vector3f __stdcall transformNormal(const Vector3f& normal, const Matrix4f& transformation) = 0;
    virtual Vector2f __stdcall putConstraint(Vector3f& inout, const Matrix4f& reference, float tiltLimit, float turnLimit) = 0;
    virtual Matrix4f __stdcall interpolate(const Matrix4f& matrix1, const Matrix4f& matrix2, float factor) = 0;
    /**
     * primitive rendering routine
     */    
    virtual void __stdcall renderLine(const Vector3f& start, const Vector3f& end, const Vector4f& color) = 0;    
    virtual void __stdcall renderOBB(const BoundingBox& obb, const Vector4f& color) = 0;
    virtual void __stdcall renderSphere(const Vector3f& center, float radius, const Vector4f& color) = 0;    
    virtual void __stdcall renderRect2d(const Vector2f& topLeft, const Vector2f& size, const Vector4f& color, ITexture* texture) = 0;
    virtual void __stdcall renderDebugInfo(void* debugInfo) = 0;
    virtual void __stdcall renderTestRect(const Matrix4f& matrix, const Vector4f& color) = 0;
};

}

#endif
