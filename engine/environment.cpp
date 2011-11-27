
#include "headers.h"
#include "texture.h"
#include "../common/istring.h"
#include "camera.h"

/**
 * environment map rendering routine
 */

static Texture*              _envMap          = NULL;
static ID3DXRenderToSurface* _renderToSurface = NULL;

void Engine::beginEnvironmentMap(
    engine::ITexture*   envMap,
    engine::CubemapFace face,
    Matrix4f            cameraMatrix,
    unsigned int        clearMode,
    const Vector4f&     clearColor
)
{
    assert( _envMap == NULL );
    _envMap = dynamic_cast<Texture*>( envMap ); assert( _envMap );
    assert( _envMap->iDirect3DCubeTexture() );

    _dxCR( D3DXCreateRenderToSurface( 
        iDirect3DDevice,
        _envMap->getWidth(),
        _envMap->getHeight(),
        _envMap->getFormat(),
        true,
        dxPresentParams.AutoDepthStencilFormat,
        &_renderToSurface
    ) );
    assert( _renderToSurface );

    D3DVIEWPORT9 viewPort;
    viewPort.X = 0;
    viewPort.Y = 0;
    viewPort.Width = _envMap->getWidth();
    viewPort.Height = _envMap->getHeight();
    viewPort.MinZ = 0;
    viewPort.MaxZ = 1;

    IDirect3DSurface9* iDirect3DSurface;
    _dxCR( _envMap->iDirect3DCubeTexture()->GetCubeMapSurface( D3DCUBEMAP_FACES(face), 0, &iDirect3DSurface ) );
    _dxCR( _renderToSurface->BeginScene( iDirect3DSurface, &viewPort ) );
    iDirect3DSurface->Release();

    // frame synchronization phase
    Frame::synchronizeAll();

    if( clearMode ) _dxCR( iDirect3DDevice->Clear( 0, NULL, clearMode, wrap( clearColor ), 1.0f, 0L ) );

    // setup transformation matrices
    Matrix m = wrap( cameraMatrix );
    D3DXMatrixInverse( &Camera::viewMatrix, NULL, &m );
    _dxCR( iDirect3DDevice->SetTransform( D3DTS_VIEW, &Camera::viewMatrix ) );    
    Matrix p;
    D3DXMatrixPerspectiveFovRH(
        &p, 
        90.0f * D3DX_PI / 180.0f, 
        float( _envMap->getWidth() ) / float( _envMap->getHeight() ), 
        Engine::instance->getDefaultCamera()->getNearClipPlane(),
        Engine::instance->getDefaultCamera()->getFarClipPlane()
    );
    Camera::projectionMatrix = p;
    _dxCR( iDirect3DDevice->SetTransform( D3DTS_PROJECTION, &p ) );

    Camera::viewPort     = viewPort;    
    Camera::eyePos       = dxPos( &m );
    Camera::eyeDirection = dxAt( &m );
    Camera::fov          = 90.0f;

    // frustrum planes
    Vector r_origin( m._41, m._42, m._43 );
    Vector vpn( m._31, m._32, m._33 );
    Vector vright( m._11, m._12, m._13 );
    Vector vup( m._21, m._22, m._23 );

    // for right-handed coordinate system
    vpn *= - 1, vright *= -1, vup *= -1;

	float orgOffset = D3DXVec3Dot( &r_origin, &vpn );
	
    // far plane
    Camera::frustrum[4].a = -vpn.x;
    Camera::frustrum[4].b = -vpn.y;
    Camera::frustrum[4].c = -vpn.z;
    Camera::frustrum[4].d = -Camera::farClipPlane - orgOffset;
    
    // near plane
	Camera::frustrum[5].a = vpn.x;
    Camera::frustrum[5].b = vpn.y;
    Camera::frustrum[5].c = vpn.z;
	Camera::frustrum[5].d = Camera::nearClipPlane + orgOffset;

    // (1.33) is reserve multiplier
    float fovx = 90.0f * 1.33f; 
    // (1.1) is reserve multiplier
	float fovy = 90.0f * Camera::viewPort.Height/Camera::viewPort.Width * 1.33f; 

	fovx *= 0.5f;
	fovy *= 0.5f;

	float tanx = tan( D3DXToRadian(fovx) );
	float tany = tan( D3DXToRadian(fovy) );

	// left plane
    Vector n = (vpn * tanx) + vright;
    D3DXVec3Normalize( &n, &n );
	Camera::frustrum[0].a = n.x;
    Camera::frustrum[0].b = n.y;
    Camera::frustrum[0].c = n.z;
	
    // right plane
    n = (vpn * tanx) - vright;
    D3DXVec3Normalize( &n, &n );
	Camera::frustrum[1].a = n.x;
    Camera::frustrum[1].b = n.y;
    Camera::frustrum[1].c = n.z;

    // bottom plane
    n = (vpn * tany) + vup;
    D3DXVec3Normalize( &n, &n );
	Camera::frustrum[2].a = n.x;
    Camera::frustrum[2].b = n.y;
    Camera::frustrum[2].c = n.z;

    // top plane
    n = (vpn * tany) - vup;
    D3DXVec3Normalize( &n, &n );
	Camera::frustrum[3].a = n.x;
    Camera::frustrum[3].b = n.y;
    Camera::frustrum[3].c = n.z;

	for( unsigned int i=0; i < 4; i++ )
	{
        n.x = Camera::frustrum[i].a;
        n.y = Camera::frustrum[i].b;
        n.z = Camera::frustrum[i].c;
		Camera::frustrum[i].d = D3DXVec3Dot( &n, &r_origin );
	}
}

void Engine::endEnvironmentMap(void)
{
    _dxCR( _renderToSurface->EndScene( D3DX_FILTER_NONE ) );
    _renderToSurface->Release();
    _renderToSurface = NULL;
    _envMap = NULL;
}