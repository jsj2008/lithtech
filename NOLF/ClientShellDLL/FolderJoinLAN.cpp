// FolderJoinLAN.cpp: implementation of the CFolderJoinLAN class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderJoinLAN.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

extern VarTrack g_vtPlayerName;

#define TERMSTRING(str) if (str) { g_pLTClient->FreeString(str); str = NULL; }
void JoinLANCallBack(LTBOOL bReturn, void *pData);

namespace
{
	int kColumnWidth = 300;
	enum eLocalCommands
	{
		CMD_EDIT_PORT = FOLDER_CMD_CUSTOM+1,
		CMD_SEARCH,
	};
	char	szOldPort[8];
	int nFind = -1;

	LTBOOL bIsInitting = LTFALSE;

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderJoinLAN::CFolderJoinLAN()
{
}

CFolderJoinLAN::~CFolderJoinLAN()
{

}

// Build the folder
LTBOOL CFolderJoinLAN::Build()
{
	if (!CBaseFolder::Build()) return LTFALSE;

	CreateTitle(IDS_TITLE_JOIN);
	if (g_pLayoutMgr->HasCustomValue(FOLDER_ID_JOIN_LAN,"ColumnWidth"))
	{
		kColumnWidth = g_pLayoutMgr->GetFolderCustomInt(FOLDER_ID_JOIN_LAN,"ColumnWidth");
	}

	LTIntPt pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_JOIN_LAN,"SearchPos");
	CLTGUITextItemCtrl *pCtrl = CreateTextItem(IDS_FIND_SERVERS,CMD_SEARCH,IDS_HELP_FIND_SERVERS,LTFALSE,GetMediumFont());
	nFind = AddFixedControl(pCtrl,pos);

	m_pPortLabel = CreateTextItem(IDS_PORT, CMD_EDIT_PORT, IDS_HELP_ENTER_PORT,LTFALSE,GetMediumFont());

	m_pPortEdit = CreateEditCtrl(" ", CMD_EDIT_PORT, IDS_HELP_ENTER_PORT, m_szPort, sizeof(m_szPort), 25, LTTRUE, GetMediumFont());
	m_pPortEdit->EnableCursor();
    m_pPortEdit->SetAlignment(LTF_JUSTIFY_LEFT);

	m_pPortGroup = CreateGroup(640,m_pPortLabel->GetHeight(),IDS_HELP_ENTER_PORT);

    LTIntPt offset = LTIntPt(0,0);
    m_pPortGroup->AddControl(m_pPortLabel,offset,LTTRUE);
	offset.x = kColumnWidth-25;
    m_pPortGroup->AddControl(m_pPortEdit,offset,LTFALSE);

	pos = g_pLayoutMgr->GetFolderCustomPoint(FOLDER_ID_JOIN_LAN,"PortPos");
	AddFixedControl(m_pPortGroup,pos);

	SAFE_STRCPY(m_szPort,"27888");

 	// Make sure to call the base class
	return LTTRUE;
}

uint32 CFolderJoinLAN::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
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
			if (GetCapture())
			{
                SetCapture(LTNULL);
				m_pPortEdit->UpdateData();
				uint16 nPort = (uint16)atoi(m_szPort);
				if (nPort == 0 || !IsValidPort(nPort))
				{
					strcpy(m_szPort,szOldPort);
					m_pPortEdit->UpdateData(LTFALSE);
				}
				
				m_pPortEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
                m_pPortEdit->Select(LTFALSE);
                m_pPortLabel->Select(LTTRUE);
				ForceMouseUpdate();
			}
			else
			{
				strcpy(szOldPort,m_szPort);
				SetCapture(m_pPortEdit);
				m_pPortEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
                m_pPortEdit->Select(LTTRUE);
                m_pPortLabel->Select(LTFALSE);
			}
		} break;

	case FOLDER_CMD_JOIN:
		{
			if (dwParam1 >= m_lstSessions.GetSize()) return 0;

		    if (g_pGameClientShell->IsInWorld())
		    {

				if (IsCurrentGame(m_lstSessions[dwParam1]))
				{
					HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
					if (g_pGameClientShell->IsInWorld() && hPlayerObj)
					{
						g_pInterfaceMgr->ChangeState(GS_PLAYING);
					}
				}
				else
				{
					HSTRING hString = g_pLTClient->FormatString(IDS_ENDCURRENTGAME);
					g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,JoinLANCallBack,this);
					g_pLTClient->FreeString(hString);
				}
		    }
		    else
		    {
				JoinCurGame();
		    }


		} break;

	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CFolderJoinLAN::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		SetSelection(nFind);

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
	}
	CBaseFolder::OnFocus(bFocus);
}


