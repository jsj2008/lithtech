//
// compress.cpp - compression routines for common types 
//
// Copyright (C) 2001 LithTech All Rights Reserved.
//

// NOTE: this module also used by the Autoview library
//       Please do not start including engine headers willy-nilly

#include "bdefs.h"
#include "compress.h"
#include "packetdefs.h"

class CCompress : public ICompress
{
public:
	declare_interface(CCompress);

	void EncodeCompressVector( CompVector *pCVec, const LTVector *pVal );
	void DecodeCompressVector( LTVector *pVal, const CompVector *pCVec );
	void EncodeCompressWorldPosition(CompWorldPos *pPos, const LTVector *pVal,
		const LTVector &world_pos_min, const LTVector &world_pos_inv_diff,
		bool bHiRes = true);
	void DecodeCompressWorldPosition(LTVector *pVal, const CompWorldPos *pPos,
		const LTVector &world_pos_min, const LTVector &world_pos_inv_max,
		bool bHiRes = true);
	void EncodeCompressRotation(const LTRotation *pRot, CompRot *pCompRot);
	void UncompressRotation(char *bytes, LTRotation *pRot);

	// CCompress helper function (not in ICompress)
protected:
	void UncompressSuperRotation(const char *bytes, LTVector *up, LTVector *forward);
};

// instantiate our implementation class
define_interface(CCompress, ICompress);

// Encodes vector to compressed form.  Uncompressed, a vector is 12 bytes.  Compressed, it is 9 bytes.
//
// Range value:		4 bytes
// Vector vals:		3 bytes
//
// Vector vals is split up into bits:
//
// Byte 0-3 Byte 4   Byte 5  Byte 6  Byte 7   Byte 8
// |------- |--------|------ |-------|------- |-----------------
// |        0123456789012345 6789012345678901 23    890      123
// |------- |--------------- |--------------- |-    |--      |--
// Coord A  B,low            C,low            Order B,high   C,high
//
// Coord A is used as the range and it can be x,y or z.  The Order bits tell which position the Coord A is.
//
// Order	Coord A	Coord B	Coord C
// 0		x		y		z
// 1		y		x		z
// 2		z		x		y
//
void CCompress::EncodeCompressVector( CompVector *pCVec, const LTVector *pVal )
{
	float fB, fC, fAbsA, fAbsVec;
//	uint32 dwB, dwC;
//	unsigned char order;

	// Pick X as the coord with largest mag...
	fAbsVec = ( float )fabs( pVal->x );
	pCVec->fA = pVal->x;
	fB = pVal->y;
	fC = pVal->z;
	fAbsA = fAbsVec;
	pCVec->order = 0;

	// Check if y has bigger mag...
	fAbsVec = ( float )fabs( pVal->y );
	if( fAbsVec > fAbsA )
	{
		pCVec->fA = pVal->y;
		fB = pVal->x;
		fAbsA = fAbsVec;
		pCVec->order = ( 1 << 6 );
	}
	
	// Check if z has bigger mag...
	fAbsVec = ( float )fabs( pVal->z );
	if( fAbsVec > fAbsA )
	{
		pCVec->fA = pVal->z;
		fB = pVal->x;
		fC = pVal->y;
		fAbsA = fAbsVec;
		pCVec->order = ( 1 << 7 );
	}

	// Create scaled value for B coord...
	pCVec->dwB = ( uint32 )(( fabs( fB ) / fAbsA ) * (( 1 << 18 ) - 1 ));
	pCVec->order |= ((( pCVec->dwB >> 16 ) & 0x03 ) << 3 );
	if( SIGN( fB ) != SIGN( pCVec->fA ))
	{
		pCVec->order |= ( 1 << 5 );
	}

	// Create scaled value for C coord...
	pCVec->dwC = ( uint32 )(( fabs( fC ) / fAbsA ) * (( 1 << 18 ) - 1 ));
	pCVec->order |= ( pCVec->dwC >> 16 ) & 0x03;
	if( SIGN( fC ) != SIGN( pCVec->fA ))
	{
		pCVec->order |= ( 1 << 2 );
	}

}

