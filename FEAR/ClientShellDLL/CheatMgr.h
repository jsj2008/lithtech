//-------------------------------------------------------------------------
//
// MODULE  : CheatMgr.h
//
// PURPOSE : Cheat system
//
// CREATED : 10/22/97 - as part of Blood2 messaging system
//
// REVISED : 10/16/01 - ripped out of MessageMgr
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
//-------------------------------------------------------------------------

#ifndef __CHEAT_MGR_H__
#define __CHEAT_MGR_H__

#include "iclientshell.h"
#include "iltclient.h"
#include "CheatDefs.h"
#include "ParsedMsg.h"

// Cheat Manager class
class CCheatMgr
{
	public:
		CCheatMgr() {}

		void	Init();

		bool	Check( CParsedMsgW const &cMsg );
		void	ClearCheater() { m_bPlayerCheated = false; }
		bool	IsCheater() { return m_bPlayerCheated; }

		bool	Process( CheatCode nCheatCode, CParsedMsgW const &cMsg );

		void	OnEnterWorld();

		struct CheatInfo
		{
			wchar_t			*pzText;
			bool			bActive;
		};

		CheatInfo const& GetCheatInfo( CheatCode eCheatCode ) const { return s_CheatInfo[eCheatCode]; }

	private:

        void    SendCheatMessage( CheatCode nCheatCode, uint32 nData );
		void	SendCheatMessageHRecord( CheatCode nCheatCode, HRECORD hRecord );

		static CheatInfo s_CheatInfo[];

		static bool m_bPlayerCheated;

		// Retail Cheats
		void	SetGodMode(bool bMode);
		void	SetAmmo();
		void	SetArmor();
		void	SetHealth(bool bPlaySound=true);
		void	SetInvisible(bool bMode);
		void	SetPos(bool bMode);
		void	SetKFA();
		void	SetFullWeapons();
		void	Tears(bool bMode);
		void	ModSquad();
		void	FullGear();
		void	SetExitLevel();
		void	NextMission();
		void	BootPlayer( CParsedMsgW const& cMsg );
		void	Version();
		void	GimmeGun( CParsedMsgW const& cMsg );
		void	GimmeMod( CParsedMsgW const& cMsg	);
		void	GimmeGear( CParsedMsgW const& cMsg );
		void	GimmeAmmo( CParsedMsgW const& cMsg );
		void	SetConsole( bool bMode );
		void	ToggleEndgameFlag();
		void	BodyGolfing(bool bMode);

		// Development only cheats...
		void	SetClipMode(bool bMode);
		void	Teleport();
		void	PosWeapon(bool bMode);
		void	RemoveAI(bool bMode);
		void	TriggerBox(bool bMode);
		void	Pos1stCam(bool bMode);
		void	SetCamPosRot(bool bMode);
		void	ChaseToggle();
		void	SaveVehicle();
};

extern CCheatMgr*		g_pCheatMgr;

#endif	// __CHEAT_MGR_H__