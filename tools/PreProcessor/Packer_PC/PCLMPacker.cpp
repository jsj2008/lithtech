//////////////////////////////////////////////////////////////////////////////
// PC Lightmap packer implementation

#include "bdefs.h"
#include "pcrendertree.h"
#include "pclmpacker.h"

#include "lightmapdefs.h"

#include "preworld.h"

#include <algorithm>
#include <list>

// Rectangular page fragment structure
struct CPCLMRectFragment
{
	CPCLMRectFragment(
		uint32 nMinX = 0, 
		uint32 nMinY = 0, 
		uint32 nSizeX = 0, 
		uint32 nSizeY = 0, 
		CPCLMRectFragment *pNeighborUp = 0,
		CPCLMRectFragment *pNeighborDown = 0,
		CPCLMRectFragment *pNeighborLeft = 0,
		CPCLMRectFragment *pNeighborRight = 0 
		)
	{
		m_nMin[0] = nMinX;
		m_nMin[1] = nMinY;
		m_nSize[0] = nSizeX;
		m_nSize[1] = nSizeY;
		m_aNeighbors[eDir_Up] = pNeighborUp;
		m_aNeighbors[eDir_Down] = pNeighborDown;
		m_aNeighbors[eDir_Left] = pNeighborLeft;
		m_aNeighbors[eDir_Right] = pNeighborRight;
	}

	uint32 m_nMin[2];
	uint32 m_nSize[2];

	enum EDirection { 
		eDir_Up = 0,
		eDir_Down = 1,
		eDir_Left = 2,
		eDir_Right = 3
	};
	CPCLMRectFragment *m_aNeighbors[4];

	void Detach();
};

//////////////////////////////////////////////////////////////////////////////
// CPCLMPacker::CPCLMRectFragment implementation

void CPCLMRectFragment::Detach()
{
	if (m_aNeighbors[eDir_Up])
		m_aNeighbors[eDir_Up]->m_aNeighbors[eDir_Down] = 0;
	m_aNeighbors[eDir_Up] = 0;
	if (m_aNeighbors[eDir_Down])
		m_aNeighbors[eDir_Down]->m_aNeighbors[eDir_Up] = 0;
	m_aNeighbors[eDir_Down] = 0;
	if (m_aNeighbors[eDir_Left])
		m_aNeighbors[eDir_Left]->m_aNeighbors[eDir_Right] = 0;
	m_aNeighbors[eDir_Left] = 0;
	if (m_aNeighbors[eDir_Right])
		m_aNeighbors[eDir_Right]->m_aNeighbors[eDir_Left] = 0;
	m_aNeighbors[eDir_Right] = 0;
}

//////////////////////////////////////////////////////////////////////////////
// CPCLMPage - Lightmap page tracking structure

struct CPCLMPage
{
	CPCLMPage(uint32 nDimX = k_MaxSize, uint32 nDimY = k_MaxSize);
	~CPCLMPage();
	// Split a fragment into 2 pieces
	void Split(
		CPCLMRectFragment *pFragment,
		// Coordinate along the split axis
		uint32 nSplit,
		// The splitting direction
		CPCLMRectFragment::EDirection eDir
	);
	// Pack a rectangle into this page
	// returns false if it wouldn't fit
	bool Pack(uint32 nSizeX, uint32 nSizeY, uint32 &nOfsX, uint32 &nOfsY);
	typedef std::list<CPCLMRectFragment*> TFragmentList;
	TFragmentList m_aFragments;

	uint32 GetWastedArea() const;

	// Maximum size of the page
	enum { k_MaxSize = 258 };
	// Minimum size of the page
	enum { k_MinSize = 18 };

	// Size of this page
	uint32 m_nWidth, m_nHeight;
};

//////////////////////////////////////////////////////////////////////////////
// CPCLMPage implementation

