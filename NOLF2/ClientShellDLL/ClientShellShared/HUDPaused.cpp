// ----------------------------------------------------------------------- //
//
// MODULE  : HUDPaused.cpp
//
// PURPOSE : Implementation of CHUDPaused to display a paused message.
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDPaused.h"
#include "InterfaceMgr.h"

CHUDPaused::CHUDPaused()
{
	m_UpdateFlags = kHUDNone;
	m_eLevel = kHUDRenderText;
}
	

LTBOOL CHUDPaused::Init()
{
	m_fScale = 1.0f;

	UpdateLayout();

	m_MsgFormat.sString = LoadTempString(IDS_PAUSED);
	m_MsgFormat.fDuration = 0.0f;
	m_Msg.Create(m_MsgFormat);

	return LTTRUE;

}
void CHUDPaused::Term()
{
	m_Msg.Destroy();
}

void CHUDPaused::Render()
{
	if (!m_Msg.IsVisible()) return;
	m_Msg.Render();
}

void CHUDPaused::Show(LTBOOL bShow)
{
	m_Msg.Show(bShow);

	if (!bShow)
		return;

	LTIntPt pos = m_BasePos;
	pos.y -= m_Msg.GetBaseHeight()/2;

	m_Msg.SetBasePos(pos);
}

void CHUDPaused::UpdateLayout()
{
	char *pTag = "PausedText";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag,"BasePos");

	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");

	m_MsgFormat.pFont = g_pInterfaceResMgr->GetFont(nFont);
	m_MsgFormat.nFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");

	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	m_MsgFormat.nTextColor =  SET_ARGB(0xFF,nR,nG,nB);
	m_MsgFormat.fDuration = 0.0f;
	m_MsgFormat.fFadeDur  = 0.0f;
	m_MsgFormat.nWidth  = 640;
	m_MsgFormat.eJustify = kMsgCenter;

}

void CHUDPaused::Update()
{
	if (m_fScale != g_pInterfaceResMgr->GetXRatio())
	{
		m_fScale = g_pInterfaceResMgr->GetXRatio();
		m_Msg.SetScale(m_fScale);
	}
}