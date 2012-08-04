// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostWeapons.cpp
//
// PURPOSE : Interface screen for choosing levels for a hosted game
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostWeapons.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "ClientRes.h"
#include "VarTrack.h"
#include "NetDefs.h"
#include "profileMgr.h"
#include "clientmultiplayermgr.h"
#include "WinUtil.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;



namespace
{
	uint8 nListFontSize = 12;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHostWeapons::CScreenHostWeapons()
{
    m_pAvailItems	= LTNULL;
    m_pSelItems   = LTNULL;
	m_pAdd			= LTNULL;
	m_pRemove		= LTNULL;
	m_pAddAll		= LTNULL;
	m_pRemoveAll    = LTNULL;
}

CScreenHostWeapons::~CScreenHostWeapons()
{
	Term();
}

// Build the screen
LTBOOL CScreenHostWeapons::Build()
{
	LTIntPt addPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"AddPos");
	LTIntPt removePos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"RemovePos");
	LTIntPt commandPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"CommandPos");
	uint8 nFontSize = g_pLayoutMgr->GetScreenFontSize((eScreenID)m_nScreenID);

	if (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID,"ListFontSize"))
		nListFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"ListFontSize");

	CreateTitle(IDS_TITLE_HOST_WEAPONS);

	m_pAdd = AddTextItem(IDS_HOST_ADD_WEAPON, CMD_ADD_LEVEL, IDS_HELP_ADD_WEAPONS, addPos);
	addPos.y -= (nFontSize + m_nItemSpacing);
	AddTextItem(IDS_ALLOWED_WEAPONS,NULL,NULL,addPos,LTTRUE);
	
//	m_pAddAll = AddTextItem(IDS_HOST_ADD_ALL, CMD_ADD_ALL, 0 /*IDS_HELP_ADD_ALL_WPNS*/, commandPos);
	m_pRemoveAll = AddTextItem(IDS_HOST_REMOVE_ALL, CMD_REMOVE_ALL, IDS_HELP_REM_ALL_WPNS, commandPos);

	m_pRemove = AddTextItem(IDS_HOST_REMOVE_WEAPON, CMD_REMOVE_LEVEL, IDS_HELP_REM_WEAPONS, removePos);
	removePos.y -= (nFontSize + m_nItemSpacing);
	AddTextItem(IDS_DISABLED_WEAPONS,NULL,NULL,removePos,LTTRUE);

	
	LTRect rcAvailRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"AvailRect");
	int nListHeight = (rcAvailRect.bottom - rcAvailRect.top);
	int nListWidth = (rcAvailRect.right - rcAvailRect.left) - 32;

	m_pAvailItems = AddList(LTIntPt(rcAvailRect.left,rcAvailRect.top),nListHeight,LTTRUE,nListWidth);
	m_pAvailItems->SetIndent(LTIntPt(5,5));
	m_pAvailItems->SetFrameWidth(2);
	m_pAvailItems->Enable(LTFALSE);

	LTRect rcSelRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"SelectRect");
	nListHeight = (rcSelRect.bottom - rcSelRect.top);
	nListWidth = (rcSelRect.right - rcSelRect.left) - 32;

	m_pSelItems = AddList(LTIntPt(rcSelRect.left,rcSelRect.top),nListHeight,LTTRUE,nListWidth);
	m_pSelItems->SetIndent(LTIntPt(5,5));
	m_pSelItems->SetFrameWidth(2);
	m_pSelItems->Enable(LTFALSE);

 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);

	

	return LTTRUE;

}

void CScreenHostWeapons::Escape()
{
	if (m_pAvailItems->IsEnabled())
	{
		m_pAvailItems->Enable(LTFALSE);
		m_pAdd->Enable(LTTRUE);
		SetSelection(GetIndex(m_pAdd));
	}
	else if (m_pSelItems->IsEnabled())
	{
		m_pSelItems->Enable(LTFALSE);
		m_pRemove->Enable(LTTRUE);
		SetSelection(GetIndex(m_pRemove));
	}
	else
	{
		CBaseScreen::Escape();
	}
}



