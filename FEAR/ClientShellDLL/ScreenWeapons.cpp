// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenWeapons.cpp
//
// PURPOSE : Interface to set weapon priorities
//
// CREATED : 07/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ScreenWeapons.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"
#include "WeaponDB.h"

#include "GameClientShell.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenWeapons::CScreenWeapons() :
		m_pWeapons (NULL)
		, m_pCurrentTxt (NULL)
		, m_pCurrentImg (NULL)
		, m_pUp (NULL)
		, m_pDown (NULL)
		, m_pSelectedWpn (NULL)
		, m_cHighlight (argbWhite)
{
}

CScreenWeapons::~CScreenWeapons()
{

}


// Build the screen
bool CScreenWeapons::Build()
{
	CreateTitle("IDS_TITLE_WEAPONS");


	CLTGUICtrl_create frameCs;
	TextureReference hFrame(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenFrameTexture));
	frameCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,0);

	CLTGUIFrame *pFrame = debug_new(CLTGUIFrame);
	pFrame->Create(hFrame,frameCs);
	AddControl(pFrame);


	CLTGUIListCtrl_create listCs;
	listCs.rnBaseRect = g_pLayoutDB->GetListRect(m_hLayout,0);
	listCs.vnArrowSz = g_pLayoutDB->GetListArrowSize(m_hLayout,0); 
