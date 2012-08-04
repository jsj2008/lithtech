// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenJoin.cpp
//
// PURPOSE : Interface screen to search for and join LAN games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenJoin.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"
#include "ClientMultiplayerMgr.h"
#include "msgids.h"
#include "MissionMgr.h"

#include "iserverdir.h"

#include "GameClientShell.h"

void JoinCallBack(LTBOOL bReturn, void *pData);

namespace
{
	int kMaxCDKeyLength = 24;
	int kColumnWidth = 300;
	int kColumnWidth_ServerName = 200;
	int kColumnWidth_MapName = 250;
	int kColumnWidth_Players = 100;
	int kListFontSize = 12;

	void EditCDKeyCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenJoin *pThisScreen = (CScreenJoin *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN);
		if (pThisScreen)
			pThisScreen->SendCommand(bReturn ? CMD_OK : CMD_CANCEL,(uint32)pData,CMD_EDIT_CDKEY);
	}

	void JoinLanConfirmationCallback(LTBOOL bReturn, void *pData)
	{
		CScreenJoin *pThisScreen = (CScreenJoin*)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN);
		if (pThisScreen)
			pThisScreen->SendCommand(bReturn ? CMD_OK : CMD_CANCEL,(uint32)pData,CMD_JOIN);
	}
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenJoin::CScreenJoin() :
	m_eCurState(eState_Inactive),
	m_nSelectedServer(0),
	m_sCurCDKey("")
{
}

CScreenJoin::~CScreenJoin()
{
}

// Build the screen
LTBOOL CScreenJoin::Build()
{
 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	CreateTitle(IDS_TITLE_JOIN);

	// Very, very temporary....
	// NYI NYI NYI

	int kColumn0 = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST,"ColumnWidth");
	int kColumn1 = (640 - GetPageLeft()) - kColumn0;

	m_pFindCtrl = AddTextItem(IDS_FIND_SERVERS,CMD_SEARCH,IDS_HELP_FIND_SERVERS);
	m_pFindCtrl->Enable(LTFALSE);

	m_pStatusCtrl = AddTextItem("Status: Waiting", 0, IDS_HELP_FIND_SERVERS);
	m_pStatusCtrl->Enable(LTFALSE);

	m_pCDKeyCtrl = AddColumnCtrl(CMD_EDIT_CDKEY, IDS_HELP_FIND_SERVERS);
	m_pCDKeyCtrl->AddColumn("CD Key", kColumn0);
	m_pCDKeyCtrl->AddColumn(m_sCurCDKey.c_str(), kColumn1, LTTRUE);

	CLTGUIColumnCtrl* pCtrl = AddColumnCtrl(LTNULL, LTNULL, kDefaultPos, LTTRUE);
	char aTemp[256];
	FormatString(IDS_SERVER_NAME,aTemp,sizeof(aTemp));
	pCtrl->AddColumn(aTemp, kColumnWidth_ServerName);
	pCtrl->AddColumn("Map", kColumnWidth_MapName);
	pCtrl->AddColumn("Players", kColumnWidth_Players);
	pCtrl->Enable(LTFALSE);

	uint16 height = GetPageBottom() - m_nextPos.y;
	m_pServerListCtrl = AddList(m_nextPos,height,LTTRUE,kColumnWidth);
	m_pServerListCtrl->SetScrollWrap(LTFALSE);

	return LTTRUE;
}

uint32 CScreenJoin::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
		case CMD_CANCEL :
		{
			ChangeState(eState_Waiting);
			break;
		}
	case CMD_OK:
		{
			return HandleCallback(dwParam1,dwParam2);
		}
		case CMD_SEARCH :
			{
			FindServers();
			break;
			}
		case CMD_EDIT_CDKEY :
		{
			ChangeState(eState_ChangeCDKey);
			break;
		}
	case CMD_JOIN:
		{
			if (dwParam1 >= m_cServerList.size())
				return 0;

			m_nSelectedServer = dwParam1;
		    if (g_pGameClientShell->IsWorldLoaded())
		    {
				if (IsCurrentGame(m_cServerList[m_nSelectedServer].m_sAddress.c_str()))
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
					mb.pFn = JoinLanConfirmationCallback;
					g_pInterfaceMgr->ShowMessageBox(IDS_ENDCURRENTGAME,&mb);
				}
		    }
		    else
		    {
				JoinCurGame();
		    }

			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

