// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenHostLevels.cpp
//
// PURPOSE : Interface screen for choosing levels for a hosted game
//
// (c) 1999-2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenHostLevels.h"
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

CScreenHostLevels::CScreenHostLevels()
{
    m_pAvailMissions	= LTNULL;
    m_pSelMissions    = LTNULL;
	m_pAdd			= LTNULL;
	m_pRemove		= LTNULL;
	m_pAddAll		= LTNULL;
	m_pRemoveAll    = LTNULL;
	m_pLoopToggle	= LTNULL;

}

CScreenHostLevels::~CScreenHostLevels()
{
	Term();
}

// Build the screen
LTBOOL CScreenHostLevels::Build()
{
	LTIntPt addPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"AddPos");
	LTIntPt removePos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"RemovePos");
	LTIntPt commandPos = g_pLayoutMgr->GetScreenCustomPoint((eScreenID)m_nScreenID,"CommandPos");

	if (g_pLayoutMgr->HasCustomValue((eScreenID)m_nScreenID,"ListFontSize"))
		nListFontSize = (uint8)g_pLayoutMgr->GetScreenCustomInt((eScreenID)m_nScreenID,"ListFontSize");

	CreateTitle(IDS_TITLE_HOST_MISSIONS);

	m_pAdd = AddTextItem(IDS_HOST_ADD_MISSION, CMD_ADD_LEVEL, IDS_HELP_ADD_MISSION, addPos);
	
	m_pAddAll = AddTextItem(IDS_HOST_ADD_ALL, CMD_ADD_ALL, IDS_HELP_ADD_ALL, commandPos);
	m_pRemoveAll = AddTextItem(IDS_HOST_REMOVE_ALL, CMD_REMOVE_ALL, IDS_HELP_REM_ALL);

	m_pRemove = AddTextItem(IDS_HOST_REMOVE_MISSION, CMD_REMOVE_LEVEL, IDS_HELP_REM_MISSION, removePos);

	
	LTRect rcAvailRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"AvailRect");
	int nListHeight = (rcAvailRect.bottom - rcAvailRect.top);
	int nListWidth = (rcAvailRect.right - rcAvailRect.left) - 32;

	m_pAvailMissions = AddList(LTIntPt(rcAvailRect.left,rcAvailRect.top),nListHeight,LTTRUE,nListWidth);
	m_pAvailMissions->SetIndent(LTIntPt(5,5));
	m_pAvailMissions->SetFrameWidth(2);
	m_pAvailMissions->Enable(LTFALSE);

	LTRect rcSelRect = g_pLayoutMgr->GetScreenCustomRect((eScreenID)m_nScreenID,"SelectRect");
	nListHeight = (rcSelRect.bottom - rcSelRect.top);
	nListWidth = (rcSelRect.right - rcSelRect.left) - 32;

	m_pSelMissions = AddList(LTIntPt(rcSelRect.left,rcSelRect.top),nListHeight,LTTRUE,nListWidth);
	m_pSelMissions->SetIndent(LTIntPt(5,5));
	m_pSelMissions->SetFrameWidth(2);
	m_pSelMissions->Enable(LTFALSE);

	m_nextPos.y += nListHeight;
	nListWidth -= 16;
	m_pLoopToggle = AddToggle(IDS_LOOP_MISSIONS,IDS_HELP_LOOP_MISSIONS,nListWidth,&m_bLoopMissions);
	m_pLoopToggle->SetOnString(LoadTempString(IDS_YES));
	m_pLoopToggle->SetOffString(LoadTempString(IDS_NO));


 	// Make sure to call the base class
	if (!CBaseScreen::Build()) return LTFALSE;

	UseBack(LTTRUE,LTTRUE);

	

	return LTTRUE;

}

void CScreenHostLevels::Escape()
{
	if (m_pAvailMissions->IsEnabled())
	{
		m_pAvailMissions->Enable(LTFALSE);
		m_pAdd->Enable(LTTRUE);
		SetSelection(GetIndex(m_pAdd));
	}
	else if (m_pSelMissions->IsEnabled())
	{
		m_pSelMissions->Enable(LTFALSE);
		m_pRemove->Enable(LTTRUE);
		SetSelection(GetIndex(m_pRemove));
	}
	else
	{
		CBaseScreen::Escape();
	}
}



