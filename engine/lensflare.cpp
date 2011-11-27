
#include "headers.h"
#include "bsp.h"
#include "asset.h"
#include "wire.h"
#include "collision.h"
#include "camera.h"
#include "sprite.h"
#include "intersection.h"

static BSP*            _bsp;
static RayIntersection _rayIntersection;

engine::CollisionTriangle* findAnyIntersectionCB(
    engine::CollisionTriangle* collTriangle,
    engine::IBSPSector* sector,
    engine::IAtomic* atomic,
    void* data
)
{
    *((bool*)(data)) = true;
    return NULL;
}

void BSP::renderLensFlares(void)
{
    _bsp = this;
    for( LightI lightI = _lights.begin(); lightI != _lights.end(); lightI++ )
    {
        renderLensFlares( *lightI );
    }
    for( ClumpI clumpI = _clumps.begin(); clumpI != _clumps.end(); clumpI++ )
    {
        (*clumpI)->forAllLights( renderLensFlaresCB, NULL );
    }
}

engine::ILight* BSP::renderLensFlaresCB(engine::ILight* light, void* data)
{
    renderLensFlares( dynamic_cast<Light*>( light ) );
    return light;
}

/**
 * effect settings
 */

static const char* name[7] = {
    "flare1",
    "flare2",
    "flare3",
    "flare4",
    "flare5",
    "flare6",
    "flare7"
};

static float weight[7] = { 0.6f, 0.5f, 0.4f, 0.3f, 0.2f, 0.1f, 0.0f };
static float size[7] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

void BSP::renderLensFlares(Light* light)
{
    // lens flares are a vaiable only for point lights
    if( light->getType() != engine::ltPoint || light->getLightset() != 0 ) 
    {
        return;
    }

    // make view (rotation only) / projection matrix
    Matrix viewProj;
    D3DXMatrixMultiply( &viewProj, &Camera::viewMatrix, &Camera::projectionMatrix );

    // calculate direction from camera position to light position
    Vector cl = light->position() - Camera::eyePos;

    // collide direction ray with BSP
    bool isIntersected = false;
    _rayIntersection.setRay( wrap( Camera::eyePos ), wrap( cl ) );
    _rayIntersection.intersect( _bsp, findAnyIntersectionCB, &isIntersected );
    if( isIntersected ) return;

    // normalize direction
    D3DXVec3Normalize( &cl, &cl );

    // calculate effect culling value
    // this value is depends from the camera field-of-view
    float minDp = cos( Camera::fov * D3DX_PI / 180.0f );

    // dot product btw. "cl" vector & camera direction gives us the inclination
    // of camera direction from the local light direction
    float dp = -D3DXVec3Dot( &cl, &Camera::eyeDirection );

    // cull effect by the camera field-of-view
    if( dp > minDp )
    {
        Vector lpT;
        D3DXVec3Project( 
            &lpT, 
            &light->position(), 
            &Camera::viewPort,
            &Camera::projectionMatrix,
            &Camera::viewMatrix,
            &identity
        );
        
        Flector screenCenter( 
            float( Engine::instance->screenWidth ) / 2,
            float( Engine::instance->screenHeight ) / 2
        );

        Flector flareOffset( lpT.x, lpT.y );

        float inclK = ( 1.0f - dp ) / ( 1.0f - minDp );

        // default flare size
        float defaultSize = Engine::instance->screenHeight * 0.25f;

        // effect color
        Color color = D3DCOLOR_RGBA(
            int( 255 * ( 1.0f - inclK ) ),
            int( 255 * ( 1.0f - inclK ) ),
            int( 255 * ( 1.0f - inclK ) ),
            int( 255 * ( 1.0f - inclK ) )
        );

        // setup render states & texture stages
        _dxCR( dxSetRenderState( D3DRS_COLORVERTEX, TRUE ) );
        _dxCR( dxSetRenderState( D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_SPECULARMATERIALSOURCE, D3DMCS_MATERIAL ) );
        _dxCR( dxSetRenderState( D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1 ) );

        _dxCR( dxSetTextureStageState( 0, D3DTSS_TEXCOORDINDEX, 0 ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1 ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1 ) );
        _dxCR( dxSetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE ) );

        _dxCR( dxSetTextureStageState( 1, D3DTSS_COLOROP, D3DTOP_MODULATE ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_COLORARG1, D3DTA_CURRENT ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_COLORARG2, D3DTA_DIFFUSE ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAOP, D3DTOP_MODULATE ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAARG1, D3DTA_CURRENT ) );
        _dxCR( dxSetTextureStageState( 1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE ) );

        _dxCR( dxSetTextureStageState( 2, D3DTSS_ALPHAOP, D3DTOP_DISABLE ) );
        _dxCR( dxSetTextureStageState( 2, D3DTSS_COLOROP, D3DTOP_DISABLE ) );

        _dxCR( dxSetRenderState( D3DRS_ALPHABLENDENABLE, TRUE ) );
        _dxCR( dxSetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE ) );
        _dxCR( dxSetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE ) );
        _dxCR( dxSetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD ) );

        _dxCR( dxSetRenderState( D3DRS_ZENABLE, FALSE ) );

        // draw flares along the line, formed by the points "screenCenter" & "flareOffset"        
        for( unsigned int i=0; i<7; i++ )
        {
            // retrieve flare texture
            TextureI textureI = Texture::textures.find( name[i] ); 
            assert( textureI != Texture::textures.end() );

            // calculate flare center
            Flector rectC(
                1.5f * weight[i] * ( screenCenter.x - flareOffset.x ) + flareOffset.x,
                1.5f * weight[i] * ( screenCenter.y - flareOffset.y ) + flareOffset.y
            );
            
            // calculate flare size
            Flector rectS( 0.5f*defaultSize*size[i], 0.5f*defaultSize*size[i] );

            // apply texture
            textureI->second->apply(0);

            // render rectangle
            dxRenderRect( 
                unsigned int( rectC.y - rectS.y ), 
                unsigned int( rectC.x - rectS.x ), 
                unsigned int( 2 * rectS.y ), 
                unsigned int( 2 * rectS.x ), 
                color
            );
        }

        // restore critical render states
        _dxCR( dxSetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA ) );
        _dxCR( dxSetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA ) );
        _dxCR( dxSetRenderState( D3DRS_ZENABLE, TRUE ) );
    }
}