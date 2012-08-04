#ifndef __RIOTMENU_H
#define __RIOTMENU_H

#include "MainMenu.h"
#include "RiotSettings.h"
#include "ClientUtilities.h"

#define MAX_MENU_SPACING	5
#define MIN_MENU_SPACING	2

class ILTClient;
class CRiotClientShell;
class CBaseMenu;

class CRiotMenu
{
public:

	CRiotMenu();
	~CRiotMenu();

	LTBOOL				Init (ILTClient* pClientDE, CRiotClientShell* pClientShell);
	void				Term();

	LTBOOL				LoadAllSurfaces();
	void				UnloadAllSurfaces();

	void				HandleInput (int vkey);

	CRiotClientShell*	GetClientShell()			{ return m_pClientShell; }
	CRiotSettings*		GetSettings()				{ return &m_Settings; }
	
	CBaseMenu*			GetCurrentMenu ()					{ return m_pCurrentMenu; }
	void				SetCurrentMenu (CBaseMenu* pMenu)	{ if (pMenu) m_pCurrentMenu = pMenu; else m_pCurrentMenu = &m_MainMenu; }

	CMainMenu*			GetMainMenu ()				{ return &m_MainMenu; }

	HSURFACE			GetMenuArtwork()			{ return m_hArt; }

	HSURFACE			GetUpArrow()				{ return m_hUpArrow; }
	HSURFACE			GetDownArrow()				{ return m_hDnArrow; }
	int					GetArrowHeight()			{ return m_nArrowHeight; }
	int					GetArrowWidth()				{ return m_nArrowWidth; }

	void				OnEnterWorld()				{ m_MainMenu.OnEnterWorld(); }
	void				OnExitWorld()				{ m_MainMenu.OnExitWorld(); }
	LTBOOL				InWorld();

	void				ScreenDimsChanged();

	void				Draw();

	void				ExitMenu (LTBOOL bLoadingLevel = LTFALSE);

	void				StopMenuMusic();

	void				SetErrorMsg (uint32 nStringID, char* strFilename, uint32 nLineNumber);
	char*				GetErrorMsg()				{ return m_strErrorMsg; }

	CFont08*			GetFont08n()				{ return &m_font08n; }
	CFont08*			GetFont08s()				{ return &m_font08s; }
	CFont12*			GetFont12n()				{ return &m_font12n; }
	CFont12*			GetFont12s()				{ return &m_font12s; }
	CFont18*			GetFont18n()				{ return &m_font18n; }
	CFont18*			GetFont18s()				{ return &m_font18s; }
	CFont28*			GetFont28n()				{ return &m_font28n; }
	CFont28*			GetFont28s()				{ return &m_font28s; }

protected:

	LTBOOL				InitControls();

protected:

	ILTClient*			m_pClientDE;
	CRiotClientShell*	m_pClientShell;

	CRiotSettings		m_Settings;

	CBaseMenu*			m_pCurrentMenu;

	HSURFACE			m_hArt;
	CSize				m_szArt;
	LTRect				m_rcArtDest;
	CSize				m_szScreen;

	HSURFACE			m_hUpArrow;
	HSURFACE			m_hDnArrow;
	int					m_nArrowHeight;
	int					m_nArrowWidth;

	CFont08				m_font08n;
	CFont08				m_font08s;
	CFont12				m_font12n;
	CFont12				m_font12s;
	CFont18				m_font18n;
	CFont18				m_font18s;
	CFont28				m_font28n;
	CFont28				m_font28s;
	
	CMainMenu			m_MainMenu;

	char				m_strErrorMsg[256];
};

#endif