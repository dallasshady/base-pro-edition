/**
 * This source code is a part of AqWARium game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description
 * Matrix definitions. Adopted from SGL (Scene Graph Library).
 */

/*****************************************************************************
 * Copyright 1999, Scott McMillan
 *****************************************************************************
 *     File: sglMatrix.hpp
 *   Author: Scott McMillan
 *  Created: 2 January 1999
 *  Summary: Matrix[234]f matrix classes and all helpful operations
 *****************************************************************************/

#ifndef HAEDF027E_64BD_442c_A440_76D86FBE1818
#define HAEDF027E_64BD_442c_A440_76D86FBE1818
#include "vector.h"
#include "float.h"

// ===========================================================================

template <class T>
class Matrix2
{
public:
   // constructors
   Matrix2() {};

   Matrix2(T m00, T m01, T m10, T m11) { set(m00, m01, m10, m11); }

#ifndef _MSC_VER
   Matrix2(const Matrix2& mat)
      {
         memcpy(m_matrix, mat.m_matrix, sizeof(m_matrix));
      }
#endif

   template <class S>
   Matrix2(const Matrix2<S>& mat)
      {
         set(mat[0][0], mat[0][1], mat[1][0], mat[1][1]);
      }

#ifndef _MSC_VER
   Matrix2(T mat[2][2])
      {
         memcpy(m_matrix, mat, sizeof(m_matrix));
      }
#endif

   template <class S>
   Matrix2(S mat[2][2])
      {
         set(mat[0][0], mat[0][1], mat[1][0], mat[1][1]);
      }

   // destructor
   ~Matrix2() {};  // not virtual, do not subclass

   inline T *getPtr() const { return (T *)m_matrix; }

   // indexing operator
   inline Vector2<T>& operator[](unsigned int i)
      { 
         return *(Vector2<T> *) m_matrix[i]; 
      }
   inline const Vector2<T>& operator[](unsigned int i) const
      {
         return *(Vector2<T> *) m_matrix[i];
      }

   // set/get
   template <class S>
   inline void set(S m00, S m01, S m10, S m11)
      {
         m_matrix[0][0] = (T)m00; m_matrix[0][1] = (T)m01;
         m_matrix[1][0] = (T)m10; m_matrix[1][1] = (T)m11;
      }

   template <class S>
   inline void setRow(const Vector2<S>& v, unsigned int i)
      {
         m_matrix[i][0] = (T)v[0]; m_matrix[i][1] = (T)v[1];
      }

   template <class S>
   inline void setCol(const Vector2<S>& v, unsigned int i)
      {
         m_matrix[0][i] = (T)v[0]; m_matrix[1][i] = (T)v[1];
      }

   template <class S>
   inline void getRow(Vector2<S>& v, unsigned int i)
      {
        v[0] = (S)m_matrix[i][0]; v[1] = (S)m_matrix[i][1];
      }

   template <class S>
   inline void getCol(Vector2<S>& v, unsigned int i)
      {
        v[0] = (S)m_matrix[0][i]; v[1] = (S)m_matrix[1][i];
      }

   // assignment operator
#ifndef _MSC_VER
   inline Matrix2 &operator=(const Matrix2& mat)
      {
         if (this != &mat)
         {
            memcpy(m_matrix, mat.m_matrix, sizeof(m_matrix));
         }
         return *this;
      }
#endif

   template <class S>
   inline Matrix2 &operator=(const Matrix2<S>& mat)
      {
         set(mat[0][0], mat[0][1], mat[1][0], mat[1][1]);
         return *this;
      }

   // boolean operators
   template <class S>
   inline bool almostEqual(const Matrix2<S>& mat, double eps) const
      {
         return ((*this)[0].almostEqual(mat[0], eps) &&
                 (*this)[1].almostEqual(mat[1], eps));
      }

   template <class S>
   inline bool operator==(const Matrix2<S>& mat) const
      {
         return (*this)[0] == mat[0] && (*this)[1] == mat[1];
      }

   template <class S>
   inline bool operator!=(const Matrix2<S>& mat) const
      {
         return (*this)[0] != mat[0] || (*this)[1] != mat[1];
      }

   // math operators
   template <class S>
   inline Matrix2& operator+=(const Matrix2<S>& mat)
      {
         (*this)[0] += mat[0], (*this)[1] += mat[1]; return *this;
      }
   template <class S>
   inline Matrix2& operator-=(const Matrix2<S>& mat)
      {
         (*this)[0] -= mat[0], (*this)[1] -= mat[1]; return *this;
      }
   template <class S>
   inline Matrix2& operator*=(const Matrix2<S>& mat)
      {
         mul(*this, mat); return *this;
      }
   inline Matrix2& operator*=(const T s)
      {
         scale(*this, s, s); return *this;
      }

   template <class S>
   inline const Matrix2<T> operator+(const Matrix2<S> &rhs) const
      {
         return Matrix2(m_matrix[0][0] + rhs[0][0],
                        m_matrix[0][1] + rhs[0][1],
                        m_matrix[1][0] + rhs[1][0],
                        m_matrix[1][1] + rhs[1][1]);
      }
   template <class S>
   inline const Matrix2<T> operator-(const Matrix2<S> &rhs) const
      {
         return Matrix2(m_matrix[0][0] - rhs[0][0],
                        m_matrix[0][1] - rhs[0][1],
                        m_matrix[1][0] - rhs[1][0],
                        m_matrix[1][1] - rhs[1][1]);
      }
   template <class S>
   inline const Matrix2<T> operator*(const Matrix2<S> &rhs) const
      {
         return Matrix2(m_matrix[0][0]*rhs[0][0] + m_matrix[0][1]*rhs[1][0],
                        m_matrix[0][0]*rhs[0][1] + m_matrix[0][1]*rhs[1][1],
                        m_matrix[1][0]*rhs[0][0] + m_matrix[1][1]*rhs[1][0],
                        m_matrix[1][0]*rhs[0][1] + m_matrix[1][1]*rhs[1][1]);
      }
   inline const Matrix2<T> operator*(T scalar) const
      {
         return Matrix2(m_matrix[0][0]*scalar,
                        m_matrix[0][1]*scalar,
                        m_matrix[1][0]*scalar,
                        m_matrix[1][1]*scalar);
      }

   // explicit math operations (for backward compatibility)
   template <class S1, class S2>
   inline void add(const Matrix2<S1>& mat1, const Matrix2<S2>& mat2)
      {
         (*this)[0].add(mat1[0],mat2[0]);
         (*this)[1].add(mat1[1],mat2[1]);
      }
   template <class S1, class S2>
   inline void sub(const Matrix2<S1>& mat1, const Matrix2<S2>& mat2)
      {
         (*this)[0].sub(mat1[0],mat2[0]);
         (*this)[1].sub(mat1[1],mat2[1]);
      }
   template <class S>
   inline void scale(const Matrix2<S>& mat, T sx, T sy)
      {
         (*this)[0].scale(mat[0],sx);
         (*this)[1].scale(mat[1],sy);
      }
   // matrix multiply
   template <class S1, class S2>
   inline void mul(const Matrix2<S1>& mat1, const Matrix2<S2>& mat2)
      {
         T t[2][2];
         t[0][0] = mat1[0][0]*mat2[0][0] + mat1[0][1]*mat2[1][0];
         t[0][1] = mat1[0][0]*mat2[0][1] + mat1[0][1]*mat2[1][1];
         t[1][0] = mat1[1][0]*mat2[0][0] + mat1[1][1]*mat2[1][0];
         t[1][1] = mat1[1][0]*mat2[0][1] + mat1[1][1]*mat2[1][1];
         memcpy(m_matrix, t, sizeof(m_matrix));
      }

