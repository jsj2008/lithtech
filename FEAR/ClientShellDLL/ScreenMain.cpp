// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMain.cpp
//
// PURPOSE : Top level interface screen
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenMain.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "WinUtil.h"
#include "direct.h"
#include "GameClientShell.h"
#include "ClientSaveLoadMgr.h"
#include "MissionMgr.h"
#include "ClientConnectionMgr.h"
//#include "ScreenMulti.h"
#include "GameModeMgr.h"
#include "ltprofileutils.h"
#include "PerformanceMgr.h"
#include "ltgamecfg.h"

namespace
{
	enum ScreenMainCmds
	{
		CMD_SINGLE_PLAYER = CMD_CUSTOM+1,
		CMD_SWITCH_TO_SINGLE,
		CMD_CONTINUE_GAME,
		CMD_MULTI_PLAYER,
		CMD_SWITCH_TO_MULTI,
		CMD_MAIN_OPTIONS,
		CMD_PROFILE,
		CMD_EXIT, 
		CMD_QUIT,
	};

	void QuitCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenMain *pThisScreen = (CScreenMain *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_EXIT,0,0);
	};

	void SwitchToSinglePlayerCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenMain *pThisScreen = (CScreenMain *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_SWITCH_TO_SINGLE,0,0);
	};

	void SwitchToMultiPlayerCallBack(bool bReturn, void *pData, void* pUserData)
	{
		CScreenMain *pThisScreen = (CScreenMain *)pUserData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_SWITCH_TO_MULTI,0,0);
	};

	int32 kTextWidth = 200;

}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMain::CScreenMain()
{
    m_pResume = NULL;

}

CScreenMain::~CScreenMain()
{
}


// Build the screen
bool CScreenMain::Build()
{


	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

#if !defined( _DEMO ) || defined( _SPDEMO )

	// Don't add the Single player menu item if it's the MPFree product.
	if( !LTGameCfg::IsMPFreeProduct())
	{
		cs.nCommandID = CMD_SINGLE_PLAYER;
		cs.szHelpID = "IDS_HELP_SINGLEPLAYER";
		AddTextItem("IDS_SINGLEPLAYER", cs );
	}

#endif // !defined( _DEMO ) || defined( _SPDEMO )

/*
	cs.nCommandID = CMD_CONTINUE_GAME;
	cs.nHelpID = IDS_HELP_CONTINUE_GAME;
	m_pResume = AddTextItem(IDS_CONTINUE_GAME, cs );
	m_pResume->Enable(false);
*/

#if !defined( _DEMO ) || defined( _MPDEMO )

	cs.nCommandID = CMD_MULTI_PLAYER;
	cs.szHelpID = "IDS_HELP_MULTIPLAYER";
	AddTextItem("IDS_MULTIPLAYER", cs );

#endif // !defined( _DEMO ) || defined( _MPDEMO )

	cs.nCommandID = CMD_MAIN_OPTIONS;
	cs.szHelpID = "IDS_HELP_OPTIONS";
	AddTextItem("IDS_OPTIONS", cs);

	cs.nCommandID = CMD_PROFILE;
	cs.szHelpID = "IDS_HELP_PROFILE";
	AddTextItem("IDS_PROFILE", cs);


	cs.nCommandID = CMD_QUIT;
	cs.szHelpID = "IDS_HELP_EXIT";
	cs.rnBaseRect = m_pScreenMgr->GetBackRect();
	AddTextItem("IDS_EXIT", cs );


	// Put the version number at versionpos, but don't let it clip off the edge.
	
	wchar_t wszVersion[256];
	g_pVersionMgr->GetDisplayVersion( wszVersion, LTARRAYSIZE( wszVersion ));
	LTStrCat( wszVersion, L" - ", LTARRAYSIZE( wszVersion ));
	if (LTStrIEquals( g_pClientConnectionMgr->GetModName( ), RETAIL_MOD_NAME ))
	{
		LTStrCat( wszVersion, LoadString("Version_Retail"), LTARRAYSIZE( wszVersion ));
	}
	else
	{
		char szMod[128];
		ModNameToModDisplayName( g_pClientConnectionMgr->GetModName( ), szMod, LTARRAYSIZE( szMod ));
		LTStrCat( wszVersion, MPA2W( szMod ).c_str( ), LTARRAYSIZE( wszVersion ));
	}

	cs.nCommandID = 0;
	cs.szHelpID = "";
	cs.rnBaseRect.Left() = 480;
	cs.rnBaseRect.Top() = 480-g_pLayoutDB->GetHelpSize();
	cs.rnBaseRect.Bottom() = 480;
	cs.rnBaseRect.Right() = 632;


	CLTGUITextCtrl *pCtrl= AddTextItem( wszVersion,cs,true,g_pLayoutDB->GetHelpFont(),g_pLayoutDB->GetHelpSize());
	pCtrl->SetAlignment(kRight);

	// Make sure to call the base class
	if (!CBaseScreen::Build()) return false;

	UseBack(false);

	return true;

}

