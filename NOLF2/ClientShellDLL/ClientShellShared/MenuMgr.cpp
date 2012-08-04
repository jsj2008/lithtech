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
extern CGameClientShell* g_pGameClientShell;


static char s_aMenuName[MENU_ID_UNASSIGNED+1][32] =
{
#define INCLUDE_AS_STRING
#include "MenuEnum.h"
#undef INCLUDE_AS_STRING

};

namespace 
{
	bool bHK = false;
	uint32 g_nSelectColor = argbWhite;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuMgr::CMenuMgr()
{
	m_nMenuIndex = 0xFF;
	m_pLastMenu = LTNULL;
	m_pCurrentMenu = LTNULL;
	m_pSubMenu = LTNULL;

	m_fSlideInTime = 0.5f;
	m_fSlideOutTime = 0.5f;

}

CMenuMgr::~CMenuMgr()
{

}


LTBOOL CMenuMgr::Init()
{
	//build menu array
	m_MenuArray.reserve(MENU_ID_UNASSIGNED);

	//add menus here
	m_MenuArray.push_back(&m_MenuSystem);
	m_MenuArray.push_back(&m_MenuMission);
	m_MenuArray.push_back(&m_MenuInventory);
	m_MenuArray.push_back(&m_MenuKeys);
	m_MenuArray.push_back(&m_MenuIntel);
	m_MenuArray.push_back(&m_MenuPlayer);

	//init menus
	MenuArray::iterator iter = m_MenuArray.begin();
	while (iter != m_MenuArray.end())
	{
		if (!(*iter)->Init())
			return LTFALSE;
		iter++;
	}

	m_fSlideInTime	= g_pLayoutMgr->GetMenuSlideInTime();
	m_fSlideOutTime = g_pLayoutMgr->GetMenuSlideOutTime();
	m_nMenuPos		= g_pLayoutMgr->GetMenuPosition();

	char szTemp[128];
	char *pTag = "Menu";

	g_pLayoutMgr->GetString(pTag,"SlideInSound",szTemp,sizeof(szTemp));
	m_sSlideInSound = szTemp;

	g_pLayoutMgr->GetString(pTag,"SlideOutSound",szTemp,sizeof(szTemp));
	m_sSlideOutSound = szTemp;

	g_pLayoutMgr->GetString(pTag,"Bar",szTemp,sizeof(szTemp));
	HTEXTURE hBar = g_pInterfaceResMgr->GetTexture(szTemp);

	g_pLayoutMgr->GetString(pTag,"BarTip",szTemp,sizeof(szTemp));
	HTEXTURE hBarTip = g_pInterfaceResMgr->GetTexture(szTemp);



	LTIntPt size = g_pLayoutMgr->GetPoint(pTag,"BarSize");
	uint8 fontFace = (uint8)g_pLayoutMgr->GetInt(pTag,"BarFont");
	uint8 fontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"BarFontSize");
	m_nBarPos = g_pLayoutMgr->GetInt(pTag,"BarPosition");
	int nBarSpacing = g_pLayoutMgr->GetInt(pTag,"BarSpacing");
	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"BarSelectColor");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	g_nSelectColor = SET_ARGB(0xFF,nR,nG,nB);


	m_MenuBar.Init(hBar,hBarTip,size);
	m_MenuBar.SetBasePos(LTIntPt(0,m_nBarPos));
	
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(fontFace);
	
	LTIntPt offset(nBarSpacing,(size.y-fontSize)/2);
	for (uint8 i =0; i < m_MenuArray.size(); i++)
	{
		CLTGUITextCtrl *pCtrl = debug_new(CLTGUITextCtrl);
		CBaseMenu *pMenu = m_MenuArray[i];
		pCtrl->Create(pMenu->GetTitle(),i,NULL,pFont,fontSize,&m_MenuBar);
		pCtrl->SetColors(g_nSelectColor,argbBlack,argbWhite);
		pCtrl->SetParam1(pMenu->GetMenuID());
		m_MenuBar.AddControl(pCtrl,offset);
		offset.x += nBarSpacing + pCtrl->GetWidth();
	}

    return LTTRUE;
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

	//term menus
	KeyArray::iterator kiter = m_HotKeys.begin();
	while (kiter != m_HotKeys.end())
	{
		debug_delete(*kiter);
		kiter++;
	}

	//clear menu array
	m_HotKeys.clear();

}