uint32 CScreenHostWeapons::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_BACK:
		{
			m_pAvailItems->Enable(LTFALSE);
			m_pSelItems->Enable(LTFALSE);
			m_pScreenMgr->EscapeCurrentScreen();
			break;
		}

	case CMD_ADD_LEVEL:
		{
			if (!m_pAvailItems->IsEnabled() && m_pAvailItems->GetNumControls())
			{
				m_pSelItems->Enable(LTFALSE);
				m_pAvailItems->Enable(LTTRUE);
				m_pAdd->Enable(LTFALSE);
				SetSelection(GetIndex(m_pAvailItems));
				m_pAvailItems->SetSelection(0);
			}
			else
			{
				char sWeapon[256] = "";

				if (m_pAvailItems->GetSelectedIndex() >= 0 )
				{
					CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pAvailItems->GetSelectedControl();
					if (pCtrl)
					{
						AddItemToList(pCtrl->GetParam1(),true,(eItemTypes)pCtrl->GetParam2());
						int nIndex = m_pAvailItems->GetSelectedIndex();
						if (nIndex >= 0)
						{
							m_pAvailItems->RemoveControl(nIndex);
						}
						

					}
				}

				UpdateButtons();
			}
		} break;
	case CMD_ADD_ALL:
		{
			if (m_pAvailItems->GetNumControls())
			{
				for (int i = 0; i < m_pAvailItems->GetNumControls(); i++)
				{
					CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pAvailItems->GetControl(i);
					if (pCtrl)
					{
						AddItemToList(pCtrl->GetParam1(),true,(eItemTypes)pCtrl->GetParam2());
					}
				}
			}
			m_pAvailItems->RemoveAll();
			m_pSelItems->ClearSelection();
			UpdateButtons();
		} break;
	case CMD_REMOVE_LEVEL:
		{
			if (!m_pSelItems->IsEnabled() && m_pSelItems->GetNumControls())
			{
				m_pAvailItems->Enable(LTFALSE);
				m_pSelItems->Enable(LTTRUE);
				m_pRemove->Enable(LTFALSE);
				SetSelection(GetIndex(m_pSelItems));
				m_pSelItems->SetSelection(0);
			}
			else
			{
				CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pSelItems->GetSelectedControl();
				if (pCtrl)
				{
					AddItemToList(pCtrl->GetParam1(),false,(eItemTypes)pCtrl->GetParam2());
					int nIndex = m_pSelItems->GetSelectedIndex();
					if (nIndex >= 0)
					{
						m_pSelItems->RemoveControl(nIndex);
					}

				}
				UpdateButtons();
			}

		} break;
	case CMD_REMOVE_ALL:
		{
			if (m_pSelItems->GetNumControls())
			{
				for (int i = 0; i < m_pSelItems->GetNumControls(); i++)
				{
					CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pSelItems->GetControl(i);
					if (pCtrl)
					{
						AddItemToList(pCtrl->GetParam1(),false,(eItemTypes)pCtrl->GetParam2());
					}
				}
			}
			m_pSelItems->RemoveAll();
			m_pAvailItems->ClearSelection();
			UpdateButtons();
		} break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenHostWeapons::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
		m_pAvailItems->Enable(LTFALSE);
		m_pAdd->Enable(LTTRUE);
		m_pSelItems->Enable(LTFALSE);
		m_pRemove->Enable(LTTRUE);

		FillAvailList();

		LoadItemList();

		if (!m_pSelItems->GetNumControls())
		{
			MakeDefaultItemList();
		}

		UpdateButtons();
        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();

		SaveItemList();

		m_pAvailItems->RemoveAll();
		m_pSelItems->RemoveAll();
	}
	CBaseScreen::OnFocus(bFocus);

}

