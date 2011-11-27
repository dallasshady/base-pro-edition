
#include "headers.h"
#include "shader.h"
#include "../common/istring.h"
#include "camera.h"
#include "effect.h"
#include "light.h"
#include "geometry.h"
#include "vertexdeclaration.h"

ID3DXEffect*    FxWaterfall::_effect = NULL;
StaticLostable* FxWaterfall::_effectLostable = NULL;
Quartector      FxWaterfall::_lightPosition[WATERFALL_MAX_NUM_LIGHTS];
D3DCOLORVALUE   FxWaterfall::_lightDiffuseColor[WATERFALL_MAX_NUM_LIGHTS];
D3DCOLORVALUE   FxWaterfall::_lightSpecularColor[WATERFALL_MAX_NUM_LIGHTS];
const char*     FxWaterfall::effectName = "Waterfall";

/**
 * protected Effect implemetation
 */

void FxWaterfall::onUpdate(float dt)
{
    _arguments.uvOffset += _arguments.uvVelocity * dt;
}

/**
 * public Effect implemetation
 */

void FxWaterfall::render(Mesh* mesh, int subsetId, Shader* shader)
{    
    shader->apply();
    
    // setup effect arguments
    Quartector uvOffset( _arguments.uvOffset.x, _arguments.uvOffset.y, 0, 0 );
    _effect->SetVector( "UvOffset", &uvOffset );

    // ambient lighting
    _effect->SetVector( "ambientColor", (D3DXVECTOR4*)( Shader::globalAmbient() ) );

    Matrix world;
    _dxCR( iDirect3DDevice->GetTransform( D3DTS_WORLD, &world ) );
    Matrix iWorld;
    D3DXMatrixInverse( &iWorld, NULL, &world );

    // camera properties (in object space)
    Vector cameraPos;
    Vector cameraDir;       
    D3DXVec3TransformCoord( &cameraPos, &Camera::eyePos, &iWorld );
    D3DXVec3TransformNormal( &cameraDir, &Camera::eyeDirection, &iWorld );
    Quartector cameraPosQ( cameraPos.x, cameraPos.y, cameraPos.z, 1.0f );
    Quartector cameraDirQ( cameraDir.x, cameraDir.y, cameraDir.z, 0.0f );
    _effect->SetVector( "cameraPos", &cameraPosQ );
    _effect->SetVector( "cameraDir", &cameraDirQ );

    // dynamic lighting (transformed in object space)
    D3DLIGHT9     lightProperties;
    BOOL          lightEnabled;
    Quartector    lightPos[8];
    D3DCOLORVALUE lightColor[8];
    Vector        temp, temp2;

    unsigned int numLights = 0;
    for( unsigned int lightId=0; lightId<8; lightId++ )
    {
        // exit cycle when palette is full
        if( numLights == 1 ) break;
        // check light is enabled
        _dxCR( iDirect3DDevice->GetLightEnable( lightId, &lightEnabled ) );
        if( !lightEnabled ) continue;
        // retrieve light properties
        _dxCR( iDirect3DDevice->GetLight( lightId, &lightProperties ) );
        if( lightProperties.Type == D3DLIGHT_POINT )
        {
            temp.x = lightProperties.Position.x;
            temp.y = lightProperties.Position.y;
            temp.z = lightProperties.Position.z;
            D3DXVec3TransformCoord( &temp2, &temp, &iWorld );
                lightPos[numLights].x = temp2.x;
                lightPos[numLights].y = temp2.y;
                lightPos[numLights].z = temp2.z;
                lightPos[numLights].w = 1.0f;
                lightColor[numLights] = lightProperties.Diffuse;
                numLights++;
        }
    }

    if( numLights )
    {
        _effect->SetVector( "lightPos", lightPos );
        _effect->SetVector( "lightColor", (D3DXVECTOR4*)lightColor );
    }        

    // material color properties
    D3DMATERIAL9 material;
    iDirect3DDevice->GetMaterial( &material );
    _effect->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
    _effect->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
    _effect->SetFloat( "materialPower", material.Power );

    // world matrix
    // _effect->SetMatrix( "world", &world );                   

    // WVP matrix
    Matrix worldViewProj;
    D3DXMatrixMultiply( &worldViewProj, &world, &Camera::viewMatrix );
    D3DXMatrixMultiply( &worldViewProj, &worldViewProj, &Camera::projectionMatrix );
    _effect->SetMatrix( "worldViewProj", &worldViewProj );

    // base texture
    _effect->SetTexture( "baseTexture", shader->layerTexture( 0 )->iDirect3DTexture() );    

    // normal map 
    _effect->SetTexture( "normalMap", shader->normalMap()->iDirect3DTexture() );

    // environment map
    // _effect->SetTexture( "environmentMap", shader->environmentMap()->iDirect3DCubeTexture() );
    
    // render
    unsigned int numPasses;
    _effect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( unsigned int pass = 0; pass < numPasses; pass++ )
    {
        _effect->BeginPass( pass );
        mesh->renderSubset( subsetId, shader );
		_effect->EndPass();
    }
    _effect->End();

    _effect->SetTexture( "baseTexture", NULL );
    _effect->SetTexture( "normalMap", NULL );
    // _effect->SetTexture( "environmentMap", NULL );

    iDirect3DDevice->SetVertexShader(NULL);
    iDirect3DDevice->SetPixelShader(NULL);
}

/**
 * class implementation
 */
    
FxWaterfall::FxWaterfall()
{
    if( _effect == NULL )
    {
        LPD3DXBUFFER compilationErrors = NULL;
        
        if( S_OK != D3DXCreateEffectFromFile( 
            iDirect3DDevice, 
            "./res/effects/waterfall.fx", 
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

    _arguments.uvOffset.x = 0, _arguments.uvOffset.y = 0;
    _arguments.uvVelocity.x = 0.0f, _arguments.uvVelocity.y = 0.0f;

    _animatedEffects.push_back( this );
}

/**
 * IEffect implemetation
 */
    
const char* FxWaterfall::getName(void)
{
    return effectName;
}

int FxWaterfall::getNumArguments(void)
{
    return 1;
}

const char* FxWaterfall::getArgumentName(int argId)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: return "UvOffset velocity";
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

Variant FxWaterfall::getArgument(int argId)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: return Variant( wrap( _arguments.uvVelocity ) );
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

void FxWaterfall::setArgument(int argId, const Variant& value)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: 
        _arguments.uvVelocity = wrap( Vector2f( value ) );
        break;    
    default:
        assert( !"shouldn't be here!" );
    }
}

bool FxWaterfall::isCompatible(engine::IShader* shader)
{
    return ( shader != NULL ) && 
           ( shader->getNumLayers() >= 1 ) &&
           ( shader->getNormalMap() != NULL );
}

bool FxWaterfall::isCompatible(engine::IGeometry* geometry)
{
    return false;
}

void FxWaterfall::releaseResources(void)
{
    if( _effect ) _effect->Release();
    if( _effectLostable ) delete _effectLostable;
}

void FxWaterfall::onLostDevice(void)
{
    if( _effect ) _effect->OnLostDevice();
}

void FxWaterfall::onResetDevice(void)
{
    if( _effect ) _effect->OnResetDevice();
}