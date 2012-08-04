// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenPlayerSkills.h
//
// PURPOSE : Interface screen for player setup
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenPlayerSkills.h"
#include "ScreenMgr.h"
#include "ScreenCommands.h"

#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;

namespace
{

	uint8 kNumAvailSkills = 0;
	uint8 s_nLevels[kNumSkills];
	uint32 s_nMaxPool = 0;

	void AreYouSureCallBack(LTBOOL bReturn, void *pData)
	{
		CScreenPlayerSkills *pThisScreen = (CScreenPlayerSkills *)g_pInterfaceMgr->GetScreenMgr()->GetScreenFromID(SCREEN_ID_PLAYER_SKILLS);
		if (pThisScreen && bReturn)
		{
			pThisScreen->DoEscape();
		}
	}

}

CScreenPlayerSkills* g_pSkillsScreen = NULL;
bool SliderCallBack(CLTGUISlider* pSlider, int nNewPos,int nOldPos)
{
	if (!g_pSkillsScreen || !pSlider) return false;
	return g_pSkillsScreen->SetSkill((int)pSlider->GetParam1(),nNewPos);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScreenPlayerSkills::CScreenPlayerSkills()
{
	memset(m_nLevels,0,sizeof(m_nLevels));
	memset(m_pSkill,0,sizeof(m_pSkill));
	m_pPool = NULL;
}

CScreenPlayerSkills::~CScreenPlayerSkills()
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CScreenPlayerSkills::Term
//
//	PURPOSE:	Terminate the screen
//
// ----------------------------------------------------------------------- //

void CScreenPlayerSkills::Term()
{
	CBaseScreen::Term();
}

// Build the screen
LTBOOL CScreenPlayerSkills::Build()
{
	g_pSkillsScreen = this;

	CreateTitle(IDS_TITLE_SKILLS);

	int kColumn = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER_SKILLS,"ColumnWidth");
	int kSlider = g_pLayoutMgr->GetScreenCustomInt(SCREEN_ID_PLAYER_SKILLS,"SliderWidth");


	s_nMaxPool = g_pSkillsButeMgr->GetMultiplayerPool();

	LTIntPt pos = m_nextPos;
	AddTextItem(IDS_SKILL_PTS,LTNULL,LTNULL,pos,LTTRUE);
	pos.x += kColumn;

	m_pPool = AddTextItem("999",LTNULL,LTNULL,pos,LTTRUE);

	pos.y += (m_pPool->GetBaseHeight() + 16);

	for (uint8 i = 0; i < kNumSkills; i++)
	{
		pos.x = GetPageLeft();
		eSkill skl = (eSkill)i;
		if (g_pSkillsButeMgr->IsAvailable(skl) )
		{
			m_pSkill[i] = AddSlider(GetSkillNameId(skl),GetSkillDescriptionId(skl),kColumn,kSlider,-1,&m_nLevels[i], pos);
			m_pSkill[i]->SetSliderIncrement(1);
			m_pSkill[i]->SetSliderRange(0,kNumSkillLevels-1);
			m_pSkill[i]->SetRangeCallback(SliderCallBack);
			m_pSkill[i]->SetParam1(i);

			pos.x += (kColumn + kSlider + 5);

			m_pLabel[i] = AddCycle(" ",NULL,20,&s_nLevels[i],pos,LTTRUE);
			for (int j = 0; j < kNumSkillLevels; j++)
			{
				eSkillLevel nxt = (eSkillLevel)(j+1);
				char szTemp[32];
				

				if ( nxt < kNumSkillLevels)
				{
					uint32 nCost = g_pSkillsButeMgr->GetCostToUpgrade(skl,nxt);
					sprintf(szTemp,"%s (%d)",GetSkillLevelName((eSkillLevel)j),nCost);

				}
				else
				{
					SAFE_STRCPY(szTemp,GetSkillLevelName((eSkillLevel)j));
				}
				m_pLabel[i]->AddString(szTemp);
				
			}


			pos.y += m_pSkill[i]->GetBaseHeight();

			kNumAvailSkills++;
		
		}
	}

	pos.x = GetPageLeft();
	pos.y += 8;

	AddTextItem(IDS_RESET_SKILLS,CMD_SKILLS,IDS_HELP_RESET_SKILLS,pos);

 	// Make sure to call the base class
	return CBaseScreen::Build();
}

