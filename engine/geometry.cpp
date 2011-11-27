
#include "headers.h"
#include "geometry.h"
#include "vertexdeclaration.h"
#include "asset.h"
#include "effect.h"
#include "wire.h"

/**
 * creation routine
 */

engine::IGeometry* Engine::createGeometry(
    int numVertices, 
    int numTriangles, 
    int numUVSets, 
    int numShaders, 
    int numPrelights, 
    bool sharedShaders,    
    const char* geometryName
)
{
    return new Geometry( numVertices, numTriangles, numUVSets, numShaders, numPrelights, sharedShaders, geometryName );
}

/**
 * class implementation
 */

Geometry::Geometry(
    int  numVertices,
    int  numTriangles,
    int  numUVSets,    
    int  numShaders,
    int  numPrelights,
    bool sharedShaders,
    const char* geometryName
)
{
    assert( numUVSets >= 0 && numUVSets < engine::maxTextureLayers );
    assert( numPrelights >= 0 && numPrelights <= engine::maxPrelightLayers );

    _numReferences = 1;
    _name          = geometryName ? geometryName : "UnnamedGeometry";
    _numVertices   = numVertices;
    _numTriangles  = numTriangles;
    _numUVSets     = numUVSets;
    _numShaders    = numShaders;
    _numPrelights  = numPrelights;
    _sharedShaders = sharedShaders;

    _vertices = new Vector[_numVertices];
    _normals  = new Vector[_numVertices];
    for( int i=0; i<engine::maxPrelightLayers; i++ ) if( i<_numPrelights ) _prelights[i] = new Color[_numVertices]; else _prelights[i] = NULL;
    for( i=0; i<engine::maxTextureLayers; i++ ) if( i<_numUVSets ) _uvs[i] = new Flector[_numVertices]; else _uvs[i] = NULL; 
    _triangles = new Triangle[_numTriangles];
    _skinnedVertices = NULL; // no skinned vertices for non-skinned mesh
    if( !_sharedShaders ) 
    {
        _shaders = new Shader*[numShaders];
        memset( _shaders, 0, sizeof(Shader*) * numShaders );
    }

    _vertexDeclaration = dxGetVertexDeclaration( _numUVSets, _numPrelights );
    _mesh = NULL;
    _ocTreeRoot = NULL;
    _effect = NULL;
}

Geometry::Geometry(
    Mesh* mesh,
    const char* geometryName
)
{
    assert( mesh->OriginalMeshData.Type == D3DXMESHTYPE_MESH );

    _numReferences = 1;
    _name          = geometryName;
    _numVertices   = 0;
    _numTriangles  = 0;
    _numUVSets     = 0;
    _numShaders    = 0;
    _numPrelights  = 0;
    _sharedShaders = false;
    _vertices      = NULL;
    _normals       = NULL;
    _triangles     = NULL;
    for( int i=0; i<engine::maxPrelightLayers; i++ ) _prelights[i] = NULL;    
    for( i=0; i<engine::maxTextureLayers; i++ ) _uvs[i] = NULL;
    _boundingBox.sup = _boundingBox.inf = Vector(0,0,0);
    _boundingSphere.center = Vector(0,0,0);
    _boundingSphere.radius = 0;        
    _shaders = NULL;    
    _effect = NULL;
    _ocTreeRoot = NULL;
    _skinnedVertices = NULL;

    // set given mesh as teh geometry mesh and capture mesh data in to 
    // hardware-independent structures
    _mesh = mesh;   
    captureMeshData( true );
}

Geometry::~Geometry()
{
    if( _effect )
    {
        reinterpret_cast<Effect*>( _effect )->release();
    }
    if( !_sharedShaders ) 
    {
        for( int i=0; i<_numShaders; i++ ) _shaders[i]->release();
        delete[] _shaders;
    }    
    if( _skinnedVertices )
    {
        delete[] _skinnedVertices;
    }
    delete[] _triangles;
    for( int i=0; i<engine::maxTextureLayers; i++ ) if( _uvs[i] ) delete[] _uvs[i];
    for( i=0; i<engine::maxPrelightLayers; i++ ) if( _prelights[i] ) delete[] _prelights[i];
    delete[] _normals;
    delete[] _vertices;

    if( _mesh ) delete _mesh;

    if( _ocTreeRoot ) delete _ocTreeRoot;
}

/**
 * IGeometry
 */

int Geometry::getNumReferences(void)
{
    return _numReferences;
}

void Geometry::release(void)
{
    _numReferences--;
    if( _numReferences == 1 ) delete this;
}

const char* Geometry::getName(void)
{
    return _name.c_str();
}

Vector3f Geometry::getAABBInf(void)
{
    return wrap( _boundingBox.inf );
}

Vector3f Geometry::getAABBSup(void)
{
    return wrap( _boundingBox.sup );
}

int Geometry::getNumShaders(void)
{
    return this->_numShaders;
}

engine::IShader* Geometry::getShader(int shaderId)
{
    assert( shaderId>=0 && shaderId<_numShaders ); 
    return _shaders[shaderId];
}

void Geometry::setShader(int shaderId, engine::IShader* shader)
{
    assert( shaderId>=0 && shaderId<_numShaders ); 
    setShader( shaderId, dynamic_cast<Shader*>( shader ) );
}