CPCLMPage::CPCLMPage(uint32 nDimX, uint32 nDimY)
{
	// Add one rectangle fragment that takes up the whole page
	m_aFragments.push_front(new CPCLMRectFragment(0,0, nDimX, nDimY));
	m_nWidth = nDimX;
	m_nHeight = nDimY;
}

CPCLMPage::~CPCLMPage()
{
	while (!m_aFragments.empty())
	{
		delete *(m_aFragments.begin());
		m_aFragments.erase(m_aFragments.begin());
	}
}

void CPCLMPage::Split(
		CPCLMRectFragment *pFragment,
		uint32 nSplit,
		CPCLMRectFragment::EDirection eDir
	)
{
	// Figure out our split axis
	uint32 nSplitAxis;
	switch (eDir)
	{
		case CPCLMRectFragment::eDir_Up :
		case CPCLMRectFragment::eDir_Down :
			nSplitAxis = 0;
			break;
		case CPCLMRectFragment::eDir_Left :
		case CPCLMRectFragment::eDir_Right :
			nSplitAxis = 1;
			break;
	}

	// Get the relative directions
	CPCLMRectFragment::EDirection eInvDir;
	CPCLMRectFragment::EDirection eLeftDir;
	CPCLMRectFragment::EDirection eRightDir;
	switch (eDir)
	{
		case CPCLMRectFragment::eDir_Up :
		case CPCLMRectFragment::eDir_Down :
			eLeftDir = CPCLMRectFragment::eDir_Left;
			eRightDir = CPCLMRectFragment::eDir_Right;
			break;
		case CPCLMRectFragment::eDir_Left :
		case CPCLMRectFragment::eDir_Right :
			eLeftDir = CPCLMRectFragment::eDir_Up;
			eRightDir = CPCLMRectFragment::eDir_Down;
			break;
	}

	switch (eDir)
	{
		case CPCLMRectFragment::eDir_Up :
			eInvDir = CPCLMRectFragment::eDir_Down;
			break;
		case CPCLMRectFragment::eDir_Down :
			eInvDir = CPCLMRectFragment::eDir_Up;
			break;
		case CPCLMRectFragment::eDir_Left :
			eInvDir = CPCLMRectFragment::eDir_Right;
			break;
		case CPCLMRectFragment::eDir_Right :
			eInvDir = CPCLMRectFragment::eDir_Left;
			break;
	}

	ASSERT((pFragment->m_nMin[nSplitAxis] < nSplit) && ((pFragment->m_nMin[nSplitAxis] + pFragment->m_nSize[nSplitAxis]) > nSplit));

	// Find the top of the field
	CPCLMRectFragment *pTop = pFragment;
	while (pTop->m_aNeighbors[eDir])
		pTop = pTop->m_aNeighbors[eDir];

	uint32 nLeftSize = nSplit - pFragment->m_nMin[nSplitAxis];
	uint32 nRightSize = pFragment->m_nSize[nSplitAxis] - nLeftSize;

	// Split the field along the axis
	CPCLMRectFragment *pFinger = pTop;
	while (pFinger)
	{
		// Shrink the current rectangle
		pFinger->m_nSize[nSplitAxis] = nLeftSize;

		// Create a rectangle for the right side
		CPCLMRectFragment *pRight = new CPCLMRectFragment;
		m_aFragments.push_front(pRight);
		pRight->m_nMin[nSplitAxis] = nSplit;
		pRight->m_nMin[!nSplitAxis] = pFinger->m_nMin[!nSplitAxis];
		pRight->m_nSize[nSplitAxis] = nRightSize;
		pRight->m_nSize[!nSplitAxis] = pFinger->m_nSize[!nSplitAxis];

		// Link the right neighbor
		if (pFinger->m_aNeighbors[eRightDir])
			pFinger->m_aNeighbors[eRightDir]->m_aNeighbors[eLeftDir] = pRight;
		pRight->m_aNeighbors[eRightDir] = pFinger->m_aNeighbors[eRightDir];

		// Link up along the split axis
		pFinger->m_aNeighbors[eRightDir] = pRight;
		pRight->m_aNeighbors[eLeftDir] = pFinger;

		// Next
		pFinger = pFinger->m_aNeighbors[eInvDir];
	}

	// Go back and link up the rectangles vertically
	pFinger = pTop;
	while (pFinger)
	{
		CPCLMRectFragment *pRight = pFinger->m_aNeighbors[eRightDir];
		CPCLMRectFragment *pUp = pFinger->m_aNeighbors[eDir];
		CPCLMRectFragment *pDown = pFinger->m_aNeighbors[eInvDir];

		pRight->m_aNeighbors[eDir] = (pUp) ? pUp->m_aNeighbors[eRightDir] : 0;
		pRight->m_aNeighbors[eInvDir] = (pDown) ? pDown->m_aNeighbors[eRightDir] : 0;

		// Next
		pFinger = pFinger->m_aNeighbors[eInvDir];
	}
}

