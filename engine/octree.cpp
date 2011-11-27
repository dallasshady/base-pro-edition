
#include "headers.h"
#include "geometry.h"
#include "collision.h"
#include "wire.h"
#include "asset.h"

OcTreeSector::OcTreeSector(OcTreeSector* parent, AABB boundingBox, Geometry* geometry)
{
    _parent       = parent;
    _geometry     = geometry;
    _boundingBox  = boundingBox;
    memset( _subtree, 0, sizeof(OcTreeSector*) * 8 );

    // build octree sector triangle list        
    if( !parent )
    {
        for( int i=0; i<geometry->getNumTriangles(); i++ ) _triangles.push_back(i);
    }
    else
    {
        Triangle* triangle;
        for( unsigned int i=0; i<parent->_triangles.size(); i++ )
        {
            triangle = geometry->getTriangles() + parent->_triangles[i];
            if( intersectionTriangleAABB(
                    geometry->getVertices() + triangle->vertexId[0],
                    geometry->getVertices() + triangle->vertexId[1],
                    geometry->getVertices() + triangle->vertexId[2],
                    &_boundingBox
            ) )
            {
                _triangles.push_back( parent->_triangles[i] );
            }
        }
    }

    if( ( _triangles.size() >= OCTREESECTOR_CAPACITY ) &&
        ( D3DXVec3Length( &_boundingBox.getDiagonal(0) ) > OCTREESECTOR_SIZE ) )
    {
        // build subtree
        AABB bsp1, bsp2;
        _boundingBox.divide( bsp1, bsp2, AABB::maxPlane );
        AABB qsp1, qsp2, qsp3, qsp4;
        bsp1.divide( qsp1, qsp2, AABB::maxPlane );
        bsp2.divide( qsp3, qsp4, AABB::maxPlane );
        AABB osp1, osp2, osp3, osp4, osp5, osp6, osp7, osp8;
        qsp1.divide( osp1, osp2, AABB::maxPlane );
        qsp2.divide( osp3, osp4, AABB::maxPlane );
        qsp3.divide( osp5, osp6, AABB::maxPlane );
        qsp4.divide( osp7, osp8, AABB::maxPlane );

        _subtree[0] = new OcTreeSector( this, osp1, geometry );
        _subtree[1] = new OcTreeSector( this, osp2, geometry );
        _subtree[2] = new OcTreeSector( this, osp3, geometry );
        _subtree[3] = new OcTreeSector( this, osp4, geometry );
        _subtree[4] = new OcTreeSector( this, osp5, geometry );
        _subtree[5] = new OcTreeSector( this, osp6, geometry );
        _subtree[6] = new OcTreeSector( this, osp7, geometry );
        _subtree[7] = new OcTreeSector( this, osp8, geometry );

        _triangles.clear();
    }
}

OcTreeSector::OcTreeSector(Chunk* chunk, IResource* resource, AssetObjectM& assetObjects)
{
    memset( _subtree, 0, sizeof(OcTreeSector*) * 8 );

    _parent = NULL;
    AssetObjectI assetObjectI = assetObjects.find( chunk->parentId );
    if( assetObjectI != assetObjects.end() )
    {
        _parent = reinterpret_cast<OcTreeSector*>( assetObjectI->second );
        if( _parent ) for( int i=0; i<8; i++ )
        {
            if( _parent->_subtree[i] == NULL ) 
            {
                _parent->_subtree[i] = this;
                break;
            }
        }
    }

    assetObjectI = assetObjects.find( chunk->geometryId );
    assert( assetObjectI != assetObjects.end() );
    _geometry = reinterpret_cast<Geometry*>( assetObjectI->second );

    _boundingBox = chunk->boundingBox;

    if( chunk->numTriangles )
    {
        _triangles.resize( chunk->numTriangles );

        ChunkHeader trianglesHeader( resource );
        if( trianglesHeader.type != BA_BINARY ) throw Exception( "Unexpected chunk type" );
        if( trianglesHeader.size != sizeof(int) * chunk->numTriangles ) throw Exception( "Incompatible binary asset version" );
        fread( &_triangles[0], sizeof(int), chunk->numTriangles, resource->getFile() );
    }
}

OcTreeSector::~OcTreeSector()
{
    if( _subtree[0] ) for( int i=0; i<8; i++ ) delete _subtree[i];
}

/**
 * behaviour
 */

int OcTreeSector::getOcTreeSize(void)
{
    if( _subtree[0] == NULL ) return 1;
    
    int subTreeSize = 0;
    for( int i=0; i<8; i++ ) subTreeSize += _subtree[i]->getOcTreeSize();
    return 1 + subTreeSize;
}

bool OcTreeSector::checkConsistency(void)
{
    if( _subtree[0] == NULL ) return true;
    for( int i=0; i<8; i++ )
    {
        if( _subtree[i] == NULL || _subtree[i]->checkConsistency() == false )
        {
            return false;
        }
    }
    return true;
}

void OcTreeSector::write(IResource* resource)
{
    ChunkHeader ocTreeHeader( BA_OCTREE, sizeof( Chunk ) );
    ocTreeHeader.write( resource );

    Chunk chunk;
    chunk.id           = (auid)( this );
    chunk.parentId     = (auid)( this->_parent );
    chunk.geometryId   = (auid)( this->_geometry );
    chunk.boundingBox  = _boundingBox;
    chunk.numTriangles = _triangles.size();

    fwrite( &chunk, sizeof( Chunk ), 1, resource->getFile() );

    // write triangle list
    if( _triangles.size() )
    {
        ChunkHeader trianglesHeader( BA_BINARY, sizeof(int) * _triangles.size() );
        trianglesHeader.write( resource );
        fwrite( &_triangles[0], sizeof(int), _triangles.size(), resource->getFile() );
    }

    // write subtree
    if( _subtree[0] ) for( int i=0; i<8; i++ )
    {
        _subtree[i]->write( resource );
    }
}

AssetObjectT OcTreeSector::read(IResource* resource, AssetObjectM& assetObjects)
{
    ChunkHeader ocTreeHeader( resource );
    if( ocTreeHeader.type != BA_OCTREE ) throw Exception( "Unexpected chunk type" );
    if( ocTreeHeader.size != sizeof(Chunk) ) throw Exception( "Incompatible binary asset version" );

    Chunk chunk;
    fread( &chunk, sizeof(Chunk), 1, resource->getFile() );

    OcTreeSector* ocTreeSector = new OcTreeSector( &chunk, resource, assetObjects );

    return AssetObjectT( chunk.id, ocTreeSector );
}

/**
 * intersection query
 */

OcTreeSector* OcTreeSector::forAllIntersections(Line* ray, OcTreeSectorRayIntersectionCallBack* callBack)
{    
    if( _triangles.size() )
    {
        return callBack( ray, this );
    }
    else if( _subtree[0] )
    {
        for( int i=0; i<8; i++ )
        {
            if( intersectionRayAABB( ray, &_subtree[i]->_boundingBox ) )
            {
                if( !_subtree[i]->forAllIntersections( ray, callBack ) ) return NULL;
            }
        }
    }
    return this;
}
