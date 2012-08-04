// ----------------------------------------------------------------------- //
//
// MODULE  : LoadingScreen.cpp
//
// PURPOSE : Background-thread loading screen encapsulation class
//
// CREATED : 2000
//
// (c) 2000-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LoadingScreen.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "WinUtil.h"
#include "BaseScreen.h"
#include "MissionMgr.h"
#include "MissionDB.h"
#include "ClientConnectionMgr.h"
#include "iltrenderer.h"
#include "LoadingScreenLayout.h"
#include "sys/win/mpstrconv.h"
#include "GameModeMgr.h"
#include "iltloadingprogress.h"
#include "time.h"
#include "ltfileoperations.h"

extern CGameClientShell* g_pGameClientShell;

void LoadingProgressCallbackFn(ELoadingProgressTask eTask, float fProgress, void* pUser)
{
	if (pUser)
	{
		((CLoadingScreen*)pUser)->LoadingProgressCallback(eTask,fProgress,pUser);
	}
}

namespace
{
	uint8 nAlpha;
	uint8 nR;
	uint8 nG;
	uint8 nB;
	bool bFade;
}

static const uint32 knKB = (1<<10);
static const float kfKB = (float)knKB;
static const uint32 knMB = (1<<20);
static const float kfMB = (float)knMB;

CLoadingScreen::CLoadingScreen() :
	m_eCurState(STATE_NONE)
{

	m_nOldFarZ = 10000;
	m_bOldFogEnable = false;

	m_bReadLayout = false;
	m_bHaveServerInfo = false;

	m_hBackTexture = NULL;

	InitializeCriticalSection(&m_MissionUpdate);
}

CLoadingScreen::~CLoadingScreen()
{
	// Terminate the object, just in case...
	Term();

	DeleteCriticalSection(&m_MissionUpdate);

}


bool CLoadingScreen::Init()
{
	if (m_eCurState != STATE_NONE)
		return false;

//	DebugCPrint(0,"%s : SettingCallback : %0.2f",__FUNCTION__,(clock() / (float)CLOCKS_PER_SEC));
	m_dlgClientLoggedIn.Attach(this,g_pClientConnectionMgr,g_pClientConnectionMgr->ClientLoggedInEvent);

	g_pLTClient->LoadingProgress()->SetLoadingCallback( LoadingProgressCallbackFn , this);
	m_fLoadProgress = 0.0f;

	if( m_pRenderScreen )
	{
		// Just let the render screen know it is about to be draw and set the state to init...

		m_pRenderScreen->OnFocus( true );
		m_eCurState = STATE_INIT;

		return true;
	}
	

	m_photo = "";
	m_bHaveServerInfo = false;

	CFontInfo Font(g_pLayoutDB->GetHelpFont(),12);

	nAlpha = 255;
	bFade = true;

	CLTGUICtrl_create cs;
	m_MissionName.Create(L"",Font,cs);
	m_LevelName.Create(L"",Font,cs);
	m_Briefing.Create(L"",Font,cs);
	m_ServerMsg.Create(L"",Font,cs);
	m_BriefingHeader.Create(LoadString("LoadScreen_Briefing"),Font,cs);
	m_ServerMsgHeader.Create(LoadString("LoadScreen_ServerMessage"),Font,cs);
	m_Help.Create(L"",Font,cs);

	m_CurrentFileName.Create(L"",Font,cs);
	m_CurrentFileTime.Create(L"",Font,cs);
	m_FilesLeft.Create(L"",Font,cs);
	m_TotalTime.Create(L"",Font,cs);
	

	HRECORD hMission = NULL;
	if( !g_pMissionMgr->IsCustomLevel( ))
	{
		uint32 nCurMission = g_pMissionMgr->GetCurrentMission( );
		hMission = g_pMissionDB->GetMission( nCurMission );
	}

	// Check if we have a mission entry.
	if (IsMultiplayerGameClient())
	{
		m_layout = g_pLayoutDB->GetLoadScreenRecord("DefaultMP");
		m_Briefing.SetClipping(true);

	}
	else
	{
		m_layout = g_pLayoutDB->GetLoadScreenRecord("Default");
		m_Briefing.SetClipping(false);

	}
	
	

	if (!g_DefaultLayout.m_bReadLayout)
	{
		g_DefaultLayout.Read(g_pLayoutDB->GetLoadScreenRecord("Default"));
	}

	if( hMission )
	{
		ReadMissionInfo(hMission);
		UpdateServerInfo();
	}

	
	if (!g_CurrentLayout.Read(m_layout))
		g_CurrentLayout = g_DefaultLayout;

	UpdateLayout();


	// Reset the frame counter
	m_nFrameCounter = 0;
	m_fLastFrameTime = CWinUtil::GetTime();
	m_fCurFrameDelta = 0.0f;
	
	m_eCurState = STATE_INIT;

	UpdateProgressBar(0.0f);
	UpdateCurrentBar(0.0f);
	UpdateTotalBar(0.0f);

	if (!hMission)
	{
		UpdateSessionName( );

		m_Briefing.SetString(L"");
		m_ServerMsg.SetString(L"");
		m_Help.SetString(L"");
		m_BriefingHeader.Show(false);
		m_ServerMsgHeader.Show(false);

	}

	if (g_pGameClientShell->IsRunningPerformanceTest())
	{
		m_Briefing.SetString(LoadString("IDS_PERFORMANCE_TEST_BRIEF"));
	}

	return true;
}

