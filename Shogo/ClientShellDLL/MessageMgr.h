//-------------------------------------------------------------------------
//
// MODULE  : MessageMgr.h
//
// PURPOSE : Riot Messaging system - ruthlessly stolen from Greg Kettell
//
// CREATED : 10/22/97
//
//-------------------------------------------------------------------------

#ifndef __MESSAGEMGR_H__
#define __MESSAGEMGR_H__


#include "clientheaders.h"
#include "iltclient.h"
#include "DynArray.h"
#include "CheatDefs.h"

// defines
#define MESSAGEFLAGS_INCOMING		0x01
#define MESSAGEFLAGS_SELFGET		0x02
#define MESSAGEFLAGS_OTHERGET		0x04
#define MESSAGEFLAGS_RESPAWN		0x08
#define MESSAGEFLAGS_MASK			0x0F

#define	NUM_RECENT_LINES			16

// Input line class
class CInputLine
{
	public:

		CInputLine() 
		{ 
			m_pClientDE = LTNULL;
			m_pClientShell = LTNULL;

			m_nTextLen = 0; 
			*m_zText = '\0'; 
			m_bShift = LTFALSE;
			memset (m_pRecentLines, 0, NUM_RECENT_LINES * (kMaxInputLine + 2));
			memset (m_bRecentLineUsed, 0, NUM_RECENT_LINES * sizeof (LTBOOL));
			m_nBaseRecentLine = 0;
			m_nCurrentRecentLine = -1;
		}

		LTBOOL		Init (ILTClient* pClientDE, CRiotClientShell* pClientShell);
		void		Clear( void );
		void		Term( void );
		void		Draw(HSURFACE hDest, HLTFONT hFont, HLTCOLOR hForeColor, HLTCOLOR hBackColor, long x, long y);
		LTBOOL		AddChar( uint8 ch );
		void		DelChar( void );
		void		Set( char *pzText );
		void		Send( void );
		LTBOOL		HandleKeyDown(int key, int rep);
		void		HandleKeyUp(int key);

	private:

		enum { kMaxInputLine = 80 };

		char				AsciiXlate(int key);	// Translates VK_ values to ascii
		void				AddToRecentList();		// adds current string to recent-string list

		ILTClient*			m_pClientDE;			// the client interface
		CRiotClientShell*	m_pClientShell;			// the client shell interface
		HSURFACE			m_hSurface;				// The surface to draw to
		LTBOOL				m_bShift;				// Shift is active
		int					m_nTextLen;				// Current length of input string
		char				m_zText[ kMaxInputLine + 2 ]; // The buffer
		char				m_pRecentLines[ NUM_RECENT_LINES ][ kMaxInputLine + 2 ];	// recent lines (for doskey effect)
		LTBOOL				m_bRecentLineUsed[ NUM_RECENT_LINES ];						// has this recent line been used?
		int					m_nBaseRecentLine;		// current most-recent-line
		int					m_nCurrentRecentLine;	// selected recent line
};

// Message structure
struct Message
{
	Message()
	{
		hMessage = LTNULL;
		hSurface = LTNULL;
	}
	LTFLOAT fExpiration;
	HSTRING  hMessage;
	HSURFACE hSurface;
};


// Message manager class
class CMessageMgr
{
	public:

		// Local constants
		enum
		{
			kMaxMessages		= 20,
			kMaxMessageSize		= 128,

			kDefaultMaxMessages	= 4,
			kDefaultMessageTime	= 5,	// seconds
		};

	public:

		CMessageMgr();
	
		LTBOOL		Init (ILTClient* pClientDE, CRiotClientShell* pClientShell);
		void		Term ()  { Clear(); if (m_hFont) m_pClientDE->DeleteFont(m_hFont); }

		void		Enable( LTBOOL bEnabled );

