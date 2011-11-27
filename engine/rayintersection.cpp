
#include "headers.h"
#include "intersection.h"
#include "collision.h"

/**
 * class implementation
 */

RayIntersection::RayIntersection(void)
{
    _ray.start = Vector( 0,0,0 );
    _ray.end   = Vector( 0,1,0 );
    _bsp = NULL;
}

RayIntersection::~RayIntersection(void)
{
}

/**
 * IRayIntersection implementation
 */

void RayIntersection::release(void)
{
    delete this;
}

void RayIntersection::setRay(const Vector3f& start, const Vector3f& direction)
{
    _ray.start = wrap( start );
    _ray.end   = wrap( direction );
}

void RayIntersection::intersect(engine::IBSP* bsp, engine::CollisionCallBack callBack, void* data)
{
    _bsp = dynamic_cast<BSP*>( bsp ); assert( _bsp );
    _callBack = callBack;
    _callBackData = data;
    _bspSector = NULL;
    _atomic = NULL;

    collideBSPSector( _bsp->getRoot() );
}

BSPSector* RayIntersection::collideBSPSector(BSPSector* sector)
{
    if( intersectionRayAABB( &_ray, sector->getBoundingBox() ) )
    {
        // is this a leaf sector?
        if( !sector->_leftSubset )
        {
            if( sector->_geometry )
            {
                _bspSector = sector;
                // collide octree sectors
                assert( sector->_geometry->getOcTreeRoot() );
                _geometry  = sector->_geometry;
                _vertices  = _geometry->getVertices();
                _triangles = _geometry->getTriangles();
                if( !collideBSPOcTreeSector( sector->_geometry->getOcTreeRoot() ) ) return NULL;
                _bspSector = NULL;
            }
        }
        else
        {
            if( !collideBSPSector( sector->_leftSubset ) ) return NULL;
            if( !collideBSPSector( sector->_rightSubset ) ) return NULL;
        }
    }
    return sector;
}

OcTreeSector* RayIntersection::collideBSPOcTreeSector(OcTreeSector* ocTreeSector)
{
    if( ocTreeSector->_triangles.size() )
    {
        // collide octree sector triangles
        Triangle* triangle;
        Vector    v0v1, v0v2, n;
        Vector    hitPoint;
        for( unsigned int i=0; i<ocTreeSector->_triangles.size(); i++ )
        {
            triangle = _triangles + ocTreeSector->_triangles[i];
            if( ::intersectionRayTriangle(
                      &_ray, 
                      _vertices + triangle->vertexId[0],
                      _vertices + triangle->vertexId[1],
                      _vertices + triangle->vertexId[2],
                      &hitPoint,
                      &_collisionTriangle.distance
              ))
            {
                _collisionTriangle.vertices[0] = wrap( _vertices[triangle->vertexId[0]] );
                _collisionTriangle.vertices[1] = wrap( _vertices[triangle->vertexId[1]] );
                _collisionTriangle.vertices[2] = wrap( _vertices[triangle->vertexId[2]] );
                v0v1 = _vertices[triangle->vertexId[1]] - _vertices[triangle->vertexId[0]];
                v0v2 = _vertices[triangle->vertexId[2]] - _vertices[triangle->vertexId[0]];
                D3DXVec3Cross( &n, &v0v1, &v0v2 );
                D3DXVec3Normalize( &n, &n );
                _collisionTriangle.normal = wrap( n );
                _collisionTriangle.collisionPoint = wrap( hitPoint );
                _collisionTriangle.shader = _geometry->shader( triangle->shaderId );
                _collisionTriangle.triangleId = ocTreeSector->_triangles[i];
                if( !_callBack( &_collisionTriangle, _bspSector, NULL, _callBackData ) ) return NULL;
                return ocTreeSector;
            }
        }
    }
    else if( ocTreeSector->_subtree[0] )
    {
        for( unsigned int i=0; i<8; i++ )
        {
            if( intersectionRayAABB( &_ray, &ocTreeSector->_subtree[i]->_boundingBox ) )
            {
                if( !collideBSPOcTreeSector( ocTreeSector->_subtree[i] ) ) return NULL;
            }
        }
    }
    return ocTreeSector;
}

void RayIntersection::intersect(engine::IAtomic* atomic, engine::CollisionCallBack callBack, void* data)
{    
    _callBack = callBack;
    _callBackData = data;
    _bsp = NULL;
    _bspSector = NULL;
    _atomic = dynamic_cast<Atomic*>( atomic ); assert( _atomic );
   
    assert( _atomic->_geometry->getOcTreeRoot() );
    _geometry  = _atomic->_geometry;
    _vertices  = _geometry->getVertices();
    _triangles = _geometry->getTriangles();

    if( _atomic->_frame->isDirtyHierarchy() )
    {
        _atomic->_frame->synchronizeSafe();
    }
    
    // transform ray to atomic space
    Matrix iLTM;
    D3DXMatrixInverse( &iLTM, NULL, &_atomic->_frame->LTM );
    D3DXVec3TransformCoord( &_asRay.start, &_ray.start, &iLTM );
    D3DXVec3TransformNormal( &_asRay.end, &_ray.end, &iLTM );

    collideAtomicOcTreeSector( _geometry->getOcTreeRoot() );
}

