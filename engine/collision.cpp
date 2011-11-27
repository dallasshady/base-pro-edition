
#include "headers.h"
#include "collision.h"
#include "wire.h"

bool intersectionSphereAABB(Sphere* sphere, AABB* aabb)
{
    // solid box - solid sphere
    float dmin = 0;

    if( sphere->center.x < aabb->inf.x )
    {
        dmin += sqr( sphere->center.x - aabb->inf.x ); 
    }
    else if( sphere->center.x > aabb->sup.x ) 
    {
        dmin += sqr( sphere->center.x - aabb->sup.x );
    }

    if( sphere->center.y < aabb->inf.y )
    {
        dmin += sqr( sphere->center.y - aabb->inf.y ); 
    }
    else if( sphere->center.y > aabb->sup.y ) 
    {
        dmin += sqr( sphere->center.y - aabb->sup.y );
    }

    if( sphere->center.z < aabb->inf.z )
    {
        dmin += sqr( sphere->center.z - aabb->inf.z ); 
    }
    else if( sphere->center.z > aabb->sup.z ) 
    {
        dmin += sqr( sphere->center.z - aabb->sup.z );
    }
    
    if( dmin <= sqr( sphere->radius ) ) return( true );
    return( false );
}

static Vector _co;
static float  _a, _b, _c, _d, _root2;
static float  _2a, _sqrtd;

bool intersectionRaySphere(Line* ray, Sphere* sphere, float* distance)
{
    _co = ray->start - sphere->center;
    _a = D3DXVec3Dot( &ray->end, &ray->end );
    _b = 2 * D3DXVec3Dot( &_co, &ray->end );
    _c = D3DXVec3Dot( &_co, &_co ) - sqr( sphere->radius );
    _d = sqr( _b ) - 4 * _a * _c;

    if( _d < 0 ) return false;
    
    _2a = 2 * _a;

    if( _d > 0 )
    {
        _sqrtd = sqrt( _d );
        *distance = ( -_b + _sqrtd ) / _2a;
        _root2 = ( -_b - _sqrtd ) / _2a;
        if( _root2 < *distance ) _root2 = *distance;
    }
    else
    {
        if( _d == 0 ) *distance = -_b * _2a;
    }
    return true;
}

/**
 * intersection of AABB and a frustum
 */

typedef int SideType;

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2

#define distTo(plane,vVec) ( plane->a * vVec.x + plane->b * vVec.y + plane->c * vVec.z - plane->d )
#define getPointSideExact(plane, vPoint) ( distTo( plane, vPoint ) > 0.0f ? SIDE_FRONT : SIDE_BACK )

inline SideType boxOnPlaneSide(D3DXPLANE* plane, AABB* aabb)
{
    Vector vPoint;
    vPoint.x = aabb->inf.x, vPoint.y = aabb->inf.y, vPoint.z = aabb->inf.z;
    int firstSide = getPointSideExact( plane, vPoint );

    vPoint.x = aabb->inf.x, vPoint.y = aabb->inf.y, vPoint.z = aabb->sup.z;
    if( getPointSideExact( plane,  vPoint ) != firstSide ) return SIDE_ON;

    vPoint.x = aabb->inf.x, vPoint.y = aabb->sup.y, vPoint.z = aabb->sup.z;
    if( getPointSideExact( plane,  vPoint ) != firstSide ) return SIDE_ON;

    vPoint.x = aabb->inf.x, vPoint.y = aabb->sup.y, vPoint.z = aabb->inf.z;
    if( getPointSideExact( plane,  vPoint ) != firstSide ) return SIDE_ON;

    vPoint.x = aabb->sup.x, vPoint.y = aabb->inf.y, vPoint.z = aabb->inf.z;
    if( getPointSideExact( plane,  vPoint ) != firstSide ) return SIDE_ON;

    vPoint.x = aabb->sup.x, vPoint.y = aabb->inf.y, vPoint.z = aabb->sup.z;
    if( getPointSideExact( plane,  vPoint ) != firstSide ) return SIDE_ON;

    vPoint.x = aabb->sup.x, vPoint.y = aabb->sup.y, vPoint.z = aabb->sup.z;
    if( getPointSideExact( plane,  vPoint ) != firstSide ) return SIDE_ON;

    vPoint.x = aabb->sup.x, vPoint.y = aabb->sup.y, vPoint.z = aabb->inf.z;
    if( getPointSideExact( plane,  vPoint ) != firstSide ) return SIDE_ON;
	
	// Ok, they're all on the same side, return that.
	return firstSide;
}

