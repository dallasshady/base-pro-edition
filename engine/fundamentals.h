/**
 * This source code is a part of D3 game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description engine fundamental types
 *
 * @author bad3p
 */

#ifndef ENGINE_FUNDAMENTALS_INCLUDED
#define ENGINE_FUNDAMENTALS_INCLUDED

#include "headers.h"
#include "../shared/engine.h"

/**
 * asset object
 */

typedef unsigned int auid;

typedef std::pair<auid,void*> AssetObjectT;
typedef std::map<auid,void*> AssetObjectM;
typedef AssetObjectM::iterator AssetObjectI;

/**
 * color value
 */

typedef D3DCOLOR Color;

const Color black      = D3DCOLOR_RGBA( 0,0,0,255 );
const Color white      = D3DCOLOR_RGBA( 255,255,255,255 );
const Color gray       = D3DCOLOR_RGBA( 128,128,128,255 );
const Color red        = D3DCOLOR_RGBA( 255,0,0,255 );
const Color darkRed    = D3DCOLOR_RGBA( 128,0,0,255 );
const Color green      = D3DCOLOR_RGBA( 0,255,0,255 );
const Color darkGreen  = D3DCOLOR_RGBA( 0,128,0,255 );
const Color blue       = D3DCOLOR_RGBA( 0,0,255,255 );
const Color darkBlue   = D3DCOLOR_RGBA( 0,0,128,255 );
const Color yellow     = D3DCOLOR_RGBA( 255,255,0,255 );
const Color darkYellow = D3DCOLOR_RGBA( 128,128,0,255 );

/**
 * volumetric (3-dimensional) vector
 */

typedef D3DXVECTOR3 Vector;

const Vector oX( 1,0,0 );
const Vector oY( 0,1,0 );
const Vector oZ( 0,0,1 );

/**
 * flector is concatenation of fl[at v]ector term
 */

typedef D3DXVECTOR2 Flector;

/**
 * quartector is concatenation of quart[er-dimensional v]ector term, or 4-d vector
 */

typedef D3DXVECTOR4 Quartector;

/**
 * transformation matrix
 */

typedef D3DXMATRIX Matrix;

typedef D3DXMATRIXA16 MatrixA16;

const Matrix identity( 1,0,0,0,
                       0,1,0,0,
                       0,0,1,0,
                       0,0,0,1 );

/**
 * quaternion
 */

typedef D3DXQUATERNION Quaternion;

/**
 * nice math
 */

#define sqr(X) (X)*(X)

#define sgn(X) ( (X) < 0 ? -1 : ( (X) > 0 ? 1 : 0 ) )

#define min(X,Y) ( (X<Y) ? (X) : (Y) )

#define max(X,Y) ( (X>Y) ? (X) : (Y) )

/**
 * matrix utilites
 */

static inline Vector dxRight(D3DMATRIX* matrix)
{
    return Vector( matrix->_11, matrix->_12, matrix->_13 );
}

static inline Vector dxUp(D3DMATRIX* matrix)
{
    return Vector( matrix->_21, matrix->_22, matrix->_23 );
}

static inline Vector dxAt(D3DMATRIX* matrix)
{
    return Vector( matrix->_31, matrix->_32, matrix->_33 );
}

static inline Vector dxPos(D3DMATRIX* matrix)
{
    return Vector( matrix->_41, matrix->_42, matrix->_43 );
}

// multiply
static inline void dxMultiply(Matrix* out, Matrix* in1, Matrix* in2)
{
    D3DXMatrixMultiply( out, in1, in2 );
}

// translate, post-concatenation
static inline void dxTranslate(Matrix* matrix, const Vector* vector)
{
    Matrix translation, result;
    D3DXMatrixTranslation( &translation, vector->x, vector->y, vector->z );
    D3DXMatrixMultiply( &result, matrix, &translation );
    *matrix = result;
}