		void		AddLine( char *szMsg, uint8 dbMessageFlag = MESSAGEFLAGS_MASK );
		void		AddLine( HSTRING hMsg, uint8 dbMessageFlag = MESSAGEFLAGS_MASK );
		void		Clear( void );
		void		Draw( void );
		LTBOOL		HandleKeyDown(int key, int rep);
		void		HandleKeyUp(int key) { m_InputLine.HandleKeyUp(key); }

		void		SetCoordinates( int x, int y );
		void		SetMaxMessages( int nMaxMessages );
		void		SetMessageTime( LTFLOAT fSeconds );
		void		SetMessageFlags( uint32 dwFlags );
		void		SetEditingState(LTBOOL bEditing) { m_bEditing = bEditing; }

		LTBOOL		GetState( void )		{ return m_bEnabled; }
		int			GetMaxMessages( void )	{ return m_nMaxMessages; }
		LTFLOAT		GetMessageTime( void )	{ return m_fMessageTime; }
		LTBOOL		GetEditingState() { return m_bEditing; }

	private:

		void				DeleteMessageData(Message *pMsg);

		ILTClient*			m_pClientDE;	// the client interface
		CRiotClientShell*	m_pClientShell;	// client shell interface
		CInputLine			m_InputLine;	// Current input message
		HLTFONT				m_hFont;		// menu font

		LTBOOL				m_bEnabled;
		LTBOOL				m_bEditing;

		int					m_x;
		int					m_y;

		int					m_nMaxMessages;
		LTFLOAT				m_fMessageTime;
		uint32				m_dwMessageFlags;

		int					m_nMessageCount;
		int					m_nFirstMessage;
		int					m_nNextMessage;

		Message				m_Messages[ kMaxMessages ];
};


inline void CMessageMgr::DeleteMessageData(Message *pMsg)
{
	if (pMsg)
	{
		if (pMsg->hSurface)
		{
			m_pClientDE->DeleteSurface(pMsg->hSurface);
			pMsg->hSurface = LTNULL;
		}
		if (pMsg->hMessage)
		{
			m_pClientDE->FreeString(pMsg->hMessage);
			pMsg->hMessage = LTNULL;
		}
	}
}


// Cheat Manager class
class CCheatMgr
{
	public:
		CCheatMgr() {}

		void	Init(ILTClient* pClientDE);

		LTBOOL	Check( char *pzText );
		void	ClearCheater() { m_bPlayerCheated = LTFALSE; }
		LTBOOL	IsCheater() { return m_bPlayerCheated; }

	protected:
		void	Process( CheatCode nCheatCode );

	private:

		void	SendCheatMessage( CheatCode nCheatCode, uint8 nData );

		ILTClient*	m_pClientDE;			// the client interface

		struct CheatInfo
		{
			char			*pzText;
			LTBOOL			bActive;
		};

		static CheatInfo s_CheatInfo[];

		static LTBOOL m_bPlayerCheated;

		// Helper functions
		void	SetGodMode(LTBOOL bMode);
		void	SetAmmo();
		void	SetArmor();
		void	SetHealth();
		void	SetClipMode(LTBOOL bMode);
		void	Teleport();
		void	SetOnFoot();
		void	SetVehicle();
		void	SetMech();
		void	SetPos(LTBOOL bMode);
		void	SetFullWeapons();
		void	SetKFA();
		void	PosWeapon(LTBOOL bMode);
		void	PosWeaponMuzzle(LTBOOL bMode);
		void	PlayerMovement(LTBOOL bMode);
		void	PlayerAccel(LTBOOL bMode);
		void	CameraOffset(LTBOOL bMode);
		void	LightScale(LTBOOL bMode);
		void	BigGuns(LTBOOL bMode);
		void	Tears(LTBOOL bMode);
		void	RemoveAI(LTBOOL bMode);
		void	TriggerBox(LTBOOL bMode);
		void	Anime(LTBOOL bMode);
		void	Robert(LTBOOL bMode);
		void	Thanks(LTBOOL bMode, uint8 nCode);
};



#endif	// __MESSAGEMGR_H__