bool intersectAABBFrustum(AABB* aabb, D3DXPLANE* frustrum)
{
    bool     result = true;
    SideType planeSide;
    for( unsigned int i=0; i<6; i++ )
    {
        planeSide = boxOnPlaneSide( frustrum+i, aabb );
        if( planeSide == SIDE_BACK )
        {
            result = false;
            break;
        }
    }
    return result;
}

bool intersectPointFrustum(Vector* point, D3DXPLANE* frustrum)
{
    bool result = true;
    for( unsigned int i=0; i<6; i++ )
    {
        if( getPointSideExact( (frustrum+i), (*point) ) != SIDE_FRONT )
        {
            result = false;
            break;
        }
    }
    return result;
}

float getDistance(D3DXPLANE* plane, Vector* point)
{
    return distTo( plane, (*point) );
}

/**
 * sphere-triangle intersection
 */

#define XDIM 0xF00
#define YDIM 0x0F0
#define ZDIM 0x00F

static inline bool isPointWithinTriangle(Vector* pt, Vector* tri[3], Vector* normal)
{
    int   dimension;
    float absX, absY, absZ;
    bool  inside = false;
    int   i, j;

    // determine weakest dimension, so we can work in 2D...
    absX = fabs( normal->x );
    absY = fabs( normal->y );
    absZ = fabs( normal->z );

    dimension = absZ > absY ? (absZ > absX ? ZDIM:XDIM) : (absY > absX ? YDIM:XDIM);
    switch( dimension )
    {
        case XDIM:
        {    
            for(i=0, j=2; i<3; j=i++)
            {
                if( (((tri[i]->y <= pt->y) && (pt->y < tri[j]->y)) ||
                    ((tri[j]->y <= pt->y) && (pt->y < tri[i]->y))) &&
                    (pt->z < (tri[j]->z - tri[i]->z) * (pt->y - tri[i]->y) / 
                    (tri[j]->y - tri[i]->y) + tri[i]->z) )
                {
                    inside = !inside;
                }
            }
            break;
        }
        case YDIM:
        {    
            for(i=0, j=2; i<3; j=i++)
            {
                if( (((tri[i]->z <= pt->z) && (pt->z < tri[j]->z)) ||
                    ((tri[j]->z <= pt->z) && (pt->z < tri[i]->z))) &&
                    (pt->x < (tri[j]->x - tri[i]->x) * (pt->z - tri[i]->z) / 
                    (tri[j]->z - tri[i]->z) + tri[i]->x) )
                {
                    inside = !inside;
                }
            }
            break;
        }
        case ZDIM:
        {    
            for(i=0, j=2; i<3; j=i++)
            {
                if( (((tri[i]->y <= pt->y) && (pt->y < tri[j]->y)) ||
                    ((tri[j]->y <= pt->y) && (pt->y < tri[i]->y))) &&
                    (pt->x < (tri[j]->x - tri[i]->x) * (pt->y - tri[i]->y) / 
                    (tri[j]->y - tri[i]->y) + tri[i]->x) )
                {
                    inside = !inside;
                }
            }
            break;
        }
        default:
            break;
    }
    return inside;    
}

static inline Vector* findNearestPointOnLine(Vector* result, Vector* point, Vector* start, Vector* end)
{
    float  mu;
    Vector line;

    D3DXVec3Subtract( &line, end, start );
    mu = D3DXVec3Dot( point, &line ) - D3DXVec3Dot( start, &line );
    if( mu <= 0 )
    {
        *result = *start;
    }
    else
    {
        float lineLength2;
        lineLength2 = D3DXVec3Dot( &line, &line );
        if( mu < lineLength2 )
        {
            mu /= lineLength2;
            D3DXVec3Scale( result, &line, mu );
            D3DXVec3Add( result, result, start );
        }
        else
        {
            *result = *end;
        }
    }
    return result;
}

