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
#include "MissionMgr.h"
#include "ClientConnectionMgr.h"
#include "WinUtil.h"
#include "ClientSaveLoadMgr.h"
#include "GameClientShell.h"
#include "CustomCtrls.h"
#include "GameRuleCtrls.h"
#include "HostOptionsMapMgr.h"

#if !defined(PLATFORM_XENON)
#include "iltgameutil.h"
static ILTGameUtil *g_pLTGameUtil;
define_holder(ILTGameUtil, g_pLTGameUtil);
#endif // !defined(PLATFORM_XENON)

static const int kMaxPortStrLen = 5;
static const int kMaxBandwidthStrLen = 8;

namespace
{
	wchar_t s_szTemp[128] = L"";

	void EditBandwidthCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenHost *pThisScreen = (CScreenHost *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_OK,(uint32)pData,CMD_EDIT_BANDWIDTH);
	};

	enum eLocalCommands 
	{
		CMD_SORT_NAME = CMD_CUSTOM+1,
		CMD_TOGGLE_PASS,
		CMD_TOGGLE_SCMDPASS,
		CMD_TOGGLE_LAN,
		CMD_CHOOSE_CAMPAIGN,
		CMD_OPTIONS_FILE,
		CMD_DOWNLOAD,
		CMD_VOTING,
		CMD_TOGGLE_PUNKBUSTER,
	};
}

void LaunchCallBack(bool bReturn, void *pData, void* pUserData);

// Find the index into the game mode based on the game mode cycle control.
static uint32 GameModeCycleToIndex( uint32 nCycle )
{
	uint32 nNumGameModes = DATABASE_CATEGORY( GameModes ).GetNumRecords( );
	uint32 nShownGameModes = 0;
	for( uint32 nGameMode = 0; nGameMode < nNumGameModes; nGameMode++ )
	{
		HRECORD hGameModeRecord = DATABASE_CATEGORY( GameModes ).GetRecordByIndex( nGameMode );

		// Check if this is a multiplayer mode.
		if( !DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( hGameModeRecord, Multiplayer ))
			continue;

		// Check if this shown game mode matches the controls number.
		if( nShownGameModes == nCycle )
		{
			return nGameMode;
		}

		nShownGameModes++;
	}

	return -1;
}

