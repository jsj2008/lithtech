// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUISlider.cpp
//
// PURPOSE : Control to display a full/empty style slider bar.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "ltguislider.h"


namespace
{
	enum eBarElements
	{
		eFull = 0,
		eEmpty,
		eLeft,
		eRight
	};
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUISlider::CLTGUISlider()
{
	m_nBaseFontSize = 0;

    m_bDisplay = false;
	m_pnValue = NULL;
	m_nMinSlider = 0;
	m_nMaxSlider = 10;
	m_nSliderPos = 0;
	m_nSliderIncrement = 1;

	m_nArrowWidth = 0;
	m_nArrowHeight = 0;

	m_bOverLeft = false;
	m_bOverRight = false;

	m_bDisplayOnly = false;

	m_pCallback = NULL;
	m_pTextCallback = NULL;
	m_pTextCallBackUserData = NULL;

}

CLTGUISlider::~CLTGUISlider()
{
	Destroy();
}


bool CLTGUISlider::Create(const wchar_t *pText, const CFontInfo& Font, const CLTGUISlider_create& cs)
{
	if (!cs.hBarTexture || !cs.hBarTextureDisabled) 
	{
		LTERROR( "Invalid slider textures!" );
		return false;
	}
	m_hBar = cs.hBarTexture;
	m_hBarDisabled = cs.hBarTextureDisabled;

	m_pnValue = cs.pnValue;

	if (!SetFont(Font))
	{
        return false;
	}

	m_nMinSlider = cs.nMin;
	m_nMaxSlider = cs.nMax;
	m_nSliderIncrement=cs.nIncrement;

	m_nBarHeight = cs.nBarHeight;
	m_nBarOffset = cs.nBarOffset;

	m_nArrowWidth = m_nBarHeight;
	m_nArrowHeight = m_nBarHeight;

	m_nDisplayWidth = cs.nDisplayWidth;
	m_nBarWidth = cs.rnBaseRect.GetWidth() - (m_nBarOffset + m_nDisplayWidth);

	if ( pText)
	{
		m_Text.SetText(pText);
	}

	InitBar();

	CLTGUICtrl::Create( (CLTGUICtrl_create)cs );

	if (cs.pnTextCallback)
		SetTextCallback(cs.pnTextCallback,cs.pUserData);
	else if (cs.bNumericDisplay)
		SetNumericDisplay(cs.bNumericDisplay);

	m_Text.SetGlowParams(cs.bGlowEnable,cs.fGlowAlpha,cs.vGlowSize);
	m_Display.SetGlowParams(cs.bGlowEnable,cs.fGlowAlpha,cs.vGlowSize);



	return true;
}

void CLTGUISlider::SetNumericDisplay(bool bDisplay) 
{ 
	if (m_pTextCallback)
		bDisplay = true;
	m_bDisplay = bDisplay;
	if (bDisplay)
	{
		UpdateDisplay();
		ScaleBar();
	}
	else
	{
		m_Display.FlushTexture();
	}
}


// Update data
void CLTGUISlider::UpdateData(bool bSaveAndValidate)
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
void CLTGUISlider::SetSliderRange(int nMin, int nMax)
{
	if (nMax <= nMin) return;

	m_nMinSlider = nMin;
	m_nMaxSlider = nMax;

	// Call this to make sure that the slider stays within range
	SetSliderPos(m_nSliderPos);
}


// Sets the slider position
void CLTGUISlider::SetSliderPos(int nPos)
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

	if (m_bDisplay)
	{
		UpdateDisplay();
	}

	ScaleBar();
}


// Handle the left command
bool CLTGUISlider::OnLeft()
{
	if (m_bDisplayOnly) return false;

	int nPos = m_nSliderPos-m_nSliderIncrement;

	if ( nPos < m_nMinSlider )
	{
		nPos=m_nMinSlider;
	}
	if (nPos != m_nSliderPos)
	{
		if (m_pCallback && !m_pCallback(this,nPos,m_nSliderPos))
			return false;

		SetSliderPos(nPos);
        return true;
	}
	return false;
}

