        
#include "headers.h"
#include "engine.h"
#include "asset.h"
#include "light.h"
#include "atomic.h"
#include "bsp.h"
#include "asset.h"
#include "wire.h"
#include "collision.h"
#include "effect.h"
#include "camera.h"
#include "intersection.h"
#include "sprite.h"
#include "rain.h"

#include "fastquat.h"
#include "../common/profiler.h"

Engine*                  Engine::instance = NULL;
engine::RenderStatictics Engine::statistics;

Engine::Engine()
{
    // quaternion interpolation
    Quaternion q1(1,0,0,0.23f), q2(0,1,0,-0.13f), qOut1, qOut2;
    unsigned int startTime = GetTickCount();
    for( unsigned int i=0; i<1000000; i++ )
    {
        D3DXQuaternionSlerp( &qOut1, &q1, &q2, 0.6677f );
    }
    unsigned int endTime = GetTickCount();
    float time1 = 0.001f * ( endTime - startTime );
    startTime = GetTickCount();
    for( unsigned int i=0; i<1000000; i++ )
    {
        quatSlerpMatrix( &qOut2, &q1, &q2, 0.6677f );
    }
    endTime = GetTickCount();
    float time2 = 0.001f * ( endTime - startTime );
    getCore()->logMessage( "D3DXQvsFAST %4.3f : %4.3f", time1, time2 );

    // check singleton instance
    assert( !instance );
    instance = this;

    progressCallback = NULL;
    _generalConfig = NULL;
    _coreConfig = NULL;
    iDirect3D9 = NULL;
    iDirect3DDevice9 = NULL;

    // fill internal parameters
    presentParams.BackBufferWidth    = 0;
    presentParams.BackBufferHeight   = 0;
    presentParams.BackBufferFormat   = D3DFMT_X8R8G8B8;
    presentParams.BackBufferCount    = 1;
    presentParams.MultiSampleType    = D3DMULTISAMPLE_NONE;
    presentParams.MultiSampleQuality = 0;
    presentParams.SwapEffect    = D3DSWAPEFFECT_DISCARD;
    presentParams.hDeviceWindow = 0;
    presentParams.Windowed      = TRUE;
    presentParams.EnableAutoDepthStencil = FALSE;
    presentParams.AutoDepthStencilFormat = D3DFMT_X8R8G8B8;
    presentParams.Flags = 0;
    presentParams.FullScreen_RefreshRateInHz = 0;
    presentParams.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    _camera = NULL;
    _onLostTrigger = false;
    _canRestoreDevice = true;

    memset( &statistics, 0, sizeof(engine::RenderStatictics) );
}

Engine::~Engine()
{
    // release alpha black texture
    alphaBlackTexture->release();

    // release debugging chain links
    if( Chain::first )
    {
        FILE* f = fopen( "engine.garbage", "wt" );
        std::string className;        
        std::string objectName;
        while( Chain::first )
        {
            className  = "class";
            objectName = "object";
            if( dynamic_cast<Texture*>( Chain::first ) )
            {
                className = "Texture";
                objectName = dynamic_cast<Texture*>( Chain::first )->getName();
            }
            fprintf( f, "\"%s\" \"%s\" %s\n", className.c_str(), objectName.c_str(), Chain::first->getContext() );
            delete Chain::first;
        }
        fclose( f );
    }

    // release effect resources
    Effect::term();
    ShadowVolume::releaseResources();
    // release rendering resources
    if( _camera ) _camera->release();
    // release engine components
    CameraEffect::term();
    Mesh::term();
    Frame::term();
    // release general Direct3D interfaces
    if( iDirect3DDevice9 ) iDirect3DDevice9->Release();
    if( iDirect3D9 ) iDirect3D9->Release();

    // release configz
    delete _generalConfig;

    // reset singleton instance
    instance = NULL;

    // memory leaks?
    // _CrtDumpMemoryLeaks();
}

/**
 * component support
 */

EntityBase* Engine::creator()
{
    return new Engine;
}

void Engine::entityDestroy()
{
    delete this;
}

