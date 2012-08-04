#include "bdefs.h"

#include "pcrendertreenode.h"
#include "pcrendertri.h"
#include "pcrendershaders.h"
#include "pcrendertools.h"
#include "pcfileio.h"
#include "pclmpacker.h"

#include "preworld.h"
#include "prepolyfragments.h"

#include "nvtristrip/nvtristrip.h"

#include "lightmapdefs.h"
#include "processing.h"

#include <algorithm>
#include <stack>
#include <float.h>	//for FLT_MIN and FLT_MAX

//////////////////////////////////////////////////////////////////////////////
// CPCRenderTreeNode_LGLightmap implementation details

struct LGSubLightmap
{
	LGSubLightmap(const CPreLightMap *pLightmap, const RECT &rArea, const CPreLightMap *pPageLightmap);

	bool Write(CAbstractIO &cFile);

	RECT m_rArea;
	typedef std::vector<uint8> TDataList;
	TDataList m_aData;

private:
	void CountDataRun(const TDataList::const_iterator &iStart, const TDataList::const_iterator &iEnd, uint8 *pSize, TDataList::const_iterator *pEndOfRun) const;
};

LGSubLightmap::LGSubLightmap(const CPreLightMap *pLightmap, const RECT &rArea, const CPreLightMap *pPageLightmap) :
	m_rArea(rArea)
{
	// First off, decompress the lightmap
	LM_DATA aDecompBuffer[LIGHTMAP_MAX_DATA_SIZE];
	pLightmap->Decompress(aDecompBuffer);

	ASSERT((uint32)(m_rArea.right - m_rArea.left) == pLightmap->m_Width);
	ASSERT((uint32)(m_rArea.bottom - m_rArea.top) == pLightmap->m_Height);

	// Now convert it into a buffer large enough to handle the expanded edges
	// Note that converting just involves stripping off the G & B channels, since
	// earlier processes already converted it to a monochromatic image
	TDataList aTempBuffer;
	uint32 nTempStride = pLightmap->m_Width + 2;
	aTempBuffer.resize(nTempStride * (pLightmap->m_Height + 2));

	TDataList::iterator iTempOut = aTempBuffer.begin() + (nTempStride + 1);
	LM_DATA *pDecompIn = aDecompBuffer;
	for (uint32 nTempY = 0; nTempY < pLightmap->m_Height; ++nTempY)
	{
		for (uint32 nTempX = 0; nTempX < pLightmap->m_Width; ++nTempX, ++iTempOut, pDecompIn += 3)
		{
			*iTempOut = *pDecompIn;
		}

		iTempOut += 2;
	}

	// Expand the edges
	uint32 nBottomRowOffset = nTempStride * (pLightmap->m_Height + 1);
	for (uint32 nExpandX = 0; nExpandX < pLightmap->m_Width; ++nExpandX)
	{
		aTempBuffer[nExpandX + 1] = aTempBuffer[nExpandX + nTempStride + 1];
		aTempBuffer[nExpandX + nBottomRowOffset + 1] = aTempBuffer[(nExpandX + nBottomRowOffset + 1) - nTempStride];
	}
	for (uint32 nExpandY = 0; nExpandY < pLightmap->m_Height; ++nExpandY)
	{
		aTempBuffer[nTempStride * (nExpandY + 1)] = aTempBuffer[nTempStride * (nExpandY + 1) + 1];
		aTempBuffer[nTempStride * (nExpandY + 2) - 1] = aTempBuffer[nTempStride * (nExpandY + 2) - 2];
	}

	// Blend the corners
	aTempBuffer[0] = (aTempBuffer[1] + aTempBuffer[nTempStride]) / 2;
	aTempBuffer[nTempStride - 1] = (aTempBuffer[nTempStride - 2] + aTempBuffer[nTempStride * 2 - 1]) / 2;
	aTempBuffer[nBottomRowOffset] = (aTempBuffer[nBottomRowOffset - nTempStride] + aTempBuffer[nBottomRowOffset + 1]) / 2;
	aTempBuffer[nBottomRowOffset + (nTempStride - 1)] = (aTempBuffer[nBottomRowOffset - 1] + aTempBuffer[nBottomRowOffset + (nTempStride - 2)]) / 2;
	
	// Expand the rectangle
	--m_rArea.top;
	--m_rArea.left;
	++m_rArea.bottom;
	++m_rArea.right;

	// Set up for a sub-rect copy
	TDataList::iterator iTopLeft = aTempBuffer.begin();
	uint32 nCopyWidth = nTempStride;
	uint32 nCopyHeight = pLightmap->m_Height + 2;

	bool bFullCopy = true;

	// Trim the edges
	if (rArea.top == 0)
	{
		iTopLeft += nTempStride;
		m_rArea.top = rArea.top;
		--nCopyHeight;
	}
	if (rArea.left == 0)
	{
		++iTopLeft;
		m_rArea.left = rArea.left;
		--nCopyWidth;
		bFullCopy = false;
	}
	if ((uint32)m_rArea.bottom > pPageLightmap->m_Height)
	{
		--nCopyHeight;
		--m_rArea.bottom;
	}
	if ((uint32)m_rArea.right > pPageLightmap->m_Width)
	{
		--nCopyWidth;
		--m_rArea.right;
		bFullCopy = false;
	}

	// Resize the destination buffer
	m_aData.resize(nCopyWidth * nCopyHeight);

	// Can we do it in one contiguous chunk?
	if (bFullCopy)
		memcpy( &(*(m_aData.begin())), &(*(iTopLeft)), nCopyWidth * nCopyHeight);
	else
	{
		// Copy it one line at a time..  :(
		TDataList::iterator iDataOut = m_aData.begin();
		for (; nCopyHeight; --nCopyHeight, iTopLeft += nTempStride, iDataOut += nCopyWidth)
		{
			memcpy( &(*(iDataOut)), &(*(iTopLeft)), nCopyWidth);
		}
		ASSERT(iDataOut == m_aData.end());
	}
}

void LGSubLightmap::CountDataRun(
	const TDataList::const_iterator &iStart, 
	const TDataList::const_iterator &iEnd, 
	uint8 *pSize, 
	TDataList::const_iterator *pEndOfRun) const
{
	*pSize = 0;
	*pEndOfRun = iStart + 1;
	while ((*pSize < 255) && (*pEndOfRun != iEnd) && (**pEndOfRun == *iStart))
	{
		++(*pEndOfRun);
		++(*pSize);
	}
}

bool LGSubLightmap::Write(CAbstractIO &cFile)
{
	// Write out the rectangle
	cFile << (uint32)m_rArea.left;
	cFile << (uint32)m_rArea.top;
	cFile << (uint32)(m_rArea.right - m_rArea.left);
	cFile << (uint32)(m_rArea.bottom - m_rArea.top);

	// Write out the RLE encoded sub-lightmap data
	uint32 nDataSizePos = cFile.GetCurPos();
	cFile << (uint32)0;

	TDataList::const_iterator iCurData = m_aData.begin();
	while (iCurData != m_aData.end())
	{
		// Get the next duplicate count
		uint8 nRunCount;
		TDataList::const_iterator iEndOfRun;
		CountDataRun(iCurData, m_aData.end(), &nRunCount, &iEndOfRun);
		// Write a duplicate tag, if there were enough duplicates
		if (nRunCount > 3)
		{
			cFile << (uint8)0xFF;
			cFile << nRunCount;
			cFile << *iCurData;

			iCurData = iEndOfRun;
		}
		else
		{
			// Handle the special case of a non-duplicated duplicate tag character
			if (*iCurData == 0xFF)
			{
				cFile << (uint8)0xFF;
				cFile << (uint8)0;
				cFile << (uint8)0;
			}
			// Just write out the character
			else
			{
				cFile << *iCurData;
			}

			++iCurData;
		}
	}

	// Write the data size
	uint32 nEndDataPos = cFile.GetCurPos();
	cFile.SeekTo(nDataSizePos);
	cFile << (uint32)(nEndDataPos - (nDataSizePos + sizeof(nDataSizePos)));
	cFile.SeekTo(nEndDataPos);

	return true;
}

class CPCRenderTreeNode_LGLightmap
{
public:
	CPCRenderTreeNode_LGLightmap() {}
	~CPCRenderTreeNode_LGLightmap();
	CPCRenderTreeNode_LGLightmap(const CPCRenderTreeNode_LGLightmap &cOther);
	CPCRenderTreeNode_LGLightmap &operator=(const CPCRenderTreeNode_LGLightmap &cOther);

	typedef std::vector<LGSubLightmap*> TSubLMList;
	TSubLMList m_aSubLMs;
};

CPCRenderTreeNode_LGLightmap::~CPCRenderTreeNode_LGLightmap()
{
	while (!m_aSubLMs.empty())
	{
		delete m_aSubLMs.back();
		m_aSubLMs.pop_back();
	}
}

CPCRenderTreeNode_LGLightmap::CPCRenderTreeNode_LGLightmap(const CPCRenderTreeNode_LGLightmap &cOther)
{
	m_aSubLMs.reserve(cOther.m_aSubLMs.size());
	TSubLMList::const_iterator iCurLM = cOther.m_aSubLMs.begin();
	for (; iCurLM != cOther.m_aSubLMs.end(); ++iCurLM)
	{
		if (*iCurLM)
			m_aSubLMs.push_back(new LGSubLightmap(**iCurLM));
		else
			m_aSubLMs.push_back(0);
	}
}

CPCRenderTreeNode_LGLightmap &CPCRenderTreeNode_LGLightmap::operator=(const CPCRenderTreeNode_LGLightmap &cOther)
{
	if (&cOther == this)
		return *this;

	while (!m_aSubLMs.empty())
	{
		delete m_aSubLMs.back();
		m_aSubLMs.pop_back();
	}

	m_aSubLMs.reserve(cOther.m_aSubLMs.size());
	TSubLMList::const_iterator iCurLM = cOther.m_aSubLMs.begin();
	for (; iCurLM != cOther.m_aSubLMs.end(); ++iCurLM)
	{
		if (*iCurLM)
			m_aSubLMs.push_back(new LGSubLightmap(**iCurLM));
		else
			m_aSubLMs.push_back(0);
	}

	return *this;
}


//////////////////////////////////////////////////////////////////////////////
// CPCRenderTreeNode::COptData implementation

struct CPCRenderTreeNode::COptData
{
	COptData(CPCRenderTreeNode *pNode);

	LTVector m_vMin, m_vInvDims;
	
	void Clear();

	uint32 GetVectorBucket(const LTVector &vPos);
	uint32 FindVertIndex(const CPCRenderVert2T &cVert);