uint32 CScreenHostLevels::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case CMD_BACK:
		{
			m_pAvailMissions->Enable(LTFALSE);
			m_pSelMissions->Enable(LTFALSE);
			m_pScreenMgr->EscapeCurrentScreen();
			break;
		}

	case CMD_ADD_LEVEL:
		{
			if (!m_pAvailMissions->IsEnabled() && m_pAvailMissions->GetNumControls())
			{
				m_pSelMissions->Enable(LTFALSE);
				m_pRemove->Enable(LTTRUE);
				m_pAvailMissions->Enable(LTTRUE);
				m_pAdd->Enable(LTFALSE);
				SetSelection(GetIndex(m_pAvailMissions));
				m_pAvailMissions->SetSelection(0);
			}
			else
			{
				char sMission[256] = "";

				if (m_pAvailMissions->GetSelectedIndex() >= 0 && (m_pSelMissions->GetNumControls() < MAX_GAME_LEVELS))
				{
					CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pAvailMissions->GetSelectedControl();
					if (pCtrl)
					{
						AddMissionToList(pCtrl->GetParam1());
						m_pSelMissions->ClearSelection();
					}
				}

				UpdateButtons();
			}
		} break;
	case CMD_ADD_ALL:
		{
			if (m_pAvailMissions->GetNumControls())
			{
				for (int i = 0; i < m_pAvailMissions->GetNumControls() && (m_pSelMissions->GetNumControls() < MAX_GAME_LEVELS); i++)
				{
					char sMission[256] = "";
					CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pAvailMissions->GetControl(i);
					if (pCtrl)
					{
						AddMissionToList(pCtrl->GetParam1());
					}
				}
			}
			m_pSelMissions->ClearSelection();
			UpdateButtons();
		} break;
	case CMD_REMOVE_LEVEL:
		{
			if (!m_pSelMissions->IsEnabled() && m_pSelMissions->GetNumControls())
			{
				m_pAvailMissions->Enable(LTFALSE);
				m_pAdd->Enable(LTTRUE);
				m_pSelMissions->Enable(LTTRUE);
				m_pRemove->Enable(LTFALSE);
				SetSelection(GetIndex(m_pSelMissions));
				m_pSelMissions->SetSelection(0);
			}
			else
			{
				int nIndex = m_pSelMissions->GetSelectedIndex();
				if (nIndex >= 0)
				{
					m_pSelMissions->RemoveControl(nIndex);
				}
				UpdateButtons();
			}

		} break;
	case CMD_REMOVE_ALL:
		{
			if (m_pSelMissions->GetNumControls() > 0)
			{
				m_pSelMissions->ClearSelection();
				m_pSelMissions->RemoveAll();
			}
			UpdateButtons();
		} break;
	default:
		return CBaseScreen::OnCommand(dwCommand,dwParam1,dwParam2);
	}
	return 1;
};


// Change in focus
void    CScreenHostLevels::OnFocus(LTBOOL bFocus)
{
	if (bFocus)
	{
	
		m_sCampaignFile = g_pProfileMgr->GetCurrentCampaignFile();
		
		if(!CWinUtil::FileExist( m_sCampaignFile.c_str() ) )
		{
			//TODO handle more cleanly
			g_pLTClient->CPrint("Could not load campaign file %s.",  m_sCampaignFile.c_str() );
			m_sCampaignFile = "";
		}


		FillAvailList();

		LoadMissionList();

		if (!m_pSelMissions->GetNumControls())
		{
			MakeDefaultMissionList();
		}


		UpdateButtons();
        UpdateData(LTFALSE);

	}
	else
	{
		UpdateData();

		if (m_sCampaignFile.length())
		{
			SaveMissionList();
		}
		m_pAvailMissions->RemoveAll();
		m_pSelMissions->RemoveAll();
	}
	CBaseScreen::OnFocus(bFocus);

}

