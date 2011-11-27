
#include "headers.h"
#include "shadows.h"
#include "collision.h"
#include "intersection.h"
#include "wire.h"

ID3DXEffect*    ShadowVolume::_effect = NULL;
StaticLostable* ShadowVolume::_effectLostable = NULL;

static RayIntersection* _rayIntersection = NULL;

/**
 * class implementation
 */

ShadowVolume::ShadowVolume(unsigned int maxExtrudedEdges)
{
    _maxExtrudedEdges = maxExtrudedEdges;

    // create rendering resources
    // WORD is size of index (16 bits), 6 is number of indices per one extruded edge of shadow volume
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6 * _maxExtrudedEdges,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &_indexBuffer,
        NULL
    ) );

    // 4 is number of vertices per one extruded edge of shadow volume
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(ShadowVolumeVertex) * 4 * _maxExtrudedEdges,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        shadowFVF,
        D3DPOOL_DEFAULT,
        &_vertexBuffer,
        NULL
    ) );

    // create mask buffer
    _dxCR( iDirect3DDevice->CreateVertexBuffer( 
        4*sizeof(ShadowMaskVertex),
        D3DUSAGE_WRITEONLY, maskFVF,
        D3DPOOL_MANAGED, &_maskBuffer, NULL 
    ) );

    _maxEdges = 0;
    _edges = NULL;
    _backFaces = 0;

    // try to create effect
    if( _effect == NULL )
    {
        LPD3DXBUFFER compilationErrors = NULL;
        
        if( S_OK != D3DXCreateEffectFromFile( 
            iDirect3DDevice, 
            "./res/effects/shadowvolume.fx",
            NULL,
            NULL, 
            D3DXSHADER_DEBUG,
            NULL,
            &_effect,
            &compilationErrors
        ) )
        {
            const char* text = (const char*)( compilationErrors->GetBufferPointer() );
            MessageBox( 0, text, "Effect compilation error", MB_OK );
            compilationErrors->Release();
        }

        _effectLostable = new StaticLostable( onLostEffectDevice, onResetEffectDevice );
    }

    if( !_rayIntersection ) _rayIntersection = new RayIntersection();
}

ShadowVolume::~ShadowVolume()
{
    if( _backFaces ) delete[] _backFaces;
    if( _edges ) delete[] _edges;
    _indexBuffer->Release();
    _vertexBuffer->Release();
    _maskBuffer->Release();
}

/**
 * lostable
 */

void ShadowVolume::onLostDevice(void)
{
    _indexBuffer->Release();
    _vertexBuffer->Release();
    _maskBuffer->Release();
}

void ShadowVolume::onResetDevice(void)
{
    // re-create rendering resources
    // WORD is size of index (16 bits), 6 is number of indices per one extruded edge of shadow volume
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        sizeof(WORD) * 6 * _maxExtrudedEdges,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC, 
        D3DFMT_INDEX16,
        D3DPOOL_DEFAULT, 
        &_indexBuffer,
        NULL
    ) );

    // 4 is number of vertices per one extruded edge of shadow volume
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        sizeof(ShadowVolumeVertex) * 4 * _maxExtrudedEdges,
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        shadowFVF,
        D3DPOOL_DEFAULT,
        &_vertexBuffer,
        NULL
    ) );

    // create mask buffer
    _dxCR( iDirect3DDevice->CreateVertexBuffer( 
        4*sizeof(ShadowMaskVertex),
        D3DUSAGE_WRITEONLY, maskFVF,
        D3DPOOL_MANAGED, &_maskBuffer, NULL 
    ) );
}

/**
 * rendering
 */

#define CAMERA_ZLOOK 1

void dxRenderTriangle(Vector* v0, Vector* v1, Vector* v2)
{
    Line edge( *v0, *v1 );
    dxRenderLine( &edge, &yellow, NULL );

    edge = Line( *v1, *v2 );
    dxRenderLine( &edge, &yellow, NULL );

    edge = Line( *v0, *v2 );
    dxRenderLine( &edge, &yellow, NULL );
}

