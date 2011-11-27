
#ifndef MATH_INTELLIGENCE_INCLUDED
#define MATH_INTELLIGENCE_INCLUDED

#include "headers.h"
#include "../shared/vector.h"
#include "gameplay.h"

const float cmsInInch = 2.54f;
const float cmsInFoot = 30.48f;
const float sqCmsInSqFoot = 929.0304f;
const float sqFootsInSqMeter = 10.7639f;

/**
 * simple functions
 */

inline float sqr(float x) { return x*x; }
inline float sgn(float x) { return ( x < 0.0f ? -1.0f : ( x > 0.0f ? 1.0f : 0.0f ) ); }

/**
 * @return angle between a pair of vectors, v1 & v2; the signum of angle are 
 *         defined by third vector, v3
 */

inline float calcAngle(const Vector3f& v1, const Vector3f& v2, const Vector3f& v3)
{
    float angle;
    float cosA = Vector3f::dot( v1, v2 );
    // range check & angle metrics
    if( cosA > 1.0f ) angle = 0.0f;
    else if( cosA < -1.0f ) angle = 180;
    else angle = acosf( cosA ) * 180 / 3.1415926f;
    // signum of angle
    Vector3f crossTest;
    crossTest.cross( v1, v2 );
    float sgnTest = Vector3f::dot( crossTest, v3 );
    return angle * -( sgnTest < 0 ? -1 : ( sgnTest > 0 ? 1 : 0 ) );
}

inline float calcAngle(const NxVec3& v1, const NxVec3& v2, const NxVec3& v3)
{
    float angle;
    float cosA = v1.dot( v2 );
    // range check & angle metrics
    if( cosA > 1.0f ) angle = 0.0f;
    else if( cosA < -1.0f ) angle = 180;
    else angle = acosf( cosA ) * 180 / 3.1415926f;
    // signum of angle
    NxVec3 crossTest;
    crossTest.cross( v1, v2 );
    float sgnTest = crossTest.dot( v3 );
    return angle * -( sgnTest < 0 ? -1 : ( sgnTest > 0 ? 1 : 0 ) );
}

/**
 * @return angle between a pair of vectors, v1 & v2; the signum of angle 
 *         is unidentified
 */

static float calcAngle(const Vector3f& v1, const Vector3f& v2)
{
    float angle;
    float cosA = Vector3f::dot( v1, v2 );
    // range check & angle metrics
    if( cosA > 1.0f ) angle = 0.0f;
    else if( cosA < -1.0f ) angle = 180;
    else angle = acosf( cosA ) * 180 / 3.1415926f;
    return angle;
}

static float calcAngle(const NxVec3& v1, const NxVec3& v2)
{
    float angle;
    float cosA = v1.dot( v2 );
    // range check & angle metrics
    if( cosA > 1.0f ) angle = 0.0f;
    else if( cosA < -1.0f ) angle = 180;
    else angle = acosf( cosA ) * 180 / 3.1415926f;
    return angle;
}

/**
 * vector extraction macro
 */

#define RIGHT(V,LTM) Vector3f V(LTM[0][0], LTM[0][1], LTM[0][2])
#define UP(V,LTM) Vector3f V(LTM[1][0], LTM[1][1], LTM[1][2])
#define AT(V,LTM) Vector3f V(LTM[2][0], LTM[2][1], LTM[2][2])
#define POS(V,LTM) Vector3f V(LTM[3][0], LTM[3][1], LTM[3][2])
#define MATRIX(M,R,U,A,P) Matrix4f M( R[0], R[1], R[2], 0.0f, U[0], U[1], U[2], 0.0f, A[0], A[1], A[2], 0.0f, P[0], P[1], P[2], 1.0f );

static inline Vector3f calcScale(const Matrix4f m)
{
    return Vector3f(
        Vector3f( m[0][0], m[0][1], m[0][2] ).length(),
        Vector3f( m[1][0], m[1][1], m[1][2] ).length(),
        Vector3f( m[2][0], m[2][1], m[2][2] ).length()
    );
}

