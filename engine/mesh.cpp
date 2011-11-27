
#include "headers.h"
#include "mesh.h"
#include "camera.h"
#include "light.h"
#include "../common/istring.h"
#include "hlsl.h"
#include "vertexdeclaration.h"
#include "atomic.h"

D3DXMATRIX** Mesh::pBoneMatrices = NULL;

ID3DXEffect*    Mesh::_effect = NULL;
ID3DXEffect*    Mesh::_effectx = NULL;
int             Mesh::_bonePaletteSize = NULL;
int             Mesh::_lightPaletteSize = NULL;
D3DXMATRIXA16*  Mesh::_pBoneMatrices = NULL;
StaticLostable* Mesh::_effectLostable = NULL;
DWORD           Mesh::_sizeOfSkinnedVertices = 0;
PBYTE           Mesh::_skinnedVertices = NULL;

/**
 * initialization & etc
 */

void Mesh::init(void)
{
    // setup mesh rendering
    TiXmlElement* skinnedMesh = Engine::instance->getConfigElement( "skinnedMesh" ); assert( skinnedMesh );
    skinnedMesh->Attribute( "bonePalette", &_bonePaletteSize );
    skinnedMesh->Attribute( "lightPalette", &_lightPaletteSize );
    _pBoneMatrices    = new D3DXMATRIXA16[_bonePaletteSize];

    // target vertex shader version
    // DWORD hiVersion = ( dxDeviceCaps.VertexShaderVersion & 0x0000FF00 ) >> 8;
    // DWORD loVersion = ( dxDeviceCaps.VertexShaderVersion & 0x000000FF );
    std::string vsVersion = "vs_2_0"; // strformat( "vs_%d_%d", hiVersion, loVersion );
    std::string psVersion = "ps_2_0";    

    // ---------------------------------------------------------------------------------
    // generate effect №1
    std::string effectCode = "\n";
    effectCode += strformat( "#define FX_LIGHT_PALETTE %d\n", _lightPaletteSize );
    effectCode += strformat( "#define FX_BONE_PALETTE %d\n", _bonePaletteSize );

    IResource* resource = getCore()->getResource( "./res/effects/skinnedmesh.fxt", "rb" );
    assert( resource );
    fseek( resource->getFile(), 0, SEEK_END );
    int fileSize = ftell( resource->getFile() );
    fseek( resource->getFile(), 0, SEEK_SET );

    char* buffer = new char[fileSize+1];
    memset( buffer, 0, fileSize+1 );
    fread( buffer, fileSize, 1, resource->getFile() );
    effectCode += buffer;
    delete[] buffer;
    resource->release();

    effectCode += strformat( "VertexShader vsVariant[%d] = {\n", 4 * ( _lightPaletteSize + 1 ) );
    bool commaMode = false;
    for( int i=0; i<=_lightPaletteSize; i++ )
    {       
        for( int j=1; j<=4; j++ )
        {
            if( !commaMode ) commaMode = true; else effectCode += ",\n";
            effectCode += strformat( "    compile %s vsSkinnedMesh(%d,%d)", vsVersion.c_str(), j, i );
        }
    }
    effectCode += "\n};\n\n";

    effectCode += "technique t0\n";
    effectCode += "{\n";
    effectCode += "    pass p0\n";
    effectCode += "    {\n";
    effectCode += "        VertexShader = ( vsVariant[numLights*4+numBones] );\n";
    effectCode += "    }\n";
    effectCode += "}\n";
   
    _effect = ::dxCompileEffect( effectCode.c_str() ); assert( _effect );

    // ---------------------------------------------------------------------------------
    // generate effect №2
    effectCode = "\n";
    effectCode += strformat( "#define FX_BONE_PALETTE %d\n", _bonePaletteSize );

    resource = getCore()->getResource( "./res/effects/skinnedmeshx.fxt", "rb" ); assert( resource );    
    fseek( resource->getFile(), 0, SEEK_END );
    fileSize = ftell( resource->getFile() );
    fseek( resource->getFile(), 0, SEEK_SET );

    buffer = new char[fileSize+1];
    memset( buffer, 0, fileSize+1 );
    fread( buffer, fileSize, 1, resource->getFile() );
    effectCode += buffer;
    delete[] buffer;
    resource->release();

    effectCode += "VertexShader vsVariant[4] = {\n";
    commaMode = false;
    for( i=1; i<=4; i++ )
    {       
        if( !commaMode ) 
        {
            commaMode = true; 
        }
        else 
        {
            effectCode += ",\n";
        }
        effectCode += strformat( "    compile %s vsSkinnedMeshX(%d)", vsVersion.c_str(), i );
    }
    effectCode += "\n};\n\n";

    effectCode += "technique t0\n";
    effectCode += "{\n";
    effectCode += "    pass p0\n";
    effectCode += "    {\n";
    effectCode += "        VertexShader = ( vsVariant[numBones] );\n";
    effectCode += "        PixelShader = compile ps_2_0 psSkinnedMeshX();\n";
    effectCode += "    }\n";
    effectCode += "}\n";
   
    _effectx = ::dxCompileEffect( effectCode.c_str() ); assert( _effectx );

    /*
    FILE* f = fopen( "skinnedmeshx.fx", "wt" );
    fwrite( effectCode.c_str(), effectCode.length(), 1, f );
    fclose( f );
    */

    _effectLostable = new StaticLostable( onLostDevice, onResetDevice );
}

