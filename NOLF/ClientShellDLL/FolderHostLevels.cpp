// FolderHostLevels.cpp: implementation of the CFolderHostLevels class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "FolderHostLevels.h"
#include "FolderMgr.h"
#include "FolderCommands.h"
#include "ClientRes.h"
#include "VarTrack.h"
#include "NetDefs.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;



extern VarTrack	g_vtNetGameType;


namespace
{
	enum eLocalCommands
	{
		CMD_ADD_LEVEL = FOLDER_CMD_CUSTOM+1,
		CMD_REMOVE_LEVEL,
		CMD_ADD_ALL,
		CMD_REMOVE_ALL,
		CMD_FILTER,
	};
    LTRect rcAvailRect;
    LTRect rcSelRect;
    LTRect rcCommandRect;

	int nBarHeight;
	int nArrowWidth;
	int nIndent;

	char szPath[128];


}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderHostLevels::CFolderHostLevels()
{
	m_nGameType		= DEATHMATCH;
    m_pAvailLevels	= LTNULL;
    m_pSelLevels    = LTNULL;
    m_pAddLevel     = LTNULL;
    m_pRemoveLevel  = LTNULL;
    m_pAddAll       = LTNULL;
    m_pRemoveAll    = LTNULL;
}

CFolderHostLevels::~CFolderHostLevels()
{

}

// Build the folder
LTBOOL CFolderHostLevels::Build()
{
	if (!g_vtNetGameType.IsInitted())
	{
        g_vtNetGameType.Init(g_pLTClient,"NetGameType",LTNULL,(float)m_nGameType);
	}

	rcAvailRect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"AvailLevelsRect");
	rcSelRect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"SelLevelsRect");
	rcCommandRect = g_pLayoutMgr->GetFolderCustomRect((eFolderID)m_nFolderID,"CommandRect");
	nIndent = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"TextIndent");
	nArrowWidth = g_pLayoutMgr->GetFolderCustomInt((eFolderID)m_nFolderID,"ArrowWidth");

	CreateTitle(IDS_TITLE_HOST_LEVELS);

	//Add Avail Levels List
    LTIntPt pos(rcAvailRect.left + nIndent,rcAvailRect.top);
	CLTGUIFont *pFont = GetLargeFont();
	nBarHeight = pFont->GetHeight();

    CStaticTextCtrl *pCtrl = CreateStaticTextItem(IDS_HOST_AVAIL_LEVELS, LTNULL, LTNULL,0,0, LTTRUE, pFont);
    AddFixedControl(pCtrl,pos,LTFALSE);
    pCtrl->Enable(LTFALSE);

	pFont = GetSmallFont();
	pos.x = rcAvailRect.left + nIndent;
	pos.y = rcAvailRect.top + nBarHeight;
	int nListHeight = (rcAvailRect.bottom - rcAvailRect.top) - nBarHeight;
	int nListWidth = (rcAvailRect.right - rcAvailRect.left) - nArrowWidth;

	m_pAvailLevels = debug_new(CListCtrl);
    m_pAvailLevels->Create(nListHeight, LTTRUE, nListWidth);
	m_pAvailLevels->SetParam1(nListWidth);
	m_pAvailLevels->SetItemSpacing(0);
    m_pAvailLevels->EnableMouseMoveSelect(LTTRUE);
	m_pAvailLevels->SetHelpID(IDS_HELP_AVAIL_LEVELS);
    AddFixedControl(m_pAvailLevels,pos,LTTRUE);

	//Add Selected Levels List
    pos = LTIntPt(rcSelRect.left + nIndent,rcSelRect.top);
	pFont = GetLargeFont();

    pCtrl = CreateStaticTextItem(IDS_HOST_SELECTED_LEVELS, LTNULL, LTNULL,0,0,LTTRUE, pFont);
    AddFixedControl(pCtrl,pos,LTFALSE);
    pCtrl->Enable(LTFALSE);

	pFont = GetSmallFont();
	pos.x = rcSelRect.left + nIndent;
	pos.y = rcSelRect.top + nBarHeight;
	nListHeight = (rcSelRect.bottom - rcSelRect.top) - nBarHeight;
	nListWidth = (rcSelRect.right - rcSelRect.left) - nArrowWidth;

	m_pSelLevels = debug_new(CListCtrl);
    m_pSelLevels->Create(nListHeight, LTTRUE, nListWidth);
	m_pSelLevels->SetParam1(nListWidth);
	m_pSelLevels->SetItemSpacing(0);
    m_pSelLevels->EnableMouseMoveSelect(LTTRUE);
	m_pSelLevels->SetHelpID(IDS_HELP_SEL_LEVELS);
    AddFixedControl(m_pSelLevels,pos,LTTRUE);

	//Add Commands
    pos = LTIntPt(rcCommandRect.left + nIndent,rcCommandRect.top);
	nListWidth = rcCommandRect.right - rcCommandRect.left;
	pFont = GetLargeFont();


	m_pAddAll = CreateStaticTextItem(IDS_HOST_ADD_ALL, CMD_ADD_ALL, IDS_HELP_ADD_ALL, nListWidth);
    AddFixedControl(m_pAddAll,pos,LTTRUE);
	pos.y += (m_nItemSpacing + pFont->GetHeight());

	m_pAddLevel = CreateStaticTextItem(IDS_HOST_ADD_LEVEL, CMD_ADD_LEVEL, IDS_HELP_ADD_LEVEL, nListWidth);
    AddFixedControl(m_pAddLevel,pos,LTTRUE);
	pos.y += (m_nItemSpacing + pFont->GetHeight());

	m_pRemoveLevel = CreateStaticTextItem(IDS_HOST_REMOVE_LEVEL, CMD_REMOVE_LEVEL, IDS_HELP_REM_LEVEL, nListWidth);
    AddFixedControl(m_pRemoveLevel,pos,LTTRUE);
	pos.y += (m_nItemSpacing + pFont->GetHeight());

	m_pRemoveAll = CreateStaticTextItem(IDS_HOST_REMOVE_ALL, CMD_REMOVE_ALL, IDS_HELP_REM_ALL, nListWidth);
    AddFixedControl(m_pRemoveAll,pos,LTTRUE);
	pos.y += (m_nItemSpacing + pFont->GetHeight());



 	// Make sure to call the base class
	if (!CBaseFolder::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);

	return LTTRUE;

}