void Engine::entityInit(Object * p)
{   
    _renderMode = 0;//engine::rmAtomicSphere;

    // subscribe triggers
    getCore()->trigAddListener( mainwnd::TriggerMainwnd::tid, getid() );

    // retrieve core configuration     
    _coreConfig = getCore()->getCoreParamPack();

    // load general configuration
    _generalConfig = new TiXmlDocument( "./cfg/config.xml" );
    _generalConfig->LoadFile();

    // retrieve configuration element for video
    TiXmlElement* video = getConfigElement( "video" );
    if( !video ) throw Exception( "Invalid configuration file!" );    

    // initialize Direct3D
    iDirect3D9 = Direct3DCreate9( D3D_SDK_VERSION ); assert( iDirect3D9 );

    // choose videoadapter
    int numAdapters = iDirect3D9->GetAdapterCount();
    int adapterId; video->Attribute( "adapter", &adapterId );
    if( adapterId < 0 || adapterId > numAdapters )
    {
        throw Exception( "Invalid videoadapter identifier: %d", adapterId );
    }

    // retrieve device capabilitiez for adapter
    _dxCR( iDirect3D->GetDeviceCaps( adapterId, D3DDEVTYPE_HAL, &deviceCaps ) );

    // retrieve screen configuration
    int windowed, refreshRate;
    video->Attribute( "windowed", &windowed );
    video->Attribute( "width", &screenWidth );
    video->Attribute( "height", &screenHeight );
    video->Attribute( "depth", &screenDepth );
    video->Attribute( "rate", &refreshRate );
    D3DFORMAT screenFormat = ( screenDepth == 16 ? D3DFMT_R5G6B5 : D3DFMT_X8R8G8B8 );
    
    // modify screen metrics for windowed mode
    if( windowed )
    {
        screenWidth = _coreConfig->getv( "engine.viewport.width", 1024 );
        screenHeight = _coreConfig->getv( "engine.viewport.height", 768 );
    }

    // choose adapter mode corresponding to screen configuration
    int numModes = iDirect3D9->GetAdapterModeCount( adapterId, screenFormat );
    D3DDISPLAYMODE targetMode;
    std::vector<int> alternativeModes;
    int targetModeId = -1;
    for( int i=0; i<numModes; i++ )
    {
        dxCheckResult( __FILE__, __LINE__, iDirect3D9->EnumAdapterModes(
            adapterId, screenFormat, i, &targetMode
        ) );
        // compare videomode parameters & configured parameters
        if( targetMode.Width == screenWidth &&
            targetMode.Height == screenHeight )
        {
            if( targetMode.RefreshRate == refreshRate )
            {
                targetModeId = i;
                break;
            }
            else
            {
                alternativeModes.push_back(i);
            }
        }
    }

    // check adapter mode
    if( !windowed && targetModeId == -1 && alternativeModes.size() == 0 ) 
    {
        throw Exception( "Can't choose configured videomode %dx%dx%dx%dhz", screenWidth, screenHeight, screenDepth, refreshRate );
    }
    if( !windowed && targetModeId == -1 && alternativeModes.size() != 0 ) 
    {
        // log
        getCore()->logMessage( "Can't choose configured videomode %dx%dx%dx%dhz", screenWidth, screenHeight, screenDepth, refreshRate );
        // retrieve alternative videomode
        targetModeId = alternativeModes[0];
        dxCheckResult( __FILE__, __LINE__, iDirect3D9->EnumAdapterModes(
            adapterId, screenFormat, targetModeId, &targetMode
        ) );
        getCore()->logMessage( "Choose alternative videomode %dx%dx%dhz", targetMode.Width, targetMode.Height, targetMode.RefreshRate );
    }

    // setup presentation
    if( windowed )
    {
        presentParams.Windowed = TRUE;
        presentParams.Flags = presentParams.Flags | D3DPRESENTFLAG_DEVICECLIP;
        presentParams.BackBufferWidth = screenWidth;
        presentParams.BackBufferHeight = screenHeight;
        presentParams.BackBufferFormat = screenFormat;
        presentParams.FullScreen_RefreshRateInHz = 0;
    }
    else
    {
        presentParams.Windowed = FALSE;
        presentParams.Flags = presentParams.Flags & ~D3DPRESENTFLAG_DEVICECLIP; 
        presentParams.BackBufferWidth = targetMode.Width;
        presentParams.BackBufferHeight = targetMode.Height;
        presentParams.BackBufferFormat = targetMode.Format;
        presentParams.FullScreen_RefreshRateInHz = targetMode.RefreshRate;
    }    
    presentParams.hDeviceWindow = reinterpret_cast<HWND>( getWindowHandle() );
    presentParams.EnableAutoDepthStencil = TRUE;
    presentParams.Flags |= D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
    presentParams.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
    if( presentParams.BackBufferFormat == D3DFMT_X8R8G8B8 )
    {
        presentParams.AutoDepthStencilFormat  = D3DFMT_D24S8;
    }
    else
    {
        presentParams.AutoDepthStencilFormat  = D3DFMT_D16;
    }

    // setup multisampling
    int multisample = 0;
    presentParams.MultiSampleType = (D3DMULTISAMPLE_TYPE)( multisample );
    presentParams.MultiSampleQuality = 0;
    presentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
    // check multisampling levels
    DWORD maxMultisampleQuality = 0;
    if( D3DERR_INVALIDCALL == iDirect3D9->CheckDeviceMultiSampleType( 
        adapterId,  D3DDEVTYPE_HAL, screenFormat, windowed, presentParams.MultiSampleType, &maxMultisampleQuality 
    ) )
    {
        throw Exception( "Device does not support specified multisample type: %d", presentParams.MultiSampleType );
    }
    if( maxMultisampleQuality < presentParams.MultiSampleQuality )
    {
        throw Exception( "Device does not support specified multisample quality level: %d", presentParams.MultiSampleQuality );
    }

    // create Direct3D device
    _dxCR( iDirect3D9->CreateDevice(
        adapterId,
        D3DDEVTYPE_HAL,
        presentParams.hDeviceWindow,
        D3DCREATE_MIXED_VERTEXPROCESSING,
        &presentParams,
        &iDirect3DDevice9
    ) );

    // initialize engine components
    Frame::init();
    Effect::init();
    Mesh::init();
    CameraEffect::init();

    // load default textures
    createTexture( "./res/effects/textures/lensflare/flare1.dds" );
    createTexture( "./res/effects/textures/lensflare/flare2.dds" );
    createTexture( "./res/effects/textures/lensflare/flare3.dds" );
    createTexture( "./res/effects/textures/lensflare/flare4.dds" );
    createTexture( "./res/effects/textures/lensflare/flare5.dds" );
    createTexture( "./res/effects/textures/lensflare/flare6.dds" );
    createTexture( "./res/effects/textures/lensflare/flare7.dds" );

    // create alpha balck texture (used as render state corrector)
    alphaBlackTexture = createTexture( "./res/alphablack.dds" );
    assert( alphaBlackTexture );

    // create default rendering resources
    _camera = createCamera( screenWidth, screenHeight );
    _camera->setFrame( createFrame( "defaultCameraFrame" ) );
    _camera->setFarClipPlane( 1000000.0f );
    _camera->setNearClipPlane( 10.0f );    
    _camera->setFOV( 60 );

    // default render states
    dxSetRenderState( D3DRS_SPECULARENABLE, TRUE );

    if( presentParams.MultiSampleType != D3DMULTISAMPLE_NONE )
    {
        dxSetRenderState( D3DRS_MULTISAMPLEANTIALIAS, TRUE );
    }
}