engine::Mesh* Geometry::createMesh(void)
{
    engine::Mesh* mesh = new engine::Mesh;
    mesh->numVertices  = _numVertices;
    mesh->numTriangles = _numTriangles;
    mesh->vertices     = new Vector3f[_numVertices];
    mesh->triangles    = new engine::Mesh::Triangle[_numTriangles];

    for( int i=0; i<_numVertices; i++ )
    {
        mesh->vertices[i] = wrap( _vertices[i] );
    }
    for( i=0; i<_numTriangles; i++ )
    {
        mesh->triangles[i].vertexId[0] = _triangles[i].vertexId[0];
        mesh->triangles[i].vertexId[1] = _triangles[i].vertexId[1];
        mesh->triangles[i].vertexId[2] = _triangles[i].vertexId[2];
        mesh->triangles[i].shaderId    = _triangles[i].shaderId;
    }

    return mesh;
}

engine::Mesh* Geometry::getMeshInfo(void)
{
    engine::Mesh* mesh = new engine::Mesh;
    mesh->numVertices  = _numVertices;
    mesh->numTriangles = _numTriangles;
    return mesh;
}

#pragma warning(disable:4018)
void Geometry::setMesh(engine::Mesh* mesh)
{    
    assert( mesh->numVertices == _numVertices );
    assert( mesh->numTriangles == _numTriangles );
    assert( mesh->numUVs == _numUVSets );

    unsigned int i,j;
    for( i=0; i<_numVertices; i++ )
    {
        _vertices[i] = wrap( mesh->vertices[i] );
    }
    for( i=0; i<_numUVSets; i++ )
    {
        for( j=0; j<_numVertices; j++ )
        {
            _uvs[i][j] = wrap( mesh->uvs[i][j] );
        }
    }
    for( i=0; i<_numTriangles; i++ )
    {
        _triangles[i].set(
            mesh->triangles[i].vertexId[0],
            mesh->triangles[i].vertexId[1],
            mesh->triangles[i].vertexId[2],
            mesh->triangles[i].shaderId
        );
    }
    // calculate per-triangle normals
    Vector e01, e02, n;
    for( i=0; i<_numTriangles; i++ )
    {
        D3DXVec3Subtract( 
            &e01, 
            _vertices + mesh->triangles[i].vertexId[1],
            _vertices + mesh->triangles[i].vertexId[0]
        );
        D3DXVec3Subtract( 
            &e02, 
            _vertices + mesh->triangles[i].vertexId[2],
            _vertices + mesh->triangles[i].vertexId[0]
        );
        D3DXVec3Cross( &n, &e02, &e01 );
        D3DXVec3Normalize( &n, &n );
        _normals[mesh->triangles[i].vertexId[0]] = n;
        _normals[mesh->triangles[i].vertexId[1]] = n;
        _normals[mesh->triangles[i].vertexId[2]] = n;
    }

    instance();
}
#pragma warning(default:4018)

void Geometry::forcePrelight(const Vector4f& color)
{
    if( _numPrelights == 0 )
    {
        _numPrelights = 1;
        _prelights[0] = new Color[_numPrelights];
    }

    Color c = wrap( color );
    for( int i=0; i<_numVertices; i++ ) _prelights[0][i] = c;

    _vertexDeclaration = dxGetVertexDeclaration( _numUVSets, _numPrelights );

    instance();
}

engine::IEffect* Geometry::getEffect(void)
{
    return reinterpret_cast<Effect*>( _effect );
}

void Geometry::setEffect(engine::IEffect* effect)
{
    Effect* e = dynamic_cast<Effect*>( effect );
    if( e ) assert( e->isCompatible( this ) );

    if( _effect != NULL ) reinterpret_cast<Effect*>( _effect )->onSetEffect( NULL );
    _effect = e;
    if( e ) e->onSetEffect( this );
}

bool Geometry::isBorderEdge(int triangleId, int edgeId)
{
    assert( triangleId>=0 && triangleId<_numTriangles );
    assert( edgeId>=0 && edgeId<3 );

    // determine edge vertices
    int vertexId[2];
    switch( edgeId )
    {
    case 0: 
        vertexId[0] = _triangles[triangleId].vertexId[0];
        vertexId[1] = _triangles[triangleId].vertexId[1];
        break;
    case 1: 
        vertexId[0] = _triangles[triangleId].vertexId[0];
        vertexId[1] = _triangles[triangleId].vertexId[2];
        break;
    case 2: 
        vertexId[0] = _triangles[triangleId].vertexId[1];
        vertexId[1] = _triangles[triangleId].vertexId[2];
        break;
    }

    // check if specified edge is not shared between more than one triangle
    int numShares = 0;
    for( int i=0; i<_numTriangles; i++ )
    {
        if( _triangles[i].vertexId[0] == vertexId[0] && _triangles[i].vertexId[1] == vertexId[1] ||
            _triangles[i].vertexId[1] == vertexId[0] && _triangles[i].vertexId[0] == vertexId[1] )
        {
            numShares++;
        }
        if( _triangles[i].vertexId[0] == vertexId[0] && _triangles[i].vertexId[2] == vertexId[1] ||
            _triangles[i].vertexId[2] == vertexId[0] && _triangles[i].vertexId[0] == vertexId[1] )
        {
            numShares++;
        }
        if( _triangles[i].vertexId[1] == vertexId[0] && _triangles[i].vertexId[2] == vertexId[1] ||
            _triangles[i].vertexId[2] == vertexId[0] && _triangles[i].vertexId[1] == vertexId[1] )
        {
            numShares++;
        }
    }

    return ( numShares == 1 );
}

