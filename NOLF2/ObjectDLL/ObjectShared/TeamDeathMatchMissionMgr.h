// ----------------------------------------------------------------------- //
//
// MODULE  : TeamDeathMatchMissionMgr.h
//
// PURPOSE : Definition of class to handle team deathmatch missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __TEAMDEATHMATCHMISSIONMGR_H__
#define __TEAMDEATHMATCHMISSIONMGR_H__

//
// Includes...
//

	#include "DeathMatchMissionMgr.h"


class CTeamDeathMatchMissionMgr : public CDeathMatchMissionMgr
{
	public:

		CTeamDeathMatchMissionMgr();
		~CTeamDeathMatchMissionMgr();

	// Game type overrides...
	public:

		// Initializes the object.  Overrides should call up.
		virtual bool	Init( );

		// Called every frame.  Overrides should call up.
		virtual void	Update();

	// Game type overrides...
	protected:

		// Init game.
		virtual bool	StartGame( ILTMessage_Read& msg );

		// Does loading of next level.
		virtual bool	FinishExitLevel( );

		// Called when the clients enter the world
		virtual void	LevelStarted();

		//handle updating multiplayer options while in game
		virtual bool	HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg );

};

#endif // __TEAMDEATHMATCHMISSIONMGR_H__