//	listCs.pCommandHandler = this;
//	listCs.nCommandID = CMD_WEAPONS;

	m_pWeapons = AddList(listCs);
	m_pWeapons->SetIndent(g_pLayoutDB->GetListIndent(m_hLayout,0));
	hFrame.Load(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,0));
	TextureReference hSelFrame(g_pLayoutDB->GetListFrameTexture(m_hLayout,0,1));
	m_pWeapons->SetFrame(hFrame,hSelFrame,g_pLayoutDB->GetListFrameExpand(m_hLayout,0));
	m_pWeapons->SetScrollWrap(false);


	frameCs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,1);

	m_pCurrentImg = debug_new(CLTGUIFrame);
	m_pCurrentImg->Create(hFrame,frameCs,true);

	AddControl(m_pCurrentImg);


	CLTGUICtrl_create cs;
	cs.rnBaseRect = g_pLayoutDB->GetRect(m_hLayout,LDB_ScreenFrameRect,2);
	m_pCurrentTxt = AddTextItem(L"",cs,true);
	m_pCurrentTxt->SetWordWrap(true);

	LTVector2 vSz = g_pLayoutDB->GetVector2(m_hLayout,LDB_ScreenAdditionalPos,2);
	LTVector2n btnSz(int32(vSz.x),int32(vSz.y));
	LTVector2n textSz(g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0),btnSz.y);

	const char* szFont = g_pLayoutDB->GetFont(m_hLayout,LDB_ScreenFontFace);
	uint32 nFontHeight = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize);

	CLTGUITextureButton_create bcs;
	LTVector2 vPos = g_pLayoutDB->GetVector2(m_hLayout,LDB_ScreenAdditionalPos,0);
	bcs.rnImageRect.m_vMin = LTVector2n(int32(vPos.x),int32(vPos.y));
	bcs.rnImageRect.m_vMax = bcs.rnImageRect.m_vMin + btnSz;
	bcs.rnTextRect.m_vMin = LTVector2n(int32(vPos.x)+ btnSz.x +8,int32(vPos.y));
	bcs.rnTextRect.m_vMax = bcs.rnTextRect.m_vMin + textSz;
	bcs.rnBaseRect.m_vMin = bcs.rnImageRect.m_vMin;
	bcs.rnBaseRect.m_vMax = bcs.rnTextRect.m_vMax;
	bcs.hNormal.Load(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenAddTex,0));
	bcs.hSelected.Load(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenAddTex,1));
	bcs.nCommandID = CMD_UP;
	bcs.szHelpID = "ScreenWeapons_MoveUp_Help";
	bcs.pCommandHandler = this;


	m_pUp = debug_new(CLTGUITextureButton);
	m_pUp->Create(bcs);
	m_pUp->SetFont(CFontInfo(szFont,nFontHeight));
	m_pUp->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_pUp->SetText(LoadString("ScreenWeapons_MoveUp"),true);
	AddControl(m_pUp);

	//restore
	vPos = g_pLayoutDB->GetVector2(m_hLayout,LDB_ScreenAdditionalPos,1);
	cs.rnBaseRect.m_vMin = LTVector2n(int32(vPos.x),int32(vPos.y));
	cs.rnBaseRect.m_vMin.y -= (textSz.y+8);
	cs.rnBaseRect.m_vMax = cs.rnBaseRect.m_vMin + textSz;
	cs.nCommandID = CMD_RESET_DEFAULTS;
	cs.szHelpID = "ScreenWeapons_Restore_Help";
	AddTextItem("ScreenWeapons_Restore", cs);
	

	vPos = g_pLayoutDB->GetVector2(m_hLayout,LDB_ScreenAdditionalPos,1);
	bcs.rnImageRect.m_vMin = LTVector2n(int32(vPos.x),int32(vPos.y));
	bcs.rnImageRect.m_vMax = bcs.rnImageRect.m_vMin + btnSz;
	bcs.rnTextRect.m_vMin = LTVector2n(int32(vPos.x)+btnSz.x+8,int32(vPos.y));
	bcs.rnTextRect.m_vMax = bcs.rnTextRect.m_vMin + textSz;
	bcs.rnBaseRect.m_vMin = bcs.rnImageRect.m_vMin;
	bcs.rnBaseRect.m_vMax = bcs.rnTextRect.m_vMax;
	bcs.hNormal.Load(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenAddTex,2));
	bcs.hSelected.Load(g_pLayoutDB->GetString(m_hLayout,LDB_ScreenAddTex,3));
	bcs.nCommandID = CMD_DOWN;
	bcs.szHelpID = "ScreenWeapons_MoveDown_Help";

	m_pDown = debug_new(CLTGUITextureButton);
	m_pDown->Create(bcs);
	m_pDown->SetFont(CFontInfo(szFont,nFontHeight));
	m_pDown->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_pDown->SetText(LoadString("ScreenWeapons_MoveDown"),true);
	AddControl(m_pDown);


	nFontHeight = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAdditionalInt,0);
	CLTGUICtrl_create newcs;
	newcs.rnBaseRect.m_vMin.y = m_DefaultPos.y;
	newcs.rnBaseRect.m_vMin.x = m_ScreenRect.m_vMin.x;
	newcs.rnBaseRect.m_vMax.x = newcs.rnBaseRect.m_vMin.x + g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,0) + g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenColumnWidths,1);
	newcs.rnBaseRect.m_vMax.y = newcs.rnBaseRect.m_vMin.y + nFontHeight;
	
	CLTGUITextCtrl *pTxt = AddTextItem("ScreenWeapons_Hint",newcs,true);
	pTxt->SetFont( CFontInfo(g_pLayoutDB->GetHelpFont(),nFontHeight) );
	pTxt->SetWordWrap(true);
	pTxt->SetColor(m_NonSelectedColor);

	uint32 nWidth = m_pWeapons->GetBaseWidth() - 16;
	uint32 nListFontSize = g_pLayoutDB->GetListSize(m_hLayout,0);

	for (int nWpn = g_pWeaponDB->GetNumDefaultWeaponPriorities()-1; nWpn >= 0; nWpn--)
	{
		HWEAPON hWpn = g_pWeaponDB->GetWeaponFromDefaultPriority(nWpn);

		if (hWpn == g_pWeaponDB->GetUnarmedRecord())
			continue;

		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWpn, !USE_AI_DATA);

		std::wstring wsName = LoadString(g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_nShortNameId));

		CLTGUICtrl_create cs;
		cs.rnBaseRect.m_vMin.Init();
		cs.rnBaseRect.m_vMax = LTVector2n(nWidth,g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenFontSize));
		cs.nCommandID = CMD_CUSTOM;
		cs.nParam1 = nWpn;

		CLTGUITextCtrl *pCtrl = CreateTextItem(wsName.c_str(),cs,false,NULL,nListFontSize);
		pCtrl->SetClipping(true);

		m_pWeapons->AddControl(pCtrl);
	}

	m_cHighlight = g_pLayoutDB->GetInt32(m_hLayout,LDB_ScreenAddColor,0);

	// Make sure to call the base class
	if (! CBaseScreen::Build()) return false;
	UseBack(true,true);
	return true;
}

