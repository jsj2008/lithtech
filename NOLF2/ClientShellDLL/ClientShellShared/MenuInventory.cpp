// ----------------------------------------------------------------------- //
//
// MODULE  : MenuInventory.cpp
//
// PURPOSE : In-game system menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MenuInventory.h"
#include "InterfaceMgr.h"
#include "ClientResShared.h"
#include "PlayerMgr.h"
#include "ClientWeaponMgr.h"

namespace
{
	enum eLocalMenuCmds
	{
		MC_WEAPON = MC_CUSTOM,
		MC_AMMO,
	};

	uint8 nWeaponID = WMGR_INVALID_ID;

	CMenuInventory* s_pInvMenu = NULL;
}

LTBOOL CAmmoMenu::OnUp()
{
	LTBOOL bHandled = CLTGUIWindow::OnUp();

	if (bHandled && s_pInvMenu)
		s_pInvMenu->UpdateAmmoText();

	return bHandled;

}

LTBOOL CAmmoMenu::OnDown()
{
	LTBOOL bHandled = CSubMenu::OnDown();

	if (bHandled && s_pInvMenu)
		s_pInvMenu->UpdateAmmoText();

	return bHandled;
}

LTBOOL CAmmoMenu::OnMouseMove(int x, int y)
{
	LTBOOL bHandled = CSubMenu::OnMouseMove(x, y);

	if (bHandled && s_pInvMenu)
		s_pInvMenu->UpdateAmmoText();

	return bHandled;
}



LTBOOL CMenuInventory::Init()
{
	m_MenuID = MENU_ID_INVENTORY;
	s_pInvMenu = this;

	if (!CBaseMenu::Init()) return LTFALSE;

	SetTitle(IDS_TITLE_INVENTORY);

	LTIntPt size = g_pLayoutMgr->GetMenuCustomPoint(m_MenuID,"PopupSize");

	m_Popup.Init(s_Frame,s_FrameTip,size);
	LTIntPt offset = m_Indent;
	offset.y = 8;

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_TitleFontFace);
	m_Name.Create("name",LTNULL,LTNULL,pFont,m_FontSize,LTNULL);
	m_Name.SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_Name.Enable(LTFALSE);
	m_Popup.AddControl(&m_Name,offset);
	offset.y += (m_Name.GetHeight() + 4);


	m_Photo.Create(LTNULL,LTNULL,s_Frame);
	m_Photo.Enable(LTFALSE);
	m_Photo.SetTextureScale(0.75f);
	m_Popup.AddControl(&m_Photo,offset);

	int nDescriptionOffset = g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"DescriptionOffset");
	if (!nDescriptionOffset)
		nDescriptionOffset = 64;
	offset.y += nDescriptionOffset;

	pFont = g_pInterfaceResMgr->GetFont(m_FontFace);
	uint8 nDescFont = (uint8)g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"DescFontSize");
	if (!nDescFont)
		nDescFont = m_FontSize;
	m_Description.Create("description",LTNULL,LTNULL,pFont,nDescFont,LTNULL);
	m_Description.SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_Description.SetFixedWidth( size.x - (m_Indent.x * 2));
	m_Description.Enable(LTFALSE);
	m_Popup.AddControl(&m_Description,offset);

	int nAmmoOffset = g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"AmmoOffset");
	if (!nAmmoOffset)
		nAmmoOffset = 64;

	offset.y += nAmmoOffset;

	m_Ammo.Create( (size.y - offset.y) - 8);
	m_Popup.AddControl(&m_Ammo,offset);

	int nPopupPos = g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"PopupPos");
	m_Popup.SetBasePos(LTIntPt(0,nPopupPos));


	uint8 nHelpFont = (uint8)g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"HelpFontSize");
	offset = g_pLayoutMgr->GetMenuCustomPoint(m_MenuID,"HelpOffset");
	uint16 nWidth = 600 - 2* offset.x;
	if (!nHelpFont)
		nHelpFont = m_FontSize;
	m_AmmoDesc.Create("description",LTNULL,LTNULL,pFont,nHelpFont,LTNULL);
	m_AmmoDesc.SetColors(m_SelectedColor,m_SelectedColor,m_SelectedColor);
	m_AmmoDesc.SetFixedWidth(nWidth);
	m_AmmoDesc.Enable(LTFALSE);
	m_Popup.AddControl(&m_AmmoDesc,offset);


	g_pInterfaceMgr->GetMenuMgr()->RegisterCommand(COMMAND_ID_INVENTORY,MENU_ID_INVENTORY);

	return LTTRUE;
}

void CMenuInventory::Term()
{

	m_Popup.RemoveAll(LTFALSE);
	
	CBaseMenu::Term();
}


