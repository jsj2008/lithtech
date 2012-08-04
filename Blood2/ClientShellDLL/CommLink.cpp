//*************************************************************************
//*************************************************************************
//***** MODULE  : CommLink.cpp
//***** PURPOSE : Blood 2 Communcation Tab
//***** CREATED : 9/29/98
//*************************************************************************
//*************************************************************************

#include "CommLink.h"
#include "ClientUtilities.h"
#include "BloodClientShell.h"
#include <stdio.h>
#include "SoundTypes.h"


//*************************************************************************
//*****	Function:	CCommLink()
//*****	Details:	Constructor
//*************************************************************************

CCommLink::CCommLink()
{
	m_pClientDE = 0;

	// Surfaces
	m_hFace = 0;
	m_hText = 0;

	// Fonts and cursors
	m_pCommCursor = 0;
	m_pCommFont1 = 0;

	m_nCommLevel = COMMTAB_INVISIBLE;
	m_nTextLevel = COMMTEXT_INVISIBLE;

	memset(m_szCommVoice, 0, COMM_MAX_VOICE);
	memset(m_szCommText, 0, COMM_MAX_TEXT);

	m_fCommUpdateTime = 0.0f;
	m_fCommScrollRatio = 0.0f;
	m_fCommLength = 0.0f;

	m_fTextUpdateTime = 0.0f;
	m_fTextScrollRatio = 0.0f;
	m_fTextScrollTime = 0.0f;
	m_fTextLength = 0.0f;

	m_sCommSound = 0;
	m_bPlayVoice = DFALSE;

	m_nCommTextWidth = 0;
	m_nFaceWidth = 0;
	m_nCommTextX = 0;
	m_nCommTextY = 0;

	// General
	m_nScreenWidth = 0;
	m_nScreenHeight = 0;
	m_hTransColor = 0;
	m_hTextColor = 0;
}

//*************************************************************************
//*****	Function:	~CCommLink()
//*****	Details:	Destructor
//*************************************************************************

CCommLink::~CCommLink()
{
	if( m_sCommSound )
		g_pClientDE->KillSound( m_sCommSound );
}

//*************************************************************************
//*****	Function:	Init(CClientDE* pClientDE)
//*****	Details:	Initializes the status screens
//*************************************************************************

DBOOL CCommLink::Init(CClientDE* pClientDE)
{
	if (!pClientDE) return DFALSE;
	Term();

	m_pClientDE = pClientDE;

	// Initialize the graphic surfaces
	m_hFace		= m_pClientDE->CreateSurfaceFromBitmap("interface/commlink/c_comm.pcx");

	// Create a font cursor
	m_pCommCursor = new CoolFontCursor();

	// Setup fonts
	m_pCommFont1 = new CoolFont();
	m_pCommFont1->Init(m_pClientDE, "interface/commlink/comm_font_1.pcx");
	m_pCommFont1->LoadXWidths("interface/commlink/comm_font_1.fnt");

	// Setup transparency color
	m_hTransColor = m_pClientDE->SetupColor1(1.0f, 0.0f, 1.0f, DFALSE);
	m_hTextColor = m_pClientDE->SetupColor1(1.0f, 1.0f, 1.0f, DFALSE);

	AdjustRes();
	return DTRUE;
}

//*************************************************************************
//*****	Function:	AdjustRes()
//*****	Details:	Sets new locations for status tabs if the game resolution changes
//*************************************************************************

void CCommLink::AdjustRes()
{
	if (!m_pClientDE) return;

	// Check to see if the screen resolution has changed... if not then return
	DDWORD	width, height;
	HSURFACE hScreen = m_pClientDE->GetScreenSurface();

	m_pClientDE->GetSurfaceDims(hScreen, &width, &height);
	if((m_nScreenWidth == width) && (m_nScreenHeight == height))	return;

	// Set the screen width and height variables to the new resolution
	m_nScreenWidth = width;
	m_nScreenHeight = height;

	// Adjust the commlink tab location to the new resolution
	m_pClientDE->GetSurfaceDims(m_hFace, &width, &height);
	m_nFaceWidth = width;
	m_nCommTextWidth = (short)(m_nScreenWidth - width - 20);
	m_nCommTextX = width + 10;
	m_nCommTextY = 10;

	// Delete the current text surface and make a new one that's the right size
	if(m_hText)		{ m_pClientDE->DeleteSurface(m_hText); m_hText = 0; }
	m_hText = m_pClientDE->CreateSurface(m_nCommTextWidth, m_pCommFont1->height * COMM_MAX_LINES);
	m_pClientDE->FillRect(m_hText, DNULL, m_hTransColor);
}