int Geometry::getNumFaces(void)
{
    return _numTriangles;
}

void Geometry::getFace(int faceId, Vector3f& v0, Vector3f& v1, Vector3f& v2, engine::IShader** shader)
{
    assert( faceId >= 0 && faceId < _numTriangles );
    v0 = wrap( _vertices[_triangles[faceId].vertexId[0]] );
    v1 = wrap( _vertices[_triangles[faceId].vertexId[1]] );
    v2 = wrap( _vertices[_triangles[faceId].vertexId[2]] );
    if( shader ) *shader = _shaders[_triangles[faceId].shaderId];
}

void Geometry::generateSkinTangents(void)
{
    // only for skinned geometry
    if( !_mesh->pSkinInfo ) return;

    // retrieve current vertex declaration
    D3DVERTEXELEMENT9 vertexDeclaration[MAX_FVF_DECL_SIZE];
    _mesh->getDeclaration( vertexDeclaration );

    // pass to the end of vertex declaration
    unsigned int i = 0;
    WORD lastStream;
    WORD lastOffset;
    BYTE lastType;
    bool haveTangents = false;
    while( !dxIsEndOfDeclaration( vertexDeclaration + i ) )
    {
        if( vertexDeclaration[i].Usage == D3DDECLUSAGE_TANGENT ||
            vertexDeclaration[i].Usage == D3DDECLUSAGE_BINORMAL )
        {
            haveTangents = true;
        }

        lastStream = vertexDeclaration[i].Stream;
        lastOffset = vertexDeclaration[i].Offset;
        lastType   = vertexDeclaration[i].Type;
        i++;
        if( i == MAX_FVF_DECL_SIZE )
        {
            throw Exception( "Unexpected vertex declaration found by Geometry::generateSkinTangents()" );
        }
    }

    // do not rebuild tangents if they already are exists
    if( haveTangents ) return;

    // insert declaration of tangent
    vertexDeclaration[i].Stream     = lastStream;
    vertexDeclaration[i].Offset     = lastOffset + dxGetDeclTypeSize( D3DDECLTYPE( lastType ) );
    vertexDeclaration[i].Type       = D3DDECLTYPE_FLOAT3;
    vertexDeclaration[i].Method     = D3DDECLMETHOD_DEFAULT;
    vertexDeclaration[i].Usage      = D3DDECLUSAGE_TANGENT;
    vertexDeclaration[i].UsageIndex = 0;
    lastOffset = vertexDeclaration[i].Offset;
    lastType   = vertexDeclaration[i].Type;
    i++;
    assert( i<MAX_FVF_DECL_SIZE );

    // insert declaration of binormal
    vertexDeclaration[i].Stream     = lastStream;
    vertexDeclaration[i].Offset     = lastOffset + dxGetDeclTypeSize( D3DDECLTYPE( lastType ) );
    vertexDeclaration[i].Type       = D3DDECLTYPE_FLOAT3;
    vertexDeclaration[i].Method     = D3DDECLMETHOD_DEFAULT;
    vertexDeclaration[i].Usage      = D3DDECLUSAGE_BINORMAL;
    vertexDeclaration[i].UsageIndex = 0;
    lastOffset = vertexDeclaration[i].Offset;
    lastType   = vertexDeclaration[i].Type;
    i++;
    assert( i<MAX_FVF_DECL_SIZE );

    // close declaration
    vertexDeclaration[i].Stream     = emptyDeclaration[0].Stream;
    vertexDeclaration[i].Offset     = emptyDeclaration[0].Offset;
    vertexDeclaration[i].Type       = emptyDeclaration[0].Type;
    vertexDeclaration[i].Method     = emptyDeclaration[0].Method;
    vertexDeclaration[i].Usage      = emptyDeclaration[0].Usage;
    vertexDeclaration[i].UsageIndex = emptyDeclaration[0].UsageIndex;

    // update mesh declaration & calculate tangents
    _mesh->updateDeclaration( D3DXMESH_VB_MANAGED | D3DXMESH_IB_MANAGED, vertexDeclaration );
    _mesh->calculateTangents( 0 );

    // check metrics of original mesh
    if( _mesh->OriginalMeshData.pMesh->GetNumVertices() != _numVertices )
    {
        // capture rebuilded mesh data, except for shaders
        captureMeshData( false );
    }
}

/**
 * module locals
 */

void Geometry::setShaders(Shader** shaders)
{
    assert( _sharedShaders );
    _shaders = shaders;
}

void Geometry::setShader(int id, Shader* shader)
{
    if( _shaders[id] ) _shaders[id]->release();
    _shaders[id] = shader;
    if( _shaders[id] ) _shaders[id]->_numReferences++;
}

