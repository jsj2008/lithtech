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
#include "ResShared.h"
#include "iserverdir.h"

#include "GameClientShell.h"

#include <algorithm>

void JoinCallBack(LTBOOL bReturn, void *pData);

namespace
{
	int kColumnWidth_ServerName		= 225;
	int kColumnWidth_PlayerName		= 225;
	int kColumnWidth_Ping			= 60;
	int kColumnWidth_Players		= 80;
	int kColumnWidth_Lock			= 25;
	int kColumnWidth_Mission		= 170;
	int kColumnWidth_Mod			= 170;

	enum eLocalCommands
	{
		CMD_SORT_NAME = CMD_CUSTOM+1,
		CMD_SORT_PING,
		CMD_SORT_PLAYER,
		CMD_SORT_LOCK,
		CMD_SORT_MISSION,
		CMD_SORT_MOD,
		CMD_FILTER_VERSION,
		CMD_FILTER_PLAYERS,
		CMD_FILTER_PING,
		CMD_FILTER_MOD,
		CMD_DETAILS,

	};


	void JoinConfirmationCallback(LTBOOL bReturn, void *pData)
	{
		CScreenJoin *pThisScreen = (CScreenJoin*)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN);
		if (bReturn && pThisScreen)
		{
			pThisScreen->CheckPassword();
		}
	}
	void EditPassCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenJoin *pThisScreen = (CScreenJoin *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_JOIN);
		if (bReturn && pThisScreen)
		{
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PASS);
		}
	};


}

class CServerSortName
{
public:

	bool operator()(const CServerEntry & x, const CServerEntry & y) const
	{
		return (stricmp(x.m_sName.c_str(), y.m_sName.c_str()) < 0 );
	}
};
class CServerSortMission
{
public:

	bool operator()(const CServerEntry & x, const CServerEntry & y) const
	{
		return (stricmp(x.m_sMission.c_str(), y.m_sMission.c_str()) < 0 );
	}
};
class CServerSortPing
{
public:

	bool operator()(const CServerEntry & x, const CServerEntry & y) const
	{
		return (x.m_nPing < y.m_nPing );
	}
};

class CServerSortPlayers
{
public:

	bool operator()(const CServerEntry & x, const CServerEntry & y) const
	{
		return (x.m_nNumPlayers < y.m_nNumPlayers );
	}
};

class CServerSortLock
{
public:

	bool operator()(const CServerEntry & x, const CServerEntry & y) const
	{
		return (x.m_bUsePassword || !y.m_bUsePassword );
	}
};

class CServerSortMod
{
public:

	bool operator()( const CServerEntry &x, const CServerEntry &y ) const
	{
		return (stricmp( x.m_sModName.c_str(), y.m_sModName.c_str() ) < 0);
	}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenJoin::CScreenJoin() :
	m_eCurState			( eState_Inactive ),
	m_nSelectedServer	( 0 ),
	m_nLastSort			( CMD_SORT_PING ),
	m_nVersionFilter	( 0 ),
	m_nPlayersFilter	( 0 ),
	m_nPingFilter		( 0 ),
	m_nModFilter		( 0 )
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

