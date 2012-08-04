// ----------------------------------------------------------------------- //
//
// MODULE  : SharedScoring.h
//
// PURPOSE : SharedScoring - shared mission summary stuff
//
// CREATED : 10/17/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SHARED_SCORE_H__
#define __SHARED_SCORE_H__

#include "ltbasetypes.h"
#include "ClientServerShared.h"




class CPlayerScore
{
public:
	CPlayerScore();

	enum ScoreEvent
	{
		eKill,
		eDeath,
		eTeamKill,
		eSuicide,
		eObjective,
		kNumScoreEvents
	};

    void	Init(uint32 nClientID);
    void    WriteData(ILTMessage_Write *pMsg);
    void    ReadData(ILTMessage_Read *pMsg);

#ifdef _SERVERBUILD
	void	AddEvent(ScoreEvent eType);
	void	AddScore(int32 nBonus);
	void	AddObjectiveScore(int32 nBonus);
	void	UpdateClients(HCLIENT hClients = NULL);
	void	Update();
	uint32	GetNumTeamKills() const {return m_nTeamKills;}
	void	ClearTeamKills() {m_nTeamKills = 0;}
#endif

	int32	GetScore() {return m_nScore;}
	uint32	GetEventCount(ScoreEvent eType) {return m_nEvents[eType];}

private:
	uint32	m_nClientID;
	int32	m_nScore;
	uint32	m_nEvents[kNumScoreEvents];

#ifdef _SERVERBUILD
	void	CheckSequentialKills();
	void	CheckMultiKill();
	uint32	m_nSequentialKills;
	uint32	m_nMultiKillCount;
	StopWatchTimer m_MultiKillTimer;
	uint32	m_nTeamKills;
#endif

	bool	m_bInitted;

};

#endif // __SHARED_SCORE_H__
