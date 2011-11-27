/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description engine entity
 *
 * @author bad3p
 */

#ifndef ENGINE_IMPLEMENTATION_INCLUDED
#define ENGINE_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "../shared/ccor.h"
#include "../shared/engine.h"
#include "../shared/mainwnd.h"
#include "../shared/import.h"
#include "../shared/language.h"

#include "fundamentals.h"
#include "errorreport.h"
#include "imem.h"

using namespace ccor;

/**
 * fast interface inclusion
 */

#define IMainWnd(X) mainwnd::IMainWnd* X; queryInterface( "MainWnd", &X );
#define IImport(X) import::IImport* X; queryInterface( "Import", &X );
#define ILanguage(X) language::ILanguage* X; queryInterface( "Language", &X );

/**
 * lostable object
 * such an objects are unsafe while device is lost and should be restored
 * according to continue common functionality
 */

class Lostable
{
public:
    typedef std::list<Lostable*> Lostables;
    typedef Lostables::iterator LostableI;
public:
    static Lostables lostables;
public:
    Lostable();
    virtual ~Lostable();
public:
    virtual void onLostDevice(void) = 0;
    virtual void onResetDevice(void) = 0;
};

/**
 * static lostable object - implemented via callbacks
 */

typedef void (*LostableCallback)(void);

class StaticLostable : public Lostable
{
protected:
    LostableCallback _onLostCallback;
    LostableCallback _onResetCallback;
public:
    StaticLostable(LostableCallback onLostCallback, LostableCallback onResetCallback) :
        _onLostCallback(onLostCallback), _onResetCallback(onResetCallback)
    {}
public:
    virtual void onLostDevice(void)
    {
        _onLostCallback();
    }
    virtual void onResetDevice(void)
    {
        _onResetCallback();
    }
};

/**
 * IEngine implementation
 */

