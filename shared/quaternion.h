/**
 * This source code is a part of AqWARium game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description 
 * Quaternion definitions. Adopted from SGL (Scene Graph Library).
 */

/*****************************************************************************
 *     File: Quaternion.hpp
 *   Author: Will Devore
 *  Created: 18 July 2000
 *  Summary: 4D templated quaternion class
 *****************************************************************************/

#ifndef __SGL_QUATERNION_HPP
#define __SGL_QUATERNION_HPP
#include "vector.h"
#include "matrix.h"

// ---------------------------------------------------------------------------
template <class T>
class Quaternion
{
public:
   // contructors
   Quaternion() {};

   Quaternion(T i, T j, T k, T w)
      {
         m_quat[0] = i; m_quat[1] = j;
         m_quat[2] = k; m_quat[3] = w;
      }

   template <class S>
   Quaternion(const Quaternion<S>& v)
      {
         m_quat[0] = v[0];
         m_quat[1] = v[1];
         m_quat[2] = v[2];
         m_quat[3] = v[3];
      }

   template <class S>
   Quaternion(const Vector4<S>& v)
      {
         m_quat[0] = v[0];
         m_quat[1] = v[1];
         m_quat[2] = v[2];
         m_quat[3] = v[3];
      }

   // axis/angle constructor
   template <class S>
   Quaternion(const Vector3<S>& v, S a)
      {
         S len = v.length();
         if (len > 0.0)
         {
            double s = sin( (double)a/2.0 );
            m_quat[0] = (T)s * v[0]/len;
            m_quat[1] = (T)s * v[1]/len;
            m_quat[2] = (T)s * v[2]/len;
            m_quat[3] = (T)cos( (double)a/2.0 );
         }
         else
         {
            m_quat[0] = m_quat[1] = m_quat[2] = 0.0;
            m_quat[3] = 1.0;
         }
      }

   // destructor
   ~Quaternion() {};  // not virtual, do not subclass

   // indexing operators
   inline       T& operator[](unsigned int i)       { return m_quat[i]; }
   inline const T& operator[](unsigned int i) const { return m_quat[i]; }

   // set from any other type
   template <class S>
   inline void set(S i, S j, S k, S w)
      {
         m_quat[0] = (T)i; m_quat[1] = (T)j;
         m_quat[2] = (T)k; m_quat[3] = (T)w;
      }

   template <class S>
   inline void set(const Vector3<S> &v, S a) // axis angle
      {
         S len = v.length();
         if (len > 0.0)
         {
            double s = sin( (double)a/2.0 );
            m_quat[0] = (T)s * v[0]/len;
            m_quat[1] = (T)s * v[1]/len;
            m_quat[2] = (T)s * v[2]/len;
            m_quat[3] = (T)cos( (double)a/2.0 );
         }
         else
         {
            m_quat[0] = m_quat[1] = m_quat[2] = 0.0;
            m_quat[3] = 1.0;
         }
      }

   // assignment operators
#ifndef _MSC_VER
   inline Quaternion& operator=(const Quaternion& q)
      {
         m_quat[0] = q[0];
         m_quat[1] = q[1];
         m_quat[2] = q[2];
         m_quat[3] = q[3];
         return *this;
      }
#endif

   template <class S>
   inline Quaternion &operator=(const Quaternion<S>& q)
      {
         m_quat[0] = (T)q[0];
         m_quat[1] = (T)q[1];
         m_quat[2] = (T)q[2];
         m_quat[3] = (T)q[3];
         return *this;
      }

   template <class S>
   inline Quaternion &operator=(const Vector4<S>& v)
      {
         m_quat[0] = v[0];
         m_quat[1] = v[1];
         m_quat[2] = v[2];
         m_quat[3] = v[3];

         return *this;
      }

   //------------------------------------------------------------------------
   // boolean operators
   template <class S>
   inline bool operator==(const Quaternion<S>& q) const
      {
         return (m_quat[0] == (T)q[0] && m_quat[1] == (T)q[1] &&
                 m_quat[2] == (T)q[2] && m_quat[3] == (T)q[3]);
      }

   template <class S>
   inline bool operator!=(const Quaternion<S>& q) const
      {
         return (m_quat[0] != (T)q[0] || m_quat[1] != (T)q[1] ||
                 m_quat[2] != (T)q[2] || m_quat[3] != (T)q[3]);
      }

   // math operators
   template <class S>
   inline Quaternion& operator+=(const Quaternion<S>& q)
      {
         m_quat[0] += (T)q[0]; m_quat[1] += (T)q[1];
         m_quat[2] += (T)q[2]; m_quat[3] += (T)q[3];
         return *this;
      }

