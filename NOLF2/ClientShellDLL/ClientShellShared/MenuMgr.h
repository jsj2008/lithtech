// ----------------------------------------------------------------------- //
//
// MODULE  : MenuMgr.h
//
// PURPOSE : Manager for in-game menus
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#if !defined(_MENU_MGR_H_)
#define _MENU_MGR_H_

#include "InterfaceResMgr.h"
#include "MenuSystem.h"
#include "MenuMission.h"
#include "MenuInventory.h"
#include "MenuKeys.h"
#include "MenuIntel.h"
#include "MenuPlayer.h"


// These flags are used in CMenuMgr::EnableMenuBar and need to correspond to each
// item on the menu bar

#define MB_SYSTEM		(1<<0)
#define MB_MISSION		(1<<1)
#define MB_INVENTORY	(1<<2)
#define MB_KEYS			(1<<3)
#define MB_INTEL		(1<<4)
#define MB_PLAYER		(1<<5)
#define MB_ALL			0xFFFFFFFF
#define MB_NUM_CTRLS	6

class CMenuHotKey
{
public:
	CMenuHotKey(int vk, int command, eMenuID id) {m_vk = vk; m_command = command; m_id = id;}
	int		m_vk;
	int		m_command;
	eMenuID	m_id;
};

class CMenuSlide
{
public:
	CMenuSlide();
	void	Start(LTIntPt startPos,LTIntPt endPos,float fDuration, bool bSlidingOut);
	void	Stop() {m_fDuration = 0.0f;}

	bool	IsStarted() {return (m_fDuration > 0.0f);}
	bool	IsDone();
	bool	IsSlidingOut() {return m_bSlidingOut;}

	LTIntPt GetCurrentPos();

private:
	LTIntPt	m_startPos;
	LTIntPt	m_offset;

	float	m_fStartTime;
	float	m_fDuration;

	bool	m_bSlidingOut;


};

class CMenuBar : public CSubMenu
{
public:
    virtual LTBOOL   HandleKeyUp(int key) {return LTFALSE;}
    virtual uint32 OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2);
};

class CMenuMgr
{
public:
	CMenuMgr();
	virtual ~CMenuMgr();
    LTBOOL	Init();
	void	Term();

	LTBOOL	HandleKeyDown (int vkey, int rep);
	void	HandleKeyUp (int vkey);
	LTBOOL  OnCommandOn(int command);
	LTBOOL  OnCommandOff(int command);


	// Mouse messages
	void	OnLButtonDown(int x, int y);
	void	OnLButtonUp(int x, int y);
	void	OnLButtonDblClick(int x, int y);
	void	OnRButtonDown(int x, int y);
	void	OnRButtonUp(int x, int y);
	void	OnRButtonDblClick(int x, int y);
	void	OnMouseMove(int x, int y);


	CBaseMenu*	GetMenu(eMenuID menuID);
	
	CBaseMenu*	GetCurrentMenu()		{return m_pCurrentMenu;}
	CBaseMenu*	GetLastMenu()			{return m_pLastMenu;}
	eMenuID		GetCurrentMenuID();
    LTBOOL		SetCurrentMenu(eMenuID menuID);
    LTBOOL		SetCurrentMenu(uint8 index);
    LTBOOL		NextMenu();
    LTBOOL		PreviousMenu();
	void		ExitMenus();

	//enables/disables menus based on game type
	void		EnableMenus();

	void		EnableMenuBar(bool bEnable=true, uint32 nMenuFlags=MB_ALL);

	const char* GetMenuName(eMenuID id);

    void	Update();

	// Renders the Menu to a surface
    void	Render();
	void	ScreenDimsChanged();

	void	ShowSubMenu(CSubMenu* pSubMenu);
	void	HideSubMenu(bool bSlideOut);

	void	RegisterHotKey(int vk, eMenuID id);
	void	RegisterCommand(int command, eMenuID id);

	void	SlideIn();
	void	SlideOut();

private:

	void	SwitchToMenu(CBaseMenu *pNewMenu);

	uint8			m_nMenuIndex;
	CBaseMenu*		m_pCurrentMenu;		// The current Menu
	CBaseMenu*		m_pLastMenu;		// The last menu displayed
	CSubMenu*		m_pSubMenu;		// The current sub Menu

	CMenuBar		m_MenuBar;

	//Menus
	typedef std::vector<CBaseMenu *> MenuArray;
	MenuArray m_MenuArray;			// Pointer to each Menu

	typedef std::vector<CMenuHotKey *> KeyArray;
	KeyArray m_HotKeys;			// Pointer to each Menu

	CMenuSystem		m_MenuSystem;
	CMenuMission	m_MenuMission;
	CMenuInventory	m_MenuInventory;
	CMenuKeys		m_MenuKeys;
	CMenuIntel		m_MenuIntel;
	CMenuPlayer		m_MenuPlayer;

	CMenuSlide		m_MenuSlide;
	CMenuSlide		m_BarSlide;
	CMenuSlide		m_SubSlide;

	float			m_fSlideInTime;
	float			m_fSlideOutTime;
	int				m_nMenuPos;
	int				m_nBarPos;
	std::string		m_sSlideInSound;
	std::string		m_sSlideOutSound;

};

#endif // _MENU_MGR_H_