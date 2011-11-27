/**
 * This source code is a part of AqWARium game project.
 * (c) Digital Dimension Development, 2004-2005
 *
 * @description 
 * Vector definitions. Adopted from SGL (Scene Graph Library).
 */

/*****************************************************************************
 * Copyright 1999, Scott McMillan
 *****************************************************************************
 *     File: sglVector.hpp
 *   Author: Scott McMillan
 *  Created: 2 January 1999
 *  Summary: 2D, 3D, and 4D vector classes
 *****************************************************************************/

#ifndef H21BB9F43_9844_4dd3_9F3B_9D2248A15B53
#define H21BB9F43_9844_4dd3_9F3B_9D2248A15B53

#include <math.h>

// ANSI value; 1E-7 should be fine too
const float fltEpsilon = (float) 1.0e-7;

// ANSI value, &1E-16 should be fine
const double dblEpsilon = 1.0e-16;

template <class T> inline T ABS_FUNCTION(T v) {
    return v>=0 ? v : -v;
}

template <class T> inline void SWAP_FUNCTION(T& a, T& b) {
    T t; t=a; a=b; b=t;
}

inline bool ALMOST_EQUAL_FUNCTION(float a, float b) {
    return (ABS_FUNCTION((a)-(b)) < fltEpsilon);
}

inline bool ALMOST_EQUAL_FUNCTION(float a, float b, float eps) {
    return (ABS_FUNCTION((a)-(b)) < eps);
}

inline bool ALMOST_EQUAL_FUNCTION(double a, double b) {
    return (ABS_FUNCTION((a)-(b)) < dblEpsilon);
}

inline bool ALMOST_EQUAL_FUNCTION(int a, int b) {
    return a==b;
}

template <class T>
class Vector2
{
public:
   // constructors
   Vector2() {};

   Vector2(T x, T y)
      { m_vector[0] = x; m_vector[1] = y; }

   template <class S>
   Vector2(const Vector2<S>& vec)
      { m_vector[0] = (T)vec[0]; m_vector[1] = (T)vec[1]; }

   // destructor
   ~Vector2() {};  // not virtual, do not subclass

   inline T *getPtr() const { return (T *)m_vector; }

   // indexing operators
   inline T& operator[](unsigned int i)             { return m_vector[i]; }
   inline const T& operator[](unsigned int i) const { return m_vector[i]; }

   // set from any other type
   template <class S>
   inline void set(S x, S y) { m_vector[0] = (T)x; m_vector[1] = (T)y; }  

   // assignment operator
#ifndef _MSC_VER
   inline Vector2& operator=(const Vector2& vec)
      {
         m_vector[0] = vec[0]; m_vector[1] = vec[1]; return *this;
      }
#endif

   template <class S>
   inline Vector2 &operator=(const Vector2<S>& vec)
      {
         m_vector[0] = (T)vec[0];  m_vector[1] = (T)vec[1]; return *this;
      }

   // boolean operators
   template <class S>
   inline bool operator==(const Vector2<S>& vec) const
      {
         return (m_vector[0] == (T)vec[0] && m_vector[1] == (T)vec[1]);
      }
   template <class S>
   inline bool operator!=(const Vector2<S>& vec) const
      {
         return (m_vector[0] != (T)vec[0] || m_vector[1] != (T)vec[1]);
      }

   template <class S>
   inline bool operator<(const Vector2<S>& vec) const
      {
         return (dot(*this) < vec.dot(vec));
      }
   template <class S>
   inline bool operator<=(const Vector2<S>& vec) const
      {
         return (dot(*this) <= vec.dot(vec));
      }
   template <class S>
   inline bool operator>(const Vector2<S>& vec) const
      {
         return (dot(*this) > vec.dot(vec));
      }
   template <class S>
   inline bool operator>=(const Vector2<S>& vec) const
      {
         return (dot(*this) >= vec.dot(vec));
      }