bool ShadowVolume::viewportMaybeShadowed(AABB* aabb, Vector* lightPos, Vector* lightDir)
{
    // calculate viewport corners
    Vector ur, lr, ul, ll;
    float nearClipPlane = Camera::nearClipPlane;
    float nearY = nearClipPlane * tan( Camera::fov / 2.0f );
    float nearX = nearY * Camera::viewPort.Width / Camera::viewPort.Height;
    float nearZ = nearClipPlane;
    float cameraZ = nearZ * CAMERA_ZLOOK; // camera z-look direction is oppose to its equals to z-axis    
    Vector v( nearX,  nearY, cameraZ );
    D3DXVec3TransformCoord( &ur, &v, &Camera::getCurrentCamera()->frame()->LTM );
    v = Vector( nearX, -nearY, cameraZ );
    D3DXVec3TransformCoord( &lr, &v, &Camera::getCurrentCamera()->frame()->LTM );
    v = Vector( nearX, -nearY, cameraZ );
    D3DXVec3TransformCoord( &lr, &v, &Camera::getCurrentCamera()->frame()->LTM );
    v = Vector( -nearX, -nearY, cameraZ );
    D3DXVec3TransformCoord( &ll, &v, &Camera::getCurrentCamera()->frame()->LTM );
    v = Vector( -nearX,  nearY, cameraZ );
    D3DXVec3TransformCoord( &ul, &v, &Camera::getCurrentCamera()->frame()->LTM );

    // occlusion pyramid
    D3DXPLANE occlusionPyramid[6];

    // all operations are in object space
    Vector objectUR = ur, objectLR = lr, objectLL = ll, objectUL = ul;
    
    D3DXPlaneFromPoints( occlusionPyramid+0, &objectLR, &objectUR, &objectUL );

    float k = lightDir ? 1.0f : 0.0f;
    Vector light3 = lightPos ? *lightPos : *lightDir;

    v = objectUR * k + light3;
    D3DXPlaneFromPoints( occlusionPyramid+1, &v, &objectUL, &objectUR );
    //dxRenderTriangle( &v, &objectUR, &objectUL );
    v = objectLR * k + light3;
    D3DXPlaneFromPoints( occlusionPyramid+2, &v, &objectUR, &objectLR  );
    //dxRenderTriangle( &v, &objectLR, &objectUR );
    v = objectLL * k + light3;
    D3DXPlaneFromPoints( occlusionPyramid+3, &v, &objectLR, &objectLL );
    //dxRenderTriangle( &v, &objectLL, &objectLR );
    v = objectUL * k + light3;
    D3DXPlaneFromPoints( occlusionPyramid+4, &v, &objectLL, &objectUL  );
    //dxRenderTriangle( &v, &objectUL, &objectLL );

    Vector viewCenter = ( objectUR + objectLR + objectLL + objectUL ) / 4.0f;
    Vector lightDirection = lightDir ? -light3 : ( viewCenter - light3 );
    D3DXVec3Normalize( &lightDirection, &lightDirection );

    if( !lightDir )
    {
        // we can put an extra culling plane right behind the light
        lightDirection *= -1;
        D3DXPlaneFromPointNormal( occlusionPyramid+5, &light3, &lightDirection );
        lightDirection *= -1;
    }
    else
    {
        occlusionPyramid[5] = occlusionPyramid[0];
    }

    Vector worldViewVector = Camera::eyeDirection * CAMERA_ZLOOK;

    if( D3DXVec3Dot( &lightDirection, &worldViewVector ) > 0 )
    {
        // light is behind us, flip all occlusion planes
        for( int i = 0; i < 6; ++i ) 
        {
            occlusionPyramid[i].a *= -1;
            occlusionPyramid[i].b *= -1;
            occlusionPyramid[i].c *= -1;
        }
    }

    return ::intersectAABBFrustum( aabb, occlusionPyramid );
}

