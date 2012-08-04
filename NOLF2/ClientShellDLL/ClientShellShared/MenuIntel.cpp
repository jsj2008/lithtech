// ----------------------------------------------------------------------- //
//
// MODULE  : MenuIntel.cpp
//
// PURPOSE : In-game intel item menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MenuIntel.h"
#include "InterfaceMgr.h"
#include "ClientResShared.h"
#include "MissionMgr.h"

namespace
{
	enum eLocalMenuCmds
	{
		MC_INTEL = MC_CUSTOM,
	};
}



LTBOOL CMenuIntel::Init()
{
	m_MenuID = MENU_ID_INTEL;

	if (!CBaseMenu::Init()) return LTFALSE;

	SetTitle(IDS_TITLE_INTEL);

	g_pInterfaceMgr->GetMenuMgr()->RegisterCommand(COMMAND_ID_INTEL,MENU_ID_INTEL);

	m_PopupText.Init(false);

	return LTTRUE;
}

void CMenuIntel::Term()
{
	CBaseMenu::Term();

	m_PopupText.Term();
}

uint32 CMenuIntel::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return 1;
	}

	switch (nCommand)
	{
		case MC_INTEL:
		{
			uint16 nID = (uint16) nParam1;
			CIntelItemList *pList = g_pPlayerStats->GetIntelList();
			INTEL_ITEM* pItem = pList->Get(nID);

			if (pItem)
			{
				m_PopupText.Show(pItem->nTextId, pItem->nPopupId);

				// Disable menu now that popup is visible.

				g_pInterfaceMgr->GetMenuMgr()->EnableMenuBar(false);
			}
		}

		default:
		{
			return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
		}
	}

	return 1;
}


// This is called when the screen gets or loses focus
void CMenuIntel::OnFocus(LTBOOL bFocus)
{
	if (!m_PopupText.IsVisible())
	{
		ClearSelection();
	}

	if (bFocus)
	{
		uint16 nextItem = 0;
		CIntelItemList *pList = g_pPlayerStats->GetIntelList();
		char szTemp[512] = "";
		for (uint16 i = 0; i < pList->GetCount(); i++)
		{
			uint16 nIndex = (pList->GetCount() - i) - 1;
			INTEL_ITEM* pItem = pList->Get(nIndex);
			if (!pItem)
			{
				continue;
			}

			GetIntelName(pItem->nTextId,szTemp,sizeof(szTemp));

			CLTGUITextCtrl* pCtrl = LTNULL;
			if (nextItem < m_List.GetNumControls())
			{
				pCtrl = (CLTGUITextCtrl*)m_List.GetControl(nextItem);
				if (pCtrl)
				{
					pCtrl->SetString(szTemp);
					pCtrl->Show(LTTRUE);
					nextItem++;
				}
			}
			else
			{
				uint16 ndx = AddControl(szTemp,MC_INTEL);
				pCtrl = (CLTGUITextCtrl*)m_List.GetControl(ndx);
				nextItem++;
			}

			if (pCtrl)
			{
				pCtrl->SetParam1(nIndex);
				if (pItem->bIsIntel)
				{
					pCtrl->SetColors(0xFFFF0000,0xFFA00000,m_DisabledColor);
				}
				else
				{
					pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
				}
			}
		}

		for (uint16 nIndex = nextItem; nIndex < m_List.GetNumControls(); nIndex++)
		{
					 
			CLTGUITextCtrl* pCtrl = (CLTGUITextCtrl*)m_List.GetControl(nIndex);
			if (pCtrl)
			{
				pCtrl->Show(LTFALSE);
			}
		}

		if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		{
			SetScale(g_pInterfaceResMgr->GetXRatio());
		}

		SetSelection(GetIndex(&m_List));
	}
}


void CMenuIntel::GetIntelName(uint32 nTextId, char *pBuf, int nBufSize)
{
	LoadString(nTextId,pBuf,nBufSize);
	if (strchr(pBuf,'@'))
	{
		char* pTok = strtok(pBuf,"@");
	}
	else
	{
		char* pTok = strtok(pBuf,"\n\t,");
		int i = 24;
		int len = strlen(pBuf);
		if (len <= i) i = len;
		int j;
		for (j = 0; j < 3; j++)
			pBuf[i+j] = '.';
		pBuf[i+j] = 0;
		
	}
};



// [KLS 6/23/02] - The following were overridden to support the m_PopupText data member...

void CMenuIntel::ClosePopup()
{
	m_PopupText.Close();
	g_pInterfaceMgr->UseCursor(LTTRUE);
	g_pLTClient->ClearInput();

	// Re-enable the menu since the popup is now closed

	g_pInterfaceMgr->GetMenuMgr()->EnableMenuBar(true);
}

void CMenuIntel::Render ( )
{
	if (!IsVisible()) return;

	CBaseMenu::Render();

	// If we're showing the intel item, render the popup...

	if (m_PopupText.IsVisible())
	{
		// Make sure the cursor isn't visible...
		g_pInterfaceMgr->UseCursor(LTFALSE, LTTRUE);

		m_PopupText.Update();
		m_PopupText.Draw();
	}
}

LTBOOL CMenuIntel::OnUp ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE;
	}

	return CBaseMenu::OnUp();
}

LTBOOL CMenuIntel::OnDown ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnDown();
}

LTBOOL CMenuIntel::OnLeft ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE;
	}

	return CBaseMenu::OnLeft();
}

LTBOOL CMenuIntel::OnRight ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnRight();
}

LTBOOL CMenuIntel::OnEnter ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnEnter();
}

LTBOOL CMenuIntel::OnEscape ( )
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnEscape();
}

LTBOOL CMenuIntel::OnLButtonDown(int x, int y)
{
	if (m_PopupText.IsVisible())
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::OnLButtonDown(x, y);
}

LTBOOL CMenuIntel::HandleKeyDown(int key, int rep)
{
	// Close popup if necessary...Use OnEscape
	// to handle escape key...
	if (m_PopupText.IsVisible() && (VK_ESCAPE != key))
	{
		ClosePopup();
		return LTTRUE; 
	}

	return CBaseMenu::HandleKeyDown(key, rep);
}

