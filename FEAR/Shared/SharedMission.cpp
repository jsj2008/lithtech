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

#include "Stdafx.h"
#include "SharedMission.h"




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MissionStats::MissionStats
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

MissionStats::MissionStats()
{
	Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MissionStats::Init
//
//	PURPOSE:	Clear data
//
// ----------------------------------------------------------------------- //

void MissionStats::Init()
{
	dwNumShotsFired		= 0;
	dwNumHits			= 0;
	dwNumTimesSeen		= 0;
	dwNumEvasions		= 0;
	dwNumBodies			= 0;
	dwNumEnemyKills		= 0;
	dwNumFriendKills	= 0;
	dwNumNeutralKills	= 0;
	dwNumTimesKilled	= 0;
	dwNumTimesHit		= 0;

	for (int i = 0; i < HL_NUM_LOCS; i++)
		dwHitLocations[i] = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MissionStats::WriteData
//
//	PURPOSE:	Write the data to be sent to the client
//
// ----------------------------------------------------------------------- //

void MissionStats::WriteData(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;


    pMsg->Writeuint32(dwNumShotsFired);
    pMsg->Writeuint32(dwNumHits);
    pMsg->Writeuint32(dwNumTimesSeen);
    pMsg->Writeuint32(dwNumEvasions);
    pMsg->Writeuint32(dwNumBodies);
    pMsg->Writeuint32(dwNumEnemyKills);
    pMsg->Writeuint32(dwNumFriendKills);
    pMsg->Writeuint32(dwNumNeutralKills);
    pMsg->Writeuint32(dwNumTimesKilled);
    pMsg->Writeuint32(dwNumTimesHit);

	for (int i = 0; i < HL_NUM_LOCS; i++)
        pMsg->Writeuint32(dwHitLocations[i]);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	MissionStats::ReadData
//
//	PURPOSE:	Read the data sent to the client
//
// ----------------------------------------------------------------------- //

void MissionStats::ReadData(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    dwNumShotsFired     = pMsg->Readuint32();
    dwNumHits           = pMsg->Readuint32();
    dwNumTimesSeen		= pMsg->Readuint32();
    dwNumEvasions		= pMsg->Readuint32();
    dwNumBodies			= pMsg->Readuint32();
    dwNumEnemyKills     = pMsg->Readuint32();
    dwNumFriendKills    = pMsg->Readuint32();
    dwNumNeutralKills   = pMsg->Readuint32();
    dwNumTimesKilled    = pMsg->Readuint32();
    dwNumTimesHit       = pMsg->Readuint32();

	for (int i = 0; i < HL_NUM_LOCS; i++)
        dwHitLocations[i] = pMsg->Readuint32();


}

