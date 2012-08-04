// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSingle.cpp
//
// PURPOSE : Interface screen for starting, loading, and saving single player
//				games.
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenSingle.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "WinUtil.h"
#include "GameClientShell.h"
#include "MissionMgr.h"
#include "ClientSaveLoadMgr.h"
#include "ClientConnectionMgr.h"
#include "resourceextensions.h"
#include "sys/win/mpstrconv.h"
#include "MissionDB.h"
#include "iltfilemgr.h"
#include "ltprofileutils.h"
#include "GameModeMgr.h"

#ifdef _FINAL
#define _REMOVE_CUSTOM_LEVELS
#endif

namespace
{
	int32 kTextWidth = 200;
}



static char* s_apszLevels[] =
{
	"Worlds\\Release\\E3_Demo_2005_short",
};
static uint32 s_nNumLevels = ARRAY_LEN( s_apszLevels );


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenSingle::CScreenSingle() :
	m_pCustomScroll(NULL)
{
    m_pLoadCtrl = NULL;

    m_pDiff = NULL;

    m_pCustom = NULL;
}

CScreenSingle::~CScreenSingle()
{
	m_Filenames.clear();
}

// Build the screen
bool CScreenSingle::Build()
{
	CreateTitle("IDS_TITLE_GAME");

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(kTextWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

	cs.nCommandID = CMD_NEW_GAME;
	cs.szHelpID = "IDS_HELP_NEW";
	AddTextItem("IDS_NEWGAME", cs);

	cs.nCommandID = CMD_LOAD_GAME;
	cs.szHelpID = "IDS_HELP_LOAD";
	m_pLoadCtrl = AddTextItem("IDS_LOADGAME",	cs);
	m_pLoadCtrl->Enable( false );


#ifndef _REMOVE_CUSTOM_LEVELS
	cs.nCommandID = CMD_CUSTOM_LEVEL;
	cs.szHelpID = "IDS_HELP_CUSTOM";
	CLTGUITextCtrl *pCustom = AddTextItem("IDS_CUSTOM_LEVEL", cs );
#endif // _REMOVE_CUSTOM_LEVELS



	CLTGUIListCtrl_create listCs;
	listCs.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,0);
	listCs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,0); 
	m_pDiff = AddList(listCs);
	if (m_pDiff)
	{
		TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
		TextureReference hSelectedFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));

		int32 nWidth = listCs.rnBaseRect.GetWidth() - 16;
		m_pDiff->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
		m_pDiff->SetFrame(hFrame,hSelectedFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));
		m_pDiff->Show(false);


		CLTGUICtrl_create cs;
		cs.rnBaseRect.m_vMin.Init();
		cs.rnBaseRect.m_vMax = LTVector2n(nWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));

		cs.nCommandID = CMD_EASY;
		cs.szHelpID = "IDS_HELP_EASY";
		CLTGUITextCtrl *pCtrl = CreateTextItem("IDS_NEW_EASY", cs);
		m_pDiff->AddControl(pCtrl);

		cs.nCommandID = CMD_MEDIUM;
		cs.szHelpID = "IDS_HELP_MEDIUM";
		pCtrl = CreateTextItem("IDS_NEW_MEDIUM", cs);
		m_pDiff->AddControl(pCtrl);

		cs.nCommandID = CMD_HARD;
		cs.szHelpID = "IDS_HELP_HARD";
		pCtrl = CreateTextItem("IDS_NEW_HARD", cs);
		m_pDiff->AddControl(pCtrl);

		cs.nCommandID = CMD_INSANE;
		cs.szHelpID = "IDS_HELP_INSANE";
		pCtrl = CreateTextItem("IDS_NEW_INSANE", cs);
		m_pDiff->AddControl(pCtrl);
	}