uint32 CFolderHostLevels::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_FILTER:
		{
			UpdateData();
			m_pAvailLevels->RemoveAllControls();
			FillLevelList(m_pAvailLevels, szPath);
			UpdateButtons();
		} break;

	case CMD_ADD_LEVEL:
		{
			char sLevel[256] = "";
			if (m_pAvailLevels->IsItemSelected() && (m_pSelLevels->GetNum() < MAX_GAME_LEVELS))
			{
				CStaticTextCtrl *pCtrl = (CStaticTextCtrl *)m_pAvailLevels->GetSelectedControl();
				if (pCtrl)
				{
					HSTRING hText = pCtrl->GetString();
                    char *pTemp = g_pLTClient->GetStringData(hText);
					sprintf(sLevel,"%s\\%s",szPath,pTemp);

				}
			}

			if (strlen(sLevel))
			{
				AddLevelToList(m_pSelLevels,sLevel);
				m_pSelLevels->ClearSelection();
			}
			UpdateButtons();

		} break;
	case CMD_ADD_ALL:
		{
			if (m_pAvailLevels->IsItemSelected())
			{
				for (int i = 0; i < m_pAvailLevels->GetNum() && (m_pSelLevels->GetNum() < MAX_GAME_LEVELS); i++)
				{
					char sLevel[256] = "";
					CStaticTextCtrl *pCtrl = (CStaticTextCtrl *)m_pAvailLevels->GetControl(i);
					if (pCtrl)
					{
						HSTRING hText = pCtrl->GetString();
                        char *pTemp = g_pLTClient->GetStringData(hText);
						sprintf(sLevel,"%s\\%s",szPath,pTemp);

					}
					if (strlen(sLevel))
					{
						AddLevelToList(m_pSelLevels,sLevel);
						m_pSelLevels->ClearSelection();
					}

					



				}
			}
			UpdateButtons();

		} break;
	case CMD_REMOVE_LEVEL:
		{
			if (m_pSelLevels->IsItemSelected())
			{
				int nIndex = m_pSelLevels->GetSelectedItem();
				m_pSelLevels->RemoveControl(nIndex);
			}
			UpdateButtons();

		} break;
	case CMD_REMOVE_ALL:
		{
			if (m_pSelLevels->GetNum() > 0)
			{
				m_pSelLevels->ClearSelection();
				m_pSelLevels->RemoveAllControls();
			}
			UpdateButtons();

		} break;
	default:
		return CBaseFolder::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CFolderHostLevels::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_nGameType = (int)g_vtNetGameType.GetFloat();

		if (m_nGameType == COOPERATIVE_ASSAULT)
		{
			strcpy(szPath,"Worlds\\Multi\\AssaultMap");
		}
		else
		{
			strcpy(szPath,"Worlds\\Multi\\DeathMatch");
		}

		FillLevelList(m_pAvailLevels, szPath);

		LoadLevelList(m_pSelLevels);

		UpdateButtons();
        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();
		SaveLevelList(m_pSelLevels);

		m_pAvailLevels->RemoveAllControls();
		m_pSelLevels->RemoveAllControls();

        g_pLTClient->WriteConfigFile("autoexec.cfg");

	}
	CBaseFolder::OnFocus(bFocus);
}