void Geometry::instance(void)
{
    // release previous mesh instance
    if( _mesh ) delete _mesh;

    // recalculate bounding box
    _boundingBox.calculate( _numVertices, _vertices );
    _boundingSphere.calculate( _numVertices, _vertices );

    _mesh = new Mesh( 
        _numVertices, 
        _numTriangles, 
        _numShaders,
        D3DXMESH_VB_DYNAMIC | D3DXMESH_IB_DYNAMIC,
        _vertexDeclaration
    );

    // vertex buffer
    unsigned char* vertexData = (unsigned char*)( _mesh->lockVertexBuffer( D3DLOCK_DISCARD ) );
    // fill vertex buffer
    int i, j, offset = 0;
    for( i=0; i<_numVertices; i++ )
    {
        memcpy( vertexData+offset, _vertices+i, sizeof(Vector) ); offset += sizeof(Vector);
        memcpy( vertexData+offset, _normals+i, sizeof(Vector) ); offset += sizeof(Vector);
        for( j=0; j<_numUVSets; j++ )
        {
            memcpy( vertexData+offset, _uvs[j]+i, sizeof(Flector) ); offset += sizeof(Flector);
        }
        for( j=0; j<_numPrelights; j++ )
        {
            memcpy( vertexData+offset, _prelights[j]+i, sizeof(Color) ); offset += sizeof(Color);
        }
    }
    _mesh->unlockVertexBuffer();

    // fill index buffer
    unsigned char* indexData = (unsigned char*)( _mesh->lockIndexBuffer( D3DLOCK_DISCARD ) );
    offset = 0;
    for( i=0; i<_numTriangles; i++ )
    {
        memcpy( indexData+offset, _triangles[i].vertexId, sizeof(WORD) * 3 );
        offset += sizeof(WORD) * 3;
    }
    _mesh->unlockIndexBuffer();

    DWORD* attrBuffer = (DWORD*)( _mesh->lockAttributeBuffer( D3DLOCK_DISCARD ) );
    for( i=0; i<_numTriangles; i++ ) 
    {
        attrBuffer[i] = _triangles[i].shaderId;
    }
    _mesh->unlockAttributeBuffer();

    // check normal maps across shaders
    bool hasNormalMap = false;
    unsigned int normalMapUV = 0;
    for( int i=0; i<_numShaders; i++ )
    {
        if( _shaders[i]->getNormalMap() )
        {
            hasNormalMap = true;
            normalMapUV  = _shaders[i]->getNormalMapUV();
            break;
        }
    }

    // update vertex declaration & rebuild mesh
    if( _numPrelights == 0 && hasNormalMap )
    {
        if( _vertexDeclaration == vertexNormalUV1 )
        {
            _vertexDeclaration = vertexNormalUV1TangentBinormal;
        }
        else if( _vertexDeclaration == vertexNormalUV2 )
        {
            _vertexDeclaration = vertexNormalUV2TangentBinormal;
        }
        else if( _vertexDeclaration == vertexNormalUV3 )
        {
            _vertexDeclaration = vertexNormalUV3TangentBinormal;
        }
        else if( _vertexDeclaration == vertexNormalUV4 )
        {
            _vertexDeclaration = vertexNormalUV4TangentBinormal;
        }
        else
        {
            assert( !"shouldn't be here!" );
        }
        _mesh->updateDeclaration( D3DXMESH_VB_MANAGED | D3DXMESH_IB_MANAGED, _vertexDeclaration );        
        _mesh->calculateTangents( normalMapUV );
    }

    // optimize mesh
    _mesh->optimize( D3DXMESH_VB_MANAGED | D3DXMESH_IB_MANAGED | D3DXMESHOPT_COMPACT | D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_VERTEXCACHE );

    if( _boundingBox.inf.x == _boundingBox.sup.x )
    {
        _boundingBox.inf.x -= 0.13f;
        _boundingBox.sup.x += 0.17f;
    }
    if( _boundingBox.inf.y == _boundingBox.sup.y )
    {
        _boundingBox.inf.y -= 0.13f;
        _boundingBox.sup.y += 0.17f;
    }
    if( _boundingBox.inf.z == _boundingBox.sup.z )
    {
        _boundingBox.inf.z -= 0.13f;
        _boundingBox.sup.z += 0.17f;
    }

    if( _ocTreeRoot == NULL ) 
    {
        _ocTreeRoot = new OcTreeSector( NULL, _boundingBox, this );
    }
}

void Geometry::render(void)
{
    if( _numPrelights )
    {
        _dxCR( dxSetRenderState( D3DRS_LIGHTING, FALSE ) );
        _dxCR( dxSetRenderState( D3DRS_COLORVERTEX, TRUE ) );
        _dxCR( dxSetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1 ) );
    }
    else
    {
        _dxCR( dxSetRenderState( D3DRS_LIGHTING, TRUE ) );
        _dxCR( dxSetRenderState( D3DRS_COLORVERTEX, FALSE ) );
        _dxCR( dxSetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL ) );
    }

    int shaderId;
    int numAttributes = _mesh->NumAttributeGroups;
    for( int i=0; i<numAttributes; i++ )
    {
        shaderId = _mesh->getAttributeId( i );

        if( !shader( shaderId )->isInvisible() )
        {
            // shading technique contains alpha blending/testing?
            if( ( _shaders[shaderId]->getFlags() & engine::sfAlphaBlending || 
                  _shaders[shaderId]->getFlags() & engine::sfAlphaTesting 
                ) &&
                ( Atomic::currentAtomic || 
                  BSPSector::currentSector 
                )
              )
            {
                if( Atomic::currentAtomic )
                {
                    BSP* bsp = getAtomicBSP( Atomic::currentAtomic );
                    if( bsp ) bsp->addAlphaGeometry( Atomic::currentAtomic, i );
                }
                else if( BSPSector::currentSector )
                {
                    BSP* bsp = BSPSector::currentSector->bsp(); assert( bsp );
                    bsp->addAlphaGeometry( BSPSector::currentSector, i );
                }
            }
            else if( _shaders[shaderId]->effect() )
            {
                reinterpret_cast<Effect*>( _shaders[shaderId]->effect() )->render( 
                    _mesh, i, _shaders[shaderId]
                );
                dxSetVertexShader( NULL );
                dxSetPixelShader( NULL );
            }
            else
            {
                if( _mesh->pSkinInfo )
                {
                    int i=0;
                }
                _shaders[shaderId]->apply();
                _mesh->renderSubset( i, _shaders[shaderId] );
                // if shader use non-default lightset, then recalculate 
                // lighting back to default lightset
                if( _shaders[shaderId]->getLightset() )
                {
                    BSP::currentBSP->calculateGlobalAmbient( 0 );
                    BSPSector::currentSector->illuminate( 0 );
                }
            }
        }
    }
}