void ShadowVolume::renderShadowCaster(Geometry* geometry, Matrix* ltm)
{
    unsigned int numEdges = 0;
    unsigned int numFaces = geometry->getNumFaces();
    bool skinned = ( geometry->mesh()->pSkinInfo != NULL );
    Triangle* faces = geometry->getTriangles();
    Vector* vertices = skinned ? geometry->getSkinnedVertices() : geometry->getVertices();

    unsigned int numFacesToRender = 0;
    unsigned int fvId, fiId;

    // draw caster
    ShadowVolumeVertex* vertex = NULL;
    WORD* index = NULL;
    lockBuffers( &vertex, &index );
    for( unsigned int i=0; i<numFaces; i++ )
    {
        fvId = numFacesToRender * 3;
        fiId = numFacesToRender * 3;

        vertex[fvId+0].extrusion.x = vertex[fvId+0].extrusion.y = 0.0f;
        vertex[fvId+0].pos = vertices[faces[i].vertexId[0]];
        vertex[fvId+1].extrusion.x = vertex[fvId+1].extrusion.y = 0.0f;
        vertex[fvId+1].pos = vertices[faces[i].vertexId[1]];
        vertex[fvId+2].extrusion.x = vertex[fvId+2].extrusion.y = 0.0f;
        vertex[fvId+2].pos = vertices[faces[i].vertexId[2]];
        
        // fill capping indices
        index[fiId+0] = numFacesToRender * 3 + 0;
        index[fiId+1] = numFacesToRender * 3 + 1;
        index[fiId+2] = numFacesToRender * 3 + 2;

        // next face
        numFacesToRender++;

        // if vertex buffer is full
        //   - each extruded edge is rendered using a pair of triangles,
        //   - so, if we want to render triangles, we have two times more space
        if( numFacesToRender == _maxExtrudedEdges )
        {
            unlockBuffers();
            renderShadowCasterBuffers( numFacesToRender, ltm );
                    
            // re-lock buffers for next pass
            lockBuffers( &vertex, &index );
            numFacesToRender = 0; 
        }
    }
    // finally unlock buffers and render rest of triangles
    unlockBuffers();
    if( numFacesToRender ) renderShadowCasterBuffers( numFacesToRender, ltm );
}

static engine::CollisionTriangle* findAnyIntersectionCB(
    engine::CollisionTriangle* collTriangle,
    engine::IBSPSector* sector,
    engine::IAtomic* atomic,
    void* data
)
{
    *((bool*)(data)) = true;
    return NULL;
}

