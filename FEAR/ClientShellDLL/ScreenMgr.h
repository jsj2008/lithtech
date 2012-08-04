// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenMgr.h
//
// PURPOSE : Interface screen manager
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_SCREEN_MGR_H_)
#define _SCREEN_MGR_H_

#include "InterfaceResMgr.h"

#define MAX_SCREEN_HISTORY 20

class CGameClientShell;
class CBaseScreen;
//class CTransitionFXMgr;

enum eScreenID
{
#define INCLUDE_AS_ENUM
#include "ScreenEnum.h"
#undef INCLUDE_AS_ENUM
};


class CScreenMgr
{
public:
	CScreenMgr();
	virtual ~CScreenMgr();
    virtual bool              Init();
	virtual void				Term();

	virtual void		HandleKeyDown (int vkey, int rep);
	virtual void		HandleKeyUp (int vkey);
	virtual void		HandleChar (wchar_t c);

	// Mouse messages
	virtual void		OnLButtonDown(int x, int y);
	virtual void		OnLButtonUp(int x, int y);
	virtual void		OnLButtonDblClick(int x, int y);
	virtual void		OnRButtonDown(int x, int y);
	virtual void		OnRButtonUp(int x, int y);
	virtual void		OnRButtonDblClick(int x, int y);
	virtual void		OnMouseMove(int x, int y);
	virtual void		OnMouseWheel(int x, int y, int zDelta);


    virtual bool		ForceScreenUpdate(eScreenID screenID);

	virtual eScreenID	GetCurrentScreenID()		{return m_eCurrentScreenID;}
	virtual eScreenID	GetLastScreenID()			{return m_eLastScreenID;}
    virtual bool		SetCurrentScreen(eScreenID screenID);
    virtual bool		PreviousScreen();
	virtual void		EscapeCurrentScreen();
	virtual void		ExitScreens();
	virtual void		EnterScreens(bool bCreateMedia);

	// Get from history, where nHistoryIndex=0 is the most recent in history.
	eScreenID			GetFromHistory( int nHistoryIndex );

	virtual const char *GetScreenName(eScreenID id);
	virtual uint16		GetScreenIDFromName(char * pName);

	// Renders the screen to a surface
    virtual bool	    Render();
	virtual bool		UpdateInterfaceSFX();
	virtual void		ScreenDimsChanged();

	virtual CBaseScreen*	GetScreenFromID(eScreenID screenID);


	virtual void ClearHistory();
	virtual void AddScreenToHistory(eScreenID screenID);

	// Accessors to common controls used on most screens.
	CLTGUITextCtrl&		GetBackCtrl( ) { return m_Back; }
	CLTGUIString&		GetHelpStr( ) { return m_HelpStr; }
	LTRect2n const&		GetHelpRect( ) const { return m_HelpRect; }
	uint32				GetHelpSize( ) const { return m_HelpSize; }
	CFontInfo&			GetHelpFont( ) { return m_HelpFont; }
	uint16				GetHelpWidth( ) const { return m_HelpWidth; }
	LTRect2n const& 	GetBackRect( ) const { return m_BackRect; }

protected:
	void			AddScreen(eScreenID screenID);
	virtual void	SwitchToScreen(CBaseScreen *pNewScreen);
	void			RemoveMedia();  // Used to free m_hMovie, m_hStaticBackground, and m_hMusic
	void			UpdateMovieDims();

protected:

	int				m_nHistoryLen;
	eScreenID		m_eScreenHistory[MAX_SCREEN_HISTORY];
	eScreenID		m_eCurrentScreenID;
	eScreenID		m_eLastScreenID;
	CBaseScreen*	m_pCurrentScreen;		// The current screen
	
//	CTransitionFXMgr * m_pTransitionFXMgr;	// ABM 2/7/02 added for screens to access

	//screens
	typedef std::vector<CBaseScreen *, LTAllocator<CBaseScreen *, LT_MEM_TYPE_CLIENTSHELL> > ScreenArray;
	ScreenArray m_screenArray;			// Pointer to each screen

	// Global screen elements used by all screens.
	CLTGUITextCtrl	m_Back;
	CLTGUIString m_HelpStr;
	LTRect2n    m_HelpRect;
	uint32		m_HelpSize;
	CFontInfo	m_HelpFont;
	uint16		m_HelpWidth;
	LTRect2n	m_BackRect;

	// Media that may play while the screen is up...
	HVIDEOTEXTURE		m_hMovie;
	LTRect2f			m_rfMovieDest;
	TextureReference	m_hStaticBackground;
	HLTSOUND			m_hMusic;
};

#endif // _SCREENMGR_H_