// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHost.cpp
//
// PURPOSE : Interface screen for hosting multi player games
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHost.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "CommonUtilities.h"
#include "ClientRes.h"
#include "MissionMgr.h"
#include "clientmultiplayermgr.h"
#include "WinUtil.h"
#include "ClientSaveLoadMgr.h"
#include "GameClientShell.h"

static const int kMaxPortStrLen = 5;
static const int kMaxBandwidthStrLen = 8;

extern bool g_bLAN;

namespace
{
	void EditNameCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHost *pThisScreen = (CScreenHost *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_NAME);
	};
	void EditPassCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHost *pThisScreen = (CScreenHost *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PASS);
	};
	void EditScmdPassCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHost *pThisScreen = (CScreenHost *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_SCMDPASS);
	};
	void EditPortCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHost *pThisScreen = (CScreenHost *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_PORT);
	};
	void EditBandwidthCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenHost *pThisScreen = (CScreenHost *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_HOST);
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_BANDWIDTH);
	};

}

void LaunchCallBack(LTBOOL bReturn, void *pData);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHost::CScreenHost()
{
	m_pName			= LTNULL;
	m_pCampaign		= LTNULL;

    m_pLoadCtrl = LTNULL;
	m_pWeapons = LTNULL;

	m_pPassword		= LTNULL;
	m_pPassToggle	= LTNULL;

	m_pPort			= LTNULL;

	m_pScmdPassword	= LTNULL;
	m_pScmdPassToggle	= LTNULL;

	m_pBandwidthCycle	= LTNULL;
	m_pBandwidth		= LTNULL;
	m_nBandwidth		= 0;
	m_sBandwidth = "";

	LoadString( IDS_PASSWORD_DEFAULT, m_sPassword.GetBuffer( MAX_PASSWORD ), MAX_PASSWORD );
	m_sPassword.ReleaseBuffer( );

	LoadString( IDS_PASSWORD_DEFAULT, m_sScmdPassword.GetBuffer( MAX_PASSWORD ), MAX_PASSWORD );
	m_sScmdPassword.ReleaseBuffer( );

	m_sCampaignName = DEFAULT_CAMPAIGN;

	m_sPort.Format( "%d", DEFAULT_PORT );
	m_bUsePassword = LTFALSE;
	m_bAllowScmdCommands = LTFALSE;
	m_bReadyToLaunch = LTFALSE;
	m_bDedicated = LTFALSE;

}


CScreenHost::~CScreenHost()
{
}

