//----------------------------------------------------------
//
// MODULE  : MessageMgr.h
//
// PURPOSE : Blood 2 Messaging system
//
// CREATED : 10/22/97
//
//----------------------------------------------------------

#include "MessageMgr.h"
#include "VKDefs.h"
#include "SharedDefs.h"
#include "stdio.h"
#include "BloodClientShell.h"

CMessageMgr *g_pMessageMgr = DNULL;
CCheatMgr	*g_pCheatMgr = DNULL;


#define ENCRYPT_CHEATS	1	// SET THIS TO NON-ZERO BEFORE WE SHIP!!!

#if ENCRYPT_CHEATS

CCheatMgr::CheatInfo CCheatMgr::s_CheatInfo[] = {	// encrypted codes
	{ "T\\Q",			DFALSE },	// god mode toggle
	{ "XSN",			DFALSE },	// Monolith Productions kicks fucking ass!
	{ "NZZ\\",			DFALSE },	// full ammo
	{ "PYV]",			DFALSE },	// Clipping
	{ "URNYaUf",		DFALSE },	// Full health
	{ "dUR_RNZV",		DFALSE },	// Display current position
	{ "UVQRZR",			DFALSE },	// Stealth
	{ "ORRSPNXR",		DFALSE },	// Triple Damage
	{ "XVYYRZNYY",		DFALSE },	// Kill all ais
	{ "`]RRQb]",		DFALSE },	// Increase speed attribute
	{ "`a_\\[TR_",		DFALSE },	// Increase strength attribute
	{ "PNYRO",			DFALSE },
	{ "\\]URYVN",		DFALSE },
	{ "V`UZNRY",		DFALSE },
	{ "TNOOf",			DFALSE },
	{ "OR_RaaN",		DFALSE },	// Weapon pickup cheats
	{ "`bOZNPUV[RTb[",	DFALSE },
	{ "SYN_RTb[",		DFALSE },
	{ "`U\\aTb[",		DFALSE },
	{ "`[V]R__VSYR",	DFALSE },
	{ "U\\dVagR_",		DFALSE },
	{ "[N]NYZPN[[\\[",	DFALSE },
	{ "`V[TbYN_Vaf",	DFALSE },
	{ "N``NbYa_VSYR",	DFALSE },
	{ "ObTOb`aR_",		DFALSE },
	{ "ZV[VTb[",		DFALSE },
	{ "YN`R__VSYR",		DFALSE },
	{ "aR`YNPN[[\\[",	DFALSE },
	{ "c\\\\Q\\\\",		DFALSE },
	{ "aUR\\_O",		DFALSE },
	{ "YVSRYRRPU",		DFALSE },
	{ "PU\\`R[X[VSR",	DFALSE },		// ChosenKnife (Added by Andy 2/3/99)
#ifdef _ADD_ON
	{ "P\\ZONa`U\\aTb[",DFALSE },		// CombatShotgun (Added by Andy 2/3/99)
	{ "SYNfR_",			DFALSE },		// Flayer (Added by Andy 2/3/99)
#endif
	{ "T\\OYR",			DFALSE },
	{ "`P\\_]V\\",		DFALSE },
	{ "a\\aN_\\",		DFALSE },
	{ "`b]R_gbT",		DFALSE },		// demo level 2
	{ "T\\`U\\]]V[T",	DFALSE },		// give all inventory items
	{ "[VPR[b_`R",		DFALSE },		// health powerup
	{ "_RNYYf[VPR[b_`R",DFALSE },		// mega health powerup
	{ "dN_Q",			DFALSE },		// ward powerup
	{ "[RdP_\\dN_Q",	DFALSE },		// necroward powerup
	{ "PN_O\\[SVOR_",	DFALSE },		// invulnerability
	{ "aNXR\\SS`U\\R`",	DFALSE },		// stealth powerup
	{ "UR_XR_Zb_",		DFALSE },		// anger powerup
	{ "ORN[`\\SP\\\\Y[R``",DFALSE },	// god mode toggle 2 (different weapons)
	{ "S\\YY\\dZR",		DFALSE }		// Chase view
};