static inline void scaleMatrix(Matrix4f& m, Vector3f& s)
{
    m[0][0] *= s[0], m[0][1] *= s[0], m[0][2] *= s[0];
    m[1][0] *= s[1], m[1][1] *= s[1], m[1][2] *= s[1];
    m[2][0] *= s[2], m[2][1] *= s[2], m[2][2] *= s[2];
}

static inline Vector3f getPRow(const Matrix4f& m)
{
    return Vector3f( m[3][0], m[3][1], m[3][2] );
}

static inline void setPRow(Matrix4f& m, const Vector3f& v)
{
    m[3][0] = v[0], m[3][1] = v[1], m[3][2] = v[2];
}

static inline void orthoNormalize(Matrix4f& m)
{
    RIGHT( x, m ); UP( y, m ); AT( z, m ); POS( p, m );
    x.normalize();
    y.normalize();
    z.normalize();
    m.set( x[0], x[1], x[2], 0.0f,
           y[0], y[1], y[2], 0.0f,
           z[0], z[1], z[2], 0.0f,
           p[0], p[1], p[2], 1.0f );
}

/**
 * physics-to-view synchronizer
 */

class MatrixConversion
{
private:
    Vector3f _scale;
    Matrix4f _transformation;
public:
    void setup(const Matrix4f& fromMatrix, const Matrix4f& toMatrix)
    {
        Matrix4f fm = fromMatrix;
        Matrix4f tm = toMatrix;
        _scale = calcScale( tm );
        orthoNormalize( tm );
        orthoNormalize( fm );
        Matrix4f ifm = Gameplay::iEngine->invertMatrix( fm );
        _transformation = Gameplay::iEngine->transformMatrix( tm, ifm );
    }
    Matrix4f convert(const Matrix4f& fromMatrix)
    {
        Matrix4f result = Gameplay::iEngine->transformMatrix( _transformation, fromMatrix );
        result[0][0] *= _scale[0], result[0][1] *= _scale[0], result[0][2] *= _scale[0];
        result[1][0] *= _scale[1], result[1][1] *= _scale[1], result[1][2] *= _scale[1];
        result[2][0] *= _scale[2], result[2][1] *= _scale[2], result[2][2] *= _scale[2];
        return result;
    }
public:
    Matrix4f getTransformation(void) { return _transformation; }
};

/**
 * wrappers
 */

static inline NxVec3 wrap(const Vector3f& v)
{
    return NxVec3( v[0] * 0.01f, v[1] * 0.01f, v[2] * 0.01f );
}

static inline Vector3f wrap(const NxVec3& v)
{
    return Vector3f( v.x * 100.0f, v.y * 100.0f, v.z * 100.0f );
}

static inline NxMat34 wrap(const Matrix4f& rhm)
{
    NxMat34 result;
    NxVec3  x( rhm[0][0], rhm[1][0], rhm[2][0] );
    NxVec3  y( rhm[0][1], rhm[1][1], rhm[2][1] );
    NxVec3  z( rhm[0][2], rhm[1][2], rhm[2][2] );
    x.normalize();
    y.normalize();
    z.normalize();
    result.M = NxMat33( x,y,z );
    result.t = NxVec3( rhm[3][0] * 0.01f, rhm[3][1] * 0.01f, rhm[3][2] * 0.01f );
    return result;
}

static inline Matrix4f wrap(const NxMat34& m)
{
    NxVec3 x,y,z;
    m.M.getColumn( 0, x );
    m.M.getColumn( 1, y );
    m.M.getColumn( 2, z );
    return Matrix4f(
        x.x, x.y, x.z, 0.0f,
        y.x, y.y, y.z, 0.0f,
        z.x, z.y, z.z, 0.0f,
        m.t.x * 100.0f, m.t.y * 100.0f, m.t.z * 100.0f, 1.0f
    );
}

#endif