// Build the screen
LTBOOL CScreenHost::Build()
{
	int kColumn0 = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST,"ColumnWidth");
	int kColumn1 = (640 - GetPageLeft()) - kColumn0;

	CreateTitle(IDS_TITLE_HOST);


	m_pName = AddColumnCtrl(CMD_EDIT_NAME, IDS_HELP_SESSION_NAME);
	m_pName->AddColumn(LoadTempString(IDS_HOST_NAME), kColumn0);
	m_pName->AddColumn("<host name>", kColumn1, LTTRUE);

	m_pLoadCtrl = AddTextItem(IDS_LOADGAME,	CMD_LOAD_GAME, IDS_HELP_LOAD);

	m_pCampaign = AddColumnCtrl(CMD_CHOOSE_CAMPAIGN, IDS_HELP_CAMPAIGN_NAME);
	m_pCampaign->AddColumn(LoadTempString(IDS_HOST_CAMPAIGN), kColumn0);
	m_pCampaign->AddColumn("<mission name>", kColumn1, LTTRUE);

	m_pPassToggle = AddToggle(IDS_USE_PASSWORD,IDS_HELP_PASSWORD,kColumn0,&m_bUsePassword);
	m_pPassToggle->NotifyOnChange(CMD_TOGGLE_PASS,this);

	m_pPassword = AddColumnCtrl(CMD_EDIT_PASS, IDS_HELP_ENTER_PASSWORD);
	m_pPassword->AddColumn(LoadTempString(IDS_PASSWORD), kColumn0);
	m_pPassword->AddColumn("<password>", kColumn1, LTTRUE);

	m_pScmdPassToggle = AddToggle(IDS_ALLOW_SCMD_COMMANDS,IDS_HELP_ALLOWSCMD,kColumn0,&m_bAllowScmdCommands);
	m_pScmdPassToggle->NotifyOnChange(CMD_TOGGLE_SCMDPASS,this);

	m_pScmdPassword = AddColumnCtrl(CMD_EDIT_SCMDPASS, IDS_HELP_ENTER_SCMDPASSWORD);
	m_pScmdPassword->AddColumn(LoadTempString(IDS_SCMDPASSWORD), kColumn0);
	m_pScmdPassword->AddColumn("<password>", kColumn1, LTTRUE);

	m_pPort = AddColumnCtrl(CMD_EDIT_PORT, IDS_HELP_ENTER_PORT);
	m_pPort->AddColumn(LoadTempString(IDS_PORT), kColumn0);
	m_pPort->AddColumn("<port>", kColumn1, LTTRUE);

	m_pBandwidthCycle = AddCycle(IDS_BANDWIDTH_CYCLE,IDS_HELP_BANDWIDTH_CYCLE,kColumn0,&m_nBandwidth);
	m_pBandwidthCycle->AddString(LoadTempString(IDS_56K));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_DSL_LOW));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_DSL_HIGH));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_CABLE));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_T1));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_T3));
	m_pBandwidthCycle->AddString(LoadTempString(IDS_CUSTOM));

	m_pBandwidth = AddColumnCtrl(CMD_EDIT_BANDWIDTH, IDS_HELP_BANDWIDTH_EDIT);
	m_pBandwidth->AddColumn(LoadTempString(IDS_BANDWIDTH_EDIT), kColumn0);
	m_pBandwidth->AddColumn("<bandwidth>", kColumn1, LTTRUE);


	AddTextItem(IDS_OPTIONS, CMD_OPTIONS, IDS_HELP_HOST_OPTIONS);
	m_pWeapons = AddTextItem(IDS_WPN_RESTRICT, CMD_WEAPONS, IDS_HELP_WPN_RESTRICT);

	CLTGUIToggle *pToggle = AddToggle(IDS_DEDICATED,IDS_HELP_DEDICATED,kColumn0,&m_bDedicated);
	pToggle->SetOnString(LoadTempString(IDS_YES));
	pToggle->SetOffString(LoadTempString(IDS_NO));


	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_HOST,"LaunchPos");
	uint8 nFont = g_pLayoutMgr->GetBackFont();
	uint8 nFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_HOST,"LaunchFontSize");
	CLTGUITextCtrl* pCtrl = AddTextItem(IDS_HOST_LAUNCH, CMD_LAUNCH, IDS_HELP_LAUNCH,pos,LTFALSE,nFont);
	pCtrl->SetFont(LTNULL,nFontSize);


 	// Make sure to call the base class
	return CBaseScreen::Build();
}

