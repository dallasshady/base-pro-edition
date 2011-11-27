
#include "headers.h"
#include "smoketrail.h"
#include "geometry.h"
#include "camera.h"
#include "ixml.h"
#include "../common/istring.h"
#include "wire.h"

const DWORD particleFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

/**
 * creation routine
 */

engine::IRendering* Engine::createSmokeTrail(engine::IShader* shader, engine::SmokeTrailScheme* scheme)
{
    return new SmokeTrail( shader, scheme );
}

/**
 * class implementation
 */

SmokeTrail::SmokeTrail(engine::IShader* shader, engine::SmokeTrailScheme* scheme)
{
    assert( shader );
    assert( scheme->numParticles ); 

    _shader = dynamic_cast<Shader*>( shader ); assert( _shader );
    _scheme = *scheme;    

    _ambientR = (unsigned char)( _scheme.ambient[0] * 255 );
    _ambientG = (unsigned char)( _scheme.ambient[1] * 255 );
    _ambientB = (unsigned char)( _scheme.ambient[2] * 255 );

    _uvs[0] = wrap( _scheme.uv[0] );
    _uvs[1] = wrap( _scheme.uv[1] );
    _uvs[2] = wrap( _scheme.uv[2] );
    _uvs[3] = wrap( _scheme.uv[3] );

    // create rendering resources
    // WORD is size of index (16 bits), 6 is number of indices per one particle
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6 * _scheme.numParticles,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &_indexBuffer,
        NULL
    ) );

    // 4 is number of vertices per one particle
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(ParticleVertex) * 4 * _scheme.numParticles,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        particleFVF,
        D3DPOOL_DEFAULT,
        &_vertexBuffer,
        NULL
    ) );

    _enabled = false;
    _emissionPoint.x = 0, _emissionPoint.y = 0, _emissionPoint.z = 0;
    _emissionDirection.x = 0, _emissionDirection.y = 0, _emissionDirection.z = 0;
    _windVelocity.x = 0, _windVelocity.y = 0, _windVelocity.z = 0;
    _emitterVelocity.x = 0, _emitterVelocity.y = 0, _emitterVelocity.z = 0;
}

SmokeTrail::~SmokeTrail()
{
    _indexBuffer->Release();
    _vertexBuffer->Release();
}

/**
 * IRendering
 */

void SmokeTrail::release(void)
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
    Flector(0,1),
    Flector(1,1),
    Flector(1,0),
    Flector(0,0)    
};