bool CLoadingScreen::Term()
{
	if (m_eCurState != STATE_INIT)
		return false;

	m_dlgClientLoggedIn.Detach();

	m_MissionName.FlushTextureStrings();
	m_LevelName.FlushTextureStrings();
	m_Briefing.FlushTextureStrings();
	m_ServerMsg.FlushTextureStrings();
	m_BriefingHeader.FlushTextureStrings();
	m_ServerMsgHeader.FlushTextureStrings();
	m_Help.FlushTextureStrings();
	m_CurrentFileName.FlushTextureStrings();
	m_CurrentFileTime.FlushTextureStrings();
	m_FilesLeft.FlushTextureStrings();
	m_TotalTime.FlushTextureStrings();

	m_hBackTexture.Free();

	m_eCurState = STATE_NONE;

	return true;
}

bool CLoadingScreen::Update()
{
	// Make sure we're in a valid state...
	if ((m_eCurState != STATE_ACTIVE) && (m_eCurState != STATE_SHOW))
		return false;

	// Only draw the render screen if we have one...

	if( m_pRenderScreen )
	{
		bool bRet = false;
		if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
		{
			EnterCriticalSection(&m_MissionUpdate);

			// Actually draw the screen...

			bRet = m_pRenderScreen->Render( );

			g_pLTClient->RenderConsoleToRenderTarget();

			LeaveCriticalSection(&m_MissionUpdate);

			g_pLTClient->GetRenderer()->End3D();
			g_pLTClient->GetRenderer()->FlipScreen();

			
		}
		return bRet;
		
	}

	g_pLTClient->GetRenderer()->ClearRenderTarget(CLEARRTARGET_ALL, 0);
	// Mmm..  Triple dimensional...
	if (LT_OK == g_pLTClient->GetRenderer()->Start3D())
	{
		g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);

		g_pDrawPrim->SetTexture(m_hBackTexture);
		g_pDrawPrim->DrawPrim(&m_BackPoly, 1);

		if (m_hPhoto && m_bHaveServerInfo)
		{
			g_pDrawPrim->SetTexture(m_hPhoto);
			g_pDrawPrim->DrawPrim(&m_photoPoly,1);
		}

		EnterCriticalSection(&m_MissionUpdate);

		m_MissionName.Render();
		m_LevelName.Render();
		m_Briefing.Render();
		if (IsMultiplayerGameClient())
		{
			m_ServerMsg.Render();
			m_BriefingHeader.Render();
			m_ServerMsgHeader.Render();
		}
		m_Help.Render();
		m_LoadProgress.Render();


		if (m_bContentTransfer)
		{
			m_ContentFrame.Render();
			m_CurrentFileName.Render();
			m_CurrentFileTime.Render();
			m_FilesLeft.Render();
			m_TotalTime.Render();
			m_CurrentFile.Render();
			m_Total.Render();

		}


		g_pLTClient->RenderConsoleToRenderTarget();

		LeaveCriticalSection(&m_MissionUpdate);
  
		g_pLTClient->GetRenderer()->End3D();
		g_pLTClient->GetRenderer()->FlipScreen();

#if !defined (PLATFORM_XENON )

		if (IsKeyDown(VK_ESCAPE))
		{
			// determine which screen we should display
			if (IsMultiplayerGameClient())
			{
				g_pInterfaceMgr->LoadFailed(SCREEN_ID_MULTI, NULL, false);
			}
			else
			{
				g_pInterfaceMgr->LoadFailed(SCREEN_ID_MAIN, LoadString( "Screen_LoadAborted" ), true);
			}
		}
		
	}
#endif

	// Count it..
	++m_nFrameCounter;

	float fCurTime = CWinUtil::GetTime();
	m_fCurFrameDelta = fCurTime - m_fLastFrameTime;
	m_fLastFrameTime = fCurTime;

	return true;
}

bool CLoadingScreen::Show(bool bNew)
{

	// Make sure we're in the correct state
	if (m_eCurState == STATE_NONE)
	{
		if (!Init())
			return false;
	}

	if (m_eCurState != STATE_INIT)
		return false;


	// Turn off the cursor
	g_pCursorMgr->UseCursor(false);

	// Set up the FarZ & turn off fog (farz of 0 is bogus)

	m_nOldFarZ = GetConsoleInt("FarZ", 10000);
	m_nOldFarZ = m_nOldFarZ == 0 ? 10000 : m_nOldFarZ;

	m_bOldFogEnable = !!GetConsoleInt("FogEnable", 0);

	g_pGameClientShell->SetFarZ(10000);
	WriteConsoleInt("FogEnable", 0);

	// Go into the right state..
	m_eCurState = STATE_SHOW;

	// Update once so the screen's showing
	Update();

	// Start updating if they wanted it to..
	return Resume();
}

bool CLoadingScreen::Pause()
{
	// Make sure we're in the right state
	if (m_eCurState != STATE_ACTIVE)
		return false;

	// Ok, it's just visible now..
	m_eCurState = STATE_SHOW;

	return true;
}

