
#include "headers.h"
#include "shader.h"
#include "engine.h"
#include "camera.h"
#include "../common/istring.h"
#include "asset.h"
#include "effect.h"

/**
 * creation routine
 */

engine::IShader* Engine::createShader(int numLayers, const char* shaderName)
{
    return new Shader( numLayers, shaderName );
}

/**
 * class implementation
 */

D3DCOLORVALUE Shader::_globalAmbient;
Shader*       Shader::_lastShader = NULL;

Shader::Shader(int numLayers, const char* shaderName)
{
    _numReferences = 1;
    _name = shaderName ? shaderName : "UnnamedShader";

    assert( numLayers>=0 && numLayers<=engine::maxTextureLayers );
    _numLayers = numLayers;

    for( int i=0; i<engine::maxTextureLayers; i++ )
    {
        _layerTexture[i] = NULL;
        _layerBlending[i] = engine::btOver;
        _layerUV[i] = 0;
        _layerConstant[i] = D3DCOLOR_RGBA( 0,0,0,0 );
    }

    dxSetColorValue( &_materialColor.Diffuse, 1,1,1,1 );
    dxSetColorValue( &_materialColor.Ambient, 0,0,0,1 );
    dxSetColorValue( &_materialColor.Specular, 0,0,0,1 );
    dxSetColorValue( &_materialColor.Emissive, 0,0,0,1 );
    _materialColor.Power = 0;

    _normalMap = NULL;
    _normalMapUV = 0;

    _environmentMap = NULL;

    _flags             = engine::sfCulling | engine::sfCollision | engine::sfCaching | engine::sfLighting;
    _lightset          = 0;
    _srcBlend          = D3DBLEND_SRCALPHA;
    _dstBlend          = D3DBLEND_INVSRCALPHA;
    _blendOp           = D3DBLENDOP_ADD;
    _alphaTestFunction = D3DCMP_GREATER;
    _alphaTestRef      = 128;
    _effect            = NULL;

    // reset hemisphere
    _hemisphere[0].r = _hemisphere[0].g = _hemisphere[0].b =
    _hemisphere[1].r = _hemisphere[1].g = _hemisphere[1].b = 0.0f;
    _hemisphere[0].a = _hemisphere[1].a = 1.0f;

    // reset cinematic lighting
    _illuminationColor.r = _illuminationColor.g = _illuminationColor.b = 0.0f;
    _illuminationColor.a = 1.0f;
    _contourColor.r = _contourColor.g = _contourColor.b = 0.0f;
    _contourColor.a = 1.0f;
}

Shader::~Shader()
{
    assert( _numReferences == 1 );

    for( int i=0; i<_numLayers; i++ )
    {
        if( _layerTexture[i] ) _layerTexture[i]->release();
    }

    if( _normalMap ) _normalMap->release();
    if( _environmentMap ) 
    {
        _environmentMap->release();
    }
}

/**
 * IShader
 */

void Shader::addReference(void)
{
    _numReferences++;
}

int Shader::getNumReferences(void)
{
    return _numReferences;
}

void Shader::release(void)
{
    _numReferences--;
    if( _numReferences == 1 ) delete this;
}

const char* Shader::getName(void)
{
    return _name.c_str();
}

int Shader::getNumLayers(void)
{
    return _numLayers;
}

void Shader::setNumLayers(int numLayers, engine::ITexture* defaultTexture)
{
    // if number of textures will be decreased
    if( _numLayers > numLayers )
    {
        // remove unnecessary textures
        for( int i=numLayers; i<_numLayers; i++ )
        {
            setLayerTexture( i, NULL );
        }
        // reintialize counter
        _numLayers = numLayers;
    }
    else if( _numLayers < numLayers )
    {
        // remember previous number of layers
        int prevNumLayers = _numLayers;
        // reintialize counter
        _numLayers = numLayers;
        // initialize fresh textures
        for( int i=prevNumLayers; i<_numLayers; i++ )
        {
            setLayerTexture( i, defaultTexture );
        }
    }
}

engine::BlendingType Shader::getLayerBlending(int layerId)
{
    assert( layerId>=0 && layerId<_numLayers );
    return _layerBlending[layerId];
}

void Shader::setLayerBlending(int layerId, engine::BlendingType blending)
{
    assert( layerId>=0 && layerId<_numLayers );
    _layerBlending[layerId] = blending;
}

engine::ITexture* Shader::getLayerTexture(int layerId)
{
    assert( layerId>=0 && layerId<_numLayers );
    return _layerTexture[layerId];
}