   // math operators
   template <class S>
   inline Vector2& operator+=(const Vector2<S>& vec)
      {
         m_vector[0] += (T)vec[0]; m_vector[1] += (T)vec[1]; return *this;
      }
   template <class S>
   inline Vector2& operator-=(const Vector2<S>& vec)
      {
         m_vector[0] -= (T)vec[0]; m_vector[1] -= (T)vec[1]; return *this;
      }
   inline Vector2& operator*=(const T s)
      {
         m_vector[0] *= s; m_vector[1] *= s; return *this;
      }
   inline Vector2& operator/=(const T s)
      {
         m_vector[0] /= s; m_vector[1] /= s; return *this;
      }

   template <class S>
   inline const Vector2<T> operator+(const Vector2<S> &rhs) const
      {
         return Vector2(m_vector[0]+(T)rhs[0],
                        m_vector[1]+(T)rhs[1]);
      }
   template <class S>
   inline const Vector2<T> operator-(const Vector2<S> &rhs) const
      {
         return Vector2(m_vector[0]-(T)rhs[0],
                        m_vector[1]-(T)rhs[1]);
      }
   inline const Vector2<T> operator*(T scalar) const
      {
         return Vector2(m_vector[0]*(T)scalar, m_vector[1]*(T)scalar);
      }
   inline const Vector2<T> operator/(T scalar) const
      {
         return Vector2(m_vector[0]/(T)scalar, m_vector[1]/(T)scalar);
      }

   // explicit math operations (for backward compatibility)
   template <class S1, class S2>
   inline void add(const Vector2<S1>& vec1, const Vector2<S2>& vec2)
      {
         m_vector[0] = (T)vec1[0] + (T)vec2[0];
         m_vector[1] = (T)vec2[1] + (T)vec1[1];
      }
   template <class S1, class S2>
   inline void sub(const Vector2<S1>& vec1, const Vector2<S2>& vec2)
      {
         m_vector[0] = (T)vec1[0] - (T)vec2[0];
         m_vector[1] = (T)vec1[1] - (T)vec2[1];
      }
   template <class S>
   inline void scale(const Vector2<S>& vec, T s)
      {
         m_vector[0] = (T)vec[0]*s;
         m_vector[1] = (T)vec[1]*s;
      }

   // other ops
   template <class S>
   inline bool almostEqual(const Vector2<S>& vec, double eps) const
      {
         return (ALMOST_EQUAL_FUNCTION(m_vector[0], vec[0], eps) &&
                 ALMOST_EQUAL_FUNCTION(m_vector[1], vec[1], eps));
      }

   template <class S>
   inline T dot(const Vector2<S>& vec) const
      {
         return (m_vector[0]*(T)vec[0] + m_vector[1]*(T)vec[1]);
      }

   template <class S, class R>
   inline static R dot(const Vector2<R>& vec1, const Vector2<S>& vec2)
      {
         return vec1[0]*(R)vec2[0] + vec1[1]*(R)vec2[1];
      }

   inline T length() const
      {
         return (T) sqrt(dot(*this));
      }

   template <class S>
   inline T distance(const Vector2<S>& vec) const
      {
         return (T) sqrt(((T)vec[0] - m_vector[0])*((T)vec[0] - m_vector[0]) +
                         ((T)vec[1] - m_vector[1])*((T)vec[1] - m_vector[1]));
      }

   inline T normalize()
      {
         T len = length();
         if (len) *this *= ( 1.0f/len );
         return len;
      }

   template <class S>
   inline T normalize(const Vector2<S>& vec)
      {
         T len = vec.length();
         if (len) scale(vec, 1.0/len);
         return len;
      }

private:
   T m_vector[2];
};



template <class T>
class Vector3
{
public:
   // constructors
   Vector3() {};

   Vector3(T x, T y, T z)
      { m_vector[0] = x; m_vector[1] = y; m_vector[2] = z; }

   template <class S>
   Vector3(const Vector3<S>& vec)
      {
         m_vector[0] = (T)vec[0];
         m_vector[1] = (T)vec[1];
         m_vector[2] = (T)vec[2];
      }

   // destructor
   ~Vector3() {};  // not virtual, do not subclass

   inline T *getPtr() const { return (T *)m_vector; }

   // indexing operators
   inline       T& operator[](unsigned int i)       { return m_vector[i]; }
   inline const T& operator[](unsigned int i) const { return m_vector[i]; }

