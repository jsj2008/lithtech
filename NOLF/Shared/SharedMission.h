// ----------------------------------------------------------------------- //
//
// MODULE  : SharedMission.h
//
// PURPOSE : SharedMission - shared mission summary stuff
//
// CREATED : 9/16/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SHARED_MISSION_H__
#define __SHARED_MISSION_H__

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "TemplateList.h"
#include "MissionMgr.h"
#include "ClientServerShared.h"


struct PLAYERRANK
{
	PLAYERRANK();

	void	Reset();
    void    Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    void    Read(ILTCSBase *pInterface, HMESSAGEREAD hRead);
	void	WriteRankData(CButeMgr & buteMgr);
	void	ReadRankData(CButeMgr & buteMgr);
	void	ClearRankData(CButeMgr & buteMgr);

	// These are the global multipliers for the player's
	// current stats

	// (between 1.0 and ?)...
    LTFLOAT  fHealthMultiplier;
    LTFLOAT  fArmorMultiplier;
    LTFLOAT  fAmmoMultiplier;
    LTFLOAT  fDamageMultiplier;

	// (between 0.0 and 1.0)...
    LTFLOAT  fPerturbMultiplier;
    LTFLOAT  fStealthMultiplier;

	// The global reputation of the player...

	uint8		nReputation;
};

struct LEVELSUMMARY
{
	LEVELSUMMARY();

	// Global data (only needs to be saved when it changes)...

	int	nTotalIntel;	// Total number of intelligence objects in this level
	int	nMaxNumIntel;	// Max number player has ever found

	// Instant data (must be saved everytime the game is saved)...

	int	nCurNumIntel;	// Number player currently has found
};

struct MISSIONSUMMARY
{
	MISSIONSUMMARY();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName, MISSION* pMission);
	void	WriteGlobalData(CButeMgr & buteMgr, char* aTagName);
	void	ClearGlobalData(CButeMgr & buteMgr, char* aTagName);
    void    WriteInstantData(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    void    ReadInstantData(ILTCSBase *pInterface, HMESSAGEREAD hRead);
    void    WriteClientData(ILTCSBase *pInterface, HMESSAGEWRITE hWrite);
    void    ReadClientData(ILTCSBase *pInterface, HMESSAGEREAD hWrite);

	// These values are not stored in the butes file...

	int		nId;
	int		nNumLevels;

	// Level specific info...

	LEVELSUMMARY Levels[MMGR_MAX_MISSION_LEVELS];

    LTFLOAT  fBestRank;          // Player's best rank for this mission
    LTFLOAT  fOldBestRank;          // Player's best rank for this mission
    LTFLOAT  fCurRank;           // Player's current rank for this mission
    LTFLOAT  fTotalMissionTime;  // Total time in mission
    uint32  dwNumShotsFired;    // Number of total shots fired
    uint32  dwNumHits;          // Number of times characters were hit
    uint32  dwNumTimesDetected; // Number of times AI detected the player
    uint32  dwNumDisturbances;  // Number of times AI was disturbed the player
    uint32  dwNumBodies;        // Number of times AI found dead bodies
    uint32  dwNumEnemyKills;    // Number of enemies killed
    uint32  dwNumFriendKills;   // Number of friendlies killed
    uint32  dwNumNeutralKills;  // Number of neutrals killed
    uint32  dwNumTimesHit;      // Number of times hit during the mission

    uint32  dwHitLocations[HL_NUM_LOCS];  // Number of times player hit each body part hit

	uint8 m_nMissionTotalIntel;
	uint8 m_nMissionMaxIntel;
	uint8 m_nMissionCurNumIntel;

};


typedef CTList<MISSIONSUMMARY*> MissionSummaryList;

#endif // __SHARED_MISSION_H__