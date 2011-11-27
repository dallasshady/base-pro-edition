
#include "headers.h"
#include "texture.h"
#include "../common/istring.h"
#include "asset.h"

std::string Texture::getTextureNameFromFilePath(const char* path)
{
    std::string compatiblePath = strbackslash( path );
    return exname( compatiblePath.c_str() );
}

/**
 * wrapping routine
 */

engine::FilterType wrap(D3DTEXTUREFILTERTYPE filterType)
{
    switch( filterType )
    {
    case D3DTEXF_NONE: return engine::ftNone;    
    case D3DTEXF_POINT: return engine::ftPoint;
    case D3DTEXF_LINEAR: return engine::ftLinear;
    case D3DTEXF_ANISOTROPIC: return engine::ftAnisotropic;
    case D3DTEXF_PYRAMIDALQUAD: return engine::ftPyramidal;
    case D3DTEXF_GAUSSIANQUAD: return engine::ftGaussian;
    default: return engine::ftPoint;
    }
}

/**
 * engine enumeration routine
 */

int Engine::getNumTextures(void)
{
    return Texture::textures.size();
}

engine::ITexture* Engine::getTexture(int id)
{
    assert( id >= 0  && id < int( Texture::textures.size() ) );
    int i=0;
    for( TextureI textureI = Texture::textures.begin();
                  textureI != Texture::textures.end();
                  textureI++, i++ )
    {
        if( i == id ) return textureI->second;
    }
    return NULL;
}

engine::ITexture* Engine::getTexture(const char* textureName)
{
    TextureI textureI = Texture::textures.find( textureName );
    if( textureI == Texture::textures.end() ) return NULL;
    return textureI->second;
}

/**
 * engine creation routine
 */

engine::ITexture* Engine::createRenderTarget(int width, int height, int depth, const char* textureName)
{
    return Texture::createRenderTarget( width, height, depth, textureName );
}

engine::ITexture* Engine::createCubeRenderTarget(int size, int depth, const char* textureName)
{
    return Texture::createCubeRenderTarget( size, depth, textureName );
}

engine::ITexture* Engine::createTexture(const char* resourcePath)
{
    return Texture::createTexture( resourcePath );
}

engine::ITexture* Engine::createDUDVFromNormalMap(engine::ITexture* normalMap, const char* dudvName)
{
    Texture* nmap = dynamic_cast<Texture*>( normalMap ); assert( nmap );

    unsigned int width  = nmap->getWidth();
    unsigned int height = nmap->getHeight();
    
    Texture* dudv = Texture::createDynamicTexture( width, height, 32, dudvName );

    D3DLOCKED_RECT nmapRect;
    D3DLOCKED_RECT dudvRect;
    nmap->iDirect3DTexture()->LockRect( 0, &nmapRect, NULL, D3DLOCK_READONLY );
    dudv->iDirect3DTexture()->LockRect( 0, &dudvRect, NULL, D3DLOCK_DISCARD );
    
    unsigned char* srcData  = (unsigned char*)( nmapRect.pBits );
    unsigned int   srcPitch = nmapRect.Pitch;
    unsigned int   srcDepth = 32;
    unsigned char* dstData  = (unsigned char*)( dudvRect.pBits );
    unsigned int   dstPitch = dudvRect.Pitch;
    unsigned int   dstDepth = 32;
    unsigned char* srcRow;
    unsigned char* srcRowAbove;
    unsigned char* srcRowBelow;    
    unsigned char* srcLeft;
    unsigned char* srcRight;
    unsigned char* srcAbove;
    unsigned char* srcBelow;

    unsigned char* src;
    unsigned char* dst = dstData;

    unsigned int x,y;
    for( y=0; y<height; ++y ) 
    {
        srcRowAbove = srcData + srcPitch* ( (y > 0 ? y : height) - 1 );
        srcRow = srcData + srcPitch * y;
        srcRowBelow = srcData + srcPitch * (y < height - 1 ? y + 1 : 0);
        for( x = 0; x < width; ++x ) 
        {
            srcLeft = srcRow + srcDepth * ((x > 0 ? x : width) - 1);
            src = srcRow + srcDepth*x;
            srcRight = srcRow + srcDepth*(x < width - 1 ? x + 1 : 0);
            srcAbove = srcRowAbove + srcDepth*x;
            srcBelow = srcRowBelow + srcDepth*x;

            // luminance bump value
            *dst++ = (unsigned char) (*src > 1) ? 63 : 127;

            // delta-v bump value
            *dst++ = (unsigned char) (((int) *srcAbove) - ((int) *srcBelow));
            //*dst++ = (RwUInt8) (x > 127 && y > 127 ? 63 : 0);

            // delta-u bump value
            *dst++ = (unsigned char) (((int) *srcLeft) - ((int) *srcRight));

            //*dst++ = (RwUInt8) (x > 127 && y > 127 ? 63 : 0);
            *dst++ = (unsigned char) 0;

            //*((RwInt32 *) dst) = *((RwInt32 *) src);
            //dst += 4;
        }
    }

    nmap->iDirect3DTexture()->UnlockRect( 0 );
    dudv->iDirect3DTexture()->UnlockRect( 0 );

    return dudv;
}