	typedef std::vector<uint32> TIndexList;
	enum { k_NumHashBuckets = (1 << 3 * 3) };
	TIndexList m_aVectorBuckets[k_NumHashBuckets];

	CPCRenderTreeNode *m_pNode;
};

CPCRenderTreeNode::COptData::COptData(CPCRenderTreeNode *pNode) :
	m_pNode(pNode)
{
	m_vMin = m_pNode->m_vCenter - m_pNode->m_vHalfDims;
	m_vInvDims.Init(
		7.0f / (m_pNode->m_vHalfDims.x * 2.0f), 
		7.0f / (m_pNode->m_vHalfDims.y * 2.0f), 
		7.0f / (m_pNode->m_vHalfDims.z * 2.0f));
}

void CPCRenderTreeNode::COptData::Clear()
{
	for (uint32 nClearLoop = 0; nClearLoop < k_NumHashBuckets; ++nClearLoop)
		m_aVectorBuckets[nClearLoop].clear();
}

uint32 CPCRenderTreeNode::COptData::GetVectorBucket(const LTVector &vPos)
{
	LTVector vAdjustedPos = (vPos - m_vMin) * m_vInvDims;
	uint32 nBucketX = (uint32)(vAdjustedPos.x + 0.001f);
	uint32 nBucketY = (uint32)(vAdjustedPos.y + 0.001f);
	uint32 nBucketZ = (uint32)(vAdjustedPos.z + 0.001f);
	uint32 nResult = nBucketX | (nBucketY << 3) | (nBucketZ << 6);
	ASSERT(nResult == (nBucketX ^ (nBucketY << 3) ^ (nBucketZ << 6)));
	return nResult % k_NumHashBuckets;
}

uint32 CPCRenderTreeNode::COptData::FindVertIndex(const CPCRenderVert2T &cVert)
{
	uint32 nVecHash = GetVectorBucket(cVert.m_vPos);
	TIndexList &vVecList = m_aVectorBuckets[nVecHash];
	for (uint32 nCurIndex = 0; nCurIndex < vVecList.size(); ++nCurIndex)
	{
		uint32 nVertIndex = vVecList[nCurIndex];
		if (cVert == m_pNode->m_aVertices[nVertIndex])
			return nVertIndex;
	}
	vVecList.push_back(m_pNode->m_aVertices.size());
	m_pNode->m_aVertices.push_back(cVert);
	return m_pNode->m_aVertices.size() - 1;
}

//////////////////////////////////////////////////////////////////////////////
// CPCRenderTreeNode::CLightGroup implementation

CPCRenderTreeNode::CLightGroup::CLightGroup(const CPCRenderTreeNode::CLightGroup &cOther) :
	m_sName(cOther.m_sName),
	m_vColor(cOther.m_vColor),
	m_aVertexIntensities(cOther.m_aVertexIntensities)
{
	m_aSectionLightmaps.reserve(cOther.m_aSectionLightmaps.size());
	TLightmapList::const_iterator iCurLM = cOther.m_aSectionLightmaps.begin();
	for (; iCurLM != cOther.m_aSectionLightmaps.end(); ++iCurLM)
	{
		if (*iCurLM)
			m_aSectionLightmaps.push_back(new CPCRenderTreeNode_LGLightmap(**iCurLM));
		else
			m_aSectionLightmaps.push_back(0);
	}
}

CPCRenderTreeNode::CLightGroup::~CLightGroup()
{
	while (!m_aSectionLightmaps.empty())
	{
		delete m_aSectionLightmaps.back();
		m_aSectionLightmaps.pop_back();
	}
}

CPCRenderTreeNode::CLightGroup &CPCRenderTreeNode::CLightGroup::operator=(const CPCRenderTreeNode::CLightGroup &cOther)
{
	if (&cOther == this)
		return *this;

	m_sName = cOther.m_sName;
	m_vColor = cOther.m_vColor;
	m_aVertexIntensities = cOther.m_aVertexIntensities;

	while (!m_aSectionLightmaps.empty())
	{
		delete m_aSectionLightmaps.back();
		m_aSectionLightmaps.pop_back();
	}

	m_aSectionLightmaps.reserve(cOther.m_aSectionLightmaps.size());
	TLightmapList::const_iterator iCurLM = cOther.m_aSectionLightmaps.begin();
	for (; iCurLM != cOther.m_aSectionLightmaps.end(); ++iCurLM)
	{
		if (*iCurLM)
			m_aSectionLightmaps.push_back(new CPCRenderTreeNode_LGLightmap(**iCurLM));
		else
			m_aSectionLightmaps.push_back(0);
	}

	return *this;
}

//////////////////////////////////////////////////////////////////////////////
// CPCRenderTreeNode implementation

CPCRenderTreeNode::CPCRenderTreeNode(const CPCRenderTree *pTree) : m_pParent(0), m_pTree(pTree)
{
	memset(m_aChildren, 0, sizeof(m_aChildren));
}

CPCRenderTreeNode::~CPCRenderTreeNode()
{
	DeleteChildren();
}

void CPCRenderTreeNode::DeleteChildren()
{
	for (uint32 nDeleteLoop = 0; nDeleteLoop < k_NumChildren; ++nDeleteLoop)
	{
		delete m_aChildren[nDeleteLoop];
	}
	memset(m_aChildren, 0, sizeof(m_aChildren));
}

CPCRenderTreeNode *CPCRenderTreeNode::Clone(const CPCRenderTree *pTree, CPCRenderTreeNode *pParent) const
{
	// Create a new node
	CPCRenderTreeNode *pResult = new CPCRenderTreeNode(pTree);
	// Copy...
	*pResult = *this;
	// Hook it back up
	pResult->m_pTree = pTree;
	pResult->m_pParent = pParent;
	// Clone the children
	for (uint32 nCurChild = 0; nCurChild < k_NumChildren; ++nCurChild)
	{
		if (m_aChildren[nCurChild])
			pResult->m_aChildren[nCurChild] = m_aChildren[nCurChild]->Clone(pTree, pResult);
	}
	return pResult;
}

uint32 CPCRenderTreeNode::CountSection(const CPrePoly *pPoly, EPCShaderType eShader, uint32 nCount)
{
	const char *pTextureName0 = NULL;
	const char *pTextureName1 = NULL;
	// Use the lightmap animation name for lightmap shaders
	if (IsPCShaderLightmap(eShader) && m_pCurLMAnim)
		pTextureName0 = m_pCurLMAnim->m_Name;
	else
	{
		if(pPoly->GetSurface()->m_Texture[0].IsValid())
			pTextureName0 = pPoly->GetSurface()->m_Texture[0].m_pTextureName;
		else
			int nBreakMe = 1;

		if(pPoly->GetSurface()->m_Texture[1].IsValid())
			pTextureName1 = pPoly->GetSurface()->m_Texture[1].m_pTextureName;
	}

	// Build a section for this poly
	const char* pEffect = pPoly->GetSurface()->m_pTextureEffect;
	CSection cCurSection(pTextureName0, pTextureName1, eShader, nCount, m_pCurLMAnim, pEffect);
	// Look for the Section already being in the list
	for (uint32 nSearch = 0; nSearch < m_aSections.size(); ++nSearch)
	{
		if (m_aSections[nSearch] == cCurSection)
		{
			// Found it!  Add the count
			m_aSections[nSearch].m_nCount += nCount;
			return nSearch;
		}
	}
	// It's a new Section, add it to the end of the list
	m_aSections.push_back(cCurSection);
	return m_aSections.size() - 1;
}

void CPCRenderTreeNode::UpdateSectionList()
{
	// Clear out the Section list counts
	for (uint32 nClearLoop = 0; nClearLoop < m_aSections.size(); ++nClearLoop)
		m_aSections[nClearLoop].m_nCount = 0;

	// Run through the triangles and count the Sections
	for (uint32 nTriLoop = 0; nTriLoop < m_aTris.size(); ++nTriLoop)
	{
		m_aSections[m_aTris[nTriLoop].m_nSection].m_nCount += 1;
	}

	// Get the number of previous empty sections for each section
	std::vector<uint32> aOffsets;
	aOffsets.resize(m_aSections.size(), 0);
	uint32 nOffset = 0;
	for (uint32 nOffsetSectionLoop = 0; nOffsetSectionLoop < m_aSections.size(); ++nOffsetSectionLoop)
	{
		aOffsets[nOffsetSectionLoop] = nOffset;
		if (m_aSections[nOffsetSectionLoop].m_nCount == 0)
			++nOffset;
	}

	// Adjust the triangle section indices
	for (uint32 nOffsetTriLoop = 0; nOffsetTriLoop < m_aTris.size(); ++nOffsetTriLoop)
	{
		CTri &cCurTri = m_aTris[nOffsetTriLoop];
		cCurTri.m_nSection -= aOffsets[cCurTri.m_nSection];
	}

	// Delete the empty sections
	TSectionList::iterator iCurSection = m_aSections.begin();
	while (iCurSection != m_aSections.end())
	{
		if ((*iCurSection).m_nCount == 0)
			iCurSection = m_aSections.erase(iCurSection);
		else
		{
			CSection &cCurSection = *iCurSection;
			cCurSection.m_nOriginalIndex -= aOffsets[cCurSection.m_nOriginalIndex];
			++iCurSection;
		}
	}
}

void CPCRenderTreeNode::UpdateBounds()
{
	if (!m_aTris.size())
		return;

	LTVector vNewMin = m_vCenter + m_vHalfDims;
	LTVector vNewMax = m_vCenter - m_vHalfDims;

	TTriList::iterator iCurTri = m_aTris.begin();
	for (; iCurTri != m_aTris.end(); ++iCurTri)
	{
		CTri &cCurTri = *iCurTri;
		VEC_MIN(vNewMin, vNewMin, cCurTri.m_Vert0.m_vPos);
		VEC_MAX(vNewMax, vNewMax, cCurTri.m_Vert0.m_vPos);
		VEC_MIN(vNewMin, vNewMin, cCurTri.m_Vert1.m_vPos);
		VEC_MAX(vNewMax, vNewMax, cCurTri.m_Vert1.m_vPos);
		VEC_MIN(vNewMin, vNewMin, cCurTri.m_Vert2.m_vPos);
		VEC_MAX(vNewMax, vNewMax, cCurTri.m_Vert2.m_vPos);
	}

	// Don't include occluders in the bounds
	/*
	TOccluderPolyList::iterator iCurPoly = m_aOccluders.begin();
	for (; iCurPoly != m_aOccluders.end(); ++iCurPoly)
	{
		CRawPoly &cCurPoly = *iCurPoly;
		VEC_MIN(vNewMin, vNewMin, cCurPoly.m_vMin);
		VEC_MAX(vNewMax, vNewMax, cCurPoly.m_vMax);
	}
	*/

	TRawPolyList::iterator iCurPoly = m_aSkyPortals.begin();
	for (; iCurPoly != m_aSkyPortals.end(); ++iCurPoly)
	{
		CRawPoly &cCurPoly = *iCurPoly;
		VEC_MIN(vNewMin, vNewMin, cCurPoly.m_vMin);
		VEC_MAX(vNewMax, vNewMax, cCurPoly.m_vMax);
	}

	ASSERT(
		(vNewMin.x >= (m_vCenter.x - (m_vHalfDims.x + 0.01f))) &&
		(vNewMin.y >= (m_vCenter.y - (m_vHalfDims.y + 0.01f))) &&
		(vNewMin.z >= (m_vCenter.z - (m_vHalfDims.z + 0.01f))) &&
		(vNewMax.x <= (m_vCenter.x + (m_vHalfDims.x + 0.01f))) &&
		(vNewMax.y <= (m_vCenter.y + (m_vHalfDims.y + 0.01f))) &&
		(vNewMax.z <= (m_vCenter.z + (m_vHalfDims.z + 0.01f)))
	);

	m_vCenter = (vNewMax + vNewMin) * 0.5f;
	m_vHalfDims = vNewMax - m_vCenter;
}

