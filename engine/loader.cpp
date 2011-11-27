
#include "headers.h"
#include "loader.h"
#include "engine.h"
#include "asset.h"

engine::ILoader* Engine::createLoader(engine::AssetType assetType, const char* resourcePath)
{
    assert( assetType == engine::atXFile || assetType == engine::atBinary );
    return new Loader( assetType, resourcePath );
}

/**
 * ILoader : thread procedures
 */

DWORD Loader::loadBinaryAsset(LPVOID lpParameter)
{
    Loader* loader = reinterpret_cast<Loader*>( lpParameter );
    BinaryAsset* ba = new BinaryAsset( loader->_resource );
    loader->_asset = ba;
    return 0;
}

DWORD Loader::loadXAsset(LPVOID lpParameter)
{
    Loader* loader = reinterpret_cast<Loader*>( lpParameter );
    XAsset* xa = new XAsset( loader->_resourcePath.c_str() );
    loader->_asset = xa;
    return 0;
}

/**
 * ILoader : class implementation
 */

Loader::Loader(engine::AssetType assetType, const char* resourcePath)
{
    _asset = NULL;
    _resourcePath = resourcePath;
    _resource = NULL;

    switch( assetType )
    {
    case engine::atBinary:
        _resource = getCore()->getResource( resourcePath, "rb" ); assert( _resource );
        fseek( _resource->getFile(), 0, SEEK_END );
        _resourceSize = ftell( _resource->getFile() );
        fseek( _resource->getFile(), 0, SEEK_SET );
        _threadHandle = CreateThread( NULL, 0, loadBinaryAsset, this, 0, &_threadId );
        break;
    case engine::atXFile:
        _threadHandle = CreateThread( NULL, 0, loadXAsset, this, 0, &_threadId );
        break;
    default:
        assert( !"shouldn't be here!" );
    }
}

Loader::~Loader()
{
    if( _resource ) _resource->release();
}

/**
 * ILoader : interface implementation
 */

void Loader::release(void)
{
    delete this;
}

float Loader::getProgress(void)
{
    if( _resource )
    {
        if( _asset ) return 1;
        return ( float( ftell( _resource->getFile() ) ) / _resourceSize );
    }
    else
    {
        return ( _asset == NULL ? 0.0f : 1.0f );
    }
}

engine::IAsset* Loader::getAsset(void)
{
    return _asset;
}

void Loader::suspend(void)
{
    if( !_asset ) SuspendThread( _threadHandle );
}

void Loader::resume(void)
{
    if( !_asset ) ResumeThread( _threadHandle );
}