// rotate, post-concatenation
static inline void dxRotate(Matrix* matrix, const Vector* axis, float angle)
{
    Matrix rotation, result;
    D3DXMatrixRotationAxis( &rotation, axis, angle * D3DX_PI / 180.0f );
    D3DXMatrixMultiply( &result, matrix, &rotation );
    *matrix = result;
}

// scale
static inline void dxScale(Matrix* matrix, const Vector* scale)
{
    Matrix scaling, result;
    D3DXMatrixScaling( &scaling, scale->x, scale->y, scale->z );
    D3DXMatrixMultiply( &result, matrix, &scaling );
    *matrix = result;
}

// transform vector
static inline void dxTransform(Vector* vector, Matrix* matrix)
{
    D3DXVECTOR4 out;
    D3DXVec3Transform( &out, vector, matrix );
    vector->x = out.x, 
    vector->y = out.y, 
    vector->z = out.z;
}

/**
 * color utilites
 */

static inline void dxSetColorValue(D3DCOLORVALUE* color, float r, float g, float b, float a)
{
    color->r = r, 
    color->g = g, 
    color->b = b, 
    color->a = a;
}

// color1 = color1 + color2
static inline void dxAddColorValue(D3DCOLORVALUE* color1, D3DCOLORVALUE* color2)
{
    color1->r += color2->r, 
    color1->g += color2->g, 
    color1->b += color2->b, 
    color1->a += color2->a;
}

// color1 = color1 * color2
static inline void dxModulateColorValue(D3DCOLORVALUE* color1, D3DCOLORVALUE* color2)
{
    color1->r *= color2->r, 
    color1->g *= color2->g, 
    color1->b *= color2->b, 
    color1->a *= color2->a;
}

static inline void dxSaturateColorValue(D3DCOLORVALUE* color)
{
    color->r = color->r < 0 ? 0 : ( color->r > 1 ? 1 : color->r ),
    color->g = color->g < 0 ? 0 : ( color->g > 1 ? 1 : color->g ),
    color->b = color->b < 0 ? 0 : ( color->b > 1 ? 1 : color->b ),
    color->a = color->a < 0 ? 0 : ( color->a > 1 ? 1 : color->a );
}

/**
 * fundamental wrappers
 */

static Quartector inline wrapQ(Vector4f vector)
{
    return Quartector( vector[0], vector[1], vector[2], vector[3] );
}

static Vector4f inline wrapQ(Quartector quartector)
{
    return Vector4f( quartector.x, quartector.y, quartector.z, quartector.w );
}

static inline Color wrap(const Vector4f& color)
{
    return D3DCOLOR_RGBA(
        int( color[0] * 255 ),
        int( color[1] * 255 ),
        int( color[2] * 255 ),
        int( color[3] * 255 )
    );
}

static inline Vector4f wrap(Color color)
{
    return Vector4f( 
        float( (color>>16)&0xff ) / 255.0f,
        float( (color>>8)&0xff  ) / 255.0f,
        float( color&0xff  ) / 255.0f,
        float( (color>>24)&0xff ) / 255.0f
    );
}

static inline D3DCOLORVALUE _wrap(const Vector4f& value)
{
    D3DCOLORVALUE result;
    result.r = value[0], result.g = value[1], result.b = value[2], result.a = value[3];
    return result;        
}

static inline Vector4f _wrap(const D3DCOLORVALUE& value)
{
    return Vector4f( value.r, value.g, value.b, value.a );
}

static inline Vector wrap(const Vector3f& vector)
{
    return Vector( vector[0], vector[1], vector[2] );
}

static inline Vector3f wrap(const Vector& vector)
{
    return Vector3f( vector.x, vector.y, vector.z );
}

static inline Flector wrap(const Vector2f& flector)
{
    return Flector( flector[0], flector[1] );
}

static inline Vector2f wrap(const Flector& flector)
{
    return Vector2f( flector.x, flector.y );
}

