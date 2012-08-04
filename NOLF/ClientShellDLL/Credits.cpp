/****************************************************************************
;
;	 MODULE:		Credits (.CPP)
;
;	PURPOSE:		Credits class
;
;	HISTORY:		07/24/98 [blg] This file was created
;					07/26/00 [jrg] modified for NOLF
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "stdafx.h"
#include "GameClientShell.h"
#include "Credits.h"
#include <stdio.h>
#include "ClientRes.h"

// Macros...

#define TERM_SURFACE(hS)	if (g_pLTClient && hS) { g_pLTClient->DeleteSurface(hS); hS = NULL; }
#define TERM_STRING(hS)		if (g_pLTClient && hS) { g_pLTClient->FreeString(hS); hS = NULL; }

LTFLOAT		CCredits::s_fSpeed = DEF_CREDITS_SPEED;


namespace
{
	HLTCOLOR   g_hTransColor = SETRGB_T(0,255,0);
	LTBOOL		s_bPause = LTFALSE;
	LTFLOAT		s_fFadeInTime = -1.0f;
	LTFLOAT		s_fHoldTime = -1.0f;
	LTFLOAT		s_fFadeOutTime = -1.0f;
	LTFLOAT		s_fDelayTime = -1.0f;
	LTIntPt		s_PositionUL;
	LTIntPt		s_PositionUR;
	LTIntPt		s_PositionLL;
	LTIntPt		s_PositionLR;
	HMODULE s_hModule = NULL;
	HRSRC   s_hRes    = NULL;
	HGLOBAL	s_hGlobal = NULL;
	char*	s_sBuf    = LTNULL;
}

static char* GetTextBuffer(char* sName);

// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::Init(char* sBuf)
{
	// Sanity checks...

	if (!sBuf) return(LTFALSE);

	// Set simple members...

	m_nState    = CS_START;

	s_bPause    = LTFALSE;

	m_Pos.x		= 320;
	m_Pos.y		= 240;
	m_ePosition	= CP_CENTER;
	m_bBig		= LTFALSE;

	// Parse the text buffer and add each line as a separate string...

	char sString[256];
	int  i = 0;

	while (*sBuf)
	{
		if ((*sBuf == '\n') || (*sBuf == '\r') || (*sBuf == '\0'))
		{
			sString[i] = '\0';
			AddString(sString); 
			i = 0;

			while (((*sBuf == '\n') || (*sBuf == '\r'))) 
			{
				sBuf = (char*)_mbsinc((const unsigned char*)sBuf);
			};
		}
		else
		{
			int nCount = _mbsnbcnt((const unsigned char*)sBuf,1);
			memcpy(&sString[i], sBuf, nCount);
			i += nCount;
			sBuf = (char*)_mbsinc((const unsigned char*)sBuf);
		}
	}

	sString[i] = '\0';
	if (strlen(sString) > 0) AddString(sString);

	CreateSurface();	


	// All done...

	return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CCredit::Term()
{
	for (int i = 0; i < MAX_CREDIT_STRINGS; i++)
	{
		TERM_STRING(m_aStrings[i]);
	}

	TERM_SURFACE(m_hSurface);

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::AddString
//
//	PURPOSE:	Adds a new string
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::AddString(char* sString)
{
	// Sanity checks...

	if (!g_pLTClient) return(LTFALSE);
	if (!sString) return(LTFALSE);
	if (m_cStrings >= MAX_CREDIT_STRINGS) return(LTFALSE);


	// Check if this is a special command string...

	if (_mbsnbcmp((const unsigned char*)sString, (const unsigned char*)">TIME:", 6) == 0)
	{
		if (_mbstrlen(sString) > 6)
		{
			m_fHoldTime = (float)atof(&sString[6]);
			return(LTTRUE);
		}
	}
	else if (_mbsnbcmp((const unsigned char*)sString, (const unsigned char*)">POS:UL", 7) == 0)
	{
		m_Pos = s_PositionUL;
		m_ePosition = CP_UL;
		return(LTTRUE);
	}
	else if (_mbsnbcmp((const unsigned char*)sString, (const unsigned char*)">POS:UR", 7) == 0)
	{
		m_Pos = s_PositionUR;
		m_ePosition = CP_UR;
		return(LTTRUE);
	}
	else if (_mbsnbcmp((const unsigned char*)sString, (const unsigned char*)">POS:LR", 7) == 0)
	{
		m_Pos = s_PositionLR;
		m_ePosition = CP_LR;
		return(LTTRUE);
	}
	else if (_mbsnbcmp((const unsigned char*)sString, (const unsigned char*)">POS:LL", 7) == 0)
	{
		m_Pos = s_PositionLL;
		m_ePosition = CP_LL;
		return(LTTRUE);
	}
	else if (_mbsnbcmp((const unsigned char*)sString, (const unsigned char*)">BIG", 4) == 0)
	{
		m_bBig = LTTRUE;
		return(LTTRUE);
	}



	// Create a new string via the engine...

	HSTRING hString = g_pLTClient->CreateString(sString);
	if (!hString) return(LTFALSE);

	m_aStrings[m_cStrings++] = hString;


	// All done...

	return(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::CreateSurface
//
//	PURPOSE:	create the string surface
//
// ----------------------------------------------------------------------- //
void CCredit::CreateSurface()
{
    LTIntPt sz;
    LTIntPt strSz;
	int i;
	
	CLTGUIFont *pFont = LTNULL;
	if (m_bBig)
		pFont = g_pInterfaceResMgr->GetTitleFont();
	else
		pFont = g_pInterfaceResMgr->GetMsgForeFont();

	for (i = 0; i < m_cStrings; i++)
	{
		strSz = pFont->GetTextExtents(m_aStrings[i]);
		sz.y += strSz.y;
		sz.x = Max(sz.x,strSz.x);
	}

    LTRect rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = sz.x;
	rect.bottom = sz.y;

	HLTCOLOR backColor = kBlack;
//	if (m_bBig)
//		backColor = g_hTransColor;

    m_hSurface  = g_pLTClient->CreateSurface(sz.x,sz.y);
	g_pLTClient->FillRect(m_hSurface,&rect,backColor);

	m_SurfaceSz = sz;
	int x = 0;
	int nJustify = LTF_JUSTIFY_LEFT;
	switch (m_ePosition)
	{
	case CP_UL:
	case CP_LL:
		nJustify = LTF_JUSTIFY_LEFT;
		x = 0; 
		break;
	case CP_CENTER:
		nJustify = LTF_JUSTIFY_CENTER;
		x = sz.x/2; 
		break;
	case CP_UR:
	case CP_LR:
		nJustify = LTF_JUSTIFY_RIGHT;
		x = sz.x; 
		break;
	}
	int y = 0;
	for (i = 0; i < m_cStrings; i++)
	{
		pFont->Draw(m_aStrings[i], m_hSurface,x,y,nJustify,kWhite);
		y += pFont->GetHeight();
	}

	g_pLTClient->OptimizeSurface(m_hSurface,backColor);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Draw
//
//	PURPOSE:	Draws all the strings of the credit
//
// ----------------------------------------------------------------------- //

void CCredit::Draw()
{
	// Sanity checks...

	if (m_cStrings <= 0) return;
	if (!m_hSurface) return;


	// Tweak our draw position...
	int x = (int)(g_pInterfaceResMgr->GetXRatio() * (LTFLOAT)m_Pos.x);
	int y = (int)(g_pInterfaceResMgr->GetYRatio() * (LTFLOAT)m_Pos.y);
	
	switch (m_ePosition)
	{
	case CP_CENTER:
		x -= m_SurfaceSz.x/2; 
		y -= m_SurfaceSz.y/2; 
		break;
	case CP_UR:
		x -= m_SurfaceSz.x; 
		break;
	case CP_LR:
		x -= m_SurfaceSz.x; 
		y -= m_SurfaceSz.y; 
		break;
	case CP_LL:
		y -= m_SurfaceSz.y; 
		break;
	}


//	if (m_bBig)
//		g_pLTClient->DrawSurfaceToSurfaceTransparent(g_pLTClient->GetScreenSurface(), m_hSurface, NULL, x, y, g_hTransColor);
//	else
	{
		uint8 nFade = (uint8)(255.0f * m_fAlpha);
		HLTCOLOR hColor = SETRGB(nFade,nFade,nFade);


		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
		g_pLTClient->SetOptimized2DColor(hColor);
		g_pLTClient->DrawSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hSurface, NULL, x+1, y+1);
		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ADD);
		g_pLTClient->DrawSurfaceToSurface(g_pLTClient->GetScreenSurface(), m_hSurface, NULL, x, y);
		g_pLTClient->SetOptimized2DColor(kWhite);
		g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ALPHA);

	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Update
//
//	PURPOSE:	Updates the credit
//
//	RETURNS:	LTFALSE = Stay with current credit
//				LTTRUE  = Advance to next credit
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::Update()
{
	// Sanity checks...

	if (!g_pLTClient) return(LTFALSE);


	// Update based on our current state...

	switch(m_nState)
	{
		case CS_START:
		{
			SetState(CS_FADEIN);
			return(LTFALSE);
		}

		case CS_FADEIN:
		{
			if (UpdateFadeIn())	SetState(CS_HOLDIN);
			return(LTFALSE);
		}

		case CS_HOLDIN:
		{
			if (UpdateHoldIn()) SetState(CS_FADEOUT);
			return(LTFALSE);
		}

		case CS_FADEOUT:
		{
			if (UpdateFadeOut()) SetState(CS_HOLDOUT);
			return(LTFALSE);
		}

		case CS_HOLDOUT:
		{
			if (UpdateHoldOut()) SetState(CS_DONE);
			return(LTFALSE);
		}

		case CS_DONE:
		{
			return(LTTRUE);
		}

		default:
		{
			return(LTTRUE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::SetState
//
//	PURPOSE:	Sets the update state
//
// ----------------------------------------------------------------------- //

void CCredit::SetState(int nState)
{
	// Set the new state...

	m_nState = nState;

	switch (nState)
	{
		case CS_START:
		{
			break;
		}

		case CS_FADEIN:
		{
			m_fTimer = s_fFadeInTime;
			m_fAlpha = 0.0f;
			break;
		}

		case CS_HOLDIN:
		{
			m_fTimer = s_fHoldTime;
			m_fAlpha = 1.0f;
			if (m_fHoldTime > 0.0f) m_fTimer = m_fHoldTime;
			break;
		}

		case CS_FADEOUT:
		{
			m_fTimer = s_fFadeOutTime;
			m_fAlpha = 1.0f;
			break;
		}

		case CS_HOLDOUT:
		{
			m_fTimer = s_fDelayTime;
			m_fAlpha = 1.0f;
			break;
		}

		case CS_DONE:
		{
			break;
		}
	}

	m_fTimerStart = m_fTimer;
	m_fAlphaStart = m_fAlpha;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::UpdateFadeIn
//
//	PURPOSE:	Updates the CS_FADEIN state
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::UpdateFadeIn()
{
	// Update the timer value...

	LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	fDelta *= CCredits::GetSpeed();
	if (s_bPause) fDelta = 0.0f;

	if (m_fTimer > fDelta)
	{
		m_fTimer -= fDelta;
	}
	else
	{
		m_fTimer = 0.0f;
	}


	// set the alpha...

	m_fAlpha = 1.0f - (m_fTimer / m_fTimerStart);
	if (m_fAlpha < 0.0f) m_fAlpha = 0.0f;
	if (m_fAlpha > 1.0f) m_fAlpha = 1.0f;

	if (m_bBig)
	{
		g_pLTClient->SetSurfaceAlpha(m_hSurface,m_fAlpha);
		g_pLTClient->OptimizeSurface(m_hSurface,g_hTransColor);
	}

	// Draw the credit...

	Draw();


	// All done...

	return(m_fTimer == 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::UpdateHoldIn
//
//	PURPOSE:	Updates the CS_HOLDIN state
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::UpdateHoldIn()
{
	// Update the timer value...

	LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	fDelta *= CCredits::GetSpeed();
	if (s_bPause) fDelta = 0;

	if (m_fTimer > fDelta)
	{
		m_fTimer -= fDelta;
	}
	else
	{
		m_fTimer = 0.0f;
	}


	// Draw the credit...

	Draw();


	// All done...

	return(m_fTimer == 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::UpdateFadeOut
//
//	PURPOSE:	Updates the CS_FADEOUT state
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::UpdateFadeOut()
{
	// Update the timer value...

	LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	fDelta *= CCredits::GetSpeed();
	if (s_bPause) fDelta = 0.0f;

	if (m_fTimer > fDelta)
	{
		m_fTimer -= fDelta;
	}
	else
	{
		m_fTimer = 0.0f;
	}


	// set the alpha...

	m_fAlpha = (m_fTimer / m_fTimerStart);
	if (m_fAlpha < 0.0f) m_fAlpha = 0.0f;
	if (m_fAlpha > 1.0f) m_fAlpha = 1.0f;

	g_pLTClient->SetSurfaceAlpha(m_hSurface,m_fAlpha);
	g_pLTClient->OptimizeSurface(m_hSurface,g_hTransColor);

	// Draw the credit...

	Draw();


	// All done...

	return(m_fTimer == 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::UpdateHoldOut
//
//	PURPOSE:	Updates the CS_HOLDOUT state
//
// ----------------------------------------------------------------------- //

LTBOOL CCredit::UpdateHoldOut()
{
	// Update the timer value...

	LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	fDelta *= CCredits::GetSpeed();
	if (s_bPause) fDelta = 0;

	if (m_fTimer > fDelta)
	{
		m_fTimer -= fDelta;
	}
	else
	{
		m_fTimer = 0.0f;
	}


	// All done...

	return(m_fTimer == 0.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

LTBOOL CCredits::Init(int nMode, LTBOOL bClearScreen)
{
	// Sanity checks...


	Term();


	// Set simple members...

	m_iCredit      = 0;
	m_nMode        = nMode;
	m_bDone        = LTFALSE;
	m_bClearScreen = bClearScreen;
	s_fSpeed       = DEF_CREDITS_SPEED;

	if (s_fFadeInTime < 0.0f)
	{
		s_fFadeInTime = g_pLayoutMgr->GetCreditsFadeInTime();
		s_fHoldTime = g_pLayoutMgr->GetCreditsHoldTime();
		s_fFadeOutTime = g_pLayoutMgr->GetCreditsFadeOutTime();
		s_fDelayTime = g_pLayoutMgr->GetCreditsDelayTime();
		s_PositionUL = g_pLayoutMgr->GetCreditsPositionUL();
		s_PositionUR = g_pLayoutMgr->GetCreditsPositionUR();
		s_PositionLL = g_pLayoutMgr->GetCreditsPositionLL();
		s_PositionLR = g_pLayoutMgr->GetCreditsPositionLR();
	}


	// Add all the credit objects...

	AddCredits();


	// All done...

	if (m_cCredits)
		m_bInited = LTTRUE;

	return(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::Term
//
//	PURPOSE:	Termination
//
// ----------------------------------------------------------------------- //

void CCredits::Term()
{
	// Term all the credits...

	for (int i = 0; i < MAX_CREDITS; i++)
	{
		CCredit* pCredit = GetCredit(i);
		if (pCredit) pCredit->Term();
	}


	// Clear all members...

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::HandleInput
//
//	PURPOSE:	Handler for input
//
// ----------------------------------------------------------------------- //

void CCredits::HandleInput(int vkey)
{
	// Handle various keys...

	switch (vkey)
	{
		case VK_UP:
		{
			if (IsIntro()) break;
			IncSpeed();
			break;
		}

		case VK_DOWN:
		{
			if (IsIntro()) break;
			DecSpeed();
			break;
		}

		case VK_HOME:
		{
			if (IsIntro()) break;
			s_fSpeed = 1.0;
			break;
		}

		case VK_RETURN:
		case VK_SPACE:
		case VK_NEXT:
		{
			if (IsIntro() || IsDemoIntro())
			{
				CCredit* pCredit = GetCredit(m_iCredit);
				if (pCredit)
				{
					if (pCredit->GetState() == CS_HOLDIN)
					{
						pCredit->SetState(CS_FADEOUT);
					}
				}
			}
			else
			{
				AdvanceCredit(CS_HOLDIN);
			}
			break;
		}

		case VK_PRIOR:
		{
			if (IsIntro() || IsDemoIntro()) break;
			BackupCredit(CS_HOLDIN);
			break;
		}

		case VK_PAUSE:
		{
			if (IsIntro() || IsDemoIntro()) break;
			s_bPause ^= 1;
			break;
		}

		case VK_ESCAPE:
		{
//			ExitToMainMenu();
			break;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::ExitToMainMenu
//
//	PURPOSE:	Exits back to the main menu game state
//
// ----------------------------------------------------------------------- //

//void CCredits::ExitToMainMenu()
//{
//	m_bDone = LTTRUE;
//}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::Update
//
//	PURPOSE:	Updates the credits
//
// ----------------------------------------------------------------------- //

void CCredits::Update()
{
	// Sanity checks...

	if (!g_pLTClient) return;
	if (m_cCredits <= 0) return;


	// Clear the screen...

	if (m_bClearScreen)
	{
		g_pLTClient->ClearScreen(NULL, CLEARSCREEN_SCREEN);
	}



	// Update the current credit...

	CCredit* pCredit = GetCredit(m_iCredit);
	if (!pCredit) return;

	if (pCredit->Update())
	{
		AdvanceCredit();
	}
}

void CCredits::AdvanceCredit(int nState)
{
	m_iCredit++;
	if(m_iCredit >= m_cCredits)
	{
		m_iCredit = 0;
//		if (IsIntro() || IsDemoIntro()) m_bDone = LTTRUE;
		m_bDone = LTTRUE;
	}
	CCredit* pCredit = GetCredit(m_iCredit);
	if (pCredit) pCredit->SetState(nState);
}

void CCredits::BackupCredit(int nState)
{
	m_iCredit--;
	if(m_iCredit < 0)
		m_iCredit = 0;

	CCredit* pCredit = GetCredit(m_iCredit);
	if (pCredit) pCredit->SetState(nState);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::AddCredits
//
//	PURPOSE:	Adds all the credit objects
//
// ----------------------------------------------------------------------- //

void CCredits::AddCredits()
{
	// Sanity checks...

	if (!g_pLTClient) return;


	// Get the credits text buffer...

	char* sName = NULL;

	switch (GetMode())
	{
		case CM_INTRO:		sName = "INTRO"; break;
		case CM_CREDITS:	sName = "CREDITS"; break;
		case CM_DEMO_INFO:	sName = "DEMOINFO"; break;
		case CM_DEMO_INTRO:	sName = "DEMOINTRO"; break;
		case CM_DEMO_MULTI:	sName = "DEMOMULTI"; break;

		default: sName = "CREDITS";
	}

	char* sBuf = GetTextBuffer(sName);
	if (!sBuf) return;


	// Parse the credits text...

	char sCredit[1024];
	int  i = 0;

	while (*sBuf)
	{

		if (*sBuf == '#' && *((char*)_mbsinc((const unsigned char*)sBuf)) == '#')
		{
			sCredit[i] = '\0';

			if (_mbsnbcmp((const unsigned char*)sCredit, (const unsigned char*)">END", 4) == 0)	// end?
			{
				return;
			}

			AddCredit(sCredit);
			i = 0;

			sBuf = (char*)_mbsinc((const unsigned char*)sBuf);
			sBuf = (char*)_mbsinc((const unsigned char*)sBuf);

			while (*sBuf != '\0' && ((*sBuf == '\n') || (*sBuf == '\r'))) sBuf++;
		}
		else
		{
			int nCount = _mbsnbcnt((const unsigned char*)sBuf,1);
			memcpy(&sCredit[i], sBuf, nCount);
			i += nCount;
			sBuf = (char*)_mbsinc((const unsigned char*)sBuf);
		}
	}
}

void CCredits::AddCredit(char* sText)
{
	// Sanity checks...

	if (!g_pLTClient) return;


	// Add the credit...

	CCredit* pCredit = GetCredit(m_cCredits);
	if (!pCredit) return;

	if (!pCredit->Init(sText)) return;


	// Inc our credit counter...

	m_cCredits++;
}





// Functions...

char* GetTextBuffer(char* sName)
{
	//if (s_sBuf)
	//{
		//return(s_sBuf);
	//}
	//else
	{
		void* hModule;
		g_pLTClient->GetEngineHook("cres_hinstance",&hModule);
		s_hModule = (HINSTANCE)hModule;
		if (!s_hModule)	return(NULL);

		s_hRes = FindResource(s_hModule, sName, "TEXT");
		if (!s_hRes) return(NULL);

		s_hGlobal = LoadResource(s_hModule, s_hRes);
		if (!s_hGlobal) return(NULL);

		s_sBuf = (char*)LockResource(s_hGlobal);
		if (!s_sBuf) return(NULL);
		
		return(s_sBuf);
	}
}