uint32 CScreenWeapons::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_CUSTOM:
		{
			CLTGUICtrl *pCtrl = m_pWeapons->GetSelectedControl();
			SelectWeapon(pCtrl);
			
		} break;
	case CMD_UP:
		{
			uint32 nIndex = m_pWeapons->GetIndex(m_pSelectedWpn);
			
			if (nIndex != CLTGUIListCtrl::kNoSelection && nIndex > 0)
			{
				uint32 nNewIndex = nIndex-1;
				m_pWeapons->SwapItems(nIndex,nNewIndex);
				m_pUp->Enable( nNewIndex > 0);
				m_pDown->Enable( nNewIndex < (m_pWeapons->GetNumControls() - 1 ));

			}
		} break;
	case CMD_DOWN:
		{
			uint32 nIndex = m_pWeapons->GetIndex(m_pSelectedWpn);
			if (nIndex != CLTGUIListCtrl::kNoSelection && nIndex < (m_pWeapons->GetNumControls()-1) )
			{
				uint32 nNewIndex = nIndex+1;
				m_pWeapons->SwapItems(nIndex,nIndex+1);
				m_pUp->Enable( nNewIndex > 0);
				m_pDown->Enable( nNewIndex < (m_pWeapons->GetNumControls() - 1 ));
			}
		} break;
	case CMD_RESET_DEFAULTS:
		{
			CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
			pProfile->RestoreDefaults(PROFILE_WEAPONS);
			LoadList();

		} break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
}

void CScreenWeapons::OnFocus(bool bFocus)
{
	if (bFocus)
	{
		LoadList();

		UpdateData(false);
	}
	else
	{
		UpdateData();

		SaveList();
		SelectWeapon(NULL);

	}
	CBaseScreen::OnFocus(bFocus);

		
}


void CScreenWeapons::SelectWeapon(CLTGUICtrl* pCtrl)
{
	if (m_pSelectedWpn)
	{
		m_pSelectedWpn->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	}
	m_pSelectedWpn = pCtrl;


	if (pCtrl)
	{
		uint32 nWpn = pCtrl->GetParam1();
		HWEAPON hWpn = g_pWeaponDB->GetWeaponFromDefaultPriority(nWpn);
		if (hWpn && hWpn != g_pWeaponDB->GetUnarmedRecord())
		{
			pCtrl->SetColor(m_cHighlight);
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWpn, !USE_AI_DATA);
			TextureReference hImg(g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_sSilhouetteIcon));
			m_pCurrentImg->SetFrame(hImg);

			const char* szName = g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_nLongNameId);
			m_pCurrentTxt->SetString(LoadString(szName));

		
			uint32 nIndex = m_pWeapons->GetIndex(pCtrl);
			m_pUp->Enable( nIndex > 0);
			m_pDown->Enable( nIndex < (m_pWeapons->GetNumControls() -1 ));

		}
		
	}
	else
	{
		m_pCurrentImg->SetFrame(NULL);
		m_pCurrentTxt->SetString(L"");
		m_pUp->Enable( false);
		m_pDown->Enable( false );
	}

}


void CScreenWeapons::LoadList()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
	//sort the elements of the list
	for (uint32 nIndex = 0; nIndex < pProfile->m_vecWeapons.size(); ++nIndex)
	{
		uint32 nWpn = (pProfile->m_vecWeapons.size() - nIndex) - 1;
		HWEAPON hWpn = pProfile->m_vecWeapons[nWpn];
		if (hWpn && g_pWeaponDB->GetUnarmedRecord() != hWpn)
		{
			uint32 nNewIndex = FindIndexOfWeapon(hWpn);
			m_pWeapons->SwapItems(nIndex,nNewIndex);
		}
		
	}

	m_pWeapons->SetSelection(0);
	SelectWeapon(m_pWeapons->GetSelectedControl());


}
void CScreenWeapons::SaveList()
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

	pProfile->m_vecWeapons.clear();
	pProfile->m_vecWeapons.push_back(g_pWeaponDB->GetUnarmedRecord());
	for (uint32 nIndex = m_pWeapons->GetNumControls()-1;nIndex < m_pWeapons->GetNumControls(); --nIndex) 
	{
		CLTGUICtrl *pCtrl = m_pWeapons->GetControl(nIndex);
		uint32 nPriority = pCtrl->GetParam1();
		HWEAPON hWpn = g_pWeaponDB->GetWeaponFromDefaultPriority(nPriority);

		pProfile->m_vecWeapons.push_back(hWpn);
	}

	pProfile->ApplyWeaponPriorities();
	pProfile->Save();

}

uint32 CScreenWeapons::FindIndexOfWeapon(HWEAPON hWpn)
{
	uint32 nPriority = g_pWeaponDB->GetDefaultWeaponPriority(hWpn);
	uint32 nIndex = 0;

	while (nIndex < m_pWeapons->GetNumControls()) 
	{
		CLTGUICtrl *pCtrl = m_pWeapons->GetControl(nIndex);
		if (pCtrl->GetParam1() == nPriority)
			return nIndex;
		++nIndex;
	}
	return CLTGUIListCtrl::kNoSelection;
}
