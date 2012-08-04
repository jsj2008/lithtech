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

#include "ltbasetypes.h"
#include "ClientServerShared.h"


struct MissionStats
{
	MissionStats();

    void	Init();
    void    WriteData(ILTMessage_Write *pMsg);
    void    ReadData(ILTMessage_Read *pMsg);


    uint32  dwNumShotsFired;    // Number of total shots fired
    uint32  dwNumHits;          // Number of times characters were hit
    uint32  dwNumTimesSeen;		// Number of times enemy AI saw the player
    uint32  dwNumEvasions;		// Number of times a pursuing AI was evaded by the player
    uint32  dwNumBodies;        // Number of times AI found dead bodies
    uint32  dwNumEnemyKills;    // Number of enemies killed
    uint32  dwNumFriendKills;   // Number of friendlies killed
    uint32  dwNumNeutralKills;  // Number of neutrals killed
    uint32  dwNumTimesKilled;   // Number of times killed (in multiplayer)
    uint32  dwNumTimesHit;      // Number of times hit during the mission

    uint32  dwHitLocations[HL_NUM_LOCS];  // Number of times player hit each body part hit


};


#endif // __SHARED_MISSION_H__