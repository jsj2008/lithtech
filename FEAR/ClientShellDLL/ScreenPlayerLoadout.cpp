// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayerLoadout.cpp
//
// PURPOSE : Interface screen for team selection
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPlayerLoadout.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "WeaponDB.h"
#include "ClientConnectionMgr.h"
#include "MissionMgr.h"
#include "GameModeMgr.h"
#include "sys/win/mpstrconv.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


namespace
{
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPlayerLoadout::CScreenPlayerLoadout()
{
	m_pLevelName = NULL;
	m_pList = NULL;

}

CScreenPlayerLoadout::~CScreenPlayerLoadout()
{

}

// Build the screen
bool CScreenPlayerLoadout::Build()
{
	m_AutoSelectTimer.SetEngineTimer( RealTimeTimer::Instance( ));

	CreateTitle("IDS_TITLE_PLAYER_LOADOUT");

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),8+m_TitleSize);

	cs.nCommandID = 0;
	cs.szHelpID = 0;
	m_pLevelName = AddTextItem( L"", cs, true, m_TitleFont.c_str(), m_TitleSize );
	m_pLevelName->SetColor( g_pLayoutDB->GetColor(m_hLayout, LDB_ScreenAddColor, 0 ) );

	CLTGUIListCtrl_create listCs;
	listCs.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,0);
	listCs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,0); 
	m_pList = AddList(listCs);
	int32 nWidth = listCs.rnBaseRect.GetWidth() - 16;
	m_pList->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));

	TextureReference hFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
	TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
	m_pList->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));



	LTRect2n rnActualImage = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,0);
	LTRect2n rnCroppedImage = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,1);

	CLTGUITextureButton_create bcs;
	bcs.rnBaseRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth(),rnCroppedImage.GetHeight());
	bcs.rnImageRect.m_vMax = rnActualImage.m_vMax;
	bcs.rnTextRect.m_vMin.x = rnCroppedImage.GetWidth() + 8;
	bcs.rnTextRect.m_vMax = LTVector2n(m_ScreenRect.GetWidth() - rnCroppedImage.GetWidth(),rnCroppedImage.GetHeight());

	for (uint8 nLoadout = 0; nLoadout < g_pWeaponDB->GetNumLoadouts(); ++nLoadout)
	{
		std::wstring wsItems;

		HRECORD hLoadout = g_pWeaponDB->GetLoadout(nLoadout);
		HATTRIBUTE hWpnAtt = g_pLTDatabase->GetAttribute(hLoadout,WDB_LOADOUT_rWeapons);
		HWEAPON hWeapon = g_pLTDatabase->GetRecordLink(hWpnAtt,0,NULL);
		if( !hWeapon )
			continue;
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		bcs.hNormal.Load(g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_sSilhouetteIcon));


		bcs.nCommandID = CMD_CUSTOM + nLoadout;
		bcs.szHelpID = g_pWeaponDB->GetString(hLoadout,WDB_LOADOUT_nDescId);
		

		CLTGUITextureButton* pBtn = debug_new(CLTGUITextureButton);
		pBtn->Create(bcs);
		pBtn->SetCommandHandler(this);
		pBtn->Enable(true);
		pBtn->SetFont(CFontInfo(g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenFontFace),g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize)));
		pBtn->SetText(LoadString(g_pWeaponDB->GetString(hLoadout,WDB_LOADOUT_nNameId)),true);
		pBtn->SetBasePos(m_DefaultPos);
		pBtn->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		pBtn->SetAlignment(kLeft);
		m_pList->AddControl(pBtn);

	}

	
 	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;
	UseBack(false);
	return true;

}

