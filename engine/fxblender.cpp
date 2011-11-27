
#include "headers.h"
#include "shader.h"
#include "../common/istring.h"
#include "camera.h"
#include "effect.h"
#include "light.h"
#include "geometry.h"
#include "vertexdeclaration.h"

const char* FxBlender::effectName = "Blender";

ID3DXEffect*    FxBlender::_effect = NULL;
StaticLostable* FxBlender::_effectLostable = NULL;

Quartector      FxBlender::_lightPosition[BLENDER_MAX_NUM_LIGHTS];
D3DCOLORVALUE   FxBlender::_lightDiffuseColor[BLENDER_MAX_NUM_LIGHTS];
D3DCOLORVALUE   FxBlender::_lightSpecularColor[BLENDER_MAX_NUM_LIGHTS];

/**
 * public Effect implemetation
 */

void FxBlender::render(Mesh* mesh, int subsetId, Shader* shader)
{
    // apply shader
    shader->apply();

    // override texture blending
    // first texture is copying over surface
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );

    // second texture is copying in to temporary register
    _dxCR( dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_TEMP ) );

    // third texture is blended with first texture by second texture
    _dxCR( dxSetTextureStageState( 2, D3DTSS_COLOROP,  D3DTOP_LERP ) );
    _dxCR( dxSetTextureStageState( 2, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
    _dxCR( dxSetTextureStageState( 2, D3DTSS_COLORARG2, D3DTA_CURRENT ) );
    _dxCR( dxSetTextureStageState( 2, D3DTSS_COLORARG0, D3DTA_TEMP ) );
    _dxCR( dxSetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_LERP ) );
    _dxCR( dxSetTextureStageState( 2, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );
    _dxCR( dxSetTextureStageState( 2, D3DTSS_ALPHAARG2, D3DTA_CURRENT ) );
    _dxCR( dxSetTextureStageState( 2, D3DTSS_ALPHAARG0, D3DTA_TEMP ) );

    // lighting
    if( shader->getFlags() & engine::sfLighting )
    {
        _dxCR( dxSetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_MODULATE ) );
        _dxCR( dxSetTextureStageState( 3, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
        _dxCR( dxSetTextureStageState( 3, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );
        _dxCR( dxSetTextureStageState( 3, D3DTSS_ALPHAOP, D3DTOP_MODULATE ) );
        _dxCR( dxSetTextureStageState( 3, D3DTSS_ALPHAARG1, D3DTA_CURRENT ) );
        _dxCR( dxSetTextureStageState( 3, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ) );
        _dxCR( dxSetTextureStageState( 4, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
        _dxCR( dxSetTextureStageState( 4, D3DTSS_COLOROP, D3DTOP_DISABLE ) );
    }
    else
    {
        _dxCR( dxSetTextureStageState( 3, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
        _dxCR( dxSetTextureStageState( 3, D3DTSS_COLOROP, D3DTOP_DISABLE ) );
    }

    // render mesh subset
    mesh->renderSubset( subsetId, shader );

    // if details texture are present - render details layer
    if( ( shader->getNumLayers() > 3 ) &&  _effect )
    {
        // setup alpha-blending
        _dxCR( dxSetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) );
        _dxCR( dxSetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA  ) );
        _dxCR( dxSetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ) );
        _dxCR( dxSetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD  ) );
        _dxCR( dxSetRenderState( D3DRS_ALPHATESTENABLE, TRUE ) );
        _dxCR( dxSetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATER ) );
        _dxCR( dxSetRenderState( D3DRS_ALPHAREF, 0 ) );
        _dxCR( dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );

        // setup fixed function pipeline for texturing
        shader->layerTexture(3)->apply(0);
        _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ) );
        _dxCR( dxSetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
        _dxCR( dxSetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE ) );

        // setup transformation matrices
        Matrix world;
        Matrix worldViewProj;    
        iDirect3DDevice->GetTransform( D3DTS_WORLD, &world );
        D3DXMatrixIdentity( &worldViewProj );
        D3DXMatrixMultiply( &worldViewProj, &world, &Camera::viewMatrix );
        D3DXMatrixMultiply( &worldViewProj, &worldViewProj, &Camera::projectionMatrix );
        _effect->SetMatrix( "world", &world );
        _effect->SetMatrix( "worldViewProj", &worldViewProj );

        // ambient lighting
        _effect->SetVector( "ambientColor", (D3DXVECTOR4*)( Shader::globalAmbient() ) );

        // camera position
        Quartector cameraPosQ( Camera::eyePos.x, Camera::eyePos.y, Camera::eyePos.z, 1.0f );
        _effect->SetVector( "cameraPos", &cameraPosQ );

        // setup dynamic lighting
        D3DLIGHT9     lightProperties;
        BOOL          lightEnabled;
        UINT          iLight;
        UINT          iEffectLight = 0;
        for( iLight=0; iLight<8; iLight++ )
        {
            iDirect3DDevice->GetLightEnable( iLight, &lightEnabled );
            if( lightEnabled )
            {
                iDirect3DDevice->GetLight( iLight, &lightProperties );
                if( lightProperties.Type == D3DLIGHT_POINT )
                {                    
                    _lightPosition[iEffectLight].x = lightProperties.Position.x;
                    _lightPosition[iEffectLight].y = lightProperties.Position.y;
                    _lightPosition[iEffectLight].z = lightProperties.Position.z;
                    _lightDiffuseColor[iEffectLight] = lightProperties.Diffuse;
                    _lightSpecularColor[iEffectLight] = lightProperties.Specular;
                    iEffectLight++;
                    if( iEffectLight > GRASS_MAX_NUM_LIGHTS ) break;
                }
            }
        }
        _effect->SetInt( "curNumLights", iEffectLight );
        if( iEffectLight )
        {
            _effect->SetVectorArray( "lightPosition", _lightPosition, iEffectLight );
            _effect->SetVectorArray( "lightDiffuseColor", (D3DXVECTOR4*)&_lightDiffuseColor, iEffectLight );
            _effect->SetVectorArray( "lightSpecularColor", (D3DXVECTOR4*)&_lightSpecularColor, iEffectLight );
        }

        // setup material color properties
        D3DMATERIAL9 material;
        iDirect3DDevice->GetMaterial( &material );
        _effect->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
        _effect->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
        _effect->SetFloat( "materialPower", material.Power );

        // setup effect
        _effect->SetFloat( "layerOffset", _arguments.detailsLayerOffset );
        _effect->SetFloat( "uvMultiplier", _arguments.detailsUVMultiplier );        

        // render layer
        unsigned int numPasses, pass;
        _effect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
        for( pass = 0; pass < numPasses; pass++ )
        {
            _effect->BeginPass( pass );
            mesh->renderSubset( subsetId, shader );
			_effect->EndPass();
        }
        _effect->End();

        // reset vertex shader
        iDirect3DDevice->SetVertexShader(NULL);

        // enable zwrite
        _dxCR( dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );
    }

    // cleanup critical stage states
    _dxCR( dxSetTextureStageState( 1, D3DTSS_RESULTARG, D3DTA_CURRENT ) );
}

