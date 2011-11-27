
#include "headers.h"
#include "camera.h"
#include "wire.h"
#include "sprite.h"
#include "../common/istring.h"

ID3DXEffect*    CameraEffect::_pEffect         = NULL;
StaticLostable* CameraEffect::_pEffectLostable = NULL;

/**
 * creation routine
 */

engine::ICameraEffect* Engine::createCameraEffect(engine::ITexture* renderTarget)
{
    Texture* texture = dynamic_cast<Texture*>( renderTarget );
    assert( texture );
    return new CameraEffect( texture );
}

/**
 * validation routine
 */

bool Engine::isPfxSupported(engine::PostEffectType pfxType)
{
    // FFP-effects are supported a priori
    if( pfxType == engine::pfxNone ||
        pfxType == engine::pfxMotionBlur )
    {
        return true;
    }
    // shader-based effects are supported only with corresponding hardware
    return ( CameraEffect::_pEffect != NULL );
}

/**
 * CameraEffect
 */

CameraEffect::CameraEffect(Texture* renderTarget)
{
    assert( renderTarget->iDirect3DTexture() );

    _frame = NULL;

    _renderTarget = renderTarget;
    _renderTarget->_numReferences++;

    _currentEffect = engine::pfxNone;

    _newImage      = NULL;
    _prevImage     = NULL;
    _effectTexture = NULL;
    _prevIsEmpty = true;

    _weight = 0;
    _quality = 0;

    // create render-to-surface for specified render target
    _dxCR( D3DXCreateRenderToSurface( 
        iDirect3DDevice,
        _renderTarget->getWidth(),
        _renderTarget->getHeight(),
        _renderTarget->getFormat(),
        true,
        dxPresentParams.AutoDepthStencilFormat,
        &_rts
    ) );    

    // setup viewport
    _viewPort.X = 0, _viewPort.Y = 0;
    _viewPort.Width  = renderTarget->getWidth();
    _viewPort.Height = renderTarget->getHeight();
    _viewPort.MinZ   = 0;
    _viewPort.MaxZ   = 1;
    _fov = 60.0f;
    _nearClipPlane = 0;
    _farClipPlane = 100;
    updateProjection();
}

CameraEffect::~CameraEffect()
{
    if( _frame ) _frame->release();

    _rts->Release();

    _renderTarget->release();
    if( _newImage ) _newImage->release();
    if( _prevImage ) _prevImage->release();
    if( _effectTexture ) _effectTexture->release();
}

/**
 * ICamera
 */

void CameraEffect::release(void)
{
    delete this;
}

void CameraEffect::beginScene(unsigned int clearMode, const Vector4f& clearColor)
{
    _effectBlocked = true;

    assert( Camera::_currentCamera == NULL );
    Camera::_currentCamera = this;

    // frame synchronization phase
    Frame::synchronizeAll();

    _dxCR( iDirect3DDevice->SetViewport( &_viewPort ) );

    if( _currentEffect == engine::pfxNone ||
        _currentEffect == engine::pfxBloom )
    {
        IDirect3DSurface9* iDirect3DSurface;
        _renderTarget->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface );
        _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
        iDirect3DSurface->Release();
    }
    else
    {
        IDirect3DSurface9* iDirect3DSurface;
        _newImage->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface );
        _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
        iDirect3DSurface->Release();
    }

    if( clearMode ) _dxCR( iDirect3DDevice->Clear( 0, NULL, clearMode, wrap( clearColor ), 1.0f, 0L ) );

    // setup transformation matrices
    if( _frame ) 
    {
        D3DXMatrixInverse( &Camera::viewMatrix, NULL, &_frame->LTM );
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_VIEW, &Camera::viewMatrix ) );
    }
    Camera::projectionMatrix = _projection;
    _dxCR( iDirect3DDevice->SetTransform( D3DTS_PROJECTION, &_projection ) );
    
    Camera::viewPort      = _viewPort;
    Camera::eyePos        = dxPos( &_frame->LTM );
    Camera::eyeDirection  = dxAt( &_frame->LTM );
    Camera::fov           = _fov;
    Camera::nearClipPlane = _nearClipPlane;
    Camera::farClipPlane  = _farClipPlane;

    // normalize camera eye
    D3DXVec3Normalize( &eyeDirection, &eyeDirection );

    // frustrum planes
    updateFrustrum();   
}

