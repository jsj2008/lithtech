// SliderCtrl.cpp: implementation of the CSliderCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SliderCtrl.h"
#include "InterfaceResMgr.h"


char CSliderCtrl::m_sLeftArrow[64] = "interface\\slider_left.pcx";
char CSliderCtrl::m_sLeftArrowH[64] = "interface\\slider_left_h.pcx";
char CSliderCtrl::m_sLeftArrowD[64] = "interface\\slider_left_d.pcx";
char CSliderCtrl::m_sFullBar[64] = "interface\\slider_full.pcx";
char CSliderCtrl::m_sEmptyBar[64] = "interface\\slider_empty.pcx";
char CSliderCtrl::m_sDisBar[64] = "interface\\slider_disabled.pcx";
char CSliderCtrl::m_sRightArrow[64] = "interface\\slider_right.pcx";
char CSliderCtrl::m_sRightArrowH[64] = "interface\\slider_right_h.pcx";
char CSliderCtrl::m_sRightArrowD[64] = "interface\\slider_right_d.pcx";

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSliderCtrl::CSliderCtrl()
{
	m_hText = LTNULL;
	m_pFont = LTNULL;
    m_bNumDisplay = LTFALSE;
	m_pnValue = LTNULL;
	m_nMinSlider = 0;
	m_nMaxSlider = 100;
	m_nSliderPos = 50;
	m_nSliderIncrement = 10;
	m_bOverLeft = LTFALSE;
	m_bOverRight = LTFALSE;
	m_dwHeight	= 0;
	m_dwWidth = 0;
	m_nBarWidth = 0;

	m_rcLeft.Init(0,0,0,0);
	m_rcBar.Init(0,0,0,0);
	m_rcRight.Init(0,0,0,0);
	m_rcSrcFull.Init(0,0,0,0);
	m_rcSrcEmpty.Init(0,0,0,0);
}

CSliderCtrl::~CSliderCtrl()
{

}


LTBOOL CSliderCtrl::Create ( HSTRING hText, CLTGUIFont *pFont, int nSliderOffset,
                              int nSliderWidth, LTBOOL bNumDisplay, int *pnValue)
{
	m_pClientDE = g_pLTClient;
	if ( hText && g_pLTClient)
	{
		m_hText=m_pClientDE->CopyString(hText);
	}
	m_pFont	= pFont;
    m_bNumDisplay = bNumDisplay;
	m_pnValue = pnValue;

	m_dwHeight = m_pFont->GetHeight() + 4;
	m_dwWidth  = nSliderOffset + nSliderWidth;


	//get arrow dimensions
	HSURFACE hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sLeftArrow);
	if (!hSurf) return LTFALSE;
	uint32 surfWidth = 0;
	uint32 surfHeight = 0;

	g_pLTClient->GetSurfaceDims(hSurf, &surfWidth, &surfHeight);
	if (surfHeight > m_dwHeight)
		m_dwHeight = surfHeight;

	//set left arrow rect
	m_rcLeft.left = nSliderOffset;
	m_rcLeft.right = nSliderOffset + surfWidth;
	if (surfHeight < m_dwHeight)
		m_rcLeft.top = (m_dwHeight-surfHeight)/2;
	else
		m_rcLeft.top = 0;
	m_rcLeft.bottom = m_rcLeft.top + surfHeight;

	//set right arrow rect
	m_rcRight.right = nSliderOffset + nSliderWidth;
	m_rcRight.left = m_rcRight.right - surfWidth;
	if (surfHeight < m_dwHeight)
		m_rcRight.top = (m_dwHeight-surfHeight)/2;
	else
		m_rcRight.top = 0;
	m_rcRight.bottom = m_rcRight.top + surfHeight;

	//set basic bar rect
	m_rcBar.left = m_rcLeft.right + 8;
	m_rcBar.right = m_rcRight.left - 8;
	m_rcBar.top = 2;
	m_rcBar.bottom = m_dwHeight - 2;

	//calculate width of bar
	m_nBarWidth = m_rcBar.right - m_rcBar.left;


	//calculate source rects for bars
	hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sFullBar);
	g_pLTClient->GetSurfaceDims(hSurf, &surfWidth, &surfHeight);
	m_rcSrcFull.right = (int)surfWidth - 1;
	m_rcSrcFull.bottom = (int)surfHeight - 1;
	// Optimize it to avoid some 2d/3d interaction issues
	g_pLTClient->OptimizeSurface(hSurf, LTNULL);

	hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sEmptyBar);
	g_pLTClient->GetSurfaceDims(hSurf, &surfWidth, &surfHeight);
	m_rcSrcEmpty.right = (int)surfWidth - 1;
	m_rcSrcEmpty.bottom = (int)surfHeight - 1;
	// Optimize it to avoid some 2d/3d interaction issues
	g_pLTClient->OptimizeSurface(hSurf, LTNULL);

	// Optimize the disabled bar to avoid some 2d/3d interaction issues
	hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sDisBar);
	g_pLTClient->OptimizeSurface(hSurf, LTNULL);

	return LTTRUE;
}

