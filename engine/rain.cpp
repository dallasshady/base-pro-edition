
#include "headers.h"
#include "rain.h"
#include "geometry.h"
#include "camera.h"
#include "ixml.h"
#include "../common/istring.h"
#include "wire.h"

Rain::RainL Rain::_rainL;

static const DWORD maxParticlesPerPass = 8192;
static const DWORD particleFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

/**
 * vertex structure
 */

struct RainParticleVertex
{
    Vector  pos;    // position
    Color   color;  // color
    Flector uv;     // uv
};

/**
 * creation routine
 */

engine::IRendering* Engine::createRain(unsigned int maxParticles, float emissionSphere, engine::ITexture* texture, Vector4f ambient)
{
    return new Rain(maxParticles, emissionSphere, texture, ambient );
}

/**
 * rain rendering implementation
 */

Rain::Rain(unsigned int maxParticles, float emissionSphere, engine::ITexture* texture, Vector4f ambient)
{
    assert( maxParticles > 0 );
    assert( texture != NULL );

    if( maxParticles > maxParticlesPerPass ) maxParticles = maxParticlesPerPass;

    _emissionSphere = emissionSphere;
    _numParticles = maxParticles;
    _particles = new RainParticle[_numParticles];
    _ambient = wrap( ambient );
    memset( _particles, 0, sizeof(RainParticle) * _numParticles );

    // create rendering resources
    // WORD is size of index (16 bits), 6 is number of indices per one particle
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6 * maxParticlesPerPass,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &_indexBuffer,
        NULL
    ) );

    // 4 is number of vertices per one particle
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(RainParticleVertex) * 4 * maxParticlesPerPass,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        particleFVF,
        D3DPOOL_DEFAULT,
        &_vertexBuffer,
        NULL
    ) );

    // create shader
    _shader = dynamic_cast<Shader*>( Engine::instance->createShader( 1, "RainShader" ) );
    _shader->_numReferences++;
    _shader->setLayerTexture( 0, texture );
    _shader->setLayerUV( 0, 0 );
    _shader->setFlags( _shader->getFlags() | engine::sfAlphaBlending | engine::sfAlphaTesting | engine::sfLighting );
    _shader->setFlags( _shader->getFlags() & ~engine::sfCulling );
    _shader->setAlphaTestFunction( engine::cfGreater );
    _shader->setAlphaTestRef( 0 );
    _shader->setSrcBlend( engine::bmSrcAlpha );
    _shader->setDestBlend( engine::bmInvSrcAlpha );
    _shader->setBlendOp( engine::bpAdd );
    _shader->setDiffuseColor( Vector4f( 1,1,1,1 ) );
    _shader->setSpecularColor( Vector4f( 1,1,1,1 ) );

    // initialize properties
    _propCenter.x   = 0.0f;
    _propCenter.y   = 0.0f;
    _propCenter.z   = 0.0f;
    _propVelocity.x = 0.0f;
    _propVelocity.y = -1000.0f;
    _propVelocity.z = 0.0f;
    _propNBias      = 0.0f;
    _propTimeSpeed  = 1.0f;
    _useEdgeOffset  = 0;

    // initialize internals
    _centerOffset.x = _centerOffset.y = _centerOffset.z = 0.0f;
    _centerVelocity.x = _centerVelocity.y = _centerVelocity.z = 0.0f;

    // register
    _rainL.push_back( this );
}

Rain::~Rain()
{
    _shader->release();
    _indexBuffer->Release();
    _vertexBuffer->Release();
    delete[] _particles;

    for( RainI rainI=_rainL.begin(); rainI!=_rainL.end(); rainI++ )
    {
        if( (*rainI) == this )
        {
            _rainL.erase( rainI );
            break;
        }
    }
}

/**
 * IRendering
 */

void Rain::setProperty(const char* propertyName, float value)
{
    if( strcmp( propertyName, "NBias" ) == 0 )
    {
        _propNBias = value;
    }
    else if( strcmp( propertyName, "TimeSpeed" ) == 0 )
    {
        _propTimeSpeed = value;
    }
    else
    {
        assert( !"Invalid floating point property!" );
    }
}

void Rain::setProperty(const char* propertyName, const Vector3f& value)
{
    if( strcmp( propertyName, "Velocity" ) == 0 )
    {
        _propVelocity = wrap( value );
    }
    else if( strcmp( propertyName, "Center" ) == 0 )
    {
        _propCenter = wrap( value );
    }
    else
    {
        assert( !"Invalid Vector3f property!" );
    }
}

void Rain::setProperty(const char* propertyName, const Vector2f& value)
{
}

void Rain::setProperty(const char* propertyName, const Vector4f& value)
{
}

void Rain::setProperty(const char* propertyName, const Matrix4f& value)
{
    assert( !"Invalid Matrix4f property!" );
}

void Rain::release(void)
{
    delete this;
}

static const Vector billboardVertices[4] = 
{
    Vector( -1.0f, -1.0f, 0.0f ),
    Vector( -1.0f, 1.0f, 0.0f ),
    Vector( 1.0f, 1.0f, 0.0f ),
    Vector( 1.0f, -1.0f, 0.0f )
};