void CameraEffect::endScene(void)
{
    assert( Camera::_currentCamera == this );
    Camera::_currentCamera = NULL;

    _dxCR( _rts->EndScene( D3DX_FILTER_LINEAR ) );    
    _effectBlocked = false;
}

/**
 * ICameraEffect
 */

engine::ICamera* CameraEffect::getCamera(void)
{
    return this;
}

engine::PostEffectType CameraEffect::getPfx(void)
{
    return _currentEffect;
}

void CameraEffect::setPfx(engine::PostEffectType pfxType)
{
    assert( Engine::instance->isPfxSupported( pfxType ) );
    
    _currentEffect = pfxType;

    switch( _currentEffect )
    {
    case engine::pfxMotionBlur:
        if( _newImage ) _newImage->release();
        if( _prevImage ) _prevImage->release();
        _newImage = Texture::createRenderTarget( 
            _renderTarget->getWidth(), _renderTarget->getHeight(), 32,
            strformat( "%s_newImageBuffer", _renderTarget->getName() ).c_str()
        );
        _prevImage = Texture::createRenderTarget( 
            _renderTarget->getWidth(), _renderTarget->getHeight(), 32,
            strformat( "%s_prevImageBuffer", _renderTarget->getName() ).c_str()
        );        
        _prevIsEmpty = true;
        break;
    case engine::pfxDOF:
        if( _newImage ) _newImage->release();
        _newImage = Texture::createRenderTarget( 
            _renderTarget->getWidth(), _renderTarget->getHeight(), 32,
            strformat( "%s_newImageBuffer", _renderTarget->getName() ).c_str()
        );
        break;
    case engine::pfxBloom:
        if( _newImage ) _newImage->release();
        _newImage = Texture::createRenderTarget( 
            _renderTarget->getWidth(), _renderTarget->getHeight(), 32,
            strformat( "%s_newImageBuffer", _renderTarget->getName() ).c_str()
        );
        break;
    }
}

unsigned int CameraEffect::getQuality(void)
{
    return _quality;
}

void CameraEffect::setQuality(unsigned int quality)
{
    // valid quality values are 0,1 and 2
    if( quality > 2 ) quality = 2;
    _quality = quality;            
}

float CameraEffect::getWeight(void)
{
    return _weight;
}

void CameraEffect::setWeight(float value)
{
    _weight = value;
}

Vector4f CameraEffect::getVector(void)
{
    return Vector4f( _vector.x, _vector.y, _vector.z, _vector.w );
}

void CameraEffect::setVector(const Vector4f& value)
{
    _vector = Quartector( value[0], value[1], value[2], value[3] );
}

engine::ITexture* CameraEffect::getTexture(void)
{
    return _effectTexture;
}

void CameraEffect::setTexture(engine::ITexture* texture)
{
    if( _effectTexture ) _effectTexture->release();
    _effectTexture = dynamic_cast<Texture*>( texture );
    if( _effectTexture ) _effectTexture->_numReferences++;
}

/**
 * effect code
 */