LTBOOL CScreenHostWeapons::FillAvailList()
{
	// Sanity checks...

    if (!m_pAvailItems) return(LTFALSE);

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	// get first and last weapon indices
	uint8 nFirstWeaponCommandId = g_pWeaponMgr->GetFirstWeaponCommandId();
	uint8 nLastWeaponCommandId = g_pWeaponMgr->GetLastWeaponCommandId();
	for (uint8 nWpnCommandId = nFirstWeaponCommandId; nWpnCommandId <= nLastWeaponCommandId; nWpnCommandId++)
	{
		uint8 nWpn = g_pWeaponMgr->GetWeaponId( nWpnCommandId);
		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( nWpn );
		if (pWeapon && pWeapon->bCanServerRestrict)
		{
			std::string sItem = pWeapon->szName;
			StringSet::iterator iter = pProfile->m_ServerGameOptions.m_setRestrictedWeapons.find(sItem);

			if (iter == pProfile->m_ServerGameOptions.m_setRestrictedWeapons.end())
				AddItemToList(pWeapon->nId,false,kWeaponType);
		}
	}

	for (uint8 nId = 0; nId <= g_pWeaponMgr->GetNumAmmoIds(); nId++)
	{
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( nId );
		if (pAmmo && pAmmo->bCanServerRestrict)
		{
			std::string sItem = pAmmo->szName;
			StringSet::iterator iter = pProfile->m_ServerGameOptions.m_setRestrictedAmmo.find(sItem);

			if (iter == pProfile->m_ServerGameOptions.m_setRestrictedAmmo.end())
				AddItemToList(pAmmo->nId,false,kAmmoType);
		}
	}

	for (uint8 nId = 0; nId <= g_pWeaponMgr->GetNumGearIds(); nId++)
	{
		GEAR const *pGear = g_pWeaponMgr->GetGear( nId );
		if (pGear && pGear->bCanServerRestrict)
		{
			std::string sItem = pGear->szName;
			StringSet::iterator iter = pProfile->m_ServerGameOptions.m_setRestrictedGear.find(sItem);

			if (iter == pProfile->m_ServerGameOptions.m_setRestrictedGear.end())
				AddItemToList(pGear->nId,false,kGearType);
		}
	}


    return (LTTRUE);
}

void CScreenHostWeapons::LoadItemList()
{
	// Sanity checks...

	if (!m_pSelItems) return;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	StringSet::iterator iter = pProfile->m_ServerGameOptions.m_setRestrictedWeapons.begin( );
	while( iter != pProfile->m_ServerGameOptions.m_setRestrictedWeapons.end( ))
	{
		std::string const& sValue = *iter;
		iter++;

		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( sValue.c_str( ));

		if(pWeapon && pWeapon->bCanServerRestrict )
			AddItemToList(pWeapon->nId,true,kWeaponType);
	}

	iter = pProfile->m_ServerGameOptions.m_setRestrictedAmmo.begin( );
	while( iter != pProfile->m_ServerGameOptions.m_setRestrictedAmmo.end( ))
	{
		std::string const& sValue = *iter;
		iter++;

		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( sValue.c_str( ));

		if(pAmmo && pAmmo->bCanServerRestrict )
			AddItemToList(pAmmo->nId,true,kAmmoType);
	}

	iter = pProfile->m_ServerGameOptions.m_setRestrictedGear.begin( );
	while( iter != pProfile->m_ServerGameOptions.m_setRestrictedGear.end( ))
	{
		std::string const& sValue = *iter;
		iter++;

		GEAR const *pGear = g_pWeaponMgr->GetGear( sValue.c_str( ));

		if(pGear && pGear->bCanServerRestrict )
			AddItemToList(pGear->nId,true,kGearType);
	}
}

void CScreenHostWeapons::MakeDefaultItemList()
{
	// Sanity checks...

	if (!m_pSelItems) return;

	m_pSelItems->RemoveAll();

}