static const Flector billboardUVs[4] =
{
    Flector(1,1),
    Flector(1,0),
    Flector(0,0),
    Flector(0,1)
};

const float rainXScale = 1.0f;
const float rainYScale = 25.0f;
const float rainZScale = 1.0f;

void Rain::render(void)
{
    float  dot;
    Vector x,y,z;
    Vector vector;
    Matrix m;
    unsigned int i;

    // culling value
    float cullDot = cos( Camera::fov * D3DX_PI / 180.0f );

    // lock buffers
    void* vertexData = NULL;
    void* indexData = NULL;
    _dxCR( _vertexBuffer->Lock( 0, _numParticles * 4 * sizeof( RainParticleVertex ), &vertexData, D3DLOCK_DISCARD ) );
    _dxCR( _indexBuffer->Lock( 0, _numParticles * 6 * sizeof( WORD ), &indexData, D3DLOCK_DISCARD ) );
    assert( vertexData );
    assert( indexData );
    RainParticleVertex* vertex = (RainParticleVertex*)( vertexData );
    WORD* index = (WORD*)( indexData );

    // render particles
    RainParticle* particle;
    unsigned int numVisibleParticles = 0;
    for( i=0; i<_numParticles; i++ )
    {
        // particle pointer
        particle = _particles + i;

        // build billboard matrix        
        z = particle->pos - Camera::eyePos;
        D3DXVec3Normalize( &z, &z );
        // particle culling
        dot = D3DXVec3Dot( &z, &Camera::eyeDirection );
        if( -dot <= cullDot ) continue;        
        // rest of billboard matrix
        D3DXVec3Scale( &y, &particle->vel, -1 );
        D3DXVec3Normalize( &y, &y );
        D3DXVec3Cross( &x, &y, &z );
        D3DXVec3Normalize( &x, &x );
        x.x *= rainXScale, x.y *= rainXScale, x.z *= rainXScale;
        y.x *= rainYScale, y.y *= rainYScale, y.z *= rainYScale;
        z.x *= rainZScale, z.y *= rainZScale, z.z *= rainZScale;
        // finalize
        m._11 = x.x, m._12 = x.y, m._13 = x.z, m._14 = 0.0f,
        m._21 = y.x, m._22 = y.y, m._23 = y.z, m._24 = 0.0f,
        m._31 = z.x, m._32 = z.y, m._33 = z.z, m._34 = 0.0f,
        m._41 = 0.0f, m._42 = 0.0f, m._43 = 0.0f, m._44 = 1.0f;
        // transform vertex coordinates by matrix
        D3DXVec3TransformCoord( &vertex[0].pos, &billboardVertices[0], &m );
        D3DXVec3TransformCoord( &vertex[1].pos, &billboardVertices[1], &m );
        D3DXVec3TransformCoord( &vertex[2].pos, &billboardVertices[2], &m );
        D3DXVec3TransformCoord( &vertex[3].pos, &billboardVertices[3], &m );
        vertex[0].pos.x += particle->pos.x,
        vertex[0].pos.y += particle->pos.y,
        vertex[0].pos.z += particle->pos.z,
        vertex[1].pos.x += particle->pos.x,
        vertex[1].pos.y += particle->pos.y,
        vertex[1].pos.z += particle->pos.z,
        vertex[2].pos.x += particle->pos.x,
        vertex[2].pos.y += particle->pos.y,
        vertex[2].pos.z += particle->pos.z,
        vertex[3].pos.x += particle->pos.x,
        vertex[3].pos.y += particle->pos.y,
        vertex[3].pos.z += particle->pos.z;
        // setup uvs        
        vertex[0].uv = billboardUVs[0];
        vertex[1].uv = billboardUVs[1];
        vertex[2].uv = billboardUVs[2];
        vertex[3].uv = billboardUVs[3];
        // setup colors
        vertex[0].color = 
        vertex[1].color = 
        vertex[2].color = 
        vertex[3].color = _ambient;
        // indices...
        index[0] = numVisibleParticles * 4 + 0;
        index[1] = numVisibleParticles * 4 + 1;
        index[2] = numVisibleParticles * 4 + 2;
        index[3] = numVisibleParticles * 4 + 0;
        index[4] = numVisibleParticles * 4 + 2;
        index[5] = numVisibleParticles * 4 + 3;
        // next particle
        vertex += 4, index += 6, numVisibleParticles++;            
    }

    // unlock buffers
    _vertexBuffer->Unlock();
    _indexBuffer->Unlock();

    // render buffers
    // render
    _dxCR( dxSetRenderState( D3DRS_LIGHTING, FALSE ) );    
    _dxCR( dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );
    _dxCR( dxSetRenderState( D3DRS_COLORVERTEX, TRUE ) );
    _dxCR( dxSetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 ) );
    _dxCR( dxSetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL ) );
    
    _shader->apply();

    _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
    _dxCR( iDirect3DDevice->SetFVF( particleFVF ) );
    _dxCR( iDirect3DDevice->SetStreamSource( 0, _vertexBuffer, 0, sizeof( RainParticleVertex ) ) );
    _dxCR( iDirect3DDevice->SetIndices( _indexBuffer ) );
    _dxCR( iDirect3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, numVisibleParticles * 4, 0, numVisibleParticles * 2 ) );

    _dxCR( dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );
    _dxCR( dxSetRenderState( D3DRS_LIGHTING, TRUE ) ); 

    // debug info
    //mainwnd::IMainWnd* iMainWnd;
    //queryInterfaceT( "MainWnd", &iMainWnd );
    //iMainWnd->setWindowText( strformat( "Rain::render(): numVisibleParticles = %d", numVisibleParticles ).c_str() );
}

