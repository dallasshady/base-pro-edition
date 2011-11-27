
#include "headers.h"
#include "shader.h"
#include "../common/istring.h"
#include "camera.h"
#include "effect.h"
#include "light.h"
#include "geometry.h"
#include "vertexdeclaration.h"

ID3DXEffect*    FxGrass::_grassEffect = NULL;
Quartector      FxGrass::_lightPosition[GRASS_MAX_NUM_LIGHTS];
D3DCOLORVALUE   FxGrass::_lightDiffuseColor[GRASS_MAX_NUM_LIGHTS];
D3DCOLORVALUE   FxGrass::_lightSpecularColor[GRASS_MAX_NUM_LIGHTS];
StaticLostable* FxGrass::_grassLostable = NULL;

const char* FxGrass::effectName = "Grass";

/**
 * public Effect implemetation
 */

void FxGrass::render(Mesh* mesh, int subsetId, Shader* shader)
{
    if( !_grassEffect ) return;

    // apply shader
    shader->apply();

    // setup transformation matrices
    Matrix world;
    Matrix worldViewProj;    
    iDirect3DDevice->GetTransform( D3DTS_WORLD, &world );
    D3DXMatrixIdentity( &worldViewProj );
    D3DXMatrixMultiply( &worldViewProj, &world, &Camera::viewMatrix );
    D3DXMatrixMultiply( &worldViewProj, &worldViewProj, &Camera::projectionMatrix );
    _grassEffect->SetMatrix( "world", &world );
    _grassEffect->SetMatrix( "worldViewProj", &worldViewProj );

    // ambient lighting
    _grassEffect->SetVector( "ambientColor", (D3DXVECTOR4*)( Shader::globalAmbient() ) );

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
    _grassEffect->SetInt( "curNumLights", iEffectLight );
    if( iEffectLight )
    {
        _grassEffect->SetVectorArray( "lightPosition", _lightPosition, iEffectLight );
        _grassEffect->SetVectorArray( "lightDiffuseColor", (D3DXVECTOR4*)&_lightDiffuseColor, iEffectLight );
        _grassEffect->SetVectorArray( "lightSpecularColor", (D3DXVECTOR4*)&_lightSpecularColor, iEffectLight );
    }

    // setup material color properties
    D3DMATERIAL9 material;
    iDirect3DDevice->GetMaterial( &material );
    _grassEffect->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
    _grassEffect->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
    _grassEffect->SetFloat( "materialPower", material.Power );

    unsigned int numPasses, pass;
    float layerHeight = _arguments.height / _arguments.numLayers;
    for( unsigned int i=0; i<_arguments.numLayers; i++ )
    {
        _grassEffect->SetFloat( "layerOffset", (i+1)*layerHeight );
        _grassEffect->SetFloat( "layerAlpha", 0.9f-((0.8f/_arguments.numLayers)*i) );
        _grassEffect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
        for( pass = 0; pass < numPasses; pass++ )
        {
            _grassEffect->BeginPass( pass );
            mesh->renderSubset( subsetId, shader );
			_grassEffect->EndPass();
        }
        _grassEffect->End();
    }

    iDirect3DDevice->SetVertexShader(NULL);
}

/**
 * class implementation
 */
    
FxGrass::FxGrass()
{
    if( _grassEffect == NULL )
    {
        LPD3DXBUFFER compilationErrors = NULL;
        
        if( S_OK != D3DXCreateEffectFromFile( 
            iDirect3DDevice, 
            _T( "./res/effects/grass.fx" ), 
            NULL,
            NULL, 
            D3DXSHADER_DEBUG,
            NULL,
            &_grassEffect,
            &compilationErrors
        ) )
        {
            const char* text = (const char*)( compilationErrors->GetBufferPointer() );
            MessageBox( 0, text, "Effect compilation error", MB_OK );
            compilationErrors->Release();
        }

        _grassLostable = new StaticLostable( onLostDevice, onResetDevice );
    }

    _arguments.numLayers = 10;
    _arguments.height = 100.0f;
}

/**
 * IEffect implemetation
 */
    
const char* FxGrass::getName(void)
{
    return effectName;
}

int FxGrass::getNumArguments(void)
{
    return 2;
}

const char* FxGrass::getArgumentName(int argId)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: return "Number of layers";
    case 1: return "Grass height";
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

Variant FxGrass::getArgument(int argId)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: return Variant( int( _arguments.numLayers ) );
    case 1: return Variant( _arguments.height );
    default:
        assert( !"shouldn't be here!" );
        return NULL;
    }
}

void FxGrass::setArgument(int argId, const Variant& value)
{
    assert( argId>=0 && argId<getNumArguments() );
    switch( argId )
    {
    case 0: 
        _arguments.numLayers = value;
        break;
    case 1:
        _arguments.height = value;
        break;
    default:
        assert( !"shouldn't be here!" );
    }
}

bool FxGrass::isCompatible(engine::IShader* shader)
{
    return ( shader != NULL ) && ( shader->getNumLayers() >= 1 );
}

bool FxGrass::isCompatible(engine::IGeometry* geometry)
{
    return false;
}

void FxGrass::releaseResources(void)
{
    if( _grassEffect ) _grassEffect->Release();
    if( _grassLostable ) delete _grassLostable;
}

void FxGrass::onLostDevice(void)
{
    if( _grassEffect ) _grassEffect->OnLostDevice();
}

void FxGrass::onResetDevice(void)
{
    if( _grassEffect ) _grassEffect->OnResetDevice();
}