   // set from any other type
   template <class S>
   inline void set(S x, S y, S z)
      {
         m_vector[0] = (T)x; m_vector[1] = (T)y; m_vector[2] = (T)z;
      }  
    
   // assignment operators
#ifndef _MSC_VER
   inline Vector3& operator=(const Vector3& vec)
      {
         m_vector[0] = vec[0];
         m_vector[1] = vec[1];
         m_vector[2] = vec[2];
         return *this;
      }
#endif

   template <class S>
   inline Vector3 &operator=(const Vector3<S>& vec)
      {
         m_vector[0] = (T)vec[0];
         m_vector[1] = (T)vec[1];
         m_vector[2] = (T)vec[2];
         return *this;
      }

   // boolean operators
   template <class S>
   inline bool operator==(const Vector3<S>& vec) const
      {
         return (m_vector[0] == (T)vec[0] &&
                 m_vector[1] == (T)vec[1] &&
                 m_vector[2] == (T)vec[2]);
      }
   template <class S>
   inline bool operator!=(const Vector3<S>& vec) const
      {
         return (m_vector[0] != (T)vec[0] ||
                 m_vector[1] != (T)vec[1] ||
                 m_vector[2] != (T)vec[2]);
      }

   template <class S>
   inline bool operator<(const Vector3<S>& vec) const
      {
         return (dot(*this) < vec.dot(vec));
      }
   template <class S>
   inline bool operator<=(const Vector3<S>& vec) const
      {
         return (dot(*this) <= vec.dot(vec));
      }
   template <class S>
   inline bool operator>(const Vector3<S>& vec) const
      {
         return (dot(*this) > vec.dot(vec));
      }
   template <class S>
   inline bool operator>=(const Vector3<S>& vec) const
      {
         return (dot(*this) >= vec.dot(vec));
      }

   // math operators
   template <class S>
   inline Vector3& operator+=(const Vector3<S>& vec)
      {
         m_vector[0]+=(T)vec[0];
         m_vector[1]+=(T)vec[1];
         m_vector[2]+=(T)vec[2];
         return *this;
      }
   template <class S>
   inline Vector3& operator-=(const Vector3<S>& vec)
      {
         m_vector[0]-=(T)vec[0];
         m_vector[1]-=(T)vec[1];
         m_vector[2]-=(T)vec[2];
         return *this;
      }
   inline Vector3& operator*=(const T s)
      {
         m_vector[0] *= s;
         m_vector[1] *= s;
         m_vector[2] *= s;
         return *this;
      }
   inline Vector3& operator/=(const T s)
      {
         m_vector[0] /= s;
         m_vector[1] /= s;
         m_vector[2] /= s;
         return *this;
      }

   template <class S>
   inline const Vector3<T> operator+(const Vector3<S> &rhs) const
      {
         return Vector3(m_vector[0]+(T)rhs[0],
                        m_vector[1]+(T)rhs[1],
                        m_vector[2]+(T)rhs[2]);
      }
   template <class S>
   inline const Vector3<T> operator-(const Vector3<S> &rhs) const
      {
         return Vector3(m_vector[0]-(T)rhs[0],
                        m_vector[1]-(T)rhs[1],
                        m_vector[2]-(T)rhs[2]);
      }
   inline const Vector3<T> operator*(T scalar) const
      {
         return Vector3(m_vector[0]*scalar,
                        m_vector[1]*scalar,
                        m_vector[2]*scalar);
      }
   inline const Vector3<T> operator/(T scalar) const
      {
         return Vector3(m_vector[0]/(T)scalar,
                        m_vector[1]/(T)scalar,
                        m_vector[2]/(T)scalar);
      }