// Handle the right command
bool CLTGUISlider::OnRight()
{
	if (m_bDisplayOnly) return false;

	int nPos = m_nSliderPos+m_nSliderIncrement;
	if ( nPos > m_nMaxSlider )
	{
		nPos = m_nMaxSlider;
	}
	if (nPos != m_nSliderPos)
	{
		if (m_pCallback && !m_pCallback(this,nPos,m_nSliderPos))
			return false;
		SetSliderPos(nPos);
       return true;
	}
    return false;
}

void CLTGUISlider::OnSelChange()
{
	if (!IsSelected())
	{
		m_bOverLeft = false;
		m_bOverRight = false;
	}
}

// Destroys the control
void CLTGUISlider::Destroy ( )
{
	m_Text.FlushTexture();
	m_Display.FlushTexture();
}

// Render the control
void CLTGUISlider::Render ()
{

	m_Text.SetColor(GetCurrentColor());
	m_Text.SetGlow(IsSelected());
	m_Text.Render();

	g_pDrawPrim->SetTexture(m_bEnabled?m_hBar:m_hBarDisabled);

	// set up the render state	
	SetRenderState();


	//	draw bar

	if (m_nSliderPos > m_nMinSlider)
	{
		g_pDrawPrim->SetTexture(m_bEnabled?m_hBar:m_hBarDisabled);
		g_pDrawPrim->DrawPrim(&m_Bar[eFull]);
	}

	if (m_nSliderPos < m_nMaxSlider)
	{
		g_pDrawPrim->SetTexture(m_bEnabled?m_hBar:m_hBarDisabled);
		g_pDrawPrim->DrawPrim(&m_Bar[eEmpty]);
	}

	//	draw arrows
	if (!m_bDisplayOnly)
	{
		if (m_bOverLeft)
			g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Additive);
		else
			g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
		g_pDrawPrim->SetTexture(m_bEnabled?m_hBar:m_hBarDisabled);
		g_pDrawPrim->DrawPrim(&m_Bar[eLeft]);

		if (m_bOverRight)
			g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Additive);
		else
			g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
		g_pDrawPrim->SetTexture(m_bEnabled?m_hBar:m_hBarDisabled);
		g_pDrawPrim->DrawPrim(&m_Bar[eRight]);
	}	


	//	Draw Numbers
	if (m_bDisplay)
	{
		m_Display.SetColor(GetCurrentColor());
		m_Display.SetGlow(IsSelected());
		m_Display.Render();
	}


}



// Render the control
void CLTGUISlider::RenderTransition(float fTrans)
{

	m_Text.SetColor(GetCurrentColor());
	m_Text.RenderTransition(fTrans);

}


bool CLTGUISlider::OnMouseMove(int x, int y)
{
	bool bWasOverLeft  = m_bOverLeft;
	bool bWasOverRight = m_bOverRight;

	LTVector2n vMousePos( x, y );

	m_bOverLeft = m_rcLeft.Contains(vMousePos);
	m_bOverRight = m_rcRight.Contains(vMousePos);

	//determine if we have changed controls
	return ((!bWasOverLeft && m_bOverLeft) || 
			(!bWasOverRight && m_bOverRight));
}


bool CLTGUISlider::OnLButtonUp(int x, int y)
{
	if (m_bDisplayOnly) return false;
	if (m_bOverLeft) return OnLeft();
	if (m_bOverRight) return OnRight();

	LTVector2n vMousePos( x, y );
	if (m_rcBar.Contains( vMousePos ))
	{
		float fPercent = (float)(x - m_rcBar.Left()) / (float)(m_rcBar.Right() - m_rcBar.Left());
		int nPos = m_nSliderIncrement * (int)((fPercent * (float)(m_nMaxSlider - m_nMinSlider) / (float)m_nSliderIncrement) + 0.5f);
		if (nPos+m_nMinSlider != m_nSliderPos)
		{
			if (m_pCallback && !m_pCallback(this,(nPos+m_nMinSlider),m_nSliderPos))
				return false;

			SetSliderPos(nPos+m_nMinSlider);
			return true;
		}

	}
	return false;

}