OcTreeSector* RayIntersection::collideAtomicOcTreeSector(OcTreeSector* ocTreeSector)
{
    if( ocTreeSector->_triangles.size() )
    {
        // collide octree sector triangles
        Triangle* triangle;
        Vector    v0v1, v0v2, n;
        Vector    hitPoint;
        Vector    temp;
        for( unsigned int i=0; i<ocTreeSector->_triangles.size(); i++ )
        {
            triangle = _triangles + ocTreeSector->_triangles[i];
            if( ::intersectionRayTriangle(
                      &_asRay, 
                      _vertices + triangle->vertexId[0],
                      _vertices + triangle->vertexId[1],
                      _vertices + triangle->vertexId[2],
                      &hitPoint,
                      &_collisionTriangle.distance
              ))
            {
                D3DXVec3TransformCoord( &temp, _vertices + triangle->vertexId[0], &_atomic->_frame->LTM );
                _collisionTriangle.vertices[0] = wrap( temp );
                D3DXVec3TransformCoord( &temp, _vertices + triangle->vertexId[1], &_atomic->_frame->LTM );
                _collisionTriangle.vertices[1] = wrap( temp );
                D3DXVec3TransformCoord( &temp, _vertices + triangle->vertexId[2], &_atomic->_frame->LTM );
                _collisionTriangle.vertices[2] = wrap( temp );
                v0v1 = _vertices[triangle->vertexId[1]] - _vertices[triangle->vertexId[0]];
                v0v2 = _vertices[triangle->vertexId[2]] - _vertices[triangle->vertexId[0]];
                D3DXVec3Cross( &n, &v0v1, &v0v2 );
                D3DXVec3Normalize( &n, &n );
                D3DXVec3TransformNormal( &temp, &n, &_atomic->_frame->LTM );
                _collisionTriangle.normal = wrap( temp );
                D3DXVec3TransformCoord( &temp, &hitPoint, &_atomic->_frame->LTM );
                _collisionTriangle.collisionPoint = wrap( temp );
                _collisionTriangle.shader = _geometry->shader( triangle->shaderId );
                _collisionTriangle.triangleId = ocTreeSector->_triangles[i];
                if( !_callBack( &_collisionTriangle, NULL, _atomic, _callBackData ) ) return NULL;
            }
        }
    }
    else if( ocTreeSector->_subtree[0] )
    {
        for( unsigned int i=0; i<8; i++ )
        {
            if( intersectionRayAABB( &_asRay, &ocTreeSector->_subtree[i]->_boundingBox ) )
            {
                if( !collideAtomicOcTreeSector( ocTreeSector->_subtree[i] ) ) return NULL;
            }
        }
    }
    return ocTreeSector;
}

void RayIntersection::intersect(Geometry* geometry, engine::CollisionCallBack callBack, void* data)
{
    _callBack = callBack;
    _callBackData = data;
    _bsp = NULL;
    _bspSector = NULL;
    _atomic = NULL;
       
    _geometry  = geometry;
    _vertices  = _geometry->getVertices();
    _triangles = _geometry->getTriangles();
    _asRay = _ray;

    collideGeometryOcTreeSector( _geometry->getOcTreeRoot() );
}

OcTreeSector* RayIntersection::collideGeometryOcTreeSector(OcTreeSector* ocTreeSector)
{
    if( !ocTreeSector ) 
    {
        return NULL;
    }
    if( ocTreeSector->_triangles.size() )
    {
        // collide octree sector triangles
        Triangle* triangle;
        Vector    v0v1, v0v2, n;
        Vector    hitPoint;
        Vector    temp;
        for( unsigned int i=0; i<ocTreeSector->_triangles.size(); i++ )
        {
            triangle = _triangles + ocTreeSector->_triangles[i];
            if( ::intersectionRayTriangle(
                      &_asRay, 
                      _vertices + triangle->vertexId[0],
                      _vertices + triangle->vertexId[1],
                      _vertices + triangle->vertexId[2],
                      &hitPoint,
                      &_collisionTriangle.distance
              ))
            {
                _collisionTriangle.vertices[0] = wrap( _vertices[triangle->vertexId[0]] );
                _collisionTriangle.vertices[1] = wrap( _vertices[triangle->vertexId[1]] );
                _collisionTriangle.vertices[2] = wrap( _vertices[triangle->vertexId[2]] );
                v0v1 = _vertices[triangle->vertexId[1]] - _vertices[triangle->vertexId[0]];
                v0v2 = _vertices[triangle->vertexId[2]] - _vertices[triangle->vertexId[0]];
                D3DXVec3Cross( &n, &v0v1, &v0v2 );
                D3DXVec3Normalize( &n, &n );
                _collisionTriangle.normal = wrap( n );
                _collisionTriangle.collisionPoint = wrap( hitPoint );
                _collisionTriangle.shader = _geometry->shader( triangle->shaderId );
                _collisionTriangle.triangleId = ocTreeSector->_triangles[i];
                if( !_callBack( &_collisionTriangle, NULL, _atomic, _callBackData ) ) return NULL;
            }
        }
    }
    else if( ocTreeSector->_subtree[0] )
    {
        for( unsigned int i=0; i<8; i++ )
        {
            if( intersectionRayAABB( &_asRay, &ocTreeSector->_subtree[i]->_boundingBox ) )
            {
                if( !collideGeometryOcTreeSector( ocTreeSector->_subtree[i] ) ) return NULL;
            }
        }
    }
    return ocTreeSector;
}