void Mesh::term(void)
{
    if( _effect ) _effect->Release();
    if( _effectx ) _effectx->Release();
    if( _pBoneMatrices ) delete[] _pBoneMatrices;
    if( _effectLostable ) delete _effectLostable;
    if( _skinnedVertices ) delete[] _skinnedVertices;
}

/**
 * construction
 */

Mesh::Mesh(DWORD numVertices, 
           DWORD numTriangles, 
           DWORD numAttributes,
           DWORD meshOptions, 
           D3DVERTEXELEMENT9* vertexDeclaration)
{
    // initialize D3DXMESHCONTAINER
    Name                   = NULL;
    pMaterials             = NULL;
    pEffects               = NULL;
    NumMaterials           = 0;
    pAdjacency             = NULL;
    pSkinInfo              = NULL;
    pNextMeshContainer     = NULL;    
    pSoftwareBones         = NULL;

    // we event do not use the default container field
    // this is because of refactoring of Mesh class
    MeshData.Type  = D3DXMESHTYPE_FORCE_DWORD;
    MeshData.pMesh = NULL;

    // this variant of mesh has no skinned mesh
    SkinnedMeshData.Type  = D3DXMESHTYPE_FORCE_DWORD;
    SkinnedMeshData.pMesh = NULL;

    // create generic un-skinned mesh
    OriginalMeshData.Type = D3DXMESHTYPE_MESH;
    OriginalMeshData.pMesh = NULL;
    _dxCR( D3DXCreateMesh(
        numTriangles,
        numVertices,
        meshOptions,
        vertexDeclaration,
        iDirect3DDevice,
        &OriginalMeshData.pMesh
    ) );
    NumAttributeGroups = numAttributes;

    NumInfluences = 0;
    pBoneCombination = NULL;
    pBoneOffsetMatrices = NULL;
    NumPaletteEntries = 0;    
    SoftwareAttributeId = 0;
    UseSoftwareVP = false;
}

Mesh::Mesh(LPCTSTR meshName,
           CONST D3DXMESHDATA* meshData,
		   CONST D3DXMATERIAL* meshMaterials,
           DWORD numMeshMaterials, 
           CONST DWORD *adjacency, 
           LPD3DXSKININFO skinInfo)
{
    // can't use progressive nor patch meshes
    assert( meshData->Type == D3DXMESHTYPE_MESH );

    // initialize D3DXMESHCONTAINER    
    pEffects           = NULL;
    pNextMeshContainer = NULL;
    pBoneCombination   = NULL;
    UseSoftwareVP      = false;
    pSoftwareBones     = NULL;

    // we event do not use the default container field
    // this is because of refactoring of Mesh class
    MeshData.Type  = D3DXMESHTYPE_FORCE_DWORD;
    MeshData.pMesh = NULL;

    // reset skinned mesh fields
    SkinnedMeshData.Type  = D3DXMESHTYPE_FORCE_DWORD;
    SkinnedMeshData.pMesh = NULL;

    Name = NULL;
    if( meshName ) 
    {
        Name = new char[ strlen(meshName) + 1 ];
        strcpy( Name, meshName );
    }

    // copy materials information
    NumMaterials = numMeshMaterials;
    pMaterials = NULL;
    if( NumMaterials )
    {
        pMaterials = new D3DXMATERIAL[NumMaterials];
        for( unsigned int i=0; i<NumMaterials; i++ )
        {
            pMaterials[i].MatD3D = meshMaterials[i].MatD3D;
            if( meshMaterials[i].pTextureFilename )
            {
                pMaterials[i].pTextureFilename = new char[ strlen( meshMaterials[i].pTextureFilename ) + 1 ];
                strcpy( pMaterials[i].pTextureFilename, meshMaterials[i].pTextureFilename );
            }
            else
            {
                pMaterials[i].pTextureFilename = NULL;
            }
        }
    }

    // copy ajacency information
    pAdjacency = new DWORD[ meshData->pMesh->GetNumFaces()*3 ];
    memcpy( pAdjacency, adjacency, sizeof(DWORD) * meshData->pMesh->GetNumFaces()*3 );

    // if there is skinning information, save off the required data and then setup 
    // for HW skinning
    if( skinInfo != NULL )
    {
        // first save off the SkinInfo and original mesh data
        pSkinInfo = skinInfo;
        pSkinInfo->AddRef();

        // store original mesh        
        OriginalMeshData = *meshData;
        OriginalMeshData.pMesh->AddRef();

        // will need an array of offset matrices to move the vertices from the figure 
        // space to the bone's space
        unsigned int numBones = pSkinInfo->GetNumBones();
        pBoneOffsetMatrices = new D3DXMATRIX[numBones];
        assert( pBoneOffsetMatrices );

        // will need an array for software skinning
        pSoftwareBones = new MatrixA16[numBones];

        // get each of the bone offset matrices so that we don't need to get them later
        for( unsigned int i=0; i<numBones; i++ ) pBoneOffsetMatrices[i] = *( pSkinInfo->GetBoneOffsetMatrix( i ) );

        // subj.
        generateSkinnedMesh();
    }
    else
    {        
        pSkinInfo              = NULL;
        OriginalMeshData       = *meshData;
        NumInfluences          = 0;
        pBoneCombination       = NULL;
        pBoneOffsetMatrices    = NULL;
        NumPaletteEntries      = 0;
        NumAttributeGroups     = NumMaterials;
        SoftwareAttributeId    = 0;
        OriginalMeshData.pMesh->AddRef();      
    }
}

