//////////////////////////////////////////////////////////////////////////////
// CPrePolyFragments implementation

#include "bdefs.h"

#include "prepolyfragments.h"

CPrePolyFragments::CPrePolyFragments()
{
}

CPrePolyFragments::~CPrePolyFragments()
{
}

bool CPrePolyFragments::GetTri(uint32 nTriIndex, SVert *pPt0, SVert *pPt1, SVert *pPt2) const
{
	uint32 nIndex = nTriIndex * 3;
	if (m_aIndices.size() <= (nIndex + 2))
		return false;

	ASSERT(m_aIndices[nIndex] < m_aVertices.size());
	*pPt0 = m_aVertices[m_aIndices[nIndex]];
	ASSERT(m_aIndices[nIndex + 1] < m_aVertices.size());
	*pPt1 = m_aVertices[m_aIndices[nIndex + 1]];
	ASSERT(m_aIndices[nIndex + 2] < m_aVertices.size());
	*pPt2 = m_aVertices[m_aIndices[nIndex + 2]];

	return true;
}

