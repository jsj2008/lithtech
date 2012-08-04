// ----------------------------------------------------------------------- //
//
// MODULE  : MenuKeys.cpp
//
// PURPOSE : In-game key item menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MenuKeys.h"
#include "InterfaceMgr.h"
#include "ClientResShared.h"
#include "KeyMgr.h"

namespace
{
	enum eLocalMenuCmds
	{
		MC_KEY = MC_CUSTOM,
		MC_POPUP,
	};

	uint8 nKeyID = WMGR_INVALID_ID;
}



LTBOOL CMenuKeys::Init()
{
	m_MenuID = MENU_ID_KEYS;

	if (!CBaseMenu::Init()) return LTFALSE;

	SetTitle(IDS_TITLE_KEYS);

	LTIntPt	popupSize = g_pLayoutMgr->GetMenuCustomPoint(m_MenuID,"PopupSize");
	m_Popup.Init(s_Frame,s_FrameTip,popupSize);
	LTIntPt offset = m_Indent;
	offset.y = 8;

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_TitleFontFace);
	m_Name.Create("name",LTNULL,LTNULL,pFont,m_TitleFontSize,LTNULL);
	m_Name.SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_Name.Enable(LTFALSE);
	m_Popup.AddControl(&m_Name,offset);
	offset.y += (m_Name.GetHeight() + 4);


	m_Image.Create(LTNULL,LTNULL,s_Frame);
	m_Image.Enable(LTFALSE);
	m_Popup.AddControl(&m_Image,offset);
	offset.y += 64;

	pFont = g_pInterfaceResMgr->GetFont(m_FontFace);
	m_Description.Create("description",MC_POPUP,LTNULL,pFont,m_FontSize,this);
	m_Description.SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_Description.SetFixedWidth( s_Size.x - (m_Indent.x * 2));
	m_Description.Enable(LTTRUE);
	m_Popup.AddControl(&m_Description,offset);
	offset.y += 64;

	m_Popup.SetSelection(m_Popup.GetIndex(&m_Description));
	m_Popup.SetBasePos(LTIntPt(320,160));

	g_pInterfaceMgr->GetMenuMgr()->RegisterCommand(COMMAND_ID_KEYS,MENU_ID_KEYS);

	return LTTRUE;
}

void CMenuKeys::Term()
{

	m_Popup.RemoveAll(LTFALSE);
	
	CBaseMenu::Term();
}


uint32 CMenuKeys::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	switch (nCommand)
	{
	case MC_KEY:
		{
			uint8 nID = (uint8) nParam1;
			KEY* pKey = g_pKeyMgr->GetKey(nID);
			m_Name.SetString(LoadTempString(pKey->nNameId));
			m_Description.SetString(LoadTempString(pKey->nDescriptionId));
			HTEXTURE hTmp = g_pInterfaceResMgr->GetTexture(pKey->szImage);
			m_Image.SetTexture(hTmp);

			g_pInterfaceMgr->GetMenuMgr()->ShowSubMenu(&m_Popup);
			nKeyID = nID;
		}
		break;
	case MC_POPUP:
		{
			g_pInterfaceMgr->GetMenuMgr()->HideSubMenu(true);
		}
		break;

	default:
		return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
	}
	return 1;
}


// This is called when the screen gets or loses focus
void CMenuKeys::OnFocus(LTBOOL bFocus)
{
	ClearSelection();
	if (bFocus)
	{
		uint16 nextItem = 0;
		uint8 nTemp;
		for (uint8 i = 0; i < (uint8)g_pKeyMgr->GetNumKeys(); i++)
		{
			if (g_pPlayerStats->GetKeys()->Have(i,nTemp))
			{
				KEY* pKey = g_pKeyMgr->GetKey(i);
				if (pKey)
				{
					if (nextItem < m_List.GetNumControls())
					{
						CLTGUITextCtrl* pCtrl = (CLTGUITextCtrl*)m_List.GetControl(nextItem);
						if (pCtrl)
						{
							pCtrl->SetString(LoadTempString(pKey->nNameId));
							pCtrl->Show(LTTRUE);
							pCtrl->SetParam1(i);
							nextItem++;
						}
					}
					else
					{
						uint16 ndx = AddControl(pKey->nNameId,MC_KEY);
						CLTGUICtrl* pCtrl = m_List.GetControl(ndx);
						pCtrl->SetParam1(i);
						nextItem++;
					}
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

	}
}
