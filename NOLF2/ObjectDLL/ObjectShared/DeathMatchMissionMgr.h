// ----------------------------------------------------------------------- //
//
// MODULE  : DeathMatchMissionMgr.h
//
// PURPOSE : Definition of class to handle deathmatch missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DEATHMATCHMISSIONMGR_H__
#define __DEATHMATCHMISSIONMGR_H__

#include "ServerMissionMgr.h"

class CDeathMatchMissionMgr : public CServerMissionMgr
{
	public:

		CDeathMatchMissionMgr();
		~CDeathMatchMissionMgr();

	// Game type overrides.
	public:

		// Initializes the object.  Overrides should call up.
		virtual bool	Init( );

		// Terminates the object.  Overrides should call up.
		virtual void	Term();

		// Save/load state of missionmgr.  Overrides should call up.
		virtual bool	Save( ILTMessage_Write& msg, uint32 dwSaveFlags );
		virtual bool	Load( ILTMessage_Read& msg, uint32 dwSaveFlags );

		// Called every frame.  Overrides should call up.
		virtual void	Update( );

	// Game type overrides.
	protected:

		// Does loading of next level.
		virtual bool	FinishExitLevel( );

		// Init game.
		virtual bool	StartGame( ILTMessage_Read& msg );

		// End game.
		virtual bool	EndGame( );

		//handle updating multiplayer options while in game
		virtual bool	HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg );

};

#endif // __DEATHMATCHMISSIONMGR_H__