void CCompress::DecodeCompressVector( LTVector *pVal, const CompVector *pCVec )
{
	float fB, fC;
	uint32 dwTemp;
	
	dwTemp = (( pCVec->order >> 3 ) & 0x03 ) << 16;
	uint32 temp_dwB = pCVec->dwB + dwTemp;
	fB = ( pCVec->fA * temp_dwB ) / ( float )( 1 << 18 );
	if( pCVec->order & ( 1 << 5 ))
		fB *= -1.0f;

	dwTemp = ( pCVec->order & 0x03 ) << 16;
	uint32 temp_dwC = pCVec->dwC + dwTemp;
	fC = ( pCVec->fA * temp_dwC ) / ( float )( 1 << 18 );
	if( pCVec->order & ( 1 << 2 ))
		fC *= -1.0f;

	switch( pCVec->order >> 6 )
	{
		default:
		case 0:
			pVal->x = pCVec->fA;
			pVal->y = fB;
			pVal->z = fC;
			break;
		case 1:
			pVal->x = fB;
			pVal->y = pCVec->fA;
			pVal->z = fC;
			break;
		case 2:
			pVal->x = fB;
			pVal->y = fC;
			pVal->z = pCVec->fA;
			break;
	}
}

void CCompress::EncodeCompressWorldPosition(CompWorldPos *pPos, const LTVector *pVal, 
    const LTVector &world_pos_min, const LTVector &world_pos_inv_diff, bool bHiRes)
{
	LTVector vWorldPercent;
	uint32 xVal, yVal, zVal, extra;

	vWorldPercent = (*pVal - world_pos_min) * world_pos_inv_diff;

	const LTVector vScale((1 << NUM_POSITION_BITS_X) - 1, (1 << NUM_POSITION_BITS_Y) - 1, (1 << NUM_POSITION_BITS_Z) - 1);

	LTVector vScaledWorldPercent = vWorldPercent * vScale;

	// Clamp it to the valid range
	const LTVector vZero(0.0f, 0.0f, 0.0f);

	VEC_MIN(vScaledWorldPercent, vScaledWorldPercent, vScale);
	VEC_MAX(vScaledWorldPercent, vScaledWorldPercent, vZero);

	xVal = (uint32)(vScaledWorldPercent.x + 0.5f);
	yVal = (uint32)(vScaledWorldPercent.y + 0.5f);
	zVal = (uint32)(vScaledWorldPercent.z + 0.5f);

	if( bHiRes )
	{
		// Write the extra high bits.. this is worth the trouble because it
		// gives so much more resolution (256k-512k as opposed to 65k).
		extra = xVal & ((1 << NUM_EXTRA_BITS_X) - 1);
		extra |= (yVal & ((1 << NUM_EXTRA_BITS_Y) - 1)) << NUM_EXTRA_BITS_X;
		extra |= (zVal & ((1 << NUM_EXTRA_BITS_Z) - 1)) << (NUM_EXTRA_BITS_X + NUM_EXTRA_BITS_Y);
	}
	else
		extra = 0;
	
	xVal >>= NUM_EXTRA_BITS_X;
	yVal >>= NUM_EXTRA_BITS_Y;
	zVal >>= NUM_EXTRA_BITS_Z;
	
	pPos->m_Pos[0] = (uint16)(xVal & 0xFFFF);
	pPos->m_Pos[1] = (uint16)(yVal & 0xFFFF);
	pPos->m_Pos[2] = (uint16)(zVal & 0xFFFF);
	pPos->m_Extra = (uint8)extra;
}



void CCompress::DecodeCompressWorldPosition(LTVector *pVal, const CompWorldPos *pPos,
    const LTVector &world_pos_min, const LTVector &world_pos_max, bool bHiRes)
{
	uint32 pos[3];

	pos[0] = (pPos->m_Pos[0] << NUM_EXTRA_BITS_X);
	pos[1] = (pPos->m_Pos[1] << NUM_EXTRA_BITS_Y);
	pos[2] = (pPos->m_Pos[2] << NUM_EXTRA_BITS_Z);

	if( bHiRes )
	{
		pos[0] += (pPos->m_Extra & ((1 << NUM_EXTRA_BITS_X) - 1));
		pos[1] += (pPos->m_Extra & ((1 << (NUM_EXTRA_BITS_X + NUM_EXTRA_BITS_Y)) - 1)) >> NUM_EXTRA_BITS_X;
		pos[2] += (pPos->m_Extra & ((1 << (NUM_EXTRA_BITS_X + NUM_EXTRA_BITS_Y + NUM_EXTRA_BITS_Z)) - 1)) >> (NUM_EXTRA_BITS_X + NUM_EXTRA_BITS_Y);
	}

	const LTVector vLowBias((1 << NUM_EXTRA_BITS_X) / 2, (1 << NUM_EXTRA_BITS_Y) / 2, (1 << NUM_EXTRA_BITS_Z) / 2);
	const LTVector vHighBias(0.5f, 0.5f, 0.5f);
	const LTVector &vBias = (bHiRes) ? vHighBias : vLowBias;

	const LTVector vScale(1 << NUM_POSITION_BITS_X, 1 << NUM_POSITION_BITS_Y, 1 << NUM_POSITION_BITS_Z);

	LTVector vScaledWorldPercent((float)pos[0], (float)pos[1], (float)pos[2]);

	LTVector vWorldPercent = (vScaledWorldPercent + vBias) / vScale;

	*pVal = world_pos_min + (world_pos_max - world_pos_min) * vWorldPercent;
}

