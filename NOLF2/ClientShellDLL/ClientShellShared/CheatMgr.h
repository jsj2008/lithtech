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

        LTBOOL  Check( CParsedMsg &cMsg );
        LTBOOL  Check( const char* pMsg ) { return (pMsg && pMsg[0] ? Check(CParsedMsg(1, (const char * const *)&pMsg)) : LTFALSE); }
        void	ClearCheater() { m_bPlayerCheated = LTFALSE; }
        LTBOOL	IsCheater() { return m_bPlayerCheated; }

		bool	Process( CheatCode nCheatCode, CParsedMsg &cMsg );

	private:

        void    SendCheatMessage( CheatCode nCheatCode, uint32 nData );


		struct CheatInfo
		{
			char			*pzText;
			LTBOOL			bActive;
		};

		static CheatInfo s_CheatInfo[];

        static LTBOOL m_bPlayerCheated;

		// Retail Cheats
        void    SetGodMode(LTBOOL bMode);
		void	SetAmmo();
		void	SetArmor();
		void	SetHealth(LTBOOL bPlaySound=LTTRUE);
		void    SetInvisible(LTBOOL bMode);
        void    SetPos(LTBOOL bMode);
		void	SetKFA();
		void	SetFullWeapons();
        void    Tears(LTBOOL bMode);
        void    Snowmobile(LTBOOL bMode);
		void	ModSquad();
		void	FullGear();
		void	SetExitLevel();
		void	NextMission();
		void	BootPlayer( CParsedMsg &cMsg );
		void	Version();
		void	BuildGuid();
		void	GimmeGun( CParsedMsg &cMsg );
		void	GimmeMod( CParsedMsg &cMsg	);
		void	GimmeGear( CParsedMsg &cMsg );
		void	GimmeAmmo( CParsedMsg &cMsg );
        void    SetConsole( LTBOOL bMode );
		void	GetSkills();
		void	ToggleEndgameFlag();
		void	BodyGolfing(LTBOOL bMode);

		// Development only cheats...
        void    SetClipMode(LTBOOL bMode);
		void	Teleport();
        void    PosWeapon(LTBOOL bMode);
        void    PosWeaponMuzzle(LTBOOL bMode);
		void	WeaponBreachOffset(LTBOOL bMode);
        void    LightScale(LTBOOL bMode);
        void    LightAdd(LTBOOL bMode);
        void    FOV(LTBOOL bMode);
        void    RemoveAI(LTBOOL bMode);
        void    TriggerBox(LTBOOL bMode);
        void    Pos1stCam(LTBOOL bMode);
        void    SetCamPosRot(LTBOOL bMode);
		void	ChaseToggle();
};

extern CCheatMgr*		g_pCheatMgr;

#endif	// __CHEAT_MGR_H__