	//read layout info -----------------------------------------------
	uint8  headerFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"HeaderFontSize");
	LTRect serverRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_JOIN,"ServerRect");

	int totalwidth = serverRect.right - serverRect.left;
	kColumnWidth_Mission = totalwidth;

	kColumnWidth_ServerName = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"NameWidth");
	kColumnWidth_Mission -= kColumnWidth_ServerName;

	kColumnWidth_Mod = g_pLayoutMgr->GetScreenCustomInt( SCREEN_ID_JOIN, "ModWidth" );
	kColumnWidth_Mission -= kColumnWidth_Mod;

	kColumnWidth_Ping = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"PingWidth");
	kColumnWidth_Mission -= kColumnWidth_Ping;

	kColumnWidth_Players = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"PlayerWidth");
	kColumnWidth_Mission -= kColumnWidth_Players;

	kColumnWidth_Lock = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"LockWidth");
	kColumnWidth_Mission -= kColumnWidth_Lock;

	ASSERT(kColumnWidth_Mission > 0);
	if (kColumnWidth_Mission <= 0) kColumnWidth_Mission = 50;


	//find servers button -----------------------------------------------
	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN,"FindPos");
	m_pFindCtrl = AddTextItem(IDS_FIND_SERVERS,CMD_SEARCH,IDS_HELP_FIND_SERVERS,pos);
	m_pFindCtrl->Enable(LTFALSE);
	m_pFindCtrl->SetFont(NULL,headerFontSize);

	// server sort buttons -----------------------------------------------
	pos.x = serverRect.left;
	pos.y = serverRect.top;

	CLTGUITextCtrl* pCtrl = AddTextItem(IDS_SERVER_NAME,CMD_SORT_NAME,IDS_HELP_SERVER_NAME,pos);
	pCtrl->SetFont(NULL,headerFontSize);
	pos.x += kColumnWidth_ServerName;

	pCtrl = AddTextItem( IDS_SERVER_MOD, CMD_SORT_MOD, IDS_HELP_SERVER_MOD, pos );
	pCtrl->SetFont( NULL, headerFontSize );
	pos.x += kColumnWidth_Mod;

	pCtrl = AddTextItem(IDS_SERVER_PING,CMD_SORT_PING,IDS_HELP_SERVER_PING,pos);
	pCtrl->SetFont(NULL,headerFontSize);
	pos.x += kColumnWidth_Ping;

	pCtrl = AddTextItem(IDS_SERVER_PLAYERS,CMD_SORT_PLAYER,IDS_HELP_SERVER_PLAYERS,pos);
	pCtrl->SetFont(NULL,headerFontSize);
	pos.x += kColumnWidth_Players;

	pCtrl = AddTextItem(IDS_SERVER_LOCKED,CMD_SORT_LOCK,0,pos,LTTRUE);
	pCtrl->SetFont(NULL,headerFontSize);
	pCtrl->Enable(LTFALSE);
	pos.x += kColumnWidth_Lock;

	pCtrl = AddTextItem(IDS_SERVER_MISSION,CMD_SORT_MISSION,IDS_HELP_SERVER_MISSION,pos);
	pCtrl->SetFont(NULL,headerFontSize);

	// server list -----------------------------------------------
	pos.x = serverRect.left;
	pos.y += headerFontSize + 4;
	uint16 height = serverRect.bottom - pos.y;
	m_pServerListCtrl = AddList(pos,height,LTTRUE,totalwidth);
	m_pServerListCtrl->SetScrollWrap(LTFALSE);
	m_pServerListCtrl->SetFrameWidth(2);
	m_pServerListCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_pServerListCtrl->SetIndent(2);


	// filter buttons -----------------------------------------------
	pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN,"BaseFilterPos");
	pCtrl = AddTextItem(IDS_SERVER_FILTERS,CMD_SORT_MISSION,0,pos,LTTRUE);
	pCtrl->SetFont(NULL,headerFontSize);
	int kFilterNameWidth = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"FilterNameWidth");

	// Temporarily set the item spacing for the filter controls...

	int nOldSpacing = GetItemSpacing();
	int nFilterSpacing = g_pLayoutMgr->GetScreenCustomInt( SCREEN_ID_JOIN, "FilterItemSpacing" );
	SetItemSpacing( nFilterSpacing );

	CLTGUICycleCtrl* pCycle = AddCycle( IDS_VERSION_FILTER,IDS_HELP_VERSION_FILTER,kFilterNameWidth, &m_nVersionFilter);
	pCycle->NotifyOnChange(CMD_FILTER_VERSION,this);
	pCycle->AddString(LoadTempString(IDS_ALL_VERSIONS));
	pCycle->AddString(LoadTempString(IDS_CURRENT_VERSION));

	pCycle = AddCycle(IDS_POP_FILTER,IDS_HELP_POP_FILTER,kFilterNameWidth,&m_nPlayersFilter);
	pCycle->NotifyOnChange(CMD_FILTER_PLAYERS,this);
	pCycle->AddString(LoadTempString(IDS_FILTER_ALLPOP));
	pCycle->AddString(LoadTempString(IDS_FILTER_NOTEMPTY));
	pCycle->AddString(LoadTempString(IDS_FILTER_NOTFULL));
	pCycle->AddString(LoadTempString(IDS_FILTER_NOT_BOTH));

	pCycle = AddCycle(IDS_FILTER_PING,IDS_HELP_FILTER_PING,kFilterNameWidth,&m_nPingFilter);
	pCycle->NotifyOnChange(CMD_FILTER_PING,this);
	pCycle->AddString(LoadTempString(IDS_FILTER_ALLPING));
	pCycle->AddString(LoadTempString(IDS_FILTER_LOWPING));
	pCycle->AddString(LoadTempString(IDS_FILTER_MEDPING));
	pCycle->AddString(LoadTempString(IDS_FILTER_HIGHPING));

	// Reset the item spacing before the last filter control is added...

	SetItemSpacing( nOldSpacing );

	pCycle = AddCycle( IDS_FILTER_MOD, IDS_HELP_FILTER_MOD, kFilterNameWidth, &m_nModFilter);
	pCycle->NotifyOnChange( CMD_FILTER_MOD, this );
	pCycle->AddString( LoadTempString(IDS_FILTER_ALLMODS) );
	pCycle->AddString( LoadTempString(IDS_RETAIL) );
	pCycle->AddString( LoadTempString(IDS_FILTER_CURRENTMOD) );
	
			
	// status text -----------------------------------------------
	pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_JOIN,"StatusPos");
	char szTmp[256] = "";
	LoadString(IDS_WAITING,szTmp,sizeof(szTmp));
	m_pStatusCtrl = AddTextItem(FormatTempString(IDS_STATUS_STRING,szTmp), 0, 0, pos, LTTRUE);

	// "please wait" popup  window -----------------------------------------------
	uint8 nFont = g_pLayoutMgr->GetDialogFontFace();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	uint8 nFontSize = g_pLayoutMgr->GetDialogFontSize();

	CLTGUITextCtrl*	pText = debug_new(CLTGUITextCtrl);
    if (!pText->Create(LoadTempString(IDS_QUERYING), NULL, LTNULL, pFont, nFontSize, this))
	{
		debug_delete(pText);
        return LTFALSE;
	}
	pText->SetColors(argbBlack, argbBlack, argbBlack);
	pText->Enable(LTFALSE);

	uint16 w = 16+pText->GetBaseWidth();
	uint16 h = 16+pText->GetBaseHeight();


	m_pWait = debug_new(CLTGUIWindow);
	char szBack[128] = "";
	g_pLayoutMgr->GetDialogFrame(szBack,sizeof(szBack));
	m_pWait->Create(g_pInterfaceResMgr->GetTexture(szBack),w,h);
	uint16 x = (640-w)/2;
	uint16 y = (480-h)/2;
	m_pWait->SetBasePos(LTIntPt(x,y));
	AddControl(m_pWait);
	
	m_pWait->AddControl(pText,LTIntPt(8,8));

	

	// details popup  window -----------------------------------------------
	LTRect detailsRect = g_pLayoutMgr->GetScreenCustomRect(SCREEN_ID_JOIN,"DetailsRect");
	uint8  optionFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"OptionsFontSize");
	uint16  optionHeight = (uint16)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_JOIN,"OptionsHeight");
	pos.x = detailsRect.left;
	pos.y = detailsRect.top;
	uint16 nDetailsWidth = detailsRect.right-detailsRect.left;
	uint16 nDetailsHeight = detailsRect.bottom-detailsRect.top;

	m_pDetails = debug_new(CLTGUIWindow);
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_JOIN,"DetailsFrame",szBack,sizeof(szBack));
	m_pDetails->Create(g_pInterfaceResMgr->GetTexture(szBack),nDetailsWidth,nDetailsHeight);
	m_pDetails->SetBasePos(pos);
	AddControl(m_pDetails);

	pos.x = 8;
	pos.y = 8;

	
	CLTGUITextCtrl* pOption = CreateTextItem(IDS_HOST_OPTIONS,0,0,pos,LTTRUE);
	pOption->SetFont(NULL,headerFontSize);
	m_pDetails->AddControl(pOption,pos);
	pos.y += (headerFontSize + 4);

	uint16 width = nDetailsWidth - 32;
	kColumnWidth_PlayerName = width - kColumnWidth_Ping;

	m_pOptions = CreateTextItem(" ",0,0,pos,LTTRUE);
	m_pOptions->SetFixedWidth(width);
	m_pOptions->SetFont(NULL,optionFontSize);
	m_pDetails->AddControl(m_pOptions,pos);
	pos.y += (optionHeight + 4);

	CLTGUITextCtrl* pPlayer = CreateTextItem(IDS_SERVER_PLAYERS,0,0,pos,LTTRUE);
	pPlayer->SetFont(NULL,headerFontSize);
	m_pDetails->AddControl(pPlayer,pos);
	pos.x += kColumnWidth_PlayerName;

	CLTGUITextCtrl* pPing = CreateTextItem(IDS_SERVER_PING,0,0,pos,LTTRUE);
	pPing->SetFont(NULL,headerFontSize);
	m_pDetails->AddControl(pPing,pos);

	pos.x = 8;
	pos.y += (headerFontSize + 4);
	height = nDetailsHeight - 3*headerFontSize - optionHeight - 32;

	m_pPlayerListCtrl = CreateList(pos,height,LTTRUE,width);
	m_pPlayerListCtrl->SetScrollWrap(LTFALSE);
	m_pPlayerListCtrl->SetScrollByPage(LTTRUE);
	m_pPlayerListCtrl->SetFrameWidth(1);
	m_pPlayerListCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_pPlayerListCtrl->SetIndent(2);
	m_pPlayerListCtrl->Enable(LTTRUE);
	m_pDetails->AddControl(m_pPlayerListCtrl,pos);

	CLTGUITextCtrl* pCancel = CreateTextItem(IDS_CANCEL,CMD_CANCEL,0);
	pCancel->SetFont(NULL,headerFontSize);
	pos.y += (height + m_nItemSpacing);
	m_pDetails->AddControl(pCancel,pos);

	m_pJoin = CreateTextItem(IDS_JOIN_GAME,CMD_JOIN,IDS_HELP_JOIN_GAME);
	m_pJoin->SetFont(NULL,headerFontSize);
	pos.x = (nDetailsWidth - 8) - m_pJoin->GetBaseWidth();

	m_pDetails->AddControl(m_pJoin,pos);

	m_pDetails->Show(LTFALSE);

	return LTTRUE;
}

