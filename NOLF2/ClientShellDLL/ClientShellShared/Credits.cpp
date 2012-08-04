/****************************************************************************
;
;	 MODULE:		Credits (.CPP)
;
;	PURPOSE:		Credits class
;
;	HISTORY:		07/24/98 [blg] This file was created
;					07/26/00 [jrg] modified for NOLF
;
;	Copyright (c) 1997-2001 Monolith Productions, Inc.  All Rights Reserved
;
****************************************************************************/


// Includes...

#include "stdafx.h"
#include "GameClientShell.h"
#include "Credits.h"
#include <stdio.h>
#include <mbstring.h>

// Macros...



LTFLOAT		CCredits::s_fSpeed = DEF_CREDITS_SPEED;


namespace
{
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
	m_hAlign = CUI_HALIGN_CENTER;

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

	// All done...
	FormatStrings();

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
	for (uint16 i = 0; i < m_Strings.size(); i++)
	{
		g_pFontManager->DestroyPolyString(m_Strings[i]);
	}
	m_Strings.clear();

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

	// Check if this is a special command string...

	if (strncmp(sString, ">TIME:", 6) == 0)
	{
		if (_mbstrlen(sString) > 6)
		{
			m_fHoldTime = (float)atof(&sString[6]);
			return(LTTRUE);
		}
	}
	if (strncmp(sString, ">POS:UL", 7) == 0)
	{
		m_hAlign = CUI_HALIGN_LEFT;
		m_Pos = s_PositionUL;
		m_ePosition = CP_UL;
		return(LTTRUE);
	}
	if (strncmp(sString, ">POS:UR", 7) == 0)
	{
		m_hAlign = CUI_HALIGN_RIGHT;
		m_Pos = s_PositionUR;
		m_ePosition = CP_UR;
		return(LTTRUE);
	}
	if (strncmp(sString, ">POS:LR", 7) == 0)
	{
		m_hAlign = CUI_HALIGN_RIGHT;
		m_Pos = s_PositionLR;
		m_ePosition = CP_LR;
		return(LTTRUE);
	}
	if (strncmp(sString, ">POS:LL", 7) == 0)
	{
		m_hAlign = CUI_HALIGN_LEFT;
		m_Pos = s_PositionLL;
		m_ePosition = CP_LL;
		return(LTTRUE);
	}
	if (strncmp(sString, ">BIG", 4) == 0)
	{
		m_bBig = LTTRUE;
		return(LTTRUE);
	}

	uint8 nFont = 0;
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);
	CUIFormattedPolyString* pStr = g_pFontManager->CreateFormattedPolyString(pFont,sString,(float)m_Pos.x,(float)m_Pos.y);
	if (pStr)
		m_Strings.push_back(pStr);

	return(LTTRUE);
}

void CCredit::FormatStrings()
{
	float x = (float)m_Pos.x * g_pInterfaceResMgr->GetXRatio();
	float y = (float)m_Pos.y * g_pInterfaceResMgr->GetYRatio();
	FPStringArray::iterator iter = m_Strings.begin();
	while (iter != m_Strings.end())
	{
		CUIFormattedPolyString* pStr = *iter;

		uint8 nSize = (uint8)(16.0f * g_pInterfaceResMgr->GetXRatio());
		if (m_bBig)
			nSize = (uint8)(24.0f * g_pInterfaceResMgr->GetYRatio());


		pStr->SetCharScreenHeight(nSize);
		pStr->SetColor(argbWhite);
		pStr->SetAlignmentH(m_hAlign);
		pStr->SetPosition(x,y);
		y += (float)pStr->GetHeight();

		iter++;

	}

	float offset = y - ((float)m_Pos.y * g_pInterfaceResMgr->GetYRatio());
	switch (m_ePosition)
	{
	case CP_CENTER:
		offset /= 2.0f;
		break;
	case CP_UR:
	case CP_UL:
		offset = 0.0f;
		break;
	}

	if (offset > 0.0f)
	{
		FPStringArray::iterator iter = m_Strings.begin();
		while (iter != m_Strings.end())
		{
			CUIFormattedPolyString* pStr = *iter;

			float x,y;
			pStr->GetPosition(&x,&y);

			y -= offset;

			pStr->SetPosition(x,y);

			iter++;

		}

	}
	

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

	if (m_Strings.size() <= 0) return;

	FPStringArray::iterator iter = m_Strings.begin();
	while (iter != m_Strings.end())
	{
		CUIFormattedPolyString* pStr = (*iter);
		float x;
		float y;
		pStr->GetPosition(&x,&y);

		//drop shadow
		pStr->SetPosition(x+2.0f,y+2.0f);
		pStr->SetColor(argbBlack);
		pStr->Render();

		pStr->SetPosition(x,y);
		pStr->SetColor(argbWhite);
		pStr->Render();
		iter++;
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

	uint8 a = (uint8)( 255.0f * m_fAlpha );

	uint32 color = SET_ARGB(a,0xFF,0xFF,0xFF);

	FPStringArray::iterator iter = m_Strings.begin();
	while (iter != m_Strings.end())
	{
		(*iter)->SetColor(argbWhite);
		iter++;
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

	uint8 a = (uint8)( 255.0f * m_fAlpha );

	uint32 color = SET_ARGB(a,0xFF,0xFF,0xFF);

	FPStringArray::iterator iter = m_Strings.begin();
	while (iter != m_Strings.end())
	{
		(*iter)->SetColor(argbWhite);
		iter++;
	}

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


void CCredit::Clear()
{
	m_nState    = CS_START;
	m_fTimer    = 0.0f;
	m_fHoldTime = -1.0f;
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

	if (m_Credits.size())
		m_bInited = LTTRUE;

	return(LTTRUE);
}

void CCredits::Clear()
{

	m_bInited      = LTFALSE;
	m_bDone        = LTTRUE;

	m_iCredit  = 0;
	m_nMode    = CM_CREDITS;

	s_fSpeed = DEF_CREDITS_SPEED;
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

	CreditArray::iterator iter = m_Credits.begin();
	while (iter != m_Credits.end())
	{
		
		(*iter)->Term();
		debug_delete( (*iter) );
		iter++;

	}

	m_Credits.clear();


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
	if (m_Credits.size() <= 0) return;


	// Clear the screen...

	if (m_bClearScreen)
	{
		g_pLTClient->ClearScreen(NULL, CLEARSCREEN_SCREEN, 0);
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
	if(m_iCredit >= m_Credits.size())
	{
		m_iCredit = 0;
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

		if (*sBuf == '#' && *(sBuf+1) == '#')
		{
			sCredit[i] = '\0';

			if (strncmp(sCredit, ">END", 4) == 0)	// end?
			{
				return;
			}

			AddCredit(sCredit);
			i = 0;

			sBuf+=2;

			while (*sBuf != '\0' && ((*sBuf == '\n') || (*sBuf == '\r'))) sBuf++;
		}
		else
		{
			memcpy(&sCredit[i], sBuf, 1);
			i++;
			sBuf++;
		}
	}
}

void CCredits::AddCredit(char* sText)
{
	// Sanity checks...

	if (!g_pLTClient) return;


	// Add the credit...

	CCredit* pCredit = debug_new(CCredit);
	if (!pCredit->Init(sText))
	{
		debug_delete(pCredit);
		return;
	}

	// Inc our credit counter...

	m_Credits.push_back(pCredit);
}


CCredit* CCredits::GetCredit(uint16 iCredit)
{
	if (iCredit >= m_Credits.size()) return(NULL);
	if (iCredit < 0) return(NULL);
	return(m_Credits[iCredit]);
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