void ShadowVolume::renderShadowVolume(Geometry* geometry, Matrix* ltm, float depth, Vector* lightPos, Vector* lightDir)
{
    assert( lightPos || lightDir );

    // world-to-object space transformation
    Matrix iLTM = identity;
    if( ltm ) D3DXMatrixInverse( &iLTM, NULL, ltm );

    // transform lighting info in to object space
    Vector osLightPos = lightPos ? *lightPos : Vector(0,0,0);
    Vector osLightDir = lightDir ? *lightDir : Vector(0,-1,0);
    if( ltm && lightPos ) D3DXVec3TransformCoord( &osLightPos, lightPos, &iLTM );
    if( ltm && lightDir ) D3DXVec3TransformCoord( &osLightDir, lightDir, &iLTM );

    // determine we are inside or outside this shadow volume
    Line osCameraToLightSource;
    D3DXVec3TransformCoord( &osCameraToLightSource.start, &Camera::eyePos, &iLTM );
    if( lightPos )
    {
        osCameraToLightSource.end = osLightPos;
    }
    else
    {
        osCameraToLightSource.end = osCameraToLightSource.start - osLightDir * depth;
    }

    // setup pixel filling
    Color volumeColor = D3DCOLOR_RGBA( 0,0,0,255 );
    _dxCR( dxSetRenderState( D3DRS_TEXTUREFACTOR, volumeColor ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE ) );

    unsigned int numEdges = 0;
    unsigned int numFaces = geometry->getNumFaces();
    bool skinned = ( geometry->mesh()->pSkinInfo != NULL );
    Triangle* faces = geometry->getTriangles();
    Vector* vertices = skinned ? geometry->getSkinnedVertices() : geometry->getVertices();

    AABB aabb;
    if( skinned )
    {
        aabb.calculate( geometry->getNumVertices(), vertices );
    }
    else
    {
        if( ltm )
        {
            aabb.calculate( geometry->getBoundingBox(), ltm );
        }
        else
        {
            aabb = *geometry->getBoundingBox();
        }
    }    
    bool isInsideShadowVolume = viewportMaybeShadowed( &aabb, lightPos, lightDir  );
    //dxRenderAABB( &aabb, isInsideShadowVolume?&red:&white, NULL );
    //return;

    // allocate/reallocate an edge list
    if( _maxEdges < numFaces*6 )
    {
        _maxEdges = numFaces*6;
        delete[] _edges;
        _edges = new WORD[_maxEdges];
        delete[] _backFaces;
        _backFaces = new BYTE[numFaces];
    }

    // cleanup flagging array
    memset( _backFaces, 0, sizeof(BYTE)*numFaces );
    
    WORD wFace0, wFace1, wFace2;
    Vector v0, v1, v2;
    Vector vLight, vCross1, vCross2, vNormal;
    float faceDot;
    
    // for each face
    for( unsigned int i=0; i<numFaces; ++i )
    {
        // face indices
        wFace0 = faces[i].vertexId[0];
        wFace1 = faces[i].vertexId[1];
        wFace2 = faces[i].vertexId[2];

        // face vertices        
        v0 = vertices[wFace0];
        v1 = vertices[wFace1];
        v2 = vertices[wFace2];
       
        vLight = lightDir ? osLightDir : (( osLightPos - v0 ) + ( osLightPos - v1 ) + ( osLightPos - v2 )) / 3.0f;
        vCross1 = v2 - v1;
        vCross2 = v1 - v0;
        D3DXVec3Cross( &vNormal, &vCross1, &vCross2 );
        faceDot = D3DXVec3Dot( &vNormal, &vLight );

        // select only faces that are pointed on light
        if( faceDot >= 0.0f )
        {
            addEdge( _edges, numEdges, wFace0, wFace1 );
            addEdge( _edges, numEdges, wFace1, wFace2 );
            addEdge( _edges, numEdges, wFace2, wFace0 );
        }
        // but also flag the backfaces
        else if( faceDot <= 0.0f )
        {
            _backFaces[i] = 1;
        }
    }
    
    unsigned int numActiveEdges = 0;
    unsigned int fvId, fiId;

    // lock buffers in order to write data
    ShadowVolumeVertex* vertex = NULL;
    WORD* index = NULL;
    lockBuffers( &vertex, &index );

    // now extrude and render the list of edges using
    // vertex shader
    for( unsigned int i=0; i<numEdges; ++i )
    {
        fvId = numActiveEdges * 4;
        fiId = numActiveEdges * 6;

        // edge vertex 0 (extrusion weight = 0)
        vertex[fvId+0].extrusion.x = vertex[fvId+0].extrusion.y = 0.0f;
        vertex[fvId+0].pos = vertices[_edges[2*i+0]];

        // edge vertex 0 (extrusion weight = 1)
        vertex[fvId+1].extrusion.x = vertex[fvId+1].extrusion.y = 1.0f;
        vertex[fvId+1].pos = vertex[fvId+0].pos;

        // edge vertex 1 (extrusion weight = 0)
        vertex[fvId+2].extrusion.x = vertex[fvId+2].extrusion.y = 0.0f;
        vertex[fvId+2].pos = vertices[_edges[2*i+1]];

        // edge vertex 1 (extrusion weight = 1)
        vertex[fvId+3].extrusion.x = vertex[fvId+3].extrusion.y = 1.0f;
        vertex[fvId+3].pos = vertex[fvId+2].pos;

        // setup indices        
        index[fiId+0] = numActiveEdges * 4 + 0;
        index[fiId+1] = numActiveEdges * 4 + 1;
        index[fiId+2] = numActiveEdges * 4 + 2;
        index[fiId+3] = numActiveEdges * 4 + 1;
        index[fiId+4] = numActiveEdges * 4 + 3;
        index[fiId+5] = numActiveEdges * 4 + 2;

        // next edge
        numActiveEdges++;

        // if vertex buffer is full
        if( numActiveEdges == _maxExtrudedEdges )
        {
            // unlock buffers & render shadow volume
            unlockBuffers();
            renderBuffers( 
                numActiveEdges, ltm, depth, lightPos, lightDir, isInsideShadowVolume 
            );

            // re-lock buffers for next pass
            lockBuffers( &vertex, &index );

            // reset edge counter
            numActiveEdges = 0;
        }
    }

    // finally unlock buffers
    unlockBuffers();

    // if there are some extruded enges to render
    if( numActiveEdges ) 
    {
        renderBuffers( 
            numActiveEdges, ltm, depth, lightPos, lightDir, isInsideShadowVolume 
        );
    }

    // if camera is inside of shadow volume
    if( isInsideShadowVolume )
    {        
        unsigned int numCappingFaces = 0;
        unsigned int fvId, fiId;

        // draw front and back cap
        lockBuffers( &vertex, &index );
        for( i=0; i<numFaces; i++ )
        {
            fvId = numCappingFaces * 3;
            fiId = numCappingFaces * 3;

            // fill capping vertices
            if( !_backFaces[i] )
            {
                // vertex shader specification:
                //  - texture coordinate U is progressive component of vertex extrusion
                //  - texture coordinate V is absolute component of vertex extrusion
                // to prevent z-fight we extrude backfaces at the very small constance 
                // distance towards the light source
                vertex[fvId+0].extrusion.x = 0.0f;
                vertex[fvId+0].extrusion.y = -1.0f;
                vertex[fvId+0].pos = vertices[faces[i].vertexId[0]];
                vertex[fvId+1].extrusion.x = 0.0f;
                vertex[fvId+1].extrusion.y = -1.0f;
                vertex[fvId+1].pos = vertices[faces[i].vertexId[1]];
                vertex[fvId+2].extrusion.x = 0.0f;
                vertex[fvId+2].extrusion.y = -1.0f;
                vertex[fvId+2].pos = vertices[faces[i].vertexId[2]];
            }
            else
            {
                vertex[fvId+0].extrusion.x = vertex[fvId+0].extrusion.y = 1.0f;
                vertex[fvId+0].pos = vertices[faces[i].vertexId[0]];
                vertex[fvId+1].extrusion.x = vertex[fvId+1].extrusion.y = 1.0f;
                vertex[fvId+1].pos = vertices[faces[i].vertexId[1]];
                vertex[fvId+2].extrusion.x = vertex[fvId+2].extrusion.y = 1.0f;
                vertex[fvId+2].pos = vertices[faces[i].vertexId[2]];
            }

            // fill capping indices
            index[fiId+0] = numCappingFaces * 3 + 0;
            index[fiId+1] = numCappingFaces * 3 + 1;
            index[fiId+2] = numCappingFaces * 3 + 2;

            // next face
            numCappingFaces++;

            // if vertex buffer is full
            //   - each extruded edge is rendered using a pair of triangles,
            //   - so, if we want to render triangles, we have two times more space
            if( numCappingFaces == _maxExtrudedEdges*2 )
            {
                unlockBuffers();
                renderCappingBuffers( numCappingFaces, ltm, depth, lightPos, lightDir, isInsideShadowVolume );
                    
                // re-lock buffers for next pass
                lockBuffers( &vertex, &index );
                numCappingFaces = 0; 
            }
        }
        // finally unlock buffers and render rest of triangles
        unlockBuffers();
        if( numCappingFaces ) renderCappingBuffers( numCappingFaces, ltm, depth, lightPos, lightDir, isInsideShadowVolume );
    }
}