   // explicit math operations (for backward compatibility)
   template <class S1, class S2>
   inline void add(const Vector3<S1>& vec1, const Vector3<S2>& vec2)
      {
         m_vector[0] = (T)vec1[0] + (T)vec2[0];
         m_vector[1] = (T)vec1[1] + (T)vec2[1];
         m_vector[2] = (T)vec1[2] + (T)vec2[2];
      }
   template <class S1, class S2>
   inline void sub(const Vector3<S1>& vec1, const Vector3<S2>& vec2)
      {
         m_vector[0] = (T)vec1[0] - (T)vec2[0];
         m_vector[1] = (T)vec1[1] - (T)vec2[1];
         m_vector[2] = (T)vec1[2] - (T)vec2[2]; 
      }
   template <class S>
   inline void scale(const Vector3<S>& vec, T s)
      {
         m_vector[0] = (T)vec[0]*s;
         m_vector[1] = (T)vec[1]*s;
         m_vector[2] = (T)vec[2]*s;
      }


   // other ops
   template <class S>
   inline bool almostEqual(const Vector3<S>& vec, double eps) const
      {
         return (ALMOST_EQUAL_FUNCTION(m_vector[0], vec[0], eps) &&
                 ALMOST_EQUAL_FUNCTION(m_vector[1], vec[1], eps) &&
                 ALMOST_EQUAL_FUNCTION(m_vector[2], vec[2], eps));
      }

   template <class S>
   inline void cross(const Vector3<S>& vec)
      {
         // temporaries required in case vec == this
         T x         = m_vector[1]*(T)vec[2] - m_vector[2]*(T)vec[1];
         T y         = m_vector[2]*(T)vec[0] - m_vector[0]*(T)vec[2];
         m_vector[2] = m_vector[0]*(T)vec[1] - m_vector[1]*(T)vec[0];
         m_vector[0] = x; m_vector[1] = y;
      }

   template <class S1, class S2>
   inline void cross(const Vector3<S1>& vec1, const Vector3<S2>& vec2)
      {
         // temporaries required in case vec1 or vec2 == this
         T x         = (T)vec1[1]*(T)vec2[2] - (T)vec1[2]*(T)vec2[1];
         T y         = (T)vec1[2]*(T)vec2[0] - (T)vec1[0]*(T)vec2[2];
         m_vector[2] = (T)vec1[0]*(T)vec2[1] - (T)vec1[1]*(T)vec2[0];
         m_vector[0] = x; m_vector[1] = y;
      }

   // reducing methods which result is a T
   template <class S>
   inline T dot(const Vector3<S>& vec) const
      {
         return (m_vector[0]*(T)vec[0] +
                 m_vector[1]*(T)vec[1] +
                 m_vector[2]*(T)vec[2]);
      }

   template <class S, class R>
   inline static R dot(const Vector3<R>& vec1, const Vector3<S>& vec2)
      {
         return (vec1[0]*(R)vec2[0] +
                 vec1[1]*(R)vec2[1] +
                 vec1[2]*(R)vec2[2]);
      }

   inline T length() const
      {
         return (T) sqrt(dot(*this));
      }

   template <class S>
   inline T distance(const Vector3<S>& vec) const
      {
         return (T) sqrt(((T)vec[0] - m_vector[0])*((T)vec[0] - m_vector[0])+
                         ((T)vec[1] - m_vector[1])*((T)vec[1] - m_vector[1])+
                         ((T)vec[2] - m_vector[2])*((T)vec[2] - m_vector[2]));
      }

   inline T normalize()
      {
         T len = length();
         if (len) *this *= (1.0f/len);
         return len;
      }

   template <class S>
   inline T normalize(const Vector3<S>& vec)
      {
         T len = vec.length();
         if (len) scale(vec, 1.0f/len);
         return len;
      }

private:
   T m_vector[3];
};


template <class T>
class Vector4
{
public:
    // contructors
   Vector4() {}; 

   Vector4(T x, T y, T z, T w)
      {
         m_vector[0] = x; m_vector[1] = y;
         m_vector[2] = z; m_vector[3] = w;
      }

   template <class S>
   Vector4(const Vector4<S>& vec)
      {
         m_vector[0] = (T)vec[0]; m_vector[1] = (T)vec[1];
         m_vector[2] = (T)vec[2]; m_vector[3] = (T)vec[3];
      }

   // destructor
   ~Vector4() {};  // not virtual, do not subclass

   inline T *getPtr() const { return (T *)m_vector; }