   template <class S>
   inline Quaternion& operator-=(const Quaternion<S>& q)
      {
         m_quat[0] -= (T)q[0]; m_quat[1] -= (T)q[1];
         m_quat[2] -= (T)q[2]; m_quat[3] -= (T)q[3];
         return *this;
      }

   inline Quaternion& operator*=(const T s)
      {
         m_quat[0]*=s; m_quat[1]*=s; m_quat[2]*=s; m_quat[3]*=s;
         return *this;
      }

   template <class S>
   inline Quaternion& operator*=(const Quaternion<S> &rhs)
      {
         T q0 = (m_quat[3]*(T)rhs[0] +
                 m_quat[0]*(T)rhs[3] +
                 m_quat[1]*(T)rhs[2] -
                 m_quat[2]*(T)rhs[1]);
         T q1 = (m_quat[3]*(T)rhs[1] +
                 m_quat[1]*(T)rhs[3] +
                 m_quat[2]*(T)rhs[0] -
                 m_quat[0]*(T)rhs[2]);
         T q2 = (m_quat[3]*(T)rhs[2] +
                 m_quat[2]*(T)rhs[3] +
                 m_quat[0]*(T)rhs[1] -
                 m_quat[1]*(T)rhs[0]);
         T q3 = (m_quat[3]*(T)rhs[3] -
                 m_quat[0]*(T)rhs[0] -
                 m_quat[1]*(T)rhs[1] -
                 m_quat[2]*(T)rhs[2]);
         set(q0, q1, q2, q3);

         return *this;
      }

   template <class S>
   inline const Quaternion<T> operator+(const Quaternion<S> &rhs) const
      {
         return Quaternion(m_quat[0]+(T)rhs[0],
                              m_quat[1]+(T)rhs[1],
                              m_quat[2]+(T)rhs[2],
                              m_quat[3]+(T)rhs[3]);
      }

   template <class S>
   inline const Quaternion<T> operator-(const Quaternion<S> &rhs) const
      {
         return Quaternion(m_quat[0]-(T)rhs[0],
                              m_quat[1]-(T)rhs[1],
                              m_quat[2]-(T)rhs[2],
                              m_quat[3]-(T)rhs[3]);
      }

   inline const Quaternion<T> operator*(T scalar) const
      {
         return Quaternion(m_quat[0]*scalar,
                              m_quat[1]*scalar,
                              m_quat[2]*scalar,
                              m_quat[3]*scalar);
      }

   // multiply two quaternions
   template <class S>
   inline const Quaternion<T> operator*(const Quaternion<S> &rhs) const
      {
         return Quaternion(m_quat[3]*(T)rhs[0] +
                              m_quat[0]*(T)rhs[3] +
                              m_quat[1]*(T)rhs[2] -
                              m_quat[2]*(T)rhs[1],

                              m_quat[3]*(T)rhs[1] +
                              m_quat[1]*(T)rhs[3] +
                              m_quat[2]*(T)rhs[0] -
                              m_quat[0]*(T)rhs[2],

                              m_quat[3]*(T)rhs[2] +
                              m_quat[2]*(T)rhs[3] +
                              m_quat[0]*(T)rhs[1] -
                              m_quat[1]*(T)rhs[0],

                              m_quat[3]*(T)rhs[3] -
                              m_quat[0]*(T)rhs[0] -
                              m_quat[1]*(T)rhs[1] -
                              m_quat[2]*(T)rhs[2]);
      }

   // explicit math operations (for backward compatibility)
   template <class S1, class S2>
   inline void add(const Quaternion<S1> &q1, const Quaternion<S2> &q2)
      {
         m_quat[0] = (T)q1[0] + (T)q2[0];
         m_quat[1] = (T)q1[1] + (T)q2[1];
         m_quat[2] = (T)q1[2] + (T)q2[2];
         m_quat[3] = (T)q1[3] + (T)q2[3];
      }

   template <class S1, class S2>
   inline void sub(const Quaternion<S1>& q1,
                   const Quaternion<S2>& q2)
      {
         m_quat[0] = (T)q1[0] - (T)q2[0];
         m_quat[1] = (T)q1[1] - (T)q2[1];
         m_quat[2] = (T)q1[2] - (T)q2[2];
         m_quat[3] = (T)q1[3] - (T)q2[3];
      }

   template <class S>
   inline void scale(const Quaternion<S> &q, T s)
      {
         m_quat[0] = (T)q[0]*s;
         m_quat[1] = (T)q[1]*s;
         m_quat[2] = (T)q[2]*s;
         m_quat[3] = (T)q[3]*s;
      }