void ShadowVolume::renderShadow(Color* maskColor)
{
    // fill mask buffer
    float sx = Engine::instance->getScreenSize()[0];
    float sy = Engine::instance->getScreenSize()[1];

    ShadowMaskVertex* v;
    _dxCR( _maskBuffer->Lock( 0, 0, (void**)&v, 0 ) );
    v[0].p = D3DXVECTOR4(  0, sy, 0.0f, 1.0f );
	v[1].p = D3DXVECTOR4(  0,  0, 0.0f, 1.0f );
	v[2].p = D3DXVECTOR4( sx, sy, 0.0f, 1.0f );
	v[3].p = D3DXVECTOR4( sx,  0, 0.0f, 1.0f );
	v[0].color = v[1].color = v[2].color = v[3].color = *maskColor;
    _dxCR( _maskBuffer->Unlock() );

    // render shadow mask
    dxSetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    dxSetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
    dxSetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
    dxSetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );

    dxSetRenderState( D3DRS_STENCILENABLE, TRUE );
    dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    dxSetRenderState( D3DRS_STENCILREF,  0x1 );
    dxSetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL );
    dxSetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );

    _dxCR( dxSetRenderState( D3DRS_TEXTUREFACTOR, *maskColor ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE ) );

    iDirect3DDevice->SetFVF( maskFVF );
    iDirect3DDevice->SetStreamSource( 0, _maskBuffer, 0, sizeof(ShadowMaskVertex) );
    iDirect3DDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );

    dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    dxSetRenderState( D3DRS_STENCILENABLE, FALSE );
}

