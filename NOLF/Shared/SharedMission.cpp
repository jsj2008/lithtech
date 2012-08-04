// ----------------------------------------------------------------------- //
//
// MODULE  : SharedMission.cpp
//
// PURPOSE : SharedMission implementation - shared mission summary stuff
//
// CREATED : 9/16/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SharedMission.h"

#define PLAYERRANK_TAG			"RankData"
#define PLAYERRANK_HEALTH		"Health"
#define PLAYERRANK_ARMOR		"Armor"
#define PLAYERRANK_AMMO			"Ammo"
#define PLAYERRANK_DAM			"Damage"
#define PLAYERRANK_PERTURB		"Perturb"
#define PLAYERRANK_STEALTH		"Stealth"
#define PLAYERRANK_REP			"Reputation"

#define MISSIONSUMMARY_TOTALLEVELINTEL	"TotalLevelIntel"
#define MISSIONSUMMARY_MAXNUMLEVELINTEL	"MaxNumLevelIntel"

static char s_aAttName[100];

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PLAYERRANK::PLAYERRANK
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PLAYERRANK::PLAYERRANK()
{
	Reset();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PLAYERRANK::Reset
//
//	PURPOSE:	Reset all the data
//
// ----------------------------------------------------------------------- //

void PLAYERRANK::Reset()
{
	fHealthMultiplier	= 1.0f;
	fArmorMultiplier	= 1.0f;
	fAmmoMultiplier		= 1.0f;
	fDamageMultiplier	= 1.0f;
	fPerturbMultiplier	= 1.0f;
	fStealthMultiplier	= 1.0f;
	nReputation			= 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PLAYERRANK::Write
//
//	PURPOSE:	Write the data to be sent to the client
//
// ----------------------------------------------------------------------- //

void PLAYERRANK::Write(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

    pInterface->WriteToMessageFloat(hWrite, fHealthMultiplier);
    pInterface->WriteToMessageFloat(hWrite, fArmorMultiplier);
    pInterface->WriteToMessageFloat(hWrite, fAmmoMultiplier);
    pInterface->WriteToMessageFloat(hWrite, fDamageMultiplier);
    pInterface->WriteToMessageFloat(hWrite, fPerturbMultiplier);
    pInterface->WriteToMessageFloat(hWrite, fStealthMultiplier);
    pInterface->WriteToMessageByte(hWrite, nReputation);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PLAYERRANK::ReadClientData
//
//	PURPOSE:	Read the data sent to the client
//
// ----------------------------------------------------------------------- //

void PLAYERRANK::Read(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    fHealthMultiplier   = pInterface->ReadFromMessageFloat(hRead);
    fArmorMultiplier    = pInterface->ReadFromMessageFloat(hRead);
    fAmmoMultiplier     = pInterface->ReadFromMessageFloat(hRead);
    fDamageMultiplier   = pInterface->ReadFromMessageFloat(hRead);
    fPerturbMultiplier  = pInterface->ReadFromMessageFloat(hRead);
    fStealthMultiplier  = pInterface->ReadFromMessageFloat(hRead);
    nReputation         = pInterface->ReadFromMessageByte(hRead);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PLAYERRANK::WriteRankData
//
//	PURPOSE:	Write the data to the butefile
//
// ----------------------------------------------------------------------- //

void PLAYERRANK::WriteRankData(CButeMgr & buteMgr)
{
	// Write the global data for each level...
	buteMgr.SetFloat(PLAYERRANK_TAG, PLAYERRANK_HEALTH,	 fHealthMultiplier);
	buteMgr.SetFloat(PLAYERRANK_TAG, PLAYERRANK_ARMOR,	 fArmorMultiplier);
	buteMgr.SetFloat(PLAYERRANK_TAG, PLAYERRANK_AMMO,	 fAmmoMultiplier);
	buteMgr.SetFloat(PLAYERRANK_TAG, PLAYERRANK_DAM,	 fDamageMultiplier);
	buteMgr.SetFloat(PLAYERRANK_TAG, PLAYERRANK_PERTURB, fPerturbMultiplier);
	buteMgr.SetFloat(PLAYERRANK_TAG, PLAYERRANK_STEALTH, fStealthMultiplier);
	buteMgr.SetInt(	 PLAYERRANK_TAG, PLAYERRANK_REP,	 nReputation);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PLAYERRANK::ClearRankData
//
//	PURPOSE:	Reset the data in the butefile
//
// ----------------------------------------------------------------------- //

void PLAYERRANK::ClearRankData(CButeMgr & buteMgr)
{
	Reset();
	WriteRankData(buteMgr);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PLAYERRANK::ReadRankData
//
//	PURPOSE:	Read the data from the butefile
//
// ----------------------------------------------------------------------- //

void PLAYERRANK::ReadRankData(CButeMgr & buteMgr)
{
	// Write the global data for each level...
	fHealthMultiplier	= buteMgr.GetFloat(PLAYERRANK_TAG, PLAYERRANK_HEALTH,	1.0f);
	fArmorMultiplier	= buteMgr.GetFloat(PLAYERRANK_TAG, PLAYERRANK_ARMOR,	1.0f);
	fAmmoMultiplier		= buteMgr.GetFloat(PLAYERRANK_TAG, PLAYERRANK_AMMO,		1.0f);
	fDamageMultiplier	= buteMgr.GetFloat(PLAYERRANK_TAG, PLAYERRANK_DAM,		1.0f);
	fPerturbMultiplier	= buteMgr.GetFloat(PLAYERRANK_TAG, PLAYERRANK_PERTURB,	1.0f);
	fStealthMultiplier	= buteMgr.GetFloat(PLAYERRANK_TAG, PLAYERRANK_STEALTH,	1.0f);
	nReputation			= buteMgr.GetByte(PLAYERRANK_TAG, PLAYERRANK_REP,		0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	LEVELSUMMARY::LEVELSUMMARY
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

LEVELSUMMARY::LEVELSUMMARY()
{
	// Global data...
	nTotalIntel  = -1;
	nMaxNumIntel = 0;

	// Instant data...
	nCurNumIntel = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONSUMMARY::MISSIONSUMMARY
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

MISSIONSUMMARY::MISSIONSUMMARY()
{
	nId			= -1;
	nNumLevels	= 0;

	fBestRank			= 0.0f;
	fOldBestRank		= 0.0f;
	fCurRank			= 0.0f;
	fTotalMissionTime	= 0.0f;
	dwNumShotsFired		= 0;
	dwNumHits			= 0;
	dwNumTimesDetected	= 0;
	dwNumDisturbances	= 0;
	dwNumBodies			= 0;
	dwNumEnemyKills		= 0;
	dwNumFriendKills	= 0;
	dwNumNeutralKills	= 0;
	dwNumTimesHit		= 0;
	m_nMissionTotalIntel  = 0;
	m_nMissionMaxIntel = 0;
	m_nMissionCurNumIntel = 0;

	for (int i = 0; i < HL_NUM_LOCS; i++)
		dwHitLocations[i] = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONSUMMARY::Init
//
//	PURPOSE:	Build the mission summary struct
//
// ----------------------------------------------------------------------- //

LTBOOL MISSIONSUMMARY::Init(CButeMgr & buteMgr, char* aTagName, MISSION* pMission)
{
    if (!aTagName || !pMission) return LTFALSE;

	// Read in the data for each level...

	nNumLevels = pMission->nNumLevels;

	for (int i=0; i < nNumLevels; i++)
	{
		sprintf(s_aAttName, "%s%d", MISSIONSUMMARY_TOTALLEVELINTEL, i);
		if (buteMgr.Exist(aTagName, s_aAttName))
		{
			Levels[i].nTotalIntel = buteMgr.GetInt(aTagName, s_aAttName);
		}
		else
		{
			buteMgr.SetInt(aTagName, s_aAttName, Levels[i].nTotalIntel);
		}

		sprintf(s_aAttName, "%s%d", MISSIONSUMMARY_MAXNUMLEVELINTEL, i);
		if (buteMgr.Exist(aTagName, s_aAttName))
		{
			Levels[i].nMaxNumIntel = buteMgr.GetInt(aTagName, s_aAttName);
		}
		else
		{
			buteMgr.SetInt(aTagName, s_aAttName, Levels[i].nMaxNumIntel);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONSUMMARY::WriteClientData
//
//	PURPOSE:	Write the data to be sent to the client
//
// ----------------------------------------------------------------------- //

void MISSIONSUMMARY::WriteClientData(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	int nTotalIntel=0, nMaxIntel=0, nCurIntel=0;
    int i;
    for (i=0; i < nNumLevels; i++)
	{
		nTotalIntel += (Levels[i].nTotalIntel  > 0 ? Levels[i].nTotalIntel  : 0);
		nMaxIntel	+= (Levels[i].nMaxNumIntel > 0 ? Levels[i].nMaxNumIntel : 0);
		nCurIntel	+= (Levels[i].nCurNumIntel > 0 ? Levels[i].nCurNumIntel : 0);
	}


    LTFLOAT fTime = fTotalMissionTime > 0 ? fTotalMissionTime : pInterface->GetTime();

    pInterface->WriteToMessageByte(hWrite, nTotalIntel);
    pInterface->WriteToMessageByte(hWrite, nMaxIntel);
    pInterface->WriteToMessageByte(hWrite, nCurIntel);
    pInterface->WriteToMessageFloat(hWrite, fBestRank);
    pInterface->WriteToMessageFloat(hWrite, fOldBestRank);
    pInterface->WriteToMessageFloat(hWrite, fCurRank);
    pInterface->WriteToMessageFloat(hWrite, fTime);
    pInterface->WriteToMessageDWord(hWrite, dwNumShotsFired);
    pInterface->WriteToMessageDWord(hWrite, dwNumHits);
    pInterface->WriteToMessageDWord(hWrite, dwNumTimesDetected);
    pInterface->WriteToMessageDWord(hWrite, dwNumDisturbances);
    pInterface->WriteToMessageDWord(hWrite, dwNumBodies);
    pInterface->WriteToMessageDWord(hWrite, dwNumEnemyKills);
    pInterface->WriteToMessageDWord(hWrite, dwNumFriendKills);
    pInterface->WriteToMessageDWord(hWrite, dwNumNeutralKills);
    pInterface->WriteToMessageDWord(hWrite, dwNumTimesHit);

	for (i = 0; i < HL_NUM_LOCS; i++)
        pInterface->WriteToMessageDWord(hWrite,dwHitLocations[i]);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONSUMMARY::ReadClientData
//
//	PURPOSE:	Read the data sent to the client
//
// ----------------------------------------------------------------------- //

void MISSIONSUMMARY::ReadClientData(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

    uint8 nTotalIntel, nMaxIntel, nCurIntel;

    nTotalIntel         = pInterface->ReadFromMessageByte(hRead);
    nMaxIntel           = pInterface->ReadFromMessageByte(hRead);
    nCurIntel           = pInterface->ReadFromMessageByte(hRead);
    fBestRank           = pInterface->ReadFromMessageFloat(hRead);
    fOldBestRank        = pInterface->ReadFromMessageFloat(hRead);
    fCurRank            = pInterface->ReadFromMessageFloat(hRead);
    fTotalMissionTime   = pInterface->ReadFromMessageFloat(hRead);
    dwNumShotsFired     = pInterface->ReadFromMessageDWord(hRead);
    dwNumHits           = pInterface->ReadFromMessageDWord(hRead);
    dwNumTimesDetected  = pInterface->ReadFromMessageDWord(hRead);
    dwNumDisturbances   = pInterface->ReadFromMessageDWord(hRead);
    dwNumBodies			= pInterface->ReadFromMessageDWord(hRead);
    dwNumEnemyKills     = pInterface->ReadFromMessageDWord(hRead);
    dwNumFriendKills    = pInterface->ReadFromMessageDWord(hRead);
    dwNumNeutralKills   = pInterface->ReadFromMessageDWord(hRead);
    dwNumTimesHit       = pInterface->ReadFromMessageDWord(hRead);

	for (int i = 0; i < HL_NUM_LOCS; i++)
        dwHitLocations[i] = pInterface->ReadFromMessageDWord(hRead);

	// Just store the intel totals in level 0...

	m_nMissionTotalIntel  = nTotalIntel;
	m_nMissionMaxIntel = nMaxIntel;
	m_nMissionCurNumIntel = nCurIntel;


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONSUMMARY::WriteGlobalData
//
//	PURPOSE:	Write the global data to the bute mgr
//
// ----------------------------------------------------------------------- //

void MISSIONSUMMARY::WriteGlobalData(CButeMgr & buteMgr, char* aTagName)
{
	if (!aTagName || !aTagName[0]) return;

	// Write the global data for each level...

	for (int i=0; i < nNumLevels; i++)
	{
		sprintf(s_aAttName, "%s%d", MISSIONSUMMARY_TOTALLEVELINTEL, i);
		buteMgr.SetInt(aTagName, s_aAttName, Levels[i].nTotalIntel);

		sprintf(s_aAttName, "%s%d", MISSIONSUMMARY_MAXNUMLEVELINTEL, i);
		buteMgr.SetInt(aTagName, s_aAttName, Levels[i].nMaxNumIntel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONSUMMARY::ClearGlobalData
//
//	PURPOSE:	Clear the global data in the bute mgr
//
// ----------------------------------------------------------------------- //

void MISSIONSUMMARY::ClearGlobalData(CButeMgr & buteMgr, char* aTagName)
{
	if (!aTagName || !aTagName[0]) return;

	// Write the global data for each level...

	for (int i=0; i < nNumLevels; i++)
	{
		Levels[i].nMaxNumIntel = 0;
		sprintf(s_aAttName, "%s%d", MISSIONSUMMARY_MAXNUMLEVELINTEL, i);
		buteMgr.SetInt(aTagName, s_aAttName, Levels[i].nMaxNumIntel);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONSUMMARY::WriteIntantData
//
//	PURPOSE:	Write the instant data
//
// ----------------------------------------------------------------------- //

void MISSIONSUMMARY::WriteInstantData(ILTCSBase *pInterface, HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	// Write the instant summary data...

    pInterface->WriteToMessageFloat(hWrite, fTotalMissionTime);
    pInterface->WriteToMessageDWord(hWrite, dwNumShotsFired);
    pInterface->WriteToMessageDWord(hWrite, dwNumHits);
    pInterface->WriteToMessageDWord(hWrite, dwNumTimesDetected);
    pInterface->WriteToMessageDWord(hWrite, dwNumDisturbances);
    pInterface->WriteToMessageDWord(hWrite, dwNumBodies);
    pInterface->WriteToMessageDWord(hWrite, dwNumEnemyKills);
    pInterface->WriteToMessageDWord(hWrite, dwNumFriendKills);
    pInterface->WriteToMessageDWord(hWrite, dwNumNeutralKills);
    pInterface->WriteToMessageDWord(hWrite, dwNumTimesHit);
    int i;
    for (i = 0; i < HL_NUM_LOCS; i++)
        pInterface->WriteToMessageDWord(hWrite,dwHitLocations[i]);

    pInterface->WriteToMessageByte(hWrite, nNumLevels);

	for (i=0; i < nNumLevels; i++)
	{
        pInterface->WriteToMessageByte(hWrite, Levels[i].nCurNumIntel);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MISSIONSUMMARY::ReadIntantData
//
//	PURPOSE:	Read the instant data
//
// ----------------------------------------------------------------------- //

void MISSIONSUMMARY::ReadInstantData(ILTCSBase *pInterface, HMESSAGEREAD hRead)
{
	if (!hRead) return;

	// Read the instant summary data...

    fTotalMissionTime   = pInterface->ReadFromMessageFloat(hRead);
    dwNumShotsFired     = pInterface->ReadFromMessageDWord(hRead);
    dwNumHits           = pInterface->ReadFromMessageDWord(hRead);
    dwNumTimesDetected  = pInterface->ReadFromMessageDWord(hRead);
    dwNumDisturbances   = pInterface->ReadFromMessageDWord(hRead);
    dwNumBodies			= pInterface->ReadFromMessageDWord(hRead);
    dwNumEnemyKills     = pInterface->ReadFromMessageDWord(hRead);
    dwNumFriendKills    = pInterface->ReadFromMessageDWord(hRead);
    dwNumNeutralKills   = pInterface->ReadFromMessageDWord(hRead);
    dwNumTimesHit       = pInterface->ReadFromMessageDWord(hRead);

    int i;
    for (i = 0; i < HL_NUM_LOCS; i++)
        dwHitLocations[i] = pInterface->ReadFromMessageDWord(hRead);

    nNumLevels          = pInterface->ReadFromMessageByte(hRead);

	for (i=0; i < nNumLevels; i++)
	{
        Levels[i].nCurNumIntel = pInterface->ReadFromMessageByte(hRead);
	}
}