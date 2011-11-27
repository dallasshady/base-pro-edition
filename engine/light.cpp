
#include "headers.h"
#include "light.h"
#include "bsp.h"
#include "collision.h"
#include "asset.h"

/**
 * creation routine
 */

engine::ILight* Engine::createLight(engine::LightType lightType)
{
    return new Light( lightType );
}

/**
 * class implementation
 */

Light::Light(engine::LightType lightType)
{
    switch( lightType )
    {
    case engine::ltAmbient:
        _lightProps.Type = D3DLIGHT_FORCE_DWORD;
        break;
    case engine::ltPoint:
        _lightProps.Type = D3DLIGHT_POINT;
        break;
    case engine::ltSpot:
        _lightProps.Type = D3DLIGHT_SPOT;
        break;
    case engine::ltDirectional:
        _lightProps.Type = D3DLIGHT_DIRECTIONAL;
        break;
    default:
        assert( !"shouldn't be here!" );
    }
    _lightProps.Diffuse      = _wrap( Vector4f( 1,1,1,1 ) );
    _lightProps.Specular     = _wrap( Vector4f( 0,0,0,1 ) );
    _lightProps.Ambient      = _wrap( Vector4f( 0,0,0,1 ) );
    _lightProps.Position     = Vector( 0,0,0 );
    _lightProps.Direction    = Vector( 0,0,1 );
    _lightProps.Range        = 100.0f;
    _lightProps.Falloff      = 1.0f;
    _lightProps.Attenuation0 = 0.0f;
    _lightProps.Attenuation1 = 0.0f;
    _lightProps.Attenuation2 = 0.0f;
    _lightProps.Theta        = D3DX_PI/2;
    _lightProps.Phi          = D3DX_PI;
    _lightset = 0;

    _frame = NULL;
    _bsp = NULL;
}

Light::Light(D3DLIGHT9* lightProps)
{
    _lightProps = *lightProps;
    _frame = NULL;
    _bsp = NULL;
    _lightset = 0;
}

Light::~Light()
{
    setFrame( NULL );
}

/**
 * ILight
 */

void Light::release(void)
{
    delete this;
}

engine::IFrame* Light::getFrame(void)
{
    return _frame;
}

void Light::setFrame(engine::IFrame* frame)
{
    if( _frame ) _frame->pAttachedObject = NULL;
    _frame = dynamic_cast<Frame*>( frame );
    if( _frame )
    {
        assert( _frame->pAttachedObject == NULL );
        _frame->pAttachedObject = this;
        onUpdate();
    }
}

engine::LightType Light::getType(void)
{
    switch( _lightProps.Type )
    {
    case D3DLIGHT_FORCE_DWORD:
        return engine::ltAmbient;
    case D3DLIGHT_POINT:
        return engine::ltPoint;
    case D3DLIGHT_SPOT:
        return engine::ltSpot;
    case D3DLIGHT_DIRECTIONAL:
        return engine::ltDirectional;
    default:
        assert( !"shouldn't be here!" );
    }
    return engine::ltAmbient;
}

unsigned int Light::getLightset(void)
{
    return _lightset;
}

void Light::setLightset(unsigned int value)
{
    _lightset = value;
}

Vector4f Light::getDiffuseColor(void)
{
    return _wrap( _lightProps.Diffuse );
}

void Light::setDiffuseColor(Vector4f value)
{
    _lightProps.Diffuse = _wrap( value );
}

Vector4f Light::getSpecularColor(void)
{
    return _wrap( _lightProps.Specular );
}

void Light::setSpecularColor(Vector4f value)
{
    _lightProps.Specular = _wrap( value );
}

float Light::getRange(void)
{
    return _lightProps.Range;
}

void Light::setRange(float value)
{
    _lightProps.Range = value;
}

Vector3f Light::getAttenuation(void)
{
    return Vector3f( 
        _lightProps.Attenuation0,
        _lightProps.Attenuation1,
        _lightProps.Attenuation2
    );
}