/**
 * class implementation
 */

TextureM Texture::textures;

Texture::Texture()
{
    _numReferences         = 1;
    _name                  = "";
    _iDirect3DTexture9     = NULL;
    _iDirect3DCubeTexture9 = NULL;
    _addressTypeU          = D3DTADDRESS_WRAP;
    _addressTypeV          = D3DTADDRESS_WRAP;
    _borderColor           = black;
    _magFilter             = D3DTEXF_POINT;
    _minFilter             = D3DTEXF_POINT;
    _mipFilter             = D3DTEXF_NONE;
    _maxAnisotropy         = 1;
    _maxMipLevel           = 0;
    _lodBias               = 0.0f;
    _textureType           = ttManaged;
    _lostableWidth         = 0;
    _lostableHeight        = 0;
    _lostableDepth         = 0;
}    

Texture::~Texture()
{
    // remove from dictionary
    int i = textures.size();
    TextureI textureI = textures.find( _name.c_str() );
    assert( textureI != textures.end() );
    textures.erase( textureI );

    // release DirectX interface
    if( _iDirect3DTexture9 != NULL )
    {
        int numReferences = _iDirect3DTexture9->Release();
        assert( numReferences == 0 );
    }
    if( _iDirect3DCubeTexture9 != NULL )
    {
        int numReferences = _iDirect3DCubeTexture9->Release();
        assert( numReferences == 0 );
    }
}

Texture* Texture::createDynamicTexture(int width, int height, int depth, const char* name)
{
    _chain( Texture* result = new Texture );

    result->_textureType    = ttDynamic;
    result->_lostableWidth  = width;
    result->_lostableHeight = height;
    result->_lostableDepth  = depth;

    D3DFORMAT textureFormat = ( depth == 32 ? D3DFMT_A8R8G8B8 : D3DFMT_A4R4G4B4 );
    _dxCR( iDirect3DDevice->CreateTexture(
        width, height, 1,
        /*D3DUSAGE_AUTOGENMIPMAP | */D3DUSAGE_DYNAMIC, 
        textureFormat, 
        D3DPOOL_DEFAULT,
        &result->_iDirect3DTexture9, NULL
    ) );

    assert( result->_iDirect3DTexture9 );

    result->_name = name;
    textures.insert( TextureT( name, result ) );

    return result;
}

Texture* Texture::createRenderTarget(int width, int height, int depth, const char* name)
{
    _chain( Texture* result = new Texture );

    result->_textureType    = ttRenderTarget;
    result->_lostableWidth  = width;
    result->_lostableHeight = height;
    result->_lostableDepth  = depth;

    D3DFORMAT textureFormat = ( depth == 32 ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5 );
    _dxCR( D3DXCreateTexture(
        iDirect3DDevice,
        width, height, 1,
        D3DUSAGE_RENDERTARGET, 
        textureFormat, 
        D3DPOOL_DEFAULT,
        &result->_iDirect3DTexture9
    ) );
    assert( result->_iDirect3DTexture9 );

    result->_name = name;
    textures.insert( TextureT( name, result ) );

    return result;
}

