
#include "headers.h"
#include "asset.h"
#include "wire.h"
#include "animation.h"

#include "rmxftmpl.h"
#include "rmxfguid.h"
#include "dxfile.h"

#include "gui.h"
#include "../common/unicode.h"

/**
 * hierarchy allocation methods
 */

class XAllocateHierarchy: public ID3DXAllocateHierarchy
{
public:
	STDMETHOD(CreateFrame)(THIS_ LPCSTR Name, LPD3DXFRAME *ppNewFrame);
    STDMETHOD(CreateMeshContainer)(
		THIS_ LPCSTR Name, 
        CONST D3DXMESHDATA *pMeshData,
        CONST D3DXMATERIAL *pMaterials, 
        CONST D3DXEFFECTINSTANCE *pEffectInstances, 
        DWORD NumMaterials, 
        CONST DWORD *pAdjacency, 
        LPD3DXSKININFO pSkinInfo, 
        LPD3DXMESHCONTAINER *ppNewMeshContainer);
    STDMETHOD(DestroyFrame)(THIS_ LPD3DXFRAME pFrameToFree);
    STDMETHOD(DestroyMeshContainer)(THIS_ LPD3DXMESHCONTAINER pMeshContainerBase);
};

HRESULT XAllocateHierarchy::CreateFrame( LPCSTR Name, LPD3DXFRAME *ppNewFrame )
{
    *ppNewFrame = new Frame( Name );
    return S_OK;
}

HRESULT XAllocateHierarchy::CreateMeshContainer(
    LPCSTR Name, 
    CONST D3DXMESHDATA *pMeshData,
    CONST D3DXMATERIAL *pMaterials, 
    CONST D3DXEFFECTINSTANCE *pEffectInstances, 
    DWORD NumMaterials, 
    CONST DWORD *pAdjacency, 
    LPD3DXSKININFO pSkinInfo, 
    LPD3DXMESHCONTAINER *ppNewMeshContainer) 
{
    *ppNewMeshContainer = new Mesh( 
        Name,
        pMeshData,
        pMaterials,
        NumMaterials,
        pAdjacency,
        pSkinInfo
    );
    return S_OK;
}

HRESULT XAllocateHierarchy::DestroyFrame(LPD3DXFRAME pFrameToFree) 
{
    if( pFrameToFree ) 
    {
        delete pFrameToFree;
    }
    return S_OK;
}

HRESULT XAllocateHierarchy::DestroyMeshContainer(LPD3DXMESHCONTAINER pMeshContainerBase)
{    
    if( pMeshContainerBase ) 
    {
        Mesh* mesh = static_cast<Mesh*>( pMeshContainerBase );
        delete mesh;
    }
    return S_OK;
}

/**
 * XAsset implementation
 */
 
XAsset::XAsset(const char* resourcePath)
{
    // report progress
    if( Engine::instance->progressCallback )
    {
        Engine::instance->progressCallback(
            wstrformat( Gui::iLanguage->getUnicodeString(4), asciizToUnicode(resourcePath).c_str() ).c_str(),
            0.0f,
            Engine::instance->progressCallbackUserData
        );
    }

    IResource* resource = getCore()->getResource( resourcePath, "rb" );
    assert( resource );
    
    // retrieve file size
    FILE* file = resource->getFile();
    fseek( file, 0, SEEK_END );
    int fileSize = ftell( file );
    fseek( file, 0, SEEK_SET );

    // create buffer & load file data in buffer
    char* buffer = new char[fileSize];
    fread( buffer, 1, fileSize, file );
    resource->release();

    // report progress
    if( Engine::instance->progressCallback )
    {
        Engine::instance->progressCallback(
            wstrformat( Gui::iLanguage->getUnicodeString(4), asciizToUnicode(resourcePath).c_str() ).c_str(),
            0.25f,
            Engine::instance->progressCallbackUserData
        );
    }

    // create hierarchy allocation interface
    XAllocateHierarchy xAlloc;

    // load hierarchy from file
    D3DXFRAME*                rootFrame;
    ID3DXAnimationController* animController;
    _dxCR( D3DXLoadMeshHierarchyFromXInMemory(
        buffer,
        fileSize,
        D3DXMESH_MANAGED, 
        iDirect3DDevice, 
        &xAlloc, 
        NULL, 
        &rootFrame, 
        &animController
    ) );

    delete[] buffer;

    // report progress
    if( Engine::instance->progressCallback )
    {
        Engine::instance->progressCallback(
            wstrformat( Gui::iLanguage->getUnicodeString(4), asciizToUnicode(resourcePath).c_str() ).c_str(),
            0.5f,
            Engine::instance->progressCallbackUserData
        );
    }

    // create D3 animation objects
    Clump* clump = new Clump( "SCENE_ROOT" );
    clump->setFrame( static_cast<Frame*>( rootFrame ) );

    if( animController )
    {        
	    ID3DXAnimationSet* animationSet;
        const char*        animationName;
    
	    animController->GetAnimationSet( 0, &animationSet );
	    unsigned int numAnimations = animationSet->GetNumAnimations();
        float period = float( animationSet->GetPeriod() );
        float numFramesPerSecond = 100.0f/3.0f;
        float frameTime = 1.0f / numFramesPerSecond;
        unsigned int numFrames = unsigned int( period * numFramesPerSecond );
        if( !numFrames ) numFrames++;

        AnimationSet* as = new AnimationSet( numAnimations );
	
	    for( unsigned int i=0; i<numAnimations; i++ )
	    {
		    SRT srt;
	
		    animationSet->GetAnimationNameByIndex( i, &animationName );

            Animation* animation = new Animation( animationName, numFrames );

		    for( unsigned int j=0; j<numFrames; j++ )
		    {
                srt.time = j * frameTime;
			    _dxCR( animationSet->GetSRT( 
				    srt.time,
				    i,
				    &srt.scale,
				    &srt.rotation,
				    &srt.translation
                ) );            
                animation->setKey( j, &srt );
            }

            assert( animation->validateKeys() );
        
            as->setAnimation( i, animation );
        }
        clump->setAnimation( as );    

        animationSet->Release();
        animController->Release();
    }

    // report progress
    if( Engine::instance->progressCallback )
    {
        Engine::instance->progressCallback(
            wstrformat( Gui::iLanguage->getUnicodeString(4), asciizToUnicode(resourcePath).c_str() ).c_str(),
            0.75f,
            Engine::instance->progressCallbackUserData
        );
    }

    // create D3 classes for hierarchy
    createD3HierarchyClasses( clump, rootFrame );

    // resolve nameless phenomena
    resolveNamelessFrames( clump );

    _clumps.push_back( clump );
    
    static_cast<Frame*>( rootFrame )->dirty();

    // report progress
    if( Engine::instance->progressCallback )
    {
        Engine::instance->progressCallback(
            wstrformat( Gui::iLanguage->getUnicodeString(4), asciizToUnicode(resourcePath).c_str() ).c_str(),
            1.0f,
            Engine::instance->progressCallbackUserData
        );
    }
}