#ifndef _REMOVE_CUSTOM_LEVELS


	{
		CLTGUIScrollBar_create csb;
		csb.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,1);
		csb.rnBaseRect.Right() += g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Left() = csb.rnBaseRect.Right() - g_pLayoutDB->GetScrollBarSize();
		csb.rnBaseRect.Expand( 0, 0 );

		m_pCustomScroll = CreateScrollBar( csb );
		if( m_pCustomScroll )
		{
			m_pCustomScroll->SetFrameWidth( 1 );
			m_pCustomScroll->Enable( true );
			m_pCustomScroll->Show( true );
		}
	}

	CLTGUIListCtrlEx_create slcs;
	slcs.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,1);
	slcs.nTextIdent = g_pLayoutDB->GetListIndent(m_hLayout,1).x;
	slcs.pScrollBar = m_pCustomScroll;
	slcs.pHeaderCtrl = NULL;
	slcs.bAutoSelect = true;
	m_pCustom = AddListEx(slcs);
	if (m_pCustom)
	{
		m_pCustom->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,1));
		m_pCustom->Show(false);
		m_pCustom->SetFrameWidth( 1 );
		m_pCustom->SetScrollWrap( true );

		BuildCustomLevelsList(slcs.rnBaseRect.GetWidth());
	}

	if( m_pCustomScroll )
		AddControl( m_pCustomScroll );

	//if no custom levels, remove the link
	if (pCustom && !m_pCustom || m_pCustom->GetNumControls() < 1)
	{
		RemoveControl(pCustom);
	}
#endif // _REMOVE_CUSTOM_LEVELS



 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return false;

	return true;

}

uint32 CScreenSingle::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if( dwCommand > eGUICtrlCmd_Base )
		return CBaseScreen::OnCommand( dwCommand, dwParam1, dwParam2 );

	if (dwCommand >= CMD_CUSTOM)
	{

		uint32 index = dwCommand - CMD_CUSTOM;

		//is custom level
		if (index < 1000)
		{
			StringSet::iterator iter = m_Filenames.begin();
			while (iter != m_Filenames.end() && index > 0)
			{
				iter++;
				--index;
			}
			if (iter != m_Filenames.end())
			{
				g_pMissionMgr->StartGameFromLevel(iter->c_str());
			}
			
		}
		else
		{
			index -= 1000;
			if (index < g_pMissionDB->GetNumMissions())
			{
				HRECORD hLevel = g_pMissionDB->GetLevel( index, 0 );
				g_pMissionMgr->StartGameFromLevel(g_pMissionDB->GetWorldName(hLevel,true));

			}
			return 1;
		}
	}

	switch(dwCommand)
	{

#if defined(_SPDEMO)
	case CMD_LEVEL1:
	case CMD_LEVEL2:
	case CMD_LEVEL3:
		{

			int nSelectedLevel = dwCommand - CMD_LEVEL1;

			uint32 nMission;
			uint32 nLevelIndex;
			if( !g_pMissionDB->IsMissionLevel( s_apszLevels[nSelectedLevel], nMission, nLevelIndex ))
				return 1;

			HRECORD hLevel = g_pMissionDB->GetLevel( nMission, nLevelIndex );
			if( hLevel && g_pClientConnectionMgr->SetupServerSinglePlayer( ))
			{
				g_pMissionMgr->StartGameFromLevel(g_pMissionDB->GetWorldName(hLevel,true));
			}

			break;
		}
#endif // defined(_SPDEMO)
	case CMD_BACK:
		{


#ifndef _REMOVE_CUSTOM_LEVELS
			m_pCustom->Show(false);
			m_pCustomScroll->Show(false);
			m_pCustom->SetSelection(kNoSelection);
			SetSelection(kNoSelection);
#endif // _REMOVE_CUSTOM_LEVELS


			m_pDiff->Show(false);
			m_pDiff->SetSelection(kNoSelection);
			m_pScreenMgr->EscapeCurrentScreen();
			break;
		}
	case CMD_NEW_GAME:
		{

#ifndef _REMOVE_CUSTOM_LEVELS
			m_pCustom->Show(false);
			m_pCustomScroll->Show(false);
#endif // _REMOVE_CUSTOM_LEVELS

			m_pDiff->Show(true);

			SetSelection(GetIndex(m_pDiff));
			m_pDiff->SetSelection(g_pGameClientShell->GetDifficulty());
			UpdateHelpText();

			break;
		}
	case CMD_EASY:
		{
			g_pGameClientShell->SetDifficulty(GD_EASY);
			g_pMissionMgr->StartGameNew( );
			break;
		}
	case CMD_MEDIUM:
		{
			g_pGameClientShell->SetDifficulty(GD_NORMAL);
			g_pMissionMgr->StartGameNew( );
			break;
		}
	case CMD_HARD:
		{
			g_pGameClientShell->SetDifficulty(GD_HARD);
			g_pMissionMgr->StartGameNew( );
			break;
		}
	case CMD_INSANE:
		{
			g_pGameClientShell->SetDifficulty(GD_VERYHARD);
			g_pMissionMgr->StartGameNew( );
			break;
		}

	case CMD_LOAD_GAME:
		{
			g_pInterfaceMgr->SwitchToScreen(SCREEN_ID_LOAD);
			break;
		}
	case CMD_CUSTOM_LEVEL:
		{
			m_pDiff->Show(false);

#ifndef _REMOVE_CUSTOM_LEVELS
			m_pCustom->Show(true);
			m_pCustomScroll->Show(true);
			SetSelection(GetIndex(m_pCustom));
#endif // _REMOVE_CUSTOM_LEVELS

			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


void CScreenSingle::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		// We should not be connected to a server at this point.
		if(g_pLTClient->IsConnected())
		{
			g_pInterfaceMgr->SetIntentionalDisconnect( true );
			g_pClientConnectionMgr->ForceDisconnect();
		}

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

		if (m_pLoadCtrl)
		m_pLoadCtrl->Enable( g_pClientSaveLoadMgr->CanContinueGame( ));
		m_pDiff->Show(false);

		// we may have gotten here after failing a load, so rebuild our history
		if (m_pScreenMgr->GetLastScreenID() == SCREEN_ID_NONE)
		{
			m_pScreenMgr->AddScreenToHistory(SCREEN_ID_MAIN);
		}

#ifndef _REMOVE_CUSTOM_LEVELS
		m_pCustom->Show(false);
		m_pCustomScroll->Show(false);
#endif _REMOVE_CUSTOM_LEVELS

		//have they completed game?
		bool bCompleted = !!LTProfileUtils::ReadUint32( "Game", "EndGame", 0, g_pVersionMgr->GetGameSystemIniFile());
	}

	CBaseScreen::OnFocus(bFocus);
}