bool CPCLMPage::Pack(uint32 nSizeX, uint32 nSizeY, uint32 &nOfsX, uint32 &nOfsY)
{
	CPCLMRectFragment *pTopLeft;
	CPCLMRectFragment *pBottomRight = 0;

	uint32 nRemainingWidth, nRemainingHeight;

	// Go searching for a matching rectangle first
	TFragmentList::iterator iTopLeft = m_aFragments.begin();
	for (; iTopLeft != m_aFragments.end() && !pBottomRight; ++iTopLeft)
	{
		pTopLeft = *iTopLeft;
		if ((pTopLeft->m_nSize[0] == nSizeX) && (pTopLeft->m_nSize[1] == nSizeY))
			pBottomRight = pTopLeft;
	}

	// Search for an area that will contain this rectangle
	if (!pBottomRight)
	{
		iTopLeft = m_aFragments.begin();
		for (; iTopLeft != m_aFragments.end() && !pBottomRight; ++iTopLeft)
		{
			pTopLeft = *iTopLeft;

			CPCLMRectFragment *pVertFinger = pTopLeft;
			CPCLMRectFragment *pHorizFinger;
			nRemainingHeight = nSizeY;
			do
			{
				// Look to see if there is enough space horizontally
				pHorizFinger = pVertFinger;
				nRemainingWidth = nSizeX;
				while (pHorizFinger && (nRemainingWidth > pHorizFinger->m_nSize[0]))
				{
					nRemainingWidth -= pHorizFinger->m_nSize[0];
					pHorizFinger = pHorizFinger->m_aNeighbors[CPCLMRectFragment::eDir_Right];
				}

				// Make sure we've still got enough space vertically
				if (nRemainingHeight > pVertFinger->m_nSize[1])
				{
					nRemainingHeight -= pVertFinger->m_nSize[1];
					pVertFinger = pVertFinger->m_aNeighbors[CPCLMRectFragment::eDir_Down];
				}
				// If we got here, it's a valid rectangle
				else
					break;
			} while (pVertFinger && pHorizFinger);

			// Did we find a valid rectangle?
			if (pVertFinger && pHorizFinger)
				pBottomRight = pHorizFinger;
		}
	}

	// We didn't find a place to fit it.. :(
	if (!pBottomRight)
		return false;

	// Remember the top-left
	nOfsX = pTopLeft->m_nMin[0];
	nOfsY = pTopLeft->m_nMin[1];

	// Split the rectangle grid vertically
	if ((nOfsX + nSizeX) < (pBottomRight->m_nMin[0] + pBottomRight->m_nSize[0]))
		Split(pBottomRight, nOfsX + nSizeX, CPCLMRectFragment::eDir_Up);

	// Split the rectangle grid horizontally
	if ((nOfsY + nSizeY) < (pBottomRight->m_nMin[1] + pBottomRight->m_nSize[1]))
		Split(pBottomRight, nOfsY + nSizeY, CPCLMRectFragment::eDir_Left);

	// Remove the covered area from the rectangle grid
	CPCLMRectFragment *pRemoveLine = pTopLeft;
	uint32 nHeight = nSizeY;
	while (pRemoveLine)
	{
		CPCLMRectFragment *pRemoveFinger = pRemoveLine;
		nHeight -= pRemoveLine->m_nSize[1];
		if (!nHeight)
			pRemoveLine = 0;
		else
		{
			ASSERT(pRemoveLine->m_aNeighbors[CPCLMRectFragment::eDir_Down]);
			pRemoveLine = pRemoveLine->m_aNeighbors[CPCLMRectFragment::eDir_Down];
		}
		uint32 nWidth = nSizeX;
		while (pRemoveFinger)
		{
			CPCLMRectFragment *pTemp = pRemoveFinger;
			nWidth -= pRemoveFinger->m_nSize[0];
			if (!nWidth)
				pRemoveFinger = 0;
			else
			{
				ASSERT(pRemoveFinger->m_aNeighbors[CPCLMRectFragment::eDir_Right]);
				pRemoveFinger = pRemoveFinger->m_aNeighbors[CPCLMRectFragment::eDir_Right];
			}

			pTemp->Detach();
			m_aFragments.erase(std::remove(m_aFragments.begin(), m_aFragments.end(), pTemp), m_aFragments.end());
			delete pTemp;
		}
	}

	return true;
}

