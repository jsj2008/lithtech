// ----------------------------------------------------------------------- //
//
// MODULE  : DoomsDayMissionMgr.h
//
// PURPOSE : Definition of class to handle deathmatch missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DOOMSDAYMISSIONMGR_H__
#define __DOOMSDAYMISSIONMGR_H__

#include "ServerMissionMgr.h"

class CDoomsDayMissionMgr : public CServerMissionMgr
{
	public:

		CDoomsDayMissionMgr();
		~CDoomsDayMissionMgr();

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

		// Called to check if gametype allows respawning.
		virtual bool	CanRespawn( CPlayerObj const& player ) const;

		// Called when the clients enter the world
		virtual void	LevelStarted();

	// Doomsday specific methods.
	public:

		// Called when activating team starts device.
		bool			SetDeviceFiring( uint8 nActivatingTeam );
		
		// Called when activating team completes effect.
		bool			SetDeviceEffectComplete( uint8 nActivatingTeam );

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

	// Doomsday specific fields.
	protected:

		// Team that activated their device.
		uint8			m_nActivatingTeam;

		// Go to next round on next update.
		bool			m_bNextRound;
};

#endif // __DoomsDayMissionMgr_H__