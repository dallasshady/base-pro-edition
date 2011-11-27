/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description alpha objects & rought, rapid alpha-sorting
 *
 * @author bad3p
 */

#ifndef ALPHA_SORTING_INCLUDED
#define ALPHA_SORTING_INCLUDED

#include "frame.h"
#include "geometry.h"
#include "engine.h"

struct AlphaGeometry
{
public:
    unsigned char key;          // sorting key
    Frame*        frame;        // transformation frame
    Geometry*     geometry;     // target geometry
    BSPSector*    sector;       // where is the target geometry? - in the sector of BSP tree!
    D3DXMATRIX**  boneMatrices; // bone matrices for skinned geometries
    unsigned int  subsetId;     // identifier specifies the alpha geometry mesh subset
public:
    AlphaGeometry()
    {
        key = 0, frame = NULL, geometry = NULL, subsetId = 0, sector = NULL, boneMatrices = NULL;
    }
    AlphaGeometry(bool n, unsigned char k, Frame* f, Geometry* g, BSPSector* bsps, D3DXMATRIX** bms, unsigned int s)
    {
        key = k, frame = f, geometry = g, sector = bsps, boneMatrices = bms, subsetId = s;
    }
};

class AlphaSorting
{
private:
    unsigned char _nestSize[256];    // corresponding nest size
    AlphaGeometry _nest[256][256];   // 256 nests for objects
    unsigned int  _numItems;         // number of items to sort
    AlphaGeometry _items[65535];     // items to sort
public:
    AlphaSorting() : _numItems(0) {}
public:
    inline unsigned int size(void)
    {
        return _numItems;
    }
    inline void clear(void)
    {
        _numItems = 0;
    }
    inline void add(AlphaGeometry* alphaGeometry)
    {
        assert( _numItems <= 65535 );
        memcpy( _items + _numItems, alphaGeometry, sizeof( AlphaGeometry ) );
        _numItems++;
    }
public:
    void render(void)
    {
        // reset nest counters
        memset( _nestSize, 0, sizeof( _nestSize ) );

        unsigned int   i,j;
        unsigned char  key;
        unsigned char* nestSize;        

        // sort
        for( i=0; i<_numItems; i++ )
        {
            key = _items[i].key;
            nestSize = _nestSize + key;

            // search for nearest thin nest
            // a priori, central nests (lower nestId) are more vacant
            while( *nestSize == 255 && key > 0 ) key--, nestSize = _nestSize + key;
            // leave alpha unsorted & unrendered if there are no space to sort it
            if( *nestSize == 255 ) continue;

            // add alpha to corresponding nest
            memcpy( _nest[key] + *nestSize, _items + i, sizeof(AlphaGeometry) );
            (*nestSize)++;
        }

        AlphaGeometry* nest;

        // render
        for( i=0; i<256; i++ )
        {            
            nestSize = _nestSize + i;
            Engine::statistics.alphaObjectsRendered += *nestSize;
            for( j=0; j<*nestSize; j++ )
            {
                nest = _nest[i] + j;
                // setup world transformation
                if( nest->frame )
                {
                    _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &nest->frame->LTM ) );
                }
                else
                {
                    _dxCR( iDirect3DDevice->SetTransform( D3DTS_WORLD, &identity ) );
                }
        
                // render subset
                if( nest->boneMatrices ) Mesh::pBoneMatrices = nest->boneMatrices;
                nest->geometry->renderAlphaGeometry( nest->subsetId );
            }
        }
    }
};

#endif