   // indexing operators
   inline       T& operator[](unsigned int i)       { return m_vector[i]; }
   inline const T& operator[](unsigned int i) const { return m_vector[i]; }

   // set from any other type
   template <class S>
   inline void set(S x, S y, S z, S w)
      {
         m_vector[0] = (T)x; m_vector[1] = (T)y;
         m_vector[2] = (T)z; m_vector[3] = (T)w;
      }  

   // from 3 to 4 D
   template <class S>
   inline void set(Vector3<S>& vec, S s = (S)1)
      {
         m_vector[0] = (T)vec[0]; m_vector[1] = (T)vec[1];
         m_vector[2] = (T)vec[2]; m_vector[3] = (T)s;
      }

   // assignment operators
#ifndef _MSC_VER
   inline Vector4& operator=(const Vector4& vec)
      {
         m_vector[0] = vec[0]; m_vector[1] = vec[1];
         m_vector[2] = vec[2]; m_vector[3] = vec[3];
         return *this;
      }
#endif

   template <class S>
   inline Vector4 &operator=(const Vector4<S>& vec)
      {
         m_vector[0] = (T)vec[0]; m_vector[1] = (T)vec[1];
         m_vector[2] = (T)vec[2]; m_vector[3] = (T)vec[3];
         return *this;
      }

   // boolean operators
   template <class S>
   inline bool operator==(const Vector4<S>& vec) const
      {
         return (m_vector[0] == (T)vec[0] && m_vector[1] == (T)vec[1] &&
                 m_vector[2] == (T)vec[2] && m_vector[3] == (T)vec[3]);
      }

   template <class S>
   inline bool operator!=(const Vector4<S>& vec) const
      {
         return (m_vector[0] != (T)vec[0] || m_vector[1] != (T)vec[1] || 
                 m_vector[2] != (T)vec[2] || m_vector[3] != (T)vec[3]);
      }

   template <class S>
   inline bool operator<(const Vector4<S>& vec) const
      {
         return (dot(*this) < vec.dot(vec));
      }
   template <class S>
   inline bool operator<=(const Vector4<S>& vec) const
      {
         return (dot(*this) <= vec.dot(vec));
      }
   template <class S>
   inline bool operator>(const Vector4<S>& vec) const
      {
         return (dot(*this) > vec.dot(vec));
      }
   template <class S>
   inline bool operator>=(const Vector4<S>& vec) const
      {
         return (dot(*this) >= vec.dot(vec));
      }

   // math operators
   template <class S>
   inline Vector4& operator+=(const Vector4<S>& vec)
      {
         m_vector[0] += (T)vec[0]; m_vector[1] += (T)vec[1];
         m_vector[2] += (T)vec[2]; m_vector[3] += (T)vec[3];
         return *this;
      }
   template <class S>
   inline Vector4& operator-=(const Vector4<S>& vec)
      {
         m_vector[0] -= (T)vec[0]; m_vector[1] -= (T)vec[1];
         m_vector[2] -= (T)vec[2]; m_vector[3] -= (T)vec[3];
         return *this;
      }
   inline Vector4& operator*=(const T s)
      {
         m_vector[0]*=s; m_vector[1]*=s; m_vector[2]*=s; m_vector[3]*=s;
         return *this;
      }

   template <class S>
   inline const Vector4<T> operator+(const Vector4<S> &rhs) const
      {
         return Vector4(m_vector[0]+(T)rhs[0],
                        m_vector[1]+(T)rhs[1],
                        m_vector[2]+(T)rhs[2],
                        m_vector[3]+(T)rhs[3]);
      }
   template <class S>
   inline const Vector4<T> operator-(const Vector4<S> &rhs) const
      {
         return Vector4(m_vector[0]-(T)rhs[0],
                        m_vector[1]-(T)rhs[1],
                        m_vector[2]-(T)rhs[2],
                        m_vector[3]-(T)rhs[3]);
      }
   inline const Vector4<T> operator*(T scalar) const
      {
         return Vector4(m_vector[0]*scalar,
                        m_vector[1]*scalar,
                        m_vector[2]*scalar,
                        m_vector[3]*scalar);
      }
   inline const Vector4<T> operator/(T scalar) const
      {
         return Vector4(m_vector[0]/(T)scalar,
                        m_vector[1]/(T)scalar,
                        m_vector[2]/(T)scalar,
                        m_vector[3]/(T)scalar);
      }