void Light::setAttenuation(const Vector3f& value)
{
    _lightProps.Attenuation0 = value[0],
    _lightProps.Attenuation1 = value[1],
    _lightProps.Attenuation2 = value[2];
}

float Light::getTheta(void)
{
    return _lightProps.Theta;
}

void Light::setTheta(float theta)
{
    _lightProps.Theta = theta;
}

float Light::getPhi(void)
{
    return _lightProps.Phi;
}

void Light::setPhi(float phi)
{
    _lightProps.Phi = phi;
}

/**
 * callbacks
 */

void Light::affectBSPSector(void* sector)
{   
    // is light sphere intersects the bounding box of world sector?
    if( getType() == engine::ltDirectional ||        
        intersectionSphereAABB( 
            &_lightSphere, 
            &reinterpret_cast<BSPSector*>( sector )->_boundingBox 
      ) )
    {
        // is this a leaf sector?
        if( !reinterpret_cast<BSPSector*>( sector )->_leftSubset ) 
        {
            reinterpret_cast<BSPSector*>( sector )->_lightsInSector.insert( this );
            _sectors.push_back( sector );
        }
        else
        {
            affectBSPSector( reinterpret_cast<BSPSector*>( sector )->_leftSubset );
            affectBSPSector( reinterpret_cast<BSPSector*>( sector )->_rightSubset );
        }
    }
}

/**
 * Updatable
 */

void Light::onUpdate(void)
{
    // position light
    if( _frame )
    {
        _lightProps.Position  = dxPos( &_frame->LTM );
        _lightProps.Direction = dxAt( &_frame->LTM );
    }

    // recalculate light sphere
    _lightSphere.center = _lightProps.Position;
    _lightSphere.radius = _lightProps.Range;
    
    // affect world sectors
    if( _bsp )
    {
        BSPSector::LightI lightI;
        for( voidI sectorI = _sectors.begin(); sectorI != _sectors.end(); sectorI++ )
        {
             lightI = reinterpret_cast<BSPSector*>( *sectorI )->_lightsInSector.find( this );
             assert( lightI != reinterpret_cast<BSPSector*>( *sectorI )->_lightsInSector.end() );
             reinterpret_cast<BSPSector*>( *sectorI )->_lightsInSector.erase( lightI );
        }
        _sectors.clear();
        affectBSPSector( reinterpret_cast<BSP*>( _bsp )->getRoot() );
    }
}

/**
 * module local
 */

void Light::apply(DWORD lightIndex)
{
    // set light
    if( _lightProps.Attenuation0 == 0 &&
        _lightProps.Attenuation1 == 0 &&
        _lightProps.Attenuation2 == 0 )
    {
        _lightProps.Attenuation1 = 0.001f;
    }

    _dxCR( dxSetLight( lightIndex, &_lightProps ) );
}

Light* Light::clone(void)
{
    Light* lightClone = new Light( getType() );
    lightClone->_lightProps  = _lightProps;
    return lightClone;
}

void Light::write(IResource* resource)
{
    ChunkHeader lightHeader( BA_LIGHT, sizeof( Chunk ) );
    lightHeader.write( resource );

    Chunk chunk;
    chunk.id = (auid)(this);
    chunk.lightset = _lightset;
    chunk.lightProps = _lightProps;
    chunk.frameId = (auid)(_frame);
    fwrite( &chunk, lightHeader.size, 1, resource->getFile() );
}

AssetObjectT Light::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader lightHeader( resource );
    if( lightHeader.type != BA_LIGHT ) throw Exception( "Unexpected chunk type" );
    if( lightHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, sizeof(Chunk), 1, resource->getFile() );

    Light* light = new Light( &chunk.lightProps );
    light->setLightset( chunk.lightset );

    AssetObjectI assetObjectI = assetObjects.find( chunk.frameId );
    if( assetObjectI != assetObjects.end() )
    {
        light->setFrame( reinterpret_cast<Frame*>( assetObjectI->second ) );
    }
    
    return AssetObjectT( chunk.id, light );
}