void Engine::entityAct(float dt)
{
    Effect::update( dt );
    Rain::update( dt );
}

void Engine::entityHandleEvent(evtid_t id, trigid_t trigId, Object* param)
{
}

IBase* Engine::entityAskInterface(iid_t id)
{
    if( id == engine::IEngine::iid ) return this;
    return NULL;
}

/**
 * module local methods
 */

int Engine::getWindowHandle(void)
{
    mainwnd::IMainWnd* mainWnd;
    queryInterface( "MainWnd", &mainWnd );
    return mainWnd->getHandle();
}

TiXmlElement* Engine::getConfigElement(const char* elementName)
{
    TiXmlNode* child = _generalConfig->FirstChild(); assert( child );
    if( child != NULL ) do 
    {
        if( child->Type() == TiXmlNode::ELEMENT && strcmp( child->Value(), elementName ) == 0 )
        {
            return static_cast<TiXmlElement*>( child );
        }
        child = child->NextSibling();
    }
    while( child != NULL );
    return NULL;
}

/**
 * IEngine methods
 */

unsigned int Engine::getRenderMode(void)
{
    return _renderMode;
}

void Engine::setRenderMode(unsigned int renderMode)
{
    _renderMode = renderMode;    
}

engine::RenderStatictics* Engine::getStatictics(void)
{
    return &statistics;
}