LTBOOL CFolderJoinLAN::InitSessions()
{
	// Remove all of the menu options
	RemoveFree();

	// Remove the sessions
	m_lstSessions.RemoveAll();

	CString csString;
	HSTRING hString;
	NetSession *pList, *pSession;
	CLTGUIColumnTextCtrl* pCtrl;
	LTIntPt pt;
	

	// We need to make sure the engine's internet driver is ready before we can get the session list
	// NOTE: The Client_de function AddInternetDriver and RemoveInternetDriver were tiny little functions that I added
	// to Sanity's version of LithTech v1.5 (you can see them under LithTechSanity in SS)
	LTRESULT res = g_pLTClient->AddInternetDriver();
	if(res == LT_ERROR)
		return LTFALSE;

	// This is a function in clientshell.cpp (cut & pasted below) that finds the engine's TCP/IP service and sets it.
	if(!SetService())
	{
		// Remove the internet driver if we added it
		if(res != LT_ALREADYEXISTS)
			g_pLTClient->RemoveInternetDriver();
		return LTFALSE;
	}

	pCtrl = AddColumnText(LTNULL, LTNULL, LTTRUE, GetMediumFont());
	pCtrl->AddColumn(IDS_SERVER_NAME, kColumnWidth, LTF_JUSTIFY_LEFT);
	pCtrl->AddColumn(IDS_SERVER_ADDRESS, kColumnWidth, LTF_JUSTIFY_LEFT);
	pCtrl->Enable(LTFALSE);

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
			pCtrl = AddColumnText(FOLDER_CMD_JOIN, IDS_HELP_JOIN_LAN, LTFALSE, GetSmallFont());
			if(!pCtrl)
			{
				g_pLTClient->FreeSessionList( pList );
				// Remove the internet driver if we added it
				if(res != LT_ALREADYEXISTS)
					g_pLTClient->RemoveInternetDriver();
				return LTFALSE;
			}


			// Do the name
			hString = g_pLTClient->CreateString(pSession->m_sName);
			pCtrl->AddColumn(hString, kColumnWidth, LTF_JUSTIFY_LEFT);
			TERMSTRING(hString);
			pCtrl->SetParam1((uint32)pos);

/*
			char szPlayers[16] ="";
			sprintf(szPlayers,"%d/%d",pSession->m_dwCurPlayers,pSession->m_dwMaxPlayers);
			hString = g_pLTClient->CreateString(szPlayers);
			pCtrl->AddColumn(hString, 50, LTF_JUSTIFY_LEFT);
			TERMSTRING(hString);
*/			
			
			// Do the address
			hString = g_pLTClient->CreateString((char *)(LPCTSTR)csString);
			pCtrl->AddColumn(hString, kColumnWidth, LTF_JUSTIFY_LEFT);
			TERMSTRING(hString);



			pSession = pSession->m_pNext;
		}
		g_pLTClient->FreeSessionList( pList );
	}

	// Remove the internet driver if we added it
	if(res != LT_ALREADYEXISTS)
		g_pLTClient->RemoveInternetDriver();


	return LTTRUE;
}


LTBOOL CFolderJoinLAN::SetService( )
{
	NetService *pCur, *pListHead;
	HNETSERVICE hNetService;

	pCur      = NULL;
	pListHead = NULL;
	hNetService = NULL;

    if( g_pLTClient->GetServiceList( pListHead ) != LT_OK || !pListHead )
        return LTFALSE;

	// Find the service specified.
	pCur = pListHead;
	while( pCur )
	{
		if( pCur->m_dwFlags & NETSERVICE_TCPIP )
		{
			hNetService = pCur->m_handle;
			break;
		}

		pCur = pCur->m_pNext;
	}

	// Free the service list.
    g_pLTClient->FreeServiceList( pListHead );

	// Check if tcp not found.
	if( !hNetService )
        return LTFALSE;

	// Select it.
    if( g_pLTClient->SelectService( hNetService ) != LT_OK )
        return LTFALSE;

    return LTTRUE;
}


LTBOOL CFolderJoinLAN::IsCurrentGame(CString sAddress)
{
	if (g_pGameClientShell->GetGameType() == SINGLE) return LTFALSE;

	char sTemp[MAX_SGR_STRINGLEN];
	strcpy(sTemp,(char *)(LPCTSTR)sAddress);
	char *pTok = strtok(sTemp,":");
	pTok = strtok(LTNULL,":");
	int p = atoi(pTok);
	return (g_pGameClientShell->CheckServerAddress(sTemp,p));

}


void JoinLANCallBack(LTBOOL bReturn, void *pData)
{
	CFolderJoinLAN *pThisFolder = (CFolderJoinLAN *)pData;
	if (bReturn && pThisFolder)
	{
		pThisFolder->JoinCurGame();
    }
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoinLAN::JoinGame
//
//	PURPOSE:	Joins the given game service
//
// ----------------------------------------------------------------------- //

void CFolderJoinLAN::JoinCurGame()
{
	// Get the currently selected game server...
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (!pCtrl) return;

	uint32 nIndex = pCtrl->GetParam1();
	if (nIndex > m_lstSessions.GetSize()) return;


	if (IsCurrentGame(m_lstSessions[nIndex]))
	{
        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (g_pGameClientShell->IsInWorld() && hPlayerObj)
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
		}
		return;
	}

	if (!DoJoinGame(m_lstSessions[nIndex]))
	{
		g_pInterfaceMgr->LoadFailed();
	}
}


