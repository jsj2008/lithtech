// ----------------------------------------------------------------------- //
//
// MODULE  : ServerMissionSettings.h
//
// PURPOSE : Definition of class to contain server options
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVERMISSIONSETTINGS_H__
#define __SERVERMISSIONSETTINGS_H__

#include "ClientServerShared.h"


class ServerMissionSettings
{
	public:

		ServerMissionSettings() { Clear();}

		void Clear () 
		{ 
			m_bUseSkills = false;
			m_bFriendlyFire = false;
			m_nMPDifficulty = 0;
			m_fPlayerDiffFactor = 0.0f;
			m_nRunSpeed = 100;
			m_nScoreLimit = 0;
			m_nTimeLimit = 0;
			m_nRounds = 1;
			m_nFragScore = 0;
			m_nTagScore = 0;
			m_nReviveScore = 0;
		};


		ServerMissionSettings& operator =( const ServerMissionSettings& other )
		{ 
			if( &other != this )
			{
				m_bUseSkills = other.m_bUseSkills;
				m_bFriendlyFire = other.m_bFriendlyFire;
				m_nMPDifficulty = other.m_nMPDifficulty;
				m_fPlayerDiffFactor = other.m_fPlayerDiffFactor;
				m_nRunSpeed = other.m_nRunSpeed;
				m_nScoreLimit = other.m_nScoreLimit;
				m_nTimeLimit = other.m_nTimeLimit;
				m_nRounds = other.m_nRounds;
				m_nFragScore = other.m_nFragScore;
				m_nTagScore = other.m_nTagScore;
				m_nReviveScore = other.m_nReviveScore;
			}

			return *this;
		};


		bool			m_bUseSkills;			// are skills used?
		bool			m_bFriendlyFire;		// can players on the same team hurt each other?
		uint8			m_nMPDifficulty;		// difficulty level of the game
		float			m_fPlayerDiffFactor;	// how much each additional player increases difficulty
		uint8			m_nRunSpeed;			// what percentage of the single player runspeed is used (100 = x1, 200 = x2, etc.)
		uint8			m_nScoreLimit;			// what score is needed to end the round
		uint8			m_nTimeLimit;			// how many minutes before the round ends
		uint8			m_nRounds;				// how many rounds before switching to the next mission
		uint8			m_nFragScore;			// points awarded for killing a player
		uint8			m_nTagScore;			// points awarded for a non-lethal tag
		uint8			m_nReviveScore;			// points awarded for reviving a teammate
};


#endif // __SERVERMISSIONSETTINGS_H__