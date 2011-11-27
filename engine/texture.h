/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description texture management routine
 *
 * @author bad3p
 */

#ifndef TEXTURE_IMPLEMENTATION_INCLUDED
#define TEXTURE_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"

/**
 * ITexture implementation
 */

class Texture;

typedef std::pair<std::string,Texture*> TextureT;
typedef std::map<std::string,Texture*> TextureM;
typedef TextureM::iterator TextureI;

class Texture : public Chain,
                virtual public engine::ITexture,
                virtual public Lostable
{
private:
    enum TextureType
    {
        ttManaged,
        ttDynamic,
        ttRenderTarget,
        ttCubeRenderTarget
    };
private:
    struct Chunk
    {
        auid                 id;
        char                 name[engine::maxNameLength];
        D3DTEXTUREADDRESS    addresTypeU;
        D3DTEXTUREADDRESS    addresTypeV;
        Color                borderColor;
        D3DTEXTUREFILTERTYPE magFilter;
        D3DTEXTUREFILTERTYPE minFilter;
        D3DTEXTUREFILTERTYPE mipFilter;
        DWORD                maxAnisotropy;
        float                lodBias;
    };
private:
    friend class Shader;
    friend class Camera;
    friend class CameraEffect;
private:
    int                    _numReferences;
    std::string            _name;    
    IDirect3DTexture9*     _iDirect3DTexture9;
    IDirect3DCubeTexture9* _iDirect3DCubeTexture9;
    D3DTEXTUREADDRESS      _addressTypeU;
    D3DTEXTUREADDRESS      _addressTypeV;
    Color                  _borderColor;
    D3DTEXTUREFILTERTYPE   _magFilter;
    D3DTEXTUREFILTERTYPE   _minFilter;
    D3DTEXTUREFILTERTYPE   _mipFilter;
    DWORD                  _maxAnisotropy;
    DWORD                  _maxMipLevel;    
    float                  _lodBias;
    TextureType            _textureType; // contains type of texture (lostable support)
    int                    _lostableWidth;
    int                    _lostableHeight;
    int                    _lostableDepth;
private:
    Texture();
public:
    // class implementation
    static Texture* createDynamicTexture(int width, int height, int depth, const char* name);
    static Texture* createRenderTarget(int width, int height, int depth, const char* name);
    static Texture* createCubeRenderTarget(int size, int depth, const char* name);
    static Texture* createTexture(const char* fileName);
    virtual ~Texture();
    // Lostable
    virtual void onLostDevice(void);
    virtual void onResetDevice(void);
    // ITexture
    virtual void __stdcall addReference(void);
    virtual int __stdcall getNumReferences(void);
    virtual void __stdcall release(void);
    virtual void __stdcall save(const char* resourceName);
    virtual const char* __stdcall getName(void);
    virtual int __stdcall getWidth(void);
    virtual int __stdcall getHeight(void);
    virtual engine::AddressType __stdcall getAddressTypeU(void);
    virtual void __stdcall setAddressTypeU(engine::AddressType addressType);
    virtual engine::AddressType __stdcall getAddressTypeV(void);
    virtual void __stdcall setAddressTypeV(engine::AddressType addressType);
    virtual Vector4f __stdcall getBorderColor(void);
    virtual void __stdcall setBorderColor(const Vector4f& borderColor);
    virtual engine::FilterType __stdcall getMagFilter(void);
    virtual void __stdcall setMagFilter(engine::FilterType filterType);
    virtual engine::FilterType __stdcall getMinFilter(void);
    virtual void __stdcall setMinFilter(engine::FilterType filterType);
    virtual engine::FilterType __stdcall getMipFilter(void);
    virtual void __stdcall setMipFilter(engine::FilterType filterType);
    virtual int __stdcall getMaxAnisotropy(void);
    virtual void __stdcall setMaxAnisotropy(int maxAnisotropy);
    virtual int __stdcall getMaxMipLevel(void);
    virtual void __stdcall setMaxMipLevel(int maxMipLevel);
    virtual float __stdcall getMipmapLODBias(void);
    virtual void __stdcall setMipmapLODBias(float lodBias);
public:
    // module local inlines
    inline IDirect3DTexture9* iDirect3DTexture(void) 
    { 
        return _iDirect3DTexture9; 
    }
    inline IDirect3DCubeTexture9* iDirect3DCubeTexture(void) 
    { 
        return _iDirect3DCubeTexture9; 
    }
public:
    // module locals
    D3DFORMAT getFormat(void);
    void write(IResource* resource);
    static AssetObjectT read(IResource* resource, AssetObjectM& assetObjects);
public:
    // module locals : texture sampler
    inline void apply(int stageId)
    {
        if( _iDirect3DTexture9 )
        {
            _dxCR( dxSetTexture( stageId, _iDirect3DTexture9 ) );
        }
        if( _iDirect3DCubeTexture9 )
        {
            _dxCR( dxSetTexture( stageId, _iDirect3DCubeTexture9 ) );
        }
        _dxCR( dxSetSamplerState( stageId, D3DSAMP_ADDRESSU, _addressTypeU ) );
        _dxCR( dxSetSamplerState( stageId, D3DSAMP_ADDRESSV, _addressTypeV ) );
        _dxCR( dxSetSamplerState( stageId, D3DSAMP_BORDERCOLOR, _borderColor ) );
        _dxCR( dxSetSamplerState( stageId, D3DSAMP_MAGFILTER, _magFilter ) );
        _dxCR( dxSetSamplerState( stageId, D3DSAMP_MINFILTER, _minFilter ) );        
        _dxCR( dxSetSamplerState( stageId, D3DSAMP_MIPFILTER, _mipFilter ) );
        _dxCR( dxSetSamplerState( stageId, D3DSAMP_MAXANISOTROPY, _maxAnisotropy ) );
        _dxCR( dxSetSamplerState( stageId, D3DSAMP_MAXMIPLEVEL, _maxMipLevel ) );
        _dxCR( dxSetSamplerState( stageId, D3DSAMP_MIPMAPLODBIAS, *((DWORD*)(&_lodBias)) ) );
    }
public:
    // path-to-name agreement
    static std::string getTextureNameFromFilePath(const char* path);
public:
    // texture dictionary
    static TextureM textures;
};

#endif