void CScreenHostWeapons::SaveItemList()
{
	// Sanity checks...

	if (!m_pSelItems) return;

	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	pProfile->m_ServerGameOptions.m_setRestrictedWeapons.clear();
	pProfile->m_ServerGameOptions.m_setRestrictedAmmo.clear();
	pProfile->m_ServerGameOptions.m_setRestrictedGear.clear();

	for (int n = 0; n < m_pSelItems->GetNumControls(); n++)
	{
		CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pSelItems->GetControl(n);
		if (pCtrl)
		{
			switch (pCtrl->GetParam2())
			{
			case kWeaponType:
				{
					int nWeaponId = pCtrl->GetParam1();
					WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( nWeaponId );
					if (pWeapon && pWeapon->bCanServerRestrict)
					{
						std::string sItem = pWeapon->szName;
						pProfile->m_ServerGameOptions.m_setRestrictedWeapons.insert(sItem);
					}
				} break;
			case kAmmoType:
				{
					int nAmmoId = pCtrl->GetParam1();
					AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( nAmmoId );
					if (pAmmo && pAmmo->bCanServerRestrict)
					{
						std::string sItem = pAmmo->szName;
						pProfile->m_ServerGameOptions.m_setRestrictedAmmo.insert(sItem);
					}
				} break;
			case kGearType:
				{
					int nGearId = pCtrl->GetParam1();
					GEAR const *pGear = g_pWeaponMgr->GetGear( nGearId );
					if (pGear && pGear->bCanServerRestrict)
					{
						std::string sItem = pGear->szName;
						pProfile->m_ServerGameOptions.m_setRestrictedGear.insert(sItem);
					}
				} break;
			}
		}
	}

	pProfile->Save();
}			
		



void CScreenHostWeapons::UpdateButtons()
{
	//if we're removing items, and our list is empty
	if (m_pSelItems->IsEnabled() && !m_pSelItems->GetNumControls())
	{
		m_pSelItems->Enable(LTFALSE);
		SetSelection(GetIndex(m_pAdd));
	}
	else
		m_pRemove->Enable(m_pSelItems->GetNumControls());

	if (m_pAvailItems->IsEnabled() && !m_pAvailItems->GetNumControls())
	{
		m_pAvailItems->Enable(LTFALSE);
		SetSelection(GetIndex(m_pRemove));
	}
	else
		m_pAdd->Enable( m_pAvailItems->GetNumControls());

	m_pRemoveAll->Enable(m_pSelItems->GetNumControls() > 0);
		
}

void CScreenHostWeapons::AddItemToList(int nId, bool bSelected, eItemTypes eType)
{
	// Sanity checks...

	CLTGUIListCtrl* pList = m_pAvailItems;
	uint32 nCmd = CMD_ADD_LEVEL;
	uint32 nHelp = IDS_HELP_ADD_WEAPON;

	if (bSelected)
	{
		pList = m_pSelItems;
		nCmd = CMD_REMOVE_LEVEL;
		nHelp = IDS_HELP_REM_WEAPON;
	}

	if (!pList) return;


	switch (eType)
	{
	case kWeaponType:
		{
			// Add the weapon to the list...
			WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( nId );
			if (pWeapon)
			{
				
				CLTGUITextCtrl *pCtrl = NULL;
				pCtrl = CreateTextItem(pWeapon->szShortName,nCmd,nHelp);
				pCtrl->SetFont(LTNULL,nListFontSize);
				pCtrl->SetParam1(nId);
				pCtrl->SetParam2(eType);
				pList->AddControl(pCtrl);
			}
			else
			{
				ASSERT(!"Invalid Weapon id");
			}
		} break;
	case kAmmoType:
		{
			// Add the ammo to the list...
			AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( nId );
			if (pAmmo)
			{
				
				CLTGUITextCtrl *pCtrl = NULL;
				pCtrl = CreateTextItem(pAmmo->szShortName,nCmd,nHelp);
				pCtrl->SetFont(LTNULL,nListFontSize);
				pCtrl->SetParam1(nId);
				pCtrl->SetParam2(eType);
				pList->AddControl(pCtrl);
			}
			else
			{
				ASSERT(!"Invalid Ammo id");
			}
		} break;
	case kGearType:
		{
			// Add the Gear to the list...
			GEAR const *pGear = g_pWeaponMgr->GetGear( nId );
			if (pGear)
			{
				
				CLTGUITextCtrl *pCtrl = NULL;
				pCtrl = CreateTextItem(LoadTempString(pGear->nNameId),nCmd,nHelp);
				pCtrl->SetFont(LTNULL,nListFontSize);
				pCtrl->SetParam1(nId);
				pCtrl->SetParam2(eType);
				pList->AddControl(pCtrl);
			}
			else
			{
				ASSERT(!"Invalid Gear id");
			}
		} break;
	}

}