// Find the index into the game mode cycle control based
// on the game mode index.
static uint32 GameModeIndexToCycle( uint32 nGameModeIndex )
{
	uint32 nNumGameModes = DATABASE_CATEGORY( GameModes ).GetNumRecords( );
	uint32 nShownGameModes = 0;
	for( uint32 nGameMode = 0; nGameMode < nNumGameModes; nGameMode++ )
	{
		HRECORD hGameModeRecord = DATABASE_CATEGORY( GameModes ).GetRecordByIndex( nGameMode );

		// Check if this is a multiplayer mode.
		if( !DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( hGameModeRecord, Multiplayer ))
			continue;

		// Check if this shown game mode matches the controls number.
		if( nGameMode == nGameModeIndex )
		{
			return nShownGameModes;
		}

		nShownGameModes++;
	}

	return -1;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHost::CScreenHost()
{
	m_pSessionName	= NULL;
	m_pOptionsFile = NULL;

	m_pWeapons = NULL;
	m_pDownload = NULL;
	m_pVoting = NULL;

	m_pPassword		= NULL;
	m_pPassToggle	= NULL;
	m_pUsePunkbuster = NULL;
	m_pLanToggle	= NULL;
	m_bLan		= false;

	m_pPort			= NULL;

	m_pScmdPassword	= NULL;
	m_pScmdPassToggle	= NULL;

	m_pBandwidthCycle	= NULL;
	m_pBandwidth		= NULL;
	m_nBandwidth		= 0;
	m_sBandwidth = L"";

	LTSNPrintF(s_szTemp,LTARRAYSIZE(s_szTemp),L"%d",DEFAULT_PORT);

	m_sPort = s_szTemp;
	m_bUsePassword = false;
	m_bAllowScmdCommands = false;
	m_bReadyToLaunch = false;
	m_bDedicated = false;
	m_bChangedContentDownload = false;
	m_bUsePunkbuster = false;

	m_nGameType = 0;
}


CScreenHost::~CScreenHost()
{
}

// Build the screen
bool CScreenHost::Build()
{
	int kColumn0 = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0);
	int kColumn1 = (640 - m_ScreenRect.Left()) - kColumn0;
	int kColumn2 = kColumn0 / 2;

	CreateTitle("IDS_TITLE_HOST");

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	LabeledEditCtrl::CreateStruct labeledEditCtrlCS;
	labeledEditCtrlCS.m_cs.rnBaseRect.m_vMin.Init();
	labeledEditCtrlCS.m_cs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	labeledEditCtrlCS.m_nLabelWidth = kColumn0;
	labeledEditCtrlCS.m_nEditWidth = kColumn1;

	labeledEditCtrlCS.m_MaxLength = MAX_SESSION_NAME;
	labeledEditCtrlCS.m_pValueChangingCB = SessionNameValueChangingCB;
	labeledEditCtrlCS.m_pUserData = this;
	m_pSessionName = new GameRuleWStringLabeledEditCtrl;
	m_pSessionName->Create( *this, GameModeMgr::Instance( ).m_grwsSessionName, labeledEditCtrlCS, true );
	labeledEditCtrlCS.m_pValueChangingCB = NULL;

	CLTGUICycleCtrl_create ccs;
	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	ccs.nCommandID = CMD_GAME;
	ccs.szHelpID = "IDS_HELP_GAME_TYPE";
	ccs.pnValue = &m_nGameType;
	ccs.nHeaderWidth = kColumn0;
	ccs.pCommandHandler = this;

	CLTGUICycleCtrl* pCycle = AddCycle("IDS_GAME_TYPE", ccs);

	uint32 nNumGameModes = DATABASE_CATEGORY( GameModes ).GetNumRecords( );
	for( uint32 nGameMode = 0; nGameMode < nNumGameModes; nGameMode++ )
	{
		HRECORD hGameModeRecord = DATABASE_CATEGORY( GameModes ).GetRecordByIndex( nGameMode );

		// Check if this is a multiplayer mode.
		if( !DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( hGameModeRecord, Multiplayer ))
			continue;

		// Add the multiplayer mode label to the cycle.
		pCycle->AddString( LoadString( DATABASE_CATEGORY( GameModes ).GETRECORDATTRIB( hGameModeRecord, Label )));
	}

	CLTGUICtrl_create cs;
  	cs.rnBaseRect.m_vMin.Init();
  	cs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
  	cs.pCommandHandler = this;
	cs.nCommandID = CMD_OPTIONS;
	cs.szHelpID = "IDS_HELP_HOST_GAMETYPE_OPTIONS";
	AddTextItem("IDS_HOST_GAMETYPE_OPTIONS", cs );

	cs.nCommandID = CMD_WEAPONS;
	cs.szHelpID = "IDS_HELP_WPN_RESTRICT";
	m_pWeapons = AddTextItem("IDS_WPN_RESTRICT", cs );

	CLTGUIToggle_create tcs;
	tcs.rnBaseRect.m_vMin.Init();
	tcs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	tcs.pCommandHandler = this;
	tcs.nHeaderWidth = kColumn0;

	cs.nCommandID = CMD_CHOOSE_CAMPAIGN;
	cs.szHelpID = "IDS_HELP_MISSIONS";
	AddTextItem( LoadString("IDS_HOST_MISSIONS"), cs );

	tcs.nCommandID = CMD_TOGGLE_PASS;
	tcs.szHelpID = "IDS_HELP_PASSWORD";
	tcs.pbValue = &m_bUsePassword;
	tcs.nHeaderWidth = kColumn0;
	tcs.pCommandHandler = this;
	m_pPassToggle = AddToggle("IDS_USE_PASSWORD",tcs);

	labeledEditCtrlCS.m_cs.szHelpID = "IDS_HELP_ENTER_PASSWORD";
	labeledEditCtrlCS.m_MaxLength = MAX_PASSWORD;
	labeledEditCtrlCS.m_pwsValue = &m_sPassword;
	labeledEditCtrlCS.m_bPreventEmptyString = true;
	m_pPassword = new LabeledEditCtrl;
	m_pPassword->Create( *this, labeledEditCtrlCS, LoadString("IDS_PASSWORD"), true );
	labeledEditCtrlCS.m_bPreventEmptyString = false;

	tcs.nCommandID = CMD_TOGGLE_SCMDPASS;
	tcs.szHelpID = "IDS_HELP_ALLOWSCMD";
	tcs.pbValue = &m_bAllowScmdCommands;
	m_pScmdPassToggle = AddToggle("IDS_ALLOW_SCMD_COMMANDS",tcs);

	labeledEditCtrlCS.m_cs.szHelpID = "IDS_HELP_ENTER_SCMDPASSWORD";
	labeledEditCtrlCS.m_MaxLength = MAX_PASSWORD;
	labeledEditCtrlCS.m_bPreventEmptyString = true;
	labeledEditCtrlCS.m_pwsValue = &m_sScmdPassword;
	m_pScmdPassword = new LabeledEditCtrl;
	m_pScmdPassword->Create( *this, labeledEditCtrlCS, LoadString("IDS_SCMDPASSWORD"), true );
	labeledEditCtrlCS.m_bPreventEmptyString = false;

	labeledEditCtrlCS.m_cs.szHelpID = "IDS_HELP_ENTER_PORT";
	labeledEditCtrlCS.m_MaxLength = kMaxPortStrLen + 1;
	labeledEditCtrlCS.m_pValueChangingCB = PortValueChangingCB;
	labeledEditCtrlCS.m_pUserData = this;
	labeledEditCtrlCS.m_eInput = kInputNumberOnly;
	labeledEditCtrlCS.m_pwsValue = &m_sPort;
	m_pPort = new LabeledEditCtrl;
	m_pPort->Create( *this, labeledEditCtrlCS, LoadString("IDS_PORT"), true );
	labeledEditCtrlCS.m_pValueChangingCB = NULL;

	tcs.nCommandID = CMD_TOGGLE_PUNKBUSTER;
	tcs.szHelpID = "SCREENHOST_PUNKBUSTER_HELP";
	tcs.pbValue = &m_bUsePunkbuster;
	tcs.nHeaderWidth = kColumn0;
	tcs.pCommandHandler = this;
	m_pUsePunkbuster = AddToggle("SCREENHOST_PUNKBUSTER",tcs);
	m_pUsePunkbuster->SetOnString(LoadString("IDS_YES"));
	m_pUsePunkbuster->SetOffString(LoadString("IDS_NO"));

	tcs.szHelpID = "IDS_HELP_DEDICATED";
	tcs.pbValue = &m_bDedicated;
	tcs.pCommandHandler = NULL;
	CLTGUIToggle *pToggle = AddToggle("IDS_DEDICATED",tcs);
	pToggle->SetOnString(LoadString("IDS_YES"));
	pToggle->SetOffString(LoadString("IDS_NO"));

	tcs.nCommandID = CMD_TOGGLE_LAN;
	tcs.szHelpID = "IDS_HELP_SERVER_LAN";
	tcs.pbValue = &m_bLan;
	tcs.pCommandHandler = this;
	m_pLanToggle = AddToggle("IDS_SERVER_LAN",tcs);
	m_pLanToggle->SetOnString(LoadString("IDS_YES"));
	m_pLanToggle->SetOffString(LoadString("IDS_NO"));

// Lan games not allowed in demos, since they cannot be turned off.
#ifdef _DEMO
	m_bLan = false;
	m_pLanToggle->Enable( false );
#endif // _DEMO

	ccs.rnBaseRect.m_vMin.Init();
	ccs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	ccs.nHeaderWidth = kColumn0;
	ccs.pnValue = &m_nBandwidth;
	ccs.szHelpID = "IDS_HELP_BANDWIDTH_CYCLE";
	m_pBandwidthCycle = AddCycle("IDS_BANDWIDTH_CYCLE",ccs);
//	m_pBandwidthCycle->AddString(LoadString("IDS_56K"));
	m_pBandwidthCycle->AddString(LoadString("IDS_DSL_LOW"));
	m_pBandwidthCycle->AddString(LoadString("IDS_DSL_HIGH"));
	m_pBandwidthCycle->AddString(LoadString("IDS_CABLE"));
	m_pBandwidthCycle->AddString(LoadString("IDS_T1"));
	m_pBandwidthCycle->AddString(LoadString("IDS_T3"));
	m_pBandwidthCycle->AddString(LoadString("IDS_CUSTOM"));

	cs.nCommandID = CMD_EDIT_BANDWIDTH;
	cs.szHelpID = "IDS_HELP_BANDWIDTH_EDIT";
	m_pBandwidth = AddColumnCtrl(cs);
	m_pBandwidth->AddColumn(LoadString("IDS_BANDWIDTH_EDIT"), kColumn0);
	m_pBandwidth->AddColumn(L"<bandwidth>", kColumn2, true);
	m_pBandwidth->AddColumn(L"<maxplayers>", kColumn0, LTTRUE);

	cs.nCommandID = CMD_DOWNLOAD;
	cs.szHelpID = "SCREENHOST_DL_HELP";
	m_pDownload = AddTextItem("SCREENHOST_DL", cs );

	cs.nCommandID = CMD_VOTING;
	cs.szHelpID = "ScreenHost_Voting_Help";
	m_pVoting = AddTextItem("ScreenHost_Voting", cs );

	cs.nCommandID = CMD_OPTIONS_FILE;
	cs.szHelpID = "IDS_HELP_HOST_OPTIONS_FILE";
	m_pOptionsFile = AddTextItem( LoadString("IDS_HOST_OPTIONS_FILE"), cs );

	//restore
	cs.nCommandID = CMD_RESET_DEFAULTS;
	cs.szHelpID = "SCREENHOST_RESETDEFAULTS_HELP";
	AddTextItem("SCREENHOST_RESETDEFAULTS", cs);

	cs.nCommandID = CMD_LAUNCH;
	cs.szHelpID = "IDS_HELP_LAUNCH";
	cs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,0);

	AddTextItem("IDS_HOST_LAUNCH",cs,false,g_pLayoutDB->GetScreenBackFont(),g_pLayoutDB->GetScreenBackSize());

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
	case CMD_GAME:
		{
			UpdateData();
			SaveOptions();
			UpdateGameType();
			
		} break;


	case CMD_EDIT_BANDWIDTH:
		{
			//show edit box here	
			MBCreate mb;
			mb.eType = LTMB_EDIT;
			mb.pFn = EditBandwidthCallBack;
			mb.pUserData = this;
			mb.pString = m_sBandwidth.c_str();
			mb.nMaxChars = kMaxBandwidthStrLen;
			mb.eInput = kInputNumberOnly;
			g_pInterfaceMgr->ShowMessageBox("IDS_BANDWIDTH_EDIT",&mb);
		} break;
	case CMD_TOGGLE_PASS:
		{
			m_pPassToggle->UpdateData();
			m_pPassword->Enable(m_bUsePassword);
		} break;
	case CMD_TOGGLE_PUNKBUSTER:
		{
			// Get the data from the control.
			m_pUsePunkbuster->UpdateData(true);

			// Update the punkbuster api and ui.
			UpdatePunkBuster();

			// Store the change the server settings now, since the control might get switched back to a different
			// value if it fails to init pb.  This way the desired setting persists, but the true setting
			// is reflected in the control.
			GameModeMgr::Instance().m_ServerSettings.SetUsePunkbuster( m_bUsePunkbuster );

			// Update the control with the current setting.
			m_pUsePunkbuster->UpdateData(false);

		} break;
	case CMD_TOGGLE_LAN:
		{
			m_pLanToggle->UpdateData();
			m_pBandwidthCycle->Enable( !m_bLan );
			m_pBandwidth->Enable( !m_bLan );
		} break;
	case CMD_TOGGLE_SCMDPASS:
		{
			m_pScmdPassToggle->UpdateData();
			m_pScmdPassword->Enable(m_bAllowScmdCommands);
		} break;
	case CMD_OPTIONS:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_OPTIONS);
		} break;
	case CMD_WEAPONS:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_WEAPONS);
		} break;
	case CMD_DOWNLOAD:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_DOWNLOAD);
		} break;
	case CMD_VOTING:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_VOTING);
		} break;
	case CMD_CHOOSE_CAMPAIGN:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_LEVELS);
		} break;
	case CMD_OPTIONS_FILE:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_HOST_OPTIONS_FILE);
		} break;
	case CMD_LAUNCH:
		{
		    if (g_pGameClientShell->IsWorldLoaded())
		    {
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = LaunchCallBack;
				mb.pUserData = this;
			    g_pInterfaceMgr->ShowMessageBox("IDS_ENDCURRENTGAME",&mb);
		    }
		    else
		    {
				HandleLaunch();
		    }

		} break;
	case CMD_RESET_DEFAULTS:
		{
			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = ConfirmResetCB;
			mb.pUserData = this;
			g_pInterfaceMgr->ShowMessageBox("SCREENHOST_CONFIRMRESET",&mb);
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// Change in focus
void    CScreenHost::OnFocus(bool bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		
	if (bFocus)
	{
		// Make sure we're disconnected from server.
		if(g_pLTClient->IsConnected())
		{
			g_pInterfaceMgr->SetIntentionalDisconnect( true );
			g_pClientConnectionMgr->ForceDisconnect();
		}

		// Read in the options file.
		GameModeMgr::Instance( ).SetRulesToMultiplayerDefault( );
		GameModeMgr::Instance( ).ReadFromOptionsFile( NULL, pProfile->m_sServerOptionsFile.c_str( ));

		m_nGameType = GameModeIndexToCycle( g_pLTDatabase->GetRecordIndex( GameModeMgr::Instance( ).GetGameModeRecord( )));

		// Update our local variables to reflect gametype.
		UpdateGameType( );
	}
	else
	{

		UpdateData( true );

		SaveOptions();

		if (GameModeMgr::Instance().m_ServerSettings.m_bAllowContentDownload) 
		{
			if (GameModeMgr::Instance( ).m_ServerSettings.GetMaxPerClientDownload() < 1024 )
			{
				//current bandwidth too low to support content download
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox( LoadString("SCREENHOST_NO_DL"), &mb );

				GameModeMgr::Instance().m_ServerSettings.m_bAllowContentDownload = false;

				GameModeMgr::Instance().WriteToOptionsFile( pProfile->m_sServerOptionsFile.c_str( ));


			}
			else
			{
				uint32 nPerClient    = GameModeMgr::Instance().m_ServerSettings.m_nMaxDownloadRatePerClient;
				uint32 nOverall      = GameModeMgr::Instance().m_ServerSettings.m_nMaxDownloadRateAllClients;
				uint32 nMaxDownloads = GameModeMgr::Instance().m_ServerSettings.m_nMaxSimultaneousDownloads;

				//see if we need to reset download settings
				if ((GameModeMgr::Instance().m_ServerSettings.GetMaxPerClientDownload() < nPerClient)
					|| (GameModeMgr::Instance().m_ServerSettings.GetMaxOverallDownload() < nOverall)
					|| (GameModeMgr::Instance().m_ServerSettings.GetMaxPlayersForBandwidth() < nMaxDownloads))
				{
					MBCreate mb;
					g_pInterfaceMgr->ShowMessageBox( LoadString("SCREENHOST_DL_RESET"), &mb );

					// reset the content download options to account for the new bandwidth setting
					uint32 nMaxPlayers = GameModeMgr::Instance().m_ServerSettings.GetMaxPlayersForBandwidth();
					GameModeMgr::Instance().m_ServerSettings.m_nMaxSimultaneousDownloads  = nMaxPlayers;
					GameModeMgr::Instance().m_ServerSettings.m_nMaxDownloadRatePerClient  = GameModeMgr::Instance().m_ServerSettings.GetDefaultPerClientDownload();
					GameModeMgr::Instance().m_ServerSettings.m_nMaxDownloadRateAllClients = GameModeMgr::Instance().m_ServerSettings.m_nMaxDownloadRatePerClient * nMaxPlayers;

					GameModeMgr::Instance().WriteToOptionsFile(g_pProfileMgr->GetCurrentProfile()->m_sServerOptionsFile.c_str());
				}
				else if (m_bChangedContentDownload
						 && (GameModeMgr::Instance().m_ServerSettings.m_bAllowContentDownload))
				{
					// display the dialog so the user knows we changed the content download settings
					MBCreate mb;
					g_pInterfaceMgr->ShowMessageBox( LoadString("SCREENHOST_DL_RESET"), &mb );

					m_bChangedContentDownload = false;
				}
			}
		}
		
		UpdateGameType();

		// Setup the host info, so that loaded games have the correct settings
		g_pClientConnectionMgr->SetupServerHost( );

	}
	CBaseScreen::OnFocus(bFocus);
}

bool CScreenHost::OnLeft()
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
		UpdateBandwidth(true);
        return true;
	}
	return CBaseScreen::OnLeft();
}