Vector3f Engine::getScreenSize(void)
{
    return Vector3f( 
        float( screenWidth ), 
        float( screenHeight ), 
        float( screenDepth )
    );
}

void Engine::setProgressCallback(engine::ProgressCallBack callback, void* userData)
{
    progressCallback         = callback;
    progressCallbackUserData = userData;
}

engine::ICamera* Engine::getDefaultCamera(void)
{
    return _camera;
}

void Engine::present(void)
{
    _dxCR( iDirect3DDevice9->Present( NULL, NULL, NULL, NULL ) );
}

void Engine::setRenderState(engine::RenderState renderState, unsigned int value)
{
    _dxCR( dxSetRenderState( (D3DRENDERSTATETYPE)( renderState ), (DWORD)( value ) ) );
}

engine::IAsset* Engine::createAsset(engine::AssetType assetType, const char* resourcePath)
{
    switch( assetType )
    {
    case engine::atImport:
        return new ImportAsset( resourcePath );
    case engine::atBinary:
        {
            IResource* resource = getCore()->getResource( resourcePath, "rb" );
            if( resource ) 
            {
                BinaryAsset* result = new BinaryAsset( resource );
                resource->release();
                return result;
            }
            else 
            {
                return new BinaryAsset( resourcePath );
            }
        }
    case engine::atXFile:
        return new XAsset( resourcePath );
    default:
        assert( !"shouldn't be here!" );
    }
    return NULL;
}

engine::IRayIntersection* Engine::createRayIntersection(void)
{
    return new RayIntersection;
}

engine::ISphereIntersection* Engine::createSphereIntersection(void)
{
    return new SphereIntersection;
}

/**
 * utilites
 */

engine::IFrame* Engine::findFrame(engine::IFrame* root, const char* frameName)
{
    return dynamic_cast<Frame*>( root )->findFrame( frameName );
}

engine::IAtomic* Engine::getAtomic(engine::IClump* clump, engine::IFrame* frame)
{
    return dynamic_cast<Clump*>( clump )->getAtomic( dynamic_cast<Frame*>( frame ) );
}

engine::Mesh* Engine::createMesh(unsigned int numVertices, unsigned int numTriangles, unsigned int numUVs)
{
    engine::Mesh* mesh = new engine::Mesh;
    mesh->numVertices  = numVertices;
    mesh->vertices     = new Vector3f[mesh->numVertices];
    mesh->numTriangles = numTriangles;
    mesh->triangles    = new engine::Mesh::Triangle[mesh->numTriangles];
    mesh->numUVs       = numUVs;
    for( unsigned int i=0; i<mesh->numUVs; i++ )
    {
        mesh->uvs[i] = new Vector2f[mesh->numVertices];
    }
    return mesh;
}

void Engine::releaseMesh(engine::Mesh* mesh)
{
    if( mesh->vertices ) delete[] mesh->vertices;
    if( mesh->triangles ) delete[] mesh->triangles;
    for( unsigned int i=0; i<engine::maxTextureLayers; i++ )
    {
        if( mesh->uvs[i] ) delete[] mesh->uvs[i];
    }
    delete mesh;
}

