#ifndef __BASEMENU_H
#define __BASEMENU_H

#include "ClientUtilities.h"
#include "font08.h"
#include "font12.h"
#include "font18.h"
#include "font28.h"

class CRiotMenu;
class CBaseMenu;

#define MAX_GENERIC_ITEMS		20

struct GENERIC_ITEM
{
	GENERIC_ITEM()				{ hMenuItem = LTNULL; hMenuItemSelected = LTNULL; szMenuItem.cx = szMenuItem.cy = 0; pChildMenu = LTNULL; }

	HSURFACE	hMenuItem;
	HSURFACE	hMenuItemSelected;
	CSize		szMenuItem;
	CBaseMenu*	pChildMenu;
};

class CBaseMenu
{
public:

	CBaseMenu();
	~CBaseMenu();

	virtual LTBOOL		Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight);
	virtual void		ScreenDimsChanged (int nScreenWidth, int nScreenHeight);
	virtual void		Reset()								{ m_nSelection = 0; m_nTopItem = 0; }

	virtual LTBOOL		LoadAllSurfaces()					{ return LTTRUE; }
	virtual void		UnloadAllSurfaces()					{}

	virtual void		OnEnterWorld();
	virtual void		OnExitWorld();

	virtual void		HandleInput (int vKey)				{}
	virtual void		Up();
	virtual void		Down();
	virtual void		Right();
	virtual void		Left();
	virtual void		PageUp();
	virtual void		PageDown();
	virtual void		Home();
	virtual void		End();
	virtual void		Return();
	virtual void		Esc();

	virtual void		PlayUpSound()				{ PlaySound ("Sounds\\Interface\\Highlight.wav"); }
	virtual void		PlayDownSound()				{ PlaySound ("Sounds\\Interface\\Highlight.wav"); }
	virtual void		PlayRightSound()			{ PlaySound ("Sounds\\Interface\\Scroll.wav"); }
	virtual void		PlayLeftSound()				{ PlaySound ("Sounds\\Interface\\Scroll.wav"); }
	virtual void		PlayPageUpSound()			{ PlaySound ("Sounds\\Interface\\Highlight.wav"); }
	virtual void		PlayPageDownSound()			{ PlaySound ("Sounds\\Interface\\Highlight.wav"); }
	virtual void		PlayHomeSound()				{ PlaySound ("Sounds\\Interface\\Highlight.wav"); }
	virtual void		PlayEndSound()				{ PlaySound ("Sounds\\Interface\\Highlight.wav"); }
	virtual void		PlayReturnSound()			{ PlaySound ("Sounds\\Interface\\Select.wav"); }
	virtual void		PlayEscSound()				{ PlaySound ("Sounds\\Interface\\Select.wav"); }
	virtual void		PlaySound (char* pSound)	{ PlaySoundLocal( pSound, SOUNDPRIORITY_MISC_MEDIUM); }

	virtual void		Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset = 0);

protected:

			int			GetMenuAreaTop()			{ return ((int)m_szScreen.cy - (int)m_szMenuArea.cy) / 2; }
			int			GetMenuAreaBottom()			{ return GetMenuAreaTop() + (int)m_szMenuArea.cy; }
			int			GetMenuAreaLeft()			{ return ((int)m_szScreen.cx - (int)m_szMenuArea.cx) / 2; }
			int			GetMenuAreaRight()			{ return GetMenuAreaLeft() + (int)m_szMenuArea.cx; }

	virtual LTBOOL		LoadSurfaces();
	virtual void		UnloadSurfaces();

	virtual HSURFACE	CropMenuItemTop (HSURFACE hSurf);
	virtual HSURFACE	CropMenuItemBottom (HSURFACE hSurf);
	virtual HSURFACE	AddUnderline (HSURFACE hSurf, int nSpacing = 2, int nThickness = 3);

	virtual void		CalculateMenuDims();
	virtual void		PostCalculateMenuDims();
	virtual void		CheckSelectionOffMenuTop();
	virtual void		CheckSelectionOffMenuBottom();

protected:

	ILTClient*		m_pClientDE;
	CRiotMenu*		m_pRiotMenu;
	CBaseMenu*		m_pParent;

	int				m_nTopItem;
	int				m_nSelection;

	CSize			m_szScreen;
	CSize			m_szMenuArea;

	HSURFACE		m_hMenuTitle;
	CSize			m_szMenuTitle;
	int				m_nMenuTitleSpacing;

	GENERIC_ITEM	m_GenericItem[MAX_GENERIC_ITEMS];
	int				m_nMenuX;
	int				m_nMenuY;
	int				m_nMenuSpacing;
	int				m_nGenericItems;
};

#endif