bool CScreenHost::OnRight()
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth >= eBandwidth_Custom)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth(true);
        return true;
	}
	return CBaseScreen::OnRight();
}

/******************************************************************/
bool CScreenHost::OnLButtonUp(int x, int y)
{
	CLTGUICtrl *pCtrl = GetSelectedControl();
	if (pCtrl == m_pBandwidthCycle)
	{
		m_nBandwidth = m_pBandwidthCycle->GetSelIndex();
		++m_nBandwidth;
		if (m_nBandwidth >= eBandwidth_Custom)
			m_nBandwidth = 0;
		m_pBandwidthCycle->SetSelIndex(m_nBandwidth);
		UpdateBandwidth(true);
        return true;
	}
	return CBaseScreen::OnLButtonUp(x,y);
}


/******************************************************************/
bool CScreenHost::OnRButtonUp(int x, int y)
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
		UpdateBandwidth(true);
        return true;
	}
	return CBaseScreen::OnRButtonUp(x,y);
}

void CScreenHost::HandleLaunch()
{
	m_bReadyToLaunch = false;
	UpdateData( true );
	SaveOptions();
	UpdateGameType( );

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	
	// Setup the host info.
	if( !g_pClientConnectionMgr->SetupServerHost( ))
	{
		// This should probably prevent them from starting the server if this function fails,
		// But it never does...
	}
	
	if (!LaunchGame())
	{
		g_pInterfaceMgr->LoadFailed(SCREEN_ID_HOST);
	}
}