uint32 CScreenJoin::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_SORT_NAME:
	case CMD_SORT_PING:
	case CMD_SORT_PLAYER:
	case CMD_SORT_LOCK:
	case CMD_SORT_MISSION:
	case CMD_SORT_MOD:
	{
		SortServers(dwCommand);
		break;
	}

	case CMD_FILTER_VERSION:
	case CMD_FILTER_PING:
	case CMD_FILTER_PLAYERS:
	case CMD_FILTER_MOD:
	{
		UpdateData();
		FilterServers();
		break;
	} 

	case CMD_CANCEL :
	{
		m_pDetails->Show(LTFALSE);
		SetCapture(NULL);
		ChangeState(eState_Waiting);
		break;
	}

	case CMD_DETAILS :
	{
		if (dwParam1 >= m_cServerList.size())
			return 0;

		m_nSelectedServer = dwParam1;

		ChangeState(eState_QueryDetails);
		break;
	}

	case CMD_SEARCH :
	{
		FindServers();
		break;
	}

	case CMD_OK:
	{
		HandleCallback(dwParam1,dwParam2);
		break;
	}
	case CMD_JOIN:
	{

		m_pDetails->Show(LTFALSE);
		SetCapture(LTNULL);

		if (m_nSelectedServer >= m_cServerList.size())
			return 0;



		if (stricmp(m_cServerList[m_nSelectedServer].m_sVersion.c_str(), g_pVersionMgr->GetBuild()) != 0)
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox(IDS_SERVER_WRONGVERSION,&mb);
			return 1;
		}

		if (m_cServerList[m_nSelectedServer].m_nNumPlayers >= m_cServerList[m_nSelectedServer].m_nMaxPlayers)
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox(IDS_SERVERFULL,&mb);
			return 1;
		}

		if( stricmp( m_cServerList[m_nSelectedServer].m_sModName.c_str(), g_pClientMultiplayerMgr->GetModName() ) != 0 )
		{
			MBCreate mb;
			g_pInterfaceMgr->ShowMessageBox( IDS_SERVER_WRONGMOD, &mb );
			return 1;
		}

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
				mb.pFn = JoinConfirmationCallback;
				g_pInterfaceMgr->ShowMessageBox(IDS_ENDCURRENTGAME,&mb);
			}
		}
		else
		{
			CheckPassword();
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
		case CMD_EDIT_PASS :
		{
			m_sPassword = (char *)dwParam1;
			JoinCurGame();
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
		m_pWait->Show(LTTRUE);
		SetCapture(m_pWait);

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

		// we may have gotten here after failing a join, so rebuild our history
		if (m_pScreenMgr->GetLastScreenID() == SCREEN_ID_NONE)
		{
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MULTI);
		}

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
	if (pServerDir->IsRequestPending(IServerDirectory::eRequest_Update_Pings))
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
			iCurServer->m_sVersion = aStringBuffer;

			cRead->ReadString(aStringBuffer, sizeof(aStringBuffer));

			int nMission, nLevel;
			iCurServer->m_sMission = "";
			if	(g_pMissionButeMgr->IsMissionLevel(aStringBuffer,nMission,nLevel))
			{
				MISSION* pMission = g_pMissionButeMgr->GetMission(nMission); 
				if (pMission)
				{
					if (pMission->nNameId > 0)
						iCurServer->m_sMission = LoadTempString(pMission->nNameId);
					else if (!pMission->sName.empty())
						iCurServer->m_sMission = pMission->sName;
				}
			}
			
			if (iCurServer->m_sMission.empty())
				iCurServer->m_sMission = aStringBuffer;

			iCurServer->m_nNumPlayers = cRead->Readuint8();
			iCurServer->m_nMaxPlayers = cRead->Readuint8();
			iCurServer->m_bUsePassword = cRead->Readbool();
			iCurServer->m_nGameType = cRead->Readuint8();
			
			cRead->ReadString( aStringBuffer, sizeof(aStringBuffer) );
			iCurServer->m_sModName = (aStringBuffer[0] ? aStringBuffer : "Retail");
		}

		if (pServerDir->GetActivePeerInfo(IServerDirectory::ePeerInfo_Ping, cMsg))
		{
			CLTMsgRef_Read cRead(cMsg.Read());
			iCurServer->m_nPing = cRead->Readuint16();
		}
		else			
			iCurServer->m_nPing = -1;

		// Ok, this one's valid
		iCurServer->m_sAddress = *iCurPeer;
	}
	

	SortServers(m_nLastSort);

}