uint32 CScreenJoin::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch (dwParam2)
	{
		case CMD_JOIN :
		{
			JoinCurGame();
			break;
		}
		case CMD_EDIT_CDKEY :
		{
			IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();
			if (!pServerDir->SetCDKey((const char *)dwParam1))
			{
				// Tell them that's a bad CD key and exit this screen
				// NYI
				ChangeState(eState_Error);
			}
			else
			{
				m_sCurCDKey = ((const char *)dwParam1);
				m_pCDKeyCtrl->SetString(1,m_sCurCDKey.c_str());
				pServerDir->QueueRequest(IServerDirectory::eRequest_Validate_CDKey);
			}
			break;
		}
	}
	ChangeState(eState_Waiting);
	return 1;
}


// Change in focus
void    CScreenJoin::OnFocus(LTBOOL bFocus)
{
	// Update our state when the focus changes
	if (bFocus)
	{
		// Go!
		if (m_eCurState == eState_Inactive)
			ChangeState(eState_Startup);
		else
			ChangeState(eState_Waiting);
	}
	else 
	{
		// Don't go!
		if (m_eCurState == eState_Waiting)
			ChangeState(eState_Inactive);
	}

	if (bFocus)
	{
		m_pCDKeyCtrl->SetString(1,m_sCurCDKey.c_str());

		SetSelection(GetIndex(m_pFindCtrl));

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();
	}

	CBaseScreen::OnFocus(bFocus);
}

void CScreenJoin::FindServers()
{
	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();
	if (pServerDir->IsRequestPending(IServerDirectory::eRequest_Update_List))
		return;

	ChangeState(eState_UpdateDir);
}

void CScreenJoin::ReadCurServerList()
{
	m_nSelectedServer = 0;

	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();
	IServerDirectory::TPeerList cPeers = pServerDir->GetPeerList();
	m_cServerList.resize(cPeers.size());
	IServerDirectory::TPeerList::const_iterator iCurPeer = cPeers.begin();
	TServerList::iterator iCurServer = m_cServerList.begin();
	for (; iCurPeer != cPeers.end(); ++iCurPeer, ++iCurServer)
	{
		// Indicate that the server's an invalid entry until we get everything..
		iCurServer->m_sAddress.clear();

		char aStringBuffer[256];
	
		// Point at this server
		if (!pServerDir->SetActivePeer(iCurPeer->c_str()))
			continue;

		// Read the name
		CAutoMessage cMsg;
		if (!pServerDir->GetActivePeerInfo(IServerDirectory::ePeerInfo_Name, cMsg))
			continue;
		{
			CLTMsgRef_Read cRead(cMsg.Read());
			cRead->ReadString(aStringBuffer, sizeof(aStringBuffer));
		}
		iCurServer->m_sName = aStringBuffer;

		// Read the summary
		if (!pServerDir->GetActivePeerInfo(IServerDirectory::ePeerInfo_Summary, cMsg))
			continue;
		{
			CLTMsgRef_Read cRead(cMsg.Read());
			cRead->ReadString(aStringBuffer, sizeof(aStringBuffer));
			iCurServer->m_sMap = aStringBuffer;
			iCurServer->m_nNumPlayers = cRead->Readuint8();
			iCurServer->m_nMaxPlayers = cRead->Readuint8();
		}

		// Ok, this one's valid
		iCurServer->m_sAddress = *iCurPeer;
	}
}

void CScreenJoin::DisplayCurServerList()
{
	m_pServerListCtrl->RemoveAll();

	TServerList::const_iterator iCurServer = m_cServerList.begin();
	for (; iCurServer != m_cServerList.end(); ++iCurServer)
	{
		if (iCurServer->m_sAddress.empty())
			continue;

		// Create a control
			CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(CMD_JOIN, IDS_HELP_JOIN_LAN);
			pCtrl->SetFont(LTNULL,kListFontSize);
			// Do the name
		pCtrl->AddColumn(iCurServer->m_sName.c_str(), kColumnWidth_ServerName);
		// Do the map
		pCtrl->AddColumn(iCurServer->m_sMap.c_str(), kColumnWidth_MapName);
		// Do the number of players
		char aPlayerBuffer[256];
		sprintf(aPlayerBuffer, "%d/%d", iCurServer->m_nNumPlayers, iCurServer->m_nMaxPlayers);
		pCtrl->AddColumn(aPlayerBuffer, kColumnWidth_Players);
		// Remember where we came from
		pCtrl->SetParam1((uint32)(iCurServer - m_cServerList.begin()));

		// Add the server
		m_pServerListCtrl->AddControl(pCtrl);
	}
}

LTBOOL CScreenJoin::Render(HSURFACE hDestSurf)
{
	Update();
	return CBaseScreen::Render(hDestSurf);
}

LTBOOL CScreenJoin::HandleKeyDown(int key, int rep)
{
	if (key == VK_F5)
	{
		FindServers();
        return LTTRUE;
		}
    return CBaseScreen::HandleKeyDown(key,rep);
}

