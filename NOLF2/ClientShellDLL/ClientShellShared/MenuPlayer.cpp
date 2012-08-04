// ----------------------------------------------------------------------- //
//
// MODULE  : MenuPlayer.cpp
//
// PURPOSE : In-game system menu
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MenuPlayer.h"
#include "InterfaceMgr.h"
#include "MsgIds.h"
#include "ClientResShared.h"


namespace
{
	uint32 g_nSkill = 0;
	uint32 MC_UPGRADE = MC_CUSTOM + 100;
	CMenuPlayer* s_pSklMenu = NULL;
	uint32 skillHighlightColor = 0xFFE0E030; 
}

LTBOOL CSkillPopup::OnUp()
{
	LTBOOL bHandled = CLTGUIWindow::OnUp();

	if (bHandled && s_pSklMenu)
		s_pSklMenu->UpdateModText();

	return bHandled;

}

LTBOOL CSkillPopup::OnDown()
{
	LTBOOL bHandled = CSubMenu::OnDown();

	if (bHandled && s_pSklMenu)
		s_pSklMenu->UpdateModText();

	return bHandled;
}

LTBOOL CSkillPopup::OnMouseMove(int x, int y)
{
	LTBOOL bHandled = CSubMenu::OnMouseMove(x, y);

	if (bHandled && s_pSklMenu)
		s_pSklMenu->UpdateModText();

	return bHandled;
}

LTBOOL CSkillPopup::HandleKeyDown (int vkey, int rep)
{

	if (vkey == VK_ESCAPE)
	{
		m_bWaitForUpdate = LTFALSE;
		g_pInterfaceMgr->GetMenuMgr()->HideSubMenu(true);
		ClearSelection();
		return LTTRUE;
	}
	if (m_bWaitForUpdate) return LTTRUE;

	return LTFALSE;
}



