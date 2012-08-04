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
#include "ClientButeMgr.h"
#include "ClientMultiplayerMgr.h"
#include "clientres.h"


#ifdef _FINAL
#define _REMOVE_CUSTOM_LEVELS
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenSingle::CScreenSingle()
{
    m_pLoadCtrl = LTNULL;

    m_pDiff = LTNULL;
	m_pDiffFrame = LTNULL;

    m_pCustom = LTNULL;
	m_pCustomFrame = LTNULL;

    m_pChapter = LTNULL;
	m_pChapterFrame = LTNULL;

}

CScreenSingle::~CScreenSingle()
{
	m_Filenames.clear();
}

// Build the screen
LTBOOL CScreenSingle::Build()
{

	CreateTitle(IDS_TITLE_GAME);

	AddTextItem(IDS_NEWGAME, CMD_NEW_GAME, IDS_HELP_NEW);

	m_pLoadCtrl = AddTextItem(IDS_LOADGAME,	CMD_LOAD_GAME, IDS_HELP_LOAD);
	m_pLoadCtrl->Enable( LTFALSE );

	m_pChapterCtrl = AddTextItem(IDS_CHAPTERS, CMD_CHAPTER, IDS_HELP_CHAPTERS);

#ifndef _REMOVE_CUSTOM_LEVELS
	CLTGUITextCtrl *pCustom = AddTextItem(IDS_CUSTOM_LEVEL, CMD_CUSTOM_LEVEL, IDS_HELP_CUSTOM);
#endif // _REMOVE_CUSTOM_LEVELS



	char szFrame[128];
	g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_SINGLE,"FrameTexture",szFrame,sizeof(szFrame));
	HTEXTURE hFrame = g_pInterfaceResMgr->GetTexture(szFrame);

	LTRect rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"DiffRect");
	int nHeight = (rect.bottom - rect.top);
	int nWidth = (rect.right - rect.left);
	LTIntPt pos = LTIntPt(rect.left,rect.top);

	m_pDiffFrame = debug_new(CLTGUIFrame);
	m_pDiffFrame->Create(hFrame,nWidth,nHeight,LTTRUE);
	m_pDiffFrame->SetBasePos(pos);
	m_pDiffFrame->Show(LTFALSE);
	AddControl(m_pDiffFrame);


	nWidth -= 16;
	m_pDiff = AddList(pos,nHeight, LTTRUE, nWidth);
	if (m_pDiff)
	{
		m_pDiff->SetIndent(LTIntPt(8,8));
		m_pDiff->SetFrameWidth(2);
		m_pDiff->Show(LTFALSE);


		CLTGUITextCtrl *pCtrl = CreateTextItem(IDS_NEW_EASY,	CMD_EASY,	IDS_HELP_EASY);
		m_pDiff->AddControl(pCtrl);

		pCtrl = CreateTextItem(IDS_NEW_MEDIUM, CMD_MEDIUM, IDS_HELP_MEDIUM);
		m_pDiff->AddControl(pCtrl);

		pCtrl = CreateTextItem(IDS_NEW_HARD,	CMD_HARD,	IDS_HELP_HARD);
		m_pDiff->AddControl(pCtrl);

		pCtrl = CreateTextItem(IDS_NEW_INSANE, CMD_INSANE, IDS_HELP_INSANE);
		m_pDiff->AddControl(pCtrl);
	}


	rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"ChapterRect");
	nHeight = (rect.bottom - rect.top);
	nWidth = (rect.right - rect.left);
	pos = LTIntPt(rect.left,rect.top);

	m_pChapterFrame = debug_new(CLTGUIFrame);
	m_pChapterFrame->Create(hFrame,nWidth,nHeight,LTTRUE);
	m_pChapterFrame->SetBasePos(pos);
	m_pChapterFrame->Show(LTFALSE);
	AddControl(m_pChapterFrame);


	nWidth -= 16;
	m_pChapter = AddList(pos,nHeight, LTTRUE, nWidth);
	if (m_pChapter)
	{
		m_pChapter->SetIndent(LTIntPt(8,8));
		m_pChapter->SetFrameWidth(2);
		m_pChapter->Show(LTFALSE);

	}