bool CScreenHost::LaunchGame()
{
	if( !g_pMissionMgr->StartGameNew( ))
		return false;

	return true;
}


void LaunchCallBack(bool bReturn, void *pData, void* pUserData)
{
	CScreenHost *pThisScreen = (CScreenHost *)pUserData;
	if (bReturn && pThisScreen)
	{
		pThisScreen->ReadyLaunch(true);
    }
}


bool CScreenHost::UpdateInterfaceSFX()
{
	if (m_bReadyToLaunch)
		HandleLaunch();

	CBaseScreen::UpdateInterfaceSFX();

	return true;
}


void CScreenHost::ReadyLaunch(bool bReady)
{
	m_bReadyToLaunch = bReady;
}

void CScreenHost::UpdateBandwidth(bool bResetMaxPlayers)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	GameModeMgr::Instance( ).m_ServerSettings.m_nBandwidthServer = m_nBandwidth;

	// get the bandwidth kbps value based on the connection type
	uint16 nBandwidthKbpsValue = 0;
	if ( m_nBandwidth >= eBandwidth_Custom )
	{
		nBandwidthKbpsValue = GameModeMgr::Instance( ).m_ServerSettings.m_nBandwidthServerCustom;
	}
	else
	{
		nBandwidthKbpsValue = g_BandwidthServer[m_nBandwidth];

		// update the custom bandwidth value to match the selected preset 
		LTSNPrintF(s_szTemp,LTARRAYSIZE(s_szTemp),L"%d",nBandwidthKbpsValue);
		m_sBandwidth = s_szTemp;
	}

	// convert it to a string and update the control
	LTSNPrintF(s_szTemp,LTARRAYSIZE(s_szTemp),L"%d",nBandwidthKbpsValue);
	m_pBandwidth->SetString(1,s_szTemp);

	// store the LAN only setting
	GameModeMgr::Instance( ).m_ServerSettings.m_bLANOnly = m_bLan;

	// get the number of players allowed at the specified bandwidth
	uint8 nBandwidthMaxPlayers = GameModeMgr::Instance( ).m_ServerSettings.GetMaxPlayersForBandwidth();

	// store the max players if we're not setting up a LAN game
	if (!m_bLan && bResetMaxPlayers)
	{
		GameModeMgr::Instance( ).m_grnMaxPlayers = nBandwidthMaxPlayers;
	}

	// format the max players string and update the control
	FormatString("SCREENHOST_MAXPLAYERS",s_szTemp,LTARRAYSIZE(s_szTemp),nBandwidthMaxPlayers);
	m_pBandwidth->SetString(2,s_szTemp);

	if (bResetMaxPlayers)
	{
		// store the new bandwidth setting now
		GameModeMgr::Instance().m_ServerSettings.m_nBandwidthServer = m_nBandwidth;
		GameModeMgr::Instance().m_ServerSettings.m_nBandwidthServerCustom = (uint16)_wtol(m_sBandwidth.c_str());
			
		// reset the content download options to account for the new bandwidth setting
		uint32 nMaxPlayersForBandwidth = GameModeMgr::Instance().m_ServerSettings.GetMaxPlayersForBandwidth();
		GameModeMgr::Instance().m_ServerSettings.m_nMaxSimultaneousDownloads  = nMaxPlayersForBandwidth;
		GameModeMgr::Instance().m_ServerSettings.m_nMaxDownloadRatePerClient  = GameModeMgr::Instance().m_ServerSettings.GetDefaultPerClientDownload();
		GameModeMgr::Instance().m_ServerSettings.m_nMaxDownloadRateAllClients = GameModeMgr::Instance().m_ServerSettings.m_nMaxDownloadRatePerClient * nMaxPlayersForBandwidth;

		m_bChangedContentDownload = true;
	}
}