void CameraEffect::applyMotionBlur(void)
{
    if( _prevIsEmpty || _weight == 0.0f )
    {
        _prevIsEmpty = false;
        
        // setup new image for texture rendering
        _newImage->apply( 0 );
        dxSetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
        dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

        // render new image in to the previous image        
        IDirect3DSurface9* iDirect3DSurface;
        _dxCR( _prevImage->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface ) );
        _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
        iDirect3DSurface->Release();        
        _dxCR( iDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, black, 1.0f, 0L ) );
        dxRenderRect( 
            0, 0, 
            Engine::instance->screenWidth-1, Engine::instance->screenHeight-1, 
            white 
        );
        _dxCR( _rts->EndScene( D3DX_FILTER_LINEAR ) );

        // render new image in to the render target
        _dxCR( _renderTarget->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface ) );
        _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
        iDirect3DSurface->Release();
        _dxCR( iDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, black, 1.0f, 0L ) );
        dxRenderRect( 
            0, 0, 
            Engine::instance->screenWidth-1, Engine::instance->screenHeight-1, 
            white 
        );
        _dxCR( _rts->EndScene( D3DX_FILTER_LINEAR ) );
    }
    else
    {
        // setup blending of new image & previous image
        _prevImage->apply( 0 );
        _newImage->apply( 1 );

        Color w = D3DCOLOR_RGBA( int(255 * _weight), int(255 * _weight), int(255 * _weight), 255 );

        dxSetRenderState( D3DRS_TEXTUREFACTOR, w );

        dxSetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

        dxSetTextureStageState( 1, D3DTSS_TEXCOORDINDEX, 0 );
        dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_LERP );
        dxSetTextureStageState( 1, D3DTSS_COLORARG0, D3DTA_TFACTOR );
        dxSetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
        dxSetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
        dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT );

        dxSetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
        dxSetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE );

        // blend new image & prev image into the render target
        IDirect3DSurface9* iDirect3DSurface;
        _dxCR( _renderTarget->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface ) );
        _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
        iDirect3DSurface->Release();
        _dxCR( iDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, black, 1.0f, 0L ) );
        dxRenderRect( 
            0, 0, 
            Engine::instance->screenWidth-1, Engine::instance->screenHeight-1, 
            white
        );
        _rts->EndScene( D3DX_FILTER_LINEAR );

        // setup rendering of render target
        _renderTarget->apply( 0 );
        dxSetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
        dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
        dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
        dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
        dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE );
        dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE );

        // copy target image into the previous image       
        _dxCR( _prevImage->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface ) );
        _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
        iDirect3DSurface->Release();
        _dxCR( iDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, black, 1.0f, 0L ) );
        dxRenderRect( 
            0, 0, 
            Engine::instance->screenWidth-1, Engine::instance->screenHeight-1, 
            white 
        );        
        _dxCR( _rts->EndScene( D3DX_FILTER_LINEAR ) );
    }
}

void CameraEffect::applyDOF(void)
{
    assert( _effectTexture );
    
    // setup effect parameters    
    _pEffect->SetInt( "quality", _quality );
    _pEffect->SetFloat( "weight", _weight );
    _pEffect->SetFloat( "screenWidth", float( Engine::instance->screenWidth ) );
    _pEffect->SetFloat( "screenHeight", float( Engine::instance->screenHeight ) );
    _pEffect->SetTexture( "newImage", _newImage->iDirect3DTexture() );
    _pEffect->SetTexture( "argumentImage", _effectTexture->iDirect3DTexture() );    
    _pEffect->SetTechnique( "DepthOfField" );    

    // blend new image & depth map into the render target
    IDirect3DSurface9* iDirect3DSurface;
    _dxCR( _renderTarget->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface ) );
    _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
    iDirect3DSurface->Release();
    _dxCR( iDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, black, 1.0f, 0L ) );
    UINT numPasses;
    _pEffect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( UINT iPass = 0; iPass < numPasses; iPass++ )
    {   
        _pEffect->BeginPass( iPass );
        dxRenderRect( 
            0, 0, 
            Engine::instance->screenWidth-1, Engine::instance->screenHeight-1, 
            white
        );
        _pEffect->EndPass();
    }
    _pEffect->End();
    iDirect3DDevice->SetPixelShader(NULL);
    _rts->EndScene( D3DX_FILTER_LINEAR );

    _pEffect->SetTexture( "newImage", NULL );
    _pEffect->SetTexture( "argumentImage", NULL );
}

