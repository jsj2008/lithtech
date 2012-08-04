//////////////////////////////////////////////////////////////////////////////
// PC-specific rendering vertex structure

#ifndef __PCRENDERVERT_H__
#define __PCRENDERVERT_H__

#include "prepoly.h"
#include "pcrendertools.h"

class CAbstractIO;

struct CPCRenderVert3T
{
	CPCRenderVert3T() {}
	CPCRenderVert3T(const CPCRenderVert3T &cOther) :
		m_vPos(cOther.m_vPos),
		m_fU0(cOther.m_fU0),
		m_fV0(cOther.m_fV0),
		m_fU1(cOther.m_fU1),
		m_fV1(cOther.m_fV1),
		m_fU2(cOther.m_fU2),
		m_fV2(cOther.m_fV2),
		m_nColor(cOther.m_nColor),
		m_vNormal(cOther.m_vNormal),
		m_vTangent(cOther.m_vTangent),
		m_vBinormal(cOther.m_vBinormal)
	{
	}
	CPCRenderVert3T(
			const LTVector &vPos, 
			float fU0, 
			float fV0, 
			float fU1, 
			float fV1, 
			float fU2,
			float fV2,
			uint32 nColor, 
			const LTVector &vNormal,
			const LTVector &vTangent,
			const LTVector &vBinormal
		) :
		m_vPos(vPos),
		m_fU0(fU0),
		m_fV0(fV0),
		m_fU1(fU1),
		m_fV1(fV1),
		m_fU2(fU2),
		m_fV2(fV2),
		m_nColor(nColor),
		m_vNormal(vNormal),
		m_vTangent(vTangent),
		m_vBinormal(vBinormal)
	{
	}

	CPCRenderVert3T(const CPrePoly *pPoly, uint32 nIndex)
	{
		const CPolyVert &cPt = pPoly->Vert(nIndex);

		m_vPos = cPt.m_Vec;
		if(pPoly->GetSurface()->m_Texture[0].IsValid())
		{
			LTVector vOfs = cPt.m_Vec - pPoly->TextureO(0);
			m_fU0 = pPoly->TextureP(0).Dot(vOfs) / pPoly->TextureWidth(0);
			m_fV0 = pPoly->TextureQ(0).Dot(vOfs) / pPoly->TextureHeight(0);
		}
		else
		{
			m_fU1 = 0.0f;
			m_fV1 = 0.0f;
		}

		if(pPoly->GetSurface()->m_Texture[1].IsValid())
		{
			LTVector vOfs = cPt.m_Vec - pPoly->TextureO(1);
			m_fU1 = pPoly->TextureP(1).Dot(vOfs) / pPoly->TextureWidth(1);
			m_fV1 = pPoly->TextureQ(1).Dot(vOfs) / pPoly->TextureHeight(1);
		}
		else
		{
			m_fU1 = 0.0f;
			m_fV1 = 0.0f;
		}

		float fLMGridScale = 1.0f / (float)pPoly->GetSurface()->GetLMGridSize();
		LTVector vOfs = cPt.m_Vec - pPoly->m_PolyO;
		m_fU2 = pPoly->P().Dot(vOfs) * fLMGridScale + 0.5f;
		m_fV2 = pPoly->Q().Dot(vOfs) * fLMGridScale + 0.5f;

		m_nColor = ConvVecPCColor(cPt.m_Color, cPt.m_Alpha);
		m_vNormal = cPt.m_Normal;

		// Create Tangent Space
		LTVector vUnitP = pPoly->TextureP(0).GetNormalized();
		LTVector vUnitQ = pPoly->TextureQ(0).GetNormalized();
		m_vTangent = (vUnitP - vUnitP.Dot(pPoly->Normal()) * pPoly->Normal()).GetNormalized();
		m_vBinormal = (vUnitQ - vUnitQ.Dot(pPoly->Normal()) * pPoly->Normal()).GetNormalized();
	}