void CScreenHost::HandleCallback(uint32 dwParam1, uint32 dwParam2)
{
	switch(dwParam2)
	{
	case CMD_EDIT_BANDWIDTH:
		{
			wchar_t *pszBandwidth = (wchar_t *)dwParam1;
			uint32 nBandwidth = (uint32)_wtoi(pszBandwidth);
			if ( IsValidBandwidth(nBandwidth) )
			{
				m_sBandwidth = pszBandwidth;
//				m_sBandwidth = m_sBandwidth.Left( kMaxBandwidthStrLen );
				m_nBandwidth = eBandwidth_Custom;
				CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
				GameModeMgr::Instance( ).m_ServerSettings.m_nBandwidthServerCustom = (uint16)_wtol(pszBandwidth);
				m_pBandwidthCycle->UpdateData(false);
				UpdateBandwidth(true);
			}
			else
			{
				wchar_t wszBuffer[384];
				FormatString( "IDS_BANDWIDTH_INVALID", wszBuffer, LTARRAYSIZE(wszBuffer), GetMinimumBandwidth(), GetMaximumBandwidth() );
				MBCreate mb;
				g_pInterfaceMgr->ShowMessageBox( wszBuffer, &mb );
			}
		}
		break;
	}
	UpdateData();
}


void CScreenHost::Escape()
{

	// Initialize to the sp mission bute.
	if( !g_pMissionDB->Init( DB_Default_File ))
	{
		g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Could not load mission bute %s." ), MPA2W( DB_Default_File ).c_str() );
		return;
  	}

	g_pClientConnectionMgr->SetupServerSinglePlayer( );

	CBaseScreen::Escape();
}

