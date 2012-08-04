// FolderMgr.cpp: implementation of the CFolderMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ClientRes.h"
#include "FolderMgr.h"
#include "ClientButeMgr.h"
#include "SoundMgr.h"

//folders
#include "BaseFolder.h"
#include "FolderMain.h"
#include "FolderSingle.h"

#include "FolderGallery.h"
#include "FolderEscape.h"
#include "FolderOptions.h"
#include "FolderDisplay.h"
#include "FolderAudio.h"
#include "FolderGame.h"
#include "FolderPerformance.h"
#include "FolderAdvDisplay.h"
#include "FolderTexture.h"
#include "FolderEffects.h"
#include "FolderControls.h"
#include "FolderCustomControls.h"
#include "FolderWeaponControls.h"
#include "FolderMouse.h"
#include "FolderKeyboard.h"
#include "FolderJoystick.h"
#include "FolderCustomLevel.h"
#include "FolderFavoriteLevels.h"
#include "FolderNew.h"
#include "FolderDifficulty.h"
#include "FolderMission.h"
#include "FolderBriefing.h"
#include "FolderMultiBriefing.h"
#include "FolderMultiSummary.h"
#include "FolderObjectives.h"
#include "FolderWeapons.h"
#include "FolderGadgets.h"
#include "FolderMods.h"
#include "FolderGear.h"
#include "FolderInventory.h"
#include "FolderViewInventory.h"
#include "FolderStats.h"
#include "FolderIntel.h"
#include "FolderSummary.h"
#include "FolderAwards.h"
#include "FolderSave.h"
#include "FolderLoad.h"
#include "FolderCrosshair.h"
#include "FolderMulti.h"
#include "FolderPlayer.h"
#include "FolderJoin.h"
#include "FolderJoinLAN.h"
#include "FolderHost.h"
#include "FolderHostOptions.h"
#include "FolderHostLevels.h"




#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFolderMgr::CFolderMgr()
{
    m_pClientDE = LTNULL;
    m_pClientShell = LTNULL;
    m_pCurrentFolder = LTNULL;
	m_eCurrentFolderID = FOLDER_ID_NONE;
	m_eLastFolderID = FOLDER_ID_NONE;
	m_nHistoryLen = 0;
}

CFolderMgr::~CFolderMgr()
{

}


//////////////////////////////////////////////////////////////////////
// Function name	: CFolderMgr::Init
// Description	    :
// Return type      : LTBOOL
// Argument         : CClientDE* pClientDE
// Argument         : CGameClientShell* pClientShell
//////////////////////////////////////////////////////////////////////

LTBOOL CFolderMgr::Init(ILTClient* pClientDE, CGameClientShell* pClientShell)
{
	if (!pClientDE)
	{
        return LTFALSE;
	}

	m_pClientDE = pClientDE;

	//build folder array
	m_folderArray.SetSize(0);

	for (int nID = FOLDER_ID_MAIN; nID < FOLDER_ID_UNASSIGNED; nID++)
	{
		AddFolder((eFolderID)nID);
	}

    return LTTRUE;
}

//////////////////////////////////////////////////////////////////////
// Function name	: CFolderMgr::Term
// Description	    :
// Return type		: void
//////////////////////////////////////////////////////////////////////

void CFolderMgr::Term()
{
	// Term the folders
	unsigned int i;
	for (i=0; i < m_folderArray.GetSize(); i++)
	{
		m_folderArray[i]->Term();
		debug_delete(m_folderArray[i]);
	}
}



// Renders the folder to a surface
LTBOOL CFolderMgr::Render(HSURFACE hDestSurf)
{
	if (m_pCurrentFolder)
	{

		return m_pCurrentFolder->Render(hDestSurf);
	}
    return LTFALSE;
}

void CFolderMgr::UpdateInterfaceSFX()
{
	if (m_pCurrentFolder)
	{

		m_pCurrentFolder->UpdateInterfaceSFX();
	}
}