Texture* Texture::createCubeRenderTarget(int size, int depth, const char* name)
{
    _chain( Texture* result = new Texture );

    result->_textureType    = ttCubeRenderTarget;
    result->_lostableWidth  = size;
    result->_lostableHeight = size;
    result->_lostableDepth  = depth;

    D3DFORMAT textureFormat = ( depth == 32 ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5 );
    _dxCR( iDirect3DDevice->CreateCubeTexture(
        size,
        0,
        D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET,
        textureFormat,
        D3DPOOL_DEFAULT,
        &result->_iDirect3DCubeTexture9,
        NULL
    ) );

    assert( result->_iDirect3DCubeTexture9 );

    result->_name = name;
    textures.insert( TextureT( name, result ) );

    return result;
}

Texture* Texture::createTexture(const char* fileName)
{
    // read surface format
    IResource* resource = getCore()->getResource( fileName, "rb" ); assert( resource );
    DWORD magicValue;
    fread( &magicValue, sizeof(DWORD), 1, resource->getFile() );
    DDSURFACEDESC2 surfaceDesc;
    fread( &surfaceDesc, sizeof(DDSURFACEDESC2), 1, resource->getFile() );
    resource->release();

    // analyse magic value
    // ...

    // create texture
    _chain( Texture* result = new Texture );
    
    result->_textureType = ttManaged;

    // is it a cube map?
    if( surfaceDesc.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP )
    {
        _dxCR( D3DXCreateCubeTextureFromFile( 
            iDirect3DDevice,
            fileName,
            &result->_iDirect3DCubeTexture9
        ) );
    }
    else
    {
        _dxCR( D3DXCreateTextureFromFileEx(
            iDirect3DDevice,
            fileName,
            D3DX_DEFAULT,
            D3DX_DEFAULT,
            D3DX_FROM_FILE,
            0,
            D3DFMT_FROM_FILE,
            D3DPOOL_MANAGED,
            D3DX_DEFAULT,
            D3DX_DEFAULT,
            0,
            NULL,
            NULL,
            &result->_iDirect3DTexture9
        ) );


        /*_dxCR( D3DXCreateTextureFromFile( 
            iDirect3DDevice,
            fileName,
            &result->_iDirect3DTexture9
        ) );*/
    }

    result->_name = getTextureNameFromFilePath( fileName );
    textures.insert( TextureT( result->_name.c_str(), result ) );
    
    return result;
}

/**
 * ITexture
 */

void Texture::addReference(void)
{
    _numReferences++;
}

int Texture::getNumReferences(void)
{
    return _numReferences;
}

void Texture::release(void)
{
    _numReferences--;
    if( _numReferences <= 1 ) 
    {
        delete this;
    }
}

void Texture::save(const char* resourceName)
{
    if( _iDirect3DTexture9 )
    {
        _dxCR( D3DXSaveTextureToFile( resourceName, D3DXIFF_DDS, _iDirect3DTexture9, NULL ) );
    }
    else
    {
        _dxCR( D3DXSaveTextureToFile( resourceName, D3DXIFF_DDS, _iDirect3DCubeTexture9, NULL ) );
    }
}

const char* Texture::getName(void)
{
    return _name.c_str();
}

int Texture::getWidth(void)
{
    D3DSURFACE_DESC surfaceDesc;
    if( _iDirect3DTexture9 )
    {
        IDirect3DSurface9* iDirect3DSurface;
        _dxCR( _iDirect3DTexture9->GetSurfaceLevel( 0, &iDirect3DSurface ) );
        _dxCR( iDirect3DSurface->GetDesc( &surfaceDesc ) );
        iDirect3DSurface->Release();
    }
    else
    {
        IDirect3DSurface9* iDirect3DSurface;
        _dxCR( _iDirect3DCubeTexture9->GetCubeMapSurface( D3DCUBEMAP_FACE_POSITIVE_X, 0, &iDirect3DSurface ) );
        _dxCR( iDirect3DSurface->GetDesc( &surfaceDesc ) );
        iDirect3DSurface->Release();
    }
    return surfaceDesc.Width;
}