uint32 CPCLMPage::GetWastedArea() const
{
	uint32 nResult = 0;

	TFragmentList::const_iterator iCurFragment = m_aFragments.begin();
	for (; iCurFragment != m_aFragments.end(); ++iCurFragment)
		nResult += (*iCurFragment)->m_nSize[0] * (*iCurFragment)->m_nSize[1];

	return nResult;
}

//////////////////////////////////////////////////////////////////////////////
// CPCLMPacker implementation

CPCLMPacker::CPCLMPacker(const CPreLightAnim *pCurAnim, const CPCRenderTree *pRenderTree) :
	m_pCurAnim(pCurAnim),
	m_pRenderTree(pRenderTree)
{
}

CPCLMPacker::~CPCLMPacker()
{
}

void CPCLMPacker::AddTri(uint32 nTriIndex, const CPrePoly *pPoly)
{
	CPolyTrack &cTracker = m_cPolyMap[pPoly];
	if (!cTracker.m_pLMData)
	{
		cTracker.m_pLMData = m_pRenderTree->GetPolyLMData(m_pCurAnim, pPoly);
		if (!cTracker.m_pLMData)
		{
			ASSERT(!"Triangle add for unknown poly encountered");
			m_cPolyMap.erase(m_cPolyMap.find(pPoly));
			return;
		}
		cTracker.m_nSize[0] = cTracker.m_pLMData->m_Width + 2;
		cTracker.m_nSize[1] = cTracker.m_pLMData->m_Height + 2;
	}

	cTracker.m_aPolyTris.push_back(nTriIndex);
}

void CPCLMPacker::GetPageTris(uint32 nPageIndex, TIndexList &aResults)
{
	aResults.clear();

	TPolyTrackMap::iterator iCurPoly = m_cPolyMap.begin();
	for (; iCurPoly != m_cPolyMap.end(); ++iCurPoly)
	{
		CPolyTrack &cCurPoly = iCurPoly->second;
		if (cCurPoly.m_nPageIndex != nPageIndex)
			continue;

		aResults.insert(aResults.end(), cCurPoly.m_aPolyTris.begin(), cCurPoly.m_aPolyTris.end());
	}
}

uint32 GetNextPow2(uint32 nValue)
{
	if (!nValue)
		return 0;

	uint32 nResult = nValue;
	while(1)
	{
		uint32 nOneBitLess = nResult & (nResult - 1);
		if (!nOneBitLess)
			break;
		nResult = nOneBitLess;
	}

	if (nResult == nValue)
		return nResult;
	else
		return nResult << 1;
}

