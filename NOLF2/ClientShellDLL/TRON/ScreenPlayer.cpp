// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayer.h
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPlayer.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{
	void EditNameCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenPlayer *pThisScreen = (CScreenPlayer *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_PLAYER);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_NAME);
	};
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPlayer::CScreenPlayer()
{
	m_pName			= LTNULL;
}

CScreenPlayer::~CScreenPlayer()
{

}

// Build the screen
LTBOOL CScreenPlayer::Build()
{

	CreateTitle(IDS_TITLE_PLAYER);
	int kColumn0 = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER,"ColumnWidth");
	int kColumn1 = (640 - GetPageLeft()) - kColumn0;

	m_pName = AddColumnCtrl(CMD_EDIT_NAME, IDS_HELP_SESSION_NAME);
	m_pName->AddColumn(LoadTempString(IDS_PLAYER_NAME), kColumn0);
	m_pName->AddColumn("<player name>", kColumn1, LTTRUE);

 	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenPlayer::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_EDIT_NAME:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditNameCallBack;
			mb.pString = m_sPlayerName.c_str();
			mb.nMaxChars = MAX_PLAYER_NAME-1;
			g_pInterfaceMgr->ShowMessageBox(IDS_PLAYER_NAME,&mb);
		} break;

	case CMD_OK:
		{
			m_sPlayerName = ((char *)dwParam1);
			m_pName->SetString(1,m_sPlayerName.c_str());
		} break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenPlayer::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		
	if (bFocus)
	{
		m_sPlayerName = pProfile->m_sPlayerName;
		m_pName->SetString(1,m_sPlayerName.c_str());
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
		pProfile->m_sPlayerName = m_sPlayerName;


		pProfile->Save();

	}
	CBaseScreen::OnFocus(bFocus);
}

