// ----------------------------------------------------------------------- //
//
// MODULE  : CoopMissionMgr.h
//
// PURPOSE : Definition of class to handle coop missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COOPMISSIONMGR_H__
#define __COOPMISSIONMGR_H__

#include "ServerMissionMgr.h"

class CCoopMissionMgr : public CServerMissionMgr
{
	// Game type overrides.
	protected:

		// Init game.
		virtual bool	StartGame( ILTMessage_Read& msg );

		// End game.
		virtual bool	EndGame( );

		//handle updating multiplayer options while in game
		virtual bool	HandleMultiplayerOptions( HCLIENT hSender, ILTMessage_Read& msg );
};

#endif // __COOPMISSIONMGR_H__