void CameraEffect::applyBloom(void)
{
    // 1) _renderTarget -> _newImage with BrightPass effect
    // setup effect
    _pEffect->SetInt( "quality", _quality );
    _pEffect->SetFloat( "weight", _weight );
    _pEffect->SetFloat( "screenWidth", float( _renderTarget->getWidth() ) );
    _pEffect->SetFloat( "screenHeight", float( _renderTarget->getHeight() ) );
    _pEffect->SetTexture( "newImage", _renderTarget->iDirect3DTexture() );
    _pEffect->SetTechnique( "BrightPass" );
    // render effect
    IDirect3DSurface9* iDirect3DSurface;
    _dxCR( _newImage->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface ) );
    _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
    iDirect3DSurface->Release();
    _dxCR( iDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, black, 1.0f, 0L ) );
    UINT numPasses;
    _pEffect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( UINT iPass = 0; iPass < numPasses; iPass++ )
    {   
        _pEffect->BeginPass( iPass );
        dxRenderRect( 
            0, 0, 
            _renderTarget->getWidth()-1, _renderTarget->getHeight()-1,
            white
        );
        _pEffect->EndPass();
    }
    _pEffect->End();
    _rts->EndScene( D3DX_FILTER_LINEAR );

    // 2) setup bloom
    float filterWidth  = float( _renderTarget->getWidth() );
    float filterHeight = float( _renderTarget->getHeight() );

    Quartector texelKernel[13];
    for( int i=0; i<13; i++ )
    {
        texelKernel[i] = Quartector(
	    float( i-6 ) / filterWidth,
	    float( i-6 ) / filterHeight,
            0,0
        );
    }	    
    _pEffect->SetVectorArray( "TexelKernel", texelKernel, 13 );

    // 3) _newImage -> _renderTarget with HorizontalBloom effect
    // setup effect
    _pEffect->SetInt( "quality", _quality );
    _pEffect->SetFloat( "weight", _vector[0] );
    _pEffect->SetFloat( "screenWidth", float( _renderTarget->getWidth() ) );
    _pEffect->SetFloat( "screenHeight", float( _renderTarget->getHeight() ) );
    _pEffect->SetTexture( "newImage", _newImage->iDirect3DTexture() );
    _pEffect->SetTechnique( "HorizontalBloom" );        
    // render effect
    _dxCR( _renderTarget->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface ) );
    _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
    iDirect3DSurface->Release();
    _dxCR( iDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, black, 1.0f, 0L ) );
    _pEffect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( iPass = 0; iPass < numPasses; iPass++ )
    {   
        _pEffect->BeginPass( iPass );
        dxRenderRect( 
            0, 0, 
            _renderTarget->getWidth()-1, _renderTarget->getHeight()-1,
            white
        );
        _pEffect->EndPass();
    }
    _pEffect->End();
    _rts->EndScene( D3DX_FILTER_LINEAR );

    // 4) _renderTarget -> _newImage with VerticalBloom effect
    // setup effect
    _pEffect->SetInt( "quality", _quality );
    _pEffect->SetFloat( "weight", _vector[0] );
    _pEffect->SetFloat( "screenWidth", float( _renderTarget->getWidth() ) );
    _pEffect->SetFloat( "screenHeight", float( _renderTarget->getHeight() ) );
    _pEffect->SetTexture( "newImage", _renderTarget->iDirect3DTexture() );
    _pEffect->SetTechnique( "VerticalBloom" );        
    // render effect
    _dxCR( _newImage->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface ) );
    _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
    iDirect3DSurface->Release();
    _dxCR( iDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, black, 1.0f, 0L ) );
    _pEffect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( iPass = 0; iPass < numPasses; iPass++ )
    {   
        _pEffect->BeginPass( iPass );
        dxRenderRect( 
            0, 0, 
            _renderTarget->getWidth()-1, _renderTarget->getHeight()-1,
            white
        );
        _pEffect->EndPass();
    }
    _pEffect->End();    
    _rts->EndScene( D3DX_FILTER_LINEAR );

    // 5) copy _newImage to _renderTarget with no effect
    // setup effect
    _pEffect->SetInt( "quality", _quality );
    _pEffect->SetFloat( "weight", _vector[0] );
    _pEffect->SetFloat( "screenWidth", float( _renderTarget->getWidth() ) );
    _pEffect->SetFloat( "screenHeight", float( _renderTarget->getHeight() ) );
    _pEffect->SetTexture( "newImage", _newImage->iDirect3DTexture() );
    _pEffect->SetTechnique( "CopySurface" );        
    // render effect
    _dxCR( _renderTarget->iDirect3DTexture()->GetSurfaceLevel( 0, &iDirect3DSurface ) );
    _dxCR( _rts->BeginScene( iDirect3DSurface, &_viewPort ) );
    iDirect3DSurface->Release();
    _dxCR( iDirect3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, black, 1.0f, 0L ) );
    _pEffect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( iPass = 0; iPass < numPasses; iPass++ )
    {   
        _pEffect->BeginPass( iPass );
        dxRenderRect( 
            0, 0, 
            _renderTarget->getWidth()-1, _renderTarget->getHeight()-1,
            white
        );
        _pEffect->EndPass();
    }
    _pEffect->End();    
    _rts->EndScene( D3DX_FILTER_LINEAR );

    // reset shaders    
    iDirect3DDevice->SetPixelShader(NULL);
    _pEffect->SetTexture( "newImage", NULL );
}

