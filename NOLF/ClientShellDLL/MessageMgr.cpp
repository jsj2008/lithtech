//------------------------------------------------------------------------
//
// MODULE  : MessageMgr.cpp
//
// PURPOSE : Messaging system - ruthlessly stolen from Greg Kettell (B2)
//
// CREATED : 10/22/97
//
// REVISED : 10/27/99 - jrg
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
//------------------------------------------------------------------------

#include "stdafx.h"
#include "GameClientShell.h"
#include "MessageMgr.h"
#include "VKDefs.h"
#include "stdio.h"
#include "MsgIDs.h"
#include "SoundTypes.h"
#include "ClientRes.h"
#include "InterfaceResMgr.h"
#include "VarTrack.h"

VarTrack		g_vtMaxNumMessages;


CMessageMgr*    g_pMessageMgr   = LTNULL;
CCheatMgr*      g_pCheatMgr     = LTNULL;
LTBOOL           g_bInfiniteAmmo = LTFALSE;
LTBOOL           g_bAllowAllMissions = LTFALSE;

extern CGameClientShell*	g_pGameClientShell;

void MikeDCallBack(LTBOOL bReturn, void *pData)
{
	if (bReturn)
	{
		g_pLTClient->Shutdown();
	}
	else
	{
		g_pClientSoundMgr->PlayInterfaceSound("voice\\14000.wav");
	}
}

CCheatMgr::CheatInfo CCheatMgr::s_CheatInfo[] = {
    { "LPVIdI^\\OW[^Hf",    LTFALSE },	// GOD:    mpimyourfather
    { "LPTABI_NLW[^fXRd",	LTFALSE },	// AMMO:   mpwegotdeathstar
    { "LPTKK@N\\KZV",       LTFALSE },	// ARMOR:  mpwonderbra
    { "LPCPAOI^c",			LTFALSE },	// HEALTH: mpdrdentz
    { "LP@JT\\",			LTFALSE },	// mpclip
    { "LPSAIO[W[\\",		LTFALSE },	// mpteleport
    { "LP_K^",				LTFALSE },	// mppos
    { "LPPMKPJ",			LTFALSE },	// KFA:			mpsanta
    { "LPJUHSVQ",			LTFALSE },	// mpmimimi		- all weapons
    { "LPT^J]",				LTFALSE },  // mpwpos       - toggle position weapon adjustment
    { "LPTI]I\\",			LTFALSE },  // mpwmpos      - toggle position weapon muzzle adjustment
    { "LP@MHO]I",			LTFALSE },  // mpcamera     - toggle camera offset adjustment
    { "LPKUBT_[JWSI",		LTFALSE },  // mplightscale - toggle light scale adjustment
    { "LPKUBT_IML",			LTFALSE },  // mplightadd   - toggle light add adjustment
    { "LPEKS",				LTFALSE },  // mpfov        - toggle fov adjustment
    { "LP_KIc",				LTFALSE },  // mppoly       - toggle menu polygrid adjustment
    { "LPHUKAH@]K_QRRdfPb`",LTFALSE },  // mpkingoftehmonstars  - All weapons, infinite ammo
    { "LPAKdS\\]JQ",		LTFALSE },  // mpboyisuck   - Remove all AI in the level
    { "LPSPTA@M[Y",			LTFALSE },  // mptriggers   - Toggle trigger boxes on/off
    { "LPAP@KLR",			LTFALSE },  // mpbreach     - Toggle breach adjust on/off
    { "LP@MH",				LTFALSE },  // mpcam        - Toggle 1st person camera adjust on/off
    { "LPQMNO]LVo",			LTFALSE },  // mpracerboy   - Spawn in a motorcycle
    { "LPQK^OM]M",			LTFALSE },  // mprosebud	- Spawn in a snowmobile
    { "LPDKLP_MJP",			LTFALSE },  // mpgoattech   - get all of the mods for currently held weapons
    { "LPfKPHHWRT^_HkPe_SRR\\gXXb[~",   LTFALSE }, // mpyoulooklikeyouneedamonkey       - All gear
    { "LPN_^MJU",			LTFALSE },  // mpasscam     - toggle chase view
    { "LPAA@J_RLZJ",		LTFALSE },  // mpbeenthere  - allow all missions
    { "LPTVJKVQ",			LTFALSE },  // mpwhoami     - reset player history
    { "LP@MH\\H[",			LTFALSE },  // mpcampos     - toggle camera pos/rot
    { "LPWKJUVM\\X",		LTFALSE },  // mphookmeup   - give the player mission default inventory
    { "LPJM]THVL",			LTFALSE },  // mpmaphole    - exit the current level
    { "LPAQTHO",			LTFALSE },	// mpbuild      - build info
    { "LPJUVOO",			LTFALSE },  // mpmiked      - mike d!
    { "LPBfJ^MQ]WUZT_Pe_d`",LTFALSE }   // mpexorbitantamounts - lots o blood
};


LTBOOL CCheatMgr::m_bPlayerCheated = LTFALSE;

namespace
{
    LTVector vMsgColor[MMGR_NUM_MESSAGE_TYPES];
    HLTCOLOR hTransColor;
}



/*******************************************************************************

	CMessageMgr

*******************************************************************************/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::CMessageMgr()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