   // methods to build a matrix
   inline void buildIdentity()
      {
         set(1, 0,  0, 1);
      }

   inline void buildRotation(double angle) // in radians
      {
         T s = (T) sin(angle);
         T c = (T) cos(angle);
         m_matrix[0][0] =  c; m_matrix[0][1] = s;
         m_matrix[1][0] = -s; m_matrix[1][1] = c;
      }

   inline void buildScale(T sx, T sy)
      {
         m_matrix[0][0] = sx;  m_matrix[0][1] = 0;
         m_matrix[1][0] = 0;   m_matrix[1][1] = sy;
      }


   // other operations
   inline void transpose()
      {
         // then upper triangle - use tmp -- transposing self
         T tmp = m_matrix[0][1];
         m_matrix[0][1] = m_matrix[1][0];
         m_matrix[1][0] = tmp;
      }

   template <class S>
   inline void transpose(const Matrix2<S>& mat)
      {
         m_matrix[0][0] = mat[0][0]; m_matrix[1][1] = mat[1][1];

         // then upper triangle - use tmp just in case transposing self
         T tmp = mat[0][1]; m_matrix[0][1] = mat[1][0]; m_matrix[1][0] = tmp;
      }

   // cofactors, determinants, inverse
   inline T cofactor(unsigned int r, unsigned int c) const
      {
         return (((r+c)&1) ? -1:1) * m_matrix[(r ? 0:1)][(c ? 0:1)];
      }

   inline T det() const
      {
         return m_matrix[0][0]*cofactor(0,0) + m_matrix[0][1]*cofactor(0,1);
      }

   template <class S>
   inline void adjoint(const Matrix2<S>& mat)
      {
         T t[2][2];
         t[0][0] = mat.cofactor(0,0);
         t[0][1] = mat.cofactor(1,0);
         t[1][0] = mat.cofactor(0,1);
         t[1][1] = mat.cofactor(1,1);
         set(t[0][0], t[0][1], t[1][0], t[1][1]);
      }
   // adjoint divided by deteminant
   template <class S>
   inline bool inverse(const Matrix2<S>& mat)
      {
         T d = mat.det();
         if (ABS_FUNCTION(d) < FLT_MIN) return false;
         adjoint(mat);
         *this *= (T)(1.0/d);
         return true;
      }

private:
   T m_matrix[2][2];
};

//----------------------------------------------------------------------------
// vector/matrix2x2 operations
//----------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// v' = v' * m (or v = m' * v)
// ---------------------------------------------------------------------------
template <class S, class T>
inline void mul(Vector2<T>& vec, const Matrix2<S>& mat)
{
   T x    = (T)(vec[0]*mat[0][0] + vec[1]*mat[1][0]);
   vec[1] = (T)(vec[0]*mat[0][1] + vec[1]*mat[1][1]);
   vec[0] = x;
}

// ---------------------------------------------------------------------------
// dest' = v' * m (or dest = m' * v)
// ---------------------------------------------------------------------------
template <class R, class S, class T>
inline void mul(const Vector2<R>& vec, const Matrix2<S>& mat,
                Vector2<T>& dest)
{
   T x     = (T)(vec[0]*mat[0][0] + vec[1]*mat[1][0]);
   dest[1] = (T)(vec[0]*mat[0][1] + vec[1]*mat[1][1]);
   dest[0] = x;
}

// ---------------------------------------------------------------------------
// v = m * v (or v' = v' * m')
// ---------------------------------------------------------------------------
template <class S, class T>
inline void mulTranspose(Vector2<T>& vec, const Matrix2<S>& mat)
{
   T x    = (T)(vec[0]*mat[0][0] + vec[1]*mat[0][1]);
   vec[1] = (T)(vec[0]*mat[1][0] + vec[1]*mat[1][1]);
   vec[0] = x;
}

// ---------------------------------------------------------------------------
// dest = m * v (or dest' = v' * m')
// ---------------------------------------------------------------------------
template <class R, class S, class T>
inline void mulTranspose(const Vector2<R>& vec, const Matrix2<S>& mat,
                         Vector2<T>& dest)
{
   T x     = (T)(vec[0]*mat[0][0] + vec[1]*mat[0][1]);
   dest[1] = (T)(vec[0]*mat[1][0] + vec[1]*mat[1][1]);
   dest[0] = x;
}

// ===========================================================================

template <class T>
class Matrix3
{
public:
   // constructors
   Matrix3() {}; 

   Matrix3(T m00, T m01, T m02, T m10, T m11, T m12, T m20, T m21, T m22)
      {
         set(m00, m01, m02, m10, m11, m12, m20, m21, m22);
      }

#ifndef _MSC_VER
   Matrix3(const Matrix3& mat)
      {
         memcpy(m_matrix, mat.m_matrix, sizeof(m_matrix));
      }                       
#endif

   template <class S>
   Matrix3(const Matrix3<S>& mat)
      {
         set(mat[0][0], mat[0][1], mat[0][2],
             mat[1][0], mat[1][1], mat[1][2],
             mat[2][0], mat[2][1], mat[2][2]);
      }                       

#ifndef _MSC_VER
   Matrix3(T mat[3][3])
      {
         memcpy(m_matrix, mat, sizeof(m_matrix));
      }
#endif

   template <class S>
   Matrix3(S mat[3][3])
      {
         set(mat[0][0], mat[0][1], mat[0][2],
             mat[1][0], mat[1][1], mat[1][2],
             mat[2][0], mat[2][1], mat[2][2]);
      }

   // destructor
   ~Matrix3() {};  // not virtual, do not subclass

   inline T *getPtr() const { return (T *)m_matrix; }

   // indexing operators
   inline Vector3<T>& operator[](unsigned int i)
      { 
         return *(Vector3<T> *) m_matrix[i]; 
      }
   inline const Vector3<T>& operator[](unsigned int i) const
      {
         return *(Vector3<T> *) m_matrix[i];
      }

   // set/get
   template <class S>
   void set(S m00, S m01, S m02, S m10, S m11, S m12, S m20, S m21, S m22)
      {
         m_matrix[0][0] = (T)m00; m_matrix[0][1] = (T)m01;
         m_matrix[0][2] = (T)m02;
         m_matrix[1][0] = (T)m10; m_matrix[1][1] = (T)m11;
         m_matrix[1][2] = (T)m12;
         m_matrix[2][0] = (T)m20; m_matrix[2][1] = (T)m21;
         m_matrix[2][2] = (T)m22;
      }

