
#include "headers.h"
#include "intersection.h"
#include "collision.h"

/**
 * class implementation
 */

SphereIntersection::SphereIntersection(void)
{
    _sphere.center.x = _sphere.center.y = _sphere.center.z = 0.0f;
    _sphere.radius = 0.0f;
}

SphereIntersection::~SphereIntersection(void)
{
}

/**
 * ISphereIntersection implementation
 */

void SphereIntersection::release(void)
{
    delete this;
}

void SphereIntersection::setSphere(const Vector3f& center, float radius)
{
    _sphere.center = wrap( center );
    _sphere.radius = radius;
    _bsp       = NULL;
    _bspSector = NULL;
    _atomic    = NULL;
}

void SphereIntersection::intersect(engine::IAtomic* atomic, engine::CollisionCallBack callBack, void* data)
{
    _bsp          = NULL;
    _bspSector    = NULL;
    _atomic       = dynamic_cast<Atomic*>( atomic ); assert( _atomic );
    _callBack     = callBack;
    _callBackData = data;

    assert( _atomic->_geometry->getOcTreeRoot() );
    _geometry  = _atomic->_geometry;
    _vertices  = _geometry->getVertices();
    _triangles = _geometry->getTriangles();

    if( _atomic->_frame->isDirtyHierarchy() ) _atomic->_frame->synchronizeSafe();
    
    // transform ray to atomic space
    Matrix iLTM;
    D3DXMatrixInverse( &iLTM, NULL, &_atomic->_frame->LTM );
    D3DXVec3TransformCoord( &_asSphere.center, &_sphere.center, &iLTM );
    _asSphere.radius = _sphere.radius;

    collideAtomicOcTreeSector( _geometry->getOcTreeRoot() );
}

OcTreeSector* SphereIntersection::collideAtomicOcTreeSector(OcTreeSector* ocTreeSector)
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
            if( intersectionSphereTriangle( 
                   &_asSphere, 
                   _vertices + triangle->vertexId[0],
                   _vertices + triangle->vertexId[1],
                   _vertices + triangle->vertexId[2],
                   &hitPoint,
                   &_collisionTriangle.distance
              ) )
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
            if( intersectionSphereAABB( &_asSphere, &ocTreeSector->_subtree[i]->_boundingBox ) )
            {
                if( !collideAtomicOcTreeSector( ocTreeSector->_subtree[i] ) ) return NULL;
            }
        }
    }
    return ocTreeSector;
}

bool SphereIntersection::intersect(
    const Vector3f& v0, 
    const Vector3f& v1, 
    const Vector3f& v2, 
    engine::CollisionTriangle* collisionTriangle
)
{
    Vector vertices[3];
    Vector v0v1, v0v2, n;
    Vector hitPoint;

    vertices[0] = wrap( v0 ), vertices[1] = wrap( v1 ), vertices[2] = wrap( v2 );

    if( intersectionSphereTriangle( 
            &_sphere, 
            vertices,
            vertices + 1,
            vertices + 2,
            &hitPoint,
            &collisionTriangle->distance
      ) )
    {
        v0v1 = vertices[1] - vertices[0];
        v0v2 = vertices[2] - vertices[0];
        D3DXVec3Cross( &n, &v0v1, &v0v2 );
        D3DXVec3Normalize( &n, &n );
        collisionTriangle->vertices[0] = v0;
        collisionTriangle->vertices[1] = v1;
        collisionTriangle->vertices[2] = v2;
        collisionTriangle->collisionPoint = wrap( hitPoint );
        collisionTriangle->shader = NULL;
        collisionTriangle->triangleId = -1;
        return true;
    }
    else
    {
        memset( collisionTriangle, 0, sizeof( engine::CollisionTriangle ) );
        collisionTriangle->triangleId = -1;
        return false;
    }
}