/**
 * matrix transformation
 */

Matrix4f Engine::rotateMatrix(const Matrix4f& matrix, const Vector3f& axis, float angle)
{
    Matrix m = wrap( matrix );
    Vector a = wrap( axis );
    dxRotate( &m, &a, angle );
    return wrap( m );
}

Matrix4f Engine::translateMatrix(const Matrix4f& matrix, const Vector3f& vector)
{
    Matrix m = wrap( matrix );
    Vector v = wrap( vector );
    dxTranslate( &m, &v );
    return wrap( m );
}

Matrix4f Engine::transformMatrix(const Matrix4f& matrix, const Matrix4f& transformation)
{
    Matrix m = wrap( matrix );
    Matrix t = wrap( transformation );
    Matrix result;
    dxMultiply( &result, &m, &t );
    return wrap( result );
}

Matrix4f Engine::invertMatrix(const Matrix4f& matrix)
{
    Matrix m = wrap( matrix );
    Matrix im;
    D3DXMatrixInverse( &im, NULL, &m );
    return wrap( im );
}

Vector3f Engine::transformCoord(const Vector3f& coord, const Matrix4f& transformation)
{
    Vector c = wrap( coord );
    Matrix t = wrap( transformation );    

    Vector result;
    D3DXVec3TransformCoord( &result, &c, &t );
    return wrap( result );
}

Vector3f Engine::transformNormal(const Vector3f& normal, const Matrix4f& transformation)
{
    Vector n = wrap( normal );
    Matrix t = wrap( transformation );    

    Vector result;
    D3DXVec3TransformCoord( &result, &n, &t );
    return wrap( result );
}

inline float measureAngle(const Vector* v1, const Vector* v2, const Vector* v3)
{
    float angle;
    float cosA = D3DXVec3Dot( v1, v2 );
    // range check & angle metrics
    if( cosA > 1.0f ) angle = 0.0f;
    else if( cosA < -1.0f ) angle = 180;
    else angle = acosf( cosA ) * 180 / 3.1415926f;
    // signum of angle
    Vector crossTest;
    D3DXVec3Cross( &crossTest, v1, v2 );
    float sgnTest = D3DXVec3Dot( &crossTest, v3 );
    return angle * -( sgnTest < 0 ? -1 : ( sgnTest > 0 ? 1 : 0 ) );
}

Vector2f Engine::putConstraint(Vector3f& inout, const Matrix4f& reference, float tiltLimit, float turnLimit)
{
    Vector v = wrap( inout );
    D3DXVec3Normalize( &v, &v );
    Matrix ref = wrap( reference );

    // remove translation component of reference matrix 
    ref._41 = ref._42 = ref._43 = 0.0f; ref._44 = 1.0f;

    // inversion of reference matrix gives transformation to reference space
    Matrix iref;
    D3DXMatrixInverse( &iref, NULL, &ref );

    // retrieve specified vector in reference space
    Vector vr;
    D3DXVec3TransformNormal( &vr, &v, &iref );
    
    // retrieve specified vector in reference space, projected on to oXoZ plane
    Vector vrXZ( vr.x, 0.0f, vr.z );
    D3DXVec3Normalize( &vrXZ, &vrXZ );
    
    // retrieve constrained turn angle
    float turnAngle = measureAngle( &oZ, &vrXZ, &oY );
    float tu = turnAngle;
    if( turnAngle > turnLimit ) turnAngle = turnLimit;
    if( turnAngle < -turnLimit ) turnAngle = -turnLimit;

    // retrieve signum vector to measure tilt angle
    Vector vrSign;
    D3DXVec3Cross( &vrSign, &oY, &vrXZ );

    // retrieve constrained tilt angle
    float tiltAngle = measureAngle( &vrXZ, &vr, &vrSign );
    float ti = tiltAngle;
    if( tiltAngle > tiltLimit ) tiltAngle = tiltLimit;
    if( tiltAngle < -tiltLimit ) tiltAngle = -tiltLimit;

    // retrieve constrained matrix
    Matrix temp = identity;
    dxRotate( &temp, &oX, -tiltAngle );
    dxRotate( &temp, &oY, -turnAngle );

    Matrix result;
    dxMultiply( &result, &temp, &ref );

    inout = wrap( dxAt( &result ) );

    return Vector2f( -turnAngle, -tiltAngle );
}