XAsset::~XAsset(void)
{
}

void XAsset::release(void)
{
    delete this;
}

void XAsset::serialize(void)
{
    assert( !"Method is not supported!" );
}

void XAsset::createD3HierarchyClasses(Clump* clump, D3DXFRAME* frame)
{
    Frame* f = static_cast<Frame*>( frame );
    while( f->pMeshContainer )
    {
        Mesh* mesh = static_cast<Mesh*>( f->pMeshContainer );
        
        const char* geometryName = mesh->Name;
        if( !geometryName ) geometryName = "NamelessGeometry";
        Geometry* geometry = new Geometry( mesh, geometryName );

        Atomic* atomic = new Atomic;
        atomic->setGeometry( geometry );
        atomic->setFrame( f );

        clump->add( atomic );

        f->pMeshContainer = f->pMeshContainer->pNextMeshContainer;
    }

    if( f->pFrameFirstChild ) 
    {
        static_cast<Frame*>( f->pFrameFirstChild )->pParentFrame = f;
        createD3HierarchyClasses( clump, f->pFrameFirstChild );
    }

    if( f->pFrameSibling ) 
    {
        static_cast<Frame*>( f->pFrameSibling )->pParentFrame = f->pParentFrame;
        createD3HierarchyClasses( clump, f->pFrameSibling );
    }
}

struct FrameChildId
{
public:
    Frame*       frame;
    unsigned int childId;
public:
    FrameChildId(Frame* f) : frame(f), childId(0) {}
};

static engine::IFrame* getChildIdCB(engine::IFrame* frame, void* data)
{
    FrameChildId* frameChildId = reinterpret_cast<FrameChildId*>( data );
    if( frame == frameChildId->frame ) return NULL;
    frameChildId->childId++;
    return frame;
}

static unsigned int getChildId(Frame* frame)
{
    assert( frame->getParent() );
    FrameChildId childId( frame );
    frame->getParent()->forAllChildren( getChildIdCB, &childId );
    return childId.childId;
}

static void resolveNamelessFrame(Frame* frame)
{
    // frame is nameless?
    if( strcmp( frame->getName(), "" ) == 0 )
    {
        delete[] frame->Name;
        // frame has a parent?
        if( frame->getParent() )
        {
            std::string frameName = strformat( "%s_child%d", frame->getParent()->getName(), getChildId( frame ) );
            frame->Name = new char[ frameName.length() + 1];
            strcpy( frame->Name, frameName.c_str() );
        }
        else
        {
            std::string frameName = "Root";
            frame->Name = new char[ frameName.length() + 1];
            strcpy( frame->Name, frameName.c_str() );
        }
    }

    // pass sibling & childrens
    if( frame->pFrameSibling ) resolveNamelessFrame( static_cast<Frame*>( frame->pFrameSibling ) );
    if( frame->pFrameFirstChild ) resolveNamelessFrame( static_cast<Frame*>( frame->pFrameFirstChild ) );
}

void XAsset::resolveNamelessFrames(Clump* clump)
{
    resolveNamelessFrame( dynamic_cast<Frame*>( clump->getFrame() ) );
}