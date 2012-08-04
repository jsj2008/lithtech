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
#include "StringUtilities.h"

enum ETeamId
{
	kTeamId0 = 0,
	kTeamId1 = 1,
	INVALID_TEAM = 0xFF,
	MAX_TEAMS = 2,
};

typedef std::set<
			uint32, 
			std::less<uint32>, 
			LTAllocator<uint32, LT_MEM_TYPE_GAMECODE> 
		> PlayerIDSet;

class CTeam
{
public:
	CTeam();

    void	Init(uint8 nID, uint32 nModel);
    void	Init(uint8 nID, ILTMessage_Read *pMsg);

    void    WriteData(ILTMessage_Write *pMsg,bool bScoreOnly);
    void    ReadData(ILTMessage_Read *pMsg);

	uint8		GetID()			const { return m_nID; }
	uint8		GetNumPlayers()	const { ASSERT( m_players.size() == (uint8)m_players.size()); return ( uint8 )m_players.size(); }
	PlayerIDSet const& GetPlayerIDSet( ) const { return m_players; }
	int32		GetScore()		const { return m_nScore; }
	const wchar_t* GetName()		const 
	{ 
		static const char* pszTeamID[] = { "IDS_TEAM_1", "IDS_TEAM_2", };
		const wchar_t* pwszTeam = LoadString( pszTeamID[GetID( )] );
		return pwszTeam;
	}
	uint32		GetModel()		const { return m_nModel; }
	uint32		GetRoundScore( ) const { return m_nRoundScore; }

	void   SetAttacking( bool bValue ) { m_bAttackingTeam = bValue; }
	bool   GetAttacking( ) const { return m_bAttackingTeam; }


#ifdef _SERVERBUILD
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
	uint32			m_nModel;

	// Team is attacking for gamemode.
	bool			m_bAttackingTeam;
};




typedef std::vector<CTeam*, LTAllocator<CTeam*, LT_MEM_TYPE_GAMECODE> > TeamArray;

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
#endif

#ifdef _SERVERBUILD
	uint8	AddTeam(uint32 nModel);
	void	AddPlayer(uint32 nClientID, uint8 nTeamID);
	void	RemovePlayer(uint32 nClientID);
	uint8	GetTeamIdOfPlayer( uint32 nClientID );
	CTeam*	GetTeamWithLeastPlayers();
	
	// Accessors to the attacking team info.
	void   SetAttackingTeam( uint8 nTeamId, bool bValue );
	bool   GetAttackingTeam( uint8 nTeamId );


	void	UpdateClient( HCLIENT hClient = NULL );
	void	UpdateClient( HCLIENT hClient, bool bScoreOnly );

	void	AddToScore( uint8 nTeamID, int32 nScore = 1 );
	void	WonRound( uint8 nTeamID );

	// Prepares for a new round.
	void	NewRound( );

	// Prepares for a new level.
	void	NewLevel( );

#endif // _SERVERBUILD


private:
	TeamArray	m_teams;

};

#endif // __TEAM_MGR_H__