static void makeSRT(SRT* srt, Matrix* m)
{
    srt->scale.x = D3DXVec3Length( &dxRight( m ) );
    srt->scale.y = D3DXVec3Length( &dxUp( m ) );
    srt->scale.z = D3DXVec3Length( &dxAt( m ) );
    srt->translation = dxPos( m );

    Matrix pureRotation = *m;
    pureRotation._11 /= srt->scale.x, 
    pureRotation._12 /= srt->scale.x, 
    pureRotation._13 /= srt->scale.x, 
    pureRotation._14 = 0.0f;
    pureRotation._21 /= srt->scale.y, 
    pureRotation._22 /= srt->scale.y, 
    pureRotation._23 /= srt->scale.y, 
    pureRotation._24 = 0.0f;
    pureRotation._31 /= srt->scale.z, 
    pureRotation._32 /= srt->scale.z, 
    pureRotation._33 /= srt->scale.z, 
    pureRotation._34 = 0.0f;
    pureRotation._41 = 0.0f, 
    pureRotation._42 = 0.0f, 
    pureRotation._43 = 0.0f, 
    pureRotation._44 = 1.0f;

    D3DXQuaternionRotationMatrix( &srt->rotation, &pureRotation );
    D3DXQuaternionNormalize( &srt->rotation, &srt->rotation );
}

Matrix4f Engine::interpolate(const Matrix4f& matrix1, const Matrix4f& matrix2, float factor)
{
    // trim interpolator range
    if( factor < 0.0f ) factor = 0.0f;
    if( factor > 1.0f ) factor = 1.0f;

    // wrap
    Matrix m1 = wrap( matrix1 );
    Matrix m2 = wrap( matrix2 );

    // make SRTs
    SRT srt1, srt2;
    makeSRT( &srt1, &m1 );
    makeSRT( &srt2, &m2 );

    // interpolate output SRT
    SRT out;
    D3DXVec3Lerp( &out.scale, &srt1.scale, &srt2.scale, factor );
    D3DXVec3Lerp( &out.translation, &srt1.translation, &srt2.translation, factor );
    D3DXQuaternionSlerp( &out.rotation, &srt1.rotation, &srt2.rotation, factor );

    // build transformations
    Matrix t, s, r;
    D3DXMatrixRotationQuaternion( &r, &out.rotation );
    D3DXMatrixScaling( &s, out.scale.x, out.scale.y, out.scale.z );
    D3DXMatrixTranslation( &t, out.translation.x, out.translation.y, out.translation.z );

    // build result from transformations
    Matrix scalingRotation, result;
    dxMultiply( &scalingRotation, &s, &r );
    dxMultiply( &result, &scalingRotation, &t );

    // result
    return wrap( result );
}

/**
 * wireframe rendering routine
 */

void Engine::renderLine(const Vector3f& start, const Vector3f& end, const Vector4f& color)
{
    Line  line( wrap( start ), wrap( end ) );
    Color rgba = wrap( color );
    dxRenderLine( &line, &rgba, NULL );
}

void Engine::renderOBB(const engine::BoundingBox& obb, const Vector4f& color)
{
    AABB aabb( wrap( obb.inf ), wrap( obb.sup ) );
    Matrix matrix = wrap( obb.matrix );
    Color rgba = wrap( color );
    dxRenderAABB( &aabb, &rgba, &matrix );
}

void Engine::renderSphere(const Vector3f& center, float radius, const Vector4f& color)
{
    Sphere sphere;
    sphere.center = wrap( center );
    sphere.radius = radius;
    Color rgba = wrap( color );
    dxRenderSphere( &sphere, &rgba, NULL );
}

