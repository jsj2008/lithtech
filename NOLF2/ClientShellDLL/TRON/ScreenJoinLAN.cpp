// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoinLAN.cpp
//
// PURPOSE : Interface screen to search for and join LAN games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenJoinLAN.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"
#include "ClientMultiplayerMgr.h"
#include "MissionMgr.h"

#include "GameClientShell.h"

void JoinLANCallBack(LTBOOL bReturn, void *pData);

namespace
{
	int kColumnWidth = 300;
	char	szOldPort[8];
	int kListFontSize = 12;
	LTBOOL bIsInitting = LTFALSE;
	void EditPortCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenJoinLAN *pThisScreen = (CScreenJoinLAN *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN_LAN);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PORT);
	};

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenJoinLAN::CScreenJoinLAN()
{
}

CScreenJoinLAN::~CScreenJoinLAN()
{

}

// Build the screen
LTBOOL CScreenJoinLAN::Build()
{
	if (!CBaseScreen::Build()) return LTFALSE;

	CreateTitle(IDS_TITLE_JOIN);
	kColumnWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN_LAN,"ColumnWidth");
	kListFontSize = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN_LAN,"ListFontSize");

	m_pFind = AddTextItem(IDS_FIND_SERVERS,CMD_SEARCH,IDS_HELP_FIND_SERVERS);

	char szTmp[64];
	m_pPort = AddColumnCtrl(CMD_EDIT_PORT, IDS_HELP_ENTER_PORT);
	FormatString(IDS_PORT,szTmp,sizeof(szTmp));
	m_pPort->AddColumn(szTmp, kColumnWidth);
	m_pPort->AddColumn("27888", kColumnWidth, LTTRUE);
	SAFE_STRCPY(m_szPort,"27888");

	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN_LAN,"ListPos");
	CLTGUIColumnCtrl* pCtrl = AddColumnCtrl(LTNULL, LTNULL, pos, LTTRUE);
	FormatString(IDS_SERVER_NAME,szTmp,sizeof(szTmp));
	pCtrl->AddColumn(szTmp, kColumnWidth);
	FormatString(IDS_SERVER_ADDRESS,szTmp,sizeof(szTmp));
	pCtrl->AddColumn(szTmp, kColumnWidth);
	pCtrl->Enable(LTFALSE);

	uint16 height = GetPageBottom() - m_nextPos.y;
	m_pServers = AddList(m_nextPos,height,LTTRUE,kColumnWidth);
	m_pServers->SetScrollWrap(LTFALSE);


 	// Make sure to call the base class
	return LTTRUE;
}

uint32 CScreenJoinLAN::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_SEARCH:
		{
			FindServers();
		}
		break;
	case CMD_EDIT_PORT:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditPortCallBack;
			mb.pString = m_szPort;
			mb.nMaxChars = 5;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(IDS_PORT,&mb);
		} break;

	case CMD_OK:
		{
			char *pszPort = (char *)dwParam1;
			uint16 nPort = (uint16)atoi(pszPort);
			if (IsValidPort(nPort))
			{
				SAFE_STRCPY(m_szPort,pszPort);
				m_pPort->SetString(1,m_szPort);
			}
		}
		break;
	case CMD_JOIN:
		{
			if (dwParam1 >= m_lstSessions.GetSize()) return 0;

		    if (g_pGameClientShell->IsWorldLoaded())
		    {

				if (IsCurrentGame(m_lstSessions[dwParam1]))
				{
					HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
					if (g_pGameClientShell->IsWorldLoaded() && hPlayerObj)
					{
						g_pInterfaceMgr->ChangeState(GS_PLAYING);
					}
				}
				else
				{
					MBCreate mb;
					mb.eType = LTMB_YESNO;
					mb.pFn = JoinLANCallBack;
					g_pInterfaceMgr->ShowMessageBox(IDS_ENDCURRENTGAME,&mb);
				}
		    }
		    else
		    {
				JoinCurGame();
		    }


		} break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenJoinLAN::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		SetSelection(GetIndex(m_pFind));
		m_pServers->RemoveAll();

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
	}
	CBaseScreen::OnFocus(bFocus);
}


