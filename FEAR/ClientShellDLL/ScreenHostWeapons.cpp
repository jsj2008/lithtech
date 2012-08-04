// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostWeapons.cpp
//
// PURPOSE : Interface screen for choosing server weapon restrictions
//
// CREATED : 11/12/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenHostWeapons.h"
#include "ScreenCommands.h"
#include "GameModeMgr.h"

static uint32  nListFontSize = 12;
static const char * sListFont = NULL;
static int32 nAvailWidth = 0;
static int32 nSelWidth = 0;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenHostWeapons::CScreenHostWeapons()
{
	m_pAvailItems	= NULL;
	m_pSelItems		= NULL;
	m_pAdd			= NULL;
	m_pRemove		= NULL;
	m_pRemoveAll    = NULL;
}

CScreenHostWeapons::~CScreenHostWeapons()
{
	Term();
}

// Build the screen
bool CScreenHostWeapons::Build()
{
	LTRect2n rcAvailRect = g_pLayoutDB->GetListRect(m_hLayout,0);
	LTRect2n rcSelRect = g_pLayoutDB->GetListRect(m_hLayout,1);
	uint32  nOffset = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize) + 4;

	LTVector2n addPos = rcAvailRect.GetTopLeft();
	addPos.y -= nOffset;
	LTVector2n removePos = rcSelRect.GetTopLeft();
	removePos.y -= nOffset;

	LTVector2n commandPos = rcAvailRect.GetTopRight();
	commandPos.x += nOffset;
	int32 nCommandWidth = rcSelRect.Left() - commandPos.x;

	nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);
	sListFont = g_pLayoutDB->GetListFont(m_hLayout,0);

	CreateTitle("IDS_TITLE_HOST_WEAPONS");

	//background frame
	CLTGUICtrl_create frameCs;
	TextureReference hFrame(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenFrameTexture));

	frameCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect);

	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame, frameCs);
	AddControl(pFrame);


	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(nCommandWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
	cs.nCommandID = CMD_ADD_LEVEL;
	cs.szHelpID = "IDS_HELP_ADD_WEAPON";
	m_pAdd = AddTextItem("IDS_HOST_ADD_WEAPON", cs, true);
	m_pAdd->SetBasePos(addPos);

	m_DefaultPos = commandPos;

	cs.nCommandID = CMD_REMOVE_ALL;
	cs.szHelpID = "IDS_HELP_REM_ALL_WPNS";
	m_pRemoveAll = AddTextItem("IDS_HOST_REMOVE_ALL_WEAPONS", cs);

	cs.nCommandID = CMD_REMOVE_LEVEL;
	cs.szHelpID = "IDS_HELP_REM_WEAPON";
	m_pRemove = AddTextItem("IDS_HOST_REMOVE_WEAPON", cs, true);
	m_pRemove->SetBasePos(removePos);

	//	int32 nListHeight = rcAvailRect.GetHeight();
	nAvailWidth = rcAvailRect.GetWidth();

	CLTGUIListCtrl_create listCs;
	listCs.rnBaseRect = rcAvailRect;
	listCs.bArrows = true;
	listCs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,0); 
	m_pAvailItems = AddList(listCs);
	m_pAvailItems->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));

	hFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
	TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
	m_pAvailItems->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));

	nSelWidth = rcSelRect.GetWidth();
	listCs.rnBaseRect = rcSelRect;
	listCs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,1); 

	m_pSelItems = AddList(listCs);
	m_pSelItems->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,1));
	hFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,1,0));
	hSelFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,1,1));
	m_pSelItems->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,1));


	// Make sure to call the base class
	if (!CBaseScreen::Build()) return false;

	UseBack(true,true);

	return true;

}