void CScreenJoin::DisplayCurServerList()
{
	m_pServerListCtrl->RemoveAll();

	uint16 nSelected = CLTGUIListCtrl::kNoSelection;

	TServerList::const_iterator iCurServer = m_cServerList.begin();
	for (; iCurServer != m_cServerList.end(); ++iCurServer)
	{
		if (iCurServer->m_sAddress.empty())
			continue;
		char aTempBuffer[256];

		// Create a control
		CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(CMD_DETAILS, IDS_HELP_JOIN);
		// Do the name
		pCtrl->AddColumn(iCurServer->m_sName.c_str(), kColumnWidth_ServerName,LTTRUE);

		// Do the mod...

		sprintf( aTempBuffer, "%s", iCurServer->m_sModName );
		pCtrl->AddColumn( aTempBuffer, kColumnWidth_Mod );

		// Do the ping
		sprintf(aTempBuffer, "%d", iCurServer->m_nPing);
		pCtrl->AddColumn(aTempBuffer, kColumnWidth_Ping);

		// Do the number of players
		sprintf(aTempBuffer, "%d/%d", iCurServer->m_nNumPlayers, iCurServer->m_nMaxPlayers);
		pCtrl->AddColumn(aTempBuffer, kColumnWidth_Players);


		if (iCurServer->m_bUsePassword)
		{
			pCtrl->AddColumn("x", kColumnWidth_Lock);
		}
		else
		{
			pCtrl->AddColumn(" ", kColumnWidth_Lock);
		}


		// Do the map
		pCtrl->AddColumn(iCurServer->m_sMission.c_str(), kColumnWidth_Mission,LTTRUE);

		// Remember where we came from
		uint16 nServerIndex = (uint32)(iCurServer - m_cServerList.begin());
		pCtrl->SetParam1(nServerIndex);

		// Add the server
		uint16 nCtrlIndex = m_pServerListCtrl->AddControl(pCtrl);

		if (nServerIndex = m_nSelectedServer)
			nSelected = nCtrlIndex;

	}

	m_pServerListCtrl->Enable( (m_pServerListCtrl->GetNumControls() > 0));
	m_pServerListCtrl->SetSelection(nSelected);
	FilterServers();
}