   // multiply two quaternions
   template <class S>
   inline void mul(const Quaternion<S> &rhs)
      {
         T q0 = (m_quat[3]*(T)rhs[0] +
                 m_quat[0]*(T)rhs[3] +
                 m_quat[1]*(T)rhs[2] -
                 m_quat[2]*(T)rhs[1]);
         T q1 = (m_quat[3]*(T)rhs[1] +
                 m_quat[1]*(T)rhs[3] +
                 m_quat[2]*(T)rhs[0] -
                 m_quat[0]*(T)rhs[2]);
         T q2 = (m_quat[3]*(T)rhs[2] +
                 m_quat[2]*(T)rhs[3] +
                 m_quat[0]*(T)rhs[1] -
                 m_quat[1]*(T)rhs[0]);
         T q3 = (m_quat[3]*(T)rhs[3] -
                 m_quat[0]*(T)rhs[0] -
                 m_quat[1]*(T)rhs[1] -
                 m_quat[2]*(T)rhs[2]);
         set(q0, q1, q2, q3);
      }

   // other ops
   template <class S>
   inline bool almostEqual(const Quaternion<S>& q, double eps) const
      {
         return (ALMOST_EQUAL_FUNCTION(m_quat[0], q[0], eps) &&
                 ALMOST_EQUAL_FUNCTION(m_quat[1], q[1], eps) &&
                 ALMOST_EQUAL_FUNCTION(m_quat[2], q[2], eps) &&
                 ALMOST_EQUAL_FUNCTION(m_quat[3], q[3], eps));
      }

   // reducing methods which result is a T
   template <class S>
   inline T dot(const Quaternion<S>& q) const
      {
         return (m_quat[0]*(T)q[0] + m_quat[1]*(T)q[1] +
                 m_quat[2]*(T)q[2] + m_quat[3]*(T)q[3]);
      }

   template <class S, class R>
   inline static R dot(const Quaternion<R>& q1, const Quaternion<S>& q2)
      {
         return (q1[0]*(R)q2[0] + q1[1]*(R)q2[1] +
                 q1[2]*(R)q2[2] + q1[3]*(R)q2[3]);
      }

   inline T norm() const
      {
         return (T) sqrt(dot(*this));
      }

   inline T normalize()
      {
         T len = norm();
         if ( len != 0.0 )
            *this *= (1.0/len);
         return len;
      }

   template <class S>
   inline T normalize(const Quaternion<S> &q)
      {
         T len = (T)q.norm();
         if (len > 0.0)
            scale(q, 1.0/len);
         return len;
      }

   inline void invert()
      {
         T len = norm();
         if (len > 0.0)
         {
            set(-m_quat[0]/len,
                -m_quat[1]/len,
                -m_quat[2]/len,
                 m_quat[3]/len);
         }
         else
            buildIdentity();
      }

   inline const Quaternion<T> conjugate( void ) const
      {
         T len = norm();
         if (len > 0.0)
         {
            return Quaternion(-m_quat[0]/len,
                                 -m_quat[1]/len,
                                 -m_quat[2]/len,
                                  m_quat[3]/len);
         }
         else
            return Quaternion(0.0, 0.0, 0.0, 1.0);
      }

   //------------------------------------------------------------------------
   // build functions
   inline void buildIdentity()
      {
         m_quat[0] = m_quat[1] = m_quat[2] = 0.0;
         m_quat[1] = 1.0;
      }

   template <class S>
   inline void buildFromAxisAngle(const Vector4<S> &v, S angle)
      {
         S len = v.norm();
         if (len > 0.0)
         {
            double s = sin( (T)angle/2.0 );
            m_quat[0] = s * (T)v[0]/len;
            m_quat[1] = s * (T)v[1]/len;
            m_quat[2] = s * (T)v[2]/len;
            m_quat[3] = cos( (T)angle/2.0 );
         }
         else
         {
            m_quat[0] = m_quat[1] = m_quat[2] = 0.0;
            m_quat[3] = 1.0;
         }
      }

   template <class S>
   inline void buildFromTransformation(const Matrix4<S> &mat)
      {
         double trace = mat[0][0] + mat[1][1] + mat[2][2];
         if (trace > 0.)
         {
            double s = sqrt(trace + 1.);
            m_quat[3] = (T)(s*0.5);
            s = 0.5/s;
            m_quat[0] = (T)((mat[1][2] - mat[2][1])*s);
            m_quat[1] = (T)((mat[2][0] - mat[0][2])*s);
            m_quat[2] = (T)((mat[0][1] - mat[1][0])*s);
         }
         else // negative or zero trace
         {
            unsigned int ix = 0;
            if (mat[1][1] > mat[0][0]) ix = 1;
            if (mat[2][2] > mat[1][1]) ix = 2;
            unsigned int iy = (ix+1)%3;
            unsigned int iz = (iy+1)%3;

            double s = sqrt((mat[ix][ix] - (mat[iy][iy] + mat[iz][iz])) + 1.);
            m_quat[ix] = (T)(s*0.5);

            if (s != 0.) s = 0.5/s;
            m_quat[3]  = (T)((mat[iy][iz] - mat[iz][iy])*s);
            m_quat[iy] = (T)((mat[ix][iy] + mat[iy][ix])*s);
            m_quat[iz] = (T)((mat[ix][iz] + mat[iz][ix])*s);
         }
      }