const Color depthColor = D3DCOLOR_RGBA( 0,0,0,255 );

void Geometry::renderDepthMap(void)
{
    // for each triangle - the fill rule is simple opaque black-colored painting    
    _dxCR( dxSetRenderState( D3DRS_TEXTUREFACTOR, depthColor ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TFACTOR ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
    _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TFACTOR ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
    _dxCR( dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_DISABLE ) );    

    int shaderId;
    int numAttributes = _mesh->NumAttributeGroups;

    for( int i=0; i<numAttributes; i++ )
    {
        shaderId = _mesh->getAttributeId( i );

        if( !shader( shaderId )->isInvisible() )
        {            
            _mesh->renderSubset( i, shader( shaderId ) );
        }
    }
}

void Geometry::renderAlphaGeometry(unsigned int subsetId)
{   
    if( _numPrelights )
    {
        _dxCR( dxSetRenderState( D3DRS_COLORVERTEX, TRUE ) );
        _dxCR( dxSetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1 ) );
    }
    else
    {
        _dxCR( dxSetRenderState( D3DRS_COLORVERTEX, FALSE ) );
        _dxCR( dxSetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL ) );
    }

    unsigned int shaderId = _mesh->getAttributeId( subsetId );
    if( _shaders[shaderId]->effect() )
    {
        reinterpret_cast<Effect*>( _shaders[shaderId]->effect() )->render( 
            _mesh, subsetId, _shaders[shaderId]
        );
        dxSetVertexShader( NULL );
        dxSetPixelShader( NULL );
    }
    else
    {
        _shaders[shaderId]->apply();
        _mesh->renderSubset( subsetId, _shaders[shaderId] );
    }
}

/**
 * serialization
 */

void Geometry::write(IResource* resource)
{
    ChunkHeader geometryHeader( BA_GEOMETRY, sizeof( Chunk ) );
    geometryHeader.write( resource );

    Chunk chunk;
    chunk.id = (auid)( this );
    memset( chunk.name, 0, engine::maxNameLength );
    if( _name.length() > engine::maxNameLength )
    {
        strncpy( chunk.name, _name.c_str(), engine::maxNameLength - 1 );
    }
    else
    {
        strcpy( chunk.name, _name.c_str() );
    }
    chunk.numShaders    = getNumShaders();
    chunk.numVertices   = getNumVertices();
    chunk.numUVSets     = getNumUVSets();
    chunk.numPrelights  = getNumPrelights();
    chunk.numTriangles  = getNumTriangles();
    chunk.sharedShaders = _sharedShaders;
    if( _ocTreeRoot )
    {
        chunk.numOcTreeSectors = _ocTreeRoot->getOcTreeSize();
    }
    else
    {
        chunk.numOcTreeSectors = 0;
    }
    chunk.hasEffect = ( _effect != NULL );

    fwrite( &chunk, sizeof( Chunk ), 1, resource->getFile() );

    // write shaders
    if( !_sharedShaders ) for( int i=0; i<getNumShaders(); i++ )
    {
        _shaders[i]->write( resource );
    }

    // write vertices
    ChunkHeader verticesHeader( BA_BINARY, sizeof(Vector) * getNumVertices() );
    verticesHeader.write( resource );
    fwrite( getVertices(), verticesHeader.size, 1, resource->getFile() );

    // write normals
    ChunkHeader normalsHeader( BA_BINARY, sizeof(Vector) * getNumVertices() );
    normalsHeader.write( resource );
    fwrite( getNormals(), normalsHeader.size, 1, resource->getFile() );

    // write UV-sets
    for( int i=0; i<getNumUVSets(); i++ )
    {
        ChunkHeader uvsHeader( BA_BINARY, sizeof(Flector) * getNumVertices() );
        uvsHeader.write( resource );
        fwrite( getUVSet(i), uvsHeader.size, 1, resource->getFile() );
    }

    // write prelights
    for( i=0; i< getNumPrelights(); i++ )
    {
        ChunkHeader prelightsHeader( BA_BINARY, sizeof(Color) * getNumVertices() );
        prelightsHeader.write( resource );
        fwrite( getPrelights(i), prelightsHeader.size, 1, resource->getFile() );
    }

    // write triangles
    ChunkHeader trianglesHeader( BA_BINARY, sizeof(Triangle) * getNumTriangles() );
    trianglesHeader.write( resource );
    fwrite( getTriangles(), trianglesHeader.size, 1, resource->getFile() );

    // write shaders
    if( !_sharedShaders )
    {
        ChunkHeader shadersHeader( BA_BINARY, sizeof(auid) * getNumShaders() );
        shadersHeader.write( resource );
        fwrite( _shaders, shadersHeader.size, 1, resource->getFile() );
    }

    // write octree
    if( _ocTreeRoot )
    {
        _ocTreeRoot->write( resource );
    }

    // write effect
    if( chunk.hasEffect )
    {
        reinterpret_cast<Effect*>( _effect )->write( resource );
    }
}