uint32 CScreenHost::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand == CMD_OK)
	{
		HandleCallback(dwParam1,dwParam2);
		return 1;
	}
	switch(dwCommand)
	{
	case CMD_EDIT_NAME:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditNameCallBack;
			mb.pString = m_sSessionName;
			mb.nMaxChars = MAX_SESSION_NAME-1;
			g_pInterfaceMgr->ShowMessageBox(IDS_HOST_NAME,&mb);
		} break;
	case CMD_LOAD_GAME:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LOAD);
			break;
		}
	case CMD_EDIT_PASS:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditPassCallBack;
			mb.pString = m_sPassword;
			mb.nMaxChars = MAX_PASSWORD-1;
			g_pInterfaceMgr->ShowMessageBox(IDS_PASSWORD,&mb);
		} break;
	case CMD_EDIT_SCMDPASS:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditScmdPassCallBack;
			mb.pString = m_sScmdPassword;
			mb.nMaxChars = MAX_PASSWORD-1;
			g_pInterfaceMgr->ShowMessageBox(IDS_SCMDPASSWORD,&mb);
		} break;
	case CMD_EDIT_PORT:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditPortCallBack;
			mb.pString = m_sPort;
			mb.nMaxChars = kMaxPortStrLen;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(IDS_PORT,&mb);
		} break;
	case CMD_EDIT_BANDWIDTH:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditBandwidthCallBack;
			mb.pString = m_sBandwidth;
			mb.nMaxChars = kMaxBandwidthStrLen;
			mb.eInput = CLTGUIEditCtrl::kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox(IDS_BANDWIDTH_EDIT,&mb);
		} break;
	case CMD_TOGGLE_PASS:
		{
			m_pPassToggle->UpdateData();
			m_pPassword->Enable(m_bUsePassword);
		} break;
	case CMD_TOGGLE_SCMDPASS:
		{
			m_pScmdPassToggle->UpdateData();
			m_pScmdPassword->Enable(m_bAllowScmdCommands);
		} break;
	case CMD_OPTIONS:
		{
			switch (m_eGameType)
			{
			case eGameTypeCooperative:
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_OPTIONS);
				break;
			case eGameTypeDeathmatch:
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_DM_OPTIONS);
				break;
			case eGameTypeTeamDeathmatch:
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_TDM_OPTIONS);
				break;
			case eGameTypeDoomsDay:
				g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_DD_OPTIONS);
				break;
			};
		} break;
	case CMD_WEAPONS:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_WEAPONS);
		} break;
	case CMD_CHOOSE_CAMPAIGN:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_MISSION);
		} break;
	case CMD_LAUNCH:
		{

		    if (g_pGameClientShell->IsWorldLoaded())
		    {
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = LaunchCallBack;
				mb.pData = this;
			    g_pInterfaceMgr->ShowMessageBox(IDS_ENDCURRENTGAME,&mb);
		    }
		    else
		    {
				HandleLaunch();
		    }

		} break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// Change in focus
void    CScreenHost::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		
	if (bFocus)
	{
		// Turn off stuff the LAN screens doen't care about..
		if (g_bLAN)
		{
			m_pBandwidthCycle->Show(LTFALSE);
			m_pBandwidth->Show(LTFALSE);
		}
		else
		{
			m_pBandwidthCycle->Show(LTTRUE);
			m_pBandwidth->Show(LTTRUE);
		}

		// Make sure we're disconnected from server.
		if(g_pLTClient->IsConnected())
		{
			g_pInterfaceMgr->SetIntentionalDisconnect( true );
			g_pClientMultiplayerMgr->ForceDisconnect();
		}

		UpdateGameType();


		m_sPassword = pProfile->m_ServerGameOptions.m_sPassword.c_str();
		m_sPassword = m_sPassword.Left( MAX_PASSWORD );
		m_sScmdPassword = pProfile->m_ServerGameOptions.m_sScmdPassword.c_str( );
		m_sScmdPassword = m_sScmdPassword.Left( MAX_PASSWORD );
		uint16 nPort = pProfile->m_ServerGameOptions.m_nPort;
		m_sPort.Format( "%d", nPort );
		m_nBandwidth = pProfile->m_ServerGameOptions.m_nBandwidthServer;
		m_bUsePassword = pProfile->m_ServerGameOptions.m_bUsePassword;
		m_bAllowScmdCommands = pProfile->m_ServerGameOptions.m_bAllowScmdCommands;
		m_bDedicated = pProfile->m_ServerGameOptions.m_bDedicated;

		m_pPassword->Enable(m_bUsePassword);

		m_pScmdPassword->Enable(m_bAllowScmdCommands);


		m_pPassword->SetString(1,m_sPassword);
		m_pScmdPassword->SetString(1,m_sScmdPassword);
		m_pPort->SetString(1,m_sPort);

		m_pWeapons->Show(!IsCoopMultiplayerGameType( ));

		if (!g_bLAN)
			UpdateBandwidth();

        UpdateData(LTFALSE);


		// Switch save/load to use mp folders.
		g_pClientSaveLoadMgr->SetUseMultiplayerFolders( true );

		m_pLoadCtrl->Enable( g_pClientSaveLoadMgr->ReloadSaveExists() );
		m_pLoadCtrl->Show(g_pGameClientShell->GetGameType() == eGameTypeCooperative);

		// Setup the host info.
		g_pClientMultiplayerMgr->SetupServerHost( pProfile->m_ServerGameOptions.m_nPort, g_bLAN );
	}
	else
	{
		UpdateData();

		SaveOptions();

		// Setup the host info, so that loaded games have the correct settings
		g_pClientMultiplayerMgr->SetupServerHost( pProfile->m_ServerGameOptions.m_nPort, g_bLAN );

	}
	CBaseScreen::OnFocus(bFocus);
}