LTBOOL CScreenJoinLAN::InitSessions()
{
	// Remove all of the menu options
	m_pServers->RemoveAll();

	// Remove the sessions
	m_lstSessions.RemoveAll();

	CString csString;
	NetSession *pList, *pSession;
	LTIntPt pt;
	

	// We need to make sure the engine's internet driver is ready before we can get the session list
	// NOTE: The Client_de function AddInternetDriver and RemoveInternetDriver were tiny little functions that I added
	// to Sanity's version of LithTech v1.5 (you can see them under LithTechSanity in SS)
	LTRESULT res = g_pLTClient->AddInternetDriver();
	if(res == LT_ERROR)
		return LTFALSE;

	// This is a function in clientshell.cpp (cut & pasted below) that finds the engine's TCP/IP service and sets it.
	if(!g_pClientMultiplayerMgr->SetService())
	{
		// Remove the internet driver if we added it
		if(res != LT_ALREADYEXISTS)
			g_pLTClient->RemoveInternetDriver();
		return LTFALSE;
	}

	char sTemp[16];
	uint16 nPort = (uint16)atoi(m_szPort);
	sprintf(sTemp,"*:%d",nPort);

	// Find the sessions
	if((g_pLTClient->GetSessionList( pList, sTemp ) == LT_OK) && pList)
	{
		pSession = pList;
		while(pSession)
		{
			csString.Format( "%s:%d", pSession->m_HostIP, pSession->m_HostPort );

			if (!m_lstSessions.Add(csString))
			{
				g_pLTClient->FreeSessionList( pList );
				// Remove the internet driver if we added it
				if(res != LT_ALREADYEXISTS)
					g_pLTClient->RemoveInternetDriver();
				return LTFALSE;
			}
			uint32 pos = m_lstSessions.GetSize()-1;

			// Add the session to our list o' sessions
			CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(CMD_JOIN, IDS_HELP_JOIN_LAN);
			if(!pCtrl)
			{
				g_pLTClient->FreeSessionList( pList );
				// Remove the internet driver if we added it
				if(res != LT_ALREADYEXISTS)
					g_pLTClient->RemoveInternetDriver();
				return LTFALSE;
			}
			pCtrl->SetFont(LTNULL,kListFontSize);


			// Do the name
			pCtrl->AddColumn(pSession->m_sName, kColumnWidth);
			pCtrl->SetParam1((uint32)pos);

/*
			char szPlayers[16] ="";
			sprintf(szPlayers,"%d/%d",pSession->m_dwCurPlayers,pSession->m_dwMaxPlayers);
			hString = g_pLTClient->CreateString(szPlayers);
			pCtrl->AddColumn(hString, 50, LTF_JUSTIFY_LEFT);
			TERMSTRING(hString);
*/			
			
			// Do the address
			pCtrl->AddColumn((LPCTSTR)csString, kColumnWidth);
			m_pServers->AddControl(pCtrl);

			pSession = pSession->m_pNext;
		}
		g_pLTClient->FreeSessionList( pList );
	}

	// Remove the internet driver if we added it
	if(res != LT_ALREADYEXISTS)
		g_pLTClient->RemoveInternetDriver();


	return LTTRUE;
}


LTBOOL CScreenJoinLAN::IsCurrentGame(CString sAddress)
{
	if (!IsMultiplayerGame( )) return LTFALSE;

	char sTemp[MAX_SGR_STRINGLEN];
	strcpy(sTemp,(char *)(LPCTSTR)sAddress);
	char *pTok = strtok(sTemp,":");
	pTok = strtok(LTNULL,":");
	int p = atoi(pTok);

	return (g_pClientMultiplayerMgr->CheckServerAddress(sTemp,p));

}


void JoinLANCallBack(LTBOOL bReturn, void *pData)
{
	CScreenJoinLAN *pThisScreen = (CScreenJoinLAN *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN_LAN);
	if (bReturn && pThisScreen)
	{
		pThisScreen->JoinCurGame();
    }
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenJoinLAN::JoinGame
//
//	PURPOSE:	Joins the given game service
//
// ----------------------------------------------------------------------- //

void CScreenJoinLAN::JoinCurGame()
{
	// Get the currently selected game server...
	CLTGUICtrl *pCtrl = m_pServers->GetSelectedControl();
	if (!pCtrl) return;

	uint32 nIndex = pCtrl->GetParam1();
	if (nIndex > m_lstSessions.GetSize()) return;


	if (IsCurrentGame(m_lstSessions[nIndex]))
	{
        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (g_pGameClientShell->IsWorldLoaded() && hPlayerObj)
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
		}
		return;
	}

	bool bOk = g_pClientMultiplayerMgr->SetupClient(m_lstSessions[nIndex]);
	bOk = bOk && g_pMissionMgr->StartGameAsClient( );

	if( !bOk )
	{
		g_pInterfaceMgr->LoadFailed();
	}
}


void CScreenJoinLAN::FindServers()
{
	if (!bIsInitting)
	{
		g_pInterfaceResMgr->DrawMessage(IDS_LOOKING_FOR_SERVERS);

		g_pLTClient->CPrint("init sessions");
		bIsInitting = LTTRUE;
		InitSessions();

		if (m_lstSessions.GetSize() == 0)
		{
			CLTGUITextCtrl *pCtrl = CreateTextItem(IDS_NO_SERVERS, LTNULL, LTNULL, kDefaultPos, LTTRUE);
			pCtrl->SetFont(LTNULL,kListFontSize);
			m_pServers->AddControl(pCtrl);
		}
		g_pLTClient->ClearInput();
	}
}





LTBOOL CScreenJoinLAN::Render(HSURFACE hDestSurf)
{
	bIsInitting = LTFALSE;
	return CBaseScreen::Render(hDestSurf);
}


LTBOOL CScreenJoinLAN::HandleKeyDown(int key, int rep)
{
	if (key == VK_F5)
	{
		FindServers();
        return LTTRUE;
	}
    return CBaseScreen::HandleKeyDown(key,rep);
}