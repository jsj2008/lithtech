#ifndef __MAINMENU_H
#define __MAINMENU_H

#include "BaseMenu.h"
#include "SinglePlayerMenu.h"
#include "OptionsMenu.h"

class CMainMenu : public CBaseMenu
{
public:

	CMainMenu();

	virtual LTBOOL		Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset();
	
	virtual LTBOOL		LoadAllSurfaces()		{ if (!m_SinglePlayerMenu.LoadAllSurfaces() || !m_OptionsMenu.LoadAllSurfaces()) return LTFALSE; return LTTRUE; }
	virtual void		UnloadAllSurfaces()		{ m_SinglePlayerMenu.UnloadAllSurfaces(); m_OptionsMenu.UnloadAllSurfaces(); }
	
	virtual void		OnEnterWorld()		{ 
											m_SinglePlayerMenu.OnEnterWorld();
											m_OptionsMenu.OnEnterWorld();
											CBaseMenu::OnEnterWorld();
											}
	
	virtual void		OnExitWorld()		{ 
											m_SinglePlayerMenu.OnExitWorld();
											m_OptionsMenu.OnExitWorld();
											CBaseMenu::OnExitWorld();
											}
	
	CSinglePlayerMenu*	GetSinglePlayerMenu()	{ return &m_SinglePlayerMenu; }

	virtual void		Return();
	virtual void		Esc();

	virtual void		Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

			LTBOOL		DoMultiplayer(LTBOOL bMinimize);

protected:

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

protected:

	LTBOOL				m_bFirstDraw;

	LTFLOAT				m_fVersionDisplayTimeLeft;
	HSURFACE			m_hVersion;
	CSize				m_szVersion;

	CSinglePlayerMenu	m_SinglePlayerMenu;
	COptionsMenu		m_OptionsMenu;
};

#endif