void CFolderMgr::HandleKeyDown (int vkey, int rep)
{
	if (m_pCurrentFolder)
	{
		if (vkey == VK_ESCAPE)
		{
			m_pCurrentFolder->Escape();
		}
		else
			m_pCurrentFolder->HandleKeyDown(vkey,rep);
	}
}

void CFolderMgr::HandleKeyUp (int vkey)
{
	if (m_pCurrentFolder)
	{
		m_pCurrentFolder->HandleKeyUp(vkey);
	}
}

void CFolderMgr::HandleChar (char c)
{
	if (m_pCurrentFolder)
	{
		m_pCurrentFolder->HandleChar(c);
	}
}

LTBOOL CFolderMgr::PreviousFolder()
{
	if (m_nHistoryLen < 1) return LTFALSE;

	CBaseFolder *pNewFolder=GetFolderFromID(m_eFolderHistory[m_nHistoryLen-1]);
	if (pNewFolder)
	{
		SwitchToFolder(pNewFolder,LTTRUE);

		// The music may change per folder...
        g_pInterfaceMgr->SetMenuMusic(LTTRUE);

        return LTTRUE;
	}
    return LTFALSE;
}

LTBOOL CFolderMgr::SetCurrentFolder(eFolderID folderID)
{

	CBaseFolder *pNewFolder=GetFolderFromID(folderID);
	if (pNewFolder)
	{
		SwitchToFolder(pNewFolder);

		// The music may change per folder...
        g_pInterfaceMgr->SetMenuMusic(LTTRUE);

        return LTTRUE;
	}
    return LTFALSE;
}

void CFolderMgr::EscapeCurrentFolder()
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->Escape();
}

void CFolderMgr::ExitFolders()
{
	// Tell the old folder that it is losing focus
	if (m_pCurrentFolder)
	{
        m_pCurrentFolder->OnFocus(LTFALSE);
	}

	//clear our folder history (no longer relevant)
	m_nHistoryLen = 0;
	m_eFolderHistory[0] = FOLDER_ID_NONE;
	m_pCurrentFolder = LTNULL;
	m_eCurrentFolderID = FOLDER_ID_NONE;

}


void CFolderMgr::SwitchToFolder(CBaseFolder *pNewFolder, LTBOOL bBack)
{

	// Tell the old folder that it is losing focus
	if (m_pCurrentFolder)
	{
        m_pCurrentFolder->OnFocus(LTFALSE);
		if (bBack)
		{
			m_nHistoryLen--;
		}
		else if (m_nHistoryLen < MAX_FOLDER_HISTORY)
		{
			m_eFolderHistory[m_nHistoryLen] = (eFolderID)m_pCurrentFolder->GetFolderID();
			m_nHistoryLen++;
		}
	}

	m_pCurrentFolder=pNewFolder;
	m_eLastFolderID = m_eCurrentFolderID;

	m_eCurrentFolderID = (eFolderID)m_pCurrentFolder->GetFolderID();

	// If the new folder hasn't been built yet... better build it!
	if (!m_pCurrentFolder->IsBuilt())
	{

		m_pCurrentFolder->Build();
	}

	// Do any special case work for each folder
//	switch(folderID)
//	{
//	case FOLDER_ID_NEW:
//		break;
//	default:
//		break;
//	}

	// Tell the new folder that it is gaining focus
	if (pNewFolder)
	{
		// Do quick shutter fx.
		// Needs to be timed...
		//g_pInterfaceMgr->ClearAllScreenBuffers();

        pNewFolder->OnFocus(LTTRUE);
//		g_pClientSoundMgr->PlayInterfaceSound(g_pInterfaceResMgr->GetSoundFolderOpen());
	}
}


// Returns a folder based on an ID
CBaseFolder *CFolderMgr::GetFolderFromID(eFolderID folderID)
{
	CBaseFolder *pFolder=NULL;

	int f = 0;
	while (f < (int)m_folderArray.GetSize() && m_folderArray[f]->GetFolderID() != (int)folderID)
		f++;

	if (f < (int)m_folderArray.GetSize())
		pFolder = m_folderArray[f];

	return pFolder;

}