/**
 * destruction
 */

Mesh::~Mesh()
{
    if( Name ) delete[] Name;
    if( pMaterials ) 
    {
        for( unsigned int i=0; i<NumMaterials; i++ )
        {
            if( pMaterials[i].pTextureFilename ) delete[] pMaterials[i].pTextureFilename;            
        }
        delete[] pMaterials;
    }
    if( pAdjacency ) delete[] pAdjacency;
    if( pSkinInfo ) pSkinInfo->Release();
    if( pBoneOffsetMatrices ) delete[] pBoneOffsetMatrices;
    if( pBoneCombination ) pBoneCombination->Release();
    if( pSoftwareBones ) delete[] pSoftwareBones;

    if( OriginalMeshData.pMesh ) OriginalMeshData.pMesh->Release();
    if( SkinnedMeshData.pMesh ) SkinnedMeshData.pMesh->Release();
}

/**
 * skined mesh generation method
 */

void Mesh::generateSkinnedMesh(void)
{
    // calculating palette size
    // first 9 shader constants are used for other data.  Each 4x3 matrix takes up 3 constants.
    // (96 - 9) /3 i.e. Maximum constant count - used constants 
    NumPaletteEntries = min( unsigned int( _bonePaletteSize ), pSkinInfo->GetNumBones() );

    DWORD Flags = D3DXMESHOPT_VERTEXCACHE;
    if( dxDeviceCaps.VertexShaderVersion >= D3DVS_VERSION(1, 1) )
    {
        UseSoftwareVP = false;
        Flags |= D3DXMESH_MANAGED;
    }
    else
    {
        UseSoftwareVP = true;
        Flags |= D3DXMESH_SYSTEMMEM;
    }

    // remove skin-related directx objects
    if( SkinnedMeshData.pMesh ) SkinnedMeshData.pMesh->Release();
    if( pBoneCombination ) pBoneCombination->Release();

    // convert original mesh to indexed blended mesh
    _dxCR( pSkinInfo->ConvertToIndexedBlendedMesh(
        OriginalMeshData.pMesh,
        Flags, 
        NumPaletteEntries, 
        pAdjacency, 
        NULL, NULL, NULL,             
        &NumInfluences,
        &NumAttributeGroups, 
        &pBoneCombination,
        &SkinnedMeshData.pMesh
    ) );    

    //
    // я понятия не имею зачем это нужно
    // было содрано из майкрософтовского сэмпла
    // при попытке добавить в меш тангенты и бинормали, FVF закатывает истерико
    // принято решение нахуй закомментировать
    //
    // FVF has to match our declarator. Vertex shaders are not as forgiving as FF pipeline    
    /*
    DWORD newFVF = ( SkinnedMeshData.pMesh->GetFVF() & D3DFVF_POSITION_MASK ) | 
                   ( D3DFVF_NORMAL | D3DFVF_TEX1 | D3DFVF_LASTBETA_UBYTE4 );
    if( newFVF != SkinnedMeshData.pMesh->GetFVF() )
    {
        ID3DXMesh* pTempMesh;
        _dxCR( SkinnedMeshData.pMesh->CloneMeshFVF( 
            SkinnedMeshData.pMesh->GetOptions(), 
            newFVF, iDirect3DDevice, &pTempMesh
        ) );
        SkinnedMeshData.pMesh->Release();
        SkinnedMeshData.pMesh = pTempMesh;
    }
    */

    D3DVERTEXELEMENT9 pDecl[MAX_FVF_DECL_SIZE];
    LPD3DVERTEXELEMENT9 pDeclCur;
    _dxCR( SkinnedMeshData.pMesh->GetDeclaration( pDecl ) );
    

    // the vertex shader is expecting to interpret the UBYTE4 as a D3DCOLOR, so update 
    // the type 
    // NOTE: this cannot be done with CloneMesh, that would convert the UBYTE4 data to 
    // float and then to D3DCOLOR; this is more of a "cast" operation
    
    pDeclCur = pDecl;
    while( pDeclCur->Stream != 0xff )
    {
        if( (pDeclCur->Usage == D3DDECLUSAGE_BLENDINDICES) && 
            (pDeclCur->UsageIndex == 0) )
        {
            pDeclCur->Type = D3DDECLTYPE_D3DCOLOR;
        }
        pDeclCur++;
    }
    SkinnedMeshData.pMesh->UpdateSemantics( pDecl );
}

/**
 * access the mesh buffers
 */

void* Mesh::lockVertexBuffer(DWORD flags)
{
    void* result = NULL;
    _dxCR( OriginalMeshData.pMesh->LockVertexBuffer( flags, &result ) );    
    return result;
}

void  Mesh::unlockVertexBuffer(void)
{
    _dxCR( OriginalMeshData.pMesh->UnlockVertexBuffer() );    
}

