
#include "headers.h"
#include "effect.h"
#include "../common/istring.h"
#include "camera.h"

/**
 * effect : module local
 */

void EffectHLSL::render(Mesh* mesh, int subsetId, Shader* shader)
{
    shader->apply();

    // setup effect
    // ...

    // ambient lighting
    _effect->SetVector( "ambientColor", (D3DXVECTOR4*)( Shader::globalAmbient() ) );

    Matrix world;
    _dxCR( iDirect3DDevice->GetTransform( D3DTS_WORLD, &world ) );
    Matrix iWorld;
    D3DXMatrixInverse( &iWorld, NULL, &world );

    // camera properties (in object space)
    if( _flags.camera )
    {
        Vector cameraPos;
        Vector cameraDir;
        
        D3DXVec3TransformCoord( &cameraPos, &Camera::eyePos, &iWorld );
        D3DXVec3TransformNormal( &cameraDir, &Camera::eyeDirection, &iWorld );

        Quartector cameraPosQ( cameraPos.x, cameraPos.y, cameraPos.z, 1.0f );
        Quartector cameraDirQ( cameraDir.x, cameraDir.y, cameraDir.z, 0.0f );

        _effect->SetVector( "cameraPos", &cameraPosQ );
        _effect->SetVector( "cameraDir", &cameraDirQ );
    }

    // dynamic lighting (transformed in object space)
    if( _flags.lightPalette )
    {
        D3DLIGHT9     lightProperties;
        BOOL          lightEnabled;
        Quartector    lightPos[8];
        D3DCOLORVALUE lightColor[8];
        Vector        temp, temp2;

        unsigned int numLights = 0;
        for( unsigned int lightId=0; lightId<8; lightId++ )
        {
            // exit cycle when palette is full
            if( numLights == _flags.lightPalette ) break;
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

        if( _flags.lightPalette == 1 && numLights == 1 )
        {
            _effect->SetVector( "lightPos", lightPos );
            _effect->SetVector( "lightColor", (D3DXVECTOR4*)lightColor );
        }
        else if( numLights )
        {
            _effect->SetInt( "numLights", numLights );
            _effect->SetVectorArray( "lightPos", lightPos, numLights );
            _effect->SetVectorArray( "lightColor", (D3DXVECTOR4*)lightColor, numLights );
        }
    }

    // material color properties
    if( _flags.material )
    {        
        D3DMATERIAL9 material;
        iDirect3DDevice->GetMaterial( &material );
        _effect->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
        _effect->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
        _effect->SetFloat( "materialPower", material.Power );
    }

    // world matrix
    if( _flags.world )
    {
        _effect->SetMatrix( "world", &world );                   
    }

    // inversion of world matrix
    if( _flags.iWorld )
    {        
        _effect->SetMatrix( "iWorld", &iWorld );
    }

    // WVP matrix
    if( _flags.worldViewProj )
    {        
        Matrix worldViewProj;
        D3DXMatrixMultiply( &worldViewProj, &world, &Camera::viewMatrix );
        D3DXMatrixMultiply( &worldViewProj, &worldViewProj, &Camera::projectionMatrix );
        _effect->SetMatrix( "worldViewProj", &worldViewProj );
    }

    // base texture
    if( _flags.baseTexture )
    {
        _effect->SetTexture( "baseTexture", shader->layerTexture( 0 )->iDirect3DTexture() );
    }

    // normal map 
    if( _flags.normalMap )
    {
        _effect->SetTexture( "normalMap", shader->normalMap()->iDirect3DTexture() );
    }

    // environment map
    if( _flags.environmentMap )
    {
        _effect->SetTexture( "environmentMap", shader->environmentMap()->iDirect3DCubeTexture() );
    }
    
    // arguments
    for( unsigned int i=0; i<_arguments.size(); i++ )
    {
        switch( _arguments[i].type )
        {
        case vtInt:
            _dxCR( _effect->SetInt( 
                _arguments[i].name.c_str(), 
                *reinterpret_cast<int*>( _buffer + _arguments[i].offset )
            ) );
            break;
        case vtFloat:
            _dxCR( _effect->SetFloat( 
                _arguments[i].name.c_str(), 
                *reinterpret_cast<float*>( _buffer + _arguments[i].offset )
            ) );
            break;
        case vtVector4f:
            _dxCR( _effect->SetVector( 
                _arguments[i].name.c_str(),
                reinterpret_cast<Quartector*>( _buffer + _arguments[i].offset )
            ) );
            break;
        default:
            assert( !"shouldn't be here!" );
        }
    }

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

    if( _flags.baseTexture ) _effect->SetTexture( "baseTexture", NULL );
    if( _flags.normalMap ) _effect->SetTexture( "normalMap", NULL );
    if( _flags.environmentMap ) _effect->SetTexture( "environmentMap", NULL );

    iDirect3DDevice->SetVertexShader(NULL);
    iDirect3DDevice->SetPixelShader(NULL);
}

/**
 * HLSL argument
 */

EffectHLSL::Argument::Argument(const char* descriptor)
{
    StringP typeName;
    strseparate( descriptor, ",", typeName );
    typeName.first = strtrim( typeName.first, " " );
    typeName.second = strtrim( typeName.second, " " );

    if( typeName.first == "int" )
    {
        type = vtInt;
        size = sizeof( int );
    }
    else if( typeName.first == "float" )
    {
        type = vtFloat;
        size = sizeof( float );
    }
    else if( typeName.first == "vector" )
    {
        type = vtVector4f;
        size = sizeof( Quartector );
    }
    else
    {
        throw Exception( "Unsupported argument type: %s", typeName.first.c_str() );
    }

    name = typeName.second;
    offset = 0;
}

/**
 * class implementation
 */
    
EffectHLSL::EffectHLSL(const char* fxName)
{   
    Effect::HLSLI hlslI = _hlslMap.find( fxName );
    assert( hlslI != _hlslMap.end() );

    _name   = hlslI->first;
    _effect = hlslI->second;

    _bufferSize = 0;

    // enumerate arguments
    unsigned int numArguments = _hlslConfig->findParams( strformat( "effect.%s.argument*", fxName ).c_str() );
    paramid_t* items = new paramid_t[numArguments];
    _hlslConfig->copyParamResult( items );
    std::string argumentDesc;
    for( unsigned int i=0; i<numArguments; i++ )
    {
        argumentDesc = _hlslConfig->gets( items[i] );
        Argument argument( argumentDesc.c_str() );
        argument.offset = _bufferSize;
        _bufferSize += argument.size;
        _arguments.push_back( argument );
    }
    delete items;

    _buffer = new char[_bufferSize];
    memset( _buffer, 0, _bufferSize );

    // read flags
    memset( &_flags, 0, sizeof( Flags ) );
    _flags.camera         = _hlslConfig->getv( strformat( "effect.%s.camera", fxName ).c_str(), false );
    _flags.lightPalette   = _hlslConfig->getv( strformat( "effect.%s.lightPalette", fxName ).c_str(), 0 );
    _flags.material       = _hlslConfig->getv( strformat( "effect.%s.material", fxName ).c_str(), false );
    _flags.world          = _hlslConfig->getv( strformat( "effect.%s.world", fxName ).c_str(), false );
    _flags.iWorld         = _hlslConfig->getv( strformat( "effect.%s.iWorld", fxName ).c_str(), false );
    _flags.worldViewProj  = _hlslConfig->getv( strformat( "effect.%s.worldViewProj", fxName ).c_str(), false );
    _flags.baseTexture    = _hlslConfig->getv( strformat( "effect.%s.baseTexture", fxName ).c_str(), false );
    _flags.normalMap      = _hlslConfig->getv( strformat( "effect.%s.normalMap", fxName ).c_str(), false );
    _flags.environmentMap = _hlslConfig->getv( strformat( "effect.%s.environmentMap", fxName ).c_str(), false );    
}

EffectHLSL::~EffectHLSL()
{
    delete[] _buffer;
}

/**
 * IEffect implemetation
 */
 
const char* EffectHLSL::getName(void)
{
    return _name.c_str();
}

int EffectHLSL::getNumArguments(void)
{
    return _arguments.size();
}

const char* EffectHLSL::getArgumentName(int argId)
{
    assert( argId>=0 && argId<getNumArguments() );
    return _arguments[argId].name.c_str();
}

Variant EffectHLSL::getArgument(int argId)
{
    assert( argId>=0 && argId<getNumArguments() );    
    switch( _arguments[argId].type )
    {
    case vtInt:
        return Variant( *reinterpret_cast<int*>( _buffer + _arguments[argId].offset ) );
    case vtFloat:
        return Variant( *reinterpret_cast<float*>( _buffer + _arguments[argId].offset ) );
    case vtVector4f:
        return Variant( wrapQ( *reinterpret_cast<Quartector*>( _buffer + _arguments[argId].offset ) ) );
    default:
        assert( !"shouldn't be here!" );
    }
    return Variant( 0 );
}

void EffectHLSL::setArgument(int argId, const Variant& value)
{
    assert( argId>=0 && argId<getNumArguments() );
    assert( _arguments[argId].type == value.type );

    switch( _arguments[argId].type )
    {
    case vtInt:
        *reinterpret_cast<int*>( _buffer + _arguments[argId].offset ) = value;
        break;
    case vtFloat:
        *reinterpret_cast<float*>( _buffer + _arguments[argId].offset ) = value;
        break;
    case vtVector4f:
        *reinterpret_cast<Quartector*>( _buffer + _arguments[argId].offset ) = wrapQ( value.operator Vector4f() );
        break;
    default:
        assert( !"shouldn't be here!" );
    }
}

bool EffectHLSL::isCompatible(engine::IShader* shader)
{
    return ( _flags.baseTexture ? ( shader->getNumLayers() > 0 ) : true ) ||
           ( _flags.environmentMap ? ( shader->getEnvironmentMap() != NULL ) : true );
}

bool EffectHLSL::isCompatible(engine::IGeometry* geometry)
{
    return false;
}