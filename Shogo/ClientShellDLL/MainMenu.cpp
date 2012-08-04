#include "clientheaders.h"
#include "MainMenu.h"
#include "TextHelper.h"
#include "RiotClientShell.h"
#include "ClientRes.h"
#include "RiotMenu.h"
#include "NetStart.h"

extern void ExitGame (LTBOOL, uint32);

#define DEMO_VERSION	"DEMO VERSION 2.1"

#define PATCH_VERSION	"v 2.2.15"

void EndGameAndDoMultiplayer (LTBOOL bYes, uint32 nUserData)
{
	if (bYes)
	{
		CMainMenu* pMainMenu = (CMainMenu*) nUserData;
		if (!pMainMenu) return;

		if (pMainMenu->DoMultiplayer (LTTRUE))
		{
			pMainMenu->Esc();
		}
	}
}

CMainMenu::CMainMenu() : CBaseMenu()
{
	m_fVersionDisplayTimeLeft = 0.0f;
	m_hVersion = LTNULL;
	m_bFirstDraw = LTTRUE;
}

LTBOOL CMainMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!CBaseMenu::Init (pClientDE, pRiotMenu, pParent, nScreenWidth, nScreenHeight)) return LTFALSE;

	if (!m_SinglePlayerMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;
	if (!m_OptionsMenu.Init (pClientDE, pRiotMenu, this, nScreenWidth, nScreenHeight)) return LTFALSE;

	if (!LoadSurfaces()) return LTFALSE;

	return LTTRUE;
}

void CMainMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	m_SinglePlayerMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);
	m_OptionsMenu.ScreenDimsChanged (nScreenWidth, nScreenHeight);

	CBaseMenu::ScreenDimsChanged (nScreenWidth, nScreenHeight);
}

void CMainMenu::Reset()
{
	if (!m_pClientDE) return;
}

void CMainMenu::Return()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	if (!pClientShell) return;
	
	if (m_nSelection != 4 && m_hVersion)
	{
		m_pClientDE->DeleteSurface (m_hVersion);
		m_hVersion = LTNULL;
		m_fVersionDisplayTimeLeft = 0.0f;
	}

	if (m_nSelection == 0)
	{
		m_SinglePlayerMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_SinglePlayerMenu);
	}
	else if (m_nSelection == 1)
	{

//#ifdef _DEMO
//		pClientShell->SetGameState(GS_DEMO_MULTIPLAYER);
//#else
		if (m_pRiotMenu->InWorld())
		{
			pClientShell->DoYesNoMessageBox (IDS_ENDCURRENTGAME, EndGameAndDoMultiplayer, (uint32)this, TH_ALIGN_CENTER);
		}
		else
		{
			if (DoMultiplayer(LTTRUE))
			{
				m_pRiotMenu->ExitMenu (LTTRUE);
			}
		}
//#endif

	}
	else if (m_nSelection == 2)
	{
		m_OptionsMenu.Reset();
		m_pRiotMenu->SetCurrentMenu (&m_OptionsMenu);
	}
	else if (m_nSelection == 3)
	{
		pClientShell->SetGameState (GS_CREDITS);
	}
	else if (m_nSelection == 4)
	{
		pClientShell->DoYesNoMessageBox (IDS_SUREWANTQUIT, ExitGame, 0, TH_ALIGN_CENTER);
	}
		
	CBaseMenu::Return();
}

void CMainMenu::Esc()
{
	if (!m_pRiotMenu || !m_pRiotMenu->GetClientShell()) return;

	if (!m_pRiotMenu->GetClientShell()->IsInWorld()) return;

	m_pRiotMenu->ExitMenu();

	CBaseMenu::Esc();
}

void CMainMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	if (!m_pClientDE) return;

	CBaseMenu::Draw (hScreen, nScreenWidth, nScreenHeight, nTextOffset);

#ifdef _DEMO
	if (!m_hVersion)
	{
		m_hVersion = CTextHelper::CreateSurfaceFromString(m_pClientDE, m_pRiotMenu->GetFont08n(), DEMO_VERSION);
	}

	m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hVersion, LTNULL, nScreenWidth - m_szVersion.cx, nScreenHeight - m_szVersion.cy, LTNULL);
	return;
#endif

	if (m_bFirstDraw)
	{
		m_bFirstDraw = LTFALSE;
		m_fVersionDisplayTimeLeft = 5.0f;
	}

	LTFLOAT fFrameTime = m_pClientDE->GetFrameTime();
	if (fFrameTime > 3.0f) fFrameTime = 0.0f;	// allow for first update/huge pauses

	if (m_fVersionDisplayTimeLeft > 0.0f && m_hVersion)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hVersion, LTNULL, nScreenWidth - m_szVersion.cx, nScreenHeight - m_szVersion.cy, LTNULL);
		m_fVersionDisplayTimeLeft -= fFrameTime;
	}
	else if (m_hVersion)
	{
		m_fVersionDisplayTimeLeft = 0.0f;
		m_pClientDE->DeleteSurface (m_hVersion);
		m_hVersion = LTNULL;
	}
}

