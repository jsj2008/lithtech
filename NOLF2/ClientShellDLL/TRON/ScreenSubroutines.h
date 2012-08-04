// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenSubroutines.h
//
// PURPOSE : Screen for managing player's inventory of "subroutines"
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef _SCREEN_SUBROUTINES_H_
#define _SCREEN_SUBROUTINES_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "BaseScreen.h"
#include "BaseScaleFX.h"
#include "ArcCtrl.h"
#include "ProceduralCtrl.h"
#include "SubroutineMgr.h"

const int NUM_SECTORS = 24;
const int PRIMS_PER_SECTOR = 3;

const float SLICE_SIZE = MATH_PI * 2.0f / (float)NUM_SECTORS;
const float PRIM_SIZE = SLICE_SIZE / PRIMS_PER_SECTOR;

typedef enum
{
	CursorPickup = 0,
	CursorSubroutineDrop,
	CursorAdditiveDrop
} SubCursorMode;


class CScreenSubroutines : public CBaseScreen
{
public:
	CScreenSubroutines();
	virtual ~CScreenSubroutines();

	// Build the screen
    LTBOOL   Build();

	// This is called when the screen gets or loses focus
    virtual void    OnFocus(LTBOOL bFocus);

	LTBOOL		OnMouseMove(int x, int y);
	LTBOOL		OnLButtonDown(int x, int y);
	LTBOOL		OnRButtonDown(int x, int y);

	// Used by the SubroutineMgr
	void		ClearScreen();
	void		AddSubroutine(PlayerSubroutine * pSubroutine);
	void		AddProcedural(Procedural * pProcedural);

	void		Compile();

protected:

    uint32		OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2);
    LTBOOL		Render(HSURFACE hDestSurf);

	void		SetCursorMode(SubCursorMode mode);
	void		UpdateSpriteVisibility();

	void		PickupSubroutine(PlayerSubroutine * pSub);
	void		PutSubroutineBack(bool bKeepIcon = true);

	// mouse button handlers
	void		OnPickupClick(int x, int y);
	void		OnSubroutineClick(int x, int y);
	void		OnAdditiveClick(int x, int y);
	void		OnPickupRClick(int x, int y);
	void		OnSubroutineRClick(int x, int y);
	void		OnAdditiveRClick(int x, int y);

	void		UpdateLibraries();
	void		OpenLibrary(CArcCtrl * pCtrl);
	void		CloseLibrary(CArcCtrl * pCtrl);

	void		RotateLibrary(SubFunction lib, bool bClockwise);

	void		BuildHotSubInfo();
	void		BuildHotProcInfo();
	void		ShowHotInfo();
	void		KillHotInfo();

	void		BuildMenuPieces();
	void		DestroyMenuPieces();

	void		UpdateMenuPieces();
	void		CreateMenuPiece(INT_MENUPIECE * pSubModel);

	int			m_nCombatLibSize;
	int			m_nDefenseLibSize;
	int			m_nUtilityLibSize;

	int			m_nCombatLibIndex;
	int			m_nDefenseLibIndex;
	int			m_nUtilityLibIndex;

	int			m_nCombatSubs;
	int			m_nDefenseSubs;
	int			m_nUtilitySubs;

	bool		m_bDoneIntro;	// flag for when to play intro
	bool		m_bDoneOutro;	// flag for completion of outro

	SubCursorMode	m_eCursorMode;
	PlayerSubroutineArray m_SubArray;
	ProceduralArray m_ProcArray;

	CArcCtrl	m_SystemMemoryCtrl;
	CArcCtrl	m_CombatCtrl;
	CArcCtrl	m_DefenseCtrl;
	CArcCtrl	m_UtilityCtrl;
	CArcCtrl	* m_pHotArc;

	CProceduralCtrl m_ProcCtrl[5];

	PlayerSubroutine * m_pHotSub;
	PlayerSubroutine * m_pCursorSub;

	Procedural * m_pHotProc;

	CUIFormattedPolyString * m_pHotStr;

	typedef std::vector<CBaseScaleFX *> ModelPieceArray;
	ModelPieceArray m_ModelPieceArray;
};

#endif // _SCREEN_SUBROUTINES_H_