void* Mesh::lockIndexBuffer(DWORD flags)
{
    void* result = NULL;
    _dxCR( OriginalMeshData.pMesh->LockIndexBuffer( flags, &result ) );
    return result;
}

void  Mesh::unlockIndexBuffer(void)
{
    _dxCR( OriginalMeshData.pMesh->UnlockIndexBuffer() );    
}

void* Mesh::lockAttributeBuffer(DWORD flags)
{
    DWORD* result = NULL;
    _dxCR( OriginalMeshData.pMesh->LockAttributeBuffer( flags, &result ) );    
    return result;
}

void Mesh::unlockAttributeBuffer(void)
{
    _dxCR( OriginalMeshData.pMesh->UnlockAttributeBuffer() );    
}

/**
 * refinement methods
 */

void Mesh::optimize(DWORD flags)
{
    assert( pSkinInfo == NULL );

    DWORD* adjancecyBuffer = NULL;   
    ID3DXMesh* optimizedMesh  = NULL;

    unsigned int numFaces = OriginalMeshData.pMesh->GetNumFaces();
    adjancecyBuffer = new DWORD[3*OriginalMeshData.pMesh->GetNumFaces()];
    _dxCR( OriginalMeshData.pMesh->GenerateAdjacency( 0.01f, adjancecyBuffer ) );
    _dxCR( OriginalMeshData.pMesh->Optimize(
        flags, adjancecyBuffer,
        NULL, NULL, NULL,
        &optimizedMesh
    ) );
    OriginalMeshData.pMesh->Release();
    OriginalMeshData.pMesh = optimizedMesh;
    delete[] adjancecyBuffer;
}

void Mesh::getDeclaration(D3DVERTEXELEMENT9* vertexDeclaration)
{
    _dxCR( OriginalMeshData.pMesh->GetDeclaration( vertexDeclaration ) );
}

void Mesh::updateDeclaration(DWORD options, const D3DVERTEXELEMENT9 *vertexDeclaration)
{
    if( OriginalMeshData.Type != D3DXMESHTYPE_MESH ) return;

    if( pSkinInfo )
    {
        ID3DXMesh* updatedMesh = NULL;
        assert( OriginalMeshData.pMesh );
        _dxCR( OriginalMeshData.pMesh->CloneMesh( options, vertexDeclaration, iDirect3DDevice, &updatedMesh ) );
        OriginalMeshData.pMesh->Release();
        OriginalMeshData.pMesh = updatedMesh;
        generateSkinnedMesh();
    }
    else
    {
        ID3DXMesh* updatedMesh = NULL;
        _dxCR( OriginalMeshData.pMesh->CloneMesh( options, vertexDeclaration, iDirect3DDevice, &updatedMesh ) );
        OriginalMeshData.pMesh->Release();
        OriginalMeshData.pMesh = updatedMesh;
    }
}

/**
 * tangent space calculation
 */

static inline int getEntryId(DWORD& value, std::vector<DWORD>& vector)
{
    for( unsigned int i=0; i<vector.size(); i++ ) if( vector[i] == value ) return i;
    return -1;
}

static inline bool fuzzyEqual(const Vector& v1, const Vector& v2, float epsilon)
{
    return ( fabs( v1.x - v2.x ) <= epsilon ) &&
           ( fabs( v1.y - v2.y ) <= epsilon ) &&
           ( fabs( v1.z - v2.z ) <= epsilon );
}

