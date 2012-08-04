//////////////////////////////////////////////////////////////////////////////
// Math routines used by the blocker polygon implementation.  
// Copyright (c) 2001, Monolith Productions, Inc.  All Rights Reserved
// Portions Copyright (c) 2000, 2001 Magic Software, Inc. (see below)

// Blocker triangle representation
struct SBlockerTri
{
	LTVector m_vOrigin, m_vEdge0, m_vEdge1;
};

// Blocker line segment representation
struct SBlockerSeg
{
	LTVector m_vOrigin, m_vDirection;
};

// Calculate the un-clamped barycentric coords of a point
inline void GetBarycentricCoords(
	const LTVector &vPt,
	const SBlockerTri &cTri,
	float fInvTriArea, 
	float *pU, 
	float *pV)
{
	LTVector vOffset = vPt - cTri.m_vOrigin;
	*pU = cTri.m_vEdge0.Cross(vOffset).Mag() * fInvTriArea;
	*pV = cTri.m_vEdge1.Cross(vOffset).Mag() * fInvTriArea;
}

inline void GetBarycentricCoords(
	const LTVector &vPt,
	const SBlockerTri &cTri,
	float *pU, 
	float *pV)
{
	GetBarycentricCoords(vPt, cTri, 1.0f / cTri.m_vEdge0.Cross(cTri.m_vEdge1).Mag(), pU, pV);
}

// The following routines are adapted from the free portion of the WildMagic 
// engine by Magic Software, Inc.  The copyright notice on the original 
// implementation is as follows:

// Magic Software, Inc.
// http://www.magic-software.com
// Copyright (c) 2000, 2001.  All Rights Reserved
//
// Source code from Magic Software is supplied under the terms of a license
// agreement and may not be copied or disclosed except in accordance with the
// terms of that agreement.  The various license agreements may be found at
// the Magic Software web site.  This file is subject to the license
//
// FREE SOURCE CODE
// http://www.magic-software.com/License/free.pdf

// Calculate the squared distance between two line segments
float DistSqrSegSeg(const SBlockerSeg& rkSeg0, const SBlockerSeg& rkSeg1,
    float* pfSegP0, float* pfSegP1);

// Calculate the distance between two line segments
inline float DistSegSeg(const SBlockerSeg& rkSeg0, const SBlockerSeg& rkSeg1,
    float* pfSegP0, float* pfSegP1)
{
	return DistSqrSegSeg(rkSeg0, rkSeg1, pfSegP0, pfSegP1);
}

// Calculate the squared distance between a point and a parallelogram
float DistSqrPtPgm(const LTVector& rkPoint,
    const SBlockerTri& rkPgm, float* pfSParam, float* pfTParam);

// Calculate the distance between a point and a parallelogram
inline float DistPtPgm(const LTVector& rkPoint,
    const SBlockerTri& rkPgm, float* pfSParam, float* pfTParam)
{
    return sqrtf(DistSqrPtPgm(rkPoint,rkPgm,pfSParam,pfTParam));
}

// Calculate the squared distance between a line segment and a parallelogram
float DistSqrSegPgm(const SBlockerSeg &rkSeg, const SBlockerTri &rkPgm, 
	float *pfSegP, float *pfPgmP0, float *pfPgmP1);

// Calculate the distance between a line segment and a parallelogram
inline float DistSegPgm(const SBlockerSeg &rkSeg, const SBlockerTri &rkPgm, 
	float *pfSegP, float *pfPgmP0, float *pfPgmP1)
{
	return sqrtf(DistSqrSegPgm(rkSeg, rkPgm, pfSegP, pfPgmP0, pfPgmP1));
}

// Calculate the squared distance between a point and a triangle
float DistSqrPtTri(const LTVector& rkPoint, const SBlockerTri& rkTri,
    float* pfSParam, float* pfTParam);

// Calculate the distance between a point and a triangle
inline float DistPtTri(const LTVector& rkPoint, const SBlockerTri& rkTri,
    float* pfSParam, float* pfTParam)
{
	return sqrtf(DistSqrPtTri(rkPoint, rkTri, pfSParam, pfTParam));
}

// Calculate the squared distance between a line segment and a triangle
float DistSqrSegTri(const SBlockerSeg& rkSeg, const SBlockerTri& rkTri,
    float* pfSegP, float* pfTriP0, float* pfTriP1);

// Calculate the distance between a line segment and a triangle
inline float DistSegTri(const SBlockerSeg& rkSeg, const SBlockerTri& rkTri,
    float* pfSegP, float* pfTriP0, float* pfTriP1)
{
	return sqrtf(DistSqrSegTri(rkSeg, rkTri, pfSegP, pfTriP0, pfTriP1));
}

// Calculate the squared distance between a triangle and a parallelogram
float DistSqrTriPgm(const SBlockerTri &rkTri, const SBlockerTri &rkPgm,
	float *pfTriP0, float *pfTriP1, float *pfPgmP0, float *pfPgmP1);

// Calculate the distance between a triangle and a parallelogram
inline float DistTriPgm(const SBlockerTri &rkTri, const SBlockerTri &rkPgm,
	float *pTriP0, float *pTriP1, float *pfPgmP0, float *pfPgmP1)
{
	return sqrtf(DistSqrTriPgm(rkTri, rkPgm, pTriP0, pTriP1, pfPgmP0, pfPgmP1));
}

