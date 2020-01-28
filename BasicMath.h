//********************************************************* 
// 
// Copyright (c) Microsoft. All rights reserved. 
// This code is licensed under the MIT License (MIT). 
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF 
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY 
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR 
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT. 
// 
//*********************************************************

#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
//#include <DirectXMath.h>


// This header defines math and matrix helper functions and structures used 
// by DirectX SDK samples.

// Common Constants

#define M_E_F 2.71828183f
#define M_PI_F 3.1415927f
#define TO_RADF (M_PI_F/180.0f)
#define TO_DEGF (180.0f/M_PI_F)

//#define max(x,y) ((x)>(y)?(x):(y))
//#define min(x,y) ((x)<(y)?(x):(y))

// Template Vector & Matrix Classes

template <class T> struct Vector2
{
	union
	{
		struct
		{
			T x;
			T y;
		};
		struct
		{
			T r;
			T g;
		};
		struct
		{
			T u;
			T v;
		};
	};

	Vector2(const float v[])
	{
		*this = v; // use assignment operator implementation
	}

	// assignment operator
	Vector2& operator=(const float v[])
	{
		this->x = v[0];
		this->y = v[1];
		return *this;
	}

	T& operator[](unsigned int index)
	{
		return static_cast<T*>(this)[index];
	}

	Vector2(T _x = 0, T _y = 0) : x(_x), y(_y) { }
};

template <class T> struct Vector3
{
	union
	{
		struct
		{
			T x;
			T y;
			T z;
		};
		struct
		{
			T r;
			T g;
			T b;
		};
		struct
		{
			T u;
			T v;
			T w;		// this is really risky
		};
	};

	Vector3(const float v[])
	{
		*this = v; // use assignment operator implementation
	}

	// assignment operator
	Vector3& operator=(const float v[])
	{
		this->x = v[0];
		this->y = v[1];
		this->y = v[2];
		return *this;
	}

	T& operator[](unsigned int index)
	{
		return static_cast<T*>(this)[index];
	}

	Vector3(T _x = 0, T _y = 0, T _z = 0) : x(_x), y(_y), z(_z) { }
};


template <class T> struct Matrix3x3
{
	union
	{
		struct
		{
			T _11; T _12; T _13;
			T _21; T _22; T _23;
			T _31; T _32; T _33;
		};
		struct
		{
			T _m00; T _m01; T _m02;
			T _m10; T _m11; T _m12;
			T _m20; T _m21; T _m22;
		};
	};

	Matrix3x3(T value = 0)
	{
		_11 = _12 = _13 = value;
		_21 = _22 = _23 = value;
		_31 = _32 = _33 = value;
	}

	Matrix3x3(
		T i11, T i12, T i13,
		T i21, T i22, T i23,
		T i31, T i32, T i33
	)
	{
		_11 = i11; _12 = i12; _13 = i13;
		_21 = i21; _22 = i22; _23 = i23;
		_31 = i31; _32 = i32; _33 = i33;
	}

	T* operator[](unsigned int index)
	{
		return &(reinterpret_cast<T*>(this)[index * 3]);
	}
};

template <class T> struct Vector4
{
	union
	{
		struct
		{
			T x;
			T y;
			T z;
			T w;
		};
		struct
		{
			T r;
			T g;
			T b;
			T a;
		};
	};

	Vector4(const float v[])
	{
		*this = v; // use assignment operator implementation
	}

	// assignment operator
	Vector4& operator=(const float v[])
	{
		this->x = v[0];
		this->y = v[1];
		this->y = v[2];
		this->y = v[3];
		return *this;
	}

	T& operator[](unsigned int index)
	{
		return static_cast<T*>(this)[index];
	}

	Vector4(T _x = 0, T _y = 0, T _z = 0, T _w = 0) : x(_x), y(_y), z(_z), w(_w) { }
};