void CLTGUISlider::SetBasePos(const LTVector2n& pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	ScaleBar();

	LTVector2 vPos = m_rfRect.m_vMin;
	m_Text.SetPos(vPos);

	vPos.x = m_rfRect.Right() + (m_vfScale.x * (float)m_nArrowWidth) - (m_vfScale.x * float(m_nDisplayWidth));
	m_Display.SetPos(vPos);
}

void CLTGUISlider::SetSize(const LTVector2n& sz )
{ 
	CLTGUICtrl::SetSize(sz);

	m_nBarWidth = m_rnBaseRect.GetWidth() - m_nBarOffset;
	ScaleBar();

	LTVector2 vPos = m_rfRect.m_vMin;
	vPos.x = m_rfRect.Right() + (m_vfScale.x * (float)m_nArrowWidth) - (m_vfScale.x * float(m_nDisplayWidth));
	m_Display.SetPos(vPos);
}


void CLTGUISlider::SetScale(const LTVector2& vfScale)
{
	CLTGUICtrl::SetScale(vfScale);
	ScaleBar();

	LTVector2 vPos = m_rfRect.m_vMin;
	m_Text.SetPos(vPos);

	vPos.x = m_rfRect.Right() + (m_vfScale.x * (float)m_nArrowWidth) - (m_vfScale.x * float(m_nDisplayWidth));
	m_Display.SetPos(vPos);

	m_Text.SetFontHeight((uint32)(m_vfScale.y * (float)m_nBaseFontSize));
	m_Display.SetFontHeight((uint32)(m_vfScale.y * (float)m_nBaseFontSize));
}


void CLTGUISlider::InitBar()
{
	ScaleBar();
	SetupQuadUVs(m_Bar[eFull], m_hBar, 0.0f,0.0f,0.5f,0.5f);
	SetupQuadUVs(m_Bar[eEmpty], m_hBar, 0.5f,0.0f,0.5f,0.5f);
	SetupQuadUVs(m_Bar[eLeft], m_hBar, 0.0f,0.5f,0.5f,0.5f);
	SetupQuadUVs(m_Bar[eRight], m_hBar, 0.5f,0.5f,0.5f,0.5f);

	for (int i = 0; i < 4; i++) 
		DrawPrimSetRGBA(m_Bar[i], 0xFF, 0xFF, 0xFF, 0xFF);

}

void CLTGUISlider::ScaleBar()
{
	//determine height of bar
	uint32 nHt = m_nBaseFontSize;
	if (m_nBarHeight > nHt) nHt = m_nBarHeight;
	if (m_nArrowHeight > nHt) nHt = m_nArrowHeight;

	float fHt = (float)nHt * m_vfScale.y;

	float fPercent = (float)(m_nSliderPos - m_nMinSlider) / (float)(m_nMaxSlider - m_nMinSlider);

	//left arrow
	float x = m_rfRect.Left() + ((float)m_nBarOffset * m_vfScale.x);
	float y = m_rfRect.Top();
	float fw = (float)m_nArrowWidth * m_vfScale.x;
	float fh = (float)m_nArrowHeight * m_vfScale.y;

	float yo = (fHt - fh) / 2.0f;
	float arrowGap = fw / 4.0f;
	float arrowSpace = fw + arrowGap;

	if (m_bDisplayOnly)
	{
		arrowGap = 0.0f;
		arrowSpace = 0.0f;
	}
	float barWidth = ((float)m_nBarWidth * m_vfScale.x) - 2 * arrowSpace;

	DrawPrimSetXYWH(m_Bar[eLeft],x,y + yo,fw,fh);

	m_rcLeft.Left() = (int)x;
	m_rcLeft.Top() = (int)(y + yo);
	m_rcLeft.Right() = (int)(x + fw);
	m_rcLeft.Bottom() = m_rcLeft.Top() + (int)fh;


	//full part of bar
	x += arrowSpace;
	fw = fPercent * barWidth;
	fh = (float)m_nBarHeight * m_vfScale.y;
	yo = ((float)fHt - fh) / 2.0f;
	DrawPrimSetXYWH(m_Bar[eFull],x,y+yo,fw,fh);
	m_rcBar.Left() = (int)x;
	m_rcBar.Top() = (int)(y + yo);

	//empty part of bar
	x += fw;
	fw = barWidth * (1.0f - fPercent);
	DrawPrimSetXYWH(m_Bar[eEmpty],x,y+yo,fw,fh);
	m_rcBar.Right() = (int)(x + fw);
	m_rcBar.Bottom() = m_rcBar.Top() + (int)fh;

	//right arrow
	fw = (float)m_nArrowWidth * m_vfScale.x;
	fh = (float)m_nArrowHeight * m_vfScale.y;
	x = m_rfRect.Right() - fw;
	x -= (float(m_nDisplayWidth) * m_vfScale.x);
	yo = (fHt - fh) / 2.0f;
	DrawPrimSetXYWH(m_Bar[eRight],x,y+yo,fw,fh);

	m_rcRight.Left() = (int)x;
	m_rcRight.Top() = (int)(y + yo);
	m_rcRight.Right() = (int)(x + fw);
	m_rcRight.Bottom() = m_rcRight.Top() + (int)fh;



}