//*************************************************************************
//*****	Function:	Term()
//*****	Details:	Terminates the status screens
//*************************************************************************

void CCommLink::Term()
{
	if(m_pClientDE)
	{
		if(m_hFace)		{ m_pClientDE->DeleteSurface(m_hFace); m_hFace = 0; }
		if(m_hText)		{ m_pClientDE->DeleteSurface(m_hText); m_hText = 0; }

		if(m_pCommCursor)	{ delete m_pCommCursor; m_pCommCursor = 0; }
		if(m_pCommFont1)	{ m_pCommFont1->Free(); delete m_pCommFont1; m_pCommFont1 = 0; }

		m_pClientDE = 0;
	}
}

//*************************************************************************
//*****	Function:	StartCommunication()
//*****	Details:	Setup the communication and turn it on
//*************************************************************************

char CCommLink::StartCommunication(DBYTE nPic, char *szFile, char *szText)
{
	if(!m_pClientDE || m_nCommLevel)	return	COMM_BUSY;

	m_nCommLevel = COMMTAB_SCROLL_IN;
	if( m_sCommSound )
	{	
		g_pClientDE->KillSound( m_sCommSound );
		m_sCommSound = 0;
	}

	// Clear and reinit the voice sound and text strings
	memset(m_szCommVoice, 0, COMM_MAX_VOICE);
	_mbscpy((unsigned char*)m_szCommVoice, (const unsigned char*)szFile);
	memset(m_szCommText, 0, COMM_MAX_TEXT);
	_mbscpy((unsigned char*)m_szCommText, (const unsigned char*)szText);

	// Get an average time to display the text, just in case the sound isn't there
	m_fCommLength = _mbstrlen(szText) / 15.0f;
	if(m_fCommLength < 2.5f)	m_fCommLength = 2.5f;

	m_fTextLength = m_fCommLength / 1.5f;
	m_fTextScrollTime = (m_fCommLength - m_fTextLength) / 2.0f;

	// Check to see if there is a valid sound file to use
	if(szFile && _mbscmp((const unsigned char*)szFile, (const unsigned char*)"default.wav"))
		m_bPlayVoice = DTRUE;
	else
		m_bPlayVoice = DFALSE;

	// Get the time that we start so that it can animate properly
	m_fCommUpdateTime = m_pClientDE->GetTime();

	// Setup the new picture and resize accordingly
	char	string1[100] = "interface/commlink/";

	switch(nPic)
	{
		case COMM_CALEB:		_mbscat((unsigned char*)string1, (const unsigned char*)"c_comm.pcx");	break;
		case COMM_OPHELIA:		_mbscat((unsigned char*)string1, (const unsigned char*)"o_comm.pcx");	break;
		case COMM_ISHMAEL:		_mbscat((unsigned char*)string1, (const unsigned char*)"i_comm.pcx");	break;
		case COMM_GABREILLA:	_mbscat((unsigned char*)string1, (const unsigned char*)"g_comm.pcx");	break;
		case COMM_VOICE:		_mbscat((unsigned char*)string1, (const unsigned char*)"v_comm.pcx");	break;
		default:				_mbscat((unsigned char*)string1, (const unsigned char*)"u_comm.pcx");	break;
	}

	if(m_hFace)		{ m_pClientDE->DeleteSurface(m_hFace); m_hFace = 0; }
	m_hFace = m_pClientDE->CreateSurfaceFromBitmap(string1);

	AdjustRes();
	return	COMM_STARTED;
}