void CScreenHost::UpdateGameType()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	// m_nGameType needs to be set prior to calling so it knows which gamemode to load.
	HRECORD hGameModeRecord = g_pLTDatabase->GetRecordByIndex( DATABASE_CATEGORY( GameModes ).GetCategory(), 
		GameModeCycleToIndex( m_nGameType ));
	GameModeMgr::Instance().SetRulesToMultiplayerDefault();
	GameModeMgr::Instance( ).ReadFromOptionsFile( hGameModeRecord, pProfile->m_sServerOptionsFile.c_str( ) );

	// Update local variables.
	m_bUsePassword = GameModeMgr::Instance( ).m_ServerSettings.m_bUsePassword;
	m_pPassword->Enable(m_bUsePassword);
	m_sPassword = GameModeMgr::Instance( ).m_ServerSettings.m_sPassword;
	if (m_sPassword.empty())
		m_sPassword = LoadString("IDS_PASSWORD_DEFAULT");
	m_bAllowScmdCommands = GameModeMgr::Instance( ).m_ServerSettings.m_bAllowScmdCommands;
	m_pScmdPassword->Enable(m_bAllowScmdCommands);
	m_sScmdPassword = GameModeMgr::Instance( ).m_ServerSettings.m_sScmdPassword.c_str( );
	if (m_sScmdPassword.empty())
		m_sScmdPassword = LoadString("IDS_PASSWORD_DEFAULT");

	// Update the punkbuster api and ui.
	m_bUsePunkbuster = GameModeMgr::Instance().m_ServerSettings.GetUsePunkbuster( );
	UpdatePunkBuster();

	uint16 nPort = GameModeMgr::Instance( ).m_ServerSettings.m_nPort;
	LTSNPrintF(s_szTemp,LTARRAYSIZE(s_szTemp),L"%d",nPort);
	m_sPort = s_szTemp;
	m_nBandwidth = GameModeMgr::Instance( ).m_ServerSettings.m_nBandwidthServer;

	uint32 nBandwidthServerCustom = GameModeMgr::Instance( ).m_ServerSettings.m_nBandwidthServerCustom;
	LTSNPrintF(s_szTemp,LTARRAYSIZE(s_szTemp),L"%d",nBandwidthServerCustom);
	m_sBandwidth = s_szTemp;

	m_bDedicated = GameModeMgr::Instance( ).m_ServerSettings.m_bDedicated;
	m_bLan = GameModeMgr::Instance( ).m_ServerSettings.m_bLANOnly;

	m_pBandwidthCycle->Enable( !m_bLan );
	m_pBandwidth->Enable( !m_bLan );

	m_pWeapons->Enable( GameModeMgr::Instance().m_grbUseWeaponRestrictions  );

	UpdateBandwidth(false);

	UpdateData( false );
}