bool CLoadingScreen::Resume()
{
	// Ensure our state
	if (m_eCurState != STATE_SHOW)
		return false;

	// Change state
	m_eCurState = STATE_ACTIVE;

	return true;
}

bool CLoadingScreen::Hide()
{
	// Ensure our state
	if (m_eCurState == STATE_ACTIVE)
	{
		// Stop!!!
		if (!Pause())
			return false;
	}

	if (m_eCurState != STATE_SHOW)
		return false;

	// Change state
	m_eCurState = STATE_INIT;

	// Clean up
	Term();

	// Re-set the console...
	g_pGameClientShell->SetFarZ(m_nOldFarZ);
	WriteConsoleInt("FogEnable", (int)m_bOldFogEnable);

	// Let the render screen know it's going away...

	if( m_pRenderScreen )
	{
		// Don't lose focus until after setting the state to init...
		
		m_pRenderScreen->OnFocus( false );
		m_pRenderScreen = NULL;
	}

	// Done!
	return true;
}



void CLoadingScreen::UpdateMissionInfo()
{
	// No need to update mission info if a different screen is being rendered...
	
	if( m_pRenderScreen )
		return;

	EnterCriticalSection(&m_MissionUpdate);

	char const* pszCurWorldName = g_pMissionMgr->GetCurrentWorldName( );
	if( pszCurWorldName && pszCurWorldName[0] && !g_pMissionMgr->IsCustomLevel( ))
	{
		uint32 nCurMission = g_pMissionMgr->GetCurrentMission( );
		HRECORD hMission = g_pMissionDB->GetMission( nCurMission );

		if( hMission )
		{
			ReadMissionInfo(hMission);
		}
	}
	else
	{
        UpdateSessionName( );
	}

	if (g_pGameClientShell->IsRunningPerformanceTest())
	{
		m_Briefing.SetString(LoadString("IDS_PERFORMANCE_TEST_BRIEF"));
	}


	LeaveCriticalSection(&m_MissionUpdate);
	Update();
}

void CLoadingScreen::ClearContentDownloadInfo()
{
	m_bContentTransfer = false;
	m_nTotalBytes = 0;
	m_nCompletedBytes = 0;
	m_nCurrentFiles = 0;
}

void CLoadingScreen::ClientContentTransferStartingNotification(uint32 nTotalBytes, uint32 nTotalFiles)
{
	//nothing to download
	if (!nTotalFiles && !nTotalBytes)
	{
		return;
	}

	// No need to update content info if a different screen is being rendered...

	if( m_pRenderScreen )
		return;

	EnterCriticalSection(&m_MissionUpdate);

	m_bContentTransfer = true;
	m_fLastUpdateTime = 0.0f;

	m_nTotalBytes = nTotalBytes;
	m_nCompletedBytes = 0;

	m_nCurrentFiles = nTotalFiles;

	wchar_t szMsg[128];
	m_CurrentFileName.SetString(L"");
	m_CurrentFileTime.SetString(L"");

	FormatString("CONTENTDL_FILES",szMsg,LTARRAYSIZE(szMsg),nTotalFiles);
	m_FilesLeft.SetString(szMsg);

	m_TotalTime.SetString(L"");

	UpdateCurrentBar(0.0f);
	UpdateTotalBar(0.0f);

	LeaveCriticalSection(&m_MissionUpdate);
	Update();
}



