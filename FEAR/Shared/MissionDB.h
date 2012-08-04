// ----------------------------------------------------------------------- //
//
// MODULE  : MissionDB.h
//
// PURPOSE : Definition of Missions database
//
// CREATED : 02/03/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __MISSIONDB_H__
#define __MISSIONDB_H__


//
// Includes...
//

#include "GameDatabaseMgr.h"
#include "resourceextensions.h"


//
// Defines...
//

#define MDB_MP_File		 "MPMissions." RESEXT_GAMEDATABASE_PACKED


//------------------------------------------------------
// Attributes shared between Missions and Levels
const char* const MDB_Name =			"Name";
const char* const MDB_DefaultWeapons =	"DefaultWeapons";
const char* const MDB_DefaultMods =		"DefaultMods";

// Mission order attributes.
const char* const MDB_MissionOrderDefault = "MissionOrder";
const char* const MDB_Missions =		    "Missions";

// Mission record attributes
const char* const MDB_NameStr =			"NameStr";
const char* const MDB_Levels =			"Levels";
const char* const MDB_SelectedWeapon =	"SelectedWeapon";
const char* const MDB_ResetPlayer =		"ResetPlayer";
const char* const MDB_Layout =			"Layout";
const char* const MDB_BriefingLayout =	"BriefingLayout";

// Level record attributes
const char* const MDB_Briefing =		"Briefing";
const char* const MDB_Help =			"Help";

//Multiplayer level attributes
const char* const MDB_Mission =			"Mission";
const char* const MDB_Photo =			"Photo";
const char* const MDB_MinPlayers =		"MinPlayers";
const char* const MDB_MaxPlayers =		"MaxPlayers";
const char* const MDB_SupportedFeatures = "SupportedFeatures";
const char* const MDB_RequiredFeatures = "RequiredFeatures";

const wchar_t* const MDB_Mission_L =	L"Mission";
const wchar_t* const MDB_NameStr_L =	L"NameStr";


// Shared record attributes
extern const char* const MDB_MissionSharedCat;
extern const char* const MDB_MissionSharedRecord;
extern const char* const MDB_SinglePlayerWorld;
extern const char* const MDB_MultiPlayerWorld;
//------------------------------------------------------

class CMissionDB;
extern CMissionDB* g_pMissionDB;

class CMissionDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CMissionDB );

public :	// Methods...

	bool	Init( const char *szDatabaseFile = DB_Default_File );
	bool	IsUsingDB(const char *szDatabaseFile);
	void	Term() {}
	void	Reset();

	HCATEGORY GetMissionCat( ) const { return m_hMissionCat; }
	HRECORD GetMissionOrderRec( ) const { return m_hMissionOrder; }

	//------------------------------------------------------
	// Access to data
	uint32		GetNumMissions() const;
	HRECORD		GetMission(uint32 nMissionId) const;
	HRECORD		GetLevel( uint32 nMissionId, uint32 nLevelId ) const;
	HRECORD		GetLevel( HRECORD hMission, uint32 nLevelId ) const;
	HRECORD		GetMissionSharedRecord() const { return m_hMissionSharedRecord; }

	// Gets the display name of the mission.
	void GetMissionDisplayName( HRECORD hMission, HRECORD hLevel, wchar_t* pwszMissionDisplayName, uint32 nLen ) const;

	const char* GetWorldName(HRECORD hLevel, bool bIncludePath) const;

	bool		IsMissionLevel( char const* pWorldFile, uint32 & nMissionId, uint32 & nLevel) const;

	//create the MP mission database
	bool	CreateMPDB();

	bool CheckMPLevelRequirements(HRECORD hMission,StringSet& setRequiredMapFeatures) const;

	//used to get a display name based on the world name
	void GetMissionDisplayName( const char* pWorldFile, wchar_t* pwszMissionDisplayName, uint32 nLen ) const;


private	:	// Members...

	HCATEGORY	m_hMissionCat;
	HCATEGORY	m_hLevelCat;
	HRECORD		m_hMissionOrder;
	HRECORD		m_hMissionSharedRecord;
};

class MultiplayerMissionIni
{
public:
	MultiplayerMissionIni(const char* szWorldName);
	virtual ~MultiplayerMissionIni();

	bool	IsValid() const {return !m_sFile.empty();}

	bool	GetName(wchar_t* pBuffer, uint32 nBufferSize) const;
	bool	GetPhoto(char* pBuffer, uint32 nBufferSize) const;

	uint32	GetMinPlayers() const;
	uint32	GetMaxPlayers() const;
	bool	GetRequiredFeatures(char* pBuffer, uint32 nBufferSize, uint8 nIndex) const;
	bool	GetSupportedFeatures(char* pBuffer, uint32 nBufferSize) const;

protected:
private:
	MultiplayerMissionIni();

	bool m_bExtracted;
	std::string m_sPath;
	std::string m_sFile;
};


#endif  // __MISSIONDB_H__