   template <class S>
   inline void setRow(const Vector3<S>& vec, unsigned int i)
      {
         m_matrix[i][0] = (T)vec[0]; m_matrix[i][1] = (T)vec[1];
         m_matrix[i][2] = (T)vec[2];
      }

   template <class S>
   inline void setCol(const Vector3<S>& vec, unsigned int i)
      {
         m_matrix[0][i] = (T)vec[0]; m_matrix[1][i] = (T)vec[1];
         m_matrix[2][i] = (T)vec[2];
      }

   template <class S>
   inline void getRow(Vector3<S>& vec, unsigned int i)
      {
         vec[0] = (S)m_matrix[i][0]; vec[1] = (S)m_matrix[i][1];
         vec[2] = (S)m_matrix[i][2];
      }

   template <class S>
   inline void getCol(Vector3<S>& vec, unsigned int i)
      {
         vec[0] = (S)m_matrix[0][i]; vec[1] = (S)m_matrix[1][i];
         vec[2] = (S)m_matrix[2][i];
      }

   // assignment operators
#ifndef _MSC_VER
   inline Matrix3 &operator=(const Matrix3& mat)
      {
         if (this != &mat)
         {
            memcpy(m_matrix, mat.m_matrix, sizeof(m_matrix));
         }
         return *this;
      }
#endif

   template <class S>
   inline Matrix3& operator=(const Matrix3<S>& mat)
      {
         set(mat[0][0], mat[0][1], mat[0][2],
             mat[1][0], mat[1][1], mat[1][2],
             mat[2][0], mat[2][1], mat[2][2]);
         return *this;
      }

   // boolean operators
   template <class S>
   inline bool almostEqual(const Matrix3<S>& mat, double eps) const
      {
         return ((*this)[0].almostEqual(mat[0], eps) &&
                 (*this)[1].almostEqual(mat[1], eps) &&
                 (*this)[2].almostEqual(mat[2], eps));
      }

   template <class S>
   inline bool operator==(const Matrix3<S>& mat) const
      {
         return ((*this)[0] == mat[0] &&
                 (*this)[1] == mat[1] &&
                 (*this)[2] == mat[2]);
      }

   template <class S>
   inline bool operator!=(const Matrix3<S>& mat) const
      {
         return ((*this)[0] != mat[0] ||
                 (*this)[1] != mat[1] ||
                 (*this)[2] != mat[2]);
      }

   // math operators
   template <class S>
   inline Matrix3& operator+=(const Matrix3<S>& mat)
      {
         (*this)[0] += mat[0]; (*this)[1] += mat[1]; (*this)[2] += mat[2];
         return *this;
      }
   template <class S>
   inline Matrix3& operator-=(const Matrix3<S>& mat)
      {
         (*this)[0] -= mat[0]; (*this)[1] -= mat[1]; (*this)[2] -= mat[2];
         return *this;
      }
   template <class S>
   inline Matrix3& operator*=(const Matrix3<S>& mat)
      {
         mul(*this, mat); return *this;
      }
   inline Matrix3& operator*=(const T s)
      {
         scale(*this, s, s, s); return *this;
      }

   template <class S>
   inline const Matrix3<T> operator+(const Matrix3<S> &rhs) const
      {
         return Matrix3(m_matrix[0][0] + rhs[0][0],
                        m_matrix[0][1] + rhs[0][1],
                        m_matrix[0][2] + rhs[0][2],
                        m_matrix[1][0] + rhs[1][0],
                        m_matrix[1][1] + rhs[1][1],
                        m_matrix[1][2] + rhs[1][2],
                        m_matrix[2][0] + rhs[2][0],
                        m_matrix[2][1] + rhs[2][1],
                        m_matrix[2][2] + rhs[2][2]);
      }
   template <class S>
   inline const Matrix3<T> operator-(const Matrix3<S> &rhs) const
      {
         return Matrix3(m_matrix[0][0] - rhs[0][0],
                        m_matrix[0][1] - rhs[0][1],
                        m_matrix[0][2] - rhs[0][2],
                        m_matrix[1][0] - rhs[1][0],
                        m_matrix[1][1] - rhs[1][1],
                        m_matrix[1][2] - rhs[1][2],
                        m_matrix[2][0] - rhs[2][0],
                        m_matrix[2][1] - rhs[2][1],
                        m_matrix[2][2] - rhs[2][2]);
      }
   template <class S>
   inline const Matrix3<T> operator*(const Matrix3<S> &rhs) const
      {
         return Matrix3(m_matrix[0][0]*rhs[0][0] +
                        m_matrix[0][1]*rhs[1][0] +
                        m_matrix[0][2]*rhs[2][0],
                        m_matrix[0][0]*rhs[0][1] +
                        m_matrix[0][1]*rhs[1][1] +
                        m_matrix[0][2]*rhs[2][1],
                        m_matrix[0][0]*rhs[0][2] +
                        m_matrix[0][1]*rhs[1][2] +
                        m_matrix[0][2]*rhs[2][2],
                        m_matrix[1][0]*rhs[0][0] +
                        m_matrix[1][1]*rhs[1][0] +
                        m_matrix[1][2]*rhs[2][0],
                        m_matrix[1][0]*rhs[0][1] +
                        m_matrix[1][1]*rhs[1][1] +
                        m_matrix[1][2]*rhs[2][1],
                        m_matrix[1][0]*rhs[0][2] +
                        m_matrix[1][1]*rhs[1][2] +
                        m_matrix[1][2]*rhs[2][2],
                        m_matrix[2][0]*rhs[0][0] +
                        m_matrix[2][1]*rhs[1][0] +
                        m_matrix[2][2]*rhs[2][0],
                        m_matrix[2][0]*rhs[0][1] +
                        m_matrix[2][1]*rhs[1][1] +
                        m_matrix[2][2]*rhs[2][1],
                        m_matrix[2][0]*rhs[0][2] +
                        m_matrix[2][1]*rhs[1][2] +
                        m_matrix[2][2]*rhs[2][2]);
      }
   inline const Matrix3<T> operator*(T scalar) const
      {
         return Matrix3(m_matrix[0][0]*scalar,
                        m_matrix[0][1]*scalar,
                        m_matrix[0][2]*scalar,
                        m_matrix[1][0]*scalar,
                        m_matrix[1][1]*scalar,
                        m_matrix[1][2]*scalar,
                        m_matrix[2][0]*scalar,
                        m_matrix[2][1]*scalar,
                        m_matrix[2][2]*scalar);
      }

   // explicit math operations (for backward compatibility)
   template <class S1, class S2>
   inline void add(const Matrix3<S1>& mat1, const Matrix3<S2>& mat2)
      {
         (*this)[0].add(mat1[0],mat2[0]);
         (*this)[1].add(mat1[1],mat2[1]);
         (*this)[2].add(mat1[2],mat2[2]);
      }
   template <class S1, class S2>
   inline void sub(const Matrix3<S1>& mat1, const Matrix3<S2>& mat2)
      {
         (*this)[0].sub(mat1[0],mat2[0]);
         (*this)[1].sub(mat1[1],mat2[1]);
         (*this)[2].sub(mat1[2],mat2[2]);
      }
   template <class S>
   inline void scale(const Matrix3<S>& mat, T sx, T sy, T sz)
      {
         (*this)[0].scale(mat[0],sx);
         (*this)[1].scale(mat[1],sy);
         (*this)[2].scale(mat[2],sz);
      }
   // matrix multiply
   template <class S1, class S2>
   void mul(const Matrix3<S1>& mat1, const Matrix3<S2>& mat2)
      {
         T t[3][3];
         for (unsigned int i=0; i<3; i++)
            for (unsigned int j=0; j<3; j++)
               t[i][j] = (mat1[i][0]*mat2[0][j] +
                          mat1[i][1]*mat2[1][j] +
                          mat1[i][2]*mat2[2][j]);
         memcpy(m_matrix, t, sizeof(m_matrix));
      }

