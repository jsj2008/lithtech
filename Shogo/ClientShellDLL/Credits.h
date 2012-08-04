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
#define CM_DEMO_INFO			2
#define CM_DEMO_INTRO			3
#define CM_DEMO_MULTI			4


// Externs...

class ILTClient;
class CRiotClientShell;


// Classes...

class CCredit
{
	// Member functions...

public:
	CCredit() { Clear(); }
	~CCredit() { Term(); }

	LTBOOL				Init(ILTClient* pClientDE, char* sBuf);
	void				Term();
	void				Clear();

	int					GetNumStrings() { return(m_cStrings); }
	HSTRING				GetString(int iString);
	int					GetState() { return(m_nState); }

	void				SetState(int nState);

	void				Draw(int x, int y, HLTCOLOR hForeColor);
	void				DrawString(HSTRING hString, HLTCOLOR hForeColor, HLTCOLOR hBackColor, int x, int y);

	LTBOOL				Update();
	LTBOOL				UpdateFadeIn();
	LTBOOL				UpdateHoldIn();
	LTBOOL				UpdateFadeOut();
	LTBOOL				UpdateHoldOut();

public:
	static	LTBOOL		SetStaticInfo(ILTClient* pClientDE, int nMode, LTBOOL bClearScreen);
	static	void		TermStaticInfo();

private:
	LTBOOL				AddString(char* sString);

private:
	ILTClient*			m_pClientDE;
	HSTRING				m_aStrings[MAX_CREDIT_STRINGS];
	int					m_cStrings;
	int					m_nState;
	LTFLOAT				m_fTimer;
	LTFLOAT				m_fColor;
	LTFLOAT				m_fTimerStart;
	LTFLOAT				m_fColorStart;
	LTFLOAT				m_fHoldTime;

	static	int			s_nFontHeight;
	static	int			s_nFontSpacing;
	static	uint32		s_dwScreenWidth;
	static	uint32		s_dwScreenHeight;
	static	HLTFONT		s_hFont;
	static	HSURFACE	s_hScreen;
	static	ILTClient*	s_pClientDE;
	static	LTFLOAT		s_fDefaultHoldTime;
	static	LTFLOAT		s_fDefaultOutTime;
	static	LTBOOL		s_bClearScreen;
};

class CCredits
{
	// Member functions...

public:
	CCredits() { Clear(); }
	~CCredits() { Term(); }

	LTBOOL				Init (ILTClient* pClientDE, CRiotClientShell* pClientShell, int nMode = CM_CREDITS, LTBOOL bClearScreen = LTTRUE);

	void				Term();
	void				Clear();

	LTBOOL				IsInited() { return(m_bInited); }
	LTBOOL				IsDone() { return(m_bDone); }
	LTBOOL				IsCredits() { return(m_nMode == CM_INTRO); }
	LTBOOL				IsIntro() { return(m_nMode == CM_INTRO); }
	LTBOOL				IsDemoIntro() { return(m_nMode == CM_DEMO_INTRO); }
	LTBOOL				IsClearingScreen() { return(m_bClearScreen); }

	CCredit*			GetCredit(int iCredit);
	int					GetNumCredits() { return(m_cCredits); }
	int					GetMode() { return(m_nMode); }
	ILTClient*			GetILTClient() { return(m_pClientDE); }
	CRiotClientShell*	GetClientShell() { return m_pClientShell; }

	void				HandleInput(int vkey);
	void				ExitToMainMenu();

	void				AddCredit(char* sText);

	void				Update();

	void				IncSpeed() { if (s_fSpeed < MAX_CREDITS_SPEED) s_fSpeed += ADJ_CREDITS_SPEED; }
	void				DecSpeed() { if (s_fSpeed > MIN_CREDITS_SPEED) s_fSpeed -= ADJ_CREDITS_SPEED; }

private:
	void				DrawCenteredStringToScreen(HLTFONT hFont, HSTRING hString, HLTCOLOR hForeColor, HLTCOLOR hBackColor, int xDraw, int yDraw);
	void				AddCredits();
	void				AdvanceCredit(int nState = CS_START);

public:
	static	LTFLOAT		GetSpeed() { return(s_fSpeed); }


	// Member variables...

private:
	ILTClient*			m_pClientDE;
	CRiotClientShell*	m_pClientShell;
	CSize				m_szScreen;
	int					m_nMode;
	LTBOOL				m_bDone;
	LTBOOL				m_bInited;
	LTBOOL				m_bClearScreen;
	HSURFACE			m_hScreen;
	CCredit				m_aCredits[MAX_CREDITS];
	int					m_cCredits;
	int					m_iCredit;

	static	LTFLOAT		s_fSpeed;
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

	m_bInited      = LTFALSE;
	m_bDone        = LTTRUE;
	m_bClearScreen = LTTRUE;

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