void CScreenJoin::ReadDetails()
{
	m_pPlayerListCtrl->RemoveAll();
	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();

	// Read the name
	CAutoMessage cMsg;
	if (!pServerDir->GetActivePeerInfo(IServerDirectory::ePeerInfo_Details, cMsg))
		return;

	CLTMsgRef_Read cRead(cMsg.Read());
	

	//read game options and build the options string
	std::string sOptions = "";


	switch (g_pGameClientShell->GetGameType())
	{
	case eGameTypeDeathmatch:
	case eGameTypeTeamDeathmatch:
	case eGameTypeDoomsDay:
		ReadDMDetails(sOptions,cRead);
		break;
	case eGameTypeCooperative:
		ReadCoopDetails(sOptions,cRead);
		break;
	}
	
	m_pOptions->SetString(sOptions.c_str());

	m_pJoin->Enable(LTTRUE);
	m_pDetails->Show(LTTRUE);
	m_pDetails->SetSelection(m_pDetails->GetIndex(m_pJoin));
	SetCapture(m_pDetails);

}
void CScreenJoin::SetDetailErrorMessage(const char *szError)
{
	m_pPlayerListCtrl->RemoveAll();

	if (m_pWait->IsVisible())
		m_pWait->Show(LTFALSE);
	if (m_pDetails->IsVisible())
		m_pDetails->Show(LTFALSE);
	SetCapture(NULL);

	//show edit box here	
	MBCreate mb;
	g_pInterfaceMgr->ShowMessageBox(szError,&mb);

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

	if (!DoJoinGame(m_cServerList[m_nSelectedServer]))
	{
		ChangeState(eState_Inactive);
		g_pInterfaceMgr->LoadFailed(SCREEN_ID_JOIN);
		g_pInterfaceMgr->ConnectionFailed(g_pClientMultiplayerMgr->GetLastConnectionResult());
	}
}


