// ----------------------------------------------------------------------- //
//
// MODULE  : HUDTransmission.cpp
//
// PURPOSE : Implementation of CHUDTransmission to display messages
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDTransmission.h"
#include "InterfaceMgr.h"

CHUDTransmission::CHUDTransmission()
{
	m_UpdateFlags = kHUDFrame;
	m_eLevel = kHUDRenderText;
}
	

bool CHUDTransmission::Init()
{
	UpdateLayout();

	MsgCreate fmt = m_MsgFormat;
	fmt.sString = L" ";
	fmt.fDuration = 0.0f;
	m_Msg.Create(fmt);

	return true;

}
void CHUDTransmission::Term()
{
	m_Msg.Destroy();
}

void CHUDTransmission::Render()
{
	if (!m_Msg.IsVisible()) return;
	m_Msg.Render();
}

void CHUDTransmission::Update()
{
	if (!m_Msg.IsVisible()) return;
	m_Msg.Update();
}

void CHUDTransmission::ScaleChanged()
{
	LTVector2n pos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(pos);
	pos.y -= m_Msg.GetBaseHeight()/2;

	m_Msg.SetBasePos(pos);
	m_Msg.SetScale(g_pInterfaceResMgr->GetScreenScale());
	
}

void CHUDTransmission::Show(const char* szMessageID)
{
	Show(LoadString(szMessageID));
}

void CHUDTransmission::Show(const wchar_t *pszString)
{
	MsgCreate fmt = m_MsgFormat;
	fmt.sString = pszString;
	m_Msg.Create(fmt);
	m_Msg.Show(true);

	ScaleChanged();
}

void CHUDTransmission::UpdateLayout()
{
	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDTransmission");
	}

	CHUDItem::UpdateLayout();

	m_MsgFormat.Font = m_sTextFont;
	m_MsgFormat.nTextColor =  m_cTextColor;
	m_MsgFormat.fDuration = m_fHoldTime;
	m_MsgFormat.fFadeDur  = m_fFadeTime;
	m_MsgFormat.nWidth  = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,0);
	m_MsgFormat.eJustify = m_eTextAlignment;

}