LTBOOL CFolderJoinLAN::DoJoinGame(CString sAddress)
{
	// Sanity checks...

	if (sAddress.IsEmpty()) return(LTFALSE);

	if (g_pGameClientShell->IsInWorld() && g_pGameClientShell->GetGameType() != SINGLE)
		g_pInterfaceMgr->StartingNewGame();

	// Get the ip address...
	char sIp[MAX_SGR_STRINGLEN];
	strcpy(sIp,(char *)(LPCTSTR)sAddress);
//	char *pTok = strtok(sIp,":");
//	pTok = strtok(LTNULL,":");
//	int p = atoi(pTok);


	// Start the game...

	StartGameRequest req;
	NetClientData clientData;

	memset( &req, 0, sizeof( req ));


	// Setup our client...

	clientData.m_dwTeam = (uint32)GetConsoleInt("NetPlayerTeam",0);
	SAFE_STRCPY(clientData.m_sName,g_vtPlayerName.GetStr());

	req.m_pClientData = &clientData;
	req.m_ClientDataLen = sizeof( clientData );

	req.m_Type = STARTGAME_CLIENTTCP;
	strncpy( req.m_TCPAddress, sIp, MAX_SGR_STRINGLEN);


    LTRESULT dr = g_pLTClient->InitNetworking(NULL, 0);
	if (dr != LT_OK)
	{
//		s_nErrorString = IDS_NETERR_INIT;
//		NetStart_DisplayError(hInst);
		g_pLTClient->CPrint("InitNetworking() : error %d", dr);
        return(LTFALSE);
	}
	
	
	g_pInterfaceMgr->DrawFragCount(LTFALSE);

	HSTRING hWorld = g_pLTClient->FormatString(IDS_CONNECTING_TO_LAN);
	g_pInterfaceMgr->SetLoadLevelString(hWorld);
	g_pLTClient->FreeString(hWorld);
	g_pInterfaceMgr->SetLoadLevelPhoto("interface\\photos\\missions\\default.pcx");

	
	g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);

	int nRetries = GetConsoleInt("NetJoinRetry", 0);
	while (nRetries >= 0)
	{
		// If successful, then we're done.
        if( g_pLTClient->StartGame( &req ) == LT_OK )
		{
            return LTTRUE;
		}

		// Wait a sec and try again.
		Sleep(1000);
		nRetries--;
	}

	// All done...
    return(LTFALSE);
}


void CFolderJoinLAN::FindServers()
{
	if (!bIsInitting)
	{
        g_pLTClient->Start3D();
        g_pLTClient->StartOptimized2D();

		g_pInterfaceResMgr->DrawMessage(GetMediumFont(),IDS_LOOKING_FOR_SERVERS);

        g_pLTClient->EndOptimized2D();
        g_pLTClient->End3D();
        g_pLTClient->FlipScreen(0);

		g_pLTClient->CPrint("init sessions");
		bIsInitting = LTTRUE;
		InitSessions();

		if (m_lstSessions.GetSize() == 0)
		{
			AddTextItem(IDS_NO_SERVERS, LTNULL, LTNULL, LTTRUE, GetSmallFont());
		}
		g_pLTClient->ClearInput();
	}
}



/******************************************************************/
LTBOOL CFolderJoinLAN::OnLButtonUp(int x, int y)
{
	CLTGUICtrl *pCapture = GetCapture();
	if (pCapture)
	{
		return pCapture->OnEnter();
	}
	return CBaseFolder::OnLButtonUp(x,y);
}

/******************************************************************/
LTBOOL CFolderJoinLAN::OnRButtonUp(int x, int y)
{
	if (GetCapture())
	{
		Escape();
		return LTTRUE;
	}
	return CBaseFolder::OnRButtonUp(x,y);
}


void CFolderJoinLAN::Escape()
{
	if (GetCapture() == m_pPortEdit)
	{
        SetCapture(LTNULL);
		strcpy(m_szPort,szOldPort);
        m_pPortEdit->UpdateData(LTFALSE);
		m_pPortEdit->SetColor(m_hNonSelectedColor,m_hNonSelectedColor,m_hNonSelectedColor);
        m_pPortEdit->Select(LTFALSE);
        m_pPortLabel->Select(LTTRUE);
		ForceMouseUpdate();
	}
	else
		CBaseFolder::Escape();

}


LTBOOL CFolderJoinLAN::Render(HSURFACE hDestSurf)
{
	bIsInitting = LTFALSE;
	return CBaseFolder::Render(hDestSurf);
}


LTBOOL CFolderJoinLAN::HandleKeyDown(int key, int rep)
{
	if (key == VK_F5)
	{
		FindServers();
        return LTTRUE;
	}
    return CBaseFolder::HandleKeyDown(key,rep);
}