//*************************************************************************
//*****	Function:	StartCommunication()
//*****	Details:	Setup the communication and turn it on
//*************************************************************************

char CCommLink::StartCommunication(char *szPic, char *szFile, char *szText)
{
	if(!m_pClientDE || m_nCommLevel)	return	COMM_BUSY;

	m_nCommLevel = COMMTAB_SCROLL_IN;
	if( m_sCommSound )
	{	
		g_pClientDE->KillSound( m_sCommSound );
		m_sCommSound = 0;
	}

	// Clear and reinit the voice sound and text strings
	memset(m_szCommVoice, 0, COMM_MAX_VOICE);
	_mbscpy((unsigned char*)m_szCommVoice, (const unsigned char*)szFile);
	memset(m_szCommText, 0, COMM_MAX_TEXT);
	_mbscpy((unsigned char*)m_szCommText, (const unsigned char*)szText);

	// Get an average time to display the text, just in case the sound isn't there
	m_fCommLength = _mbstrlen(szText) / 15.0f;
	if(m_fCommLength < 2.5f)	m_fCommLength = 2.5f;

	m_fTextLength = m_fCommLength / 1.5f;
	m_fTextScrollTime = m_fTextLength / 2.0f;

	// Check to see if there is a valid sound file to use
	if(szFile && _mbscmp((const unsigned char*)szFile, (const unsigned char*)"default.wav"))
		m_bPlayVoice = DTRUE;
	else
		m_bPlayVoice = DFALSE;

	// Get the time that we start so that it can animate properly
	m_fCommUpdateTime = m_pClientDE->GetTime();

	// Setup the new picture and resize accordingly
	char	string1[100] = "interface/commlink/";

	if(szPic)	_mbscat((unsigned char*)string1, (const unsigned char*)szPic);
		else	_mbscat((unsigned char*)string1, (const unsigned char*)"c_comm.pcx");

	if(m_hFace)		{ m_pClientDE->DeleteSurface(m_hFace); m_hFace = 0; }
	m_hFace = m_pClientDE->CreateSurfaceFromBitmap(string1);

	AdjustRes();
	return	COMM_STARTED;
}

//*************************************************************************
//*****	Function:	Draw()
//*****	Details:	Draw the picture and text
//*************************************************************************