template <class T> struct Matrix4x4
{
	union
	{
		struct
		{
			T _11; T _12; T _13; T _14;
			T _21; T _22; T _23; T _24;
			T _31; T _32; T _33; T _34;
			T _41; T _42; T _43; T _44;
		};
		struct
		{
			T _m00; T _m01; T _m02; T _m03;
			T _m10; T _m11; T _m12; T _m13;
			T _m20; T _m21; T _m22; T _m23;
			T _m30; T _m31; T _m32; T _m33;
		};
	};

	Matrix4x4(T value = 0)
	{
		_11 = _12 = _13 = _14 = value;
		_21 = _22 = _23 = _24 = value;
		_31 = _32 = _33 = _34 = value;
		_41 = _42 = _43 = _44 = value;
	}

	Matrix4x4(
		T i11, T i12, T i13, T i14,
		T i21, T i22, T i23, T i24,
		T i31, T i32, T i33, T i34,
		T i41, T i42, T i43, T i44
		)
	{
		_11 = i11; _12 = i12; _13 = i13; _14 = i14;
		_21 = i21; _22 = i22; _23 = i23; _24 = i24;
		_31 = i31; _32 = i32; _33 = i33; _34 = i34;
		_41 = i41; _42 = i42; _43 = i43; _44 = i44;
	}

	T* operator[](unsigned int index)
	{
		return &(reinterpret_cast<T*>(this)[index*4]);
	}
};
// TODO really should add Double4x4


// Template Vector Operations

template <class T>
T dot(Vector2<T> a, Vector2<T> b)
{
	return a.x * b.x + a.y * b.y;
}

template <class T>
T dot(Vector3<T> a, Vector3<T> b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <class T>
T dot(Vector4<T> a, Vector4<T> b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w + b.w;
}

template <class T>
T length(Vector2<T> a)
{
	return sqrtf(a.x * a.x + a.y * a.y);
}

template <class T>
T length(Vector3<T> a)
{
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);
}

template <class T>
T length(Vector4<T> a)
{
	return sqrtf(a.x * a.x + a.y * a.y + a.z * a.z + a.w * a.w);
}

template <class T>
T distance( Vector2<T> a, Vector2<T> b )
{
	return sqrtf( dot( a-b, a-b ) );
}

template <class T>
T distance( Vector3<T> a, Vector3<T> b )
{
	return sqrtf( dot( a-b, a-b ) );
}

template <class T>
T distance(Vector4<T> a, Vector4<T> b)
{
	return sqrtf( dot( a-b, a-b ) );
}

template <class T>
float cross(Vector2<T> a, Vector2<T> b)
{
	return (a.x*b.y) - (a.y*b.x);
}
template <class T>
Vector3<T> cross(Vector3<T> a, Vector3<T> b)
{
	return Vector3<T>((a.y*b.z)-(a.z*b.y), (a.z*b.x)-(a.x*b.z), (a.x*b.y)-(a.y*b.x));
}

template <class T>
Vector2<T> normalize(Vector2<T> a)
{
	T len = length(a);
	return Vector2<T>(a.x / len, a.y / len);
}

template <class T>
Vector3<T> normalize(Vector3<T> a)
{
	T len = length(a);
	return Vector3<T>(a.x / len, a.y / len, a.z / len);
}

template <class T>
Vector4<T> normalize(Vector4<T> a)
{
	T len = length(a);
	return Vector4<T>(a.x / len, a.y / len, a.z / len, a.w / len);
}

// Template Vector Operators

template <class T>
Vector2<T> operator-(Vector2<T> a, Vector2<T> b)
{
	return Vector2<T>(a.x - b.x, a.y - b.y);
}

template <class T>
Vector2<T> operator-(Vector2<T> a)
{
	return Vector2<T>(-a.x, -a.y);
}