void CScreenSingle::Escape()
{
	if (m_pDiff->IsVisible())
	{
		m_pDiff->SetSelection(kNoSelection);
		m_pDiff->Show(false);

		SetSelection(0);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
#ifndef _REMOVE_CUSTOM_LEVELS
	else if (m_pCustom->IsVisible())
	{
		m_pCustom->SetSelection(kNoSelection);
		m_pCustom->Show(false);
		SetSelection(2);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
#endif // _REMOVE_CUSTOM_LEVELS

	else
	{
		CBaseScreen::Escape();
	}
}


// Build the list of Custom Levels
void CScreenSingle::BuildCustomLevelsList(int32 nWidth)
{
	m_Filenames.clear();
	m_pCustom->RemoveAll();

	// Get a list of world names and sort them alphabetically
	CMissionDB& MissionDatabase = CMissionDB::Instance();
	uint8 nNumPaths = MissionDatabase.GetNumValues(MissionDatabase.GetMissionSharedRecord(), MDB_SinglePlayerWorld);

    char pathBuf[128];
	FileEntry** pFilesArray = debug_newa(FileEntry*, nNumPaths);

	if (pFilesArray)
	{
		for (int i=0; i < nNumPaths; ++i)
		{
			pathBuf[0] = '\0';
			LTStrCpy(pathBuf, MissionDatabase.GetString(MissionDatabase.GetMissionSharedRecord(), MDB_SinglePlayerWorld, i), LTARRAYSIZE(pathBuf));

			if (pathBuf[0])
			{
                pFilesArray[i] = g_pLTBase->FileMgr()->GetFileList(pathBuf);
			}
			else
			{
                pFilesArray[i] = NULL;
			}
		}
	}



	char Buf[255];

	for (int i=0; i < nNumPaths; ++i)
	{
		pathBuf[0] = '\0';
		LTStrCpy(pathBuf, MissionDatabase.GetString(MissionDatabase.GetMissionSharedRecord(), MDB_SinglePlayerWorld, i), LTARRAYSIZE(pathBuf));

		if (pathBuf[0] && pFilesArray[i])
		{
			LTSNPrintF( Buf, LTARRAYSIZE( Buf ), "%s\\", pathBuf);
			AddFilesToFilenames(pFilesArray[i], Buf);
            g_pLTBase->FileMgr()->FreeFileList(pFilesArray[i]);
		}
	}

	debug_deletea(pFilesArray);

	uint32 nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);
	int index = 0;
	StringSet::iterator iter = m_Filenames.begin();
	CLTGUICtrl_create cs;
	cs.pCommandHandler = this;
	cs.rnBaseRect.Right() = nWidth;
	cs.rnBaseRect.Bottom() = nListFontSize;
	cs.szHelpID = "IDS_HELP_CUSTOMLEVEL";

	char const* pszListFont = g_pLayoutDB->GetListFont(m_hLayout,0);

	while (iter != m_Filenames.end())
	{
		cs.nCommandID = CMD_CUSTOM+index;

		CLTGUIColumnCtrlEx* pColumnCtrl = debug_new(CLTGUIColumnCtrlEx);
		if( !pColumnCtrl )
		{
			LTERROR( "Couldn't create column control for rule list." );
			continue;
		}
		pColumnCtrl->Create(cs);
		pColumnCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());
		pColumnCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		pColumnCtrl->SetFont( CFontInfo(pszListFont, nListFontSize) );
		pColumnCtrl->AddTextColumn( MPA2WEX<MAX_PATH>(iter->c_str()).c_str(), nWidth, true );

		m_pCustom->AddControl( pColumnCtrl );
		++index;
		iter++;
	}

}