void CLTGUISlider::SetRenderState()
{
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
}


//used to calculate the x offset of the slider at a given position
float CLTGUISlider::CalculateSliderOffset(int pos)
{

	float fPercent = (float)(pos - m_nMinSlider) / (float)(m_nMaxSlider - m_nMinSlider);

	//left arrow
	float x = ((float)m_nBarOffset * m_vfScale.x);
	float fw = (float)m_nArrowWidth * m_vfScale.x;

	float arrowGap = fw / 4.0f;
	float arrowSpace = fw + arrowGap;
	float barWidth = ((float)m_nBarWidth * m_vfScale.x) - 2 * arrowSpace;

	//full part of bar
	x += arrowSpace;
	x += fPercent * barWidth;

	return x;
}

// Set the font
bool CLTGUISlider::SetFont(const CFontInfo& Font)
{
	if (!Font.m_nHeight)
	{
		return false;
	}

	m_nBaseFontSize = Font.m_nHeight;

	CFontInfo tmpFont = Font;
	tmpFont.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

	m_Text.SetFont(tmpFont);
	m_Display.SetFont(tmpFont);

	return true;
}

// Set the font
bool CLTGUISlider::SetFontHeight(uint32 nFontHeight)
{
	if (!nFontHeight)
	{
		return false;
	}

	m_nBaseFontSize = nFontHeight;

	uint32 nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

	m_Text.SetFontHeight(nHeight);
	m_Display.SetFontHeight(nHeight);

	return true;
}

// for controls to display a value, but not accept input
void CLTGUISlider::SetDisplayOnly(bool bDisplayOnly)
{
	m_bDisplayOnly = bDisplayOnly;
	ScaleBar();
}



// free texture memory by flushing any texture strings owned by the control
void CLTGUISlider::FlushTextureStrings()
{
	m_Text.FlushTexture();
	m_Display.FlushTexture();
}

// rebuild any texture strings owned by the control
void CLTGUISlider::RecreateTextureStrings()
{
	m_Text.CreateTexture();
	m_Display.CreateTexture();
}

//note: if a text callback is specified, numeric display option is overridden
void CLTGUISlider::SetTextCallback(SliderTextCallBackFn pCallback, void* pUserData) 
{
	m_pTextCallback = pCallback; 
	m_pTextCallBackUserData = pUserData;
	m_bDisplay = (pCallback != NULL);
}


void CLTGUISlider::UpdateDisplay()
{
	if (m_pTextCallback)
	{
		m_Display.SetText(m_pTextCallback(m_nSliderPos, m_pTextCallBackUserData));
	}
	else
	{
		wchar_t wszNum[16];
		swprintf(wszNum,L"%d",m_nSliderPos);
		m_Display.SetText(wszNum);
	}
}