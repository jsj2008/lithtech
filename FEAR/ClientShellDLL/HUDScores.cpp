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
#include "ClientConnectionMgr.h"
#include "HUDMessageQueue.h"
#include "sys/win/mpstrconv.h"
#include "GameModeMgr.h"
#include "CharacterFX.h"

enum eColumns
{
	eColumnName,
	eColumnScore,
	eColumnKill,
	eColumnDeath,
	eColumnObjective,
	eColumnTK,
	eColumnSuicide,
	eColumnPing,
	kNumColumns
};

CHUDScores::CHUDScores()
{
	m_UpdateFlags = kHUDScores;
	m_eLevel = kHUDRenderText;

	m_bDraw = false;
	m_bScreen = false;
	m_bControlsInited = false;
	m_bInitialized = false;
}
	

bool CHUDScores::Init()
{
	Term( );
	m_bInitialized = true;
	UpdateLayout( );
	return true;
}

void CHUDScores::Term()
{
	if (m_bControlsInited)
	{
		m_SingleFrame.Destroy();
		m_Server.Destroy();
		m_RoundInfo.Destroy();
		for (int team = 0; team < kNumTeams; team++)
		{
			m_Frame[team].Destroy();
			m_Team[team].Destroy();
			m_Rounds[team].Destroy();
			m_Header[team].Destroy();
			for (int i = 0; i < kMaxPlayers; i++)
				m_Columns[team][i].Destroy();
		}

		m_bControlsInited = false;
	}

	m_bInitialized = false;
}

void CHUDScores::Show(bool bDraw, bool bScreen) 
{	
	if( !m_bInitialized )
		return;

	if (!m_bDraw)
		m_bFirstScreenUpdate = bScreen;
	m_bDraw = bDraw;

	GameModeMgr& gameModeMgr = GameModeMgr::Instance();

	if (bDraw)
	{
		wchar_t wszTmp[64];

		for (int team = 0; team < kNumTeams; team++)
		{
			FormatString("IDS_SCORE_KILLS",wszTmp,LTARRAYSIZE(wszTmp),(uint32)gameModeMgr.m_grnFragScorePlayer);
			m_Header[team].SetString( eColumnKill, wszTmp );

			FormatString("IDS_SCORE_DEATHS",wszTmp,LTARRAYSIZE(wszTmp),(int32)gameModeMgr.m_grnDeathScorePlayer);
			m_Header[team].SetString( eColumnDeath, wszTmp );

			if (GameModeMgr::Instance( ).m_grbUseTeams)
			{
				FormatString("IDS_SCORE_TK",wszTmp,LTARRAYSIZE(wszTmp),(int32)gameModeMgr.m_grnTKScore);
				m_Header[team].SetString( eColumnTK, wszTmp );
				m_Header[team].ShowColumn(eColumnTK,true);
			}
			else
			{
				m_Header[team].ShowColumn(eColumnTK,false);
			}

			int32 n = GameModeMgr::Instance( ).m_grnSuicideScorePlayer;
			FormatString("IDS_SCORE_SUICIDE",wszTmp,LTARRAYSIZE(wszTmp),n);
			m_Header[team].SetString( eColumnSuicide, wszTmp );
		}
	}
	
	m_bScreen = bScreen; 
	Update();
}