void Shader::setLayerTexture(int layerId, engine::ITexture* texture)
{
    assert( layerId>=0 && layerId<_numLayers );

    Texture* t = dynamic_cast<Texture*>( texture );
    if( t == _layerTexture[layerId] ) return;
    if( _layerTexture[layerId] ) _layerTexture[layerId]->release();
    if( t ) t->_numReferences++;    
    _layerTexture[layerId] = t;
}

int Shader::getLayerUV(int layerId)
{
    assert( layerId>=0 && layerId<_numLayers );
    return _layerUV[layerId];
}

void Shader::setLayerUV(int layerId, int uvId)
{
    assert( layerId>=0 && layerId<_numLayers );
    _layerUV[layerId] = uvId;
}

Vector4f Shader::getLayerConstant(int layerId)
{
    assert( layerId>=0 && layerId<_numLayers );
    return wrap( _layerConstant[layerId] );
}

void Shader::setLayerConstant(int layerId, const Vector4f& layerConstant)
{
    assert( layerId>=0 && layerId<_numLayers );
    _layerConstant[layerId] = wrap( layerConstant );
}

Vector4f Shader::getDiffuseColor(void)
{
    return _wrap( _materialColor.Diffuse );
}

void Shader::setDiffuseColor(const Vector4f& value)
{
    _materialColor.Diffuse = _wrap( value );
}

Vector4f Shader::getSpecularColor(void)
{
    return _wrap( _materialColor.Specular );
}

void Shader::setSpecularColor(const Vector4f& value)
{
    _materialColor.Specular = _wrap( value );
}

float Shader::getSpecularPower(void)
{
    return _materialColor.Power;
}

void Shader::setSpecularPower(float value)
{
    _materialColor.Power = value;
}

engine::ITexture* Shader::getNormalMap(void)
{
    return _normalMap;
}

void Shader::setNormalMap(engine::ITexture* texture)
{
    if( _normalMap == dynamic_cast<Texture*>( texture ) ) return;

    if( _normalMap ) _normalMap->release();
    _normalMap = dynamic_cast<Texture*>( texture );
    if( _normalMap ) _normalMap->_numReferences++;
}

int Shader::getNormalMapUV(void)
{
    return _normalMapUV;
}

void Shader::setNormalMapUV(int normalMapUV)
{
    _normalMapUV = normalMapUV;
}

engine::ITexture* Shader::getEnvironmentMap(void)
{
    return _environmentMap;
}

void Shader::setEnvironmentMap(engine::ITexture* texture)
{
    Texture* t = dynamic_cast<Texture*>( texture );
    if( t && t->iDirect3DCubeTexture() == NULL )
    {
        assert( !"Cubemap expected!" );
        return;
    }

    if( _environmentMap == t ) return;

    if( _environmentMap ) _environmentMap->release();
    _environmentMap = t;
    if( _environmentMap ) _environmentMap->_numReferences++;
}

/** 
 * flagging
 */

unsigned int Shader::getFlags(void)
{
    return _flags;
}

void Shader::setFlags(unsigned int flags)
{
    _flags = flags;
}

engine::BlendingMode Shader::getSrcBlend(void)
{
    return wrap( _srcBlend );
}

void Shader::setSrcBlend(engine::BlendingMode blendMode)
{
    _srcBlend = wrap( blendMode );
}

engine::BlendingMode Shader::getDestBlend(void)
{
    return wrap( _dstBlend );
}

void Shader::setDestBlend(engine::BlendingMode blendMode)
{
    _dstBlend = wrap( blendMode );
}

engine::BlendingOperation Shader::getBlendOp(void)
{
    return wrap( _blendOp );    
}

void Shader::setBlendOp(engine::BlendingOperation blendOp)
{
    _blendOp = wrap( blendOp );    
}

engine::CompareFunction Shader::getAlphaTestFunction(void)
{
    switch( _alphaTestFunction )
    {
    case D3DCMP_NEVER: return engine::cfNever;
    case D3DCMP_LESS: return engine::cfLess;
    case D3DCMP_EQUAL: return engine::cfEqual;
    case D3DCMP_LESSEQUAL: return engine::cfLessEqual;
    case D3DCMP_GREATER: return engine::cfGreater;
    case D3DCMP_NOTEQUAL: return engine::cfNotEqual;
    case D3DCMP_GREATEREQUAL: return engine::cfGreaterEqual;
    case D3DCMP_ALWAYS: return engine::cfAlways;
    default:
        assert( !"shouldn't be here!" );
        return engine::cfAlways;
    }
}