LTBOOL CMainMenu::DoMultiplayer(LTBOOL bMinimize)
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;
		
	CRiotClientShell* pClientShell = m_pRiotMenu->GetClientShell();
	if (!pClientShell) return LTFALSE;

	if (bMinimize)
	{
		NetStart_MinimizeMainWnd(m_pClientDE);
		pClientShell->MainWindowMinimized();
	}

	NetStart ns;
	LTBOOL bRet = NetStart_DoWizard(m_pClientDE, &ns);

	if (bRet)
	{
		StartGameRequest req;
		memset(&req, 0, sizeof(req));

		if (ns.m_bHost)
		{
			req.m_Type = STARTGAME_HOST;

			SAFE_STRCPY(req.m_WorldName, ns.m_sLevel);
			memcpy(&req.m_HostInfo, &ns.m_NetHost, sizeof(NetHost));
		}
		else
		{
			req.m_Type        = STARTGAME_CLIENT;
			req.m_pNetSession = ns.m_pNetSession;

			if (ns.m_bHaveTcpIp)
			{
				req.m_Type = STARTGAME_CLIENTTCP;
				SAFE_STRCPY(req.m_TCPAddress, ns.m_sAddress);
			}
		}

		req.m_pGameInfo   = NetStart_GetGameStruct();
		req.m_GameInfoLen = sizeof(NetGame_t);

		uint32 cxLoading, cyLoading, cxScreen, cyScreen;
		HSURFACE hLoading = m_pClientDE->CreateSurfaceFromBitmap ("interface/loading.pcx");
		m_pClientDE->GetSurfaceDims (hLoading, &cxLoading, &cyLoading);
		HSURFACE hScreen = m_pClientDE->GetScreenSurface();
		m_pClientDE->GetSurfaceDims (hScreen, &cxScreen, &cyScreen);

		m_pClientDE->ClearScreen (LTNULL, CLEARSCREEN_SCREEN);
		m_pClientDE->Start3D();
		m_pClientDE->StartOptimized2D();
		m_pClientDE->DrawSurfaceToSurface (hScreen, hLoading, LTNULL, ((int)cxScreen - (int)cxLoading) / 2, ((int)cyScreen - (int)cyLoading) / 2);
		m_pClientDE->EndOptimized2D();
		m_pClientDE->End3D(END3D_CANDRAWCONSOLE);
		m_pClientDE->FlipScreen (0);

		m_pClientDE->DeleteSurface (hLoading);
		
		pClientShell->SetGameState (GS_LOADINGLEVEL);
		pClientShell->ZeroClearScreenCount();
		
		LTRESULT dr = m_pClientDE->StartGame(&req);

		NetStart_FreeSessionList(m_pClientDE);

		if (bMinimize && pClientShell->IsMainWindowMinimized())	// StartGame() may have caused PreLoadWorld() to be called, which will restore the window
		{
			NetStart_RestoreMainWnd();
			pClientShell->MainWindowRestored();
		}

		if (dr != LT_OK)
		{
			if (pClientShell)
			{
				pClientShell->SetGameState (GS_MENU);
				pClientShell->DoMessageBox (IDS_NOLOADLEVEL, TH_ALIGN_CENTER);
			}
			return(LTFALSE);
		}
	}
	else
	{
		NetStart_FreeSessionList(m_pClientDE);

		if (bMinimize)
		{
			NetStart_RestoreMainWnd();
			pClientShell->MainWindowRestored();
		}

		return(LTFALSE);
	}

	NetStart_RunServerOptions(m_pClientDE);


	// All done...

	return(LTTRUE);
}

LTBOOL CMainMenu::LoadSurfaces()
{
	if (!m_pClientDE || !m_pRiotMenu) return LTFALSE;

	CBitmapFont* pFontNormal = LTNULL;
	CBitmapFont* pFontSelected = LTNULL;
	if (m_szScreen.cx < 512)
	{
		pFontNormal = m_pRiotMenu->GetFont12n();
		pFontSelected = m_pRiotMenu->GetFont12s();
	}
	else if (m_szScreen.cx < 640)
	{
		pFontNormal = m_pRiotMenu->GetFont18n();
		pFontSelected = m_pRiotMenu->GetFont18s();
	}
	else
	{
		pFontNormal = m_pRiotMenu->GetFont28n();
		pFontSelected = m_pRiotMenu->GetFont28s();
	}

	int nCreditsId = IDS_CREDITS;
#ifdef _DEMO
	nCreditsId = IDS_DEMOINFO;
#endif

	m_GenericItem[0].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_SINGLEPLAYER, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[1].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_MULTIPLAYER, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[2].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_OPTIONS, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[3].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, nCreditsId, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[4].hMenuItem = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_EXIT, IDS_MENUREPLACEMENTFONT);
	
	m_GenericItem[0].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_SINGLEPLAYER, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[1].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_MULTIPLAYER, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[2].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_OPTIONS, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[3].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, nCreditsId, IDS_MENUREPLACEMENTFONT);
	m_GenericItem[4].hMenuItemSelected = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontSelected, IDS_EXIT, IDS_MENUREPLACEMENTFONT);
	
	m_hMenuTitle = CTextHelper::CreateSurfaceFromString (m_pClientDE, pFontNormal, IDS_TITLE_MAINMENU, IDS_MENUREPLACEMENTFONT);
	m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);