static inline void calcSphereTriangleDistance(
    Sphere* sphere,
    Vector* normal,
    Vector* v0, Vector* v1, Vector* v2,
    Vector* collPoint,
    float* distance 
)
{
    // project sphere center onto plane of triangle.
    Vector* center = &sphere->center;
    Vector  projPoint;
    float   dist2plane = D3DXVec3Dot( v0, normal ) - D3DXVec3Dot( center, normal );
    D3DXVec3Scale( &projPoint, normal, dist2plane );
    D3DXVec3Add( &projPoint, &projPoint, center );

    // does the projected point lie within the collision triangle?
    Vector* vertices[3];
    vertices[0] = v0, vertices[1] = v1, vertices[2] = v2;
    if( isPointWithinTriangle( &projPoint, vertices, normal ) )
    {
        *distance = fabs( dist2plane );
        *collPoint = projPoint;
        return;
    }
    // projected point lies outside the triangle, so find the nearest
    // point on the triangle boundary...
    else
    {
        float currdist;
        Vector closestPoint, temp;
    
        findNearestPointOnLine( &closestPoint, &projPoint, v0, v1 );
        D3DXVec3Subtract( &temp, center, &closestPoint );
        *distance = D3DXVec3Length(&temp), *collPoint = closestPoint;

        findNearestPointOnLine( &closestPoint, &projPoint, v1, v2 );
        D3DXVec3Subtract(&temp, center, &closestPoint);
        currdist = D3DXVec3Length(&temp);
        if( *distance > currdist ) *distance = currdist, *collPoint = closestPoint;

        findNearestPointOnLine( &closestPoint, &projPoint, v0, v2 );
        D3DXVec3Subtract(&temp, center, &closestPoint);
        currdist = D3DXVec3Length(&temp);
        if( *distance > currdist ) *distance = currdist, *collPoint = closestPoint;
    }
}