void CLoadingScreen::FileReceiveProgressNotification(const std::string& strFilename,
													 uint32				nFileBytesReceived,
													 uint32				nFileBytesTotal,
													 uint32				nTransferBytesTotal,
													 float				fTransferRate)
{
	// No need to update content info if a different screen is being rendered...

	if( m_pRenderScreen )
		return;

	EnterCriticalSection(&m_MissionUpdate);

	// update the byte counts
	m_nCurrentBytes = m_nCompletedBytes + nFileBytesReceived;
	m_nTotalBytes = nTransferBytesTotal;

	wchar_t szMsg[128];
	if (nFileBytesTotal < knKB)
	{
		FormatString("CONTENTDL_CURRENT_FILE_B",szMsg,LTARRAYSIZE(szMsg),MPA2W(strFilename.c_str()).c_str(),nFileBytesTotal);
	}
	else if (nFileBytesTotal < knMB)
	{
		FormatString("CONTENTDL_CURRENT_FILE_KB",szMsg,LTARRAYSIZE(szMsg),MPA2W(strFilename.c_str()).c_str(),nFileBytesTotal/knKB);
	}
	else
	{
		FormatString("CONTENTDL_CURRENT_FILE_MB",szMsg,LTARRAYSIZE(szMsg),MPA2W(strFilename.c_str()).c_str(),(float)nFileBytesTotal/kfMB );
	}
	m_CurrentFileName.SetString( szMsg );

	FormatString("CONTENTDL_FILES",szMsg,LTARRAYSIZE(szMsg),m_nCurrentFiles);
	m_FilesLeft.SetString(szMsg);
	

	double fElapsed = RealTimeTimer::Instance().GetTimerAccumulatedS() - m_fLastUpdateTime;
	if (fElapsed > 1.0f) 
	{
		m_fLastUpdateTime = RealTimeTimer::Instance().GetTimerAccumulatedS();

		float fTimeLeft = 999.0f * 60.0f;
		if (fTransferRate > 0.0f)
		{
			fTimeLeft = float(nFileBytesTotal - nFileBytesReceived) / fTransferRate;
		}
		if (fTimeLeft > 90.0f)
			FormatString("CONTENTDL_TIME_M",szMsg,LTARRAYSIZE(szMsg),uint32(fTimeLeft/60.0f + 0.5f), (fTransferRate / kfKB) );
		else
			FormatString("CONTENTDL_TIME_S",szMsg,LTARRAYSIZE(szMsg),uint32(fTimeLeft), (fTransferRate / kfKB) );
		m_CurrentFileTime.SetString(szMsg);

		if (fTransferRate > 0.0f)
		{
			fTimeLeft = float(m_nTotalBytes - m_nCurrentBytes) / fTransferRate;
		}
		if (fTimeLeft > 90.0f)
			FormatString("CONTENTDL_TIME_M",szMsg,LTARRAYSIZE(szMsg),uint32(fTimeLeft/60.0f + 0.5f), (fTransferRate / kfKB) );
		else
			FormatString("CONTENTDL_TIME_S",szMsg,LTARRAYSIZE(szMsg),uint32(fTimeLeft), (fTransferRate / kfKB) );
		m_TotalTime.SetString(szMsg);
	}



	float fPercent = 0.0f;
	if (nFileBytesTotal > 0)
	{
		fPercent = float(nFileBytesReceived) / float(nFileBytesTotal);
	}
	UpdateCurrentBar(fPercent);


	fPercent = 0.0f;
	if (m_nTotalBytes > 0)
	{
		fPercent = float(m_nCurrentBytes) / float(m_nTotalBytes);
	}
	UpdateTotalBar(fPercent);

	LeaveCriticalSection(&m_MissionUpdate);
	Update();
}

void CLoadingScreen::FileReceiveCompletedNotification(const std::string& strFilename,
													  uint32			 nFileBytesTotal,
													  uint32			 nTransferBytesTotal,
													  float				 fTransferRate)
{
	// No need to update content info if a different screen is being rendered...

	if( m_pRenderScreen )
		return;

	EnterCriticalSection(&m_MissionUpdate);

	// update the byte counts and file count
	m_nCompletedBytes += nFileBytesTotal;
	m_nCurrentBytes = m_nCompletedBytes;
	m_nTotalBytes = nTransferBytesTotal;
	m_nCurrentFiles--;


	wchar_t szMsg[128];
	if (nFileBytesTotal < knKB)
	{
		FormatString("CONTENTDL_CURRENT_FILE_B",szMsg,LTARRAYSIZE(szMsg),MPA2W(strFilename.c_str()).c_str(),nFileBytesTotal);
	}
	else if (nFileBytesTotal < knMB)
	{
		FormatString("CONTENTDL_CURRENT_FILE_KB",szMsg,LTARRAYSIZE(szMsg),MPA2W(strFilename.c_str()).c_str(),nFileBytesTotal/knKB);
	}
	else
	{
		FormatString("CONTENTDL_CURRENT_FILE_MB",szMsg,LTARRAYSIZE(szMsg),MPA2W(strFilename.c_str()).c_str(),(float)nFileBytesTotal/kfMB );
	}
	m_CurrentFileName.SetString( szMsg );
	m_CurrentFileTime.SetString(L"");

	FormatString("CONTENTDL_FILES",szMsg,LTARRAYSIZE(szMsg),m_nCurrentFiles);
	m_FilesLeft.SetString(szMsg);

	UpdateCurrentBar(1.0f);

	double fElapsed = RealTimeTimer::Instance().GetTimerAccumulatedS() - m_fLastUpdateTime;
	if (fElapsed > 0.01f) 
	{

		m_fLastUpdateTime = RealTimeTimer::Instance().GetTimerAccumulatedS();

		float fTimeLeft = 999.0f * 60.0f;
		if (fTransferRate > 0.0f)
		{
			fTimeLeft = float(m_nTotalBytes - m_nCurrentBytes) / fTransferRate;
		}
		if (fTimeLeft > 90.0f)
			FormatString("CONTENTDL_TIME_M",szMsg,LTARRAYSIZE(szMsg),uint32(fTimeLeft/60.0f + 0.5f), (fTransferRate / kfKB) );
		else
			FormatString("CONTENTDL_TIME_S",szMsg,LTARRAYSIZE(szMsg),uint32(fTimeLeft), (fTransferRate / kfKB) );
		m_TotalTime.SetString(szMsg);
	}


	LeaveCriticalSection(&m_MissionUpdate);
	Update();
}


