
#include "headers.h"
#include "camera.h"
#include "wire.h"
#include "sprite.h"

/**
 * current camera properties
 */

Camera*      Camera::_currentCamera = NULL;
float        Camera::_prevNearClipPlane;
float        Camera::_prevFarClipPlane;
D3DVIEWPORT9 Camera::viewPort;
Matrix       Camera::viewMatrix;
Matrix       Camera::projectionMatrix;
Vector       Camera::eyePos;
Vector       Camera::eyeDirection;
D3DXPLANE    Camera::frustrum[6];
float        Camera::fov;
float        Camera::nearClipPlane;
float        Camera::farClipPlane;

/**
 * creation routine
 */

engine::ICamera* Engine::createCamera(unsigned int width, unsigned int height)
{
    return new ScreenCamera( width, height );
}

/**
 * Camera
 */

void Camera::updateProjection(void)
{
    D3DXMatrixPerspectiveFovRH(
        &_projection, 
        _fov * D3DX_PI / 180.0f, 
        float( _viewPort.Width ) / float( _viewPort.Height ), 
        _nearClipPlane, 
        _farClipPlane
    );
}

void Camera::updateFrustrum(void)
{
    // frustrum planes
    Vector r_origin( _frame->LTM._41, _frame->LTM._42, _frame->LTM._43 );
    Vector vpn( _frame->LTM._31, _frame->LTM._32, _frame->LTM._33 );
    Vector vright( _frame->LTM._11, _frame->LTM._12, _frame->LTM._13 );
    Vector vup( _frame->LTM._21, _frame->LTM._22, _frame->LTM._23 );

    //r_origin = Vector( 0,0,0 );
    //vright = Vector( 1,0,0 );
    //vup    = Vector( 0,1,0 );
    //vpn    = Vector( 0,0,1 );

    // for right-handed coordinate system
    vpn *= - 1, vright *= -1, vup *= -1;

	float orgOffset = D3DXVec3Dot( &r_origin, &vpn );
    
    // far plane
    frustrum[4].a = -vpn.x;
    frustrum[4].b = -vpn.y;
    frustrum[4].c = -vpn.z;
	frustrum[4].d = -_farClipPlane - orgOffset;
    
    // near plane
	frustrum[5].a = vpn.x;
    frustrum[5].b = vpn.y;
    frustrum[5].c = vpn.z;
	frustrum[5].d = _nearClipPlane + orgOffset;

    // (1.33) is reserve multiplier
    float fovx = _fov * 1.33f; 
    // (1.1) is reserve multiplier
	float fovy = fovx * _viewPort.Height/_viewPort.Width * 1.1f; 

	fovx *= 0.5f;
	fovy *= 0.5f;

	float tanx = tan( D3DXToRadian(fovx) );
	float tany = tan( D3DXToRadian(fovy) );

	// left plane
    Vector n = (vpn * tanx) + vright;
    D3DXVec3Normalize( &n, &n );
	frustrum[0].a = n.x;
    frustrum[0].b = n.y;
    frustrum[0].c = n.z;
	
    // right plane
    n = (vpn * tanx) - vright;
    D3DXVec3Normalize( &n, &n );
	frustrum[1].a = n.x;
    frustrum[1].b = n.y;
    frustrum[1].c = n.z;

    // bottom plane
    n = (vpn * tany) + vup;
    D3DXVec3Normalize( &n, &n );
	frustrum[2].a = n.x;
    frustrum[2].b = n.y;
    frustrum[2].c = n.z;

    // top plane
    n = (vpn * tany) - vup;
    D3DXVec3Normalize( &n, &n );
	frustrum[3].a = n.x;
    frustrum[3].b = n.y;
    frustrum[3].c = n.z;

	for( unsigned int i=0; i < 4; i++ )
	{
        n.x = frustrum[i].a;
        n.y = frustrum[i].b;
        n.z = frustrum[i].c;
		frustrum[i].d = D3DXVec3Dot( &n, &r_origin );
	}
}
   
engine::IFrame* Camera::getFrame(void)
{
    return _frame;
}

void Camera::setFrame(engine::IFrame* frame)
{
    _frame = dynamic_cast<Frame*>( frame );
}

float Camera::getNearClipPlane(void)
{
    return _nearClipPlane;
}

void Camera::setNearClipPlane(float nearClip)
{
    _nearClipPlane = nearClip;
    updateProjection();
}

float Camera::getFarClipPlane(void)
{
    return _farClipPlane;
}

void Camera::setFarClipPlane(float farClip)
{
    _farClipPlane = farClip;
    updateProjection();
}

float Camera::getFOV(void)
{
    return _fov;
}

void Camera::setFOV(float fov)
{
    _fov = fov;
    updateProjection();
}

void Camera::renderTexture(engine::ITexture* texture)
{
    Texture* t = dynamic_cast<Texture*>( texture ); assert( t );

    t->apply( 0 );
    dxSetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

    dxRenderRect( 0, 0, _viewPort.Width, _viewPort.Height, white  );
}