// Change in focus
void CScreenPlayerSkills::OnFocus(LTBOOL bFocus)
{
	CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();
		
	if (bFocus)
	{
		m_nPool = g_pSkillsButeMgr->GetMultiplayerPool();
		for (int i = 0; i < kNumSkills; i++)
		{
			eSkill skl = (eSkill)i;
			if (g_pSkillsButeMgr->IsAvailable(skl) )
			{
				int tgt = pProfile->m_nPlayerSkills[i];
				m_nLevels[i] = 0;
				int old = 0;

				while (old < tgt && SetSkill(i,old+1))
				{
					old++;
				}

			}
		}
		
			
        UpdateData(LTFALSE);
	}
	else
	{
		UpdateData();

		for (int i = 0; i < kNumSkills; i++)
		{
			pProfile->m_nPlayerSkills[i] = m_nLevels[i];
		}

		pProfile->Save();

	}
	CBaseScreen::OnFocus(bFocus);

}


bool CScreenPlayerSkills::SetSkill(int nSkl, int nNew)
{
	if (nNew < 0 || nNew >= kNumSkillLevels) return false;
	int nOld = m_nLevels[nSkl];
	if (nNew == nOld) return true;

	

	eSkill skl = (eSkill)nSkl;

	int nAvail = m_nPool;
	while (nNew < nOld)
	{
		int nCost = (int)g_pSkillsButeMgr->GetCostToUpgrade(skl,(eSkillLevel)(nOld));
		nAvail += nCost;
		nOld--;
	}

	while (nNew > nOld)
	{
		int nCost = (int)g_pSkillsButeMgr->GetCostToUpgrade(skl,(eSkillLevel)(nOld+1));

		if (nCost > nAvail)
			return false;

		nAvail -= nCost;
		nOld++;
	}


	m_nPool = nAvail;
	m_pPool->SetString(FormatTempString(IDS_X_OF_Y,m_nPool,s_nMaxPool));

	m_nLevels[nSkl] = nNew;
	s_nLevels[nSkl] = (uint8)m_nLevels[nSkl];
	m_pLabel[nSkl]->UpdateData(LTFALSE);

	return true;
}

void CScreenPlayerSkills::UpdateData(LTBOOL bSaveAndValidate)
{
	if (!bSaveAndValidate)
	{
		for (int i = 0; i < kNumSkills; i++)
		{
			s_nLevels[i] = (uint8)m_nLevels[i];
		}
		m_pPool->SetString(FormatTempString(IDS_X_OF_Y,m_nPool,s_nMaxPool));

	}
	CBaseScreen::UpdateData(bSaveAndValidate);
}

uint32 CScreenPlayerSkills::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_SKILLS:
	{
		for (int i = 0; i < kNumSkills; i++)
		{
			eSkill skl = (eSkill)i;
			if (g_pSkillsButeMgr->IsAvailable(skl) )
			{
				SetSkill(i,0);
			}
		}
		UpdateData(LTFALSE);

	} break;

	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}

	return 1;
};


void CScreenPlayerSkills::Escape()
{

	if (m_nPool > 0)
	{
		char szTemp[512];
		FormatString(IDS_CONFIRM_SKILLS,szTemp,sizeof(szTemp),m_nPool);
		MBCreate mb;
		mb.eType = LTMB_YESNO;
		mb.pFn = AreYouSureCallBack;
		g_pInterfaceMgr->ShowMessageBox(szTemp,&mb,0,LTFALSE);
	}
	else
		CBaseScreen::Escape();

}

void CScreenPlayerSkills::DoEscape()
{
	CBaseScreen::Escape();
}