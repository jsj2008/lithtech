/****************************************************************************
;
;	 MODULE:		Credits (.H)
;
;	PURPOSE:		Credits class
;
;	HISTORY:		07/24/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


#ifndef _CREDITS_H_
#define _CREDITS_H_


// Includes...

#include "ClientUtilities.h"


// Defines...

#define MAX_CREDIT_STRINGS		12

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


// Externs...

class CBloodClientShell;


// Classes...

class CCredit
{
	// Member functions...

public:
	CCredit() { Clear(); }
	~CCredit() { Term(); }

	DBOOL				Init(CClientDE* pClientDE, char* sBuf);
	void				Term();
	void				Clear();

	int					GetNumStrings() { return(m_cStrings); }
	HSTRING				GetString(int iString);
	int					GetState() { return(m_nState); }

	void				SetState(int nState);

	void				Draw(int x, int y, HDECOLOR hForeColor);
	void				DrawString(HSTRING hString, HDECOLOR hForeColor, HDECOLOR hBackColor, int x, int y);

	DBOOL				Update();
	DBOOL				UpdateFadeIn();
	DBOOL				UpdateHoldIn();
	DBOOL				UpdateFadeOut();
	DBOOL				UpdateHoldOut();

public:
	static	DBOOL		SetStaticInfo(CClientDE* pClientDE, int nMode, DBOOL bClearScreen);
	static	void		TermStaticInfo();

private:
	DBOOL				AddString(char* sString);

private:
	CClientDE*			m_pClientDE;
	HSTRING				m_aStrings[MAX_CREDIT_STRINGS];
	int					m_cStrings;
	int					m_nState;
	DFLOAT				m_fTimer;
	DFLOAT				m_fColor;
	DFLOAT				m_fTimerStart;
	DFLOAT				m_fColorStart;
	DFLOAT				m_fHoldTime;

	static	int			s_nFontHeight;
	static	int			s_nFontSpacing;
	static	DDWORD		s_dwScreenWidth;
	static	DDWORD		s_dwScreenHeight;
	static	HDEFONT		s_hFont;
	static	HSURFACE	s_hScreen;
	static	CClientDE*	s_pClientDE;
	static	float		s_fDefaultHoldTime;
	static	float		s_fDefaultOutTime;
	static	DBOOL		s_bClearScreen;
};

class CCredits
{
	// Member functions...

public:
	CCredits() { Clear(); }
	~CCredits() { Term(); }

	DBOOL				Init (CClientDE* pClientDE, CBloodClientShell* pClientShell, int nMode = CM_CREDITS, DBOOL bClearScreen = DTRUE);

	void				Term();
	void				Clear();

	DBOOL				IsInited() { return(m_bInited); }
	DBOOL				IsDone() { return(m_bDone); }
	DBOOL				IsCredits() { return(m_nMode == CM_INTRO); }
	DBOOL				IsIntro() { return(m_nMode == CM_INTRO); }
	DBOOL				IsClearingScreen() { return(m_bClearScreen); }

	CCredit*			GetCredit(int iCredit);
	int					GetNumCredits() { return(m_cCredits); }
	int					GetMode() { return(m_nMode); }
	CClientDE*			GetClientDE() { return(m_pClientDE); }
	CBloodClientShell*	GetClientShell() { return m_pClientShell; }

	void				HandleInput(int vkey);
	void				ExitToMainMenu();

	void				AddCredit(char* sText);

	void				Update();

	void				IncSpeed() { if (s_fSpeed < MAX_CREDITS_SPEED) s_fSpeed += ADJ_CREDITS_SPEED; }
	void				DecSpeed() { if (s_fSpeed > MIN_CREDITS_SPEED) s_fSpeed -= ADJ_CREDITS_SPEED; }

private:
	void				DrawCenteredStringToScreen(HDEFONT hFont, HSTRING hString, HDECOLOR hForeColor, HDECOLOR hBackColor, int xDraw, int yDraw);
	void				AddCredits();
	void				AdvanceCredit(int nState = CS_START);

public:
	static	DFLOAT		GetSpeed() { return(s_fSpeed); }


	// Member variables...

private:
	CClientDE*			m_pClientDE;
	CBloodClientShell*	m_pClientShell;
	CSize				m_szScreen;
	int					m_nMode;
	DBOOL				m_bDone;
	DBOOL				m_bInited;
	DBOOL				m_bClearScreen;
	HSURFACE			m_hScreen;
	CCredit				m_aCredits[MAX_CREDITS];
	int					m_cCredits;
	int					m_iCredit;

	static	DFLOAT		s_fSpeed;
};


// Inlines...

inline void CCredit::Clear()
{
	m_pClientDE = NULL;
	for (int i = 0; i < MAX_CREDIT_STRINGS; i++) m_aStrings[i] = NULL;
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
	m_pClientDE    = NULL;
	m_pClientShell = NULL;
	m_hScreen      = NULL;

	m_szScreen.cx = 640;
	m_szScreen.cy = 480;

	m_bInited      = DFALSE;
	m_bDone        = DTRUE;
	m_bClearScreen = DTRUE;

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
