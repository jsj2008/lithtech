//////////////////////////////////////////////////////////////////////
// RVTrackerTextWrap.h - Header for the far clipping distance tracker 

#ifndef __RVTRACKERTEXTWRAP_H__
#define __RVTRACKERTEXTWRAP_H__

#include "rvtracker.h"
#include "rvtrackertag.h"

class CRVTrackerTextureWrap : public CRVTrackerTag
{
public:
	CRVTrackerTextureWrap(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerTextureWrap();

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	// Disable functions in RVTrackerTag...
	virtual void OnCancel() { CRegionViewTracker::OnCancel(); };
	virtual void WatchEvent(const CUIEvent &cEvent);

private:

	//skip tracker
	CRegionViewTracker		*m_pSkipTracker;

	// Keep track of when the dialog pops up for proper focus handling
	BOOL m_bShowingDialog;

	CPolyRef m_rBasePoly;
	CPoint m_cStartPoint;

	// The options we get from the dialog
	enum { k_WrapLinear = 0, k_WrapPath = 1, k_WrapInvalid };
	int m_nWrapStyle;
	BOOL m_bScaleToFit;
	BOOL m_bAdjustOffset;
	BOOL m_bRestrictWalkDir;

	// Edge information tracking..
	struct CTWEdgeInfo
	{
		CTWEdgeInfo(const CVector &vPt1, const CVector &vPt2, const CVector &vNormal)
		{
			m_aPt[0] = vPt1; 
			m_aPt[1] = vPt2;
			m_vDir = vPt2 - vPt1;
			m_fLen = m_vDir.Mag();
			m_vDir *= 1.0f / m_fLen;
			m_Plane.m_Normal = vNormal.Cross(m_vDir);
			m_Plane.m_Normal.Norm();
			m_Plane.m_Dist = m_Plane.m_Normal.Dot(vPt1);
		}
		CVector m_aPt[2]; // The points of the edge
		CVector m_vDir; // Normalized direction vector
		float m_fLen; // How long the edge is
		CEditPlane m_Plane; // A plane defined by the edge and the poly normal
	};

	// Poly information tracking..
	struct CTWPolyInfo
	{
		CTWPolyInfo() : m_pPoly(NULL), m_bTouched(FALSE) {}
		~CTWPolyInfo() { 
			// Delete the edge list
			for (uint32 nEdgeLoop = 0; nEdgeLoop < m_aEdges.GetSize(); ++nEdgeLoop)
				delete m_aEdges[nEdgeLoop];
		}
		CEditPoly *m_pPoly; // The polygon
		CMoArray<CTWEdgeInfo*> m_aEdges; // The edges of the polygon
		CMoArray<CTWPolyInfo*> m_aNeighbors; // The neighbors on each edge
		BOOL m_bTouched; // Have you been textured yet?
	};

	// Get a similar edge direction from a polygon
	BOOL GetSimilarEdgeDir(const CTWPolyInfo *pPoly, const CVector &vDir, CVector &vResult, float fMinDot = 0.0f) const;

	// A list of polys
	typedef CMoArray<CTWPolyInfo *> CTWPolyList;

	// Build a polygon list
	void BuildPolyList(CTWPolyList &aList) const;

	// Find a poly in a list
	uint32 FindPolyIndex(const CTWPolyList &aList, CEditPoly *pPoly) const;

	// Texture extents structure
	struct CTextExtents
	{
		CTextExtents() : m_fMinU(65536.0f), m_fMaxU(-65536.0f), m_fMinV(65536.0f), m_fMaxV(-65536.0f) {}
		float m_fMinU, m_fMaxU;
		float m_fMinV, m_fMaxV;
	};

	// Wrap the textures, starting at a poly
	void WrapTexture(CTWPolyInfo *pPoly, const CVector &vWrapDir, CTextExtents &cExtents) const;

	// Get the first available text multiplier
	BOOL GetFirstTextureMul(CTWPolyList &aList, float &fTextureU, float &fTextureV) const;

	// Scale the texture spaces to an even multiple
	void ScaleToMultiple(CTWPolyList &aList, CTextExtents &cExtents) const;

	// Offset the textures to fit at the top/left
	void AlignToTopLeft(CTWPolyList &aList, CTextExtents &cExtents) const;

	// Clear a polygon set
	void ClearPolyList(CTWPolyList &aList) const;

	void FlushTracker();
};

#endif //__RVTRACKERTEXTWRAP_H__