void Camera::renderTextureAdditive(engine::ITexture* texture)
{
	Texture* t = dynamic_cast<Texture*>( texture ); assert( t );

    t->apply( 0 );
    dxSetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 );
    dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 );
    dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 );
    dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );

	_dxCR( dxSetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) );
	_dxCR( dxSetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE ) );
    _dxCR( dxSetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE ) );
    _dxCR( dxSetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD ) );

    dxRenderRect( 0, 0, _viewPort.Width, _viewPort.Height, white  );

	_dxCR( dxSetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA ) );
    _dxCR( dxSetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ) );
}

void Camera::buildPickRay(float x, float y, Vector3f& start, Vector3f& dir)
{
    Vector v;
    v.x =  ( ( ( 2.0f * x ) / _viewPort.Width ) - 1 ) / _projection._11;
    v.y = -( ( ( 2.0f * y ) / _viewPort.Height ) - 1 ) / _projection._22;
    v.z =  1.0f;

    Vector rayOrigin,rayDir;
    rayDir.x = v.x*_frame->LTM._11 + v.y*_frame->LTM._21 + v.z*_frame->LTM._31;
    rayDir.y = v.x*_frame->LTM._12 + v.y*_frame->LTM._22 + v.z*_frame->LTM._32;
    rayDir.z = v.x*_frame->LTM._13 + v.y*_frame->LTM._23 + v.z*_frame->LTM._33;
    rayOrigin.x = _frame->LTM._41;
    rayOrigin.y = _frame->LTM._42;
    rayOrigin.z = _frame->LTM._43;

    start = wrap( rayOrigin );
    dir = wrap( rayDir );
}

Vector3f Camera::projectPosition(const Vector3f& position)
{
    assert( _frame );
    _frame->getLTM();

    Vector p = wrap( position );
    Vector s;
    Matrix v;

    // view matrix
    D3DXMatrixInverse( &v, NULL, &_frame->LTM );
    // projection routine
    D3DXVec3Project( &s, &p, &_viewPort, &_projection, &v, &identity );

    return wrap( s );
}

Camera* Camera::getCurrentCamera()
{
    return _currentCamera;
}

/**
 * stencil shadow ssupport
 */

void Camera::beginStencilShadows(float nearClipPlane, float farClipPlane)
{
    assert( _currentCamera );

    Camera* camera = _currentCamera;

    // end rendering
    camera->endScene();

    // switch near clip plane
    _prevNearClipPlane = camera->getNearClipPlane();
    _prevFarClipPlane  = camera->getFarClipPlane();
    camera->setNearClipPlane( nearClipPlane );
    camera->setFarClipPlane( farClipPlane );

    // continue rendering
    camera->beginScene( 0, Vector4f( 0,0,0,0 ) );
}

void Camera::endStencilShadows(void)
{
    assert( _currentCamera );

    Camera* camera = _currentCamera;

    // end rendering
    camera->endScene();

    // switch near clip plane
    camera->setNearClipPlane( _prevNearClipPlane );
    camera->setFarClipPlane( _prevFarClipPlane );

    // restart rendering
    camera->beginScene( 0, Vector4f( 0,0,0,0 ) );
}

/**
 * ScreenCamera
 */

ScreenCamera::ScreenCamera(unsigned int width, unsigned int height)
{
    _frame = NULL;

    // setup viewport
    _viewPort.X = 0, _viewPort.Y = 0;
    _viewPort.Width  = width,
    _viewPort.Height = height;
    _viewPort.MinZ   = 0;
    _viewPort.MaxZ   = 1;
    _fov = 60.0f;
    updateProjection();
}

ScreenCamera::~ScreenCamera()
{
    if( _frame ) _frame->release();
}

void ScreenCamera::release(void)
{
    delete this;
}

void ScreenCamera::beginScene(unsigned int clearMode, const Vector4f& clearColor)
{
    assert( Camera::_currentCamera == NULL );
    Camera::_currentCamera = this;

    // frame synchronization phase
    Frame::synchronizeAll();

    _dxCR( iDirect3DDevice->SetViewport( &_viewPort ) );

    if( clearMode ) _dxCR( iDirect3DDevice->Clear( 0, NULL, clearMode, wrap( clearColor ), 1.0f, 0L ) );
    
    _dxCR( iDirect3DDevice->BeginScene() );

    // setup transformation matrices
    if( _frame ) 
    {
        D3DXMatrixInverse( &viewMatrix, NULL, &_frame->LTM );
        _dxCR( iDirect3DDevice->SetTransform( D3DTS_VIEW, &viewMatrix ) );
    }
    projectionMatrix = _projection;
    _dxCR( iDirect3DDevice->SetTransform( D3DTS_PROJECTION, &_projection ) );
    
    viewPort      = _viewPort;
    eyePos        = dxPos( &_frame->LTM );
    eyeDirection  = dxAt( &_frame->LTM );    
    fov           = _fov;
    nearClipPlane = _nearClipPlane;
    farClipPlane  = _farClipPlane;

    // normalize camera eye
    D3DXVec3Normalize( &eyeDirection, &eyeDirection );

    // update frustrum planes
    updateFrustrum();
}

void ScreenCamera::endScene(void)
{
    assert( Camera::_currentCamera == this );
    Camera::_currentCamera = NULL;

    _dxCR( iDirect3DDevice->EndScene() );
}