LTBOOL CFolderHostLevels::FillLevelList(CListCtrl *pList, char* sDir)
{
	// Sanity checks...

    if (!pList) return(LTFALSE);
    if (!sDir) return(LTFALSE);

    FileEntry* pFiles = g_pLTClient->GetFileList(sDir);
	if (!pFiles) return(FALSE);

	FileEntry* ptr = pFiles;

	// Array of controls to add to the list
	CMoArray<CString *>	m_levels;
	m_levels.SetSize(0);



	while (ptr)
	{
		if (ptr->m_Type == TYPE_FILE)
		{
			if (strnicmp(&ptr->m_pBaseFilename[strlen(ptr->m_pBaseFilename) - 4], ".dat", 4) == 0)
			{
				char sLevel[128];
				strcpy(sLevel, ptr->m_pBaseFilename);
				int len = strlen(sLevel);
				if (len > 4) sLevel[len - 4] = '\0';

				CString *pStr = debug_new1(CString, sLevel);


				m_levels.Add(pStr);

			}
		}

		ptr = ptr->m_pNext;
	}

    g_pLTClient->FreeFileList(pFiles);

	while (m_levels.GetSize())
	{
		uint32 nCur = 0;
		uint32 nTest = 1;
		while (nTest < m_levels.GetSize())
		{
		
			if (m_levels[nTest]->CompareNoCase((char *)(LPCTSTR)(*m_levels[nCur])) < 0)
			{
				nCur = nTest;
			}
			nTest++;
		}

		
		CStaticTextCtrl *pCtrl = CreateStaticTextItem((char *)(LPCTSTR)(*m_levels[nCur]),LTNULL,LTNULL,(int)pList->GetParam1(),GetSmallFont()->GetHeight(),LTFALSE,GetSmallFont());
		pList->AddControl(pCtrl);
		debug_delete(m_levels[nCur]);
		m_levels.Remove(nCur);

	}


    return (LTTRUE);
}

void CFolderHostLevels::LoadLevelList(CListCtrl *pList)
{
	// Sanity checks...

	if (!pList) return;


	// Get the level count...

	int cLevels = 0;
	if (m_nGameType == COOPERATIVE_ASSAULT)
	{
		cLevels = GetConsoleInt("NetCANumLevels", 0);
	}
	else
	{
		cLevels = GetConsoleInt("NetNumLevels", 0);
	}
	if (cLevels <= 0) return;


	// Get each level...

	for (int i = 0; i < cLevels; i++)
	{
		char sLevel[256] = { "" };
		char sLabel[32];

		if (m_nGameType == COOPERATIVE_ASSAULT)
		{
			wsprintf(sLabel, "NetCALevel%i", i);
		}
		else
		{
			wsprintf(sLabel, "NetLevel%i", i);
		}

		GetConsoleString(sLabel, sLevel, "");
		if (strlen(sLevel))
		{
			AddLevelToList(m_pSelLevels,sLevel);
		}
	}
}

void CFolderHostLevels::SaveLevelList(CListCtrl *pList)
{
	// Sanity checks...

	if (!pList) return;


	// Write out the level count...
	int cLevels = pList->GetNum();

	if (m_nGameType == COOPERATIVE_ASSAULT)
	{
		WriteConsoleInt("NetCANumLevels", cLevels);
	}
	else
	{
		WriteConsoleInt("NetNumLevels", cLevels);
	}


	// Write out each level...

	for (int i = 0; i < cLevels; i++)
	{
		char sLabel[32];
		if (m_nGameType == COOPERATIVE_ASSAULT)
		{
			wsprintf(sLabel, "NetCALevel%i", i);
		}
		else
		{
			wsprintf(sLabel, "NetLevel%i", i);
		}
		CStaticTextCtrl *pCtrl = (CStaticTextCtrl *)pList->GetControl(i);
		HSTRING hText = pCtrl->GetString();
        char *pTemp = g_pLTClient->GetStringData(hText);
		char sLevel[256] = "";
		sprintf(sLevel,"%s\\%s",szPath,pTemp);

		WriteConsoleString(sLabel, sLevel);
	}
}