bool CScreenJoin::DoJoinGame(const CServerEntry &cServer)
{
	if( !g_pClientMultiplayerMgr->SetupClient( cServer.m_sAddress.c_str( ), cServer.m_sName.c_str(), m_sPassword.c_str() ))
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
	if (!PostState(m_eCurState))
	{
		return false;
	}

	m_eCurState = eState_Transition;

	if (!PreState(eNewState))
	{
		m_pFindCtrl->Enable(LTTRUE);
		if (m_pWait->IsVisible())
			m_pWait->Show(LTFALSE);
		if (GetCapture() == m_pWait)
			SetCapture(NULL);
		m_eCurState = eState_Waiting;
		return false;
	}

	m_eCurState = eNewState;

	return true;
}

void CScreenJoin::Update()
{
	char aTempBuffer[256];

	FormatString(IDS_STATUS_STRING,aTempBuffer,sizeof(aTempBuffer),g_pClientMultiplayerMgr->GetServerDir()->GetCurStatusString());
	m_pStatusCtrl->SetString(aTempBuffer);

	switch (m_eCurState)
	{
		case eState_Startup :
			Update_State_Startup();
			return;
		case eState_UpdateDir :
			Update_State_UpdateDir();
			return;
		case eState_QueryDetails :
			Update_State_QueryDetails();
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
		pServerDir = g_pClientMultiplayerMgr->CreateServerDir( );
	}

	return true;
}

bool CScreenJoin::PreState_UpdateDir()
{
	m_pFindCtrl->Enable(LTFALSE);

	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();

	pServerDir->ClearPeerList();
	if (!pServerDir->QueueRequest(IServerDirectory::eRequest_Update_List))
		return false;
	if (!pServerDir->QueueRequest(IServerDirectory::eRequest_Update_Pings))
		return false;

	return true;
}

bool CScreenJoin::PreState_QueryDetails()
{
	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();
	
	// Get the active server
	if (m_nSelectedServer >= m_cServerList.size())
	{
		return false;
	}

	if (stricmp(m_cServerList[m_nSelectedServer].m_sVersion.c_str(), g_pVersionMgr->GetBuild()) != 0)
	{
		SetDetailErrorMessage(LoadTempString(IDS_SERVER_DETAIL_VERSION));
		return false;
	}

	pServerDir->SetActivePeer(m_cServerList[m_nSelectedServer].m_sAddress.c_str());

	if (!pServerDir->QueueRequest(IServerDirectory::eRequest_Peer_Details))
	{
		SetDetailErrorMessage(LoadTempString(IDS_SERVER_NORESPONSE));
		return false;
	}

	return true;
}


bool CScreenJoin::PreState_Waiting()
{
	m_pFindCtrl->Enable(LTTRUE);

	if (m_pWait->IsVisible())
		m_pWait->Show(LTFALSE);
	if (GetCapture() == m_pWait)
		SetCapture(NULL);


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
	m_pFindCtrl->Enable(LTTRUE);
	return true;
}

