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
			m_nMPDifficulty = 0;
			m_fPlayerDiffFactor = 0.0f;
		};

		uint8			m_nMPDifficulty;		// difficulty level of the game
		float			m_fPlayerDiffFactor;	// how much each additional player increases difficulty
		uint8			m_nTagScore;			// points awarded for a non-lethal tag
};


#endif // __SERVERMISSIONSETTINGS_H__
