
#ifndef PARTICLE_SYSTEM_IMPLEMENTATION_INCLUDED
#define PARTICLE_SYSTEM_IMPLEMENTATION_INCLUDED

#include "headers.h"
#include "engine.h"
#include "texture.h"
#include "camera.h"
#include "shader.h"

const Vector billboardVertices[4] = 
{
    Vector( -1.0f, -1.0f, 0.0f ),
    Vector( -1.0f, 1.0f, 0.0f ),
    Vector( 1.0f, 1.0f, 0.0f ),
    Vector( 1.0f, -1.0f, 0.0f )
};

class ParticleSystem : public engine::IParticleSystem,
                       virtual public Lostable
{
private:
    struct AlphaSorter
    {
    public:
        unsigned int* unsortedIndices; // unsorted indices
        unsigned char nestSize[256];   // record contains corresponding nest size
        unsigned int  nest[256][256];  // sorted indices
    public:
        AlphaSorter(unsigned int numIndices);
        ~AlphaSorter();
    };
private:
    struct ParticleVertex
    {
        Vector  pos;   // transformed (worldspace) position
        Color   color; // color
        Flector uv;    // uv    
    };    
private:
    Shader*                 _shader;
    unsigned int            _numParticles;
    unsigned int            _numActiveParticles;
    engine::Particle*       _particles;    
    float                   _ambientR;
    float                   _ambientG;
    float                   _ambientB;    
    IDirect3DVertexBuffer9* _vertexBuffer;
    IDirect3DIndexBuffer9*  _indexBuffer;
    float                   _alphaSortDepth;
    AlphaSorter*            _alphaSorter;
private:
    // for primitive builder
    Matrix  _matrix;
    Vector  _x, _y, _z, _p, _temp1, _temp2;;
private:
    void alphaSortParticles(void);
private:
    inline void buildPrimitive(engine::Particle* particle, ParticleVertex* vertex, WORD* index, unsigned int particleId)
    {
        // build oriented billboard
        if( ( particle->direction[0] + particle->direction[1] + particle->direction[2] ) != 0 )
        {
            D3DXVec3Subtract( &_z, &_p, &Camera::eyePos ); 
            D3DXVec3Normalize( &_z, &_z );            
            _y = wrap( particle->direction );
            D3DXVec3Normalize( &_y, &_y );
            D3DXVec3Cross( &_x, &_y, &_z );
            D3DXVec3Normalize( &_x, &_x );
            D3DXVec3Scale( &_x, &_x, particle->size[0] );
            D3DXVec3Scale( &_y, &_y, particle->size[1] );           
            _temp1 = _y;
            D3DXVec3Normalize( &_temp1, &_temp1 );
            D3DXVec3Scale( &_p, &_temp1, particle->size[1] * -0.5f );
            _temp1 = wrap( particle->position );
            D3DXVec3Add( &_temp2, &_temp1, &_p );
            _p = _temp2;
        }
        // build true billboard
        else
        {
            _p = wrap( particle->position );
            D3DXVec3Subtract( &_z, &_p, &Camera::eyePos ); 
            D3DXVec3Normalize( &_z, &_z );
            D3DXVec3Cross( &_x, &oY, &_z ); 
            D3DXVec3Normalize( &_x, &_x );
            D3DXVec3Cross( &_y, &_z, &_x ); 
            D3DXVec3Normalize( &_y, &_y );
            D3DXVec3Scale( &_x, &_x, particle->size[0] );
            D3DXVec3Scale( &_y, &_y, particle->size[1] );
        }
        // build final transformation matrix
        _matrix._11 = _x.x, _matrix._12 = _x.y, _matrix._13 = _x.z, _matrix._14 = 0.0f,
        _matrix._21 = _y.x, _matrix._22 = _y.y, _matrix._23 = _y.z, _matrix._24 = 0.0f,
        _matrix._31 = _z.x, _matrix._32 = _z.y, _matrix._33 = _z.z, _matrix._34 = 0.0f,
        _matrix._41 = 0.0f, _matrix._42 = 0.0f, _matrix._43 = 0.0f, _matrix._44 = 1.0f;
        if( particle->rotation != 0 ) dxRotate( &_matrix, &_z, particle->rotation );
        _matrix._41 = _p.x,
        _matrix._42 = _p.y,
        _matrix._43 = _p.z,
        _matrix._44 = 1.0f;
        // transform vertex coordinates with billboard matrix
        D3DXVec3TransformCoord( &vertex[0].pos, &billboardVertices[0], &_matrix );
        D3DXVec3TransformCoord( &vertex[1].pos, &billboardVertices[1], &_matrix );
        D3DXVec3TransformCoord( &vertex[2].pos, &billboardVertices[2], &_matrix );
        D3DXVec3TransformCoord( &vertex[3].pos, &billboardVertices[3], &_matrix );
        // setup uvs
        vertex[0].uv = wrap( particle->uv[0] );
        vertex[1].uv = wrap( particle->uv[1] );
        vertex[2].uv = wrap( particle->uv[2] );
        vertex[3].uv = wrap( particle->uv[3] );
        // setup colors
        vertex[0].color = 
        vertex[1].color = 
        vertex[2].color = 
        vertex[3].color = D3DCOLOR_RGBA( 
            unsigned int( particle->color[0] * 255 ),
            unsigned int( particle->color[1] * 255 ),
            unsigned int( particle->color[2] * 255 ),
            unsigned int( particle->color[3] * 255 )
        );

        // indices
        index[0] = particleId * 4 + 0;
        index[1] = particleId * 4 + 1;
        index[2] = particleId * 4 + 2;
        index[3] = particleId * 4 + 0;
        index[4] = particleId * 4 + 2;
        index[5] = particleId * 4 + 3;
    }
public:
    // class implementation
    ParticleSystem(unsigned int numParticles, Shader* shader, float alphaSortDepth);
    virtual ~ParticleSystem();
    // IParticleSystem
    virtual void __stdcall release(void);
    virtual unsigned int __stdcall getNumParticles(void);
    virtual engine::Particle* __stdcall getParticles(void);
    virtual Vector4f __stdcall getAmbient(void);
    virtual void __stdcall setAmbient(Vector4f color);    
    // Lostable
    virtual void onLostDevice(void);
    virtual void onResetDevice(void);
public:
    // local methods
    void render(void);
};

#endif