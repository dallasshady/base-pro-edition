/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description assets
 *
 * @author bad3p
 */

#ifndef ASSET_IMPLEMENTATION_INCLUDED
#define ASSET_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "bsp.h"
#include "../common/istring.h"
#include "../shared/import.h"

/**
 * asset implementation base, provides storage types and "walk-through" methods 
 */

class Asset : public engine::IAsset
{
protected:
    typedef std::list<Clump*> ClumpL;
    typedef ClumpL::iterator ClumpI;
    typedef std::list<BSP*> BSPL;
    typedef BSPL::iterator BSPI;
protected:
    ClumpL _clumps;
    BSPL   _bsps;
public:
    // class implementation
    virtual ~Asset()
    {
        clear();
    }
public:
    // IAsset
    virtual void __stdcall clear(void)
    {
        for( ClumpI clumpI = _clumps.begin(); clumpI != _clumps.end(); clumpI++ )
        {
            (*clumpI)->release();
        }
        for( BSPI bspI = _bsps.begin(); bspI != _bsps.end(); bspI++ )
        {
            (*bspI)->release();
        }
        _clumps.clear();
        _bsps.clear();
    }
    virtual bool __stdcall isEmpty(void)
    {
        return ( _clumps.size() + _bsps.size() ) == 0;
    }
    virtual void __stdcall forAllClumps(engine::IClumpCallBack callBack, void* data)
    {
        for( ClumpI clumpI = _clumps.begin(); clumpI != _clumps.end(); clumpI++ )
        {
            if( NULL == callBack( *clumpI, data ) ) break;
        }
    }
    virtual void __stdcall forAllBSPs(engine::IBSPCallBack callBack, void* data)
    {
        for( BSPI bspI = _bsps.begin(); bspI != _bsps.end(); bspI++ )
        {
            if( NULL == callBack( *bspI, data ) ) break;
        }
    }
    virtual void __stdcall seize(engine::IAsset* victimAsset)
    {
        Asset* asset = dynamic_cast<Asset*>( victimAsset );
        assert( asset );
        for( ClumpI clumpI = asset->_clumps.begin();
                    clumpI != asset->_clumps.end();
                    clumpI++ )
        {
            _clumps.push_back( *clumpI );
        }
        for( BSPI bspI = asset->_bsps.begin(); bspI != asset->_bsps.end(); bspI++ )
        {
            _bsps.push_back( *bspI );
        }
        asset->_clumps.clear();
        asset->_bsps.clear();
    }
    virtual void __stdcall add(engine::IClump* clump)
    {
        Clump* c = dynamic_cast<Clump*>( clump ); assert( c );
        _clumps.push_back( c );
    }
    virtual void __stdcall remove(engine::IClump* clump)
    {
        Clump* c = dynamic_cast<Clump*>( clump ); assert( c );
        for( ClumpI clumpI = _clumps.begin();
                    clumpI != _clumps.end();
                    clumpI++ )
        {
            if( *clumpI == c )
            {
                _clumps.erase( clumpI );
                return;
            }
        }
    }
};

/**
 * asset used to manage binary resources
 */

// binary asset chunk types
#define BA_ASSET     0x61623364
#define BA_TEXTURE   0x7478740D 
#define BA_CLUMP     0x706C630D 
#define BA_FRAME     0x6D72660D
#define BA_GEOMETRY  0x6F65670D
#define BA_SKIN      0x696B730D
#define BA_BONE      0x6E6F620D
#define BA_SHADER    0x6468730D
#define BA_ATOMIC    0x6F74610D
#define BA_LIGHT     0x74696C0D
#define BA_BSP       0x7073620D 
#define BA_SECTOR    0x6365730D
#define BA_OCTREE    0x74636F0D
#define BA_EFFECT    0x7866650D 
#define BA_BINARY    0x6E69620D
#define BA_EXTENSION 0x7478650D

// binary asset extension types
#define BAEXT_LIGHTMAPS 0x6D746C0D

