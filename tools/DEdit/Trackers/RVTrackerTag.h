//////////////////////////////////////////////////////////////////////
// RVTrackerTag.h - Header for the tagging tracker 

#ifndef __RVTRACKERTAG_H__
#define __RVTRACKERTAG_H__

#include "rvtracker.h"

class CRVTrackerTag : public CRegionViewTracker
{
protected:
	enum ESelectMode {
		SELECT_OFF,
		SELECT_ON,
		SELECT_INVERT
	};

	// Supporting function for selecting a node
	//takes elements needed to select a specific node, as well as whether or not
	//it should select it in the node view. This is for performance reasons since
	//if a large list of nodes is being selected, only the last one should be jumped to
	//since jumping to each one requires a TON of unneeded drawing of the list -JohnO
	void SelectNode(CEditRegion *pRegion, CWorldNode *pNode, HTREEITEM hItem, ESelectMode eMode, BOOL bSelectInTree = TRUE);

	// Find the polygon that's closest to the ray
	CPolyRef FindBestPoly(CEditRay ray);

public:
	enum EctorFlags { 
		FLAG_NONE = 0,
		FLAG_OBJECT = 1, 
		FLAG_BRUSH = 2,
		FLAG_GEOMETRY = 4,
		FLAG_ALL = 31
	};

	CRVTrackerTag(LPCTSTR pName = "", CRegionView *pView = NULL, int iFlags = FLAG_NONE);
	virtual ~CRVTrackerTag();

	BOOL m_bObject, m_bBrush, m_bGeometry;

	// Track whether or not the "add" key is down
	CRegionViewTracker* m_pAddTracker;
	BOOL m_bAdd;

	// Track whether or not the "invert" key is down
	CRegionViewTracker* m_pInvertTracker;
	BOOL m_bInvert;

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();
	virtual void OnCancel();

	virtual void WatchEvent(const CUIEvent &cEvent);

	void FlushTracker();
	void SetupChildTracker(CRegionViewTracker* pTracker);
};

#endif //__RVTRACKERTAG_H__