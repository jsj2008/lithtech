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
	

bool CHUDPaused::Init()
{

	UpdateLayout();

	m_MsgFormat.sString = LoadString("IDS_PAUSED");
	m_MsgFormat.fDuration = 0.0f;
	m_Msg.Create(m_MsgFormat);

	ScaleChanged();

	return true;

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

void CHUDPaused::Show(bool bShow)
{
	m_Msg.Show(bShow);

	if (!bShow)
		return;

}

void CHUDPaused::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDPaused");
	}

	CHUDItem::UpdateLayout();

	m_MsgFormat.Font = m_sTextFont;

	m_MsgFormat.nTextColor =  m_cTextColor;
	m_MsgFormat.fDuration = 0.0f;
	m_MsgFormat.fFadeDur  = 0.0f;
	m_MsgFormat.nWidth  = 640;
	m_MsgFormat.eJustify = m_eTextAlignment;

}

void CHUDPaused::Update()
{
}

void CHUDPaused::ScaleChanged()
{

	LTVector2n pos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(pos);

	pos.y -= m_Msg.GetBaseHeight()/2;

	m_Msg.SetBasePos(pos);

}
