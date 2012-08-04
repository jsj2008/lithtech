// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMissionText.cpp
//
// PURPOSE : Implementation of CHUDMissionText to display messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDMissionText.h"
#include "InterfaceResMgr.h"
#include "LayoutMgr.h"
#include "ClientUtilities.h"

CHUDMissionText::CHUDMissionText()
{
	m_UpdateFlags = kHUDFrame;
	m_eLevel = kHUDRenderText;
	m_bVisible = LTFALSE;
	m_bPause = LTFALSE;
	m_fScale = 1.0f;
}
	

LTBOOL CHUDMissionText::Init()
{
	UpdateLayout();
	if (!m_pText)
		return LTFALSE;

	return LTTRUE;

}
void CHUDMissionText::Term()
{
	m_Text.Clear();
	if (m_pText)
	{
		g_pFontManager->DestroyPolyString(m_pText);
		m_pText = LTNULL;
	}

}

void CHUDMissionText::Render()
{
	if (!m_bVisible) return;
	m_Text.Render();
}

void CHUDMissionText::Update()
{
	// Sanity checks...
	if (!IsVisible() || m_bPause) return;

	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		SetScale(g_pInterfaceResMgr->GetXRatio());

	m_Text.Update();

}

void CHUDMissionText::Start(int nMessageID)
{
	Start(LoadTempString(nMessageID));
}

void CHUDMissionText::Start(char *pszString)
{

	ASSERT(m_pText);
	m_pText->SetText(pszString);
	m_bVisible = LTTRUE;
	m_bPause = LTFALSE;

	SetScale(g_pInterfaceResMgr->GetXRatio());

	// Set up the timed text and start it off
	m_Text.Init( m_Format );
	m_Text.Start();
	m_Text.Show();
	

}

void CHUDMissionText::Clear()
{
	m_Text.Clear();
}

void CHUDMissionText::Pause(LTBOOL bPause)
{
	m_bPause = bPause;
	if (bPause)
		m_Text.Pause();
	else
		m_Text.Resume();
}

void CHUDMissionText::SetScale(float fScale)
{
	m_fScale = fScale;
	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);
	if (m_pText)
	{
		float x = m_fScale * (float)m_BasePos.x;
		float y = m_fScale * (float)m_BasePos.y;
		m_pText->SetCharScreenHeight(m_nFontSize);
		m_pText->SetPosition(x,y);

		uint16 nTextWidth = (uint16)(m_fScale * (float)m_nWidth);
		m_pText->SetWrapWidth(nTextWidth);

	}

}

void CHUDMissionText::UpdateLayout()
{

	char *pTag = "MissionText";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag,"Pos");

	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");
	m_pFont = g_pInterfaceResMgr->GetFont(nFont);
	m_nFontSize = m_nBaseFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");

	float x = m_fScale * (float)m_BasePos.x;
	float y = m_fScale * (float)m_BasePos.y;
	m_pText = g_pFontManager->CreateFormattedPolyString(m_pFont," ",x,y);
	if (!m_pText)
		return;

	m_nWidth = (uint16)g_pLayoutMgr->GetInt(pTag,"Width");
	m_pText->SetWrapWidth(m_nWidth);

	m_Format.text = m_pText;

	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	m_Format.color = SET_ARGB(0xFF,nR,nG,nB);
	
	m_Format.useDroppedShadow = LTTRUE;

	m_Format.numberOfLinesBeforeScroll = g_pLayoutMgr->GetInt(pTag,"NumLines");
	m_Format.clipRect = NULL;
	m_Format.initialDelay = 0.0f;
	m_Format.characterDelay = g_pLayoutMgr->GetFloat(pTag,"LetterDelay");
	m_Format.lineDelay = g_pLayoutMgr->GetFloat(pTag,"LineDelay");
	m_Format.scrollTime  = g_pLayoutMgr->GetFloat(pTag,"LineScrollTime");
	m_Format.completeDelay = g_pLayoutMgr->GetFloat(pTag,"FadeDelay");
	m_Format.fadeTime = g_pLayoutMgr->GetFloat(pTag,"FadeTime");

	g_pLayoutMgr->GetString(pTag,"TypeSound",m_Format.textDisplaySound, TIMED_TEXT_SOUND_NAME_LENGTH );
	g_pLayoutMgr->GetString(pTag,"ScrollSound",m_Format.scrollSound, TIMED_TEXT_SOUND_NAME_LENGTH );

}