LTBOOL CMenuMgr::HandleKeyDown (int vkey, int rep)
{

	//check for hot keys

	//if the key is escape, switch to the system menu, only if there is no open menu
	//otherwise (i.e. either not Escape or there is no open menu) see if we hit a hot key and switch to the menu
	if (vkey != VK_ESCAPE || !m_pCurrentMenu)
	{
		LTBOOL found = LTFALSE;
		KeyArray::iterator iter = m_HotKeys.begin();
		while (iter != m_HotKeys.end() && !found)
		{
			found = (vkey == (*iter)->m_vk);
			if (!found)
				iter++;
		}
		if (found && (!m_pCurrentMenu || (*iter)->m_id != m_pCurrentMenu->GetMenuID()) )
		{
			g_pInterfaceMgr->SwitchToMenu((*iter)->m_id);
			bHK = true;
			return LTTRUE;
		}
	}


	//if we're here, we didn't hit a hot key, so if we don't have a menu, we don't want the key
	if (!m_pCurrentMenu)
	{
		return LTFALSE;
	}

	//see who wants the key
	CLTGUIWindow *pMenu = LTNULL;
	if (m_pSubMenu)
	{
		pMenu = m_pSubMenu;
	}
	else
		pMenu = m_pCurrentMenu;

	//try to map the key to a standard action...
	LTBOOL handled = LTFALSE;
	while (pMenu && !handled)
	{
		switch (vkey)
		{
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
				LTBOOL handled = LTFALSE;
				handled = pMenu->OnRight();
				if (!handled && pMenu == m_pCurrentMenu)
				{
					handled = NextMenu();
					g_pInterfaceMgr->RequestInterfaceSound(IS_PAGE);
				}
				break;
			}
		case VK_UP:
			{
				if (pMenu->OnUp())
				{
					g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
					handled = LTTRUE;
				}
				break;
			}
		case VK_DOWN:
			{
				if (pMenu->OnDown())
				{
					g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
					handled = LTTRUE;
				}
				break;
			}
		case VK_RETURN:
			{
				if (pMenu->OnEnter())
				{
					g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
					handled = LTTRUE;
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
			if (pMenu == m_pSubMenu)
				pMenu = m_pCurrentMenu;
			else
				pMenu = NULL;
		}
		
	}

	return handled;
}

void CMenuMgr::HandleKeyUp (int vkey)
{
	//check for hot keys
	if (m_pCurrentMenu)
	{
		LTBOOL found = LTFALSE;
		KeyArray::iterator iter = m_HotKeys.begin();
		while (iter != m_HotKeys.end() && !found)
		{
			found = (vkey == (*iter)->m_vk);
			if (!found)
				iter++;
		}
		if (found && (*iter)->m_id == m_pCurrentMenu->GetMenuID())
		{
			if (!bHK)
				SlideOut();
			bHK = false;
			return;
		}


	}

	if (vkey == VK_ESCAPE)
	{
		CLTGUIWindow *pMenu = LTNULL;
		if (m_pSubMenu)
		{
			HideSubMenu(true);
		}
		else if (m_pCurrentMenu && !m_pCurrentMenu->OnEscape())
		{
			SlideOut();
		}

	}
}

		



LTBOOL CMenuMgr::SetCurrentMenu(eMenuID menuID)
{
	MenuArray::iterator iter = m_MenuArray.begin();
	uint8 index = 0;
	while (iter != m_MenuArray.end() && (*iter)->GetMenuID() != menuID )
	{
		index++;
		iter++;
	}

	if (iter != m_MenuArray.end() && (*iter)->IsEnabled())
	{
		m_nMenuIndex = index;
		SwitchToMenu(*iter);
		return LTTRUE;
	}
	return LTFALSE;
}

LTBOOL CMenuMgr::SetCurrentMenu(uint8 index)
{
	if (index < m_MenuArray.size() && m_MenuArray[index]->IsEnabled() )
	{
		m_nMenuIndex = index;
		SwitchToMenu(m_MenuArray[index]);
		return LTTRUE;
	}
	return LTFALSE;
}

LTBOOL CMenuMgr::NextMenu()
{
	if (m_MenuArray.size() == 0) return LTFALSE;

	uint8 index = m_nMenuIndex + 1;
	if (index >= m_MenuArray.size())
				index = 0;		

	while (index != m_nMenuIndex)
	{
		if (SetCurrentMenu(index))
			return LTTRUE;
		else
		{
			index++;
			if (index >= m_MenuArray.size())
				index = 0;
		}
	}


	return LTFALSE;
}

LTBOOL CMenuMgr::PreviousMenu()
{
	if (m_MenuArray.size() == 0) return LTFALSE;

	uint8 index = m_nMenuIndex;
	if (index == 0)
		index = m_MenuArray.size();
	index--;

	while (index != m_nMenuIndex)
	{
		if (SetCurrentMenu(index))
			return LTTRUE;
		else
		{
			if (index == 0)
				index = m_MenuArray.size();
			index--;
		}
	}
	return LTFALSE;
}

void CMenuMgr::ExitMenus()
{
	if (m_pSubMenu) m_pSubMenu = LTNULL;
	if (m_pCurrentMenu)
	{
        m_pCurrentMenu->OnFocus(LTFALSE);
		m_pLastMenu = m_pCurrentMenu;
	}
	m_pCurrentMenu = LTNULL;
}



void CMenuMgr::Update()
{
	if (!m_pCurrentMenu) return;
	if (m_MenuSlide.IsStarted())
	{
		m_pCurrentMenu->SetBasePos(m_MenuSlide.GetCurrentPos());
		if (m_MenuSlide.IsDone())
		{
			m_MenuSlide.Stop();
			if (m_MenuSlide.IsSlidingOut())
			{
				g_pInterfaceMgr->ChangeState(GS_PLAYING);
				m_SubSlide.Stop();
				m_BarSlide.Stop();
			}
		}

	}

	if (m_SubSlide.IsStarted() && m_pSubMenu)
	{
		m_pSubMenu->SetBasePos(m_SubSlide.GetCurrentPos());
		if (m_SubSlide.IsDone())
		{
			m_SubSlide.Stop();
			if (m_SubSlide.IsSlidingOut())
				m_pSubMenu = NULL;
		}

	}

	if (m_BarSlide.IsStarted())
	{
		m_MenuBar.SetBasePos(m_BarSlide.GetCurrentPos());
		if (m_BarSlide.IsDone())
		{
			m_BarSlide.Stop();
		}

	}



}

void CMenuMgr::Render()
{
	if (m_pCurrentMenu)
	{
		m_pCurrentMenu->Render();
		m_MenuBar.Render();
	}
	if (m_pSubMenu)
	{
		m_pSubMenu->Render();
	}
	
}


void CMenuMgr::SlideIn()
{
	if (m_MenuSlide.IsStarted()) return;
	ASSERT(m_pCurrentMenu);
	if (!m_pCurrentMenu) return;
	
	LTIntPt startPos, endPos;
	startPos.x = m_nMenuPos;
	startPos.y = -((3 * m_pCurrentMenu->GetHeight()) / 2 + m_pCurrentMenu->GetWidth()) / 2 ;

	endPos.x = m_nMenuPos;
	endPos.y = 0;

	g_pClientSoundMgr->PlayInterfaceSound(m_sSlideInSound.c_str());
	m_MenuSlide.Start(startPos,endPos,m_fSlideInTime, false);

	startPos.x = -((3 * m_MenuBar.GetHeight()) / 2 + m_MenuBar.GetWidth()) / 2 ;
	startPos.y = m_nBarPos;

	endPos.x = 0;
	endPos.y = m_nBarPos;

	m_BarSlide.Start(startPos,endPos,m_fSlideInTime, false);


}

void CMenuMgr::SlideOut()
{
	if (m_MenuSlide.IsStarted()) return;
	if (m_pSubMenu)
	{
		HideSubMenu(true);
	}

	LTIntPt startPos, endPos;
	endPos.x = m_nMenuPos;
	endPos.y = -((3 * m_pCurrentMenu->GetHeight()) / 2 + m_pCurrentMenu->GetWidth()) / 2;

	startPos.x = m_nMenuPos;
	startPos.y = 0;

	g_pClientSoundMgr->PlayInterfaceSound(m_sSlideOutSound.c_str());
	m_MenuSlide.Start(startPos,endPos,m_fSlideInTime,true);

	startPos.x = 0;
	startPos.y = m_nBarPos;

	endPos.x = -((3 * m_MenuBar.GetHeight()) / 2 + m_MenuBar.GetWidth()) / 2 ;
	endPos.y = m_nBarPos;

	m_BarSlide.Start(startPos,endPos,m_fSlideInTime,true);

}



void CMenuMgr::ScreenDimsChanged()
{
	if (m_pCurrentMenu)
	{
		m_pCurrentMenu->SetScale(g_pInterfaceResMgr->GetXRatio());
	}
	if (m_pSubMenu)
	{
		m_pSubMenu->SetScale(g_pInterfaceResMgr->GetXRatio());
	}
	m_MenuBar.SetScale(g_pInterfaceResMgr->GetXRatio());
}

void CMenuMgr::SwitchToMenu(CBaseMenu *pNewMenu)
{
	if (!pNewMenu || !pNewMenu->IsEnabled())
		return;

	if (m_pSubMenu)
	{
		HideSubMenu(true);
	}
	// Tell the old menu that it is losing focus
	if (m_pCurrentMenu)
	{
        m_pCurrentMenu->OnFocus(LTFALSE);
	}

	m_pLastMenu = m_pCurrentMenu;
	m_pCurrentMenu=pNewMenu;

	// Tell the new menu that it is gaining focus
	if (pNewMenu)
	{
		LTIntPt tmp = pNewMenu->GetBasePos();
		tmp.y = 0;
		pNewMenu->SetBasePos(tmp);
        pNewMenu->OnFocus(LTTRUE);

		m_MenuBar.SetScale(g_pInterfaceResMgr->GetXRatio());
		for (uint8 i =0; i < m_MenuArray.size(); i++)
		{
			CLTGUICtrl* pCtrl = m_MenuBar.GetControl(i);
			pCtrl->Enable(i != m_nMenuIndex);
		}


	}
}

// Mouse messages
void	CMenuMgr::OnLButtonDown(int x, int y)
{
	if (m_pSubMenu && m_pSubMenu->OnLButtonDown(x,y)) return;
	if (m_pCurrentMenu)
	{
		if (!m_pCurrentMenu->OnLButtonDown(x,y))
			m_MenuBar.OnLButtonDown(x,y);
	}
}

void	CMenuMgr::OnLButtonUp(int x, int y)
{
	if (m_pSubMenu && m_pSubMenu->OnLButtonUp(x,y))	return;
	if (m_pCurrentMenu)
	{
		if (!m_pCurrentMenu->OnLButtonUp(x,y))
			m_MenuBar.OnLButtonUp(x,y);
	}
}

void	CMenuMgr::OnLButtonDblClick(int x, int y)
{
	if (m_pSubMenu &&m_pSubMenu->OnLButtonDblClick(x,y)) return;
	if (m_pCurrentMenu)
		m_pCurrentMenu->OnLButtonDblClick(x,y);
}

void	CMenuMgr::OnRButtonDown(int x, int y)
{
	if (m_pSubMenu && m_pSubMenu->OnRButtonDown(x,y)) return;
	if (m_pCurrentMenu)
		m_pCurrentMenu->OnRButtonDown(x,y);
}

void CMenuMgr::OnRButtonUp(int x, int y)
{
	CLTGUIWindow *pMenu = LTNULL;
	if (m_pSubMenu)
	{
		HideSubMenu(true);
	}
	else if (m_pCurrentMenu && !m_pCurrentMenu->OnEscape())
	{
		SlideOut();
	}
}

void	CMenuMgr::OnRButtonDblClick(int x, int y)
{
	if (m_pSubMenu && m_pSubMenu->OnRButtonDblClick(x,y)) return;
	if (m_pCurrentMenu)
		m_pCurrentMenu->OnRButtonDblClick(x,y);
}

void	CMenuMgr::OnMouseMove(int x, int y)
{
	if (m_pSubMenu && m_pSubMenu->OnMouseMove(x,y))
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
		return;
	}
	if (m_pCurrentMenu)
	{
		m_pCurrentMenu->OnMouseMove(x,y);
		if (m_MenuBar.IsOnMe(x,y))
		{
			if (m_MenuBar.OnMouseMove(x,y))
				g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
		}
		else
			m_MenuBar.ClearSelection();
	}
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
	return LTNULL;
}