#ifndef _REMOVE_CUSTOM_LEVELS

	rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"CustomRect");
	nHeight = (rect.bottom - rect.top);
	nWidth = (rect.right - rect.left);
	pos = LTIntPt(rect.left,rect.top);

	m_pCustomFrame = debug_new(CLTGUIFrame);
	m_pCustomFrame->Create(hFrame,nWidth,nHeight,LTTRUE);
	m_pCustomFrame->SetBasePos(pos);
	m_pCustomFrame->Show(LTFALSE);
	AddControl(m_pCustomFrame);

	nWidth -= 16;
	m_pCustom = AddList(pos,nHeight, LTTRUE, nWidth);
	if (m_pCustom)
	{
		m_pCustom->SetIndent(LTIntPt(8,8));
		m_pCustom->SetFrameWidth(2);
		m_pCustom->Show(LTFALSE);

		BuildCustomLevelsList(nWidth-16);


	}

	//if no custom levels, remove the link
	if (pCustom && !m_pCustom || m_pCustom->GetNumControls() < 1)
	{
		RemoveControl(pCustom);
	}
#endif // _REMOVE_CUSTOM_LEVELS



 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;


	return LTTRUE;

}

uint32 CScreenSingle::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	if (dwCommand >= CMD_CUSTOM)
	{

		int index = dwCommand - CMD_CUSTOM;

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
			if (index < g_pMissionButeMgr->GetNumMissions())
			{
				const MISSION* pMission = g_pMissionButeMgr->GetMission(index);
				const LEVEL* pLevel = &pMission->aLevels[0];
				g_pMissionMgr->StartGameFromLevel(pLevel->szLevel);

			}
			return 1;
		}
	}

	switch(dwCommand)
	{

	case CMD_BACK:
		{


#ifndef _REMOVE_CUSTOM_LEVELS
			m_pCustom->Show(LTFALSE);
			m_pCustom->SetSelection(kNoSelection);
			m_pCustomFrame->Show(LTFALSE);
#endif // _REMOVE_CUSTOM_LEVELS


			m_pDiff->Show(LTFALSE);
			m_pDiff->SetSelection(kNoSelection);
			m_pDiffFrame->Show(LTFALSE);

			m_pChapter->Show(LTFALSE);
			m_pChapter->SetSelection(kNoSelection);
			m_pChapterFrame->Show(LTFALSE);


			m_pScreenMgr->EscapeCurrentScreen();
			break;
		}
	case CMD_NEW_GAME:
		{

#ifndef _REMOVE_CUSTOM_LEVELS
			m_pCustom->Show(LTFALSE);
			m_pCustomFrame->Show(LTFALSE);
#endif // _REMOVE_CUSTOM_LEVELS

			m_pDiff->Show(LTTRUE);
			m_pDiffFrame->Show(LTTRUE);

			m_pChapter->Show(LTFALSE);
			m_pChapterFrame->Show(LTFALSE);


			SetSelection(GetIndex(m_pDiff));
			m_pDiff->SetSelection(g_pGameClientShell->GetDifficulty());
			UpdateHelpText();

			break;
		}

	case CMD_CHAPTER:
		{

#ifndef _REMOVE_CUSTOM_LEVELS
			m_pCustom->Show(LTFALSE);
			m_pCustomFrame->Show(LTFALSE);
#endif // _REMOVE_CUSTOM_LEVELS

			m_pDiff->Show(LTFALSE);
			m_pDiffFrame->Show(LTFALSE);

			m_pChapter->Show(LTTRUE);
			m_pChapterFrame->Show(LTTRUE);


			SetSelection(GetIndex(m_pChapter));
//			m_pDiff->SetSelection(g_pGameClientShell->GetDifficulty());

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
			m_pDiff->Show(LTFALSE);
			m_pDiffFrame->Show(LTFALSE);
			m_pChapter->Show(LTFALSE);
			m_pChapterFrame->Show(LTFALSE);

#ifndef _REMOVE_CUSTOM_LEVELS
			m_pCustom->Show(LTTRUE);
			m_pCustomFrame->Show(LTTRUE);
			SetSelection(GetIndex(m_pCustom));
#endif // _REMOVE_CUSTOM_LEVELS

			break;
		}
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


void CScreenSingle::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		// We should not be connected to a server at this point.
		if(g_pLTClient->IsConnected())
		{
			_ASSERT(!"ERROR: ClientMultiplayerMgr::StartClient( ) : Already connected!");
			g_pLTClient->Disconnect();
		}

		// Initialize to the sp mission bute.
		if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
		{
			g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
			return;
  		}

		m_pLoadCtrl->Enable( g_pClientSaveLoadMgr->ReloadSaveExists() );
		m_pDiff->Show(LTFALSE);
		m_pDiffFrame->Show(LTFALSE);


		m_pChapter->Show(LTFALSE);
		m_pChapterFrame->Show(LTFALSE);


#ifndef _REMOVE_CUSTOM_LEVELS
		m_pCustom->Show(LTFALSE);
		m_pCustomFrame->Show(LTFALSE);
#endif _REMOVE_CUSTOM_LEVELS

		//have they completed game?
		LTBOOL bCompleted = LTFALSE;
		CRegMgr* pRegMgr = g_pVersionMgr->GetRegMgr();
		if (pRegMgr->IsValid())
		{
			bCompleted = (pRegMgr->Get("EndGame") > 0);
		}

		m_pChapterCtrl->Show(bCompleted);

		if (bCompleted && m_pChapter->GetNumControls() == 0)
			BuildChapterList();






	}

	CBaseScreen::OnFocus(bFocus);
}