/**
 * buffer rendering
 */

void ShadowVolume::renderBuffers(unsigned int numActiveEdges, Matrix* ltm, float depth, Vector* lightPos, Vector* lightDir, bool inside)
{
    // calculate world matrix & its inverse
    Matrix world = ltm ? *ltm : identity;
    Matrix iWorld;
    D3DXMatrixInverse( &iWorld, NULL, &world );
        
    // setup world-view-projection matrix
    Matrix worldViewProj;
    D3DXMatrixMultiply( &worldViewProj, &world, &Camera::viewMatrix );
    D3DXMatrixMultiply( &worldViewProj, &worldViewProj, &Camera::projectionMatrix );
    _effect->SetMatrix( "worldViewProj", &worldViewProj );

    // setup effect arguments
    if( lightPos )
    {
        Vector osLightPos;
        D3DXVec3TransformCoord( &osLightPos, lightPos, &iWorld );
        Quartector q( osLightPos.x, osLightPos.y, osLightPos.z, 0 );
        _effect->SetVector( "osLightPos", &q );
    }
    if( lightDir )
    {
        Vector osLightDir;
        D3DXVec3TransformCoord( &osLightDir, lightDir, &iWorld );
        D3DXVec3Normalize( &osLightDir, &osLightDir );
        Quartector q( osLightDir.x, osLightDir.y, osLightDir.z, 0 );
        _effect->SetVector( "osLightDir", &q );
    }
    _effect->SetFloat( "depth", depth );        

    // setup technique
    if( lightPos )
    {
        _dxCR( _effect->SetTechnique( _effect->GetTechniqueByName( "tExtrudeForPointLight" ) ) );
    }
    else
    {
        _dxCR( _effect->SetTechnique( _effect->GetTechniqueByName( "tExtrudeForDirectionalLight" ) ) );
    }

    // first pass - increment stencil buffer
    dxSetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    dxSetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
    dxSetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
    dxSetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
    dxSetRenderState( D3DRS_STENCILENABLE, TRUE );
    dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    dxSetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );    
    dxSetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
    dxSetRenderState( D3DRS_STENCILREF, 0x1 );
    dxSetRenderState( D3DRS_STENCILMASK, 0xffffffff );
    dxSetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
    
    if( inside ) // means inside of shadow volume
    {
        // carmack's reverse
        dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
        dxSetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR );
        dxSetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
    }
    else
    {
        // default stencil pass
        dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
        dxSetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
        dxSetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCR );
    }

    unsigned int numPasses;
    _effect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( unsigned int pass = 0; pass < numPasses; pass++ )
    {
        _effect->BeginPass( pass );
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
        _dxCR( iDirect3DDevice->SetFVF( shadowFVF ) );
        _dxCR( iDirect3DDevice->SetStreamSource( 0, _vertexBuffer, 0, sizeof( ShadowVolumeVertex ) ) );
        _dxCR( iDirect3DDevice->SetIndices( _indexBuffer ) );
        _dxCR( iDirect3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, numActiveEdges * 4, 0, numActiveEdges * 2 ) );
        _effect->EndPass();
    }
    _effect->End();

    // second pass - decrement stencil buffer, obtain shadow mask
    if( inside ) // means inside of shadow volume
    {
        // carmack's reverse
        dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
        dxSetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR );
        dxSetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
    }
    else
    {
        // default stencil pass
        dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
        dxSetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );
        dxSetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
    }

    _effect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( unsigned int pass = 0; pass < numPasses; pass++ )
    {
        _effect->BeginPass( pass );
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
        _dxCR( iDirect3DDevice->SetFVF( shadowFVF ) );
        _dxCR( iDirect3DDevice->SetStreamSource( 0, _vertexBuffer, 0, sizeof( ShadowVolumeVertex ) ) );
        _dxCR( iDirect3DDevice->SetIndices( _indexBuffer ) );
        _dxCR( iDirect3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, numActiveEdges * 4, 0, numActiveEdges * 2 ) );
        _effect->EndPass();
    }
    _effect->End();

    // restore render mode
    dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
    dxSetRenderState( D3DRS_STENCILENABLE, FALSE );
}