template <class T>
Vector3<T> operator-(Vector3<T> a, Vector3<T> b)
{
	return Vector3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template <class T>
Vector3<T> operator-(Vector3<T> a)
{
	return Vector3<T>(-a.x, -a.y, -a.z);
}

template <class T>
Vector4<T> operator-(Vector4<T> a, Vector4<T> b)
{
	return Vector4<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

template <class T>
Vector4<T> operator-(Vector4<T> a)
{
	return Vector4<T>(-a.x, -a.y, -a.z, -a.w);
}

template <class T>
Vector2<T> operator+(Vector2<T> a, Vector2<T> b)
{
	return Vector2<T>(a.x + b.x, a.y + b.y);
}

template <class T>
Vector3<T> operator+(Vector3<T> a, Vector3<T> b)
{
	return Vector3<T>(a.x + b.x, a.y + b.y, a.z + b.z);
}
/*
template <class T>
Vector3<T> operator+=(Vector3<T> a, Vector3<T> b)	
{
//	return Vector3<T>(a.x + b.x, a.y + b.y, a.z + b.z);		// this is a no-op for some reason  BUG TODO
	a.x += b.x; a.y += b.y; a.z += b.z;	return a;
}
*/

template <class T>
Vector4<T> operator+(Vector4<T> a, Vector4<T> b)
{
	return Vector4<T>(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

template <class T>
Vector2<T> operator*(Vector2<T> a, T s)
{
	return Vector2<T>(a.x * s, a.y * s);
}

template <class T>
Vector2<T> operator*(T s, Vector2<T> a)
{
	return a * s;
}

template <class T>
Vector2<T> operator*(Vector2<T> a, Vector2<T> b)
{
	return Vector2<T>(a.x * b.x, a.y * b.y);
}

template <class T>
Vector2<T> operator/(Vector2<T> a, T s)
{
	return Vector2<T>(a.x / s, a.y / s);
}
template <class T>
Vector2<T> operator/=(Vector2<T> a, T s)
{
	return Vector2<T>(a.x / s, a.y / s);
}

template <class T>
Vector3<T> operator*(Vector3<T> a, T s)
{
	return Vector3<T>(a.x * s, a.y * s, a.z * s);
}
template <class T>
Vector3<T> operator*=(Vector3<T> a, T s)
{
	return Vector3<T>(a.x * s, a.y * s, a.z * s);
}

template <class T>
Vector3<T> operator*(T s, Vector3<T> a)
{
	return a * s;
}

template <class T>
Vector3<T> operator*(Vector3<T> a, Vector3<T> b)
{
	return Vector3<T>(a.x * b.x, a.y * b.y, a.z * b.z);
}

template <class T>
Vector3<T> operator/(Vector3<T> a, T s)
{
	return Vector3<T>(a.x / s, a.y / s, a.z / s);
}
template <class T>
Vector3<T> operator/=(Vector3<T> a, T s)
{
	return Vector3<T>(a.x / s, a.y / s, a.z / s);
}

template <class T>
Vector3<T> operator/(Vector3<T> a, Vector3<T> b)
{
	return Vector3<T>(a.x / b.x, a.y / b.y, a.z / b.z);
}

template <class T>
Vector3<T> operator/=(Vector3<T> a, Vector3<T> b)
{
	return Vector3<T>(a.x / b.x, a.y / b.y, a.z / b.z);
}

template <class T>
Vector4<T> operator*(Vector4<T> a, T s)
{
	return Vector4<T>(a.x * s, a.y * s, a.z * s, a.w * s);
}

template <class T>
Vector4<T> operator*(T s, Vector4<T> a)
{
	return a * s;
}

template <class T>
Vector4<T> operator*(Vector4<T> a, Vector4<T> b)
{
	return Vector4<T>(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}

template <class T>
Vector4<T> operator/(Vector4<T> a, T s)
{
	return Vector4<T>(a.x / s, a.y / s, a.z / s, a.w / s);
}

// project a vector into a plane with normal
template <class T>
Vector3<T> project( Vector3<T> vec, Vector3<T> normal )
{
	float component = dot( vec, normal );		// component of vec normal to plane
	return vec - component*normal;				// subtract to get in-plane component
}

// project a point into a plane defined by origin and normal
template <class T>
Vector3<T> project( Vector3<T> point, Vector3<T> origin, Vector3<T> normal )
{
	Vector3<T> toPoint = point - origin;
	return project( toPoint, normal ) + origin;
}

// clamp all components to 0..1 range
template <class T>
Vector2<T> saturate(Vector2<T> v)
{
	v.x = (v.x > 1.0f ? 1.0f : v.x);
	v.x = (v.x < 0.0f ? 0.0f : v.x);
	v.y = (v.y > 1.0f ? 1.0f : v.y);
	v.y = (v.y < 0.0f ? 0.0f : v.y);

	return Vector2<T>(v.x, v.y );
}

// clamp all components to 0..1 range
template <class T>
Vector3<T> saturate( Vector3<T> v)
{
	v.x = (v.x > 1.0f ? 1.0f : v.x);
	v.x = (v.x < 0.0f ? 0.0f : v.x);
	v.y = (v.y > 1.0f ? 1.0f : v.y);
	v.y = (v.y < 0.0f ? 0.0f : v.y);
	v.z = (v.z > 1.0f ? 1.0f : v.z);
	v.z = (v.z < 0.0f ? 0.0f : v.z);

	return Vector3<T>(v.x, v.y, v.z);
}

#if 0
float clamp(float v, float min, float max)
{
	if (v > max) v = max; else
	if (v < min) v = min;
	return v;
}
#endif

template <class T>
Vector2<T> clamp(Vector2<T> v, float min, float max)
{
	if (v.x > max) v.x = max; else
	if (v.x < min) v.x = min;
	if (v.y > max) v.y = max; else
	if (v.y < min) v.y = min;
	return Vector2<T>(v.x, v.y, v.z);
}

template <class T>
Vector3<T> clamp(Vector3<T> v, float min, float max)
{
	if (v.x > max) v.x = max; else
	if (v.x < min) v.x = min;
	if (v.y > max) v.y = max; else
	if (v.y < min) v.y = min;
	if (v.z > max) v.z = max; else
	if (v.z < min) v.z = min;
	return Vector3<T>(v.x, v.y, v.z);
}

template <class T>
Vector4<T> clamp(Vector4<T> v, float min, float max)
{
	if (v.x > max) v.x = max; else
	if (v.x < min) v.x = min;
	if (v.y > max) v.y = max; else
	if (v.y < min) v.y = min;
	if (v.z > max) v.z = max; else
	if (v.z < min) v.z = min;
	if (v.w > max) v.w = max; else
	if (v.w < min) v.w = min;
	return Vector4<T>(v.x, v.y, v.z);
}

// Template Matrix Operations
// 3x3
template <class T>
Vector3<T> mul(Matrix3x3<T> m, Vector3<T> v)
{
	return Vector3<T>
		(v.x * m._11 + v.y * m._12 + v.z * m._13,
		 v.x * m._21 + v.y * m._22 + v.z * m._23,
		 v.x * m._31 + v.y * m._32 + v.z * m._33 );
}

template <class T>
Vector3<T> operator * (Matrix3x3<T> m, Vector3<T> v)
{
	return mul(m, v);
}

template <class T>
Vector3<T> mul(Vector3<T> v, Matrix3x3<T> m)
{
	return Vector3<T>
	(v.x * m._11 + v.y * m._21 + v.z * m._31,
		v.x * m._12 + v.y * m._22 + v.z * m._32,
		v.x * m._13 + v.y * m._23 + v.z * m._33);
}

template <class T>
Vector3<T> operator * (Vector3<T> v, Matrix3x3<T> m)
{
	return mul(v, m);
}

template <class T>
Matrix3x3<T> transpose(Matrix3x3<T> m)
{
	return Matrix3x3<T>(
		m._11, m._21, m._31,
		m._12, m._22, m._32,
		m._13, m._23, m._33);
}

// if performance is too slow, we can replace with DirectXMath version
template <class T>
Matrix3x3<T> mul( Matrix3x3<T> m1, Matrix3x3<T> m2)
{
	Matrix3x3<T> mOut;

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				mOut[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
	return mOut;
}

template <class T>
Matrix3x3<T> operator * (Matrix3x3<T> m1, Matrix3x3<T> m2)
{
	return mul(m1, m2);
}

template <class T>
Matrix3x3<T> operator *= (Matrix3x3<T> m1, Matrix3x3<T> m2)		// this is broken for some reason!
{
	return mul(m1, m2);
}

// Cramer's rule is fast enough for 3x3
template <class T>
Matrix3x3<T> inv(Matrix3x3<T> m)
{
	double det =  (double) m._11 * ((double)m._22 * (double)m._33 - (double)m._23 * (double)m._32)
				- (double) m._12 * ((double)m._21 * (double)m._33 - (double)m._23 * (double)m._31)
				+ (double) m._13 * ((double)m._21 * (double)m._32 - (double)m._22 * (double)m._31);
	if (abs(det) < 1.0e-9)
	{
		cerr << "Error: Matrix Not Invertible" << endl;
		abort();
	}
	Matrix3x3<T> ret;
	ret._11 = (float)((m._22 * m._33 - m._23 * m._32) / det);
	ret._12 = (float)((m._13 * m._32 - m._12 * m._33) / det);
	ret._13 = (float)((m._12 * m._23 - m._13 * m._22) / det);
	ret._21 = (float)((m._23 * m._31 - m._21 * m._33) / det);
	ret._22 = (float)((m._11 * m._33 - m._13 * m._31) / det);
	ret._23 = (float)((m._13 * m._21 - m._11 * m._23) / det);
	ret._31 = (float)((m._21 * m._32 - m._22 * m._31) / det);
	ret._32 = (float)((m._12 * m._31 - m._11 * m._32) / det);
	ret._33 = (float)((m._11 * m._22 - m._12 * m._21) / det);
	return ret;
}


// 4x4
template <class T>
Vector4<T> mul( Matrix4x4<T> m, Vector4<T> v )
 {
  return Vector4<T>
	   (v.x * m._11 + v.y * m._12 + v.z * m._13 + v.w * m._14,
        v.x * m._21 + v.y * m._22 + v.z * m._23 + v.w * m._24,
        v.x * m._31 + v.y * m._32 + v.z * m._33 + v.w * m._34,
        v.x * m._41 + v.y * m._42 + v.z * m._43 + v.w * m._44);
 }

template <class T>
Vector4<T> operator * ( Matrix4x4<T> m, Vector4<T> v )
{
    return mul( m, v );
}


template <class T>
Vector3<T> mul(  Vector3<T> v, Matrix4x4<T> m )
 {
  return Vector3<T>
	   (v.x * m._11 + v.y * m._21 + v.z * m._31 + m._41,
        v.x * m._12 + v.y * m._22 + v.z * m._32 + m._42,
        v.x * m._13 + v.y * m._23 + v.z * m._33 + m._43 );
 }

template <class T>
Vector3<T> mulNorm(  Vector3<T> v, Matrix4x4<T> m )
 {
  return Vector3<T>
	   (v.x * m._11 + v.y * m._21 + v.z * m._31,
        v.x * m._12 + v.y * m._22 + v.z * m._32,
        v.x * m._13 + v.y * m._23 + v.z * m._33 );
 }

template <class T>
Vector3<T> operator * ( Vector3<T> v, Matrix4x4<T> m )
{
    return mul( v, m );
}

template <class T>
Vector3<T> mul( Matrix4x4<T> m, Vector3<T> v )
 {
  return Vector3<T>
	   (v.x * m._11 + v.y * m._12 + v.z * m._13 + m._14,
        v.x * m._21 + v.y * m._22 + v.z * m._23 + m._24,
        v.x * m._31 + v.y * m._32 + v.z * m._33 + m._34 );
 }

template <class T>
Vector3<T> operator * ( Matrix4x4<T> m, Vector3<T> v )
{
    return mul( m, v );
}


template <class T>
Matrix4x4<T> transpose(Matrix4x4<T> m)
{
	return Matrix4x4<T>(
		m._11, m._21, m._31, m._41,
		m._12, m._22, m._32, m._42,
		m._13, m._23, m._33, m._43,
		m._14, m._24, m._34, m._44 );
}

// if performance is too slow, we can replace with DirectXMath version
template <class T>
Matrix4x4<T> mul(Matrix4x4<T> m1, Matrix4x4<T> m2)
{
	Matrix4x4<T> mOut;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				mOut[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}

	return mOut;
}

template <class T>
Matrix4x4<T> operator * ( Matrix4x4<T> m1, Matrix4x4<T> m2 )
{
    return mul( m1, m2 );
}
/*
template <class T>
Matrix4x4<T> operator *= ( Matrix4x4<T> m1, Matrix4x4<T> m2 )		// this is broken for some reason!
{
    return mul( m1, m2 );
}
*/
// clamp all components to 0..1 range
template <class T>
Vector4<T> saturate(Vector4<T> v)
{
	v.x = (v.x > 1.0f ? 1.0f : v.x);
	v.x = (v.x < 0.0f ? 0.0f : v.x);
	v.y = (v.y > 1.0f ? 1.0f : v.y);
	v.y = (v.y < 0.0f ? 0.0f : v.y);
	v.z = (v.z > 1.0f ? 1.0f : v.z);
	v.z = (v.z < 0.0f ? 0.0f : v.z);
	v.w = (v.w > 1.0f ? 1.0f : v.w);
	v.w = (v.w < 0.0f ? 0.0f : v.w);

	return Vector4<T>(v.x, v.y, v.z, v.w);
}

// works with scale factor as long as it is same in all 3 axes
template <class T>
Matrix4x4<T> fastMatrixInverse( Matrix4x4<T> matIn )
{
	Matrix4x4<T> mat;
	double sx;

    // Calculate the square of the isotropic scale factor
    sx = matIn._11*matIn._11 + matIn._12*matIn._12 + matIn._13*matIn._13;
	sx = 1.0/sx;

	// invert rotation component (upper left 3x3) by transposing
	mat._11 = (float) sx*matIn._11;
	mat._12 = (float) sx*matIn._21;
	mat._13 = (float) sx*matIn._31;
	mat._21 = (float) sx*matIn._12;
	mat._22 = (float) sx*matIn._22;
	mat._23 = (float) sx*matIn._32;
	mat._31 = (float) sx*matIn._13;
	mat._32 = (float) sx*matIn._23;
	mat._33 = (float) sx*matIn._33;

	// negate translations and xform them by rotation component (rotate them)
	float tx = -matIn._41;
	float ty = -matIn._42;
	float tz = -matIn._43;
	mat._41 = tx*mat._11 + ty*mat._21 + tz*mat._31;
	mat._42 = tx*mat._12 + ty*mat._22 + tz*mat._32;
	mat._43 = tx*mat._13 + ty*mat._23 + tz*mat._33;

	// do last column
	mat._14 = matIn._14;
	mat._24 = matIn._24;
	mat._34 = matIn._34;
	mat._44 = matIn._44;

	return mat;
}

// Common HLSL-compatible vector typedefs

typedef unsigned int uint;

typedef Vector2<float> float2;
typedef Vector3<float> float3;
typedef Vector4<float> float4;

typedef Matrix3x3<float> float3x3;
typedef Matrix4x4<float> float4x4;
typedef Matrix4x4<float> matrix;

typedef Vector2<float> half2;
typedef Vector3<float> half3;
typedef Vector4<float> half4;

// TODO: add constructor for D2D_POINT_2F etc.

// Standard Matrix Intializers


inline float4x4 identity()
{
	float4x4 mOut;

	mOut._11 = 1.0f; mOut._12 = 0.0f; mOut._13 = 0.0f; mOut._14 = 0.0f;
	mOut._21 = 0.0f; mOut._22 = 1.0f; mOut._23 = 0.0f; mOut._24 = 0.0f;
	mOut._31 = 0.0f; mOut._32 = 0.0f; mOut._33 = 1.0f; mOut._34 = 0.0f;
	mOut._41 = 0.0f; mOut._42 = 0.0f; mOut._43 = 0.0f; mOut._44 = 1.0f;

	return mOut;
}

inline float4x4 translation(float x, float y, float z)
{
	float4x4 mOut;

	mOut._11 = 1.0f; mOut._12 = 0.0f; mOut._13 = 0.0f; mOut._14 = 0.0f;
	mOut._21 = 0.0f; mOut._22 = 1.0f; mOut._23 = 0.0f; mOut._24 = 0.0f;
	mOut._31 = 0.0f; mOut._32 = 0.0f; mOut._33 = 1.0f; mOut._34 = 0.0f;
	mOut._41 =    x; mOut._42 =    y; mOut._43 =    z; mOut._44 = 1.0f;

	return mOut;
}

inline float4x4 translation( float3 t )
{
	return translation( t.x, t.y, t.z );
}

inline float4x4 scale(float x, float y, float z)
{
	float4x4 mOut;

	mOut._11 = x;	 mOut._12 = 0.0f; mOut._13 = 0.0f; mOut._14 = 0.0f;
	mOut._21 = 0.0f; mOut._22 = y;	  mOut._23 = 0.0f; mOut._24 = 0.0f;
	mOut._31 = 0.0f; mOut._32 = 0.0f; mOut._33 = z;    mOut._34 = 0.0f;
	mOut._41 = 0.0f; mOut._42 = 0.0f; mOut._43 = 0.0f; mOut._44 = 1.0f;

	return mOut;
}

inline float4x4 scale(float s)
{
	float4x4 mOut;

	mOut._11 = s;	 mOut._12 = 0.0f; mOut._13 = 0.0f; mOut._14 = 0.0f;
	mOut._21 = 0.0f; mOut._22 = s;	  mOut._23 = 0.0f; mOut._24 = 0.0f;
	mOut._31 = 0.0f; mOut._32 = 0.0f; mOut._33 = s;    mOut._34 = 0.0f;
	mOut._41 = 0.0f; mOut._42 = 0.0f; mOut._43 = 0.0f; mOut._44 = 1.0f;

	return mOut;
}

inline float4x4 rotationX(float a)
{
	float sinAngle = sinf(a);
	float cosAngle = cosf(a);

	float4x4 mOut;

	mOut._11 = 1.0f; mOut._12 = 0.0f;	  mOut._13 = 0.0f;	    mOut._14 = 0.0f;
	mOut._21 = 0.0f; mOut._22 = cosAngle; mOut._23 = -sinAngle; mOut._24 = 0.0f;
	mOut._31 = 0.0f; mOut._32 = sinAngle; mOut._33 = cosAngle;  mOut._34 = 0.0f;
	mOut._41 = 0.0f; mOut._42 = 0.0f;	  mOut._43 = 0.0f;	    mOut._44 = 1.0f;

	return mOut;
}

inline float4x4 rotationY(float a)
{
	float sinAngle = sinf(a);
	float cosAngle = cosf(a);

	float4x4 mOut;

	mOut._11 = cosAngle;  mOut._12 = 0.0f; mOut._13 = sinAngle; mOut._14 = 0.0f;
	mOut._21 = 0.0f;	  mOut._22 = 1.0f; mOut._23 = 0.0f;	 mOut._24 = 0.0f;
	mOut._31 = -sinAngle; mOut._32 = 0.0f; mOut._33 = cosAngle; mOut._34 = 0.0f;
	mOut._41 = 0.0f;	  mOut._42 = 0.0f; mOut._43 = 0.0f;	 mOut._44 = 1.0f;

	return mOut;
}

inline float4x4 rotationZ(float a)
{
	float sinAngle = sinf(a);
	float cosAngle = cosf(a);

	float4x4 mOut;

	mOut._11 = cosAngle; mOut._12 = -sinAngle; mOut._13 = 0.0f; mOut._14 = 0.0f;
	mOut._21 = sinAngle; mOut._22 = cosAngle;  mOut._23 = 0.0f; mOut._24 = 0.0f;
	mOut._31 = 0.0f;	 mOut._32 = 0.0f;	   mOut._33 = 1.0f; mOut._34 = 0.0f;
	mOut._41 = 0.0f;	 mOut._42 = 0.0f;	   mOut._43 = 0.0f; mOut._44 = 1.0f;

	return mOut;
}
/*
// 3D Rotation matrix for an arbitrary axis specified by x, y and z
inline float4x4 rotationArbitrary(float3 axis, float a)
{
	axis = normalize(axis);

	float sinAngle = sinf(a);
	float cosAngle = cosf(a);
	float oneMinusCosAngle = 1 - cosAngle;

	float4x4 mOut;

	mOut._11 = 1.0f + oneMinusCosAngle * (axis.x * axis.x - 1.0f);
	mOut._12 = axis.z * sinAngle + oneMinusCosAngle * axis.x * axis.y;
	mOut._13 = -axis.y * sinAngle + oneMinusCosAngle * axis.x * axis.z;
	mOut._41 = 0.0f;

	mOut._21 = -axis.z * sinAngle + oneMinusCosAngle * axis.y * axis.x;
	mOut._22 = 1.0f + oneMinusCosAngle * (axis.y * axis.y - 1.0f);
	mOut._23 = axis.x * sinAngle + oneMinusCosAngle * axis.y * axis.z;
	mOut._24 = 0.0f;

	mOut._31 = axis.y * sinAngle + oneMinusCosAngle * axis.z * axis.x;
	mOut._32 = -axis.x * sinAngle + oneMinusCosAngle * axis.z * axis.y;
	mOut._33 = 1.0f + oneMinusCosAngle * (axis.z * axis.z - 1.0f);
	mOut._34 = 0.0f;

	mOut._41 = 0.0f;
	mOut._42 = 0.0f;
	mOut._43 = 0.0f;
	mOut._44 = 1.0f;

	return mOut;
}

//==================================================================================================
// vLookAt	-stuffs vectors into a matrix to orient object from vBase towards vTarget in plane of Normal
//==================================================================================================
inline float4x4 lookAt( const float3 vBase, const float3 vTarget, const float3 vPlaneNormal )
{
	float4x4 mOut;
	float3 vForward, vLeft, vVert;

	vForward = vTarget - vBase;
	vForward = normalize( vForward );
	vVert = cross( vForward, vPlaneNormal );
	vVert = normalize( vVert );

	mOut._11 = vPlaneNormal.x;
	mOut._12 = vPlaneNormal.y;
	mOut._13 = vPlaneNormal.z;
	mOut._14 = 0;

	mOut._21 = vVert.x;
	mOut._22 = vVert.y;
	mOut._23 = vVert.z;
	mOut._24 = 0;

	mOut._31 = vForward.x;
	mOut._32 = vForward.y;
	mOut._33 = vForward.z;
	mOut._34 = 0;

	mOut._41 = vBase.x;
	mOut._42 = vBase.y;
	mOut._43 = vBase.z;
	mOut._44 = 1;

	return mOut;
}

//==================================================================================================
// vLookAtYZ	point Y-axis along vUp, and up-orientation () from fwd vector.
//==================================================================================================
inline float4x4 lookAtYZ( const float3 vPos, const float3 vTo, const float3 vUp )
{
	float4x4 mOut;

	float3 vLeft = normalize( cross( vTo, vUp ) );		// or could extract from top row of bugMat

	mOut._11 = vLeft.x;
	mOut._12 = vLeft.y;
	mOut._13 = vLeft.z;
	mOut._14 = 0;

	mOut._21 = vTo.x;
	mOut._22 = vTo.y;
	mOut._23 = vTo.z;
	mOut._24 = 0;

	float3 vFwd = normalize( cross( vLeft, vTo ) );
	mOut._31 = vFwd.x;
	mOut._32 = vFwd.y;
	mOut._33 = vFwd.z;
	mOut._34 = 0;

	mOut._41 = vPos.x;
	mOut._42 = vPos.y;
	mOut._43 = vPos.z;
	mOut._44 = 1;


	return mOut;
}

inline float stepFunc( float t )
{
	if ( t <= 0 )
		return 0;
	else if ( t>0 )
		return 1;
}

inline float smoothStep( float t )
{
	if ( t <= 0 )
		return 0;
	else if ( t>=1 )
		return 1;
	else
		return t*t*(3 - 2*t);
}

float periodicSmoothStep( float x )
{
	float base = floorf( x );
	float frac = x - base;
	return base + smoothStep( frac );
}
*/