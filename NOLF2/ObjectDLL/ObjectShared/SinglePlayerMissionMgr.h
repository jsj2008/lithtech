// ----------------------------------------------------------------------- //
//
// MODULE  : SinglePlayerMissionMgr.h
//
// PURPOSE : Definition of class to handle deathmatch missions
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SINGLEPLAYERMISSIONMGR_H__
#define __SINGLEPLAYERMISSIONMGR_H__

#include "ServerMissionMgr.h"

class CSinglePlayerMissionMgr : public CServerMissionMgr
{
	// Game type overrides.
	protected:

		// Init game.
		virtual bool	StartGame( ILTMessage_Read& msg );

		// End game.
		virtual bool	EndGame( );

};

#endif // __SINGLEPLAYERMISSIONMGR_H__