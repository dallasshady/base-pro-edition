
#ifndef SHADOW_VOLUME_INCLUDED
#define SHADOW_VOLUME_INCLUDED

#include "headers.h"
#include "../shared/engine.h"
#include "tinyxml.h"
#include "fundamentals.h"
#include "texture.h"
#include "shader.h"
#include "../common/istring.h"
#include "geometry.h"
#include "camera.h"

/**
 * shadow volume is used to render shadows for specified geometry
 */

const DWORD shadowFVF = D3DFVF_XYZ | D3DFVF_TEX1;
const DWORD maskFVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

class ShadowVolume : public Lostable
{
public:
    struct ShadowVolumeVertex
    {
    public:
        Vector  pos;       // position
        Flector extrusion; // extrusion weight, that is 0 (no extrusion) or 1 (full extrusion)
    };
    struct ShadowMaskVertex
    {
    public:
        D3DXVECTOR4 p;
        D3DCOLOR    color;
    };
private:
    static ID3DXEffect*    _effect;
    static StaticLostable* _effectLostable;
private:
    // effect Lostable
    static void onLostEffectDevice(void);
    static void onResetEffectDevice(void);
private:
    unsigned int            _maxExtrudedEdges; // limit for edges of shadow volume, rendered per single pass
    IDirect3DVertexBuffer9* _vertexBuffer;     // vertex buffer for shadow volume
    IDirect3DIndexBuffer9*  _indexBuffer;      // index buffer for shadow volume
    IDirect3DVertexBuffer9* _maskBuffer;       // vertex buffer for shadow mask
    unsigned int            _maxEdges;         // current size of reallocable buffer for edges
    WORD*                   _edges;          // reallocable buffer for edges
    BYTE*                   _backFaces;        // reallocable buffer for flagging backfaces
public:
    ShadowVolume(unsigned int maxExtrudedEdges);
    virtual ~ShadowVolume();
public:
    void renderShadowCaster(Geometry* geometry, Matrix* ltm);
    void renderShadowVolume(Geometry* geometry, Matrix* ltm, float depth, Vector* lightPos, Vector* lightDir);
    void renderShadow(Color* maskColor);
public:
    // this class Lostable
    virtual void onLostDevice(void);
    virtual void onResetDevice(void);
public:
    // effect resource management
    static void releaseResources(void);
private:
    // private behaviour
    bool viewportMaybeShadowed(AABB* aabb, Vector* lightPos, Vector* lightDir);
    void renderBuffers(unsigned int numActiveEdges, Matrix* ltm, float depth, Vector* lightPos, Vector* lightDir, bool inside);
    void renderCappingBuffers(unsigned int numCappingFaces, Matrix* ltm, float depth, Vector* lightPos, Vector* lightDir, bool inside);
    void renderShadowCasterBuffers(unsigned int numFaces, Matrix* ltm);
    // private inlines
    inline void lockBuffers(ShadowVolumeVertex** vertex, WORD** index)
    {
        void* vertexData = NULL;
        void* indexData  = NULL;
        _dxCR( _vertexBuffer->Lock( 0, _maxExtrudedEdges * 4 * sizeof( ShadowVolumeVertex ), &vertexData, D3DLOCK_DISCARD ) );
        _dxCR( _indexBuffer->Lock( 0, _maxExtrudedEdges * 6 * sizeof( WORD ), &indexData, D3DLOCK_DISCARD ) );
        assert( vertexData );
        assert( indexData );
        *vertex = (ShadowVolumeVertex*)( vertexData );
        *index = (WORD*)( indexData );
    }
    inline void unlockBuffers(void)
    {
        _dxCR( _vertexBuffer->Unlock() );
        _dxCR( _indexBuffer->Unlock() );
    }
    inline void addEdge(WORD* edges, unsigned int& numEdges, WORD v0, WORD v1)
    {
        // remove interior edges (which appear in the list twice)
        for( unsigned int i=0; i<numEdges; ++i )
        {
            if( ( edges[2*i+0] == v0 && edges[2*i+1] == v1 ) ||
                ( edges[2*i+0] == v1 && edges[2*i+1] == v0 ) )
            {
                if( numEdges > 1 )
                {
                    edges[2*i+0] = edges[2*(numEdges-1)+0];
                    edges[2*i+1] = edges[2*(numEdges-1)+1];
                }
                --numEdges;
                return;
            }
        }  
        edges[2*numEdges+0] = v0;
        edges[2*numEdges+1] = v1;
        numEdges++;
    }    
};

#endif