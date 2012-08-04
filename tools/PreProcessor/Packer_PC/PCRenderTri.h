//////////////////////////////////////////////////////////////////////////////
// PC-Specific rendering triangle structure

#ifndef __PCRENDERTRI_H__
#define __PCRENDERTRI_H__

#include "pcrendervert.h"

class CAbstractIO;

struct CPCRenderTri
{
	CPCRenderTri() {}
	CPCRenderTri(const CPCRenderVert3T &v0, const CPCRenderVert3T &v1, const CPCRenderVert3T &v2) :
		m_Vert0(v0),
		m_Vert1(v1),
		m_Vert2(v2)
	{
		m_vNormal = (v0.m_vPos - v1.m_vPos).Cross(v2.m_vPos - v1.m_vPos);
		m_vNormal.Normalize();
	}
	CPCRenderTri(const CPCRenderVert3T &v0, const CPCRenderVert3T &v1, const CPCRenderVert3T &v2, const LTVector &vNormal) :
		m_Vert0(v0),
		m_Vert1(v1),
		m_Vert2(v2),
		m_vNormal(vNormal)
	{
	}
	CPCRenderTri(const CPCRenderTri &cOther) :
		m_Vert0(cOther.m_Vert0),
		m_Vert1(cOther.m_Vert1),
		m_Vert2(cOther.m_Vert2),
		m_vNormal(cOther.m_vNormal)
	{
	}

	CPCRenderTri &operator=(const CPCRenderTri &cOther) {
		m_Vert0 = cOther.m_Vert0;
		m_Vert1 = cOther.m_Vert1;
		m_Vert2 = cOther.m_Vert2;
		m_vNormal = cOther.m_vNormal;
		return *this;
	}

	bool IntersectAABB(const LTVector &vCenter, const LTVector &vHalfVec);

	friend CAbstractIO &operator<<(CAbstractIO &file, const CPCRenderTri &cVert);

	CPCRenderVert3T &Vert(uint32 nIndex) { return ((CPCRenderVert3T*)(&m_Vert0))[nIndex]; }
	const CPCRenderVert3T &Vert(uint32 nIndex) const { return ((const CPCRenderVert3T*)(&m_Vert0))[nIndex]; }

public:
	CPCRenderVert3T m_Vert0, m_Vert1, m_Vert2;
	LTVector m_vNormal;
};


#endif //__PCRENDERTRI_H__