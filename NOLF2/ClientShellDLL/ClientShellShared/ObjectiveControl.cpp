// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectiveControl.cpp
//
// PURPOSE : GUI control to display of objectives.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ObjectiveControl.h"
#include "InterfaceMgr.h"

LTBOOL CObjectiveCtrl::Create (const char *pText, 	CUIFont *pFont, uint8 nFontSize, uint16 nTextOffset, HTEXTURE hTex)
{
	CLTGUITextCtrl::Create(pText,LTNULL,LTNULL,pFont,nFontSize,LTNULL);
	SetIndent(nTextOffset);

	m_BulletTexture = hTex;

	InitPoly();

	return LTTRUE;
}

void CObjectiveCtrl::SetBasePos ( LTIntPt pos )
{
	CLTGUITextCtrl::SetBasePos(pos);
	ScalePoly();
}

void CObjectiveCtrl::SetScale(float fScale)
{
	CLTGUITextCtrl::SetScale(fScale);
	ScalePoly();
}

void CObjectiveCtrl::SetString(const char *pText)
{
	CLTGUITextCtrl::SetString(pText);
	ScalePoly();
}

void CObjectiveCtrl::SetTexture(HTEXTURE hTex)
{
	if (hTex == m_BulletTexture) return;

	m_BulletTexture = hTex;
	ScalePoly();
	SetupQuadUVs(m_Poly, m_BulletTexture, 0.0f,0.0f,1.0f,1.0f);
}


// Render the control
void CObjectiveCtrl::Render ()
{
	// Sanity checks...
	if (!IsVisible()) return;

	CLTGUITextCtrl::Render();

	if (m_BulletTexture)
		g_pDrawPrim->SetTexture(m_BulletTexture);

//	g_pDrawPrim->SetRGBA(&m_Poly,GetCurrentColor());

	// set up the render state	
	SetRenderState();

	// draw our button
	g_pDrawPrim->DrawPrim(&m_Poly);
}


void CObjectiveCtrl::InitPoly()
{
	ScalePoly();
	SetupQuadUVs(m_Poly, m_BulletTexture, 0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(&m_Poly,0xFFFFFFFF);

}

void CObjectiveCtrl::ScalePoly()
{
	if (!m_BulletTexture) return;
//	uint32 w,h;
//	g_pTexInterface->GetTextureDims(m_BulletTexture,w,h);

	float strH = 0.0f;
	if (m_pString)
	{
		strH = m_pString->GetHeight();
	}

//	float fw = (float)w * m_fScale;
//	float fh = (float)h * m_fScale;
	float fh = (float)m_nFontSize;

	float x = (float)(m_basePos.x) * m_fScale;
	float y = (float)(m_basePos.y) * m_fScale + (strH-fh)/2.0f;


	g_pDrawPrim->SetXYWH(&m_Poly,x,y,fh,fh);

}

void CObjectiveCtrl::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		
}
