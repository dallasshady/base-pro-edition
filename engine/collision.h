
#ifndef COLLISION_ROUTINE_INCLUDED
#define COLLISION_ROUTINE_INCLUDED

#include "headers.h"
#include "fundamentals.h"

/**
 * intersection between two AABBs
 */

inline bool intersectionAABBAABB(AABB* aabb1, AABB* aabb2)
{
    if( aabb1->sup.x < aabb2->inf.x ||
		aabb2->sup.x < aabb1->inf.x ||
        aabb1->sup.y < aabb2->inf.y ||
		aabb2->sup.y < aabb1->inf.y ||
        aabb1->sup.z < aabb2->inf.z ||
		aabb2->sup.z < aabb1->inf.z )
    {
        return false;
    }
    return true;
}

/**
 * a simple method for box-sphere intersection testing by Jim Arvo
 * from "Graphics Gems", Academic Press, 1990
 */

bool intersectionSphereAABB(Sphere* sphere, AABB* aabb);

/**
 * a simple method for ray-sphere intersection
 */

bool intersectionRaySphere(Line* ray, Sphere* sphere, float* distance);

/**
 * a fast method for aabb-frustrum intersection
 */

const unsigned int cameraFrustrum = 0x3F;

bool intersectAABBFrustum(AABB* aabb, D3DXPLANE* frustrum);
bool intersectPointFrustum(Vector* point, D3DXPLANE* frustrum);
float getDistance(D3DXPLANE* plane, Vector* point);

/**
 * ray-box intersection by DirectX SDK
 */

inline bool intersectionRayAABB(Line* ray, AABB* aabb)
{
    Vector rayEnd = ray->start + ray->end;
    if( aabb->isInside( ray->start ) || aabb->isInside( rayEnd ) )
    {
        return true;
    }    
    if( ( aabb->sup.x < ray->start.x && aabb->sup.x < rayEnd.x ) ||
        ( aabb->inf.x > ray->start.x && aabb->inf.x > rayEnd.x ) ||
        ( aabb->sup.y < ray->start.y && aabb->sup.y < rayEnd.y ) ||
        ( aabb->inf.y > ray->start.y && aabb->inf.y > rayEnd.y ) ||
        ( aabb->sup.z < ray->start.z && aabb->sup.z < rayEnd.z ) ||
        ( aabb->inf.z > ray->start.z && aabb->inf.z > rayEnd.z ) )
    {
        return false;
    }
    return 0 != D3DXBoxBoundProbe( 
        &aabb->inf, &aabb->sup, 
        &ray->start, &ray->end
    );
}

inline bool intersectionLineAABB(Line* line, AABB* aabb)
{
    return intersectionRayAABB( 
        &Line( line->start, line->end - line->start ),
        aabb
    );
}

/**
 * ray-triangle intersection by DirectX SDK
 */

inline bool intersectionRayTriangle(Line* ray, Vector* v0, Vector* v1, Vector* v2, Vector* hitPoint, float* distance)
{
    float u,v;
    if( 0 != D3DXIntersectTri( 
            v0, v1, v2,
            &ray->start, &ray->end,
            &u, &v, distance
      ) && *distance <= 1.0f )
    {
        *hitPoint = *v0 + ( *v1 - *v0 ) * u + ( *v2 - *v0 ) * v;
        return true;
    }
    return false;
}

inline bool intersectionLineTriangle(Line* line, Vector* v0, Vector* v1, Vector* v2, Vector* hitPoint, float* distance)
{
    float u,v;
    if( 0 != D3DXIntersectTri(
            v0, v1, v2,
            &line->start, &( line->end - line->start ),
            &u, &v, distance
      ) && *distance <= 1.0f )
    {
        *hitPoint = *v0 + ( *v1 - *v0 ) * u + ( *v2 - *v0 ) * v;
        return true;
    }
    return false;
}

/**
 * triangle-aabb intersection based on DirectX SDK
 */

inline bool intersectionTriangleAABB(Vector* v0, Vector* v1, Vector* v2, AABB* aabb)
{
    if( aabb->isInside( *v0 ) || aabb->isInside( *v1 ) || aabb->isInside( *v2 ) ) 
    {
        return true;
    }

    if( ( aabb->sup.x < v0->x && aabb->sup.x < v1->x && aabb->sup.x < v2->x ) ||
        ( aabb->inf.x > v0->x && aabb->inf.x > v1->x && aabb->inf.x > v2->x ) ||
        ( aabb->sup.y < v0->y && aabb->sup.y < v1->y && aabb->sup.y < v2->y ) ||
        ( aabb->inf.y > v0->y && aabb->inf.y > v1->y && aabb->inf.y > v2->y ) ||
        ( aabb->sup.z < v0->z && aabb->sup.z < v1->z && aabb->sup.z < v2->z ) ||
        ( aabb->inf.z > v0->z && aabb->inf.z > v1->z && aabb->inf.z > v2->z ) )
    {
        return false;
    }


    if( 0 != D3DXBoxBoundProbe( &aabb->inf, &aabb->sup, v0, &( *v1 - *v0 ) ) ) 
    {
        return true;
    }

    if( 0 != D3DXBoxBoundProbe( &aabb->inf, &aabb->sup, v0, &( *v2 - *v0 ) ) ) 
    {
        return true;
    }

    if( 0 != D3DXBoxBoundProbe( &aabb->inf, &aabb->sup, v1, &( *v2 - *v1 ) ) ) 
    {
        return true;
    }

    float u,v,d;
    if( 0 != D3DXIntersectTri( v0, v1, v2, &aabb->getCorner(0), &aabb->getDiagonal(0), &u, &v, &d ) )
    {
        if( d <= 1.0f ) return true;
    }

    if( 0 != D3DXIntersectTri( v0, v1, v2, &aabb->getCorner(1), &aabb->getDiagonal(1), &u, &v, &d ) )
    {
        if( d <= 1.0f ) return true;
    }

    if( 0 != D3DXIntersectTri( v0, v1, v2, &aabb->getCorner(2), &aabb->getDiagonal(2), &u, &v, &d ) )
    {
        if( d <= 1.0f ) return true;
    }

    if( 0 != D3DXIntersectTri( v0, v1, v2, &aabb->getCorner(3), &aabb->getDiagonal(3), &u, &v, &d ) )
    {
        if( d <= 1.0f ) return true;
    }

    return false;
}

/**
 * sphere-triangle intersection based on OPCODE (optimized collision detection code)
 */

bool intersectionSphereTriangle(Sphere* sphere, Vector* v0, Vector* v1, Vector* v2, Vector* hitPoint, float* distance);

#endif