void CCommLink::Draw(DBOOL bDrawBar)
{
	if(!m_pClientDE || !m_nCommLevel || !bDrawBar) return;

	HSURFACE	hScreen	= m_pClientDE->GetScreenSurface();
	DFLOAT		fTime = m_pClientDE->GetTime();

	if (!hScreen) return;

	AdjustRes();

	m_pCommCursor->SetFont(m_pCommFont1);
	m_pCommCursor->SetDest(m_hText);
	m_pCommCursor->SetJustify(CF_JUSTIFY_LEFT);
	m_pCommCursor->SetLoc(0, 0);

	// Handle the scrolling in and out of the inventory item tabs
	switch(m_nCommLevel)
	{
		case	COMMTAB_SCROLL_IN:
			if(fTime - m_fCommUpdateTime > COMM_SCROLL_TIME)
			{
				m_fCommScrollRatio = 1.0f;
				m_nCommLevel = COMMTAB_STOPPED;
				m_fCommUpdateTime = fTime;

				m_nTextLevel = COMMTEXT_SCROLL_IN;
				m_fTextUpdateTime = fTime;

				if(m_bPlayVoice)
				{
					g_pBloodClientShell->GetVoiceMgr()->StopAll();	// [blg] Stop any voice mgr sounds

					if( m_sCommSound )
					{	
						g_pClientDE->KillSound( m_sCommSound );
						m_sCommSound = 0;
					}
					m_sCommSound = PlaySoundLocal(m_szCommVoice, SOUNDPRIORITY_MISC_HIGH, DFALSE, DTRUE, DTRUE, DFALSE, 100);

					if (m_sCommSound)	// [blg] Don't let voice mgr interrupt
					{
						DFLOAT  fSoundTime = 3;

						m_pClientDE->GetSoundDuration(m_sCommSound, &fSoundTime);
						g_pBloodClientShell->GetVoiceMgr()->SetNextPlayTime(fSoundTime + 0.5f);
					}
				}
			}
			else
			{
				m_fCommScrollRatio = (fTime - m_fCommUpdateTime) / COMM_SCROLL_TIME;

			}
			break;

		case	COMMTAB_STOPPED:
//			if(!m_sCommSound || !m_bPlayVoice)
//			{
				if(fTime - m_fCommUpdateTime > m_fCommLength)
				{
					m_nCommLevel = COMMTAB_SCROLL_OUT;
					m_fCommUpdateTime = fTime;
					if(m_sCommSound)
					{
						m_pClientDE->KillSound(m_sCommSound);
						m_sCommSound = DNULL;
					}

				}
//			}
//			else if(m_bPlayVoice && m_pClientDE->IsDone(m_sCommSound))
//			{
//				m_nCommLevel = COMMTAB_SCROLL_OUT;
//				m_fCommUpdateTime = fTime;
//				if(m_sCommSound)	m_pClientDE->KillSound(m_sCommSound);
//			}
			break;

		case	COMMTAB_SCROLL_OUT:
			if(fTime - m_fCommUpdateTime > COMM_SCROLL_TIME)
			{
				m_fCommScrollRatio = 0.0f;
				m_nCommLevel = COMMTAB_INVISIBLE;
				m_fCommUpdateTime = fTime;
			}
			else
				m_fCommScrollRatio = 1.0f - (fTime - m_fCommUpdateTime) / COMM_SCROLL_TIME;

			break;
	}

	// Handle the text scrolling ratios and stuff...
	switch(m_nTextLevel)
	{
		case	COMMTEXT_SCROLL_IN:
			if(fTime - m_fTextUpdateTime > m_fTextScrollTime)
			{
				m_nTextLevel = COMMTEXT_STOPPED;
				m_fTextUpdateTime = fTime;
			}
			else
			{
				m_fTextScrollRatio = (fTime - m_fTextUpdateTime) / m_fTextScrollTime;

				m_pClientDE->FillRect(m_hText, DNULL, m_hTransColor);
				m_pCommCursor->DrawFormatTimed(m_szCommText, m_nCommTextWidth, m_fTextScrollRatio, DTRUE);
			}
			break;

		case	COMMTEXT_STOPPED:
			if(fTime - m_fTextUpdateTime > m_fTextLength)
			{
				m_nTextLevel = COMMTEXT_SCROLL_OUT;
				m_fTextUpdateTime = fTime;
			}
			else
			{
				m_pClientDE->FillRect(m_hText, DNULL, m_hTransColor);
				m_pCommCursor->DrawFormatTimed(m_szCommText, m_nCommTextWidth, 1.0f, DTRUE);
			}
			break;

		case	COMMTEXT_SCROLL_OUT:
			if(fTime - m_fTextUpdateTime > m_fTextScrollTime)
			{
				m_nTextLevel = COMMTEXT_INVISIBLE;
				m_fTextUpdateTime = fTime;
			}
			else
			{
				m_fTextScrollRatio = (fTime - m_fTextUpdateTime) / m_fTextScrollTime;

				m_pClientDE->FillRect(m_hText, DNULL, m_hTransColor);
				m_pCommCursor->DrawFormatTimed(m_szCommText, m_nCommTextWidth, m_fTextScrollRatio, DFALSE);
			}
			break;
	}

	// Draw the face to the screen
	if(m_hFace)
	{
		DDWORD nNewX = (DDWORD)(m_nFaceWidth * m_fCommScrollRatio) - m_nFaceWidth;
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hFace, DNULL, nNewX, m_nCommTextY, m_hTransColor);
	}

	// Draw the text to the screen
	if(m_hText && m_nTextLevel)
		m_pClientDE->DrawSurfaceToSurfaceTransparent(hScreen, m_hText, DNULL, m_nCommTextX, m_nCommTextY, m_hTransColor);
}