void CScreenSingle::AddFilesToFilenames(FileEntry* pFiles, char* pPath)
{
	if (!pFiles || !pPath) return;

	char strBaseName[256];
	char* pBaseName = NULL;
	char* pBaseExt = NULL;
	FileEntry* ptr = pFiles;

	while (ptr)
	{
		if (ptr->m_Type == eLTFileEntryType_File)
		{
			LTStrCpy(strBaseName, ptr->m_pBaseFilename, LTARRAYSIZE(strBaseName));
			pBaseName = strtok (strBaseName, ".");
			pBaseExt = strtok (NULL, "\0");
			if (pBaseExt && LTStrICmp (pBaseExt, RESEXT_WORLD_PACKED) == 0)
			{
				char szString[512];
				LTSNPrintF( szString, LTARRAYSIZE( szString ), "%s%s", pPath, pBaseName);

				// add this to the array
				m_Filenames.insert(szString);
			}
		}

		ptr = ptr->m_pNext;
	}
}

/******************************************************************/
bool CScreenSingle::OnMouseMove(int x, int y)
{

/*
	uint16 oldSelect = kNoSelection;


#ifndef _REMOVE_CUSTOM_LEVELS
	if (GetSelectedControl() == m_pCustom)
	{
		oldSelect = m_pCustom->GetSelectedIndex();
	}
#endif // _REMOVE_CUSTOM_LEVELS

*/
	bool bHandled = CBaseScreen::OnMouseMove(x,y);

/*
#ifndef _REMOVE_CUSTOM_LEVELS
	if (bHandled && GetSelectedControl() == m_pCustom && oldSelect != kNoSelection && oldSelect != m_pCustom->GetSelectedIndex())
	{
		CLTGUICtrl *pSelCtrl = m_pCustom->GetSelectedControl();
		LTVector2n pos = pSelCtrl->GetPos();
		if (m_bSelectFXCenter)
			pos.x += (pSelCtrl->GetWidth() / 2);
		pos.y += (pSelCtrl->GetHeight() / 2);

		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);

		g_pInterfaceMgr->ShowSelectFX(pos);
	}
#endif // _REMOVE_CUSTOM_LEVELS
*/

	return bHandled;

}

/******************************************************************/
bool CScreenSingle::HandleKeyDown(int key, int rep)
{

/*
	uint16 oldSelect = kNoSelection;


#ifndef _REMOVE_CUSTOM_LEVELS
	if (GetSelectedControl() == m_pCustom)
	{
		oldSelect = m_pCustom->GetSelectedIndex();
	}
#endif // _REMOVE_CUSTOM_LEVELS

*/
	bool bHandled = CBaseScreen::HandleKeyDown(key,rep);

/*
#ifndef _REMOVE_CUSTOM_LEVELS
	if (bHandled && GetSelectedControl() == m_pCustom && oldSelect != kNoSelection && oldSelect != m_pCustom->GetSelectedIndex())
	{
		m_pCustom->GetWidth();
		CLTGUICtrl *pSelCtrl = m_pCustom->GetSelectedControl();
		LTVector2n pos = pSelCtrl->GetPos();
		if (m_bSelectFXCenter)
			pos.x += (pSelCtrl->GetWidth() / 2);
		pos.y += (pSelCtrl->GetHeight() / 2);

		g_pInterfaceMgr->ShowSelectFX(pos);
	}
#endif // _REMOVE_CUSTOM_LEVELS

*/
	return bHandled;

}