void CLoadingScreen::ClientContentTransferCompletedNotification()
{
	// No need to update content info if a different screen is being rendered...
	if( m_pRenderScreen )
		return;

	EnterCriticalSection(&m_MissionUpdate);

	// rebuild and reinitialize the mission database
	g_pMissionDB->Reset();
	g_pMissionDB->Init(DB_Default_File);
	g_pMissionDB->CreateMPDB();

	char szPath[MAX_PATH*2];
	LTFileOperations::GetUserDirectory(szPath, LTARRAYSIZE(szPath));
	LTStrCat( szPath, MDB_MP_File, LTARRAYSIZE( szPath ));
	g_pMissionDB->Init(szPath);

	m_CurrentFileName.SetString(L"");
	m_CurrentFileTime.SetString(L"");
	m_FilesLeft.SetString(L"");
	m_TotalTime.SetString(L"");

	UpdateCurrentBar(1.0f);
	UpdateTotalBar(1.0f);

	m_bContentTransfer = false;

	LeaveCriticalSection(&m_MissionUpdate);
	Update();
}

void CLoadingScreen::UpdateProgressBar(float fPercent)
{
	m_LoadProgress.Update(m_rfLoadProgress.Left(),m_rfLoadProgress.Top(),fPercent*m_rfLoadProgress.GetWidth(),m_rfLoadProgress.GetWidth(),m_rfLoadProgress.GetHeight());
}

void CLoadingScreen::UpdateCurrentBar(float fPercent)
{
	m_CurrentFile.Update(m_rfCurrentFile.Left(),m_rfCurrentFile.Top(),fPercent*m_rfCurrentFile.GetWidth(),m_rfCurrentFile.GetWidth(),m_rfCurrentFile.GetHeight());
}


void CLoadingScreen::UpdateTotalBar(float fPercent)
{
	m_Total.Update(m_rfTotal.Left(),m_rfTotal.Top(),fPercent*m_rfTotal.GetWidth(),m_rfTotal.GetWidth(),m_rfTotal.GetHeight());
}




bool CLoadingScreen::NeedsPostLoadScreen() const
{
	// Always go to postload screen if in mp.  We may need to wait there for other players.
	if( IsMultiplayerGameClient( ))
		return true;

	// Only go to postload if we had a briefing for sp.
	return ( !LTStrEmpty(m_Briefing.GetString()));
}

void CLoadingScreen::UpdateSessionName( )
{
	// If connecting to a remote server, set our mission descriptor to 
	// the ip we're connecting to.
	if( g_pClientConnectionMgr->IsConnectedToRemoteServer( ))
	{
		// Make a loading string using the IP to be joined.
		wchar_t szLoadingString[256];

//		LTSNPrintF( szSession, LTARRAYSIZE(szSession), L"%s", g_pClientConnectionMgr->GetStartGameRequest().m_pNetSession->m_sName );
		if ( !LTStrEmpty(g_pClientConnectionMgr->GetServerName()))
		{
			LTSNPrintF( szLoadingString, LTARRAYSIZE(szLoadingString), L"%s:  %s", LoadString("IDS_CONNECTING_TO_SERVER"), 
				g_pClientConnectionMgr->GetServerName() );
		}
		else
		{
			LTSNPrintF( szLoadingString, LTARRAYSIZE(szLoadingString), L"%s", LoadString("IDS_CONNECTING_TO_SERVER"));
		}
		m_MissionName.SetFont(g_CurrentLayout.m_sLevelFont);
		m_MissionName.SetString(szLoadingString);

		LTSNPrintF( szLoadingString, LTARRAYSIZE(szLoadingString), L"    (%S)", g_pClientConnectionMgr->GetStartGameRequest( ).m_TCPAddress );

		m_LevelName.SetString(szLoadingString);
	}
	// Local game, set the mission descriptor to the level name.
	else
	{
		m_MissionName.SetFont(g_CurrentLayout.m_sTitleFont);
		if (g_pGameClientShell->IsRunningPerformanceTest())
		{
			m_MissionName.SetString(LoadString( "IDS_TITLE_PERFORMANCE_TEST" ));
			m_LevelName.SetString(L"");
		}
		else
		{
			m_MissionName.SetString(LoadString( "IDS_CUSTOM_LEVEL" ));
			// Split the worldname up into parts so we can get the load string.
			wchar_t szWorldTitle[MAX_PATH] = L"";
			_wsplitpath( MPA2W(g_pMissionMgr->GetCurrentWorldName( )).c_str(), NULL, NULL, szWorldTitle, NULL );
			m_LevelName.SetString(szWorldTitle);
		}
	}
}

//update server information
void CLoadingScreen::UpdateServerInfo()
{
	if (IsMultiplayerGameClient())
	{
		GameModeMgr& gameModeMgr = GameModeMgr::Instance();

		// Show a briefing for the game mode if there is one...
		if (gameModeMgr.m_ServerSettings.m_sBriefingOverrideMessage.empty( ))
		{
			m_Briefing.SetString(gameModeMgr.m_grwsBriefingStringId );
		}
		else
		{
			m_Briefing.SetString(gameModeMgr.m_ServerSettings.m_sBriefingOverrideMessage.c_str());
		}

		m_ServerMsg.SetString(gameModeMgr.m_ServerSettings.m_sServerMessage.c_str() );

		m_BriefingHeader.Show(true);
		m_ServerMsgHeader.Show(true);

		m_layout = g_pLayoutDB->GetLoadScreenRecord( "DefaultMPBriefing" );
		g_CurrentLayout.Read(m_layout);

		if (m_eCurState != STATE_NONE)
		{
			UpdateLayout();
		}

	}

	m_bHaveServerInfo = true;
}

