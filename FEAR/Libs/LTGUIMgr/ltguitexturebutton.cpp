// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUITextureButton.cpp
//
// PURPOSE : button control with three states (normal, selected, and disabled)
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "LTGUITextureButton.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUITextureButton::CLTGUITextureButton() :
	m_bCenterImage(false),
	m_bCenterText(true)
{

	m_hNormal   = NULL;
	m_hSelected   = NULL;
	m_hDisabled   = NULL;

	m_bHighlightText = false;

	m_nBaseFontSize		= 0;
	m_eAlignment		= kCenter;
}


CLTGUITextureButton::~CLTGUITextureButton()
{
	Destroy();
}

// Create the control
bool CLTGUITextureButton::Create ( const CLTGUITextureButton_create& cs)
{
	if (!cs.hNormal) return false;

	SetTexture(cs.hNormal,cs.hSelected,cs.hDisabled);


	CLTGUICtrl::Create( (CLTGUICtrl_create)cs );

	m_rnImageRect = cs.rnImageRect;
	m_rnTextRect = cs.rnTextRect;
	m_bCenterImage = cs.bCenterImage;
	m_bCenterText = cs.bCenterText;

	m_Text.SetGlowParams(cs.bGlowEnable,cs.fGlowAlpha,cs.vGlowSize);

	InitPoly();

    return true;
}

// Set the font
bool CLTGUITextureButton::SetFont(const CFontInfo& Font)
{
	if (!Font.m_nHeight)
	{
		return false;
	}


	m_nBaseFontSize = Font.m_nHeight;

	CFontInfo tmpFont = Font;
	tmpFont.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

	m_Text.SetFont(tmpFont);

	return true;
}

// Set the font
bool CLTGUITextureButton::SetFontHeight(uint32 nFontHeight)
{
	if (!nFontHeight)
	{
		return false;
	}

	m_nBaseFontSize = nFontHeight;
	m_Text.SetFontHeight( (uint32)(m_vfScale.y * (float)m_nBaseFontSize) );


	return true;
}

bool CLTGUITextureButton::SetText(const wchar_t *pString, bool bHighlightText)
{
	if ( !pString )
		return false;

	m_Text.SetText(pString);
	m_Text.SetAlignment(m_eAlignment);

	m_bHighlightText = bHighlightText;

	return true;
}


void CLTGUITextureButton::SetTexture(HTEXTURE hNormal, HTEXTURE hSelected, HTEXTURE hDisabled)
{
	m_hNormal	= hNormal;
	m_hSelected = hSelected;
	m_hDisabled = hDisabled;

}

// Render the control
void CLTGUITextureButton::Render()
{
	// Sanity checks...
	if (!IsVisible()) return;

	HTEXTURE hCurrTex;
	if (IsSelected() && m_hSelected)
		hCurrTex = m_hSelected;
	else if (IsDisabled() && m_hDisabled)
		hCurrTex = m_hDisabled;
	else
		hCurrTex = m_hNormal;

	//setup the texture
	g_pDrawPrim->SetTexture(hCurrTex);

	//setup the UV coordinates on our quad, since it is dependant upon
	//the texture for correct filtering
	SetupQuadUVs(m_Poly, hCurrTex, 0.0f, 0.0f, 1.0f, 1.0f);

	// set up the render state	
	SetRenderState();

	// draw our button
	g_pDrawPrim->DrawPrim(&m_Poly);

	uint32 argbColor = m_argbNormal;
	if (m_bHighlightText)
		argbColor = GetCurrentColor();

	m_Text.SetColor(argbColor);
	m_Text.SetGlow(IsSelected());

	m_Text.Render();
}

// Render the control
void CLTGUITextureButton::RenderTransition(float fTrans)
{
	// Sanity checks...
	if (!IsVisible()) return;

	g_pDrawPrim->SetTexture(m_hNormal);

	//setup the UV coordinates on our quad, since it is dependant upon
	//the texture for correct filtering
	SetupQuadUVs(m_Poly, m_hNormal, 0.0f, 0.0f, 1.0f, 1.0f);

	// set up the render state	
	SetRenderState();

	// draw our button
	g_pDrawPrim->DrawPrim(&m_Poly);

	m_Text.SetColor(m_argbNormal);
	m_Text.RenderTransition(fTrans);
}




// Enter was pressed
bool CLTGUITextureButton::OnEnter ( )
{
	// Send the command
	if ( m_pCommandHandler && m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2) )
		return true;
    return false;
}