void SmokeTrail::render(void)
{
    // lock buffers
    void* vertexData = NULL;
    void* indexData = NULL;
    _dxCR( _vertexBuffer->Lock( 0, _scheme.numParticles * 4 * sizeof( ParticleVertex ), &vertexData, D3DLOCK_DISCARD ) );
    _dxCR( _indexBuffer->Lock( 0, _scheme.numParticles * 6 * sizeof( WORD ), &indexData, D3DLOCK_DISCARD ) );
    assert( vertexData );
    assert( indexData );
    ParticleVertex* vertex = (ParticleVertex*)( vertexData );
    WORD* index = (WORD*)( indexData );

    // fill buffers
    Vector fromPos, toPos;
    Vector x,y,z,p;
    float sx, sy;
    float sizeFactor;
    Matrix m;
    unsigned int numVisibleParticles = 0;
    TrailParticleI particleI, prevParticleI;
    Vector* ageragingVertex0;
    Vector* ageragingVertex1;
    float alphaFactor;
    for( particleI = _particles.begin(); particleI != _particles.end(); particleI++ )
    {
        // build particle basis
        // last emitted particle?
        if( numVisibleParticles == 0 )
        {
            fromPos = particleI->position;
            toPos   = _emissionPoint;
        }
        // intermediate particle?
        else
        {
            prevParticleI = particleI;            
            prevParticleI--;
            fromPos = particleI->position;
            toPos   = prevParticleI->position;
        }
        p = fromPos + ( toPos - fromPos ) * 0.5f;
        y = toPos - fromPos;
        sy = D3DXVec3Length( &y ) * 0.5f;
        D3DXVec3Normalize( &y, &y );
        z = Camera::eyePos - p;
        D3DXVec3Normalize( &z, &z );
        D3DXVec3Cross( &x, &y, &z );
        D3DXVec3Normalize( &x, &x );
        D3DXVec3Cross( &z, &x, &y );
        D3DXVec3Normalize( &z, &z );
        sizeFactor = particleI->lifeTime / particleI->sizeTime;
        sizeFactor = sizeFactor > 1 ? 1 : sizeFactor;
        sx = particleI->startSize * ( 1 - sizeFactor ) + particleI->endSize * sizeFactor;
        // finalize
        m._11 = x.x * sx, m._12 = x.y * sx, m._13 = x.z * sx, m._14 = 0.0f,
        m._21 = y.x * sy, m._22 = y.y * sy, m._23 = y.z * sy, m._24 = 0.0f,
        m._31 = z.x, m._32 = z.y, m._33 = z.z, m._34 = 0.0f,
        m._41 = 0.0f, m._42 = 0.0f, m._43 = 0.0f, m._44 = 1.0f;
        // transform vertex coordinates by matrix
        D3DXVec3TransformCoord( &vertex[0].pos, &billboardVertices[0], &m );
        D3DXVec3TransformCoord( &vertex[1].pos, &billboardVertices[1], &m );
        D3DXVec3TransformCoord( &vertex[2].pos, &billboardVertices[2], &m );
        D3DXVec3TransformCoord( &vertex[3].pos, &billboardVertices[3], &m );
        vertex[0].pos.x += p.x,
        vertex[0].pos.y += p.y,
        vertex[0].pos.z += p.z,
        vertex[1].pos.x += p.x,
        vertex[1].pos.y += p.y,
        vertex[1].pos.z += p.z,
        vertex[2].pos.x += p.x,
        vertex[2].pos.y += p.y,
        vertex[2].pos.z += p.z,
        vertex[3].pos.x += p.x,
        vertex[3].pos.y += p.y,
        vertex[3].pos.z += p.z;
        // average vectex coordinates
        if( numVisibleParticles == 0 )
        {
            vertex[1].pos = _emissionPoint;
            vertex[2].pos = _emissionPoint;            
        }
        else
        {
            vertex[1].pos = 0.5f * ( *ageragingVertex0 + vertex[1].pos );
            vertex[2].pos = 0.5f * ( *ageragingVertex1 + vertex[2].pos );
            *ageragingVertex0 = vertex[1].pos;
            *ageragingVertex1 = vertex[2].pos;
        }
        ageragingVertex0 = &vertex[0].pos;
        ageragingVertex1 = &vertex[3].pos;
        // setup uvs
        vertex[0].uv = _uvs[0];
        vertex[1].uv = _uvs[1];
        vertex[2].uv = _uvs[2];
        vertex[3].uv = _uvs[3];
        // particle's alpha
        alphaFactor = ( _scheme.lifeTime - particleI->lifeTime ) / _scheme.fadeTime; 
        alphaFactor = alphaFactor > 1 ? 1 : alphaFactor;
        // setup colors
        vertex[0].color = 
        vertex[1].color = 
        vertex[2].color = 
        vertex[3].color = D3DCOLOR_RGBA( 
            particleI->colorR, 
            particleI->colorG, 
            particleI->colorB, 
            unsigned int( 255.0f * alphaFactor ) 
        );
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

    if( numVisibleParticles == 0 ) return;

    // render
    _dxCR( dxSetRenderState( D3DRS_LIGHTING, FALSE ) );
    
    _dxCR( dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE ) );
    _dxCR( dxSetRenderState( D3DRS_COLORVERTEX, TRUE ) );    
    _dxCR( dxSetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1 ) );
    _dxCR( dxSetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL ) );
    
    _dxCR( dxSetRenderState( D3DRS_CULLMODE, D3DCULL_NONE ) );
    _dxCR( dxSetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) );
    _dxCR( dxSetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA ) );
    _dxCR( dxSetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ) );
    _dxCR( dxSetRenderState( D3DRS_BLENDOP,  D3DBLENDOP_ADD ) );
    
    _shader->apply();
    /*
    const Color depthColor = D3DCOLOR_RGBA( 0,0,0,255 );
    _dxCR( dxSetRenderState( D3DRS_TEXTUREFACTOR, depthColor ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE ) );
    */

    _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
    _dxCR( iDirect3DDevice->SetFVF( particleFVF ) );
    _dxCR( iDirect3DDevice->SetStreamSource( 0, _vertexBuffer, 0, sizeof( ParticleVertex ) ) );
    _dxCR( iDirect3DDevice->SetIndices( _indexBuffer ) );
    _dxCR( iDirect3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, numVisibleParticles * 4, 0, numVisibleParticles * 2 ) );

    _dxCR( dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );
    _dxCR( dxSetRenderState( D3DRS_LIGHTING, TRUE ) ); 
}

void SmokeTrail::setProperty(const char* propertyName, float value)
{
    if( strcmp( propertyName, "enabled" ) == 0 )
    {
        _enabled = ( value != 0.0f );
    }
    else if( strcmp( propertyName, "update" ) == 0 )
    {
        // update emission
        update( value );
    }
    else
    {
        assert( !"Unexpected property for SmokeTrail object!" );
    }
}