void CScreenSingle::Escape()
{
	if (m_pDiff->IsVisible())
	{
		m_pDiff->SetSelection(kNoSelection);
		m_pDiff->Show(LTFALSE);
		m_pDiffFrame->Show(LTFALSE);

		SetSelection(0);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	else if (m_pChapter->IsVisible())
	{
		m_pChapter->SetSelection(kNoSelection);
		m_pChapter->Show(LTFALSE);
		m_pChapterFrame->Show(LTFALSE);
		SetSelection(2);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}

#ifndef _REMOVE_CUSTOM_LEVELS
	else if (m_pCustom->IsVisible())
	{
		m_pCustom->SetSelection(kNoSelection);
		m_pCustom->Show(LTFALSE);
		m_pCustomFrame->Show(LTFALSE);
		SetSelection(3);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
#endif // _REMOVE_CUSTOM_LEVELS

	else
	{
		CBaseScreen::Escape();
	}
}


// Build the list of Custom Levels
void CScreenSingle::BuildCustomLevelsList(int nWidth)
{
	m_Filenames.clear();
	m_pCustom->RemoveAll();

	// Get a list of world names and sort them alphabetically

    uint8 nNumPaths = g_pClientButeMgr->GetNumSingleWorldPaths();

    char pathBuf[128];
	FileEntry** pFilesArray = debug_newa(FileEntry*, nNumPaths);

	if (pFilesArray)
	{
		for (int i=0; i < nNumPaths; ++i)
		{
			pathBuf[0] = '\0';
			g_pClientButeMgr->GetWorldPath(i, pathBuf, ARRAY_LEN(pathBuf));

			if (pathBuf[0])
			{
                pFilesArray[i] = g_pLTClient->GetFileList(pathBuf);
			}
			else
			{
                pFilesArray[i] = LTNULL;
			}
		}
	}



	char Buf[255];

	for (int i=0; i < nNumPaths; ++i)
	{
		pathBuf[0] = '\0';
		g_pClientButeMgr->GetWorldPath(i, pathBuf, ARRAY_LEN(pathBuf));

		if (pathBuf[0] && pFilesArray[i])
		{
			sprintf(Buf, "%s\\", pathBuf);
			AddFilesToFilenames(pFilesArray[i], Buf);
            g_pLTClient->FreeFileList(pFilesArray[i]);
		}
	}

	debug_deletea(pFilesArray);

	CLTGUITextCtrl* pItem;

	uint8 nListFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"ListFontSize");
	int index = 0;
	StringSet::iterator iter = m_Filenames.begin();
	while (iter != m_Filenames.end())
	{
		pItem = CreateTextItem((char *)iter->c_str(), CMD_CUSTOM+index, IDS_HELP_CUSTOMLEVEL);
		pItem->SetFont(LTNULL, nListFontSize);
		pItem->SetFixedWidth(nWidth,LTTRUE);

		m_pCustom->AddControl(pItem);
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
		if (ptr->m_Type == TYPE_FILE)
		{
			SAFE_STRCPY(strBaseName, ptr->m_pBaseFilename);
			pBaseName = strtok (strBaseName, ".");
			pBaseExt = strtok (NULL, "\0");
			if (pBaseExt && stricmp (pBaseExt, "dat") == 0)
			{
				char szString[512];
				sprintf(szString, "%s%s", pPath, pBaseName);

				// add this to the array
				m_Filenames.insert(szString);
			}
		}

		ptr = ptr->m_pNext;
	}
}

// Build the list of Chapters
void CScreenSingle::BuildChapterList()
{
	LTRect rect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"ChapterRect");
	int nWidth = (rect.right - rect.left) - 16;


	CLTGUITextCtrl* pItem;

	uint8 nListFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"ListFontSize");
	for (int i = 0; i < g_pMissionButeMgr->GetNumMissions(); i++)
	{
		const MISSION* pMission = g_pMissionButeMgr->GetMission(i);

		pItem = CreateTextItem(pMission->nNameId, CMD_CUSTOM+1000+i, pMission->nDescId);
		pItem->SetFont(LTNULL, nListFontSize);
		pItem->SetFixedWidth(nWidth,LTTRUE);

		m_pChapter->AddControl(pItem);
		
	}
}

