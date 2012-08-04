// ----------------------------------------------------------------------- //
//
// MODULE  : HUDScores.cpp
//
// PURPOSE : Implementation of CHUDScores to display player scores
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //
#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDScores.h"
#include "InterfaceMgr.h"
#include "ClientMultiplayerMgr.h"
#include "ResShared.h"

CHUDScores::CHUDScores()
{
	m_UpdateFlags = kHUDScores;
	m_eLevel = kHUDRenderText;

	m_fScale = 0.0f;
	m_nDraw = 0;
	m_bScreen = false;

}
	

LTBOOL CHUDScores::Init()
{
	m_bControlsInited = false;
	UpdateLayout();

	return LTTRUE;

}

void CHUDScores::Term()
{
	if (m_bControlsInited)
	{
		m_SingleFrame.Destroy();
		m_Server.Destroy();
		for (int team = 0; team < kNumTeams; team++)
		{
			m_Frame[team].Destroy();
			m_Team[team].Destroy();
			m_Rounds[team].Destroy();
			m_Header[team].Destroy();
			for (int i = 0; i < kMaxPlayers; i++)
				m_Columns[team][i].Destroy();
		}
	}

}

void CHUDScores::Show(bool bDraw, bool bScreen) 
{	
	if (bDraw)
		m_nDraw++;
	else if (m_nDraw > 0)
		m_nDraw--;

	if (bDraw)
	{
	
		if (strlen(g_pClientMultiplayerMgr->GetStartGameRequest( ).m_HostInfo.m_sName))
		{
			std::string str = g_pClientMultiplayerMgr->GetStartGameRequest( ).m_HostInfo.m_sName;
			if ( g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
			{
				str += " : ";
				str += g_pClientMultiplayerMgr->GetStartGameRequest( ).m_TCPAddress;
			}
			m_Server.SetString(str.c_str());
		}
		else
		{
			m_Server.SetString("");
		}

	}
	
	m_bScreen = bScreen; 
	Update();
}

void CHUDScores::Render()
{
	g_pChatMsgs->CanDraw( true );

	if (m_nDraw <= 0) return;
	
	g_pChatMsgs->CanDraw( false );

	if (m_bControlsInited)
	{
		if (!m_bScreen && !IsTeamGameType())
			m_SingleFrame.Render();

		for (int team = 0; team < kNumTeams; team++)
		{
			if (IsTeamGameType())
			{
				m_Frame[team].Render();
				m_Team[team].Render();
				m_Rounds[team].Render();
			}
			m_Header[team].Render();
			for (int i = 0; i < kMaxPlayers; i++)
				m_Columns[team][i].Render();
		}
		m_Server.Render();

	}


}

void CHUDScores::Update()
{
	if (m_nDraw <= 0) return;

	uint32 textCol = (m_bScreen ? m_nScreenTextColor : m_nTextColor);
	uint32 playerTextCol = (m_bScreen ? m_nScreenPlayerTextColor : m_nPlayerTextColor);

	float fScale = g_pInterfaceResMgr->GetXRatio();
	if (fScale != m_fScale)
	{
		m_fScale = fScale;
		m_Server.SetScale(fScale);
		m_SingleFrame.SetScale(fScale);
		for (int team = 0; team < kNumTeams; team++)
		{
			m_Team[team].SetScale(fScale);
			m_Rounds[team].SetScale(fScale);
			m_Header[team].SetScale(fScale);
			m_Frame[team].SetScale(fScale);

			for (int i = 0; i < kMaxPlayers; i++)
			{
				m_Columns[team][i].SetScale(fScale);
			}
		}

	}

	if (IsTeamGameType())
	{
		CUserProfile *pProfile = g_pProfileMgr->GetCurrentProfile();

		for( uint8 team = 0; team < kNumTeams; ++team )
		{
			CTeam* pTeam = CTeamMgr::Instance().GetTeam(team);
			if (!pTeam) continue;
			
			char szTmp[128];
			sprintf(szTmp,"%s : %d",pTeam->GetName(),pTeam->GetScore());
			m_Team[team].SetString(szTmp);

			sprintf(szTmp,"%s : %d", LoadTempString(IDS_ROUNDS), pTeam->GetRoundScore( ));
			m_Rounds[team].SetString(szTmp);
		
			m_Header[team].Show(LTTRUE);
		}

	}
	else
	{
		m_Team[0].SetString("");
		m_Rounds[0].SetString("");
		m_Header[1].Show(LTFALSE);
	}


	m_Server.SetColors(textCol,textCol,textCol);

	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;

	CLIENT_INFO* pCI = pCIMgr->GetFirstClient();
	int nTotal = 0;
	int count[kNumTeams] = {0,0};
	char szTmp[64] = "";
	uint16 nHeight[kNumTeams];
	nHeight[0] = m_Server.GetBaseHeight() + m_Header[0].GetBaseHeight() + m_Team[0].GetBaseHeight() + 24;
	nHeight[1] = m_Team[1].GetBaseHeight() + m_Header[1].GetBaseHeight() + 16;
	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID (&nLocalID);


	while (pCI && (nTotal < kMaxPlayers))
	{
		uint8 nTeam = 0;
		
		if (IsTeamGameType())
		{
			nTeam = pCI->nTeamID;
		}


		int ndx = count[nTeam];


		
		if (nTeam < kNumTeams)
		{
			sprintf( szTmp, "%s%s",pCI->sName.c_str( ), pCI->bIsAdmin ? "[*]" : "" );
			m_Columns[nTeam][ndx].GetPolyString(0)->SetText( szTmp );
			sprintf(szTmp,"%d",pCI->sScore.GetScore());
			m_Columns[nTeam][ndx].GetPolyString(1)->SetText(szTmp);

			sprintf(szTmp,"%d",pCI->sScore.GetFrags());
			m_Columns[nTeam][ndx].GetPolyString(2)->SetText(szTmp);

			sprintf(szTmp,"%d",pCI->sScore.GetTags());
			m_Columns[nTeam][ndx].GetPolyString(3)->SetText(szTmp);

			sprintf(szTmp,"%d",pCI->nPing);
			m_Columns[nTeam][ndx].GetPolyString(4)->SetText(szTmp);

			if (nLocalID == pCI->nID)
			{
				m_Columns[nTeam][ndx].SetColors(playerTextCol,playerTextCol,playerTextCol);
			}
			else
			{
				m_Columns[nTeam][ndx].SetColors(textCol,textCol,textCol);
			}
			
			m_Columns[nTeam][ndx].Show(LTTRUE);

			nHeight[nTeam] += m_Columns[nTeam][ndx].GetBaseHeight();
		}

		pCI = pCI->pNext;
		++count[nTeam];
	}


	
	for (int team = 0; team < kNumTeams; team++)
	{

		m_Team[team].SetColors(textCol,textCol,textCol);
		m_Rounds[team].SetColors(textCol,textCol,textCol);
		m_Header[team].SetColors(textCol,textCol,textCol);

		nHeight[team] += 16;
		m_Frame[team].SetSize(m_nFrameWidth,nHeight[team]);

		while (count[team] < kMaxPlayers)
		{
			m_Columns[team][count[team]].Show(LTFALSE);
			++count[team];
		}

		LTIntPt pos = m_BasePos;
		if (IsTeamGameType() && team > 0)
		{
			pos.y += nHeight[team-1] + 8;
			UpdateTeamPos(team,pos);
		}

	}
	m_SingleFrame.SetSize(m_nFrameWidth,nHeight[0]+8);


	
}


void CHUDScores::UpdateLayout()
{
	char *pTag = "Scores";
	m_BasePos = g_pLayoutMgr->GetPoint(pTag,"BasePos");
	uint8 nFont = (uint8)g_pLayoutMgr->GetInt(pTag,"Font");
	m_nBaseFontSize = (uint8)g_pLayoutMgr->GetInt(pTag,"FontSize");
	LTVector vCol = g_pLayoutMgr->GetVector(pTag,"TextColor");
	uint8 nR = (uint8)vCol.x;
	uint8 nG = (uint8)vCol.y;
	uint8 nB = (uint8)vCol.z;
	m_nTextColor =  SET_ARGB(0xFF,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector(pTag,"PlayerTextColor");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_nPlayerTextColor =  SET_ARGB(0xFF,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector(pTag,"ScreenPlayerTextColor");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_nScreenPlayerTextColor =  SET_ARGB(0xFF,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector(pTag,"ScreenTextColor");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	m_nScreenTextColor =  SET_ARGB(0xFF,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector(pTag,"FrameColor");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	float fAlpha = g_pLayoutMgr->GetFloat(pTag,"FrameAlpha");
	uint8 nA = (uint8)(255.0f * fAlpha);
	uint32 nFrameColor =  SET_ARGB(nA,nR,nG,nB);

	uint32 nTeamColors[kNumTeams] = {argbBlack,argbBlack};
	vCol = g_pLayoutMgr->GetVector(pTag,"Team1Color");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	nTeamColors[0]=  SET_ARGB(nA,nR,nG,nB);

	vCol = g_pLayoutMgr->GetVector(pTag,"Team2Color");
	nR = (uint8)vCol.x;
	nG = (uint8)vCol.y;
	nB = (uint8)vCol.z;
	nTeamColors[1]=  SET_ARGB(nA,nR,nG,nB);


	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	uint16 nCol1 = g_pLayoutMgr->GetInt(pTag,"NameColumn");
	uint16 nCol2 = g_pLayoutMgr->GetInt(pTag,"ScoreColumn");
	uint16 nCol3 = g_pLayoutMgr->GetInt(pTag,"KillColumn");
	uint16 nCol4 = g_pLayoutMgr->GetInt(pTag,"TagColumn");
	uint16 nCol5 = g_pLayoutMgr->GetInt(pTag,"PingColumn");

	m_nFrameWidth = (nCol1+nCol2+nCol3+nCol4+nCol5) + 16;

	if (!m_bControlsInited)
	{
		m_SingleFrame.Create(nFrameColor,m_nFrameWidth,32);
		for (int team = 0; team < kNumTeams; team++)
		{
			m_Frame[team].Create(nTeamColors[team],m_nFrameWidth,32);


			m_Team[team].Create("",0,0,pFont,(m_nBaseFontSize+4),NULL);
			m_Team[team].SetColors(m_nTextColor,m_nTextColor,m_nTextColor);

			m_Rounds[team].Create("",0,0,pFont,(m_nBaseFontSize+4),NULL);
			m_Rounds[team].SetColors(m_nTextColor,m_nTextColor,m_nTextColor);
			m_Rounds[team].GetString()->SetAlignmentH(CUI_HALIGN_RIGHT);

			m_Header[team].Create ( 0,0, pFont, (m_nBaseFontSize+4), NULL);

			m_Header[team].SetColors(m_nTextColor,m_nTextColor,m_nTextColor);
			m_Header[team].AddColumn(LoadTempString(IDS_SCORE_PLAYERS), (nCol1-8), LTFALSE);
			m_Header[team].AddColumn(LoadTempString(IDS_SCORE_SCORE), nCol2, LTFALSE);
			m_Header[team].AddColumn(LoadTempString(IDS_SCORE_KILLS), nCol3, LTFALSE);
			m_Header[team].AddColumn(LoadTempString(IDS_SCORE_TAGS), nCol4, LTFALSE);
			m_Header[team].AddColumn(LoadTempString(IDS_SCORE_PING), nCol5, LTFALSE);

			for (int i = 0; i < kMaxPlayers; i++)
			{
				m_Columns[team][i].Create ( 0,0, pFont, m_nBaseFontSize, NULL);
				m_Columns[team][i].SetColors(m_nTextColor,m_nTextColor,m_nTextColor);
				m_Columns[team][i].AddColumn("<player name>", nCol1, LTTRUE);
				m_Columns[team][i].AddColumn("<Score>", nCol2, LTTRUE);
				m_Columns[team][i].AddColumn("<Frags>", nCol3, LTTRUE);
				m_Columns[team][i].AddColumn("<Tags>", nCol4, LTTRUE);
				m_Columns[team][i].AddColumn("<Ping>", nCol5, LTTRUE);
				m_Columns[team][i].Show(LTFALSE);

			}
		}

		m_Server.Create("",0,0,pFont,(m_nBaseFontSize+4),NULL);
		m_Server.SetColors(m_nTextColor,m_nTextColor,m_nTextColor);

		LTIntPt framePos = m_BasePos;
		framePos.x -= 8;
		framePos.y -= 4;
		m_SingleFrame.SetBasePos(framePos);
		UpdateTeamPos(0,m_BasePos);


        m_bControlsInited = true;        
	}

}

void CHUDScores::UpdateTeamPos(uint8 nTeam, LTIntPt pos)
{
	LTIntPt framePos = pos;
	framePos.x -= 8;
	framePos.y -= 4;
	m_Frame[nTeam].SetBasePos(framePos);

	if (nTeam == 0)
	{
		m_Server.SetBasePos(pos);
		pos.y += m_nBaseFontSize + 16;
	}

	m_Team[nTeam].SetBasePos(pos);

	LTIntPt roundPos = pos;
	roundPos.x += m_nFrameWidth-24;
	m_Rounds[nTeam].SetBasePos(roundPos);

	pos.y += m_nBaseFontSize;

	m_Header[nTeam].SetBasePos(pos);
	pos.y += m_nBaseFontSize + 8;


	for (int i = 0; i < kMaxPlayers; i++)
	{
		m_Columns[nTeam][i].SetBasePos(pos);
		pos.y += m_nBaseFontSize;
	}
}