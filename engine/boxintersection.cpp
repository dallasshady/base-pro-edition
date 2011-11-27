
#include "headers.h"
#include "intersection.h"
#include "collision.h"
#include "wire.h"

#define AABBVERTEX(TARGET,AABB,INDEX)\
    TARGET.x = INDEX & 1 ? AABB.sup.x : AABB.inf.x,\
    TARGET.y = INDEX & 2 ? AABB.sup.y : AABB.inf.y,\
    TARGET.z = INDEX & 4 ? AABB.sup.z : AABB.inf.z;

unsigned int edgeStartId[12] = { 0, 1, 3, 2, 4, 5, 7, 6, 0, 1, 2, 3 };
unsigned int edgeEndId[12]   = { 1, 3, 2, 0, 5, 7, 6, 4, 4, 5, 6, 7 };

bool Engine::intersectOBB(const engine::BoundingBox& obb1, const engine::BoundingBox& obb2)
{
    AABB box1( wrap( obb1.inf ), wrap( obb1.sup ) );
    Matrix world1 = wrap( obb1.matrix );
    AABB aabb1;
    aabb1.calculate( &box1, &world1 );

    AABB box2( wrap( obb2.inf ), wrap( obb2.sup ) );
    Matrix world2 = wrap( obb2.matrix );
    AABB aabb2;
    aabb2.calculate( &box2, &world2 );

    // first test : intersect aabbs
    if( !intersectionAABBAABB( &aabb1, &aabb2 ) ) return false;

    // calculate transformation matrices
    Matrix iworld1, iworld2;
    D3DXMatrixInverse( &iworld1, NULL, &world1 );
    D3DXMatrixInverse( &iworld2, NULL, &world2 );
    Matrix t12, t21;
    D3DXMatrixMultiply( &t12, &world1, &iworld2 );
    D3DXMatrixMultiply( &t21, &world2, &iworld1 );

    Vector vertex[8], temp;
    Line edge;

    // retrieve vertices of obb1 in object-space of obb2    
    for( unsigned int i=0; i<8; i++ ) 
    {
        AABBVERTEX( temp, box1, i );
        D3DXVec3TransformCoord( vertex+i, &temp, &t12 );
        // simply, test intersection of aabb & corresponding vertex
        if( box2.isInside( vertex[i] ) ) return true;
    }
    // pass edges of obb1    
    for( i=0; i<12; i++ )
    {
        edge.start = vertex[edgeStartId[i]];
        edge.end = vertex[edgeEndId[i]];
        // intersection test
        if( intersectionLineAABB( &edge, &box2 ) ) return true;
    }

    // retrieve vertices of obb2 in object-space of obb1
    for( i=0; i<8; i++ ) 
    {
        AABBVERTEX( temp, box2, i );
        D3DXVec3TransformCoord( vertex+i, &temp, &t21 );
        // simply, test intersection of aabb & corresponding vertex
        if( box1.isInside( vertex[i] ) ) return true;
    }
    // pass edges of obb1    
    for( i=0; i<12; i++ )
    {
        edge.start = vertex[edgeStartId[i]];
        edge.end = vertex[edgeEndId[i]];
        // intersection test
        if( intersectionLineAABB( &edge, &box1 ) ) return true;
    }

    return false;
}