// Update data
void CSliderCtrl::UpdateData(LTBOOL bSaveAndValidate)
{
	//sanity check
	if (!m_pnValue)
	{
		return;
	}

	if (bSaveAndValidate)
	{
		*m_pnValue=GetSliderPos();
	}
	else
	{
		SetSliderPos(*m_pnValue);
	}
}

// Sets the min and max of the slider bar
void CSliderCtrl::SetSliderRange(int nMin, int nMax)
{
	m_nMinSlider = nMin;
	m_nMaxSlider = nMax;

	// Call this to make sure that the slider stays within range
	SetSliderPos(m_nSliderPos);
}

// Sets the slider position
void CSliderCtrl::SetSliderPos(int nPos)
{
	m_nSliderPos=nPos;
	if ( m_nSliderPos > m_nMaxSlider )
	{
		m_nSliderPos=m_nMaxSlider;
	}
	if ( m_nSliderPos < m_nMinSlider )
	{
		m_nSliderPos=m_nMinSlider;
	}
}


// Handle the left command
LTBOOL CSliderCtrl::OnLeft()
{
	int nPos = m_nSliderPos-m_nSliderIncrement;
	if ( nPos < m_nMinSlider )
	{
		nPos=m_nMinSlider;
	}
	if (nPos != m_nSliderPos)
	{
		SetSliderPos(nPos);
		g_pInterfaceMgr->RequestInterfaceSound(IS_LEFT);
        return LTTRUE;
	}
	g_pInterfaceMgr->RequestInterfaceSound(IS_NO_SELECT);
    return LTFALSE;
}

// Handle the right command
LTBOOL CSliderCtrl::OnRight()
{
	int nPos = m_nSliderPos+m_nSliderIncrement;
	if ( nPos > m_nMaxSlider )
	{
		nPos = m_nMaxSlider;
	}
	if (nPos != m_nSliderPos)
	{
		SetSliderPos(nPos);
		g_pInterfaceMgr->RequestInterfaceSound(IS_RIGHT);
        return LTTRUE;
	}
	g_pInterfaceMgr->RequestInterfaceSound(IS_NO_SELECT);
    return LTFALSE;
}

void CSliderCtrl::OnSelChange()
{
	if (!IsSelected())
	{
		m_bOverLeft = LTFALSE;
		m_bOverRight = LTFALSE;
	}
}

// Destroys the control
void CSliderCtrl::Destroy ( )
{
	if ( m_hText && m_pClientDE)
	{
		m_pClientDE->FreeString(m_hText);
        m_hText=LTNULL;
	}
}