uint32 CMenuInventory::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	switch (nCommand)
	{
	case MC_WEAPON:
		{
			uint8 nID = (uint8) nParam1;
			WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nID);
			m_Name.SetString(pWeapon->szLongName);
			m_Description.SetString(LoadTempString(pWeapon->nDescriptionId));

			std::string icon = pWeapon->GetSilhouetteIcon();
			HTEXTURE hTmp = g_pInterfaceResMgr->GetTexture(icon.c_str());
			m_Photo.SetTexture(hTmp);

			CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_FontFace);
			m_Ammo.RemoveAll();
			m_nAmmo = 0;

			uint8 index = 0;
			for (uint8 i = 0; i < pWeapon->nNumAmmoIds; i++)
			{
				uint8 nAmmoID = pWeapon->aAmmoIds[i];
				if (g_pPlayerStats->GetAmmoCount(nAmmoID))
				{
					AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoID);
					char szAmmoStr[64];
					if (pWeapon->bInfiniteAmmo)
					{
						LTStrCpy(szAmmoStr,pAmmo->szLongName,sizeof(szAmmoStr));
					}
					else
					{
						sprintf(szAmmoStr,"%s : %d/%d",pAmmo->szLongName,g_pPlayerStats->GetAmmoCount(nAmmoID),pAmmo->GetMaxAmount(LTNULL));
					}

					char szFinalAmmoStr[128];
					sprintf(szFinalAmmoStr, "%s %s",LoadTempString(IDS_SELECT_AMMO_TEXT), szAmmoStr);

					CLTGUITextCtrl *pCtrl = debug_new(CLTGUITextCtrl);
					pCtrl->Create(szFinalAmmoStr,MC_AMMO,0,pFont,m_FontSize,this);
					pCtrl->SetParam1(nAmmoID);

					m_Ammo.AddControl(pCtrl);
					m_nAmmoID[index] = nAmmoID;
					if (nAmmoID == g_pPlayerStats->GetCurrentAmmo())
						m_nAmmo = index;
					++index;					

				}
			}
			m_Ammo.SetStartIndex(0);

			if (!index)
			{
//				m_Ammo.AddString("No ammo available.");
				m_nAmmoID[0] = WMGR_INVALID_ID;
				m_Ammo.SetSelection(CLTGUIListCtrl::kNoSelection);
			}
			else
				m_Ammo.SetSelection(m_nAmmo);
			m_Ammo.UpdateData(LTFALSE);

			UpdateAmmoText(true);

			m_Popup.SetSelection(m_Popup.GetIndex(&m_Ammo));

			g_pInterfaceMgr->GetMenuMgr()->ShowSubMenu(&m_Popup);
			nWeaponID = nID;
		}
		break;
	case MC_AMMO:
		{
			uint8 nAmmoID = (uint8) nParam1;
			if (nParam1 != WMGR_INVALID_ID)
			{
				// change the weapon
				ASSERT( 0 != g_pPlayerMgr );
				g_pPlayerMgr->ChangeWeapon( nWeaponID, nAmmoID );

	
				// change the state
				g_pInterfaceMgr->GetMenuMgr()->SlideOut();
			}
		}
		break;

	default:
		return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
	}
	return 1;
}


// This is called when the screen gets or loses focus
void CMenuInventory::OnFocus(LTBOOL bFocus)
{
	ClearSelection();
	if (bFocus)
	{
		uint16 nextItem = 0;
		uint16 nSel = 1;

		// get first and last weapon indices
		int nFirstWeaponCommandId =g_pWeaponMgr->GetFirstWeaponCommandId();
		int nLastWeaponCommandId = g_pWeaponMgr->GetLastWeaponCommandId();

		for (int nCmd = nFirstWeaponCommandId; nCmd <= nLastWeaponCommandId; nCmd++)
		{
			uint8 nWpn = g_pWeaponMgr->GetWeaponId(nCmd);
			if (g_pPlayerStats->HaveWeapon(nWpn))
			{
				WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(nWpn);
				if (pWeapon)
				{
					if (nextItem < m_List.GetNumControls())
					{
						CLTGUITextCtrl* pCtrl = (CLTGUITextCtrl*)m_List.GetControl(nextItem);
						if (pCtrl)
						{
							pCtrl->SetString(pWeapon->szLongName);
							pCtrl->Show(LTTRUE);
							pCtrl->SetParam1(nWpn);
							nextItem++;
						}
					}
					else
					{
						uint16 ndx = AddControl(pWeapon->szLongName,MC_WEAPON);
						CLTGUICtrl* pCtrl = m_List.GetControl(ndx);
						pCtrl->SetParam1(nWpn);
						nextItem++;

					}
					if (g_pPlayerStats->GetCurrentWeapon() == nWpn)
						nSel = nextItem-1;
				}
			}
		}

		for (uint16 nIndex = nextItem; nIndex < m_List.GetNumControls(); nIndex++)
		{
					 
			CLTGUITextCtrl* pCtrl = (CLTGUITextCtrl*)m_List.GetControl(nIndex);
			if (pCtrl)
				pCtrl->Show(LTFALSE);
		}

	


		if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		{
			SetScale(g_pInterfaceResMgr->GetXRatio());
			m_Popup.SetScale(g_pInterfaceResMgr->GetXRatio());
		}

		SetSelection(GetIndex(&m_List));

		m_List.SetSelection(nSel);
	}
}

void CMenuInventory::UpdateAmmoText(bool bForce)
{
	if (m_Ammo.GetSelectedIndex() != m_nAmmo || bForce)	
	{
		m_nAmmo = (uint8)m_Ammo.GetSelectedIndex();
		if (m_nAmmo == CLTGUIListCtrl::kNoSelection || m_nAmmoID[m_nAmmo] == WMGR_INVALID_ID)
		{
			m_AmmoDesc.SetString(" ");
			return;
		}

		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmoID[m_nAmmo]);
		m_AmmoDesc.SetString(LoadTempString(pAmmo->nDescId));

	}
}