bool CPCRenderTreeNode::FixTJuncOnTri(const CTri &cMainTri, const LTVector &vSplitPt, CTri *pLeftResult, CTri *pRightResult)
{
	const float k_fZeroEdgeSqrEpsilon = 0.001f;
	const float k_fEndPtEpsilon = 0.1f;
	const float k_fOnEdgeSqrEpsilon = 0.01f;

	// Jump out if the vertex is already in the triangle
	if (cMainTri.Vert(0).m_vPos.NearlyEquals(vSplitPt, k_fEndPtEpsilon) || 
		cMainTri.Vert(1).m_vPos.NearlyEquals(vSplitPt, k_fEndPtEpsilon) || 
		cMainTri.Vert(2).m_vPos.NearlyEquals(vSplitPt, k_fEndPtEpsilon))
		return false;

	for (uint32 nCurEdge = 0; nCurEdge < 3; ++nCurEdge)
	{
		// Don't do T-Junction fixing on non-exterior edges
		if (!cMainTri.IsExteriorEdge(nCurEdge))
			continue;

		uint32 nStartIndex = nCurEdge;
		uint32 nEndIndex = (nCurEdge + 1) % 3;
		const CPCRenderVert3T &vEdgeStart = cMainTri.Vert(nStartIndex);
		const CPCRenderVert3T &vEdgeEnd = cMainTri.Vert(nEndIndex);

		LTVector vEdgeDir = vEdgeEnd.m_vPos - vEdgeStart.m_vPos;
		float fEdgeMag = vEdgeDir.MagSqr();
		if (fEdgeMag < k_fZeroEdgeSqrEpsilon)
			continue;
		fEdgeMag = sqrtf(fEdgeMag);
		vEdgeDir *= 1.0f / fEdgeMag;

		// Is this point within the range of the edge?
		LTVector vPtOfs = vSplitPt - vEdgeStart.m_vPos;

		float fT = vPtOfs.Dot(vEdgeDir);
		if ((fT < k_fEndPtEpsilon) || (fT > (fEdgeMag - k_fEndPtEpsilon)))
			continue;

		// Is it actually touching the edge?
		float fLineDistSqr = (vSplitPt - (vEdgeStart.m_vPos + vEdgeDir * fT)).MagSqr();
		if (fLineDistSqr > k_fOnEdgeSqrEpsilon)
			continue;

		// Normalize fT
		fT /= fEdgeMag;

		// Get the end points of the full edge
		LTVector vFullEdgeStart, vFullEdgeEnd;

		if (cMainTri.m_pTJunc[nStartIndex])
		{
			float fEdgeSwapDot = vEdgeDir.Dot(cMainTri.m_pTJunc[nStartIndex]->m_vEnd - cMainTri.m_pTJunc[nStartIndex]->m_vStart);
			if (fEdgeSwapDot > 0.0f)
				vFullEdgeStart = cMainTri.m_pTJunc[nStartIndex]->m_vStart;
			else
				vFullEdgeStart = cMainTri.m_pTJunc[nStartIndex]->m_vEnd;
		}
		else
			vFullEdgeStart = vEdgeStart.m_vPos;

		if (cMainTri.m_pTJunc[nEndIndex])
		{
			float fEdgeSwapDot = vEdgeDir.Dot(cMainTri.m_pTJunc[nEndIndex]->m_vEnd - cMainTri.m_pTJunc[nEndIndex]->m_vStart);
			if (fEdgeSwapDot > 0.0f)
				vFullEdgeEnd = cMainTri.m_pTJunc[nEndIndex]->m_vEnd;
			else
				vFullEdgeEnd = cMainTri.m_pTJunc[nEndIndex]->m_vStart;
		}
		else
			vFullEdgeEnd = vEdgeEnd.m_vPos;

		// Set up the "left" result
		*pLeftResult = cMainTri;
		if (!pLeftResult->m_pTJunc[nEndIndex])
			pLeftResult->m_pTJunc[nEndIndex] = new CTriTJunc(vFullEdgeStart, vFullEdgeEnd);
		pLeftResult->Vert(nEndIndex) = CPCRenderVert3T::Lerp(vEdgeStart, vEdgeEnd, fT);
		pLeftResult->Vert(nEndIndex).m_vPos = vSplitPt;
		pLeftResult->SetExteriorEdge(nEndIndex, false);

		// Set up the "right" result
		*pRightResult = cMainTri;
		if (!pRightResult->m_pTJunc[nStartIndex])
			pRightResult->m_pTJunc[nStartIndex] = new CTriTJunc(vFullEdgeStart, vFullEdgeEnd);
		pRightResult->Vert(nStartIndex) = pLeftResult->Vert(nEndIndex);
		uint32 nOtherIndex = 3 - (nStartIndex + nEndIndex);
		pRightResult->SetExteriorEdge(nOtherIndex, false);

		// We split, so jump out
		return true;
	}

	return false;
}

// 3-D range class (aka YAAABB) for the vector octree
struct SVectorRange
{
	SVectorRange() : m_vMin(FLT_MAX, FLT_MAX, FLT_MAX), m_vMax(-FLT_MAX, -FLT_MAX, -FLT_MAX) {}
	SVectorRange(const SVectorRange &sOther) : m_vMin(sOther.m_vMin), m_vMax(sOther.m_vMax) {}
	SVectorRange(const LTVector &vMin, const LTVector &vMax) : m_vMin(vMin), m_vMax(vMax) {}
	SVectorRange &operator=(const SVectorRange &sOther) {
		m_vMin = sOther.m_vMin;
		m_vMax = sOther.m_vMax;
		return *this;
	}

	bool Overlaps(const SVectorRange &sOther) const 
	{
		return (m_vMin.x <= sOther.m_vMax.x) && (m_vMin.y <= sOther.m_vMax.y) && (m_vMin.z <= sOther.m_vMax.z) &&
			(m_vMax.x >= sOther.m_vMin.x) && (m_vMax.y >= sOther.m_vMin.y) && (m_vMax.z >= sOther.m_vMin.z);
	}
	bool Contains(const LTVector &vPos) const 
	{
		return (m_vMin.x <= vPos.x) && (m_vMin.y <= vPos.y) && (m_vMin.z <= vPos.z) && 
			(m_vMax.x >= vPos.x) && (m_vMax.y >= vPos.y) && (m_vMax.z >= vPos.z);
	}
	void Expand(const SVectorRange &sOther)
	{
		VEC_MIN(m_vMin, m_vMin, sOther.m_vMin);
		VEC_MAX(m_vMax, m_vMax, sOther.m_vMax);
	}
	void Expand(const LTVector &vPos)
	{
		VEC_MIN(m_vMin, m_vMin, vPos);
		VEC_MAX(m_vMax, m_vMax, vPos);
	}
	void Expand(float fValue)
	{
		m_vMin.x -= fValue;
		m_vMin.y -= fValue;
		m_vMin.z -= fValue;
		m_vMax.x += fValue;
		m_vMax.y += fValue;
		m_vMax.z += fValue;
	}
	SVectorRange Intersection(const SVectorRange &sOther) const {
		SVectorRange sResult;
		VEC_MAX(sResult.m_vMin, m_vMin, sOther.m_vMin);
		VEC_MIN(sResult.m_vMax, m_vMax, sOther.m_vMax);
		return sResult;
	}
public:
	LTVector m_vMin, m_vMax;
};

// Octree node for spatially sorting vectors.  Used by the T-Junction fixing for reducing the order of the searches
class CVectorOctreeNode
{
public:
	CVectorOctreeNode() : 
		m_pChildren(0),
		m_pVertices(0),
		m_nNumVerts(0)
	{
	}
	~CVectorOctreeNode() {
		delete[] m_pChildren;
		delete[] m_pVertices;
	}

	// Add a vector to the node, or its children
	void AddVector(const SVectorRange &sNodeRange, const LTVector &vPos);
	// Find the vector closest to a position, within a maximum distance
	bool FindClosestVector(const SVectorRange &sNodeRange, const LTVector &vPos, float fMaxDist, LTVector *pResult);
	// Find the set of vectors inside of a range
	typedef std::vector<LTVector> TVectorList;
	bool FindVectorsInRange(const SVectorRange &sNodeRange, const SVectorRange &sFindRange, TVectorList *pResults);
private:

	// This node's vertices are full.  Split this node into children.
	void Split(const SVectorRange &sNodeRange);

	// Internal adaptor function for FindClosestVector to avoid re-calculating things per-node
	bool FindClosestVector_I(const SVectorRange &sNodeRange, const LTVector &vPos, SVectorRange *pPosRange, float *pMaxDistSqr, LTVector *pResult);

	// Get the range for a child
	void GetChildRange(uint32 nChildIndex, const SVectorRange &sParent, SVectorRange *pChild);

	// This node's children (exclusive with m_pVertices)
	CVectorOctreeNode *m_pChildren;
	// This node's vertices (exclusive with m_pChildren)
	LTVector *m_pVertices;
	enum { k_nVerticesPerLeaf = 64 };

	// Current number of vertices in the node
	uint32 m_nNumVerts;
};