uint32  CScreenHostWeapons::OnCommand(uint32  dwCommand, uint32  dwParam1, uint32  dwParam2)
{
	switch(dwCommand)
	{
	case CMD_BACK:
		{
			m_pScreenMgr->EscapeCurrentScreen();
			break;
		}

	case CMD_ADD_LEVEL:
		{
			{
				char sWeapon[256] = "";

				if (m_pAvailItems->GetSelectedIndex() >= 0 )
				{
					CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pAvailItems->GetSelectedControl();
					if (pCtrl)
					{
						AddItemToList(pCtrl->GetParam1(),true,(eItemTypes)pCtrl->GetParam2());
						uint32  nIndex = m_pAvailItems->GetSelectedIndex();
						if (nIndex >= 0)
						{
							m_pAvailItems->ClearSelection();
							m_pAvailItems->RemoveControl(nIndex);
							uint32  numLeft = m_pAvailItems->GetNumControls();
							if (numLeft > 0)
							{
								if (nIndex >= numLeft)
									nIndex = numLeft-1;
								m_pAvailItems->SetSelection(nIndex);
							}

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
				for (uint32  i = 0; i < m_pAvailItems->GetNumControls(); i++)
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
			{
				CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pSelItems->GetSelectedControl();
				if (pCtrl)
				{
					AddItemToList(pCtrl->GetParam1(),false,(eItemTypes)pCtrl->GetParam2());
					uint32  nIndex = m_pSelItems->GetSelectedIndex();
					if (nIndex >= 0)
					{
						m_pSelItems->ClearSelection();
						m_pSelItems->RemoveControl(nIndex);
						uint32  numLeft = m_pSelItems->GetNumControls();
						if (numLeft > 0)
						{
							if (nIndex >= numLeft)
								nIndex = numLeft-1;
							m_pSelItems->SetSelection(nIndex);
						}
					}

				}
				UpdateButtons();
			}

		} break;
	case CMD_REMOVE_ALL:
		{
			if (m_pSelItems->GetNumControls())
			{
				for (uint32  i = 0; i < m_pSelItems->GetNumControls(); i++)
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
void    CScreenHostWeapons::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		GameModeMgr& gameModeMgr = GameModeMgr::Instance( );
		m_setRestrictedWeapons.clear();
		m_setRestrictedGear.clear();
		DelimitedStringToStringContainer(gameModeMgr.m_grsRestrictedWeapons.GetValue(),m_setRestrictedWeapons,",");
		DelimitedStringToStringContainer(gameModeMgr.m_grsRestrictedGear.GetValue(),m_setRestrictedGear,",");

		FillAvailList();
		LoadItemList();

		if (!m_pSelItems->GetNumControls())
		{
			MakeDefaultItemList();
		}

		UpdateButtons();
		UpdateData(false);

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

bool CScreenHostWeapons::FillAvailList()
{
	// Sanity checks...

	if (!m_pAvailItems) return(false);

	uint32  nWeapons = g_pWeaponDB->GetNumPlayerWeapons();
	for (uint32  nWpn = 0; nWpn < nWeapons; ++nWpn)
	{
		HWEAPON hWeapon = g_pWeaponDB->GetPlayerWeapon(nWpn);
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		//can't restrict dual weapons separately from theit single weapon versions, so don't show them in the list
		if (g_pWeaponDB->GetBool(hWpnData,WDB_ALL_bCanServerRestrict) && NULL == g_pWeaponDB->GetRecordLink(hWpnData,WDB_WEAPON_rSingleWeapon))
		{
			std::string sItem = g_pWeaponDB->GetRecordName(hWeapon);
			
			StringSet::iterator iter = m_setRestrictedWeapons.find(sItem);
			if (iter == m_setRestrictedWeapons.end())
				AddItemToList(g_pWeaponDB->GetRecordIndex(hWeapon),false,kWeaponType);
		}
	}


	uint32  nNumGear = g_pWeaponDB->GetNumGear();
	for (uint32  nGear = 0; nGear < nNumGear; ++nGear)
	{
		HGEAR hGear = g_pWeaponDB->GetGearRecord(nGear);
		if (g_pWeaponDB->GetBool(hGear,WDB_ALL_bCanServerRestrict))
		{
			std::string sItem = g_pWeaponDB->GetRecordName(hGear);

			StringSet::iterator iter = m_setRestrictedGear.find(sItem);
			if (iter == m_setRestrictedGear.end())
				AddItemToList(nGear,false,kGearType);
		}
	}

	return (true);
}

void CScreenHostWeapons::LoadItemList()
{
	// Sanity checks...

	if (!m_pSelItems) return;

	StringSet::iterator iter = m_setRestrictedWeapons.begin();
	while( iter != m_setRestrictedWeapons.end( ))
	{
		std::string const& sValue = *iter;
		iter++;

		HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord(sValue.c_str());
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		if (g_pWeaponDB->GetBool(hWpnData,WDB_ALL_bCanServerRestrict) && NULL == g_pWeaponDB->GetRecordLink(hWpnData,WDB_WEAPON_rSingleWeapon))
		{
			AddItemToList(g_pWeaponDB->GetRecordIndex(hWeapon),true,kWeaponType);
		}
	}

	iter = m_setRestrictedGear.begin();
	while( iter != m_setRestrictedGear.end( ))
	{
		std::string const& sValue = *iter;
		iter++;

		HGEAR hGear = g_pWeaponDB->GetGearRecord(sValue.c_str());
		if (g_pWeaponDB->GetBool(hGear,WDB_ALL_bCanServerRestrict))
		{
			AddItemToList(g_pWeaponDB->GetRecordIndex(hGear),true,kGearType);
		}
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

	GameModeMgr& gameModeMgr = GameModeMgr::Instance( );

	m_setRestrictedWeapons.clear();
	m_setRestrictedGear.clear();

	for (uint32  n = 0; n < m_pSelItems->GetNumControls(); n++)
	{
		CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pSelItems->GetControl(n);
		if (pCtrl)
		{
			switch (pCtrl->GetParam2())
			{
			case kWeaponType:
				{
					uint32  nWeaponId = pCtrl->GetParam1();
					HWEAPON hWpn = g_pWeaponDB->GetWeaponRecord(nWeaponId);
					HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWpn, !USE_AI_DATA);
					if (g_pWeaponDB->GetBool(hWpnData,WDB_ALL_bCanServerRestrict))
					{
						std::string sItem = g_pWeaponDB->GetRecordName(hWpn);
						m_setRestrictedWeapons.insert(sItem);
					}
				} break;
			case kGearType:
				{
					uint32  nGearId = pCtrl->GetParam1();
					HWEAPON hGear = g_pWeaponDB->GetGearRecord(nGearId);
					if (g_pWeaponDB->GetBool(hGear,WDB_ALL_bCanServerRestrict))
					{
						std::string sItem = g_pWeaponDB->GetRecordName(hGear);
						m_setRestrictedGear.insert(sItem);
					}
				} break;
			}
		}
	}

	std::string sList;
	StringContainerToDelimitedString(m_setRestrictedWeapons,sList,",");
	gameModeMgr.m_grsRestrictedWeapons = sList.c_str();

	StringContainerToDelimitedString(m_setRestrictedGear,sList,",");
	gameModeMgr.m_grsRestrictedGear = sList.c_str();
	GameModeMgr::Instance().WriteToOptionsFile( g_pProfileMgr->GetCurrentProfile( )->m_sServerOptionsFile.c_str( ));

}			


void CScreenHostWeapons::UpdateButtons()
{
	m_pRemoveAll->Enable(m_pSelItems->GetNumControls() > 0);

}

void CScreenHostWeapons::AddItemToList(uint32  nId, bool bSelected, eItemTypes eType)
{
	// Sanity checks...

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMin.Init();
	cs.rnBaseRect.m_vMax = LTVector2n(nAvailWidth,nListFontSize);

	CLTGUIListCtrl* pList = m_pAvailItems;
	cs.nCommandID = CMD_ADD_LEVEL;
	cs.szHelpID = "IDS_HELP_ADD_WEAPON";

	if (bSelected)
	{
		pList = m_pSelItems;
		cs.nCommandID = CMD_REMOVE_LEVEL;
		cs.szHelpID = "IDS_HELP_REM_WEAPON";
	}

	if (!pList) return;


	switch (eType)
	{
	case kWeaponType:
		{
			// Add the weapon to the list...
			HWEAPON hWpn = g_pWeaponDB->GetWeaponRecord(nId);
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWpn, !USE_AI_DATA);

			CLTGUITextCtrl *pCtrl = NULL;
			pCtrl = CreateTextItem( LoadString(g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_nShortNameId)), cs, false, sListFont, nListFontSize);
			pCtrl->SetParam1(nId);
			pCtrl->SetParam2(eType);
			pList->AddControl(pCtrl);
		} break;
	case kGearType:
		{
			// Add the gear item to the list...
			HGEAR hGear = g_pWeaponDB->GetGearRecord(nId);

			CLTGUITextCtrl *pCtrl = NULL;
			pCtrl = CreateTextItem( LoadString(g_pWeaponDB->GetString(hGear,WDB_GEAR_nNameId)), cs, false, sListFont, nListFontSize);
			pCtrl->SetParam1(nId);
			pCtrl->SetParam2(eType);
			pList->AddControl(pCtrl);
		} break;
	}

}