// Render the control
void CSliderCtrl::Render ( HSURFACE hDestSurf )
{
	HLTCOLOR color = GetCurrentColor();
	int xPos = m_pos.x;
	int yPos = m_pos.y + 2;
	m_pFont->Draw(m_hText, hDestSurf, xPos, yPos, LTF_JUSTIFY_LEFT, color);

	
	HSURFACE hSurf = LTNULL;
	if (!m_bEnabled)
		hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sLeftArrowD);
	else if (m_bOverLeft)
		hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sLeftArrowH);
	else
		hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sLeftArrow);

	if (!hSurf) return;
	xPos = m_pos.x + m_rcLeft.left;
	yPos = m_pos.y + m_rcLeft.top;
    g_pLTClient->DrawSurfaceToSurfaceTransparent(hDestSurf, hSurf, LTNULL, xPos, yPos, m_hTransColor);

	if (!m_bEnabled)
		hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sRightArrowD);
	else if (m_bOverRight)
		hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sRightArrowH);
	else
		hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sRightArrow);
	if (!hSurf) return;
	xPos = m_pos.x + m_rcRight.left;
	yPos = m_pos.y + m_rcRight.top;
    g_pLTClient->DrawSurfaceToSurfaceTransparent(hDestSurf, hSurf, LTNULL, xPos, yPos, m_hTransColor);

	// Calculate the position for the tab
	float fPercentage = 1.0f;
	if (m_bEnabled)
		fPercentage=(float)(m_nSliderPos-m_nMinSlider)/(float)(m_nMaxSlider-m_nMinSlider);
	int pos = (int)(fPercentage * (float)m_nBarWidth);

	LTRect rcTemp = m_rcBar;
	rcTemp.left += (m_pos.x - 1); 
	rcTemp.top += (m_pos.y - 1); 
	rcTemp.bottom += (m_pos.y + 1); 
	rcTemp.right += (m_pos.x + 1); 
	g_pLTClient->FillRect(hDestSurf, &rcTemp, LTNULL);

	rcTemp = m_rcBar;
	rcTemp.left += m_pos.x; 
	rcTemp.top += m_pos.y; 
	rcTemp.bottom += m_pos.y; 
	rcTemp.right = rcTemp.left + pos;
	if (fPercentage > 0.0f)
	{
		if (m_bEnabled)
			hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sFullBar);
		else
			hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sDisBar);
		if (!hSurf) return;
		g_pLTClient->OptimizeSurface(hSurf, LTNULL);
		g_pLTClient->ScaleSurfaceToSurface(hDestSurf, hSurf, &rcTemp, &m_rcSrcFull);
	}


	if (fPercentage < 1.0f)
	{
		rcTemp.left = rcTemp.right;
		rcTemp.right = m_rcBar.right+m_pos.x;
		hSurf = g_pInterfaceResMgr->GetSharedSurface(m_sEmptyBar);
		if (!hSurf) return;
		g_pLTClient->OptimizeSurface(hSurf, LTNULL);
		g_pLTClient->ScaleSurfaceToSurface(hDestSurf, hSurf, &rcTemp, &m_rcSrcEmpty);

		rcTemp.right = rcTemp.left + 1;
		g_pLTClient->FillRect(hDestSurf, &rcTemp, LTNULL);
	}


	if (m_bNumDisplay)
	{
		char szVal[16];
		sprintf(szVal,"%d",m_nSliderPos);
		xPos = m_pos.x + m_rcBar.left + m_nBarWidth/2;
		yPos = m_pos.y + m_rcBar.top;
		m_pFont->Draw(szVal, hDestSurf, xPos, yPos, LTF_JUSTIFY_CENTER, color);
	}


}

LTBOOL CSliderCtrl::OnMouseMove(int x, int y)
{
	int tX = x - m_pos.x;
	int tY = y - m_pos.y;
	m_bOverLeft = (tX >= m_rcLeft.left && tY >= m_rcLeft.top && tX <= m_rcLeft.right && tY <= m_rcLeft.bottom);
	m_bOverRight = (tX >= m_rcRight.left && tY >= m_rcRight.top && tX <= m_rcRight.right && tY <= m_rcRight.bottom);
	return (m_bOverLeft || m_bOverRight);
}


LTBOOL CSliderCtrl::OnLButtonUp(int x, int y)
{
	if (m_bOverLeft) return OnLeft();
	if (m_bOverRight) return OnRight();
	return LTFALSE;

}