void CameraEffect::applyEffect(void)
{
    assert( _effectBlocked == false );

    switch( _currentEffect )
    {
    case engine::pfxMotionBlur: applyMotionBlur(); break;
    case engine::pfxDOF: applyDOF(); break;
    case engine::pfxBloom: applyBloom(); break;
    default:
        assert( !"shouldn't be here!" );
    };
}

/**
 * initialization & etc.
 */

void CameraEffect::init(void)
{
    // check pixel shader version
    DWORD hiVersion = ( dxDeviceCaps.PixelShaderVersion & 0x0000FF00 ) >> 8;
    DWORD loVersion = ( dxDeviceCaps.PixelShaderVersion & 0x000000FF );
    if( hiVersion < 2 ) return;

    // create effect
    LPD3DXBUFFER compilationErrors = NULL;       
    if( S_OK != D3DXCreateEffectFromFile( 
        iDirect3DDevice, 
        "./res/effects/camera.fx",
        NULL,
        NULL, 
        D3DXSHADER_DEBUG, 
        NULL, 
        &_pEffect, 
        &compilationErrors
    ) )
    {
        const char* text = (const char*)( compilationErrors->GetBufferPointer() );
        MessageBox( 0, text, "Effect compilation error", MB_OK );        
        compilationErrors->Release();
    }

    // create lostable object
    _pEffectLostable = new StaticLostable( onStaticLostDevice, onStaticResetDevice );
}

void CameraEffect::term(void)
{
    if( _pEffect ) _pEffect->Release();
    if( _pEffectLostable ) delete _pEffectLostable;
}

void CameraEffect::onStaticLostDevice(void)
{
    if( _pEffect ) _pEffect->OnLostDevice();
}

void CameraEffect::onStaticResetDevice(void)
{
    if( _pEffect ) _pEffect->OnResetDevice();    
}

void CameraEffect::onLostDevice(void)
{
    if( _rts ) _rts->OnLostDevice();
}

void CameraEffect::onResetDevice(void)
{
    if( _rts ) _rts->OnResetDevice();
}