CMessageMgr::CMessageMgr()
{
	g_pMessageMgr = this;
	m_x	= 15;
	m_y	= 0;

	m_nMaxMessages	= 5;
	m_fMessageTime	= 5.0;
	m_fMessageFade	= 1.0;

	m_nMessageCount	= 0;
	m_nFirstMessage	= 0;
	m_nNextMessage	= m_nFirstMessage;
    m_bEditing = LTFALSE;
    m_bTeamMsg = LTFALSE;

    m_bEnabled = LTFALSE;

    m_pForeFont = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Init (CClientDE* pClientDE)
//
//	PURPOSE:	Initializes the Message manager
//
// ----------------------------------------------------------------------- //

LTBOOL CMessageMgr::Init ()
{

    if (!m_InputLine.Init()) return LTFALSE;

	m_pForeFont = g_pInterfaceResMgr->GetMsgForeFont();

	hTransColor = kTransBlack;
	vMsgColor[MMGR_DEFAULT].Init(1.0f, 1.0f, 1.0f);
	vMsgColor[MMGR_CHAT].Init(1.0f, 1.0f, 0.5f);
	vMsgColor[MMGR_PICKUP].Init(0.4f, 0.0f, 0.7f);
	vMsgColor[MMGR_TEAMCHAT].Init(0.0f, 1.0f, 0.0f);
	vMsgColor[MMGR_TEAM_1].Init(0.3f, 0.3f, 1.0f);
	vMsgColor[MMGR_TEAM_2].Init(1.0f, 0.3f, 0.3f);

	m_nMaxMessages	= g_pLayoutMgr->GetMaxNumMessages();
	m_fMessageTime	= g_pLayoutMgr->GetMessageTime();
	m_fMessageFade	= g_pLayoutMgr->GetMessageFade();

    g_vtMaxNumMessages.Init(g_pLTClient, "MaxNumMessages", NULL, 5.0f);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Enable()
//
//	PURPOSE:	Sets the enabled flag
//
// ----------------------------------------------------------------------- //

void CMessageMgr::Enable( LTBOOL bEnabled )
{
	if (m_bEnabled && !bEnabled)
		Clear();

	m_bEnabled = bEnabled;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::AddLine()
//
//	PURPOSE:	Adds a new line to the Message buffer
//
// ----------------------------------------------------------------------- //

void CMessageMgr::AddLine( char *szMsg, eMessageType eType, HSURFACE hSurf )
{
	if (szMsg)
	{
        HSTRING hString = g_pLTClient->CreateString(szMsg);
		AddLine(hString, eType, hSurf);
        g_pLTClient->FreeString(hString);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::AddLine()
//
//	PURPOSE:	Adds a new line to the Message buffer
//
// ----------------------------------------------------------------------- //

void CMessageMgr::AddLine( int nStringId, eMessageType eType, HSURFACE hSurf )
{
    HSTRING hString = g_pLTClient->FormatString(nStringId);
	AddLine(hString, eType, hSurf);
    g_pLTClient->FreeString(hString);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::AddLine()
//
//	PURPOSE:	Adds a new line to the Message buffer
//
// ----------------------------------------------------------------------- //

void CMessageMgr::AddLine( HSTRING hMsg, eMessageType eType, HSURFACE hSurf )
{
	LTFLOAT fDuration = m_fMessageTime;
	if (eType == MMGR_PICKUP)
		fDuration = GetConsoleFloat("PickupMessageDuration",m_fMessageTime);
	if (fDuration <= 0.0f) return;

	Message *pMsg = &m_Messages[ m_nNextMessage ];

	// Set next Message pointer
	m_nNextMessage = (m_nNextMessage + 1) % kMaxMessages;

	m_nMaxMessages = (int)g_vtMaxNumMessages.GetFloat();
	if (m_nMaxMessages > kMaxMessages)
	{
		m_nMaxMessages = kMaxMessages;
		g_vtMaxNumMessages.SetFloat((float)m_nMaxMessages);
	}
	else if (m_nMaxMessages < 1)
	{
		m_nMaxMessages = 1;
		g_vtMaxNumMessages.SetFloat((float)m_nMaxMessages);
	}

	// don't overflow the maximum number of Messages
	++m_nMessageCount;
	if (  m_nMessageCount > m_nMaxMessages )
	{
		// refresh the screen to get rid of oldest message
		g_pInterfaceMgr->AddToClearScreenCount();

	}
	while (  m_nMessageCount > m_nMaxMessages )
	{

		// expire the oldest Message and recalculate the first Message in the buffer
		Message *pOld = &m_Messages[ m_nFirstMessage ];

		DeleteMessageData(pOld);

		m_nFirstMessage = (m_nFirstMessage + 1) % kMaxMessages;

		// reset the Message count to max
		--m_nMessageCount;
	}

    char* pStr = g_pLTClient->GetStringData(hMsg);
    g_pLTClient->CPrint(pStr);

	pMsg->fExpiration = g_pLTClient->GetTime() + fDuration;

	pMsg->eType = eType;

    uint32 surfHt = 0;
    uint32 surfWd = 0;
	if (hSurf)
	{
        g_pLTClient->GetSurfaceDims(hSurf,&surfWd,&surfHt);
		pMsg->textOffset.x = (int)surfWd + 8;
	}
	else
		pMsg->textOffset.x = 0;

	int nWidth = (int)(480.0f * g_pInterfaceResMgr->GetXRatio());

    LTIntPt txtSize = m_pForeFont->GetTextExtentsFormat(hMsg,nWidth);

	if (txtSize.y > (int)surfHt)
	{
        pMsg->textOffset.y = 0;
	}
	else
		pMsg->textOffset.y = ((int)surfHt - txtSize.y) /2;

	pMsg->hForeText = g_pInterfaceResMgr->CreateSurfaceFromString(m_pForeFont,hMsg,kTransBlack,0,0,nWidth);
	pMsg->nHeight = Max((int)surfHt,txtSize.y);

    g_pLTClient->OptimizeSurface(pMsg->hSurface,kTransBlack);
	pMsg->hSurface = hSurf;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Draw()
//
//	PURPOSE:	Displays the Message list
//
// ----------------------------------------------------------------------- //

void CMessageMgr::Draw( void )
{
	// early return if there are no Messages in the queue,
	// or Messages are disabled, or while typing Messages
	if ( !m_bEditing && (m_nMessageCount == 0 || !m_bEnabled) )
		return;

	int nCount = m_nMessageCount;	// use temporary values
	int nFirst = m_nFirstMessage;	// because the real ones may change
	int i;

//	int nShade = ClipHigh(nCount * 8, 48);
//	g_pGameClientShell->AddToClearScreenCount();


    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 nScreenHeight, nScreenWidth;
    g_pLTClient->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);

	float xRatio			= (float)nScreenWidth / 640.0f;
	float yRatio			= (float)nScreenHeight / 480.0f;

	int y = 5;
	int baseX = 5;

	for ( i = 0; i < nCount; i++)
	{
		LTFLOAT fAlpha = 1.0f;
		uint32  iAlpha = 0xFF000000;
		int x = baseX;
		Message *pMsg = &m_Messages[ (nFirst + i) % kMaxMessages ];

		// expire Messages that have overstayed their welcome
        if ( g_pLTClient->GetTime() >= pMsg->fExpiration )
		{
			g_pInterfaceMgr->AddToClearScreenCount();

			DeleteMessageData(pMsg);
			// expire the oldest Message and recalculate the Message position and counter
			m_nFirstMessage = (m_nFirstMessage + 1) % kMaxMessages;
			m_nMessageCount--;

			// don't draw the expired Message
			continue;
		}
		else if (m_fMessageFade > 0.0f)
		{
            fAlpha = (pMsg->fExpiration - g_pLTClient->GetTime()) / m_fMessageFade;
			if (fAlpha < 1.0f)
                g_pLTClient->SetSurfaceAlpha(pMsg->hSurface,fAlpha);
		}


		if (pMsg->hForeText)
		{
			HLTCOLOR hForeColor = kWhite;
			HLTCOLOR hBackColor = kWhite;

			if (fAlpha < 1.0f)
			{

				uint8 fade = (uint8)(255.0f * fAlpha);
				hBackColor = SETRGB(fade,fade,fade);
				uint8 r = (uint8)(255.0f * vMsgColor[pMsg->eType].x * fAlpha);
				uint8 g = (uint8)(255.0f * vMsgColor[pMsg->eType].y * fAlpha);
				uint8 b = (uint8)(255.0f * vMsgColor[pMsg->eType].z * fAlpha);
				hForeColor = SETRGB(r,g,b);
			}
			else
			{
				uint8 r = (uint8)(255.0f * vMsgColor[pMsg->eType].x);
				uint8 g = (uint8)(255.0f * vMsgColor[pMsg->eType].y);
				uint8 b = (uint8)(255.0f * vMsgColor[pMsg->eType].z);
				hForeColor = SETRGB(r,g,b);

			}

			g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
			g_pLTClient->SetOptimized2DColor(hBackColor);
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen,pMsg->hForeText,LTNULL,x+pMsg->textOffset.x+1,y+pMsg->textOffset.y+1,LTNULL);
			g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ADD);
			g_pLTClient->SetOptimized2DColor(hForeColor);
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen,pMsg->hForeText,LTNULL,x+pMsg->textOffset.x,y+pMsg->textOffset.y,LTNULL);
			g_pLTClient->SetOptimized2DColor(kWhite);
			g_pLTClient->SetOptimized2DBlend(LTSURFACEBLEND_ALPHA);

		}

		if (pMsg->hSurface)
		{
            g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen,pMsg->hSurface,LTNULL,x,y,hTransColor);
		}
		y += pMsg->nHeight;


	}

	if (m_bEditing)
	{
		int y = (int) (yRatio * 420.0f);

		m_InputLine.Draw(hScreen, m_x, y - m_pForeFont->GetHeight());
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Clear()
//
//	PURPOSE:	removes all Messages from the array
//
// ----------------------------------------------------------------------- //

void CMessageMgr::Clear( void )
{
	int i;
	for ( i = 0; i < m_nMessageCount; i++)
	{
		Message *pMsg = &m_Messages[ (m_nFirstMessage + i) % kMaxMessages ];
		DeleteMessageData(pMsg);
	}

	m_nFirstMessage = m_nNextMessage = m_nMessageCount = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::SetMaxMessages()
//
//	PURPOSE:	Sets the maximum number of Messages to display
//
// ----------------------------------------------------------------------- //

void CMessageMgr::SetMaxMessages( int nMaxMessages )
{
	if (nMaxMessages < 1) nMaxMessages = 1;
	if (nMaxMessages > kMaxMessages) nMaxMessages = kMaxMessages;

	m_nMaxMessages = nMaxMessages;
}


void CMessageMgr::SetCoordinates( int x, int y )
{
	m_x = x;
	m_y = y;
}

void CMessageMgr::SetMessageTime( LTFLOAT fSeconds )
{
	m_fMessageTime = fSeconds;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::HandleKeyDown()
//
//	PURPOSE:	Processes keystrokes
//
// ----------------------------------------------------------------------- //

LTBOOL CMessageMgr::HandleKeyDown(int key, int rep)
{
	if (m_bEditing)
		return m_InputLine.HandleKeyDown(key, rep);
	else
        return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::HandleChar()
//
//	PURPOSE:	Processes keystrokes
//
// ----------------------------------------------------------------------- //

void CMessageMgr::HandleChar(char c)
{
	if (m_bEditing)
		m_InputLine.HandleChar(c);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::SetEditingState()
//
//	PURPOSE:	enter/leave editing mode
//
// ----------------------------------------------------------------------- //

void CMessageMgr::SetEditingState(LTBOOL bEditing, LTBOOL bTeamMsg)
{
	m_bEditing = bEditing;
	m_bTeamMsg = bTeamMsg;

	// Send the Message to the server
	HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_PLAYER_CHATMODE);
    g_pLTClient->WriteToMessageByte(hMsg, (uint8)bEditing);
    g_pLTClient->EndMessage(hMsg);

}

/*******************************************************************************

	CInputLine

*******************************************************************************/


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::Init (CClientDE* pClientDE)
//
//	PURPOSE:	Initializes the player Message
//
// ----------------------------------------------------------------------- //

LTBOOL CInputLine::Init ()
{
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

void CInputLine::Clear( void )
{
	*m_zText = '\0';	// nil the Message
	m_nTextLen = 0;	// zero the length
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

void CInputLine::Term( void )
{
	Clear();
    g_pMessageMgr->SetEditingState(LTFALSE);
    g_pLTClient->SetInputState(LTTRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

void CInputLine::Draw(HSURFACE hDest, long x, long y)
{
	char zTemp[ kMaxInputLine + 2 + 4];

    HSTRING hStr = LTNULL;
	if (!g_pMessageMgr->IsTeamMsg())
		hStr = g_pLTClient->FormatString (IDS_SAY);
	else
		hStr = g_pLTClient->FormatString (IDS_TEAMSAY);
    SAFE_STRCPY(zTemp, g_pLTClient->GetStringData (hStr));
    g_pLTClient->FreeString (hStr);

	strcat(zTemp, m_zText);

    double fTime = g_pLTClient->GetTime();
	double tmp;

	fTime = modf(fTime, &tmp);
	if (fTime > 0.5f)
		strcat(zTemp,"_");	// add a cursor
	int nWidth = g_pInterfaceResMgr->GetScreenWidth() - (x+20);


	// Draws the string.
	g_pInterfaceResMgr->GetMsgForeFont()->DrawFormat(zTemp,hDest, x+1, y+1, nWidth);
	g_pInterfaceResMgr->GetMsgForeFont()->DrawFormat(zTemp,hDest, x, y, nWidth ,kWhite);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

LTBOOL CInputLine::AddChar( uint8 ch )
{
	if (m_nTextLen < kMaxInputLine)	// space left in the Message
	{
		m_zText[m_nTextLen] = ch;	// add a character
		m_nTextLen++;				// increment the length
		m_zText[m_nTextLen] = '\0';	// terminate the string
        return LTTRUE;               // indicate success
	}
    return LTFALSE;  // indicate failure
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

void CInputLine::DelChar( void )
{
	if (m_nTextLen > 0)	// text in the Message?
	{
		m_nTextLen--;				// back up a character
		m_zText[m_nTextLen] = '\0';	// terminate the string
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

void CInputLine::Set( char *pzText )
{
	strncpy(m_zText, pzText, kMaxInputLine);				// copy the text
	m_zText[kMaxInputLine] = '\0';							// enforce null termination
	m_nTextLen = strlen(pzText);							// enforce max length
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

void CInputLine::Send( void )
{
	// First check and see if it was a cheat that was entered...

	if (g_pCheatMgr->Check(m_zText))
	{
		g_pClientSoundMgr->PlayInterfaceSound("Menu\\Snd\\Cheat.wav");
		Term();
		return;
	}

	// get the user's name
	char strName[128];
	memset (strName, 0, 128);

    uint32 nLocalID = 0;
    g_pLTClient->GetLocalClientID (&nLocalID);

	int nGameMode = 0;
    g_pLTClient->GetGameMode(&nGameMode);
    if (nGameMode == STARTGAME_NORMAL || nGameMode == GAMEMODE_NONE || nLocalID == LT_NOTCONNECTED)
	{
        HSTRING hStr = g_pLTClient->FormatString (IDS_PLAYER);
        strncpy (strName, g_pLTClient->GetStringData (hStr), 127);
        g_pLTClient->FreeString (hStr);
	}
    else
	{
		CClientInfoMgr* pClientInfoMgr = g_pInterfaceMgr->GetClientInfoMgr();
		strncpy (strName, pClientInfoMgr->GetPlayerName (nLocalID), 127);
	}

	// create the message
	char strMessage[kMaxInputLine + 56];
	if (g_pGameClientShell->GetGameType() == SINGLE)
	{
		strncpy (strMessage, m_zText, sizeof(strMessage)-1);
	}
	else
	{
		strncpy (strMessage, strName, sizeof(strMessage)-1);
		strcat (strMessage, ": ");
		strcat (strMessage, m_zText);
	}

	// Send the Message to the server
	HMESSAGEWRITE hMsg;
	if (g_pGameClientShell->GetPlayerState() == PS_GHOST)
		hMsg = g_pLTClient->StartMessage(MID_PLAYER_GHOSTMESSAGE);
	else if (g_pMessageMgr->IsTeamMsg())
        hMsg = g_pLTClient->StartMessage(MID_PLAYER_TEAMMESSAGE);
	else
        hMsg = g_pLTClient->StartMessage(MID_PLAYER_MESSAGE);
    g_pLTClient->WriteToMessageString(hMsg, strMessage);
    g_pLTClient->EndMessage(hMsg);

	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //

LTBOOL CInputLine::HandleKeyDown(int key, int rep)
{
	switch( key )
	{
		case VK_ESCAPE:
		{
			Term();
		}
		break;

		case VK_HOME:
		{
			Clear();
		}
		break;

		case VK_BACK:
		{
			DelChar();
		}
		break;

		case VK_SHIFT:
		{
            m_bShift = LTTRUE;
		}
		break;

		case VK_UP:
		{
			if (m_nCurrentRecentLine == m_nBaseRecentLine + 1 ||
				(m_nCurrentRecentLine == 0 && m_nBaseRecentLine == NUM_RECENT_LINES - 1))
			{
				break;
			}

			int nTest = 0;
			if (m_nCurrentRecentLine == -1)
			{
				nTest = m_nBaseRecentLine;
			}
			else
			{
				nTest = m_nCurrentRecentLine - 1;
			}
			if (nTest < 0) nTest = NUM_RECENT_LINES - 1;

			if (!m_bRecentLineUsed[nTest]) break;
			m_nCurrentRecentLine = nTest;
			SAFE_STRCPY(m_zText, m_pRecentLines[m_nCurrentRecentLine]);
		}
		break;

		case VK_DOWN:
		{
			if (m_nCurrentRecentLine == m_nBaseRecentLine || m_nCurrentRecentLine == -1)
			{
				break;
			}

			m_nCurrentRecentLine++;
			if (m_nCurrentRecentLine == NUM_RECENT_LINES) m_nCurrentRecentLine = 0;
			SAFE_STRCPY(m_zText, m_pRecentLines[m_nCurrentRecentLine]);
		}
		break;

		case VK_RETURN:
		{
			AddToRecentList();
			Send();
            g_pLTClient->ClearInput(); // Don't process the key they hit...
		}
		break;

	}
    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::HandleKeyUp()
//
//	PURPOSE:	Handles a key up Message, used for tracking shift keys
//
// ----------------------------------------------------------------------- //

void CInputLine::HandleKeyUp(int key)
{
	switch( key )
	{
		case VK_SHIFT:
            m_bShift = LTFALSE;
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::HandleChar()
//
//	PURPOSE:	Handles a OnChar Message
//
// ----------------------------------------------------------------------- //

void CInputLine::HandleChar(char c)
{
	AddChar(c);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::AddToRecentList()
//
//	PURPOSE:	Adds the current string to the recent-string list
//
// ----------------------------------------------------------------------- //

void CInputLine::AddToRecentList()
{
	// add the current line to the array of past input lines

	m_nBaseRecentLine++;
	if (m_nBaseRecentLine >= NUM_RECENT_LINES)
		m_nBaseRecentLine = 0;
	else if(m_nBaseRecentLine < 0)
		m_nBaseRecentLine = 0;

	SAFE_STRCPY(m_pRecentLines[m_nBaseRecentLine], m_zText);
    m_bRecentLineUsed[m_nBaseRecentLine] = LTTRUE;
	m_nCurrentRecentLine = -1;
}





/*******************************************************************************

	CCheatMgr

*******************************************************************************/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Init()
//
//	PURPOSE:	Initializes the cheat manager
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Init()
{
	g_pCheatMgr = this;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Check()
//
//	PURPOSE:	See if a string is a cheat code
//
// ----------------------------------------------------------------------- //

LTBOOL CCheatMgr::Check( char *pzText )
{
	char buf[100];


#ifdef _DEMO
    return LTFALSE;      // Don't do cheats in the demo...
#endif

	// copy their text
	strncpy(buf, pzText, sizeof(buf)-1);

	// It should start with "MP"
	if ( strncmp(buf, "mp", 2) != 0 )
        return LTFALSE;

	// convert it to cheat compatible text
    unsigned int i;
    for ( i = 0; i < strlen(pzText); i++ )
		buf[i] = ((buf[i] ^ 38) + i) ^ 7;

	// then compare the converted text
	for ( i = 0; i < CHEAT_MAX; i++ )
	{
		if ( strcmp( buf, s_CheatInfo[i].pzText ) == 0)
		{
			Process( (CheatCode)i );
            return LTTRUE;
		}
	}
    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Process()
//
//	PURPOSE:	Calls the appropriate cheat function
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Process( CheatCode nCheatCode )
{
	if ( nCheatCode <= CHEAT_NONE || nCheatCode >= CHEAT_MAX ) return;

	// Don't do cheats in multiplayer...
    if (g_pGameClientShell->IsMultiplayerGame())
	{
		// Well, okay, let them toggle between 1st and 3rd person ;)
		// and, well, blood is pretty cool...
		if (nCheatCode == CHEAT_CHASETOGGLE)
		{
			ChaseToggle();
		}
		else if (nCheatCode == CHEAT_BIGBLOOD)
		{
			BigBlood(!s_CheatInfo[nCheatCode].bActive);
		}

		return;
	}

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ProcessCheat(nCheatCode);
	}

	// process cheat codes
	switch ( nCheatCode )
	{
		case CHEAT_GOD:			// god mode toggle
			SetGodMode(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_AMMO:		// full ammo
			SetAmmo();
			break;

		case CHEAT_ARMOR:		// full armor
			SetArmor();
		break;

		case CHEAT_HEALTH:		// full health
			SetHealth();
		break;

		case CHEAT_KFA:			// give em everything
			SetKFA();
		break;

		case CHEAT_EXITLEVEL:	// exit the current level
			SetExitLevel();
		break;

		case CHEAT_FULL_WEAPONS:   // give all weapons
			SetFullWeapons();
		break;

		case CHEAT_MOTORCYCLE:	  // spawn in motorcycle
			Motorcycle(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_SNOWMOBILE:	  // spawn in snowmobile
			Snowmobile(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_MODSQUAD:	// give all mods for current weapons
			ModSquad();
		break;

		case CHEAT_FULL_GEAR:	// give all gear
			FullGear();
		break;

		case CHEAT_CHASETOGGLE:	   // toggle 3rd person view
			ChaseToggle();
		break;

		case CHEAT_TEARS:	      // toggle tears cheat
			Tears(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_VERSION:	      // display version info
			Version();
		break;

		case CHEAT_MIKED:	      // we love you!
			MikeD();
		break;

		case CHEAT_BIGBLOOD:	  // lots o blood
			BigBlood(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_POS:				// show/hide player position
			SetPos(!s_CheatInfo[nCheatCode].bActive);
		break;

#ifndef _FINAL
		case CHEAT_CLIP:		// toggle clipping mode
			SetClipMode(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_TELEPORT:	// teleport to beginning
			Teleport();
		break;

		case CHEAT_CAM_POSROT:    // show/hide camera position/rotation
			SetCamPosRot(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_MISSION_INVENTORY:  // give em mission default inventory
			SetMissionInventory();
		break;

		case CHEAT_POSWEAPON:		    // toggle adjust of weapon pos
			PosWeapon(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_POSWEAPON_MUZZLE:	// toggle adjust of weapon muzzle pos
			PosWeaponMuzzle(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_CAMERAOFFSET:	   // toggle adjust of camera offset
			Pos1stCam(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_LIGHTSCALE:	      // toggle client light scale offset
			LightScale(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_LIGHTADD:	      // toggle client light add offset
			LightAdd(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_FOV:				// toggle fov cheat
			FOV(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_INTERFACEADJUST:     // toggle menu polygrid cheat
			InterfaceAdjust(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_REMOVEAI:	  // remove all ai
			RemoveAI(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_TRIGGERBOX:	  // toggle trigger boxes on/off
			TriggerBox(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_POSBREACH:	  // toggle hand-held weapon breach adjust on/off
			Breach(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_POS1STCAM:	  // toggle 1st person camera adjust on/off
			Pos1stCam(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_ALL_MISSIONS:   // allow all missions
			AllowAllMissions(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_RESET_HISTORY:   // clear player history
			ResetHistory();
		break;
#endif  // _FINAL

		default:
			return;				// skip setting global cheat indicator for unhandled cheats
	}

    m_bPlayerCheated = LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SendCheatMessage()
//
//	PURPOSE:	sends a cheat to the server
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SendCheatMessage( CheatCode nCheatCode, uint8 nData )
{
	// Send the Message to the server
    HMESSAGEWRITE hMsg = g_pLTClient->StartMessage(MID_PLAYER_CHEAT);
    g_pLTClient->WriteToMessageByte(hMsg, (uint8)nCheatCode);
    g_pLTClient->WriteToMessageByte(hMsg, nData);
    g_pLTClient->EndMessage(hMsg);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetGodMode()
//
//	PURPOSE:	Sets/resets God mode
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetGodMode(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_GOD].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_GOD, bMode);

	if (bMode)
	{
		g_pMessageMgr->AddLine("God Mode: ON");
	}
	else
	{
 		g_pMessageMgr->AddLine("God Mode: OFF");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetAmmo()
//
//	PURPOSE:	Gives full ammo
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetAmmo()
{
    s_CheatInfo[CHEAT_AMMO].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_AMMO, LTTRUE);

	g_pMessageMgr->AddLine("You can never have too many bullets...");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetArmor()
//
//	PURPOSE:	Gives full armor
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetArmor()
{
    s_CheatInfo[CHEAT_ARMOR].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_ARMOR, LTTRUE);

	g_pMessageMgr->AddLine("We got def star, we got def star!");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetHealth()
//
//	PURPOSE:	Gives full health
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetHealth(LTBOOL bPlaySound)
{
    s_CheatInfo[CHEAT_HEALTH].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_HEALTH, LTTRUE);

	g_pMessageMgr->AddLine("Doctor Dentz!");

	if (bPlaySound)
	{
		g_pClientSoundMgr->PlayInterfaceSound("voice\\1025.wav");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetClipMode()
//
//	PURPOSE:	Sets/resets Clip mode
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetClipMode(LTBOOL bMode)
{
	// Can't do this in multiplayer.
	if(g_pGameClientShell && g_pGameClientShell->IsMultiplayerGame())
		return;

	s_CheatInfo[CHEAT_CLIP].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->SetSpectatorMode(bMode);
	}

	// Tell the server
	SendCheatMessage(CHEAT_CLIP, bMode);

	if (bMode)
	{
        g_pMessageMgr->AddLine("Spectator mode enabled");
	}
	else
	{
        g_pMessageMgr->AddLine("Spectator mode disabled");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Teleport()
//
//	PURPOSE:	Teleports to beginning of level
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Teleport()
{
    SendCheatMessage(CHEAT_TELEPORT, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetFullWeapons()
//
//	PURPOSE:	Give us all the weapons
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetFullWeapons()
{
    s_CheatInfo[CHEAT_FULL_WEAPONS].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_FULL_WEAPONS, LTTRUE);

    g_pMessageMgr->AddLine("Lots of guns...");
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetKFA()
//
//	PURPOSE:	Give us all weapons, ammo, armor, and health...
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetKFA()
{
    s_CheatInfo[CHEAT_KFA].bActive = LTTRUE;

	// Give us all weapons, ammo, armor, and health...
	SetFullWeapons(); // Gives us all ammo too
	SetHealth(LTFALSE);
	SetArmor();

    g_pMessageMgr->AddLine("Knock em out the box Luke...");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetExitLevel()
//
//	PURPOSE:	Exit the current level
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetExitLevel()
{
    s_CheatInfo[CHEAT_EXITLEVEL].bActive = LTTRUE;

	g_pGameClientShell->HandleExitLevel(0, LTNULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::ModSquad()
//
//	PURPOSE:	Give us all the mods
//
// ----------------------------------------------------------------------- //

void CCheatMgr::ModSquad()
{
    s_CheatInfo[CHEAT_MODSQUAD].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_MODSQUAD, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::FullGear()
//
//	PURPOSE:	Give us all gear
//
// ----------------------------------------------------------------------- //

void CCheatMgr::FullGear()
{
    s_CheatInfo[CHEAT_FULL_GEAR].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_FULL_GEAR, LTTRUE);

    g_pMessageMgr->AddLine("Gear! (as in 'Boy the Beatles sure are Gear!')");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetPos()
//
//	PURPOSE:	Toggle displaying of position on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetPos(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_POS].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ShowPlayerPos(bMode);
	}

	if (bMode)
	{
        g_pMessageMgr->AddLine("Show position enabled.");
	}
	else
	{
        g_pMessageMgr->AddLine("Show position disabled.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetCamPosRot()
//
//	PURPOSE:	Toggle displaying of camera pos/rot on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetCamPosRot(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_CAM_POSROT].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ShowCamPosRot(bMode);
	}

	if (bMode)
	{
        g_pMessageMgr->AddLine("Show Camera Pos/Rot enabled.");
	}
	else
	{
        g_pMessageMgr->AddLine("Show Camera Pos/Rot disabled.");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetMissionInventory()
//
//	PURPOSE:	Give the player the default inventory for the current
//				mission
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetMissionInventory()
{
	s_CheatInfo[CHEAT_MISSION_INVENTORY].bActive = LTTRUE;

	if (g_pInterfaceMgr)
	{
		g_pInterfaceMgr->DoMissionOutfitCheat();
	}

    g_pMessageMgr->AddLine("You're good to go...");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::PosWeapon()
//
//	PURPOSE:	Toggle positioning of player view weapon on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::PosWeapon(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_POSWEAPON].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_POSWEAPON, bMode);

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_POSWEAPON);
	}

	if (bMode)
	{
        g_pMessageMgr->AddLine("Adjust player-view weapon position: ON");
	}
	else
	{
        g_pMessageMgr->AddLine("Adjust player-view weapon position: OFF");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::PosWeaponMuzzle()
//
//	PURPOSE:	Toggle positioning of player view weapon muzzle on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::PosWeaponMuzzle(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_POSWEAPON_MUZZLE].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_POSWEAPON_MUZZLE, bMode);

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_POSWEAPON_MUZZLE);
	}

	if (bMode)
	{
        g_pMessageMgr->AddLine("Adjust player-view weapon muzzle position: ON");
	}
	else
	{
        g_pMessageMgr->AddLine("Adjust player-view weapon muzzle position: OFF");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::LightScale()
//
//	PURPOSE:	Toggle adjustment of light scale offset on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::LightScale(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_LIGHTSCALE].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_LIGHTSCALE);
	}

	if (bMode)
	{
        g_pMessageMgr->AddLine("Adjust Light Scale: ON");
	}
	else
	{
        g_pMessageMgr->AddLine("Adjust Light Scale: OFF");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::LightAdd()
//
//	PURPOSE:	Toggle adjustment of light add offset on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::LightAdd(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_LIGHTADD].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_LIGHTADD);
	}

	if (bMode)
	{
        g_pMessageMgr->AddLine("Adjust Light Add: ON");
	}
	else
	{
        g_pMessageMgr->AddLine("Adjust Light Add: OFF");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::FOV()
//
//	PURPOSE:	Toggle adjustment of FOV on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::FOV(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_FOV].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_FOV);
	}

	if (bMode)
	{
        g_pMessageMgr->AddLine("Adjust FOV: ON");
	}
	else
	{
        g_pMessageMgr->AddLine("Adjust FOV: OFF");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::InterfaceAdjust()
//
//	PURPOSE:	Toggle adjustment of the interface on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::InterfaceAdjust(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_INTERFACEADJUST].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_INTERFACEADJUST);
	}

	if (bMode)
	{
        g_pMessageMgr->AddLine("Adjust Interface: ON");
	}
	else
	{
        g_pMessageMgr->AddLine("Adjust Interface: OFF");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Tears()
//
//	PURPOSE:	Toggle tears cheat on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Tears(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_TEARS].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_TEARS, bMode);

	if (bMode)
	{
        g_bInfiniteAmmo = LTTRUE;
        g_pMessageMgr->AddLine("RAWWR!!!");
	}
	else
	{
        g_bInfiniteAmmo = LTFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::RemoveAI()
//
//	PURPOSE:	Remove all AI in the level
//
// ----------------------------------------------------------------------- //

void CCheatMgr::RemoveAI(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_REMOVEAI].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_REMOVEAI, bMode);

	if (bMode)
	{
        g_pMessageMgr->AddLine("Cheaters never prosper...");
	}
	else
	{
        g_pMessageMgr->AddLine("Sheeze, you really ARE pathetic...");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::TriggerBox()
//
//	PURPOSE:	Toggle trigger boxes on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::TriggerBox(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_TRIGGERBOX].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_TRIGGERBOX, bMode);

	if (bMode)
	{
        g_pMessageMgr->AddLine("Ah shucks, that takes all the fun out of it...");
	}
	else
	{
        g_pMessageMgr->AddLine("That's better sport!");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Breach()
//
//	PURPOSE:	Toggle adjusting hand-held weapon breach on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Breach(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_POSBREACH].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_POSBREACH, bMode);

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_POSBREACH);
	}

	if (bMode)
	{
        g_pMessageMgr->AddLine("Adjust hand-held weapon breach offset: ON");
	}
	else
	{
        g_pMessageMgr->AddLine("Adjust hand-held weapon breach offset: OFF");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Pos1stCam()
//
//	PURPOSE:	Toggle adjusting 1st person camera on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Pos1stCam(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_POS1STCAM].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_POS1STCAM, bMode);

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_POS1STCAM);
	}

	if (bMode)
	{
        g_pMessageMgr->AddLine("Adjust 1st person camera offset: ON");
	}
	else
	{
        g_pMessageMgr->AddLine("Adjust 1st person camera offset: OFF");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Motorcycle()
//
//	PURPOSE:	Spawn in a motorcycle
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Motorcycle(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_MOTORCYCLE].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_MOTORCYCLE, bMode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Snowmobile()
//
//	PURPOSE:	Spawn in a snowmobile
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Snowmobile(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_SNOWMOBILE].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_SNOWMOBILE, bMode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::ChaseToggle()
//
//	PURPOSE:	Toggle 3rd person camera on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::ChaseToggle()
{
	// Tell the server

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_CHASETOGGLE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Version()
//
//	PURPOSE:	Display version info
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Version()
{
	// Display the version info...

	g_pMessageMgr->AddLine((char*)g_pVersionMgr->GetBuild());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::MikeD()
//
//	PURPOSE:	To show our love
//
// ----------------------------------------------------------------------- //

void CCheatMgr::MikeD()
{
     HSTRING hString = g_pLTClient->CreateString("Would you like EXORBITANT amounts of money?");
	 g_pInterfaceMgr->ShowMessageBox(hString, LTMB_YESNO, MikeDCallBack, LTNULL);
	 g_pLTClient->FreeString(hString);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::BigBlood()
//
//	PURPOSE:	Buckets O blood
//
// ----------------------------------------------------------------------- //

void CCheatMgr::BigBlood(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_BIGBLOOD].bActive = bMode;

	if (bMode)
	{
        WriteConsoleInt("BigBlood", 1);
        g_pMessageMgr->AddLine("Everybody down, hard rain is falling!");
	}
	else
	{
        WriteConsoleInt("BigBlood", 0);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::AllowAllMissions()
//
//	PURPOSE:	Allow player to select any mission
//
// ----------------------------------------------------------------------- //

void CCheatMgr::AllowAllMissions(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_ALL_MISSIONS].bActive = bMode;

	if (bMode)
	{
        g_pMessageMgr->AddLine("Been there, done that.");
	}
	else
	{
        g_pMessageMgr->AddLine("Back in Kansas again.");
	}

	g_bAllowAllMissions = bMode;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::ResetHistory()
//
//	PURPOSE:	Reset player history
//
// ----------------------------------------------------------------------- //

void CCheatMgr::ResetHistory()
{
    g_pMessageMgr->AddLine("This is not my beautiful wife.");
	g_pGameClientShell->GetPlayerSummary()->ClearStatus();
}