void Engine::renderRect2d(const Vector2f& topLeft, const Vector2f& size, const Vector4f& color, engine::ITexture* texture)
{
    if( texture )
    {
        Texture* t = dynamic_cast<Texture*>( texture ); assert( t );
        t->apply( 0 );
        dxSetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE );
        dxSetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
        dxSetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_CURRENT );
        dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE );
        dxSetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
        dxSetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
        dxSetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
        dxSetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );
    }
    else
    {
        dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_DIFFUSE );
        dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE );
        dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
        dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );
    }

    dxRenderRect( 
        unsigned int( topLeft[1] ),
        unsigned int( topLeft[0] ),
        unsigned int( size[0] ), 
        unsigned int( size[1] ),
        wrap( color ) 
    );
}

void Engine::renderDebugInfo(void* debugInfo)
{
    NxDebugRenderable* renderable = reinterpret_cast<NxDebugRenderable*>( debugInfo );

    unsigned int numLines = renderable->getNbLines();
    Line* lines = new Line[ numLines ];
    Color* colors = new Color[ numLines ];
    const NxDebugLine* nxLines = renderable->getLines();

    for( unsigned int i=0; i<numLines; i++ )
    {
        lines[i].start[0] = 100 * nxLines[i].p0.x;
        lines[i].start[1] = 100 * nxLines[i].p0.y;
        lines[i].start[2] = 100 * nxLines[i].p0.z;
        lines[i].end[0] = 100 * nxLines[i].p1.x;
        lines[i].end[1] = 100 * nxLines[i].p1.y;
        lines[i].end[2] = 100 * nxLines[i].p1.z;
        colors[i] = nxLines[i].color;
    }

    unsigned int position = 0;
    unsigned int numStageLines;

    while( numLines > 0 )
    {
        numStageLines = numLines > MAX_LINES ? MAX_LINES : numLines;
        dxRenderLines( 
            numStageLines, 
            lines+position, 
            colors+position, 
            NULL 
        );
        numLines -= numStageLines;
        position += numStageLines;
    }

    delete[] lines;
    delete[] colors;
}

const DWORD particleFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

struct ParticleVertex
{
    Vector  pos;
    Color   color;
};

/* 
 * already declared in psys.h
 */

/*
static const Vector billboardVertices[4] = 
{
    Vector( -1.0f, -1.0f, 0.0f ),
    Vector( -1.0f, 1.0f, 0.0f ),
    Vector( 1.0f, 1.0f, 0.0f ),
    Vector( 1.0f, -1.0f, 0.0f )
};
*/