class Engine : public EntityBase,
               virtual public engine::IEngine
{
private:
    // engine configuration
    TiXmlDocument* _generalConfig;
    IParamPack*    _coreConfig;
public:
    // general DirectX interfaces are public fields
    IDirect3D9*              iDirect3D9;
    IDirect3DDevice9*        iDirect3DDevice9;
    D3DCAPS9                 deviceCaps;
    D3DPRESENT_PARAMETERS    presentParams;
    int                      screenWidth;
    int                      screenHeight;
    int                      screenDepth;
    engine::ProgressCallBack progressCallback;
    void*                    progressCallbackUserData;    
    engine::ITexture*        alphaBlackTexture;    
private:
    // engine private fields    
    engine::ICamera*  _camera;    
    bool              _onLostTrigger;
    bool              _canRestoreDevice;
private:
    // system probes
    unsigned int _renderMode;
public:
    // class implementation
    Engine();
    virtual ~Engine();
    // component support
    static EntityBase* creator();
    virtual void __stdcall entityDestroy();
    // EntityBase 
    virtual void __stdcall entityInit(Object * p);
    virtual void __stdcall entityAct(float dt);
    virtual void __stdcall entityHandleEvent(evtid_t id, trigid_t trigId, Object* param);
    virtual IBase* __stdcall entityAskInterface(iid_t id);
    // IEngine
    virtual unsigned int __stdcall getRenderMode(void);
    virtual void __stdcall setRenderMode(unsigned int renderMode);
    virtual engine::RenderStatictics* __stdcall getStatictics(void);
    virtual Vector3f __stdcall getScreenSize(void);
    virtual void __stdcall setProgressCallback(engine::ProgressCallBack callback, void* userData);
    virtual engine::DeviceState __stdcall handleCooperativeLevel(void);
    virtual bool __stdcall canRestoreDevice(void);
    virtual engine::ICamera* __stdcall getDefaultCamera(void);
	virtual void __stdcall present(void);
    virtual void __stdcall setRenderState(engine::RenderState renderState, unsigned int value);
    virtual engine::ITexture* __stdcall createRenderTarget(int width, int height, int depth, const char* textureName);
    virtual engine::ITexture* __stdcall createCubeRenderTarget(int size, int depth, const char* textureName);
    virtual engine::ITexture* __stdcall createTexture(const char* resourcePath);
    virtual engine::ITexture* __stdcall createDUDVFromNormalMap(engine::ITexture* normalMap, const char* dudvName);
    virtual engine::IShader* __stdcall createShader(int numLayers, const char* shaderName);
    virtual engine::IFrame* __stdcall createFrame(const char* frameName);
    virtual engine::IGeometry* __stdcall createGeometry(int numVertices, int numTriangles, int numUVSets, int numShaders, int numPrelights, bool sharedShaders, const char* geometryName);
    virtual engine::ICamera* __stdcall createCamera(unsigned int width, unsigned int height);
    virtual engine::ICameraEffect* __stdcall createCameraEffect(engine::ITexture* renderTarget);
    virtual engine::ILight* __stdcall createLight(engine::LightType lightType);
    virtual engine::IAtomic* __stdcall createAtomic(void);
    virtual engine::IBatch* __stdcall createBatch(unsigned int batchSize, engine::BatchScheme* batchScheme);
    virtual engine::IClump* __stdcall createClump(const char* clumpName);
    virtual engine::IBSP* __stdcall createBSP(const char* bspName, const Vector3f& boxInf, const Vector3f& boxSup);
    virtual engine::IEffect* __stdcall createEffect(const char* effectName);
    virtual engine::IAsset* __stdcall createAsset(engine::AssetType assetType, const char* resourcePath);
    virtual engine::ILoader* __stdcall createLoader(engine::AssetType assetType, const char* resourcePath);
    virtual engine::IParticleSystem* __stdcall createParticleSystem(unsigned int numParticles, engine::IShader* shader, float alphaSortDepth);
    virtual engine::IRendering* __stdcall createGrass(const char* resourcePath, engine::IAtomic* templateAtomic, engine::ITexture* texture, engine::GrassScheme* grassScheme, float fadeStart, float fadeEnd);
    virtual engine::IRendering* __stdcall createRain(unsigned int maxParticles, float emissionSphere, engine::ITexture* texture, Vector4f ambient);    
    virtual engine::IRendering* __stdcall createSmokeTrail(engine::IShader* shader, engine::SmokeTrailScheme* scheme);
    virtual engine::IRendering* __stdcall createGlow(engine::IBSP* bsp, const char* textureResource);
    virtual void __stdcall beginEnvironmentMap(
        engine::ITexture* envMap, 
        engine::CubemapFace face,
        Matrix4f        cameraMatrix,
        unsigned int    clearMode, 
        const Vector4f& clearColor
    );    
    virtual void __stdcall endEnvironmentMap(void);
    virtual engine::IFrame* __stdcall findFrame(engine::IFrame* root, const char* frameName);
    virtual engine::IAtomic* __stdcall getAtomic(engine::IClump* clump, engine::IFrame* frame);
    virtual engine::Mesh* __stdcall createMesh(unsigned int numVertices, unsigned int numTriangles, unsigned int numUVs);
    virtual void __stdcall releaseMesh(engine::Mesh* mesh);
    virtual engine::IRayIntersection* __stdcall createRayIntersection(void);
    virtual engine::ISphereIntersection* __stdcall createSphereIntersection(void);
    virtual int __stdcall getNumTextures(void);
    virtual engine::ITexture* __stdcall getTexture(int id);
    virtual engine::ITexture* __stdcall getTexture(const char* textureName);
    virtual int __stdcall getNumEffects(void);
    virtual const char* __stdcall getEffectName(int effectId);
    virtual bool __stdcall isPfxSupported(engine::PostEffectType pfxType);
    virtual Matrix4f __stdcall rotateMatrix(const Matrix4f& matrix, const Vector3f& axis, float angle);
    virtual Matrix4f __stdcall translateMatrix(const Matrix4f& matrix, const Vector3f& vector);
    virtual Matrix4f __stdcall transformMatrix(const Matrix4f& matrix, const Matrix4f& transformation);
    virtual Matrix4f __stdcall invertMatrix(const Matrix4f& matrix);
    virtual Vector3f __stdcall transformCoord(const Vector3f& coord, const Matrix4f& transformation);
    virtual Vector3f __stdcall transformNormal(const Vector3f& normal, const Matrix4f& transformation);
    virtual Vector2f __stdcall putConstraint(Vector3f& inout, const Matrix4f& reference, float tiltLimit, float turnLimit);
    virtual Matrix4f __stdcall interpolate(const Matrix4f& matrix1, const Matrix4f& matrix2, float factor);
    virtual void __stdcall renderLine(const Vector3f& start, const Vector3f& end, const Vector4f& color);
    virtual void __stdcall renderOBB(const engine::BoundingBox& obb, const Vector4f& color);
    virtual void __stdcall renderSphere(const Vector3f& center, float radius, const Vector4f& color);
    virtual void __stdcall renderRect2d(const Vector2f& topLeft, const Vector2f& size, const Vector4f& color, engine::ITexture* texture);
    virtual void __stdcall renderDebugInfo(void* debugInfo);
    virtual void __stdcall renderTestRect(const Matrix4f& matrix, const Vector4f& color);
    virtual bool __stdcall intersectOBB(const engine::BoundingBox& obb1, const engine::BoundingBox& obb2);
public:
    // module local methods
    int getWindowHandle(void);
    TiXmlElement* getConfigElement(const char* elementName);
public:
    // engine instance for module internal usage
    static Engine* instance;
    static engine::RenderStatictics statistics;
};

