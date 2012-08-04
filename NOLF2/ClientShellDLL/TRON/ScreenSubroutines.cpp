// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSubroutines.cpp
//
// PURPOSE : Screen for managing player's inventory of "subroutines"
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

// todo 3/13

// Add the "Progress" meter to any subroutine with a procedural
// Build a popup window class that we can add information to
// Rotate libraries
// remove combat, defense, utility specifics and replace with array[3]
// change the Escape() in Compile() to change back to gameplay state

#include "stdafx.h"
#include "ScreenSubroutines.h"
#include "ScreenMgr.h"
#include "LayoutMgr.h"
#include "ScreenCommands.h"
#include "WinUtil.h"
#include "GameClientShell.h"
#include "ModelButeMgr.h"
#include "SubroutineMgr.h"
#include "TRONPlayerStats.h"
#include "TransitionFXMgr.h"

extern CGameClientShell* g_pGameClientShell;

namespace
{
	const int CMD_COMPILE = CMD_CUSTOM + 1;

	const float MEMORY_INNER_RADIUS = 118;
	const float MEMORY_OUTER_RADIUS = 166;

	const float LIBRARY_INNER_RADIUS = 178;
	const float LIBRARY_OUTER_RADIUS = 226;

    LTVector g_vPos, g_vU, g_vR, g_vF;

	LTIntPt CompilePos;

	typedef enum {
		LIB_STATE_NONE = 0,
		LIB_STATE_CLOSED,
		LIB_STATE_OPEN,
		LIB_STATE_OPENING,
		LIB_STATE_CLOSING
	} LibraryState;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenSubroutines::CScreenSubroutines()
{
}

CScreenSubroutines::~CScreenSubroutines()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::Build()
//
//	PURPOSE:	Build any data structures one time
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenSubroutines::Build()
{
	CreateTitle(IDS_TITLE_SUBROUTINES);

	// Add a "Compile" button
	CompilePos = g_pLayoutMgr->GetScreenCustomPoint(SCREEN_ID_SUBROUTINES,"CompilePos");
	AddTextItem(IDS_COMPILE, CMD_COMPILE, IDS_HELP_COMPILE, CompilePos);

	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE);

	LTIntPt center;
	center.x = 320;
	center.y = 240;

	m_SystemMemoryCtrl.Init(center, MEMORY_INNER_RADIUS, MEMORY_OUTER_RADIUS, 24);
	m_SystemMemoryCtrl.ShowOutlines(LTFALSE);
	m_SystemMemoryCtrl.SetArcType(ARC_TYPE_SYSTEM_MEMORY);

	m_CombatCtrl.Init(center, LIBRARY_INNER_RADIUS, LIBRARY_OUTER_RADIUS, 6);
	m_CombatCtrl.SetArcType(ARC_TYPE_LIBRARY);
	m_CombatCtrl.SetArc(45.0f, 135.0f);
	m_CombatCtrl.SetNumHighlightSegments(1);
	m_CombatCtrl.ShowOutlines(LTFALSE);

	m_DefenseCtrl.Init(center, LIBRARY_INNER_RADIUS, LIBRARY_OUTER_RADIUS, 6);
	m_DefenseCtrl.SetArcType(ARC_TYPE_LIBRARY);
	m_DefenseCtrl.SetArc(165.0f, 255.0f);
	m_DefenseCtrl.SetNumHighlightSegments(1);
	m_DefenseCtrl.ShowOutlines(LTFALSE);

	m_UtilityCtrl.Init(center, LIBRARY_INNER_RADIUS, LIBRARY_OUTER_RADIUS, 6);
	m_UtilityCtrl.SetArcType(ARC_TYPE_LIBRARY);
	m_UtilityCtrl.SetArc(285.0f, 15.0f);
	m_UtilityCtrl.SetNumHighlightSegments(1);
	m_UtilityCtrl.ShowOutlines(LTFALSE);

	m_nCombatSubs		= 0;
	m_nDefenseSubs		= 0;
	m_nUtilitySubs		= 0;

	m_nCombatLibSize	= 0;
	m_nDefenseLibSize	= 0;
	m_nUtilityLibSize	= 0;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::OnFocus
//
//	PURPOSE:	Handle setup/cleanup as player enters and exits screen
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_bDoneIntro = false;

		// Set the cursor mode to "pickup"
		m_eCursorMode = CursorPickup;
		m_SystemMemoryCtrl.SetNumHighlightSegments(-1);
		m_pHotSub	= LTNULL;
		m_pHotProc	= LTNULL;
		m_pHotStr	= LTNULL;
		m_pHotArc	= LTNULL;
		m_pCursorSub = LTNULL;
		BuildMenuPieces();
		m_pScreenMgr->GetTransitionFXMgr()->ClearScreenHistory();
		m_pScreenMgr->GetTransitionFXMgr()->SetLooping(false);

		g_pCursorMgr->UseSprite("Interface\\cursor.dtx");
		g_pCursorMgr->UseGlowSprite("Interface\\cursor_glow.dtx");
		g_pCursorMgr->SetCenter(32,32);
	}
	else
	{
		DestroyMenuPieces();
		m_pScreenMgr->GetTransitionFXMgr()->SetLooping(true);

		// Force arcs to wipe all their contents
		ClearScreen();
		g_pCursorMgr->UseSprite((CScreenSprite *)LTNULL);
		g_pCursorMgr->SetCenter(16,16);
	}

	CBaseScreen::OnFocus(bFocus);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::DestroyMenuPieces
