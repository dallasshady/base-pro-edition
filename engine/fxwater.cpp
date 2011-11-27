
#include "headers.h"
#include "shader.h"
#include "../common/istring.h"
#include "camera.h"
#include "effect.h"
#include "light.h"
#include "geometry.h"
#include "vertexdeclaration.h"

ID3DXEffect*    FxWater::_effect = NULL;
StaticLostable* FxWater::_effectLostable = NULL;
const char* FxWater::effectName = "Water";

/**
 * protected Effect implemetation
 */

void FxWater::onUpdate(float dt)
{
    _arguments.uvOffset1 += _arguments.uvVelocity1 * dt;
    _arguments.uvOffset2 += _arguments.uvVelocity2 * dt;
}

/**
 * public Effect implemetation
 */

void FxWater::render(Mesh* mesh, int subsetId, Shader* shader)
{
    if( !_effect ) 
    {
        mesh->renderSubset( subsetId, shader );
        return;
    }

    shader->apply();

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
    Quartector    lightPos;
    D3DCOLORVALUE lightColor;
    Vector        temp, temp2;

    for( unsigned int lightId=0; lightId<8; lightId++ )
    {        
        // check light is enabled
        _dxCR( iDirect3DDevice->GetLightEnable( lightId, &lightEnabled ) );
        if( !lightEnabled ) continue;
        // retrieve light properties
        _dxCR( iDirect3DDevice->GetLight( lightId, &lightProperties ) );
        if( lightProperties.Type == D3DLIGHT_POINT )
        {
            // retrieve light props
            temp.x = lightProperties.Position.x;
            temp.y = lightProperties.Position.y;
            temp.z = lightProperties.Position.z;
            D3DXVec3TransformCoord( &temp2, &temp, &iWorld );
            lightPos.x = temp2.x;
            lightPos.y = temp2.y;
            lightPos.z = temp2.z;
            lightPos.w = 1.0f;
            lightColor = lightProperties.Diffuse;
            // setup effect
            _effect->SetVector( "lightPos", (D3DXVECTOR4*)&lightPos );
            _effect->SetVector( "lightColor", (D3DXVECTOR4*)&lightColor );
            break;
        }
    }

    // material color properties
    D3DMATERIAL9 material;
    iDirect3DDevice->GetMaterial( &material );
    _effect->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
    _effect->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
    _effect->SetFloat( "materialPower", material.Power );

    // world matrix
    // _effect->SetMatrix( "world", &world );

    // inversion of world matrix
    // _effect->SetMatrix( "iWorld", &iWorld );

    // WVP matrix
    Matrix worldViewProj;
    D3DXMatrixMultiply( &worldViewProj, &world, &Camera::viewMatrix );
    D3DXMatrixMultiply( &worldViewProj, &worldViewProj, &Camera::projectionMatrix );
    _effect->SetMatrix( "worldViewProj", &worldViewProj );

    // base texture
    _effect->SetTexture( "baseTexture", shader->layerTexture( 0 )->iDirect3DTexture() );

    // normal maps
    _effect->SetTexture( "normalMap", shader->normalMap()->iDirect3DTexture() );

    // environment map
    _effect->SetTexture( "environmentMap", shader->environmentMap()->iDirect3DCubeTexture() );    
    
    // arguments
    Quartector uvOffset1( _arguments.uvOffset1.x, _arguments.uvOffset1.y, 0, 0 );
    Quartector uvOffset2( _arguments.uvOffset2.x, _arguments.uvOffset2.y, 0, 0 );
    _effect->SetFloat( "Reflectivity", _arguments.reflectivity );
    _effect->SetVector( "UvOffset1", &uvOffset1 );
    _effect->SetVector( "UvOffset2", &uvOffset2 );

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
    _effect->SetTexture( "environmentMap", NULL );

    iDirect3DDevice->SetVertexShader(NULL);
    iDirect3DDevice->SetPixelShader(NULL);
}

/**
 * class implementation
 */
    
FxWater::FxWater()
{
    if( _effect == NULL )
    {
        LPD3DXBUFFER compilationErrors = NULL;
        
        if( S_OK != D3DXCreateEffectFromFile( 
            iDirect3DDevice, 
            "./res/effects/water.fx", 
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

    _arguments.reflectivity = 0.5f;
    _arguments.uvOffset1.x = 0, _arguments.uvOffset1.y = 0;
    _arguments.uvOffset2.x = 0, _arguments.uvOffset2.y = 0;
    _arguments.uvVelocity1.x = 0.7f, _arguments.uvVelocity1.y = -0.7f;
    _arguments.uvVelocity2.x = -0.7f, _arguments.uvVelocity2.y = 0.7f;

    _animatedEffects.push_back( this );
}

/**
 * IEffect implemetation
 */
    
const char* FxWater::getName(void)
{
    return effectName;
}

int FxWater::getNumArguments(void)
{
    return 3;
}

const char* FxWater::getArgumentName(int argId)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: return "Reflectivity";
    case 1: return "UvOffset0 velocity";
    case 2: return "UvOffset1 velocity";
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

Variant FxWater::getArgument(int argId)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: return Variant( _arguments.reflectivity );
    case 1: return Variant( wrap( _arguments.uvVelocity1 ) );
    case 2: return Variant( wrap( _arguments.uvVelocity2 ) );
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

void FxWater::setArgument(int argId, const Variant& value)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: 
        _arguments.reflectivity = value;
        break;
    case 1:
        _arguments.uvVelocity1 = wrap( Vector2f( value ) );
        break;
    case 2:
        _arguments.uvVelocity2 = wrap( Vector2f( value ) );
        break;
    default:
        assert( !"shouldn't be here!" );
    }
}

bool FxWater::isCompatible(engine::IShader* shader)
{
    return ( shader != NULL ) && ( shader->getNumLayers() >= 1 ) &&
           ( shader->getNormalMap() != NULL ) &&
           ( shader->getEnvironmentMap() != NULL );
}

bool FxWater::isCompatible(engine::IGeometry* geometry)
{
    return false;
}

void FxWater::releaseResources(void)
{
    if( _effect ) _effect->Release();
    if( _effectLostable ) delete _effectLostable;
}

void FxWater::onLostDevice(void)
{
    if( _effect ) _effect->OnLostDevice();
}

void FxWater::onResetDevice(void)
{
    if( _effect ) _effect->OnResetDevice();
}