   // explicit math operations (for backward compatibility)
   template <class S1, class S2>
   inline void add(const Vector4<S1>& vec1, const Vector4<S2>& vec2)
      {
         m_vector[0] = (T)vec1[0] + (T)vec2[0];
         m_vector[1] = (T)vec1[1] + (T)vec2[1];
         m_vector[2] = (T)vec1[2] + (T)vec2[2];
         m_vector[3] = (T)vec1[3] + (T)vec2[3];
      }
   template <class S1, class S2>
   inline void sub(const Vector4<S1>& vec1, const Vector4<S2>& vec2)
      {
         m_vector[0] = (T)vec1[0] - (T)vec2[0];
         m_vector[1] = (T)vec1[1] - (T)vec2[1];
         m_vector[2] = (T)vec1[2] - (T)vec2[2];
         m_vector[3] = (T)vec1[3] - (T)vec2[3]; 
      }
   template <class S>
   inline void scale(const Vector4<S>& vec, T s)
      {
         m_vector[0] = (T)vec[0]*s; m_vector[1] = (T)vec[1]*s;
         m_vector[2] = (T)vec[2]*s; m_vector[3] = (T)vec[3]*s;
      }

   // other ops
   template <class S>
   inline bool almostEqual(const Vector4<S>& vec, double eps) const
      {
         return (ALMOST_EQUAL_FUNCTION(m_vector[0], vec[0], eps) && 
                 ALMOST_EQUAL_FUNCTION(m_vector[1], vec[1], eps) &&
                 ALMOST_EQUAL_FUNCTION(m_vector[2], vec[2], eps) &&
                 ALMOST_EQUAL_FUNCTION(m_vector[3], vec[3], eps));
      }

   // reducing methods which result is a T
   template <class S>
   inline T dot(const Vector4<S>& vec) const
      {
         return (m_vector[0]*(T)vec[0] + m_vector[1]*(T)vec[1] +
                 m_vector[2]*(T)vec[2] + m_vector[3]*(T)vec[3]);
      }

   template <class S, class R>
   inline static R dot(const Vector4<R>& vec1, const Vector4<S>& vec2)
      {
         return (vec1[0]*(R)vec2[0] + vec1[1]*(R)vec2[1] +
                 vec1[2]*(R)vec2[2] + vec1[3]*(R)vec2[3]);
      }

   inline T length() const
      {
         return (T) sqrt(dot(*this));
      }

   template <class S>
   inline T distance(const Vector4<S>& vec) const
      {
         return (T) sqrt(((T)vec[0] - m_vector[0])*((T)vec[0] - m_vector[0]) + 
                         ((T)vec[1] - m_vector[1])*((T)vec[1] - m_vector[1]) + 
                         ((T)vec[2] - m_vector[2])*((T)vec[2] - m_vector[2]) + 
                         ((T)vec[3] - m_vector[3])*((T)vec[3] - m_vector[3]));
      }

   inline T normalize()
      {
         T len = length();
         if (len) *this *= (1.0/len);
         return len;
      }

   template <class S>
   inline T normalize(const Vector4<S>& vec)
      {
         T len = vec.length();
         if (len) scale(vec, 1.0/len);
         return len;
      }

private:
   T m_vector[4];
};



typedef Vector2<long>   Vector2l;
typedef Vector2<int>    Vector2i;
typedef Vector2<char>   Vector2c;
typedef Vector2<float>  Vector2f;
typedef Vector2<double> Vector2d;
typedef Vector3<long>   Vector3l;
typedef Vector3<int>    Vector3i;
typedef Vector3<char>   Vector3c;
typedef Vector3<float>  Vector3f;
typedef Vector3<double> Vector3d;
typedef Vector4<long>   Vector4l;
typedef Vector4<int>    Vector4i;
typedef Vector4<char>   Vector4c;
typedef Vector4<float>  Vector4f;
typedef Vector4<double> Vector4d;


#endif
