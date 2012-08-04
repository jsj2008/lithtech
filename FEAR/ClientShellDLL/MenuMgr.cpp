// ----------------------------------------------------------------------- //
//
// MODULE  : MenuMgr.cpp
//
// PURPOSE : Manager for in-game menus
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "MenuMgr.h"
#include "GameClientShell.h"
#include "CommandIDs.h"
#include "ltguimgr.h"
#include "BindMgr.h"
extern CGameClientShell* g_pGameClientShell;


static char s_aMenuName[MENU_ID_UNASSIGNED+1][32] =
{
#define INCLUDE_AS_STRING
#include "MenuEnum.h"
#undef INCLUDE_AS_STRING

};

namespace 
{
	// Set to true when we are processing a hotkey.  Turned off when hotkey is let up.  Prevents a hotkey
	// from opening and closing a menu in one key press.
	bool bHK = false;
	uint32 g_nSelectColor = argbWhite;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuMgr::CMenuMgr()
{
	m_bOpen = false;
}

CMenuMgr::~CMenuMgr()
{
	Term( );
}


bool CMenuMgr::Init()
{
	//init menus
	MenuArray::iterator iter = m_MenuArray.begin();
	while (iter != m_MenuArray.end())
	{
		if (!(*iter)->Init( *this ))
			return false;
		iter++;
	}

	m_bOpen = false;

	return true;
}

void CMenuMgr::Term()
{
	//term menus
	MenuArray::iterator iter = m_MenuArray.begin();
	while (iter != m_MenuArray.end())
	{
		(*iter)->Term();
		iter++;
	}

	//clear menu array
	m_MenuArray.clear();

	//clear hotkeys
	m_HotKeys.clear();

	m_bOpen = false;

}


bool CMenuMgr::HandleKeyDown(int vkey, int rep)
{
	// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	//check for hot keys

	//if the key is escape, switch to the system menu, only if there is no open menu
	//otherwise (i.e. either not Escape or there is no open menu) see if we hit a hot key and switch to the menu
	if (vkey != VK_ESCAPE || !m_pCurrentMenu)
	{
		bool found = false;
		KeyArray::iterator iter = m_HotKeys.begin();
		while (iter != m_HotKeys.end() && !found)
		{
			found = (vkey == (*iter).m_vk);
			if (!found)
				iter++;
		}
		if (found && (!m_pCurrentMenu || (*iter).m_id != m_pCurrentMenu->GetMenuID()) )
		{
			SetCurrentMenu((*iter).m_id);
			bHK = true;
			return true;
		}
	}


	//if we're here, we didn't hit a hot key, so if we don't have a menu, we don't want the key
	if (!m_pCurrentMenu)
	{
		return false;
	}

	//see who wants the key
	CLTGUIWindow *pMenu = m_pCurrentMenu;

	//try to map the key to a standard action...
	bool handled = false;
	while (pMenu && !handled)
	{
		switch (vkey)
		{
/*
		case VK_LEFT:
			{
				handled = pMenu->OnLeft();
				if (!handled && pMenu == m_pCurrentMenu)
				{
					handled = PreviousMenu();
					g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
				}
				break;
			}
		case VK_RIGHT:
			{
				bool handled = false;
				handled = pMenu->OnRight();
				if (!handled && pMenu == m_pCurrentMenu)
				{
					handled = NextMenu();
					g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
				}
				break;
		}
		*/
		case VK_UP:
			{
				if (pMenu->OnUp())
				{
					g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
					handled = true;
				}
				break;
			}
		case VK_DOWN:
			{
				if (pMenu->OnDown())
				{
					g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
					handled = true;
				}
				break;
			}
		case VK_RETURN:
			{
				if (pMenu->OnEnter())
				{
					g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
					handled = true;
				}
				break;
			}
		default:
			{
				//no standard action, see if the menu wants to handle it directly
				handled = m_pCurrentMenu->HandleKeyDown(vkey,rep);
				break;
			}

		}
		if (!handled)
		{
			pMenu = NULL;
		}
	}

	return handled;

#else // PLATFORM_XENON
	return false;
#endif // PLATFORM_XENON

}

bool CMenuMgr::HandleKeyUp (int vkey)
{
	//check for hot keys
	if (m_pCurrentMenu)
	{
		bool found = false;
		KeyArray::iterator iter = m_HotKeys.begin();
		while (iter != m_HotKeys.end() && !found)
		{
			found = (vkey == (*iter).m_vk);
			if (!found)
				iter++;
		}
		if (found && (*iter).m_id == m_pCurrentMenu->GetMenuID())
		{
			if (!bHK)
				Close();
			bHK = false;
			return true;
		}

		m_pCurrentMenu->HandleKeyUp(vkey);
	}

	// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	if (vkey == VK_ESCAPE)
	{
		if (m_pCurrentMenu && !m_pCurrentMenu->OnEscape())
		{
			Close();
			return true;
		}
	}

#endif // !PLATFORM_XENON

	return false;
}

bool CMenuMgr::SetCurrentMenu(eMenuID menuID)
{
	MenuArray::iterator iter = m_MenuArray.begin();
	uint8 index = 0;
	while (iter != m_MenuArray.end() && (*iter)->GetMenuID() != menuID )
	{
		index++;
		iter++;
	}

	return SetCurrentMenu( index );
}

bool CMenuMgr::SetCurrentMenu(uint8 index)
{
	if (index < m_MenuArray.size() && m_MenuArray[index]->IsEnabled() )
	{
		m_nMenuIndex = index;
		SwitchToMenu(m_MenuArray[index]);
		return true;
	}
	return false;
}

bool CMenuMgr::NextMenu()
{
	if (m_MenuArray.size() == 0) return false;

	uint8 index = m_nMenuIndex + 1;
	if (index >= m_MenuArray.size())
		index = 0;

	while (index != m_nMenuIndex)
	{
		if (SetCurrentMenu(index))
			return true;
		else
		{
			index++;
			if (index >= m_MenuArray.size())
				index = 0;
		}
	}


	return false;
}

bool CMenuMgr::PreviousMenu()
{
	if (m_MenuArray.size() == 0) return false;

	uint8 index = m_nMenuIndex;
	if (index == 0)
	{
		ASSERT( m_MenuArray.size() == ( uint8 )m_MenuArray.size());
		index = ( uint8 )m_MenuArray.size();
	}
	index--;

	while (index != m_nMenuIndex)
	{
		if (SetCurrentMenu(index))
			return true;
		else
		{
			if (index == 0)
			{
				ASSERT( m_MenuArray.size() == ( uint8 )m_MenuArray.size());
				index = ( uint8 )m_MenuArray.size();
			}
			index--;
		}
	}
	return false;
}

void CMenuMgr::ExitMenus()
{
	if (m_pCurrentMenu)
	{
		m_pCurrentMenu->OnFocus(false);
		m_pLastMenu = m_pCurrentMenu;
	}
	m_pCurrentMenu = NULL;
}





void CMenuMgr::Render()
{
	if( !m_bOpen )
		return;

	if (m_pCurrentMenu)
	{
		m_pCurrentMenu->Render();
	}
}


void CMenuMgr::Open()
{
	ASSERT(m_pCurrentMenu);
	if (!m_pCurrentMenu) return;

	m_pCurrentMenu->SetBasePos(m_pCurrentMenu->GetDefaultPos( ));
	g_pClientSoundMgr->PlayInterfaceSound(g_pLayoutDB->GetString(m_pCurrentMenu->GetMenuRecord( ),"SlideInSound"));

	m_bOpen = true;
}

void CMenuMgr::Close()
{
	if( !m_pCurrentMenu )
	{
		LTERROR( "Invalid menu" );
		return;
	}

	LTVector2n endPos;
	endPos.x = m_pCurrentMenu->GetBasePos( ).x;
	endPos.y = -(int)m_pCurrentMenu->GetBaseHeight();
	m_pCurrentMenu->SetBasePos(endPos);

	g_pClientSoundMgr->PlayInterfaceSound( g_pLayoutDB->GetString(m_pCurrentMenu->GetMenuRecord( ),"SlideOutSound"));

	m_bOpen = false;
}


void CMenuMgr::ScreenDimsChanged()
{
	MenuArray::iterator iter = m_MenuArray.begin();
	while (iter != m_MenuArray.end())
	{
		(*iter)->SetScale(g_pInterfaceResMgr->GetScreenScale());
		iter++;
	}
}

bool CMenuMgr::SwitchToMenu(CBaseMenu *pNewMenu)
{
	if (!pNewMenu || !pNewMenu->IsEnabled())
		return false;

	// Tell the old menu that it is losing focus
	if (m_pCurrentMenu)
	{
		m_pCurrentMenu->OnFocus(false);
	}

	m_pLastMenu = m_pCurrentMenu;
	m_pCurrentMenu=pNewMenu;

	// Tell the new menu that it is gaining focus
	if (pNewMenu)
	{
		pNewMenu->OnFocus(true);
	}

	return true;
}

// Mouse messages
void	CMenuMgr::OnLButtonDown(int x, int y)
{
	if (m_pCurrentMenu)
		m_pCurrentMenu->OnLButtonDown(x,y);			
}

void	CMenuMgr::OnLButtonUp(int x, int y)
{
	if (m_pCurrentMenu)
		m_pCurrentMenu->OnLButtonUp(x,y);
}

void	CMenuMgr::OnLButtonDblClick(int x, int y)
{
	if (m_pCurrentMenu)
		m_pCurrentMenu->OnLButtonDblClick(x,y);
}

void	CMenuMgr::OnRButtonDown(int x, int y)
{
	if (m_pCurrentMenu)
		m_pCurrentMenu->OnRButtonDown(x,y);
}

void CMenuMgr::OnRButtonUp(int x, int y)
{
	// To exit menus on right-click, re-enable this code.
	
	//if (m_pCurrentMenu && !m_pCurrentMenu->OnEscape())
	//{
	//	Close();
	//}
}

void	CMenuMgr::OnRButtonDblClick(int x, int y)
{
	if (m_pCurrentMenu)
		m_pCurrentMenu->OnRButtonDblClick(x,y);
}

void	CMenuMgr::OnMouseMove(int x, int y)
{
	if (m_pCurrentMenu)
		m_pCurrentMenu->OnMouseMove(x,y);
}

void	CMenuMgr::OnMouseWheel(int x, int y, int zDelta)
{
	if (m_pCurrentMenu)
		m_pCurrentMenu->OnMouseWheel(x,y,zDelta);
}

const char* CMenuMgr::GetMenuName(eMenuID id)
{
	return s_aMenuName[id];
}

CBaseMenu*	CMenuMgr::GetMenu(eMenuID menuID)
{
	MenuArray::iterator iter = m_MenuArray.begin();
	while (iter != m_MenuArray.end() && (*iter)->GetMenuID() != menuID )
	{
		iter++;
	}

	if (iter != m_MenuArray.end())
	{
		return (*iter);
	}
	return NULL;
}

void CMenuMgr::RegisterHotKey(int vk, eMenuID id)
{
	bool found = false;
	KeyArray::iterator iter = m_HotKeys.begin();
	while (iter != m_HotKeys.end() && !found)
	{
		found = (vk == (*iter).m_vk);
		if (!found)
			iter++;
	}
	if (found) 
	{
		(*iter).m_id = id;
	}
	else
	{
		CMenuHotKey newKey(vk,COMMAND_ID_UNASSIGNED,id);
		m_HotKeys.push_back(newKey);
	}

}
void CMenuMgr::RegisterCommand(int command, eMenuID id)
{
	bool found = false;
	KeyArray::iterator iter = m_HotKeys.begin();
	while (iter != m_HotKeys.end() && !found)
	{
		found = (command == (*iter).m_command);
		if (!found)
			iter++;
	}
	if (found) 
	{
		(*iter).m_id = id;
	}
	else
	{
		CMenuHotKey newKey(-1,command,id);
		m_HotKeys.push_back(newKey);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenuMgr::OnCommandOn()
//
//	PURPOSE:	Handle command on
//
// ----------------------------------------------------------------------- //

bool CMenuMgr::OnCommandOn(int command)
{
	bool found = false;
	KeyArray::iterator iter = m_HotKeys.begin();
	while (iter != m_HotKeys.end() && !found)
	{
		found = (command == (*iter).m_command);
		if (!found)
			iter++;
	}


	if (found)
	{
		if (SetCurrentMenu((*iter).m_id))
		{
			bHK = true;
			return true;
		}
	}

	return false;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenuMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off
//
// ----------------------------------------------------------------------- //

bool CMenuMgr::OnCommandOff(int command)
{
	if (!m_pCurrentMenu ) return false;

	bool found = false;
	KeyArray::iterator iter = m_HotKeys.begin();
	while (iter != m_HotKeys.end() && !found)
	{
		found = (command == (*iter).m_command);
		if (!found)
			iter++;
	}


	//if we found a menu that uses the command as a hotkey...
	if (found)
	{
		//close the current menu if the hotkey belongs to it..
		if ((*iter).m_id == m_pCurrentMenu->GetMenuID())
		{
			if (!bHK)
				Close();
			bHK = false;
			return true;
		}
		else
		{
			//switch to the appropriate menu
			SetCurrentMenu((*iter).m_id);
			bHK = false;
			return true;

		}
	}

	return false;

}

bool CUserMenuMgr::Init()
{
	//add menus here
	m_MenuArray.push_back(&m_MenuSystem);
	m_MenuArray.push_back(&m_MenuMission);

	return CMenuMgr::Init( );
}

bool CUserMenuMgr::SwitchToMenu(CBaseMenu *pNewMenu)
{
	if( !CMenuMgr::SwitchToMenu( pNewMenu ))
		return false;

	if( g_pInterfaceMgr->GetGameState() != GS_MENU )
	{
		if( !g_pInterfaceMgr->ChangeState( GS_MENU )) 
			return false;
		Open();
	}

	return true;
}

void CUserMenuMgr::Open()
{
	CMenuMgr::Open();
}

void CUserMenuMgr::Close()
{
	CMenuMgr::Close();

	g_pInterfaceMgr->ChangeState(GS_PLAYING);
}

bool CUserMenuMgr::HandleKeyDown (int vkey, int rep)
{
	GameState eGameState = g_pInterfaceMgr->GetGameState();
	if( eGameState != GS_MENU && eGameState != GS_PLAYING )
		return false;

	//can't go the menus if dead or dying in single player
	if (!g_pPlayerMgr->IsPlayerAlive() && !IsMultiplayerGameClient( ))
	{
		return false;
	}

	return CMenuMgr::HandleKeyDown(vkey, rep);
}

bool CUserMenuMgr::HandleKeyUp (int vkey)
{
	GameState eGameState = g_pInterfaceMgr->GetGameState();
	if( eGameState != GS_MENU )
		return false;

	return CMenuMgr::HandleKeyUp(vkey);
}

bool CUserMenuMgr::OnCommandOn(int command)
{
	GameState eGameState = g_pInterfaceMgr->GetGameState( );
	if( eGameState != GS_MENU && eGameState != GS_PLAYING )
		return false;

	if( CMenuMgr::OnCommandOn( command ) || eGameState == GS_MENU )
		return true;

	return false;
}

bool CUserMenuMgr::OnCommandOff(int command)
{
	if (GS_MENU != g_pInterfaceMgr->GetGameState()) 
		return false;

	return CMenuMgr::OnCommandOff( command );
}

void CUserMenuMgr::OnLButtonDown(int x, int y) 
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_MENU )
		CMenuMgr::OnLButtonDown(x,y);
}
void CUserMenuMgr::OnLButtonUp(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_MENU )
		CMenuMgr::OnLButtonUp(x,y);
}
void CUserMenuMgr::OnLButtonDblClick(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_MENU )
		CMenuMgr::OnLButtonDblClick(x,y);
}
void CUserMenuMgr::OnRButtonDown(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_MENU )
		CMenuMgr::OnRButtonDown(x,y);
}
void CUserMenuMgr::OnRButtonUp(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_MENU )
		CMenuMgr::OnRButtonUp(x,y);
}
void CUserMenuMgr::OnRButtonDblClick(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_MENU )
		CMenuMgr::OnRButtonDblClick(x,y);
}
void CUserMenuMgr::OnMouseMove(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_MENU )
		CMenuMgr::OnMouseMove(x,y);
}
void CUserMenuMgr::OnMouseWheel(int x, int y, int zDelta)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_MENU )
		CMenuMgr::OnMouseWheel(x,y,zDelta);
}