void CScreenHost::SaveOptions()
{
	GameModeMgr& gameModeMgr = GameModeMgr::Instance( );
	gameModeMgr.m_ServerSettings.m_bUsePassword = m_bUsePassword;
	gameModeMgr.m_ServerSettings.m_sPassword = m_sPassword;
	gameModeMgr.m_ServerSettings.m_sScmdPassword = m_sScmdPassword;
	gameModeMgr.m_ServerSettings.m_bAllowScmdCommands = m_bAllowScmdCommands;
	gameModeMgr.m_ServerSettings.m_nPort = (uint16)_wtol(m_sPort.c_str());
	gameModeMgr.m_ServerSettings.m_nBandwidthServer = m_nBandwidth;
	gameModeMgr.m_ServerSettings.m_nBandwidthServerCustom = (uint16)_wtol(m_sBandwidth.c_str());
	gameModeMgr.m_ServerSettings.m_bDedicated = !!m_bDedicated;
	gameModeMgr.m_ServerSettings.m_bLANOnly = m_bLan;
	gameModeMgr.m_ServerSettings.SetUsePunkbuster(m_bUsePunkbuster);

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	GameModeMgr::Instance().WriteToOptionsFile( pProfile->m_sServerOptionsFile.c_str( ));
	CHostOptionsMapMgr::Instance().Save();
}