void CHUDScores::Render()
{
	if( !m_bInitialized )
		return;

	if ( !m_bDraw ) 
		return;
	
	if (!m_bScreen)
	{
		LTPoly_G4 back;
		DrawPrimSetRGBA(back, SET_ARGB(0xB0,0,0,0));
		DrawPrimSetXYWH(back,0.0f,0.0f,(float)g_pInterfaceResMgr->GetScreenWidth(),(float)g_pInterfaceResMgr->GetScreenHeight());
		g_pDrawPrim->DrawPrim(&back,1);
	}

	if (m_bControlsInited)
	{
		if (!m_bScreen && !GameModeMgr::Instance( ).m_grbUseTeams)
			m_SingleFrame.Render();

		for (int team = 0; team < kNumTeams; team++)
		{
			if (GameModeMgr::Instance( ).m_grbUseTeams)
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
		m_RoundInfo.Render();

	}


}

void CHUDScores::Update()
{
	if( !m_bInitialized )
		return;

	if( !m_bDraw )
		return;

//	uint32 textCol = (m_bScreen ? m_cScreenTextColor : m_cTextColor);
//	uint32 playerTextCol = (m_bScreen ? m_cScreenPlayerTextColor : m_cPlayerTextColor);

	//for the screen mode scoreboard, don't update the text once we've drawn it
	if (m_bScreen && !m_bFirstScreenUpdate)
		return;

	m_bFirstScreenUpdate = false;

	if (GameModeMgr::Instance( ).m_grbUseTeams)
	{
		for( uint8 nTeamNum = 0; nTeamNum < kNumTeams; ++nTeamNum )
		{
			uint8 team;
			if (g_pInterfaceMgr->GetClientInfoMgr()->IsLocalTeam(nTeamNum))
				team = 0;
			else
				team = 1;

			CTeam* pTeam = CTeamMgr::Instance().GetTeam(nTeamNum);
			if (!pTeam) continue;
			
			wchar_t wszTmp[128];
			LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"%s : %d",pTeam->GetName(),pTeam->GetScore());
			m_Team[team].SetString(wszTmp);

			LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"%s : %d", LoadString("IDS_ROUNDS"), pTeam->GetRoundScore( ));
			m_Rounds[team].SetString(wszTmp);
		
			m_Header[team].Show(true);
		}

	}
	else
	{
		m_Team[0].SetString(L"");
		m_Rounds[0].SetString(L"");
		m_Header[1].Show(false);
	}


	m_Server.SetColor(m_cTextColor);

	if ( !GameModeMgr::Instance( ).m_grwsSessionName.GetValue().empty())
	{
		std::wstring wstr = GameModeMgr::Instance( ).m_grwsSessionName;
		if ( g_pClientConnectionMgr->IsConnectedToRemoteServer( ))
		{
			wstr += L" : ";
			wstr += MPA2W(g_pClientConnectionMgr->GetStartGameRequest( ).m_TCPAddress).c_str();
		}

		m_Server.SetString(wstr.c_str());
	}
	else
	{
		m_Server.SetString(L"");
	}

	// Update the round counter.
	m_RoundInfo.SetColor(m_cTextColor);
	wchar_t wszRound[32];
	uint8 nCurrentRound = g_pClientConnectionMgr ? g_pClientConnectionMgr->GetCurrentRound() : 0;
	uint8 nNumRounds = GameModeMgr::Instance( ).m_grnNumRounds;
	FormatString( "HUD_SCORES_ROUNDINFO", wszRound, LTARRAYSIZE( wszRound ), nCurrentRound + 1, nNumRounds );
	m_RoundInfo.SetString( wszRound );

	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;

	CLIENT_INFO* pCI = pCIMgr->GetFirstClient();
	int nTotal = 0;
	int count[kNumTeams] = {0,0};
	wchar_t wszTmp[64] = L"";
	uint32 nHeight[kNumTeams];
	nHeight[0] = m_Server.GetBaseHeight() + m_Header[0].GetBaseHeight() + m_Team[0].GetBaseHeight() + 24;
	nHeight[1] = m_Team[1].GetBaseHeight() + m_Header[1].GetBaseHeight() + 16;
	uint32 nLocalID = 0;
	g_pLTClient->GetLocalClientID (&nLocalID);


	while (pCI && (nTotal < kMaxPlayers))
	{
		uint8 nTeam = 0;
		CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromClientID(pCI->nID);
		
		if (GameModeMgr::Instance( ).m_grbUseTeams)
		{
			if (g_pInterfaceMgr->GetClientInfoMgr()->IsLocalTeam(pCI->nTeamID))
				nTeam = 0;
			else
				nTeam = 1;
		}


		int ndx = count[nTeam];


		
		if (nTeam < kNumTeams)
		{
			LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"%s%s",pCI->sName.c_str( ), pCI->bIsAdmin ? L"[*]" : L"" );
			m_Columns[nTeam][ndx].SetString( eColumnName, wszTmp );

			LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"%d",pCI->sScore.GetScore());
			m_Columns[nTeam][ndx].SetString( eColumnScore, wszTmp );

			LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"%d",pCI->sScore.GetEventCount(CPlayerScore::eKill));
			m_Columns[nTeam][ndx].SetString( eColumnKill, wszTmp );

			LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"%d",pCI->sScore.GetEventCount(CPlayerScore::eDeath));
			m_Columns[nTeam][ndx].SetString( eColumnDeath, wszTmp );

			LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"%d",pCI->sScore.GetEventCount(CPlayerScore::eObjective));
			m_Columns[nTeam][ndx].SetString( eColumnObjective, wszTmp );

			if (GameModeMgr::Instance( ).m_grbUseTeams)
			{
				LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"%d",pCI->sScore.GetEventCount(CPlayerScore::eTeamKill));
				m_Columns[nTeam][ndx].SetString( eColumnTK, wszTmp );
				m_Columns[nTeam][ndx].ShowColumn(eColumnTK, true);
			}
			else
			{
				m_Columns[nTeam][ndx].ShowColumn(eColumnTK, false);
			}


			LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"%d",pCI->sScore.GetEventCount(CPlayerScore::eSuicide));
			m_Columns[nTeam][ndx].SetString( eColumnSuicide, wszTmp );

			LTSNPrintF(wszTmp,LTARRAYSIZE(wszTmp),L"%d",pCI->nPing);
			m_Columns[nTeam][ndx].SetString( eColumnPing, wszTmp );

			if (nLocalID == pCI->nID)
			{
				if (!pCharacter || pCharacter->IsPlayerDead() || pCharacter->m_cs.bIsSpectating )
					m_Columns[nTeam][ndx].SetColor(m_cPlayerDeadColor);
				else
					m_Columns[nTeam][ndx].SetColor(m_cPlayerTextColor);
			}
			else
			{
				if (!pCharacter || pCharacter->IsPlayerDead() || pCharacter->m_cs.bIsSpectating )
					m_Columns[nTeam][ndx].SetColor(m_cDeadColor);
				else
					m_Columns[nTeam][ndx].SetColor(m_cTextColor);
			}
			
			m_Columns[nTeam][ndx].Show(true);

			nHeight[nTeam] += m_Columns[nTeam][ndx].GetBaseHeight();
		}

		pCI = pCI->pNext;
		++count[nTeam];
	}


	
	for (uint8 team = 0; team < kNumTeams; team++)
	{

		m_Team[team].SetColor(m_cTextColor);
		m_Rounds[team].SetColor(m_cTextColor);
		m_Header[team].SetColor(m_cTextColor);

		nHeight[team] += 16;
		m_Frame[team].SetSize(LTVector2n(m_nFrameWidth,nHeight[team]));

		while (count[team] < kMaxPlayers)
		{
			m_Columns[team][count[team]].Show(false);
			++count[team];
		}

		LTVector2n pos = m_vBasePos;
		g_pInterfaceResMgr->ScaleScreenPos(pos);
		if (GameModeMgr::Instance( ).m_grbUseTeams && team > 0)
		{
			pos.y += nHeight[team-1] + 8;
		}
		UpdateTeamPos(team,pos);

	}
	m_SingleFrame.SetSize(LTVector2n(m_nFrameWidth,nHeight[0]+8));
}