#define iDirect3D Engine::instance->iDirect3D9
#define iDirect3DDevice Engine::instance->iDirect3DDevice9
#define dxDeviceCaps Engine::instance->deviceCaps
#define dxPresentParams Engine::instance->presentParams

#define dxFilterSupported(T) ( dxDeviceCaps.TextureFilterCaps & T )

#define dxSetLight Engine::instance->iDirect3DDevice9->SetLight
#define dxGetLight Engine::instance->iDirect3DDevice9->GetLight
#define dxGetLightEnable Engine::instance->iDirect3DDevice9->GetLightEnable
#define dxLightEnable Engine::instance->iDirect3DDevice9->LightEnable

#define dxSetMaterial Engine::instance->iDirect3DDevice9->SetMaterial

#define dxSetTexture Engine::instance->iDirect3DDevice9->SetTexture
#define dxSetTextureStageState Engine::instance->iDirect3DDevice9->SetTextureStageState
#define dxGetRenderState Engine::instance->iDirect3DDevice9->GetRenderState
#define dxSetRenderState Engine::instance->iDirect3DDevice9->SetRenderState
#define dxSetSamplerState Engine::instance->iDirect3DDevice9->SetSamplerState

#define dxCreateVertexShader Engine::instance->iDirect3DDevice9->CreateVertexShader 
#define dxCreatePixelShader Engine::instance->iDirect3DDevice9->CreatePixelShader
#define dxSetVertexShader Engine::instance->iDirect3DDevice9->SetVertexShader
#define dxSetPixelShader Engine::instance->iDirect3DDevice9->SetPixelShader

#define dxSetVertexShaderConstantF Engine::instance->iDirect3DDevice9->SetVertexShaderConstantF
#define dxSetPixelShaderConstantF Engine::instance->iDirect3DDevice9->SetPixelShaderConstantF

#define dxCreateVertexBuffer Engine::instance->iDirect3DDevice9->CreateVertexBuffer
#define dxCreateIndexBuffer Engine::instance->iDirect3DDevice9->CreateIndexBuffer
#define dxSetFVF Engine::instance->iDirect3DDevice9->SetFVF
#define dxSetStreamSource Engine::instance->iDirect3DDevice9->SetStreamSource
#define dxSetIndices Engine::instance->iDirect3DDevice9->SetIndices

#define dxDrawPrimitive Engine::instance->iDirect3DDevice9->DrawPrimitive

static inline void dxSetVertexShaderConstant(unsigned int startRegister, const Vector& vector)
{
    float value[4];
    value[0] = vector.x, value[1] = vector.y, value[2] = vector.z, value[3] = 1.0f;
    _dxCR( dxSetVertexShaderConstantF( startRegister, value, 1 ) );
}

static inline void dxSetVertexShaderConstant(unsigned int startRegister, const Matrix& matrix)
{
    _dxCR( dxSetVertexShaderConstantF( startRegister, matrix, 4 ) );
}

static inline void dxSetVertexShaderConstant(unsigned int startRegister, const Quartector& quartector)
{
    _dxCR( dxSetVertexShaderConstantF( startRegister, quartector, 1 ) );
}

static inline void dxSetVertexShaderConstant(unsigned int startRegister, const D3DCOLORVALUE& color)
{
    float value[4];
    value[0] = color.r, value[1] = color.g, value[2] = color.b, value[3] = color.a;
    _dxCR( dxSetVertexShaderConstantF( startRegister, value, 1 ) );
}

static inline void dxSetVertexShaderConstant(unsigned int startRegister, float x, float y, float z, float w)
{
    float value[4];
    value[0] = x, value[1] = y, value[2] = z, value[3] = w;
    _dxCR( dxSetVertexShaderConstantF( startRegister, value, 1 ) );
}

static inline void dxSetPixelShaderConstant(unsigned int startRegister, const Vector& vector)
{
    float value[4];
    value[0] = vector.x, value[1] = vector.y, value[2] = vector.z, value[3] = 1.0f;
    _dxCR( dxSetPixelShaderConstantF( startRegister, value, 1 ) );
}

static inline void dxSetPixelShaderConstant(unsigned int startRegister, const Quartector& quartector)
{
    _dxCR( dxSetPixelShaderConstantF( startRegister, quartector, 1 ) );
}

#endif