void Mesh::calculateTangents(unsigned int texStageId)
{
    if( pSkinInfo )
    {
        // algorithm:
        // 1. clean mesh (remove bowties)
        // 2. generate new skininfo
        // 3. generate tangents
        // 4. generate skinned mesh

        DWORD startTime = GetTickCount();

        // generate anjancecy for cleaning operation
        unsigned int numFaces = OriginalMeshData.pMesh->GetNumFaces();
        DWORD* adjancecyBuffer = new DWORD[3*OriginalMeshData.pMesh->GetNumFaces()];        
        DWORD* cleanedAdjancecyBuffer = new DWORD[3*OriginalMeshData.pMesh->GetNumFaces()];
        _dxCR( OriginalMeshData.pMesh->GenerateAdjacency( 0.01f, adjancecyBuffer ) );

        // clean mesh
        ID3DXMesh* cleanedMesh = NULL;
        ID3DXBuffer* errorsAndWarnings = NULL;
        _dxCR( D3DXCleanMesh( 
            D3DXCLEAN_BOWTIES,
            OriginalMeshData.pMesh,
            adjancecyBuffer,
            &cleanedMesh,
            cleanedAdjancecyBuffer,
            &errorsAndWarnings
        ) );
        delete[] adjancecyBuffer;
        if( errorsAndWarnings ) errorsAndWarnings->Release();
        
        // generate new skininfo
        if( cleanedMesh != OriginalMeshData.pMesh )
        {
            // original declaration
            D3DVERTEXELEMENT9 originalVertexDeclaration[MAX_FVF_DECL_SIZE];
            _dxCR( OriginalMeshData.pMesh->GetDeclaration( originalVertexDeclaration ) );
            assert( originalVertexDeclaration[0].Usage == D3DDECLUSAGE_POSITION );

            // cleaned declaration
            D3DVERTEXELEMENT9 cleanedVertexDeclaration[MAX_FVF_DECL_SIZE];
            _dxCR( cleanedMesh->GetDeclaration( cleanedVertexDeclaration ) );
            assert( cleanedVertexDeclaration[0].Usage == D3DDECLUSAGE_POSITION );

            // calculate vertex strides
            unsigned int originalVertexStride = ::dxGetStride( originalVertexDeclaration );
            unsigned int cleanedVertexStride  = ::dxGetStride( cleanedVertexDeclaration );

            // obtain vertex buffers of cleaned and original mesh
            void* originalBuffer = NULL;
            void* cleanedBuffer = NULL;
            _dxCR( OriginalMeshData.pMesh->LockVertexBuffer( D3DLOCK_READONLY, &originalBuffer ) );
            _dxCR( cleanedMesh->LockVertexBuffer( D3DLOCK_READONLY, &cleanedBuffer ) );
            BYTE* originalVertexData = (BYTE*)(originalBuffer);
            BYTE* cleanedVertexData = (BYTE*)(cleanedBuffer);
            
            // build transition table
            unsigned int numBones = pSkinInfo->GetNumBones();
            unsigned int numOriginalMeshVertices = OriginalMeshData.pMesh->GetNumVertices();
            unsigned int numCleanedMeshVertices = cleanedMesh->GetNumVertices();
            DWORD* transition = new DWORD[numCleanedMeshVertices];

            // pass all vertices of cleaned mesh
            Vector cleanedVertex;
            Vector originalVertex;
            DWORD cleanedVertexId;
            DWORD originalVertexId;
            for( cleanedVertexId = 0; 
                 cleanedVertexId < numCleanedMeshVertices; 
                 cleanedVertexId++ )
            {
                // obtain vertex of cleaned mesh
                cleanedVertex = *((Vector*)( cleanedVertexData + cleanedVertexStride * cleanedVertexId ));
                // obtain original vertex at the index of cleaned vertex
                originalVertex = *((Vector*)( originalVertexData + originalVertexStride * cleanedVertexId ));
                // are cleaned and original vertices almost equal??
                if( fuzzyEqual( cleanedVertex, originalVertex, 0.01f ) )
                {
                    // write direct transition
                    transition[cleanedVertexId] = cleanedVertexId;
                }
                // else, pass all vertices of original mesh
                else for( originalVertexId = 0; 
                          originalVertexId < numOriginalMeshVertices; 
                          originalVertexId++ )
                {
                    // obtain original vertex 
                    originalVertex = *((Vector*)( originalVertexData + originalVertexStride * originalVertexId ));
                    // are cleaned and original vertices almost equal??
                    if( fuzzyEqual( cleanedVertex, originalVertex, 0.01f ) )
                    {
                        // write reference transition
                        transition[cleanedVertexId] = originalVertexId;
                    }
                }
            }

            // create skin info for cleaned mesh
            ID3DXSkinInfo* cleanedSkinInfo = NULL;
            _dxCR( D3DXCreateSkinInfo(
                cleanedMesh->GetNumVertices(),
                cleanedVertexDeclaration,
                pSkinInfo->GetNumBones(),
                &cleanedSkinInfo
            ) );

            // rebuild skin info
            unsigned int boneId;
            unsigned int numBoneInfluences;
            DWORD transitionId;
            int entryId;
            for( boneId=0; boneId<numBones; boneId++ )
            {
                std::vector<DWORD> originalVertices, cleanedVertices;
                std::vector<FLOAT> originalWeights, cleanedWeights;                
                // obtain number of influences for original mesh
                numBoneInfluences = pSkinInfo->GetNumBoneInfluences( boneId );
                // obtain influences
                originalVertices.resize( numBoneInfluences );
                originalWeights.resize( numBoneInfluences );
                pSkinInfo->GetBoneInfluence( boneId, &originalVertices[0], &originalWeights[0] );
                
                // pass all vertices of cleaned mesh
                for( cleanedVertexId = 0; 
                     cleanedVertexId < numCleanedMeshVertices; 
                     cleanedVertexId++ )
                {
                    // obtain transition id of cleaned vertex
                    transitionId = transition[cleanedVertexId];

                    // does current bone influences an original vertex?
                    entryId = getEntryId( transitionId, originalVertices );
                    if( entryId != -1 )
                    {
                        // put cleaned vertex under incluence of bone
                        cleanedVertices.push_back( cleanedVertexId );
                        cleanedWeights.push_back( originalWeights[entryId] );
                    }
                }

                // setup bone influences for cleaned mesh
                _dxCR( cleanedSkinInfo->SetBoneName( 
                    boneId,
                    pSkinInfo->GetBoneName( boneId )
                ) );
                _dxCR( cleanedSkinInfo->SetBoneInfluence(
                    boneId,
                    cleanedVertices.size(),
                    &cleanedVertices[0],
                    &cleanedWeights[0]
                ) );
            }

            delete[] transition;
            _dxCR( cleanedMesh->UnlockVertexBuffer() );
            _dxCR( OriginalMeshData.pMesh->UnlockVertexBuffer() );

            // replace original mesh & original skin info
            pSkinInfo->Release();
            pSkinInfo = cleanedSkinInfo;
            OriginalMeshData.pMesh->Release();
            OriginalMeshData.pMesh = cleanedMesh;            
        }

        // compute tangents
        _dxCR( D3DXComputeTangent( OriginalMeshData.pMesh, texStageId, 0, 0, 0, cleanedAdjancecyBuffer ) );
        delete[] cleanedAdjancecyBuffer;

        // rebuild skinned mesh
        generateSkinnedMesh();        

        DWORD endTime = GetTickCount();
        getCore()->logMessage( 
            "engine: ID3DXSkinInfo for %d vertices rebuilded at %3.2f seconds", 
            OriginalMeshData.pMesh->GetNumVertices(),
            float(endTime-startTime) * 0.001f
        );
    }
    else
    {
        DWORD* adjancecyBuffer = NULL;   

        unsigned int numFaces = OriginalMeshData.pMesh->GetNumFaces();
        adjancecyBuffer = new DWORD[3*OriginalMeshData.pMesh->GetNumFaces()];
        _dxCR( OriginalMeshData.pMesh->GenerateAdjacency( 0.01f, adjancecyBuffer ) );
        _dxCR( D3DXComputeTangent( OriginalMeshData.pMesh, texStageId, 0, 0, 0, adjancecyBuffer ) );
        delete[] adjancecyBuffer;
    }
}

