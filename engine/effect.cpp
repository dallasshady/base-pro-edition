
#include "headers.h"
#include "shader.h"
#include "../common/istring.h"
#include "camera.h"
#include "effect.h"
#include "asset.h"
#include "hlsl.h"

/**
 * effect enumeration
 */

int Engine::getNumEffects(void)
{
    return Effect::getNumEffects();
}

const char* Engine::getEffectName(int effectId)
{
    return Effect::getEffectName( effectId );
}

engine::IEffect* Engine::createEffect(const char* effectName)
{
    return Effect::create( effectName );
}

/**
 * effect animation
 */

Effect::EffectL Effect::_animatedEffects;
Effect::HLSLMap Effect::_hlslMap;
IParamPack*     Effect::_hlslConfig = NULL;
StaticLostable* Effect::_hlslLostable = NULL;

Effect::~Effect()
{
    for( EffectI effectI = _animatedEffects.begin();
                 effectI != _animatedEffects.end();
                 effectI++ )
    {
        if( *effectI == this )
        {
            _animatedEffects.erase( effectI );
            break;
        }
    }
}

/**
 * serialization
 */ 

void Effect::write(IResource* resource)
{
    ChunkHeader effectHeader( BA_EFFECT, sizeof(Chunk) );
    effectHeader.write( resource );

    Chunk chunk;
    chunk.id = (auid)( this );
    memset( chunk.name, 0, engine::maxNameLength );
    if( strlen( getName() ) > engine::maxNameLength )
    {
        strncpy( chunk.name, getName(), engine::maxNameLength - 1 );
    }
    else
    {
        strcpy( chunk.name, getName() );
    }

    chunk.hasArguments = ( getBufferSize() != 0 );

    fwrite( &chunk, sizeof(Chunk), 1, resource->getFile() );

    if( chunk.hasArguments )
    {
        ChunkHeader argumentsHeader( BA_BINARY, getBufferSize() );
        argumentsHeader.write( resource );
        fwrite( getBuffer(), getBufferSize(), 1, resource->getFile() );
    }
}

AssetObjectT Effect::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader effectHeader( resource );
    if( effectHeader.type != BA_EFFECT ) throw Exception( "Unexpected chunk type" );
    if( effectHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, sizeof(chunk), 1, resource->getFile() );

    Effect* effect = Effect::create( chunk.name );
    if( effect && chunk.hasArguments )
    {
        ChunkHeader argumentsHeader( resource );
        if( argumentsHeader.type != BA_BINARY ) throw Exception( "Unexpected chunk type" );
        if( argumentsHeader.size != effect->getBufferSize() ) throw Exception( "Incompatible binary asset version" );
        fread( effect->getBuffer(), effect->getBufferSize(), 1, resource->getFile() );
    }

    return AssetObjectT( chunk.id, effect );
}

/**
 * creation routine
 */

const numEffectClasses = 4; // number of complicated effects

int Effect::getNumEffects(void)
{
    return numEffectClasses + _hlslMap.size();
}

const char* Effect::getEffectName(int effectId)
{
    assert( effectId>=0 && effectId<getNumEffects() );

    // complicated?
    if( effectId < numEffectClasses )
    {
        switch( effectId )
        {
        case 0: return FxGrass::effectName;
        case 1: return FxWater::effectName;
        case 2: return FxBlender::effectName;
        case 3: return FxWaterfall::effectName;
        default: assert( !"shouldn't be here!" );
        }
    }
    // hlsl?
    else
    {
        effectId -= numEffectClasses;
        HLSLI hlslI = _hlslMap.begin();
        for( unsigned int i=0; i<_hlslMap.size(); i++, hlslI++ )
        {
            if( i == effectId ) return hlslI->first.c_str();
        }
        assert( !"shouldn't be here!" );
    }

    return NULL;
}

Effect* Effect::create(const char* effectName)
{
    // hlsl-effect?
    HLSLI hlslI = _hlslMap.find( effectName );
    if( hlslI != _hlslMap.end() )
    {
        return new EffectHLSL( effectName );
    }
    // complicated effect?
    else if( strcmp( effectName, FxGrass::effectName ) == 0 )
    {
        return new FxGrass;
    }
    else if( strcmp( effectName, FxWater::effectName ) == 0 )
    {
        return new FxWater;
    }
    else if( strcmp( effectName, FxBlender::effectName ) == 0 )
    {
        return new FxBlender;
    }
    else if( strcmp( effectName, FxWaterfall::effectName ) == 0 )
    {
        return new FxWaterfall;
    }
    return NULL;
}

void Effect::update(float dt)
{
    for( EffectI effectI = _animatedEffects.begin();
                 effectI != _animatedEffects.end();
                 effectI++ )
    {
        (*effectI)->onUpdate( dt );
    }
}

/** 
 * system routine
 */

void Effect::init(void)
{
    _hlslConfig = getCore()->getParamPackFactory()->load( "./res/effects/hlsl.config" );
    assert( _hlslConfig );

    unsigned int numItems = _hlslConfig->findParams( "effect.name*" );
    paramid_t* items = new paramid_t[numItems];
    _hlslConfig->copyParamResult( items );

    for( unsigned int i=0; i<numItems; i++ )
    {
        std::string effectName = _hlslConfig->gets( items[i] );
        std::string effectFile = _hlslConfig->gets( strformat( "effect.%s", effectName.c_str() ).c_str() );
        ID3DXEffect* effect = dxCompileEffect( effectFile.c_str(), NULL ); assert( effect );
        _hlslMap.insert( HLSL( effectName, effect ) );
    }

    delete[] items;

    _hlslLostable = new StaticLostable( onLostDevice, onResetDevice );
}

void Effect::term(void)
{
    FxGrass::releaseResources();
    FxWater::releaseResources();
    FxBlender::releaseResources();
    FxWaterfall::releaseResources();
    for( HLSLI hlslI = _hlslMap.begin(); hlslI != _hlslMap.end(); hlslI++ )
    {
        hlslI->second->Release();
        hlslI->second = NULL;
    }
    if( _hlslConfig ) 
    {
        _hlslConfig->release();
        _hlslConfig = NULL;
    }
    delete _hlslLostable;
}

void Effect::onLostDevice(void)
{
    for( HLSLI hlslI = _hlslMap.begin(); hlslI != _hlslMap.end(); hlslI++ )
    {
        hlslI->second->OnLostDevice();
    }
}

void Effect::onResetDevice(void)
{
    for( HLSLI hlslI = _hlslMap.begin(); hlslI != _hlslMap.end(); hlslI++ )
    {
        hlslI->second->OnResetDevice();
    }
}