#ifdef _DEMO
	HSTRING hstrVersion = m_pClientDE->CreateString(DEMO_VERSION);
	m_hVersion = CTextHelper::CreateSurfaceFromString (m_pClientDE, m_pRiotMenu->GetFont08n(), m_pClientDE->GetStringData (hstrVersion));
	m_pClientDE->GetSurfaceDims (m_hVersion, &m_szVersion.cx, &m_szVersion.cy);
	m_pClientDE->FreeString (hstrVersion);
#else
//	HSTRING hstrVersion = m_pClientDE->FormatString(0);
	// now since it is possible for the version to be present in the chsell and be wrong we define it in the code
	// then we search the end of the old cshell version and get the possible foreign language specific part 
	HSTRING hstrOldVersion = m_pClientDE->FormatString(0);
	char* sLanguageSuffix = NULL;
	if (hstrOldVersion != NULL)
	{
		sLanguageSuffix = const_cast<char *>(m_pClientDE->GetStringData(hstrOldVersion));
		// skip initial v
		while ((sLanguageSuffix[0] !='\0') && ((sLanguageSuffix[0] == 'v') || (sLanguageSuffix[0] == 'V'))) sLanguageSuffix++;
		// skip space
		while ((sLanguageSuffix[0] !='\0') && (sLanguageSuffix[0] == ' ')) sLanguageSuffix++;
		// skip digits
		while ((sLanguageSuffix[0] !='\0') && ((sLanguageSuffix[0] >= '0') && (sLanguageSuffix[0] <= '9'))) sLanguageSuffix++;
		// skip .
		while ((sLanguageSuffix[0] !='\0') && (sLanguageSuffix[0] == '.')) sLanguageSuffix++;
		// skip more digits
		while ((sLanguageSuffix[0] !='\0') && ((sLanguageSuffix[0] >= '0') && (sLanguageSuffix[0] <= '9'))) sLanguageSuffix++;
	}
	char sNewVersion[256];
	strncpy(sNewVersion, PATCH_VERSION, 256);
	sNewVersion[255] = '\0';
	if (sLanguageSuffix != NULL) 
	{
		strncat(sNewVersion, sLanguageSuffix, 256);
		sNewVersion[255] = '\0';
	}
	m_hVersion = CTextHelper::CreateSurfaceFromString (m_pClientDE, m_pRiotMenu->GetFont08n(), sNewVersion);
	m_pClientDE->GetSurfaceDims (m_hVersion, &m_szVersion.cx, &m_szVersion.cy);
	if (hstrOldVersion != NULL) m_pClientDE->FreeString (hstrOldVersion);
#endif


	for (int i = 0; i < 5; i++)
	{
		if (!m_GenericItem[i].hMenuItem || !m_GenericItem[i].hMenuItemSelected)
		{
			UnloadSurfaces();
			return LTFALSE;
		}
	}

 	for (int i = 0; i < 5; i++)
	{
		m_pClientDE->GetSurfaceDims(m_GenericItem[i].hMenuItem, &m_GenericItem[i].szMenuItem.cx, &m_GenericItem[i].szMenuItem.cy);
	}
	
	return CBaseMenu::LoadSurfaces();
}

void CMainMenu::UnloadSurfaces()
{
	if (!m_pClientDE) return;

	for (int i = 0; i < 5; i++)
	{
		if (m_GenericItem[i].hMenuItem) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItem);
		if (m_GenericItem[i].hMenuItemSelected) m_pClientDE->DeleteSurface (m_GenericItem[i].hMenuItemSelected);
		m_GenericItem[i].hMenuItem = LTNULL;
		m_GenericItem[i].hMenuItemSelected = LTNULL;
		m_GenericItem[i].szMenuItem.cx = m_GenericItem[i].szMenuItem.cy = 0;
	}

	if (m_hMenuTitle) m_pClientDE->DeleteSurface (m_hMenuTitle);
	m_hMenuTitle = LTNULL;

	CBaseMenu::UnloadSurfaces();
}