   // methods to build a matrix
   inline void buildIdentity()
      {
         set(1, 0, 0,  0, 1, 0,  0, 0, 1);
      }

   inline void buildScale(T sx, T sy, T sz)
      {
         m_matrix[0][0] = sx; m_matrix[0][1] = 0;  m_matrix[0][2] = 0; 
         m_matrix[1][0] = 0;  m_matrix[1][1] = sy; m_matrix[1][2] = 0; 
         m_matrix[2][0] = 0;  m_matrix[2][1] = 0;  m_matrix[2][2] = sz;
      }

   void buildRotation(float x, float y, float z, double angle)  // radians
      {
         double mag = x*x + y*y + z*z;

         if ((angle == 0.0) || (mag == 0.0))
         {
            buildIdentity();
            return;
         }

         mag = sqrt(mag);
         x /= mag;  y /= mag;  z /= mag;

         double s = sin(angle);
         double c = cos(angle);
         double a = 1.0 - c;

         double xx = x*x, yy = y*y, zz = z*z;
         double xy = x*y, yz = y*z, zx = z*x;
         double sx = x*s, sy = y*s, sz = z*s;

         m_matrix[0][0] = (T)(a*xx + c);
         m_matrix[0][1] = (T)(a*xy + sz);
         m_matrix[0][2] = (T)(a*zx - sy);

         m_matrix[1][0] = (T)(a*xy - sz);
         m_matrix[1][1] = (T)(a*yy + c);
         m_matrix[1][2] = (T)(a*yz + sx);

         m_matrix[2][0] = (T)(a*zx + sy);
         m_matrix[2][1] = (T)(a*yz - sx);
         m_matrix[2][2] = (T)(a*zz + c);
      }

   // other operations
   inline void transpose(const Matrix3& mat)
      {
         // diagonal first
         m_matrix[0][0] = mat[0][0];
         m_matrix[1][1] = mat[1][1];
         m_matrix[2][2] = mat[2][2];
         // then upper triangle
         T tmp = mat[0][1]; m_matrix[0][1] = mat[1][0]; m_matrix[1][0] = tmp;
           tmp = mat[0][2]; m_matrix[0][2] = mat[2][0]; m_matrix[2][0] = tmp;
           tmp = mat[1][2]; m_matrix[1][2] = mat[2][1]; m_matrix[2][1] = tmp;
      }

   // cofactors, determinants, inverse
   inline T cofactor(unsigned int r,unsigned int c) const
      {
         const unsigned int index[3][2] = { {1,2}, {0,2}, {0,1} };
         unsigned int i0 = index[r][0], i1 = index[r][1];
         unsigned int j0 = index[c][0], j1 = index[c][1];
         return (((r+c)&1) ? -1: 1)*(m_matrix[i0][j0]*m_matrix[i1][j1] -
                                     m_matrix[i0][j1]*m_matrix[i1][j0]);
      }
    
   inline T det() const
      {
         return (m_matrix[0][0] * cofactor(0,0) +
                 m_matrix[0][1] * cofactor(0,1) +
                 m_matrix[0][2] * cofactor(0,2));
      }

   template <class S>
   void adjoint(const Matrix3<S>& mat)
      {
         T t[3][3];
         t[0][0] = mat.cofactor(0,0);
         t[0][1] = mat.cofactor(1,0);
         t[0][2] = mat.cofactor(2,0);
         t[1][0] = mat.cofactor(0,1);
         t[1][1] = mat.cofactor(1,1);
         t[1][2] = mat.cofactor(2,1);
         t[2][0] = mat.cofactor(0,2);
         t[2][1] = mat.cofactor(1,2);
         t[2][2] = mat.cofactor(2,2);
         set(t[0][0], t[0][1], t[0][2],
             t[1][0], t[1][1], t[1][2],
             t[2][0], t[2][1], t[2][2]);  // memcpy does not work here
      }

   // try a adjoint divided by deteminant
   template <class S>
   inline bool inverse(const Matrix3<S>& mat)
      {
         T d = mat.det();
         if (ABS_FUNCTION(d) < FLT_MIN) return false;
         adjoint(mat);
         *this *= (T)(1.0/d);
         return true;
      }

private:
   //Vector3<T> m_matrix[3];
   T m_matrix[3][3];
};

//----------------------------------------------------------------------------
// vector/matrix3x3 operations
//----------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// this' = this' * m (or this = m' * this)
// ---------------------------------------------------------------------------
template <class S, class T>
inline void mul(Vector3<T>& vec, const Matrix3<S>& mat)
{
   T x    = (T)(vec[0]*mat[0][0] + vec[1]*mat[1][0] + vec[2]*mat[2][0]);
   T y    = (T)(vec[0]*mat[0][1] + vec[1]*mat[1][1] + vec[2]*mat[2][1]);
   vec[2] = (T)(vec[0]*mat[0][2] + vec[1]*mat[1][2] + vec[2]*mat[2][2]);
   vec[0] = x; vec[1] = y;
}

// ---------------------------------------------------------------------------
//  dest' = v' * m (or dest = m' * v)
// ---------------------------------------------------------------------------
template <class R, class S, class T>
inline void mul(const Vector3<R> &vec, const Matrix3<S>& mat,
                Vector3<T>& dest)
{
   T x     = (T)(vec[0]*mat[0][0] + vec[1]*mat[1][0] + vec[2]*mat[2][0]);
   T y     = (T)(vec[0]*mat[0][1] + vec[1]*mat[1][1] + vec[2]*mat[2][1]);
   dest[2] = (T)(vec[0]*mat[0][2] + vec[1]*mat[1][2] + vec[2]*mat[2][2]);
   dest[0] = x; dest[1] = y;
}

// ---------------------------------------------------------------------------
//  dest' = [v[0] v[1] z] * m (or dest = m' * v)
// ---------------------------------------------------------------------------
template <class R, class S, class T>
inline void mul(const Vector2<R>& vec, R z, const Matrix3<S>& mat,
                Vector3<T>& dest)
{
   dest[0] = (T)(vec[0]*mat[0][0] + vec[1]*mat[1][0] + z*mat[2][0]);
   dest[1] = (T)(vec[0]*mat[0][1] + vec[1]*mat[1][1] + z*mat[2][1]);
   dest[2] = (T)(vec[0]*mat[0][2] + vec[1]*mat[1][2] + z*mat[2][2]);
}