bool CSystemMenuMgr::Init()
{
	//add menus here
	m_MenuArray.push_back(&m_MenuEndRound);

	return CMenuMgr::Init( );
}

void CSystemMenuMgr::Open()
{
	CMenuMgr::Open();

	if (!m_pCurrentMenu)
	{
		return;
	}

	m_pCurrentMenu->Show( true );
}

void CSystemMenuMgr::Close()
{
	CMenuMgr::Close();
}

bool CSystemMenuMgr::HandleKeyDown (int vkey, int rep)
{
	GameState eGameState = g_pInterfaceMgr->GetGameState();
	if( eGameState != GS_PLAYING )
		return false;

	return CMenuMgr::HandleKeyDown(vkey, rep);
}

bool CSystemMenuMgr::HandleKeyUp (int vkey)
{
	GameState eGameState = g_pInterfaceMgr->GetGameState();
	if( eGameState != GS_PLAYING )
		return false;

	if (vkey == VK_ESCAPE)
	{
		return false;
	}

	return CMenuMgr::HandleKeyUp(vkey);
}

bool CSystemMenuMgr::OnCommandOn(int command)
{
	GameState eGameState = g_pInterfaceMgr->GetGameState( );
	if( eGameState != GS_PLAYING && eGameState != GS_PLAYING )
		return false;

	if( CMenuMgr::OnCommandOn( command ))
		return true;

	return false;
}

