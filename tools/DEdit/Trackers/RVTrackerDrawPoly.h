//////////////////////////////////////////////////////////////////////
// RVTrackerDrawPoly.h - Header for the polygon creation tracker 

#ifndef __RVTRACKERDRAWPOLY_H__
#define __RVTRACKERDRAWPOLY_H__

#include "rvtracker.h"

class CRVTrackerDrawPoly : public CRegionViewTracker
{
protected:

	CRegionViewTracker	*m_pUndoTracker;
	CRegionViewTracker	*m_pNewVertexTracker;
	CRegionViewTracker	*m_pSplitTracker;
	CRegionViewTracker	*m_pRotateTracker;
	CRegionViewTracker	*m_pInsertEdgeTracker;
	CRegionViewTracker	*m_pVertSnapTracker;
	CRegionViewTracker	*m_pCloseTracker;

	//sets up a child tracker
	void	SetupChildTracker(CRegionViewTracker* pTracker);

	// Whether or not to allow cancelling
	BOOL m_bAllowCancel;
	BOOL m_bFinishDrawingPoly;

	//specify if we want to snap to existing vertices in the world
	BOOL m_bVertSnap;

	// Action commands in response to events.  Returns TRUE to end the tracker
	virtual BOOL OnUndo();
	virtual BOOL OnNewVertex();
	virtual BOOL OnSplit();
	virtual BOOL OnRotate();
	virtual BOOL OnNewEdge();

	//called to close the polygon. If there are enough vertices, this will
	//connect the first and last points together and finishes the polygon
	virtual BOOL ClosePoly();

public:

	CRVTrackerDrawPoly(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerDrawPoly();

	// Cancel override
	virtual void Cancel();

	void FlushTracker();

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

private:

	// Updates the last vertex on the current drawing poly.
	void		UpdateDrawPolyVertex( CPoint &point );

	//given a point on the screen, it will calculate the point on the edit grid that should
	//be used for the brush
	LTVector	CalcCurrMouseVert(CPoint& point);


	// Internal supporting routines for inserting new edges
	BOOL FindSplittingBrushInfo(CEditBrush *pBrush, float &fTestError, CPolyRef &rTestRef, CEditVert &vVert1, CEditVert &vVert2);
	BOOL FindSplittingInfo(CPolyRef &rRef, CEditVert &vVert1, CEditVert &vVert2);

};

#endif //__RVTRACKERDRAWPOLY_H__