LTBOOL CScreenHost::OnLeft()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		
		if (!m_nBandwidth)
			m_nBandwidth = eBandwidth_Custom-1;
		else
			--m_nBandwidth;
			
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}
	return CBaseScreen::OnLeft();
}

LTBOOL CScreenHost::OnRight()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth >= eBandwidth_Custom)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}
	return CBaseScreen::OnRight();
}

/******************************************************************/
LTBOOL CScreenHost::OnLButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth >= eBandwidth_Custom)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}
	return CBaseScreen::OnLButtonUp(x,y);
}


/******************************************************************/
LTBOOL CScreenHost::OnRButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		if (!m_nBandwidth)
			m_nBandwidth = eBandwidth_Custom-1;
		else
			--m_nBandwidth;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth();
        return LTTRUE;
	}
	return CBaseScreen::OnRButtonUp(x,y);
}

void CScreenHost::HandleLaunch()
{
	m_bReadyToLaunch = LTFALSE;
	UpdateData();

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	SaveOptions();
	
	// Setup the host info.
	if( !g_pClientMultiplayerMgr->SetupServerHost( pProfile->m_ServerGameOptions.m_nPort, g_bLAN ))
	{
		// This should probably prevent them from starting the server if this function fails,
		// But it never does...
	}
	
	if (!LaunchGame())
	{
		g_pInterfaceMgr->LoadFailed(SCREEN_ID_HOST);
	}
}



LTBOOL CScreenHost::LaunchGame()
{
	if( !g_pMissionMgr->StartGameNew( ))
		return LTFALSE;

	return LTTRUE;
}


void LaunchCallBack(LTBOOL bReturn, void *pData)
{
	CScreenHost *pThisScreen = (CScreenHost *)pData;
	if (bReturn && pThisScreen)
	{
		pThisScreen->ReadyLaunch(LTTRUE);
    }
}


bool CScreenHost::UpdateInterfaceSFX()
{
	if (m_bReadyToLaunch)
		HandleLaunch();

	CBaseScreen::UpdateInterfaceSFX();

	return true;
}


void CScreenHost::ReadyLaunch(LTBOOL bReady)
{
	m_bReadyToLaunch = bReady;
}

void CScreenHost::UpdateBandwidth()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	if ( m_nBandwidth >= eBandwidth_Custom )
	{
		m_sBandwidth.Format( "%d", pProfile->m_ServerGameOptions.m_nBandwidthServerCustom);
		m_pBandwidth->SetString(1,m_sBandwidth);
	}
	else
	{

		m_sBandwidth.Format( "%d", g_BandwidthServer[m_nBandwidth]);
		m_pBandwidth->SetString(1,m_sBandwidth);
	}
}


