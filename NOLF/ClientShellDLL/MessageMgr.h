//-------------------------------------------------------------------------
//
// MODULE  : MessageMgr.h
//
// PURPOSE : Messaging system - ruthlessly stolen from Greg Kettell (B2)
//
// CREATED : 10/22/97
//
// REVISED : 10/27/99 - jrg
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __MESSAGE_MGR_H__
#define __MESSAGE_MGR_H__

#include "iclientshell.h"
#include "iltclient.h"
#include "CheatDefs.h"
#include "LTGUIMgr.h"

// defines
enum eMessageType
{
	MMGR_DEFAULT = 0,
	MMGR_CHAT,
	MMGR_PICKUP,
	MMGR_TEAMCHAT,
	MMGR_TEAM_1,
	MMGR_TEAM_2,

	MMGR_NUM_MESSAGE_TYPES
};


#define	NUM_RECENT_LINES			16

class CGameClientShell;

// Input line class
class CInputLine
{
	public:

		CInputLine()
		{
			m_nTextLen = 0;
			*m_zText = '\0';
            m_bShift = LTFALSE;
			memset (m_pRecentLines, 0, NUM_RECENT_LINES * (kMaxInputLine + 2));
            memset (m_bRecentLineUsed, 0, NUM_RECENT_LINES * sizeof (LTBOOL));
			m_nBaseRecentLine = 0;
			m_nCurrentRecentLine = -1;
		}

        LTBOOL       Init ();
		void		Clear( void );
		void		Term( void );
		void		Draw(HSURFACE hDest,long x, long y);
        LTBOOL       AddChar( uint8 ch );
		void		DelChar( void );
		void		Set( char *pzText );
		void		Send( void );
        LTBOOL       HandleKeyDown(int key, int rep);
		void		HandleKeyUp(int key);
		void		HandleChar(char c);

	private:

		enum { kMaxInputLine = 80 };

		void				AddToRecentList();		// adds current string to recent-string list

		HSURFACE			m_hSurface;				// The surface to draw to
        LTBOOL               m_bShift;               // Shift is active
		int					m_nTextLen;				// Current length of input string
		char				m_zText[ kMaxInputLine + 2 ]; // The buffer
		char				m_pRecentLines[ NUM_RECENT_LINES ][ kMaxInputLine + 2 ];	// recent lines (for doskey effect)
        LTBOOL               m_bRecentLineUsed[ NUM_RECENT_LINES ];                      // has this recent line been used?
		int					m_nBaseRecentLine;		// current most-recent-line
		int					m_nCurrentRecentLine;	// selected recent line
};

// Message structure
struct Message
{
	Message()
	{
        hForeText = LTNULL;
        hSurface = LTNULL;
		eType = MMGR_DEFAULT;
		nHeight = 0;
	}
    LTFLOAT         fExpiration;
	HSURFACE		hForeText;
	HSURFACE		hSurface;
	eMessageType	eType;
	int				nHeight;
	LTIntPt			textOffset;
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
		};

	public:

		CMessageMgr();

        LTBOOL       Init ();
		void		Term ()  { Clear();  }

        void        Enable( LTBOOL bEnabled );

        void        AddLine( char *szMsg, eMessageType eType = MMGR_DEFAULT, HSURFACE hSurf = LTNULL);
        void        AddLine( HSTRING hMsg, eMessageType eType = MMGR_DEFAULT, HSURFACE hSurf = LTNULL);
        void        AddLine( int nStringId, eMessageType eType = MMGR_DEFAULT, HSURFACE hSurf = LTNULL);
		void		Clear( void );
		void		Draw( void );
        LTBOOL       HandleKeyDown(int key, int rep);
		void		HandleKeyUp(int key) { m_InputLine.HandleKeyUp(key); }
		void		HandleChar(char c);

		void		SetCoordinates( int x, int y );
		void		SetMaxMessages( int nMaxMessages );
        void        SetMessageTime( LTFLOAT fSeconds );
        void        SetEditingState(LTBOOL bEditing, LTBOOL bTeamMsg = LTFALSE);

        LTBOOL       IsTeamMsg()     {return m_bEditing && m_bTeamMsg; }

        LTBOOL       GetState( void )        { return m_bEnabled; }
		int			GetMaxMessages( void )	{ return m_nMaxMessages; }
        LTFLOAT      GetMessageTime( void )  { return m_fMessageTime; }
        LTBOOL       GetEditingState() { return m_bEditing; }

	private:

		void				DeleteMessageData(Message *pMsg);

		CInputLine			m_InputLine;	// Current input message
		CLTGUIFont*			m_pForeFont;		// menu font

        LTBOOL               m_bEnabled;
        LTBOOL               m_bEditing;
        LTBOOL               m_bTeamMsg;

		int					m_x;
		int					m_y;

		int					m_nMaxMessages;
        LTFLOAT              m_fMessageTime;
        LTFLOAT              m_fMessageFade;

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
            g_pLTClient->DeleteSurface(pMsg->hSurface);
            pMsg->hSurface = LTNULL;
		}
		if (pMsg->hForeText)
		{
            g_pLTClient->DeleteSurface(pMsg->hForeText);
            pMsg->hForeText = LTNULL;
		}
	}
}


// Cheat Manager class
class CCheatMgr
{
	public:
		CCheatMgr() {}

		void	Init();

        LTBOOL   Check( char *pzText );
        void    ClearCheater() { m_bPlayerCheated = LTFALSE; }
        LTBOOL   IsCheater() { return m_bPlayerCheated; }

	protected:
		void	Process( CheatCode nCheatCode );

	private:

        void    SendCheatMessage( CheatCode nCheatCode, uint8 nData );


		struct CheatInfo
		{
			char			*pzText;
            LTBOOL           bActive;
		};

		static CheatInfo s_CheatInfo[];

        static LTBOOL m_bPlayerCheated;

		// Helper functions
        void    SetGodMode(LTBOOL bMode);
		void	SetAmmo();
		void	SetArmor();
		void	SetHealth(LTBOOL bPlaySound=LTTRUE);
        void    SetClipMode(LTBOOL bMode);
		void	Teleport();
        void    SetPos(LTBOOL bMode);
        void    SetCamPosRot(LTBOOL bMode);
		void	SetMissionInventory();
		void	SetFullWeapons();
		void	SetKFA();
		void	ModSquad();
		void	FullGear();
        void    PosWeapon(LTBOOL bMode);
        void    PosWeaponMuzzle(LTBOOL bMode);
        void    LightScale(LTBOOL bMode);
        void    LightAdd(LTBOOL bMode);
        void    FOV(LTBOOL bMode);
        void    InterfaceAdjust(LTBOOL bMode);
        void    HeadBob(LTBOOL bMode);
        void    Tears(LTBOOL bMode);
        void    RemoveAI(LTBOOL bMode);
        void    TriggerBox(LTBOOL bMode);
        void    Breach(LTBOOL bMode);
        void    Pos1stCam(LTBOOL bMode);
        void    Motorcycle(LTBOOL bMode);
        void    Snowmobile(LTBOOL bMode);
        void    GolfCart(LTBOOL bMode);
		void	ChaseToggle();
        void    AllowAllMissions(LTBOOL bMode);
		void	ResetHistory();
		void	SetExitLevel();
		void	Version();
		void	MikeD();
		void	BigBlood(LTBOOL bMode);
};



#endif	// __MESSAGE_MGR_H__