/**
 * rendering method
 */

DWORD Mesh::getAttributeId(int subsetId)
{
    if( !pSkinInfo ) return subsetId;
    assert( pBoneCombination );
    return ( reinterpret_cast<D3DXBONECOMBINATION*>( pBoneCombination->GetBufferPointer() ) + subsetId )->AttribId;
}

void Mesh::renderSubset(int subsetId, Shader* shader)
{
    // skinned mesh with per-pexel lighting
    if( pSkinInfo && shader->normalMap() )
    {
        assert( shader->getLayerTexture(0) );

        if( UseSoftwareVP ) iDirect3DDevice->SetSoftwareVertexProcessing( TRUE );

        LPD3DXBONECOMBINATION pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>( pBoneCombination->GetBufferPointer() );
        
        // first calculate all the world matrices        
        UINT          iPaletteEntry;
        UINT          iMatrixIndex;
        D3DXMATRIXA16 tempMatrix;        

        for( iPaletteEntry=0; iPaletteEntry<NumPaletteEntries; ++iPaletteEntry )
        {
            iMatrixIndex = pBoneComb[subsetId].BoneId[iPaletteEntry];
            if( iMatrixIndex != UINT_MAX )
            {
                D3DXMatrixMultiply( 
                    &tempMatrix, 
                    &pBoneOffsetMatrices[iMatrixIndex], 
                    pBoneMatrices[iMatrixIndex] 
                );
                _pBoneMatrices[iPaletteEntry] = tempMatrix;
            }
        }        
        _effectx->SetMatrixArray( 
            "bone", 
            _pBoneMatrices, 
            NumPaletteEntries
        );

        // view-projection matrix
        D3DXMatrixMultiply( 
            &tempMatrix, 
            &Camera::viewMatrix,
            &Camera::projectionMatrix
        );
        _effectx->SetMatrix( "viewProj", &tempMatrix );

        // camera position & direction 
        Quartector cameraPos( Camera::eyePos.x, Camera::eyePos.y, Camera::eyePos.z, 0.0f );
        _effectx->SetVector( "cameraPos", &cameraPos );
        Quartector cameraDir( Camera::eyeDirection.x, Camera::eyeDirection.y, Camera::eyeDirection.z, 0.0f );
        _effectx->SetVector( "cameraDir", &cameraDir );           

        // ambient lighting
        _effectx->SetVector( "ambientColor", (D3DXVECTOR4*)( Shader::globalAmbient() ) );

        // setup dynamic lighting
        BOOL          lightEnabled;
        D3DLIGHT9     lightProperties;
        Quartector    lightPos;
        D3DCOLORVALUE lightColor;
        for( unsigned int lightId=0; lightId<8; lightId++ )
        {            
            // check light is enabled
            _dxCR( iDirect3DDevice->GetLightEnable( lightId, &lightEnabled ) );
            if( !lightEnabled ) continue;
            // retrieve light properties
            _dxCR( iDirect3DDevice->GetLight( lightId, &lightProperties ) );
            if( lightProperties.Type == D3DLIGHT_POINT )
            {
                lightPos.x = lightProperties.Position.x;
                lightPos.y = lightProperties.Position.y;
                lightPos.z = lightProperties.Position.z;
                lightPos.w = 1.0f;
                lightColor = lightProperties.Diffuse;
                _effectx->SetVector( "lightPos", &lightPos );
                _effectx->SetVector( "lightColor", (D3DXVECTOR4*)&lightColor );
                break;
            }
        }
        
        // set material color properties
        D3DMATERIAL9 material;
        iDirect3DDevice->GetMaterial( &material );
        _effectx->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
        _effectx->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
        _effectx->SetFloat( "materialPower", material.Power );

        // if cinematic lighting is enabled - setup cinematic properties
        if( ( shader->getFlags() & engine::sfCinematicLighting ) &&
            ( Atomic::currentAtomic != NULL ) )
        {
            Quartector value;

            // enable cinematic effect
            _effectx->SetBool( "cinematicEnabled", true );            
            // setup illumination color
            _effectx->SetVector( "illuminationColor", (D3DXVECTOR4*)(shader->cinematicIlluminationColor()) );
            // setup contour color
            _effectx->SetVector( "contourColor", (D3DXVECTOR4*)(shader->cinematicContourColor()) );
            // setup direction of contour light source
            value.x = Atomic::currentAtomic->cinematicContourDirection()->x;
            value.y = Atomic::currentAtomic->cinematicContourDirection()->y;
            value.z = Atomic::currentAtomic->cinematicContourDirection()->z;
            value.w = 0.0f;
            _effectx->SetVector( "contourDir", &value );
        }
        else
        {
            if( shader->getFlags() & engine::sfCinematicLighting )
            {
                getCore()->logMessage( "engine: no contour light source defined for shader \"%s\"!", shader->getName() );
            }

            // set no cinematic lighting
            _effectx->SetBool( "cinematicEnabled", false );
        }

        // set CurNumBones to select the correct vertex shader for the number of bones
        _effectx->SetInt( "numBones", NumInfluences-1 );

        // set textures
        _effectx->SetTexture( "baseTexture", shader->layerTexture( 0 )->iDirect3DTexture() );
        _effectx->SetTexture( "normalMap", shader->normalMap()->iDirect3DTexture() );

        // start the effect now all parameters have been updated
        UINT numPasses;
        _effectx->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
        for( UINT iPass = 0; iPass < numPasses; iPass++ )
        {
            _effectx->BeginPass( iPass );
            // draw the subset with the current world matrix palette and material state
            SkinnedMeshData.pMesh->DrawSubset( subsetId );
            _effectx->EndPass();
        }
        _effectx->End();   

        _effectx->SetTexture( "baseTexture", NULL );
        _effectx->SetTexture( "normalMap", NULL );

        iDirect3DDevice->SetVertexShader(NULL);
        iDirect3DDevice->SetPixelShader(NULL);

        // remember to reset back to hw vertex processing if software was required
        if( UseSoftwareVP ) iDirect3DDevice->SetSoftwareVertexProcessing( FALSE );
    }
    else
    // skinned mesh with per-vertex lighting
    if( pSkinInfo )
    {
        IDirect3DPixelShader9* currentPixelShader;
        iDirect3DDevice->GetPixelShader( &currentPixelShader );
        assert( currentPixelShader == NULL );

        if( UseSoftwareVP ) iDirect3DDevice->SetSoftwareVertexProcessing( TRUE );

        LPD3DXBONECOMBINATION pBoneComb = reinterpret_cast<LPD3DXBONECOMBINATION>( pBoneCombination->GetBufferPointer() );
        
        // first calculate all the world matrices        
        UINT          iPaletteEntry;
        UINT          iMatrixIndex;
        D3DXMATRIXA16 tempMatrix;        

        for( iPaletteEntry=0; iPaletteEntry<NumPaletteEntries; ++iPaletteEntry )
        {
            iMatrixIndex = pBoneComb[subsetId].BoneId[iPaletteEntry];
            if( iMatrixIndex != UINT_MAX )
            {
                D3DXMatrixMultiply( 
                    &tempMatrix, 
                    &pBoneOffsetMatrices[iMatrixIndex], 
                    pBoneMatrices[iMatrixIndex] 
                );
                _pBoneMatrices[iPaletteEntry] = tempMatrix;
            }
        }        
        _effect->SetMatrixArray( 
            "bone", 
            _pBoneMatrices, 
            NumPaletteEntries
        );

        // view-projection matrix
        D3DXMatrixMultiply( 
            &tempMatrix, 
            &Camera::viewMatrix,
            &Camera::projectionMatrix
        );
        _effect->SetMatrix( "viewProj", &tempMatrix );

        // camera position
        Quartector cameraPos( Camera::eyePos.x, Camera::eyePos.y, Camera::eyePos.z, 1.0f );
        _effect->SetVector( "cameraPos", &cameraPos );

        // ambient lighting
        _effect->SetVector( "ambientColor", (D3DXVECTOR4*)( Shader::globalAmbient() ) );

        // cube ambient
        if( shader->getFlags() & engine::sfHemisphere )
        {
            _effect->SetBool( "hemisphereEnabled", true );
            _effect->SetVector( "upperHemisphere", (D3DXVECTOR4*)(shader->hemisphere()) );
            _effect->SetVector( "lowerHemisphere", (D3DXVECTOR4*)(shader->hemisphere()+1) );
        }
        else
        {
            _effect->SetBool( "hemisphereEnabled", false );
        }

        // setup dynamic lighting
        D3DLIGHT9     lightProperties;
        BOOL          lightEnabled;
        Quartector    lightPos[8];
        D3DCOLORVALUE lightColor[8];

        unsigned int numLights = 0;
        for( unsigned int lightId=0; lightId<8; lightId++ )
        {
            // exit cycle when palette is full
            if( numLights == 8 ) break;
            // check light is enabled
            _dxCR( iDirect3DDevice->GetLightEnable( lightId, &lightEnabled ) );
            if( !lightEnabled ) continue;
            // retrieve light properties
            _dxCR( iDirect3DDevice->GetLight( lightId, &lightProperties ) );
            if( lightProperties.Type == D3DLIGHT_POINT )
            {
                lightPos[numLights].x = lightProperties.Position.x;
                lightPos[numLights].y = lightProperties.Position.y;
                lightPos[numLights].z = lightProperties.Position.z;
                lightPos[numLights].w = 1.0f;
                lightColor[numLights] = lightProperties.Diffuse;
                numLights++;
            }
        }

        _effect->SetInt( "numLights", numLights );
        if( numLights )
        {
            _effect->SetVectorArray( "lightPos", lightPos, numLights );
            _effect->SetVectorArray( "lightColor", (D3DXVECTOR4*)lightColor, numLights );
        }
        
        // set material color properties
        D3DMATERIAL9 material;
        iDirect3DDevice->GetMaterial( &material );
        _effect->SetVector( "materialDiffuse", (D3DXVECTOR4*)&material.Diffuse );
        _effect->SetVector( "materialSpecular", (D3DXVECTOR4*)&material.Specular );
        _effect->SetFloat( "materialPower", material.Power );

        // set CurNumBones to select the correct vertex shader for the number of bones
        _effect->SetInt( "numBones", NumInfluences-1 );

        // start the effect now all parameters have been updated
        UINT numPasses;
        _effect->Begin( &numPasses, D3DXFX_DONOTSAVESTATE );
        for( UINT iPass = 0; iPass < numPasses; iPass++ )
        {
            _effect->BeginPass( iPass );
            // draw the subset with the current world matrix palette and material state
            SkinnedMeshData.pMesh->DrawSubset( subsetId );
            _effect->EndPass();
        }
        _effect->End();        

        iDirect3DDevice->SetVertexShader(NULL);

        // remember to reset back to hw vertex processing if software was required
        if( UseSoftwareVP ) iDirect3DDevice->SetSoftwareVertexProcessing( FALSE );
    }
    // un-skinned mesh
    else
    {
        OriginalMeshData.pMesh->DrawSubset( subsetId );        
    }
}