bool CScreenJoin::IsCurrentGame(const char *pAddr)
{
	if (!IsMultiplayerGame( )) return false;

	uint32 aAddr[4];
	uint32 nPort = 0;
	aAddr[0] = 0;
	aAddr[1] = 0;
	aAddr[2] = 0;
	aAddr[3] = 0;
	sscanf(pAddr, "%d.%d.%d.%d:%d", &aAddr[0], &aAddr[1], &aAddr[2], &aAddr[3], &nPort);

	uint8 nServerAddr[4];
	uint16 nServerPort;
	g_pLTClient->GetServerIPAddress(nServerAddr, &nServerPort);

	if ((aAddr[0] == (uint32)nServerAddr[0]) &&
		(aAddr[1] == (uint32)nServerAddr[1]) &&
		(aAddr[2] == (uint32)nServerAddr[2]) &&
		(aAddr[3] == (uint32)nServerAddr[3]) &&
		(nPort == (uint32)nServerPort))
		return true;
	else
		return false;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenJoin::JoinGame
//
//	PURPOSE:	Joins the given game service
//
// ----------------------------------------------------------------------- //

void CScreenJoin::JoinCurGame()
{
	// Get the currently selected game server...

	if (m_nSelectedServer >= m_cServerList.size())
		return;

	const CServerEntry &cServerAddr = m_cServerList[m_nSelectedServer];

	if (IsCurrentGame(cServerAddr.m_sAddress.c_str()))
	{
        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (g_pGameClientShell->IsWorldLoaded() && hPlayerObj)
		{
			ChangeState(eState_Inactive);
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
		}
		return;
	}

	// Check the number of players to make sure it's not full
	// NYI

	// Check the version number
	// NYI

	// Check for password protection...
	// NYI

	if (!DoJoinGame(m_cServerList[m_nSelectedServer]))
	{
		ChangeState(eState_Inactive);
		g_pInterfaceMgr->LoadFailed();
		g_pInterfaceMgr->ConnectionFailed(0);
	}
}


bool CScreenJoin::DoJoinGame(const CServerEntry &cServer)
{
	if( !g_pClientMultiplayerMgr->SetupClient( cServer.m_sAddress.c_str( )))
		return false;

	if( !g_pMissionMgr->StartGameAsClient( ))
		return false;

	return true;
}

bool CScreenJoin::PreState(EState eNewState)
{
	switch (eNewState)
	{
		case eState_Inactive :
			return PreState_Inactive();
		case eState_Startup :
			return PreState_Startup();
		case eState_UpdateDir :
			return PreState_UpdateDir();
		case eState_QueryDetails :
			return PreState_QueryDetails();
		case eState_Waiting :
			return PreState_Waiting();
		case eState_ChangeCDKey :
			return PreState_ChangeCDKey();
		default :
			return true;
	}
}

bool CScreenJoin::PostState(EState eNewState)
{
	switch (m_eCurState)
	{
		case eState_UpdateDir :
			return PostState_UpdateDir(eNewState);
		case eState_QueryDetails :
			return PostState_QueryDetails(eNewState);
		case eState_Waiting :
			return PostState_Waiting(eNewState);
		default :
			return true;
	}
}

bool CScreenJoin::ChangeState(EState eNewState)
{
	if (!PostState(eNewState))
	{
		return false;
	}

	m_eCurState = eState_Transition;

	if (!PreState(eNewState))
	{
		return false;
	}

	m_eCurState = eNewState;

	return true;
}

void CScreenJoin::Update()
{
	char aTempBuffer[256];
	sprintf(aTempBuffer, "Status: %s", g_pClientMultiplayerMgr->GetServerDir()->GetCurStatusString().c_str());
	m_pStatusCtrl->SetString(aTempBuffer);

	switch (m_eCurState)
	{
		case eState_Startup :
			Update_State_Startup();
			return;
		case eState_UpdateDir :
			Update_State_UpdateDir();
			return;
		default :
			return;
	}
}

bool CScreenJoin::PreState_Inactive()
{
	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();

	// Make sure we're not doing anything...
	pServerDir->ClearRequestList();

	// Make sure we don't have anything lying around...
	m_pServerListCtrl->RemoveAll();

	// Disable the "Find servers" control
	m_pFindCtrl->Enable(LTFALSE);

	return true;
}

bool CScreenJoin::PreState_Startup()
{
	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();
	// Make a serverdir if we don't have one
	// Note : Find a way to put this back in the game client shell!!!  It has to be
	// here right now so that we can delete it when we start up a server..
	// NYI NYI NYI
	if (!pServerDir)
	{
		pServerDir = Factory_Create_IServerDirectory_Titan();
		g_pClientMultiplayerMgr->SetServerDir(pServerDir);
		// Set the game's name
		pServerDir->SetGameName("TheOperative2");
		// Set the version
		pServerDir->SetVersion(g_pVersionMgr->GetBuild());
		// Set up the packet header
		CAutoMessage cMsg;
		cMsg.Writeuint8(11); // CMSG_MESSAGE
		cMsg.Writeuint8(MID_MULTIPLAYER_SERVERDIR);
		pServerDir->SetNetHeader(*cMsg.Read());
	}

	// Load the default CD key
	if (pServerDir->GetCDKey(&m_sCurCDKey))
		m_pCDKeyCtrl->SetString(1,m_sCurCDKey.c_str());

	bool bResult = true;
	bResult &= pServerDir->QueueRequest(IServerDirectory::eRequest_MOTD);
	bResult &= pServerDir->QueueRequest(IServerDirectory::eRequest_Version);
	if (!bResult)
	{
		ChangeState(eState_Error);
		return false;
	}

	// Set the CD key
	pServerDir->SetCDKey(m_sCurCDKey.c_str());

	bResult &= pServerDir->QueueRequest(IServerDirectory::eRequest_Validate_CDKey);
	if (!bResult)
	{
		ChangeState(eState_ChangeCDKey);
		return false;
	}

	return bResult;
}

bool CScreenJoin::PreState_UpdateDir()
{
	m_pFindCtrl->Enable(LTFALSE);

	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();

	pServerDir->ClearPeerList();
	if (!pServerDir->QueueRequest(IServerDirectory::eRequest_Update_List))
		return false;

	return true;
}

bool CScreenJoin::PreState_QueryDetails()
{
	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();

	// Get the active server
	if (m_nSelectedServer >= m_cServerList.size())
		return true;
	pServerDir->SetActivePeer(m_cServerList[m_nSelectedServer].m_sAddress.c_str());

	if (!pServerDir->QueueRequest(IServerDirectory::eRequest_Peer_Details))
		return true;

	return true;
}

bool CScreenJoin::PreState_ChangeCDKey()
		{
	// Show the CD key change dialog
	MBCreate mb;
	mb.eType = LTMB_EDIT;
	mb.pFn = EditCDKeyCallBack;
	mb.pString = m_sCurCDKey.c_str();
	mb.nMaxChars = kMaxCDKeyLength;
	g_pInterfaceMgr->ShowMessageBox("Change CD Key",&mb);
	return true;
}

bool CScreenJoin::PreState_Waiting()
{
	m_pFindCtrl->Enable(LTTRUE);

	return true;
		}

bool CScreenJoin::PostState_UpdateDir(EState eNewState)
{
	// Read the directory list
	ReadCurServerList();
	// Show it
	DisplayCurServerList();
	return true;
	}

bool CScreenJoin::PostState_QueryDetails(EState eNewState)
{
	// Read the details
	// NYI
	return true;
}

bool CScreenJoin::PostState_Waiting(EState eNewState)
{
	m_pFindCtrl->Enable(LTFALSE);

	return true;
}

void CScreenJoin::Update_State_Startup()
{
	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();

	// Are we still waiting?
	switch (pServerDir->GetCurStatus())
	{
		case IServerDirectory::eStatus_Processing : 
			return;
		case IServerDirectory::eStatus_Waiting : 
		{
			// Display the MOTD if it's new
			// NYI

			// Display a version warning/error if we're out of date
			// NYI

			// Update the directory list
			ChangeState(eState_UpdateDir);
			break;
		}
		case IServerDirectory::eStatus_Error : 
		{
			// Ignore errors in the MOTD/Version queries for now...
			// NYI
			IServerDirectory::ERequest eErrorRequest = pServerDir->GetLastErrorRequest();
			if ((eErrorRequest == IServerDirectory::eRequest_MOTD) || 
				(eErrorRequest == IServerDirectory::eRequest_Version))
			{
				pServerDir->ProcessRequestList();
				break;
			}

			// Whoops, something went wrong.
			// Drop out of this screen with an error
			// NYI
			break;
		}
		default :
{
			ASSERT(!"Unknown directory status encountered");
			ChangeState(eState_Waiting);
			break;
		}
	}
}

void CScreenJoin::Update_State_UpdateDir()
{
	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();

	// Are we still waiting?
	switch (pServerDir->GetCurStatus())
	{
		case IServerDirectory::eStatus_Processing : 
			return;
		case IServerDirectory::eStatus_Waiting : 
		{
			// Go back to waiting
			ChangeState(eState_Waiting);
			break;
		}
		case IServerDirectory::eStatus_Error : 
{
			// Whoops, something went wrong.
			// Drop out of this screen with an error
			// NYI
			break;
		}
		default :
	{
			ASSERT(!"Unknown directory status encountered");
			ChangeState(eState_Waiting);
			break;
		}
	}
}
