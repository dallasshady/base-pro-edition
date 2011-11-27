
#include "headers.h"
#include "batch.h"
#include "bsp.h"
#include "asset.h"
#include "camera.h"
#include "collision.h"
#include "wire.h"

//#define RENDER_BATCH_BSP

#define RANDOM_INSTANCE_BRIGHTNESS
#define MIN_INSTANCE_BRIGHTNESS 0.75f
#define MAX_INSTANCE_BRIGHTNESS 1.00f

/**
 * kurieitoru 
 */

engine::IBatch* Engine::createBatch(unsigned int batchSize, engine::BatchScheme* batchScheme)
{
    if( dxDeviceCaps.VertexShaderVersion >= D3DVS_VERSION(3,0) )
    {
        return new HardwareBatch( batchSize, batchScheme );
    }
    else
    {
        return new ShaderBatch( batchSize, batchScheme );
    }
}

/**
 * Batch imupurimenutatsu
 */

ID3DXEffect*    Batch::_effect = NULL;
StaticLostable* Batch::_effectLostable = NULL;
unsigned int    Batch::_numGlobalReferences = 0;

Batch::Batch(unsigned int batchSize, engine::BatchScheme* batchScheme) : _batchSize(batchSize)
{
    assert( batchScheme->numLods );

    // store scheme
    _batchScheme = *batchScheme;

    // create lods
    for( unsigned int i=0; i<_batchScheme.numLods; i++ )
    {
        // downcast geometry
        _lods[i].lodGeometry = dynamic_cast<Geometry*>( _batchScheme.lodGeometry[i] );
        assert( _lods[i].lodGeometry );

        // check geometry
        if( !_lods[i].lodGeometry->getNumShaders() ) throw Exception( 
            "Batch pipeline can handle only single-shaded geometry, while \"%s\" have no shaders!",
            _lods[i].lodGeometry->getName()
        );
        if( _lods[i].lodGeometry->getNumShaders() > 1 ) throw Exception( 
            "Batch pipeline can handle only single-shaded geometry, while \"%s\" have %d different shaders!",
            _lods[i].lodGeometry->getName(),
            _lods[i].lodGeometry->getNumShaders()
        );
        if( !_lods[i].lodGeometry->getNumUVSets() ) throw Exception(
            "Batch pipeline can handle only textured geometries, while \"%s\" have no UVs!",
            _lods[i].lodGeometry->getName()
        );

        // capture geometry
        _lods[i].lodGeometry->_numReferences++;

        // allocate space for LOD instances
        _lods[i].lodMatrices = new Matrix[_batchSize];
        _lods[i].lodColors   = new Quartector[_batchSize];

        // calculate squared distance
        _lods[i].lodSqDistance = _batchScheme.lodDistance[i] * _batchScheme.lodDistance[i];
    }

    // create & reset instance data 
    _matrices = new Matrix[_batchSize];
    _colors   = new Quartector[_batchSize];
    for( unsigned int i=0; i<_batchSize; i++ ) 
    {
        _matrices[i] = identity;  
        #ifdef RANDOM_INSTANCE_BRIGHTNESS
            float brightness = getCore()->getRandToolkit()->getUniform(
                MIN_INSTANCE_BRIGHTNESS,
                MAX_INSTANCE_BRIGHTNESS
            );
            _colors[i] = Quartector( brightness, brightness, brightness, 1.0f );
        #else
            _colors[i] = white;
        #endif
    }

    // load effect
    if( _numGlobalReferences == 0 )
    {
        LPD3DXBUFFER compilationErrors = NULL;
        if( S_OK != D3DXCreateEffectFromFile( 
            iDirect3DDevice, 
            "./res/effects/batch.fx",
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
        _effectLostable = new StaticLostable( onLostDeviceStatic, onResetDeviceStatic );        
    }
    _numGlobalReferences++;

    // no BSP
    _rootSector = NULL;
}

Batch::~Batch()
{
    if( _rootSector ) delete _rootSector;

    // release lods
    for( unsigned int i=0; i<_batchScheme.numLods; i++ )
    {
        _lods[i].lodGeometry->release();
        delete[] _lods[i].lodMatrices;
        delete[] _lods[i].lodColors;
    }

    // release instance data
    delete[] _matrices;

    // release shared effect
    _numGlobalReferences--;
    if( _numGlobalReferences == 0 )
    {
        delete _effectLostable;
        _effectLostable = NULL;
        _effect->Release();
        _effect = NULL;
    }
}

void Batch::onLostDeviceStatic(void)
{
    if( _effect ) _effect->OnLostDevice();
}

void Batch::onResetDeviceStatic(void)
{
    if( _effect ) _effect->OnResetDevice();
}
 
void Batch::release(void)
{    
    delete this;
}

unsigned int Batch::getBatchSize(void)
{
    return _batchSize;
}

unsigned int Batch::getLodSize(unsigned int lodId)
{
    assert( lodId < _batchScheme.numLods );
    return _lods[lodId].lodSize;
}

engine::BatchScheme* Batch::getBatchScheme(void)
{
    return &_batchScheme;
}

Matrix4f Batch::getMatrix(unsigned int batchId)
{
    assert( batchId < _batchSize );
    return wrap( _matrices[batchId] );
}

void Batch::setMatrix(unsigned int batchId, const Matrix4f& matrix)
{
    assert( batchId < _batchSize );
    _matrices[batchId] = wrap( matrix );
}

void Batch::createBatchTree(unsigned int leafSize, const char* resourceName)
{
    if( _rootSector ) delete _rootSector;

    IResource* resource = getCore()->getResource( resourceName, "rb" );
    if( resource )
    {
        _rootSector = new Sector( resource );
        resource->release();
    }
    else
    {
        _rootSector = Sector::createTree( leafSize, _lods[0].lodGeometry, _matrices, _batchSize );
        resource = getCore()->getResource( resourceName, "wb" ); assert( resource );
        _rootSector->write( resource );
        resource->release();
    }
    assert( static_cast<Sector*>(_rootSector)->getNumInstancesInHierarchy() == _batchSize );
}

void Batch::forAllInstancesInAABB(Vector3f aabbInf, Vector3f aabbSup, engine::IBatchCallback callback, void* data)
{
    AABB aabb( wrap( aabbInf ), wrap( aabbSup ) );
    assert( aabb.inf.x < aabb.sup.x );
    assert( aabb.inf.y < aabb.sup.y );
    assert( aabb.inf.z < aabb.sup.z );

    if( _rootSector )
    {
        static_cast<Sector*>(_rootSector)->forAllInstancesInAABB( _lods[0].lodGeometry, _matrices, &aabb, callback, data );
    }
    else
    {
        assert( !"Batch::forAllInstancesInAABB() : method doesn't support such query without of BSP speedup!" );
    }
}

void Batch::updateLODs(void)
{
    unsigned int i,j;
    Vector pos;
    Vector distance;
    float distanceLengthSq;

    // clear LODs
    for( i=0; i<_batchScheme.numLods; i++ ) _lods[i].lodSize = 0;

    // if BSP acceleration is enabled
    if( _rootSector )
    {
        // pass all sectors
        updateLODs( static_cast<Sector*>(_rootSector) );
    }
    // pass all instances
    else for( i=0; i<_batchSize; i++ )
    {
        // calculate instance position
        pos.x = _matrices[i]._41,
        pos.y = _matrices[i]._42,
        pos.z = _matrices[i]._43;
        // calculate squared distance from camera to instace position
        D3DXVec3Subtract( &distance, &pos, &Camera::eyePos );
        distanceLengthSq = D3DXVec3LengthSq( &distance );
        // pass all LODs
        for( j=0; j<_batchScheme.numLods; j++ )
        {            
            if( _lods[j].lodSqDistance > distanceLengthSq )
            {
                _lods[j].lodMatrices[_lods[j].lodSize] = _matrices[i];
                _lods[j].lodColors[_lods[j].lodSize] = _colors[i];
                _lods[j].lodSize++;
                break;
            }
        }
    }
}

void Batch::updateLODs(Sector* sector)
{
    if( ::intersectAABBFrustum( &sector->boundingBox, Camera::frustrum ) )
    {
        // leaf?
        if( sector->left == NULL && sector->right == NULL )
        {
            Vector pos;
            Vector distance;
            float distanceLengthSq;

            // calculate nearest (rough) distance to this sector
            D3DXVec3Subtract( &distance, &sector->boundingSphere.center, &Camera::eyePos );
            float nearestDistance = D3DXVec3Length( &distance ) - sector->boundingSphere.radius;

            // filter too far sectors
            if( nearestDistance < _batchScheme.lodDistance[_batchScheme.numLods-1] )
            {
                // collect instaces
                unsigned int index, instanceId, lodId;
                for( index = 0; index < sector->numIndices; index++ )
                {
                    instanceId = sector->indices[index];
                    // calculate instance position
                    pos.x = _matrices[instanceId]._41,
                    pos.y = _matrices[instanceId]._42,
                    pos.z = _matrices[instanceId]._43;
                    // calculate squared distance from camera to instace position
                    D3DXVec3Subtract( &distance, &pos, &Camera::eyePos );
                    distanceLengthSq = D3DXVec3LengthSq( &distance );
                    // pass all LODs
                    for( lodId=0; lodId<_batchScheme.numLods; lodId++ )
                    {            
                        if( _lods[lodId].lodSqDistance > distanceLengthSq )
                        {
                            _lods[lodId].lodMatrices[_lods[lodId].lodSize] = _matrices[instanceId];
                            _lods[lodId].lodColors[_lods[lodId].lodSize] = _colors[instanceId];
                            _lods[lodId].lodSize++;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            if( sector->left ) updateLODs( static_cast<Sector*>(sector->left) );
            if( sector->right ) updateLODs( static_cast<Sector*>(sector->right) );
        }
    }
}

/**
 * ShaderBatch
 */

#define BATCH_SIZE_VS_2_0 56

// vertex declaration for shader instancing
D3DVERTEXELEMENT9 vertexShaderInstance[] = 
{
    { 0, 0,     D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION,  0 },
    { 0, 3 * 4, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,    0 },
    { 0, 6 * 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  0 },
    { 0, 8 * 4, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD,  1 },
    D3DDECL_END()
};

ShaderBatch::ShaderBatch(unsigned int batchSize, engine::BatchScheme* batchScheme) :
    Batch( batchSize, batchScheme )
{
    // create the vertex declaration that we need
    _dxCR( iDirect3DDevice->CreateVertexDeclaration( vertexShaderInstance, &_vertexDeclaration ) );

    // call onResetDevice() will create and fill buffers with model data
    onResetDevice();
}

ShaderBatch::~ShaderBatch()
{
    _dxCR( _vertexDeclaration->Release() );
    onLostDevice();
}

void ShaderBatch::onLostDevice(void)
{
    for( unsigned int i=0; i<_batchScheme.numLods; i++ )
    {
        _dxCR( _vbModel[i]->Release() );
        _dxCR( _ibModel[i]->Release() );
    } 
}

void ShaderBatch::onResetDevice(void)
{
    for( unsigned int lodId=0; lodId<_batchScheme.numLods; lodId++ )
    {
        unsigned int numVertices  = _lods[lodId].lodGeometry->getNumVertices();
        unsigned int numTriangles = _lods[lodId].lodGeometry->getNumTriangles();
        // build a VB to hold the model data
        _dxCR( iDirect3DDevice->CreateVertexBuffer( 
            BATCH_SIZE_VS_2_0 * numVertices * sizeof(InstanceVertex),
            0,
            0,
            D3DPOOL_MANAGED,
            &_vbModel[lodId],
            0
        ) );
        // build an IB to go with VB
        _dxCR( iDirect3DDevice->CreateIndexBuffer(
            BATCH_SIZE_VS_2_0 * numTriangles * 3 * sizeof( WORD ),
            0,
            D3DFMT_INDEX16,
            D3DPOOL_MANAGED,
            &_ibModel[lodId],
            0
        ) );
        // lock and fill model VB&IB
        InstanceVertex* vertices;
        WORD*           indices;
        _dxCR( _vbModel[lodId]->Lock( 0, NULL, (void**)(&vertices), 0 ) );
        _dxCR( _ibModel[lodId]->Lock( 0, NULL, (void**)(&indices), 0 ) );
        for( int i=0; i<BATCH_SIZE_VS_2_0; i++ )
        {
            for( int j=0; j<_lods[lodId].lodGeometry->getNumVertices(); j++ )
            {
                vertices[i*numVertices+j].pos      = _lods[lodId].lodGeometry->getVertices()[j];
                vertices[i*numVertices+j].normal   = _lods[lodId].lodGeometry->getNormals()[j];
                vertices[i*numVertices+j].uv       = _lods[lodId].lodGeometry->getUVSet(0)[j];
                vertices[i*numVertices+j].instance = float( i );
            }
            for( int j=0; j<_lods[lodId].lodGeometry->getNumTriangles(); j++ )
            {
                indices[i*3*numTriangles+j*3]   = i*numVertices + _lods[lodId].lodGeometry->getTriangles()[j].vertexId[0];
                indices[i*3*numTriangles+j*3+1] = i*numVertices + _lods[lodId].lodGeometry->getTriangles()[j].vertexId[1];
                indices[i*3*numTriangles+j*3+2] = i*numVertices + _lods[lodId].lodGeometry->getTriangles()[j].vertexId[2];
            }
        }
        _dxCR( _ibModel[lodId]->Unlock() );
        _dxCR( _vbModel[lodId]->Unlock() );
    }
}

void ShaderBatch::render(void)
{
    if( _batchScheme.numLods == 1 )
    {
        renderNoLODs();
    }
    else
    {
        updateLODs();
        renderLODs();
    }
    #ifdef RENDER_BATCH_BSP
        if( _rootSector )
        {
            static_cast<Sector*>(_rootSector)->render( _batchScheme.lodDistance[_batchScheme.numLods-1] );
        }
    #endif
}

void ShaderBatch::renderNoLODs(void)
{
    // apply shader
    _lods[0].lodGeometry->shader(0)->apply();

    // camera properties
    _effect->SetVector( "cameraPos", &Quartector( Camera::eyePos.x, Camera::eyePos.y, Camera::eyePos.z, 1.0f ) );
    _effect->SetVector( "cameraDir", &Quartector( Camera::eyeDirection.x, Camera::eyeDirection.y, Camera::eyeDirection.z, 0.0f ) );

    // view-projection matrix
    Matrix tempMatrix;
    D3DXMatrixMultiply( &tempMatrix, &Camera::viewMatrix, &Camera::projectionMatrix );
    _effect->SetMatrix( "viewProj", &tempMatrix );

    // ambient lighting
    _effect->SetVector( "ambientColor", (D3DXVECTOR4*)( Shader::globalAmbient() ) );        

    // dynamic lighting
    D3DLIGHT9 lightProperties;
    BOOL      lightEnabled; 
    for( unsigned int lightId=0; lightId<8; lightId++ )
    {        
        // check light is enabled
        _dxCR( iDirect3DDevice->GetLightEnable( lightId, &lightEnabled ) );
        if( !lightEnabled ) continue;
        // retrieve light properties
        _dxCR( iDirect3DDevice->GetLight( lightId, &lightProperties ) );
        if( lightProperties.Type == D3DLIGHT_POINT )
        {
            // setup effect
            _effect->SetVector( "lightPos", &Quartector( lightProperties.Position.x, lightProperties.Position.y, lightProperties.Position.z, 1.0f ) );
            _effect->SetVector( "lightColor", &Quartector( lightProperties.Diffuse.r, lightProperties.Diffuse.g, lightProperties.Diffuse.b, lightProperties.Diffuse.a) );
            break;
        }
    }

    // material properties
    D3DMATERIAL9 material;
    iDirect3DDevice->GetMaterial( &material );
    _effect->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
    _effect->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
    _effect->SetFloat( "materialPower", material.Power );

    // set declaration
    _dxCR( iDirect3DDevice->SetVertexDeclaration( _vertexDeclaration ) );

    // stream zero is our model, and its frequency is how we communicate 
    // the number of instances required, which in this case is the size of batch
    _dxCR( iDirect3DDevice->SetStreamSource( 0, _vbModel[0], 0, sizeof(InstanceVertex)) );
    _dxCR( iDirect3DDevice->SetIndices( _ibModel[0] ) );

    // set technique
    _dxCR( _effect->SetTechnique( _effect->GetTechniqueByName( "ShaderBatch" ) ) );

    // render batch using HLSL effect
    UINT iPass, cPasses;
    _dxCR( _effect->Begin( &cPasses, 0 ) );
    for( iPass = 0; iPass < cPasses; iPass++ )
    {
        _dxCR( _effect->BeginPass( iPass ) );
        
        int numRenderInstances = 0;
        int numRemainingInstances = _batchSize;
        while( numRemainingInstances > 0 )
        {
	        // determine how many instances are in this batch (up to g_nNumBatchInstance)
            numRenderInstances = min( numRemainingInstances, BATCH_SIZE_VS_2_0 );

            // set the box instancing array
            _dxCR( _effect->SetMatrixArray( "worldInstance", _matrices + _batchSize - numRemainingInstances, numRenderInstances ) );
            
            // The effect interface queues up the changes and performs them 
            // with the CommitChanges call. You do not need to call CommitChanges if 
            // you are not setting any parameters between the BeginPass and EndPass.
            _dxCR( _effect->CommitChanges() );
            
            _dxCR( iDirect3DDevice->DrawIndexedPrimitive( 
                D3DPT_TRIANGLELIST, 0, 0, 
                numRenderInstances * _lods[0].lodGeometry->getNumVertices(), 
                0, 
                numRenderInstances * _lods[0].lodGeometry->getNumTriangles() 
            ) );

            // subtract the rendered boxes from the remaining boxes
            numRemainingInstances -= numRenderInstances;            
        }

        _dxCR( _effect->EndPass() );
    }
    _dxCR( _effect->End() );
    
    _dxCR( iDirect3DDevice->SetVertexShader(NULL) );
    _dxCR( iDirect3DDevice->SetPixelShader(NULL) );
}

void ShaderBatch::renderLODs(void)
{
    // common properties for effect
    // camera properties
    _effect->SetVector( "cameraPos", &Quartector( Camera::eyePos.x, Camera::eyePos.y, Camera::eyePos.z, 1.0f ) );
    _effect->SetVector( "cameraDir", &Quartector( Camera::eyeDirection.x, Camera::eyeDirection.y, Camera::eyeDirection.z, 0.0f ) );
    // view-projection matrix
    Matrix tempMatrix;
    D3DXMatrixMultiply( &tempMatrix, &Camera::viewMatrix, &Camera::projectionMatrix );
    _effect->SetMatrix( "viewProj", &tempMatrix );
    // ambient lighting
    _effect->SetVector( "ambientColor", (D3DXVECTOR4*)( Shader::globalAmbient() ) );        
    // dynamic lighting
    D3DLIGHT9 lightProperties;
    BOOL      lightEnabled; 
    for( unsigned int lightId=0; lightId<8; lightId++ )
    {        
        // check light is enabled
        _dxCR( iDirect3DDevice->GetLightEnable( lightId, &lightEnabled ) );
        if( !lightEnabled ) continue;
        // retrieve light properties
        _dxCR( iDirect3DDevice->GetLight( lightId, &lightProperties ) );
        if( lightProperties.Type == D3DLIGHT_POINT )
        {
            // setup effect
            _effect->SetVector( "lightPos", &Quartector( lightProperties.Position.x, lightProperties.Position.y, lightProperties.Position.z, 1.0f ) );
            _effect->SetVector( "lightColor", &Quartector( lightProperties.Diffuse.r, lightProperties.Diffuse.g, lightProperties.Diffuse.b, lightProperties.Diffuse.a) );
            break;
        }
    }
    // declaration
    _dxCR( iDirect3DDevice->SetVertexDeclaration( _vertexDeclaration ) );
    // technique
    _dxCR( _effect->SetTechnique( _effect->GetTechniqueByName( "ShaderBatch" ) ) );

    // render LODS
    for( unsigned int lodId=0; lodId<_batchScheme.numLods; lodId++ )
    {
        // apply shader
        _lods[lodId].lodGeometry->shader(0)->apply();

        // material properties
        D3DMATERIAL9 material;
        iDirect3DDevice->GetMaterial( &material );
        _effect->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
        _effect->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
        _effect->SetFloat( "materialPower", material.Power );

        // stream zero is our model, and its frequency is how we communicate 
        // the number of instances required, which in this case is the size of batch
        _dxCR( iDirect3DDevice->SetStreamSource( 0, _vbModel[lodId], 0, sizeof(InstanceVertex)) );
        _dxCR( iDirect3DDevice->SetIndices( _ibModel[lodId] ) );

        // render batch LOD using HLSL effect
        UINT iPass, cPasses;
        _dxCR( _effect->Begin( &cPasses, 0 ) );
        for( iPass = 0; iPass < cPasses; iPass++ )
        {
            _dxCR( _effect->BeginPass( iPass ) );
                    
            int numRenderInstances = 0;
            int numRemainingInstances = _lods[lodId].lodSize;
            int batchSize = _lods[lodId].lodSize;
            while( numRemainingInstances > 0 )
            {
                // determine how many instances are in this batch (up to g_nNumBatchInstance)
                numRenderInstances = min( numRemainingInstances, BATCH_SIZE_VS_2_0 );

                // set the instancing array
                _dxCR( _effect->SetMatrixArray( "worldInstance", _lods[lodId].lodMatrices + batchSize - numRemainingInstances, numRenderInstances ) );
            
                // The effect interface queues up the changes and performs them 
                // with the CommitChanges call. You do not need to call CommitChanges if 
                // you are not setting any parameters between the BeginPass and EndPass.
                _dxCR( _effect->CommitChanges() );
            
                _dxCR( iDirect3DDevice->DrawIndexedPrimitive( 
                    D3DPT_TRIANGLELIST, 0, 0, 
                    numRenderInstances * _lods[lodId].lodGeometry->getNumVertices(), 
                    0, 
                    numRenderInstances * _lods[lodId].lodGeometry->getNumTriangles() 
                ) );

                // subtract the rendered boxes from the remaining boxes
                numRemainingInstances -= numRenderInstances;            
            }
            _dxCR( _effect->EndPass() );
        }
        _dxCR( _effect->End() );
    }

    // set no effect
    _dxCR( iDirect3DDevice->SetVertexShader(NULL) );
    _dxCR( iDirect3DDevice->SetPixelShader(NULL) );
}


/**
 * implementation of hardware batch
 */

#define MAX_VB_INSTANCE_SIZE 1024

// vertex declaration for hardware instancing
D3DVERTEXELEMENT9 vertexHWInstance[] =
{
    { 0, 0,      D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_POSITION, 0 }, // geometry: untransformed position
    { 0, 3 * 4,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_NORMAL,   0 }, // geometry: untransformed normal
    { 0, 6 * 4,  D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 0 }, // geometry: texture coordinates
    { 1, 0,      D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 1 }, // instance: x-transformation vector
    { 1, 4  * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 2 }, // instance: y-transformation vector
    { 1, 8  * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 3 }, // instance: z-transformation vector
    { 1, 12 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_TEXCOORD, 4 }, // instance: p-transformation vector
    { 1, 16 * 4, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT,  D3DDECLUSAGE_COLOR,    0 }, // instance color
    D3DDECL_END()
};

HardwareBatch::HardwareBatch(unsigned int batchSize, engine::BatchScheme* batchScheme) :
    Batch( batchSize, batchScheme )
{
    // create the vertex declaration we need
    _dxCR( iDirect3DDevice->CreateVertexDeclaration( vertexHWInstance, &_vertexDeclaration ) );

    // select size of instance buffer (can't be too large due to perfomance penalty)
    _vbInstanceSize = batchSize < MAX_VB_INSTANCE_SIZE ? batchSize : MAX_VB_INSTANCE_SIZE;

    // create buffers
    onResetDevice();
}

HardwareBatch::~HardwareBatch()
{    
    onLostDevice();
    _dxCR( _vertexDeclaration->Release() );
}

void HardwareBatch::onLostDevice(void)
{    
    _dxCR( _vbModel->Release() ); 
    _dxCR( _ibModel->Release() );
    _dxCR( _vbInstance->Release() );
}

void HardwareBatch::onResetDevice(void)
{
    // determine the buffer size we need
    unsigned int numSharedVertices  = 0;
    unsigned int numSharedTriangles = 0;    
    for( unsigned int lodId=0; lodId<_batchScheme.numLods; lodId++ )
    {
        numSharedVertices  += _lods[lodId].lodGeometry->getNumVertices();
        numSharedTriangles += _lods[lodId].lodGeometry->getNumTriangles();
    }

    // build a VB to hold the model data
    _dxCR( iDirect3DDevice->CreateVertexBuffer(
        numSharedVertices * sizeof(InstanceVertex),
        0,
        0,
        D3DPOOL_MANAGED,
        &_vbModel,
        0
    ) );
    // build an IB to go with VB
    _dxCR( iDirect3DDevice->CreateIndexBuffer(
        numSharedTriangles * 3 * sizeof( WORD ),
        0,
        D3DFMT_INDEX16,
        D3DPOOL_MANAGED,
        &_ibModel,
        0
    ) );

    // lock and fill model VB&IB
    InstanceVertex* vertices;
    WORD*           indices;
    _dxCR( _vbModel->Lock( 0, NULL, (void**)(&vertices), 0 ) );
    _dxCR( _ibModel->Lock( 0, NULL, (void**)(&indices), 0 ) );   
    unsigned int currStartVertex = 0;
    unsigned int currStartIndex = 0;
    for( unsigned int lodId=0; lodId<_batchScheme.numLods; lodId++ )
    {
        _startVertex[lodId] = currStartVertex;
        _startIndex[lodId] = currStartIndex;
        for( int i=0; i<_lods[lodId].lodGeometry->getNumVertices(); i++ )
        {
            vertices[currStartVertex+i].pos    = _lods[lodId].lodGeometry->getVertices()[i];
            vertices[currStartVertex+i].normal = _lods[lodId].lodGeometry->getNormals()[i];
            vertices[currStartVertex+i].uv     = _lods[lodId].lodGeometry->getUVSet(0)[i];
        }
        for( int i=0; i<_lods[lodId].lodGeometry->getNumTriangles(); i++ )
        {
            indices[currStartIndex+i*3]   = currStartVertex + _lods[lodId].lodGeometry->getTriangles()[i].vertexId[0];
            indices[currStartIndex+i*3+1] = currStartVertex + _lods[lodId].lodGeometry->getTriangles()[i].vertexId[1];
            indices[currStartIndex+i*3+2] = currStartVertex + _lods[lodId].lodGeometry->getTriangles()[i].vertexId[2];
        }
        currStartVertex += _lods[lodId].lodGeometry->getNumVertices();
        currStartIndex += _lods[lodId].lodGeometry->getNumTriangles() * 3;
    }
    _dxCR( _ibModel->Unlock() );
    _dxCR( _vbModel->Unlock() );

    // build an VB for instancing data
    _dxCR( iDirect3DDevice->CreateVertexBuffer( 
        _vbInstanceSize * sizeof( Instance ),
        0,
        0,
        D3DPOOL_MANAGED, &_vbInstance,
        0
    ) );
}

void HardwareBatch::render(void)
{
    if( _batchScheme.numLods == 1 )
    {
        renderNoLODs();
    }
    else
    {        
        updateLODs();
        renderLODs();
    }
    #ifdef RENDER_BATCH_BSP
        if( _rootSector )
        {
            static_cast<Sector*>(_rootSector)->render( _batchScheme.lodDistance[_batchScheme.numLods-1] );
        }
    #endif
}

void HardwareBatch::renderNoLODs(void)
{
    // apply shader
    _lods[0].lodGeometry->shader(0)->apply();

    // camera properties
    _effect->SetVector( "cameraPos", &Quartector( Camera::eyePos.x, Camera::eyePos.y, Camera::eyePos.z, 1.0f ) );
    _effect->SetVector( "cameraDir", &Quartector( Camera::eyeDirection.x, Camera::eyeDirection.y, Camera::eyeDirection.z, 0.0f ) );

    // view-projection matrix
    Matrix tempMatrix;
    D3DXMatrixMultiply( &tempMatrix, &Camera::viewMatrix, &Camera::projectionMatrix );
    _effect->SetMatrix( "viewProj", &tempMatrix );

    // ambient lighting
    _effect->SetVector( "ambientColor", (D3DXVECTOR4*)( Shader::globalAmbient() ) );

    // dynamic lighting
    D3DLIGHT9 lightProperties;
    BOOL      lightEnabled;
    for( unsigned int lightId=0; lightId<8; lightId++ )
    {
        // check light is enabled
        _dxCR( iDirect3DDevice->GetLightEnable( lightId, &lightEnabled ) );
        if( !lightEnabled ) continue;
        // retrieve light properties
        _dxCR( iDirect3DDevice->GetLight( lightId, &lightProperties ) );
        if( lightProperties.Type == D3DLIGHT_POINT )
        {
            // setup effect
            _effect->SetVector( "lightPos", &Quartector( lightProperties.Position.x, lightProperties.Position.y, lightProperties.Position.z, 1.0f ) );
            _effect->SetVector( "lightColor", &Quartector( lightProperties.Diffuse.r, lightProperties.Diffuse.g, lightProperties.Diffuse.b, lightProperties.Diffuse.a) );
            break;
        }
    }

    // material properties
    D3DMATERIAL9 material;
    iDirect3DDevice->GetMaterial( &material );
    _effect->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
    _effect->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
    _effect->SetFloat( "materialPower", material.Power );

    // set declaration
    _dxCR( iDirect3DDevice->SetVertexDeclaration( _vertexDeclaration ) );

    // set technique
    _dxCR( _effect->SetTechnique( _effect->GetTechniqueByName( "HardwareBatch" ) ) );

    // render batch using HLSL effect    
    UINT iPass, cPasses;
    Instance* instance;
    _dxCR( _effect->Begin( &cPasses, 0 ) );
    for( iPass = 0; iPass < cPasses; iPass++ )
    {
        _dxCR( _effect->BeginPass( iPass ) );
        
        int iInst;
        int numRenderInstances = 0;
        int numRemainingInstances = _batchSize;
        while( numRemainingInstances > 0 )
        {
	        // determine how many instances are in this batch (up to g_nNumBatchInstance)
            numRenderInstances = min( numRemainingInstances, int(_vbInstanceSize) );

            // set shader buffers
            _dxCR( iDirect3DDevice->SetStreamSource( 0, _vbModel, 0, sizeof(InstanceVertex)) );
            _dxCR( iDirect3DDevice->SetIndices( _ibModel ) );

            // stream zero is our model, and its frequency is how we communicate
            // the number of instances required, which in this case is the size of batch        
            _dxCR( iDirect3DDevice->SetStreamSourceFreq( 0, D3DSTREAMSOURCE_INDEXEDDATA | numRenderInstances ) );

            // stream one is the instancing buffer, so this advances to the next value
            // after each instance has been drawn, so the divider is 1.
            _dxCR( iDirect3DDevice->SetStreamSource( 1, _vbInstance, 0, sizeof( Instance ) ) );
            _dxCR( iDirect3DDevice->SetStreamSourceFreq( 1, D3DSTREAMSOURCE_INSTANCEDATA | 1ul ) );
            // lock and fill instance buffer    
            _dxCR( _vbInstance->Lock( 0, NULL,  (void**)(&instance), 0 ) );
            for( iInst=0; iInst<numRenderInstances; iInst++ )
            {
                memcpy( &instance[iInst].matrix, _matrices + _batchSize - numRemainingInstances + iInst, sizeof(Matrix) );
                memcpy( &instance[iInst].color, _colors + _batchSize - numRemainingInstances + iInst, sizeof(Quartector) );
                //memcpy( instanceData, _matrices + _batchSize - numRemainingInstances, sizeof(Matrix)*numRenderInstances );
            }            
            _dxCR( _vbInstance->Unlock() );
            
            // draw primitives
            _dxCR( iDirect3DDevice->DrawIndexedPrimitive( 
                D3DPT_TRIANGLELIST, 0, 0, 
                _lods[0].lodGeometry->getNumVertices(), 
                0, 
                _lods[0].lodGeometry->getNumTriangles() 
            ) );

            // subtract the rendered boxes from the remaining boxes
            numRemainingInstances -= numRenderInstances;            
        }

        _dxCR( _effect->EndPass() );
    }
    _dxCR( _effect->End() );

    _dxCR( iDirect3DDevice->SetStreamSourceFreq( 0, 1 ) );
    _dxCR( iDirect3DDevice->SetStreamSourceFreq( 1, 1 ) );
    _dxCR( iDirect3DDevice->SetVertexShader(NULL) );
    _dxCR( iDirect3DDevice->SetPixelShader(NULL) );
}

void HardwareBatch::renderLODs(void)
{
    // apply shader
    _lods[0].lodGeometry->shader(0)->apply();

    // camera properties
    _effect->SetVector( "cameraPos", &Quartector( Camera::eyePos.x, Camera::eyePos.y, Camera::eyePos.z, 1.0f ) );
    _effect->SetVector( "cameraDir", &Quartector( Camera::eyeDirection.x, Camera::eyeDirection.y, Camera::eyeDirection.z, 0.0f ) );

    // view-projection matrix
    Matrix tempMatrix;
    D3DXMatrixMultiply( &tempMatrix, &Camera::viewMatrix, &Camera::projectionMatrix );
    _effect->SetMatrix( "viewProj", &tempMatrix );

    // ambient lighting
    _effect->SetVector( "ambientColor", (D3DXVECTOR4*)( Shader::globalAmbient() ) );

    // dynamic lighting
    D3DLIGHT9 lightProperties;
    BOOL      lightEnabled;
    for( unsigned int lightId=0; lightId<8; lightId++ )
    {
        // check light is enabled
        _dxCR( iDirect3DDevice->GetLightEnable( lightId, &lightEnabled ) );
        if( !lightEnabled ) continue;
        // retrieve light properties
        _dxCR( iDirect3DDevice->GetLight( lightId, &lightProperties ) );
        if( lightProperties.Type == D3DLIGHT_POINT )
        {
            // setup effect
            _effect->SetVector( "lightPos", &Quartector( lightProperties.Position.x, lightProperties.Position.y, lightProperties.Position.z, 1.0f ) );
            _effect->SetVector( "lightColor", &Quartector( lightProperties.Diffuse.r, lightProperties.Diffuse.g, lightProperties.Diffuse.b, lightProperties.Diffuse.a) );
            break;
        }
    }

    // material properties
    D3DMATERIAL9 material;
    iDirect3DDevice->GetMaterial( &material );
    _effect->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
    _effect->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
    _effect->SetFloat( "materialPower", material.Power );

    // set declaration
    _dxCR( iDirect3DDevice->SetVertexDeclaration( _vertexDeclaration ) );

    // pass LODs
    for( unsigned int lodId=0; lodId<_batchScheme.numLods; lodId++ ) 
    {        
        // set technique
        _dxCR( _effect->SetTechnique( _effect->GetTechniqueByName( "HardwareBatch" ) ) );

        // render batch LOD using HLSL effect
        UINT iPass, cPasses;
        Instance* instance;
        _dxCR( _effect->Begin( &cPasses, 0 ) );
        for( iPass = 0; iPass < cPasses; iPass++ )
        {
            _dxCR( _effect->BeginPass( iPass ) );
                
            int iInst;
            int numRenderInstances = 0;
            int numRemainingInstances = _lods[lodId].lodSize;
            int batchSize = _lods[lodId].lodSize;
            while( numRemainingInstances > 0 )
            {
                // determine how many instances are in this batch (up to g_nNumBatchInstance)
                numRenderInstances = min( numRemainingInstances, int(_vbInstanceSize) );

                // set shader buffers
                _dxCR( iDirect3DDevice->SetStreamSource( 0, _vbModel, 0, sizeof(InstanceVertex)) );
                _dxCR( iDirect3DDevice->SetIndices( _ibModel ) );

                // stream zero is our model, and its frequency is how we communicate
                // the number of instances required, which in this case is the size of batch        
                _dxCR( iDirect3DDevice->SetStreamSourceFreq( 0, D3DSTREAMSOURCE_INDEXEDDATA | numRenderInstances ) );

                // stream one is the instancing buffer, so this advances to the next value
                // after each instance has been drawn, so the divider is 1.
                _dxCR( iDirect3DDevice->SetStreamSource( 1, _vbInstance, 0, sizeof( Instance ) ) );
                _dxCR( iDirect3DDevice->SetStreamSourceFreq( 1, D3DSTREAMSOURCE_INSTANCEDATA | 1ul ) );

                // lock and fill instance buffer    
                _dxCR( _vbInstance->Lock( 0, NULL,  (void**)(&instance), 0 ) );
                for( iInst=0; iInst<numRenderInstances; iInst++ )
                {
                    memcpy( &instance[iInst].matrix, _lods[lodId].lodMatrices + batchSize - numRemainingInstances + iInst, sizeof(Matrix) );
                    memcpy( &instance[iInst].color, _lods[lodId].lodColors + batchSize - numRemainingInstances + iInst, sizeof(Quartector) );
                    //memcpy( instanceData, _lods[lodId].lodMatrices + batchSize - numRemainingInstances, sizeof(Matrix)*numRenderInstances );
                }
                _dxCR( _vbInstance->Unlock() );
                       
                _dxCR( iDirect3DDevice->DrawIndexedPrimitive( 
                    D3DPT_TRIANGLELIST, 0, 
                    _startVertex[lodId], 
                    _lods[lodId].lodGeometry->getNumVertices(), 
                    _startIndex[lodId], 
                    _lods[lodId].lodGeometry->getNumTriangles() 
                ) );

                // subtract the rendered boxes from the remaining boxes
                numRemainingInstances -= numRenderInstances;            
            }
            _dxCR( _effect->EndPass() );
        }
        _dxCR( _effect->End() );
    }
    _dxCR( iDirect3DDevice->SetStreamSourceFreq( 0, 1 ) );
    _dxCR( iDirect3DDevice->SetStreamSourceFreq( 1, 1 ) );
    _dxCR( iDirect3DDevice->SetVertexShader(NULL) );
    _dxCR( iDirect3DDevice->SetPixelShader(NULL) );
}