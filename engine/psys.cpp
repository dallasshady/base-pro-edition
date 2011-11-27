
#include "headers.h"
#include "psys.h"

const DWORD particleFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;

/**
 * creation routine
 */

engine::IParticleSystem* Engine::createParticleSystem(unsigned int numParticles, engine::IShader* shader, float alphaSortDepth)
{
    return new ParticleSystem( numParticles, dynamic_cast<Shader*>( shader ), alphaSortDepth );
}

/**
 * alpha sorter
 */

ParticleSystem::AlphaSorter::AlphaSorter(unsigned int numIndices)
{
    assert( numIndices );
    unsortedIndices = new unsigned int[numIndices];
}

ParticleSystem::AlphaSorter::~AlphaSorter()
{
    delete[] unsortedIndices;
}

/**
 * class implementation
 */

ParticleSystem::ParticleSystem(unsigned int numParticles, Shader* shader, float alphaSortDepth)
{
    assert( numParticles );
    assert( shader );
    assert( shader->getFlags() & engine::sfLighting );

    // just a copy, this class doesn't use reference counter of shader
    _shader = shader;

    // allocate particles
    _numParticles = numParticles;
    _numActiveParticles = 0;
    _particles = new engine::Particle[_numParticles];
    memset( _particles, 0, sizeof(engine::Particle) * _numParticles );

    // reset emitter internal properties
    _ambientR = 1.0f;
    _ambientG = 1.0f;
    _ambientB = 1.0f;

    // create alpha sorter
    _alphaSorter = NULL;
    _alphaSortDepth = alphaSortDepth;
    if( _alphaSortDepth > 0 ) _alphaSorter = new AlphaSorter( _numParticles );

    // DirectX interfaces
    // WORD is size of index (16 bits), 6 is number of indices per one particle
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6 * _numParticles,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &_indexBuffer,
        NULL
    ) );

    // 4 is number of vertices per one particle
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(ParticleVertex) * 4 * _numParticles,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        particleFVF,
        D3DPOOL_DEFAULT,
        &_vertexBuffer,
        NULL
    ) );
}

ParticleSystem::~ParticleSystem()
{
    _dxCR( _indexBuffer->Release() );
    _dxCR( _vertexBuffer->Release() );
    if( _alphaSorter ) delete _alphaSorter;
    delete[] _particles;
}

/**
 * IParticleSystem
 */

void ParticleSystem::release(void)
{
    delete this;
}

unsigned int ParticleSystem::getNumParticles(void)
{
    return _numParticles;
}

engine::Particle* ParticleSystem::getParticles(void)
{
    return _particles;
}

Vector4f ParticleSystem::getAmbient(void)
{
    return Vector4f( _ambientR, _ambientG, _ambientB, 1.0f );
}

void ParticleSystem::setAmbient(Vector4f color)
{
    _ambientR = color[0];
    _ambientG = color[1];
    _ambientB = color[2];
}

void ParticleSystem::render(void)
{
    // update number of active particles
    _numActiveParticles = 0;
    for( unsigned int i=0; i<_numParticles; i++ ) if( _particles[i].visible ) _numActiveParticles++;
    
    // break rendering if nothing to render
    if( !_numActiveParticles ) return;

    // alpha-sorting
    if( _alphaSorter != NULL ) alphaSortParticles();

    // lock buffers
    void* vertexData = NULL;
    void* indexData = NULL;
    _dxCR( _vertexBuffer->Lock( 0, _numActiveParticles * 4 * sizeof( ParticleVertex ), &vertexData, D3DLOCK_DISCARD ) );
    _dxCR( _indexBuffer->Lock( 0, _numActiveParticles * 6 * sizeof( WORD ), &indexData, D3DLOCK_DISCARD ) );
    assert( vertexData );
    assert( indexData );

    // make more useful pointers
    ParticleVertex* vertex = (ParticleVertex*)( vertexData );
    WORD* index = (WORD*)( indexData );

    // build primitives
    unsigned int j;
    if( _alphaSorter )
    {
        unsigned int particleId = 0;
        unsigned char* nestSize;
        for( i=0; i<256; i++ )
        {
            nestSize = _alphaSorter->nestSize + i;
            for( j=0; j<*nestSize; j++ )
            {
                buildPrimitive( _particles + _alphaSorter->nest[i][j], vertex, index, particleId );
                particleId++;
                vertex += 4;
                index += 6;
            }
        }
    }
    else
    {
        unsigned int particleId = 0;
        for( i=0; i<_numParticles; i++ )
        {
            if( _particles[i].visible )
            {
                buildPrimitive( _particles + i, vertex, index, particleId );
                particleId++;
                vertex += 4;
                index += 6;
            }
        }
    }

    // unlock buffers
    _vertexBuffer->Unlock();
    _indexBuffer->Unlock();

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
    _dxCR( iDirect3DDevice->SetStreamSource( 0, _vertexBuffer, 0, sizeof( ParticleVertex ) ) );
    _dxCR( iDirect3DDevice->SetIndices( _indexBuffer ) );
    _dxCR( iDirect3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, _numActiveParticles * 4, 0, _numActiveParticles * 2 ) );

    _dxCR( dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE ) );
    _dxCR( dxSetRenderState( D3DRS_LIGHTING, TRUE ) );
}

/**
 * private behaviour
 */

void ParticleSystem::alphaSortParticles(void)
{
    unsigned int i,j;

    // build list of indices
    j = 0;
    for( i=0; i<_numParticles; i++ )
    {
        if( _particles[i].visible ) _alphaSorter->unsortedIndices[j] = i, j++;
    }

    // reset nest counters
    memset( _alphaSorter->nestSize, 0, sizeof( _alphaSorter->nestSize ) );

    // sort particles
    unsigned char key;
    unsigned char* nestSize;
    Vector distanceV;
    float distance;
    engine::Particle* particle;
    for( i=0; i<_numActiveParticles; i++ )
    {
        particle = _particles + _alphaSorter->unsortedIndices[i];
        distanceV = wrap( particle->position ) - Camera::eyePos;
        distance = D3DXVec3Length( &distanceV ) / _alphaSortDepth;
        if( distance > 1.0f ) distance = 1.0f;
        key = 255 - unsigned char( 255 * distance );
        nestSize = _alphaSorter->nestSize + key;
        // search for nearest thin nest
        // a priori, central nests (lower nestId) are more vacant
        while( *nestSize == 255 && key > 0 ) key--, nestSize = _alphaSorter->nestSize + key;
        // leave particle unsorted & unrendered if there are no space to sort it
        if( *nestSize == 255 ) continue;
        // add particle to corresponding nest
        _alphaSorter->nest[key][*nestSize] = _alphaSorter->unsortedIndices[i];
        (*nestSize)++;
    }
}

void ParticleSystem::onLostDevice(void)
{
    _indexBuffer->Release();
    _vertexBuffer->Release();
}

void ParticleSystem::onResetDevice(void)
{
    // DirectX interfaces
    // WORD is size of index (16 bits), 6 is number of indices per one particle
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6 * _numParticles,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &_indexBuffer,
        NULL
    ) );

    // 4 is number of vertices per one particle
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(ParticleVertex) * 4 * _numParticles,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        particleFVF,
        D3DPOOL_DEFAULT,
        &_vertexBuffer,
        NULL
    ) );
}