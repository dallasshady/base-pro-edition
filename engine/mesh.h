/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2007
 *
 * @description directx mesh manipulation, ported from DirectX sample factory
 * @author bad3p
 *
 * this source was refactored in October 2007
 */

#ifndef DX_MESH_INCLUDED
#define DX_MESH_INCLUDED

#include "headers.h"
#include "engine.h"
#include "shader.h"
#include "frame.h"

/**
 * Mesh is a extension of D3DXMESHCONTAINER structure, provided with skinning
 * and other effects behaviour
 */

struct Mesh : public D3DXMESHCONTAINER
{
private:
    static ID3DXEffect*    _effect;           // vertex shader for skinned mesh
    static ID3DXEffect*    _effectx;          // vertex shader + per-pexel lighting
    static int             _bonePaletteSize;  // (setup) number of bones per skin patch
    static int             _lightPaletteSize; // (setup) number of light per render query
    static D3DXMATRIXA16*  _pBoneMatrices;
    static StaticLostable* _effectLostable;
    static DWORD           _sizeOfSkinnedVertices; // current size of buffer for SW skinning
    static PBYTE           _skinnedVertices;       // buffer for SW skinning
public:
    // special skinning extension 
    D3DXMESHDATA OriginalMeshData;    // original mesh data
    D3DXMESHDATA SkinnedMeshData;     // skinned mesh data
    DWORD        NumInfluences;       // number of bone influences
    ID3DXBuffer* pBoneCombination;    // bone combination buffer
    D3DXMATRIX*  pBoneOffsetMatrices; // bone offset matrices
    MatrixA16*   pSoftwareBones;      // array of bones for software skinning
    DWORD        NumPaletteEntries;   // number of index palette entries
    DWORD        NumAttributeGroups;  // number of attribute groups
    DWORD        SoftwareAttributeId; // denotes the split between SW and HW if necessary for non-indexed skinning
    DWORD        UseSoftwareVP;       // flag of software vertex processing
public:
    // construction
    Mesh(DWORD numVertices,
         DWORD numTriangles,
         DWORD numAttributes,
         DWORD meshOptions,
         D3DVERTEXELEMENT9* vertexDeclaration);
    Mesh(LPCTSTR meshName,
         CONST D3DXMESHDATA* meshData,
         CONST D3DXMATERIAL* meshMaterials,
         DWORD numMeshMaterials,
         CONST DWORD *adjacency,
         LPD3DXSKININFO skinInfo);
public:
    // destruction
    virtual ~Mesh();
private:
    // cooperative work
    static void onLostDevice(void);
    static void onResetDevice(void);
private:
    void generateSkinnedMesh(void);
public:
    // access mesh buffers
    void* lockVertexBuffer(DWORD flags);
    void  unlockVertexBuffer(void);
    void* lockIndexBuffer(DWORD flags);
    void  unlockIndexBuffer(void);
    void* lockAttributeBuffer(DWORD flags);
    void  unlockAttributeBuffer(void);
public:
    // mesh refinement
    void optimize(DWORD flags);
    void getDeclaration(D3DVERTEXELEMENT9* vertexDeclaration);
    void updateDeclaration(DWORD options, const D3DVERTEXELEMENT9 *vertexDeclaration);
    void calculateTangents(unsigned int texStageId);
public:
    // mesh rendering
    DWORD getAttributeId(int subsetId);
    void renderSubset(int subsetId, Shader* shader);    
public:
    // skinning support: assembling of bone matrices
    D3DXMATRIX** assembleBoneMatrices(Frame* root);
    // skinning support: fill buffer with current skinned vertices
    void getSkinnedVertices(Vector* buffer);
    // skinning support: current bone matrices
    static D3DXMATRIX** pBoneMatrices;
public:
    // initialization & etc.
    static void init(void);
    static void term(void);
};

#endif