void CPCLMPacker::GetNewPageDims(TPolySortList::const_iterator iStart, TPolySortList::const_iterator iEnd, uint32 *pDimsX, uint32 *pDimsY)
{
	*pDimsX = 0;
	*pDimsY = 0;

	// Find the largest extents in each dimension
	TPolySortList::const_iterator iCurPoly = iStart;
	for (; iCurPoly != iEnd; ++iCurPoly)
	{
		*pDimsX = LTMAX(*pDimsX, (*iCurPoly)->m_nSize[0]);
		*pDimsY = LTMAX(*pDimsY, (*iCurPoly)->m_nSize[1]);
	}

	// Add some space for the rest of the polys in the page
	uint32 nAreaRemaining = 0;
	iCurPoly = iStart;
	for (; iCurPoly != iEnd; ++iCurPoly)
	{
		nAreaRemaining += (*iCurPoly)->m_nSize[0] * (*iCurPoly)->m_nSize[1];
	}
	nAreaRemaining -= *pDimsX * *pDimsY;

	*pDimsX += nAreaRemaining / (*pDimsY * 2);

	// Get the final dims
	*pDimsX = GetNextPow2(*pDimsX) + 2;
	*pDimsX = LTCLAMP(*pDimsX, CPCLMPage::k_MinSize, CPCLMPage::k_MaxSize);
	*pDimsY = GetNextPow2(*pDimsY) + 2;
	*pDimsY = LTCLAMP(*pDimsY, CPCLMPage::k_MinSize, CPCLMPage::k_MaxSize);
}

void CPCLMPacker::Pack(CPCRenderTreeNode::TTriList &aTris, uint32 *pNumPages, uint32 *pArea, uint32 *pPageArea)
{
	// Dump the map into a vector
	TPolySortList aSortedPolys;
	aSortedPolys.reserve(m_cPolyMap.size());
	TPolyTrackMap::iterator iCurMap = m_cPolyMap.begin();
	for (; iCurMap != m_cPolyMap.end(); ++iCurMap)
		aSortedPolys.push_back(&(iCurMap->second));
	
	// Sort the polys by lightmap area
	std::sort(aSortedPolys.begin(), aSortedPolys.end(), CPolyTrack_SortLargest());

	// Pack each of them into a page
	TPolySortList::iterator iCurPoly = aSortedPolys.begin();
	for (; iCurPoly != aSortedPolys.end(); ++iCurPoly)
	{
		CPolyTrack &cCurPoly = **iCurPoly;

		TPageList::iterator iCurPage = m_aPages.begin();
		uint32 nPageIndex = 0;
		for (; iCurPage != m_aPages.end(); ++iCurPage, ++nPageIndex)
		{
			if ((*iCurPage)->Pack(cCurPoly.m_nSize[0], cCurPoly.m_nSize[1], 
				cCurPoly.m_nMin[0], cCurPoly.m_nMin[1]))
			{
				break;
			}
		}
		if (nPageIndex >= m_aPages.size())
		{
			uint32 nPageDimsX, nPageDimsY;
			GetNewPageDims(iCurPoly, aSortedPolys.end(), &nPageDimsX, &nPageDimsY);
			CPCLMPage *pNewPage = new CPCLMPage(nPageDimsX, nPageDimsY);
			m_aPages.push_back(pNewPage);
			bool bPackResult = pNewPage->Pack(cCurPoly.m_nSize[0], cCurPoly.m_nSize[1], 
				cCurPoly.m_nMin[0], cCurPoly.m_nMin[1]);
			ASSERT("Error packing polygon on empty page" && bPackResult);
		}
		cCurPoly.m_nPageIndex = nPageIndex;
	}

	// Go through the triangles and adjust their TCoords
	iCurPoly = aSortedPolys.begin();
	for (; iCurPoly != aSortedPolys.end(); ++iCurPoly)
	{
		CPolyTrack &cCurPoly = **iCurPoly;

		float fUOfs = (float)cCurPoly.m_nMin[0];
		float fVOfs = (float)cCurPoly.m_nMin[1];

		CPCLMPage *pCurPage = m_aPages[cCurPoly.m_nPageIndex];

		float fPageWidthScale = 1.0f / (float)(pCurPage->m_nWidth - 2);
		float fPageHeightScale = 1.0f / (float)(pCurPage->m_nHeight - 2);

		TIndexList::iterator iCurTri = cCurPoly.m_aPolyTris.begin();
		for (; iCurTri != cCurPoly.m_aPolyTris.end(); ++iCurTri)
		{
			CPCRenderTreeNode::CTri &cTri = aTris[*iCurTri];
			for (uint32 nCurVert = 0; nCurVert < 3; ++nCurVert)
			{
				CPCRenderVert3T &cCurVert = cTri.Vert(nCurVert);
				cCurVert.m_fU2 += fUOfs;
				cCurVert.m_fU2 *= fPageWidthScale;
				cCurVert.m_fV2 += fVOfs;
				cCurVert.m_fV2 *= fPageHeightScale;
			}
		}
	}

	// Gather statistics
	*pNumPages = 0;
	*pPageArea = 0;
	*pArea = 0;

	TPageList::const_iterator iCurPage = m_aPages.begin();
	for (; iCurPage != m_aPages.end(); ++iCurPage)
	{
		++*pNumPages;
		uint32 nCurPageArea = (*iCurPage)->m_nWidth * (*iCurPage)->m_nHeight;
		
		*pPageArea += nCurPageArea;
		*pArea += nCurPageArea - (*iCurPage)->GetWastedArea();
	}
}

