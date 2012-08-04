// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMain.cpp
//
// PURPOSE : Top level interface screen
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenMain.h"
#include "ScreenMgr.h"
#include "LayoutMgr.h"
#include "ScreenCommands.h"
#include "WinUtil.h"
#include "GameClientShell.h"
#include "ModelButeMgr.h"
#include "ClientSaveLoadMgr.h"
#include "MissionMgr.h"
#include "ClientMultiplayerMgr.h"
#include "TronScreenMgr.h"

namespace
{
	void QuitCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenMain *pThisScreen = (CScreenMain *)pData;
		if (bReturn && pThisScreen)
			pThisScreen->SendCommand(CMD_EXIT,0,0);
	};
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenMain::CScreenMain()
{
    m_pResume = LTNULL;
}

CScreenMain::~CScreenMain()
{
}


// Build the screen
LTBOOL CScreenMain::Build()
{
#ifdef _TRON_E3_DEMO
	BuildCustomLevelsList();

	// Set a console variable based on m_bBackDemo
	WriteConsoleInt("E3BackroomDemo", m_bBackDemo ? 1 : 0);

	if (m_bBackDemo)
	{
		int index = 0;
		StringSet::iterator iter = m_Filenames.begin();
		while (iter != m_Filenames.end())
		{
			AddTextItem((char*)iter->c_str(),CMD_CUSTOM+index,LTNULL);
			++index;
			iter++;
		}
	}
	else
	{
		AddTextItem(IDS_TRON_E3_DEMO, CMD_CUSTOM, IDS_HELP_SINGLEPLAYER);
	}
#else

	char szTmp[1024];
	FormatString(IDS_SINGLEPLAYER,szTmp,sizeof(szTmp));

	AddTextItem(szTmp, CMD_SINGLE_PLAYER, IDS_HELP_SINGLEPLAYER);

	m_pResume = AddTextItem(IDS_CONTINUE_GAME, CMD_CONTINUE_GAME, IDS_HELP_CONTINUE_GAME);
	m_pResume->Enable(LTFALSE);

	AddTextItem(IDS_MULTIPLAYER, CMD_MULTI_PLAYER, IDS_HELP_MULTIPLAYER);
	AddTextItem(IDS_OPTIONS, CMD_OPTIONS, IDS_HELP_OPTIONS);

	CLTGUITextCtrl* pCtrl = AddTextItem(IDS_PROFILE, CMD_PROFILE, IDS_HELP_PROFILE);

	AddTextItem(IDS_EXIT, CMD_QUIT, IDS_HELP_EXIT, s_BackPos);
#endif

	LTIntPt pos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"VersionPos");
	uint8 nFont = g_pLayoutMgr->GetHelpFont();
	char *pStr = (char *)g_pVersionMgr->GetVersion();
	pCtrl= AddTextItem(pStr,LTNULL,LTNULL,pos,LTTRUE,nFont);
	pCtrl->SetFont(LTNULL,g_pLayoutMgr->GetHelpSize());
	pos.x -= pCtrl->GetWidth();
	pCtrl->SetBasePos(pos);

	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTFALSE);

	return LTTRUE;
}

void CScreenMain::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		((CTronScreenMgr*)m_pScreenMgr)->PlayMenuMusic();

		SetSelection(-1);

		// Make sure we're always disconnected when we get to this screen.  You must abort to get here.
		if(g_pLTClient->IsConnected())
		{
			g_pInterfaceMgr->StartingNewGame( );
			g_pLTClient->Disconnect();
		}

		// Setup continue game selection.
			m_pResume->SetHelpID(IDS_HELP_CONTINUE_GAME);
			m_pResume->CLTGUICtrl::Create(CMD_CONTINUE_GAME,0,0);
		m_pResume->Enable( g_pClientSaveLoadMgr->CanContinueGame() );
		m_pResume->SetString( LoadTempString( IDS_CONTINUE_GAME ));

		// Always assume sp save/load when in the main screen.  This is so the 
		// "continue game" assumes sp.
		g_pClientSaveLoadMgr->SetUseMultiplayerFolders( false );

	}

	CBaseScreen::OnFocus(bFocus);
}