void Engine::renderTestRect(const Matrix4f& matrix, const Vector4f& color)
{

    IDirect3DVertexBuffer9* vertexBuffer;
    IDirect3DIndexBuffer9*  indexBuffer;

    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &indexBuffer,
        NULL
    ) );

    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(ParticleVertex) * 4,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        particleFVF,
        D3DPOOL_DEFAULT,
        &vertexBuffer,
        NULL
    ) );

    // lock buffers
    void* vertexData = NULL;
    void* indexData = NULL;
    _dxCR( vertexBuffer->Lock( 0, 4 * sizeof( ParticleVertex ), &vertexData, D3DLOCK_DISCARD ) );
    _dxCR( indexBuffer->Lock( 0, 6 * sizeof( WORD ), &indexData, D3DLOCK_DISCARD ) );
    assert( vertexData );
    assert( indexData );
    ParticleVertex* vertex = (ParticleVertex*)( vertexData );
    WORD* index = (WORD*)( indexData );

    Matrix m = wrap( matrix );
    m._41 = m._42 = m._43 = 0.0f, m._44 = 1.0f;
    Vector p;
    p.x = matrix[3][0], p.y = matrix[3][1], p.z = matrix[3][2];

    D3DXVec3TransformCoord( &vertex[0].pos, &billboardVertices[0], &m );
    D3DXVec3TransformCoord( &vertex[1].pos, &billboardVertices[1], &m );
    D3DXVec3TransformCoord( &vertex[2].pos, &billboardVertices[2], &m );
    D3DXVec3TransformCoord( &vertex[3].pos, &billboardVertices[3], &m );

    vertex[0].pos.x += p.x,
    vertex[0].pos.y += p.y,
    vertex[0].pos.z += p.z,
    vertex[1].pos.x += p.x,
    vertex[1].pos.y += p.y,
    vertex[1].pos.z += p.z,
    vertex[2].pos.x += p.x,
    vertex[2].pos.y += p.y,
    vertex[2].pos.z += p.z,
    vertex[3].pos.x += p.x,
    vertex[3].pos.y += p.y,
    vertex[3].pos.z += p.z;

    vertex[0].color = 
    vertex[1].color = 
    vertex[2].color = 
    vertex[3].color = wrap( color );

    index[0] = 0;
    index[1] = 1;
    index[2] = 2;
    index[3] = 0;
    index[4] = 2;
    index[5] = 3;

    // unlock buffers
    vertexBuffer->Unlock();
    indexBuffer->Unlock();

    // render
    _dxCR( dxSetRenderState( D3DRS_LIGHTING, FALSE ) );    
    _dxCR( dxSetRenderState( D3DRS_COLORVERTEX, TRUE ) );    
    _dxCR( dxSetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 ) );
    _dxCR( dxSetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL ) );

    _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
    _dxCR( iDirect3DDevice->SetFVF( particleFVF ) );
    _dxCR( iDirect3DDevice->SetStreamSource( 0, vertexBuffer, 0, sizeof( ParticleVertex ) ) );
    _dxCR( iDirect3DDevice->SetIndices( indexBuffer ) );
    _dxCR( iDirect3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2 ) );

    _dxCR( dxSetRenderState( D3DRS_LIGHTING, TRUE ) ); 

    _dxCR( indexBuffer->Release() );
    _dxCR( vertexBuffer->Release() );
}

engine::DeviceState Engine::handleCooperativeLevel(void)
{
    // retrieve current cooperative level
    HRESULT result = iDirect3DDevice9->TestCooperativeLevel();

    // perform certain action
    switch( result )
    {
    case D3D_OK: return engine::dsOk;
    case D3DERR_DEVICELOST: return engine::dsLost;
    case D3DERR_DEVICENOTRESET: break;
    case D3DERR_DRIVERINTERNALERROR: return engine::dsDriverInternalError;
    default: throw Exception( "Unexpected result from IDirect3DDevice9::TestCooperativeLevel()" );
    }

    getCore()->logMessage( "Device is lost but not restored" );

    // release all lostable objects
    if( !_onLostTrigger )
    {
        for( Lostable::LostableI lostableI = Lostable::lostables.begin();
                                lostableI != Lostable::lostables.end();
                                lostableI++ )
        {
            (*lostableI)->onLostDevice();
        }
        _onLostTrigger = true;
    }

    // reset device
    result = iDirect3DDevice9->Reset( &presentParams );

    // perform certain action
    switch( result )
    {
    case D3D_OK: break;
    case D3DERR_DEVICELOST: return engine::dsLost;
    case D3DERR_DEVICENOTRESET: return engine::dsNotReset;
    case D3DERR_DRIVERINTERNALERROR: return engine::dsDriverInternalError;
    case D3DERR_INVALIDCALL: _canRestoreDevice = false; return engine::dsLost;
    case D3DERR_OUTOFVIDEOMEMORY: throw Exception( "Out of videomemory in IDirect3DDevice9::Reset()" );
    case E_OUTOFMEMORY: throw Exception( "Out of omemory in IDirect3DDevice9::Reset()" );
    default: PostQuitMessage(0); throw Exception( "Unexpected result from IDirect3DDevice9::Reset()" );
    }

    getCore()->logMessage( "Device is restored" );

    // restore all lostable objects
    for( Lostable::LostableI lostableI = Lostable::lostables.begin();
                             lostableI != Lostable::lostables.end();
                             lostableI++ )
    {
        (*lostableI)->onResetDevice();
    }
    _onLostTrigger = false;

    return engine::dsOk;
}

bool Engine::canRestoreDevice(void)
{
    return _canRestoreDevice;
}