void CScreenHost::PortValueChangingCB( std::wstring& wsValue, void* pUserData )
{
    CScreenHost *pThisScreen = (CScreenHost *)pUserData;

	if( !wsValue.empty( ))
	{
		// Make sure the input is in range.
		uint16 nPort = (uint16)LTCLAMP( LTStrToLong(wsValue.c_str( )), 0, 65535 );
		wchar_t wszClampedPort[10];
		LTSNPrintF( wszClampedPort, LTARRAYSIZE( wszClampedPort ), L"%d", nPort );
		wsValue = wszClampedPort;

		// Setup the host info.
		GameModeMgr::Instance( ).m_ServerSettings.m_nPort = nPort;
		g_pClientConnectionMgr->SetupServerHost( );
	}
	else
	{
		// set it to the previous value
		wchar_t wszPerviousPort[10];
		LTSNPrintF( wszPerviousPort, LTARRAYSIZE( wszPerviousPort ), L"%d", GameModeMgr::Instance( ).m_ServerSettings.m_nPort );
		wsValue = wszPerviousPort;
	}
};

void CScreenHost::SessionNameValueChangingCB( std::wstring& wsValue, void* pUserData )
{
	if( wsValue.empty( ))
	{
		wsValue = GameModeMgr::Instance( ).m_grwsSessionName.GetDefault();
	}
};

void CScreenHost::ConfirmResetCB(bool bReturn, void *pData, void* pUserData)
{
	CScreenHost *pScreenHost = ( CScreenHost * )pUserData;

	// Check if they chose to reset to the defaults.
	if( bReturn )
	{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

		// Delete the current file.
		if( !pProfile->m_sServerOptionsFile.empty( ))
		{
			char szFilePath[MAX_PATH*2];
			GameModeMgr::Instance( ).GetOptionsFilePath( pProfile->m_sServerOptionsFile.c_str( ), 
				szFilePath, LTARRAYSIZE( szFilePath ));
			LTFileOperations::DeleteFile( szFilePath );
		}

		// Reset to defaults, but keep the same mode setting.
		GameModeMgr::Instance( ).ResetToMode( GameModeMgr::Instance( ).GetGameModeRecord( ));

		// Update the controls with our new settings.
		pScreenHost->UpdateGameType( );

		// Write out our file.
		pScreenHost->SaveOptions( );
	}
}

// Updates the punkbuster api and ui based on current user setting.
void CScreenHost::UpdatePunkBuster( )
{
	// Try to set the setting on punkbuster.  If it failed, then reset the 
	// UI control.
	IPunkBusterServer::StartupInfo PBstartupInfo;
	IPunkBusterServer* pPunkBusterServer = g_pLTGameUtil->CreatePunkBusterServer( PBstartupInfo );
	if( pPunkBusterServer )
	{
		pPunkBusterServer->Initialize();

		char szOut[256];
		sprintf( szOut, "%s\n", __FUNCTION__ );
		OutputDebugString( szOut );

		if( m_bUsePunkbuster && !pPunkBusterServer->IsEnabledRequested( ))
		{
			pPunkBusterServer->Enable( );
		}
		else if( !m_bUsePunkbuster && pPunkBusterServer->IsEnabledRequested( ))
		{
			pPunkBusterServer->Disable( );
		}

		// Get the requested setting.
		m_bUsePunkbuster = pPunkBusterServer->IsEnabledRequested( );

		// If the user tries disable, but the real state is enabled, then tell them this will be possible on the
		// next restart.
		bool bShowNoOnRestart = ( !m_bUsePunkbuster && pPunkBusterServer->IsEnabled( ));

		sprintf( szOut, "%s bShowNoOnRestart(%d)\n", __FUNCTION__, bShowNoOnRestart );
		OutputDebugString( szOut );

		if( bShowNoOnRestart )
		{
			sprintf( szOut, "%s no onnextrestart\n", __FUNCTION__, bShowNoOnRestart );
			OutputDebugString( szOut );

			m_pUsePunkbuster->SetOffString(LoadString("EnablePunkbuster_Yes_NoOnNextRestart"));
		}
		else
		{
			sprintf( szOut, "%s no no\n", __FUNCTION__, bShowNoOnRestart );
			OutputDebugString( szOut );

			m_pUsePunkbuster->SetOffString(LoadString("IDS_NO"));
		}

		// Don't need this any longer.
		g_pLTGameUtil->DestroyPunkBusterServer( pPunkBusterServer );
		pPunkBusterServer = NULL;
	}
	else
	{
		// Pb not available.
		m_bUsePunkbuster = false;
	}
}