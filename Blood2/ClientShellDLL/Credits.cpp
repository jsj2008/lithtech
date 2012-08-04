/****************************************************************************
;
;	 MODULE:		Credits (.CPP)
;
;	PURPOSE:		Credits class
;
;	HISTORY:		07/24/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "client_de.h"
#include "Credits.h"
#include "BloodClientShell.h"
#include "vkdefs.h"
#include <stdio.h>
#include "CreditsWin.h"
#include "ClientRes.h"
#include "mbstring.h"


// Macros...

#define TERM_COLOR(hC)	if (m_pClientDE && hC) { m_pClientDE->DeleteColor(hC); hC = NULL; }
#define TERM_FONT(hF)	if (m_pClientDE && hF) { m_pClientDE->DeleteFont(hF); hF = NULL; }
#define TERM_STRING(hS)	if (m_pClientDE && hS) { m_pClientDE->FreeString(hS); hS = NULL; }


// Statics...

int			CCredit::s_nFontHeight      = 0;
int			CCredit::s_nFontSpacing     = 0;
DDWORD		CCredit::s_dwScreenWidth    = 0;
DDWORD		CCredit::s_dwScreenHeight   = 0;
HDEFONT		CCredit::s_hFont            = NULL;
HSURFACE	CCredit::s_hScreen          = NULL;
CClientDE*	CCredit::s_pClientDE        = NULL;
float		CCredit::s_fDefaultHoldTime = 2.0f;
float		CCredit::s_fDefaultOutTime  = 0.6f;
DBOOL		CCredit::s_bClearScreen     = DTRUE;

DFLOAT		CCredits::s_fSpeed = DEF_CREDITS_SPEED;

DBOOL		s_bPause = DFALSE;


// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Init
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

DBOOL CCredit::Init(CClientDE* pClientDE, char* sBuf)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);
	if (!sBuf) return(DFALSE);


	// Set simple members...

	m_pClientDE = pClientDE;
	m_nState    = CS_START;

	s_bPause    = DFALSE;


	// Parse the text buffer and add each line as a seperate string...

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
	if (_mbstrlen(sString) > 0) AddString(sString);


	// All done...

	return(DTRUE);
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

	Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::AddString
//
//	PURPOSE:	Adds a new string
//
// ----------------------------------------------------------------------- //

DBOOL CCredit::AddString(char* sString)
{
	// Sanity checks...

	if (!m_pClientDE) return(DFALSE);
	if (!sString) return(DFALSE);
	if (m_cStrings >= MAX_CREDIT_STRINGS) return(DFALSE);


	// Check if this is a special command string...

	if (_mbsnbcmp((const unsigned char*)sString, (const unsigned char*)">TIME:", 6) == 0)
	{
		if (_mbstrlen(sString) > 6)
		{
			m_fHoldTime = (float)atof(&sString[6]);
			return(DTRUE);
		}
	}


	// Create a new string via the engine...

	HSTRING hString = m_pClientDE->CreateString(sString);
	if (!hString) return(DFALSE);

	m_aStrings[m_cStrings++] = hString;


	// All done...

	return(DTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::SetStaticInfo
//
//	PURPOSE:	Sets all the static data that we'll need
//
// ----------------------------------------------------------------------- //

DBOOL CCredit::SetStaticInfo(CClientDE* pClientDE, int nMode, DBOOL bClearScreen)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);
	s_pClientDE = pClientDE;


	// Set the static screen info...

	s_hScreen = pClientDE->GetScreenSurface();
	if (!s_hScreen) return(DFALSE);

	pClientDE->GetSurfaceDims(s_hScreen, &s_dwScreenWidth, &s_dwScreenHeight);


	// Figure out the font size we should use...

	int xFont = 8;
	int yFont = 14;

	if (s_dwScreenWidth > 360)
	{
		xFont = 10;
		yFont = 16;
	}

	if (s_dwScreenWidth > 520)
	{
		xFont = 14;
		yFont = 22;
	}


	// Set the static font info...

	HSTRING hstrFont = pClientDE->FormatString(IDS_CREDITSFONT);
	if (hstrFont)
	{
		s_hFont = pClientDE->CreateFont(pClientDE->GetStringData(hstrFont), xFont, yFont, DFALSE, DFALSE, DFALSE);
		pClientDE->FreeString (hstrFont);
	}

	if (!s_hFont)
	{
		s_hFont = pClientDE->CreateFont("Arial", xFont, yFont, DFALSE, DFALSE, DFALSE);
		if (!s_hFont) return(DFALSE);
	}

	HSTRING hString = pClientDE->CreateString("Test");
	if (!hString) return(DFALSE);

	int nTemp;
	pClientDE->GetStringDimensions(s_hFont, hString, &nTemp, &s_nFontHeight);
	pClientDE->FreeString(hString);

	s_nFontSpacing = 4;


	// Set the default hold times base on the mode...

	if (nMode == CM_CREDITS)
	{
		s_fDefaultHoldTime = 4.0f;
		s_fDefaultOutTime = 2.0f;
	}

	if (nMode == CM_INTRO)
	{
		s_fDefaultHoldTime = 4.0f;
		s_fDefaultOutTime  = 1.2f;
	}


	// Set the clear-screen flag...

	s_bClearScreen = bClearScreen;


	// All done...

	return(DTRUE);
}

void CCredit::TermStaticInfo()
{
	if (s_pClientDE && s_hFont)
	{
		s_pClientDE->DeleteFont(s_hFont);
		s_hFont = NULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Draw
//
//	PURPOSE:	Draws all the strings of the credit
//
// ----------------------------------------------------------------------- //

void CCredit::Draw(int x, int y, HDECOLOR hForeColor)
{
	// Sanity checks...

	if (m_cStrings <= 0) return;


	// Figure out where to start drawing...

	int nTotalHeight = (m_cStrings * (s_nFontHeight + s_nFontSpacing));
	int yTop         = y - (nTotalHeight / 2);


	// Draw each string...

	int yCur = yTop + ((s_nFontHeight + s_nFontSpacing) / 2);

	for (int i = 0; i < m_cStrings; i++)
	{
		HDECOLOR hBackColor = m_pClientDE->SetupColor2(0, 0, 0, DFALSE);
		DrawString(GetString(i), hForeColor, hBackColor, x, yCur);
		yCur += (s_nFontHeight + s_nFontSpacing);
	}
}

void CCredit::DrawString(HSTRING hString, HDECOLOR hForeColor, HDECOLOR hBackColor, int x, int y)
{
	// Sanity checks...

	if (!hString) return;


	// Calculate the bounding rect of this string...

	int xString = 100;
	int yString = 100;

	m_pClientDE->GetStringDimensions(s_hFont, hString, &xString, &yString);

	DRect rcDraw;

	rcDraw.left   = x - (xString / 2);
	rcDraw.top    = y - (yString / 2);
	rcDraw.right  = s_dwScreenWidth - 1;
	rcDraw.bottom = s_dwScreenHeight - 1;


	// Draw the string to the screen surface...

	if (!s_bClearScreen) hBackColor = SETRGB_T(0, 0, 0);

	m_pClientDE->DrawStringToSurface(s_hScreen, s_hFont, hString, &rcDraw, hForeColor, hBackColor);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::Update
//
//	PURPOSE:	Updates the credit
//
//	RETURNS:	DFALSE = Stay with current credit
//				DTRUE  = Advance to next credit
//
// ----------------------------------------------------------------------- //

DBOOL CCredit::Update()
{
	// Sanity checks...

	if (!m_pClientDE) return(DFALSE);


	// Update based on our current state...

	switch(m_nState)
	{
		case CS_START:
		{
			SetState(CS_FADEIN);
			return(DFALSE);
		}

		case CS_FADEIN:
		{
			if (UpdateFadeIn())	SetState(CS_HOLDIN);
			return(DFALSE);
		}

		case CS_HOLDIN:
		{
			if (UpdateHoldIn()) SetState(CS_FADEOUT);
			return(DFALSE);
		}

		case CS_FADEOUT:
		{
			if (UpdateFadeOut()) SetState(CS_HOLDOUT);
			return(DFALSE);
		}

		case CS_HOLDOUT:
		{
			if (UpdateHoldOut()) SetState(CS_DONE);
			return(DFALSE);
		}

		case CS_DONE:
		{
			return(DTRUE);
		}

		default:
		{
			return(DTRUE);
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
			m_fTimer = 0.5f;
			m_fColor = 0.0f;
			break;
		}

		case CS_HOLDIN:
		{
			m_fTimer = s_fDefaultHoldTime;
			m_fColor = 1.0f;
			if (m_fHoldTime > 0.0) m_fTimer = m_fHoldTime;
			break;
		}

		case CS_FADEOUT:
		{
			m_fTimer = 0.400f;
			m_fColor = 1.0f;
			break;
		}

		case CS_HOLDOUT:
		{
			m_fTimer = s_fDefaultOutTime;
			m_fColor = 1.0f;
			break;
		}

		case CS_DONE:
		{
			break;
		}
	}

	m_fTimerStart = m_fTimer;
	m_fColorStart = m_fColor;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredit::UpdateFadeIn
//
//	PURPOSE:	Updates the CS_FADEIN state
//
// ----------------------------------------------------------------------- //

DBOOL CCredit::UpdateFadeIn()
{
	// Update the timer value...

	DFLOAT fDelta = m_pClientDE->GetFrameTime();
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


	// Update our color value...

	m_fColor = 1.0f - (m_fTimer / m_fTimerStart);


	// Create the color stuff...

	if (m_fColor > 1.0f) m_fColor = 1.0f;

	HDECOLOR hColor = m_pClientDE->SetupColor1(m_fColor, 0, 0, DFALSE);


	// Draw the credit...

	Draw(s_dwScreenWidth / 2, s_dwScreenHeight / 2, hColor);


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

DBOOL CCredit::UpdateHoldIn()
{
	// Update the timer value...

	DFLOAT fDelta = m_pClientDE->GetFrameTime();
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


	// Create the color stuff...

	m_fColor = 1.0f;

	HDECOLOR hColor = m_pClientDE->SetupColor1(m_fColor, 0, 0, DFALSE);


	// Draw the credit...

	Draw(s_dwScreenWidth / 2, s_dwScreenHeight / 2, hColor);


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

DBOOL CCredit::UpdateFadeOut()
{
	// Update the timer value...

	DFLOAT fDelta = m_pClientDE->GetFrameTime();
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


	// Update our color value...

	m_fColor = m_fTimer / m_fTimerStart;


	// Create the color stuff...

	if (m_fColor < 0.0f) m_fColor = 0.0f;
	if (!s_bClearScreen && m_fColor < 0.1f)
	{
		m_fColor = 0.1f;
		m_fTimer = 0.0f;
	}

	HDECOLOR hColor = m_pClientDE->SetupColor1(m_fColor, 0, 0, DFALSE);


	// Draw the credit...

	Draw(s_dwScreenWidth / 2, s_dwScreenHeight / 2, hColor);


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

DBOOL CCredit::UpdateHoldOut()
{
	// Update the timer value...

	DFLOAT fDelta = m_pClientDE->GetFrameTime();
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

DBOOL CCredits::Init(CClientDE* pClientDE, CBloodClientShell* pClientShell, int nMode, DBOOL bClearScreen)
{
	// Sanity checks...

	if (!pClientDE) return(DFALSE);
	if (!pClientShell) return(DFALSE);

	Term();


	// Set simple members...

	m_pClientDE    = pClientDE;
	m_pClientShell = pClientShell;
	m_iCredit      = 0;
	m_nMode        = nMode;
	m_bDone        = DFALSE;
	m_bClearScreen = bClearScreen;
	s_fSpeed       = DEF_CREDITS_SPEED;


	// Init the screen size...

	m_hScreen = m_pClientDE->GetScreenSurface();
	if (!m_hScreen) return(DFALSE);

	m_pClientDE->GetSurfaceDims(m_hScreen, &m_szScreen.cx, &m_szScreen.cy);


	// Add all the credit objects...

	AddCredits();


	// All done...

	m_bInited = DTRUE;

	return(DTRUE);
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

	CCredit::TermStaticInfo();

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
	// Handle various kes...

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
			if (IsIntro())
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

		case VK_PAUSE:
		{
			if (IsIntro()) break;
			s_bPause ^= 1;
			break;
		}

		case VK_ESCAPE:
		{
			ExitToMainMenu();
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

void CCredits::ExitToMainMenu()
{
	m_bDone = DTRUE;
}


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

	if (!m_pClientDE) return;
	if (m_cCredits <= 0) return;


	// Clear the screen...

	if (m_bClearScreen)
	{
		m_pClientDE->ClearScreen(NULL, CLEARSCREEN_SCREEN);
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
		if (IsIntro()) m_bDone = DTRUE;
	}
	CCredit* pCredit = GetCredit(m_iCredit);
	if (pCredit) pCredit->SetState(nState);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCredits::DrawCenteredString
//
//	PURPOSE:	Draws the given string centered at the give coord
//
// ----------------------------------------------------------------------- //

void CCredits::DrawCenteredStringToScreen(HDEFONT hFont, HSTRING hString, HDECOLOR hForeColor, HDECOLOR hBackColor, int xDraw, int yDraw)
{
	// Sanity checks...

	if (!hFont) return;
	if (!hString) return;


	// Calculate the bounding rect of this string...

	int xString = 100;
	int yString = 100;

	m_pClientDE->GetStringDimensions(hFont, hString, &xString, &yString);

	DRect rcDraw;

	rcDraw.left   = xDraw - (xString / 2);
	rcDraw.top    = yDraw - (yString / 2);
	rcDraw.right  = m_szScreen.cx - 1;
	rcDraw.bottom = m_szScreen.cy - 1;


	// Draw the string to the screen surface...

	if (!m_bClearScreen) hBackColor = SETRGB_T(0, 0, 0);

	m_pClientDE->DrawStringToSurface(m_hScreen, hFont, hString, &rcDraw, hForeColor, hBackColor);
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

	if (!m_pClientDE) return;


	// Set the static info...

	if (!CCredit::SetStaticInfo(m_pClientDE, GetMode(), IsClearingScreen()))
	{
		return;
	}


	// Get the credits text buffer...

	char* sName = NULL;

#ifdef _ADDON
	if (IsIntro()) sName = "INTRO_AO";
	else sName = "CREDITS_AO";
#else
	if (IsIntro()) sName = "INTRO";
	else sName = "CREDITS";
#endif

	void* hModule = NULL;
	m_pClientDE->GetEngineHook("cres_hinstance", &hModule);

	char* sBuf = CreditsWin_GetTextBuffer(sName, hModule);
	if (!sBuf) return;


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

	if (!m_pClientDE) return;


	// Add the credit...

	CCredit* pCredit = GetCredit(m_cCredits);
	if (!pCredit) return;

	if (!pCredit->Init(m_pClientDE, sText)) return;


	// Inc our credit counter...

	m_cCredits++;
}





