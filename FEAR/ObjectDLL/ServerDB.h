// ----------------------------------------------------------------------- //
//
// MODULE  : ServerDB.h
//
// PURPOSE : Definition of database for miscellaneous server values
//
// CREATED : 03/26/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVERDB_H__
#define __SERVERDB_H__

#include "GameDatabaseMgr.h"
#include "ClientServerSharedEnums.h"


class CServerDB;
extern CServerDB* g_pServerDB;

const char* const SrvDB_rDeathMarker			= "Death Marker";
const char* const SrvDB_rSlowMo					= "SlowMo";
const char* const SrvDB_rMPSlowMo				= "MultiplayerSlowMo";
const char* const SrvDB_fDeathDelay				= "DeathDelay";
const char* const SrvDB_fMultiplayerDeathDelay	= "MultiplayerDeathDelay";
const char* const SrvDB_fMultiplayerBodyLifetime = "MultiplayerBodyLifetime";

class RegenerationData
{
public:
	RegenerationData() : 
		m_fDelay(0.0f)
		,m_fThreshold(0.0f)
		,m_fBaseRegeneration(0.0f)
		,m_fMoveRegeneration(0.0f)
		{
		}
	virtual ~RegenerationData() {}

	float m_fDelay;
	float m_fThreshold;
	float m_fBaseRegeneration;
	float m_fMoveRegeneration;
};


class CServerDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CServerDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	void	Term() {};

	HRECORD GetPlayerDefaultWeapon() const;
	HRECORD GetPlayerRecord( ) const { return m_hPlayer; }
	HRECORD GetPlayerRecordLink(const char* pszAttribute) const;
	uint32	GetPlayerInt(const char* pszAttribute) const;
	float	GetPlayerFloat(const char* pszAttribute) const;

	HRECORD	GetServerLimitsRecord() const {return m_hLimits; }
	uint32  GetGibFrequencyCap() const;
	uint32  GetSeverFrequencyCap() const;
	uint32  GetSeverTotalCap() const;
	uint32  GetBodyCapTotalCount() const;
	uint32  GetBodyCapRadiusCount() const;
	float	GetBodyCapRadius() const;

	uint32	GetProximityLimit() const;
	uint32	GetRemoteChargeLimit() const;
	uint32	GetMaxPlayerHealth() const;
	uint32	GetMaxPlayerArmor() const;


	float	GetDifficultyFactor(GameDifficulty eDiff) const;
	float	GetDamageAdjustment(GameDifficulty eDiff) const;

	const RegenerationData& GetRegeneration(GameDifficulty eDiff) const {return m_Regeneration[eDiff];}
	const RegenerationData& GetRegenerationMP() const {return m_RegenerationMP;}

private:
	HRECORD	GetRegenerationRecord(GameDifficulty eDiff) const;
	HRECORD	GetRegenerationMPRecord() const;
	HRECORD	GetDifficultyRecord(GameDifficulty eDiff) const;

	HRECORD	m_hPlayer;
	HRECORD m_hLimits;
	RegenerationData m_Regeneration[kNumDifficultyLevels];
	RegenerationData m_RegenerationMP;
};

#endif  // __SERVERDB_H__