AssetObjectT Geometry::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader geometryHeader( resource );
    if( geometryHeader.type != BA_GEOMETRY ) throw Exception( "Unexpected chunk type" );
    if( geometryHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, sizeof(Chunk), 1, resource->getFile() );

    // read shaders
    if( !chunk.sharedShaders ) for( int i=0; i<chunk.numShaders; i++ )
    {
        assetObjects.insert( Shader::read( resource, assetObjects ) );
    }

    // create geometry
    Geometry* geometry = new Geometry( chunk.numVertices, chunk.numTriangles, chunk.numUVSets, chunk.numShaders, chunk.numPrelights, chunk.sharedShaders, chunk.name );

    // read vertices
    ChunkHeader verticesHeader( resource );
    if( verticesHeader.type != BA_BINARY ) throw Exception( "Unexpected chunk type" );
    if( verticesHeader.size != sizeof(Vector)*chunk.numVertices ) throw Exception( "Incompatible binary asset version" );
    fread( geometry->getVertices(), verticesHeader.size, 1, resource->getFile() );

    // read normals
    ChunkHeader normalsHeader( resource );
    if( normalsHeader.type != BA_BINARY ) throw Exception( "Unexpected chunk type" );
    if( normalsHeader.size != sizeof(Vector)*chunk.numVertices ) throw Exception( "Incompatible binary asset version" );
    fread( geometry->getNormals(), normalsHeader.size, 1, resource->getFile() );

    // read UV-sets
    for( int i=0; i<geometry->getNumUVSets(); i++ )
    {
        ChunkHeader uvsHeader( resource );
        if( uvsHeader.type != BA_BINARY ) throw Exception( "Unexpected chunk type" );
        if( uvsHeader.size != sizeof(Flector)*chunk.numVertices ) throw Exception( "Incompatible binary asset version" );
        fread( geometry->getUVSet(i), uvsHeader.size, 1, resource->getFile() );
    }

    // read prelights
    for( i=0; i<geometry->getNumPrelights(); i++ )
    {
        ChunkHeader prelightsHeader( resource );
        if( prelightsHeader.type != BA_BINARY ) throw Exception( "Unexpected chunk type" );
        if( prelightsHeader.size != sizeof(Color)*chunk.numVertices ) throw Exception( "Incompatible binary asset version" );
        fread( geometry->getPrelights(i), prelightsHeader.size, 1, resource->getFile() );
    }

    // read triangles
    ChunkHeader trianglesHeader( resource );
    if( trianglesHeader.type != BA_BINARY ) throw Exception( "Unexpected chunk type" );
    if( trianglesHeader.size != sizeof(Triangle)*chunk.numTriangles ) throw Exception( "Incompatible binary asset version" );    
    fread( geometry->getTriangles(), trianglesHeader.size, 1, resource->getFile() );

    // read shaders
    if( !chunk.sharedShaders )
    {
        ChunkHeader shadersHeader( resource );
        if( shadersHeader.type != BA_BINARY ) throw Exception( "Unexpected chunk type" );
        if( shadersHeader.size != sizeof(auid)*chunk.numShaders ) throw Exception( "Incompatible binary asset version" );
        auid* shaders = new auid[chunk.numShaders];
        fread( shaders, shadersHeader.size, 1, resource->getFile() );
        for( i=0; i<chunk.numShaders; i++ )
        {
            AssetObjectI assetObjectI = assetObjects.find( shaders[i] );
            if( assetObjectI != assetObjects.end() )
            {
                geometry->setShader( i, reinterpret_cast<Shader*>( assetObjectI->second ) );
            }
        }
        delete[] shaders;
    }

    assetObjects.insert( AssetObjectT( chunk.id, geometry ) );

    // read octree
    if( chunk.numOcTreeSectors )
    {
        for( int i=0; i<chunk.numOcTreeSectors; i++ )
        {
            AssetObjectT assetObjectT = OcTreeSector::read( resource, assetObjects );
            if( i == 0 ) geometry->_ocTreeRoot = reinterpret_cast<OcTreeSector*>( assetObjectT.second );
            assetObjects.insert( assetObjectT );
        }
        assert( geometry->_ocTreeRoot->checkConsistency() );
    }

    // read effect
    if( chunk.hasEffect )
    {
        AssetObjectT assetObjectT = Effect::read( resource, assetObjects );
        assetObjects.insert( assetObjectT );
        geometry->setEffect( reinterpret_cast<Effect*>( assetObjectT.second ) );
    }

    return AssetObjectT( chunk.id, geometry );
}

/**
 * software skinning
 */

Vector* Geometry::getSkinnedVertices(void)
{
    if( !_mesh->pSkinInfo ) return NULL;

    _mesh->getSkinnedVertices( _skinnedVertices );
    return _skinnedVertices;
}

/**
 * captures the geometry data from ID3DXMesh object
 */