void CLoadingScreen::ReadMissionInfo(HRECORD hMission)
{
	uint32 nCurLevel = g_pMissionMgr->GetCurrentLevel( );
	HRECORD hLevel = g_pMissionDB->GetLevel(hMission,nCurLevel);


	if (IsMultiplayerGameClient())
	{
		m_photo = g_pMissionDB->GetString(hLevel,MDB_Photo);
	}
	else
	{
		// Show a briefing for this level if it exists.  Also, only
		// show the briefing if we haven't been to this level before.
		const char* szBriefingId = g_pMissionDB->GetString(hLevel,MDB_Briefing);
		if( (!LTStrIEquals(szBriefingId,"<none>")) && !g_pMissionMgr->IsRestoringLevel( ))
		{
			m_Briefing.SetString(LoadString( szBriefingId ));
			char const* pszLoadScreen = g_pMissionDB->GetString( hMission, MDB_BriefingLayout );
			m_layout = g_pLayoutDB->GetLoadScreenRecord( pszLoadScreen );
		}
		else
		{
			m_Briefing.SetString(L"");
			char const* pszLoadScreen = g_pMissionDB->GetString( hMission, MDB_Layout );
			m_layout = g_pLayoutDB->GetLoadScreenRecord( pszLoadScreen );
		}

		m_ServerMsg.SetString(L"");

		g_CurrentLayout.Read(m_layout);

	}

	if (m_eCurState != STATE_NONE)
	{
		UpdateLayout();
	}


	wchar_t wszMissionDiplayName[MAX_PATH];
	g_pMissionDB->GetMissionDisplayName( hMission, hLevel, wszMissionDiplayName, LTARRAYSIZE( wszMissionDiplayName ));

	m_MissionName.SetFont(g_CurrentLayout.m_sTitleFont);
	m_MissionName.SetString( wszMissionDiplayName );
	m_LevelName.SetString(LoadString( g_pMissionDB->GetString(hLevel,MDB_Name)));

	// Show help text for this level if it exists.
	const char* szHelpId = g_pMissionDB->GetString(hLevel,MDB_Help);
	if( szHelpId[0] != '\0' )
	{
		m_Help.SetString(CreateHelpString( szHelpId ));
	}
	else
	{
		m_Help.SetString(L"");
	}

}

void CLoadingScreen::LoadingProgressCallback(ELoadingProgressTask eTask, float fProgress, void* pUser)
{
	// No need to update content info if a different screen is being rendered...
	if( m_pRenderScreen )
		return;

	EnterCriticalSection(&m_MissionUpdate);

	static float fWeights[eLoadingProgressTask_NumTasks] =
	{
		0.05f, //eLoadingProgressTask_WorldBsp,
		0.15f, //eLoadingProgressTask_Objects,
		0.80f, //eLoadingProgressTask_Assets,
		0.00f, //eLoadingProgressTask_User0,
		0.00f, //eLoadingProgressTask_User1,
		0.00f, //eLoadingProgressTask_User2,
		0.00f, //eLoadingProgressTask_User3,
		0.00f, //eLoadingProgressTask_User4,
		0.00f, //eLoadingProgressTask_User5,
		0.00f, //eLoadingProgressTask_User6,
		0.00f, //eLoadingProgressTask_User7,

	};

	float fCurrWt = fWeights[eTask] * fProgress;
	UpdateProgressBar(m_fLoadProgress + fCurrWt);
	if (fProgress == 1.0f)
	{
		m_fLoadProgress += fWeights[eTask];
	}

/*
	DebugCPrint(0,"%s progress = %0.2f",__FUNCTION__,m_fLoadProgress);

	switch(eTask)
	{
	case eLoadingProgressTask_WorldBsp:
		DebugCPrint(0,"%s (eLoadingProgressTask_WorldBsp) %0.2f : %0.2f",__FUNCTION__,fProgress,  (clock() / (float)CLOCKS_PER_SEC));
		break;
	case eLoadingProgressTask_Objects:
		DebugCPrint(0,"%s (eLoadingProgressTask_Objects) %0.2f : %0.2f",__FUNCTION__,fProgress,(clock() / (float)CLOCKS_PER_SEC));
		break;
	case eLoadingProgressTask_Assets:
		DebugCPrint(0,"%s (eLoadingProgressTask_Assets) %0.2f : %0.2f",__FUNCTION__,fProgress,(clock() / (float)CLOCKS_PER_SEC));
		break;
	default:
		if (eTask >= eLoadingProgressTask_User0 && eTask < eLoadingProgressTask_User7)
		{
			DebugCPrint(0,"%s (eLoadingProgressTask_User) %0.2f : %0.2f",__FUNCTION__,fProgress,(clock() / (float)CLOCKS_PER_SEC));
		}
		break;
	}
*/

	LeaveCriticalSection(&m_MissionUpdate);

	Update();

	// let GameClientShell perform any progress related processing
	g_pGameClientShell->OnWorldLoadingProgress();
}

