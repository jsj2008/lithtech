//////////////////////////////////////////////////////////////////////////////
// CPrePolyFragments - Triangulated polygon fragment data class

#ifndef __PREPOLYFRAGMENTS_H__
#define __PREPOLYFRAGMENTS_H__

#include <vector>

class CPrePolyFragments
{
public:
	CPrePolyFragments();
	~CPrePolyFragments();

	struct SVert
	{
		SVert() {}
		SVert(const SVert &sOther)
		{
			*this = sOther;
		}
		SVert(const LTVector &vPos, const LTVector &vNormal, const LTVector &vColor, float fAlpha) :
			m_vPos(vPos), m_vNormal(vNormal), m_vColor(vColor), m_fAlpha(fAlpha) {}

		SVert &operator=(const SVert &sOther) {
			m_vPos = sOther.m_vPos;
			m_vNormal = sOther.m_vNormal;
			m_vColor = sOther.m_vColor;
			m_fAlpha = sOther.m_fAlpha;
			return *this;
		}
		LTVector m_vPos, m_vNormal;
		LTVector m_vColor;
		float    m_fAlpha;
	};

	typedef std::vector<uint32> TIndexList;
	typedef std::vector<SVert> TVertList;

	bool GetTri(uint32 nTriIndex, SVert *pPt0, SVert *pPt1, SVert *pPt2) const;
	uint32 NumTris() const { return m_aIndices.size() / 3; }

public:

	TIndexList m_aIndices;
	TVertList m_aVertices;
};

#endif //__PREPOLYFRAGMENTS_H__