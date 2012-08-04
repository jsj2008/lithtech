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
#include "BaseMenu.h"
#include "MenuSystem.h"
#include "MenuMission.h"
#include "MenuEndRound.h"
#include "EngineTimer.h"


class CMenuHotKey
{
public:
	CMenuHotKey(int vk, int command, eMenuID id) {m_vk = vk; m_command = command; m_id = id;}
	int		m_vk;
	int		m_command;
	eMenuID	m_id;
};



class CMenuMgr
{
public:
	CMenuMgr();
	virtual ~CMenuMgr();
    virtual bool	Init();
	virtual void	Term();

	virtual bool	HandleKeyDown (int vkey, int rep);
	virtual bool	HandleKeyUp (int vkey);
	virtual bool	OnCommandOn(int command);
	virtual bool	OnCommandOff(int command);


	// Mouse messages
	virtual void	OnLButtonDown(int x, int y);
	virtual void	OnLButtonUp(int x, int y);
	virtual void	OnLButtonDblClick(int x, int y);
	virtual void	OnRButtonDown(int x, int y);
	virtual void	OnRButtonUp(int x, int y);
	virtual void	OnRButtonDblClick(int x, int y);
	virtual void	OnMouseMove(int x, int y);
	virtual void	OnMouseWheel(int x, int y, int zDelta);

	CBaseMenu*	GetMenu(eMenuID menuID);
	
	CBaseMenu*	GetCurrentMenu()		{return m_pCurrentMenu;}
	CBaseMenu*	GetLastMenu()			{return m_pLastMenu;}
	eMenuID		GetCurrentMenuID( )	{ CBaseMenu* pCurMenu = GetCurrentMenu( ); return ( pCurMenu ) ? pCurMenu->GetMenuID() : MENU_ID_NONE; }
    bool		SetCurrentMenu( eMenuID menuID );
    bool		SetCurrentMenu( uint8 index );
    bool		NextMenu( );
    bool		PreviousMenu();
	void		ExitMenus();

	const char* GetMenuName(eMenuID id);

	// Renders the Menu to a surface
    void	Render();
	void	ScreenDimsChanged();

	void	RegisterHotKey(int vk, eMenuID id);
	void	RegisterCommand(int command, eMenuID id);

	bool			IsOpen( ) const { return m_bOpen; }
	virtual void	Open();
	virtual void	Close();

protected:

	virtual bool SwitchToMenu(CBaseMenu *pNewMenu);

		uint8			m_nMenuIndex;
		CBaseMenu*		m_pCurrentMenu;		// The current Menu
		CBaseMenu*		m_pLastMenu;		// The last menu displayed

		typedef std::vector<CBaseMenu *, LTAllocator<CBaseMenu *, LT_MEM_TYPE_CLIENTSHELL> > MenuArray;
		MenuArray m_MenuArray;			// Pointer to each Menu

	typedef std::vector<CMenuHotKey, LTAllocator<CMenuHotKey, LT_MEM_TYPE_CLIENTSHELL> > KeyArray;
	KeyArray m_HotKeys;			// Pointer to each Menu

//	LTVector2n		m_vMenuPos;
//	std::string		m_sSlideInSound;
//	std::string		m_sSlideOutSound;

	bool			m_bOpen;
};


class CUserMenuMgr : public CMenuMgr
{
public:

	virtual bool	Init();
	virtual void	Open();
	virtual void	Close();

	virtual bool	HandleKeyDown (int vkey, int rep);
	virtual bool	HandleKeyUp (int vkey);
	virtual bool	OnCommandOn(int command);
	virtual bool	OnCommandOff(int command);

	virtual void	OnLButtonDown(int x, int y);
	virtual void	OnLButtonUp(int x, int y);
	virtual void	OnLButtonDblClick(int x, int y);
	virtual void	OnRButtonDown(int x, int y);
	virtual void	OnRButtonUp(int x, int y);
	virtual void	OnRButtonDblClick(int x, int y);
	virtual void	OnMouseMove(int x, int y);
	virtual void	OnMouseWheel(int x, int y, int zDelta);

protected:

	virtual bool	SwitchToMenu(CBaseMenu *pNewMenu);

private:

	CMenuSystem		m_MenuSystem;
	CMenuMission	m_MenuMission;
};

class CSystemMenuMgr : public CMenuMgr
{
public:

	virtual bool	Init();
	virtual void	Open();
	virtual void	Close();

	virtual bool	HandleKeyDown (int vkey, int rep);
	virtual bool	HandleKeyUp (int vkey);
	virtual bool	OnCommandOn(int command);
	virtual bool	OnCommandOff(int command);

	virtual void	OnLButtonDown(int x, int y);
	virtual void	OnLButtonUp(int x, int y);
	virtual void	OnLButtonDblClick(int x, int y);
	virtual void	OnRButtonDown(int x, int y);
	virtual void	OnRButtonUp(int x, int y);
	virtual void	OnRButtonDblClick(int x, int y);
	virtual void	OnMouseMove(int x, int y);
	virtual void	OnMouseWheel(int x, int y, int zDelta);

private:

	CMenuEndRound	m_MenuEndRound;
};


#endif // _MENU_MGR_H_