// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIButton.cpp
//
// PURPOSE : button control with three states (normal, selected, and disabled)
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "ltguibutton.h"

/*
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIButton::CLTGUIButton()
{

	m_hNormal   = NULL;
	m_hSelected   = NULL;
	m_hDisabled   = NULL;

	m_nBaseWidth			= 0;
	m_nBaseHeight			= 0;

    m_pCommandHandler   = NULL;

	m_bHighlightText = false;

	m_nBaseFontSize		= 0;


	memset(&m_Poly,0,sizeof(m_Poly));
}


CLTGUIButton::~CLTGUIButton()
{
	Destroy();
}

// Create the control
bool CLTGUIButton::Create ( uint32 nCommandID, uint32 nHelpID, uint32 nWidth, uint32 nHeight,
					   HTEXTURE hNormal, HTEXTURE hSelected, HTEXTURE hDisabled, 
					   CLTGUICommandHandler *pCommandHandler, uint32 nParam1, uint32 nParam2)
{
	if (!hNormal) return false;

	m_pCommandHandler	= pCommandHandler;

	SetTexture(hNormal,hSelected,hDisabled);

	m_nBaseWidth = nWidth;
	m_nBaseHeight = nHeight;

	InitPoly();

	CLTGUICtrl::Create(nCommandID, nHelpID, nParam1,nParam2);

    return true;
}

// Set the font
bool CLTGUIButton::SetFont(const CFontInfo& Font)
{
	if (!Font.m_nHeight)
	{
		return false;
	}

	m_Font.SetStyle(Font.m_nStyle);
	LTStrCpy(m_Font.m_szTypeface,Font.m_szTypeface,LTARRAYSIZE(m_Font.m_szTypeface));
	m_nBaseFontSize = Font.m_nHeight;

	m_Font.m_nHeight = (uint32)(m_fScale * (float)m_nBaseFontSize);

	RecreateTextureStrings();

    return true;
}

bool CLTGUIButton::SetText(const wchar_t *pString, bool bHighlightText)
{
	if ( !pString || !m_Font.m_nHeight)
		return false;

	m_Text.Set(pString,m_Font);
	m_bHighlightText = bHighlightText;

	RecreateTextureStrings();

	return true;
}


void CLTGUIButton::SetTexture(HTEXTURE hNormal, HTEXTURE hSelected, HTEXTURE hDisabled, bool bFreeOld)
{
	if (bFreeOld)
	{
		if (m_hNormal && m_hNormal != hNormal)
			g_pTexInterface->ReleaseTextureHandle(m_hNormal);
		if (m_hSelected && m_hSelected != hSelected)
			g_pTexInterface->ReleaseTextureHandle(m_hSelected);
		if (m_hDisabled && m_hDisabled != hDisabled)
			g_pTexInterface->ReleaseTextureHandle(m_hDisabled);
	}

	m_hNormal	= hNormal;
	m_hSelected = hSelected;
	m_hDisabled = hDisabled;

}

// Render the control
void CLTGUIButton::Render()
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

	// if we have a string, but not a texture, try to rebuild the texture
	if (!m_Text.IsEmpty() && !m_Text.IsValid())
		RecreateTextureStrings();

	if (m_Text.IsValid())
	{
		uint32 argbColor = m_argbNormal;
		if (m_bHighlightText)
			argbColor = GetCurrentColor();
		LTVector2 vPos = m_pos;
		vPos.x += (float)GetWidth() / 2.0f;
		vPos.y += (float)GetHeight() / 2.0f;

		g_pTextureString->RenderString( m_Text.m_hString, g_pDrawPrim, vPos, argbColor, LTVector2(0.5f,0.5f) );
	}
}


// Enter was pressed
bool CLTGUIButton::OnEnter ( )
{
	// Send the command
	if ( m_pCommandHandler && m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2) )
		return true;
    return false;
}


void CLTGUIButton::SetBasePos ( LTIntPt pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	ScalePoly();
}

void CLTGUIButton::SetScale(float fScale)
{
	if (fScale == m_fScale)
		return;

	CLTGUICtrl::SetScale(fScale);
	ScalePoly();

	m_Font.m_nHeight = (uint32)(m_fScale * (float)m_nBaseFontSize);
	RecreateTextureStrings();

}

void CLTGUIButton::SetSize(uint32 nWidth, uint32 nHeight )
{
	m_nBaseWidth = nWidth;
	m_nBaseHeight = nHeight;

	ScalePoly();
}

void CLTGUIButton::InitPoly()
{
	ScalePoly();
	DrawPrimSetUVWH(m_Poly,0.0f,0.0f,1.0f,1.0f);
	DrawPrimSetRGBA(m_Poly, 0xFF, 0xFF, 0xFF, 0xFF);
}

void CLTGUIButton::ScalePoly()
{
	if (!m_hNormal) return;
	float x = (float)m_basePos.x * m_fScale;
	float y = (float)m_basePos.y * m_fScale;
	float fw = (float)m_nBaseWidth * m_fScale;
	float fh = (float)m_nBaseHeight * m_fScale;

	DrawPrimSetXYWH(m_Poly,x,y,fw,fh);
}

void CLTGUIButton::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		
}

// free texture memory by flushing any texture strings owned by the control
void CLTGUITextCtrl::FlushTextureStrings()
{
	m_Text.Flush();
}

// rebuild any texture strings owned by the control
void CLTGUITextCtrl::RecreateTextureStrings()
{
	if (m_Text.IsEmpty() || m_Font.m_nHeight == 0)
		return;
	m_Text.Reset(m_Font);
}

*/