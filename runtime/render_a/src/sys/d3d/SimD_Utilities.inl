//////////////////////////////////////////////////////////////////////////////
//
// UTILS.INL
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


extern const F32vec4 _ZERONE_;
extern const F32vec4 _0FFF_;


			/***********************************************************/
			/*                Matrix Constructors                      */
			/***********************************************************/


inline GPMatrix::GPMatrix(	float f11, float f12, float f13, float f14,
							float f21, float f22, float f23, float f24,
							float f31, float f32, float f33, float f34,
							float f41, float f42, float f43, float f44	)
{
	_L1 = F32vec4(f14, f13, f12, f11);
	_L2 = F32vec4(f24, f23, f22, f21);
	_L3 = F32vec4(f34, f33, f32, f31);
	_L4 = F32vec4(f44, f43, f42, f41);
}

// ----------------------------------------------------------------------------
//  Name:	ZeroMatrix
//  Desc:	Sets all elements in [Res] to zero.
// ----------------------------------------------------------------------------
inline GPMatrix ZeroMatrix() {
	GPMatrix Res;
	Res._L1 = Res._L2 = Res._L3 = Res._L4 = _mm_setzero_ps();
	return Res;
}
inline void GPMatrix::ZeroMatrix() {
	_L1 = _L2 = _L3 = _L4 = _mm_setzero_ps();
}


// ----------------------------------------------------------------------------
//  Name:	IdentityMatrix
//  Desc:	Returns the Identity Matrix.
// ----------------------------------------------------------------------------
inline GPMatrix IdentityMatrix() {
	GPMatrix Res;
	__m128 zerone = _ZERONE_;
	_mm_storel_pi((__m64 *)&Res._11, zerone);
	_mm_storel_pi((__m64 *)&Res._13, _mm_setzero_ps());
	_mm_storeh_pi((__m64 *)&Res._21, zerone);
	_mm_storel_pi((__m64 *)&Res._23, _mm_setzero_ps());
	_mm_storel_pi((__m64 *)&Res._31, _mm_setzero_ps());
	_mm_storel_pi((__m64 *)&Res._33, zerone);
	_mm_storel_pi((__m64 *)&Res._41, _mm_setzero_ps());
	_mm_storeh_pi((__m64 *)&Res._43, zerone);
	return Res;
}
inline void GPMatrix::IdentityMatrix() {
	__m128 zerone = _ZERONE_;
	_mm_storel_pi((__m64 *)&_11, zerone);
	_mm_storel_pi((__m64 *)&_13, _mm_setzero_ps());
	_mm_storeh_pi((__m64 *)&_21, zerone);
	_mm_storel_pi((__m64 *)&_23, _mm_setzero_ps());
	_mm_storel_pi((__m64 *)&_31, _mm_setzero_ps());
	_mm_storel_pi((__m64 *)&_33, zerone);
	_mm_storel_pi((__m64 *)&_41, _mm_setzero_ps());
	_mm_storeh_pi((__m64 *)&_43, zerone);
}

			/***********************************************************/
			/*                     Matrix Operators                    */
			/***********************************************************/

// ----------------------------------------------------------------------------
//  Name:	MatrixMult
//  Desc:	Matrix multiplication of A and B. [Res] = [A]*[B].
// ----------------------------------------------------------------------------
inline void MatrixMult(const GPMatrix &A, const GPMatrix &B, GPMatrix &Res) {
	F32vec4 Result;
	F32vec4 B1 = B._L1, B2 = B._L2, B3 = B._L3, B4 = B._L4;

	Result = F32vec4(A._11) * B1;
	Result += F32vec4(A._12) * B2;
	Result += F32vec4(A._13) * B3;
	Result += F32vec4(A._14) * B4;
	Res._L1 = Result;

	Result = F32vec4(A._21) * B1;
	Result += F32vec4(A._22) * B2;
	Result += F32vec4(A._23) * B3;
	Result += F32vec4(A._24) * B4;
	Res._L2 = Result;

	Result = F32vec4(A._31) * B1;
	Result += F32vec4(A._32) * B2;
	Result += F32vec4(A._33) * B3;
	Result += F32vec4(A._34) * B4;
	Res._L3 = Result;

	Result = F32vec4(A._41) * B1;
	Result += F32vec4(A._42) * B2;
	Result += F32vec4(A._43) * B3;
	Result += F32vec4(A._44) * B4;
	Res._L4 = Result;
}