LTBOOL CMenuPlayer::Init()
{
	m_MenuID = MENU_ID_PLAYER;
	s_pSklMenu = this;

	if (!CBaseMenu::Init()) return LTFALSE;

	int nNameWidth = g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"NameWidth");
	int nLevelWidth = g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"LevelWidth");
	int nUpgradeWidth = s_Size.x - ( (m_Indent.x * 2) + nNameWidth + nLevelWidth);


	LTVector vColor = g_pLayoutMgr->GetMenuCustomVector(m_MenuID,"HighlightColor");
	uint8 nA = 255;
	uint8 nR = (uint8)vColor.x;
	uint8 nG = (uint8)vColor.y;
	uint8 nB = (uint8)vColor.z;

	skillHighlightColor= SET_ARGB(nA,nR,nG,nB);


	uint8 nSmallerFont = (uint8)g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"SmallerFontSize");

	SetTitle(IDS_TITLE_SUMMARY);

	LTIntPt	popupSize = g_pLayoutMgr->GetMenuCustomPoint(m_MenuID,"PopupSize");
	m_Popup.Init(s_Frame,s_FrameTip,popupSize);
	m_Popup.m_bWaitForUpdate = LTFALSE;
	LTIntPt offset = m_Indent;
	offset.y = 8;

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_TitleFontFace);
	m_Name.Create("name",LTNULL,LTNULL,pFont,m_TitleFontSize,LTNULL);
	m_Name.SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_Name.Enable(LTFALSE);
	m_Popup.AddControl(&m_Name,offset);
	offset.y += (m_Name.GetHeight() + 4);


	pFont = g_pInterfaceResMgr->GetFont(m_FontFace);
	m_Level.Create("level",LTNULL,LTNULL,pFont,m_FontSize,LTNULL);
	m_Level.SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_Level.Enable(LTFALSE);
	m_Popup.AddControl(&m_Level,offset);
	offset.y += (m_Level.GetHeight() + 12);

	m_Header.Create(LTNULL,LTNULL,pFont,m_FontSize, LTNULL);
	m_Header.AddColumn(" ",nNameWidth);
	m_Header.AddColumn(LoadTempString(IDS_CURRENT),60);
	m_Header.AddColumn(LoadTempString(IDS_UPGRADE),60);
	m_Header.SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_Header.Enable(LTFALSE);
	m_Popup.AddControl(&m_Header,offset);
	offset.y += (m_Header.GetHeight() + 2);


	for (uint8 m = 0; m < kMaxModifiers; ++m)
	{
		m_Mods[m].Create(LTNULL,LTNULL,pFont,m_FontSize,LTNULL);
		m_Mods[m].AddColumn(" ",nNameWidth);
		m_Mods[m].AddColumn("100",60);
		m_Mods[m].AddColumn("100",60);
		m_Mods[m].SetColors(skillHighlightColor,m_NonSelectedColor,m_DisabledColor);
		m_Mods[m].Enable(LTTRUE);
		m_Popup.AddControl(&m_Mods[m],offset);
		offset.y += (m_Mods[m].GetHeight() + 2);

	}
	offset.y += 8;

	m_Upgrade.Create("Upgrade",MC_UPGRADE,IDS_HELP_UPGRADE,pFont,m_FontSize,this);
	m_Upgrade.SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	m_Popup.AddControl(&m_Upgrade,offset);
	offset.y += (m_Upgrade.GetHeight() + 4);

	m_Points.Create("avail",LTNULL,LTNULL,pFont,m_FontSize,LTNULL);
	m_Points.SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_Points.SetFont(LTNULL,nSmallerFont);
	m_Points.Enable(LTFALSE);
	m_Popup.AddControl(&m_Points,offset);

	uint8 nHelpFont = (uint8)g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"HelpFontSize");
	offset = g_pLayoutMgr->GetMenuCustomPoint(m_MenuID,"HelpOffset");
	uint16 nWidth = 600 - 2* offset.x;
	if (!nHelpFont)
		nHelpFont = m_FontSize;
	m_ModDesc.Create("description",LTNULL,LTNULL,pFont,nHelpFont,LTNULL);
	m_ModDesc.SetColors(m_SelectedColor,m_SelectedColor,m_SelectedColor);
	m_ModDesc.SetFixedWidth(nWidth);
	m_ModDesc.Enable(LTFALSE);
	m_Popup.AddControl(&m_ModDesc,offset);



	int nPopupPos = g_pLayoutMgr->GetMenuCustomInt(m_MenuID,"PopupPos");
	m_Popup.SetBasePos(LTIntPt(0,nPopupPos));

	g_pInterfaceMgr->GetMenuMgr()->RegisterCommand(COMMAND_ID_STATUS,MENU_ID_PLAYER);

	//Rank control
	m_pRank = AddColumnCtrl();
	m_pRank->AddColumn(LoadTempString(IDS_RANK),nNameWidth);
	m_pRank->AddColumn("Novice",nNameWidth);

	//Total points control
	m_pTotal = AddColumnCtrl();
	m_pTotal->AddColumn(LoadTempString(IDS_SCORE),nNameWidth);
	m_pTotal->AddColumn("0",nNameWidth);

	//Avail points control
	m_pAvail = AddColumnCtrl();
	m_pAvail->AddColumn(LoadTempString(IDS_SKILL_PTS),nNameWidth);
	m_pAvail->AddColumn("0",nNameWidth);

	AddControl(" ",0,LTTRUE);


//	CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_FontFace);
	for (uint8 i = 0; i < kNumSkills; i++)
	{
		eSkill skl = (eSkill)i;
		if (g_pSkillsButeMgr->IsAvailable(skl) )
		{
			m_pSkills[i] = debug_new(CSkillCtrl);
			m_pSkills[i]->Create(skl,(MC_CUSTOM+i),pFont,m_FontSize,this,nNameWidth);
			m_List.AddControl(m_pSkills[i]);
		}

	}
	

	return LTTRUE;
}

void CMenuPlayer::Term()
{

	m_Popup.RemoveAll(LTFALSE);
	
	CBaseMenu::Term();
}