#else

CCheatMgr::CheatInfo CCheatMgr::s_CheatInfo[] = {		// decrypted codes
	{ "GOD",			DFALSE },	// god mode toggle
	{ "KFA",			DFALSE },	// Monolith Productions kicks fucking ass!
	{ "AMMO",			DFALSE },	// full ammo
	{ "CLIP",			DFALSE },	// Clipping
	{ "HEALTHY",		DFALSE },	// Full health
	{ "WHEREAMI",		DFALSE },	// Display current position
	{ "HIDEME",			DFALSE },	// Invisibility
	{ "BEEFCAKE",		DFALSE },	// Triple Damage
	{ "KILLEMALL",		DFALSE },	// Kill all ais
	{ "SPEEDUP",		DFALSE },	// Increase speed attribute
	{ "STRONGER",		DFALSE },	// Increase strength attribute
	{ "CALEB",			DFALSE },
	{ "OPHELIA",		DFALSE },
	{ "ISHMAEL",		DFALSE },
	{ "GABBY",			DFALSE },
	{ "BERETTA",		DFALSE },	// Weapon pickup cheats
	{ "SUBMACHINEGUN",	DFALSE },
	{ "FLAREGUN",		DFALSE },
	{ "SHOTGUN",		DFALSE },
	{ "SNIPERRIFLE",	DFALSE },
	{ "HOWITZER",		DFALSE },
	{ "NAPALMCANNON",	DFALSE },
	{ "SINGULARITY",	DFALSE },
	{ "ASSAULTRIFLE",	DFALSE },
	{ "BUGBUSTER",		DFALSE },
	{ "MINIGUN",		DFALSE },
	{ "LASERRIFLE",		DFALSE },
	{ "TESLACANNON",	DFALSE },
	{ "VOODOO",			DFALSE },
	{ "THEORB",			DFALSE },
	{ "LIFELEECH",		DFALSE },
	{ "CHOSENKNIFE",	DFALSE },	// ChosenKnife (Added by Andy 2/3/99)
#ifdef _ADD_ON
	{ "COMBATSHOTGUN",	DFALSE },	// CombatShotgun (Added by Andy 2/3/99)
	{ "FLAYER",			DFALSE },	// Flayer (Added by Andy 2/3/99)
#endif
	{ "GOBLE",			DFALSE },
	{ "SCORPIO",		DFALSE },
	{ "TOTARO",			DFALSE },
	{ "SUPERZUG",		DFALSE },	// demo level 2
	{ "GOSHOPPING",		DFALSE },	// give all inventory items
	{ "NICENURSE",		DFALSE },	// health powerup
	{ "HOTNURSE",		DFALSE },	// mega health powerup
	{ "WARD",			DFALSE },	// ward powerup
	{ "NEWCROWARD",		DFALSE },	// necroward powerup
	{ "CARBONFIBER",	DFALSE },	// invulnerability
	{ "TAKEOFFSHOES",	DFALSE },	// stealth powerup
	{ "HERKERMUR",		DFALSE },	// anger powerup
	{ "BEANSOFCOOLNESS",DFALSE },	// god mode toggle 2 (different weapons)
	{ "FOLLOWME",		DFALSE }	// Chase view
};

#endif


// Externs...

extern	DBOOL	g_bLevelChange3rdPersonCam;


// Statics...

DBOOL CCheatMgr::m_bPlayerCheated = DFALSE;


// Prototypes...

DBOOL IsValidDemoCheat(int nCheat);


// Functions...

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
	m_x	= 1;
	m_y	= 0;

	m_nMaxMessages	= kDefaultMaxMessages;
	m_fMessageTime	= kDefaultMessageTime;
	m_dwMessageFlags = MESSAGEFLAGS_MASK;

	m_nMessageCount	= 0;
	m_nFirstMessage	= 0;
	m_nNextMessage	= m_nFirstMessage;
	m_bEditing = DFALSE;

	m_hFont = DNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Init (CClientDE* pClientDE)
