/****************************************************************************
;
;	 MODULE:		Credits (.H)
;
;	PURPOSE:		Credits class
;
;	HISTORY:		07/24/98 [blg] This file was created
;					07/26/00 [jrg] modified for NOLF
;
;	COMMENT:		Copyright (c) 1998-2000, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _CREDITS_H_
#define _CREDITS_H_

#include "iltfontmanager.h"
#include "ltguimgr.h"

// Defines...

#define ADJ_CREDITS_SPEED		0.2f
#define MAX_CREDITS_SPEED		5.0f
#define MIN_CREDITS_SPEED		0.4f
#define DEF_CREDITS_SPEED		1.0f

#define CS_START				0
#define CS_FADEIN				1
#define CS_HOLDIN				2
#define CS_FADEOUT				3
#define CS_HOLDOUT				4
#define CS_DONE					5

#define CM_CREDITS				0
#define CM_INTRO				1
#define CM_DEMO_INFO			2
#define CM_DEMO_INTRO			3
#define CM_DEMO_MULTI			4


enum CreditPosition
{
	CP_CENTER,
	CP_UL,
	CP_UR,
	CP_LR,
	CP_LL
};

// Classes...

class CCredit
{
	// Member functions...

public:
	CCredit() { Clear(); }
	~CCredit() { Term(); }

	LTBOOL				Init( char* sBuf);
	void				Term();
	void				Clear();

	uint16				GetNumStrings() { return(m_Strings.size()); }
//	HSTRING				GetString(int iString);
	int					GetState() { return(m_nState); }

	void				SetState(int nState);

	void				Draw();

	LTBOOL				Update();
	LTBOOL				UpdateFadeIn();
	LTBOOL				UpdateHoldIn();
	LTBOOL				UpdateFadeOut();
	LTBOOL				UpdateHoldOut();


private:
	LTBOOL				AddString(char* sString);
	void				FormatStrings();

private:
	
	FPStringArray		m_Strings;
	int					m_nState;
	LTFLOAT				m_fTimer;
	LTFLOAT				m_fTimerStart;
	LTFLOAT				m_fAlpha;
	LTFLOAT				m_fAlphaStart;
	LTFLOAT				m_fHoldTime;

	LTIntPt				m_Pos;
	CreditPosition		m_ePosition;
	CUI_ALIGNMENTTYPE	m_hAlign;

	LTBOOL				m_bBig;

};

class CCredits
{
	// Member functions...

public:
	CCredits() { Clear(); }
	~CCredits() { Term(); }

	LTBOOL				Init (int nMode = CM_CREDITS, LTBOOL bClearScreen = LTFALSE);

	void				Term();
	void				Clear();

	LTBOOL				IsInited() { return(m_bInited); }
	LTBOOL				IsDone() { return(m_bDone); }
	LTBOOL				IsCredits() { return(m_nMode == CM_CREDITS); }
	LTBOOL				IsIntro() { return(m_nMode == CM_INTRO); }
	LTBOOL				IsDemoIntro() { return(m_nMode == CM_DEMO_INTRO); }
	LTBOOL				IsClearingScreen() { return(m_bClearScreen); }

	CCredit*			GetCredit(uint16 iCredit);
	uint16				GetNumCredits() { return(m_Credits.size()); }
	int					GetMode() { return(m_nMode); }

	void				HandleInput(int vkey);

	void				AddCredit(char* sText);

	void				Update();

	void				IncSpeed() { if (s_fSpeed < MAX_CREDITS_SPEED) s_fSpeed += ADJ_CREDITS_SPEED; }
	void				DecSpeed() { if (s_fSpeed > MIN_CREDITS_SPEED) s_fSpeed -= ADJ_CREDITS_SPEED; }

private:
	void				AddCredits();
	void				AdvanceCredit(int nState = CS_START);
	void				BackupCredit(int nState = CS_START);

public:
	static	LTFLOAT		GetSpeed() { return(s_fSpeed); }


	// Member variables...

private:
	int					m_nMode;
	LTBOOL				m_bDone;
	LTBOOL				m_bInited;
	LTBOOL				m_bClearScreen;
	typedef std::vector<CCredit*> CreditArray;
	CreditArray			m_Credits;
	uint16				m_iCredit;

	static	LTFLOAT		s_fSpeed;
};



// EOF...

#endif
