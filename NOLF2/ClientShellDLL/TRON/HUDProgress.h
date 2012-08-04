// ----------------------------------------------------------------------- //
//
// MODULE  : HUDProgress.h
//
// PURPOSE : HUDItem to display objectives and working procedurals
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __HUD_PROGRESS_H
#define __HUD_PROGRESS_H

#include "HUDItem.h"

typedef enum {
	// outer frame
	FRAME_TOP_LEFT = 0,
	FRAME_TOP,
	FRAME_TOP_RIGHT,
	FRAME_LEFT,
	FRAME_BOTTOM_LEFT,
	FRAME_BOTTOM,

	// inner box
	BOX_TOP_LEFT,
	BOX_TOP,
	BOX_TOP_RIGHT,
	BOX_LEFT,
//	BOX_LEFT_INSET,
	BOX_CENTER,
	BOX_RIGHT,
	BOX_BOTTOM_LEFT,
	BOX_BOTTOM,
	BOX_BOTTOM_RIGHT,
	LAST_PRIM,
} ePrimType;

typedef enum {
	PROC_TOP_LEFT,
	PROC_TOP,
	PROC_TOP_RIGHT,
	PROC_LEFT,
	PROC_CENTER,
	PROC_RIGHT,
	PROC_BOTTOM_LEFT,
	PROC_BOTTOM,
	PROC_BOTTOM_RIGHT,
} eBoxPieceType;

//******************************************************************************************
//** HUD Ratings display
//******************************************************************************************
class CHUDProgress : public CHUDItem
{
public:
	CHUDProgress();

    LTBOOL      Init();
	void		Term();

    void        Render();
    void        Update();

    void        UpdateLayout();

private:
	void		UpdateGraphics();
	int			GetPieceWidth(ePrimType eType);
	int			GetPieceHeight(ePrimType eType);

	void		BuildProcRect(int iProc, LTRect rcProc);

	void		SetUVWH(LT_POLYFT4 * pPrim, LTRect rect);

	float		m_fScale;

	bool		m_bShow;
	bool		m_bProceduralActive;

	int			m_nNumObjectives;
	int			m_nNumCompletedObjectives;
	int			m_nNumSecondaryObjectives;
	int			m_nNumProcs;

	// Drawing parameters
	LTRect		m_rcProgress;
	int			m_iTitleFont;
	int			m_iTitleFontSize;
	uint32		m_iTitleFontColor;

	int			m_iObjectiveFont;
	int			m_iObjectiveFontSize;
	uint32		m_iObjectiveFontColor;
	uint32		m_iCompletedObjectiveFontColor;
	uint32		m_iSecondaryObjectiveFontColor;

	int			m_iProceduralFont;
	int			m_iProceduralFontSize;
	uint32		m_iProceduralFontColor;
	int			m_iProceduralHeight;

	// Drawing elements
	HTEXTURE	m_hProgressTex;

	LT_POLYFT4	m_PrimArray[LAST_PRIM];
	LTRect		m_rcPrimDims[LAST_PRIM];

	LT_POLYFT4	m_InsetPrim;
	LTRect		m_InsetPrimDims;
	LT_POLYFT4	m_InsetFullPrim;
	LTRect		m_InsetFullPrimDims;

	LT_POLYFT4	m_ProcPrimArray[5][9];	// procedural rectangles
	bool		m_bIsProcActive[5];
	float		m_fProcPercent[5];
	CUIPolyString * m_pProcStr[5];
	CUIPolyString * m_pSubroutineStr[5];

	LT_POLYFT4	m_ProcPrim[5];
	LT_POLYFT4	m_SubPrim[5];
	HTEXTURE	m_hProcTex[5];
	HTEXTURE	m_hSubTex[5];
//	HTEXTURE	m_hBuild[3];

	LT_POLYFT4	m_BaseBar[5];
	LT_POLYFT4	m_ProgressBar[5];
	LTRect		m_rcSeparator;
	LT_POLYFT4	m_SeparatorPrim;

	CUIFormattedPolyString * m_pTitleStr;
	CUIFormattedPolyString * m_pOptionalStr;

	CUIFormattedPolyString * m_pObjectiveStr[10];
	CUIFormattedPolyString * m_pSecondaryObjectiveStr[10];
	CUIFormattedPolyString * m_pCompletedObjectiveStr[10];
};

#endif