void ShadowVolume::renderCappingBuffers(unsigned int numCappingFaces, Matrix* ltm, float depth, Vector* lightPos, Vector* lightDir, bool inside)
{
    // calculate world matrix & its inverse
    Matrix world = ltm ? *ltm : identity;
    Matrix iWorld;
    D3DXMatrixInverse( &iWorld, NULL, &world );
        
    // setup world-view-projection matrix
    Matrix worldViewProj;
    D3DXMatrixMultiply( &worldViewProj, &world, &Camera::viewMatrix );
    D3DXMatrixMultiply( &worldViewProj, &worldViewProj, &Camera::projectionMatrix );
    _effect->SetMatrix( "worldViewProj", &worldViewProj );

    // setup effect arguments
    if( lightPos )
    {
        Vector osLightPos;
        D3DXVec3TransformCoord( &osLightPos, lightPos, &iWorld );
        Quartector q( osLightPos.x, osLightPos.y, osLightPos.z, 0 );
        _effect->SetVector( "osLightPos", &q );
    }
    if( lightDir )
    {
        Vector osLightDir;
        D3DXVec3TransformCoord( &osLightDir, lightDir, &iWorld );
        D3DXVec3Normalize( &osLightDir, &osLightDir );
        Quartector q( osLightDir.x, osLightDir.y, osLightDir.z, 0 );
        _effect->SetVector( "osLightDir", &q );
    }
    _effect->SetFloat( "depth", depth );        

    // setup technique
    if( lightPos )
    {
        _dxCR( _effect->SetTechnique( _effect->GetTechniqueByName( "tExtrudeForPointLight" ) ) );
    }
    else
    {
        _dxCR( _effect->SetTechnique( _effect->GetTechniqueByName( "tExtrudeForDirectionalLight" ) ) );
    }

    // first pass - increment stencil buffer
    dxSetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
    dxSetRenderState( D3DRS_SRCBLEND, D3DBLEND_ZERO );
    dxSetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
    dxSetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
    dxSetRenderState( D3DRS_STENCILENABLE, TRUE );
    dxSetRenderState( D3DRS_ZWRITEENABLE, FALSE );
    dxSetRenderState( D3DRS_STENCILFUNC, D3DCMP_ALWAYS );    
    dxSetRenderState( D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP );
    dxSetRenderState( D3DRS_STENCILREF, 0x1 );
    dxSetRenderState( D3DRS_STENCILMASK, 0xffffffff );
    dxSetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );
    
    if( inside /* means inside of shadow volume */ )
    {
        // carmack's reverse
        dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
        dxSetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR );
        dxSetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
    }
    else
    {
        // default stencil pass
        dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
        dxSetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
        dxSetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_INCR );
    }

    unsigned int numPasses;
    _effect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( unsigned int pass = 0; pass < numPasses; pass++ )
    {
        _effect->BeginPass( pass );
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
        _dxCR( iDirect3DDevice->SetFVF( shadowFVF ) );
        _dxCR( iDirect3DDevice->SetStreamSource( 0, _vertexBuffer, 0, sizeof( ShadowVolumeVertex ) ) );
        _dxCR( iDirect3DDevice->SetIndices( _indexBuffer ) );
        _dxCR( iDirect3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, numCappingFaces * 3, 0, numCappingFaces ) );
        _effect->EndPass();
    }
    _effect->End();

    // second pass - decrement stencil buffer, obtain shadow mask
    if( inside /* means inside of shadow volume */ )
    {
        // carmack's reverse
        dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CW );
        dxSetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR );
        dxSetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
    }
    else
    {
        // default stencil pass
        dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
        dxSetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );
        dxSetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
    }

    _effect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( unsigned int pass = 0; pass < numPasses; pass++ )
    {
        _effect->BeginPass( pass );
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
        _dxCR( iDirect3DDevice->SetFVF( shadowFVF ) );
        _dxCR( iDirect3DDevice->SetStreamSource( 0, _vertexBuffer, 0, sizeof( ShadowVolumeVertex ) ) );
        _dxCR( iDirect3DDevice->SetIndices( _indexBuffer ) );
        _dxCR( iDirect3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, numCappingFaces * 3, 0, numCappingFaces ) );
        _effect->EndPass();
    }
    _effect->End();

    // restore render mode
    dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE );
    dxSetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
    dxSetRenderState( D3DRS_STENCILENABLE, FALSE );
}

