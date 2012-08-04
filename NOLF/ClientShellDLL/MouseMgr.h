#if !defined(_MOUSEMGR_H_)
#define _MOUSEMGR_H_

#include "iltclient.h"



class CMouseMgr
{
public:

	CMouseMgr();
	~CMouseMgr() { }

	bool Init(int xPosInit = 320, int yPosInit = 240,
		      int xMin = 0, int yMin = 0,
			  int xMax = 639, int yMax = 479
			 );

	void Term();

	void Draw(HSURFACE hSurf, int xOffset = 0, int yOffset = 0);

	void SetMouseControl() { m_bMouseControl = true; }
	void ReleaseMouseControl() { m_bMouseControl = false; }
	void SetMousePos(int x, int y);
	void SetClickPos(int x, int y) { m_xClick = x; m_yClick = y; }

	int GetClickPosX() { return m_xClick; }
	int GetClickPosY() { return m_yClick; }

	void SetDims(int xMin, int yMin, int xMax, int yMax) { m_xMin = xMin; m_xMax = xMax, m_yMin = yMin, m_yMax = yMax; }

	int GetPosX() { return m_xPos; }
	int GetPosY() { return m_yPos; }

	int GetPrevX() { return m_xPrev; }
	int GetPrevY() { return m_yPrev; }

	int GetDeltaX() { return m_xPos - m_xPrev; }
	int GetDeltaY() { return m_yPos - m_yPrev; }

	void SetClipRect(CRect *prcClip,BOOL bLockToXAxis = FALSE, BOOL bLockToYAxis = FALSE);

protected:

	int m_xPos;
	int m_yPos;
	int m_xPrev;
	int m_yPrev;
	int m_xClick;
	int m_yClick;

	int m_xMin;
	int m_xMax;
	int m_yMin;
	int m_yMax;

	CRect m_rcClip;
	bool m_bDefault;

	bool m_bMouseControl;
	BOOL m_bLockToXAxis;
	BOOL m_bLockToYAxis;

};

inline void CMouseMgr::SetClipRect(CRect *prcClip, BOOL bLockToXAxis, BOOL bLockToYAxis)
{
	if(prcClip)
	{
		m_rcClip = *prcClip;
		m_bDefault = false;
	}
	else
	{
		if(m_bDefault)
			return;

		m_rcClip.SetRect(m_xMin,m_yMin,m_xMax,m_yMax);
		m_bDefault = true;
	}
	m_bLockToXAxis = bLockToXAxis;
	m_bLockToYAxis = bLockToYAxis;
}



extern CMouseMgr g_mouseMgr;




#endif