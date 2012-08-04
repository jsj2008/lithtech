//////////////////////////////////////////////////////////////////////////////
// PC lightmap packer class

#ifndef __PCLMPACKER_H__
#define __PCLMPACKER_H__

#include "pcrendertreenode.h"

#include <vector>

class CPreLightAnim;
class CPreMainWorld;
class CPCRenderTree;

// Page data structure declaration
struct CPCLMPage;

class CPCLMPacker
{
public:
	CPCLMPacker(const CPreLightAnim *pCurAnim, const CPCRenderTree *pRenderTree);
	~CPCLMPacker();

	// Note : Any triangle which is added must not be moved in memory until after
	// the packer is destroyed
	void AddTri(uint32 nTriIndex, const CPrePoly *pPoly);
	void Pack(CPCRenderTreeNode::TTriList &aTris, uint32 *pNumPages, uint32 *pArea, uint32 *pPageArea);

	// Get the actual data for a lightmap on a given page
	void CompressLightmap(uint32 nPage, CPCRenderTreeNode::CSection &cSection);

	typedef std::vector<uint32> TIndexList;

	void GetPageTris(uint32 nPageIndex, TIndexList &aResults);

	uint32 GetPageCount() { return m_aPages.size(); }

private:
	struct CPolyTrack
	{
		CPolyTrack() :
			m_nPageIndex(0),
			m_pLMData(0)
		{
			m_nMin[0] = 0;
			m_nMin[1] = 0;
		}
		uint32 m_nMin[2];
		uint32 m_nSize[2];
		uint32 m_nPageIndex;
		const CPreLightMap *m_pLMData;
		TIndexList m_aPolyTris;
	};

	typedef std::map<const CPrePoly *, CPolyTrack> TPolyTrackMap;

	struct CPolyTrack_SortLargest
	{
		bool operator()(const CPolyTrack *pLeft, const CPolyTrack *pRight) {
			return (pLeft->m_nSize[0] * pLeft->m_nSize[1]) > (pRight->m_nSize[0] * pRight->m_nSize[1]);
		}
	};

	typedef std::vector<CPCLMPage *> TPageList;

	// A list of pointers to polys, specifically for sorting
	typedef std::vector<CPolyTrack *> TPolySortList;

	void GetNewPageDims(TPolySortList::const_iterator iStart, TPolySortList::const_iterator iEnd, uint32 *pDimsX, uint32 *pDimsY);
private:
	const CPreLightAnim *m_pCurAnim;
	const CPCRenderTree *m_pRenderTree;

	TPageList m_aPages;

	TPolyTrackMap m_cPolyMap;
};


#endif //__PCLMPACKER_H__