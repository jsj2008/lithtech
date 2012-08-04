#include "StdAfx.h"
#include "MouseMgr.h"
//#include "ClientShell.h"


CMouseMgr g_mouseMgr;



CMouseMgr::CMouseMgr()
{
	m_xPos = 0;
	m_yPos = 0;
	m_xPrev = 0;
	m_yPrev = 0;
	m_xClick = 0;
	m_yClick = 0;

	m_bDefault = false;
	m_bMouseControl = false;
	m_bLockToXAxis = FALSE;
	m_bLockToYAxis = FALSE;
}



bool CMouseMgr::Init(int xPosInit, int yPosInit,
					 int xMin, int yMin,
					 int xMax, int yMax
					)
{
	m_xPos = xPosInit;
	m_yPos = yPosInit;
	m_xPrev = m_xPos;
	m_yPrev = m_yPos;

	m_xMin = xMin;
	m_xMax = xMax;
	m_yMin = yMin;
	m_yMax = yMax;

	m_bDefault = false;
	SetClipRect(NULL);

	return true;
}



void CMouseMgr::Term()
{

}


void CMouseMgr::SetMousePos(int x, int y)
{
	m_xPrev = m_xPos;
	m_yPrev = m_yPos;
	m_xPos = x;
	m_yPos = y;

	// Check for movement restrictions
	if(m_bLockToXAxis)
	{
		m_yPos = m_yPrev;
	}
	else
	if(m_bLockToYAxis)
	{
		m_xPos = m_xPrev;
	}

	if (m_xPos < m_rcClip.left)	{ m_xPos = m_rcClip.left; }
	if (m_xPos > m_rcClip.right) { m_xPos = m_rcClip.right; }
	if (m_yPos < m_rcClip.top) { m_yPos = m_rcClip.top;	}
	if (m_yPos > m_rcClip.bottom) { m_yPos = m_rcClip.bottom; }
}

void CMouseMgr::Draw(HSURFACE hSurf, int xOffset, int yOffset)
{
    //g_pLTClient->DrawSurfaceToSurfaceTransparent(hSurf, g_pPrClientShell->GetCursorSurf(), NULL, m_xPos + xOffset, m_yPos + yOffset, g_hColorTransparent);
}
