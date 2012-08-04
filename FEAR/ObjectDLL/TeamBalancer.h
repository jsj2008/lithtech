// ----------------------------------------------------------------------- //
//
// MODULE  : TeamBalancer.h
//
// PURPOSE : Definition of class handling automatic team balancing
//
// CREATED : 06/06/06
//
// (c) 2006 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TEAMBALANCER_H__
#define __TEAMBALANCER_H__

#include "ServerMissionMgr.h"

class TeamBalancer
{
private:

	TeamBalancer();
	~TeamBalancer();

public:

	static TeamBalancer& Instance();

	void Init();
	void Update();

	// Declare our delegate to receive BeginRound events.
	static void HandleBeginRound( TeamBalancer* pTeamBalancer, CServerMissionMgr* pServerMissionMgr, EventCaster::NotifyParams& notifyParams ) 
				{ TeamBalancer::Instance().OnBeginRound(); }
	Delegate< TeamBalancer, CServerMissionMgr, TeamBalancer::HandleBeginRound > m_delegateBeginRound;
	void OnBeginRound();

	// Declare our delegate to receive EndRound events.
	static void HandleEndRound( TeamBalancer* pTeamBalancer, CServerMissionMgr* pServerMissionMgr, EventCaster::NotifyParams& notifyParams )
				{ TeamBalancer::Instance().OnEndRound(); }
	Delegate< TeamBalancer, CServerMissionMgr, TeamBalancer::HandleEndRound > m_delegateEndRound;
	void OnEndRound();

	// Declare our delegate to receive EndMap events.
	static void HandleEndMap( TeamBalancer* pTeamBalancer, CServerMissionMgr* pServerMissionMgr, EventCaster::NotifyParams& notifyParams )
				{ TeamBalancer::Instance().OnEndMap(); }
	Delegate< TeamBalancer, CServerMissionMgr, TeamBalancer::HandleEndMap > m_delegateEndMap;
	void OnEndMap();

	// Declare our delegate to receive GameRulesUpdated events.
	static void HandleGameRulesUpdated( TeamBalancer* pTeamBalancer, CServerMissionMgr* pServerMissionMgr, EventCaster::NotifyParams& notifyParams )
				{ TeamBalancer::Instance().OnGameRulesUpdated(); }
	Delegate< TeamBalancer, CServerMissionMgr, TeamBalancer::HandleGameRulesUpdated > m_delegateGameRulesUpdated;
	void OnGameRulesUpdated();

	// Initialize our search table.
	enum BalanceFrequency
	{
		eBalanceFrequency_Never,
		eBalanceFrequency_QuarterRound,
		eBalanceFrequency_HalfRound,
		eBalanceFrequency_OneRound,
		eBalanceFrequency_TwoRounds,
		eBalanceFrequency_ThreeRounds,
		eBalanceFrequency_MapChange,
		eBalanceFrequency_Invalid,
	};

	typedef std::pair<uint32, int32> PlayerScoreHistory;
	struct PlayerScoreHistoryCompare
	{
		bool operator()(const PlayerScoreHistory &sLHS, const PlayerScoreHistory &sRHS) const { return sLHS.second > sRHS.second; }
	};
	typedef std::vector< PlayerScoreHistory > PlayerScoreHistoryArray;


private:
	PREVENT_OBJECT_COPYING( TeamBalancer );

	BalanceFrequency	GetBalanceFrequency(const char* pszString);
	void				SetRoundTimer();
	bool				BalanceTeamSize();
	bool				BalanceTeamScore();
	void				PlaygroundSort();
	void				AddPlayerScore(uint32 nID, int32 nScore);

	StopWatchTimer	m_tmrRound;
	BalanceFrequency m_eTeamSizeBalance;
	BalanceFrequency m_eTeamScoreBalance;
	uint8			m_nRoundsSinceScoreBalance;

	int32			m_nTeamScores[2];
	PlayerScoreHistoryArray m_vecPlayerScores;


};

#endif  // __TEAMBALANCER_H__
