
#include "headers.h"
#include "bsp.h"
#include "asset.h"
#include "wire.h"
#include "collision.h"
#include "camera.h"
#include "sprite.h"

/**
 * depth map
 */

void BSP::renderDepthMap(void)
{
    _renderFrameId++;
    Engine::statistics.bspTotal += _root->getNumLeafSectors();

    float minDepth = Camera::nearClipPlane;
    float maxDepth = Camera::farClipPlane;

    dxSetRenderState( D3DRS_SRCBLEND, D3DBLEND_ONE );
    dxSetRenderState( D3DRS_DESTBLEND, D3DBLEND_ZERO );
    dxSetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
    dxSetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );

    // setup linear fog, from z-depth = 0 to z-depth = camera far clipping plane
    dxSetRenderState( D3DRS_FOGENABLE, TRUE );
    dxSetRenderState( D3DRS_FOGCOLOR, D3DCOLOR_RGBA(255,255,255,255) );
    dxSetRenderState( D3DRS_FOGTABLEMODE, D3DFOG_LINEAR );
    dxSetRenderState( D3DRS_FOGSTART, *(DWORD*)(&minDepth) );
    dxSetRenderState( D3DRS_FOGEND,   *(DWORD*)(&maxDepth) );

    // disable light sources
    for( int i=0; i<8; i++ )
    {
        _dxCR( dxLightEnable( i, FALSE ) );
    }

    // render bsp sectors
    sectorRenderDepthMap( _root );

    // reset fog
    dxSetRenderState( D3DRS_FOGENABLE, FALSE );        
}

BSPSector* BSP::sectorRenderDepthMap(BSPSector* sector)
{   
    if( intersectAABBFrustum( &sector->_boundingBox, Camera::frustrum ) )
    {
        if( sector->_leftSubset )
        {
            sector->_lsc = sector->_leftSubset->getBoundingBox()->inf + 0.5f * ( sector->_leftSubset->getBoundingBox()->sup - sector->_leftSubset->getBoundingBox()->inf );
            sector->_rsc = sector->_rightSubset->getBoundingBox()->inf + 0.5f * ( sector->_rightSubset->getBoundingBox()->sup - sector->_rightSubset->getBoundingBox()->inf );
            sector->_lsd = Camera::eyePos - sector->_lsc;
            sector->_rsd = Camera::eyePos - sector->_rsd;

            if( D3DXVec3LengthSq( &sector->_lsd ) < D3DXVec3LengthSq( &sector->_rsd ) )
            {
                sectorRenderDepthMap( sector->_leftSubset );
                sectorRenderDepthMap( sector->_rightSubset );
            }
            else
            {
                sectorRenderDepthMap( sector->_rightSubset );
                sectorRenderDepthMap( sector->_leftSubset );
            }
        }
        else
        {
            sector->renderDepthMap();
        }
    }
    return sector;
}