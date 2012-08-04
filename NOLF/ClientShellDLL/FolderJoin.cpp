// FolderJoin.cpp: implementation of the CFolderJoin class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderJoin.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "NetDefs.h"
#include "MsgIDs.h"
#include "ClientButeMgr.h"
#include "VarTrack.h"
#include "NetDefs.h"
#include "WinUtil.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

extern VarTrack g_vtPlayerName;


// Defines...

#define FREQ_SEL_PING		30000
#define FREQ_SEL_ALLDATA	60000

#define GAMESPY_GAME_NAME	"nolf"		// "ut"		
#define GAMESPY_SECRET_KEY	"Jn3Ab4"	// "HA6zkS"

#define FSS_SORT_PING		0
#define FSS_SORT_NAME		1
#define FSS_SORT_PLAYERS	2
#define FSS_SORT_FRAGS		3
#define FSS_SORT_GAME		4
#define FSS_SORT_MAP		5


VarTrack	g_vtNetVersionFilter;
VarTrack	g_vtNetGameFilter;
VarTrack	g_vtNetPopFilter;
VarTrack	g_vtNetServerSortKey;
VarTrack	g_vtNetPlayerSortKey;

void JoinCallBack(LTBOOL bReturn, void *pData);

namespace
{
	LTBOOL bDblClick = LTFALSE;
	enum LocalCommands
	{
		CMD_SELECT_SERVER = FOLDER_CMD_CUSTOM + 1,
		CMD_REFRESH_SERVERS,
		CMD_REPING_SERVERS,
		CMD_GAME_FILTER,
		CMD_POP_FILTER,
		CMD_VERSION_FILTER,
		CMD_SORT_SERV_NAME,
		CMD_SORT_SERV_PLAYERS,
		CMD_SORT_SERV_PING,
		CMD_SORT_SERV_GAME,
		CMD_SORT_SERV_MAP,
		CMD_SORT_PLYR_NAME,
		CMD_SORT_PLYR_SCORE,
		CMD_SORT_PLYR_PING,
		CMD_JOIN,
		CMD_EDIT_PASS,

	};
    LTRect rcServerRect;
    LTRect rcPlayerRect;
    LTRect rcOptionRect;
    LTRect rcStatusRect;
    LTRect rcCommandRect;

	int nBarHeight;
	int nGap;
	int nIndent;

	int nGameWidth;
	int nPlayerWidth;
	int nPingWidth;
	int nTypeWidth;
	int nMapWidth;
	int nNameWidth;
	int nScoreWidth;
	int nOptionWidth;

	int nArrowWidth;
	int nServerGroupWidth;
	int nPlayerGroupWidth;
	int nOptionGroupWidth;

	int nOldIndex;

	int anGameFilterID[3] =
	{
		IDS_FILTER_ALLGAMES,
		IDS_FILTER_COOP,
		IDS_FILTER_DM
	};


	int anPopFilterID[4] =
	{
		IDS_FILTER_ALLPOP,
		IDS_FILTER_NOTEMPTY,
		IDS_FILTER_NOTFULL,
		IDS_FILTER_NOT_BOTH
	};

    CLTGUICtrl *pServerNameCtrl = LTNULL;
    CLTGUICtrl *pServerPlayersCtrl = LTNULL;
    CLTGUICtrl *pServerPingCtrl = LTNULL;
    CLTGUICtrl *pServerGameCtrl = LTNULL;
    CLTGUICtrl *pServerMapCtrl = LTNULL;
    CLTGUICtrl *pPlayerNameCtrl = LTNULL;
    CLTGUICtrl *pPlayerFragCtrl = LTNULL;
    CLTGUICtrl *pPlayerPingCtrl = LTNULL;