// ----------------------------------------------------------------------------
//  Name:	MatrixMult
//  Desc:	Matrix multiplication of A and B. Returns [A]*[B].
// ----------------------------------------------------------------------------
inline GPMatrix MatrixMult(const GPMatrix &A, const GPMatrix &B) {
	GPMatrix Res;
	F32vec4 Result;
	F32vec4 B1 = B._L1, B2 = B._L2, B3 = B._L3, B4 = B._L4;

	Result = F32vec4(A._11) * B1;
	Result += F32vec4(A._12) * B2;
	Result += F32vec4(A._13) * B3;
	Result += F32vec4(A._14) * B4;
	Res._L1 = Result;

	Result = F32vec4(A._21) * B1;
	Result += F32vec4(A._22) * B2;
	Result += F32vec4(A._23) * B3;
	Result += F32vec4(A._24) * B4;
	Res._L2 = Result;

	Result = F32vec4(A._31) * B1;
	Result += F32vec4(A._32) * B2;
	Result += F32vec4(A._33) * B3;
	Result += F32vec4(A._34) * B4;
	Res._L3 = Result;

	Result = F32vec4(A._41) * B1;
	Result += F32vec4(A._42) * B2;
	Result += F32vec4(A._43) * B3;
	Result += F32vec4(A._44) * B4;
	Res._L4 = Result;

	return Res;
}

// ----------------------------------------------------------------------------
//  Name:	GPMatrix * GPMatrix
//  Desc:	Matrix multiplication of A and B. Returns [A]*[B].
// ----------------------------------------------------------------------------
inline GPMatrix operator * (const GPMatrix &A, const GPMatrix &B) {
	return MatrixMult(A, B);
}