void Geometry::captureMeshData(bool captureShaders)
{
    assert( _mesh->OriginalMeshData.Type == D3DXMESHTYPE_MESH );

    // release previous structures
    if( _vertices ) delete[] _vertices;
    if( _normals ) delete[] _normals;
    if( _triangles ) delete[] _triangles;
    for( int i=0; i<engine::maxPrelightLayers; i++ ) 
    {
        if( _prelights[i] ) 
        {
            delete[] _prelights[i];
            _prelights[i] = NULL;
        }
    }
    for( i=0; i<engine::maxTextureLayers; i++ ) 
    {
        if( _uvs[i] ) 
        {
            delete[] _uvs[i];
            _uvs[i] = NULL;
        }
    }
    if( _shaders && captureShaders )
    {
        for( i=0; i<_numShaders; i++ ) _shaders[i]->release();
        delete[] _shaders;
    }
    if( _skinnedVertices ) delete[] _skinnedVertices;

    _numVertices   = _mesh->OriginalMeshData.pMesh->GetNumVertices();
    _numTriangles  = _mesh->OriginalMeshData.pMesh->GetNumFaces();    
    _numShaders    = _mesh->NumMaterials;
    _numPrelights  = 0;
    _numUVSets     = 0;
    _sharedShaders = false;

    // retrieve vertex declaration
    D3DVERTEXELEMENT9 vertexDeclaration[MAX_FVF_DECL_SIZE];
    _dxCR( _mesh->OriginalMeshData.pMesh->GetDeclaration( vertexDeclaration ) );

    // analyze vertex declaration
    _numUVSets    = dxGetNumUVs( vertexDeclaration );
    _numPrelights = dxGetNumPrelights( vertexDeclaration );

    // create device-independent storage
    _vertices  = new Vector[_numVertices];
    _normals   = new Vector[_numVertices];
    _triangles = new Triangle[_numTriangles];
    for( int i=0; i<engine::maxPrelightLayers; i++ ) 
    {
        if( i < _numPrelights ) _prelights[i] = new Color[_numVertices]; 
        else _prelights[i] = NULL;
    }
    for( i=0; i<engine::maxTextureLayers; i++ ) 
    {
        if( i < _numUVSets ) _uvs[i] = new Flector[_numVertices];
        else _uvs[i] = NULL;
    }

    // retrieve geometry data
    char*  vertexData = (char*)( _mesh->lockVertexBuffer( D3DLOCK_READONLY ) );
    char*  indexData  = (char*)( _mesh->lockIndexBuffer( D3DLOCK_READONLY ) );
    DWORD* attrData   = (DWORD*)( _mesh->lockAttributeBuffer( D3DLOCK_READONLY ) );
    int    offset = 0;
    int    j, uvId, prelightId;
    for( i=0; i<_numVertices; i++ )
    {
        j = prelightId = uvId = 0;
        while( j<MAX_FVF_DECL_SIZE && vertexDeclaration[j].Stream != 0xFF )
        {
            switch( vertexDeclaration[j].Usage )
            {
            case D3DDECLUSAGE_POSITION:
                _vertices[i] = *((Vector*)(vertexData+offset));
                break;
            case D3DDECLUSAGE_NORMAL:
                _normals[i] = *((Vector*)(vertexData+offset));
                break;
            case D3DDECLUSAGE_COLOR:
                _prelights[prelightId][i] = *((Color*)(vertexData+offset));
                prelightId++;
                break;
            case D3DDECLUSAGE_TEXCOORD:
                _uvs[uvId][i] = *((Flector*)(vertexData+offset));
                uvId++;
                break;
            case D3DDECLUSAGE_BLENDWEIGHT:
                break;
            case D3DDECLUSAGE_BLENDINDICES:
                break;
            case D3DDECLUSAGE_TANGENT:
            case D3DDECLUSAGE_BINORMAL:
            case D3DDECLUSAGE_TESSFACTOR:
            case D3DDECLUSAGE_POSITIONT:            
            case D3DDECLUSAGE_FOG:
            case D3DDECLUSAGE_DEPTH:
            case D3DDECLUSAGE_SAMPLE:
                break;
            default:
                assert( !"shouldn't be here!" );
            }
            offset += dxGetDeclTypeSize( static_cast<D3DDECLTYPE>( vertexDeclaration[j].Type ) );
            j++;            
        }        
    }    
    offset = 0;
    for( i=0; i<_numTriangles; i++ )
    {
        _triangles[i].set( 
            *(WORD*)(indexData+offset),
            *(WORD*)(indexData+offset+sizeof(WORD)),
            *(WORD*)(indexData+offset+sizeof(WORD)*2),
            attrData[i]
        );
        offset += sizeof(WORD)*3;
    }
    _mesh->unlockVertexBuffer();
    _mesh->unlockIndexBuffer();
    _mesh->unlockAttributeBuffer();

    // recalculate bounding box
    _boundingBox.calculate( _numVertices, _vertices );
    _boundingSphere.calculate( _numVertices, _vertices );

    if( captureShaders )
    {
        _shaders = new Shader*[_numShaders];
        memset( _shaders, 0, sizeof(Shader*) * _numShaders );
        for( i=0; i<_numShaders; i++ )
        {
            Texture* texture = NULL;
            if( _mesh->pMaterials[i].pTextureFilename != NULL )
            {
                TextureI textureI = Texture::textures.find( Texture::getTextureNameFromFilePath( _mesh->pMaterials[i].pTextureFilename ) );
                if( textureI != Texture::textures.end() )
                {
                    texture = textureI->second;
                }
                else
                {
                    texture = Texture::createTexture( _mesh->pMaterials[i].pTextureFilename );
                    texture->setMagFilter( engine::ftLinear );
                    texture->setMinFilter( engine::ftLinear );
                    texture->setMipFilter( engine::ftLinear );
                }
            }
            Shader* shader = new Shader( 
                texture != NULL ? 1 : 0,
                strformat( "%s_shader_%d", _name.c_str(), i ).c_str() 
            );
            if( texture ) shader->setLayerTexture( 0, texture );
            shader->setFlags( shader->getFlags() | engine::sfCaching | engine::sfLighting );
            shader->setDiffuseColor( Vector4f(
                _mesh->pMaterials[i].MatD3D.Diffuse.r,
                _mesh->pMaterials[i].MatD3D.Diffuse.g,
                _mesh->pMaterials[i].MatD3D.Diffuse.b,
                _mesh->pMaterials[i].MatD3D.Diffuse.a
            ) );
            shader->setSpecularColor( Vector4f(
                _mesh->pMaterials[i].MatD3D.Specular.r,
                _mesh->pMaterials[i].MatD3D.Specular.g,
                _mesh->pMaterials[i].MatD3D.Specular.b,
                _mesh->pMaterials[i].MatD3D.Specular.a
            ) );
            shader->setSpecularPower( _mesh->pMaterials[i].MatD3D.Power );
            setShader( i, shader );
        }
    }

    _vertexDeclaration = dxGetVertexDeclaration( _numUVSets, _numPrelights );

    // allocate space for software skinning
    _skinnedVertices = NULL;
    if( _mesh->pSkinInfo ) _skinnedVertices = new Vector[_numVertices];
}

