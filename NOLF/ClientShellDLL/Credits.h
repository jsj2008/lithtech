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



// Defines...

#define MAX_CREDIT_STRINGS		20

#define MAX_CREDITS				256

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

	int					GetNumStrings() { return(m_cStrings); }
	HSTRING				GetString(int iString);
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
	void				CreateSurface();

private:
	HSTRING				m_aStrings[MAX_CREDIT_STRINGS];
	int					m_cStrings;
	HSURFACE			m_hSurface;
	LTIntPt				m_SurfaceSz;
	int					m_nState;
	LTFLOAT				m_fTimer;
	LTFLOAT				m_fTimerStart;
	LTFLOAT				m_fAlpha;
	LTFLOAT				m_fAlphaStart;
	LTFLOAT				m_fHoldTime;

	LTIntPt				m_Pos;
	CreditPosition		m_ePosition;

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

	CCredit*			GetCredit(int iCredit);
	int					GetNumCredits() { return(m_cCredits); }
	int					GetMode() { return(m_nMode); }

	void				HandleInput(int vkey);
//	void				ExitToMainMenu();

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
//	HSURFACE			m_hScreen;
	CCredit				m_aCredits[MAX_CREDITS];
	int					m_cCredits;
	int					m_iCredit;

	static	LTFLOAT		s_fSpeed;
};


// Inlines...

inline void CCredit::Clear()
{
	for (int i = 0; i < MAX_CREDIT_STRINGS; i++) m_aStrings[i] = NULL;
	m_hSurface = LTNULL;
	m_cStrings  = 0;
	m_nState    = CS_START;
	m_fTimer    = 0.0f;
	m_fHoldTime = -1.0f;
}

inline HSTRING CCredit::GetString(int iString)
{
	if (iString >= MAX_CREDIT_STRINGS) return(NULL);
	if (iString < 0) return(NULL);
	return(m_aStrings[iString]);
}

inline void CCredits::Clear()
{

	m_bInited      = LTFALSE;
	m_bDone        = LTTRUE;
//	m_bClearScreen = LTTRUE;

	m_cCredits = 0;
	m_iCredit  = 0;
	m_nMode    = CM_CREDITS;

	s_fSpeed = DEF_CREDITS_SPEED;
}

inline CCredit* CCredits::GetCredit(int iCredit)
{
	if (iCredit >= MAX_CREDITS) return(NULL);
	if (iCredit < 0) return(NULL);
	return(&m_aCredits[iCredit]);
}


// EOF...

#endif