//
//	PURPOSE:	Initializes the Message manager
//
// ----------------------------------------------------------------------- //

DBOOL CMessageMgr::Init (CClientDE* pClientDE)
{
	if (!pClientDE) return DFALSE;

	m_pClientDE = pClientDE;
	m_InputLine.Init(pClientDE);

	m_hFont = m_pClientDE->CreateFont("Arial", 6, 16, DFALSE, DFALSE, DFALSE);
	m_nFontHeight = 16;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Enable()
//
//	PURPOSE:	Sets the enabled flag
//
// ----------------------------------------------------------------------- //
			
void CMessageMgr::Enable( DBOOL bEnabled )
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
			
void CMessageMgr::AddLine( char *szMsg, DBYTE uMessageFlag )
{
	if (szMsg)
	{
		HSTRING hString = m_pClientDE->CreateString(szMsg);
		AddLine(hString, uMessageFlag);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::AddLine()
//
//	PURPOSE:	Adds a new line to the Message buffer
//
// ----------------------------------------------------------------------- //
			
void CMessageMgr::AddLine( HSTRING hMsg, DBYTE uMessageFlag )
{
	// don't bother adding Messages disallowed by the user
	if ( !(uMessageFlag && m_dwMessageFlags) )
		return;

	HDECOLOR hForeground = m_pClientDE->SetupColor1 (1.0f, 1.0f, 1.0f, DFALSE);

	Message *pMsg = &m_Messages[ m_nNextMessage ];

	// Set next Message pointer
	m_nNextMessage = (m_nNextMessage + 1) % kMaxMessages;

	// don't overflow the maximum number of Messages
	if ( ++m_nMessageCount > m_nMaxMessages )
	{
		// expire the oldest Message and recalculate the first Message in the buffer
		Message *pOld = &m_Messages[ m_nFirstMessage ];

		DeleteMessageData(pOld);

		m_nFirstMessage = (m_nFirstMessage + 1) % kMaxMessages;

		// reset the Message count to max
		m_nMessageCount = m_nMaxMessages;
	}

	pMsg->hSurface = m_pClientDE->CreateSurfaceFromString (m_hFont, hMsg, hForeground, NULL, 10, 0);
	if (!pMsg->hSurface)  // Error
	{
		return;
	}

	pMsg->hMessage = hMsg;
	pMsg->fExpiration = m_pClientDE->GetTime() + m_fMessageTime;
	
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
	if (!m_bEnabled)
		return;

	// Try to draw a cheat message
	DBOOL bDrawCheatMsg = DrawCheatMsg();

	// early return if there are no Messages in the queue,
	// or Messages are disabled, or while typing Messages
	if ( !m_bEditing && (m_nMessageCount == 0))
		return;

	int nCount = m_nMessageCount;	// use temporary values
	int nFirst = m_nFirstMessage;	// because the real ones may change
	int i;
	int y = m_y;

	if (bDrawCheatMsg) y += m_nFontHeight;

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();

	for ( i = 0; i < nCount; i++)
	{
		Message *pMsg = &m_Messages[ (nFirst + i) % kMaxMessages ];

		// expire Messages that have overstayed their welcome
		if ( m_pClientDE->GetTime() >= pMsg->fExpiration )
		{
			DeleteMessageData(pMsg);
			// expire the oldest Message and recalculate the Message position and counter
			m_nFirstMessage = (m_nFirstMessage + 1) % kMaxMessages;
			m_nMessageCount--;

			// don't draw the expired Message
			continue;
		}

		// draw the text
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, pMsg->hSurface, NULL, m_x, y, NULL);
		y += m_nFontHeight;
	}

	if (m_bEditing)
	{
		HDECOLOR hForeground = m_pClientDE->SetupColor1 (1.0f, 1.0f, 1.0f, DFALSE);
		m_InputLine.Draw(hScreen, m_hFont, hForeground, NULL, m_x, y);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::Draw()
//
//	PURPOSE:	Draws a cheat message if enabled
//
// ----------------------------------------------------------------------- //
			
DBOOL CMessageMgr::DrawCheatMsg( void )
{
	DBOOL bRetVal = DFALSE;
	char szCheatString[200];

	_mbscpy((unsigned char*)szCheatString, (const unsigned char*)"");

	if (!m_bEnabled || !g_pCheatMgr || !m_pClientDE)
		return bRetVal;

	if (g_pCheatMgr->s_CheatInfo[CHEAT_WHEREAMI].bActive)
	{
		DVector vPos;
		VEC_INIT(vPos);

		HLOCALOBJ hClientObj = m_pClientDE->GetClientObject();
		if (hClientObj)
			m_pClientDE->GetObjectPos(hClientObj, &vPos);

		sprintf(szCheatString, "MY POS: X: %f, Y: %f, Z: %f", vPos.x, vPos.y, vPos.z);
	}

	// Draw the message
	if (_mbstrlen(szCheatString))
	{
		HDECOLOR hForeground = m_pClientDE->SetupColor1 (1.0f, 0.0f, 0.0f, DFALSE);
		HSTRING hString = m_pClientDE->CreateString(szCheatString);
		HSURFACE hSurface = m_pClientDE->CreateSurfaceFromString (m_hFont, hString, hForeground, NULL, 10, 0);

		m_pClientDE->DrawSurfaceToSurfaceTransparent (m_pClientDE->GetScreenSurface(), hSurface, NULL, m_x, m_y, NULL);

		m_pClientDE->DeleteSurface(hSurface);
		m_pClientDE->FreeString(hString);
		bRetVal = DTRUE;
	}

	return bRetVal;
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

void CMessageMgr::SetMessageTime( DFLOAT fSeconds )
{
	m_fMessageTime = fSeconds;
}

void CMessageMgr::SetMessageFlags( DDWORD dwFlags )
{
	m_dwMessageFlags = dwFlags & MESSAGEFLAGS_MASK;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMessageMgr::HandleKeyDown()
//
//	PURPOSE:	Processes keystrokes
//
// ----------------------------------------------------------------------- //
			
DBOOL CMessageMgr::HandleKeyDown(int key, int rep)
{
	if (m_bEditing)
		return m_InputLine.HandleKeyDown(key, rep);
	else
		return DFALSE;
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

void CInputLine::Init (CClientDE* pClientDE)
{
	m_pClientDE = pClientDE;
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
	g_pMessageMgr->SetEditingState(DFALSE);

	if (!g_bLevelChange3rdPersonCam)
	{
		m_pClientDE->SetInputState(DTRUE);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //
			
void CInputLine::Draw(HSURFACE hDest, HDEFONT hFont, HDECOLOR hForeColor, HDECOLOR hBackColor, long x, long y)
{
	char zTemp[ MAXINPUTLINE + 2 ];
	_mbscpy((unsigned char*)zTemp, (const unsigned char*)"SAY:");
	_mbscat((unsigned char*)zTemp, (const unsigned char*)m_zText);

	double fTime = m_pClientDE->GetTime();
	double tmp;

	fTime = modf(fTime, &tmp);
	if (fTime > 0.5f)
		_mbscat((unsigned char*)zTemp, (const unsigned char*)"_");	// add a cursor

	HSTRING hString = m_pClientDE->CreateString(zTemp);

	// Draws the string.
	HSURFACE hSurface = m_pClientDE->CreateSurfaceFromString(hFont, hString, hForeColor, hBackColor, 0, 0);
	m_pClientDE->DrawSurfaceToSurfaceTransparent(hDest, hSurface, NULL, x, y, hBackColor);
	m_pClientDE->DeleteSurface(hSurface);
	m_pClientDE->FreeString(hString);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //
			
DBOOL CInputLine::AddChar( DBYTE ch )
{
	if (m_nTextLen < MAXINPUTLINE)	// space left in the Message
	{
		m_zText[m_nTextLen] = ch;	// add a character
		m_nTextLen++;				// increment the length
		m_zText[m_nTextLen] = '\0';	// terminate the string
		return DTRUE;				// indicate success
	}
	return DFALSE;	// indicate failure
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
	_mbsncpy((unsigned char*)m_zText, (const unsigned char*)pzText, MAXINPUTLINE);					// copy the text
	m_zText[MAXINPUTLINE] = '\0';							// enforce null termination
	m_nTextLen = _mbstrlen(pzText);							// enforce max length
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
	// Send the Message to the server
	HMESSAGEWRITE hMsg = m_pClientDE->StartMessage(CMSG_PLAYERMESSAGE);
	m_pClientDE->WriteToMessageString(hMsg, m_zText);
	m_pClientDE->EndMessage(hMsg);

	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::CInputLine()
//
//	PURPOSE:	constructor
//
// ----------------------------------------------------------------------- //
			
DBOOL CInputLine::HandleKeyDown(int key, int rep)
{
	switch( key )
	{
		case VK_ESCAPE:
			Term();
			break;

		case VK_HOME:
			Clear();
			break;

		case VK_BACK:
			DelChar();
			break;

		case VK_SHIFT:
			m_bShift = DTRUE;
			break;

		case VK_RETURN:
				if ( g_pCheatMgr->Check(m_zText) )
					Term();
				else
					Send();
			break;

		default:
		{
			char ch;
			if (ch = AsciiXlate(key))
			{
				if (!AddChar(ch) )
				{
				}
			}
			break;
		}
	}
	return DTRUE;
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
			m_bShift = DFALSE;
			break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CInputLine::AsciiXlate()
//
//	PURPOSE:	Translates a VK_ code into something viewable.
//
// ----------------------------------------------------------------------- //
			
char CInputLine::AsciiXlate(int key)
{
	char ch = 0;
	char *zNumShift = ")!@#$%^&*(";

	// Check for a letter
	if (key >= VK_A && key <= VK_Z)
	{
		ch = m_bShift ? key : key - 'A' + 'a';
	}
	// Check for a number
	else if (key >= VK_0 && key <= VK_9)
	{
		ch = m_bShift ? zNumShift[key - VK_0] : key;
	}
	// Now check for the remaining usable keys
	else
	{
		switch(key)
		{
			case VK_SPACE: ch = ' '; break;
			case 189: ch = m_bShift ? '_' : '-'; break;
			case 187: ch = m_bShift ? '+' : '='; break;
			case 219: ch = m_bShift ? '{' : '['; break;
			case 221: ch = m_bShift ? '}' : ']'; break;
			case 220: ch = m_bShift ? '|' : '\\'; break;
			case 186: ch = m_bShift ? ':' : ';'; break;
			case 222: ch = m_bShift ? '"' : '\''; break;
			case 188: ch = m_bShift ? '<' : ','; break;
			case 190: ch = m_bShift ? '>' : '.'; break;
			case 191: ch = m_bShift ? '?' : '/'; break;
		}
	}
	return ch;

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
			
void CCheatMgr::Init(CClientDE* pClientDE)
{
	m_pClientDE = pClientDE;
	g_pCheatMgr = this;

#if ENCRYPT_CHEATS
	Decrypt();
#endif

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Reset()
//
//	PURPOSE:	Resets all cheats to the off position
//
// ----------------------------------------------------------------------- //
			
void CCheatMgr::Reset()
{
	for (int i = 0; i < CHEAT_MAX; i++)
	{
		s_CheatInfo[i].bActive = DFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Decrypt()
//
//	PURPOSE:	Decrypts the cheat codes
//
// ----------------------------------------------------------------------- //
			
void CCheatMgr::Decrypt()
{
	// Decrypt each cheat code...

	for (int i = 0; i < CHEAT_MAX; i++)
	{
		char* sCheat = s_CheatInfo[i].pzText;

		while (*sCheat != '\0' && *sCheat != NULL)
		{
			*sCheat -= 13;
			sCheat++;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Check()
//
//	PURPOSE:	See if a string is a cheat code
//
// ----------------------------------------------------------------------- //
			
DBOOL CCheatMgr::Check( char *pzText )
{
	char buf[MAXINPUTLINE+2];
	unsigned i;

	// copy their text
	_mbscpy((unsigned char*)buf, (const unsigned char*)pzText);
	strupr(buf);

	// It should start with "MP"
	if ( _mbsncmp((const unsigned char*)buf, (const unsigned char*)"MP", 2) != 0 )
		return DFALSE;

	// convert it to cheat compatible text
//	for ( i = 0; i < _mbstrlen(pzText); i++ )
//		buf[i]++;

	// then compare the converted text (skip the first two chars, already know they are "MP"
	for ( i = 0; i < CHEAT_MAX; i++ )
	{
		if ( _mbscmp((const unsigned char*) buf+2, (const unsigned char*)(s_CheatInfo[i].pzText) ) == 0)
		{
			Process( (CheatCodes)i );
			return DTRUE;
		}
	}
	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Process()
//
//	PURPOSE:	Calls the appropriate cheat function
//
// ----------------------------------------------------------------------- //
			
void CCheatMgr::Process( CheatCodes nCheatCode )
{
	// No cheats in multiplayer
	if (g_pBloodClientShell && g_pBloodClientShell->IsMultiplayerGame())
		return;

	if ( nCheatCode < CHEAT_NONE || nCheatCode >= CHEAT_MAX )
		return;

#ifdef _DEMO
	if (!IsValidDemoCheat(nCheatCode))
	{
		return;
	}
#endif

	// Weapon cheat
	if ( nCheatCode >= CHEAT_FIRSTWEAPON && nCheatCode <= CHEAT_LASTWEAPON)
	{
		s_CheatInfo[nCheatCode].bActive = DTRUE;
		// Tell the server
		SendCheatMessage(nCheatCode, DTRUE);
		return;
	}

	// process cheat codes
	switch ( nCheatCode )
	{
		case CHEAT_GOD:			// god mode toggle
			ToggleCheat(nCheatCode, "You are a GOD.", "God mode off. Help!");
			break;

		case CHEAT_KFA:			// god mode toggle
			ActivateCheat(nCheatCode, "Monolith Rulez...");
			break;

		case CHEAT_KFA2:		// god mode toggle (different weapons)
			ActivateCheat(nCheatCode, "Monolith Rulez...");
			break;

		case CHEAT_AMMO:		// full ammo
			ActivateCheat(nCheatCode, "You have full ammo.");
			break;

		case CHEAT_CLIP:		// toggle clipping mode
			ToggleCheat(nCheatCode, "Spectator mode on.", "Spectator mode off.");	
			break;

		case CHEAT_HEALTH:		// full health
			ActivateCheat(nCheatCode, "You have full health.");
			break;

		case CHEAT_WHEREAMI:	// Display position
			ToggleCheat(nCheatCode, "Display pos on.", "Display pos off.");	
			break;

		case CHEAT_STEALTH:		// Stealth mode
			ActivateCheat(nCheatCode, "Where did I go??.");
			break;

		case CHEAT_TRIPLEDAMAGE:	// Triple Damage
			ActivateCheat(nCheatCode, "Beefcake! BEEFCAKE!");
			break;

		case CHEAT_KILLALLAI:
			ActivateCheat(nCheatCode, "...Let Tchernobog sort'em out.");
			break;

		case CHEAT_INCSPEED:
		case CHEAT_INCSTRENGTH:	// Increase strength
		case CHEAT_CALEB:
		case CHEAT_OPHELIA:
		case CHEAT_ISHMAEL:
		case CHEAT_GABRIELLA:
			ActivateCheat(nCheatCode, DNULL);
			break; 
			
		case CHEAT_GOBLE:
		case CHEAT_SCORPIO:
			ActivateCheat(nCheatCode, "Brian L. Goble is a programming god!");
			break;

		case CHEAT_TOTARO:
			ActivateCheat(nCheatCode, "Jim Totaro is da man!");
			break;

#ifdef _DEMO
		case CHEAT_DEMOLEVEL2:
			g_pBloodClientShell->StartNewWorld("Demo_02", GAMETYPE_SINGLE, LOADTYPE_NEW_LEVEL);
			break;
#endif

		case CHEAT_GIVEALLINV:
		{
			ActivateCheat(nCheatCode, "You just bought our entire inventory!");
			break;
		}

		case CHEAT_POW_HEALTH:
		{
			ActivateCheat(nCheatCode, "Thanks nurse!");
			break;
		}
			
		case CHEAT_POW_MEGAHEALTH:
		{
			ActivateCheat(nCheatCode, "Thanks nurse! How bout a date?");
			break;
		}
			
		case CHEAT_POW_WARD:
		{
			ActivateCheat(nCheatCode, "Ward awarded");
			break;
		}
			
		case CHEAT_POW_NECROWARD:
		{
			ActivateCheat(nCheatCode, "Necroward awarded");
			break;
		}
			
		case CHEAT_POW_INVULN:
		{
			ActivateCheat(nCheatCode, "Light and strong");
			break;
		}
			
		case CHEAT_POW_STEALTH:
		{
			ActivateCheat(nCheatCode, "They'll never hear you coming");
			break;
		}
			
		case CHEAT_POW_ANGER:
		{
			ActivateCheat(nCheatCode, "...and the hurtfulness...");
			break;
		}

		case CHEAT_CHASEVIEW:
		{
			ToggleCheat(nCheatCode, DNULL, DNULL);
			break;
		}

		default:
			return;				// skip setting global cheat indicator for unhandled cheats
	}

	m_bPlayerCheated = DTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SendCheatMessage()
//
//	PURPOSE:	sends a cheat to the server
//
// ----------------------------------------------------------------------- //
			
void CCheatMgr::SendCheatMessage( CheatCodes nCheatCode, DBOOL bState )
{
	// Send the Message to the server
	HMESSAGEWRITE hMsg = m_pClientDE->StartMessage(CMSG_CHEAT);
	m_pClientDE->WriteToMessageByte(hMsg, (DBYTE)nCheatCode);
	m_pClientDE->WriteToMessageByte(hMsg, (DBYTE)bState);
	m_pClientDE->EndMessage(hMsg);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::ToggleCheat()
//
//	PURPOSE:	Generic cheat toggle function can be used for many cheats
//
// ----------------------------------------------------------------------- //
			
void CCheatMgr::ToggleCheat(CheatCodes nCheatCode, char *szMsgOn, char *szMsgOff)
{
	s_CheatInfo[nCheatCode].bActive = !s_CheatInfo[nCheatCode].bActive;

	if (s_CheatInfo[nCheatCode].bActive)
		g_pMessageMgr->AddLine(szMsgOn);
	else
		g_pMessageMgr->AddLine(szMsgOff);

	// Tell the server
	SendCheatMessage(nCheatCode, s_CheatInfo[nCheatCode].bActive);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::ActivateCheat()
//
//	PURPOSE:	Activates the given cheat code
//
// ----------------------------------------------------------------------- //
			
void CCheatMgr::ActivateCheat(CheatCodes eCheatCode, char *szMsgOn)
{
	s_CheatInfo[eCheatCode].bActive = DTRUE;

	// Tell the server
	SendCheatMessage(eCheatCode, DTRUE);

	if (szMsgOn)
		g_pMessageMgr->AddLine(szMsgOn);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsValidDemoCheat
//
//	PURPOSE:	Determines if the given cheat code is valid in the demo
//
// ----------------------------------------------------------------------- //
			
DBOOL IsValidDemoCheat(int nCheat)
{
	// Check if this is a valid cheat code...

	if (nCheat == CHEAT_GOD) return(DTRUE);
	if (nCheat == CHEAT_KFA) return(DTRUE);
	if (nCheat == CHEAT_WHEREAMI) return(DTRUE);
	if (nCheat == CHEAT_DEMOLEVEL2) return(DTRUE);
	if (nCheat == CHEAT_CLIP) return(DTRUE);


	// If we get here, it's not a valid cheat for the demo...

	return(DFALSE);
}