bool CScreenJoin::PostState_Waiting(EState eNewState)
{
	m_pFindCtrl->Enable(LTFALSE);

	m_pWait->Show(LTTRUE);
	SetCapture(m_pWait);

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
			// Update the directory list
			ChangeState(eState_UpdateDir);
			break;
		}
		case IServerDirectory::eStatus_Error : 
		{
			IServerDirectory::ERequest eErrorRequest = pServerDir->GetLastErrorRequest();

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

void CScreenJoin::Update_State_QueryDetails()
{
	IServerDirectory *pServerDir = g_pClientMultiplayerMgr->GetServerDir();

	// Are we still waiting?
	switch (pServerDir->GetCurStatus())
	{
		case IServerDirectory::eStatus_Processing : 
			return;
		case IServerDirectory::eStatus_Waiting : 
		{
			// Read the details
			ReadDetails();

			// Go back to waiting
			ChangeState(eState_Waiting);
			break;
		}
		case IServerDirectory::eStatus_Error : 
		{
			// Whoops, something went wrong. Tell us about it...
			SetDetailErrorMessage(g_pClientMultiplayerMgr->GetServerDir()->GetCurStatusString());

			//get back to work
			pServerDir->ProcessRequestList();
			ChangeState(eState_Waiting);
			

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

void CScreenJoin::SortServers(uint32 nSortField)
{
	switch(nSortField)
	{
	case CMD_SORT_NAME:
		std::sort(m_cServerList.begin(),m_cServerList.end(),CServerSortName());
		break;
	case CMD_SORT_PING:
		std::sort(m_cServerList.begin(),m_cServerList.end(),CServerSortPing());
		break;
	case CMD_SORT_PLAYER:
		std::sort(m_cServerList.begin(),m_cServerList.end(),CServerSortPlayers());
		break;
	case CMD_SORT_LOCK:
		std::sort(m_cServerList.begin(),m_cServerList.end(),CServerSortLock());
		break;
	case CMD_SORT_MISSION:
		std::sort(m_cServerList.begin(),m_cServerList.end(),CServerSortMission());
		break;
	case CMD_SORT_MOD:
		std::sort( m_cServerList.begin(), m_cServerList.end(), CServerSortMod() );
		break;

	default:
		ASSERT(!"Unknown sort command.");
		return;

	};

	DisplayCurServerList();
	m_nLastSort = nSortField;
}

void CScreenJoin::FilterServers()
{

	uint16 iFirst = CLTGUIListCtrl::kNoSelection; 
	for (uint16 i = 0 ;i < m_pServerListCtrl->GetNumControls(); ++i)
	{
		uint32 nIndex = m_pServerListCtrl->GetControl(i)->GetParam1();
		if (nIndex >= m_cServerList.size())
			return;

		bool bShow = true;

		//Version Filter
		if (m_nVersionFilter)
		{
			bShow &= (stricmp(m_cServerList[nIndex].m_sVersion.c_str(), g_pVersionMgr->GetBuild()) == 0);
		}

		//Game type filter
		uint8 nGameType = m_cServerList[nIndex].m_nGameType;
		//since versions 1.1 and 1.1 didn't have this they'll report 0 for co-op games
		if (nGameType == 0)
			nGameType = eGameTypeCooperative;

		if (g_pGameClientShell->GetGameType() != nGameType)
		{
			bShow = false;
		}
		

		switch (m_nPlayersFilter)
		{
		case 0:
			break;
		case 1:
			bShow &= (m_cServerList[nIndex].m_nNumPlayers > 0);
			break;
		case 2:
			bShow &= (m_cServerList[nIndex].m_nNumPlayers < m_cServerList[nIndex].m_nMaxPlayers);
			break;
		case 3:
			bShow &= (m_cServerList[nIndex].m_nNumPlayers > 0);
			bShow &= (m_cServerList[nIndex].m_nNumPlayers < m_cServerList[nIndex].m_nMaxPlayers);
			break;
		}

		switch (m_nPingFilter)
		{
		case 0:
			break;
		case 1:
			bShow &= (m_cServerList[nIndex].m_nPing < 100);
			break;
		case 2:
			bShow &= (m_cServerList[nIndex].m_nPing < 300);
			break;
		case 3:
			bShow &= (m_cServerList[nIndex].m_nPing < 500);
			break;
		}

		switch( m_nModFilter )
		{
			case 0:		// All Mods (no filtering)
			break; 
		
			case 1:		// Retail games only
			{
				bShow &= (stricmp( m_cServerList[nIndex].m_sModName.c_str(), LoadTempString( IDS_RETAIL )) == 0 );
			}
			break;
		
			case 2:		// Currently selected mod only
			{
				bShow &= (stricmp( m_cServerList[nIndex].m_sModName.c_str(), g_pClientMultiplayerMgr->GetModName() ) == 0 );
			}
			break;

			default:
			break;
		}

		if (bShow && iFirst == CLTGUIListCtrl::kNoSelection)
			iFirst = i;

		m_pServerListCtrl->GetControl(i)->Show(bShow);
		m_pServerListCtrl->GetControl(i)->Enable(bShow);

	}

	if (iFirst == CLTGUIListCtrl::kNoSelection)
	{
		m_pServerListCtrl->Enable(false);
	}
	else
	{
		m_pServerListCtrl->Enable(true);
		m_pServerListCtrl->SetStartIndex(iFirst);
	}

	m_pServerListCtrl->SetSelection(iFirst);
	m_nSelectedServer = iFirst;
}



void CScreenJoin::CheckPassword()
{
	if (m_nSelectedServer >= m_cServerList.size())
		return;

	const CServerEntry &cServerAddr = m_cServerList[m_nSelectedServer];



	m_sPassword = "";
	if (cServerAddr.m_bUsePassword)
	{	
		//show edit box here	
		MBCreate mb;
		mb.eType = LTMB_EDIT;
		mb.pFn = EditPassCallBack;
		mb.pString = "";
		mb.nMaxChars = MAX_PASSWORD-1;
		g_pInterfaceMgr->ShowMessageBox(IDS_PASSWORD,&mb);
	}
	else
	{
		JoinCurGame();
	}
}

void CScreenJoin::Escape()
{
	if (m_pDetails->IsVisible())
	{
		m_pDetails->Show(LTFALSE);
		SetCapture(NULL);
		ChangeState(eState_Waiting);
		return;
	}


	CBaseScreen::Escape();
}


void CScreenJoin::ReadDMDetails(std::string& sOptions,CLTMsgRef_Read& cRead)
{
	char aStringBuffer[256];
	bool bSkills = cRead->Readbool();
/*
	sprintf(aStringBuffer,"%s: ",LoadTempString(IDS_USE_SKILLS));
	sOptions += aStringBuffer;
	if (bSkills)
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_YES));
	else
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NO));
	sOptions += aStringBuffer;
*/
	bool bFriendlyFire = cRead->Readbool();
	sprintf(aStringBuffer,"%s: ",LoadTempString(IDS_FRIENDLY_FIRE));
	sOptions += aStringBuffer;
	if (bFriendlyFire)
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_YES));
	else
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NO));
	sOptions += aStringBuffer;

	//ignore diff for DM
	uint8 nDiff = cRead->Readuint8();
	float fPlayerDiff = cRead->Readfloat();

	//read player list
	bool bPlayerToRead = cRead->Readbool();
	while (bPlayerToRead)
	{
		cRead->ReadString(aStringBuffer, sizeof(aStringBuffer));
		uint16 nPing = cRead->Readuint16();
		bPlayerToRead = cRead->Readbool();

		// Create a control
		CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(0,0,kDefaultPos,LTTRUE);
		// Do the name
		pCtrl->AddColumn(aStringBuffer, kColumnWidth_PlayerName,LTTRUE);

		// Do the ping
		sprintf(aStringBuffer, "%d", nPing);
		pCtrl->AddColumn(aStringBuffer, kColumnWidth_Ping);

		m_pPlayerListCtrl->AddControl(pCtrl);
	}


	uint8 nRunSpeed = cRead->Readuint8();
	sprintf(aStringBuffer,"%s: %0.2f",LoadTempString(IDS_RUN_SPEED), (float)nRunSpeed/100.0f);
	sOptions += aStringBuffer;

	uint8 nScoreLimit = cRead->Readuint8();
	sprintf(aStringBuffer,"; %s: %d",LoadTempString(IDS_SCORE_LIMIT), nScoreLimit);
	sOptions += aStringBuffer;

	uint8 nTimeLimit = cRead->Readuint8();
	sprintf(aStringBuffer,"; %s: %d",LoadTempString(IDS_TIME_LIMIT), nTimeLimit);
	sOptions += aStringBuffer;
}


