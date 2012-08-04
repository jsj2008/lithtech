// ----------------------------------------------------------------------- //
//
// MODULE  : ScmdShared.h
//
// PURPOSE : Shared handling of SCMD commands.  Provides remote control of
//				server.
//
// CREATED : 10/21/02
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef SCMDSHARED_H
#define SCMDSHARED_H

enum ScmdCommand
{
	kScmdCommandHelp,
	kScmdCommandLogin,
	kScmdCommandLogout,
	kScmdCommandListClients,
	kScmdCommandListMissions,
	kScmdCommandNextMission,
	kScmdCommandNextRound,
	kScmdCommandSetMission,
	kScmdCommandBootName,
	kScmdCommandBootId,
	kScmdCommandBanUser,
	kScmdCommandUnbanUser,
	kScmdCommandListUserBans,
	kScmdCommandListGameOptions,
	kScmdCommandShowGameOption,
	kScmdCommandSetGameOption,
};


enum ScmdCommandStatus
{
	kScmdCommandStatusOk,
	kScmdCommandStatusFailed,
	kScmdCommandStatusAdminAlreadyLoggedIn,
	kScmdCommandStatusNotLoggedIn,
	kScmdCommandStatusIncorrectPassword,
};


#endif // SCMDSHARED_H
