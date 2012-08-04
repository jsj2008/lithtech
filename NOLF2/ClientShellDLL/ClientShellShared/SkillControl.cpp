// ----------------------------------------------------------------------- //
//
// MODULE  : SkillControl.cpp
//
// PURPOSE : GUI control to display of objectives.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "SkillControl.h"
#include "InterfaceMgr.h"

HTEXTURE  CSkillCtrl::m_hChecked = NULL;
HTEXTURE  CSkillCtrl::m_hUnchecked = NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkillCtrl::CSkillCtrl()
{
    m_pFont             = LTNULL;
	m_nFontSize			= 0;
	m_nBaseFontSize		= 0;
    m_pCommandHandler   = LTNULL;
	m_nWidth			= 0;
	m_nHeight			= 0;
	m_pLabel			= LTNULL;

}

CSkillCtrl::~CSkillCtrl()
{
	Destroy();
}

// Destroys the control
void CSkillCtrl::Destroy ( )
{
	if (m_pLabel)
	{
		g_pFontManager->DestroyPolyString(m_pLabel);
		m_pLabel = LTNULL;
	}


}



LTBOOL CSkillCtrl::Create (eSkill skl, uint32 nCommandID, CUIFont *pFont, uint8 nFontSize, CLTGUICommandHandler *pCommandHandler, uint16 nNameGap)
{
	m_Skill = skl;

	if (!m_hChecked)
	{
		m_hChecked = g_pInterfaceResMgr->GetTexture("interface\\hud\\skill_full.dtx");
		m_hUnchecked = g_pInterfaceResMgr->GetTexture("interface\\hud\\skill_empty.dtx");
	}

	m_pCommandHandler	= pCommandHandler;

	m_pFont = pFont;
	m_nFontSize = m_nBaseFontSize = nFontSize;
	m_nNameGap = nNameGap;

	if ( !m_pFont || !m_nFontSize)
		return LTFALSE;

	if (!m_pLabel)
		m_pLabel = g_pFontManager->CreateFormattedPolyString(m_pFont,(char *)GetSkillName(skl),0.0f,0.0f);
	else
		m_pLabel->SetText(GetSkillName(skl));


	m_pLabel->SetCharScreenHeight(m_nFontSize);

	CLTGUICtrl::Create(nCommandID, GetSkillDescriptionId(skl), (uint32)skl, 0);

	InitPolies();

	SetSkillLevel( (eSkillLevel) 0);

	return LTTRUE;
}

void CSkillCtrl::SetBasePos ( LTIntPt pos )
{
	CLTGUICtrl::SetBasePos(pos);
	ScalePolies();
}

void CSkillCtrl::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);
	ScalePolies();

}


// Render the control
void CSkillCtrl::Render ()
{
	// Sanity checks...
	if (!IsVisible()) return;


    uint32 argbColor=GetCurrentColor();
	m_pLabel->SetColor(argbColor);
	m_pLabel->Render();

	// set up the render state	
	SetRenderState();

	uint8 n = (uint8)m_Level;
	static uint8 nLast = (kNumSkillLevels-1);

	g_pDrawPrim->SetTexture(m_hChecked);
	g_pDrawPrim->DrawPrim(&m_Poly[0],(n+1));

	if (n < nLast)
	{
		g_pDrawPrim->SetTexture(m_hUnchecked);
		g_pDrawPrim->DrawPrim(&m_Poly[n+1],(nLast-n));
	}



}


void CSkillCtrl::InitPolies()
{
	ScalePolies();
	for (uint8 i = 0; i < kNumSkillLevels; ++i)
	{
		SetupQuadUVs(m_Poly[i], m_hChecked, 0.0f,0.0f,1.0f,1.0f);
		g_pDrawPrim->SetRGBA(&m_Poly[i],0xFFFFFFFF);
	}

}

void CSkillCtrl::ScalePolies()
{
	float fNameGap = m_fScale * (float)m_nNameGap;
	float x = (float)m_pos.x;
	float y = (float)m_pos.y;
	float fw = (float)m_nFontSize;

	//place label string
	if (m_pLabel)
	{
		m_pLabel->SetPosition(x,y);
		m_pLabel->SetCharScreenHeight(m_nFontSize);
	}
	x += fNameGap;

	//place skill check boxes
	for (uint8 i = 0; i < kNumSkillLevels; ++i)
	{
		g_pDrawPrim->SetXYWH(&m_Poly[i],x,y,fw,fw);
		x += (1.2f * fw);
	}

	m_nWidth = (uint16)(x - (float)m_pos.x);
	if (m_pLabel)
		m_nHeight = (uint16)m_pLabel->GetHeight() + 2;
	else
		m_nHeight = m_nFontSize + 2;
}

void CSkillCtrl::SetRenderState()
{
	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);
		
}

void CSkillCtrl::SetSkillLevel(eSkillLevel level)
{
	if (!g_pPlayerStats) return;

	if (level < kNumSkillLevels)
		m_Level = level;
	else
		m_Level = (eSkillLevel)(kNumSkillLevels-1);

	eSkillLevel nxt = (eSkillLevel)(m_Level + 1);

}

// Enter was pressed
LTBOOL CSkillCtrl::OnEnter ( )
{
	// Send the command
	if ( m_pCommandHandler && m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2) )
		return LTTRUE;
    return LTFALSE;
}

