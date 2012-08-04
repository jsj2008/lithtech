//////////////////////////////////////////////////////////////////////////////
//
// UTILS.H
//
//  Version 1.0
//  Written by Zvi Devir, Intel MSL
//
//////////////////////////////////////////////////////////////////////////////

//
//   Copyright (c) 1999 Intel Corporation.
//
// Permition is granted to use, copy, distribute and prepare derivative works 
// of this library for any purpose and without fee, provided, that the above 
// copyright notice and this statement appear in all copies.  
// Intel makes no representations about the suitability of this library for 
// any purpose, and specifically disclaims all warranties. 
// See LEGAL.TXT for all the legal information.
//

#ifndef __SIMD_UTILITIES_H__
#define __SIMD_UTILITIES_H__

// General switches:
//#define USE_AMLIB		// Enable the use of AMath Library.
//#define ZERO_SINGULAR	// Defines if Inverse() of singular matrix zeros the matrix.
//#define ZERO_VECTOR 	// Defines if normalization of zero vector zeros the vector.

/*
#ifndef __ICL
	#error "This file must be compiled with Intel C/C++ Compiler"
#endif

#include <fvec.h>

#pragma pack(push,16) 

class GPMatrix;
class GPVector;
class GPVector3;


class GPMatrix {
public:
	union {
		struct {
			__m128 _L1, _L2, _L3, _L4;
		};
		struct {
			float	_11, _12, _13, _14;
			float	_21, _22, _23, _24;
			float	_31, _32, _33, _34;
			float	_41, _42, _43, _44;
		};
	};

// Constructors and convertions...
	GPMatrix() {}
	GPMatrix(const GPMatrix &m) : _L1(m._L1), _L2(m._L2), _L3(m._L3), _L4(m._L4) {}
	GPMatrix(float _11, float _12, float _13, float _14,
			 float _21, float _22, float _23, float _24,
			 float _31, float _32, float _33, float _34,
			 float _41, float _42, float _43, float _44);

	float& operator() (int i, int j) {
		assert((0<=i) && (i<=3) && (0<=j) && (j<=3));
		return *(((float *)&_11) + (i<<2)+j);
	}
	F32vec4& operator() (int i) {
		assert((0<=i) && (i<=3));
		return *(((F32vec4 *)&_11) + i);
	}
	F32vec4& operator[] (int i) {
		assert((0<=i) && (i<=3));
		return *(((F32vec4 *)&_11) + i);
	}
	F32vec4& operator[] (int i) const {
		assert((0<=i) && (i<=3));
		return *(((F32vec4 *)&_11) + i);
	}

	GPMatrix& operator= (const GPMatrix &a) {
		_L1 = a._L1; _L2 = a._L2; _L3 = a._L3; _L4 = a._L4;
		return *this;
	}

	friend GPMatrix operator * (const GPMatrix&, const GPMatrix&);
	friend GPMatrix operator + (const GPMatrix&, const GPMatrix&);
	friend GPMatrix operator - (const GPMatrix&, const GPMatrix&);
	friend GPMatrix operator + (const GPMatrix&);
	friend GPMatrix operator - (const GPMatrix&);
	friend GPMatrix operator * (const GPMatrix&, const float);
	friend GPMatrix operator * (const float, const GPMatrix&);

	GPMatrix & operator *= (const GPMatrix &);
	GPMatrix & operator *= (const float);
	GPMatrix & operator += (const GPMatrix &);
	GPMatrix & operator -= (const GPMatrix &);

	void	Transpose();	// Transposes the matrix
	float	Inverse();		// Inverses the matrix and returns the determinant

	float	Determinant();	// Returns the determinant
	float	MinValue(); 	// Returns the minimum absolute value of the matrix
	float	MaxValue();		// Returns the maximum absolute value of the matrix

	// Other Constructors:
	void ZeroMatrix();
	void IdentityMatrix();
	void TranslateMatrix(const float dx, const float dy, const float dz);
	void ScaleMatrix(const float a, const float b, const float c);
	void ScaleMatrix(const float a);
//	void RotateXMatrix(const float rads);
//	void RotateYMatrix(const float rads);
//	void RotateZMatrix(const float rads);
#ifdef USE_AMLIB	// Aproximations using AMath 
	void RotateXMatrixA(const float rads);
	void RotateYMatrixA(const float rads);
	void RotateZMatrixA(const float rads);
#endif

};

class GPVector {
public:
	union {
		__m128 vec;
		struct { 
			float	x,y,z,w;
		};
	};

// Constructors and convertions...
	GPVector() {}
	GPVector(const GPVector &v) : vec(v.vec) {}
	GPVector(const __m128 &m) : vec(m) {}
	GPVector(const F32vec4 &m) : vec(m) {}
	GPVector(const float x, const float y, const float z, const float w=0.0f) : vec(F32vec4(w,z,y,x)) {}
	operator __m128() const { return vec; }
	operator F32vec4() const { return vec; }

	GPVector& operator = (const GPVector &a) { vec = a.vec; return *this; }
	GPVector& operator = (const F32vec4 &a) { vec = a; return *this; }
	GPVector& operator = (const __m128 &a) { vec = a; return *this; }
	GPVector& operator = (const GPVector3 &);

	float& operator() (int i) {
		assert((0<=i) && (i<=3));
		return *(((float *)&vec) + i);
	}
	float& operator[] (int i) {
		assert((0<=i) && (i<=3));
		return *(((float *)&vec) + i);
	}
	float& operator[] (int i) const {
		assert((0<=i) && (i<=3));
		return *(((float *)&vec) + i);
	}

	friend GPVector operator * (const GPVector&, const GPMatrix&);
	friend float operator * (const GPVector &, const GPVector &);			// Dot Product
	friend GPVector operator % (const GPVector &, const GPVector &);	 	// Cross Product
	friend GPVector operator * (const GPVector &, const float);
	friend GPVector operator * (const float, const GPVector &);
	friend GPVector operator + (const GPVector&);
	friend GPVector operator + (const GPVector&, const GPVector&);
	friend GPVector operator - (const GPVector&);
	friend GPVector operator - (const GPVector&, const GPVector&);
	friend GPVector operator ~ (const GPVector&);							// Normalize

	GPVector & operator *= (const GPMatrix &);
	GPVector & operator *= (const float);
	GPVector & operator += (const GPVector &);
	GPVector & operator -= (const GPVector &);

	float Length();
	GPVector& Normalize();
};
#define GPvector4 GPVector

class GPVector3 {
public:
	union {
		__m128 vec;
		struct { 
			float	x,y,z;
		//	float	_spacer_;
		};
	};

// Constructors and convertions...
	GPVector3() {}
	GPVector3(const GPVector3 &v) : vec(v.vec) {}
	GPVector3(const __m128 &m) : vec(m) {}
	GPVector3(const F32vec4 &m) : vec(m) {}
	GPVector3(const float x, const float y, const float z) : vec(F32vec4(0.0f,z,y,x)) {}
	operator __m128() const { return vec; }
	operator F32vec4() const { return vec; }

	GPVector3& operator = (const GPVector3 &a) { vec = a.vec; return *this; }

	friend GPVector3 operator * (const GPVector3&, const GPMatrix&);
	friend float operator * (const GPVector3 &, const GPVector3 &);			// Dot Product
	friend GPVector3 operator % (const GPVector3 &, const GPVector3 &);		// Cross Product
	friend GPVector3 operator * (const GPVector3 &, const float);
	friend GPVector3 operator * (const float, const GPVector3 &);
	friend GPVector3 operator + (const GPVector3&);
	friend GPVector3 operator + (const GPVector3&, const GPVector3&);
	friend GPVector3 operator - (const GPVector3&);
	friend GPVector3 operator - (const GPVector3&, const GPVector3&);
	friend GPVector3 operator ~ (const GPVector3&);							// Normalize

	GPVector3 & operator *= (const GPMatrix &);
	GPVector3 & operator *= (const float);
	GPVector3 & operator += (const GPVector3 &);
	GPVector3 & operator -= (const GPVector3 &);

	float Length();
	GPVector3& Normalize();
};

#pragma pack(pop) 


GPMatrix ZeroMatrix();
GPMatrix IdentityMatrix();
GPMatrix TranslateMatrix(const float dx, const float dy, const float dz);
GPMatrix ScaleMatrix(const float a, const float b, const float c);
GPMatrix ScaleMatrix(const float a);
GPMatrix RotateXMatrix(const float rads);
GPMatrix RotateYMatrix(const float rads);
GPMatrix RotateZMatrix(const float rads);

#ifdef USE_AMLIB	// Aproximations using AMaths //
GPMatrix RotateXMatrixA(const float rads);
GPMatrix RotateYMatrixA(const float rads);
GPMatrix RotateZMatrixA(const float rads);
#endif


void MatrixMult(const GPMatrix &A, const GPMatrix &B, GPMatrix &Res);
void VectorMult(const GPVector &Vec, const GPMatrix &Mat, GPVector &Res);
GPMatrix MatrixMult(const GPMatrix &A, const GPMatrix &B);

inline GPMatrix MatrixInverse(GPMatrix &m) 		{ GPMatrix l=m; l.Inverse(); return l; }
inline float Determinant(GPMatrix &m)			{ return m.Determinant(); }
inline GPMatrix MatrixTranspose(GPMatrix &m)	{ GPMatrix l=m; l.Transpose(); return l; }


extern const F32vec4 _MASKSIGN_;

// Some useful macros:

#define _mm_ror_ps(vec,i)	\
	(((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((unsigned char)(i+3)%4,(unsigned char)(i+2)%4,(unsigned char)(i+1)%4,(unsigned char)(i+0)%4))) : (vec))
#define _mm_rol_ps(vec,i)	\
	(((i)%4) ? (_mm_shuffle_ps(vec,vec, _MM_SHUFFLE((unsigned char)(7-i)%4,(unsigned char)(6-i)%4,(unsigned char)(5-i)%4,(unsigned char)(4-i)%4))) : (vec))

#define _mm_abs_ps(vec)		_mm_andnot_ps(_MASKSIGN_,vec)
#define _mm_neg_ps(vec)		_mm_xor_ps(_MASKSIGN_,vec)


// Include all the inlined functions
#include "SimD_Utilities.inl"

*/
#endif // __SIMD_UTILITIES_H__