// Mouse messages
void	CFolderMgr::OnLButtonDown(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnLButtonDown(x,y);
}

void	CFolderMgr::OnLButtonUp(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnLButtonUp(x,y);
}

void	CFolderMgr::OnLButtonDblClick(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnLButtonDblClick(x,y);
}

void	CFolderMgr::OnRButtonDown(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnRButtonDown(x,y);
}

void	CFolderMgr::OnRButtonUp(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnRButtonUp(x,y);
}

void	CFolderMgr::OnRButtonDblClick(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnRButtonDblClick(x,y);
}

void	CFolderMgr::OnMouseMove(int x, int y)
{
	if (m_pCurrentFolder)
		m_pCurrentFolder->OnMouseMove(x,y);
}


void CFolderMgr::AddFolder(eFolderID folderID)
{
    CBaseFolder* pFolder = LTNULL;
	switch (folderID)
	{
	case FOLDER_ID_MAIN:
		pFolder = debug_new(CFolderMain);
		break;

	case FOLDER_ID_SINGLE:
		pFolder = debug_new(CFolderSingle);
		break;

	case FOLDER_ID_GALLERY:
		pFolder = debug_new(CFolderGallery);
		break;

	case FOLDER_ID_ESCAPE:
		pFolder = debug_new(CFolderEscape);
		break;

	case FOLDER_ID_MULTIPLAYER:
		pFolder = debug_new(CFolderMulti);
		break;

	case FOLDER_ID_PLAYER:
		pFolder = debug_new(CFolderPlayer);
		break;

	case FOLDER_ID_JOIN:
		pFolder = debug_new(CFolderJoin);
		break;

	case FOLDER_ID_JOIN_LAN:
		pFolder = debug_new(CFolderJoinLAN);
		break;

	case FOLDER_ID_HOST:
		pFolder = debug_new(CFolderHost);
		break;

	case FOLDER_ID_HOST_OPTIONS:
		pFolder = debug_new(CFolderHostOptions);
		break;

	case FOLDER_ID_HOST_LEVELS:
		pFolder = debug_new(CFolderHostLevels);
		break;

	case FOLDER_ID_OPTIONS:
		pFolder = debug_new(CFolderOptions);
		break;

	case FOLDER_ID_DISPLAY:
		pFolder = debug_new(CFolderDisplay);
		break;

	case FOLDER_ID_AUDIO:
		pFolder = debug_new(CFolderAudio);
		break;

	case FOLDER_ID_GAME:
		pFolder = debug_new(CFolderGame);
		break;

	case FOLDER_ID_PERFORMANCE:
		pFolder = debug_new(CFolderPerformance);
		break;

	case FOLDER_ID_ADVDISPLAY:
		pFolder = debug_new(CFolderAdvDisplay);
		break;

	case FOLDER_ID_TEXTURE:
		pFolder = debug_new(CFolderTexture);
		break;

	case FOLDER_ID_EFFECTS:
		pFolder = debug_new(CFolderEffects);
		break;

	case FOLDER_ID_CONTROLS:
		pFolder = debug_new(CFolderControls);
		break;

	case FOLDER_ID_CUST_CONTROLS:
		pFolder = debug_new(CFolderCustomControls);
		break;

	case FOLDER_ID_WPN_CONTROLS:
		pFolder = debug_new(CFolderWeaponControls);
		break;

	case FOLDER_ID_CROSSHAIR:
		pFolder = debug_new(CFolderCrosshair);
		break;

	case FOLDER_ID_MOUSE:
		pFolder = debug_new(CFolderMouse);
		break;

	case FOLDER_ID_KEYBOARD:
		pFolder = debug_new(CFolderKeyboard);
		break;

	case FOLDER_ID_JOYSTICK:
		pFolder = debug_new(CFolderJoystick);
		break;

	case FOLDER_ID_CUSTOM_LEVEL:
		pFolder = debug_new(CFolderCustomLevel);
		break;

	case FOLDER_ID_FAVORITE_LEVEL:
		pFolder = debug_new(CFolderFavoriteLevels);
		break;

	case FOLDER_ID_NEW:
		pFolder = debug_new(CFolderNew);
		break;

	case FOLDER_ID_DIFFICULTY:
		pFolder = debug_new(CFolderDifficulty);
		break;

	case FOLDER_ID_SAVE:
		pFolder = debug_new(CFolderSave);
		break;

	case FOLDER_ID_LOAD:
		pFolder = debug_new(CFolderLoad);
		break;

	case FOLDER_ID_BRIEFING:
		pFolder = debug_new(CFolderBriefing);
		break;

	case FOLDER_ID_MP_BRIEFING:
		pFolder = debug_new(CFolderMultiBriefing);
		break;

	case FOLDER_ID_MP_SUMMARY:
		pFolder = debug_new(CFolderMultiSummary);
		break;

	case FOLDER_ID_OBJECTIVES:
		pFolder = debug_new(CFolderObjectives);
		break;

	case FOLDER_ID_WEAPONS:
		pFolder = debug_new(CFolderWeapons);
		break;

	case FOLDER_ID_GADGETS:
		pFolder = debug_new(CFolderGadgets);
		break;

	case FOLDER_ID_MODS:
		pFolder = debug_new(CFolderMods);
		break;

	case FOLDER_ID_GEAR:
		pFolder = debug_new(CFolderGear);
		break;

	case FOLDER_ID_INVENTORY:
		pFolder = debug_new(CFolderInventory);
		break;

	case FOLDER_ID_VIEW_INV:
		pFolder = debug_new(CFolderViewInventory);
		break;

	case FOLDER_ID_MISSION:
		pFolder = debug_new(CFolderMission);
		break;

	case FOLDER_ID_STATS:
		pFolder = debug_new(CFolderStats);
		break;

	case FOLDER_ID_INTEL:
		pFolder = debug_new(CFolderIntel);
		break;

	case FOLDER_ID_SUMMARY:
		pFolder = debug_new(CFolderSummary);
		break;

	case FOLDER_ID_AWARDS:
		pFolder = debug_new(CFolderAwards);
		break;

	case FOLDER_ID_FAILURE:
		pFolder = debug_new(CFolderSummary);
		break;

	}

	if (pFolder)
	{
		pFolder->Init(folderID);
//		if (folderID != FOLDER_ID_DISPLAY)
//			pFolder->Build();
		m_folderArray.Add(pFolder);
	}

}

LTBOOL CFolderMgr::ForceFolderUpdate(eFolderID folderID)
{
    if (!m_pCurrentFolder || m_eCurrentFolderID != folderID) return LTFALSE;

	return m_pCurrentFolder->HandleForceUpdate();
}

// Code for equipping the player with the default junk
// Ripped from FolderObjectives
void CFolderMgr::SkipOutfitting(void)
{
	CFolderWeapons *pWeapons = (CFolderWeapons *)GetFolderFromID(FOLDER_ID_WEAPONS);
	if (pWeapons)
	{
		pWeapons->SkipOutfitting();
	}
	
	CFolderGadgets *pGadgets = (CFolderGadgets *)GetFolderFromID(FOLDER_ID_GADGETS);
	if (pGadgets)
	{
		pGadgets->SkipOutfitting();
	}
	
	CFolderMods *pMods = (CFolderMods *)GetFolderFromID(FOLDER_ID_MODS);
	if (pMods)
	{
		pMods->SkipOutfitting();
	}
	
	CFolderGear *pGear = (CFolderGear *)GetFolderFromID(FOLDER_ID_GEAR);
	if (pGear)
	{
		pGear->SkipOutfitting();
	}
}
