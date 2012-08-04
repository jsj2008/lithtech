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


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIButton::CLTGUIButton()
{

	m_hNormal   = LTNULL;
	m_hSelected   = LTNULL;
	m_hDisabled   = LTNULL;

	m_nWidth			= 0;
	m_nHeight			= 0;

    m_pCommandHandler   = LTNULL;

	m_pText = LTNULL;
	m_bHighlightText = LTFALSE;

    m_pFont             = LTNULL;
	m_nFontSize			= 0;
	m_nBaseFontSize		= 0;

	m_fTextureScale = 1.0f;

	memset(&m_Poly,0,sizeof(m_Poly));
}

CLTGUIButton::~CLTGUIButton()
{
	Destroy();
}

// Create the control
LTBOOL CLTGUIButton::Create (	uint32 nCommandID,
								uint32 nHelpID,
								HTEXTURE hNormal,
								HTEXTURE hSelected,
								HTEXTURE hDisabled,
								CLTGUICommandHandler *pCommandHandler,
								uint32 nParam1,
								uint32 nParam2)
{
	if (!hNormal) return LTFALSE;

	m_pCommandHandler	= pCommandHandler;

	SetTexture(hNormal,hSelected,hDisabled);

	InitPoly();

	CLTGUICtrl::Create(nCommandID, nHelpID, nParam1,nParam2);

    return LTTRUE;
}



void CLTGUIButton::Destroy()
{
	if (LTNULL != m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}
}



// Set the font
LTBOOL CLTGUIButton::SetFont(CUIFont *pFont, uint8 nFontSize)
{
	if (!pFont && !nFontSize)
	{
        return LTFALSE;
	}

	LTBOOL bApply = LTFALSE;
	if (pFont && m_pFont != pFont)
	{
		m_pFont = pFont;
		bApply = LTTRUE;
	}
	if (nFontSize && m_nBaseFontSize != nFontSize)
	{
		m_nBaseFontSize = nFontSize;
		bApply = LTTRUE;
	}

	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);

	if (bApply && m_pText)
	{
		m_pText->SetFont(m_pFont);
		m_pText->SetCharScreenHeight(m_nFontSize);
	}


    return LTTRUE;
}

LTBOOL CLTGUIButton::SetText(const char* pText, LTBOOL bHighlightText)
{
	if (!m_pFont) return LTFALSE;

	m_bHighlightText = bHighlightText;

	if (!m_pText)
	{
		m_pText = g_pFontManager->CreateFormattedPolyString(m_pFont,(char *)pText,(float)m_pos.x,(float)m_pos.y);
	}
	else
		m_pText->SetText((char *)pText);

	if (m_pText)
	{
		m_pText->SetCharScreenHeight(m_nFontSize);
		float x = (float)m_pos.x + ((float)GetWidth() - m_pText->GetWidth()) / 2.0f;
		float y = (float)m_pos.y + ((float)GetHeight() - m_pText->GetHeight()) / 2.0f;
		m_pText->SetPosition(x,y);
	}

	return LTTRUE;
}


void CLTGUIButton::SetTexture(HTEXTURE hNormal, HTEXTURE hSelected, HTEXTURE hDisabled, LTBOOL bFreeOld)
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

	if (m_pText)
	{
		uint32 argbColor = m_argbNormal;
		if (m_bHighlightText)
			argbColor = GetCurrentColor();
		m_pText->SetColor(argbColor);
		m_pText->Render();
	}
}


// Enter was pressed
LTBOOL CLTGUIButton::OnEnter ( )
{
	// Send the command
	if ( m_pCommandHandler && m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2) )
		return LTTRUE;
    return LTFALSE;
}


void CLTGUIButton::SetBasePos ( LTIntPt pos )
{
	CLTGUICtrl::SetBasePos(pos);
	ScalePoly();
	if (m_pText)
	{
		float x = (float)m_pos.x + ((float)GetWidth() - m_pText->GetWidth()) / 2.0f;
		float y = (float)m_pos.y + ((float)GetHeight() - m_pText->GetHeight()) / 2.0f;
		m_pText->SetPosition(x,y);
	}
}

void CLTGUIButton::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	ScalePoly();
	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);
	if (m_pText)
	{
		m_pText->SetCharScreenHeight(m_nFontSize);
		float x = (float)m_pos.x + ((float)GetWidth() - m_pText->GetWidth()) / 2.0f;
		float y = (float)m_pos.y + ((float)GetHeight() - m_pText->GetHeight()) / 2.0f;
		m_pText->SetPosition(x,y);
	}

}

void CLTGUIButton::SetTextureScale(float fScale)
{
	m_fTextureScale = fScale;
	ScalePoly();
	if (m_pText)
	{
		float x = (float)m_pos.x + ((float)GetWidth() - m_pText->GetWidth()) / 2.0f;
		float y = (float)m_pos.y + ((float)GetHeight() - m_pText->GetHeight()) / 2.0f;
		m_pText->SetPosition(x,y);
	}

}

void CLTGUIButton::InitPoly()
{
	ScalePoly();
	g_pDrawPrim->SetUVWH(&m_Poly,0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(&m_Poly,0xFFFFFFFF);

}

void CLTGUIButton::ScalePoly()
{
	if (!m_hNormal) return;
	uint32 w,h;
	g_pTexInterface->GetTextureDims(m_hNormal,w,h);
	float x = (float)m_basePos.x * m_fScale;
	float y = (float)m_basePos.y * m_fScale;
	float fw = (float)w * m_fScale * m_fTextureScale;
	float fh = (float)h * m_fScale * m_fTextureScale;

	g_pDrawPrim->SetXYWH(&m_Poly,x,y,fw,fh);

	m_nWidth = (uint16)fw;
	m_nHeight = (uint16)fh;

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