int Texture::getHeight(void)
{
    D3DSURFACE_DESC surfaceDesc;
    if( _iDirect3DTexture9 )
    {
        IDirect3DSurface9* iDirect3DSurface;
        _dxCR( _iDirect3DTexture9->GetSurfaceLevel( 0, &iDirect3DSurface ) );
        _dxCR( iDirect3DSurface->GetDesc( &surfaceDesc ) );
        iDirect3DSurface->Release();        
    }
    else
    {
        IDirect3DSurface9* iDirect3DSurface;
        _dxCR( _iDirect3DCubeTexture9->GetCubeMapSurface( D3DCUBEMAP_FACE_POSITIVE_X, 0, &iDirect3DSurface ) );
        _dxCR( iDirect3DSurface->GetDesc( &surfaceDesc ) );
        iDirect3DSurface->Release();
    }
    return surfaceDesc.Height;
}

static engine::AddressType wrap(D3DTEXTUREADDRESS value)
{
    switch( value )
    {
    case D3DTADDRESS_WRAP: return engine::atWrap;
    case D3DTADDRESS_MIRROR: return engine::atMirror;
    case D3DTADDRESS_CLAMP: return engine::atClamp;
    case D3DTADDRESS_BORDER: return engine::atBorder;
    default: return engine::atWrap;
    }
}

static D3DTEXTUREADDRESS wrap(engine::AddressType value)
{
    switch( value )
    {
    case engine::atWrap: return D3DTADDRESS_WRAP;
    case engine::atMirror: return D3DTADDRESS_MIRROR;
    case engine::atClamp: return D3DTADDRESS_CLAMP;
    case engine::atBorder: return D3DTADDRESS_BORDER;
    default: return D3DTADDRESS_WRAP;
    }
}

engine::AddressType Texture::getAddressTypeU(void)
{
    return wrap( _addressTypeU );
}

void Texture::setAddressTypeU(engine::AddressType addressType)
{
    _addressTypeU = wrap( addressType );
}

engine::AddressType Texture::getAddressTypeV(void)
{
    return wrap( _addressTypeV );
}

void Texture::setAddressTypeV(engine::AddressType addressType)
{
    _addressTypeV = wrap( addressType );
}

Vector4f Texture::getBorderColor(void)
{
    return wrap( _borderColor );
}

void Texture::setBorderColor(const Vector4f& borderColor)
{
    _borderColor = wrap( borderColor );
}

engine::FilterType Texture::getMagFilter(void)
{
    return wrap( _magFilter );
}

void Texture::setMagFilter(engine::FilterType filterType)
{
    switch( filterType )
    {
    case engine::ftNone:
        _magFilter = D3DTEXF_POINT;
        break;
    case engine::ftPoint:
        _magFilter = D3DTEXF_POINT;
        break;
    case engine::ftLinear:
        if( !dxFilterSupported( D3DPTFILTERCAPS_MAGFLINEAR ) ) setMagFilter( engine::ftPoint ); else _magFilter = D3DTEXF_LINEAR;
        break;
    case engine::ftAnisotropic:        
        if( !dxFilterSupported( D3DPTFILTERCAPS_MAGFANISOTROPIC ) ) setMagFilter( engine::ftLinear ); else _magFilter = D3DTEXF_ANISOTROPIC;
        break;
    case engine::ftPyramidal:
        if( !dxFilterSupported( D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD ) ) setMagFilter( engine::ftLinear ); else _magFilter = D3DTEXF_PYRAMIDALQUAD;
        break;
    case engine::ftGaussian:
        if( !dxFilterSupported( D3DPTFILTERCAPS_MAGFGAUSSIANQUAD ) ) setMagFilter( engine::ftLinear ); else _magFilter = D3DTEXF_GAUSSIANQUAD;
        break;
    default: 
        _magFilter = D3DTEXF_POINT;
        break;
    }
}

engine::FilterType Texture::getMinFilter(void)
{
    return wrap( _minFilter );
}