    LTBOOL bFrameDelay = LTTRUE;
	uint32 nConnectError = 0;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderJoin::CFolderJoin()
{
	m_nState             = FSS_IDLE;
    m_bForceNextUpdate   = LTFALSE;
    m_bNeedServerSorting = LTFALSE;

	m_nNumServers       = 0;
	m_nNumServersListed = 0;
	m_nPopFilter        = POP_FILTER_ALL;
	m_nGameFilter       = NGT_FILTER_ALL;


	m_nServerSort = FSS_SORT_PING;
	m_nPlayerSort = FSS_SORT_FRAGS;

    m_pServerList = LTNULL;
    m_pPlayerList = LTNULL;
    m_pOptionList = LTNULL;

    m_pRefresh = LTNULL;
    m_pRePing = LTNULL;
    m_pVersionFilter = LTNULL;
    m_pGameFilter = LTNULL;
    m_pPopFilter = LTNULL;
    m_pResort = LTNULL;
    m_pJoin = LTNULL;

    g_vtNetVersionFilter.Init(g_pLTClient,"NetVersionFilter",LTNULL,0.0f);
    g_vtNetGameFilter.Init(g_pLTClient,"NetGameFilter",LTNULL,(float)m_nGameFilter);
    g_vtNetPopFilter.Init(g_pLTClient,"NetPopFilter",LTNULL,(float)m_nPopFilter);
    g_vtNetServerSortKey.Init(g_pLTClient,"NetServerSortKey",LTNULL,(float)m_nServerSort);
    g_vtNetPlayerSortKey.Init(g_pLTClient,"NetPlayerSortKey",LTNULL,(float)m_nPlayerSort);

	m_bAskingForPassword = LTFALSE;
	m_szPassword[0] = LTNULL;
	m_pPassEdit = LTNULL;
	m_pPassLabel = LTNULL;
	m_pPassBack = LTNULL;

	m_hServersShown = LTNULL;
	m_hDummyStatus = LTNULL;
	m_hStatus = LTNULL;

}

CFolderJoin::~CFolderJoin()
{
	GetGameSpyClientMgr()->Term();
    m_pCurServerHandle = LTNULL;
	m_nState           = FSS_IDLE;
}

void CFolderJoin::Term()
{
	CBaseFolder::Term();

	if (m_hServersShown)
	{
		g_pLTClient->FreeString(m_hServersShown);
	}

	if (m_hDummyStatus)
	{
		g_pLTClient->FreeString(m_hDummyStatus);
	}

	if (m_hStatus)
	{
		g_pLTClient->FreeString(m_hStatus);
	}



}

// Build the folder
LTBOOL CFolderJoin::Build()
{
	rcServerRect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"ServerRect");
	rcPlayerRect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"PlayerRect");
	rcOptionRect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"OptionRect");
	rcStatusRect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"StatusRect");
	rcCommandRect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"CommandRect");

	nGap = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"ColumnGap");
	nIndent = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"TextIndent");

	nGameWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"GameWidth");
	nPlayerWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"PlayerWidth");
	nPingWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"PingWidth");
	nTypeWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"TypeWidth");
	nMapWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"MapWidth");
	nNameWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"NameWidth");
	nScoreWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"ScoreWidth");
	nOptionWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"OptionWidth");

	nArrowWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"ArrowWidth");

	CreateTitle(IDS_TITLE_JOIN);

	//Add Server Header
    LTIntPt pos(rcServerRect.left + nIndent,rcServerRect.top);
	CLTGUIFont *pFont = GetMediumFont();
	nBarHeight = pFont->GetHeight();


	CStaticTextCtrl *pCtrl = CreateStaticTextItem(IDS_SERVER_NAME,CMD_SORT_SERV_NAME,IDS_HELP_SERVER_NAME,nGameWidth,pFont->GetHeight(),LTFALSE,pFont);
    AddFixedControl(pCtrl,pos,LTTRUE);
	pServerNameCtrl = pCtrl;
	pos.x += nGameWidth+nGap;

	pCtrl = CreateStaticTextItem(IDS_SERVER_PLAYERS,CMD_SORT_SERV_PLAYERS,IDS_HELP_SERVER_PLAYERS,nPlayerWidth,pFont->GetHeight(),LTFALSE,pFont);
    AddFixedControl(pCtrl,pos,LTTRUE);
	pServerPlayersCtrl = pCtrl;
	pos.x += nPlayerWidth+nGap;

	pCtrl = CreateStaticTextItem(IDS_SERVER_PING,CMD_SORT_SERV_PING,IDS_HELP_SERVER_PING,nPingWidth,pFont->GetHeight(),LTFALSE,pFont);
    AddFixedControl(pCtrl,pos,LTTRUE);
	pServerPingCtrl = pCtrl;
	pos.x += nPingWidth+nGap;

	pFont = GetMediumFont();
    pCtrl = CreateStaticTextItem(IDS_SERVER_GAME,CMD_SORT_SERV_GAME,LTNULL,nTypeWidth,pFont->GetHeight(),LTFALSE,pFont);
    AddFixedControl(pCtrl,pos,LTFALSE);
	pServerGameCtrl = pCtrl;
	pos.x += nTypeWidth+nGap;

    pCtrl = CreateStaticTextItem(IDS_SERVER_MAP,CMD_SORT_SERV_MAP,LTNULL,nMapWidth,pFont->GetHeight(),LTFALSE,pFont);
	pServerMapCtrl = pCtrl;
    AddFixedControl(pCtrl,pos,LTFALSE);

	//Add Server List
	pos.x = rcServerRect.left + nIndent;
	pos.y = rcServerRect.top + nBarHeight;
	int nListHeight = (rcServerRect.bottom - rcServerRect.top) - nBarHeight;
	nListHeight += 19;	// [blg] tweak

	nServerGroupWidth = (rcServerRect.right - rcServerRect.left) - nArrowWidth;
	m_pServerList = debug_new(CListCtrl);
    m_pServerList->Create(nListHeight, LTTRUE, nServerGroupWidth);
	m_pServerList->SetItemSpacing(0);
    m_pServerList->EnableMouseClickSelect(LTTRUE);
    AddFixedControl(m_pServerList,pos,LTTRUE);

	//Add Player Header
	pFont = GetMediumFont();
    pos = LTIntPt(rcPlayerRect.left + nIndent,rcPlayerRect.top);

	pCtrl = CreateStaticTextItem(IDS_JOIN_PLAYER_NAME,CMD_SORT_PLYR_NAME,IDS_HELP_SORT_PLYR_NAME,nNameWidth,pFont->GetHeight(),LTFALSE,pFont);
    AddFixedControl(pCtrl,pos,LTTRUE);
	pPlayerNameCtrl = pCtrl;
	pos.x += nNameWidth+nGap;

	pCtrl = CreateStaticTextItem(IDS_JOIN_PLAYER_SCORE,CMD_SORT_PLYR_SCORE,IDS_HELP_SORT_PLYR_SCORE,nScoreWidth,pFont->GetHeight(),LTFALSE,pFont);
    AddFixedControl(pCtrl,pos,LTTRUE);
	pPlayerFragCtrl = pCtrl;
	pos.x += nScoreWidth+nGap;

	pCtrl = CreateStaticTextItem(IDS_SERVER_PING,CMD_SORT_PLYR_PING,IDS_HELP_SORT_PLYR_PING,nPingWidth,pFont->GetHeight(),LTFALSE,pFont);
	pPlayerPingCtrl = pCtrl;
    AddFixedControl(pCtrl,pos,LTTRUE);

	//Add PlayerList Here
	pos.x = rcPlayerRect.left + nIndent;
	pos.y = rcPlayerRect.top + nBarHeight;
	nListHeight = (rcPlayerRect.bottom - rcPlayerRect.top) - nBarHeight;
	nListHeight += 24;	// [blg] tweak
	nPlayerGroupWidth = (rcPlayerRect.right - rcPlayerRect.left) - nArrowWidth;
	m_pPlayerList = debug_new(CListCtrl);
    m_pPlayerList->Create(nListHeight, LTTRUE, nPlayerGroupWidth);
	m_pPlayerList->SetItemSpacing(0);
//    m_pPlayerList->Enable(LTFALSE);
    AddFixedControl(m_pPlayerList,pos,LTTRUE);


	//Add Commands
	pFont = GetMediumFont();
	pos.x = rcCommandRect.left;
	pos.y = rcCommandRect.top;
	m_pRefresh = CreateTextItem(IDS_SERVER_REFRESH,CMD_REFRESH_SERVERS,IDS_HELP_REFRESH,LTFALSE,pFont);
    AddFixedControl(m_pRefresh,pos,LTTRUE);
	pos.y += pFont->GetHeight();

	// [blg] re-ping is not support in the GameSpy SDK
	//m_pRePing = CreateTextItem(IDS_SERVER_REPING,CMD_REPING_SERVERS,IDS_HELP_REPING,LTFALSE,pFont);
    //AddFixedControl(m_pRePing,pos,LTTRUE);
	//pos.y += pFont->GetHeight();

	m_pVersionFilter = CreateTextItem(IDS_ALL_VERSIONS,CMD_VERSION_FILTER,IDS_HELP_VERSION_FILTER,LTFALSE,pFont,&m_nVersionFilter);
	m_pVersionFilter->AddString(IDS_CURRENT_VERSION);
    AddFixedControl(m_pVersionFilter,pos,LTTRUE);
	pos.y += pFont->GetHeight();

	m_pGameFilter = CreateTextItem(anGameFilterID[0],CMD_GAME_FILTER,IDS_HELP_GAME_FILTER,LTFALSE,pFont,&m_nGameFilter);
	m_pGameFilter->AddString(anGameFilterID[1]);
	m_pGameFilter->AddString(anGameFilterID[2]);
    AddFixedControl(m_pGameFilter,pos,LTTRUE);
	pos.y += pFont->GetHeight();

	m_pPopFilter = CreateTextItem(anPopFilterID[0],CMD_POP_FILTER,IDS_HELP_POP_FILTER,LTFALSE,pFont,&m_nPopFilter);
	m_pPopFilter->AddString(anPopFilterID[1]);
	m_pPopFilter->AddString(anPopFilterID[2]);
	m_pPopFilter->AddString(anPopFilterID[3]);
    AddFixedControl(m_pPopFilter,pos,LTTRUE);
	pos.y += pFont->GetHeight();

	m_pJoin = CreateTextItem(IDS_JOIN_GAME,CMD_JOIN,IDS_HELP_JOIN_GAME,LTFALSE,pFont);
    m_pJoin->Enable(LTFALSE);
    AddFixedControl(m_pJoin,pos,LTFALSE);
	pos.y += pFont->GetHeight();


	//Add Option Header
	pFont = GetMediumFont();
    pos = LTIntPt(rcOptionRect.left + nIndent,rcOptionRect.top);

    pCtrl = CreateStaticTextItem(IDS_OPTION_NAME,LTNULL,LTNULL,nNameWidth,pFont->GetHeight(),LTTRUE,pFont);
    pCtrl->Enable(LTFALSE);
    AddFixedControl(pCtrl,pos,LTFALSE);
	pos.x += nOptionWidth+nGap;

    pCtrl = CreateStaticTextItem(IDS_OPTION_VALUE,LTNULL,LTNULL,nScoreWidth,pFont->GetHeight(),LTTRUE,pFont);
    pCtrl->Enable(LTFALSE);
    AddFixedControl(pCtrl,pos,LTFALSE);

	//Add OptionList Here
	pos.x = rcOptionRect.left + nIndent;
	pos.y = rcOptionRect.top + nBarHeight;
	nListHeight = (rcOptionRect.bottom - rcOptionRect.top) - nBarHeight;
	nOptionGroupWidth = (rcOptionRect.right - rcOptionRect.left) - nArrowWidth;
	m_pOptionList = debug_new(CListCtrl);
    m_pOptionList->Create(nListHeight, LTTRUE, nOptionGroupWidth);
	m_pOptionList->SetItemSpacing(0);