void Shader::setAlphaTestFunction(engine::CompareFunction function)
{
    switch( function )
    {
    case engine::cfNever:
        _alphaTestFunction = D3DCMP_NEVER;
        break;
    case engine::cfLess:
        _alphaTestFunction = D3DCMP_LESS;
        break;
    case engine::cfEqual:
        _alphaTestFunction = D3DCMP_EQUAL;
        break;
    case engine::cfLessEqual:
        _alphaTestFunction = D3DCMP_LESSEQUAL;
        break;
    case engine::cfGreater:
        _alphaTestFunction = D3DCMP_GREATER;
        break;
    case engine::cfNotEqual:
        _alphaTestFunction = D3DCMP_NOTEQUAL;
        break;
    case engine::cfGreaterEqual:
        _alphaTestFunction = D3DCMP_GREATEREQUAL;
        break;
    case engine::cfAlways:
        _alphaTestFunction = D3DCMP_ALWAYS;
        break;
    default:
        assert( !"shouldn't be gere!" );
        _alphaTestFunction = D3DCMP_ALWAYS;
    }
}

unsigned char Shader::getAlphaTestRef(void)
{
    return ((unsigned char)( _alphaTestRef ));
}

void Shader::setAlphaTestRef(unsigned char alphaRef)
{
    _alphaTestRef = alphaRef;
}

unsigned int Shader::getLightset(void)
{
    return _lightset;
}

void Shader::setLightset(unsigned int value)
{
    _lightset = value;
}

engine::IEffect* Shader::getEffect(void)
{
    return reinterpret_cast<Effect*>( _effect );
}

bool Shader::setEffect(engine::IEffect* effect)
{
    Effect* e = dynamic_cast<Effect*>( effect );
    if( e && !e->isCompatible( this ) ) return false;
    _effect = e;
    return true;
}

Vector4f Shader::getUpperHemisphere(void)
{
    return Vector4f( _hemisphere[0].r, _hemisphere[0].g, _hemisphere[0].b, 1.0f );
}

Vector4f Shader::getLowerHemisphere(void)
{
    return Vector4f( _hemisphere[1].r, _hemisphere[1].g, _hemisphere[1].b, 1.0f );
}

void Shader::setHemisphere(Vector4f upperColor, Vector4f lowerColor)
{
    _hemisphere[0].r = upperColor[0],
    _hemisphere[0].g = upperColor[1],
    _hemisphere[0].b = upperColor[2],
    _hemisphere[0].a = 1.0f;
    _hemisphere[1].r = lowerColor[0],
    _hemisphere[1].g = lowerColor[1],
    _hemisphere[1].b = lowerColor[2],
    _hemisphere[1].a = 1.0f;
}

Vector4f Shader::getIlluminationColor(void)
{
    return Vector4f( 
        _illuminationColor.r,
        _illuminationColor.g,
        _illuminationColor.b,
        _illuminationColor.a
    );
}

Vector4f Shader::getContourColor(void)
{
    return Vector4f( 
        _contourColor.r,
        _contourColor.g,
        _contourColor.b,
        _contourColor.a
    );
}

void Shader::setIlluminationColor(const Vector4f& value)
{
    _illuminationColor.r = value[0];
    _illuminationColor.g = value[1];
    _illuminationColor.b = value[2];
    _illuminationColor.a = value[3];
}

void Shader::setContourColor(const Vector4f& value)
{
    _contourColor.r = value[0];
    _contourColor.g = value[1];
    _contourColor.b = value[2];
    _contourColor.a = value[3];
}

/**
 * shading technique 
 */