void CScreenHost::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch(dwParam2)
	{
	case CMD_EDIT_NAME:
		{
			const char* pszNewVal = (const char*)dwParam1;

			if( pszNewVal[0] )
			{
				m_sSessionName = pszNewVal;
			}
			else
			{
				GetDefaultSessionName( m_eGameType, m_sSessionName.GetBuffer( MAX_SESSION_NAME ), MAX_SESSION_NAME );
				m_sSessionName.ReleaseBuffer( );
			}

			m_sSessionName = m_sSessionName.Left( MAX_SESSION_NAME );
			m_pName->SetString(1,m_sSessionName);
		}
		break;
	case CMD_EDIT_PASS:
		m_sPassword = ((char *)dwParam1);
		m_sPassword = m_sPassword.Left( MAX_PASSWORD );
		m_pPassword->SetString(1,m_sPassword);
		break;
	case CMD_EDIT_SCMDPASS:
		m_sScmdPassword = ((char *)dwParam1);
		m_sScmdPassword = m_sScmdPassword.Left( MAX_PASSWORD );
		m_pScmdPassword->SetString(1,m_sScmdPassword);
		break;
	case CMD_EDIT_PORT:
		{
			char *pszPort = (char *)dwParam1;
			uint16 nPort = (uint16)atoi(pszPort);
			m_sPort = pszPort;
			m_sPort = m_sPort.Left( kMaxPortStrLen );
			m_pPort->SetString(1,m_sPort);

			// Setup the host info.
			g_pClientMultiplayerMgr->SetupServerHost( nPort, g_bLAN );
		}
		break;
	case CMD_EDIT_BANDWIDTH:
		{
			char *pszBandwidth = (char *)dwParam1;
			uint32 nBandwidth = (uint32)atoi(pszBandwidth);
			if ( IsValidBandwidth(nBandwidth) )
			{
				m_sBandwidth = pszBandwidth;
				m_sBandwidth = m_sBandwidth.Left( kMaxBandwidthStrLen );
				m_nBandwidth = eBandwidth_Custom;
				CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
				pProfile->m_ServerGameOptions.m_nBandwidthServerCustom = (uint16)atol(m_sBandwidth);
				m_pBandwidthCycle->UpdateData(LTFALSE);
				UpdateBandwidth();
			}
			else
			{
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox(FormatTempString(IDS_BANDWIDTH_INVALID,GetMinimumBandwidth(),GetMaximumBandwidth()),&mb);
			}
		}
		break;
	}
	UpdateData();
}


void CScreenHost::Escape()
{
	// Switch save/load to back to use sp folders.
	g_pClientSaveLoadMgr->SetUseMultiplayerFolders( false );

	// Initialize to the sp mission bute.
	if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
	{
		g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
		return;
  	}

	g_pClientMultiplayerMgr->SetupServerSinglePlayer( );

	CBaseScreen::Escape();
}



void CScreenHost::UpdateGameType()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	m_eGameType = g_pGameClientShell->GetGameType();
	pProfile->m_ServerGameOptions.m_eGameType = m_eGameType;

	
	m_sSessionName = pProfile->m_ServerGameOptions.GetSessionName();
	m_sCampaignName = pProfile->m_ServerGameOptions.GetCampaignName();

	// If the session name isn't specified, then get the default.
	if( m_sSessionName.GetLength() == 0 )
	{
		GetDefaultSessionName( m_eGameType, m_sSessionName.GetBuffer( MAX_SESSION_NAME ), MAX_SESSION_NAME );
		m_sSessionName.ReleaseBuffer( );
	}
	m_pName->SetString(1,m_sSessionName);


	CreateDefaultCampaign();
	bool isDefault = (m_sCampaignName.CompareNoCase(DEFAULT_CAMPAIGN) == 0);
	
	if (!isDefault)
	{
		char const* pszCampaignFile = GetCampaignFile( pProfile->m_ServerGameOptions );

		if (!CWinUtil::FileExist( pszCampaignFile ))
		{
			switch (m_eGameType)
			{
			case eGameTypeCooperative:
				pProfile->m_ServerGameOptions.GetCoop().m_sCampaignName = DEFAULT_CAMPAIGN;
				break;
			case eGameTypeDeathmatch:
				pProfile->m_ServerGameOptions.GetDeathmatch().m_sCampaignName = DEFAULT_CAMPAIGN;
				break;
			case eGameTypeTeamDeathmatch:
				pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_sCampaignName = DEFAULT_CAMPAIGN;
				break;
			case eGameTypeDoomsDay:
				pProfile->m_ServerGameOptions.GetDoomsday().m_sCampaignName = DEFAULT_CAMPAIGN;
				break;
				
			}
			isDefault = true;
		}
	}
	
	m_sCampaignName = m_sCampaignName.Left( MAX_SESSION_NAME );

	if (isDefault)
		m_pCampaign->SetString(1,LoadTempString(IDS_HOST_CAMPAIGN_DEFAULT));
	else
		m_pCampaign->SetString(1,m_sCampaignName);

	g_pGameClientShell->SetGameType(m_eGameType);


}