void CCompress::UncompressSuperRotation(const char *bytes, LTVector *up, LTVector *forward)
{
	float t;

	forward->x = ((bytes[1] / 127.0f) - 0.5f) * 2.0f;
	forward->y = ((bytes[0] / 127.0f) - 0.5f) * 2.0f;
	forward->z = (float)bytes[2] / 127.0f;

	// Compressed.. figure out the up vector.
	up->Init(forward->x, forward->y+50.0f, forward->z); // Here's our fake up vector.
	t = -forward->Dot(*up) / forward->MagSqr();
	*up += *forward * t;
}

void CCompress::UncompressRotation(char *bytes, LTRotation *pRot)
{
	LTMatrix mat;
	LTVector right, up, forward;
	LTBOOL bFlip;

	if(bytes[0] < 0)
	{
		//ASSERT(-bytes[0] <= 127);
		bytes[0] = -bytes[0];
		
		bFlip = LTFALSE;
		if(bytes[1] < 0)
		{
			bytes[1] = -bytes[1];
			bFlip = LTTRUE;
		}

		UncompressSuperRotation(bytes, &up, &forward);

		up.Norm();
		forward.Norm();

		if(bFlip)
		{
			up = -up;
		}
	}
	else
	{
		forward.y = ((bytes[0] / 127.0f) - 0.5f) * 2.0f;
		forward.x = ((bytes[1] / 127.0f) - 0.5f) * 2.0f;
		forward.z = (float)bytes[2] / 127.0f;

		up.x = (float)bytes[3] / 127.0f;
		up.y = (float)bytes[4] / 127.0f;
		up.z = (float)bytes[5] / 127.0f;
	}

	// Fixup.
	right = forward.Cross(up);
	forward = up.Cross(right); // This ensures that all 3 are orthogonal.
	right.Norm();
	forward.Norm();
	up = right.Cross(forward);

	Mat_SetBasisVectors(&mat, &right, &up, &forward);
	quat_ConvertFromMatrix((float*)pRot, mat.m);
}


void CCompress::EncodeCompressRotation(const LTRotation *pRot, CompRot *pCompRot)
{
	LTMatrix mat;
	LTBOOL bSuperCompressed;
	LTVector testUp, testForward;
	LTVector realUp;
	float dot;

	quat_ConvertToMatrix((float*)pRot, mat.m);
	realUp.x = mat.m[0][1];
	realUp.y = mat.m[1][1];
	realUp.z = mat.m[2][1];

	pCompRot->m_Bytes[0] = (char)((1.0f + mat.m[1][2]) * 63.9f);
	pCompRot->m_Bytes[1] = (char)((1.0f + mat.m[0][2]) * 63.9f);
	pCompRot->m_Bytes[2] = (char)(mat.m[2][2] * 127.9f);
	pCompRot->m_Bytes[3] = (char)(mat.m[0][1] * 127.9f);
	pCompRot->m_Bytes[4] = (char)(mat.m[1][1] * 127.9f);
	pCompRot->m_Bytes[5] = (char)(mat.m[2][1] * 127.9f);

	// Figure out if we can reduce it to 3 bytes.
	bSuperCompressed = LTFALSE;
	UncompressSuperRotation(pCompRot->m_Bytes, &testUp, &testForward);
	if(VEC_MAGSQR(testUp) > 0.1f)
	{
		VEC_NORM(testUp);
		
		dot = VEC_DOT(testUp, realUp);
		if(dot > ROTATION_COMPRESS_LIMIT)
		{
			bSuperCompressed = LTTRUE;
		}
		else if(dot < -ROTATION_COMPRESS_LIMIT)
		{
			bSuperCompressed = LTTRUE;
			pCompRot->m_Bytes[1] = pCompRot->m_Bytes[1] == 0 ? -1 : -pCompRot->m_Bytes[1];
		}
	}
	
	if(bSuperCompressed)
	{
		pCompRot->m_Bytes[0] = pCompRot->m_Bytes[0] == 0 ? -1 : -pCompRot->m_Bytes[0];
	}
}