   //------------------------------------------------------------------------
   // Extractors
   //------------------------------------------------------------------------
   // An extractor for Axis Angle assuming a normalized quaternion
   template <class S>
   inline void getAxisAngle(Vector3<S> &axis, S &angle) const
      {
         if (ALMOST_EQUAL_FUNCTION(norm(), 1.0, 1.0e-4))
         {
            double st2 = sqrt(m_quat[0]*m_quat[0] +
                              m_quat[1]*m_quat[1] +
                              m_quat[2]*m_quat[2]);
            angle = 2.0*atan2(st2, m_quat[3]); // sin(angle/2), cos(angle/2);

            if (st2 > 1.0e-5)
            {
               axis.set((S)m_quat[0]/st2,
                        (S)m_quat[1]/st2,
                        (S)m_quat[2]/st2);
            }
            else
            {
               axis.set(1,0,0);  // some reasonable default axis
            }
         }
         else
         {
            axis.set(0,0,0);
            angle = 0;
         }
      }

   inline const Matrix4<T> getTransformation( void ) const
      {
         T Nq = dot( *this );
         T s = (Nq > 0.0) ? (2.0 / Nq) : 0.0;
         T xs = m_quat[0] * s;
         T ys = m_quat[1] * s;
         T zs = m_quat[2] * s;
         T wx = m_quat[3] * xs;
         T wy = m_quat[3] * ys;
         T wz = m_quat[3] * zs;
         T xx = m_quat[0] * xs;
         T xy = m_quat[0] * ys;
         T xz = m_quat[0] * zs;
         T yy = m_quat[1] * ys;
         T yz = m_quat[1] * zs;
         T zz = m_quat[2] * zs;

         return Matrix4<T>(1.0-(yy + zz), xy + wz,       xz - wy,       0.0,
                           xy - wz,       1.0-(xx + zz), yz + wx,       0.0,
                           xz + wy,       yz - wx,       1.0-(xx + yy), 0.0,
                           0.0,           0.0,           0.0,           1.0);
      }

   template <class S>
   inline Vector3<S> rotate(const Vector3<S> &vec) const
      {
         if (ALMOST_EQUAL_FUNCTION(norm(), 1.0f, (float) 1.0e-4))
         {
            S ss = (S)m_quat[3]*m_quat[3];
            S vdotn = (vec[0]*(S)m_quat[0] +
                       vec[1]*(S)m_quat[1] +
                       vec[2]*(S)m_quat[2]);
            Vector3<S> ncrossv((S)m_quat[1]*vec[2] - (S)m_quat[2]*vec[1],
                               (S)m_quat[2]*vec[0] - (S)m_quat[0]*vec[2],
                               (S)m_quat[0]*vec[1] - (S)m_quat[1]*vec[0]);
            Vector3<S> svnv(m_quat[3]*vec[0] + ncrossv[0],
                            m_quat[3]*vec[1] + ncrossv[1],
                            m_quat[3]*vec[2] + ncrossv[2]);

            return Vector3<S>(
               ss*vec[0] + m_quat[3]*ncrossv[0] + vdotn*m_quat[0] +
                           m_quat[1]*svnv[2] - m_quat[2]*svnv[1],
               ss*vec[1] + m_quat[3]*ncrossv[1] + vdotn*m_quat[1] +
                           m_quat[2]*svnv[0] - m_quat[0]*svnv[2],
               ss*vec[2] + m_quat[3]*ncrossv[2] + vdotn*m_quat[2] +
                           m_quat[0]*svnv[1] - m_quat[1]*svnv[0]);
         }
         else
         {
            // I don't think this result makes much sense.
            Quaternion<S> result =
               *this * Quaternion<S>(vec, (S)0.0) * (*this).conjugate();
            return Vector3<S>((S)result[0], (S)result[1], (S)result[2]);
         }
      }

private:
   T m_quat[4];
};

//----------------------------------------------------------------------------
typedef Quaternion<double> Quaterniond;
typedef Quaternion<float>  Quaternionf;

#endif