LTBOOL CFolderHostLevels::OnLButtonDown(int x, int y)
{
    LTBOOL bHandled = CBaseFolder::OnLButtonDown(x,y);
	return bHandled;
}
LTBOOL CFolderHostLevels::OnRButtonDown(int x, int y)
{
    LTBOOL bHandled = CBaseFolder::OnRButtonDown(x,y);
	return bHandled;
}


void CFolderHostLevels::UpdateButtons()
{
	if  ( m_pSelLevels->GetNum() < MAX_GAME_LEVELS  &&
		  ( m_pAvailLevels->IsItemSelected() )
		)
	{
        m_pAddLevel->Enable(LTTRUE);
        m_pAddAll->Enable(LTTRUE);
	}
	else
	{
        m_pAddLevel->Enable(LTFALSE);
        m_pAddAll->Enable(LTFALSE);
	}

	m_pRemoveLevel->Enable(m_pSelLevels->IsItemSelected());
	m_pRemoveAll->Enable(m_pSelLevels->GetNum() > 0);
		
}
LTBOOL CFolderHostLevels::Render(HSURFACE hDestSurf)
{
	DrawFrame(hDestSurf,&rcAvailRect,(GetSelectedControl() == m_pAvailLevels));
	DrawFrame(hDestSurf,&rcSelRect,(GetSelectedControl() == m_pSelLevels));
	return CBaseFolder::Render(hDestSurf);
}

void CFolderHostLevels::DrawFrame(HSURFACE hDestSurf, LTRect *rect, LTBOOL bSel)
{
	int xo = g_pInterfaceResMgr->GetXOffset();
	int yo = g_pInterfaceResMgr->GetYOffset();

    HLTCOLOR hColor = (bSel ? m_hSelectedColor : m_hNonSelectedColor);


    LTRect tmpRect(xo+rect->left, yo+rect->top+nBarHeight, xo+rect->right, yo+rect->top+nBarHeight+2);
    g_pLTClient->FillRect(hDestSurf,&tmpRect,hColor);

	tmpRect.bottom = yo+rect->bottom;
	tmpRect.top = tmpRect.bottom - 2;
    g_pLTClient->FillRect(hDestSurf,&tmpRect,hColor);

    tmpRect = LTRect(xo+rect->left, yo+rect->top+nBarHeight+2, xo+rect->left+2, yo+rect->bottom-2);
    g_pLTClient->FillRect(hDestSurf,&tmpRect,hColor);

	tmpRect.right = xo+rect->right;
	tmpRect.left = tmpRect.right - 2;
    g_pLTClient->FillRect(hDestSurf,&tmpRect,hColor);

}


void CFolderHostLevels::AddLevelToList(CListCtrl *pList, char* sGameLevel)
{
	// Sanity checks...

	if (!pList) return;
	if (!sGameLevel) return;
	if (sGameLevel[0] == '\0') return;
	if (pList->GetNum() == MAX_GAME_LEVELS) return;

	char sLevel[128];
	char sTemp[128];
	strcpy(sLevel, sGameLevel);
	strcpy(sTemp, sGameLevel);

	// Prepare the level name by stripping out the prefixes...
	int nLen = strlen(sTemp);
	if (nLen > 2)
	{
		int i = nLen - 1;

		while (i > 0 && sTemp[i] != '\\')
		{
			i--;
		}

		if (i < nLen - 1)
		{
			if (sTemp[i] == '\\') i++;
			strcpy(sLevel, &sTemp[i]);
		}
	}


	// Add the level to the list...

    CStaticTextCtrl *pCtrl = CreateStaticTextItem(sLevel,LTNULL,LTNULL,(int)pList->GetParam1(),GetSmallFont()->GetHeight(),LTFALSE,GetSmallFont());
	pList->AddControl(pCtrl);

}

LTBOOL CFolderHostLevels::OnUp()
{
	if (GetSelectedControl() == m_pSelLevels && m_pSelLevels->OnUp())
		return LTTRUE;
	if (GetSelectedControl() == m_pAvailLevels && m_pAvailLevels->OnUp())
		return LTTRUE;
	LTBOOL bHandled = CBaseFolder::OnUp();
	UpdateButtons();
	return bHandled;
}

