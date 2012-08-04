//-------------------------------------------------------------------------
//
// MODULE  : ProceduralCtrl.h
//
// PURPOSE : GUI element for interacting with the visual representation of
//			 procedurals
//
// CREATED : 4/3/02 - for TRON
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __PROCEDURAL_CTRL_H
#define __PROCEDURAL_CTRL_H

#include "ScreenSpriteMgr.h"
#include "BaseScaleFX.h"
#include "TronLayoutMgr.h"
#include "SubroutineMgr.h"


class CProceduralCtrl
{
public:
	CProceduralCtrl();
	~CProceduralCtrl();

	bool			Init(Procedural * pProc);
	void			Term();

	bool			Render();

	LTBOOL			OnMouseMove(int mx, int my, PlayerSubroutine * pCursorSub);
	bool			IsHot() {return m_bHot;}

	//
	void			SetObject(HOBJECT hObj) {m_hObj = hObj;}

	// Accessors
	Procedural *	GetProc() {return m_pProc;}
	PlayerSubroutine * GetSub() {return m_pProc ? m_pProc->pSub : LTNULL;}

	void			RemoveSub();
	void			AddSub(PlayerSubroutine * pSub);
	void			AssociateHObject(HOBJECT hObj, char * szSocketName);

private:

	HOBJECT			m_hObj;
	char			m_szSocketName[32];
	bool			m_bWorking;
	// get the node name

	Procedural *	m_pProc;			// which actual procedural do we represent?

	CBaseScaleFX *	CreateFX(char * szModel, char * szSkin, char * szRenderStyle, char *szAnim);
	void			KillFX(CBaseScaleFX * pFX);	// clean up a single BaseScaleFX
	void			TransformFX(CBaseScaleFX * pFX, LTransform transform);

	bool			m_bHot;				// flag for when a mouse cursor is over us

	int				m_iSlot;			// slot from 1-5 where it is on screen (may not be used)
	LTIntPt			m_CenterPos;		// screen coordinates for center of procedural
	int				m_iRadius;			// size

	// These are the models that comprise the appearance of the Procedural
	CBaseScaleFX *	m_pIdleFX;			// waiting
	CBaseScaleFX *	m_pWorkingFX;		// working
	CBaseScaleFX *	m_pSubroutineFX;	// model of the subroutine
	CBaseScaleFX *	m_pSubBuildFX;		// subroutine alpha, beta, gold model
	CBaseScaleFX *	m_pConditionFX;		// model of how much the subroutine is affected
	CBaseScaleFX *	m_pProgressFX;		// model of how much the subroutine has been cleared
	CBaseScaleFX *	m_pHighlightFX;		// highlight when the mouse cursor is over
};


#endif // __PROCEDURAL_CTRL_H