/**
 * up-to-date
 */

void Rain::onUpdate(float dt)
{
    // actual dt
    dt *= _propTimeSpeed;

    // squared size of emission sphere
    float emissionSphereSq = _emissionSphere * _emissionSphere;

    // offset of emission center
    _centerOffset = _propCenter - _centerOffset;
    bool predictionIsAvaiable = ( D3DXVec3LengthSq( &_centerOffset ) < emissionSphereSq );

    // rough speed of emission center
    D3DXVec3Scale( &_centerVelocity, &_centerOffset, 1.0f/dt );

    // motion direction of of emission center
    Vector centerMotionN;
    D3DXVec3Normalize( &centerMotionN, &_centerVelocity );

    // normal of particle velocity
    Vector velocityN;
    D3DXVec3Normalize( &velocityN, &_propVelocity );

    // magnitude of particle initial velocity
    float velocityM = D3DXVec3Length( &_propVelocity );

    // pass all of particles
    Vector r,axis;
    Matrix m;
    float space;
    RainParticle* particle;    
    for( unsigned int i=0; i<_numParticles; i++ )
    {
        // current particle
        particle = _particles + i;
        // check particle is outside of emission sphere
        D3DXVec3Subtract( &r, &particle->pos, &_propCenter );
        if( D3DXVec3LengthSq( &r ) > emissionSphereSq )
        {
            // randomize position
            if( predictionIsAvaiable ) 
            {
                r.x = getCore()->getRandToolkit()->getUniform( -1,1 );
                r.y = getCore()->getRandToolkit()->getUniform( -1,1 );
                r.z = getCore()->getRandToolkit()->getUniform( -1,1 );
                D3DXVec3Normalize( &r, &r );
                r += centerMotionN;
            }
            else
            {
                r.x = getCore()->getRandToolkit()->getUniform( -1,1 );
                r.y = getCore()->getRandToolkit()->getUniform( -1,1 );
                r.z = getCore()->getRandToolkit()->getUniform( -1,1 );
            }
            D3DXVec3Normalize( &r, &r );
            space = getCore()->getRandToolkit()->getUniform( 0, _emissionSphere );
            D3DXVec3Scale( &particle->pos, &r, space );
            particle->pos += _propCenter;
            // predict position by velocity of emission center
            if( predictionIsAvaiable ) particle->pos += _centerVelocity * dt;

            // setup particle velocity
            if( _propNBias > 0 )
            {
                m = identity;
                axis.x = getCore()->getRandToolkit()->getUniform( -1,1 );
                axis.y = getCore()->getRandToolkit()->getUniform( -1,1 );
                axis.z = getCore()->getRandToolkit()->getUniform( -1,1 );
                D3DXVec3Normalize( &axis, &axis );
                dxRotate( &m, &axis, getCore()->getRandToolkit()->getUniform( - _propNBias, _propNBias ) );
                D3DXVec3TransformNormal( &r, &velocityN, &m );
                D3DXVec3Scale( &particle->vel, &r, velocityM );
            }
            else
            {
                particle->vel = _propVelocity;
            }
            // carefully move particle towards edge of emission sphere
            // (this feature avaiable is for each second particle)
            if( _useEdgeOffset )
            {
                _useEdgeOffset = 0;
                D3DXVec3Normalize( &r, &particle->vel );
                space = _emissionSphere - space;
                r.x *= space, r.y *= space, r.z *= space;
                particle->pos -= r;
            }
            else
            {
                _useEdgeOffset = 1;
            }
        }
    }

    // pass all of particles
    for( i=0; i<_numParticles; i++ )
    {
        // current particle
        particle = _particles + i;
        // move particle
        particle->pos += particle->vel * dt;
    }

    // store current emission center
    _centerOffset = _propCenter;
}

void Rain::update(float dt)
{
    for( RainI rainI=_rainL.begin(); rainI!=_rainL.end(); rainI++ )
    {
        (*rainI)->onUpdate( dt );
    }
}

void Rain::onLostDevice(void)
{
    _indexBuffer->Release();
    _vertexBuffer->Release();
}

void Rain::onResetDevice(void)
{
    // create rendering resources
    // WORD is size of index (16 bits), 6 is number of indices per one particle
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6 * maxParticlesPerPass,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &_indexBuffer,
        NULL
    ) );

    // 4 is number of vertices per one particle
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(RainParticleVertex) * 4 * maxParticlesPerPass,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        particleFVF,
        D3DPOOL_DEFAULT,
        &_vertexBuffer,
        NULL
    ) );
}