void CVectorOctreeNode::AddVector(const SVectorRange &sNodeRange, const LTVector &vPos)
{
	// If we're full, split
	if (m_nNumVerts == k_nVerticesPerLeaf)
		Split(sNodeRange);

	// If we're not a leaf node, add it to the containing child
	if (m_pChildren)
	{
		SVectorRange sChildRange;
		for (uint32 nChild = 0; nChild < 8; ++nChild)
		{
			GetChildRange(nChild, sNodeRange, &sChildRange);
			if (sChildRange.Contains(vPos))
			{
				m_pChildren[nChild].AddVector(sChildRange, vPos);
				return;
			}
		}
	}
	else
	{
		// Allocate our vertex array if we need to
		if (!m_pVertices)
			m_pVertices = new LTVector[k_nVerticesPerLeaf];
		// Add the vector to the list
		m_pVertices[m_nNumVerts++] = vPos;
	}
}

bool CVectorOctreeNode::FindClosestVector(const SVectorRange &sNodeRange, const LTVector &vPos, float fMaxDist, LTVector *pResult)
{
	// Pre-calculate the range of the position
	SVectorRange sPosRange(vPos - LTVector(fMaxDist, fMaxDist, fMaxDist), vPos + LTVector(fMaxDist, fMaxDist, fMaxDist));
	// Pre-calculate the distance squared
	float fMaxDistSqr = fMaxDist * fMaxDist;
	// Do the work
	return FindClosestVector_I(sNodeRange, vPos, &sPosRange, &fMaxDistSqr, pResult);
}

bool CVectorOctreeNode::FindClosestVector_I(const SVectorRange &sNodeRange, const LTVector &vPos, SVectorRange *pPosRange, float *pMaxDistSqr, LTVector *pResult)
{
	bool bResult = false;

	// If we're a non-leaf node, hand this call off to our children
	if (m_pChildren)
	{
		SVectorRange sChildRange;
		for (uint32 nChild = 0; nChild < 8; ++nChild)
		{
			GetChildRange(nChild, sNodeRange, &sChildRange);
			if (sChildRange.Overlaps(*pPosRange))
			{
				bResult |= m_pChildren[nChild].FindClosestVector_I(sChildRange, vPos, pPosRange, pMaxDistSqr, pResult);
			}
		}
	}
	// Find the closest vertex
	else if (m_pVertices)
	{
		for (uint32 nCurVert = 0; nCurVert < m_nNumVerts; ++nCurVert)
		{
			float fDistSqr = vPos.DistSqr(m_pVertices[nCurVert]);
			if (fDistSqr < *pMaxDistSqr)
			{
				*pMaxDistSqr = fDistSqr;
				*pResult = m_pVertices[nCurVert];
				bResult = true;
			}
		}

		if (bResult)
		{
			float fMaxDist = sqrtf(*pMaxDistSqr);
			*pPosRange = SVectorRange(vPos - LTVector(fMaxDist, fMaxDist, fMaxDist), vPos + LTVector(fMaxDist, fMaxDist, fMaxDist));
		}
	}

	return bResult;
}

bool CVectorOctreeNode::FindVectorsInRange(const SVectorRange &sNodeRange, const SVectorRange &sFindRange, TVectorList *pResults)
{
	bool bResult = false;

	// If we're a non-leaf node, hand this call off to our children
	if (m_pChildren)
	{
		SVectorRange sChildRange;
		for (uint32 nChild = 0; nChild < 8; ++nChild)
		{
			GetChildRange(nChild, sNodeRange, &sChildRange);
			if (sChildRange.Overlaps(sFindRange))
			{
				bResult |= m_pChildren[nChild].FindVectorsInRange(sChildRange, sFindRange, pResults);
			}
		}
	}
	// Find the vectors in the provided range
	else if (m_pVertices)
	{
		for (uint32 nCurVert = 0; nCurVert < m_nNumVerts; ++nCurVert)
		{
			if (sFindRange.Contains(m_pVertices[nCurVert]))
			{
				pResults->push_back(m_pVertices[nCurVert]);
				bResult = true;
			}
		}
	}

	return bResult;
}

void CVectorOctreeNode::Split(const SVectorRange &sNodeRange)
{
	// Make sure we're ripe for splitting
	if (m_pChildren)
	{
		ASSERT(!"Split called on non-leaf octree node!");
		return;
	}

	if (!m_pVertices)
	{
		ASSERT(!"Split called on empty octree node!");
		return;
	}

	// Allocate the children
	m_pChildren = new CVectorOctreeNode[8];

	// Pre-calculate the ranges of the children
	SVectorRange aChildRanges[8];

	for (uint32 nChild = 0; nChild < 8; ++nChild)
	{
		GetChildRange(nChild, sNodeRange, &aChildRanges[nChild]);
	}

	// Filter our vertices into the children
	for (uint32 nCurVert = 0; nCurVert < m_nNumVerts; ++nCurVert)
	{
		for (uint32 nChild = 0; nChild < 8; ++nChild)
		{
			if (aChildRanges[nChild].Contains(m_pVertices[nCurVert]))
			{
				m_pChildren[nChild].AddVector(aChildRanges[nChild], m_pVertices[nCurVert]);
				break;
			}
		}

		ASSERT((nChild < 8) && "Vector could not be filtered into children");
	}

	// Get rid of our vertices
	delete[] m_pVertices;
	m_pVertices = 0;
	m_nNumVerts = 0;
}

void CVectorOctreeNode::GetChildRange(uint32 nChildIndex, const SVectorRange &sParent, SVectorRange *pChild)
{
	LTVector vCenter = (sParent.m_vMin + sParent.m_vMax) * 0.5f;
	switch (nChildIndex)
	{
		case 0 :
			*pChild = SVectorRange(sParent.m_vMin, vCenter);
			break;
		case 1 :
			*pChild = SVectorRange(LTVector(vCenter.x, sParent.m_vMin.y, sParent.m_vMin.z), LTVector(sParent.m_vMax.x, vCenter.y, vCenter.z));
			break;
		case 2 :
			*pChild = SVectorRange(LTVector(sParent.m_vMin.x, sParent.m_vMin.y, vCenter.z), LTVector(vCenter.x, vCenter.y, sParent.m_vMax.z));
			break;
		case 3 :
			*pChild = SVectorRange(LTVector(vCenter.x, sParent.m_vMin.y, vCenter.z), LTVector(sParent.m_vMax.x, vCenter.y, sParent.m_vMax.z));
			break;
		case 4 :
			*pChild = SVectorRange(LTVector(sParent.m_vMin.x, vCenter.y, sParent.m_vMin.z), LTVector(vCenter.x, sParent.m_vMax.y, vCenter.z));
			break;
		case 5 :
			*pChild = SVectorRange(LTVector(vCenter.x, vCenter.y, sParent.m_vMin.z), LTVector(sParent.m_vMax.x, sParent.m_vMax.y, vCenter.z));
			break;
		case 6 :
			*pChild = SVectorRange(LTVector(sParent.m_vMin.x, vCenter.y, vCenter.z), LTVector(vCenter.x, sParent.m_vMax.y, sParent.m_vMax.z));
			break;
		case 7 :
			*pChild = SVectorRange(vCenter, sParent.m_vMax);
			break;
		default :
			ASSERT(!"Invalid octree child range requested");
			break;
	}
}


void CPCRenderTreeNode::FixTJunctions()
{
	// Any closer than this, and they're the same position
	const float k_fDuplicateVertexEpsilon = 0.1f;

	CVectorOctreeNode sVectorOctree;
	SVectorRange sOctreeRange;

	// Get the range for the vector octree
	TTriList::iterator iCurTri = m_aTris.begin();
	for (; iCurTri != m_aTris.end(); ++iCurTri)
	{
		for (uint32 nCurVert = 0; nCurVert < 3; ++nCurVert)
		{
			sOctreeRange.Expand(iCurTri->Vert(nCurVert).m_vPos);
		}
	}

	// Set up the vector octree & clean up the triangles
	iCurTri = m_aTris.begin();
	for (; iCurTri != m_aTris.end(); ++iCurTri)
	{
		// Remember which way the triangle was facing
		LTVector vOldFacing = (iCurTri->Vert(0).m_vPos - iCurTri->Vert(1).m_vPos).Cross((iCurTri->Vert(2).m_vPos - iCurTri->Vert(1).m_vPos));

		bool bSnapped = false;

		for (uint32 nCurVert = 0; nCurVert < 3; ++nCurVert)
		{
			LTVector vPos = iCurTri->Vert(nCurVert).m_vPos;
			LTVector vClosest;
			if (sVectorOctree.FindClosestVector(sOctreeRange, vPos, k_fDuplicateVertexEpsilon, &vClosest))
			{
				// Snap the vertex to the position if we found a close one
				iCurTri->Vert(nCurVert).m_vPos = vClosest;
				bSnapped = true;
			}
			else
				// Otherwise add it to the octree
				sVectorOctree.AddVector(sOctreeRange, vPos);
		}

		// Make sure we didn't reverse the facing on the triangle by snapping it
		if (bSnapped)
		{
			LTVector vNewFacing = (iCurTri->Vert(0).m_vPos - iCurTri->Vert(1).m_vPos).Cross((iCurTri->Vert(2).m_vPos - iCurTri->Vert(1).m_vPos));
			if (vNewFacing.Dot(vOldFacing) < 0.0f)
				iCurTri->Flip();
		}
	}

	// This is where the new list of triangles is going to end up
	TTriList aNewTriList;

	// Keep a stack of triangles that are currently being processed
	typedef std::stack<CTri> TTriStack;
	TTriStack aWaitingTris;

	TTriList::const_iterator iCurInputTri = m_aTris.begin();
	for (; iCurInputTri != m_aTris.end(); ++iCurInputTri)
	{
		// Check for triangles which got turned degenerate due to the vertex snapping
		if (iCurInputTri->Vert(0).m_vPos.NearlyEquals(iCurInputTri->Vert(1).m_vPos, k_fDuplicateVertexEpsilon) || 
			iCurInputTri->Vert(0).m_vPos.NearlyEquals(iCurInputTri->Vert(2).m_vPos, k_fDuplicateVertexEpsilon) || 
			iCurInputTri->Vert(2).m_vPos.NearlyEquals(iCurInputTri->Vert(1).m_vPos, k_fDuplicateVertexEpsilon))
			continue;

		// Put the current tri on the stack
		aWaitingTris.push(*iCurInputTri);

		while (!aWaitingTris.empty())
		{
			// Get a tri off the stack
			CTri cCurTri = aWaitingTris.top();
			aWaitingTris.pop();

			bool bFoundTJunc = false;

			SVectorRange sTriRange;
			sTriRange.Expand(cCurTri.m_Vert0.m_vPos);
			sTriRange.Expand(cCurTri.m_Vert1.m_vPos);
			sTriRange.Expand(cCurTri.m_Vert2.m_vPos);
			sTriRange.Expand(k_fDuplicateVertexEpsilon);

			static CVectorOctreeNode::TVectorList aCandidateVertices;
			aCandidateVertices.clear();

			if (sVectorOctree.FindVectorsInRange(sOctreeRange, sTriRange, &aCandidateVertices))
			{
				CTri cLeftTri, cRightTri;
				while (!aCandidateVertices.empty())
				{
					if (FixTJuncOnTri(cCurTri, aCandidateVertices.back(), &cLeftTri, &cRightTri))
					{
						// We found a triangle with a T-Junction on the end, so split it up
						// and put both sides on the "waiting" list
						bFoundTJunc = true;
						aWaitingTris.push(cLeftTri);
						aWaitingTris.push(cRightTri);
						break;
					}
					aCandidateVertices.pop_back();
				}
			}

			// If we didn't have to split this tri, put it on the new triangle list
			if (!bFoundTJunc)
				aNewTriList.push_back(cCurTri);
		}
	}

	// Use the new & improved triangle list
	m_aTris.swap(aNewTriList);

	// The section list is now out of whack, so fix it.
	UpdateSectionList();
}