//
//	PURPOSE:	Iterate through the array of models, and clean them up
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::DestroyMenuPieces()
{
	// iterate through m_ModelPieceArray and remove them, reset them and term them
	ModelPieceArray::iterator iter = m_ModelPieceArray.begin();
	while (iter != m_ModelPieceArray.end())
	{
		g_pInterfaceMgr->RemoveInterfaceSFX(*iter);
		(*iter)->Reset();
		(*iter)->Term();
		debug_delete(*iter);
		iter++;
	}
	m_ModelPieceArray.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::BuildMenuPieces
//
//	PURPOSE:	Request all of the BaseScaleFX used for this interface and
//				create them.
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::BuildMenuPieces()
{
	// Iterate through ScreenSubroutine and request all the pieces
	int iPieceNum = 0;

	char szAttName[30];
	char szPieceName[128] = "";

	// First step: spawn a series of clientfx for any procedurals that the player has
	ProceduralArray::iterator iter = m_ProcArray.begin();
	while (iter != m_ProcArray.end())
	{
		Procedural * pProc = *iter;
		if (pProc->szIntroFX[0])
		{
			// attempt to launch this introFX.  It's a fire-and-forget
			CLIENTFX_LINK IntroFX;
			INT_FX* pFX = g_pLayoutMgr->GetFX(pProc->szIntroFX);
			if (pFX)
			{
				g_pInterfaceMgr->AddInterfaceFX(&IntroFX, pFX->szFXName,pFX->vPos,false);
			}
		}
		++iter;
	}

	bool bFound = false;
	do
	{
		szPieceName[0] = 0;
		bFound = false;

		sprintf(szAttName,"Piece%d",iPieceNum);
		if (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID,szAttName))
		{
			bFound = true;
			g_pLayoutMgr->GetScreenCustomString((eScreenID)m_nScreenID,szAttName,szPieceName,128);

			INT_MENUPIECE * pPiece = ((CTronLayoutMgr *)(g_pLayoutMgr))->GetMenuPiece(szPieceName);
			if (pPiece)
				CreateMenuPiece(pPiece);

		}
		iPieceNum++;
	} while (bFound);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::OnCommand
//
//	PURPOSE:	Handle incoming commands from controls
//
// ----------------------------------------------------------------------- //

uint32 CScreenSubroutines::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{

	case CMD_COMPILE:
		{
			Compile();
			Escape();
			break;
		}

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::UpdateMenuPieces
//
//	PURPOSE:	Handle switching from clientfx to internally handled models
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::UpdateMenuPieces()
{
	if (m_bDoneIntro)
		return;

	CTransitionFXMgr * pMgr = m_pScreenMgr->GetTransitionFXMgr();

	// Check for the transitionFXmgr.  If there are no FX, or the FX are done,
	// then term the TransitionFX.
	if (!pMgr->HasTransitionFX())
	{
		m_bDoneIntro = true;
	}
	else if (pMgr->IsTransitionComplete())
	{
		pMgr->EndTransition();
		m_bDoneIntro = true;
	}

	// Only continue if no transition or transition is done
	if (!m_bDoneIntro)
		return;
	
	//	If there are FX ready, set them all to visible.
	ModelPieceArray::iterator iter = m_ModelPieceArray.begin();
	while (iter != m_ModelPieceArray.end())
	{
		g_pInterfaceMgr->AddInterfaceSFX(*iter, IFX_WORLD);
		iter++;
	}

	m_SystemMemoryCtrl.ShowSprites(true);
	m_SystemMemoryCtrl.SetLibraryState(LIB_STATE_CLOSED);

	m_CombatCtrl.SetLibraryState(LIB_STATE_CLOSED);

	m_DefenseCtrl.SetLibraryState(LIB_STATE_CLOSED);

	m_UtilityCtrl.SetLibraryState(LIB_STATE_CLOSED);

/* This is an old block of code from when we ran the intros ourselves instead of using
   clientfx
	uint32 dwState = 0;
	ANIMTRACKERID nTracker;
	HOBJECT hObj = m_SubModelFX.GetObject();
	g_pLTClient->GetModelLT()->GetMainTracker( hObj, nTracker);
	g_pLTClient->GetModelLT()->GetPlaybackState(hObj, nTracker, dwState);
	if (dwState & MS_PLAYDONE)
	{
		m_bDoneIntro = true;
		g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, "loop"));
	}
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::Render
//
//	PURPOSE:	Render any ClientFX, GUI controls, and the arc controls
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenSubroutines::Render(HSURFACE hDestSurf)
{
	m_SystemMemoryCtrl.Render();

	// render procedural controls
	for (int i = 0; i < 5; i++)
	{
		m_ProcCtrl[i].Render();
	}

	if (!m_bDoneIntro)	
		UpdateMenuPieces(); // See if any models need to be made visible
	else
		UpdateLibraries(); // set any animations on the three browsers

	LTBOOL lResult = CBaseScreen::Render(hDestSurf);

	if (!m_bDoneIntro)
		return lResult;

	m_CombatCtrl.Render();
	m_DefenseCtrl.Render();
	m_UtilityCtrl.Render();

	// TODO render additive libraries

	ShowHotInfo();

    return lResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::OnMouseMove
//
//	PURPOSE:	Pass on any mouse updates to the relevant controls
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenSubroutines::OnMouseMove(int x, int y)
{
	PlayerSubroutine * pOldSub	= m_pHotSub;
	CArcCtrl * pOldArc			= m_pHotArc;
	Procedural * pOldProc		= m_pHotProc;
	int iActiveSegment = 0;

	m_pHotArc = LTNULL;
	m_pHotSub = LTNULL;
	m_pHotProc = LTNULL;

	for (int i = 0; i < 5; i++)
	{
		m_ProcCtrl[i].OnMouseMove(x,y, m_pCursorSub);
		if (!m_pHotProc)
		{
			if (m_ProcCtrl[i].IsHot())
			{
				m_pHotProc = m_ProcCtrl[i].GetProc();
			}
		}
	}
	// 1.  Send the mouse move to the system memory and any open libraries
	//     Note which arc the mouse is over

	m_SystemMemoryCtrl.OnMouseMove(x,y);
	if (m_SystemMemoryCtrl.IsHot())
	{
		m_pHotArc = &m_SystemMemoryCtrl;
	}

	// Optional update to CombatCtrl
	if (m_CombatCtrl.GetLibraryState() == LIB_STATE_OPEN || m_eCursorMode == CursorPickup)
	{
		m_CombatCtrl.OnMouseMove(x,y);
		if (m_CombatCtrl.IsHot())
		{
			m_pHotArc = &m_CombatCtrl;
		}
	}

	// Optional update to DefenseCtrl
	if (m_DefenseCtrl.GetLibraryState() == LIB_STATE_OPEN || m_eCursorMode == CursorPickup)
	{
		m_DefenseCtrl.OnMouseMove(x,y);
		if (m_DefenseCtrl.IsHot())
		{
			m_pHotArc = &m_DefenseCtrl;
		}
	}

	// Optional update to UtilityCtrl
	if (m_UtilityCtrl.GetLibraryState() == LIB_STATE_OPEN || m_eCursorMode == CursorPickup)
	{
		m_UtilityCtrl.OnMouseMove(x,y);
		if (m_UtilityCtrl.IsHot())
		{
			m_pHotArc = &m_UtilityCtrl;
		}
	}

	// 2. If the mouse cursor is over an arc, then also note which subroutine
	//	  (if any) that the cursor is over.
	if (m_pHotArc)
	{
		iActiveSegment = m_pHotArc->GetActiveSegment();
		m_pHotSub = m_pHotArc->GetHotSub();
	}

	// 3. Update the browser libraries.  If one was open and needs to be closed
	//    then close it.  If a new one needs to be opened, then open it.

	if (pOldArc != m_pHotArc)
	{
		// There has been a change in "hotness".  See if a library needs to be closed
		if (pOldArc && pOldArc != &m_SystemMemoryCtrl)
		{
			// it's one of the three browsers.  If we're in "pickup mode" then close
			// the browser.  If we're carrying a subroutine, then the only browser open
			// is the one that needs to stay open anyhow.
			if (m_eCursorMode == CursorPickup)
			{
				// Close the library.  Determine which one it is.
				CloseLibrary(pOldArc);
			}
		}

		if (m_pHotArc && m_pHotArc != &m_SystemMemoryCtrl)
		{
			if (m_eCursorMode == CursorPickup)
			{
				OpenLibrary(m_pHotArc);
			}
		}
	}

	if (m_pHotSub != pOldSub || m_pHotProc != pOldProc)
	{
		KillHotInfo();

		// The heat has changed.
		if (m_pHotSub && !m_pCursorSub)
		{
			// rebuild the "hot" window
			BuildHotSubInfo();
		}
		else if (m_pHotProc && !pOldProc)
		{
			BuildHotProcInfo();
		}
	}
	return CBaseScreen::OnMouseMove(x, y);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::ClearScreen
//
//	PURPOSE:	Clear up dynamic allocations, hide sprites, wipe the control
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::ClearScreen()
{
	m_nCombatSubs = 0;
	m_nDefenseSubs = 0;
	m_nUtilitySubs = 0;

	m_nCombatLibSize = 0;
	m_nDefenseLibSize = 0;
	m_nUtilityLibSize = 0;

	m_nCombatLibIndex = 0;
	m_nDefenseLibIndex = 0;
	m_nUtilityLibIndex = 0;

	// clear the arcctrls (call their build() functions)
	m_SystemMemoryCtrl.Build();
	m_CombatCtrl.Build();
	m_DefenseCtrl.Build();
	m_UtilityCtrl.Build();

	m_SubArray.clear();
	m_ProcArray.clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::AddSubroutine
//
//	PURPOSE:	SubroutineMgr callback function.  Called for each subroutine
//				that needs to be added to system memory or a library when
//				g_pSubroutineMgr->PopulateSubroutineScreen() is called.
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::AddSubroutine(PlayerSubroutine * pSubroutine)
{
	m_SubArray.push_back(pSubroutine);
	pSubroutine->nTempSlot = pSubroutine->nSlot;

	// note if a procedural is working on the subroutine.  Procedurals will have
	// already arrived at this stage, so if we find a procedural that is working
	// on this subroutine, hide the icon.

	TronSubroutine * pTsub = pSubroutine->pTronSubroutine;

	if (pSubroutine->eState == SUBSTATE_DELETED) // this subroutine has been "shredded".  Do not add it to the interface
	{
		pSubroutine->nTempSlot = 400;
		return;
	}
	// If it's in a library, determine where it should go
	else if (pSubroutine->nTempSlot < 0 || pSubroutine->nTempSlot >= 100)
	{
		if (pTsub->eFunction == FUNCTION_COMBAT)
		{
			pSubroutine->nTempSlot = 100 + m_nCombatSubs;
			// add to screen if visible
			if (m_nCombatSubs < 6)
			{
				m_CombatCtrl.OccupySegment(m_nCombatSubs, pSubroutine);
			}
			m_nCombatSubs++;
			m_nCombatLibSize++;
		}
		else if (pTsub->eFunction == FUNCTION_DEFENSE)
		{
			pSubroutine->nTempSlot = 200 + m_nDefenseSubs;
			// add to screen if visible
			if (m_nDefenseSubs < 6)
			{
				m_DefenseCtrl.OccupySegment(m_nDefenseSubs, pSubroutine);
			}
			m_nDefenseSubs++;
			m_nDefenseLibSize++;
		}
		else if (pTsub->eFunction == FUNCTION_UTILITY)
		{
			pSubroutine->nTempSlot = 300 + m_nUtilitySubs;
			// add to screen if visible
			if (m_nUtilitySubs < 6)
			{
				m_UtilityCtrl.OccupySegment(m_nUtilitySubs, pSubroutine);
			}
			m_nUtilitySubs++;
			m_nUtilityLibSize++;
		}
	}
	else
	{
		// it's in system memory.  Place it, and add a blank spot to the library
		if (pTsub->eFunction == FUNCTION_COMBAT)
		{
			m_nCombatLibSize++;
		}
		else if (pTsub->eFunction == FUNCTION_DEFENSE)
		{
			m_nDefenseLibSize++;
		}
		else if (pTsub->eFunction == FUNCTION_UTILITY)
		{
			m_nUtilityLibSize++;
		}
		m_SystemMemoryCtrl.SetNumHighlightSegments(3 - (int)(pSubroutine->eVersion));
		m_SystemMemoryCtrl.OccupySegment(pSubroutine->nTempSlot, pSubroutine);
		m_SystemMemoryCtrl.SetNumHighlightSegments(-1);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::AddProcedural
//
//	PURPOSE:	SubroutineMgr callback function.  Called for each procedural
//				that needs to be added to the subroutine screen when
//				g_pSubroutineMgr->PopulateSubroutineScreen() is called.
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::AddProcedural(Procedural * pProcedural)
{
	// This will be an array of 5 items
	m_ProcArray.push_back(pProcedural);

	int iSlot = pProcedural->iProcSlot;

	ASSERT(iSlot >= 0 && iSlot <= 4);

	// Hook the control to its procedural
	// NOTE that we want to hook the procedural to the control only if the player has it.
	// If the player does not, then we initialize with LTNULL, which attaches the empty model/skin
	m_ProcCtrl[iSlot].Init(pProcedural);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::Compile
//
//	PURPOSE:	Takes all current information about the system memory and
//				sends it back to the SubroutineMgr.
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::Compile()
{
	// iterate through and tell g_pSubroutineMgr about what subroutine is
	// where.
	g_pSubroutineMgr->ClearPlayerSubroutines();
	PlayerSubroutineArray::iterator iter = m_SubArray.begin();
	while (iter != m_SubArray.end())
	{
		PlayerSubroutine *pSub = *iter;
		pSub->nSlot = pSub->nTempSlot;
		if (pSub->nSlot == 400) // this is a shredded sub??
		{
			if (pSub->eState == SUBSTATE_OKAY) // was it shredded on this session?
			{
				pSub->eState = SUBSTATE_DELETED;
				// TODO send a message to screen that this subroutine was shredded
			}
		}
		g_pSubroutineMgr->AddPlayerSubroutine(pSub);
		iter++;
	}

	// TODO tell g_pSubroutineMgr about the ratings and additives, too
	g_pSubroutineMgr->Compile();
}

void CScreenSubroutines::PickupSubroutine(PlayerSubroutine * pSub)
{
	// Sanity Chex
	if (!pSub)
		return;

	if (!pSub->pTronSubroutine)
		return;

	if (pSub->pTronSubroutine->eFunction == FUNCTION_BASECODE)
		return;

	m_pCursorSub = pSub;
	SetCursorMode(CursorSubroutineDrop);

	// Pickup from sysmem?
	if (pSub->nTempSlot >= 0 && pSub->nTempSlot < 100)
	{
		// Special case for Bad Blocks.  Just hide the icon on the subroutine
		// instead of clearing memory.
		if (pSub->pTronSubroutine->eFunction == FUNCTION_BADBLOCK)
		{
			m_SystemMemoryCtrl.ShowIcon(pSub->nTempSlot, false);
		}
		else
		{
			m_SystemMemoryCtrl.ClearSegment(m_pHotSub->nTempSlot);
		}

		// Pickup triggers opening and closing of libraries.
		CloseLibrary(&m_CombatCtrl);
		CloseLibrary(&m_DefenseCtrl);
		CloseLibrary(&m_UtilityCtrl);
		
		switch (m_pHotSub->pTronSubroutine->eFunction)
		{
		case FUNCTION_COMBAT:
			OpenLibrary(&m_CombatCtrl);
			break;

		case FUNCTION_DEFENSE:
			OpenLibrary(&m_DefenseCtrl);
			break;

		case FUNCTION_UTILITY:
			OpenLibrary(&m_UtilityCtrl);
			break;

		default:
			break;
		}
	}

	// Pickup was from a library.  No change to open/close states
	// Pickup from combatlib?
	else if (pSub->nTempSlot >= 100 && pSub->nTempSlot < 200)
	{
		m_CombatCtrl.ClearSegment(pSub->nTempSlot - 100);
	}
	// Pickup from defenselib?
	else if (pSub->nTempSlot >= 200 && pSub->nTempSlot < 300)
	{
		m_DefenseCtrl.ClearSegment(pSub->nTempSlot - 200);
	}
	// Pickup from utilitylib?
	else if (pSub->nTempSlot >= 300 && pSub->nTempSlot < 400)
	{
		m_UtilityCtrl.ClearSegment(pSub->nTempSlot - 300);
	}

	// Set the drop cursor size, and change state
	m_SystemMemoryCtrl.SetNumHighlightSegments(3 - int(m_pHotSub->eVersion));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::OnPickupClick
//									OnAdditiveClick
//									OnSubroutineClick
//									OnPickupRClick
//									OnAdditiveRClick
//									OnSubroutineRClick
//
//	PURPOSE:	Handlers for clicking left and right mouse buttons in each
//				of the three cursor states
//
// ----------------------------------------------------------------------- //

// left-click in pickup mode
void CScreenSubroutines::OnPickupClick(int x, int y)
{
	// Clicked on a subroutine in a browser or system memory
	if (m_pHotSub)
	{
		if (m_pHotSub->pTronSubroutine->eFunction != FUNCTION_BASECODE)
		{
			PickupSubroutine(m_pHotSub);

			if (m_pHotArc)
				m_pHotArc->OnMouseMove(x, y);

			// The spot that the mouse was over is now empty, so update hot text to reflect this change
			KillHotInfo();
			m_pHotSub = LTNULL;

			return;
		}
	}

	// Clicked on a procedural
	else if (m_pHotProc)
	{
		// Procedural was working on a subroutine?
		if (m_pHotProc->pSub)
		{
			// pick up sub
			// disconnect from proc
			// update hot text
		}
	}

	// TODO clicked on an additive
	// TODO clicked on an arrow for a performance rating
	else
	{
		//  anywhere else, drop to base class
		CBaseScreen::OnLButtonDown(x,y);
	}
}

// left click in additive dragging mode
void CScreenSubroutines::OnAdditiveClick(int x, int y)
{
}

// left click in subroutine dragging mode
void CScreenSubroutines::OnSubroutineClick(int x, int y)
{
	// Dropping on a procedural?
	if (m_pHotProc)
	{
		// Dismiss if this procedural won't work on the current cursor/subroutine
		if (!g_pSubroutineMgr->CanProcHookSub(m_pHotProc, m_pCursorSub))
			return;

		for (int i = 0; i < 5; i++)
		{
			if (m_ProcCtrl[i].GetProc() == m_pHotProc)
			{
				m_ProcCtrl[i].AddSub(m_pCursorSub);
			}
		}

		g_pSubroutineMgr->HookProcToSub(m_pHotProc, m_pCursorSub);

		// drop the subroutine back where it came from, unless it was a bad block.
		// If it was a bad block, then we technically never picked it up.
		if (m_pCursorSub->eState != SUBSTATE_UNUSABLE)
		{
			PutSubroutineBack(false);
		}

		SetCursorMode(CursorPickup);
		
		// close the library associated with this subroutine
		CloseLibrary(&m_CombatCtrl);
		CloseLibrary(&m_DefenseCtrl);
		CloseLibrary(&m_UtilityCtrl);

		return;
	}

	if (m_SystemMemoryCtrl.IsOnMe(x,y))
	{
		// Easy exit case first.  If the player is holding a bad block, then the only
		// place that they can drop it is back on itself.
		if (m_pCursorSub->eState == SUBSTATE_UNUSABLE)
		{
			if (m_SystemMemoryCtrl.GetActiveSegment() == m_pCursorSub->nTempSlot)
			{
				PutSubroutineBack();
				SetCursorMode(CursorPickup);
			}
			return;
		}

		PlayerSubroutine * pOccupied = LTNULL;
		int numSegs = 3 - int(m_pCursorSub->eVersion);
		for (int i = 0; i < numSegs; i++)
		{
			int curSeg = (m_SystemMemoryCtrl.GetActiveSegment() + i) % 24;
			PlayerSubroutine * pObj = (PlayerSubroutine *)m_SystemMemoryCtrl.GetSegmentObject(curSeg);
			if (pObj)
			{
				if (pObj->eState == SUBSTATE_UNUSABLE)
				{
					// instant exit case.  Play an error sound
					return;
				}
				if (pObj != pOccupied)
				{
					if (!pOccupied)
					{
						pOccupied = pObj;
					}
					else
					{
						// exit case.  Play an error sound
						return;
					}
				}
			} // if pObj
		} // for

		// Note where the new sub should drop.  Do this here because clearing a segment
		// will also set ActiveSegment to -1
		m_pCursorSub->nTempSlot = m_SystemMemoryCtrl.GetActiveSegment();

		// if there was a single occupancy, remove it from system memory
		if (pOccupied)
		{
			m_SystemMemoryCtrl.ClearSegment(pOccupied->nTempSlot);
		}

		m_SystemMemoryCtrl.OccupySegment(m_pCursorSub->nTempSlot, m_pCursorSub);
		SetCursorMode(CursorPickup);
		CloseLibrary(&m_CombatCtrl);
		CloseLibrary(&m_DefenseCtrl);
		CloseLibrary(&m_UtilityCtrl);

		// Was a swap? Set cursor back to "sub drop" mode
		if (pOccupied)
		{
			m_pCursorSub = pOccupied;
			m_pCursorSub->nTempSlot = -1;
			SetCursorMode(CursorSubroutineDrop);
			m_SystemMemoryCtrl.SetNumHighlightSegments(3 - m_pCursorSub->eVersion);

			if (m_pCursorSub->pTronSubroutine->eFunction == FUNCTION_COMBAT)
				OpenLibrary(&m_CombatCtrl);
			else if (m_pCursorSub->pTronSubroutine->eFunction == FUNCTION_DEFENSE)
				OpenLibrary(&m_DefenseCtrl);
			else if (m_pCursorSub->pTronSubroutine->eFunction == FUNCTION_UTILITY)
				OpenLibrary(&m_UtilityCtrl);
		}
		return;
	}

	//  on the optimizer
	//    ????????????????????/
	// PutSubroutineBack()?
	//    Drop the subroutine back where it was, with the optimizer pointing to it...

	if (m_CombatCtrl.IsOnMe(x,y) && m_CombatCtrl.GetLibraryState() == LIB_STATE_OPEN)
	{
		int iSlot = m_CombatCtrl.GetActiveSegment();
		PlayerSubroutine * pSub = m_CombatCtrl.GetHotSub();

		// If over another subroutine, swap it
		if (pSub)
		{
			m_CombatCtrl.ClearSegment(iSlot);
		}
		// drop the subroutine in this slot
		m_pCursorSub->nTempSlot = 100 + iSlot;

		m_CombatCtrl.OccupySegment(iSlot, m_pCursorSub);
		SetCursorMode(CursorPickup);
		if (pSub)
		{
			m_pCursorSub = pSub;
			pSub->nTempSlot = -1;
			SetCursorMode(CursorSubroutineDrop);
			m_SystemMemoryCtrl.SetNumHighlightSegments(3 - pSub->eVersion);
		}
		return;
	}

	if (m_DefenseCtrl.IsOnMe(x,y) && m_DefenseCtrl.GetLibraryState() == LIB_STATE_OPEN)
	{
		int iSlot = m_DefenseCtrl.GetActiveSegment();
		PlayerSubroutine * pSub = m_DefenseCtrl.GetHotSub();

		// If over another subroutine, swap it
		if (pSub)
		{
			m_DefenseCtrl.ClearSegment(iSlot);
		}
		// drop the subroutine in this slot
		m_pCursorSub->nTempSlot = 200 + iSlot;

		m_DefenseCtrl.OccupySegment(iSlot, m_pCursorSub);
		SetCursorMode(CursorPickup);
		if (pSub)
		{
			m_pCursorSub = pSub;
			pSub->nTempSlot = -1;
			SetCursorMode(CursorSubroutineDrop);
			m_SystemMemoryCtrl.SetNumHighlightSegments(3 - pSub->eVersion);
		}
		return;
	}

	if (m_UtilityCtrl.IsOnMe(x,y) && m_UtilityCtrl.GetLibraryState() == LIB_STATE_OPEN)
	{
		int iSlot = m_UtilityCtrl.GetActiveSegment();
		PlayerSubroutine * pSub = m_UtilityCtrl.GetHotSub();

		// If over another subroutine, swap it
		if (pSub)
		{
			m_UtilityCtrl.ClearSegment(iSlot);
		}
		// drop the subroutine in this slot
		m_pCursorSub->nTempSlot = 300 + iSlot;

		m_UtilityCtrl.OccupySegment(iSlot, m_pCursorSub);
		SetCursorMode(CursorPickup);
		if (pSub)
		{
			m_pCursorSub = pSub;
			pSub->nTempSlot = -1;
			SetCursorMode(CursorSubroutineDrop);
			m_SystemMemoryCtrl.SetNumHighlightSegments(3 - pSub->eVersion);
		}
		return;
	}
	//  anywhere else
	//    cancel
}

// right click in pickup mode
void CScreenSubroutines::OnPickupRClick(int x, int y)
{
	// over an active procedural, cancel procedural
	if (m_pHotProc)
	{
		PlayerSubroutine * pSub = m_pHotProc->pSub;

		if (pSub)
		{
			for (int i = 0; i < 5; i++)
			{
				if (m_ProcCtrl[i].GetProc() == m_pHotProc)
				{
					m_ProcCtrl[i].RemoveSub();
				}
			}
			g_pSubroutineMgr->HookProcToSub(m_pHotProc, LTNULL);
			m_pCursorSub = pSub;
			PutSubroutineBack();
		}
		return;
	}

	// over a subroutine, if a procedural is working on it, disconnect the procedural	
	if (m_pHotSub)
	{
		Procedural * pProc = g_pSubroutineMgr->WhoHookedToSub(m_pHotSub);
		if (pProc)
		{
			g_pSubroutineMgr->HookProcToSub(pProc, LTNULL);
		}
	}
	//  anywhere else
	//    cancel
}

// right click in additive dragging mode
void CScreenSubroutines::OnAdditiveRClick(int x, int y)
{
}

// right click in subroutine dragging mode
void CScreenSubroutines::OnSubroutineRClick(int x, int y)
{
	if (m_pHotArc)
	{
		m_pHotArc->OnMouseMove(-1,-1);
	}
	PutSubroutineBack();
	CloseLibrary(&m_CombatCtrl);
	CloseLibrary(&m_DefenseCtrl);
	CloseLibrary(&m_UtilityCtrl);
	SetCursorMode(CursorPickup);
	if (m_pHotArc)
	{
		m_pHotArc->OnMouseMove(x, y);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::OnLButtonDown
//
//	PURPOSE:	Handle presses of left mouse button
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenSubroutines::OnLButtonDown(int x, int y)
{
	// Precedence goes to the library rotation buttons
	switch (m_eCursorMode)
	{
	case CursorPickup:
		{
			OnPickupClick(x,y);
			return LTTRUE;
		}
		break;

	case CursorAdditiveDrop:
		{
			OnAdditiveClick(x,y);
			return LTTRUE;
		}

	case CursorSubroutineDrop:
		{
			OnSubroutineClick(x,y);
			return LTTRUE;
		}
		break;
	}

	// Should never reach this line
	ASSERT(0);
	return CBaseScreen::OnLButtonDown(x,y);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::OnRButtonDown
//
//	PURPOSE:	Handle presses of right mouse button
//
// ----------------------------------------------------------------------- //

LTBOOL CScreenSubroutines::OnRButtonDown(int x, int y)
{
	switch (m_eCursorMode)
	{
	case CursorPickup:
		{
			OnPickupRClick(x,y);
			return LTTRUE;
		}
		break;

	case CursorAdditiveDrop:
		{
			OnAdditiveRClick(x,y);
			return LTTRUE;
		}
		break;

	case CursorSubroutineDrop:
		{
			OnSubroutineRClick(x,y);
			return LTTRUE;
		}
		break;
	}

	// Impossible case
	ASSERT(0);
	return CBaseScreen::OnRButtonDown(x,y);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::PutSubroutineBack
//
//	PURPOSE:	Find the appropriate place to put the subroutine attached
//				to the cursor
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::PutSubroutineBack(bool bKeepIcon)
{
	int i;

	if (!m_pCursorSub)
		return;

	if (m_pCursorSub->eState == SUBSTATE_UNUSABLE)
	{
		m_SystemMemoryCtrl.ShowIcon(m_pCursorSub->nTempSlot,true);
		return;
	}

	// first check the nSlot.  If it's 0-100, then our job is easy
	if (m_pCursorSub->nTempSlot >= 0 && m_pCursorSub->nTempSlot < 24)
	{
		m_SystemMemoryCtrl.OccupySegment(m_pCursorSub->nTempSlot, m_pCursorSub);
		if (bKeepIcon == false)
		{
			m_SystemMemoryCtrl.ShowIcon(m_pCursorSub->nTempSlot, false);
		}
		return;
	}

	// if not, then note the type
	bool bFilled[100];
	for (i = 0; i < 100; i++)
		bFilled[i] = false;

	int iOffset = 1000;
	if (m_pCursorSub->pTronSubroutine->eFunction == FUNCTION_COMBAT)
		iOffset = 100;
	else if (m_pCursorSub->pTronSubroutine->eFunction == FUNCTION_DEFENSE)
		iOffset = 200;
	else if (m_pCursorSub->pTronSubroutine->eFunction == FUNCTION_UTILITY)
		iOffset = 300;

	// iterate through all the subroutines of that type and mark their spots
	// as filled.
	PlayerSubroutineArray::iterator iter = m_SubArray.begin();
	while (iter != m_SubArray.end())
	{
		if (m_pCursorSub != *iter)
		{
			if (m_pCursorSub->pTronSubroutine->eFunction == (*iter)->pTronSubroutine->eFunction)
			{
				int nSlot = (*iter)->nTempSlot - iOffset;
				if (nSlot >= 0 && nSlot < 100)
					bFilled[nSlot] = true;
			}
		}
		iter++;
	}


	i = 0;
	// then go through the list bFilled until we find an open spot.
	// add the subroutine to the library in that spot.
	while (i < 100)
	{
		if (!bFilled[i])
		{
			m_pCursorSub->nTempSlot = i+iOffset;
			// We found a spot.  Add the subroutine to the correct library
			switch(m_pCursorSub->pTronSubroutine->eFunction)
			{
			case FUNCTION_COMBAT:
				{
					m_CombatCtrl.OccupySegment(i, m_pCursorSub);
					if (bKeepIcon == false)
					{
						m_CombatCtrl.ShowIcon(i, false);
					}
				}
				break;

			case FUNCTION_DEFENSE:
				{
					m_DefenseCtrl.OccupySegment(i, m_pCursorSub);
					if (bKeepIcon == false)
					{
						m_DefenseCtrl.ShowIcon(i, false);
					}
				}
				break;

			case FUNCTION_UTILITY:
				{
					m_UtilityCtrl.OccupySegment(i, m_pCursorSub);
					if (bKeepIcon == false)
					{
						m_UtilityCtrl.ShowIcon(i, false);
					}
				}
				break;

			default:
				{
					// Impossible case.
					ASSERT(0);
				}
				break;
			}
			return;
		}
		i++;
	}
	m_pCursorSub->nTempSlot = -1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::SetCursorMode
//
//	PURPOSE:	Handle state-change for the cursor
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::SetCursorMode(SubCursorMode mode)
{
	int i;

	// if old and new mode match, exit.
	if (m_eCursorMode == mode)
		return;

	// Do cleanup on the old mode
	switch (m_eCursorMode)
	{
	case CursorPickup:
		{
		// nothing to do here
		}
		break;

	case CursorAdditiveDrop:
		{
		}
		break;

	case CursorSubroutineDrop:
		{
			// restore the cursor
			g_pCursorMgr->UseSprite("Interface\\cursor.dtx");
			g_pCursorMgr->UseGlowSprite("Interface\\cursor_glow.dtx");
			g_pCursorMgr->SetCenter(32,32);

			// reactivate all procedurals
			// open all the libraries
			m_SystemMemoryCtrl.SetNumHighlightSegments(-1);

			// clear all "inactive" from libraries
			for (i = 0; i < 6; i++)
			{
				m_CombatCtrl.IgnoreSegment(i,false);
				m_DefenseCtrl.IgnoreSegment(i, false);
				m_UtilityCtrl.IgnoreSegment(i, false);
			}
		}
		break;

	default:
		{
		ASSERT(0);
		}
		break;
	}

	// Set up the new mode

	switch (mode)
	{
	case CursorPickup:
		{
			m_pCursorSub = LTNULL;
			// TODO set cursor to the default sprite
		}
		break;

	case CursorAdditiveDrop:
		{
		}
		break;

	case CursorSubroutineDrop:
		{
			if (m_pCursorSub->eState != SUBSTATE_UNUSABLE) // no halo for bad blocks
			{
				g_pCursorMgr->UseSprite(szRingTex[m_pCursorSub->eVersion]);
				g_pCursorMgr->UseBackgroundSprite(m_pCursorSub->pTronSubroutine->szSprite);
			}
			else // bad block
			{
				g_pCursorMgr->UseSprite(m_pCursorSub->pTronSubroutine->szSprite);
			}
			g_pCursorMgr->UseGlowSprite("Interface\\cursor_subglow.dtx");
			// TODO close the irrelevant libraries
		}
		break;

	default:
		{
		ASSERT(0);
		}
		break;
	}
	m_eCursorMode = mode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenSubroutines::RotateLibrary
//
//	PURPOSE:	Handle rotation of one of the three libraries
//
// ----------------------------------------------------------------------- //

void CScreenSubroutines::RotateLibrary(SubFunction lib, bool bClockwise)
{
	CArcCtrl * pCtrl;

	// Which library?
	if (lib == FUNCTION_COMBAT)
		pCtrl = &m_CombatCtrl;
	else if (lib == FUNCTION_DEFENSE)
		pCtrl = &m_DefenseCtrl;
	else if (lib == FUNCTION_UTILITY)
		pCtrl = &m_UtilityCtrl;
	else
	{
		ASSERT(0); // impossible situation
		return;
	}
	// TODO code the animation or non-animation of rotating the appropriate library
}

void CScreenSubroutines::UpdateLibraries()
{
	uint32 dwState = 0;
	ANIMTRACKERID nTracker;
	HOBJECT hObj = LTNULL;

	LibraryState eState = (LibraryState)m_CombatCtrl.GetLibraryState();

	if (eState == LIB_STATE_OPENING || eState == LIB_STATE_CLOSING)
	{
		hObj = m_CombatCtrl.GetHObject();
		g_pLTClient->GetModelLT()->GetMainTracker( hObj, nTracker);
		g_pLTClient->GetModelLT()->GetPlaybackState(hObj, nTracker, dwState);
		if (dwState & MS_PLAYDONE)
		{
			if (eState == LIB_STATE_OPENING)
			{
				m_CombatCtrl.SetLibraryState(LIB_STATE_OPEN);
				m_CombatCtrl.ShowSprites(true);
				g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, "open_top"));
			}
			else if (eState == LIB_STATE_CLOSING)
			{
				m_CombatCtrl.SetLibraryState(LIB_STATE_CLOSED);
				g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, "closed_top"));
			}
		}
	}

	eState = (LibraryState)m_DefenseCtrl.GetLibraryState();
	if (eState == LIB_STATE_OPENING || eState == LIB_STATE_CLOSING)
	{
		hObj = m_DefenseCtrl.GetHObject();
		g_pLTClient->GetModelLT()->GetMainTracker( hObj, nTracker);
		g_pLTClient->GetModelLT()->GetPlaybackState(hObj, nTracker, dwState);
		if (dwState & MS_PLAYDONE)
		{
			if (eState == LIB_STATE_OPENING)
			{
				m_DefenseCtrl.SetLibraryState(LIB_STATE_OPEN);
				m_DefenseCtrl.ShowSprites(true);
				g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, "open_left"));
			}
			else if (eState == LIB_STATE_CLOSING)
			{
				m_DefenseCtrl.SetLibraryState(LIB_STATE_CLOSED);
				g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, "closed_left"));
			}
		}
	}

	eState = (LibraryState)m_UtilityCtrl.GetLibraryState();
	if (eState == LIB_STATE_OPENING || eState == LIB_STATE_CLOSING)
	{
		hObj = m_UtilityCtrl.GetHObject();
		g_pLTClient->GetModelLT()->GetMainTracker( hObj, nTracker);
		g_pLTClient->GetModelLT()->GetPlaybackState(hObj, nTracker, dwState);
		if (dwState & MS_PLAYDONE)
		{
			if (eState == LIB_STATE_OPENING)
			{
				m_UtilityCtrl.SetLibraryState(LIB_STATE_OPEN);
				m_UtilityCtrl.ShowSprites(true);
				g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, "open_right"));
			}
			else if (eState == LIB_STATE_CLOSING)
			{
				m_UtilityCtrl.SetLibraryState(LIB_STATE_CLOSED);
				g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, "closed_right"));
			}
		}
	}
}

void CScreenSubroutines::OpenLibrary(CArcCtrl * pCtrl)
{
	HOBJECT hObj = LTNULL;
	char * szOpen;

	if (!pCtrl)
		return;

	if (pCtrl == &m_CombatCtrl)
	{
		szOpen = "expand_top";
	}
	else if (pCtrl == &m_DefenseCtrl)
	{
		szOpen = "expand_left";
	}
	else if (pCtrl == &m_UtilityCtrl)
	{
		szOpen = "expand_right";
	}
	else
	{
		// invalid library!
		return;
	}

	hObj = pCtrl->GetHObject();

	if (!hObj)
	{
		g_pLTClient->CPrint("ERROR OpenLibrary: no hObject\n");
		return;
	}

	LibraryState eState = (LibraryState)pCtrl->GetLibraryState();

	if (eState == LIB_STATE_OPENING || eState == LIB_STATE_OPEN)
	{
		return;
	}

	g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, szOpen));
	pCtrl->SetLibraryState(LIB_STATE_OPENING);
}


void CScreenSubroutines::CloseLibrary(CArcCtrl * pCtrl)
{
	HOBJECT hObj = LTNULL;
	char * szClose;
	char * szClosed;

	if (!pCtrl)
		return;

	if (m_pHotArc == pCtrl)
		return;

	// Which library?
	if (pCtrl == &m_CombatCtrl)
	{
		szClose = "collapse_top";
		szClosed = "closed_top";
	}
	else if (pCtrl == &m_DefenseCtrl)
	{
		szClose = "collapse_left";
		szClosed = "closed_left";
	}
	else if (pCtrl == &m_UtilityCtrl)
	{
		szClose = "collapse_right";
		szClosed = "closed_right";
	}
	else
	{
		// invalid library!
		return;
	}

	pCtrl->ShowSprites(false);

	hObj = pCtrl->GetHObject();

	if (!hObj)
		return;

	LibraryState eState = (LibraryState)pCtrl->GetLibraryState();

	if (eState == LIB_STATE_CLOSING || eState == LIB_STATE_CLOSED)
		return;

	if (eState == LIB_STATE_OPENING)
	{
		g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, szClosed));
		pCtrl->SetLibraryState(LIB_STATE_CLOSED);
	}
	else if (eState == LIB_STATE_OPEN)
	{
		g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, szClose));
		pCtrl->SetLibraryState(LIB_STATE_CLOSING);
	}
}


void CScreenSubroutines::BuildHotSubInfo()
{
	if (!m_pHotSub)
		return;

	if (!m_pHotSub->pTronSubroutine)
		return;

	CUIFont * pFont = g_pInterfaceResMgr->GetFont(1);

	char buf[1024];

	TronSubroutine * pSub = m_pHotSub->pTronSubroutine;

	Procedural * pProc = LTNULL;
	for (int i = 0; i < 5; i++)
	{
		if (m_ProcCtrl[i].GetProc())
		{
			if (m_ProcCtrl[i].GetProc()->pSub == m_pHotSub)
			{
				pProc = m_ProcCtrl[i].GetProc();
			}
		}
	}

    HSTRING hNameStr = g_pLTClient->FormatString(pSub->nNameId);
	HSTRING hDescriptionStr = g_pLTClient->FormatString(pSub->nDescriptionId);

	if (pProc)
	{
		HSTRING hProcNameStr = g_pLTClient->FormatString(pProc->nNameId);
		sprintf(buf, "%s\n\n%s\n\nWorked on by %s (%d%%)",
					g_pLTClient->GetStringData(hNameStr),
					g_pLTClient->GetStringData(hDescriptionStr),
					g_pLTClient->GetStringData(hProcNameStr),
					(int)(m_pHotSub->fPercentDone * 100.0f));
		g_pLTClient->FreeString(hProcNameStr);
	}
	else
	{
		sprintf(buf, "%s\n\n%s", g_pLTClient->GetStringData(hNameStr), g_pLTClient->GetStringData(hDescriptionStr));
	}
	g_pLTClient->FreeString(hNameStr);
	g_pLTClient->FreeString(hDescriptionStr);

	m_pHotStr = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont, buf);
	m_pHotStr->SetColor(0xFFFFFFFF);
	m_pHotStr->SetAlignmentH(CUI_HALIGN_CENTER);
	m_pHotStr->SetWrapWidth(250);

	// Center the string vertically
	float fWidth, fHeight;
	m_pHotStr->GetDims(&fWidth, &fHeight);

	float x = (float)g_pInterfaceResMgr->GetScreenWidth() * 0.5f;
	float y = (float)g_pInterfaceResMgr->GetScreenHeight() * 0.5f;

	y-= (fHeight * 0.5f);

	m_pHotStr->SetPosition(x, y);
}

void CScreenSubroutines::BuildHotProcInfo()
{
	if (!m_pHotProc)
		return;

	CUIFont * pFont = g_pInterfaceResMgr->GetFont(1);

	char buf[1024];

	Procedural * pProc = m_pHotProc;

    HSTRING hNameStr = g_pLTClient->FormatString(pProc->nNameId);
	HSTRING hDescriptionStr = g_pLTClient->FormatString(pProc->nDescriptionId);

	if (pProc->pSub)
	{
		HSTRING hSubNameStr = g_pLTClient->FormatString(pProc->pSub->pTronSubroutine->nNameId);
		sprintf(buf,"%s\n\n%s\n\nWorking on %s (%d%%)",
				g_pLTClient->GetStringData(hNameStr),
				g_pLTClient->GetStringData(hDescriptionStr),
				g_pLTClient->GetStringData(hSubNameStr),
				(int)(pProc->pSub->fPercentDone * 100.0f));
		g_pLTClient->FreeString(hSubNameStr);
	}
	else
	{
		sprintf(buf, "%s\n\n%s", g_pLTClient->GetStringData(hNameStr), g_pLTClient->GetStringData(hDescriptionStr));
	}

	g_pLTClient->FreeString(hNameStr);
	g_pLTClient->FreeString(hDescriptionStr);

	m_pHotStr = g_pLTClient->GetFontManager()->CreateFormattedPolyString(pFont, buf);
	m_pHotStr->SetColor(0xFFFFFFFF);
	m_pHotStr->SetAlignmentH(CUI_HALIGN_CENTER);
	m_pHotStr->SetWrapWidth(250);

	// Center the string vertically
	float fWidth, fHeight;
	m_pHotStr->GetDims(&fWidth, &fHeight);

	float x = (float)g_pInterfaceResMgr->GetScreenWidth() * 0.5f;
	float y = (float)g_pInterfaceResMgr->GetScreenHeight() * 0.5f;

	y-= (fHeight * 0.5f);

	m_pHotStr->SetPosition(x, y);
}

void CScreenSubroutines::ShowHotInfo()
{
	if (m_pHotStr)
		m_pHotStr->Render();
}

void CScreenSubroutines::KillHotInfo()
{
	if (m_pHotStr)
	{
		g_pLTClient->GetFontManager()->DestroyPolyString(m_pHotStr);
		m_pHotStr = LTNULL;
	}
}


void CScreenSubroutines::CreateMenuPiece(INT_MENUPIECE * pSubModel)
{
	if (!pSubModel)
		return;

	// Get some vars
	HOBJECT hCamera = g_pInterfaceMgr->GetInterfaceCamera();
	if (!hCamera) return;

    g_pLTClient->GetObjectPos(hCamera, &g_vPos);

	LTRotation rRot;
    g_pLTClient->GetObjectRotation(hCamera, &rRot);
	g_vU = rRot.Up();
	g_vR = rRot.Right();
	g_vF = rRot.Forward();

	BSCREATESTRUCT bcs;
	LTVector vPos, vTemp, vScale(1.0f,1.0f,1.0f);

	char modName[128];

	SAFE_STRCPY(modName, pSubModel->szModel);

	VEC_COPY(vPos,g_vPos);
	VEC_SET(vScale,1.0f,1.0f,1.0f);

	LTVector vModPos = pSubModel->vPos;

	LTFLOAT fRot = 0.0f;
	fRot  = MATH_PI + DEG2RAD(fRot);
	rRot.Rotate(g_vU, fRot);

	VEC_MULSCALAR(vTemp, g_vF, vModPos.z);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_MULSCALAR(vTemp, g_vR, vModPos.x);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_MULSCALAR(vTemp, g_vU, vModPos.y);
	VEC_ADD(vPos, vPos, vTemp);

	VEC_COPY(bcs.vPos, vPos);
	bcs.rRot = rRot;
	VEC_COPY(bcs.vInitialScale, vScale);
	VEC_COPY(bcs.vFinalScale, vScale);
	VEC_SET(bcs.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(bcs.vFinalColor, 1.0f, 1.0f, 1.0f);
	bcs.bUseUserColors = LTTRUE;

	bcs.pFilename = modName;
	bcs.pSkinReader = &(pSubModel->blrSkins);
	bcs.pRenderStyleReader = &(pSubModel->blrRenderStyles);
	bcs.dwFlags = FLAG_VISIBLE | FLAG_FOGDISABLE;// | FLAG_NOLIGHT;

	bcs.nType = OT_MODEL;
	bcs.fInitialAlpha = 0.99f;
	bcs.fFinalAlpha = 0.99f;
	bcs.fLifeTime = 1000000.0f;
	bcs.bLoop = LTFALSE;			// all models start out as non-looping

	bcs.fMinRotateVel = 1.0f;
	bcs.fMaxRotateVel = 1.0f;

	CBaseScaleFX * pPiece = debug_new(CBaseScaleFX);
	ASSERT(pPiece != LTNULL);

	if (pPiece->Init(&bcs))
	{
		pPiece->CreateObject(g_pLTClient);
		HOBJECT hObj = pPiece->GetObject();
		if (hObj)
		{
			if (pSubModel->szLoopAnim)
			{
				g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, pSubModel->szLoopAnim));
			}
			else
			{
				g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, "loop"));
			}
			m_ModelPieceArray.push_back(pPiece);

			// Note the extra functionality of this piece, if any, and assign it to a library
			if (pSubModel->szFunction[0])
			{
				if (!strcmpi(pSubModel->szFunction, "hub"))
				{
					m_SystemMemoryCtrl.AssociateHObject(hObj, "skt_hub");
					g_pInterfaceMgr->AddInterfaceSFX(pPiece, IFX_WORLD);
					g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, "intro"));
				}
				else if (!strcmpi(pSubModel->szFunction, "browser_top"))
				{
					m_CombatCtrl.AssociateHObject(hObj, LTNULL);
				}
				else if (!strcmpi(pSubModel->szFunction, "browser_left"))
				{
					m_DefenseCtrl.AssociateHObject(hObj, LTNULL);
				}
				else if (!strcmpi(pSubModel->szFunction, "browser_right"))
				{
					m_UtilityCtrl.AssociateHObject(hObj, LTNULL);
				}
				else if (!strcmpi(pSubModel->szFunction, "proc_dock"))
				{
					for (int i = 0; i < 5; i++)
					{
						char buf[20];
						sprintf(buf,"skt_%d",i);
						m_ProcCtrl[i].AssociateHObject(hObj, buf);
						g_pInterfaceMgr->AddInterfaceSFX(pPiece, IFX_WORLD);
						g_pLTClient->SetModelAnimation(hObj, g_pLTClient->GetAnimIndex(hObj, "intro"));
					}
				}
			}
			return;
		}
	}
	// Fail case.  Should only be reached if no object was created
	debug_delete(pPiece);
}