LTBOOL CFolderHostLevels::OnDown()
{
	if (GetSelectedControl() == m_pSelLevels && m_pSelLevels->OnDown())
		return LTTRUE;
	if (GetSelectedControl() == m_pAvailLevels && m_pAvailLevels->OnDown())
		return LTTRUE;
	LTBOOL bHandled = CBaseFolder::OnDown();
	UpdateButtons();
	return bHandled;
}

LTBOOL CFolderHostLevels::OnLeft()
{
	if (CBaseFolder::OnLeft()) return LTTRUE;
	if (GetSelectedControl() == m_pSelLevels)
		return m_pRemoveLevel->OnEnter();		
	if (GetSelectedControl() == m_pRemoveLevel)
		return m_pRemoveLevel->OnEnter();		
	if (GetSelectedControl() == m_pRemoveAll)
		return m_pRemoveAll->OnEnter();		
	return LTFALSE;
}
LTBOOL CFolderHostLevels::OnRight()
{
	if (CBaseFolder::OnRight()) return LTTRUE;
	if (GetSelectedControl() == m_pAvailLevels)
		return m_pAddLevel->OnEnter();		
	if (GetSelectedControl() == m_pAddLevel)
		return m_pAddLevel->OnEnter();		
	if (GetSelectedControl() == m_pAddAll)
		return m_pAddAll->OnEnter();		
	return LTFALSE;
}

LTBOOL CFolderHostLevels::OnEnter()
{
	if (CBaseFolder::OnEnter()) return LTTRUE;
	if (m_pAvailLevels->IsItemSelected())
		return m_pAddLevel->OnEnter();		
	if (m_pSelLevels->IsItemSelected())
		return m_pRemoveLevel->OnEnter();		
	return LTFALSE;
}

LTBOOL CFolderHostLevels::OnLButtonUp(int x, int y)
{
//	if (GetSelectedControl() == m_pAvailLevels ||
//		GetSelectedControl() == m_pSelLevels)
//		return LTFALSE;
//
	int adjX = x+g_pInterfaceResMgr->GetXOffset();
	int adjY = y+g_pInterfaceResMgr->GetYOffset();

	if (GetSelectedControl() == m_pAvailLevels)
	{
		int ndx = 0;
		if (m_pAvailLevels->GetControlUnderPoint(adjX,adjY,&ndx))
		{
			if (ndx >= 0)
				return m_pAddLevel->OnEnter();		
		}
			
	}
	if (GetSelectedControl() == m_pSelLevels)
	{
		int ndx = 0;
		if (m_pSelLevels->GetControlUnderPoint(adjX,adjY,&ndx))
		{
			if (ndx >= 0)
				return m_pRemoveLevel->OnEnter();
		}
	}
	return CBaseFolder::OnLButtonUp(x,y);
}
LTBOOL CFolderHostLevels::OnRButtonUp(int x, int y)
{
//	if (GetSelectedControl() == m_pAvailLevels ||
//		GetSelectedControl() == m_pSelLevels)
//		return LTFALSE;
//
	int adjX = x+g_pInterfaceResMgr->GetXOffset();
	int adjY = y+g_pInterfaceResMgr->GetYOffset();

	if (GetSelectedControl() == m_pAvailLevels)
	{
		int ndx = 0;
		if (m_pAvailLevels->GetControlUnderPoint(adjX,adjY,&ndx))
		{
			if (ndx >= 0)
				return m_pAddLevel->OnEnter();		
		}
			
	}
	if (GetSelectedControl() == m_pSelLevels)
	{
		int ndx = 0;
		if (m_pSelLevels->GetControlUnderPoint(adjX,adjY,&ndx))
		{
			if (ndx >= 0)
				return m_pRemoveLevel->OnEnter();
		}
	}
	return CBaseFolder::OnRButtonUp(x,y);
}

LTBOOL CFolderHostLevels::OnLButtonDblClick(int x, int y)
{
	if (CBaseFolder::OnLButtonDblClick(x,y)) return LTTRUE;
	return LTFALSE;
}

LTBOOL CFolderHostLevels::OnMouseMove(int x, int y)
{
	if (CBaseFolder::OnMouseMove(x,y))
	{
		UpdateButtons();
		return LTTRUE;
	}
	return LTFALSE;
}