bool intersectionSphereTriangle(Sphere* sphere, Vector* v0, Vector* v1, Vector* v2, Vector* hitPoint, float* distance)
{
	// early exit if one of the vertices is inside the sphere
	Vector kDiff = *v2 - sphere->center;
	float fC = D3DXVec3Length( &kDiff );
	if( fC <= sphere->radius ) 
    {
        Vector v0v1 = *v1 - *v0;
        Vector v0v2 = *v2 - *v0;
        Vector normal;
        D3DXVec3Cross( &normal, &v0v1, &v0v2 );
        D3DXVec3Normalize( &normal, &normal );
        calcSphereTriangleDistance( sphere, &normal, v0, v1, v2, hitPoint, distance );
        return true;
    }

	kDiff = *v1 - sphere->center;
	fC = D3DXVec3Length( &kDiff );
	if( fC <= sphere->radius )
    {
        Vector v0v1 = *v1 - *v0;
        Vector v0v2 = *v2 - *v0;
        Vector normal;
        D3DXVec3Cross( &normal, &v0v1, &v0v2 );
        D3DXVec3Normalize( &normal, &normal );
        calcSphereTriangleDistance( sphere, &normal, v0, v1, v2, hitPoint, distance );
        return true;
    }

	kDiff = *v0 - sphere->center;
	fC = D3DXVec3Length( &kDiff );
	if( fC <= sphere->radius )
    {
        Vector v0v1 = *v1 - *v0;
        Vector v0v2 = *v2 - *v0;
        Vector normal;
        D3DXVec3Cross( &normal, &v0v1, &v0v2 );
        D3DXVec3Normalize( &normal, &normal );
        calcSphereTriangleDistance( sphere, &normal, v0, v1, v2, hitPoint, distance );
        return true;
    }

	// else, do the full distance test
	Vector triEdge0	= *v1 - *v0;
	Vector triEdge1	= *v2 - *v0;

    kDiff = *v0 - sphere->center;
    fC	  = D3DXVec3LengthSq( &kDiff );

	float fA00	= D3DXVec3LengthSq( &triEdge0 );
	float fA01	= D3DXVec3Dot( &triEdge0,  &triEdge1 );
	float fA11	= D3DXVec3LengthSq( &triEdge1 );
	float fB0	= D3DXVec3Dot( &kDiff, &triEdge0 );
	float fB1	= D3DXVec3Dot( &kDiff, &triEdge1 );
	float fDet	= fabsf( fA00*fA11 - fA01*fA01 );
	float u		= fA01*fB1-fA11*fB0;
	float v		= fA01*fB0-fA00*fB1;
	float sqrDist;

	if( u + v <= fDet )
	{
		if( u < 0.0f )
		{
			if( v < 0.0f )
			{
				if( fB0 < 0.0f )
				{
					if( -fB0>=fA00 )
                    { 
                        sqrDist = fA00+2.0f*fB0+fC;	
                    }
					else
                    { 
                        u = -fB0/fA00;	
                        sqrDist = fB0*u+fC;
                    }
				}
				else
				{
					if( fB1>=0.0f )
                    { 
                        sqrDist = fC;
                    }
					else if( -fB1>=fA11 )
                    { 
                        sqrDist = fA11+2.0f*fB1+fC;	
                    }
					else
                    {
                        v = -fB1/fA11;	
                        sqrDist = fB1*v+fC;
                    }
				}
			}
			else
			{
				if( fB1>=0.0f )
                {
                    sqrDist = fC;
                }
				else if( -fB1>=fA11 )
                {
                    sqrDist = fA11+2.0f*fB1+fC;
                }
				else
                { 
                    v = -fB1/fA11;
                    sqrDist = fB1*v+fC;
                }
			}
		}
		else if( v < 0.0f )
		{
			if( fB0>=0.0f )
            {
                sqrDist = fC;
            }
			else if( -fB0>=fA00 )
            { 
                sqrDist = fA00+2.0f*fB0+fC;
            }
			else
            { 
                u = -fB0/fA00;
                sqrDist = fB0*u+fC;
            }
		}
		else
		{
			// minimum at interior point
			if( fDet==0.0f )
			{
				sqrDist = 3.4E38f;
			}
			else
			{
				float fInvDet = 1.0f/fDet;
				u *= fInvDet;
				v *= fInvDet;
				sqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
			}
		}
	}
	else
	{
		float fTmp0, fTmp1, fNumer, fDenom;
		if( u < 0.0f )
		{
			fTmp0 = fA01 + fB0;
			fTmp1 = fA11 + fB1;
			if( fTmp1 > fTmp0 )
			{
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00-2.0f*fA01+fA11;
				if( fNumer >= fDenom )
				{
					sqrDist = fA00+2.0f*fB0+fC;
				}
				else
				{
					u = fNumer/fDenom;
					v = 1.0f - u;
					sqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
				}
			}
			else
			{
				if( fTmp1 <= 0.0f )
                { 
                    sqrDist = fA11+2.0f*fB1+fC;	
                }
				else if( fB1 >= 0.0f )
                { 
                    sqrDist = fC;
                }
				else
                {
                    v = -fB1/fA11;
                    sqrDist = fB1*v+fC;
                }
			}
		}
		else if( v < 0.0f )
		{
			fTmp0 = fA01 + fB1;
			fTmp1 = fA00 + fB0;
			if( fTmp1 > fTmp0 )
			{
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00-2.0f*fA01+fA11;
				if( fNumer >= fDenom )
				{
					sqrDist = fA11+2.0f*fB1+fC;
				}
				else
				{
					v = fNumer/fDenom;
					u = 1.0f - v;
					sqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
				}
			}
			else
			{
				if( fTmp1 <= 0.0f )
                { 
                    sqrDist = fA00+2.0f*fB0+fC;	
                }
				else if( fB0 >= 0.0f )
                {
                    sqrDist = fC;
                }
				else				
                {
                    u = -fB0/fA00;
                    sqrDist = fB0*u+fC;
                }
			}
		}
		else
		{
			fNumer = fA11 + fB1 - fA01 - fB0;
			if( fNumer <= 0.0f )
			{
				sqrDist = fA11+2.0f*fB1+fC;
			}
			else
			{
				fDenom = fA00-2.0f*fA01+fA11;
				if( fNumer >= fDenom )
				{
					sqrDist = fA00+2.0f*fB0+fC;
				}
				else
				{
					u = fNumer/fDenom;
					v = 1.0f - u;
					sqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
				}
			}
		}
	}

	if( fabsf( sqrDist ) < sphere->radius*sphere->radius )
    {
        Vector v0v1 = *v1 - *v0;
        Vector v0v2 = *v2 - *v0;
        Vector normal;
        D3DXVec3Cross( &normal, &v0v1, &v0v2 );
        D3DXVec3Normalize( &normal, &normal );
        calcSphereTriangleDistance( sphere, &normal, v0, v1, v2, hitPoint, distance );
        return true;
    }
    else
    {
        return false;
    }
}