	CPCRenderVert3T(const CPrePoly *pPoly, const LTVector &vPos, const LTVector &vNormal, const LTVector &vColor, float fAlpha)
	{
		m_vPos = vPos;
		if(pPoly->GetSurface()->m_Texture[0].IsValid())
		{
			LTVector vOfs = vPos - pPoly->TextureO(0);
			m_fU0 = pPoly->TextureP(0).Dot(vOfs) / pPoly->TextureWidth(0);
			m_fV0 = pPoly->TextureQ(0).Dot(vOfs) / pPoly->TextureHeight(0);
		}
		else
		{
			m_fU1 = 0.0f;
			m_fV1 = 0.0f;
		}

		if(pPoly->GetSurface()->m_Texture[1].IsValid())
		{
			LTVector vOfs = vPos - pPoly->TextureO(1);
			m_fU1 = pPoly->TextureP(1).Dot(vOfs) / pPoly->TextureWidth(1);
			m_fV1 = pPoly->TextureQ(1).Dot(vOfs) / pPoly->TextureHeight(1);
		}
		else
		{
			m_fU1 = 0.0f;
			m_fV1 = 0.0f;
		}
		
		float fLMGridScale = 1.0f / (float)pPoly->GetSurface()->GetLMGridSize();
		LTVector vOfs = vPos - pPoly->m_PolyO;
		m_fU2 = pPoly->P().Dot(vOfs) * fLMGridScale + 0.5f;
		m_fV2 = pPoly->Q().Dot(vOfs) * fLMGridScale + 0.5f;
		m_nColor = ConvVecPCColor(vColor, fAlpha);
		m_vNormal = vNormal;

		// Create Tangent Space
		LTVector vUnitP = pPoly->TextureP(0).GetNormalized();
		LTVector vUnitQ = pPoly->TextureQ(0).GetNormalized();
		m_vTangent = (vUnitP - vUnitP.Dot(pPoly->Normal()) * pPoly->Normal()).GetNormalized();
		m_vBinormal = (vUnitQ - vUnitQ.Dot(pPoly->Normal()) * pPoly->Normal()).GetNormalized();
	}

	CPCRenderVert3T &operator=(const CPCRenderVert3T &cOther)
	{
		m_vPos = cOther.m_vPos;
		m_fU0 = cOther.m_fU0;
		m_fV0 = cOther.m_fV0;
		m_fU1 = cOther.m_fU1;
		m_fV1 = cOther.m_fV1;
		m_fU2 = cOther.m_fU2;
		m_fV2 = cOther.m_fV2;
		m_nColor = cOther.m_nColor;
		m_vNormal = cOther.m_vNormal;
		m_vTangent = cOther.m_vTangent;
		m_vBinormal = cOther.m_vBinormal;
		return *this;
	}

	bool operator==(const CPCRenderVert3T &cOther) const
	{
		return
			m_vPos.NearlyEquals(cOther.m_vPos, 0.0001f) &&
			(fabs(m_fU0 - cOther.m_fU0) < 0.0001f) &&
			(fabs(m_fV0 - cOther.m_fV0) < 0.0001f) &&
			(fabs(m_fU1 - cOther.m_fU1) < 0.0001f) &&
			(fabs(m_fV1 - cOther.m_fV1) < 0.0001f) &&
			(fabs(m_fU2 - cOther.m_fU2) < 0.0001f) &&
			(fabs(m_fV2 - cOther.m_fV2) < 0.0001f) &&
			(m_nColor == cOther.m_nColor) &&
			m_vNormal.NearlyEquals(cOther.m_vNormal, 0.0001f) &&
			m_vTangent.NearlyEquals(cOther.m_vTangent, 0.0001f) &&
			m_vBinormal.NearlyEquals(cOther.m_vBinormal, 0.0001f)
		;
	}

	// Get a new vertex representing a linear interpolation between v1 and v2
	static CPCRenderVert3T Lerp(const CPCRenderVert3T &v1, const CPCRenderVert3T &v2, float fPercent)
	{
		CPCRenderVert3T vResult;
		VEC_LERP(vResult.m_vPos, v1.m_vPos, v2.m_vPos, fPercent);
		vResult.m_fU0 = LTLERP(v1.m_fU0, v2.m_fU0, fPercent);
		vResult.m_fV0 = LTLERP(v1.m_fV0, v2.m_fV0, fPercent);
		vResult.m_fU1 = LTLERP(v1.m_fU1, v2.m_fU1, fPercent);
		vResult.m_fV1 = LTLERP(v1.m_fV1, v2.m_fV1, fPercent);
		vResult.m_fU2 = LTLERP(v1.m_fU2, v2.m_fU2, fPercent);
		vResult.m_fV2 = LTLERP(v1.m_fV2, v2.m_fV2, fPercent);

		// Need to put in a faster color interpolator
		vResult.m_nColor = (uint32)(LTLERP((float)(v1.m_nColor & 0xFF), (float)(v2.m_nColor & 0xFF), fPercent)) & 0xFF;
		vResult.m_nColor |= (uint32)(LTLERP((float)(v1.m_nColor & 0xFF00), (float)(v2.m_nColor & 0xFF00), fPercent)) & 0xFF00;
		vResult.m_nColor |= (uint32)(LTLERP((float)(v1.m_nColor & 0xFF0000), (float)(v2.m_nColor & 0xFF0000), fPercent)) & 0xFF0000;
		vResult.m_nColor |= (uint32)(LTLERP((float)(v1.m_nColor & 0xFF000000), (float)(v2.m_nColor & 0xFF000000), fPercent)) & 0xFF000000;
		VEC_LERP(vResult.m_vNormal, v1.m_vNormal, v2.m_vNormal, fPercent);
		vResult.m_vNormal.Normalize();
		VEC_LERP(vResult.m_vTangent, v1.m_vTangent, v2.m_vTangent, fPercent);
		vResult.m_vTangent.Normalize();
		VEC_LERP(vResult.m_vBinormal, v1.m_vBinormal, v2.m_vBinormal, fPercent);
		vResult.m_vBinormal.Normalize();
		return vResult;
	}

public:
	LTVector m_vPos;
	float m_fU0, m_fV0, m_fU1, m_fV1, m_fU2, m_fV2;
	uint32 m_nColor;
	LTVector m_vNormal;
	LTVector m_vTangent;
	LTVector m_vBinormal;
};