/**
 * class implementation
 */
    
FxBlender::FxBlender()
{
    if( _effect == NULL )
    {
        LPD3DXBUFFER compilationErrors = NULL;
        
        if( S_OK != D3DXCreateEffectFromFile( 
            iDirect3DDevice, 
            "./res/effects/details.fx", 
            NULL,
            NULL, 
            D3DXSHADER_DEBUG,
            NULL,
            &_effect,
            &compilationErrors
        ) )
        {
            const char* text = (const char*)( compilationErrors->GetBufferPointer() );
            MessageBox( 0, text, "Effect compilation error", MB_OK );
            compilationErrors->Release();
        }

        _effectLostable = new StaticLostable( onLostDevice, onResetDevice );
    }

    _arguments.detailsLayerOffset  = 1.0f;
    _arguments.detailsUVMultiplier = 1.0f;
}

/**
 * IEffect implemetation
 */
    
const char* FxBlender::getName(void)
{
    return effectName;
}

int FxBlender::getNumArguments(void)
{
    return 2;
}

const char* FxBlender::getArgumentName(int argId)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: return "Details layer offset";
    case 1: return "Details UV multiplier";
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

Variant FxBlender::getArgument(int argId)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: return Variant( _arguments.detailsLayerOffset );
    case 1: return Variant( _arguments.detailsUVMultiplier );
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

void FxBlender::setArgument(int argId, const Variant& value)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: 
        _arguments.detailsLayerOffset = value;
        break;
    case 1:
        _arguments.detailsUVMultiplier = value;
        break;
    default:
        assert( !"shouldn't be here!" );
    }
}

bool FxBlender::isCompatible(engine::IShader* shader)
{
    return ( shader != NULL ) && ( shader->getNumLayers() >= 3 );
}

bool FxBlender::isCompatible(engine::IGeometry* geometry)
{
    return false;
}

/**
 * resource management
 */

void FxBlender::releaseResources(void)
{
    if( _effect ) _effect->Release();
    if( _effectLostable ) delete _effectLostable;
}

/**
 * lostable support
 */

void FxBlender::onLostDevice(void)
{
    if( _effect ) _effect->OnLostDevice();
}

void FxBlender::onResetDevice(void)
{
    if( _effect ) _effect->OnResetDevice();
}