/**
 * skinning
 */

D3DXMATRIX** Mesh::assembleBoneMatrices(Frame* root)
{
    if( pSkinInfo == NULL ) return NULL;
    
    int numBones = pSkinInfo->GetNumBones();
    D3DXMATRIX** boneMatrices = new D3DXMATRIX*[numBones];
    assert( boneMatrices );

    for( int i=0; i<numBones; i++ )
    {
        const char* boneName = pSkinInfo->GetBoneName( i );
        Frame* frame = static_cast<Frame*>( D3DXFrameFind( root, boneName ) );
        assert( frame );
        boneMatrices[i] = &frame->LTM;
    }
    return boneMatrices;
}

void Mesh::getSkinnedVertices(Vector* buffer)
{
    assert( pSkinInfo );
    assert( pBoneMatrices );

    int numBones = pSkinInfo->GetNumBones();

    // prepare bone matrices
    for( int boneId=0; boneId<numBones; boneId++ )
    {
        D3DXMatrixMultiply( 
            pSoftwareBones + boneId,
            &pBoneOffsetMatrices[boneId],
            Mesh::pBoneMatrices[boneId]
        );
    }

    IDirect3DVertexBuffer9* vertexBuffer;
    _dxCR( OriginalMeshData.pMesh->GetVertexBuffer( &vertexBuffer ) );

    D3DVERTEXBUFFER_DESC vertexBufferDesc;
    _dxCR( vertexBuffer->GetDesc( &vertexBufferDesc ) );

    if( _sizeOfSkinnedVertices < vertexBufferDesc.Size )
    {
        _sizeOfSkinnedVertices = vertexBufferDesc.Size;
        if( _skinnedVertices ) delete[] _skinnedVertices;
        _skinnedVertices = new BYTE[_sizeOfSkinnedVertices];
    }

    PBYTE unskinnedVertices = NULL;
    _dxCR( OriginalMeshData.pMesh->LockVertexBuffer(
        D3DLOCK_READONLY, 
        (LPVOID*)&unskinnedVertices
    ) );

    _dxCR( pSkinInfo->UpdateSkinnedMesh(
        pSoftwareBones, 
        NULL, unskinnedVertices, _skinnedVertices
    ) );

    D3DVERTEXELEMENT9 declaration[MAX_FVF_DECL_SIZE];
    _dxCR( OriginalMeshData.pMesh->GetDeclaration( declaration ) );
    assert( declaration[0].Usage == D3DDECLUSAGE_POSITION );

    unsigned int numVertices = OriginalMeshData.pMesh->GetNumVertices();
    unsigned int stride = ::dxGetStride( declaration );
    assert( vertexBufferDesc.Size % numVertices == 0 );

    for( unsigned int i=0; i<numVertices; i++ )
    {
        memcpy( buffer+i, _skinnedVertices + stride * i, sizeof(Vector) );
    }

    _dxCR( OriginalMeshData.pMesh->UnlockVertexBuffer() );    
}

/**
 * lostable
 */

void Mesh::onLostDevice(void)
{
    _effect->OnLostDevice();
    _effectx->OnLostDevice();
}

void Mesh::onResetDevice(void)
{
    _effect->OnResetDevice();
    _effectx->OnResetDevice();
}