void CScreenJoin::ReadCoopDetails(std::string& sOptions,CLTMsgRef_Read& cRead)
{
	char aStringBuffer[256];
	bool bSkills = cRead->Readbool();
	sprintf(aStringBuffer,"%s: ",LoadTempString(IDS_USE_SKILLS));
	sOptions += aStringBuffer;
	if (bSkills)
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_YES));
	else
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NO));
	sOptions += aStringBuffer;

	bool bFriendlyFire = cRead->Readbool();
	sprintf(aStringBuffer,"%s: ",LoadTempString(IDS_FRIENDLY_FIRE));
	sOptions += aStringBuffer;
	if (bFriendlyFire)
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_YES));
	else
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NO));
	sOptions += aStringBuffer;

	uint8 nDiff = cRead->Readuint8();
	sprintf(aStringBuffer,"%s: ",LoadTempString(IDS_DIFFICULTY));
	sOptions += aStringBuffer;
	switch  (nDiff)
	{
	case 3:
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NEW_INSANE));
		break;
	case 2:
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NEW_HARD));
		break;
	case 1:
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NEW_MEDIUM));
		break;
	case 0:
	default:
		sprintf(aStringBuffer,"%s; ",LoadTempString(IDS_NEW_EASY));
		break;

	}
	sOptions += aStringBuffer;

	float fPlayerDiff = cRead->Readfloat();
	sprintf(aStringBuffer,"%s: %0.2f; ",LoadTempString(IDS_HOST_PLAYERDIFF),fPlayerDiff);
	sOptions += aStringBuffer;

	//read player list
	bool bPlayerToRead = cRead->Readbool();
	while (bPlayerToRead)
	{
		cRead->ReadString(aStringBuffer, sizeof(aStringBuffer));
		uint16 nPing = cRead->Readuint16();
		bPlayerToRead = cRead->Readbool();

		// Create a control
		CLTGUIColumnCtrl* pCtrl = CreateColumnCtrl(0,0,kDefaultPos,LTTRUE);
		// Do the name
		pCtrl->AddColumn(aStringBuffer, kColumnWidth_PlayerName,LTTRUE);

		// Do the ping
		sprintf(aStringBuffer, "%d", nPing);
		pCtrl->AddColumn(aStringBuffer, kColumnWidth_Ping);

		m_pPlayerListCtrl->AddControl(pCtrl);

	}
}