// binary asset chunk header
struct ChunkHeader
{
public:
    int type; // chunk type
    int size; // chunk size
public:
    ChunkHeader(IResource* resource);
    ChunkHeader(int t, int s);
public:
    void write(IResource* resource);
};

class BinaryAsset : public Asset
{
private:
    // binary asset chunk
    struct Chunk
    {
    public:
        int        numTextures;
        int        numBSPs;
        int        numClumps;
        SYSTEMTIME creationTime;
    };  
private:
    // binary asset extension chunk
    struct ChunkExtension
    {
    public:
        int exType;      // type of extension
        int exNumChunks; // number of following binary chunks
        int exReservedA; // reserved for future use
        int exReservedB; // reserved for future use
        int exReservedC; // reserved for future use
    };
private:
    std::string  _resourcePath;
    AssetObjectM _assetObjects;
private:
    void collectTextures(std::list<Texture*>* textures);
    void readLightmaps(unsigned int numLightmaps, IResource* resource);
public:
    // class implementation
    BinaryAsset(const char* resourceName);
    BinaryAsset(IResource* resource);
    virtual ~BinaryAsset(void);
public:
    // IAsset
    virtual void __stdcall release(void);
    virtual void __stdcall serialize(void);
};

/**
 * asset used to manage imported resources (read-only)
 */

class ImportAsset : public Asset
{
private:
    // imported objects storage types
    typedef std::pair<import::iuid,Texture*> TextureT;
    typedef std::map<import::iuid,Texture*> TextureM;
    typedef TextureM::iterator TextureI;
    typedef std::pair<import::iuid,Shader*> ShaderT;
    typedef std::map<import::iuid,Shader*> ShaderM;
    typedef ShaderM::iterator ShaderI;
    typedef std::pair<import::iuid,Frame*> FrameT;
    typedef std::map<import::iuid,Frame*> FrameM;
    typedef FrameM::iterator FrameI;
    typedef std::pair<import::iuid,Geometry*> GeometryT;
    typedef std::map<import::iuid,Geometry*> GeometryM;
    typedef GeometryM::iterator GeometryI;
    typedef std::pair<import::iuid,Atomic*> AtomicT;
    typedef std::map<import::iuid,Atomic*> AtomicM;
    typedef AtomicM::iterator AtomicI;
    typedef std::pair<import::iuid,BSPSector*> BSPSectorT;
    typedef std::map<import::iuid,BSPSector*> BSPSectorM;
    typedef BSPSectorM::iterator BSPSectorI;
    typedef std::pair<import::iuid,BSP*> BSPT;
    typedef std::map<import::iuid,BSP*> BSPM;
    typedef BSPM::iterator BSPI;
    typedef std::pair<import::iuid,Light*> LightT;
    typedef std::map<import::iuid,Light*> LightM;
    typedef LightM::iterator LightI;
private:
    // imported objects storage
    TextureM     _textures;
    ShaderM      _shaders;
    FrameM       _frames;
    GeometryM    _geometries;
    AtomicM      _atomics;
    LightM       _lights;
    BSPSectorM   _bspSectorM;
    BSPM         _bspM;    
public:
    // class implementation
    ImportAsset(const char* resourcePath);
    virtual ~ImportAsset(void);
public:
    // IAsset
    virtual void __stdcall release(void);
    virtual void __stdcall serialize(void);
};

/**
 * asset used to manage x-file resource
 * restrictions for x-file asset:
 *  - all asset data will be converted to single clump, named as file name
 *  - all exported camera frames will be ignored
 */

class XAsset : public Asset
{
private:
    void resolveNamelessFrames(Clump* clump);
    void createD3HierarchyClasses(Clump* clump, D3DXFRAME* frame);
public:
    // class implementation
    XAsset(const char* resourcePath);
    virtual ~XAsset(void);
public:
    // IAsset
    virtual void __stdcall release(void);
    virtual void __stdcall serialize(void);
};

#endif