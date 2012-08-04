//////////////////////////////////////////////////////////////////////////////
//
// UTILS.CPP
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
// Intel makes no representations about the suitability of this software for 
// any purpose, and specifically disclaims all warranties. 
// See LEGAL.TXT for all the legal information.
//
/*
#include <windows.h>
#include "SimD_Utilities.h"
#include <memory.h>
#ifdef USE_AMLIB
#include "amaths.h"
#endif

typedef unsigned long DWORD;

#define M_PI 3.14159265358979323846f

const F32vec4 _MASKSIGN_;	// - - - -

const F32vec4 _ZERONE_;		// 1 0 0 1
const F32vec4 _0FFF_;		// 0 * * *
const F32vec4 Sign_PNPN;	// + - + -
const F32vec4 Sign_NPNP;	// - + - +
#ifdef USE_AMLIB
const F32vec4 _PIsY_;	// -sin cos cos sin
const F32vec4 _PIsXZ_;	// sin cos cos -sin
#endif


// This workaround prevents the library from causing Illegal Instruction 
// Exception during initialization, if the program is executed on machine 
// that does not support SSE.
class UTILSConstatntsInit {
public:
	UTILSConstatntsInit() {
		const struct {
			float _a[4];
			DWORD _b[16];
			float _c[8];
		} INITData = {
			1.0f, 0.0f, 0.0f, 1.0f,								// 1 0 0 1
			0x00000000, 0x80000000, 0x00000000, 0x80000000, 	// + - + -
			0x80000000, 0x00000000, 0x80000000, 0x00000000, 	// - + - +
			0x80000000, 0x80000000, 0x80000000, 0x80000000, 	// - - - -
			0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000, 	// 0 * * *
			0.0f, M_PI*0.5f, M_PI*0.5f, -M_PI,				// -sin cos cos sin
			-M_PI, M_PI*0.5f, M_PI*0.5f, 0.0f				// sin cos cos -sin
		};

		#define BLINDCOPY(var, source) memcpy((void *)&(var), &(source), sizeof(__m128))
		BLINDCOPY(_ZERONE_, INITData._a[0]);
		BLINDCOPY(Sign_PNPN, INITData._b[0]);
		BLINDCOPY(Sign_NPNP, INITData._b[4]);
		BLINDCOPY(_MASKSIGN_, INITData._b[8]);
		BLINDCOPY(_0FFF_, INITData._b[12]);
		#ifdef USE_AMLIB
		BLINDCOPY(_PIsY_, INITData._c[0]);
		BLINDCOPY(_PIsXZ_, INITData._c[4]);
		#endif
		#undef BLINDCOPY
	};
} UTILSConstatnts;




// ----------------------------------------------------------------------------
//  Name:	RotateXMatrix
//  Desc:	Returns the rotation matrix of X axis of specific angle.
// ----------------------------------------------------------------------------

void GPMatrix::RotateXMatrix(const float rads) {
	__asm { 
		xorps	xmm0,xmm0
		mov 	eax, this;
		fld		rads //float ptr rads
		movaps	[eax+0x10], xmm0		// clear line _L2
		fsincos
		fst		[eax+0x14] //float ptr [eax+0x14]	// set element _22
		movaps	[eax+0x20], xmm0		// clear line _L3
		fstp	[eax+0x28] //float ptr [eax+0x28]	// set element _33
		fst		[eax+0x18] //float ptr [eax+0x18]	// set element _23
		fchs
		movaps	[eax+0x00], xmm0		// clear line _L1
		fstp	[eax+0x24] //float ptr [eax+0x24]	// set element _32
		fld1
		fst		[eax+0x00] //float ptr [eax+0x00]	// set element _11
		movaps	[eax+0x30], xmm0		// clear line _L4
		fstp	[eax+0x3C] //float ptr [eax+0x3C]	// set element _44
	}
} 
GPMatrix RotateXMatrix(const float rads) {
	GPMatrix ret;
	ret.RotateXMatrix(rads);
	return ret;
} 

#ifdef USE_AMLIB
void GPMatrix::RotateXMatrixA(const float rads) {
	F32vec4 cossin = F32vec4(rads)+_PIsXZ_;
	_L1 = _L2 = _L3 = _L4 = _mm_setzero_ps();
	cossin = am_sin_ps(cossin); _m_empty();
	_11 = _44 = 1.0f;
	_mm_storeh_pi((__m64 *)&_22,cossin);
	_mm_storel_pi((__m64 *)&_32,cossin);
} 
GPMatrix RotateXMatrixA(const float rads) {
	GPMatrix ret;
	F32vec4 cossin = F32vec4(rads)+_PIsXZ_;
	ret._L1 = ret._L2 = ret._L3 = ret._L4 = _mm_setzero_ps();
	cossin = am_sin_ps(cossin); _m_empty();
	ret._11 = ret._44 = 1.0f;
	_mm_storeh_pi((__m64 *)&ret._22,cossin);
	_mm_storel_pi((__m64 *)&ret._32,cossin);
	return ret;
} 
#endif


// ----------------------------------------------------------------------------
//  Name:	RotateYMatrix
//  Desc:	Returns the rotation matrix of Y axis of specific angle.
// ----------------------------------------------------------------------------

void GPMatrix::RotateYMatrix(const float rads) {
	__asm { 
		xorps	xmm0,xmm0
		mov 	eax, this
		fld		rads //float ptr rads
		movaps	[eax+0x00], xmm0		// clear line _L1
		fsincos
		fst		[eax+0x00] //float ptr [eax+0x00]	// set element _11
		movaps	[eax+0x20], xmm0		// clear line _L3
		fstp	[eax+0x28] //float ptr [eax+0x28]	// set element _33
		fst		[eax+0x20] //float ptr [eax+0x20]	// set element _31
		fchs
		movaps	[eax+0x10], xmm0		// clear line _L2
		fstp	[eax+0x08] //float ptr [eax+0x08]	// set element _13
		fld1
		fst		[eax+0x14] //float ptr [eax+0x14]	// set element _22
		movaps	[eax+0x30], xmm0		// clear line _L4
		fstp	[eax+0x3C] //float ptr [eax+0x3C]	// set element _44
	}
} 
GPMatrix RotateYMatrix(const float rads) {
	GPMatrix ret;
	ret.RotateYMatrix(rads);
	return ret;
} 

#ifdef USE_AMLIB
void GPMatrix::RotateYMatrixA(const float rads) {
	F32vec4 cossin = F32vec4(rads)+_PIsY_;
	_L2 = _L4 = _mm_setzero_ps();
	cossin = am_sin_ps(cossin); _m_empty();
	_22 = _44 = 1.0f;
	_L1 = _mm_unpackhi_ps(cossin,_mm_setzero_ps());
	_L3 = _mm_unpacklo_ps(cossin,_mm_setzero_ps());
} 
GPMatrix RotateYMatrixA(const float rads) {
	GPMatrix ret;
	F32vec4 cossin = F32vec4(rads)+_PIsY_;
	cossin = am_sin_ps(cossin); _m_empty();
	ret._L2 = ret._L4 = _mm_setzero_ps();
	ret._22 = ret._44 = 1.0f;
	ret._L1 = _mm_unpackhi_ps(cossin,_mm_setzero_ps());
	ret._L3 = _mm_unpacklo_ps(cossin,_mm_setzero_ps());
	return ret;
} 
#endif


// ----------------------------------------------------------------------------
//  Name:	RotateZMatrix
//  Desc:	Returns the rotation matrix of Z axis of specific angle.
// ----------------------------------------------------------------------------

void GPMatrix::RotateZMatrix(const float rads) {
	__asm { 
		xorps	xmm0,xmm0
		mov 	eax, this
		fld		rads //float ptr rads
		movaps	[eax+0x00], xmm0		// clear line _L1
		fsincos
		fst		[eax+0x00] //float ptr [eax+0x00]	// set element _11
		movaps	[eax+0x10], xmm0		// clear line _L2
		fstp	[eax+0x14] //float ptr [eax+0x14]	// set element _22
		fst		[eax+0x04] //float ptr [eax+0x04]	// set element _12
		fchs
		movaps	[eax+0x20], xmm0		// clear line _L3
		fstp	[eax+0x10] //float ptr [eax+0x10]	// set element _21
		fld1
		fst		[eax+0x28] //float ptr [eax+0x28]	// set element _33
		movaps	[eax+0x30], xmm0		// clear line _L4
		fstp	[eax+0x3C] //float ptr [eax+0x3C]	// set element _44
	}
} 
//GPMatrix RotateZMatrix(const float rads) {
//	GPMatrix ret;
//	ret.RotateZMatrix(rads);
//	return ret;
//} 

#ifdef USE_AMLIB
void GPMatrix::RotateZMatrixA(const float rads) {
	F32vec4 cossin = F32vec4(rads)+_PIsXZ_;
	_L1 = _L2 = _L3 = _L4 = _mm_setzero_ps();
	cossin = am_sin_ps(cossin); _m_empty();
	_33 = _44 = 1.0f;
	_mm_storeh_pi((__m64 *)&_11,cossin);
	_mm_storel_pi((__m64 *)&_21,cossin);
} 
GPMatrix RotateZMatrixA(const float rads) {
	GPMatrix ret;
	F32vec4 cossin = F32vec4(rads)+_PIsXZ_;
	ret._L1 = ret._L2 = ret._L3 = ret._L4 = _mm_setzero_ps();
	cossin = am_sin_ps(cossin); _m_empty();
	ret._33 = ret._44 = 1.0f;
	_mm_storeh_pi((__m64 *)&ret._11,cossin);
	_mm_storel_pi((__m64 *)&ret._21,cossin);
	return ret;
} 
#endif


// ----------------------------------------------------------------------------
//  Name:	TranslateMatrix
//  Desc:	Returns the translation matrix for specific translation.
// ----------------------------------------------------------------------------
GPMatrix TranslateMatrix(const float dx, const float dy, const float dz)
{
	GPMatrix ret;
	ret.IdentityMatrix();
	ret._41 = dx;
	ret._42 = dy;
	ret._43 = dz;
	return ret;
} 
void GPMatrix::TranslateMatrix(const float dx, const float dy, const float dz)
{
	IdentityMatrix();
	_41 = dx;
	_42 = dy;
	_43 = dz;
} 

// ----------------------------------------------------------------------------
//  Name:	ScaleMatrix
//  Desc:	Returns the scaling matrix for x,y,z axis.
// ----------------------------------------------------------------------------
GPMatrix ScaleMatrix(const float a, const float b, const float c)
{
	GPMatrix ret;
	ret.IdentityMatrix();
	ret._11 = a;
	ret._22 = b;
	ret._33 = c;
	return ret;
} 
void GPMatrix::ScaleMatrix(const float a, const float b, const float c)
{
	IdentityMatrix();
	_11 = a;
	_22 = b;
	_33 = c;
} 

// ----------------------------------------------------------------------------
//  Name:	ScaleMatrix
//  Desc:	Returns the scaling matrix by scalar.
// ----------------------------------------------------------------------------
GPMatrix ScaleMatrix(const float a)
{
	GPMatrix ret;
	ret.IdentityMatrix();
	ret._11 = ret._22 = ret._33 = a;
	return ret;
} 
void GPMatrix::ScaleMatrix(const float a)
{
	IdentityMatrix();
	_11 = _22 = _33 = a;
} 




// ----------------------------------------------------------------------------
//  Name:	GPMatrix::MinValue
//  Desc:	Returns the asbolute minimum element of the matrix.
// ----------------------------------------------------------------------------
float GPMatrix::MinValue() {
	F32vec4 min1 = _mm_min_ps(_mm_abs_ps(_L1), _mm_abs_ps(_L2));
	F32vec4 min2 = _mm_min_ps(_mm_abs_ps(_L3), _mm_abs_ps(_L4));

	F32vec4 min = _mm_min_ps(min1, min2);
	min = _mm_min_ps(min, _mm_movehl_ps(min,min));
	min = _mm_min_ss(min, _mm_shuffle_ps(min,min,0x01));
	return min[0];
}

// ----------------------------------------------------------------------------
//  Name:	GPMatrix::MaxValue
//  Desc:	Returns the asbolute maximum element of the matrix.
// ----------------------------------------------------------------------------
float GPMatrix::MaxValue() {
	F32vec4 max1 = _mm_max_ps(_mm_abs_ps(_L1), _mm_abs_ps(_L2));
	F32vec4 max2 = _mm_max_ps(_mm_abs_ps(_L3), _mm_abs_ps(_L4));

	F32vec4 max = _mm_max_ps(max1, max2);
	max = _mm_max_ps(max, _mm_movehl_ps(max,max));
	max = _mm_max_ss(max, _mm_shuffle_ps(max,max,0x01));
	return max[0];
}


// ----------------------------------------------------------------------------
//  Name:	GPMatrix::Determinant
//  Desc:	Return the matrix determinant. A = det[M].
// ----------------------------------------------------------------------------
float GPMatrix::Determinant() {
	__m128 Va,Vb,Vc;
	__m128 r1,r2,r3,t1,t2,sum;
	F32vec4 Det;

	// First, Let's calculate the first four minterms of the first line
	t1 = _L4; t2 = _mm_ror_ps(_L3,1); 
	Vc = _mm_mul_ps(t2,_mm_ror_ps(t1,0));					// V3'·V4
	Va = _mm_mul_ps(t2,_mm_ror_ps(t1,2));					// V3'·V4"
	Vb = _mm_mul_ps(t2,_mm_ror_ps(t1,3));					// V3'·V4^

	r1 = _mm_sub_ps(_mm_ror_ps(Va,1),_mm_ror_ps(Vc,2));		// V3"·V4^ - V3^·V4"
	r2 = _mm_sub_ps(_mm_ror_ps(Vb,2),_mm_ror_ps(Vb,0));		// V3^·V4' - V3'·V4^
	r3 = _mm_sub_ps(_mm_ror_ps(Va,0),_mm_ror_ps(Vc,1));		// V3'·V4" - V3"·V4'

	Va = _mm_ror_ps(_L2,1);		sum = _mm_mul_ps(Va,r1);
	Vb = _mm_ror_ps(Va,1);		sum = _mm_add_ps(sum,_mm_mul_ps(Vb,r2));
	Vc = _mm_ror_ps(Vb,1);		sum = _mm_add_ps(sum,_mm_mul_ps(Vc,r3));

	// Now we can calculate the determinant:
	Det = _mm_mul_ps(sum,_L1);
	Det = _mm_add_ps(Det,_mm_movehl_ps(Det,Det));
	Det = _mm_sub_ss(Det,_mm_shuffle_ps(Det,Det,1));
	return Det[0];
}


// ----------------------------------------------------------------------------
//  Name:	GPMatrix::Inverse
//  Desc:	Inverse the 4x4 matrix. Matrix is set to inv[M].
//	Note:	In case of non-inversable matrix, sets the matrix to a Zero matrix (depending on the switch).
// ----------------------------------------------------------------------------
float GPMatrix::Inverse()
{
	__m128 Va,Vb,Vc;
	__m128 r1,r2,r3,tt,tt2;
	__m128 sum,Det,RDet;
	GPMatrix Minterms;
	__m128 trns0,trns1,trns2,trns3;

	// Calculating the minterms for the first line.

	// _mm_ror_ps is just a macro using _mm_shuffle_ps().
	tt = _L4; tt2 = _mm_ror_ps(_L3,1); 
	Vc = _mm_mul_ps(tt2,_mm_ror_ps(tt,0));					// V3'·V4
	Va = _mm_mul_ps(tt2,_mm_ror_ps(tt,2));					// V3'·V4"
	Vb = _mm_mul_ps(tt2,_mm_ror_ps(tt,3));					// V3'·V4^

	r1 = _mm_sub_ps(_mm_ror_ps(Va,1),_mm_ror_ps(Vc,2));		// V3"·V4^ - V3^·V4"
	r2 = _mm_sub_ps(_mm_ror_ps(Vb,2),_mm_ror_ps(Vb,0));		// V3^·V4' - V3'·V4^
	r3 = _mm_sub_ps(_mm_ror_ps(Va,0),_mm_ror_ps(Vc,1));		// V3'·V4" - V3"·V4'

	tt = _L2;
	Va = _mm_ror_ps(tt,1);		sum = _mm_mul_ps(Va,r1);
	Vb = _mm_ror_ps(tt,2);		sum = _mm_add_ps(sum,_mm_mul_ps(Vb,r2));
	Vc = _mm_ror_ps(tt,3);		sum = _mm_add_ps(sum,_mm_mul_ps(Vc,r3));

	// Calculating the determinant.
	Det = _mm_mul_ps(sum,_L1);
	Det = _mm_add_ps(Det,_mm_movehl_ps(Det,Det));

	Minterms._L1 = _mm_xor_ps(sum,Sign_PNPN);

	// Calculating the minterms of the second line (using previous results).
	tt = _mm_ror_ps(_L1,1);		sum = _mm_mul_ps(tt,r1);
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));
	Minterms._L2 = _mm_xor_ps(sum,Sign_NPNP);

	// Testing the determinant.
	Det = _mm_sub_ss(Det,_mm_shuffle_ps(Det,Det,1));
	#ifdef ZERO_SINGULAR
		int flag = _mm_comieq_ss(Det,_mm_sub_ss(tt,tt));
		// Using _mm_sub_ss, as only the first element has to be zeroed.
	#endif

	// Calculating the minterms of the third line.
	tt = _mm_ror_ps(_L1,1);
	Va = _mm_mul_ps(tt,Vb);									// V1'·V2"
	Vb = _mm_mul_ps(tt,Vc);									// V1'·V2^
	Vc = _mm_mul_ps(tt,_L2);								// V1'·V2

	r1 = _mm_sub_ps(_mm_ror_ps(Va,1),_mm_ror_ps(Vc,2));		// V1"·V2^ - V1^·V2"
	r2 = _mm_sub_ps(_mm_ror_ps(Vb,2),_mm_ror_ps(Vb,0));		// V1^·V2' - V1'·V2^
	r3 = _mm_sub_ps(_mm_ror_ps(Va,0),_mm_ror_ps(Vc,1));		// V1'·V2" - V1"·V2'

	tt = _mm_ror_ps(_L4,1);		sum = _mm_mul_ps(tt,r1);
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));
	Minterms._L3 = _mm_xor_ps(sum,Sign_PNPN);

	// Dividing is FASTER than rcp_nr! (Because rcp_nr causes many register-memory RWs).
	RDet = _mm_div_ss(_mm_load_ss((float *)&_ZERONE_), Det);
	RDet = _mm_shuffle_ps(RDet,RDet,0x00);

	// Devide the first 12 minterms with the determinant.
	Minterms._L1 = _mm_mul_ps(Minterms._L1, RDet);
	Minterms._L2 = _mm_mul_ps(Minterms._L2, RDet);
	Minterms._L3 = _mm_mul_ps(Minterms._L3, RDet);

	// Calculate the minterms of the forth line and devide by the determinant.
	tt = _mm_ror_ps(_L3,1);		sum = _mm_mul_ps(tt,r1);
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r2));
	tt = _mm_ror_ps(tt,1);		sum = _mm_add_ps(sum,_mm_mul_ps(tt,r3));
	Minterms._L4 = _mm_xor_ps(sum,Sign_NPNP);
	Minterms._L4 = _mm_mul_ps(Minterms._L4, RDet);

	#ifdef ZERO_SINGULAR
	// Check if the matrix is inversable.
	// Uses a delayed branch here, so the test would not interfere the calculations.
	// Assuming most of the matrices are inversable, the previous calculations are 
	// not wasted. It is faster this way.
		if (flag) {
			ZeroMatrix();
			return 0.0f;
		}
	#endif

	// Now we just have to transpose the minterms matrix.
	trns0 = _mm_unpacklo_ps(Minterms._L1,Minterms._L2);
	trns1 = _mm_unpacklo_ps(Minterms._L3,Minterms._L4);
	trns2 = _mm_unpackhi_ps(Minterms._L1,Minterms._L2);
	trns3 = _mm_unpackhi_ps(Minterms._L3,Minterms._L4);
	_L1 = _mm_movelh_ps(trns0,trns1);
	_L2 = _mm_movehl_ps(trns1,trns0);
	_L3 = _mm_movelh_ps(trns2,trns3);
	_L4 = _mm_movehl_ps(trns3,trns2);
	
	// That's all folks!
	return *(float *)&Det;	// Det[0]
}

*/