bool CSystemMenuMgr::OnCommandOff(int command)
{
	if (GS_PLAYING != g_pInterfaceMgr->GetGameState()) 
		return false;

	return CMenuMgr::OnCommandOff( command );
}

void CSystemMenuMgr::OnLButtonDown(int x, int y) 
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_PLAYING )
		CMenuMgr::OnLButtonDown(x,y);
}
void CSystemMenuMgr::OnLButtonUp(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_PLAYING )
		CMenuMgr::OnLButtonUp(x,y);
}
void CSystemMenuMgr::OnLButtonDblClick(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_PLAYING )
		CMenuMgr::OnLButtonDblClick(x,y);
}
void CSystemMenuMgr::OnRButtonDown(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_PLAYING )
		CMenuMgr::OnRButtonDown(x,y);
}
void CSystemMenuMgr::OnRButtonUp(int x, int y)
{ 
	// Don't call up, because the default behavior is to close the menu, which is not
	// allowed for the system menu.
}

void CSystemMenuMgr::OnRButtonDblClick(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_PLAYING )
		CMenuMgr::OnRButtonDblClick(x,y);
}
void CSystemMenuMgr::OnMouseMove(int x, int y)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_PLAYING )
		CMenuMgr::OnMouseMove(x,y);
}
void CSystemMenuMgr::OnMouseWheel(int x, int y, int zDelta)
{ 
	if( g_pInterfaceMgr->GetGameState() == GS_PLAYING )
		CMenuMgr::OnMouseWheel(x,y,zDelta);
}