void CScreenHost::SaveOptions()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	pProfile->m_ServerGameOptions.m_eGameType = m_eGameType;
	pProfile->m_ServerGameOptions.m_bUsePassword = !!m_bUsePassword;
	pProfile->m_ServerGameOptions.m_sPassword = m_sPassword;
	pProfile->m_ServerGameOptions.m_sScmdPassword = m_sScmdPassword;
	pProfile->m_ServerGameOptions.m_bAllowScmdCommands = !!m_bAllowScmdCommands;
	pProfile->m_ServerGameOptions.m_nPort = (uint16)atoi(m_sPort);
	pProfile->m_ServerGameOptions.m_nBandwidthServer = m_nBandwidth;
	pProfile->m_ServerGameOptions.m_nBandwidthServerCustom = (uint16)atol(m_sBandwidth);
	pProfile->m_ServerGameOptions.m_bDedicated = !!m_bDedicated;

	switch (m_eGameType)
	{
	case eGameTypeCooperative:
		pProfile->m_ServerGameOptions.GetCoop().m_sSessionName = m_sSessionName;
		pProfile->m_ServerGameOptions.GetCoop().m_sCampaignName = m_sCampaignName;
		break;
	case eGameTypeDeathmatch:
		pProfile->m_ServerGameOptions.GetDeathmatch().m_sSessionName = m_sSessionName;
		pProfile->m_ServerGameOptions.GetDeathmatch().m_sCampaignName = m_sCampaignName;
		break;
	case eGameTypeTeamDeathmatch:
		pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_sSessionName = m_sSessionName;
		pProfile->m_ServerGameOptions.GetTeamDeathmatch().m_sCampaignName = m_sCampaignName;
		break;
	case eGameTypeDoomsDay:
		pProfile->m_ServerGameOptions.GetDoomsday().m_sSessionName = m_sSessionName;
		pProfile->m_ServerGameOptions.GetDoomsday().m_sCampaignName = m_sCampaignName;
		break;
		
	};

	pProfile->ApplyMultiplayer(g_bLAN);
	pProfile->Save(true);
}



void CScreenHost::CreateDefaultCampaign()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	std::string sFN = GetCampaignDir( g_pProfileMgr->GetCurrentProfileName( ), g_pGameClientShell->GetGameType() );


	
	if( !CWinUtil::DirExist( sFN.c_str( )))
	{
		if( !CWinUtil::CreateDir( sFN.c_str( )))
		{
			//TODO: error message
			return;
		}
	}

	sFN += DEFAULT_CAMPAIGN_FILE;

	if (CWinUtil::FileExist(sFN.c_str()))
	{
		remove(sFN.c_str());
	}

	char szString[256];
	char szNum[4];

	sprintf(szNum, "%d", (IsCoopMultiplayerGameType() ? 0 : 1) );
	CWinUtil::WinWritePrivateProfileString( "MissionList", "LoopMissions", szNum, sFN.c_str());

	CWinUtil::WinWritePrivateProfileString( "MissionList", "MissionSourceFile", g_pMissionButeMgr->GetAttributeFile(), sFN.c_str());


	uint16 nCount = 0;
	bool bAdd =false;
	for (int nMission = 0; nMission < g_pMissionButeMgr->GetNumMissions(); nMission++)
	{
		MISSION *pMission = g_pMissionButeMgr->GetMission(nMission);
		if (pMission)
		{
			char szWorldTitle[MAX_PATH] = "";
			_splitpath( pMission->aLevels[0].szLevel, NULL, NULL, szWorldTitle, NULL );

			bAdd = false;
			switch (g_pGameClientShell->GetGameType())
			{
			case eGameTypeDeathmatch:
			case eGameTypeTeamDeathmatch:
				if (strnicmp(szWorldTitle,"DM_",3) == 0)
				{
					bAdd = true;
				}
				break;
			case eGameTypeDoomsDay:
				if (strnicmp(szWorldTitle,"DD_",3) == 0)
				{
					bAdd = true;
				}
				break;
			default:
				bAdd = true;
				break;
			}

			if (bAdd)
			{
				sprintf(szString,"Mission%d",nCount);
				sprintf(szNum,"%d",nMission);
				CWinUtil::WinWritePrivateProfileString( "MissionList", szString, szNum, sFN.c_str());
				nCount++;
			}

		}
		
	}

	// Flush the file.
	CWinUtil::WinWritePrivateProfileString( NULL, NULL, NULL, sFN.c_str());


}
