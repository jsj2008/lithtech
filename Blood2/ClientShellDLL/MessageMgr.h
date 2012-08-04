//----------------------------------------------------------
//
// MODULE  : MessageMgr.h
//
// PURPOSE : Blood 2 Messaging system - Sorta derived from
//			 Blood's messaging system
//
// CREATED : 10/22/97
//
//----------------------------------------------------------

#ifndef __MESSAGEMGR_H__
#define __MESSAGEMGR_H__


#include "cpp_clientshell_de.h"
#include "client_de.h"
#include "DynArray.h"
#include "SharedDefs.h"


// defines
#define MESSAGEFLAGS_INCOMING		0x01
#define MESSAGEFLAGS_SELFGET		0x02
#define MESSAGEFLAGS_OTHERGET		0x04
#define MESSAGEFLAGS_RESPAWN		0x08
#define MESSAGEFLAGS_MASK			0x0F


#define MAXINPUTLINE	80

// Input line class
class CInputLine
{
	public:

		CInputLine() 
		{ 
			m_nTextLen = 0; 
			*m_zText = '\0'; 
			m_bShift = DFALSE;
		}

		void		Init (CClientDE* pClientDE);
		void		Clear( void );
		void		Term( void );
		void		Draw(HSURFACE hDest, HDEFONT hFont, HDECOLOR hForeColor, HDECOLOR hBackColor, long x, long y);
		DBOOL		AddChar( DBYTE ch );
		void		DelChar( void );
		void		Set( char *pzText );
		void		Send( void );
		DBOOL		HandleKeyDown(int key, int rep);
		void		HandleKeyUp(int key);

	private:

		char		AsciiXlate(int key);	// Translates VK_ values to ascii

		CClientDE*	m_pClientDE;			// the client interface
		HSURFACE	m_hSurface;				// The surface to draw to
		DBOOL		m_bShift;				// Shift is active
		int			m_nTextLen;				// Current length of input string
		char		m_zText[ MAXINPUTLINE + 2 ]; // The buffer

};

// Message structure
struct Message
{
	Message()
	{
		hMessage = DNULL;
		hSurface = DNULL;
	}
	DFLOAT fExpiration;
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
	
		DBOOL		Init (CClientDE* pClientDE);
		void		Term ()  { Clear(); if (m_hFont) m_pClientDE->DeleteFont(m_hFont); }

		void		Enable( DBOOL bEnabled );

		void		AddLine( char *szMsg, DBYTE dbMessageFlag = MESSAGEFLAGS_MASK );
		void		AddLine( HSTRING hMsg, DBYTE dbMessageFlag = MESSAGEFLAGS_MASK );
		void		Clear( void );
		void		Draw( void );
		DBOOL		DrawCheatMsg( void );
		DBOOL		HandleKeyDown(int key, int rep);
		void		HandleKeyUp(int key) { m_InputLine.HandleKeyUp(key); }

		void		SetCoordinates( int x, int y );
		void		SetMaxMessages( int nMaxMessages );
		void		SetMessageTime( DFLOAT fSeconds );
		void		SetMessageFlags( DDWORD dwFlags );
		void		SetEditingState(DBOOL bEditing) { m_bEditing = bEditing; }

		DBOOL		GetState( void )		{ return m_bEnabled; }
		int			GetMaxMessages( void )	{ return m_nMaxMessages; }
		DFLOAT		GetMessageTime( void )	{ return m_fMessageTime; }
		DBOOL		GetEditingState() { return m_bEditing; }

	private:

		void		DeleteMessageData(Message *pMsg);

		CClientDE*	m_pClientDE;			// the client interface
		CInputLine	m_InputLine;			// Current input message
		HDEFONT		m_hFont;				// menu font

		DBOOL		m_bEnabled;
		DBOOL		m_bEditing;

		int			m_x;
		int			m_y;

		int			m_nFontHeight;

		int			m_nMaxMessages;
		DFLOAT		m_fMessageTime;
		DDWORD		m_dwMessageFlags;

		int			m_nMessageCount;
		int			m_nFirstMessage;
		int			m_nNextMessage;

		Message		m_Messages[ kMaxMessages ];
};

inline void CMessageMgr::DeleteMessageData(Message *pMsg)
{
	if (pMsg)
	{
		if (pMsg->hSurface)
		{
			m_pClientDE->DeleteSurface(pMsg->hSurface);
			pMsg->hSurface = DNULL;
		}
		if (pMsg->hMessage)
		{
			m_pClientDE->FreeString(pMsg->hMessage);
			pMsg->hMessage = DNULL;
		}
	}
}


// Cheat Manager class
class CCheatMgr
{
	friend		CMessageMgr;
	public:
		CCheatMgr() {}

		void	Init(CClientDE* pClientDE);

		void	Reset();

		DBOOL	Check( char *pzText );
		void	ClearCheater() { m_bPlayerCheated = DFALSE; }
		DBOOL	IsCheater() { return m_bPlayerCheated; }
		DBOOL	IsCheatActive(CheatCodes nCheatCode) 
		{ 
			if ( nCheatCode < CHEAT_NONE || nCheatCode >= CHEAT_MAX )
				return DFALSE;
			else
				return s_CheatInfo[nCheatCode].bActive;
		}

	protected:
		void	Process( CheatCodes nCheatCode );

	private:

		void	SendCheatMessage( CheatCodes nCheatCode, DBOOL bState );
		void	Decrypt();

		CClientDE*	m_pClientDE;			// the client interface

		struct CheatInfo
		{
			char			*pzText;
			DBOOL			bActive;
		};

		static CheatInfo s_CheatInfo[];

		static DBOOL m_bPlayerCheated;

		// Helper functions
		void	ToggleCheat(CheatCodes eCheatCode, char* pOnMsg, char* pOffMsg);
		void	ActivateCheat(CheatCodes eCheatCode, char* pOnMsg);
};



#endif	// __MESSAGEMGR_H__
