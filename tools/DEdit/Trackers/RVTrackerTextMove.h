//////////////////////////////////////////////////////////////////////
// RVTrackerTextMove.h - Header for the texture movement tracker 

#ifndef __RVTRACKERTEXTMOVE_H__
#define __RVTRACKERTEXTMOVE_H__

#include "rvtracker.h"

class CRVTrackerTextureMove : public CRegionViewTracker
{
protected:

	CRegionViewTracker *m_pRotateTracker, *m_pScaleTracker, *m_pStepTracker;
	BOOL m_bRotate, m_bScale, m_bStep;

	//array to hold the selected polygons as well as their original PQ vectors
	CMoArray<CTexturedPlane*>	m_Textures;
	CMoArray<LTVector>			m_TextureNormals;
	CMoArray<CWorldNode*>		m_UndoNodes;
	CMoArray<LTVector>			m_OriginalO;
	CMoArray<LTVector>			m_OriginalP;	
	CMoArray<LTVector>			m_OriginalQ;

	int32 m_iTotalRotate;
	int32 m_iTotalScaleX, m_iTotalScaleY;
	int32 m_iTotalOfsX, m_iTotalOfsY;

	BOOL m_bSaveUndo;

	void SetupChildTracker(CRegionViewTracker* pTracker);

public:
	CRVTrackerTextureMove(LPCTSTR pName = "", CRegionView *pView = NULL);
	virtual ~CRVTrackerTextureMove();

	// Overridden functions
	virtual void WatchEvent(const CUIEvent &cEvent);

	virtual BOOL OnStart();
	virtual BOOL OnUpdate(const CUIEvent &cEvent);
	virtual BOOL OnEnd();

	virtual void FlushTracker();

};

#endif //__RVTRACKERTEXTMOVE_H__