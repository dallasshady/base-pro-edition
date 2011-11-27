
#include "headers.h"
#include "asset.h"
#include "gui.h"
#include "lightmap.h"
#include "../common/unicode.h"

/**
 * binary asset chunk header
 */

ChunkHeader::ChunkHeader(IResource* resource)
{
    fread( this, sizeof(ChunkHeader), 1, resource->getFile() );
} 

ChunkHeader::ChunkHeader(int t, int s)
{
    type = t, size = s;
}

void ChunkHeader::write(IResource* resource)
{
    fwrite( this, sizeof(ChunkHeader), 1, resource->getFile() );
}

/**
 * class implementation
 */

BinaryAsset::BinaryAsset(const char* resourceName)
{
    _resourcePath = resourceName;
}

BinaryAsset::BinaryAsset(IResource* resource)
{
    // prepare progress callback
    fseek( resource->getFile(), 0, SEEK_END );    
    unsigned int fileSize = ftell( resource->getFile() );
    fseek( resource->getFile(), 0, SEEK_SET );
    unsigned int filePos;

    _resourcePath = resource->getName();

    ChunkHeader assetHeader( resource );
    if( assetHeader.type != BA_ASSET ) throw Exception( "Incompatible binary asset format" );
    if( assetHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );
        
    // read asset chunk
    Chunk chunk;
    fread( &chunk, sizeof(Chunk), 1, resource->getFile() );

    // read textures
    for( int i=0; i<chunk.numTextures; i++ )
    {
        _assetObjects.insert( Texture::read( resource, _assetObjects ) );
        if( Engine::instance->progressCallback )
        {            
            filePos = ftell( resource->getFile() );
            Engine::instance->progressCallback(
                wstrformat( Gui::iLanguage->getUnicodeString(4), asciizToUnicode(_resourcePath.c_str()).c_str() ).c_str(),
                float( filePos ) / float( fileSize ),
                Engine::instance->progressCallbackUserData
            );
        }
    }

    // read bsps
    for( i=0; i<chunk.numBSPs; i++ )
    {
        AssetObjectT assetObjectT = BSP::read( resource, _assetObjects );
        _bsps.push_back( reinterpret_cast<BSP*>( assetObjectT.second ) );
        _assetObjects.insert( assetObjectT );
        if( Engine::instance->progressCallback )
        {            
            filePos = ftell( resource->getFile() );
            Engine::instance->progressCallback(
                wstrformat( Gui::iLanguage->getUnicodeString(4), asciizToUnicode(_resourcePath.c_str()).c_str() ).c_str(),
                float( filePos ) / float( fileSize ),
                Engine::instance->progressCallbackUserData
            );
        }
    }  

    // read clumps
    for( i=0; i<chunk.numClumps; i++ )
    {
        AssetObjectT assetObjectT = Clump::read( resource, _assetObjects );
        _clumps.push_back( reinterpret_cast<Clump*>( assetObjectT.second ) );
        _assetObjects.insert( assetObjectT );
        if( Engine::instance->progressCallback )
        {            
            filePos = ftell( resource->getFile() );
            Engine::instance->progressCallback(
                wstrformat( Gui::iLanguage->getUnicodeString(4), asciizToUnicode(_resourcePath.c_str()).c_str() ).c_str(),
                float( filePos ) / float( fileSize ),
                Engine::instance->progressCallbackUserData
            );
        }
    }

    // read extensions
    size_t      extensionResult = 0;
    ChunkHeader extensionHeader( 0, 0 );
    do
    {        
        extensionResult = fread( &extensionHeader, sizeof(ChunkHeader), 1, resource->getFile() );
        if( extensionResult == 1 )
        {
            // make sure it is an extension chunk
            assert( extensionHeader.type == BA_EXTENSION );

            // read extension chunk
            ChunkExtension extension;
            fread( &extension, sizeof(ChunkExtension), 1, resource->getFile() );

            // process extension
            switch( extension.exType )
            {
            case BAEXT_LIGHTMAPS:
                readLightmaps( extension.exNumChunks, resource );
                break;
            default:
                assert( !"Unknown asset extension!" );
            }
        }
    }
    while( extensionResult == 1 );
}