// ---------------------------------------------------------------------------
// v' = v' * m' (or v = m * v)
// ---------------------------------------------------------------------------
template <class S, class T>
inline void mulTranspose(Vector3<T>& vec, const Matrix3<S>& mat)
{
   T x    = (T)(vec[0]*mat[0][0] + vec[1]*mat[0][1] + vec[2]*mat[0][2]);
   T y    = (T)(vec[0]*mat[1][0] + vec[1]*mat[1][1] + vec[2]*mat[1][2]);
   vec[2] = (T)(vec[0]*mat[2][0] + vec[1]*mat[2][1] + vec[2]*mat[2][2]);
   vec[0] = x; vec[1] = y;
}

// ---------------------------------------------------------------------------
//  dest' = v' * m' (or dest = m * v)
// ---------------------------------------------------------------------------
template <class R, class S, class T>
inline void mulTranspose(const Vector3<R> &vec, const Matrix3<S>& mat,
                         Vector3<T> &dest)
{
   T x     = (T)(vec[0]*mat[0][0] + vec[1]*mat[0][1] + vec[2]*mat[0][2]);
   T y     = (T)(vec[0]*mat[1][0] + vec[1]*mat[1][1] + vec[2]*mat[1][2]);
   dest[2] = (T)(vec[0]*mat[2][0] + vec[1]*mat[2][1] + vec[2]*mat[2][2]);
   dest[0] = x; dest[1] = y;
}

// ---------------------------------------------------------------------------
//  dest' = v' * m' (or dest = m * v)
// ---------------------------------------------------------------------------
template <class R, class S, class T>
inline void mulTranspose(const Vector2<R> &vec, R z, const Matrix3<S>& mat,
                         Vector3<T>& dest)
{
   dest[0] = (T)(vec[0]*mat[0][0] + vec[1]*mat[0][1] + z*mat[0][2]);
   dest[1] = (T)(vec[0]*mat[1][0] + vec[1]*mat[1][1] + z*mat[1][2]);
   dest[2] = (T)(vec[0]*mat[2][0] + vec[1]*mat[2][1] + z*mat[2][2]);
}

// ===========================================================================

template <class T>
class Matrix4
{
public:
    // constructors
   Matrix4() {}; 

   Matrix4(T m00, T m01, T m02, T m03,
           T m10, T m11, T m12, T m13,
           T m20, T m21, T m22, T m23,
           T m30, T m31, T m32, T m33)
      {
         set(m00, m01, m02, m03, m10, m11, m12, m13,
             m20, m21, m22, m23, m30, m31, m32, m33);
      }

#ifndef _MSC_VER
   Matrix4(const Matrix4& mat)
      {
         memcpy(m_matrix, mat.m_matrix, sizeof(m_matrix));
      }                       
#endif