struct SLMDataGrid
{
	SLMDataGrid(LM_DATA *pData, uint32 nWidth, uint32 nHeight) :
		m_pData(pData),
		m_nWidth(nWidth),
		m_nHeight(nHeight)
	{}

	uint8 &Data(uint32 nX, uint32 nY, uint32 nIndex) {
		ASSERT(nX < m_nWidth);
		ASSERT(nY < m_nHeight);
		return m_pData[(nY * m_nWidth * 3) + (nX * 3) + nIndex];
	}

	uint8 *m_pData;
	uint32 m_nWidth;
	uint32 m_nHeight;
};

void CPCLMPacker::CompressLightmap(uint32 nPage, CPCRenderTreeNode::CSection &cSection)
{
	if (nPage >= m_aPages.size())
	{
		ASSERT(!"Invalid compression request found");
		return;
	}
	CPCLMPage *pPage = m_aPages[nPage];

	// Allocate a temporary array to dump the packed lightmaps into
	uint32 nDataStride = pPage->m_nWidth * 3;
	uint32 nImageDataSize = pPage->m_nHeight * nDataStride;
	LM_DATA *pTempData = new LM_DATA[nImageDataSize];
	memset(pTempData, 0, nImageDataSize);

	SLMDataGrid sLMGrid(pTempData, pPage->m_nWidth, pPage->m_nHeight);

	// Allocate a temporary array to get the old lightmaps from
	LM_DATA aDecompBuffer[LIGHTMAP_MAX_DATA_SIZE];

	TPolyTrackMap::iterator iCurPoly = m_cPolyMap.begin();
	for (; iCurPoly != m_cPolyMap.end(); ++iCurPoly)
	{
		CPolyTrack &cCurPoly = iCurPoly->second;
		if (cCurPoly.m_nPageIndex != nPage)
			continue;

		// Add the poly to the section's polygon map
		RECT rPolyRect;
		rPolyRect.left = cCurPoly.m_nMin[0];
		rPolyRect.top = cCurPoly.m_nMin[1];
		rPolyRect.right = rPolyRect.left + cCurPoly.m_nSize[0] - 2;
		rPolyRect.bottom = rPolyRect.top + cCurPoly.m_nSize[1] - 2;
		ASSERT(cSection.m_cLMPolyMap.find(iCurPoly->first) == cSection.m_cLMPolyMap.end());
		cSection.m_cLMPolyMap[iCurPoly->first] = rPolyRect;

		cCurPoly.m_pLMData->Decompress(aDecompBuffer);

		// Copy the lightmap into the final buffer
		uint32 nDataOfs = (cCurPoly.m_nMin[0] + 1) * 3 + (cCurPoly.m_nMin[1] + 1) * nDataStride;
		LM_DATA *pDataLine = &pTempData[nDataOfs];
		LM_DATA *pDecompLine = aDecompBuffer;
		uint32 nDecompStride = cCurPoly.m_pLMData->m_Width * 3;
		for (uint32 nCopyY = 0; nCopyY < cCurPoly.m_pLMData->m_Height; ++nCopyY)
		{
			memcpy(pDataLine, pDecompLine, nDecompStride);
			pDataLine += nDataStride;
			pDecompLine += nDecompStride;
		}

		// Duplicate the edge texels
		uint32 nLeft = cCurPoly.m_nMin[0] + 1;
		uint32 nTop = cCurPoly.m_nMin[1] + 1;
		uint32 nRight = nLeft + cCurPoly.m_nSize[0] - 3;
		uint32 nBottom = nTop + cCurPoly.m_nSize[1] - 3;
		ASSERT((cCurPoly.m_nSize[0] - 2) == cCurPoly.m_pLMData->m_Width);
		ASSERT((cCurPoly.m_nSize[1] - 2) == cCurPoly.m_pLMData->m_Height);
		for (uint32 nDupeX = 0; nDupeX < cCurPoly.m_pLMData->m_Width; ++nDupeX)
		{
			sLMGrid.Data(nLeft + nDupeX, nTop - 1, 0) = sLMGrid.Data(nLeft + nDupeX, nTop, 0);
			sLMGrid.Data(nLeft + nDupeX, nTop - 1, 1) = sLMGrid.Data(nLeft + nDupeX, nTop, 1);
			sLMGrid.Data(nLeft + nDupeX, nTop - 1, 2) = sLMGrid.Data(nLeft + nDupeX, nTop, 2);

			sLMGrid.Data(nLeft + nDupeX, nBottom + 1, 0) = sLMGrid.Data(nLeft + nDupeX, nBottom, 0);
			sLMGrid.Data(nLeft + nDupeX, nBottom + 1, 1) = sLMGrid.Data(nLeft + nDupeX, nBottom, 1);
			sLMGrid.Data(nLeft + nDupeX, nBottom + 1, 2) = sLMGrid.Data(nLeft + nDupeX, nBottom, 2);
		}

		for (uint32 nDupeY = 0; nDupeY < cCurPoly.m_pLMData->m_Height; ++nDupeY)
		{
			sLMGrid.Data(nLeft - 1, nTop + nDupeY, 0) = sLMGrid.Data(nLeft, nTop + nDupeY, 0);
			sLMGrid.Data(nLeft - 1, nTop + nDupeY, 1) = sLMGrid.Data(nLeft, nTop + nDupeY, 1);
			sLMGrid.Data(nLeft - 1, nTop + nDupeY, 2) = sLMGrid.Data(nLeft, nTop + nDupeY, 2);

			sLMGrid.Data(nRight + 1, nTop + nDupeY, 0) = sLMGrid.Data(nRight, nTop + nDupeY, 0);
			sLMGrid.Data(nRight + 1, nTop + nDupeY, 1) = sLMGrid.Data(nRight, nTop + nDupeY, 1);
			sLMGrid.Data(nRight + 1, nTop + nDupeY, 2) = sLMGrid.Data(nRight, nTop + nDupeY, 2);
		}

		// Blend the corner texels
		sLMGrid.Data(nLeft - 1, nTop - 1, 0) = sLMGrid.Data(nLeft - 1, nTop, 0) / 2 + sLMGrid.Data(nLeft, nTop - 1, 0) / 2;
		sLMGrid.Data(nLeft - 1, nTop - 1, 1) = sLMGrid.Data(nLeft - 1, nTop, 1) / 2 + sLMGrid.Data(nLeft, nTop - 1, 1) / 2;
		sLMGrid.Data(nLeft - 1, nTop - 1, 2) = sLMGrid.Data(nLeft - 1, nTop, 2) / 2 + sLMGrid.Data(nLeft, nTop - 1, 2) / 2;

		sLMGrid.Data(nRight + 1, nTop - 1, 0) = sLMGrid.Data(nRight + 1, nTop, 0) / 2 + sLMGrid.Data(nRight, nTop - 1, 0) / 2;
		sLMGrid.Data(nRight + 1, nTop - 1, 1) = sLMGrid.Data(nRight + 1, nTop, 1) / 2 + sLMGrid.Data(nRight, nTop - 1, 1) / 2;
		sLMGrid.Data(nRight + 1, nTop - 1, 2) = sLMGrid.Data(nRight + 1, nTop, 2) / 2 + sLMGrid.Data(nRight, nTop - 1, 2) / 2;

		sLMGrid.Data(nLeft - 1, nBottom + 1, 0) = sLMGrid.Data(nLeft - 1, nBottom, 0) / 2 + sLMGrid.Data(nLeft, nBottom + 1, 0) / 2;
		sLMGrid.Data(nLeft - 1, nBottom + 1, 1) = sLMGrid.Data(nLeft - 1, nBottom, 1) / 2 + sLMGrid.Data(nLeft, nBottom + 1, 1) / 2;
		sLMGrid.Data(nLeft - 1, nBottom + 1, 2) = sLMGrid.Data(nLeft - 1, nBottom, 2) / 2 + sLMGrid.Data(nLeft, nBottom + 1, 2) / 2;

		sLMGrid.Data(nRight + 1, nBottom + 1, 0) = sLMGrid.Data(nRight + 1, nBottom, 0) / 2 + sLMGrid.Data(nRight, nBottom + 1, 0) / 2;
		sLMGrid.Data(nRight + 1, nBottom + 1, 1) = sLMGrid.Data(nRight + 1, nBottom, 1) / 2 + sLMGrid.Data(nRight, nBottom + 1, 1) / 2;
		sLMGrid.Data(nRight + 1, nBottom + 1, 2) = sLMGrid.Data(nRight + 1, nBottom, 2) / 2 + sLMGrid.Data(nRight, nBottom + 1, 2) / 2;
	}

	// Strip off the outside edge, as that's going to be handled by texture clamping
	uint32 nFinalStride = (pPage->m_nWidth - 2) * 3;
	uint32 nFinalDataSize = (pPage->m_nHeight - 2) * nFinalStride;
	LM_DATA *pFinalData = new LM_DATA[nFinalDataSize];
	memset(pFinalData, 0, nFinalDataSize);
	LM_DATA *pFinalFinger = pFinalData;
	LM_DATA *pTempFinger = &pTempData[nDataStride + 3];
	for (uint32 nFinalY = 0; nFinalY < (pPage->m_nHeight - 2); ++nFinalY)
	{
		memcpy(pFinalFinger, pTempFinger, nFinalStride);
		pFinalFinger += nFinalStride;
		pTempFinger += nDataStride;
	}

	// Compress the lightmap data
	cSection.m_cLightmap.Compress(pPage->m_nWidth - 2, pPage->m_nHeight - 2, pFinalData);

	// Get rid of the temporary buffers
	delete[] pFinalData;
	delete[] pTempData;
}