BinaryAsset::~BinaryAsset(void)
{
}

/**
 * private behaviour : extensions
 */

void BinaryAsset::readLightmaps(unsigned int numLightmaps, IResource* resource)
{
    LightmapChunk  lightmap;

    for( unsigned int i=0; i<numLightmaps; i++ )
    {
        // read chunk header
        ChunkHeader lightmapHeader( resource );        
        assert( lightmapHeader.type == BA_BINARY );
        assert( lightmapHeader.size = sizeof( LightmapChunk ) );

        // read binary data
        fread( &lightmap, sizeof(LightmapChunk), 1, resource->getFile() );

        // link lightmap to asset object
        AssetObjectI textureObject = _assetObjects.find( lightmap.textureId );
        assert( textureObject != _assetObjects.end() );
        assert( textureObject->second != NULL );
        Texture* texture = reinterpret_cast<Texture*>( textureObject->second );
        if( lightmap.atomicId )
        {
            AssetObjectI atomicObject = _assetObjects.find( lightmap.atomicId );
            assert( atomicObject != _assetObjects.end() );
            assert( atomicObject->second != NULL );
            reinterpret_cast<Atomic*>( atomicObject->second )->setLightMap( texture );
        }
        else if( lightmap.sectorId )
        {
            AssetObjectI sectorObject = _assetObjects.find( lightmap.sectorId );
            assert( sectorObject != _assetObjects.end() );
            assert( sectorObject->second != NULL );
            reinterpret_cast<BSPSector*>( sectorObject->second )->setLightMap( texture );
        }
    }
}

/**
 * private behaviour
 */

static engine::IBSPSector* collectSectorTexturesCB(engine::IBSPSector* sector, void* data)
{
    std::list<Texture*>* textures = reinterpret_cast<std::list<Texture*>*>( data );
    std::list<Texture*>::iterator textureI;

    BSPSector* bspSector = dynamic_cast<BSPSector*>( sector ); assert( bspSector );
    Geometry* geometry = dynamic_cast<Geometry*>( sector->getGeometry() );        
    
    if( !geometry ) return sector;

    if( bspSector->lightmap() )
    {
        textures->push_back( bspSector->lightmap() );
    }

    for( int i=0; i<geometry->getNumShaders(); i++ )
    {
        Shader* shader = geometry->shader( i );
        for( int j=0; j<shader->getNumLayers(); j++ )
        {
            Texture* texture = dynamic_cast<Texture*>( shader->getLayerTexture( j ) );
            bool alreadyInList = false;
            for( textureI = textures->begin(); textureI != textures->end(); textureI++ )
            {
                if( *textureI == texture )
                {
                    alreadyInList = true;
                    break;
                }
            }
            if( !alreadyInList ) textures->push_back( texture );
        }
        Texture* texture = dynamic_cast<Texture*>( shader->getNormalMap() );
        if( texture )
        {
            bool alreadyInList = false;
            for( textureI = textures->begin(); textureI != textures->end(); textureI++ )
            {
                if( *textureI == texture )
                {
                    alreadyInList = true;
                    break;
                }
            }
            if( !alreadyInList ) textures->push_back( texture );
        }
        texture = dynamic_cast<Texture*>( shader->getEnvironmentMap() );
        if( texture )
        {
            bool alreadyInList = false;
            for( textureI = textures->begin(); textureI != textures->end(); textureI++ )
            {
                if( *textureI == texture )
                {
                    alreadyInList = true;
                    break;
                }
            }
            if( !alreadyInList ) textures->push_back( texture );
        }
    }

    return sector;
}