void Texture::setMinFilter(engine::FilterType filterType)
{
    switch( filterType )
    {
    case engine::ftNone:
        _minFilter = D3DTEXF_POINT;
        break;
    case engine::ftPoint:
        _minFilter = D3DTEXF_POINT;
        break;
    case engine::ftLinear:
        if( !dxFilterSupported( D3DPTFILTERCAPS_MINFLINEAR ) ) setMinFilter( engine::ftPoint ); else _minFilter = D3DTEXF_LINEAR;
        break;
    case engine::ftAnisotropic:        
        if( !dxFilterSupported( D3DPTFILTERCAPS_MINFANISOTROPIC ) ) setMinFilter( engine::ftLinear ); else _minFilter = D3DTEXF_ANISOTROPIC;
        break;
    case engine::ftPyramidal:
        if( !dxFilterSupported( D3DPTFILTERCAPS_MINFPYRAMIDALQUAD ) ) setMinFilter( engine::ftLinear ); else _minFilter = D3DTEXF_PYRAMIDALQUAD;
        break;
    case engine::ftGaussian:
        if( !dxFilterSupported( D3DPTFILTERCAPS_MINFGAUSSIANQUAD ) ) setMinFilter( engine::ftLinear ); else _minFilter = D3DTEXF_GAUSSIANQUAD;
        break;
    default: 
        _minFilter = D3DTEXF_POINT;
        break;
    }
}

engine::FilterType Texture::getMipFilter(void)
{
    return wrap( _mipFilter );
}

void Texture::setMipFilter(engine::FilterType filterType)
{
    switch( filterType )
    {
    case engine::ftNone:
        _mipFilter = D3DTEXF_NONE;
        break;
    case engine::ftPoint:
        _mipFilter = D3DTEXF_POINT;
        break;
    case engine::ftLinear:
        if( !dxFilterSupported( D3DPTFILTERCAPS_MIPFLINEAR ) ) setMipFilter( engine::ftPoint ); else _mipFilter = D3DTEXF_LINEAR;
        break;
    case engine::ftAnisotropic:        
        setMipFilter( engine::ftLinear );
        break;
    case engine::ftPyramidal:
        setMipFilter( engine::ftLinear );
        break;
    case engine::ftGaussian:
        setMipFilter( engine::ftLinear );
        break;
    default: 
        _mipFilter = D3DTEXF_POINT;
        break;
    }    
}

int Texture::getMaxAnisotropy(void)
{
    return _maxAnisotropy;
}

void Texture::setMaxAnisotropy(int maxAnisotropy)
{
    _maxAnisotropy = maxAnisotropy;
}

int Texture::getMaxMipLevel(void)
{
    return _maxMipLevel;
}

void Texture::setMaxMipLevel(int maxMipLevel)
{
    _maxMipLevel = maxMipLevel;
}

float Texture::getMipmapLODBias(void)
{
    return _lodBias;
}

void Texture::setMipmapLODBias(float lodBias)
{
    _lodBias = lodBias;
}

/**
 * module locals
 */

D3DFORMAT Texture::getFormat(void)
{
    D3DSURFACE_DESC surfaceDesc;
    if( _iDirect3DTexture9 )
    {
        IDirect3DSurface9* iDirect3DSurface;
        _dxCR( _iDirect3DTexture9->GetSurfaceLevel( 0, &iDirect3DSurface ) );
        _dxCR( iDirect3DSurface->GetDesc( &surfaceDesc ) );
        iDirect3DSurface->Release();        
    }
    else
    {
        IDirect3DSurface9* iDirect3DSurface;
        _dxCR( _iDirect3DCubeTexture9->GetCubeMapSurface( D3DCUBEMAP_FACE_POSITIVE_X, 0, &iDirect3DSurface ) );
        _dxCR( iDirect3DSurface->GetDesc( &surfaceDesc ) );
        iDirect3DSurface->Release();
    }
    return surfaceDesc.Format;
}