uint32 CMenuPlayer::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{

	if (m_Popup.m_bWaitForUpdate)
	{
		if (nCommand == MC_UPDATE)
		{
			m_Popup.m_bWaitForUpdate = LTFALSE;
//			g_pInterfaceMgr->GetMenuMgr()->HideSubMenu(true);
			UpdateControls();
		}

		return 1;

	}
	if (nCommand == MC_UPGRADE && m_Upgrade.IsEnabled())
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_SKILLS);
		cMsg.Writeuint8((uint8)g_nSkill);
		g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

		m_Popup.m_bWaitForUpdate = LTTRUE;

		// Play upgrade sound...
		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Snd\\upgraded_skill.wav");
	}
	else if (nCommand >= MC_CUSTOM)
	{
		g_nSkill = nCommand - MC_CUSTOM;
		UpdatePopup();
		m_Points.SetString(FormatTempString(IDS_SKILL_AVAIL,g_pPlayerStats->GetAvailSkillPoints()));
		m_Popup.ClearSelection();
		m_Popup.PreviousSelection();
		UpdateModText(true);
		g_pInterfaceMgr->GetMenuMgr()->ShowSubMenu(&m_Popup);
		
	}
	else
	{
		return CBaseMenu::OnCommand(nCommand,nParam1,nParam2);
	}

	return 1;
}


// This is called when the screen gets or loses focus
void CMenuPlayer::OnFocus(LTBOOL bFocus)
{
	CBaseMenu::OnFocus(bFocus);
	if (bFocus)
	{
		m_Popup.m_bWaitForUpdate = LTFALSE;
		UpdateControls();

		m_List.NextSelection();

	}
}

void CMenuPlayer::UpdateControls()
{
	const RANK* pRank = g_pPlayerStats->GetRank();
	if (pRank)
		m_pRank->GetColumn(1)->SetString(LoadTempString(pRank->nNameId));
	else
		m_pRank->GetColumn(1)->SetString("<error>");

	char szTmp[32];
	sprintf(szTmp,"%d",g_pPlayerStats->GetTotalSkillPoints());
	m_pTotal->GetColumn(1)->SetString(szTmp);
	m_pTotal->Show(!IsMultiplayerGame());

	sprintf(szTmp,"%d",g_pPlayerStats->GetAvailSkillPoints());
	m_pAvail->GetColumn(1)->SetString(szTmp);
	m_pAvail->Show(!IsMultiplayerGame());

	for (uint8 i = 0; i < kNumSkills; i++)
	{
		eSkill skl = (eSkill)i;
		if (m_pSkills[i])
		{
			eSkillLevel lvl = (eSkillLevel)g_pPlayerStats->GetSkillLevel(skl);
			m_pSkills[i]->SetSkillLevel(lvl);
		}
	}

	if (m_Popup.IsVisible())
		UpdatePopup();
}



CLTGUIColumnCtrl* CMenuPlayer::AddColumnCtrl(uint32 nCommand)
{
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_FontFace);

	CLTGUIColumnCtrl* pCtrl=debug_new(CLTGUIColumnCtrl);
    if (!pCtrl->Create(nCommand,LTNULL,pFont,m_FontSize, this))
	{
		debug_delete(pCtrl);
        return LTFALSE;
	}

	pCtrl->SetBasePos(m_nextPos);

	if (nCommand > 0)
	{
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		pCtrl->Enable(LTTRUE);
	}
	else
	{
		pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		pCtrl->Enable(LTFALSE);
	}
	pCtrl->SetScale(g_pInterfaceResMgr->GetXRatio());

	m_List.AddControl(pCtrl);

	return pCtrl;
}