void BinaryAsset::collectTextures(std::list<Texture*>* textures)
{
    std::list<Texture*>::iterator textureI;
    for( ClumpI clumpI = _clumps.begin(); clumpI != _clumps.end(); clumpI++ )
    {
        for( Clump::AtomicI atomicI = (*clumpI)->_atomics.begin();
                            atomicI != (*clumpI)->_atomics.end();
                            atomicI++ )
        {
            if( (*atomicI)->lightmap() )
            {
                textures->push_back( (*atomicI)->lightmap() );
            }
            Geometry* geometry = dynamic_cast<Geometry*>( (*atomicI)->getGeometry() );
            for( int i=0; i<geometry->getNumShaders(); i++ )
            {
                Shader* shader = geometry->shader( i );
                for( int j=0; j<shader->getNumLayers(); j++ )
                {
                    Texture* texture = dynamic_cast<Texture*>( shader->getLayerTexture( j ) );
                    bool alreadyInList = false;
                    for( textureI = textures->begin(); textureI != textures->end(); textureI++ )
                    {
                        if( *textureI == texture )
                        {
                            alreadyInList = true;
                            break;
                        }
                    }
                    if( !alreadyInList ) textures->push_back( texture );
                }
                Texture* texture = dynamic_cast<Texture*>( shader->getNormalMap() );
                if( texture )
                {
                    bool alreadyInList = false;
                    for( textureI = textures->begin(); textureI != textures->end(); textureI++ )
                    {
                        if( *textureI == texture )
                        {
                            alreadyInList = true;
                            break;
                        }
                    }
                    if( !alreadyInList ) textures->push_back( texture );
                }
                texture = dynamic_cast<Texture*>( shader->getEnvironmentMap() );
                if( texture )
                {
                    bool alreadyInList = false;
                    for( textureI = textures->begin(); textureI != textures->end(); textureI++ )
                    {
                        if( *textureI == texture )
                        {
                            alreadyInList = true;
                            break;
                        }
                    }
                    if( !alreadyInList ) textures->push_back( texture );
                }
            }
        }
    }

    for( BSPI bspI = _bsps.begin(); bspI != _bsps.end(); bspI++ )
    {
        (*bspI)->forAllSectors( collectSectorTexturesCB, textures );
    }
}

/**
 * IAsset
 */
   
void BinaryAsset::release(void)
{
    delete this;
}

void BinaryAsset::serialize(void)
{
    IResource* resource = getCore()->getResource( _resourcePath.c_str(), "wb" );
    if( !resource )
    {
        throw Exception( "Can't write resource \"%s\"", _resourcePath.c_str() );
    }

    ChunkHeader assetHeader( BA_ASSET, sizeof(Chunk) );
    assetHeader.write( resource );

    Chunk chunk;
    GetSystemTime( &chunk.creationTime );
    chunk.numClumps = _clumps.size();
    chunk.numBSPs   = _bsps.size();

    // collect asset textures
    std::list<Texture*> textures;
    collectTextures( &textures );
    chunk.numTextures = textures.size();

    // write binary asset chunk
    fwrite( &chunk, sizeof(Chunk), 1, resource->getFile() );

    // write textures
    for( std::list<Texture*>::iterator textureI = textures.begin();
                                       textureI != textures.end();
                                       textureI++ )
    {
        (*textureI)->write( resource );
    }

    // write bsps
    for( BSPI bspI = _bsps.begin(); bspI != _bsps.end(); bspI++ )
    {
        (*bspI)->write( resource );
    }              

    // write clumps
    for( ClumpI clumpI = _clumps.begin(); clumpI != _clumps.end(); clumpI++ ) 
    {
        (*clumpI)->write( resource );
    }

    // collect lightmaps
    LightmapChunks lightmaps;
    for( bspI = _bsps.begin(); bspI != _bsps.end(); bspI++ )
    {
        collectLightmaps( (*bspI), &lightmaps );
    }
    for( clumpI = _clumps.begin(); clumpI != _clumps.end(); clumpI++ ) 
    {
        collectLightmaps( (*clumpI), &lightmaps );
    }

    // write lightmaps
    if( lightmaps.size() )
    {
        ChunkExtension extension;
        extension.exType      = BAEXT_LIGHTMAPS;
        extension.exNumChunks = lightmaps.size();
        extension.exReservedA = 0;
        extension.exReservedB = 0;
        extension.exReservedC = 0;

        ChunkHeader extensionHeader( BA_EXTENSION, sizeof( ChunkExtension ) );
        extensionHeader.write( resource );

        // write extension chunk
        fwrite( &extension, sizeof(ChunkExtension), 1, resource->getFile() );

        // write lightmaps
        writeLightmaps( &lightmaps, resource );
    }

    resource->release();
}