LTBOOL CScreenHostLevels::FillAvailList()
{
	// Sanity checks...

    if (!m_pAvailMissions) return(LTFALSE);


	for (int nMission = 0; nMission < g_pMissionButeMgr->GetNumMissions(); nMission++)
	{
		MISSION* pMission = g_pMissionButeMgr->GetMission(nMission);
		if (pMission)
		{
			CLTGUITextCtrl *pCtrl = CreateTextItem(LoadTempString(pMission->nNameId),CMD_ADD_LEVEL,LTNULL);
			pCtrl->SetFont(LTNULL,nListFontSize);
			pCtrl->SetParam1(nMission);
			m_pAvailMissions->AddControl(pCtrl);
		}
	}


    return (LTTRUE);
}

void CScreenHostLevels::LoadMissionList()
{
	// Sanity checks...

	if (!m_pSelMissions) return;

	char szString[256];
	CWinUtil::WinGetPrivateProfileString( "MissionList", "LoopMissions", "0", szString, ARRAY_LEN( szString ), m_sCampaignFile.c_str());

	m_bLoopMissions = ( atoi(szString) > 0 );

	char szKey[64];
	uint8 numMissions = 0;
	do
	{
		sprintf(szKey,"Mission%d",numMissions);
		CWinUtil::WinGetPrivateProfileString( "MissionList", szKey, "", szString, ARRAY_LEN( szString ), m_sCampaignFile.c_str());
		if (strlen(szString))
		{
			int nMissionId = atoi(szString);
			AddMissionToList(nMissionId);
			numMissions++;
		}

	} while (strlen(szString));

}

void CScreenHostLevels::MakeDefaultMissionList()
{
	// Sanity checks...

	if (!m_pAvailMissions) return;
	if (!m_pSelMissions) return;

	for (int nMission = 0; nMission < g_pMissionButeMgr->GetNumMissions(); nMission++)
	{
		AddMissionToList(nMission);
	}

}

void CScreenHostLevels::SaveMissionList()
{
	// Sanity checks...

	if (!m_pSelMissions) return;

	remove(m_sCampaignFile.c_str());

	char szString[256];
	char szNum[4];

	sprintf(szNum, "%d", (m_bLoopMissions ? 1 : 0) );
	CWinUtil::WinWritePrivateProfileString( "MissionList", "LoopMissions", szNum, m_sCampaignFile.c_str());

	CWinUtil::WinWritePrivateProfileString( "MissionList", "MissionSourceFile", g_pMissionButeMgr->GetAttributeFile(), m_sCampaignFile.c_str());

	for (int n = 0; n < m_pSelMissions->GetNumControls(); n++)
	{
		CLTGUITextCtrl *pCtrl = (CLTGUITextCtrl *)m_pSelMissions->GetControl(n);
		if (pCtrl)
		{
			sprintf(szString,"Mission%d",n);
			sprintf(szNum,"%d",pCtrl->GetParam1());
			CWinUtil::WinWritePrivateProfileString( "MissionList", szString, szNum, m_sCampaignFile.c_str());
		}
	}



}			
		



void CScreenHostLevels::UpdateButtons()
{
	if (m_pSelMissions->IsEnabled() && !m_pSelMissions->GetNumControls())
	{
		m_pSelMissions->Enable(LTFALSE);
		m_pRemove->Enable(LTTRUE);
		SetSelection(GetIndex(m_pRemove));
	}
	m_pAddAll->Enable( m_pSelMissions->GetNumControls() < MAX_GAME_LEVELS && m_pAvailMissions->GetNumControls() > 0);
	m_pRemoveAll->Enable(m_pSelMissions->GetNumControls() > 0);
		
}

void CScreenHostLevels::AddMissionToList(int nMissionId)
{
	// Sanity checks...

	if (!m_pSelMissions) return;
	if (m_pSelMissions->GetNumControls() == MAX_GAME_LEVELS) return;


	// Add the level to the list...

	MISSION *pMission = g_pMissionButeMgr->GetMission(nMissionId);

	if (pMission)
	{
		CLTGUITextCtrl *pCtrl = CreateTextItem(LoadTempString(pMission->nNameId),CMD_REMOVE_LEVEL,LTNULL);
		pCtrl->SetFont(LTNULL,nListFontSize);
		pCtrl->SetParam1(nMissionId);
		m_pSelMissions->AddControl(pCtrl);
	}
	else
	{
		ASSERT(!"Invalid mission id");
	}

}