uint32 CScreenMain::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
#ifdef _TRON_E3_DEMO
	if (dwCommand >= CMD_CUSTOM)
	{
		int index = dwCommand - CMD_CUSTOM;
		StringSet::iterator iter = m_Filenames.begin();
		while (iter != m_Filenames.end() && index > 0)
		{
			iter++;
			--index;
		}
		if (iter != m_Filenames.end())
		{
			if( g_pClientMultiplayerMgr->SetupServerSinglePlayer( ))
				g_pMissionMgr->StartGameFromLevel(iter->c_str());
		}
		return 1;
	}
#endif
	switch(dwCommand)
	{
	case CMD_SINGLE_PLAYER:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_SINGLE);
			break;
		}
	case CMD_MULTI_PLAYER:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_MULTI);
			break;

		}
	case CMD_OPTIONS:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_OPTIONS);
			break;
		}
	case CMD_PROFILE:
		{
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_PROFILE);
			break;
		}
/*
	case CMD_SUBROUTINES:
		{
			// Temporarily go to the SubroutineMgr to populate the screen.
			// Remember to remove the #include when you take this out.
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_SUBROUTINES);
			g_pSubroutineMgr->PopulateSubroutineScreen();
			break;
		}

	case CMD_RATINGS:
		{
			// Temporarily go to the SubroutineMgr to populate the screen.
			// Remember to remove the #include when you take this out.
			m_pScreenMgr->SetCurrentScreen(SCREEN_ID_RATINGS);
			g_pSubroutineMgr->PopulateSubroutineScreen();
			break;
		}
*/
	case CMD_QUIT:
		{

			MBCreate mb;
			mb.eType = LTMB_YESNO;
			mb.pFn = QuitCallBack;
			mb.pData = this;
			g_pInterfaceMgr->ShowMessageBox(IDS_SUREWANTQUIT,&mb);
			break;
		}
	case CMD_EXIT:
		{
#if(defined(_DEMO) || defined(_TRON_E3_DEMO))
			g_pInterfaceMgr->ShowDemoScreens(LTTRUE);
#else
            g_pLTClient->Shutdown();
#endif
			break;
		}
	case CMD_RESUME:
		{
			Escape();
			break;
		}
	case CMD_CONTINUE_GAME:
		{
			// Initialize to the sp mission bute.
			if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
			{
				g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
				return 0;
			}

			if( !g_pClientMultiplayerMgr->SetupServerSinglePlayer( ))
				return 0;

			g_pGameClientShell->SetGameType( eGameTypeSingle );

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
LTBOOL   CScreenMain::Render(HSURFACE hDestSurf)
{
	if (CBaseScreen::Render(hDestSurf))
	{
		//render build str
        return LTTRUE;
	}

    return LTFALSE;

}

#ifdef _TRON_E3_DEMO
// Build the list of Custom Levels
void CScreenMain::BuildCustomLevelsList()
{
	m_Filenames.clear();
	m_bBackDemo = false;

	// Get a list of world names and sort them alphabetically

    char pathBuf[128] = "worlds\\TronE3Demo";
	char Buf[255];

	FileEntry* pFilesArray = debug_new(FileEntry*);

	if (pFilesArray)
	{
        pFilesArray = g_pLTClient->GetFileList(pathBuf);
		sprintf(Buf, "%s\\", pathBuf);
		AddFilesToFilenames(pFilesArray, Buf);
        g_pLTClient->FreeFileList(pFilesArray);
		debug_delete(pFilesArray);
	}
	if (m_Filenames.size() > 1)
		m_bBackDemo = true;
}

void CScreenMain::AddFilesToFilenames(FileEntry* pFiles, char* pPath)
{
	if (!pFiles || !pPath) return;

	char strBaseName[256];
	char* pBaseName = NULL;
	char* pBaseExt = NULL;
	FileEntry* ptr = pFiles;

	while (ptr)
	{
		if (ptr->m_Type == TYPE_FILE)
		{
			SAFE_STRCPY(strBaseName, ptr->m_pBaseFilename);
			pBaseName = strtok (strBaseName, ".");
			pBaseExt = strtok (NULL, "\0");
			if (pBaseExt && stricmp (pBaseExt, "dat") == 0)
			{
				// add this to the array
				m_Filenames.insert(pBaseName);
			}
		}

		ptr = ptr->m_pNext;
	}
}
#endif