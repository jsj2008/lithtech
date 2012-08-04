// ----------------------------------------------------------------------- //
//
// MODULE  : TeamBalancer.cpp
//
// PURPOSE : Implementation of class handling automatic team balancing
//
// CREATED : 06/06/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "TeamBalancer.h"
#include "GameModeMgr.h"
#include "ServerConnectionMgr.h"
#include "TeamMgr.h"
#include "PlayerObj.h"

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::Instance
//
//	PURPOSE:	Retrieve the singleton instance
//
// --------------------------------------------------------------------------- //
TeamBalancer& TeamBalancer::Instance()
{
	static TeamBalancer instance;
	return instance;
}

//constructor
TeamBalancer::TeamBalancer( ) :
	m_eTeamSizeBalance(eBalanceFrequency_Never),
	m_eTeamScoreBalance(eBalanceFrequency_Never),
	m_nRoundsSinceScoreBalance(0)
{
}

//destructor
TeamBalancer::~TeamBalancer()
{
	m_delegateBeginRound.Detach();
	m_delegateEndRound.Detach();
	m_delegateEndMap.Detach();
	m_delegateGameRulesUpdated.Detach();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::Init
//
//	PURPOSE:	initialization on server startup
//
// --------------------------------------------------------------------------- //
void TeamBalancer::Init()
{
	m_delegateBeginRound.Attach(this,g_pServerMissionMgr,g_pServerMissionMgr->BeginRound);
	m_delegateEndRound.Attach(this,g_pServerMissionMgr,g_pServerMissionMgr->EndRound);
	m_delegateEndMap.Attach(this,g_pServerMissionMgr,g_pServerMissionMgr->EndMap);
	m_delegateGameRulesUpdated.Attach(this,g_pServerMissionMgr,g_pServerMissionMgr->GameRulesUpdated);

	m_tmrRound.SetEngineTimer( SimulationTimer::Instance( ));

	if( !GameModeMgr::Instance().m_grbUseTeams) return;

	m_eTeamSizeBalance = GetBalanceFrequency((const char*)GameModeMgr::Instance( ).m_greTeamSizeBalancing);
	if (m_eTeamSizeBalance == eBalanceFrequency_Invalid)
	{
		LTERROR_PARAM1("Unknown value for TeamSizeBalancing: %s",(const char*)GameModeMgr::Instance( ).m_greTeamSizeBalancing);
		m_eTeamSizeBalance = eBalanceFrequency_Never;
	}
	m_eTeamScoreBalance = GetBalanceFrequency((const char*)GameModeMgr::Instance( ).m_greTeamScoreBalancing);
	if (m_eTeamScoreBalance == eBalanceFrequency_Invalid)
	{
		LTERROR_PARAM1("Unknown value for TeamScoreBalancing: %s",(const char*)GameModeMgr::Instance( ).m_greTeamScoreBalancing);
		m_eTeamScoreBalance = eBalanceFrequency_Never;
	}

	m_nRoundsSinceScoreBalance = 0;
	m_nTeamScores[0] = 0;
	m_nTeamScores[1] = 0;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::Update
//
//	PURPOSE:	handle update ping
//
// --------------------------------------------------------------------------- //
void TeamBalancer::Update()
{
	if( !GameModeMgr::Instance().m_grbUseTeams) return;
	if (m_tmrRound.IsStarted() && m_tmrRound.IsTimedOut())
	{
		BalanceTeamSize();
		SetRoundTimer();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::OnBeginRound
//
//	PURPOSE:	handle BeginRound event
//
// --------------------------------------------------------------------------- //
void TeamBalancer::OnBeginRound()
{
	if( !GameModeMgr::Instance().m_grbUseTeams) return;
	SetRoundTimer();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::OnEndRound
//
//	PURPOSE:	handle EndRound event
//
// --------------------------------------------------------------------------- //
void TeamBalancer::OnEndRound()
{
	if( !GameModeMgr::Instance().m_grbUseTeams) return;
		

	bool bDidScoreBalance = false;
	if (m_eTeamScoreBalance != eBalanceFrequency_Never)
	{
		++m_nRoundsSinceScoreBalance;

		//if we're doing score balancing, add the player scores to the history
		ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
		ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
		for( ; iter != gameClientDataList.end( ); iter++ )
		{
			GameClientData* pGameClientData = *iter;
			if( !pGameClientData->GetClient( ))
				continue;
			uint32 nID =  g_pLTServer->GetClientID(pGameClientData->GetClient( ));
			int32 nScore = 0;
			CPlayerScore* pScore = pGameClientData->GetPlayerScore(); 
			if (pScore)
			{
				nScore = pScore->GetScore();
			}
			AddPlayerScore(nID,nScore);
		}

		for (uint8 nTeam = 0;nTeam < 2;++nTeam)
		{
			CTeam* pTeam = CTeamMgr::Instance( ).GetTeam(nTeam);
			if (pTeam)
			{
				m_nTeamScores[nTeam] += pTeam->GetScore();
			}
		}

		//is it time to rebalance?
		bool bDoBalance = false;
		switch(m_eTeamScoreBalance)
		{
		case eBalanceFrequency_OneRound:
			bDoBalance = true;
			break;
		case eBalanceFrequency_TwoRounds:
			bDoBalance = (m_nRoundsSinceScoreBalance >= 2);
			break;
		case eBalanceFrequency_ThreeRounds:
			bDoBalance = (m_nRoundsSinceScoreBalance >= 3);
			break;
		}
		if (bDoBalance)
		{
			bDidScoreBalance = BalanceTeamScore();
		}
	}

	//if we didn't score balance, see if we should size balance
	if (!bDidScoreBalance)
	{
		if (m_eTeamSizeBalance == eBalanceFrequency_QuarterRound ||
			m_eTeamSizeBalance == eBalanceFrequency_HalfRound ||
			m_eTeamSizeBalance == eBalanceFrequency_OneRound
			)
		{
			BalanceTeamSize();
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::OnEndMap
//
//	PURPOSE:	handle EndMap event
//
// --------------------------------------------------------------------------- //
void TeamBalancer::OnEndMap()
{
	if( !GameModeMgr::Instance().m_grbUseTeams) return;
	bool bDidScoreBalance = false;
	if (m_eTeamScoreBalance == eBalanceFrequency_MapChange)
	{
		bDidScoreBalance = BalanceTeamScore();
	}
	if (!bDidScoreBalance && m_eTeamSizeBalance == eBalanceFrequency_MapChange)
	{
		BalanceTeamSize();
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::OnGameRulesUpdated
//
//	PURPOSE:	handle GameRulesUpdated event
//
// --------------------------------------------------------------------------- //
void TeamBalancer::OnGameRulesUpdated()
{
	if( !GameModeMgr::Instance().m_grbUseTeams) return;
	m_eTeamSizeBalance = GetBalanceFrequency((const char*)GameModeMgr::Instance( ).m_greTeamSizeBalancing);
	if (m_eTeamSizeBalance == eBalanceFrequency_Invalid)
	{
		LTERROR_PARAM1("Unknown value for TeamSizeBalancing: %s",(const char*)GameModeMgr::Instance( ).m_greTeamSizeBalancing);
		m_eTeamSizeBalance = eBalanceFrequency_Never;
	}
	m_eTeamScoreBalance = GetBalanceFrequency((const char*)GameModeMgr::Instance( ).m_greTeamScoreBalancing);
	if (m_eTeamScoreBalance == eBalanceFrequency_Invalid)
	{
		LTERROR_PARAM1("Unknown value for TeamScoreBalancing: %s",(const char*)GameModeMgr::Instance( ).m_greTeamScoreBalancing);
		m_eTeamScoreBalance = eBalanceFrequency_Never;
	}

	if (m_eTeamScoreBalance == eBalanceFrequency_Never)
	{
		m_vecPlayerScores.clear();
		m_nTeamScores[0] = 0;
		m_nTeamScores[1] = 0;
	}
	SetRoundTimer();
}




// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::GetBalanceFrequency
//
//	PURPOSE:	get frequency enum from string
//
// --------------------------------------------------------------------------- //
TeamBalancer::BalanceFrequency TeamBalancer::GetBalanceFrequency(const char* pszString)
{
	struct StringToEnum
	{
		CParsedMsg::CToken	m_tokString;
		BalanceFrequency	m_eValue;
	};

	static StringToEnum table[] = 
	{
		{ "Never", eBalanceFrequency_Never },
		{ "QuarterRound", eBalanceFrequency_QuarterRound },
		{ "HalfRound", eBalanceFrequency_HalfRound },
		{ "OneRound", eBalanceFrequency_OneRound },
		{ "TwoRounds", eBalanceFrequency_TwoRounds },
		{ "ThreeRounds", eBalanceFrequency_ThreeRounds },
		{ "MapChange", eBalanceFrequency_MapChange },
	};
	static uint32 nTableCount = LTARRAYSIZE( table );

	CParsedMsg::CToken tokValue = pszString;
	uint32 i;
	for( i = 0; i < nTableCount; i++ )	
	{
		if( table[i].m_tokString == tokValue )
			break;
	}
	//enum not found, return an invalid value
	if( i == nTableCount )
	{
		i = 0;
		return eBalanceFrequency_Invalid;
	}
	
	//found it, return the proper value
	return table[i].m_eValue;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::SetRoundTimer
//
//	PURPOSE:	set timer based on current game rules
//
// --------------------------------------------------------------------------- //
void TeamBalancer::SetRoundTimer()
{
	//no time limit, no timer
	if( GameModeMgr::Instance( ).m_grnTimeLimit <= 0 )
	{
		m_tmrRound.Stop();
		return;
	}

	//timer is only needed for half and quarter round frequencies
	double fPeriod = 0.0;
	switch(m_eTeamSizeBalance)
	{
	case eBalanceFrequency_QuarterRound:
		//period is 15 seconds for every minute of total duration
		fPeriod = (double)GameModeMgr::Instance( ).m_grnTimeLimit * 15.0;
		break;
	case eBalanceFrequency_HalfRound:
		//period is 30 seconds for every minute of total duration
		fPeriod = (double)GameModeMgr::Instance( ).m_grnTimeLimit * 30.0;
		break;
	default:
		m_tmrRound.Stop();
		return;
	}

	double fTimeInRound = g_pServerMissionMgr->GetTimeInRound();
	double fTimeLeft = ((double)GameModeMgr::Instance( ).m_grnTimeLimit * 60.0f - fTimeInRound);

	//not enough time left before the round ends
	if (fTimeLeft < fPeriod)
	{
		m_tmrRound.Stop();
		return;
	}

	//calculate the time until the next period...
	while (fTimeInRound >= fPeriod)
	{
		fTimeInRound -= fPeriod;
	}

	//set the timer
	m_tmrRound.Start(fPeriod-fTimeInRound);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::BalanceTeamSize
//
//	PURPOSE:	balance team sizes by moving the lowest score player(s) from the larger team
//
// --------------------------------------------------------------------------- //
bool TeamBalancer::BalanceTeamSize()
{
	bool bMovedPlayer = false;
	uint32 nTeamSizes[2] = {0,0};
	for (uint8 nTeam = 0;nTeam < 2;++nTeam)
	{
		CTeam* pTeam = CTeamMgr::Instance( ).GetTeam(nTeam);
		if (pTeam)
		{
			nTeamSizes[nTeam] = pTeam->GetNumPlayers();
		}
	}

	//check to see if one team is larger by at least 2 people
	if (LTDIFF(nTeamSizes[0],nTeamSizes[1]) > 1)
	{
		uint8 nLargerTeam = ((nTeamSizes[0] < nTeamSizes[1]) ? 1 : 0);
		uint8 nSmallerTeam = (nLargerTeam + 1) % MAX_TEAMS;

		//make a list of players on the team
		PlayerScoreHistoryArray vecScores;
		ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
		ServerConnectionMgr::GameClientDataList::iterator iter = gameClientDataList.begin( );
		for( ; iter != gameClientDataList.end( ); iter++ )
		{
			GameClientData* pGameClientData = *iter;
			if( !pGameClientData->GetClient( ))
				continue;

			//if player is on the larger team, add him to our list
			if (nLargerTeam == pGameClientData->GetLastTeamId())
			{
				PlayerScoreHistory pairScore;
				pairScore.first = g_pLTServer->GetClientID(pGameClientData->GetClient( ));
				pairScore.second = 0;
				CPlayerScore* pScore = pGameClientData->GetPlayerScore(); 
				if (pScore)
				{
					pairScore.second = pScore->GetScore();
				}
				vecScores.push_back(pairScore);
			}
		}
		//sort the player list from highest to lowest
		std::sort(vecScores.begin(),vecScores.end(),PlayerScoreHistoryCompare());

		//while the teams are still unbalanced move lowest scoring players from the larger team to the smaller
		PlayerScoreHistoryArray::reverse_iterator scoreIter = vecScores.rbegin();
		while (LTDIFF(nTeamSizes[nLargerTeam],nTeamSizes[nSmallerTeam]) > 1 && scoreIter != vecScores.rend())
		{
			
			uint32 nID = scoreIter->first;
			GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientDataByClientId(nID);

			if (pGameClientData)
			{
				pGameClientData->SetRequestedTeam(nSmallerTeam);
				CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(pGameClientData->GetPlayer());
				if (pPlayer)
				{
					pPlayer->HandleTeamSwitchRequest();
				}
				//shrink the larger team and grow the smaller
				nTeamSizes[nLargerTeam]--;
				nTeamSizes[nSmallerTeam]++;

				bMovedPlayer = true;
			}
			scoreIter++;
		}
	}

	if (bMovedPlayer)
	{
		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_EVENT);
		cClientMsg.Writeuint8(kPEAutobalance);
		cClientMsg.Writebool( false ); //didn't do score balancing
		g_pLTServer->SendToClient(cClientMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}

	return bMovedPlayer;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::BalanceTeamScore
//
//	PURPOSE:	balance teams by using playground sorting based on player scores
//
// --------------------------------------------------------------------------- //
bool TeamBalancer::BalanceTeamScore()
{

	m_nRoundsSinceScoreBalance = 0;

	bool bDoSort = false;


	//figure out the score difference
	int32 nDiff = LTDIFF(m_nTeamScores[0],m_nTeamScores[1]);
	if (nDiff > 0)
	{
		ServerConnectionMgr::GameClientDataList& gameClientDataList = ServerConnectionMgr::Instance( ).GetGameClientDataList( );
		//for 2 or fewer players don't bother doing a complicated sort, just balance the team sizes if needed
//		if (gameClientDataList.size() <= 2)
//		{
//			BalanceTeamSize();
//		}
//		else
		{
			int32 nLower = LTMIN(m_nTeamScores[0],m_nTeamScores[1]);

			//if the lower score is 0, treat it as one for these calculations to avoid divide by 0 errors
			if (nLower == 0)
			{
				nLower = 1;
			}

			//how much did the winners win by? This formula 1+(diff/lower) is used instead of
			//	(high/low) in order to handle 0 and negative high score more cleanly
			float fRatio = 1.0f + (float)nDiff/ (float)LTAbs(nLower);
			
			//see if the margin was wide enough to trigger a shuffle
			bDoSort = (fRatio >= GameModeMgr::Instance().m_grfTeamScoreBalancingPercent);
		}

	}

	if (bDoSort)
	{
		PlaygroundSort();

		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_EVENT);
		cClientMsg.Writeuint8(kPEAutobalance);
		cClientMsg.Writebool( true ); //did score balancing
		g_pLTServer->SendToClient(cClientMsg.Read(), NULL, MESSAGE_GUARANTEED);

	}

	//chear the history, whether or not we actually sorted
	m_nTeamScores[0] = 0;
	m_nTeamScores[1] = 0;
	m_vecPlayerScores.clear();

	return bDoSort;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::PlaygroundSort
//
//	PURPOSE:	sort the players onto teams that are roughly equal given their scoring history
//					the first will go onto team 0, 2nd and 3rd to Team 1, 4th and 5th to Team 0, etc.
//
// --------------------------------------------------------------------------- //
void TeamBalancer::PlaygroundSort()
{
	uint32 nTeamSizes[2] = {0,0};

	//sort the player score list from highest to lowest
	std::sort(m_vecPlayerScores.begin(),m_vecPlayerScores.end(),PlayerScoreHistoryCompare());

	uint8 nTargetTeam = INVALID_TEAM;
	uint8 nOtherTeam = INVALID_TEAM;

	//step through the list of players, sorting them onto the teams
	PlayerScoreHistoryArray::iterator scoreIter = m_vecPlayerScores.begin();
	while (scoreIter != m_vecPlayerScores.end())
	{
		uint32 nID = scoreIter->first;
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientDataByClientId(nID);
		if (pGameClientData)
		{
			//we want to leave the highest scoring player on his own team
			//  so if we haven't set the target yet, set it to the team of the highest scoring player
			if (nTargetTeam == INVALID_TEAM)
			{
				nTargetTeam = pGameClientData->GetLastTeamId();
				if (nTargetTeam == INVALID_TEAM)
				{
					nTargetTeam = 0;
				}
				nOtherTeam = (nTargetTeam + 1) % MAX_TEAMS;
			}

			//move the player to the current team
			pGameClientData->SetRequestedTeam(nTargetTeam);
			CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(pGameClientData->GetPlayer());
			if (pPlayer)
			{
				pPlayer->HandleTeamSwitchRequest();
			}
			//increment the team count
			nTeamSizes[nTargetTeam]++;

			//if the team we're adding to is larger, start adding to the other team
			if (nTeamSizes[nTargetTeam] > nTeamSizes[nOtherTeam])
			{
				nTargetTeam = nOtherTeam;
				nOtherTeam = (nTargetTeam + 1) % MAX_TEAMS;
			}

		}
		scoreIter++;
	}

}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	TeamBalancer::AddPlayerScore
//
//	PURPOSE:	add a player's score to the running total
//
// --------------------------------------------------------------------------- //
void TeamBalancer::AddPlayerScore(uint32 nID, int32 nScore)
{
	//step through the player score history and find the pair for this client
	PlayerScoreHistoryArray::iterator scoreIter = m_vecPlayerScores.begin();
	while (scoreIter != m_vecPlayerScores.end() && nID != scoreIter->first)
	{
		scoreIter++;
	}

	//found the client, add his score to the running total
	if (scoreIter != m_vecPlayerScores.end())
	{
		scoreIter->second += nScore;
	}
	else
	{
		//didn't find it, so add a new one
		m_vecPlayerScores.push_back(PlayerScoreHistory(nID,nScore));
	}

}

