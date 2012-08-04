// ----------------------------------------------------------------------- //
//
// MODULE  : HUDMgr.cpp
//
// PURPOSE : Implementation of CHUDMgr class
//
// CREATED : 07/17/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "GameClientShell.h"


CHUDMgr*			g_pHUDMgr = LTNULL;
CHUDChatMsgQueue*	g_pChatMsgs = LTNULL;
CHUDPickupMsgQueue*	g_pPickupMsgs = LTNULL;
CHUDChatInput*		g_pChatInput = LTNULL;
CHUDTransmission*	g_pTransmission = LTNULL;
CHUDMissionText*	g_pMissionText = LTNULL;
CHUDSubtitles*		g_pSubtitles = LTNULL;
CHUDDecision*		g_pDecision = LTNULL;
CHUDPopup*			g_pPopup = LTNULL;
CHUDRadar*			g_pRadar = LTNULL;
CHUDRewardMsgQueue*	g_pRewardMsgs = LTNULL;
CHUDPaused*			g_pPaused = LTNULL;
CHUDDisplayMeter*	g_pDisplayMeter = LTNULL;
CHUDScores*			g_pScores = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::CHUDMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CHUDMgr::CHUDMgr()
{
	m_itemArray.reserve(12);
	m_eLevel = kHUDRenderNone;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::~CHUDMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CHUDMgr::~CHUDMgr()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CHUDMgr::Init()
{
	m_itemArray.push_back(&m_RewardMsgs);
	m_itemArray.push_back(&m_Popup);
	m_itemArray.push_back(&m_Decision);
	m_itemArray.push_back(&m_Subtitles);
	m_itemArray.push_back(&m_ChatMsgs);
	m_itemArray.push_back(&m_ChatInput);
	m_itemArray.push_back(&m_MissionText);
	m_itemArray.push_back(&m_Transmission);
	m_itemArray.push_back(&m_PickupMsgs);
	m_itemArray.push_back(&m_Damage);
	m_itemArray.push_back(&m_Radar);
	m_itemArray.push_back(&m_DisplayMeter);
	m_itemArray.push_back(&m_Scores);
	
	g_pPopup = &m_Popup;
	g_pDecision = &m_Decision;
	g_pSubtitles = &m_Subtitles;
	g_pChatMsgs = &m_ChatMsgs;
	g_pChatInput = &m_ChatInput;
	g_pMissionText = &m_MissionText;
	g_pTransmission = &m_Transmission;
	g_pPickupMsgs = &m_PickupMsgs;
	g_pRadar = &m_Radar;
	g_pRewardMsgs = &m_RewardMsgs;
	g_pPaused = &m_Paused;
	g_pDisplayMeter = &m_DisplayMeter;
	g_pScores = &m_Scores;

	m_nCurrentLayout = GetConsoleInt("HUDLayout",0);
	
	ItemArray::iterator iter = m_itemArray.begin();
	while (iter != m_itemArray.end())
	{
		if (!(*iter)->Init())
			return LTFALSE;
		iter++;
	}

	m_UpdateFlags = kHUDAll;

	g_pHUDMgr = this;

	m_bVisible = LTTRUE;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CHUDMgr::Term()
//
//	PURPOSE:	Terminate the HUD
//
// ----------------------------------------------------------------------- //

void CHUDMgr::Term()
{
	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		(*iter)->Term();
		iter++;
	}

	m_itemArray.clear();

	g_pHUDMgr = LTNULL;
}


void CHUDMgr::Update()
{

	int nLayout = GetConsoleInt("HUDLayout",0);
	if (nLayout != m_nCurrentLayout)
	{
		m_nCurrentLayout = nLayout;
		UpdateLayout();
	}
	m_UpdateFlags |= kHUDFrame;

	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		if (m_UpdateFlags & (*iter)->GetUpdateFlags())
			(*iter)->Update();
		iter++;
	}

	m_UpdateFlags = kHUDNone;

}

void CHUDMgr::Render()
{
	if (!m_bVisible) return;

	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		//we need to make sure that this hud item is up to date though
		if (m_UpdateFlags & (*iter)->GetUpdateFlags())
			(*iter)->Update();

		if ((*iter)->GetRenderLevel() <= m_eLevel)
			(*iter)->Render();
		iter++;
	}
	
}


void CHUDMgr::QueueUpdate(uint32 nUpdateFlag)
{
	m_UpdateFlags |= nUpdateFlag;
}

void CHUDMgr::ScreenDimsChanged()
{
	m_UpdateFlags = kHUDAll;
}

void CHUDMgr::NextLayout()
{
	m_nCurrentLayout++;
	if (m_nCurrentLayout >= g_pLayoutMgr->GetNumHUDLayouts())
		m_nCurrentLayout = 0;
    WriteConsoleInt("HUDLayout",m_nCurrentLayout);
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_nLayout = m_nCurrentLayout;
	pProfile->Save();
	UpdateLayout();
}

void CHUDMgr::PrevLayout()
{
	if (m_nCurrentLayout == 0)
		m_nCurrentLayout =  g_pLayoutMgr->GetNumHUDLayouts() - 1;
	else
		m_nCurrentLayout--;
    WriteConsoleInt("HUDLayout",m_nCurrentLayout);
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	pProfile->m_nLayout = m_nCurrentLayout;
	pProfile->Save();
	UpdateLayout();
}

void CHUDMgr::UpdateLayout()
{
	ItemArray::iterator iter = m_itemArray.begin();

	while (iter != m_itemArray.end())
	{
		(*iter)->UpdateLayout();
		iter++;
	}


	m_UpdateFlags = kHUDAll;

}