void CScreenMain::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		// Make sure we're disconnected from server.
		if(g_pLTClient->IsConnected())
		{
			g_pInterfaceMgr->SetIntentionalDisconnect( true );
			g_pClientConnectionMgr->ForceDisconnect();
		}

		//since can get here from various points in the game, let's make sure that we're not fading in or out...
		g_pInterfaceMgr->AbortScreenFade();


		SetSelection(-1);

		// Always assume sp save/load when in the main screen.  This is so the 
		// "continue game" and "quick load" assumes sp.
		HRECORD hGameModeRecord = g_pLTDatabase->GetRecord( DATABASE_CATEGORY( GameModes ).GetCategory( ), GameModeMgr::GetSinglePlayerRecordName( ));
		if( !GameModeMgr::Instance().ResetToMode( hGameModeRecord ))
		{
			g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Invalid single player game mode" ));
			return;
		}

		// Initialize to the sp mission bute.
		if( !g_pMissionDB->Init( DB_Default_File ))
		{
			g_pLTClient->ShutdownWithMessage( LT_WCHAR_T( "Could not load mission bute %s." ), MPA2W( DB_Default_File ).c_str() );
			return;
  		}

		g_pClientSaveLoadMgr->SetUseMultiplayerFolders( false );
		g_pClientConnectionMgr->SetupServerSinglePlayer( );

		if( m_pResume )
			m_pResume->Enable( g_pClientSaveLoadMgr->CanContinueGame() );

		// We don't go thru the main screen when joining from command line, so disable it if
		// we get here.
		g_pInterfaceMgr->SetCommandLineJoin( false );
	}

	CBaseScreen::OnFocus(bFocus);
}


uint32 CScreenMain::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_SINGLE_PLAYER:
		{
			// If running MP, then ask user to switch to SP exe.
			if( IsMultiplayerGameClient( ))
			{
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = SwitchToSinglePlayerCallBack;
				mb.pUserData = this;
				g_pInterfaceMgr->ShowMessageBox("ScreenMain_SwitchToSinglePlayer",&mb);
			}
			else
			{
				m_pScreenMgr->SetCurrentScreen(SCREEN_ID_SINGLE);
			}
			break;
		}

	case CMD_SWITCH_TO_SINGLE:
		{
			// Switch to the SP executable.
			if( !LaunchApplication::LaunchSinglePlayerExe( LaunchApplication::kSwitchToScreen_Single ))
				return false;
		}
		break;

	case CMD_SWITCH_TO_MULTI:
		{
			// Switch to the SP executable.
			if( !LaunchApplication::LaunchMultiPlayerExe( LaunchApplication::kSwitchToScreen_Multi ))
				return false;
		}
		break;

	case CMD_MULTI_PLAYER:
		{
			// If running SP, then ask user to switch to MP exe.
			if( !IsMultiplayerGameClient( ))
			{
				MBCreate mb;
				mb.eType = LTMB_YESNO;
				mb.pFn = SwitchToMultiPlayerCallBack;
				mb.pUserData = this;
				g_pInterfaceMgr->ShowMessageBox("ScreenMain_SwitchToMultiPlayer",&mb);
			}
			else
			{
				m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MULTI);
			}
			break;
		}

	case CMD_MAIN_OPTIONS:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_OPTIONS);
			break;
		}
	case CMD_PROFILE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_PROFILE);
			break;
		}
	case CMD_QUIT:
		{

			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = QuitCallBack;
			mb.pUserData = this;
			g_pInterfaceMgr->ShowMessageBox("IDS_SUREWANTQUIT",&mb);
			break;
		}
	case CMD_EXIT:
		{
#ifdef _DEMO
			g_pInterfaceMgr->ShowDemoScreens(true);
#else
            g_pLTClient->Shutdown();
#endif
			break;
		}
	case CMD_CONTINUE_GAME:
		{
			// Initialize to the sp mission bute.
			if( !g_pMissionDB->Init( DB_Default_File ))
			{
				g_pLTClient->ShutdownWithMessage(LT_WCHAR_T("Could not load mission bute %s."), MPA2W(DB_Default_File).c_str());
				return 0;
  			}

			// Start the game from the continue save.
			if( !g_pMissionMgr->StartGameFromContinue( ))
				return 0;

			return 1;
			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};

// Screen specific rendering
bool CScreenMain::Render()
{
	bool bReturn = CBaseScreen::Render();

	static bool bFirstTime = true;
	if( bFirstTime )
	{
		AutoDetectPerformanceSettings();
		bFirstTime = false;
	}

	// Check if a patch URL came in.  Only ask the user once if they
	// want to patch.
	if (!g_pInterfaceMgr->HasAskedToPatch())
	{
		g_pInterfaceMgr->AskToPatch();
	}

	return bReturn;
}


void CScreenMain::Escape()
{
	CBaseScreen::Escape();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenMain::AutoDetectPerformanceSettings()
//
//	PURPOSE:	automatically adjusts the performance settings the first 
//				time the game is run
//
// ----------------------------------------------------------------------- //
void CScreenMain::AutoDetectPerformanceSettings()
{
	uint32 nGameRuns = LTProfileUtils::ReadUint32( "Game", "GameRuns", 0, g_pVersionMgr->GetGameSystemIniFile() );
	if( nGameRuns != 1 )
		return;

	g_pInterfaceResMgr->DrawMessage("PerformanceMessage_FirstTimeAutoDetect");
	CPerformanceMgr::Instance().SetBasedOnPerformanceStats();
	CPerformanceMgr::Instance().ApplyQueuedConsoleChanges(true);
	g_pProfileMgr->GetCurrentProfile()->Save();

}