void CPCRenderTreeNode::AddPoly(const CPrePoly *pPoly, bool bOverrideLightmap)
{
	EPCShaderType eShader = GetPCShaderType(pPoly);

	if (eShader == ePCShader_Lightmap)
	{
		if(bOverrideLightmap)
		{
			//see if this is a single or dual textured lightmapped polygon
			if(pPoly->GetSurface()->m_Texture[0].IsValid() && pPoly->GetSurface()->m_Texture[1].IsValid())
			{
				//dual textured
				eShader = ePCShader_Lightmap_DualTexture;
			}
			else
			{
				//single textured
				eShader = ePCShader_Lightmap_Texture;
			}
		}
	}

	// Don't add invisible polys
	if (eShader == ePCShader_None)
		return;
	// Put sky portals into the sky portal list
	if (eShader == ePCShader_SkyPortal)
	{
		m_aSkyPortals.push_back(CRawPoly(pPoly));
		return;
	}
	// Put occluders into the occluder list
	if (eShader == ePCShader_Occluder)
	{
		m_aOccluders.push_back(COccluderPoly(pPoly));
		return;
	}
	// Put occluders into the splitter list
	if (eShader == ePCShader_Splitter)
	{
		m_aSplitters.push_back(CRawPoly(pPoly));
		return;
	}

	// Add the triangles to the list
	// Count the Section
	uint32 nSection = CountSection(pPoly, eShader, pPoly->NumVerts() - 2);

	CPCRenderVert3T v0(pPoly, 0);
	for (uint32 nAddLoop = 2; nAddLoop < pPoly->NumVerts(); ++nAddLoop)
	{
		m_aTris.push_back(
			CTri(v0, CPCRenderVert3T(pPoly, nAddLoop - 1), CPCRenderVert3T(pPoly, nAddLoop), pPoly, nSection, 
				((nAddLoop == 2) ? CTri::k_nExterior0 : 0) | CTri::k_nExterior1 | ((nAddLoop == (pPoly->NumVerts() - 1)) ? CTri::k_nExterior2 : 0)));
	}
}

bool CPCRenderTreeNode::WriteLightGroup(CAbstractIO &file, const CLightGroup &cLightGroup)
{
	// Write the name
	if (!file.WriteString(cLightGroup.m_sName.c_str()))
		return false;

	// Write the color
	file << cLightGroup.m_vColor;

	uint32 nLengthPos = file.GetCurPos();
	file << (uint32)0;

	ASSERT(cLightGroup.m_aVertexIntensities.size() == m_aVertices.size());

	// Write and 0-compress the vertex intensities
	bool bCompressing = false;
	uint8 nCount;
	uint32 nTotalData = 0;
	CLightGroup::TVertexIntensityList::const_iterator iCurIntensity = cLightGroup.m_aVertexIntensities.begin();
	for (; iCurIntensity != cLightGroup.m_aVertexIntensities.end(); ++iCurIntensity)
	{
		// Compress the 0's
		if (*iCurIntensity == 0)
		{
			if (bCompressing)
			{
				if (nCount == 255)
				{
					file << (uint8)255;
					nTotalData += 255;
					nCount = 0;
				}
				else
				{
					++nCount;
					continue;
				}
			}
			else
			{
				bCompressing = true;
				nCount = 0;
			}
		}
		// Handle the end of the 0's
		else if (bCompressing)
		{
			file << (uint8)nCount;
			nTotalData += nCount;
			bCompressing = false;
		}

		// Write the current intensity value
		file << (uint8)*iCurIntensity;
		++nTotalData;
	}

	if (bCompressing)
	{
		file << (uint8)nCount;
		nTotalData += nCount;
	}

	ASSERT(nTotalData == m_aVertices.size());

	// Go back and save how much we wrote into the file
	uint32 nEndPos = file.GetCurPos();
	uint32 nIntensitySize = nEndPos - (nLengthPos + sizeof(uint32));

	file.SeekTo(nLengthPos);
	file << (uint32)nIntensitySize;
	file.SeekTo(nEndPos);

	// Write out the section lightmap fix-ups
	ASSERT(cLightGroup.m_aSectionLightmaps.size() == m_aSections.size());
	file << (uint32)cLightGroup.m_aSectionLightmaps.size();
	CLightGroup::TLightmapList::const_iterator iCurSection = cLightGroup.m_aSectionLightmaps.begin();
	for (; iCurSection != cLightGroup.m_aSectionLightmaps.end(); ++iCurSection)
	{
		if (!*iCurSection)
		{
			file << (uint32)0;
			continue;
		}

		file << (uint32)(*iCurSection)->m_aSubLMs.size();
		CPCRenderTreeNode_LGLightmap::TSubLMList::const_iterator iCurSubLM = (*iCurSection)->m_aSubLMs.begin();
		for (; iCurSubLM != (*iCurSection)->m_aSubLMs.end(); ++iCurSubLM)
		{
			(*iCurSubLM)->Write(file);
		}
	}

	return true;
}

bool CPCRenderTreeNode::Write(CAbstractIO &file)
{
	// Write out this node's main information
	file << m_vCenter;
	file << m_vHalfDims;

	// Write out the Section list
	file << m_aSections.size();
	for (uint32 nSectionLoop = 0; nSectionLoop < m_aSections.size(); ++nSectionLoop)
	{
		CSection &cCurSection = m_aSections[nSectionLoop];

		if(cCurSection.m_pTexture0)
			file.WriteString(const_cast<char*>(cCurSection.m_pTexture0));
		else
			file << (uint16)0;

		if(cCurSection.m_pTexture1)
			file.WriteString(const_cast<char*>(cCurSection.m_pTexture1));
		else
			file << (uint16)0;

		file << (uint8)cCurSection.m_eShader;
		file << cCurSection.m_nCount;
		ASSERT(cCurSection.m_nCount != 0);

		if(cCurSection.m_pTextureEffect)
			file.WriteString(const_cast<char*>(cCurSection.m_pTextureEffect));
		else
			file << (uint16)0;

		// Write out the lightmap data for this section
		file << (uint32)cCurSection.m_cLightmap.m_Width;
		file << (uint32)cCurSection.m_cLightmap.m_Height;
		file << (uint32)cCurSection.m_cLightmap.m_Data.GetSize();
		file.Write(cCurSection.m_cLightmap.m_Data.GetArray(), cCurSection.m_cLightmap.m_Data.GetSize());
	}

	// Write out the vertex list
	ASSERT("Vertex list too long!" && m_aVertices.size() < 65536);
	file << m_aVertices.size();
	for (uint32 nVertLoop = 0; nVertLoop < m_aVertices.size(); ++nVertLoop)
	{
		file << m_aVertices[nVertLoop];
	}

	// Write out the triangle list
	// Note : These are sorted by Section
	ASSERT("Triangle list too long!" && m_aIndexTris.size() < 65536);
	ASSERT("Triangle count mismatch!" && m_aIndexTris.size() == m_aTris.size());
	file << m_aIndexTris.size();
	for (uint32 nTriLoop = 0; nTriLoop < m_aIndexTris.size(); ++nTriLoop)
	{
		CIndexTri &cCurTri = m_aIndexTris[nTriLoop];
		file << cCurTri.m_nIndex0 << cCurTri.m_nIndex1 << cCurTri.m_nIndex2;
		file << m_aTris[nTriLoop].m_pPoly->m_Index;
	}

	// Write out the sky portal list
	file << m_aSkyPortals.size();
	TRawPolyList::iterator iCurSkyPoly = m_aSkyPortals.begin();
	for (; iCurSkyPoly != m_aSkyPortals.end(); ++iCurSkyPoly)
	{
		CRawPoly &cCurPoly = *iCurSkyPoly;
		ASSERT(cCurPoly.m_aVerts.size() < 256);
		file << (uint8)cCurPoly.m_aVerts.size();
		CRawPoly::TVertList::const_iterator iCurVert = cCurPoly.m_aVerts.begin();
		for (; iCurVert != cCurPoly.m_aVerts.end(); ++iCurVert)
		{
			file << *iCurVert;
		}
		file << cCurPoly.m_cPlane.m_Normal;
		file << cCurPoly.m_cPlane.m_Dist;
	}

	// Write out the occluder list
	file << m_aOccluders.size();
	TOccluderPolyList::iterator iCurOccluderPoly = m_aOccluders.begin();
	for (; iCurOccluderPoly != m_aOccluders.end(); ++iCurOccluderPoly)
	{
		COccluderPoly &cCurPoly = *iCurOccluderPoly;
		ASSERT(cCurPoly.m_aVerts.size() < 256);
		file << (uint8)cCurPoly.m_aVerts.size();
		CRawPoly::TVertList::const_iterator iCurVert = cCurPoly.m_aVerts.begin();
		for (; iCurVert != cCurPoly.m_aVerts.end(); ++iCurVert)
		{
			file << *iCurVert;
		}
		file << cCurPoly.m_cPlane.m_Normal;
		file << cCurPoly.m_cPlane.m_Dist;
		file << cCurPoly.m_nName;
	}

	// Write out the lightgroup list
	file << m_aLightGroups.size();
	TLightGroupList::const_iterator iCurLightGroup = m_aLightGroups.begin();
	for (; iCurLightGroup != m_aLightGroups.end(); ++iCurLightGroup)
	{
		if (!WriteLightGroup(file, *iCurLightGroup))
			return false;
	}

	// Write out the children flags
	uint8 nChildFlags = 0;
	uint8 nMask = 1;
	for (uint8 nChildFlagLoop = 0; nChildFlagLoop < k_NumChildren; ++nChildFlagLoop)
	{
		if (m_aChildren[nChildFlagLoop])
			nChildFlags |= nMask;
		nMask = nMask << 1;
	}
	file << nChildFlags;

	// Write out the children indices
	for (uint8 nChildIndexLoop = 0; nChildIndexLoop < k_NumChildren; ++nChildIndexLoop)
	{
		if (m_aChildren[nChildIndexLoop])
			file << (uint32)m_aChildren[nChildIndexLoop]->GetIndex();
		else
			file << (uint32)0;
	}

	return true;
}

