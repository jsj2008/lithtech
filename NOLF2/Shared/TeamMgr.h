// ----------------------------------------------------------------------- //
//
// MODULE  : TeamMgr.h
//
// PURPOSE : TeamMgr - shared team management
//
// CREATED : 11/19/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TEAM_MGR_H__
#define __TEAM_MGR_H__

#include "ltbasetypes.h"
#include "ClientServerShared.h"

#define MAX_TEAMS 2
#define INVALID_TEAM (uint8)-1

typedef std::set<uint32> PlayerIDSet;

class CTeam
{
public:
	CTeam();

    void	Init(uint8 nID, const char *szName, uint32 nModel);
    void	Init(uint8 nID, ILTMessage_Read *pMsg);

    void    WriteData(ILTMessage_Write *pMsg,bool bScoreOnly);
    void    ReadData(ILTMessage_Read *pMsg);

	uint8		GetID()			const { return m_nID; }
	uint8		GetNumPlayers()	const { return m_players.size(); }
	PlayerIDSet const& GetPlayerIDSet( ) const { return m_players; }
	int32		GetScore()		const { return m_nScore; }
	const char* GetName()		const { return m_sName.c_str(); }
	uint32		GetModel()		const { return m_nModel; }
	uint32		GetRoundScore( ) const { return m_nRoundScore; }

#ifndef _CLIENTBUILD
	void	AddPlayer(uint32 nClientID);
	void	RemovePlayer(uint32 nClientID);

	void	AddScore(int32 nScore);
	void	WonRound( );
	void	UpdateClient(bool bScoreOnly, HCLIENT hClient = NULL);

	// Prepares for a new round.
	void	NewRound( );

	// Prepares for a new level.
	void	NewLevel( );
#endif

private:
	uint8			m_nID;
	PlayerIDSet		m_players;
	int32			m_nScore;
	uint32			m_nRoundScore;
	std::string		m_sName;
	uint32			m_nModel;
};




typedef std::vector<CTeam*> TeamArray;

class CTeamMgr
{
protected:

	// Not allowed to create directly.  Use Instance().
	CTeamMgr();
	
	// Copy ctor and assignment operator not implemented and should never be used.
	CTeamMgr( CTeamMgr const &other );
	CTeamMgr& operator=( CTeamMgr const &other );
	
public:

	// Call this to get the singleton instance of the team mgr.
	static CTeamMgr& Instance();

	~CTeamMgr();

	CTeam*	GetTeam(uint8 nID);
	uint8	GetNumTeams();

#ifdef _CLIENTBUILD
	void	UpdateTeam(ILTMessage_Read *pMsg);
#else // _CLIENTBUILD
	uint8	AddTeam(const char *szName, uint32 nModel);
	void	AddPlayer(uint32 nClientID, uint8 nTeamID);
	void	RemovePlayer(uint32 nClientID);
	uint8	GetTeamIdOfPlayer( uint32 nClientID );
	CTeam*	GetTeamWithLeastPlayers();
	

	void	UpdateClient(HCLIENT hClient = NULL);

	void	AddToScore( uint8 nTeamID, int32 nScore = 1 );
	void	WonRound( uint8 nTeamID );

	// Prepares for a new round.
	void	NewRound( );

	// Prepares for a new level.
	void	NewLevel( );

#endif // _CLIENTBUILD


private:
	TeamArray	m_teams;

};

#endif // __TEAM_MGR_H__