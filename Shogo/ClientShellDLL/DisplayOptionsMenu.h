#ifndef __DISPLAYOPTIONSMENU_H
#define __DISPLAYOPTIONSMENU_H

#include "BaseMenu.h"
#include "DisplayModeMenu.h"
#include "DetailSettingsMenu.h"

class CDisplayOptionsMenu : public CBaseMenu
{
public:

	CDisplayOptionsMenu();

	virtual LTBOOL		Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset();
	
	virtual LTBOOL		LoadAllSurfaces()		{ if (!m_DisplayModeMenu.LoadAllSurfaces() || !m_DetailSettingsMenu.LoadAllSurfaces()) return LTFALSE; return LoadSurfaces(); }
	virtual void		UnloadAllSurfaces()		{ m_DisplayModeMenu.UnloadAllSurfaces(); m_DetailSettingsMenu.UnloadAllSurfaces(); UnloadSurfaces(); }

	virtual void		OnEnterWorld()		{ 
											m_DisplayModeMenu.OnEnterWorld();
											m_DetailSettingsMenu.OnEnterWorld();
											CBaseMenu::OnEnterWorld(); 
											}

	virtual void		OnExitWorld()		{ 
											m_DisplayModeMenu.OnExitWorld();
											m_DetailSettingsMenu.OnExitWorld();
											CBaseMenu::OnExitWorld(); 
											}

	virtual void		Up();
	virtual void		Down();
	virtual void		Left();
	virtual void		Right();
	virtual void		PageUp();
	virtual void		PageDown();
	virtual void		Home();
	virtual void		End();
	virtual void		Return();
	virtual void		Esc();

	virtual void		Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

	void				SetGlobalDetail (int nSetting);
	void				SetupCurrentRendererSurfaces();

protected:

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

	virtual void		PostCalculateMenuDims();

	virtual void		DrawNoGoreVersion (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

protected:

	int					m_nSecondColumn;
	
	GENERIC_ITEM		m_RendererLine1;
	GENERIC_ITEM		m_RendererLine2;
	GENERIC_ITEM		m_Resolution;
	GENERIC_ITEM		m_TextureDepth;

	GENERIC_ITEM		m_CurrentRenderer;
	GENERIC_ITEM		m_GoreSetting;
	GENERIC_ITEM		m_ScreenFlash;
	GENERIC_ITEM		m_DetailSetting;

	LTFLOAT				m_fOriginalDetailLevel;

	CDisplayModeMenu	m_DisplayModeMenu;
	CDetailSettingsMenu	m_DetailSettingsMenu;
};

#endif