CPCRenderTreeNode::ESide CPCRenderTreeNode::GetPointSide(const LTVector &vPos, uint32 nDimension, float fCenter)
{
	// The epsilon value for considering a vertex to be on the dividing plane
	const float fSideEpsilon = 0.0001f;

	float fOfs = vPos[nDimension] - fCenter;

	if (fOfs < -fSideEpsilon)
		return eSide_Left;
	if (fOfs > fSideEpsilon)
		return eSide_Right;
	return eSide_Intersect;
}

CPCRenderTreeNode::ESide CPCRenderTreeNode::GetTriSide(const CTri &cTri, uint32 nDimension, float fCenter)
{
	uint32 nLeft = 0;
	uint32 nRight = 0;
	for (uint32 nCurVert = 0; nCurVert < 3; ++nCurVert)
	{
		ESide eCurSide = GetPointSide(cTri.Vert(nCurVert).m_vPos, nDimension, fCenter);
		if (eCurSide == eSide_Left)
			++nLeft;
		else if (eCurSide == eSide_Right)
			++nRight;
	}

	if (nLeft == 0)
		return eSide_Right;
	else if (nRight == 0)
		return eSide_Left;
	else
		return eSide_Intersect;
}

CPCRenderTreeNode::ESide CPCRenderTreeNode::GetRawPolySide(const CRawPoly &cPoly, uint32 nDimension, float fCenter)
{
	if (cPoly.m_vMax[nDimension] < fCenter)
		return eSide_Left;
	else if (cPoly.m_vMin[nDimension] > fCenter)
		return eSide_Right;
	else
		return eSide_Intersect;
}

bool AABBIntersect(
	const LTVector &vAABB1Min, const LTVector &vAABB1Max, 
	const LTVector &vAABB2Min, const LTVector &vAABB2Max)
{
	return 
		(vAABB1Min.x <= vAABB2Max.x) &&
		(vAABB1Min.y <= vAABB2Max.y) &&
		(vAABB1Min.z <= vAABB2Max.z) &&
		(vAABB1Max.x >= vAABB2Min.x) &&
		(vAABB1Max.y >= vAABB2Min.y) &&
		(vAABB1Max.z >= vAABB2Min.z);
}

bool CPCRenderTreeNode::GetSplitAxis(uint32 *pAxisIndex, float *pAxisValue) const
{
	const float k_fSplitPlaneEpsilon = 0.9999f;
	const float k_fMinIntrusion = 0.01f;
	
	// Look for an occluder that would be a good idea to split along
	// (Including the occluders contained in the parent nodes...)
	const LTPlane *pSplitPlane = 0;
	float fSplitSize = 0.0f;

	LTVector vIntersectMin = m_vCenter - m_vHalfDims * (1.0f - k_fMinIntrusion);
	LTVector vIntersectMax = m_vCenter + m_vHalfDims * (1.0f - k_fMinIntrusion);

	const CPCRenderTreeNode *pSearchNode = this;
	do
	{
		TOccluderPolyList::const_iterator iCurOccluder = pSearchNode->m_aOccluders.begin();
		for (; iCurOccluder != pSearchNode->m_aOccluders.end(); ++iCurOccluder)
		{
			const LTVector &vPlaneNormal = iCurOccluder->m_cPlane.m_Normal;
			if ((fabsf(vPlaneNormal.x) < k_fSplitPlaneEpsilon) &&
				(fabsf(vPlaneNormal.y) < k_fSplitPlaneEpsilon) &&
				(fabsf(vPlaneNormal.z) < k_fSplitPlaneEpsilon))
				continue;

			// Ignore it if it doesn't intersect this node
			if (!AABBIntersect(iCurOccluder->m_vMin, iCurOccluder->m_vMax, vIntersectMin, vIntersectMax))
				continue;

			float fCurOccluderSize = (iCurOccluder->m_vMax - iCurOccluder->m_vMin).Mag();
			if (fCurOccluderSize < fSplitSize)
				continue;

			// Remember this one...
			pSplitPlane = &iCurOccluder->m_cPlane;
			fSplitSize = fCurOccluderSize;
		}

		pSearchNode = pSearchNode->GetParent();
	} while (pSearchNode);

	// If we didn't find a good one, try looking through the splitters
	if (!pSplitPlane)
	{
		const CPCRenderTreeNode *pSearchNode = this;
		do
		{
			TRawPolyList::const_iterator iCurSplitter = pSearchNode->m_aSplitters.begin();
			for (; iCurSplitter != pSearchNode->m_aSplitters.end(); ++iCurSplitter)
			{
				const LTVector &vPlaneNormal = iCurSplitter->m_cPlane.m_Normal;
				if ((fabsf(vPlaneNormal.x) < k_fSplitPlaneEpsilon) &&
					(fabsf(vPlaneNormal.y) < k_fSplitPlaneEpsilon) &&
					(fabsf(vPlaneNormal.z) < k_fSplitPlaneEpsilon))
					continue;

				// Ignore it if it doesn't intersect this node
				if (!AABBIntersect(iCurSplitter->m_vMin, iCurSplitter->m_vMax, vIntersectMin, vIntersectMax))
					continue;

				float fCurSplitterSize = (iCurSplitter->m_vMax - iCurSplitter->m_vMin).Mag();
				if (fCurSplitterSize < fSplitSize)
					continue;

				// Remember this one...
				pSplitPlane = &iCurSplitter->m_cPlane;
				fSplitSize = fCurSplitterSize;
			}

			pSearchNode = pSearchNode->GetParent();
		} while (pSearchNode);

		// If we still didn't find a good one, split by the middle of the longest axis
		if (!pSplitPlane)
		{
			// Decide which axis is the biggest
			*pAxisIndex = 0;
			if (fabs(m_vHalfDims[1]) > fabs(m_vHalfDims[*pAxisIndex]))
				*pAxisIndex = 1;
			if (fabs(m_vHalfDims[2]) > fabs(m_vHalfDims[*pAxisIndex]))
				*pAxisIndex = 2;

			*pAxisValue = m_vCenter[*pAxisIndex];
			return false;
		}
	}

	*pAxisIndex = 0;
	if (fabs(pSplitPlane->m_Normal[1]) > fabs(pSplitPlane->m_Normal[*pAxisIndex]))
		*pAxisIndex = 1;
	if (fabs(pSplitPlane->m_Normal[2]) > fabs(pSplitPlane->m_Normal[*pAxisIndex]))
		*pAxisIndex = 2;

	*pAxisValue = pSplitPlane->m_Normal[*pAxisIndex] * pSplitPlane->m_Dist;

	return true;
}

void CPCRenderTreeNode::ReduceTriCount()
{
	// Fix the T-Junctions on the triangles if it's the root node
	// Note : This would be much faster if it could use the post-reduced triangle sets
	if (!GetParent())
		FixTJunctions();

	// Get the splitting axis
	uint32 nSplitIndex;
	float fSplitValue;
	bool bForced = GetSplitAxis(&nSplitIndex, &fSplitValue);

	// Don't do anything if the constraints haven't been broken
	// (For now, this just reduces the triangle count to an arbitrary value)
	if (!bForced && (m_aTris.size() < k_MaxTrisPerNode)) // NYI
		return;

	// Don't split if it doesn't have any width in that direction
	const float k_fMinSplitWidth = 0.001f;
	if (m_vHalfDims[nSplitIndex] < k_fMinSplitWidth)
		return;

	CPCRenderTreeNode *pLeft = new CPCRenderTreeNode(m_pTree);
	CPCRenderTreeNode *pRight = new CPCRenderTreeNode(m_pTree);

	// Set up the parent/child relationship
	m_aChildren[0] = pLeft;
	m_aChildren[1] = pRight;

	pLeft->m_pParent = this;
	pRight->m_pParent = this;

	float fSplitBoxMin = m_vCenter[nSplitIndex] - m_vHalfDims[nSplitIndex];
	float fSplitBoxMax = m_vCenter[nSplitIndex] + m_vHalfDims[nSplitIndex];

	ASSERT(fSplitBoxMin < fSplitValue);
	ASSERT(fSplitBoxMax > fSplitValue);

	// Set up their bounding boxes
	LTVector vLeftHalfDims = m_vHalfDims;
	vLeftHalfDims[nSplitIndex] = (fSplitValue - fSplitBoxMin) * 0.5f;
	pLeft->SetHalfDims(vLeftHalfDims);

	LTVector vRightHalfDims = m_vHalfDims;
	vRightHalfDims[nSplitIndex] = (fSplitBoxMax - fSplitValue) * 0.5f;
	pRight->SetHalfDims(vRightHalfDims);

	LTVector vLeftCenter = m_vCenter;
	vLeftCenter[nSplitIndex] = (fSplitBoxMin + fSplitValue) * 0.5f;
	pLeft->SetCenter(vLeftCenter);
	LTVector vRightCenter = m_vCenter;
	vRightCenter[nSplitIndex] = (fSplitBoxMax + fSplitValue) * 0.5f;
	pRight->SetCenter(vRightCenter);

	// Give the children some breathing room in their triangle lists
	pLeft->m_aTris.reserve(m_aTris.size() / 2);
	pRight->m_aTris.reserve(m_aTris.size() / 2);

	// Filter all the triangles into the children
	{
		TTriList aNewTriList;
		for (uint32 nFilterLoop = 0; nFilterLoop < m_aTris.size(); ++nFilterLoop)
		{
			CTri &cCurTri = m_aTris[nFilterLoop];
			switch (GetTriSide(cCurTri, nSplitIndex, fSplitValue))
			{
				case eSide_Left : 
					pLeft->m_aTris.push_back(cCurTri);
					break;
				case eSide_Right : 
					pRight->m_aTris.push_back(cCurTri);
					break;
				case eSide_Intersect :
					aNewTriList.push_back(cCurTri);
					break;
			}
		}

		// If we ended up with both of the children being empty, consider ourselves a leaf
		if (pLeft->m_aTris.empty() && pRight->m_aTris.empty())
		{
			delete pLeft;
			delete pRight;
			m_aChildren[0] = 0;
			m_aChildren[1] = 0;
			return;
		}

		m_aTris.swap(aNewTriList);
	}

	// Filter all the occluders into the children
	{
		TOccluderPolyList aNewOccluderList;
		TOccluderPolyList::iterator iCurOccluder = m_aOccluders.begin();
		for (; iCurOccluder != m_aOccluders.end(); ++iCurOccluder)
		{
			switch (GetRawPolySide(*iCurOccluder, nSplitIndex, fSplitValue))
			{
				case eSide_Left : 
					pLeft->m_aOccluders.push_back(*iCurOccluder);
					break;
				case eSide_Right : 
					pRight->m_aOccluders.push_back(*iCurOccluder);
					break;
				case eSide_Intersect :
					aNewOccluderList.push_back(*iCurOccluder);
					break;
			}
		}
		m_aOccluders.swap(aNewOccluderList);
	}

	// Filter all the splitters into the children
	{
		TRawPolyList aNewSplitterList;
		TRawPolyList::iterator iCurSplitter = m_aSplitters.begin();
		for (; iCurSplitter != m_aSplitters.end(); ++iCurSplitter)
		{
			switch (GetRawPolySide(*iCurSplitter, nSplitIndex, fSplitValue))
			{
				case eSide_Left : 
					pLeft->m_aSplitters.push_back(*iCurSplitter);
					break;
				case eSide_Right : 
					pRight->m_aSplitters.push_back(*iCurSplitter);
					break;
				case eSide_Intersect :
					aNewSplitterList.push_back(*iCurSplitter);
					break;
			}
		}
		m_aSplitters.swap(aNewSplitterList);
	}

	// Filter all the sky portals into the children
	{
		TRawPolyList aNewSkyPortalList;
		TRawPolyList::iterator iCurSkyPortal = m_aSkyPortals.begin();
		for (; iCurSkyPortal != m_aSkyPortals.end(); ++iCurSkyPortal)
		{
			switch (GetRawPolySide(*iCurSkyPortal, nSplitIndex, fSplitValue))
			{
				case eSide_Left : 
					pLeft->m_aSkyPortals.push_back(*iCurSkyPortal);
					break;
				case eSide_Right : 
					pRight->m_aSkyPortals.push_back(*iCurSkyPortal);
					break;
				case eSide_Intersect :
					aNewSkyPortalList.push_back(*iCurSkyPortal);
					break;
			}
		}
		m_aSkyPortals.swap(aNewSkyPortalList);
	}

	// Give our section list to our children
	pLeft->m_aSections = m_aSections;
	pRight->m_aSections = m_aSections;

	// Tell the children to update their Section lists
	pLeft->UpdateSectionList();
	pRight->UpdateSectionList();

	// Update our Section list
	UpdateSectionList();

	// Tell the children to update their bounding boxes
	pLeft->UpdateBounds();
	pRight->UpdateBounds();

	// Tell the children to split
	pLeft->ReduceTriCount();
	pRight->ReduceTriCount();
}