void SmokeTrail::setProperty(const char* propertyName, const Vector3f& value)
{
    if( strcmp( propertyName, "emissionPoint" ) == 0 )
    {
        _emissionPoint = wrap( value );
    }
    else if( strcmp( propertyName, "emissionDirection" ) == 0 )
    {
        _emissionDirection = wrap( value );
    }    
    else if( strcmp( propertyName, "windVelocity" ) == 0 )
    {
        _windVelocity = wrap( value );
    }
    else if( strcmp( propertyName, "emitterVelocity" ) == 0 )
    {
        _emitterVelocity = wrap( value );
    }
    else if( strcmp( propertyName, "ambientColor" ) == 0 )
    {
        _ambientR = unsigned char( 255 * value[0] );
        _ambientG = unsigned char( 255 * value[1] );
        _ambientB = unsigned char( 255 * value[2] );
    }
    else
    {
        assert( !"Unexpected property for SmokeTrail object!" );
    }
}

void SmokeTrail::setProperty(const char* propertyName, const Matrix4f& value)
{
}

void SmokeTrail::setProperty(const char* propertyName, const Vector2f& value)
{
}

void SmokeTrail::setProperty(const char* propertyName, const Vector4f& value)
{
}

/**
 * updating
 */

void SmokeTrail::update(float dt)
{
    if( !_enabled ) return;

    Vector windDirection;
    D3DXVec3Normalize( &windDirection, &_windVelocity );
    windDirection *= -1;
    float windMagnitude = D3DXVec3Length( &_windVelocity );

    // update particles
    Vector dampingAcceleration;
    Vector windAcceleration;
    Vector heatAcceleration;
    float windDirectionVelocity;
    float windAccelerationFactor;
    for( TrailParticleI particleI = _particles.begin(); 
                        particleI != _particles.end(); 
                        particleI++ )
    {
        // update lifetime
        particleI->lifeTime += dt;

        // update position
        particleI->position += particleI->velocity * dt;

        // simulate wind force
        windDirectionVelocity = D3DXVec3Dot( &windDirection, &particleI->velocity );
        windAccelerationFactor = windDirectionVelocity / windMagnitude;
        if( windAccelerationFactor > 1 ) windAccelerationFactor = 1;
        if( windAccelerationFactor < 0 ) windAccelerationFactor = 0;
        windAccelerationFactor = 1 - windAccelerationFactor;
        windAcceleration = windDirection * windMagnitude * windAccelerationFactor;
        particleI->velocity += windAcceleration * dt;

        // simulate heat force
        heatAcceleration.x = 0;
        heatAcceleration.z = 0;
        heatAcceleration.y = _scheme.heat;
        particleI->velocity += heatAcceleration * dt;

        // simulate damping force
        dampingAcceleration = -particleI->velocity * _scheme.damping;
        particleI->velocity += dampingAcceleration * dt;
    }

    // remove scattered particles
    TrailParticleI lastParticleI = _particles.end();
    lastParticleI--;
    if( lastParticleI->lifeTime > _scheme.lifeTime )
    {
        _particles.erase( lastParticleI );
    }

    // determine fission conditions
    bool fissionByAlgorithm = ( _particles.size() == 0 );
    bool fissionByDistance = false;
    if( !fissionByAlgorithm ) 
    {
        float fissionDistance = _scheme.fissionLERP.getSaturatedValue( D3DXVec3Length( &_emitterVelocity ) );
        Vector lastParticleDistance = ( _particles.begin()->position - _emissionPoint );
        fissionByDistance = ( D3DXVec3Length( &lastParticleDistance ) >= fissionDistance );
    }

    // emit particle
    if( fissionByAlgorithm || fissionByDistance )
    {
        // handle overload
        if( _particles.size() == _scheme.numParticles )
        {
            TrailParticleI lastParticleI = _particles.end();
            lastParticleI--;
            _particles.erase( lastParticleI );
        }
        // add new particle
        _particles.push_front( 
            TrailParticle( 
                _emissionPoint,
                _emissionDirection + _emitterVelocity,
                0.0f,
                _scheme.sizeTimeLERP.getSaturatedValue( D3DXVec3Length( &_emitterVelocity ) ),
                _scheme.startSizeLERP.getSaturatedValue( D3DXVec3Length( &_emitterVelocity ) ),
                _scheme.endSizeLERP.getSaturatedValue( D3DXVec3Length( &_emitterVelocity ) ),
                _ambientR,
                _ambientG,
                _ambientB
            )
        );
    }
}

void SmokeTrail::onLostDevice(void)
{
    _indexBuffer->Release();
    _vertexBuffer->Release();
}

void SmokeTrail::onResetDevice(void)
{
    // create rendering resources
    // WORD is size of index (16 bits), 6 is number of indices per one particle
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6 * _scheme.numParticles,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &_indexBuffer,
        NULL
    ) );

    // 4 is number of vertices per one particle
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(ParticleVertex) * 4 * _scheme.numParticles,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        particleFVF,
        D3DPOOL_DEFAULT,
        &_vertexBuffer,
        NULL
    ) );
}