void Shader::apply(void)
{
    // simple caching
    if( ( _lightset == 0 ) && ( _flags & engine::sfCaching ) && ( _lastShader == this ) ) 
    {
        Engine::instance->statistics.shaderCacheHits++;
        return;
    }
    _lastShader = this;

    // setup lighting
    if( _lightset == 0 )
    {
        _materialColor.Emissive = _globalAmbient;    
        dxModulateColorValue( &_materialColor.Emissive, &_materialColor.Diffuse );
        _dxCR( dxSetMaterial( &_materialColor ) );
    }
    else
    {
        BSP::currentBSP->calculateGlobalAmbient( _lightset );
        BSPSector::currentSector->illuminate( _lightset );
        _materialColor.Emissive = _globalAmbient;    
        dxModulateColorValue( &_materialColor.Emissive, &_materialColor.Diffuse );
        _dxCR( dxSetMaterial( &_materialColor ) );
    }

    // current lightmap (if exists)
    Texture* lightmap = NULL;
    if( BSPSector::currentSector )
    {
        if( BSPSector::currentSector->lightmap() )
        {
            lightmap = BSPSector::currentSector->lightmap();
        }
        if( Atomic::currentAtomic && Atomic::currentAtomic->lightmap() )
        {
            lightmap = Atomic::currentAtomic->lightmap();
        }
    }

    // cannot cache shaders with lightmaps
    if( lightmap ) _lastShader = NULL;

    // setup culling    
    if( _flags & engine::sfCulling )
    {
        _dxCR( dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CW ) );
    }
    else
    {
        _dxCR( dxSetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) );
    }

    // setup alpha-blending
    if( _flags & engine::sfAlphaBlending )
    {
        _dxCR( dxSetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) );
        _dxCR( dxSetRenderState( D3DRS_SRCBLEND, _srcBlend ) );
        _dxCR( dxSetRenderState( D3DRS_DESTBLEND, _dstBlend ) );
        _dxCR( dxSetRenderState( D3DRS_BLENDOP, _blendOp ) );
    }
    else
    {
        _dxCR( dxSetRenderState( D3DRS_ALPHABLENDENABLE, FALSE ) );
    }

    // setup alpha-testing
    if( _flags & engine::sfAlphaTesting )
    {               
        _dxCR( dxSetRenderState( D3DRS_ALPHATESTENABLE, TRUE ) );
        _dxCR( dxSetRenderState( D3DRS_ALPHAFUNC, _alphaTestFunction ) );
        _dxCR( dxSetRenderState( D3DRS_ALPHAREF, _alphaTestRef ) );
    }
    else
    {
        _dxCR( dxSetRenderState( D3DRS_ALPHATESTENABLE, FALSE ) );
    }

    // fixed function pipeline : first texture
    if( _numLayers )
    {
        assert( _layerTexture[0] );
        _layerTexture[0]->apply( 0 );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, _layerUV[0] ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );
    }

    // fixed function pipeline : multitextures
    for( int stageId=1; stageId<_numLayers; stageId++ )
    {
        assert( _layerTexture[stageId] );
        _layerTexture[stageId]->apply( stageId );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_TEXCOORDINDEX, _layerUV[stageId] ) );
        switch( _layerBlending[stageId] )
        {
        case engine::btOver:
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_SELECTARG2 ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
        case engine::btAdd:
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_ADD ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
            break;
        case engine::btSub:
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_SUBTRACT ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
            break;
        case engine::btAddSigned:
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_ADDSIGNED ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
            break;
        case engine::btAddSmooth:
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_ADDSMOOTH ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
            break;
        case engine::btModulate:
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_MODULATE ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
            break;
        case engine::btModulate2x:
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_MODULATE2X ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
            break;
        case engine::btModulate4x:
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_MODULATE4X ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
            break;
        case engine::btBlendTextureAlpha:
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_BLENDTEXTUREALPHA ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_CURRENT ) );
            break;
        case engine::btLerp:
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_LERP ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG0, D3DTA_CONSTANT ) );
            _dxCR( dxSetTextureStageState( stageId, D3DTSS_CONSTANT, _layerConstant[stageId] ) );
            break;
        default:
            assert( !"Shouldn't be here!" );
        }

        _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAARG1, D3DTA_CURRENT ) );
    }

    // lighting
    if( _flags & engine::sfLighting ) 
    {
        if( lightmap )
        {
            if( _numLayers )
            {
                lightmap->apply( stageId );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_TEXCOORDINDEX, 1 ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_MODULATE ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAOP, D3DTOP_MODULATE ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAARG1, D3DTA_CURRENT ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAARG2, D3DTA_TEXTURE ) );
                stageId++;
            }
            else
            {
                lightmap->apply( 0 );
                _dxCR( dxSetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 1 ) );
                _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
                _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
                _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
                _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );
                stageId++;
            }
        }
        else
        {
            if( _numLayers )
            {
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_MODULATE ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAOP, D3DTOP_MODULATE ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAARG1, D3DTA_CURRENT ) );
                _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ) );
                stageId++;
            }
            else
            {
                _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 ) );
                _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );
                _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 ) );
                _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ) );
            }
        }
    }

    // normal map ( for programmable pipeline only )
    /*if( _normalMap )
    {
        _normalMap->apply( stageId );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_TEXCOORDINDEX, 0 ) );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAARG1, D3DTA_CURRENT ) );
        stageId++;
    }*/

    // environment map ( for programmable pipeline only )
    /*if( _environmentMap )
    {
        _environmentMap->apply( stageId );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_TEXCOORDINDEX, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR | 1 ) ); 
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_ADD ) );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLORARG2, D3DTA_TEXTURE ) );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
        _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAARG1, D3DTA_CURRENT ) );
        stageId++;
    }*/

    _dxCR( dxSetTextureStageState( stageId, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
    _dxCR( dxSetTextureStageState( stageId, D3DTSS_COLOROP, D3DTOP_DISABLE ) );
}

/**
 * serialization
 */

void Shader::write(IResource* resource)
{
    ChunkHeader shaderHeader( BA_SHADER, sizeof( Chunk ) );
    shaderHeader.write( resource );

    Chunk chunk;
    chunk.id = (auid)( this );
    memset( chunk.name, 0, engine::maxNameLength );
    if( _name.length() > engine::maxNameLength )
    {
        strncpy( chunk.name, _name.c_str(), engine::maxNameLength - 1 );
    }
    else
    {
        strcpy( chunk.name, _name.c_str() );
    }
    chunk.numLayers = _numLayers;
    for( int i=0; i<_numLayers; i++ )
    {
        chunk.layerTexture[i] = (auid)( _layerTexture[i] );
        chunk.layerUV[i] = _layerUV[i];
        chunk.layerBlending[i] = _layerBlending[i];
        chunk.layerConstant[i] = _layerConstant[i];
    }
    chunk.materialColor = _materialColor;
    chunk.normalMap = (auid)( _normalMap );
    chunk.normalMapUV = _normalMapUV;  

    chunk.environmentMap = (auid)( _environmentMap );

    chunk.flags = _flags;
    chunk.lightset = _lightset;
    chunk.alphaTestFunction = _alphaTestFunction;
    chunk.alphaTestRef = _alphaTestRef;
    chunk.blendOp = _blendOp;
    chunk.srcBlend = _srcBlend;
    chunk.dstBlend = _dstBlend;

    chunk.hasEffect = ( _effect != NULL );

    fwrite( &chunk, sizeof(Chunk), 1, resource->getFile() );

    if( _effect )
    {
        reinterpret_cast<Effect*>( _effect )->write( resource );
    }
}

AssetObjectT Shader::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader shaderHeader( resource );
    if( shaderHeader.type != BA_SHADER ) throw Exception( "Unexpected chunk type" );
    if( shaderHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, sizeof(Chunk), 1, resource->getFile() );

    Shader* shader = new Shader( chunk.numLayers, chunk.name );
    for( int i=0; i<chunk.numLayers; i++ )
    {
        AssetObjectI assetObjectI = assetObjects.find( chunk.layerTexture[i] );
        if( assetObjectI != assetObjects.end() )
        {
            shader->setLayerTexture( i, reinterpret_cast<Texture*>( assetObjectI->second ) );
        }
        shader->_layerUV[i] = chunk.layerUV[i];
        shader->_layerBlending[i] = chunk.layerBlending[i];
        shader->_layerConstant[i] = chunk.layerConstant[i];
    }
    shader->_materialColor = chunk.materialColor;
    AssetObjectI assetObjectI = assetObjects.find( chunk.normalMap );
    if( assetObjectI != assetObjects.end() )
    {
        shader->setNormalMap( reinterpret_cast<Texture*>( assetObjectI->second ) );
    }
    shader->_normalMapUV = chunk.normalMapUV;

    assetObjectI = assetObjects.find( chunk.environmentMap );
    if( assetObjectI != assetObjects.end() )
    {
        shader->setEnvironmentMap( reinterpret_cast<Texture*>( assetObjectI->second ) );
    }

    shader->_flags = chunk.flags | engine::sfCaching | engine::sfLighting;
    shader->_lightset = chunk.lightset;
    shader->_alphaTestFunction = chunk.alphaTestFunction;
    shader->_alphaTestRef = chunk.alphaTestRef;
    shader->_blendOp = chunk.blendOp;
    shader->_srcBlend = chunk.srcBlend;
    shader->_dstBlend = chunk.dstBlend;

    if( chunk.hasEffect )
    {
        AssetObjectT assetObjectT = Effect::read( resource, assetObjects );
        shader->setEffect( reinterpret_cast<Effect*>( assetObjectT.second ) );
        assetObjects.insert( assetObjectT );
    }
    
    return AssetObjectT( chunk.id, shader );
}