//    m_pOptionList->Enable(LTFALSE);
    AddFixedControl(m_pOptionList,pos,LTFALSE);

	rcStatusRect.top = rcStatusRect.bottom - nBarHeight;

    bFrameDelay = LTTRUE;

	// Make sure to call the base class
	return CBaseFolder::Build();
}


LTBOOL CFolderJoin::Render(HSURFACE hDestSurf)
{
	// There's no update pump, so we'll have to do it here...

	if (GetGameSpyClientMgr()->IsInitialized())
	{
		Update(hDestSurf);
	}
	else
	{
		UpdateDummyStatus(hDestSurf);
	}

	int xo = g_pInterfaceResMgr->GetXOffset();
	int yo = g_pInterfaceResMgr->GetYOffset();

	//Draw server bar
	DrawBar(hDestSurf,&rcServerRect);

	if (m_pServerList->GetLastDisplayedIndex() >= 0)
	{
		if (m_pServerList->GetLastDisplayedIndex() != nOldIndex)
		{
			if (m_hServersShown)
			{
				g_pLTClient->FreeString(m_hServersShown);
			}
	//(servers %d-%d)
			m_hServersShown = g_pLTClient->FormatString(IDS_SERVERS_SHOWN,m_pServerList->GetStartIndex()+1,m_pServerList->GetLastDisplayedIndex()+1);
			nOldIndex = m_pServerList->GetLastDisplayedIndex();
		}
		
		GetSmallFont()->Draw(m_hServersShown, hDestSurf, xo+rcServerRect.right-nIndent, yo+rcServerRect.top+nIndent, LTF_JUSTIFY_RIGHT, m_hNonSelectedColor);
	}

    CLTGUICtrl *pSortCtrl = LTNULL;
	switch (m_nServerSort)
	{
	case FSS_SORT_NAME:
		pSortCtrl = pServerNameCtrl;
		break;
	case FSS_SORT_PLAYERS:
		pSortCtrl = pServerPlayersCtrl;
		break;
	case FSS_SORT_PING:
		pSortCtrl = pServerPingCtrl;
		break;
	case FSS_SORT_GAME:
		pSortCtrl = pServerGameCtrl;
		break;
	case FSS_SORT_MAP:
		pSortCtrl = pServerMapCtrl;
		break;
	}
	if (pSortCtrl)
	{
        LTIntPt pos = pSortCtrl->GetPos();
		pos.x += xo;
		pos.y += yo;
        LTRect rect(pos.x, (pos.y + pSortCtrl->GetHeight()) - 5, pos.x + pSortCtrl->GetWidth(), (pos.y + pSortCtrl->GetHeight()) - 3);
        g_pLTClient->FillRect(hDestSurf,&rect,m_hSelectedColor);

	}

	//Draw player bar
	DrawBar(hDestSurf,&rcPlayerRect);

    pSortCtrl = LTNULL;
	switch (m_nPlayerSort)
	{
	case FSS_SORT_NAME:
		pSortCtrl = pPlayerNameCtrl;
		break;
	case FSS_SORT_FRAGS:
		pSortCtrl = pPlayerFragCtrl;
		break;
	case FSS_SORT_PING:
		pSortCtrl = pPlayerPingCtrl;
		break;
	}
	if (pSortCtrl)
	{
        LTIntPt pos = pSortCtrl->GetPos();
		pos.x += xo;
		pos.y += yo;
        LTRect rect(pos.x, (pos.y + pSortCtrl->GetHeight()) - 5, pos.x + pSortCtrl->GetWidth(), (pos.y + pSortCtrl->GetHeight()) - 3);
        g_pLTClient->FillRect(hDestSurf,&rect,m_hSelectedColor);

	}

	//Draw option bar
	DrawBar(hDestSurf,&rcOptionRect);

	//Draw option bar
	DrawBar(hDestSurf,&rcStatusRect);

	if (m_hStatus)
	{

		if (m_nState == FSS_IDLE)
			GetSmallFont()->Draw(m_hStatus, hDestSurf, xo+nIndent, yo+rcStatusRect.top+nIndent, LTF_JUSTIFY_LEFT, m_hNonSelectedColor);
		else
			GetSmallFont()->Draw(m_hStatus, hDestSurf, xo+nIndent, yo+rcStatusRect.top+nIndent, LTF_JUSTIFY_LEFT, m_hSelectedColor);
	}

    LTBOOL bOK = CBaseFolder::Render(hDestSurf);

	if (GetGameSpyClientMgr()->IsInitialized())
	{
		// Let the GameSpy client manager do another update...
//		GetGameSpyClientMgr()->Update();
	}
	else
	{
		if (bFrameDelay)
		{
            bFrameDelay = LTFALSE;
		}
		else
		{
			// Init the GameSpy client manager...
			BOOL bRet = GetGameSpyClientMgr()->Init(GAMESPY_GAME_NAME, GAMESPY_SECRET_KEY);
			if (!bRet)
			{
                return LTFALSE;
			}
		}
	}

	return bOK;
}


void CFolderJoin::Update(HSURFACE hDestSurf)
{

	// Let the GameSpy client manager update...

	GetGameSpyClientMgr()->Update();


	// Update based on our current state...

	switch (m_nState)
	{
		case FSS_IDLE:
		{
			UpdateIdle(hDestSurf);
			break;
		}

		case FSS_GETSERVICES:
		{
			UpdateGetServices(hDestSurf);
			break;
		}

		case FSS_GETPINGS:
		{
			UpdateGetPings(hDestSurf);
			break;
		}

		case FSS_GETALLDATA:
		{
			UpdateGetAllData(hDestSurf);
			break;
		}

		case FSS_GETSELPING:
		{
			UpdateGetSelPing(hDestSurf);
			break;
		}

		case FSS_DUMMYSTATUS:
		{
			UpdateDummyStatus(hDestSurf);
			break;
		}
	}


	// Update our sub menus...

	UpdateLists();


	// Check for required sorting...

	if (m_bNeedServerSorting)
	{
        m_bNeedServerSorting = LTFALSE;
		SortServers();
	}


	// Enable or disable the join control as necessary...

    m_pJoin->Enable((GetCurGameServerHandle() != LTNULL));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateDummyStatus
//
//	PURPOSE:	Updates the FSS_DUMMYSTATUS state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateDummyStatus(HSURFACE hDestSurf)
{
	// Set our dummy status text...

	SetStatusText(m_hDummyStatus);


	// Check for ending...

	if (GetTickCount() >= m_timeDummyEnd)
	{
		SetState(m_nNextDummyState);
		return;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateGetServices
//
//	PURPOSE:	Updates the FSS_GETSERVICES state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateGetServices(HSURFACE hDestSurf)
{
	int nState = GetGameSpyClientMgr()->GetState();


	switch (nState)
	{
		case sl_idle:		// done updating
		{
			SortServers();
			m_bNeedServerSorting = FALSE;
			HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_QUERYDONE);
			SetDummyStatusState(hStr, 1000, FSS_IDLE);
			g_pLTClient->FreeString(hStr);
			break;
		}

		case sl_listxfer:	// getting list
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_GETLIST);
			SetStatusText(hStr);
			g_pLTClient->FreeString(hStr);
			break;
		}

		case sl_lanlist:	// searching lan
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_GETLIST);
			SetStatusText(hStr);
			g_pLTClient->FreeString(hStr);
			break;
		}

		case sl_querying:	// querying servers
		{
			HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_QUERY, GetGameSpyClientMgr()->GetProgress(), m_nNumServersListed);
			SetStatusText(hStr);
			g_pLTClient->FreeString(hStr);
			break;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateGetPings
//
//	PURPOSE:	Updates the FSS_GETPINGS state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateGetPings(HSURFACE hDestSurf)
{
	HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_PINGDONE, m_nNumServersListed);
	SetDummyStatusState(hStr, 1000, FSS_IDLE);
	g_pLTClient->FreeString(hStr);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateGetAllData
//
//	PURPOSE:	Updates the FSS_GETALLDATA state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateGetAllData(HSURFACE hDestSurf)
{
	// Update the ping stuff...

	SortPlayers();
	HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_GETEXTRA, m_nNumServers, m_nNumServersListed);
	SetDummyStatusState(hStr, 1000);
	g_pLTClient->FreeString(hStr);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateIdle
//
//	PURPOSE:	Updates the FSS_IDLE state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateIdle(HSURFACE hDestSurf)
{
	// If we're still getting the server list, go back to the proper state...

	if (GetGameSpyClientMgr()->GetState() != sl_idle)
	{
		UpdateGetServices(hDestSurf);
		return;
	}


	// Draw the status info...

	HSTRING hStr= LTNULL;

	if (m_nNumServers == 0)
	{
		hStr = g_pLTClient->FormatString(IDS_STATUS_IDLENONE);
	}
	else
	{
		hStr = g_pLTClient->FormatString(IDS_STATUS_IDLE, m_nNumServersListed);
	}

	SetStatusText(hStr);
	g_pLTClient->FreeString(hStr);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateGetSelPing
//
//	PURPOSE:	Updates the FSS_GETSELPING state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateGetSelPing(HSURFACE hDestSurf)
{
	HSTRING hStr= g_pLTClient->FormatString(IDS_STATUS_PINGSEL);
	SetDummyStatusState(hStr, 1000);
	g_pLTClient->FreeString(hStr);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::SetState
//
//	PURPOSE:	Sets the server finding state
//
// ----------------------------------------------------------------------- //

void CFolderJoin::SetState(int nNewState)
{
	// Set the new state accordingly...

	switch (nNewState)
	{
		case FSS_GETSERVICES:
		{
            SetCurGameServerHandle(LTNULL);
			GetGameSpyClientMgr()->RefreshServers();
            m_bNeedServerSorting = LTTRUE;
            UpdateLists(LTTRUE);
			break;
		}

		case FSS_GETPINGS:
		{
            UpdateLists(LTTRUE);
			break;
		}

		case FSS_GETALLDATA:
		{
			break;
		}

		case FSS_IDLE:
		{
			break;
		}

		case FSS_GETSELPING:
		{
			break;
		}

		case FSS_DUMMYSTATUS:
		{
			break;
		}

		default:
		{
			return;
		}
	}

	// Set the new state value...

	m_timeStateStart = GetTickCount();
	m_nState         = nNewState;
}

void CFolderJoin::SetStatusText(HSTRING hStr)
{

	if (m_hStatus)
	{
		g_pLTClient->FreeString(m_hStatus);
	}
	m_hStatus = g_pLTClient->CopyString(hStr);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::SetDummyStatusState
//
//	PURPOSE:	Sets the dummy status state for just displing status
//				text for a short amount of time.  Otherwise, some status
//				messages go by too quickly to read!
//
// ----------------------------------------------------------------------- //

void CFolderJoin::SetDummyStatusState(HSTRING hStr, uint32 dwWaitTime, int nNextState)
{
	// Set dummy status values...

	m_timeDummyEnd    = GetTickCount() + dwWaitTime;
	m_nNextDummyState = nNextState;

	if (m_hDummyStatus)
	{
		g_pLTClient->FreeString(m_hDummyStatus);
	}
	m_hDummyStatus = g_pLTClient->CopyString(hStr);

	// Set the state...

	SetState(FSS_DUMMYSTATUS);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::JoinGame
//
//	PURPOSE:	Joins the given game service
//
// ----------------------------------------------------------------------- //

void CFolderJoin::JoinCurGame(LTBOOL bPasswordSet)
{
	// Get the currently selected game server...

	CGameSpyServer* pGame = GetGameSpyClientMgr()->GetServerFromHandle(m_pCurServerHandle);
    if (!pGame) return;

	if (IsCurrentGame(pGame))
	{
        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (g_pGameClientShell->IsInWorld() && hPlayerObj)
		{
			g_pInterfaceMgr->ChangeState(GS_PLAYING);
		}
		return;
	}

	if (pGame->GetNumPlayers() >= pGame->GetMaxPlayers())
	{
        HSTRING hString = g_pLTClient->FormatString(IDS_SERVERFULL);
		g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
		g_pLTClient->FreeString(hString);
		return;
	}

	char sVer[16];
	sprintf(sVer,"%d.%.3d",GAME_HANDSHAKE_VER_MAJOR,(GAME_HANDSHAKE_VER_MINOR-1));
	char* sGameVer = pGame->GetStringValue("gamever");
	if (sGameVer && stricmp(sVer,sGameVer)!=0)
	{
        HSTRING hString = g_pLTClient->FormatString(IDS_SERVER_WRONGVERSION);
		g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL,g_pInterfaceResMgr->IsEnglish());
		g_pLTClient->FreeString(hString);
		return;
	}


	// Check for password protection...

	char* sPassword = pGame->GetStringValue("Password");
	if (sPassword && sPassword[0] != '\0')
	{
		if (bPasswordSet)
		{
			if (stricmp(m_szPassword, sPassword) != 0)
			{
                HSTRING hString = g_pLTClient->FormatString(IDS_ACCESSDENIED);
			    g_pInterfaceMgr->ShowMessageBox(hString,LTMB_OK,LTNULL,LTNULL);
				g_pLTClient->FreeString(hString);
				return;
			}
		}
		else
		{
			AskForPassword();
			return;
		}
	}

	if (!DoJoinGame(pGame))
	{
		g_pInterfaceMgr->LoadFailed();
		g_pInterfaceMgr->ConnectionFailed(nConnectError);
	}
}


LTBOOL CFolderJoin::DoJoinGame(CGameSpyServer* pGame)
{
	// Sanity checks...

	if (!pGame) return(LTFALSE);

	if (g_pGameClientShell->IsInWorld() && g_pGameClientShell->GetGameType() != SINGLE)
		g_pInterfaceMgr->StartingNewGame();


	// Get the ip address...

	char sIp[MAX_SGR_STRINGLEN] = { "" };

	char* sTemp = pGame->GetAddress();
	int port = pGame->GetIntValue("hostport",0);
	if (!sTemp) return(LTFALSE);
	sprintf(sIp, "%s:%d", sTemp, port);


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
	


	// Try to join a game...

	int nType = DEATHMATCH;
	if (stricmp(pGame->GetGameType(), "H.A.R.M. vs. UNITY") == 0) nType = COOPERATIVE_ASSAULT;


	g_pGameClientShell->SetGameType((GameType)nType);
	g_pInterfaceMgr->DrawFragCount(LTFALSE);
	
	char szWorld[128];
	sprintf(szWorld,"%s",pGame->GetMap());
	HSTRING hWorld = g_pLTClient->CreateString(pGame->GetMap());
	g_pInterfaceMgr->SetLoadLevelString(hWorld);
	g_pLTClient->FreeString(hWorld);
	g_pInterfaceMgr->ChangeState(GS_LOADINGLEVEL);


	char szPhoto[512] = "interface\\photos\\missions\\default.pcx";
	if (nType == COOPERATIVE_ASSAULT)
		sprintf(szPhoto,"Worlds\\Multi\\AssaultMap\\%s.pcx",szWorld);
	else
		sprintf(szPhoto,"Worlds\\Multi\\DeathMatch\\%s.pcx",szWorld);

	g_pInterfaceMgr->SetLoadLevelPhoto(szPhoto);

    LTRESULT dr = g_pLTClient->InitNetworking(NULL, 0);
	if (dr != LT_OK)
	{
//		s_nErrorString = IDS_NETERR_INIT;
//		NetStart_DisplayError(hInst);
		g_pLTClient->CPrint("InitNetworking() : error %d", dr);
        return(LTFALSE);
	}

	int nRetries = GetConsoleInt("NetJoinRetry", 0);
	while (nRetries >= 0)
	{
		// If successful, then we're done.
		nConnectError = g_pLTClient->StartGame( &req );
        if(nConnectError == LT_OK )
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


uint32 CFolderJoin::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_SELECT_SERVER:
		{
			int nIndex = m_pServerList->GetSelectedItem();
            void* pGame = LTNULL;
			if (nIndex != CListCtrl::kNoSelection)
			{
				pGame = (void*)m_pServerList->GetControl(nIndex)->GetParam1();
			}

			if (pGame != GetCurGameServerHandle())
			{
				SetCurGameServerHandle(pGame);
				SetState(FSS_GETALLDATA);

			}
			UpdatePlayers(TRUE);


		} break;
	case CMD_REFRESH_SERVERS:
		{
			SetState(FSS_GETSERVICES);
			break;
		}

	case CMD_REPING_SERVERS:
		{
			//SetState(FSS_GETPINGS);
			break;
		}

	case CMD_VERSION_FILTER:
		{
			m_nVersionFilter = !m_nVersionFilter;
            m_pVersionFilter->UpdateData(LTFALSE);
			ForceNextUpdate();

		} break;

	case CMD_GAME_FILTER:
		{
			m_nGameFilter++;
			if (m_nGameFilter > NGT_FILTER_LAST)
				m_nGameFilter = 0;
            m_pGameFilter->UpdateData(LTFALSE);
			ForceNextUpdate();

		} break;
	case CMD_POP_FILTER:
		{
			m_nPopFilter++;
			if (m_nPopFilter > POP_FILTER_LAST)
				m_nPopFilter = 0;
            m_pPopFilter->UpdateData(LTFALSE);
			ForceNextUpdate();

		} break;
	case CMD_SORT_SERV_NAME:
		{
			SetServerSort(FSS_SORT_NAME);
			SortServers();
		} break;

	case CMD_SORT_SERV_PLAYERS:
		{
			SetServerSort(FSS_SORT_PLAYERS);
			SortServers();
		} break;
	case CMD_SORT_SERV_PING:
		{
			SetServerSort(FSS_SORT_PING);
			SortServers();
		} break;
	case CMD_SORT_SERV_GAME:
		{
			SetServerSort(FSS_SORT_GAME);
			SortServers();
		} break;
	case CMD_SORT_SERV_MAP:
		{
			SetServerSort(FSS_SORT_MAP);
			SortServers();
		} break;
	case CMD_SORT_PLYR_NAME:
		{
			SetPlayerSort(FSS_SORT_NAME);
			SortPlayers();
		} break;
	case CMD_SORT_PLYR_SCORE:
		{
			SetPlayerSort(FSS_SORT_FRAGS);
			SortPlayers();
		} break;
	case CMD_SORT_PLYR_PING:
		{
			SetPlayerSort(FSS_SORT_PING);
			SortPlayers();
		} break;
	case CMD_EDIT_PASS:
		{
			SetCapture(LTNULL);
			m_pPassEdit->UpdateData();
			RemoveFixedControl(m_pPassEdit);
			RemoveFixedControl(m_pPassLabel);
			RemoveFixedControl(m_pPassBack);
			m_bAskingForPassword = LTFALSE;
			ForceMouseUpdate();
			JoinCurGame(LTTRUE);
		} break;
	case CMD_JOIN:
		{
			CGameSpyServer* pGame = GetGameSpyClientMgr()->GetServerFromHandle(m_pCurServerHandle);
		    if (g_pGameClientShell->IsInWorld() && pGame)
		    {

				if (IsCurrentGame(pGame))
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
					g_pInterfaceMgr->ShowMessageBox(hString,LTMB_YESNO,JoinCallBack,this);
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
void    CFolderJoin::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// Reset other values...
		bDblClick = LTFALSE;

		m_nNumServers       = 0;
		m_nNumServersListed = 0;
		m_nVersionFilter = (int)g_vtNetVersionFilter.GetFloat();
		m_nGameFilter = (int)g_vtNetGameFilter.GetFloat();
		m_nPopFilter = (int)g_vtNetPopFilter.GetFloat();
		m_nServerSort = (int)g_vtNetServerSortKey.GetFloat();
		m_nPlayerSort = (int)g_vtNetPlayerSortKey.GetFloat();

        bFrameDelay = LTTRUE;
		m_szPassword[0] = LTNULL;

		nOldIndex = -1;

		HSTRING hStr = g_pLTClient->FormatString(IDS_STATUS_CONNECTING);
		SetDummyStatusState(hStr, 500, FSS_GETSERVICES);
		g_pLTClient->FreeString(hStr);
		

        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		g_vtNetVersionFilter.WriteFloat((float)m_nVersionFilter);
		g_vtNetGameFilter.WriteFloat((float)m_nGameFilter);
		g_vtNetPopFilter.WriteFloat((float)m_nPopFilter);
		g_vtNetServerSortKey.WriteFloat((float)m_nServerSort);
		g_vtNetPlayerSortKey.WriteFloat((float)m_nPlayerSort);

		SetState(FSS_IDLE);
		GetGameSpyClientMgr()->Term();
        bFrameDelay = LTTRUE;
		m_pServerList->RemoveAllControls();
		m_pPlayerList->RemoveAllControls();
		m_pOptionList->RemoveAllControls();
        SetCurGameServerHandle(LTNULL);

	}
	CBaseFolder::OnFocus(bFocus);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::SortServers
//
//	PURPOSE:	Sorts the servers
//
// ----------------------------------------------------------------------- //

void CFolderJoin::SortServers()
{
	// Set our sort dir based on the sort type...

	switch (m_nServerSort)
	{
		case FSS_SORT_NAME:
		{
			GetGameSpyClientMgr()->SortServersByName();
			break;
		}

		case FSS_SORT_PING:
		{
			GetGameSpyClientMgr()->SortServersByPing();
			break;
		}

		case FSS_SORT_PLAYERS:
		{
			GetGameSpyClientMgr()->SortServersByPlayers();
			break;
		}

		case FSS_SORT_GAME:
		{
			GetGameSpyClientMgr()->SortServersByGameType();
			break;
		}

		case FSS_SORT_MAP:
		{
			GetGameSpyClientMgr()->SortServersByMap();
			break;
		}
	}


	// Force the next update...

	ForceNextUpdate();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::SetCurGameServerHandle
//
//	PURPOSE:	Sets the current game server handle
//
// ----------------------------------------------------------------------- //

void CFolderJoin::SetCurGameServerHandle(void* pHandle)
{
	// Set the current game server handle...

	m_pCurServerHandle = pHandle;
}


int CFolderJoin::AddServerCtrl(CGameSpyServer* pGame)
{
	CLTGUIFont *pFont = GetSmallFont();
    CGroupCtrl *pGroup = CreateGroup(nServerGroupWidth,pFont->GetHeight(),LTNULL);
    LTIntPt pos(0,0);
    pGroup->SetParam1((uint32)pGame->GetHandle());

	char sTemp[128] = "";
	sprintf(sTemp,"%s",pGame->GetName());
    CStaticTextCtrl *pCtrl = CreateStaticTextItem(sTemp,CMD_SELECT_SERVER,LTNULL,nGameWidth,pFont->GetHeight(),LTFALSE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);
	pos.x += nGameWidth+nGap;

	sprintf(sTemp,"%d/%d",pGame->GetNumPlayers(), pGame->GetMaxPlayers());
    pCtrl = CreateStaticTextItem(sTemp,LTNULL,LTNULL,nPlayerWidth,pFont->GetHeight(),LTFALSE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);
	pos.x += nPlayerWidth+nGap;

	if (pGame->GetPing() >= 9999) strcpy(sTemp, "???");
	else sprintf(sTemp,"%d",pGame->GetPing());
    pCtrl = CreateStaticTextItem(sTemp,LTNULL,LTNULL,nPingWidth,pFont->GetHeight(),LTFALSE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);
	pos.x += nPingWidth+nGap;

	sprintf(sTemp,"%s",pGame->GetGameType());
    pCtrl = CreateStaticTextItem(sTemp,LTNULL,LTNULL,nTypeWidth,pFont->GetHeight(),LTFALSE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);
	pos.x += nTypeWidth+nGap;

	sprintf(sTemp,"%s",pGame->GetMap());
    pCtrl = CreateStaticTextItem(sTemp,LTNULL,LTNULL,nMapWidth,pFont->GetHeight(),LTFALSE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);

	return m_pServerList->AddControl(pGroup);
}


int CFolderJoin::AddPlayerCtrl(CGameSpyPlayer* pPlr)
{
	CLTGUIFont *pFont = GetSmallFont();
    CGroupCtrl *pGroup = CreateGroup(nPlayerGroupWidth,pFont->GetHeight(),LTNULL);
    LTIntPt pos(0,0);
    pGroup->SetParam1((uint32)pPlr);

	char sTemp[128] = "";
	strcpy(sTemp, pPlr->GetName());
    CStaticTextCtrl *pCtrl = CreateStaticTextItem(sTemp,LTNULL,LTNULL,nNameWidth,pFont->GetHeight(),LTTRUE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);
	pos.x += nNameWidth+nGap;

	sprintf(sTemp, "%i", pPlr->GetFrags());
    pCtrl = CreateStaticTextItem(sTemp,LTNULL,LTNULL,nScoreWidth,pFont->GetHeight(),LTTRUE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);
	pos.x += nScoreWidth+nGap;

	if (pPlr->GetPing() >= 9999) strcpy(sTemp, "???");
	else sprintf(sTemp,"%d",pPlr->GetPing());
    pCtrl = CreateStaticTextItem(sTemp,LTNULL,LTNULL,nPingWidth,pFont->GetHeight(),LTTRUE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);

	pGroup->Enable(LTFALSE);
	return m_pPlayerList->AddControl(pGroup);

}

int CFolderJoin::AddOptionCtrl(int nID, int nStringID)
{
	CLTGUIFont *pFont = GetSmallFont();
    CGroupCtrl *pGroup = CreateGroup(nOptionGroupWidth,pFont->GetHeight(),LTNULL);
    LTIntPt pos(0,0);

    CStaticTextCtrl *pCtrl = CreateStaticTextItem(nID,LTNULL,LTNULL,nOptionWidth,pFont->GetHeight(),LTTRUE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);
	pos.x += nOptionWidth+nGap;

    pCtrl = CreateStaticTextItem(nStringID,LTNULL,LTNULL,(nOptionGroupWidth-pos.x),pFont->GetHeight(),LTTRUE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);

	pGroup->Enable(LTFALSE);
	return m_pOptionList->AddControl(pGroup);
}


int CFolderJoin::AddOptionCtrl(int nID, char *pszValue)
{
	CLTGUIFont *pFont = GetSmallFont();
    CGroupCtrl *pGroup = CreateGroup(nOptionGroupWidth,pFont->GetHeight(),LTNULL);
    LTIntPt pos(0,0);

    CStaticTextCtrl *pCtrl = CreateStaticTextItem(nID,LTNULL,LTNULL,nOptionWidth,pFont->GetHeight(),LTTRUE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);
	pos.x += nOptionWidth+nGap;

    pCtrl = CreateStaticTextItem(pszValue,LTNULL,LTNULL,(nOptionGroupWidth-pos.x),pFont->GetHeight(),LTTRUE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);

	return m_pOptionList->AddControl(pGroup);
}
/*
int CFolderJoin::AddOptionCtrl(char *pszOption, char *pszValue)
{
	CLTGUIFont *pFont = GetSmallFont();
    CGroupCtrl *pGroup = CreateGroup(nOptionGroupWidth,pFont->GetHeight(),LTNULL);
    LTIntPt pos(0,0);

    CStaticTextCtrl *pCtrl = CreateStaticTextItem(pszOption,LTNULL,LTNULL,nOptionWidth,pFont->GetHeight(),LTFALSE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);
	pos.x += nOptionWidth+nGap;

    pCtrl = CreateStaticTextItem(pszValue,LTNULL,LTNULL,(nOptionGroupWidth-pos.x),pFont->GetHeight(),LTFALSE,pFont);
    pGroup->AddControl(pCtrl,pos,LTTRUE);

	return m_pOptionList->AddControl(pGroup);
}

*/

void CFolderJoin::DrawBar(HSURFACE hDestSurf,LTRect *rect)
{
	int xo = g_pInterfaceResMgr->GetXOffset();
	int yo = g_pInterfaceResMgr->GetYOffset();

	//Draw server bar
    LTRect barRect(xo+rect->left, yo+rect->top, xo+rect->right, yo+rect->top+nBarHeight);
    g_pLTClient->FillRect(hDestSurf,&barRect,m_hBarColor);

	barRect.left += 2;
	barRect.right -= 2;
	barRect.top += 2;
	barRect.bottom -= 2;
    g_pLTClient->FillRect(hDestSurf,&barRect,m_hShadeColor);

}
void CFolderJoin::DrawFrame(HSURFACE hDestSurf,LTRect *rect)
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateLists
//
//	PURPOSE:	Updates the lists
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateLists(LTBOOL bForce)
{
	// Check for a forced update...

	if (m_bForceNextUpdate)
	{
        bForce             = LTTRUE;
        m_bForceNextUpdate = LTFALSE;
	}


	// Check if we got a new server...

	static LTBOOL bDelayNewServer = LTFALSE;

	LTBOOL bNewServer = LTFALSE;

	if (GetGameSpyClientMgr()->ExistNewServer())
	{
		bNewServer = LTTRUE;
	}

	if (bDelayNewServer)
	{
		bNewServer = LTTRUE;
	}


	// Make sure we don't do a full update too often...

	static uint32 timeLastFullUpdate = 0;

	if (!bForce && bNewServer)
	{
		if ((GetTickCount() - timeLastFullUpdate) < 1000)
		{
			bDelayNewServer = LTTRUE;
		}
		else
		{
			bForce = LTTRUE;
		}
	}
	else if (bNewServer)
	{
		bForce = LTTRUE;
	}

	if (bForce)
	{
		timeLastFullUpdate = GetTickCount();
		bDelayNewServer    = LTFALSE;
	}


	// Do the updates...

	UpdateServers(bForce);
	UpdatePlayers(bForce);
	UpdateOptions(bForce);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateServers
//
//	PURPOSE:	Updates the servers
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateServers(LTBOOL bForce)
{
	// Determine if our selection has changed...

	int nOldSel   = m_pServerList->GetSelectedItem();
	int nOldStart = m_pServerList->GetStartIndex();


	// Determine if any data changed...

	if (!bForce)
	{
		return;
	}


	// Get our filters...

	int nGameFilter = GetGameFilter();
	int nPopFilter  = GetPopFilter();

	char sVer[16];
	sprintf(sVer,"%d.%.3d",GAME_HANDSHAKE_VER_MAJOR,(GAME_HANDSHAKE_VER_MINOR-1));


	// Get the previous selected item...

	void*  pCurHandle = GetCurGameServerHandle();
    LTBOOL bSelSet    = LTFALSE;


	// Rebuild the entire list...

	m_pServerList->RemoveAllControls();

    CGameSpyServer* pGame = GetGameSpyClientMgr()->GetFirstServer();
	int nIndex = -1;

	while (pGame && pGame->IsInitialized())
	{
		// Check our filters...

        LTBOOL bPass = LTTRUE;

		if (0 == pGame->GetMaxPlayers())
			bPass = LTFALSE;

		if (m_nVersionFilter)
		{
			char* sGameVer = pGame->GetStringValue("gamever");
			if (sGameVer && stricmp(sVer,sGameVer)!=0)
				bPass = LTFALSE;
		}

		if (nGameFilter != NGT_FILTER_ALL)
		{
			char* sType = pGame->GetGameType();
			int   nType = -1;

			if (stricmp(sType, "deathmatch") == 0) nType = DEATHMATCH;
			if (stricmp(sType, "H.A.R.M. vs. UNITY") == 0) nType = COOPERATIVE_ASSAULT;
            if (nType != nGameFilter) bPass = LTFALSE;
		}

		if (nPopFilter != POP_FILTER_ALL)
		{
			int nPlrs = pGame->GetNumPlayers();
			int nMax  = pGame->GetMaxPlayers();

			switch (nPopFilter)
			{
                case POP_FILTER_NOTEMPTY: if (nPlrs == 0) bPass = LTFALSE; break;
                case POP_FILTER_NOTFULL:  if (nPlrs >= nMax) bPass = LTFALSE; break;
                case POP_FILTER_NOTBOTH:  if (nPlrs == 0 || nPlrs >= nMax) bPass = LTFALSE; break;
			}
		}


		if (!bPass)
		{
            pGame = GetGameSpyClientMgr()->GetNextServer();
			continue;
		}


		// Add a new control for this game service...

		nIndex = AddServerCtrl(pGame);


		// Update the current selection...

		if (pGame->GetHandle() == pCurHandle)
		{
			m_pServerList->SelectItem(nIndex);
            bSelSet = LTTRUE;
		}

		// Get the next game service...

        pGame = GetGameSpyClientMgr()->GetNextServer();
	}


	// Update our current game service if necessary...

	if (bSelSet)
	{
		int nOldOffset = nOldSel - nOldStart;
		int nNewSel = m_pServerList->GetSelectedItem();
		int nNewStart = m_pServerList->GetStartIndex();

		int nStart = m_pServerList->GetSelectedItem() - nOldOffset;
		if (nStart >= 0)
			m_pServerList->SetStartIndex(nStart);

		void* pHandle = (void*)m_pServerList->GetControl(nNewSel)->GetParam1();
		SetCurGameServerHandle(pHandle);
	}
	else
	{
		m_pServerList->SelectItem(CListCtrl::kNoSelection);
        SetCurGameServerHandle(LTNULL);
	}


	// Update our server counts...

    uint32 dwNumServers = GetGameSpyClientMgr()->GetNumServers();
	SetServerCounts(dwNumServers, nIndex+1);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdatePlayers
//
//	PURPOSE:	Updates the sub menu
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdatePlayers(LTBOOL bForce)
{
	// Determine if any data changed...

	if (!bForce)
	{
		return;
	}


	// Remove all options...

	m_pPlayerList->RemoveAllControls();


	// Get the current game service...

	void* pHandle = GetCurGameServerHandle();
	if (!pHandle) return;

	CGameSpyServer* pGame = GetGameSpyClientMgr()->GetServerFromHandle(pHandle);
	if (!pGame) return;
	if (!pGame->IsInitialized()) return;

	pGame->UpdatePlayers();


	// Set our sort dir based on the sort type...

	switch (m_nPlayerSort)
	{
		case FSS_SORT_NAME:
		{
			pGame->SortPlayersByName();
			break;
		}

		case FSS_SORT_PING:
		{
			pGame->SortPlayersByPing();
			break;
		}

		case FSS_SORT_FRAGS:
		{
			pGame->SortPlayersByFrags();
			break;
		}

	}


	// Add each player...

	CGameSpyPlayer* pPlr  = pGame->GetFirstPlayer();
	int nIndex = 0;

	while (pPlr)
	{
		// Add a new control for this player...
		nIndex = AddPlayerCtrl(pPlr);

		// Get the next player
		pPlr = pGame->GetNextPlayer();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::UpdateOptions
//
//	PURPOSE:	Updates the sub menu
//
// ----------------------------------------------------------------------- //

void CFolderJoin::UpdateOptions(LTBOOL bForce)
{
	// Update our sort light...

	// Determine if any data changed...

	if (!bForce)
	{
		return;
	}


	// Remove all options...

	m_pOptionList->RemoveAllControls();


	// Get the current game service...

	void* pHandle = GetCurGameServerHandle();
	if (!pHandle) return;

	CGameSpyServer* pGame = GetGameSpyClientMgr()->GetServerFromHandle(pHandle);
	if (!pGame) return;
	if (!pGame->IsInitialized()) return;

	GameType eGameType = SINGLE;
	if (stricmp(pGame->GetGameType(), "H.A.R.M. vs. UNITY") == 0)
	{
		eGameType = COOPERATIVE_ASSAULT;
	}
	else if (stricmp(pGame->GetGameType(), "Deathmatch") == 0)
	{
		eGameType = DEATHMATCH;
	}
	if (eGameType == SINGLE) return;
	
	char* sGameVer = pGame->GetStringValue("gamever");
	if (sGameVer && sGameVer[0] != '\0')
	{
#ifdef _DEMO
		char sVer[16];
		sprintf(sVer,"%d.%.3d",GAME_HANDSHAKE_VER_MAJOR,(GAME_HANDSHAKE_VER_MINOR-1));
    	HSTRING hDemoBuildVersion = g_pLTClient->FormatString(IDS_DEMOVERSION);

		if ((stricmp(sVer,sGameVer) == 0) && hDemoBuildVersion)
		{
			AddOptionCtrl(IDS_VERSION_FILTER, g_pLTClient->GetStringData(hDemoBuildVersion));
		}
		else
		{
			AddOptionCtrl(IDS_VERSION_FILTER, sGameVer);
		}
#else
		AddOptionCtrl(IDS_VERSION_FILTER, sGameVer);
#endif
	}

	char* sAddr = pGame->GetAddress();
	if (sAddr && sAddr[0] != '\0')
	{
		AddOptionCtrl(IDS_SERVER_ADDRESS,sAddr);
	}


	char sTemp[8];
	int port = pGame->GetIntValue("hostport",0);
	sprintf(sTemp, "%d", port);
	AddOptionCtrl(IDS_SERVER_PORT,sTemp);


	// Check for password protection...

	char* sPassword = pGame->GetStringValue("Password");
	if (sPassword && sPassword[0] != '\0')
	{
		AddOptionCtrl(IDS_USE_PASSWORD,IDS_YES);
	}


	int defStrId = IDS_SPACER;
	for (int i = 0; i < g_pServerOptionMgr->GetNumOptions(); i++)
	{
		OPTION* pOpt = g_pServerOptionMgr->GetOption(i);
		if (pOpt->eGameType != SINGLE && pOpt->eGameType != eGameType) continue;
		// Add the options/value info...

		char sTemp[32] ="";
		int nVal = 0;
		int nStringID = 0;

		switch (pOpt->eType)
		{
		case SO_TOGGLE:
		case SO_CYCLE:
			nVal = pGame->GetIntValue(pOpt->szVariable);
			nStringID = pOpt->nStringId[nVal];
			AddOptionCtrl(pOpt->nNameId,nStringID);
			break;
		case SO_SLIDER:
		case SO_SLIDER_NUM:
			if (pOpt->fSliderScale < 1.0f || pOpt->fSliderScale > 1.0f)
				sprintf(sTemp,"%0.2f", pGame->GetFloatValue(pOpt->szVariable) * pOpt->fSliderScale);
			else
				sprintf(sTemp,"%d", pGame->GetIntValue(pOpt->szVariable));
			AddOptionCtrl(pOpt->nNameId,sTemp);
			break;
		case SO_SPECIAL:
			{
				if (stricmp(pOpt->szVariable,"NetDefaultWeapon") == 0)
				{
					nVal = pGame->GetIntValue(pOpt->szVariable);
					if (nVal)
					{
						int nWeaponId = g_pWeaponMgr->GetWeaponId(nVal);
						if (nWeaponId != WMGR_INVALID_ID)
						{
							WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(nWeaponId);
							if (pWeapon)
							{
								nStringID = pWeapon->nNameId;
								AddOptionCtrl(pOpt->nNameId,nStringID);
							}
						}
					}
				}
			}
		}		
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFolderJoin::SortPlayers
//
//	PURPOSE:	Sorts the players
//
// ----------------------------------------------------------------------- //

void CFolderJoin::SortPlayers()
{
	// Sanity checks...

	if (!m_pCurServerHandle) return;


	// Get the server from the handle...

	CGameSpyServer* pGame = GetGameSpyClientMgr()->GetServerFromHandle(GetCurGameServerHandle());
	if (!pGame) return;

	pGame->UpdatePlayers();


	// Set our sort dir based on the sort type...

	switch (m_nPlayerSort)
	{
		case FSS_SORT_NAME:
		{
			pGame->SortPlayersByName();
			break;
		}

		case FSS_SORT_PING:
		{
			pGame->SortPlayersByPing();
			break;
		}

		case FSS_SORT_FRAGS:
		{
			pGame->SortPlayersByFrags();
			break;
		}

	}


	// Force the next update...

	ForceNextUpdate();
}


void JoinCallBack(LTBOOL bReturn, void *pData)
{
	CFolderJoin *pThisFolder = (CFolderJoin *)pData;
	if (bReturn && pThisFolder)
	{
		pThisFolder->JoinCurGame();
    }
}


LTBOOL CFolderJoin::OnUp()
{
	if (! CBaseFolder::OnUp()) return LTFALSE;
	if (GetSelectedControl() == m_pServerList)
	{
		SendCommand(CMD_SELECT_SERVER,0,0);
	}
	return LTTRUE;
}

LTBOOL CFolderJoin::OnDown()
{
	if (! CBaseFolder::OnDown()) return LTFALSE;
	if (GetSelectedControl() == m_pServerList)
	{
		SendCommand(CMD_SELECT_SERVER,0,0);
	}
	return LTTRUE;
}

LTBOOL CFolderJoin::OnEnter()
{
	if (GetSelectedControl() == m_pServerList && m_pServerList->IsItemSelected())
			return m_pJoin->OnEnter();		
	if (CBaseFolder::OnEnter()) return LTTRUE;
	return LTFALSE;
}

void CFolderJoin::Escape()
{
	if (m_pPassEdit && GetCapture() == m_pPassEdit)
	{
        SetCapture(LTNULL);
		m_szPassword[0] = LTNULL;
		RemoveFixedControl(m_pPassEdit);
		RemoveFixedControl(m_pPassLabel);
		RemoveFixedControl(m_pPassBack);
		m_bAskingForPassword = FALSE;
		ForceMouseUpdate();
	}
	else
		CBaseFolder::Escape();

}


void CFolderJoin::AskForPassword()
{
	m_bAskingForPassword = LTTRUE;
	m_szPassword[0] = LTNULL;
	m_pPassLabel = CreateTextItem(IDS_PASSWORD, CMD_EDIT_PASS, IDS_HELP_ENTER_PASSWORD);

	m_pPassEdit = CreateEditCtrl(" ", CMD_EDIT_PASS, IDS_HELP_ENTER_PASSWORD, m_szPassword, sizeof(m_szPassword), 25, LTTRUE);
	m_pPassEdit->EnableCursor();
    m_pPassEdit->Enable(LTFALSE);
	m_pPassEdit->SetAlignment(LTF_JUSTIFY_CENTER);

	char szBack[128] = "";
	g_pLayoutMgr->GetMessageBoxBackground(szBack,sizeof(szBack));

	m_pPassBack = debug_new(CBitmapCtrl);
    m_pPassBack->Create(g_pLTClient,szBack);

	LTIntPt pos(0,0);

	pos.x = 320 - m_pPassBack->GetWidth() / 2;
	pos.y = 240 - m_pPassBack->GetHeight() / 2;
	AddFixedControl(m_pPassBack,pos,LTFALSE);

	pos.x += 16;
	pos.y = 244 + m_pPassEdit->GetHeight();
	AddFixedControl(m_pPassEdit,pos,LTTRUE);

	pos.x = 320 - m_pPassLabel->GetWidth() / 2;
	pos.y = 236 - m_pPassLabel->GetHeight();
	AddFixedControl(m_pPassLabel,pos,LTFALSE);



	SetCapture(m_pPassEdit);
	m_pPassEdit->SetColor(m_hSelectedColor,m_hSelectedColor,m_hSelectedColor);
    m_pPassEdit->Select(LTTRUE);
    m_pPassLabel->Select(LTFALSE);


}


LTBOOL CFolderJoin::IsCurrentGame(CGameSpyServer* pGame)
{
	if (g_pGameClientShell->GetGameType() == SINGLE) return LTFALSE;

	int p = pGame->GetIntValue("hostport",0);
	// Get the ip address...
	char* sTemp = pGame->GetAddress();
	if (!sTemp) return LTFALSE;

	return (g_pGameClientShell->CheckServerAddress(sTemp,p));

}

/******************************************************************/
LTBOOL	CFolderJoin::OnLButtonUp(int x, int y)
{

	if (bDblClick)
	{
		bDblClick = LTFALSE;
		return LTFALSE;
	}

	if (GetCapture())
	{
		return m_pPassEdit->OnEnter();
	}
	return CBaseFolder::OnLButtonUp(x,y);
}
LTBOOL	CFolderJoin::OnRButtonUp(int x, int y)
{
	if (GetCapture())
	{
		Escape();
		return LTTRUE;
	}
	return CBaseFolder::OnRButtonUp(x,y);
}


LTBOOL CFolderJoin::OnLButtonDblClick(int x, int y)
{
	bDblClick = LTTRUE;
	CLTGUICtrl* pCtrl = GetSelectedControl();
	if (pCtrl==m_pServerList && !m_pServerList->OnLButtonDblClick( x, y))
		return OnEnter();
    
	return CBaseFolder::OnLButtonDblClick( x, y);
}


LTBOOL CFolderJoin::HandleKeyDown(int key, int rep)
{
	if (key == VK_F5)
	{
		SetState(FSS_GETSERVICES);
        return LTTRUE;
	}
    return CBaseFolder::HandleKeyDown(key,rep);
}