eMenuID CMenuMgr::GetCurrentMenuID()
{
	if (m_pCurrentMenu)
		return m_pCurrentMenu->GetMenuID();
	else
		return MENU_ID_NONE;
}


void CMenuMgr::RegisterHotKey(int vk, eMenuID id)
{
	LTBOOL found = LTFALSE;
	KeyArray::iterator iter = m_HotKeys.begin();
	while (iter != m_HotKeys.end() && !found)
	{
		found = (vk == (*iter)->m_vk);
		if (!found)
			iter++;
	}
	if (found) 
	{
		(*iter)->m_id = id;
	}
	else
	{
		CMenuHotKey* newKey = debug_new3(CMenuHotKey,vk,COMMAND_ID_UNASSIGNED,id);
		m_HotKeys.push_back(newKey);
	}

}
void CMenuMgr::RegisterCommand(int command, eMenuID id)
{
	LTBOOL found = LTFALSE;
	KeyArray::iterator iter = m_HotKeys.begin();
	while (iter != m_HotKeys.end() && !found)
	{
		found = (command == (*iter)->m_command);
		if (!found)
			iter++;
	}
	if (found) 
	{
		(*iter)->m_id = id;
	}
	else
	{
		CMenuHotKey* newKey = debug_new3(CMenuHotKey,-1,command,id);
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

LTBOOL CMenuMgr::OnCommandOn(int command)
{
	if (m_pSubMenu) return LTFALSE;

	if (GS_PLAYING != g_pInterfaceMgr->GetGameState()) return LTFALSE;

	

	LTBOOL found = LTFALSE;
	KeyArray::iterator iter = m_HotKeys.begin();
	while (iter != m_HotKeys.end() && !found)
	{
		found = (command == (*iter)->m_command);
		if (!found)
			iter++;
	}


	if (found)
	{
		if (g_pInterfaceMgr->SwitchToMenu((*iter)->m_id))
		{
			bHK = true;
			return LTTRUE;
		}
	}

	return LTFALSE;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMenuMgr::OnCommandOff()
//
//	PURPOSE:	Handle command off
//
// ----------------------------------------------------------------------- //

LTBOOL CMenuMgr::OnCommandOff(int command)
{
	if (m_pSubMenu || !m_pCurrentMenu ) return LTFALSE;


	if (GS_MENU != g_pInterfaceMgr->GetGameState()) return LTFALSE;

	

	LTBOOL found = LTFALSE;
	KeyArray::iterator iter = m_HotKeys.begin();
	while (iter != m_HotKeys.end() && !found)
	{
		found = (command == (*iter)->m_command);
		if (!found)
			iter++;
	}


	//if we found a menu that uses the command as a hotkey...
	if (found)
	{
		//close the current menu if the hotkey belongs to it..
		if ((*iter)->m_id == m_pCurrentMenu->GetMenuID())
		{
			if (!bHK)
				SlideOut();
			bHK = false;
			return LTTRUE;
		}
		else
		{
			//switch to the appropriate menu
			g_pInterfaceMgr->SwitchToMenu((*iter)->m_id);
			bHK = false;
			return LTTRUE;

		}
	}

	return LTFALSE;

}



void CMenuMgr::ShowSubMenu(CSubMenu* pSubMenu) 
{
	if (pSubMenu)
	{
		if (m_pSubMenu)
		{
			g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
		}
		else
		{

			if (m_SubSlide.IsStarted()) return;

			LTIntPt startPos, endPos;

			endPos = pSubMenu->GetBasePos();
			endPos.x = 0;

			startPos = endPos;
			startPos.x = -((3 * pSubMenu->GetWidth()) / 2 + pSubMenu->GetHeight()) / 2 ;

			g_pClientSoundMgr->PlayInterfaceSound(m_sSlideInSound.c_str());
			m_SubSlide.Start(startPos,endPos,m_fSlideInTime, false);
		}

	}
	m_pSubMenu = pSubMenu;
	if (m_pSubMenu)
		m_pSubMenu->SetScale(g_pInterfaceResMgr->GetXRatio());


}

void CMenuMgr::HideSubMenu(bool bSlideOut)
{
	if (!bSlideOut)
	{
		m_pSubMenu = NULL;
		m_SubSlide.Stop();
		return;
	}
	if (m_pSubMenu)
	{
		m_pSubMenu->SetScale(g_pInterfaceResMgr->GetXRatio());

		if (m_SubSlide.IsStarted()) return;

		LTIntPt startPos, endPos;

		endPos = m_pSubMenu->GetBasePos();
		endPos.x = -((3 * m_pSubMenu->GetWidth()) / 2 + m_pSubMenu->GetHeight()) / 2 ;

		startPos = endPos;
		startPos.x = 0;
	
		

		if (!m_MenuSlide.IsStarted())
			g_pClientSoundMgr->PlayInterfaceSound(m_sSlideOutSound.c_str());

		m_SubSlide.Start(startPos,endPos,m_fSlideOutTime, true);
	}

}


void CMenuMgr::EnableMenuBar(bool bEnable/*=true*/, uint32 nMenuFlags/*=MB_ALL*/)
{
	// Determine the state of each control based on the flags passed...

	uint32 MBCtrlFlags[MB_NUM_CTRLS] =
	{
		MB_SYSTEM,
		MB_MISSION,
		MB_INVENTORY,
		MB_KEYS,
		MB_INTEL,
		MB_PLAYER
	};

	if (!IsCoopMultiplayerGameType())
	{
		nMenuFlags &= ~(MB_MISSION & MB_KEYS & MB_INTEL & MB_PLAYER);
	}

	for (int i=0; i < MB_NUM_CTRLS; i++)
	{
		CLTGUICtrl* pCtrl = m_MenuBar.GetControl(i);
		if (pCtrl)
		{
			if (MBCtrlFlags[i] & nMenuFlags)
			{
				pCtrl->Enable(bEnable);
			}
			else
			{
				pCtrl->Enable(!bEnable);
			}

			// Okay this is pretty much a hack since we use the disabled state to
			// really specify a selected control (see CMenuMgr::SwitchToMenu()), but in the
			// case of Enable = true we really want the menu bar to be disabled, we'll handle
			// this here by changing the color of the disabled state...

			if (pCtrl->IsDisabled())
			{
				pCtrl->SetColors(g_nSelectColor,argbBlack,argbGray);
			}
			else
			{
				// Control is enabled, so we'll set it's "disabled" color to white...

				pCtrl->SetColors(g_nSelectColor,argbBlack,argbWhite);

				// We need to disable the current menu control here using the new
				// color (i.e., it will appear white signifying it is actually
				// the current control)...
				
				if (i == m_nMenuIndex)
				{
					pCtrl->Enable(false);
				}
			}
		}
	}
}



void CMenuMgr::EnableMenus()
{
	bool bSPMenus = IsCoopMultiplayerGameType();

	m_MenuMission.Enable(bSPMenus);
	m_MenuKeys.Enable(bSPMenus);
	m_MenuIntel.Enable(bSPMenus);
	m_MenuPlayer.Enable(bSPMenus);

}






//////////////////////////////////////////////////////////////////////
// DataStructure to handle sliding in of menu elements
//////////////////////////////////////////////////////////////////////


CMenuSlide::CMenuSlide()
{
	m_fStartTime = 0.0f;
	m_fDuration = 0.0f;
	m_bSlidingOut = false;
}

void CMenuSlide::Start(LTIntPt startPos,LTIntPt endPos,float fDuration, bool bSlidingOut)
{
	m_startPos = startPos;
	m_offset.x = endPos.x - startPos.x;
	m_offset.y = endPos.y - startPos.y;

	m_fStartTime = g_pLTClient->GetTime();
	m_fDuration = fDuration;

	m_bSlidingOut = bSlidingOut;
}

LTIntPt CMenuSlide::GetCurrentPos()
{
	LTIntPt curPos;
	float fPercent = 1.0f;
	
	if (m_fDuration > 0.0f)
	{
		float fElapsed = g_pLTClient->GetTime() - m_fStartTime;
		if (fElapsed < m_fDuration)
			fPercent = fElapsed / m_fDuration;
	}

	
	curPos.x = m_startPos.x + (int)(fPercent * (float)m_offset.x);
	curPos.y = m_startPos.y + (int)(fPercent * (float)m_offset.y);

	return curPos;
}

bool CMenuSlide::IsDone()
{
	if (m_fDuration <= 0.0f) return true;

	float fElapsed = g_pLTClient->GetTime() - m_fStartTime;

	return (fElapsed >= m_fDuration);

}


uint32 CMenuBar::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	if (nParam1 >= 0 && nParam1 < MENU_ID_UNASSIGNED)
	{
		g_pInterfaceMgr->GetMenuMgr()->SetCurrentMenu((eMenuID)nParam1);
		g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);
		return 1;
	}
	else
	{
		g_pInterfaceMgr->GetMenuMgr()->SlideOut();
		return 1;
	}
	return 0;
}