// ----------------------------------------------------------------------------
//  Name:	GPMatrix *= GPMatrix
//  Desc:	Matrix multiplication of A by B. [A] = [A]*[B].
// ----------------------------------------------------------------------------
inline GPMatrix & GPMatrix::operator *= (const GPMatrix &B) {
	MatrixMult(*this, B, *this);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	GPMatrix * float
//  Desc:	Matrix elements multiplication with scalar. Returns [A]*s.
// ----------------------------------------------------------------------------
inline GPMatrix operator * (const GPMatrix &A, const float s) {
	GPMatrix Res;
	F32vec4 S = F32vec4(s);
	for (int i=0; i<4; i++) Res[i] = A[i] * S;
	return Res;
}
inline GPMatrix operator * (const float s, const GPMatrix &A) {
	GPMatrix Res;
	F32vec4 S = F32vec4(s);
	for (int i=0; i<4; i++) Res[i] = A[i] * S;
	return Res;
}

// ----------------------------------------------------------------------------
//  Name:	GPMatrix *= float
//  Desc:	Matrix elements multiplication by scalar. [A] = [A]*s.
// ----------------------------------------------------------------------------
inline GPMatrix & GPMatrix::operator *= (const float s) {
	F32vec4 S = F32vec4(s);
	_L1 = _mm_mul_ps(_L1,S);
	_L2 = _mm_mul_ps(_L2,S);
	_L3 = _mm_mul_ps(_L3,S);
	_L4 = _mm_mul_ps(_L4,S);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	GPMatrix + GPMatrix
//  Desc:	Matrix elements addition between A and B. Returns [A]+[B].
// ----------------------------------------------------------------------------
inline GPMatrix operator + (const GPMatrix &A, const GPMatrix &B) {
	GPMatrix Res;
	for (int i=0; i<4; i++) Res[i] = A[i] + B[i];
	return Res;
}

// ----------------------------------------------------------------------------
//  Name:	GPMatrix += GPMatrix
//  Desc:	Matrix addition of A by B. [A] = [A]+[B].
// ----------------------------------------------------------------------------
inline GPMatrix & GPMatrix::operator += (const GPMatrix &B) {
	_L1 = _mm_add_ps(_L1,B._L1);
	_L2 = _mm_add_ps(_L2,B._L2);
	_L3 = _mm_add_ps(_L3,B._L3);
	_L4 = _mm_add_ps(_L4,B._L4);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	GPMatrix - GPMatrix
//  Desc:	Matrix elements substraction between A and B. Returns [A]-[B].
// ----------------------------------------------------------------------------
inline GPMatrix operator - (const GPMatrix &A, const GPMatrix &B) {
	GPMatrix Res;
	for (int i=0; i<4; i++) Res[i] = A[i] - B[i];
	return Res;
}

// ----------------------------------------------------------------------------
//  Name:	GPMatrix -= GPMatrix
//  Desc:	Matrix substraction of A by B. [A] = [A]-[B].
// ----------------------------------------------------------------------------
inline GPMatrix & GPMatrix::operator -= (const GPMatrix &B) {
	_L1 = _mm_sub_ps(_L1,B._L1);
	_L2 = _mm_sub_ps(_L2,B._L2);
	_L3 = _mm_sub_ps(_L3,B._L3);
	_L4 = _mm_sub_ps(_L4,B._L4);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	-GPMatrix (Unary minus)
//  Desc:	Matrix elements are negate. Returns -[A].
// ----------------------------------------------------------------------------
inline GPMatrix operator - (const GPMatrix &A) {
	GPMatrix Res;
	__m128 masksign = _MASKSIGN_;
	for (int i=0; i<4; i++) Res[i] = _mm_xor_ps(masksign,A[i]);
	return Res;
}

// ----------------------------------------------------------------------------
//  Name:	+GPMatrix (Unary plus)
//  Desc:	Returns the same matrix [A].
// ----------------------------------------------------------------------------
inline GPMatrix operator + (const GPMatrix &A) {
	return A;
}

// ----------------------------------------------------------------------------
//  Name:	GPMatrix::Transpose                           t
//  Desc:	Transpose the 4x4 matrix. Matrix is set to [M] .
// ----------------------------------------------------------------------------
inline void GPMatrix::Transpose() {
	__m128	xmm0 = _mm_unpacklo_ps(_L1,_L2),
			xmm1 = _mm_unpacklo_ps(_L3,_L4),
			xmm2 = _mm_unpackhi_ps(_L1,_L2),
			xmm3 = _mm_unpackhi_ps(_L3,_L4);

	_L1 = _mm_movelh_ps(xmm0,xmm1);
	_L2 = _mm_movehl_ps(xmm1,xmm0);
	_L3 = _mm_movelh_ps(xmm2,xmm3);
	_L4 = _mm_movehl_ps(xmm3,xmm2);
}


			/***********************************************************/
			/*                    Vector[4] Operators                  */
			/***********************************************************/

// ----------------------------------------------------------------------------
//  Name:	VectorMult                          ___     ___
//  Desc:	Vector multiplication with matrix. [Res] = [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline void VectorMult(const GPVector &Vec, const GPMatrix &Mat, GPVector &Res) {
	F32vec4 Result;

	Result = F32vec4(Vec.x) * Mat._L1;
	Result += F32vec4(Vec.y) * Mat._L2;
	Result += F32vec4(Vec.z) * Mat._L3;
	Result += F32vec4(Vec.w) * Mat._L4;

	Res = Result;
}

// ----------------------------------------------------------------------------
//  Name:	VectorMult                                  ___
//  Desc:	Vector multiplication with matrix. Returns [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline GPVector VectorMult(const GPVector &Vec, const GPMatrix &Mat) {
	F32vec4 Result;

	Result = F32vec4(Vec.x) * Mat._L1;
	Result += F32vec4(Vec.y) * Mat._L2;
	Result += F32vec4(Vec.z) * Mat._L3;
	Result += F32vec4(Vec.w) * Mat._L4;
	return Result;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector * GPMatrix                         ___
//  Desc:	Vector multiplication with matrix. Returns [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline GPVector operator * (const GPVector& Vec, const GPMatrix& Mat) {
	return VectorMult(Vec, Mat);
}

// ----------------------------------------------------------------------------
//  Name:	GPVector *= GPMatrix                ___     ___
//  Desc:	Vector multiplication with matrix. [Vec] = [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline GPVector& GPVector::operator *= (const GPMatrix& Mat) {
	VectorMult(*this, Mat, *this);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector * GPVector	                     _   _
//  Desc:	Dot product of the two vectors. Returns [A]·[B].
// ----------------------------------------------------------------------------
inline float operator * (const GPVector& A, const GPVector& B) {
	F32vec4 r = _mm_mul_ps(A,B);
	r = _mm_add_ps(_mm_movehl_ps(r,r),r);
	F32vec1 t = _mm_add_ss(_mm_shuffle_ps(r,r,1), r);
	return *(float *)&t;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector % GPVector                           _   _
//  Desc:	Cross product of the two 3D vectors. Returns [A]x[B].
// ----------------------------------------------------------------------------
inline GPVector operator % (const GPVector& A, const GPVector& B) {
	F32vec4 l1, l2, m1, m2;
	l1 = _mm_shuffle_ps(A,A, _MM_SHUFFLE(3,1,0,2));
	l2 = _mm_shuffle_ps(B,B, _MM_SHUFFLE(3,0,2,1));
	m2 = l1*l2;
	m2[3] = 0.0f;
	l1 = _mm_shuffle_ps(A,A, _MM_SHUFFLE(3,0,2,1));
	l2 = _mm_shuffle_ps(B,B, _MM_SHUFFLE(3,1,0,2));
	m1 = l1*l2;
	return m1-m2;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector * float                                 _
//  Desc:	Multiplies the vector elements by scalar. Returns [V]*s.
// ----------------------------------------------------------------------------
inline GPVector operator * (const GPVector &V, const float s) {
	return V.vec * F32vec4(s);
}
inline GPVector operator * (const float s, const GPVector &V) {
	return V.vec * F32vec4(s);
}

// ----------------------------------------------------------------------------
//  Name:	GPVector *= float                ___     ___
//  Desc:	Vector multiplication by float. [Vec] = [Vec]*s.
// ----------------------------------------------------------------------------
inline GPVector& GPVector::operator *= (const float s) {
	vec = vec * F32vec4(s);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector + GPVector       _   _
//  Desc:	Vector addition. Returns [A]+[B].
// ----------------------------------------------------------------------------
inline GPVector operator + (const GPVector& A, const GPVector& B) {
	return _mm_add_ps(A.vec, B.vec);
}

// ----------------------------------------------------------------------------
//  Name:	GPVector - GPVector           _   _
//  Desc:	Vector substraction. Returns [A]-[B].
// ----------------------------------------------------------------------------
inline GPVector operator - (const GPVector& A, const GPVector& B) {
	return _mm_sub_ps(A.vec, B.vec);
}

// ----------------------------------------------------------------------------
//  Name:	GPVector += GPVector    _     _   _
//  Desc:	Vector addition.       [A] = [A]+[B].
// ----------------------------------------------------------------------------
inline GPVector & GPVector::operator += (const GPVector &B) {
	vec = _mm_add_ps(vec, B.vec);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector -= GPVector    _     _   _
//  Desc:	Vector substraction.   [A] = [A]-[B].
// ----------------------------------------------------------------------------
inline GPVector & GPVector::operator -= (const GPVector &B) {
	vec = _mm_sub_ps(vec, B.vec);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	-GPVector (Unary minus)                _
//  Desc:	Vectors elements are negate. Returns -[A].
// ----------------------------------------------------------------------------
inline GPVector operator - (const GPVector &A) {
	return _mm_xor_ps(_MASKSIGN_,A);
}

// ----------------------------------------------------------------------------
//  Name:	+GPVector (Unary plus)
//  Desc:	Returns the same vector.
// ----------------------------------------------------------------------------
inline GPVector operator + (const GPVector &A) {
	return A;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector::Length
//  Desc:	Returns the length of the vector.
// ----------------------------------------------------------------------------
inline float GPVector::Length() {
	F32vec4 r = _mm_mul_ps(vec,vec);
	r = _mm_add_ps(_mm_movehl_ps(r,r),r);
	F32vec1 t = _mm_add_ss(_mm_shuffle_ps(r,r,1), r);
	t = sqrt(t);
	return *(float *)&t;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector::Normalize
//  Desc:	Normalized the (X,Y,Z) elements of the vector.
// ----------------------------------------------------------------------------
inline GPVector& GPVector::Normalize() {
	F32vec4 r = _mm_mul_ps(vec,vec);
	r = _mm_add_ps(_mm_movehl_ps(r,r),r);
	F32vec1 t = _mm_add_ss(_mm_shuffle_ps(r,r,1), r);
	#ifdef ZERO_VECTOR
		t = _mm_cmpneq_ss(t, _mm_setzero_ps()) & rsqrt_nr(t);
	#else
		t = rsqrt_nr(t);
	#endif
	vec = _mm_mul_ps(vec, _mm_shuffle_ps(t,t,0x00));
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	~GPVector [Normalize]
//  Desc:	Normalized the source vector.
// ----------------------------------------------------------------------------
inline GPVector operator ~ (const GPVector &V) {
	F32vec4 r = _mm_mul_ps(V,V);
	r = _mm_add_ps(_mm_movehl_ps(r,r),r);
	F32vec1 t = _mm_add_ss(_mm_shuffle_ps(r,r,1), r);
	#ifdef ZERO_VECTOR
		t = _mm_cmpneq_ss(t, _mm_setzero_ps()) & rsqrt_nr(t);
	#else
		t = rsqrt_nr(t);
	#endif
	r = _mm_mul_ps(V, _mm_shuffle_ps(t,t,0x00));
	return r;
}


// ----------------------------------------------------------------------------
//  Name:	GPVector = GPVector3 (assignment)
//  Desc:	Convert a GPVector3 into GPVector.
// ----------------------------------------------------------------------------
inline GPVector& GPVector::operator = (const GPVector3 &a) {
	vec = _mm_and_ps(a.vec, _0FFF_);	// Set W to zero.
	return *this;
}



			/***********************************************************/
			/*                    Vector[3] Operators                  */
			/***********************************************************/

// ----------------------------------------------------------------------------
//  Name:	VectorMult                          ___     ___
//  Desc:	Vector multiplication with matrix. [Res] = [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline void VectorMult(const GPVector3 &Vec, const GPMatrix &Mat, GPVector3 &Res) {
	F32vec4 Result = Mat._L4;
	Result += F32vec4(Vec.x) * Mat._L1;
	Result += F32vec4(Vec.y) * Mat._L2;
	Result += F32vec4(Vec.z) * Mat._L3;

	F32vec1 W = _mm_shuffle_ps(Result, Result, 0xFF);
	W = rcp_nr(W);
	Res = Result * _mm_shuffle_ps(W, W, 0x00);
}

// ----------------------------------------------------------------------------
//  Name:	VectorMult                                  ___
//  Desc:	Vector multiplication with matrix. Returns [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline GPVector3 VectorMult(const GPVector3 &Vec, const GPMatrix &Mat) {
	F32vec4 Result = Mat._L4;
	Result += F32vec4(Vec.x) * Mat._L1;
	Result += F32vec4(Vec.y) * Mat._L2;
	Result += F32vec4(Vec.z) * Mat._L3;

	F32vec1 W = _mm_shuffle_ps(Result, Result, 0xFF);
	W = rcp_nr(W);
	return Result * _mm_shuffle_ps(W, W, 0x00);
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3 * GPMatrix                        ___
//  Desc:	Vector multiplication with matrix. Returns [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline GPVector3 operator * (const GPVector3& Vec, const GPMatrix& Mat) {
	return VectorMult(Vec, Mat);
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3 *= GPMatrix               ___     ___
//  Desc:	Vector multiplication with matrix. [Vec] = [Vec]*[Mat].
// ----------------------------------------------------------------------------
inline GPVector3& GPVector3::operator *= (const GPMatrix& Mat) {
	VectorMult(*this, Mat, *this);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3 * GPVector3	                 _   _
//  Desc:	Dot product of the two vectors. Returns [A]·[B].
// ----------------------------------------------------------------------------
inline float operator * (const GPVector3& A, const GPVector3& B) {
	F32vec4 r = _mm_mul_ps(A,B);
	F32vec1 t = _mm_add_ss(_mm_shuffle_ps(r,r,1), _mm_add_ps(_mm_movehl_ps(r,r),r));
	return *(float *)&t;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3 % GPVector3                      _   _
//  Desc:	Cross product of the two vectors. Returns [A]x[B].
// ----------------------------------------------------------------------------
inline GPVector3 operator % (const GPVector3& A, const GPVector3& B) {
	F32vec4 l1, l2, m1, m2;
	l1 = _mm_shuffle_ps(A,A, _MM_SHUFFLE(3,1,0,2));
	l2 = _mm_shuffle_ps(B,B, _MM_SHUFFLE(3,0,2,1));
	m2 = l1*l2;
	l1 = _mm_shuffle_ps(A,A, _MM_SHUFFLE(3,0,2,1));
	l2 = _mm_shuffle_ps(B,B, _MM_SHUFFLE(3,1,0,2));
	m1 = l1*l2;
	return m1-m2;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3 * float                                _
//  Desc:	Multiply the vector elements by scalar. Returns [V]*s.
// ----------------------------------------------------------------------------
inline GPVector3 operator * (const GPVector3 &V, const float s) {
	return V.vec * F32vec4(s);
}
inline GPVector3 operator * (const float s, const GPVector3 &V) {
	return V.vec * F32vec4(s);
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3 *= float               ___     ___
//  Desc:	Vector multiplication by float. [Vec] = [Vec]*s.
// ----------------------------------------------------------------------------
inline GPVector3& GPVector3::operator *= (const float s) {
	vec = vec * F32vec4(s);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3 + GPVector3     _   _
//  Desc:	Vector addition. Returns [A]+[B].
// ----------------------------------------------------------------------------
inline GPVector3 operator + (const GPVector3& A, const GPVector3& B) {
	return _mm_add_ps(A.vec, B.vec);
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3 - GPVector3         _   _
//  Desc:	Vector substraction. Returns [A]-[B].
// ----------------------------------------------------------------------------
inline GPVector3 operator - (const GPVector3& A, const GPVector3& B) {
	return _mm_sub_ps(A.vec, B.vec);
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3 += GPVector3  _     _   _
//  Desc:	Vector addition.       [A] = [A]+[B].
// ----------------------------------------------------------------------------
inline GPVector3 & GPVector3::operator += (const GPVector3 &B) {
	vec = _mm_add_ps(vec, B.vec);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3 -= GPVector3  _     _   _
//  Desc:	Vector substraction.   [A] = [A]-[B].
// ----------------------------------------------------------------------------
inline GPVector3 & GPVector3::operator -= (const GPVector3 &B) {
	vec = _mm_sub_ps(vec, B.vec);
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	-GPVector3 (Unary minus)                _
//  Desc:	Vectors elements are negate. Returns -[A].
// ----------------------------------------------------------------------------
inline GPVector3 operator - (const GPVector3 &A) {
	return _mm_xor_ps(_MASKSIGN_,A);
}

// ----------------------------------------------------------------------------
//  Name:	+GPVector3 (Unary plus)
//  Desc:	Returns the same vector.
// ----------------------------------------------------------------------------
inline GPVector3 operator + (const GPVector3 &A) {
	return A;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3::Length
//  Desc:	Returns the length of the vector.
// ----------------------------------------------------------------------------
inline float GPVector3::Length() {
	F32vec4 r = _mm_mul_ps(vec,vec);
	F32vec1 t = _mm_add_ss(_mm_shuffle_ps(r,r,1), _mm_add_ss(_mm_movehl_ps(r,r),r));
	t = sqrt(t);
	return *(float *)&t;
}

// ----------------------------------------------------------------------------
//  Name:	GPVector3::Normalize
//  Desc:	Normalized the (X,Y,Z) elements of the vector.
// ----------------------------------------------------------------------------
inline GPVector3& GPVector3::Normalize() {
	F32vec4 r = _mm_mul_ps(vec,vec);
	F32vec1 t = _mm_add_ss(_mm_shuffle_ps(r,r,1), _mm_add_ss(_mm_movehl_ps(r,r),r));
	#ifdef ZERO_VECTOR
		t = _mm_cmpneq_ss(t, _mm_setzero_ps()) & rsqrt_nr(t);
	#else
		t = rsqrt_nr(t);
	#endif
	vec = _mm_mul_ps(vec, _mm_shuffle_ps(t,t,0x00));
	return *this;
}

// ----------------------------------------------------------------------------
//  Name:	~GPVector3 [Normalize]
//  Desc:	Normalized the source vector.
// ----------------------------------------------------------------------------
inline GPVector3 operator ~ (const GPVector3 &V) {
	F32vec4 r = _mm_mul_ps(V.vec,V.vec);
	F32vec1 t = _mm_add_ss(_mm_shuffle_ps(r,r,1), _mm_add_ss(_mm_movehl_ps(r,r),r));
	#ifdef ZERO_VECTOR
		t = _mm_cmpneq_ss(t, _mm_setzero_ps()) & rsqrt_nr(t);
	#else
		t = rsqrt_nr(t);
	#endif
	return _mm_mul_ps(V.vec, _mm_shuffle_ps(t,t,0x00));
}

