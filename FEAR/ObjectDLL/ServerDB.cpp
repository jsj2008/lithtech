// ----------------------------------------------------------------------- //
//
// MODULE  : ServerDB.cpp
//
// PURPOSE : Implementation of database for miscellaneous server values
//
// CREATED : 03/26/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "ServerDB.h"
#include "PlayerButes.h"

//
// Globals...
//

CServerDB* g_pServerDB = NULL;

const char* const SrvDB_PlayerCat =	"Server/Player";
const char* const SrvDB_LimitsCat =	"Server/Limits";

const char* const SrvDB_Player =	"Player";
const char* const SrvDB_Limits =	"Limits";


extern VarTrack g_vtBodySeverTest;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerDB::CServerDB()
//
//	PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CServerDB::CServerDB()
:	CGameDatabaseMgr( ),
	m_hPlayer(NULL),
	m_hLimits(NULL)
{

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerDB::~CServerDB()
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CServerDB::~CServerDB()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CServerDB::Init()
//
//	PURPOSE:	Initialize the database...
//
// ----------------------------------------------------------------------- //
bool CServerDB::Init( const char *szDatabaseFile /* = DB_Default_File  */ )
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global database pointer...
	g_pServerDB = this;

	m_hPlayer = g_pLTDatabase->GetRecord(m_hDatabase,SrvDB_PlayerCat,SrvDB_Player);
	m_hLimits = g_pLTDatabase->GetRecord(m_hDatabase,SrvDB_LimitsCat,SrvDB_Limits);

	
	for (uint8 n = GD_EASY; n < kNumDifficultyLevels; ++n)
	{
		HRECORD hRec = GetRegenerationRecord((GameDifficulty)n);
		if (hRec)
		{
			m_Regeneration[n].m_fDelay = GetFloat(hRec,"Delay",0,0.0f);
			m_Regeneration[n].m_fThreshold = GetFloat(hRec,"Threshold",0,0.0f);
			m_Regeneration[n].m_fBaseRegeneration = GetFloat(hRec,"BaseRegeneration",0,0.0f);
			m_Regeneration[n].m_fMoveRegeneration = GetFloat(hRec,"MoveRegeneration",0,0.0f);
		}
		
	};
	HRECORD hRec = GetRegenerationMPRecord();
	if (hRec)
	{
		m_RegenerationMP.m_fDelay = GetFloat(hRec,"Delay",0,0.0f);
		m_RegenerationMP.m_fThreshold = GetFloat(hRec,"Threshold",0,0.0f);
		m_RegenerationMP.m_fBaseRegeneration = GetFloat(hRec,"BaseRegeneration",0,0.0f);
		m_RegenerationMP.m_fMoveRegeneration = GetFloat(hRec,"MoveRegeneration",0,0.0f);
	}

	return true;
}


HRECORD CServerDB::GetPlayerDefaultWeapon() const
{
	LTASSERT(m_hPlayer,"Unable to retrieve Player record.");
	if (!m_hPlayer) return NULL;

	HRECORD hWeap = GetRecordLink(m_hPlayer,PLAYER_BUTE_DEFAULTWEAPON);
	LTASSERT(hWeap,"Unable to retrieve Player default weapon record.");
	return hWeap;

}

HRECORD CServerDB::GetPlayerRecordLink(const char* pszAttribute) const
{
	return GetRecordLink(m_hPlayer, pszAttribute );
}

uint32 CServerDB::GetPlayerInt(const char* pszAttribute) const
{
	return GetInt32(m_hPlayer,pszAttribute);
}

float CServerDB::GetPlayerFloat(const char* pszAttribute) const
{
	return GetFloat(m_hPlayer,pszAttribute);
}


uint32 CServerDB::GetGibFrequencyCap() const
{
	return GetInt32(m_hLimits,"GibFrequencyCap");
}


uint32 CServerDB::GetSeverFrequencyCap() const
{
	return GetInt32(m_hLimits,"SeverFrequencyCap");
}


uint32 CServerDB::GetSeverTotalCap() const
{
	// no cap for testing...
	if (g_vtBodySeverTest.GetFloat() > 0.0f)
	{
		return 9999;
	}

	return GetInt32(m_hLimits,"SeverTotalCap");
}


uint32 CServerDB::GetBodyCapTotalCount() const
{
	return GetInt32(m_hLimits,"BodyCapTotalCount");
}


uint32 CServerDB::GetBodyCapRadiusCount() const
{
	return GetInt32(m_hLimits,"BodyCapRadiusCount");
}


float CServerDB::GetBodyCapRadius() const
{
	return GetFloat(m_hLimits,"BodyCapRadius");
}

uint32 CServerDB::GetProximityLimit() const
{
	return GetInt32(m_hLimits,"ProximityLimit");
}
uint32 CServerDB::GetRemoteChargeLimit() const
{
	return GetInt32(m_hLimits,"RemoteChargeLimit");
}
uint32 CServerDB::GetMaxPlayerHealth() const
{
	return GetInt32(m_hLimits,"MaxPlayerHealth");
}
uint32 CServerDB::GetMaxPlayerArmor() const
{
	return GetInt32(m_hLimits,"MaxPlayerArmor");
}



HRECORD CServerDB::GetDifficultyRecord(GameDifficulty eDiff) const
{
	return GetRecordLink(m_hPlayer,"Difficulty",eDiff,NULL);
}

HRECORD CServerDB::GetRegenerationRecord(GameDifficulty eDiff) const
{
	HRECORD hDiff = GetDifficultyRecord(eDiff);
	if (!hDiff)
	{
		return NULL;
	}
	return GetRecordLink(hDiff,"Regeneration",0,NULL);
}

HRECORD CServerDB::GetRegenerationMPRecord() const
{
	return GetRecordLink(m_hPlayer,"MultiplayerRegeneration",0,NULL);
}

float CServerDB::GetDifficultyFactor(GameDifficulty eDiff) const
{
	HRECORD hDiff = GetDifficultyRecord(eDiff);
	if (!hDiff)
	{
		return 0.0f;
	}
	return GetFloat(hDiff,"AIDifficultyFactor",0,0.0f);
}

float CServerDB::GetDamageAdjustment(GameDifficulty eDiff) const
{
	HRECORD hDiff = GetDifficultyRecord(eDiff);
	if (!hDiff)
	{
		return 0.0f;
	}
	return GetFloat(hDiff,"DamageAdjustment",0,0.0f);
}