void CLoadingScreen::UpdateLayout()
{
	float w = float(g_pInterfaceResMgr->GetScreenWidth());
	float h = float(g_pInterfaceResMgr->GetScreenHeight());

	//*******************************************************************************
	// Build Mission Title String

	m_MissionName.SetFont(g_CurrentLayout.m_sTitleFont);
	m_MissionName.SetBasePos(g_CurrentLayout.m_TitlePos);
	m_MissionName.SetColor(g_CurrentLayout.m_cTitleColor);
	m_MissionName.SetScale(g_pInterfaceResMgr->GetScreenScale());
	m_MissionName.SetDropShadow(1);

	//*******************************************************************************
	// Build Mission Level String
	m_LevelName.SetFont(g_CurrentLayout.m_sLevelFont);
	m_LevelName.SetBasePos(g_CurrentLayout.m_LevelPos);
	m_LevelName.SetColor(g_CurrentLayout.m_cLevelColor);
	m_LevelName.SetScale(g_pInterfaceResMgr->GetScreenScale());
	m_LevelName.SetDropShadow(1);


	//*******************************************************************************
	// Build Mission Briefing String

	LTRect2n rTemp = g_CurrentLayout.m_BriefingRect;
	if (IsMultiplayerGameClient())
	{
		rTemp.Top() += (g_CurrentLayout.m_sBriefingFont.m_nHeight * 3 / 2);
	}

	m_Briefing.SetFont(g_CurrentLayout.m_sBriefingFont);
	m_Briefing.SetBasePos(rTemp.m_vMin);
	m_Briefing.SetColor(g_CurrentLayout.m_cBriefingColor);
	m_Briefing.SetScale(g_pInterfaceResMgr->GetScreenScale());
	m_Briefing.SetDropShadow(1);

	LTVector2n sz = g_CurrentLayout.m_BriefingRect.m_vMax - rTemp.m_vMin;
	m_Briefing.SetSize( sz );
	m_Briefing.SetWordWrap(true);

	m_BriefingHeader.SetFont(g_CurrentLayout.m_sBriefingHeaderFont);
	m_BriefingHeader.SetBasePos(g_CurrentLayout.m_BriefingRect.m_vMin);
	m_BriefingHeader.SetColor(g_CurrentLayout.m_cBriefingColor);
	m_BriefingHeader.SetScale(g_pInterfaceResMgr->GetScreenScale());
	m_BriefingHeader.SetDropShadow(1);

	LTVector2n headerSz(g_CurrentLayout.m_BriefingRect.GetWidth(),g_CurrentLayout.m_sBriefingFont.m_nHeight);
	m_BriefingHeader.SetSize( sz );



	//*******************************************************************************
	// Build Server Message String

	rTemp = g_CurrentLayout.m_ServerMsgRect;
	rTemp.Top() += (g_CurrentLayout.m_sServerMsgFont.m_nHeight * 3 / 2);

	m_ServerMsg.SetFont(g_CurrentLayout.m_sServerMsgFont);
	m_ServerMsg.SetBasePos(rTemp.m_vMin);
	m_ServerMsg.SetColor(g_CurrentLayout.m_cServerMsgColor);
	m_ServerMsg.SetScale(g_pInterfaceResMgr->GetScreenScale());
	m_ServerMsg.SetDropShadow(1);
	m_ServerMsg.SetClipping(true);

	sz = g_CurrentLayout.m_ServerMsgRect.m_vMax - rTemp.m_vMin;
	m_ServerMsg.SetSize( sz );
	m_ServerMsg.SetWordWrap(true);

	m_ServerMsgHeader.SetFont(g_CurrentLayout.m_sServerMsgHeaderFont);
	m_ServerMsgHeader.SetBasePos(g_CurrentLayout.m_ServerMsgRect.m_vMin);
	m_ServerMsgHeader.SetColor(g_CurrentLayout.m_cServerMsgColor);
	m_ServerMsgHeader.SetScale(g_pInterfaceResMgr->GetScreenScale());
	m_ServerMsgHeader.SetDropShadow(1);

	headerSz = LTVector2n(g_CurrentLayout.m_ServerMsgRect.GetWidth(),g_CurrentLayout.m_sServerMsgFont.m_nHeight);
	m_ServerMsgHeader.SetSize( sz );

	//*******************************************************************************
	// Build Mission Help String

	m_Help.SetFont(g_CurrentLayout.m_sHelpFont);
	m_Help.SetBasePos(g_CurrentLayout.m_HelpRect.m_vMin);
	m_Help.SetColor(g_CurrentLayout.m_cHelpColor);
	m_Help.SetWordWrap(true);
	sz = g_CurrentLayout.m_HelpRect.m_vMax - g_CurrentLayout.m_HelpRect.m_vMin;
	m_Help.SetSize(sz);
	m_Help.SetScale(g_pInterfaceResMgr->GetScreenScale());
	m_Help.SetDropShadow(1);

	//m_Help.Show(pProfile->m_bLoadScreenTips);

	m_hBackTexture.Load(g_CurrentLayout.m_sBackTexture.c_str());

	LTVector2 vHalfTexel(0.5f / (float)w, 0.5f / (float)h);
	SetupQuadUVs(m_BackPoly, m_hBackTexture, -vHalfTexel.x, -vHalfTexel.y, 1.0f, 1.0f );
	DrawPrimSetRGBA(m_BackPoly,argbWhite);
	//	float offset = (w - float(g_pInterfaceResMgr->GetScreenHeight())) / 2.0f;
	//	DrawPrimSetXYWH(m_BackPoly,0.0f,-offset,w,w);
	DrawPrimSetXYWH(m_BackPoly,-0.5f,-0.5f,w,h);



	uint8 nA;
	GET_ARGB(g_CurrentLayout.m_cContinueColor,nA,nR,nG,nB);

	if( IsMultiplayerGameClient() && !m_photo.empty() )
	{
		m_hPhoto.Load(m_photo.c_str());
		if (m_hPhoto)
		{
			SetupQuadUVs(m_photoPoly, m_hPhoto, 0.0f, 0.0f, 1.0f, 1.0f);
			DrawPrimSetRGBA(m_photoPoly,argbWhite);

			float fScale = g_pInterfaceResMgr->GetXRatio();
			float fx = (float)g_CurrentLayout.m_PhotoRect.Left() * fScale;
			float fy = (float)g_CurrentLayout.m_PhotoRect.Top() * fScale;

			float fw = (float)(g_CurrentLayout.m_PhotoRect.GetWidth()) * fScale;
			float fh = (float)(g_CurrentLayout.m_PhotoRect.GetHeight()) * fScale;

			DrawPrimSetXYWH(m_photoPoly,fx,fy,fw,fh);

		}
	}
	else
	{
		m_hPhoto = NULL;
	}

	m_CurrentFileName.SetFont(g_CurrentLayout.m_sContentFont);
	m_CurrentFileName.SetBasePos(g_CurrentLayout.m_CurrentFileNamePos);
	m_CurrentFileName.SetColor(g_CurrentLayout.m_cContentColor);
	m_CurrentFileName.SetScale(g_pInterfaceResMgr->GetScreenScale());

	m_CurrentFileTime.SetFont(g_CurrentLayout.m_sContentFont);
	m_CurrentFileTime.SetBasePos(g_CurrentLayout.m_CurrentFileTimePos);
	m_CurrentFileTime.SetColor(g_CurrentLayout.m_cContentColor);
	m_CurrentFileTime.SetScale(g_pInterfaceResMgr->GetScreenScale());

	m_FilesLeft.SetFont(g_CurrentLayout.m_sContentFont);
	m_FilesLeft.SetBasePos(g_CurrentLayout.m_FilesLeftPos);
	m_FilesLeft.SetColor(g_CurrentLayout.m_cContentColor);
	m_FilesLeft.SetScale(g_pInterfaceResMgr->GetScreenScale());

	m_TotalTime.SetFont(g_CurrentLayout.m_sContentFont);
	m_TotalTime.SetBasePos(g_CurrentLayout.m_TotalTimePos);
	m_TotalTime.SetColor(g_CurrentLayout.m_cContentColor);
	m_TotalTime.SetScale(g_pInterfaceResMgr->GetScreenScale());

	m_LoadProgress.Init(g_CurrentLayout.m_hLoadProgressTexture);
	m_CurrentFile.Init(g_CurrentLayout.m_hContentTexture);
	m_Total.Init(g_CurrentLayout.m_hContentTexture);


	m_rfLoadProgress.Init(	g_CurrentLayout.m_LoadProgressRect.Left() * g_pInterfaceResMgr->GetXRatio(),
		g_CurrentLayout.m_LoadProgressRect.Top() * g_pInterfaceResMgr->GetYRatio(),
		g_CurrentLayout.m_LoadProgressRect.Right() * g_pInterfaceResMgr->GetXRatio(),
		g_CurrentLayout.m_LoadProgressRect.Bottom() * g_pInterfaceResMgr->GetYRatio() );

	m_rfCurrentFile.Init(	g_CurrentLayout.m_CurrentFileRect.Left() * g_pInterfaceResMgr->GetXRatio(),
		g_CurrentLayout.m_CurrentFileRect.Top() * g_pInterfaceResMgr->GetYRatio(),
		g_CurrentLayout.m_CurrentFileRect.Right() * g_pInterfaceResMgr->GetXRatio(),
		g_CurrentLayout.m_CurrentFileRect.Bottom() * g_pInterfaceResMgr->GetYRatio() );

	m_rfTotal.Init(	g_CurrentLayout.m_TotalRect.Left() * g_pInterfaceResMgr->GetXRatio(),
		g_CurrentLayout.m_TotalRect.Top() * g_pInterfaceResMgr->GetYRatio(),
		g_CurrentLayout.m_TotalRect.Right() * g_pInterfaceResMgr->GetXRatio(),
		g_CurrentLayout.m_TotalRect.Bottom() * g_pInterfaceResMgr->GetYRatio() );

	if (g_CurrentLayout.m_hContentFrameTexture)
	{
		CLTGUICtrl_create cs;
		cs.rnBaseRect = g_CurrentLayout.m_ContentFrameRect;
		m_ContentFrame.Create(g_CurrentLayout.m_hContentFrameTexture,cs);
		m_ContentFrame.SetScale(g_pInterfaceResMgr->GetScreenScale());
	}

};
