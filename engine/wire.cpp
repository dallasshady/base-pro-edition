
#include "headers.h"
#include "wire.h"
#include "shader.h"

/**
 * line renderer
 */

static IDirect3DVertexBuffer9* _lineVB = NULL;

static void setWireRenderStates(const Color* color)
{
    _dxCR( dxSetRenderState( D3DRS_COLORVERTEX, FALSE ) );
    _dxCR( dxSetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL ) );
    _dxCR( dxSetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL ) );
    D3DMATERIAL9 material;
    material.Diffuse  = _wrap( wrap( *color ) );
    material.Ambient  = material.Diffuse;
    material.Specular = material.Diffuse;
    material.Emissive = material.Diffuse;
    _dxCR( dxSetMaterial( &material ) ); 
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG2 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG2 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE ) );
}

static void createLineVB(void)
{
    _dxCR( dxCreateVertexBuffer( 
        2 * MAX_LINES * sizeof(Vector),
        D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
        D3DFVF_XYZ,
        D3DPOOL_DEFAULT,
        &_lineVB,
        NULL
    ) );
}

void dxRenderLine(Line* line, const Color* color, Matrix* ltm)
{
    if( !_lineVB ) createLineVB();

    // make line vertex data
    void* lockedData;
    _dxCR( _lineVB->Lock( 0, sizeof(Vector)*2, &lockedData, D3DLOCK_DISCARD ) );
    char* vertexStream = (char*)( lockedData );
    int offset = 0;
    *( (Vector*)( vertexStream + offset ) ) = line->start; offset += sizeof( Vector );
    *( (Vector*)( vertexStream + offset ) ) = line->end; offset += sizeof( Vector );
    _dxCR( _lineVB->Unlock() );

    // render line
    if( ltm ) _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, ltm ) ); 
    else 
    {
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
    }
    setWireRenderStates( color );
    _dxCR( dxSetFVF( D3DFVF_XYZ ) );
    _dxCR( dxSetStreamSource( 0, _lineVB, 0, sizeof(Vector) ) );
    _dxCR( dxDrawPrimitive( D3DPT_LINELIST, 0, 1 ) );
}

void dxRenderLines(int numLines, Line* lines, const Color* color, Matrix* ltm)
{
    assert( numLines <= MAX_LINES );

    if( !_lineVB ) createLineVB();

    // make line vertex data
    void* lockedData;
    _dxCR( _lineVB->Lock( 0, sizeof(Vector)*2*numLines, &lockedData, D3DLOCK_DISCARD ) );
    char* vertexStream = (char*)( lockedData );
    int offset = 0;
    for( int i=0; i<numLines; i++ )
    {
        *( (Vector*)( vertexStream + offset ) ) = ( lines + i )->start; offset += sizeof( Vector );
        *( (Vector*)( vertexStream + offset ) ) = ( lines + i )->end; offset += sizeof( Vector );
    }
    _dxCR( _lineVB->Unlock() );

    // render line
    if( ltm ) _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, ltm ) ); 
    else _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
    setWireRenderStates( color );
    _dxCR( dxSetFVF( D3DFVF_XYZ ) );
    _dxCR( dxSetStreamSource( 0, _lineVB, 0, sizeof(Vector) ) );
    _dxCR( dxDrawPrimitive( D3DPT_LINELIST, 0, numLines ) );
}

void dxRenderAABB(AABB* box, const Color* color, Matrix* ltm)
{
    Vector vertex[8];
    for( int i=0; i<8; i++ ) vertex[i] = Vector(
        i & 1 ? box->sup.x : box->inf.x,
        i & 2 ? box->sup.y : box->inf.y,
        i & 4 ? box->sup.z : box->inf.z
    );
    
    Line lines[12];
    lines[0]  = Line( vertex[0], vertex[1] );
    lines[1]  = Line( vertex[1], vertex[3] );
    lines[2]  = Line( vertex[3], vertex[2] );
    lines[3]  = Line( vertex[2], vertex[0] );
    lines[4]  = Line( vertex[4], vertex[5] );
    lines[5]  = Line( vertex[5], vertex[7] );
    lines[6]  = Line( vertex[7], vertex[6] );
    lines[7]  = Line( vertex[6], vertex[4] );
    lines[8]  = Line( vertex[0], vertex[4] );
    lines[9]  = Line( vertex[1], vertex[5] );
    lines[10] = Line( vertex[2], vertex[6] );
    lines[11] = Line( vertex[3], vertex[7] );

    dxRenderLines( 12, lines, color, ltm );
}

void dxRenderSphere(Sphere* sphere, const Color* color, Matrix* ltm)
{    
    Vector circleVertex[SPHERE_DETAILS +1];
    Line   circleLines[SPHERE_DETAILS];

    // generate circle in XY plane
    for( int i=0; i<SPHERE_DETAILS + 1; i++)
    {
        circleVertex[i].x = cos(i / (SPHERE_DETAILS / 2.0f) * D3DX_PI) * sphere->radius;
        circleVertex[i].y = sin(i / (SPHERE_DETAILS / 2.0f) * D3DX_PI) * sphere->radius; 
        circleVertex[i].z = 0;
    }    
    for( i=0; i<SPHERE_DETAILS; i++ ) 
    {
        circleLines[i].start = circleVertex[i], circleLines[i].end = circleVertex[i+1];
    }

    Matrix m;
    Vector temp( 1,1,0 );
    Vector scale, translation;
    float  transf;
    for( i=0; i<SPHERE_BELTS; i++ )
    {
        D3DXMatrixIdentity( &m );
        dxRotate( &m, &oY, 180.0f / SPHERE_BELTS * i );
        dxTranslate( &m, &sphere->center );
        dxRenderLines( SPHERE_DETAILS, circleLines, color, &m );

        D3DXMatrixIdentity( &m );
        transf = 1.0f/(SPHERE_BELTS)*i;
        scale = temp * sin( acos( transf ) );        
        dxScale( &m, &scale );
        translation = Vector( 0,0, transf * sphere->radius );        
        dxTranslate( &m, &translation );
        dxRotate( &m, &oX, 90 );
        dxTranslate( &m, &sphere->center );       
        dxRenderLines( SPHERE_DETAILS, circleLines, color, &m );

        D3DXMatrixIdentity( &m );
        dxScale( &m, &scale );
        translation = Vector( 0,0, -transf * sphere->radius );        
        dxTranslate( &m, &translation );
        dxRotate( &m, &oX, 90 );
        dxTranslate( &m, &sphere->center );
        dxRenderLines( SPHERE_DETAILS, circleLines, color, &m );
    }
}