   template <class S>
   Matrix4(const Matrix4<S>& mat)
      {
         set(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
             mat[1][0], mat[1][1], mat[1][2], mat[1][3],
             mat[2][0], mat[2][1], mat[2][2], mat[2][3],
             mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
      }                       

#ifndef _MSC_VER
   Matrix4(T mat[4][4])
      {
         memcpy(m_matrix, mat,sizeof(m_matrix));
      }
#endif

   template <class S>
   Matrix4(S mat[4][4])
      {
         set(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
             mat[1][0], mat[1][1], mat[1][2], mat[1][3],
             mat[2][0], mat[2][1], mat[2][2], mat[2][3],
             mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
      }

   // destructor
   ~Matrix4() {};  // not virtual, do not subclass

   inline T *getPtr() const { return (T *)m_matrix; }

   // indexing operators
   inline Vector4<T>& operator[](unsigned int i)
      { 
         return *(Vector4<T> *) m_matrix[i]; 
      }
   inline const Vector4<T>& operator[](unsigned int i) const
      {
         return *(Vector4<T> *) m_matrix[i];
      }

   // set/get
   template <class S>
   inline void set(S m00, S m01, S m02, S m03,
                   S m10, S m11, S m12, S m13,
                   S m20, S m21, S m22, S m23,
                   S m30, S m31, S m32, S m33)
      {
         m_matrix[0][0] = (T)m00; m_matrix[0][1] = (T)m01;
         m_matrix[0][2] = (T)m02; m_matrix[0][3] = (T)m03;
         m_matrix[1][0] = (T)m10; m_matrix[1][1] = (T)m11;
         m_matrix[1][2] = (T)m12; m_matrix[1][3] = (T)m13;
         m_matrix[2][0] = (T)m20; m_matrix[2][1] = (T)m21;
         m_matrix[2][2] = (T)m22; m_matrix[2][3] = (T)m23;
         m_matrix[3][0] = (T)m30; m_matrix[3][1] = (T)m31;
         m_matrix[3][2] = (T)m32; m_matrix[3][3] = (T)m33;
      }

   template <class S>
   inline void setRow(const Vector4<S>& vec, unsigned int i)
      {
         m_matrix[i][0] = (T)vec[0]; m_matrix[i][1] = (T)vec[1];
         m_matrix[i][2] = (T)vec[2]; m_matrix[i][3] = (T)vec[3];
      }
   template <class S>
   inline void setCol(const Vector4<S>& vec, unsigned int i)
      {
         m_matrix[0][i] = (T)vec[0]; m_matrix[1][i] = (T)vec[1];
         m_matrix[2][i] = (T)vec[2]; m_matrix[3][i] = (T)vec[3];
      }

   template <class S>
   inline void getRow(Vector4<S>& vec, unsigned int i)
      {
         vec[0] = (S)m_matrix[i][0]; vec[1] = (S)m_matrix[i][1];
         vec[2] = (S)m_matrix[i][2]; vec[3] = (S)m_matrix[i][3];
      }

   template <class S>
   inline void getCol(Vector4<S>& vec, unsigned int i)
      {
         vec[0] = (S)m_matrix[0][i]; vec[1] = (S)m_matrix[1][i];
         vec[2] = (S)m_matrix[2][i]; vec[3] = (S)m_matrix[3][i];
      }

   // assignment operators
#ifndef _MSC_VER
   inline Matrix4& operator=(const Matrix4& mat)
      {
         if (this != &mat)
         {
            memcpy(m_matrix,mat.m_matrix,sizeof(m_matrix));
         }
         return *this;
      }
#endif

   template <class S>
   inline Matrix4& operator=(const Matrix4<S>& mat)
      {
         set(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
             mat[1][0], mat[1][1], mat[1][2], mat[1][3],
             mat[2][0], mat[2][1], mat[2][2], mat[2][3],
             mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
         return *this;
      }

   // boolean operators
   template <class S>
   inline bool almostEqual(const Matrix4<S>& mat, double eps) const
      {
         return ((*this)[0].almostEqual(mat[0], eps) &&
                 (*this)[1].almostEqual(mat[1], eps) &&
                 (*this)[2].almostEqual(mat[2], eps) &&
                 (*this)[3].almostEqual(mat[3], eps));
      }

   template <class S>
   inline bool operator==(const Matrix4<S>& mat) const
      {
         return ((*this)[0] == mat[0] && (*this)[1] == mat[1] &&
                 (*this)[2] == mat[2] && (*this)[3] == mat[3]);
      }

   template <class S>
   inline bool operator!=(const Matrix4<S>& mat) const
      {
         return ((*this)[0] != mat[0] || (*this)[1] != mat[1] ||
                 (*this)[2] != mat[2] || (*this)[3] != mat[3]);
      }

   // math operators
   template <class S>
   inline Matrix4<T>& operator+=(const Matrix4<S>& mat)
      {
         (*this)[0] += mat[0]; (*this)[1] += mat[1];
         (*this)[2] += mat[2]; (*this)[3] += mat[3];
         return *this;
      }
   template <class S>
   inline Matrix4<T>& operator-=(const Matrix4<S>& mat)
      {
         (*this)[0] -= mat[0]; (*this)[1] -= mat[1];
         (*this)[2] -= mat[2]; (*this)[3] -= mat[3];
         return *this;
      }
   template <class S>
   inline Matrix4<T>& operator*=(const Matrix4<S>& mat)
      {
         mul(*this, mat);   return *this;
      }

   inline Matrix4<T>& operator*=(const T s)
      {
         scale(*this, s, s, s, s);   return *this;
      }

   template <class S>
   inline const Matrix4<T> operator+(const Matrix4<S> &rhs) const
      {
         return Matrix4(m_matrix[0][0] + rhs[0][0],
                        m_matrix[0][1] + rhs[0][1],
                        m_matrix[0][2] + rhs[0][2],
                        m_matrix[0][3] + rhs[0][3],

                        m_matrix[1][0] + rhs[1][0],
                        m_matrix[1][1] + rhs[1][1],
                        m_matrix[1][2] + rhs[1][2],
                        m_matrix[1][3] + rhs[1][3],

                        m_matrix[2][0] + rhs[2][0],
                        m_matrix[2][1] + rhs[2][1],
                        m_matrix[2][2] + rhs[2][2],
                        m_matrix[2][3] + rhs[2][3],

                        m_matrix[3][0] + rhs[3][0],
                        m_matrix[3][1] + rhs[3][1],
                        m_matrix[3][2] + rhs[3][2],
                        m_matrix[3][3] + rhs[3][3]);
      }
   template <class S>
   inline const Matrix4<T> operator-(const Matrix4<S> &rhs) const
      {
         return Matrix4(m_matrix[0][0] - rhs[0][0],
                        m_matrix[0][1] - rhs[0][1],
                        m_matrix[0][2] - rhs[0][2],
                        m_matrix[0][3] - rhs[0][3],

                        m_matrix[1][0] - rhs[1][0],
                        m_matrix[1][1] - rhs[1][1],
                        m_matrix[1][2] - rhs[1][2],
                        m_matrix[1][3] - rhs[1][3],

                        m_matrix[2][0] - rhs[2][0],
                        m_matrix[2][1] - rhs[2][1],
                        m_matrix[2][2] - rhs[2][2],
                        m_matrix[2][3] - rhs[2][3],

                        m_matrix[3][0] - rhs[3][0],
                        m_matrix[3][1] - rhs[3][1],
                        m_matrix[3][2] - rhs[3][2],
                        m_matrix[3][3] - rhs[3][3]);
      }
   template <class S>
   inline const Matrix4<T> operator*(const Matrix4<S> &rhs) const
      {
         Matrix4<T> tmp;
         for (unsigned int i=0; i<4; i++)
         {
            for (unsigned int j=0; j<4; j++)
            {
               tmp[i][j] = (m_matrix[i][0]*rhs[0][j] +
                            m_matrix[i][1]*rhs[1][j] +
                            m_matrix[i][2]*rhs[2][j] +
                            m_matrix[i][3]*rhs[3][j]);
            }
         }
         return tmp;
      }

   inline const Matrix4<T> operator*(T scalar) const
      {
         return Matrix4(m_matrix[0][0]*scalar,
                        m_matrix[0][1]*scalar,
                        m_matrix[0][2]*scalar,
                        m_matrix[0][3]*scalar,

                        m_matrix[1][0]*scalar,
                        m_matrix[1][1]*scalar,
                        m_matrix[1][2]*scalar,
                        m_matrix[1][3]*scalar,

                        m_matrix[2][0]*scalar,
                        m_matrix[2][1]*scalar,
                        m_matrix[2][2]*scalar,
                        m_matrix[2][3]*scalar,

                        m_matrix[3][0]*scalar,
                        m_matrix[3][1]*scalar,
                        m_matrix[3][2]*scalar,
                        m_matrix[3][3]*scalar);
      }

   // explicit math operations (for backward compatibility)
   template <class S1, class S2>
   inline void add(const Matrix4<S1>& mat1, const Matrix4<S2>& mat2)
      {
         (*this)[0].add(mat1[0],mat2[0]);
         (*this)[1].add(mat1[1],mat2[1]);
         (*this)[2].add(mat1[2],mat2[2]);
         (*this)[3].add(mat1[3],mat2[3]);
      }
   template <class S1, class S2>
   inline void sub(const Matrix4<S1>& mat1, const Matrix4<S2>& mat2)
      {
         (*this)[0].sub(mat1[0],mat2[0]);
         (*this)[1].sub(mat1[1],mat2[1]);
         (*this)[2].sub(mat1[2],mat2[2]);
         (*this)[3].sub(mat1[3],mat2[3]);
      }
   template <class S>
   inline void scale(const Matrix4<S>& mat, T sx, T sy, T sz, T sw=1)
      {
         (*this)[0].scale(mat[0], sx);
         (*this)[1].scale(mat[1], sy);
         (*this)[2].scale(mat[2], sz);
         // scale does not apply to the translation line
         (*this)[3].scale(mat[3], sw);
      }
   template <class S>
   inline void translate(const Matrix4<S>& mat, T tx, T ty, T tz)
      {
         *this = mat;
         for (unsigned int i=0; i<3; i++)
         {
            (*this)[3][i] +=
               mat[0][i]*tx +
               mat[1][i]*ty +
               mat[2][i]*tz;
         }
      }

// matrix product
   template <class S1, class S2>
   void mul(const Matrix4<S1>& mat1, const Matrix4<S2>& mat2)
      {
         T t[4][4];
         for (unsigned int i=0; i<4; i++)
            for (unsigned int j=0; j<4; j++)
               t[i][j] = (mat1[i][0]*mat2[0][j] +
                          mat1[i][1]*mat2[1][j] +
                          mat1[i][2]*mat2[2][j] +
                          mat1[i][3]*mat2[3][j]);

         memcpy(m_matrix, t, sizeof(m_matrix));
      }

   // methods to build a matrix
   inline void buildIdentity()
      {
         set(1, 0, 0, 0,   0, 1, 0, 0,   0, 0, 1, 0,   0, 0, 0, 1);
      }

   inline void buildScale(T sx, T sy, T sz, T sw = 1)
      {
         buildIdentity();
         m_matrix[0][0] = sx; m_matrix[1][1] = sy; m_matrix[2][2] = sz;
         m_matrix[3][3] = sw;
      }

   inline void buildTranslation(T tx, T ty, T tz)
      {
         buildIdentity();
         m_matrix[3][0] = tx; m_matrix[3][1] = ty; m_matrix[3][2] = tz;
      }

   void buildRotation(float x, float y, float z, double angle)  // radians
      {
         double mag = x*x + y*y + z*z;

         if ((angle == 0.0) || (mag == 0.0))
         {
            buildIdentity();
            return;
         }

         mag = (T)sqrt( mag );
         x /= mag;  y /= mag;  z /= mag;

         double s = sin(angle), c = cos(angle);
         double a = 1 - c;

         double xx = x*x, yy = y*y, zz = z*z;
         double xy = x*y, yz = y*z, zx = z*x;
         double sx = x*s, sy = y*s, sz = z*s;

         m_matrix[0][0] = (T)(a*xx + c);
         m_matrix[0][1] = (T)(a*xy + sz);
         m_matrix[0][2] = (T)(a*zx - sy);
         m_matrix[0][3] = 0;

         m_matrix[1][0] = (T)(a*xy - sz);
         m_matrix[1][1] = (T)(a*yy + c);
         m_matrix[1][2] = (T)(a*yz + sx);
         m_matrix[1][3] = 0;

         m_matrix[2][0] = (T)(a*zx + sy);
         m_matrix[2][1] = (T)(a*yz - sx);
         m_matrix[2][2] = (T)(a*zz + c);
         m_matrix[2][3] = 0;

         m_matrix[3][0] = m_matrix[3][1] = m_matrix[3][2] = 0;
         m_matrix[3][3] = 1;
      }

   // other operations
   inline void transpose(const Matrix4& mat)
      {
         // diagonal first
         m_matrix[0][0] = mat[0][0];
         m_matrix[1][1] = mat[1][1];
         m_matrix[2][2] = mat[2][2];
         m_matrix[3][3] = mat[3][3];
         // then upper triangle
         T tmp;
         tmp = mat[0][1]; m_matrix[0][1] = mat[1][0]; m_matrix[1][0] = tmp;
         tmp = mat[0][2]; m_matrix[0][2] = mat[2][0]; m_matrix[2][0] = tmp;
         tmp = mat[0][3]; m_matrix[0][3] = mat[3][0]; m_matrix[3][0] = tmp;
         tmp = mat[1][2]; m_matrix[1][2] = mat[2][1]; m_matrix[2][1] = tmp;
         tmp = mat[1][3]; m_matrix[1][3] = mat[3][1]; m_matrix[3][1] = tmp;
         tmp = mat[2][3]; m_matrix[2][3] = mat[3][2]; m_matrix[3][2] = tmp;
      }

// cofactors, determinants, inverse
   inline T cofactor(unsigned int r, unsigned int c) const
      {
         static const unsigned int index[4][3] = { {1,2,3}, {0,2,3},
                                                   {0,1,3}, {0,1,2} };
         unsigned int i0=index[r][0], i1=index[r][1], i2=index[r][2];
         unsigned int j0=index[c][0], j1=index[c][1], j2=index[c][2];
#define __SUBDET2(r,c,i0,i1,j0,j1) (((r+c)&1) ? -1.f: 1.f) * \
              (m_matrix[i0][j0]*m_matrix[i1][j1] - \
               m_matrix[i0][j1]*m_matrix[i1][j0])
      return (m_matrix[i0][j0] * __SUBDET2(i0,j0,i1,i2,j1,j2)+
              m_matrix[i0][j1] * __SUBDET2(i0,j1,i1,i2,j0,j2)+
              m_matrix[i0][j2] * __SUBDET2(i0,j2,i1,i2,j0,j1));
#undef __SUBDET2
   }

// reducing methods which result is a T
   inline T det() const
      {
         return (m_matrix[0][0] * cofactor(0,0) +
                 m_matrix[0][1] * cofactor(0,1) +
                 m_matrix[0][2] * cofactor(0,2) +
                 m_matrix[0][3] * cofactor(0,3));
      }

   template <class S>
   void adjoint(const Matrix4<S>& mat)
      {
         T t[4][4];
         for (unsigned int i=0; i<4; i++)
            for (unsigned int j=0; j<4; j++)
               t[i][j] = mat.cofactor(i,j);
         set(t[0][0], t[0][1], t[0][2], t[0][3],
             t[1][0], t[1][1], t[1][2], t[1][3],
             t[2][0], t[2][1], t[2][2], t[2][3],
             t[3][0], t[3][1], t[3][2], t[3][3]);
      }

   template <class S>
   bool inverse(const Matrix4<S>& mat)
      {
         unsigned int indxc[4], indxr[4], ipiv[4];
         unsigned int i,j,k,l,ll;
         unsigned int icol = 0;
         unsigned int irow = 0;
         T pivinv, dum, big;

         // copy in place this may be unnecessary
         *this = mat;

         for (j=0; j<4; j++) ipiv[j]=0;

         for(i=0;i<4;i++)
         {
            big=(T)0.0;
            for (j=0; j<4; j++)
               if (ipiv[j] != 1)
                  for (k=0; k<4; k++)
                  {
                     if (ipiv[k] == 0)
                     {
                        if (ABS_FUNCTION(m_matrix[j][k]) >= big)
                        {
                           big = ABS_FUNCTION(m_matrix[j][k]);
                           irow=j;
                           icol=k;
                        }
                     }
                     else if (ipiv[k] > 1)
                        return false;
                  }
            ++(ipiv[icol]);
            if (irow != icol)
               for (l=0; l<4; l++) SWAP_FUNCTION(m_matrix[irow][l],
                                            m_matrix[icol][l] );

            indxr[i]=irow;
            indxc[i]=icol;
            if (m_matrix[icol][icol] == 0)
               return false;

            pivinv = 1.0/m_matrix[icol][icol];
            m_matrix[icol][icol] = 1;
            for (l=0; l<4; l++) m_matrix[icol][l] *= pivinv;
            for (ll=0; ll<4; ll++)
               if (ll != icol)
               {
                  dum=m_matrix[ll][icol];
                  m_matrix[ll][icol] = 0;
                  for (l=0; l<4; l++) m_matrix[ll][l] -= m_matrix[icol][l]*dum;
               }
         }
         for (unsigned int lx=4; lx>0; lx--)
         {
            if (indxr[lx-1] != indxc[lx-1])
               for (k=0; k<4; k++) SWAP_FUNCTION(m_matrix[k][indxr[lx-1]],
                                            m_matrix[k][indxc[lx-1]] );
         }

         return true;
      }

   template <class S>
   void inverseAffine(const Matrix4<S>& mat)
      {
         // |   R    p |'   |   R' -R'p |'
         // |          | -> |           |
         // | 0 0 0  1 |    | 0 0 0  1  |
         for (unsigned int i=0; i<3; i++)
         {
            m_matrix[i][3] = 0;
            m_matrix[3][i] = (T) -(mat[i][0]*mat[3][0] +
                                   mat[i][1]*mat[3][1] +
                                   mat[i][2]*mat[3][2]); 
            for (unsigned int j=0; j<3; j++)
            {
               m_matrix[i][j] = (T)mat[j][i];
            }
         }
         m_matrix[3][3] = 1;
      }

private:
   T m_matrix[4][4];
};

//----------------------------------------------------------------------------
// vector/matrix4x4 operations
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// v' = v' * m (or v = m' * v)
//----------------------------------------------------------------------------
template <class S, class T>
inline void mul(Vector4<T>& vec, const Matrix4<S>& mat)
{
   T x = (T)(vec[0]*mat[0][0] + vec[1]*mat[1][0] +
                                         vec[2]*mat[2][0] + vec[3]*mat[3][0]);
   T y = (T)(vec[0]*mat[0][1] + vec[1]*mat[1][1] +
                                         vec[2]*mat[2][1] + vec[3]*mat[3][1]);
   T z = (T)(vec[0]*mat[0][2] + vec[1]*mat[1][2] +
                                         vec[2]*mat[2][2] + vec[3]*mat[3][2]);
   vec[3] = (T)(vec[0]*mat[0][3] + vec[1]*mat[1][3] +
                                         vec[2]*mat[2][3] + vec[3]*mat[3][3]);
   vec[0] = x; vec[1] = y; vec[2] = z;
}

//----------------------------------------------------------------------------
// dest' = v' * m (or dest = m' * v)
//----------------------------------------------------------------------------
template <class R, class S, class T>
inline void mul(const Vector4<R>& vec, const Matrix4<S>& mat,
                Vector4<T>& dest)
{
   T x = (T)(vec[0]*mat[0][0] + vec[1]*mat[1][0] + vec[2]*mat[2][0] +
             vec[3]*mat[3][0]);
   T y = (T)(vec[0]*mat[0][1] + vec[1]*mat[1][1] + vec[2]*mat[2][1] +
             vec[3]*mat[3][1]);
   T z = (T)(vec[0]*mat[0][2] + vec[1]*mat[1][2] + vec[2]*mat[2][2] +
             vec[3]*mat[3][2]);
   dest[3] = (T)(vec[0]*mat[0][3] + vec[1]*mat[1][3] + vec[2]*mat[2][3] +
                 vec[3]*mat[3][3]);
   dest[0] = x; dest[1] = y; dest[2] = z;
}

//----------------------------------------------------------------------------
// dest' = [v[0] v[1] v[2] w] * m (or dest = m' * v)
//----------------------------------------------------------------------------
template <class R, class S, class T>
inline void mul(const Vector3<R>& vec, R w, const Matrix4<S>& mat,
                Vector4<T>& dest)
{
   dest[0] = (T)(vec[0]*mat[0][0] + vec[1]*mat[1][0] + vec[2]*mat[2][0] +
                 w*mat[3][0]);
   dest[1] = (T)(vec[0]*mat[0][1] + vec[1]*mat[1][1] + vec[2]*mat[2][1] +
                 w*mat[3][1]);
   dest[2] = (T)(vec[0]*mat[0][2] + vec[1]*mat[1][2] + vec[2]*mat[2][2] +
                 w*mat[3][2]);
   dest[3] = (T)(vec[0]*mat[0][3] + vec[1]*mat[1][3] + vec[2]*mat[2][3] +
                 w*mat[3][3]);
}

//----------------------------------------------------------------------------
// v' = v' * m' (or v = m * v)
//----------------------------------------------------------------------------
template <class S, class T>
inline void mulTranspose(Vector4<T>& vec, const Matrix4<S>& mat)
{
   T x =    (T)(vec[0]*mat[0][0] + vec[1]*mat[0][1] + vec[2]*mat[0][2] +
                vec[3]*mat[0][3]);
   T y =    (T)(vec[0]*mat[1][0] + vec[1]*mat[1][1] + vec[2]*mat[1][2] +
                vec[3]*mat[1][3]);
   T z =    (T)(vec[0]*mat[2][0] + vec[1]*mat[2][1] + vec[2]*mat[2][2] +
                vec[3]*mat[2][3]);
   vec[3] = (T)(vec[0]*mat[3][0] + vec[1]*mat[3][1] + vec[2]*mat[3][2] +
                vec[3]*mat[3][3]);
   vec[0] = x; vec[1] = y; vec[2] = z;
}

//----------------------------------------------------------------------------
// dest' = v' * m' (or dest = m * v)
//----------------------------------------------------------------------------
template <class R, class S, class T>
inline void mulTranspose(const Vector4<R> &vec, const Matrix4<S>& mat,
                         Vector4<T>& dest)
{
   T x = (T)(vec[0]*mat[0][0] + vec[1]*mat[0][1] + vec[2]*mat[0][2] +
             vec[3]*mat[0][3]);
   T y = (T)(vec[0]*mat[1][0] + vec[1]*mat[1][1] + vec[2]*mat[1][2] +
             vec[3]*mat[1][3]);
   T z = (T)(vec[0]*mat[2][0] + vec[1]*mat[2][1] + vec[2]*mat[2][2] +
             vec[3]*mat[2][3]);
   dest[3] = (T)(vec[0]*mat[3][0] + vec[1]*mat[3][1] + vec[2]*mat[3][2] +
                     vec[3]*mat[3][3]);
   dest[0] = x; dest[1] = y; dest[2] = z;
}

//----------------------------------------------------------------------------
//  dest' = [v[0] v[1] v[2] w] * m' (or dest = m * v)
//----------------------------------------------------------------------------
template <class R, class S, class T>
inline void mulTranspose(const Vector3<R>& vec, R w, const Matrix4<S>& mat,
                         Vector4<T>& dest)
{
   dest[0] = (T)(vec[0]*mat[0][0] + vec[1]*mat[0][1] + vec[2]*mat[0][2] +
                     w*mat[0][3]);
   dest[1] = (T)(vec[0]*mat[1][0] + vec[1]*mat[1][1] + vec[2]*mat[1][2] +
                     w*mat[1][3]);
   dest[2] = (T)(vec[0]*mat[2][0] + vec[1]*mat[2][1] + vec[2]*mat[2][2] +
                     w*mat[2][3]);
   dest[3] = (T)(vec[0]*mat[3][0] + vec[1]*mat[3][1] + vec[2]*mat[3][2] +
                     w*mat[3][3]);
}

// ===========================================================================

typedef Matrix2<float>  Matrix2f;
typedef Matrix2<double> Matrix2d;
typedef Matrix3<float>  Matrix3f;
typedef Matrix3<double> Matrix3d;
typedef Matrix4<float>  Matrix4f;
typedef Matrix4<double> Matrix4d;

// ===========================================================================
// identity Matrices for convenience

const Matrix2f imat2f(1.0f, 0.0f,
                      0.0f, 1.0f);
const Matrix3f imat3f(1.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 1.0f);
const Matrix4f imat4f(1.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 1.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 1.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 1.0f);

const Matrix2d imat2d(1.0, 0.0,
                      0.0, 1.0);
const Matrix3d imat3d(1.0, 0.0, 0.0,
                      0.0, 1.0, 0.0,
                      0.0, 0.0, 1.0);
const Matrix4d imat4d(1.0, 0.0, 0.0, 0.0,
                      0.0, 1.0, 0.0, 0.0,
                      0.0, 0.0, 1.0, 0.0,
                      0.0, 0.0, 0.0, 1.0);


#endif