void CMenuPlayer::UpdatePopup()
{
	eSkill skl = (eSkill)g_nSkill;
	eSkillLevel lvl = (eSkillLevel)g_pPlayerStats->GetSkillLevel(skl);
	eSkillLevel nxt = (eSkillLevel)(g_pPlayerStats->GetSkillLevel(skl) + 1);

	m_Name.SetString(GetSkillName(skl));
	m_Level.SetString(GetSkillLevelName(lvl));

	m_Header.GetColumn(2)->Show(!IsMultiplayerGame());

	uint8 nMax = g_pSkillsButeMgr->GetNumModifiers(skl);
	
	for (uint8 m = 0; m < kMaxModifiers; ++m)
	{
		if (m < nMax)
		{
			m_Mods[m].GetColumn(0)->SetString(LoadTempString(g_pSkillsButeMgr->GetModifierNameId(skl,m)));
			m_Mods[m].SetHelpID(g_pSkillsButeMgr->GetModifierDescriptionId(skl,m));

			char szTmp[8];

			float fBase = g_pSkillsButeMgr->GetModifier(skl,SKL_NOVICE,m);
			
			if (fBase <= 0.0f)
			{
				ASSERT(!"MenuPlayer::Bad skill mod");
				fBase = 1.0f;
			}
			float fCur = g_pSkillsButeMgr->GetModifier(skl,lvl,m);
			if (fCur <= 0.0f)
			{
				ASSERT(!"MenuPlayer::Bad skill mod");
				fCur = 1.0f;
			}

			if (fCur >= fBase)
			{
				sprintf(szTmp,"%3.0f%%",100.0f*fCur/fBase);
			}
			else
			{
				sprintf(szTmp,"%3.0f%%",100.0f*fBase/fCur);
			}

			m_Mods[m].GetColumn(1)->SetString(szTmp);


			if ( nxt < kNumSkillLevels)
			{
				float fNxt = g_pSkillsButeMgr->GetModifier(skl,nxt,m);
				if (fNxt <= 0.0f)
				{
					ASSERT(!"MenuPlayer::Bad skill mod");
					fNxt = 1.0f;
				}
				if (fNxt >= fBase)
				{
					sprintf(szTmp,"%3.0f%%",100.0f*fNxt/fBase);
				}
				else
				{
					sprintf(szTmp,"%3.0f%%",100.0f*fBase/fNxt);
				}

			}
			else
			{
				sprintf(szTmp," - ");
			}
			m_Mods[m].GetColumn(2)->SetString(szTmp);
			m_Mods[m].GetColumn(2)->Show(!IsMultiplayerGame());
		
			
			m_Mods[m].Show(true);
		}
		else
		{
			m_Mods[m].Show(false);
		}
	}

	
	if (IsMultiplayerGame())
	{
		m_Upgrade.Show(LTFALSE);
		m_Upgrade.Enable(LTFALSE);
		m_Upgrade.Select(LTFALSE);
		m_Popup.ClearSelection();
		m_Popup.NextSelection();
	}
	else if ( nxt < kNumSkillLevels)
	{
		uint32 nCost = g_pPlayerStats->GetCostToUpgrade(skl);

		char szNext[32];
		SAFE_STRCPY(szNext,GetSkillLevelName(nxt));
		m_Upgrade.SetString(FormatTempString(IDS_SKILL_UPGRADE,szNext,nCost));
		m_Upgrade.SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
		LTBOOL bCanAfford =  (nCost <= g_pPlayerStats->GetAvailSkillPoints()  );

		m_Upgrade.Show(LTTRUE);
		m_Upgrade.Enable(bCanAfford);
		m_Upgrade.Select(bCanAfford);
		if (!bCanAfford)
		{
			m_Popup.ClearSelection();
			m_Popup.NextSelection();
		}
	}
	else
	{
		m_Upgrade.Show(LTTRUE);
		m_Upgrade.SetString(FormatTempString(IDS_SKILL_MAX));
		m_Upgrade.SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		m_Upgrade.Enable(LTFALSE);
		m_Upgrade.Select(LTFALSE);
		m_Popup.ClearSelection();
		m_Popup.NextSelection();

	}

	m_Points.SetString(FormatTempString(IDS_SKILL_AVAIL,g_pPlayerStats->GetAvailSkillPoints()));
	m_Points.Show(!IsMultiplayerGame());
	
}

void CMenuPlayer::UpdateModText(bool bForce)
{
	static uint16 nCurrentHelp = 0;
	uint16 nHelp = m_Popup.GetHelpID();

	if (bForce || nHelp != nCurrentHelp)
	{
		nCurrentHelp = nHelp;
		if (nCurrentHelp)
		{
			m_ModDesc.SetString(LoadTempString(nCurrentHelp));
		}
		else
			m_ModDesc.SetString(" ");
	}
}