CPCRenderVert2T GetShaderVert(const CPCRenderVert3T &cVert, EPCShaderType eShader)
{
	switch (eShader)
	{
		// Use the second set of TC's for lightmaps
		case ePCShader_Lightmap :
			return CPCRenderVert2T(cVert.m_vPos, cVert.m_fU2, cVert.m_fV2, 0.0f, 0.0f, cVert.m_nColor, cVert.m_vNormal, cVert.m_vTangent, cVert.m_vBinormal);
		case ePCShader_Lightmap_DualTexture:
		case ePCShader_DualTexture:
			return CPCRenderVert2T(cVert.m_vPos, cVert.m_fU0, cVert.m_fV0, cVert.m_fU1, cVert.m_fV1, cVert.m_nColor, cVert.m_vNormal, cVert.m_vTangent, cVert.m_vBinormal);
		default :
			return CPCRenderVert2T(cVert.m_vPos, cVert.m_fU0, cVert.m_fV0, 0.0f, 0.0f, cVert.m_nColor, cVert.m_vNormal, cVert.m_vTangent, cVert.m_vBinormal);
	}
}

void CPCRenderTreeNode::Optimize(SOptimizeStats *pOptStats)
{
	m_pOptData = new COptData(this);

	// Get the section indices
	for (uint32 nSectionIndex = 0; nSectionIndex < m_aSections.size(); ++nSectionIndex)
		m_aSections[nSectionIndex].m_nOriginalIndex = nSectionIndex;

	// Sort the sections
	std::sort(m_aSections.begin(), m_aSections.end());

	// Pack the lightmaps
	uint32 nCurSection = 0;
	while (nCurSection < m_aSections.size())
	{
		CSection &cCurSection = m_aSections[nCurSection];

		if (!IsPCShaderLightmap(cCurSection.m_eShader))
		{
			++nCurSection;
			continue;
		}

		// Pack the lightmaps for this shader/animation/whatever
		CPCLMPacker cPacker(cCurSection.m_pLMAnim, m_pTree);

		uint32 nSectionAdded = 0;
		for (uint32 nCurTri = 0; nCurTri < m_aTris.size(); ++nCurTri)
		{
			CTri &cCurTri = m_aTris[nCurTri];
			if (cCurTri.m_nSection == cCurSection.m_nOriginalIndex)
				cPacker.AddTri(nCurTri, cCurTri.m_pPoly);
		}

		uint32 nNumPages, nArea, nPageArea;
		cPacker.Pack(m_aTris, &nNumPages, &nArea, &nPageArea);

		if (pOptStats)
		{
			pOptStats->m_nLMPages += nNumPages;
			pOptStats->m_nLMArea += nArea;
			pOptStats->m_nLMPageArea += nPageArea;
		}

		uint32 nNewPageCount = cPacker.GetPageCount() - 1;

		ASSERT(cPacker.GetPageCount() > 0);

		if (nNewPageCount)
		{
			// Shift forward the triangles
			for (uint32 nCurTri = 0; nCurTri < m_aTris.size(); ++nCurTri)
			{
				CTri &cCurTri = m_aTris[nCurTri];
				if (cCurTri.m_nSection > cCurSection.m_nOriginalIndex)
					cCurTri.m_nSection += nNewPageCount;
			}

			// Shift forward the sections
			TSectionList::iterator iShiftSection = m_aSections.begin();
			for(; iShiftSection != m_aSections.end(); ++iShiftSection)
			{
				if ((*iShiftSection).m_nOriginalIndex > cCurSection.m_nOriginalIndex)
					(*iShiftSection).m_nOriginalIndex += nNewPageCount;
			}

			// Make some new sections
			// Note : Can't use cCurSection after this point
			m_aSections.insert(m_aSections.begin() + nCurSection + 1, nNewPageCount, cCurSection);
			for (uint32 nOffset = 1; nOffset <= nNewPageCount; ++nOffset)
			{
				m_aSections[nCurSection + nOffset].m_nOriginalIndex += nOffset;
			}

			// Adjust the triangles to point at the right pages
			CPCLMPacker::TIndexList aPageTris;
			for (uint32 nAdjLoop = 1; nAdjLoop <= nNewPageCount; ++nAdjLoop)
			{
				cPacker.GetPageTris(nAdjLoop, aPageTris);

				m_aSections[nCurSection].m_nCount -= aPageTris.size();
				m_aSections[nCurSection + nAdjLoop].m_nCount = aPageTris.size();

				CPCLMPacker::TIndexList::iterator iCurTri = aPageTris.begin();
				for (; iCurTri != aPageTris.end(); ++iCurTri)
				{
					m_aTris[*iCurTri].m_nSection += nAdjLoop;
				}
			}
		}

		for (uint32 nCompressLoop = 0; nCompressLoop <= nNewPageCount; ++nCompressLoop, ++nCurSection)
			cPacker.CompressLightmap(nCompressLoop, m_aSections[nCurSection]);
	}

	// Re-order the triangles by section
	// And convert to indexed tris
	for (nCurSection = 0; nCurSection < m_aSections.size(); ++nCurSection)
	{
		CSection &cCurSection = m_aSections[nCurSection];

		// Clear the vertex map so we don't share vertices between sections
		m_pOptData->Clear();

		uint32 nSectionAdded = 0;
		for (uint32 nCurTri = 0; nCurTri < m_aTris.size(); ++nCurTri)
		{
			CTri &cCurTri = m_aTris[nCurTri];
			if (cCurTri.m_nSection != cCurSection.m_nOriginalIndex)
				continue;

			++nSectionAdded;
			CIndexTri cNewTri(
					m_pOptData->FindVertIndex(GetShaderVert(cCurTri.m_Vert0, cCurSection.m_eShader)),
					m_pOptData->FindVertIndex(GetShaderVert(cCurTri.m_Vert1, cCurSection.m_eShader)),
					m_pOptData->FindVertIndex(GetShaderVert(cCurTri.m_Vert2, cCurSection.m_eShader)),
					cCurTri.m_pPoly
				);
			// Copy the T-Junction data
			for (uint32 nTJuncCopy = 0; nTJuncCopy < 3; ++nTJuncCopy)
			{
				cNewTri.m_pTJunc[nTJuncCopy] = (cCurTri.m_pTJunc[nTJuncCopy]) ? new CTriTJunc(*(cCurTri.m_pTJunc[nTJuncCopy])) : 0;
			}
			m_aIndexTris.push_back(cNewTri);
		}
		ASSERT("Section sort mismatch encountered!" && nSectionAdded == cCurSection.m_nCount);
		// Correct the section count so it loads "correctly"
		cCurSection.m_nCount = nSectionAdded;
	}

	// Get rid of the optimization data
	delete m_pOptData;
	m_pOptData = 0;

	// Build a re-ordering list for the triangles and reset the section indices
	std::vector<uint32> aReorder;
	aReorder.resize(m_aSections.size(), m_aSections.size());
	for (uint32 nReorderSection = 0; nReorderSection < m_aSections.size(); ++nReorderSection)
	{
		ASSERT(m_aSections[nReorderSection].m_nOriginalIndex < aReorder.size());
		aReorder[m_aSections[nReorderSection].m_nOriginalIndex] = nReorderSection;
		m_aSections[nReorderSection].m_nOriginalIndex = nReorderSection;
	}

	// Re-order the triangles
	for (uint32 nCurTri = 0; nCurTri < m_aTris.size(); ++nCurTri)
	{
		CTri &cCurTri = m_aTris[nCurTri];
		ASSERT(cCurTri.m_nSection < aReorder.size());
		ASSERT(aReorder[cCurTri.m_nSection] < m_aSections.size());
		cCurTri.m_nSection = aReorder[cCurTri.m_nSection];
	}

	// Update our section list
	UpdateSectionList();

	// Perform a cache optimization on the triangle list
	uint32 nStartTri = 0;
	TSectionList::iterator iCurSection = m_aSections.begin();
	for (; iCurSection != m_aSections.end(); ++iCurSection)
	{
		CacheReOrder(nStartTri, iCurSection->m_nCount);
		nStartTri += iCurSection->m_nCount;
	}
}