/**
 * edge computation
 */

EdgeHash::EdgeHash()
{
    indices[0] = -1;
    indices[1] = -1;
    vertices   = NULL;
    hash       = 0;
}

EdgeHash::EdgeHash(int idx0, int idx1, Vector* v)
{
    indices[0] = idx0;
    indices[1] = idx1;
    vertices   = v;
    hash = ::hashCode( vertices + indices[0] ) +
           ::hashCode( vertices + indices[1] );
}

bool EdgeHash::operator==(const EdgeHash& rhs)
{
    // return true for any edge that has vertices in equivilant positions
    return ( fuzzyEq( vertices + indices[0], rhs.vertices + rhs.indices[0] ) &&
             fuzzyEq( vertices + indices[1], rhs.vertices + rhs.indices[1] ) ) ||
           ( fuzzyEq( vertices + indices[0], rhs.vertices + rhs.indices[1] ) &&
             fuzzyEq( vertices + indices[1], rhs.vertices + rhs.indices[0] ) );
}

Edge* Geometry::getEdges(void)
{
    if( !_numVertices ) return NULL;
    if( _edges.size() ) return &_edges[0];    

    Table<EdgeHash,int> edgeTable;
    for( int i=0; i<_numTriangles; i++ ) 
    {
        // make sure this is not a degenerate triangle
        // (ie the vertices are so close the triangle is extremely small)
        assert( !fuzzyEq( _vertices + _triangles[i].vertexId[0],  _vertices + _triangles[i].vertexId[1] ) );
        assert( !fuzzyEq( _vertices + _triangles[i].vertexId[1],  _vertices + _triangles[i].vertexId[2] ) );
        assert( !fuzzyEq( _vertices + _triangles[i].vertexId[0],  _vertices + _triangles[i].vertexId[2] ) );

        // add three edges for each face
        addEdge( edgeTable, _edges, _triangles[i].vertexId[0], _triangles[i].vertexId[1], i );
        addEdge( edgeTable, _edges, _triangles[i].vertexId[1], _triangles[i].vertexId[2], i );
        addEdge( edgeTable, _edges, _triangles[i].vertexId[0], _triangles[i].vertexId[2], i );
    }

    return &_edges[0]; 
}

void Geometry::addEdge(Table<EdgeHash,int>& edgeTable, std::vector<Edge>& edgeVector, int v0, int v1, int face)
{    
    EdgeHash edgeKey( v0, v1, _vertices );

    if( edgeTable.containsKey(edgeKey) ) 
    {
        // if this is the second face referencing this edge
        int edgeIndex = edgeTable[edgeKey];
        Edge& existingEdge = edgeVector[edgeIndex];

        // make sure the vertices are wound correctly
        // (ie in opposite directions for the two different faces)
        assert( fuzzyEq( _vertices + existingEdge.vertexId[0], _vertices + v1 ) );
        assert( fuzzyEq( _vertices + existingEdge.vertexId[1], _vertices + v0 ) );

        // make sure this is only the second face to reference this edge
        assert( existingEdge.triangleId[0] >= 0 );
        assert( existingEdge.triangleId[0] != face );
        assert( existingEdge.triangleId[1] == -1 );

        // set second face
        existingEdge.triangleId[1] = face;
    } 
    else 
    {
        // if this is the first face referencing this edge
        Edge newEdge;
        newEdge.vertexId[0] = v0;
        newEdge.vertexId[1] = v1;
        newEdge.triangleId[0] = face;
        newEdge.triangleId[1] = -1;
        // add edge to lookup table and array
        edgeTable.set( edgeKey, edgeVector.size() );
        edgeVector.push_back(newEdge);
    }
}