static inline Matrix wrap(const Matrix4f& matrix)
{
    return Matrix(
        matrix[0][0], matrix[0][1], matrix[0][2], 0.0f,
        matrix[1][0], matrix[1][1], matrix[1][2], 0.0f,
        matrix[2][0], matrix[2][1], matrix[2][2], 0.0f,
        matrix[3][0], matrix[3][1], matrix[3][2], 1.0f
    );
}

static inline Matrix4f wrap(const Matrix& matrix)
{
    return Matrix4f(
        matrix.m[0][0], matrix.m[0][1], matrix.m[0][2], matrix.m[0][3],
        matrix.m[1][0], matrix.m[1][1], matrix.m[1][2], matrix.m[1][3],
        matrix.m[2][0], matrix.m[2][1], matrix.m[2][2], matrix.m[2][3],
        matrix.m[3][0], matrix.m[3][1], matrix.m[3][2], matrix.m[3][3]
    );
}


/**
 * axis aligned bounding box
 */

class AABB
{
public:
    enum AABBPlane { xPlane, yPlane, zPlane, maxPlane };
public:
    Vector inf;
    Vector sup;
public:
    AABB() : inf( 0,0,0 ), sup( 0,0,0 ) {}
    AABB(const Vector& point) : inf(point), sup(point) {}
    AABB(const Vector& _inf, const Vector& _sup) : inf(_inf), sup(_sup) {}
public:
    inline void addPoint(const Vector& point)
    {
        if( point.x < inf.x ) inf.x = point.x;
        if( point.x > sup.x ) sup.x = point.x;
        if( point.y < inf.y ) inf.y = point.y;
        if( point.y > sup.y ) sup.y = point.y;
        if( point.z < inf.z ) inf.z = point.z;
        if( point.z > sup.z ) sup.z = point.z;
    }
    inline void calculate(int numPoints, const Vector* points)
    {
        assert( numPoints );
        inf = sup = *points;
        for( int i=1; i<numPoints; i++ )
        {
            if( points[i].x < inf.x ) inf.x = points[i].x;
            if( points[i].x > sup.x ) sup.x = points[i].x;
            if( points[i].y < inf.y ) inf.y = points[i].y;
            if( points[i].y > sup.y ) sup.y = points[i].y;
            if( points[i].z < inf.z ) inf.z = points[i].z;
            if( points[i].z > sup.z ) sup.z = points[i].z;
        }
    }
    inline bool isInside(const Vector& point)
    {
        return ( inf.x <= point.x && sup.x >= point.x &&
                 inf.y <= point.y && sup.y >= point.y &&
                 inf.z <= point.z && sup.z >= point.z );
    }
    inline Vector getCorner(int cornerId)
    {
        switch( cornerId )
        {
        case 0: return inf;
        case 1: return Vector( inf.x, sup.y, inf.z );
        case 2: return Vector( sup.x, sup.y, inf.z );
        case 3: return Vector( sup.x, inf.y, inf.z );
        case 4: return Vector( sup.x, inf.y, sup.z );
        case 5: return Vector( inf.x, inf.y, sup.z );
        case 6: return Vector( inf.x, sup.y, sup.z );
        case 7: return sup;
        default: assert( !"shouldn't be here!" ); return inf;
        }
    }
    inline Vector getDiagonal(int diagonalId)
    {
        switch( diagonalId )
        {
        case 0: return sup - inf;
        case 1: return Vector( sup.x, inf.y, sup.z ) - Vector( inf.x, sup.y, inf.z );
        case 2: return Vector( inf.x, inf.y, sup.z ) - Vector( sup.x, sup.y, inf.z );
        case 3: return Vector( inf.x, sup.y, sup.z ) - Vector( sup.x, inf.y, inf.z );
        default: assert( !"shouldn't be here!" ); return Vector( 0,0,0 );
        }
    }
    // divides AABB dy specified plane
    inline void divide(AABB& left, AABB& right, AABBPlane plane)
    {
        if( plane == maxPlane )
        {
            float     maxPlaneSize = sup.x - inf.x;
            AABBPlane maxPlaneId   = xPlane;
            if( sup.y - inf.y > maxPlaneSize )
            {
                maxPlaneSize = sup.y - inf.y;
                maxPlaneId   = yPlane;
            }
            if( sup.z - inf.z > maxPlaneSize )
            {
                maxPlaneId = zPlane;
            }
            plane = maxPlaneId;
        }
        left = right = *this;
        switch( plane )
        {
        case xPlane:            
            left.sup.x = right.inf.x = inf.x + ( sup.x - inf.x ) * 0.5f;
            break;
        case yPlane:
            left.sup.y = right.inf.y = inf.y + ( sup.y - inf.y ) * 0.5f;
            break;
        case zPlane:
            left.sup.z = right.inf.z = inf.z + ( sup.z - inf.z ) * 0.5f;
            break;
        default:
            assert( !"shouldn't be here!" );
        }
    }
    // build AABB for specified OBB
    inline void calculate(AABB* aabb, Matrix* matrix)
    {
        Vector osv, wsv;
        for( int i=0; i<8; i++ ) 
        {
            // object-space vertex
            osv.x = i & 1 ? aabb->sup.x : aabb->inf.x,
            osv.y = i & 2 ? aabb->sup.y : aabb->inf.y,
            osv.z = i & 4 ? aabb->sup.z : aabb->inf.z;
            // world-space vertex
            D3DXVec3TransformCoord( &wsv, &osv, matrix );
            // add to box
            if( i == 0 ) 
            {
                inf = wsv, sup = wsv; 
            }
            else 
            {
                addPoint( wsv );
            }
        }
    }
    inline void addAABB(AABB* aabb)
    {
        Vector wsv;
        for( int i=0; i<8; i++ ) 
        {
            // object-space vertex
            wsv.x = i & 1 ? aabb->sup.x : aabb->inf.x,
            wsv.y = i & 2 ? aabb->sup.y : aabb->inf.y,
            wsv.z = i & 4 ? aabb->sup.z : aabb->inf.z;                        
            addPoint( wsv );
        }
    }
};