void Texture::write(IResource* resource)
{
    ChunkHeader textureHeader( BA_TEXTURE, sizeof( Chunk ) );
    textureHeader.write( resource );

    Chunk chunk;    
    memset( chunk.name, 0, engine::maxNameLength );
    if( _name.length() > engine::maxNameLength )
    {
        strncpy( chunk.name, _name.c_str(), engine::maxNameLength - 1 );
    }
    else
    {
        strcpy( chunk.name, _name.c_str() );
    }
    chunk.id = (auid)(this);
    chunk.addresTypeU   = _addressTypeU;
    chunk.addresTypeV   = _addressTypeV;
    chunk.borderColor   = _borderColor;
    chunk.magFilter     = _magFilter;
    chunk.minFilter     = _minFilter;
    chunk.mipFilter     = _mipFilter;
    chunk.maxAnisotropy = _maxAnisotropy;
    chunk.lodBias       = _lodBias;
    fwrite( &chunk, sizeof( Chunk ), 1, resource->getFile() );

    std::string path = expath( resource->getName() );
    path += "textures/";
    path += chunk.name;
    path += ".dds";

    save( path.c_str() );
}

AssetObjectT Texture::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader textureHeader( resource );
    if( textureHeader.type != BA_TEXTURE ) throw Exception( "Unexpected chunk header" );
    if( textureHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, sizeof(Chunk), 1, resource->getFile() );
    
    TextureI textureI = Texture::textures.find( chunk.name );
    if( textureI != Texture::textures.end() )
    {
        return AssetObjectT( chunk.id, textureI->second );
    }

    std::string path = expath( resource->getName() );
    path += "textures/";
    path += chunk.name;
    path += ".dds";

    Texture* texture = Texture::createTexture( path.c_str() );

    texture->_addressTypeU  = chunk.addresTypeU;
    texture->_addressTypeV  = chunk.addresTypeV;
    texture->_borderColor   = chunk.borderColor;
    texture->_magFilter     = chunk.magFilter;
    texture->_maxAnisotropy = chunk.maxAnisotropy;
    texture->_minFilter     = chunk.minFilter;
    texture->_mipFilter     = chunk.mipFilter;
    texture->_lodBias       = chunk.lodBias;
    return AssetObjectT( chunk.id, texture );
}

/**
 * some texture are lostable
 */

void Texture::onLostDevice(void)
{
    if( _textureType != ttManaged )
    {
        // release DirectX interface
        if( _iDirect3DTexture9 != NULL )
        {
            int numReferences = _iDirect3DTexture9->Release();
            assert( numReferences == 0 );
            _iDirect3DTexture9 = NULL;
        }
        if( _iDirect3DCubeTexture9 != NULL )
        {
            int numReferences = _iDirect3DCubeTexture9->Release();
            assert( numReferences == 0 );
            _iDirect3DCubeTexture9 = NULL;
        }
    }
}

void Texture::onResetDevice(void)
{    
    switch( _textureType )
    {
    case ttDynamic:
        {
            D3DFORMAT textureFormat = ( _lostableDepth == 32 ? D3DFMT_A8R8G8B8 : D3DFMT_A4R4G4B4 );
            _dxCR( iDirect3DDevice->CreateTexture(
                _lostableWidth, _lostableHeight, 0,
                D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_DYNAMIC, 
                textureFormat, 
                D3DPOOL_DEFAULT,
                &_iDirect3DTexture9, NULL
            ) );
            assert( _iDirect3DTexture9 );
        }
        break;
    case ttRenderTarget:
        {
            D3DFORMAT textureFormat = ( _lostableDepth == 32 ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5 );
            _dxCR( D3DXCreateTexture(
                iDirect3DDevice,
                _lostableWidth, _lostableHeight, 1,
                D3DUSAGE_RENDERTARGET, 
                textureFormat, 
                D3DPOOL_DEFAULT,
                &_iDirect3DTexture9
            ) );
            assert( _iDirect3DTexture9 );
        }
        break;
    case ttCubeRenderTarget:
        {
            D3DFORMAT textureFormat = ( _lostableDepth == 32 ? D3DFMT_X8R8G8B8 : D3DFMT_R5G6B5 );
            _dxCR( iDirect3DDevice->CreateCubeTexture(
                _lostableWidth,
                0,
                D3DUSAGE_AUTOGENMIPMAP | D3DUSAGE_RENDERTARGET,
                textureFormat,
                D3DPOOL_DEFAULT,
                &_iDirect3DCubeTexture9,
                NULL
            ) );
            assert( _iDirect3DCubeTexture9 );
        }
        break;
    }
}