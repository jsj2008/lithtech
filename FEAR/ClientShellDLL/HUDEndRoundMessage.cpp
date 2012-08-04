// ----------------------------------------------------------------------- //
//
// MODULE  : HUDEndRoundMessage.cpp
//
// PURPOSE : Implementation of CHUDEndRoundMessage used to display messages
//			 at the end of multiplayer rounds

// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDEndRoundMessage.h"
#include "InterfaceMgr.h"

CHUDEndRoundMessage::CHUDEndRoundMessage()
{
	m_UpdateFlags = kHUDFrame;
	m_eLevel = kHUDRenderText;
}
	
bool CHUDEndRoundMessage::Init()
{
	UpdateLayout();

	MsgCreate fmt = m_MsgFormat;
	fmt.sString = L" ";
	fmt.fDuration = 0.0f;
	m_Msg.Create(fmt);

	return true;

}
void CHUDEndRoundMessage::Term()
{
	m_Msg.Destroy();
}

void CHUDEndRoundMessage::Render()
{
	if (!m_Msg.IsVisible()) return;
	m_Msg.Render();
}

void CHUDEndRoundMessage::Update()
{
	if (!m_Msg.IsVisible()) return;
	m_Msg.Update();
}

void CHUDEndRoundMessage::ScaleChanged()
{
	LTVector2n pos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(pos);
	pos.y -= m_Msg.GetBaseHeight()/2;

	m_Msg.SetScale(g_pInterfaceResMgr->GetScreenScale());
	m_Msg.SetBasePos(pos);
}

void CHUDEndRoundMessage::Show(const char* szMessageID)
{
	Show(LoadString(szMessageID));
}

void CHUDEndRoundMessage::Show(const wchar_t *pszString)
{
	MsgCreate fmt = m_MsgFormat;
	fmt.sString = pszString;
	m_Msg.Create(fmt);
	m_Msg.Show(true);

	ScaleChanged();
}

void CHUDEndRoundMessage::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDEndRoundMessage");
	}

	CHUDItem::UpdateLayout();

	m_MsgFormat.Font = m_sTextFont;
	m_MsgFormat.nTextColor =  m_cTextColor;
	m_MsgFormat.fDuration = m_fHoldTime;
	m_MsgFormat.fFadeDur  = m_fFadeTime;
	m_MsgFormat.nWidth  = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);
	m_MsgFormat.eJustify = m_eTextAlignment;
	m_MsgFormat.bDropShadow = true;
}