struct CPCRenderVert2T
{
	CPCRenderVert2T() {}
	CPCRenderVert2T(const CPCRenderVert2T &cOther) :
		m_vPos(cOther.m_vPos),
		m_fU0(cOther.m_fU0),
		m_fV0(cOther.m_fV0),
		m_fU1(cOther.m_fU1),
		m_fV1(cOther.m_fV1),
		m_nColor(cOther.m_nColor),
		m_vNormal(cOther.m_vNormal),
		m_vTangent(cOther.m_vTangent),
		m_vBinormal(cOther.m_vBinormal)
	{
	}
	CPCRenderVert2T(
			const LTVector &vPos, 
			float fU0, 
			float fV0, 
			float fU1, 
			float fV1, 
			uint32 nColor, 
			const LTVector &vNormal,
			const LTVector &vTangent,
			const LTVector &vBinormal
		) :
		m_vPos(vPos),
		m_fU0(fU0),
		m_fV0(fV0),
		m_fU1(fU1),
		m_fV1(fV1),
		m_nColor(nColor),
		m_vNormal(vNormal),
		m_vTangent(vTangent),
		m_vBinormal(vBinormal)
	{
	}

	CPCRenderVert2T(const CPrePoly *pPoly, uint32 nIndex)
	{
		const CPolyVert &cPt = pPoly->Vert(nIndex);

		m_vPos = cPt.m_Vec;
		LTVector vOfs = cPt.m_Vec - pPoly->TextureO(0);
		m_fU0 = pPoly->TextureP(0).Dot(vOfs) / pPoly->TextureWidth(0);
		m_fV0 = pPoly->TextureQ(0).Dot(vOfs) / pPoly->TextureHeight(0);
		float fLMGridScale = 1.0f / (float)pPoly->GetSurface()->GetLMGridSize();
		vOfs = cPt.m_Vec - pPoly->m_PolyO;
		m_fU1 = pPoly->P().Dot(vOfs) * fLMGridScale + 0.5f;
		m_fV1 = pPoly->Q().Dot(vOfs) * fLMGridScale + 0.5f;
		m_nColor = ConvVecPCColor(cPt.m_Color, cPt.m_Alpha);
		m_vNormal = cPt.m_Normal;

		// Create Tangent Space
		LTVector vUnitP = pPoly->TextureP(0).GetNormalized();
		LTVector vUnitQ = pPoly->TextureQ(0).GetNormalized();
		m_vTangent = (vUnitP - vUnitP.Dot(pPoly->Normal()) * pPoly->Normal()).GetNormalized();
		m_vBinormal = (vUnitQ - vUnitQ.Dot(pPoly->Normal()) * pPoly->Normal()).GetNormalized();
	}

	CPCRenderVert2T(const CPrePoly *pPoly, const LTVector &vPos, const LTVector &vNormal, const LTVector &vColor, float fAlpha)
	{
		m_vPos = vPos;
		LTVector vOfs = vPos - pPoly->TextureO(0);
		m_fU0 = pPoly->TextureP(0).Dot(vOfs) / pPoly->TextureWidth(0);
		m_fV0 = pPoly->TextureQ(0).Dot(vOfs) / pPoly->TextureHeight(0);
		float fLMGridScale = 1.0f / (float)pPoly->GetSurface()->GetLMGridSize();
		vOfs = vPos - pPoly->m_PolyO;
		m_fU1 = pPoly->P().Dot(vOfs) * fLMGridScale + 0.5f;
		m_fV1 = pPoly->Q().Dot(vOfs) * fLMGridScale + 0.5f;
		m_nColor = ConvVecPCColor(vColor, fAlpha);
		m_vNormal = vNormal;

		// Create Tangent Space
		LTVector vUnitP = pPoly->TextureP(0).GetNormalized();
		LTVector vUnitQ = pPoly->TextureQ(0).GetNormalized();
		m_vTangent = (vUnitP - vUnitP.Dot(pPoly->Normal()) * pPoly->Normal()).GetNormalized();
		m_vBinormal = (vUnitQ - vUnitQ.Dot(pPoly->Normal()) * pPoly->Normal()).GetNormalized();
	}