void ShadowVolume::renderShadowCasterBuffers(unsigned int numFaces, Matrix* ltm)
{
    // calculate world matrix & its inverse
    Matrix world = ltm ? *ltm : identity;
    Matrix iWorld;
    D3DXMatrixInverse( &iWorld, NULL, &world );
        
    // setup world-view-projection matrix
    Matrix worldViewProj;
    D3DXMatrixMultiply( &worldViewProj, &world, &Camera::viewMatrix );
    D3DXMatrixMultiply( &worldViewProj, &worldViewProj, &Camera::projectionMatrix );
    _effect->SetMatrix( "worldViewProj", &worldViewProj );

    // setup effect arguments    
    Quartector osDumbLightDir( 0, 1, 0, 0 );
    _effect->SetVector( "osLightDir", &osDumbLightDir );
    _effect->SetFloat( "depth", 0.0f );        

    // setup technique
    _dxCR( _effect->SetTechnique( _effect->GetTechniqueByName( "tExtrudeForDirectionalLight" ) ) );

    // first pass - increment stencil buffer
    dxSetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );    
    dxSetRenderState( D3DRS_ZWRITEENABLE, TRUE );    

    unsigned int numPasses;
    _effect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
    for( unsigned int pass = 0; pass < numPasses; pass++ )
    {
        _effect->BeginPass( pass );
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
        _dxCR( iDirect3DDevice->SetFVF( shadowFVF ) );
        _dxCR( iDirect3DDevice->SetStreamSource( 0, _vertexBuffer, 0, sizeof( ShadowVolumeVertex ) ) );
        _dxCR( iDirect3DDevice->SetIndices( _indexBuffer ) );
        _dxCR( iDirect3DDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, 0, 0, numFaces * 3, 0, numFaces ) );
        _effect->EndPass();
    }
    _effect->End();
}

/**
 * effect management
 */

void ShadowVolume::onLostEffectDevice(void)
{
    if( _effect ) _effect->OnLostDevice();
}

void ShadowVolume::onResetEffectDevice(void)
{
    if( _effect ) _effect->OnResetDevice();
}

void ShadowVolume::releaseResources(void)
{
    if( _effect ) _effect->Release();
    if( _effectLostable ) delete _effectLostable;
    if( _rayIntersection ) _rayIntersection->release();
}