uint32 CScreenPlayerLoadout::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{

	if (dwCommand == CMD_LAUNCH)
	{
		//autoselect...
		Escape();
	}
	else if (dwCommand >= CMD_CUSTOM)
	{
		m_nLoadout = (dwCommand - CMD_CUSTOM);
		Escape();
	}
	else
	{
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void CScreenPlayerLoadout::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		GameModeMgr& gameModeMgr = GameModeMgr::Instance( );
		m_setRestrictedWeapons.clear();
		
		m_setRestrictedGear.clear();

		if (gameModeMgr.m_grbUseWeaponRestrictions)
		{
			DelimitedStringToStringContainer(gameModeMgr.m_grsRestrictedWeapons.GetValue(),m_setRestrictedWeapons,",");
			DelimitedStringToStringContainer(gameModeMgr.m_grsRestrictedGear.GetValue(),m_setRestrictedGear,",");
		}
		


		uint32 nCurMission = g_pMissionMgr->GetCurrentMission( );
		HRECORD hMission = g_pMissionDB->GetMission( nCurMission );
		uint32 nCurLevel = g_pMissionMgr->GetCurrentLevel( );
		HRECORD hLevel = g_pMissionDB->GetLevel(hMission,nCurLevel);

		const char* szNameId = g_pMissionDB->GetString(hMission,MDB_Name);
		std::wstring sName;
		if( szNameId[0] != '\0' )
		{
			sName = LoadString(szNameId);
		}
		else
		{
			sName = g_pMissionDB->GetWString(hMission,MDB_NameStr);
			if (!sName.length())
			{
				sName = MPA2W(g_pMissionDB->GetWorldName(hLevel,false)).c_str();
			}
		}
		m_pLevelName->SetString( sName.c_str() );

		uint8 nFirstVisible = -1;
		m_nLoadout = g_pClientConnectionMgr->GetLoadout();

		for (uint8 nLoadout = 0; nLoadout < g_pWeaponDB->GetNumLoadouts(); ++nLoadout)
		{
			HRECORD hLoadout = g_pWeaponDB->GetLoadout(nLoadout);
			CLTGUITextureButton* pBtn = (CLTGUITextureButton*)m_pList->GetControl(nLoadout);
			bool bVis = CheckRestrictions(hLoadout);
			pBtn->Show( bVis );
			pBtn->Enable( bVis );
			uint32 com = pBtn->GetCommandID();
			if (bVis && nFirstVisible >= g_pWeaponDB->GetNumLoadouts())
			{
				nFirstVisible = nLoadout;
			}
			if (m_nLoadout == nLoadout && !bVis)
			{
				m_nLoadout = -1;
			}
		}

		m_pList->CalculatePositions();
		SetSelection(GetIndex(m_pList),true);
		if (m_nLoadout < g_pWeaponDB->GetNumLoadouts())
		{
			m_pList->SetSelection(m_nLoadout);
		}
		else if (nFirstVisible < g_pWeaponDB->GetNumLoadouts())
		{
			m_nLoadout = nFirstVisible;
			m_pList->SetSelection(nFirstVisible);
		}


		//if we have one or more allowed choices, wait 15 seconds before auto-selecting
		if (nFirstVisible <  g_pWeaponDB->GetNumLoadouts() )
		{
			m_AutoSelectTimer.Start(15.0f);
		}
		else
		{
			//send up special code to indicate fallback layout should be used, and auto-select immediately
			m_nLoadout = g_pWeaponDB->GetNumLoadouts();
			m_AutoSelectTimer.Start(0.001f);
		}

		UpdateData(false);
	}
	else
	{
		UpdateData();

	}
	CBaseScreen::OnFocus(bFocus);
}

void CScreenPlayerLoadout::Escape()
{

	if (!g_pClientConnectionMgr->HasSelectedLoadout())
	{
		g_pClientConnectionMgr->SelectLoadout(m_nLoadout);
		g_pInterfaceMgr->UpdatePostLoad();
		g_pClientConnectionMgr->SendMultiPlayerInfo();
	}
	else
	{
		g_pClientConnectionMgr->SelectLoadout(m_nLoadout);
		g_pClientConnectionMgr->SendMultiPlayerInfo();

		CBaseScreen::Escape();
	}
}



bool CScreenPlayerLoadout::UpdateInterfaceSFX()
{
	if (m_AutoSelectTimer.IsTimedOut())
	{
		//handle auto-select
		OnCommand(CMD_LAUNCH,0,0);
		return true;
	}
	return CBaseScreen::UpdateInterfaceSFX();
}



bool CScreenPlayerLoadout::CheckRestrictions(HRECORD hLoadout)
{
	if (!hLoadout)
		return false;

	

	HATTRIBUTE hWpnAtt = g_pLTDatabase->GetAttribute(hLoadout,WDB_LOADOUT_rWeapons);

	bool bHasWeapon = false;
	bool bAllGrenadesRestricted = true;
	bool bAllRestricted = true;
	for (uint32 nWpn = 0; nWpn < g_pLTDatabase->GetNumValues(hWpnAtt); ++nWpn )
	{
		HWEAPON hWeapon = g_pLTDatabase->GetRecordLink(hWpnAtt,nWpn,NULL);
		if( !hWeapon )
			continue;
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

		HWEAPON hSingleWeapon = g_pWeaponDB->GetRecordLink(hWpnData,WDB_WEAPON_rSingleWeapon);
		if (hSingleWeapon)
		{
			hWeapon = hSingleWeapon;
			hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		}

		//keep track of whether we have any non-grenade weapons in the loadout
		if (!g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bIsGrenade))
		{
			bHasWeapon = true;
		}		

		std::string sItem = g_pWeaponDB->GetRecordName(hWeapon);
		StringSet::iterator iter = m_setRestrictedWeapons.find(sItem);

		//make sure the weapon is not in the restricted set
		if (iter == m_setRestrictedWeapons.end())
		{

			if (g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bIsGrenade))
			{
				bAllGrenadesRestricted = false;
			}
			else
			{
				bAllRestricted = false;
			}

			
		}

	}

	//if we have at least one non-grenade weapons, check to see if at least one is unrestricted
	if (bHasWeapon)
	{
		return (!bAllRestricted);
	}

	//we have no non-grenade weapons, so make sure we have at least on unrestricted grenade
	return !bAllGrenadesRestricted;

}