	CPCRenderVert2T &operator=(const CPCRenderVert2T &cOther)
	{
		m_vPos = cOther.m_vPos;
		m_fU0 = cOther.m_fU0;
		m_fV0 = cOther.m_fV0;
		m_fU1 = cOther.m_fU1;
		m_fV1 = cOther.m_fV1;
		m_nColor = cOther.m_nColor;
		m_vNormal = cOther.m_vNormal;
		m_vTangent = cOther.m_vTangent;
		m_vBinormal = cOther.m_vBinormal;
		return *this;
	}

	bool operator==(const CPCRenderVert2T &cOther) const
	{
		return
			m_vPos.NearlyEquals(cOther.m_vPos, 0.0001f) &&
			(fabs(m_fU0 - cOther.m_fU0) < 0.0001f) &&
			(fabs(m_fV0 - cOther.m_fV0) < 0.0001f) &&
			(fabs(m_fU1 - cOther.m_fU1) < 0.0001f) &&
			(fabs(m_fV1 - cOther.m_fV1) < 0.0001f) &&
			(m_nColor == cOther.m_nColor) &&
			m_vNormal.NearlyEquals(cOther.m_vNormal, 0.0001f) &&
			m_vTangent.NearlyEquals(cOther.m_vTangent, 0.0001f) &&
			m_vBinormal.NearlyEquals(cOther.m_vBinormal, 0.0001f)
		;
	}

	// Get a new vertex representing a linear interpolation between v1 and v2
	static CPCRenderVert2T Lerp(const CPCRenderVert2T &v1, const CPCRenderVert2T &v2, float fPercent)
	{
		CPCRenderVert2T vResult;
		VEC_LERP(vResult.m_vPos, v1.m_vPos, v2.m_vPos, fPercent);
		vResult.m_fU0 = LTLERP(v1.m_fU0, v2.m_fU0, fPercent);
		vResult.m_fV0 = LTLERP(v1.m_fV0, v2.m_fV0, fPercent);
		vResult.m_fU1 = LTLERP(v1.m_fU1, v2.m_fU1, fPercent);
		vResult.m_fV1 = LTLERP(v1.m_fV1, v2.m_fV1, fPercent);
		// Need to put in a faster color interpolator
		vResult.m_nColor = (uint32)(LTLERP((float)(v1.m_nColor & 0xFF), (float)(v2.m_nColor & 0xFF), fPercent)) & 0xFF;
		vResult.m_nColor |= (uint32)(LTLERP((float)(v1.m_nColor & 0xFF00), (float)(v2.m_nColor & 0xFF00), fPercent)) & 0xFF00;
		vResult.m_nColor |= (uint32)(LTLERP((float)(v1.m_nColor & 0xFF0000), (float)(v2.m_nColor & 0xFF0000), fPercent)) & 0xFF0000;
		vResult.m_nColor |= (uint32)(LTLERP((float)(v1.m_nColor & 0xFF000000), (float)(v2.m_nColor & 0xFF000000), fPercent)) & 0xFF000000;
		VEC_LERP(vResult.m_vNormal, v1.m_vNormal, v2.m_vNormal, fPercent);
		vResult.m_vNormal.Normalize();
		VEC_LERP(vResult.m_vTangent, v1.m_vTangent, v2.m_vTangent, fPercent);
		vResult.m_vTangent.Normalize();
		VEC_LERP(vResult.m_vBinormal, v1.m_vBinormal, v2.m_vBinormal, fPercent);
		vResult.m_vBinormal.Normalize();
		return vResult;
	}

	friend CAbstractIO &operator<<(CAbstractIO &file, const CPCRenderVert2T &cVert);

public:
	LTVector m_vPos;
	float m_fU0, m_fV0, m_fU1, m_fV1;
	uint32 m_nColor;
	LTVector m_vNormal;
	LTVector m_vTangent;
	LTVector m_vBinormal;
};

// Streaming operator
CAbstractIO &operator<<(CAbstractIO &file, const CPCRenderVert2T &cVert);

#endif //__PCRENDERVERT_H__