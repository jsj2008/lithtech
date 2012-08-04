// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSingle.cpp
//
// PURPOSE : Interface screen for starting, loading, and saving single player
//				games.
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
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


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenSingle::CScreenSingle()
{
    m_pLoadCtrl = LTNULL;
    m_pDiff = LTNULL;
    m_pCustom = LTNULL;

	m_pCustomSFX = LTNULL;
	m_pDiffSFX = LTNULL;

	m_fDiffAlpha = 0.0f;
	m_fCustomAlpha = 0.0f;
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

	CLTGUITextCtrl *pCustom = AddTextItem(IDS_CUSTOM_LEVEL, CMD_CUSTOM_LEVEL, IDS_HELP_CUSTOM);

	LTRect diffRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"DiffRect");
	int nWidth = (diffRect.right - diffRect.left) - 32;
	m_pDiff = AddList(LTIntPt(diffRect.left,diffRect.top),diffRect.bottom - diffRect.top, LTTRUE, nWidth);
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
	
	LTRect customRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"CustomRect");
	nWidth = (customRect.right - customRect.left) - 32;
	m_pCustom = AddList(LTIntPt(customRect.left,customRect.top),customRect.bottom - customRect.top, LTTRUE, nWidth);
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


 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;


	return LTTRUE;

}

uint32 CScreenSingle::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
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
	switch(dwCommand)
	{
	case CMD_BACK:
		{
			m_pDiff->Show(LTFALSE);
			m_pCustom->Show(LTFALSE);
			ShowSFX(LTFALSE,m_pCustomSFX,m_fCustomAlpha);
			ShowSFX(LTFALSE,m_pDiffSFX,m_fDiffAlpha);

			m_pDiff->SetSelection(kNoSelection);
			m_pCustom->SetSelection(kNoSelection);
			m_pScreenMgr->EscapeCurrentScreen();
			break;
		}
	case CMD_NEW_GAME:
		{
			m_pCustom->Show(LTFALSE);
			m_pDiff->Show(LTTRUE);
			ShowSFX(LTFALSE,m_pCustomSFX,m_fCustomAlpha);
			ShowSFX(LTTRUE,m_pDiffSFX,m_fDiffAlpha);

			SetSelection(GetIndex(m_pDiff));
			m_pDiff->SetSelection(g_pGameClientShell->GetDifficulty());

			if( g_pClientMultiplayerMgr->SetupServerSinglePlayer( ))
				g_pMissionMgr->StartGameNew( );

			break;
		}
	case CMD_EASY:
		{
			g_pGameClientShell->SetDifficulty(GD_EASY);
			break;
		}
	case CMD_MEDIUM:
		{
			g_pGameClientShell->SetDifficulty(GD_NORMAL);
			break;
		}
	case CMD_HARD:
		{
			g_pGameClientShell->SetDifficulty(GD_HARD);
			break;
		}
	case CMD_INSANE:
		{
			g_pGameClientShell->SetDifficulty(GD_VERYHARD);
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
			m_pCustom->Show(LTTRUE);
			ShowSFX(LTTRUE,m_pCustomSFX,m_fCustomAlpha);
			ShowSFX(LTFALSE,m_pDiffSFX,m_fDiffAlpha);
			SetSelection(GetIndex(m_pCustom));
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
		m_pLoadCtrl->Enable( g_pClientSaveLoadMgr->ReloadSaveExists() );

		m_pDiff->Show(LTFALSE);
		m_pCustom->Show(LTFALSE);

		// Initialize to the sp mission bute.
		if( !g_pMissionButeMgr->Init( MISSION_DEFAULT_FILE ))
		{
			g_pLTClient->ShutdownWithMessage("Could not load mission bute %s.", MISSION_DEFAULT_FILE );
			return;
  		}
  
		g_pGameClientShell->SetGameType( eGameTypeSingle );

		if( !g_pClientMultiplayerMgr->SetupServerSinglePlayer( ))
		{
			g_pLTClient->ShutdownWithMessage("Could not setup server." );
			return;
  		}
	}

	CBaseScreen::OnFocus(bFocus);

	if (bFocus)
	{
		char szFX[256] = "";
		g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_SINGLE,"CustomFrame",szFX,sizeof(szFX));
		m_pCustomSFX = CreateScaleFX(szFX);

		szFX[0] = 0;
		g_pLayoutMgr->GetScreenCustomString(SCREEN_ID_SINGLE,"DiffFrame",szFX,sizeof(szFX));
		m_pDiffSFX = CreateScaleFX(szFX);

		LTVector vColor;
		if (m_pCustomSFX)
			g_pLTClient->GetObjectColor(m_pCustomSFX->GetObject(), &(vColor.x), &(vColor.y), &(vColor.z), &m_fCustomAlpha);
		if (m_pDiffSFX)
			g_pLTClient->GetObjectColor(m_pDiffSFX->GetObject(), &(vColor.x), &(vColor.y), &(vColor.z), &m_fDiffAlpha);

		ShowSFX(LTFALSE,m_pCustomSFX,m_fCustomAlpha);
		ShowSFX(LTFALSE,m_pDiffSFX,m_fDiffAlpha);
	}
	else
	{
		m_pCustomSFX = NULL;
		m_pDiffSFX = NULL;
	}
}


void CScreenSingle::Escape()
{
	if (m_pDiff->IsVisible())
	{
		m_pDiff->Show(LTFALSE);
		ShowSFX(LTFALSE,m_pDiffSFX,m_fDiffAlpha);

		SetSelection(0);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
	else if (m_pCustom->IsVisible())
	{
		m_pCustom->Show(LTFALSE);
		ShowSFX(LTFALSE,m_pCustomSFX,m_fCustomAlpha);
		SetSelection(3);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
	}
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



/******************************************************************/
LTBOOL CScreenSingle::OnMouseMove(int x, int y)
{
	uint16 oldSelect = kNoSelection;

	if (GetSelectedControl() == m_pCustom)
	{
		oldSelect = m_pCustom->GetSelectedIndex();
	}

	LTBOOL bHandled = CBaseScreen::OnMouseMove(x,y);

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

	return bHandled;

}

/******************************************************************/
LTBOOL CScreenSingle::HandleKeyDown(int key, int rep)
{
	uint16 oldSelect = kNoSelection;

	if (GetSelectedControl() == m_pCustom)
	{
		oldSelect = m_pCustom->GetSelectedIndex();
	}

	LTBOOL bHandled = CBaseScreen::HandleKeyDown(key,rep);

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

	return bHandled;

}
void CScreenSingle::ShowSFX(LTBOOL bShow, CBaseScaleFX *pSFX, float fBaseAlpha)
{
	if (!pSFX || !pSFX->GetObject())
		return;
    LTFLOAT a = 0.0f;
	LTVector vColor;

	g_pLTClient->GetObjectColor(pSFX->GetObject(), &(vColor.x), &(vColor.y), &(vColor.z), &a);


	if (bShow)
		a = fBaseAlpha;
	else
		a = 0.0f;
	
	g_pLTClient->SetObjectColor(pSFX->GetObject(), vColor.x, vColor.y, vColor.z, a);
}