void CHUDScores::ScaleChanged()
{
	if( !m_bInitialized )
		return;

	//force the update to recalc pos
	bool bDraw = m_bDraw;
	m_bDraw = true;
	Update();
	m_bDraw = bDraw;

	LTVector2n framePos = m_vBasePos;
	g_pInterfaceResMgr->ScaleScreenPos(framePos);
	framePos.x -= 8;
	framePos.y -= 4;
	m_SingleFrame.SetBasePos(framePos);

}



void CHUDScores::UpdateLayout( )
{
	if( !m_bInitialized )
		return;

	//if we haven't initialized our layout info
	if (!m_hLayout)
	{
		m_hLayout = g_pLayoutDB->GetHUDRecord("HUDScores");
	}

	CHUDItem::UpdateLayout();

	CFontInfo headerFont = m_sTextFont;
	headerFont.m_nHeight = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,kNumColumns);
	headerFont.m_nStyle = CFontInfo::kStyle_Bold;

	m_cPlayerTextColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,0);
	m_cDeadColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,1);
	m_cPlayerDeadColor = g_pLayoutDB->GetColor(m_hLayout,LDB_HUDAddColor,2);

	for (uint8 nTeam = 0; nTeam < kNumTeams; nTeam++)
	{
		m_hFrameTexture[nTeam].Load( g_pLayoutDB->GetString(m_hLayout,LDB_HUDAddTex,nTeam));
	}

	uint32 nCol[kNumColumns];

	m_nFrameWidth = 16;
	for (uint8 n = 0; n < kNumColumns; ++n)
	{
		nCol[n] = g_pLayoutDB->GetInt32(m_hLayout,LDB_HUDAddInt,n);
		m_nFrameWidth += nCol[n];
	}

	if( !m_bControlsInited )
	{
		CLTGUICtrl_create cs;
		cs.rnBaseRect.Right() = m_nFrameWidth;
		cs.rnBaseRect.Bottom() = 32;
		m_SingleFrame.Create(m_hIconTexture,cs);
		for (int team = 0; team < kNumTeams; team++)
		{
			m_Frame[team].Create(m_hFrameTexture[team],cs);

			CLTGUICtrl_create tcs;
			tcs.rnBaseRect.Right() = nCol[0];
			tcs.rnBaseRect.Bottom() = headerFont.m_nHeight;
			m_Team[team].Create(L"",headerFont,tcs);
			m_Team[team].SetColor(m_cTextColor);

			m_Rounds[team].Create(L"",headerFont, tcs);
			m_Rounds[team].SetColor(m_cTextColor);
			m_Rounds[team].SetAlignment(kRight);

			m_Header[team].Create(headerFont, tcs);

			m_Header[team].SetColor(m_cTextColor);
			m_Header[team].AddColumn(LoadString("IDS_SCORE_PLAYERS"), (nCol[eColumnName]-8), false);
			m_Header[team].AddColumn(LoadString("IDS_SCORE_SCORE"), nCol[eColumnScore], false);
			m_Header[team].AddColumn(LoadString("IDS_SCORE_KILLS"), nCol[eColumnKill], false);
			m_Header[team].AddColumn(LoadString("IDS_SCORE_DEATHS"), nCol[eColumnDeath], false);
			m_Header[team].AddColumn(LoadString("IDS_SCORE_OBJECTIVE"), nCol[eColumnObjective], false);
			m_Header[team].AddColumn(LoadString("IDS_SCORE_TK"), nCol[eColumnTK], false);
			m_Header[team].AddColumn(LoadString("IDS_SCORE_SUICIDE"), nCol[eColumnSuicide], false);
			m_Header[team].AddColumn(LoadString("IDS_SCORE_PING"), nCol[eColumnPing], false);

			for (int i = 0; i < kMaxPlayers; i++)
			{
				tcs.rnBaseRect.Bottom() = m_sTextFont.m_nHeight;
				m_Columns[team][i].Create ( m_sTextFont,tcs);
				m_Columns[team][i].SetColor(m_cTextColor);
				m_Columns[team][i].AddColumn(L"<player name>", nCol[eColumnName], true);
				m_Columns[team][i].AddColumn(L"<Score>", nCol[eColumnScore], true);
				m_Columns[team][i].AddColumn(L"<Frags>", nCol[eColumnKill], true);
				m_Columns[team][i].AddColumn(L"<Deaths>", nCol[eColumnDeath], true);
				m_Columns[team][i].AddColumn(L"<Objective>", nCol[eColumnObjective], true);
				m_Columns[team][i].AddColumn(L"<Team Kills>", nCol[eColumnTK], true);
				m_Columns[team][i].AddColumn(L"<Suicides>", nCol[eColumnSuicide], true);
				m_Columns[team][i].AddColumn(L"<Ping>", nCol[eColumnPing], true);
				m_Columns[team][i].Show(false);

			}
		}

		cs.rnBaseRect.Right() = nCol[0];
		cs.rnBaseRect.Bottom() = headerFont.m_nHeight;
		m_Server.Create(L"",headerFont,cs);
		m_Server.SetColor(m_cTextColor);

		cs.rnBaseRect.Right() = m_nFrameWidth - 16;
		cs.rnBaseRect.Bottom() = headerFont.m_nHeight;
		m_RoundInfo.Create(L"",headerFont,cs);
		m_RoundInfo.SetColor(m_cTextColor);
		m_RoundInfo.SetAlignment( kRight );

		LTVector2n framePos = m_vBasePos;
		g_pInterfaceResMgr->ScaleScreenPos(framePos);
		framePos.x -= 8;
		framePos.y -= 4;
		m_SingleFrame.SetBasePos(framePos);

		LTVector2n pos = m_vBasePos;
		g_pInterfaceResMgr->ScaleScreenPos(pos);
		UpdateTeamPos(0,pos);

		m_bControlsInited = true;
	}
}

void CHUDScores::UpdateTeamPos(uint8 nTeam, LTVector2n pos)
{
	LTVector2n framePos = pos;
	framePos.x -= 8;
	framePos.y -= 4;
	m_Frame[nTeam].SetBasePos(framePos);

	if (nTeam == 0)
	{
		m_Server.SetBasePos(pos);
		m_RoundInfo.SetBasePos( pos );
		pos.y += m_sTextFont.m_nHeight + 16;
	}

	m_Team[nTeam].SetBasePos(pos);

	LTVector2n roundPos = pos;
	roundPos.x += m_nFrameWidth-24;
	roundPos.x -= m_Rounds[nTeam].GetBaseWidth();
	m_Rounds[nTeam].SetBasePos(roundPos);

	pos.y += m_sTextFont.m_nHeight;

	m_Header[nTeam].SetBasePos(pos);
	pos.y += m_sTextFont.m_nHeight + 8;


	for (int i = 0; i < kMaxPlayers; i++)
	{
		m_Columns[nTeam][i].SetBasePos(pos);
		pos.y += m_sTextFont.m_nHeight;
	}
}

void CHUDScores::SetBasePos( LTVector2n vBasePos )
{
	CHUDItem::SetBasePos( vBasePos );
	ScaleChanged();
}