bool CPCRenderTreeNode::ReadIndexTriLightGroupData(const CIndexTri &cIndexTri, const CPrePoly *pPoly, const uint8 *pIntensities, uint32 nNumIntensities, CLightGroup &cLightGroup)
{
	if (nNumIntensities < pPoly->NumVerts())
	{
		ASSERT(!"Invalid light group intensity list encountered!");
		return false;
	}

	bool bResult = false;

	const float k_fDuplicateVertexEpsilon = 0.1f;

	for (uint32 nCurVert = 0; nCurVert < pPoly->NumVerts(); ++nCurVert)
	{
		// Find the matching vertex in this triangle, if it exists
		LTVector vVertPos = pPoly->Vert(nCurVert).m_Vec;
		uint32 nVertIndex;
		if (vVertPos.NearlyEquals(m_aVertices[cIndexTri.m_nIndex0].m_vPos, k_fDuplicateVertexEpsilon))
			nVertIndex = cIndexTri.m_nIndex0;
		else if (vVertPos.NearlyEquals(m_aVertices[cIndexTri.m_nIndex1].m_vPos, k_fDuplicateVertexEpsilon))
			nVertIndex = cIndexTri.m_nIndex1;
		else if (vVertPos.NearlyEquals(m_aVertices[cIndexTri.m_nIndex2].m_vPos, k_fDuplicateVertexEpsilon))
			nVertIndex = cIndexTri.m_nIndex2;
		else 
			continue;

		ASSERT(nVertIndex < cLightGroup.m_aVertexIntensities.size());

		if (cLightGroup.m_aVertexIntensities[nVertIndex] != 0)
		{
			// If we've already found this vertex intensity, it should be the same value every time
			// Sometimes it's not, and I'm not sure why, but it doesn't seem to be hurting anything
			// in my tests..  Look in on it further if it ever creates problems.
			//ASSERT(pIntensities[nCurVert] == cLightGroup.m_aVertexIntensities[nVertIndex]);
			continue;
		}

		// Write it into the intensity list
		cLightGroup.m_aVertexIntensities[nVertIndex] = pIntensities[nCurVert];
		bResult |= (pIntensities[nCurVert] != 0);
	}

	// Fill in the TJunction data
	for (uint32 nTriVert = 0; nTriVert < 3; ++nTriVert)
	{
		const CTriTJunc *pTJunc = cIndexTri.m_pTJunc[nTriVert];

		if (!pTJunc)
			continue;

		// Find the non-T-Junction intensities in the poly data
		bool bFoundStart = false;
		uint8 nStartIntensity = 0;
		bool bFoundEnd = false;
		uint8 nEndIntensity = 0;
		for (uint32 nPolyVert = 0; (nPolyVert < pPoly->NumVerts()) && (!bFoundStart || !bFoundEnd); ++nPolyVert)
		{
			LTVector vVertPos = pPoly->Vert(nPolyVert).m_Vec;
			if (pTJunc->m_vStart.NearlyEquals(vVertPos, k_fDuplicateVertexEpsilon))
			{
				bFoundStart = true;
				nStartIntensity = pIntensities[nPolyVert];
			}
			if (pTJunc->m_vEnd.NearlyEquals(vVertPos, k_fDuplicateVertexEpsilon))
			{
				bFoundEnd = true;
				nEndIntensity = pIntensities[nPolyVert];
			}
		}

		// Skip it if we didn't find both  (This really shouldn't happen...)
		if (!bFoundStart || !bFoundEnd)
		{
			ASSERT(!"Unable to find both ends of a T-Junction split in the lightgroup processing");
			continue;
		}

		// Get the vertex index
		uint32 nVertIndex = (nTriVert == 0) ? cIndexTri.m_nIndex0 : (nTriVert == 1) ? cIndexTri.m_nIndex1 : cIndexTri.m_nIndex2;

		// We don't already have an intensity here, do we?
		if (cLightGroup.m_aVertexIntensities[nVertIndex] != 0)
			continue;

		// Figure out the interpolated intensity
		float fEdgeMag = (pTJunc->m_vEnd - pTJunc->m_vStart).Mag();
		LTVector vPosOfs = m_aVertices[nVertIndex].m_vPos - pTJunc->m_vStart;
		float fPosOfsMag = vPosOfs.Mag();
		float fInterpolant;
		if (fPosOfsMag < k_fDuplicateVertexEpsilon)
			fInterpolant = 0.0f;
		else
		{
			fInterpolant = fPosOfsMag / fEdgeMag;
			fInterpolant = LTCLAMP(fInterpolant, 0.0f, 1.0f);
		}
		float fIntensity = LTLERP((float)nStartIntensity, (float)nEndIntensity, fInterpolant);
		uint8 nFinalIntensity = (uint8)fIntensity;

		// Write the intensity into the lightgroup data
		cLightGroup.m_aVertexIntensities[nVertIndex] = nFinalIntensity;
		bResult |= (nFinalIntensity != 0);
	}

	return bResult;
}

void CPCRenderTreeNode::ReadLightGroups(const CPreLightGroup * const *pLightGroups, uint32 nNumLightGroups)
{
	m_aLightGroups.reserve(nNumLightGroups);
	CLightGroup lightGroup;

	const CPreLightGroup * const *iCurLightGroup = pLightGroups;
	const CPreLightGroup * const *iEndLightGroup = &pLightGroups[nNumLightGroups];
	for (; iCurLightGroup != iEndLightGroup; ++iCurLightGroup)
	{
		// Build our lightgroup data object
		CLightGroup cLG;
		cLG.m_sName = (*iCurLightGroup)->GetName();
		cLG.m_vColor = (*iCurLightGroup)->GetColor();
		cLG.m_aVertexIntensities.resize(m_aVertices.size(), 0);
		cLG.m_aSectionLightmaps.resize(m_aSections.size(), 0);

		uint32 nTrisFound = 0;
		uint32 nLightmapsFound = 0;
		const CPrePoly *pPoly = 0;

		// Fill it in with the polys we've got triangles from
		const CPreLightGroup::TPolyDataList &cPolyData = (*iCurLightGroup)->GetPolyData();
		CPreLightGroup::TPolyDataList::const_iterator iCurLGPoly = cPolyData.begin();
		for (; iCurLGPoly != cPolyData.end(); ++iCurLGPoly)
		{
			pPoly = 0;

			TIndexTriList::const_iterator iCurTri = m_aIndexTris.begin();
			while (iCurTri != m_aIndexTris.end())
			{
				const CPrePoly *pTriPoly;

				// Find the next triangle associated with this poly
				for (; iCurTri != m_aIndexTris.end(); ++iCurTri)
				{
					pTriPoly = iCurTri->m_pPoly;
					if ((pTriPoly->m_Index == iCurLGPoly->m_hPoly.m_iPoly) &&
						(pTriPoly->m_WorldIndex == iCurLGPoly->m_hPoly.m_iWorld))
					{
						pPoly = pTriPoly;
						break;
					}
				}
				if (iCurTri == m_aIndexTris.end())
					break;
				
				if (ReadIndexTriLightGroupData(*iCurTri, pTriPoly, &(*(iCurLGPoly->m_aVertexIntensities.begin())), iCurLGPoly->m_aVertexIntensities.size(), cLG))
					++nTrisFound;

				++iCurTri;
			}

			// If the poly wasn't in the indexed triangle list, it's not actually touched by this node
			if (!pPoly)
				continue;

			// We don't need to read the lightmaps if there isn't a lightmap on this poly
			if (!iCurLGPoly->m_pLightmap)
				continue;

			// Read the lightmaps from the sections
			TSectionList::const_iterator iCurSection = m_aSections.begin();
			CLightGroup::TLightmapList::iterator iCurSectionLightmap = cLG.m_aSectionLightmaps.begin();
			for (; iCurSection != m_aSections.end(); ++iCurSection, ++iCurSectionLightmap)
			{
				// Skip sections that aren't lightmapped
				if (iCurSection->m_eShader != ePCShader_Lightmap)
					continue;

				// Look for this polygon in the section
				CSection::TLMPolyMap::const_iterator iPolyLightmap = iCurSection->m_cLMPolyMap.find(pPoly);
				if (iPolyLightmap == iCurSection->m_cLMPolyMap.end())
					continue;

				// Make sure we've got a lightmap fix-up list associated with this section
				if (!*iCurSectionLightmap)
					*iCurSectionLightmap = new CPCRenderTreeNode_LGLightmap;
				
				(*iCurSectionLightmap)->m_aSubLMs.push_back(new LGSubLightmap(iCurLGPoly->m_pLightmap, iPolyLightmap->second, &iCurSection->m_cLightmap));
				// Count it
				++nLightmapsFound;
			}
		}

		// Don't add it to the list if we weren't touching any of the data
		if (!nTrisFound && !nLightmapsFound)
			continue;

		// Add the light group to the end of the list
		m_aLightGroups.push_back(lightGroup);
		m_aLightGroups.back().swap(cLG);
	}
}

// Use the nVidia tri-stripper to re-order the mesh
// Note : This is currently commented out because it's generating a longer triangle list
// than it was provided with, and I don't have time to look in on handling that at this point.
// Also note: This is really not going to work with the T-Junction data, since all the
// triangles get randomly re-ordered without updating m_pTJunc
void CPCRenderTreeNode::CacheReOrder(uint32 nStartTri, uint32 nTriCount)
{
	/*
	typedef std::vector<uint16> TIndexList;
	TIndexList aIndices;
	aIndices.reserve(nTriCount);
	TIndexTriList::iterator iCurTri = m_aIndexTris.begin() + nStartTri;
	TIndexTriList::iterator iEndTri = iCurTri + nTriCount;
	for (; iCurTri != iEndTri; ++iCurTri)
	{
		aIndices.push_back((uint16)iCurTri->m_nIndex0);
		aIndices.push_back((uint16)iCurTri->m_nIndex1);
		aIndices.push_back((uint16)iCurTri->m_nIndex2);
	}

	PrimitiveGroup *pPrimGroup;
	uint16 nNumGroups;
	GenerateStrips(aIndices.begin(), aIndices.size(), &pPrimGroup, &nNumGroups);
	ASSERT(nNumGroups == 1);
	ASSERT(pPrimGroup->numIndices == (nTriCount * 3));

	uint16 *pCurIndex = pPrimGroup->indices;
	iCurTri = m_aIndexTris.begin() + nStartTri;
	for (; iCurTri != iEndTri; ++iCurTri)
	{
		iCurTri->m_nIndex0 = *pCurIndex;
		++pCurIndex;
		iCurTri->m_nIndex1 = *pCurIndex;
		++pCurIndex;
		iCurTri->m_nIndex2 = *pCurIndex;
		++pCurIndex;
	}
	*/
}