/**
 * sphere
 */

class Sphere
{
public:
    Vector center;
    float  radius;
public:
    Sphere() : center(0,0,0), radius(0) {}
public:
    inline void calculate(int numPoints, const Vector* points)
    {
        assert( numPoints );
        // calculate center of bounding sphere
        center = *points;
        for( int i=1; i<numPoints; i++ ) center += points[i];
        center /= float( numPoints );
        // calculate radius of bounding sphere
        Vector radiusV;
        float  temp;
        radiusV = center - *points;
        radius = D3DXVec3LengthSq( &radiusV );
        for( i=1; i<numPoints; i++ )
        {
            radiusV = center - points[i];
            temp = D3DXVec3LengthSq( &radiusV );
            if( temp > radius ) radius = temp;
        }
        radius = sqrt( radius );
    }
    inline void calculate(int numMatrices, D3DXMATRIX** matrices)
    {
        assert( numMatrices );
        // calculate center of bounding sphere
        center = dxPos( matrices[0] );
        for( int i=1; i<numMatrices; i++ ) center += dxPos( matrices[i] );
        center /= float( numMatrices );
        // calculate radius of bounding sphere
        Vector radiusV;
        float  temp;
        radiusV = center - dxPos( matrices[0] );
        radius = D3DXVec3LengthSq( &radiusV );
        for( i=1; i<numMatrices; i++ )
        {
            radiusV = center - dxPos( matrices[i] );
            temp = D3DXVec3LengthSq( &radiusV );
            if( temp > radius ) radius = temp;
        }
        radius = sqrt( radius );
    }
};

/**
 * line
 */

class Line
{
public:
    Vector start;
    Vector end;
public:
    Line() : start( 0,0,0 ), end( 0,0,0 ) {}
    Line(Vector s, Vector e) : start(s), end(e) {}
};

#endif