void CLTGUITextureButton::SetBasePos ( const LTVector2n& pos )
{ 
	LTVector2n oldPos = m_rnBaseRect.m_vMin;
	CLTGUICtrl::SetBasePos(pos);

	LTVector2n offset = pos - oldPos;
	m_rnImageRect.m_vMin += offset;
	m_rnImageRect.m_vMax += offset;
	m_rnTextRect.m_vMin += offset;
	m_rnTextRect.m_vMax += offset;

	ScalePoly();
}

void CLTGUITextureButton::SetScale(const LTVector2& vfScale)
{

	bool bRebuild = (vfScale != m_vfScale);

	CLTGUICtrl::SetScale(vfScale);

	ScalePoly();

	if (bRebuild)
		m_Text.SetFontHeight((uint32)(m_vfScale.y * (float)m_nBaseFontSize));

}

void CLTGUITextureButton::SetSize(const LTVector2n& sz)
{
	CLTGUICtrl::SetSize(sz);
	ScalePoly();
}

void CLTGUITextureButton::InitPoly()
{
	ScalePoly();
	DrawPrimSetUVWH(m_Poly,0.0f,0.0f,1.0f,1.0f);
	DrawPrimSetRGBA(m_Poly, 0xFF, 0xFF, 0xFF, 0xFF);
}

void CLTGUITextureButton::ScalePoly()
{
	if (!m_hNormal) return;

	LTRect2f rfRect;

	if( m_bCenterImage )
	{
		int32 nWidth = m_rnImageRect.GetWidth();
		int32 nHeight = m_rnImageRect.GetHeight();

		int32 nBaseWidth = (int32)m_rnBaseRect.GetWidth();
		int32 nBaseHeight = (int32)m_rnBaseRect.GetHeight();

		m_rnImageRect.Left() = m_rnBaseRect.Left() + (nBaseWidth - nWidth)/2;
		m_rnImageRect.Top() = m_rnBaseRect.Top() + (nBaseHeight - nHeight)/2;
		m_rnImageRect.Right() = m_rnImageRect.Left() + nWidth;
		m_rnImageRect.Bottom() = m_rnImageRect.Top() + nHeight;
	}

	if (m_rnImageRect.GetWidth() > 0)
	{
		rfRect.m_vMin.x = (m_vfScale.x * (float)m_rnImageRect.m_vMin.x);
		rfRect.m_vMin.y = (m_vfScale.y * (float)m_rnImageRect.m_vMin.y);
		rfRect.m_vMax.x = (m_vfScale.x * (float)m_rnImageRect.m_vMax.x);
		rfRect.m_vMax.y = (m_vfScale.y * (float)m_rnImageRect.m_vMax.y);
	}
	else
	{
		rfRect = m_rfRect;
	}

	float fw = rfRect.GetWidth();
	float fh = rfRect.GetHeight();
	DrawPrimSetXYWH(m_Poly,rfRect.m_vMin.x,rfRect.m_vMin.y,fw,fh);

	if (m_rnTextRect.GetWidth() > 0)
	{
		rfRect.m_vMin.x = (m_vfScale.x * (float)m_rnTextRect.m_vMin.x);
		rfRect.m_vMin.y = (m_vfScale.y * (float)m_rnTextRect.m_vMin.y);
		rfRect.m_vMax.x = (m_vfScale.x * (float)m_rnTextRect.m_vMax.x);
		rfRect.m_vMax.y = (m_vfScale.y * (float)m_rnTextRect.m_vMax.y);
	}
	else
	{
		rfRect = m_rfRect;
	}

	LTVector2 vPos;
	float th = (float)m_Text.GetFontHeight();
	fh = rfRect.GetHeight();
	vPos.y = rfRect.m_vMin.y;
	if (m_bCenterText)
	{
		vPos.y += (fh - th )/ 2.0f;
	}

	switch(m_Text.GetAlignment()) 
	{
	case kLeft:
	default:
		vPos.x = rfRect.m_vMin.x;
		break;
	case kRight:
		vPos.x = rfRect.Right();
		break;
	case kCenter:
		vPos.x = (rfRect.Left() + rfRect.Right()) / 2;
		break;
	}

	m_Text.SetPos(vPos);

}

void CLTGUITextureButton::SetRenderState()
{
	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
}

// free texture memory by flushing any texture strings owned by the control
void CLTGUITextureButton::FlushTextureStrings()
{
	m_Text.FlushTexture();
}

// rebuild any texture strings owned by the control
void CLTGUITextureButton::RecreateTextureStrings()
{
	if (m_Text.IsEmpty())
		return;
	m_Text.CreateTexture();
}

void CLTGUITextureButton::SetAlignment(eTextAlign align) 
{
	m_eAlignment = align;
	m_Text.SetAlignment(align);
	ScalePoly();
}