/******************************************************************/
LTBOOL CScreenSingle::OnMouseMove(int x, int y)
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
	LTBOOL bHandled = CBaseScreen::OnMouseMove(x,y);

/*
#ifndef _REMOVE_CUSTOM_LEVELS
	if (bHandled && GetSelectedControl() == m_pCustom && oldSelect != kNoSelection && oldSelect != m_pCustom->GetSelectedIndex())
	{
		CLTGUICtrl *pSelCtrl = m_pCustom->GetSelectedControl();
		LTIntPt pos = pSelCtrl->GetPos();
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
LTBOOL CScreenSingle::HandleKeyDown(int key, int rep)
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
	LTBOOL bHandled = CBaseScreen::HandleKeyDown(key,rep);

/*
#ifndef _REMOVE_CUSTOM_LEVELS
	if (bHandled && GetSelectedControl() == m_pCustom && oldSelect != kNoSelection && oldSelect != m_pCustom->GetSelectedIndex())
	{
		m_pCustom->GetWidth();
		CLTGUICtrl *pSelCtrl = m_pCustom->GetSelectedControl();
		LTIntPt pos = pSelCtrl->GetPos();
		if (m_bSelectFXCenter)
			pos.x += (pSelCtrl->GetWidth() / 2);
		pos.y += (pSelCtrl->GetHeight() / 2);

		g_pInterfaceMgr->ShowSelectFX(pos);
	}
#endif // _REMOVE_CUSTOM_LEVELS

*/
	return bHandled;

}


