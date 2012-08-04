//-------------------------------------------------------------------------
//
// MODULE  : CheatMgr.cpp
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

#include "stdafx.h"
#include "GameClientShell.h"
#include "CheatMgr.h"
#include "HUDMgr.h"
#include "MsgIDs.h"
#include "SoundTypes.h"
#include "ClientResShared.h"


CCheatMgr*      g_pCheatMgr     = LTNULL;
bool             g_bInfiniteAmmo = false;

void CheatFn(int argc, char **argv)
{
	if (argc < 1 || !g_pCheatMgr) return;

	CParsedMsg cMsg( argc, argv );

	if (g_pCheatMgr->Check( cMsg ))
	{
		g_pClientSoundMgr->PlayInterfaceSound("Interface\\Menu\\Snd\\Cheat.wav");
	}
}


CCheatMgr::CheatInfo CCheatMgr::s_CheatInfo[] = {

#ifdef _FINAL  // Retail cheat names (what the world sees)

    { "FMC",			LTFALSE },	// god				- can't die
    { "@KJK",			LTFALSE },	// ammo				- full ammo
    { "@RJK_",			LTFALSE },	// armor			- full armor
    { "ICNJQT",			LTFALSE },	// health			- full health
    { "QMKR@^@MPY[",	LTFALSE },	// poltergeist		- AI won't see you (can still hear you)
    { "QMP",			LTFALSE },	// pos				- give world position
    { "JFN",			LTFALSE },	// kfa				- god, all weapons
    { "FSM_",			LTFALSE },	// guns				- all weapons
    { "UCNP^",			LTFALSE },  // tears			- All weapons, infinite ammo
    { "SMPAO_O",		LTFALSE },  // rosebud			- Spawn in a snowmobile
    { "LMC_",			LTFALSE },  // mods				- get all of the mods for currently held weapons
    { "FCNP",			LTFALSE },  // gear				- All gear
    { "LO_VJHN",		LTFALSE },  // maphole			- exit the current level
    { "OCgRHS\\[PUU",	LTFALSE },  // nextmission		- exit the current mission
    { "CMLR",			LTFALSE },  // boot				- boot a player
    { "CSVJA",			LTFALSE },	// build			- build info
    { "RC@P@P\\Y\\_YXHP",	LTFALSE },	// secretsquirrel	- build guid
	{ "FWJI@A^H",		LTFALSE },	// gimmegun <gun>	- give specific weapon
	{ "DUHWNUVL",		LTFALSE },	// gimmemod <mod>	- give specific mod
	{ "FWJI@WHN",		LTFALSE },	// gimmegear <gear> - give specific gear
	{ "FWJI@KVUV",		LTFALSE },	// gimmeammo <ammo> - give specific ammo
    { "NBSQ^O",			LTFALSE },  // obtuse			- enables the console in release builds (not supported currently)
    { "RIVJIf",			LTFALSE },  // skillz			- give skill points
    { "DNCCLWN",		LTFALSE },  // endgame			- unlock the chapter list as if you had completed the game
    { "COCBLNHWT",		LTFALSE },  // baddaboom		- bodies fly far with explosion damage
    { "CSCKKUJNVVP",	LTFALSE },  // budonkadonk		- toggle chase view

#else  // Development cheat names (what we're used to)...

    { "LPDKA",				LTFALSE },	// mpgod
    { "LPNIHI",				LTFALSE },	// mpammo
    { "LPNPHI]",			LTFALSE },	// mparmor
    { "LPWALH_R",			LTFALSE },	// mphealth
    { "LP_KIPN\\NK^gY",		LTFALSE },	// mppoltergeist
    { "LP_K^",				LTFALSE },	// mppos
    { "LPHDL",				LTFALSE },	// mpkfa
    { "LPDQK]",				LTFALSE },	// mpguns			- all weapons
    { "LPSAL^\\",			LTFALSE },  // mptears			- All weapons, infinite ammo
    { "LPQK^OM]M",			LTFALSE },  // mprosebud		- Spawn in a snowmobile
    { "LPJKA]",				LTFALSE },  // mpmods			- get all of the mods for currently held weapons
    { "LPDAL^",				LTFALSE },  // mpgear			- All gear
    { "LPJM]THVL",			LTFALSE },  // mpmaphole		- exit the current level
    { "LPMAePVQZY^SS",		LTFALSE },  // mpnextmission	- exit the current mission
    { "LPAKJP",				LTFALSE },  // mpboot			- boot a player
    { "LPAQTHO",			LTFALSE },	// mpbuild			- build info
    { "LPPAN^N^ZgZ]gfV^",	LTFALSE },	// mpsecretsquirrel	- build guid
	{ "LPDUHWNO\\V",		LTFALSE },	// mpgimmegun <gun> - give specific weapon
	{ "LPDUHWNUVL",			LTFALSE },	// mpgimmemod <mod> - give specific mod
	{ "LPDUHWNOLWY",		LTFALSE },	// mpgimmegear <gear> - give specific gear
	{ "LPDUHWNITST",		LTFALSE },	// mpgimmeammo <ammo> - give specific ammo
    { "LPL@Q_\\M",			LTFALSE },  // mpobtuse			- enables the console in release builds
    { "LPPWTHWd",			LTFALSE },  // mpskillz			- give skill points
    { "LPBLAAJUL",			LTFALSE },  // mpendgame		- unlock the chapter list as if you had completed the game
    { "LPAMA@JLVUR",		LTFALSE },  // mpbaddaboom		- bodies fly far with explosion damage
    { "LPN_^MJU",			LTFALSE },  // mpasscam			- toggle chase view

#endif // _FINAL

// Development only cheats...
    { "LP@JT\\",			LTFALSE },	// mpclip			- spectator mode
    { "LPSAIO[W[\\",		LTFALSE },	// mpteleport		- teleport to level start
    { "LPT^J]",				LTFALSE },  // mpwpos			- toggle position weapon adjustment
    { "LPTI]I\\",			LTFALSE },  // mpwmpos			- toggle position weapon muzzle adjustment 
    { "LPAP@KLRVNMgHX",		LTFALSE },  // mpbreachoffset	- adjust player-view breach offset
    { "LPKUBT_[JWSI",		LTFALSE },  // mplightscale		- toggle light scale adjustment
    { "LPKUBT_IML",			LTFALSE },  // mplightadd		- toggle light add adjustment
    { "LPEKS",				LTFALSE },  // mpfov			- toggle fov adjustment
    { "LPAKdS\\]JQ",		LTFALSE },  // mpboyisuck		- Remove all AI in the level
    { "LPSPTA@M[Y",			LTFALSE },  // mptriggers		- Toggle trigger boxes on/off
	{ "LP@MHO]I",			LTFALSE },  // mpcamera			- Toggle 1st person camera adjust on/off
    { "LP@MH\\H[",			LTFALSE },  // mpcampos			- toggle camera pos/rot
};


LTBOOL CCheatMgr::m_bPlayerCheated = LTFALSE;



/*******************************************************************************

	CCheatMgr

*******************************************************************************/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Init()
//
//	PURPOSE:	Initializes the cheat manager
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Init()
{
	g_pCheatMgr = this;
    g_pLTClient->RegisterConsoleProgram("Cheat", CheatFn);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Check()
//
//	PURPOSE:	See if a string is a cheat code
//
// ----------------------------------------------------------------------- //

LTBOOL CCheatMgr::Check( CParsedMsg &cMsg )
{
	char buf[100];

	// copy their text
	strncpy(buf, cMsg.GetArg(0).c_str(), sizeof(buf)-1);

	// convert it to cheat compatible text
    unsigned int i;
    for ( i = 0; i < strlen(cMsg.GetArg(0).c_str()); i++ )
		buf[i] = ((buf[i] ^ 38) + i) ^ 7;

	// then compare the converted text
	for ( i = 0; i < CHEAT_MAX; i++ )
	{
		if ( strcmp( buf, s_CheatInfo[i].pzText ) == 0)
		{
			return Process( (CheatCode)i, cMsg );
		}
	}
    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Process()
//
//	PURPOSE:	Calls the appropriate cheat function
//
// ----------------------------------------------------------------------- //

bool CCheatMgr::Process( CheatCode nCheatCode, CParsedMsg &cMsg )
{
	if ( nCheatCode <= CHEAT_NONE || nCheatCode >= CHEAT_MAX ) return false;

#ifdef _FINAL
	// Don't do cheats in multiplayer...
    if (IsMultiplayerGame())
	{
		// Well, okay, let them toggle between 1st and 3rd person ;)
		// and, well, blood is pretty cool...
		switch ( nCheatCode )
		{
			case CHEAT_CHASETOGGLE:
				ChaseToggle();
			break;

			case CHEAT_EXITLEVEL:	// exit the current level
				SetExitLevel();
			break;

			case CHEAT_NEXTMISSION:	// exit the current mission
				NextMission();
			break;

			case CHEAT_BOOT:		// boot players
				BootPlayer(cMsg);
			break;

			default :
				return false;
			break;
		}

		m_bPlayerCheated = LTTRUE;
		return true;
	}
#endif // _FINAL

	// process cheat codes
	switch ( nCheatCode )
	{
		case CHEAT_GOD:			// god mode toggle
			SetGodMode(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_SKILLZ:		// give em skill points
			GetSkills();
		break;

		case CHEAT_ARMOR:		// full armor
			SetArmor();
		break;

		case CHEAT_HEALTH:		// full health
			SetHealth();
		break;

		case CHEAT_EXITLEVEL:	// exit the current level
			SetExitLevel();
		break;

		case CHEAT_VERSION:		// display version info
			Version();
		break;

		case CHEAT_BUILDGUID:	// display build guid
			BuildGuid();
		break;

		case CHEAT_INVISIBLE:	// time to mess with the AI
			SetInvisible(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_BODYGOLFING:	// bodies fly far	
			BodyGolfing(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_POS:			// show/hide player position
			SetPos(!s_CheatInfo[nCheatCode].bActive);
		break;

#ifndef _TO2DEMO
		
		case CHEAT_KFA:			// give em everything
			SetKFA();
		break;

		case CHEAT_AMMO:		// full ammo
			SetAmmo();
		break;

		case CHEAT_MODSQUAD:	// give all mods for current weapons
			ModSquad();
		break;

		case CHEAT_CONSOLE:
			SetConsole(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_NEXTMISSION:	// exit the current mission
			NextMission();
		break;

		case CHEAT_BOOT:	// exit the current mission
			BootPlayer(cMsg);
		break;

		case CHEAT_FULL_WEAPONS:   // give all weapons
			SetFullWeapons();
		break;

		case CHEAT_SNOWMOBILE:	  // spawn in snowmobile
			Snowmobile(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_FULL_GEAR:	// give all gear
			FullGear();
		break;

		case CHEAT_TEARS:	      // toggle tears cheat
			Tears(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_GIMMEGUN:
			GimmeGun( cMsg );
		break;

		case CHEAT_GIMMEMOD:
			GimmeMod( cMsg );
		break;

		case CHEAT_GIMMEGEAR:
			GimmeGear( cMsg );
		break;

		case CHEAT_GIMMEAMMO:
			GimmeAmmo( cMsg );
		break;

		case CHEAT_ENDGAME:
			ToggleEndgameFlag();
		break;

#ifndef _FINAL

		case CHEAT_CHASETOGGLE:	   // toggle 3rd person view
			ChaseToggle();
		break;

		case CHEAT_CLIP:		// toggle clipping mode
			SetClipMode(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_TELEPORT:	// teleport to beginning
			Teleport();
		break;

		case CHEAT_CAM_POSROT:    // show/hide camera position/rotation
			SetCamPosRot(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_POSWEAPON:		    // toggle adjust of weapon pos
			PosWeapon(!s_CheatInfo[nCheatCode].bActive);
		break;

  		case CHEAT_POSWEAPON_MUZZLE:	// toggle adjust of weapon muzzle pos
  			PosWeaponMuzzle(!s_CheatInfo[nCheatCode].bActive);
  		break;

  		case CHEAT_WEAPON_BREACHOFFSET:	// toggle adjust of weapon breach offset pos
  			WeaponBreachOffset(!s_CheatInfo[nCheatCode].bActive);
  		break;

		case CHEAT_LIGHTSCALE:	      // toggle client light scale offset
			LightScale(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_LIGHTADD:	      // toggle client light add offset
			LightAdd(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_FOV:				// toggle fov cheat
			FOV(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_REMOVEAI:	  // remove all ai
			RemoveAI(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_TRIGGERBOX:	  // toggle trigger boxes on/off
			TriggerBox(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_POS1STCAM:	  // toggle 1st person camera adjust on/off
			Pos1stCam(!s_CheatInfo[nCheatCode].bActive);
		break;

#endif  // _FINAL
#endif // _TO2DEMO

		default:
			return false;			// skip setting global cheat indicator for unhandled cheats
	}

    m_bPlayerCheated = LTTRUE;

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SendCheatMessage()
//
//	PURPOSE:	sends a cheat to the server
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SendCheatMessage( CheatCode nCheatCode, uint32 nData )
{
	// Send the Message to the server
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_CHEAT);
	cMsg.Writeuint8((uint8)nCheatCode);
    cMsg.Writeuint32(nData);
    g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetGodMode()
//
//	PURPOSE:	Sets/resets God mode
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetGodMode(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_GOD].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_GOD, (uint32)bMode);

	if (bMode)
	{
		g_pChatMsgs->AddMessage("God Mode: ON",kMsgCheatConfirm);
	}
	else
	{
 		g_pChatMsgs->AddMessage("God Mode: OFF",kMsgCheatConfirm);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetAmmo()
//
//	PURPOSE:	Gives full ammo
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetAmmo()
{
    s_CheatInfo[CHEAT_AMMO].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_AMMO, LTTRUE);

	g_pChatMsgs->AddMessage("You can never have too many bullets...",kMsgCheatConfirm);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetArmor()
//
//	PURPOSE:	Gives full armor
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetArmor()
{
    s_CheatInfo[CHEAT_ARMOR].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_ARMOR, LTTRUE);

	g_pChatMsgs->AddMessage("We got def star, we got def star!",kMsgCheatConfirm);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetHealth()
//
//	PURPOSE:	Gives full health
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetHealth(LTBOOL bPlaySound)
{
    s_CheatInfo[CHEAT_HEALTH].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_HEALTH, LTTRUE);

	g_pChatMsgs->AddMessage("Doctor Dentz!",kMsgCheatConfirm);

	if (bPlaySound)
	{
		g_pClientSoundMgr->PlayInterfaceSound("voice\\1025.wav");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetClipMode()
//
//	PURPOSE:	Sets/resets Clip mode
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetClipMode(LTBOOL bMode)
{
#ifdef _FINAL
	// Can't do this in multiplayer.
	if(IsMultiplayerGame())
		return;
#endif // _FINAL

	if (!g_pPlayerMgr) return;

	s_CheatInfo[CHEAT_CLIP].bActive = bMode;

	g_pPlayerMgr->SetSpectatorMode(bMode);

	// Tell the server
	SendCheatMessage(CHEAT_CLIP, bMode);

	if (bMode)
	{
        g_pChatMsgs->AddMessage("Spectator mode enabled",kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("Spectator mode disabled",kMsgCheatConfirm);
	}

	// If we were in invisible mode before changing to spectator mode
	// change us back...

	if (!g_pPlayerMgr->IsSpectatorMode() && g_pPlayerMgr->IsInvisibleMode())
	{
		SetInvisible(LTTRUE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetInvisible()
//
//	PURPOSE:	Sets/resets invisibility
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetInvisible(LTBOOL bMode)
{
#ifdef _FINAL
	// Can't do this in multiplayer.
	if(IsMultiplayerGame())
		return;
#endif // _FINAL

	// Don't ghost when in spectator mode...

	if (!g_pPlayerMgr || g_pPlayerMgr->IsSpectatorMode()) return;

	s_CheatInfo[CHEAT_INVISIBLE].bActive = bMode;

	g_pPlayerMgr->SetInvisibleMode(bMode);

	// Tell the server
	SendCheatMessage(CHEAT_INVISIBLE, bMode);

	if (bMode)
	{
        g_pChatMsgs->AddMessage("I see dead people...", kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("I live...again.", kMsgCheatConfirm);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Teleport()
//
//	PURPOSE:	Teleports to beginning of level
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Teleport()
{
    SendCheatMessage(CHEAT_TELEPORT, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetFullWeapons()
//
//	PURPOSE:	Give us all the weapons
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetFullWeapons()
{
    s_CheatInfo[CHEAT_FULL_WEAPONS].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_FULL_WEAPONS, LTTRUE);

    g_pChatMsgs->AddMessage("Lots of guns...",kMsgCheatConfirm);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetKFA()
//
//	PURPOSE:	Give us all weapons, ammo, armor, and health...
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetKFA()
{
    s_CheatInfo[CHEAT_KFA].bActive = LTTRUE;

	// Give us all weapons, ammo, armor, and health...
	SetFullWeapons(); // Gives us all ammo too
	SetHealth(LTFALSE);
	SetArmor();

    g_pChatMsgs->AddMessage("Knock em out the box Luke...",kMsgCheatConfirm);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::GetSkills()
//
//	PURPOSE:	Give us skill points
//
// ----------------------------------------------------------------------- //

void CCheatMgr::GetSkills()
{
    s_CheatInfo[CHEAT_SKILLZ].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_SKILLZ, 0);


    g_pChatMsgs->AddMessage("You got mad skillz, d00d...",kMsgCheatConfirm);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetExitLevel()
//
//	PURPOSE:	Exit the current level
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetExitLevel()
{
    s_CheatInfo[CHEAT_EXITLEVEL].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage( CHEAT_EXITLEVEL, 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::NextMission()
//
//	PURPOSE:	Exit the current mission
//
// ----------------------------------------------------------------------- //

void CCheatMgr::NextMission()
{
    s_CheatInfo[CHEAT_NEXTMISSION].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage( CHEAT_NEXTMISSION, 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::BootPlayer()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CCheatMgr::BootPlayer(CParsedMsg &cMsg)
{
	if (!IsMultiplayerGame()) return;

	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;

	if( cMsg.GetArgCount() < 2 )
		return;

	// The full name of the player might be split between several 
	// arguments of the message so build the name from all arguments
	// except the name of the actual cheat (Arg 1).
	
	char szPlayerName[MAX_PLAYER_NAME] = {0};
	cMsg.ReCreateMsg( szPlayerName, sizeof( szPlayerName ), 1 );

	CLIENT_INFO* pInfo = pCIMgr->GetFirstClient();
	while (pInfo && stricmp(pInfo->sName.c_str(),szPlayerName) != 0)
		pInfo = pInfo->pNext;

	if (pInfo)
	{

		// Tell the server
		SendCheatMessage( CHEAT_BOOT, pInfo->nID );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::ModSquad()
//
//	PURPOSE:	Give us all the mods
//
// ----------------------------------------------------------------------- //

void CCheatMgr::ModSquad()
{
    s_CheatInfo[CHEAT_MODSQUAD].bActive = LTTRUE;

	g_pChatMsgs->AddMessage("Groovy",kMsgCheatConfirm);

	// Tell the server
    SendCheatMessage(CHEAT_MODSQUAD, LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::FullGear()
//
//	PURPOSE:	Give us all gear
//
// ----------------------------------------------------------------------- //

void CCheatMgr::FullGear()
{
    s_CheatInfo[CHEAT_FULL_GEAR].bActive = LTTRUE;

	// Tell the server
    SendCheatMessage(CHEAT_FULL_GEAR, LTTRUE);

    g_pChatMsgs->AddMessage("Gear! (as in 'Boy the Beatles sure are Gear!')",kMsgCheatConfirm);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetPos()
//
//	PURPOSE:	Toggle displaying of position on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetPos(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_POS].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ShowPlayerPos((bMode==LTTRUE));
	}

	if (bMode)
	{
        g_pChatMsgs->AddMessage("Show position enabled.",kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("Show position disabled.",kMsgCheatConfirm);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetCamPosRot()
//
//	PURPOSE:	Toggle displaying of camera pos/rot on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetCamPosRot(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_CAM_POSROT].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ShowCamPosRot((bMode==LTTRUE));
	}

	if (bMode)
	{
        g_pChatMsgs->AddMessage("Show Camera Pos/Rot enabled.",kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("Show Camera Pos/Rot disabled.",kMsgCheatConfirm);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::PosWeapon()
//
//	PURPOSE:	Toggle positioning of player view weapon on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::PosWeapon(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_POSWEAPON].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_POSWEAPON, bMode);

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_POSWEAPON);
	}

	if (bMode)
	{
        g_pChatMsgs->AddMessage("Adjust player-view weapon position: ON",kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("Adjust player-view weapon position: OFF",kMsgCheatConfirm);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::PosWeaponMuzzle()
//
//	PURPOSE:	Toggle positioning of player view weapon muzzle on/off
//
// ----------------------------------------------------------------------- //
  
void CCheatMgr::PosWeaponMuzzle(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_POSWEAPON_MUZZLE].bActive = bMode;
  
  	// Tell the server
  	SendCheatMessage(CHEAT_POSWEAPON_MUZZLE, bMode);
  
  	if (g_pGameClientShell)
  	{
  		g_pGameClientShell->ToggleDebugCheat(CHEAT_POSWEAPON_MUZZLE);
  	}
  
  	if (bMode)
  	{
          g_pChatMsgs->AddMessage("Adjust player-view weapon muzzle position: ON",kMsgCheatConfirm);
  	}
  	else
  	{
          g_pChatMsgs->AddMessage("Adjust player-view weapon muzzle position: OFF",kMsgCheatConfirm);
  	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::WeaponBreachOffset()
//
//	PURPOSE:	Toggle positioning of player view breach offset on/off
//
// ----------------------------------------------------------------------- //
  
void CCheatMgr::WeaponBreachOffset(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_WEAPON_BREACHOFFSET].bActive = bMode;
  
  	// Tell the server
  	SendCheatMessage(CHEAT_WEAPON_BREACHOFFSET, bMode);
  
  	if (g_pGameClientShell)
  	{
  		g_pGameClientShell->ToggleDebugCheat(CHEAT_WEAPON_BREACHOFFSET);
  	}
  
  	if (bMode)
  	{
          g_pChatMsgs->AddMessage("Adjust player-view weapon breach offset: ON",kMsgCheatConfirm);
  	}
  	else
  	{
          g_pChatMsgs->AddMessage("Adjust player-view weapon breach offset: OFF",kMsgCheatConfirm);
  	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::LightScale()
//
//	PURPOSE:	Toggle adjustment of light scale offset on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::LightScale(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_LIGHTSCALE].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_LIGHTSCALE);
	}

	if (bMode)
	{
        g_pChatMsgs->AddMessage("Adjust Light Scale: ON",kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("Adjust Light Scale: OFF",kMsgCheatConfirm);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::LightAdd()
//
//	PURPOSE:	Toggle adjustment of light add offset on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::LightAdd(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_LIGHTADD].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_LIGHTADD);
	}

	if (bMode)
	{
        g_pChatMsgs->AddMessage("Adjust Light Add: ON",kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("Adjust Light Add: OFF",kMsgCheatConfirm);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::FOV()
//
//	PURPOSE:	Toggle adjustment of FOV on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::FOV(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_FOV].bActive = bMode;

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_FOV);
	}

	if (bMode)
	{
        g_pChatMsgs->AddMessage("Adjust FOV: ON",kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("Adjust FOV: OFF",kMsgCheatConfirm);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Tears()
//
//	PURPOSE:	Toggle tears cheat on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Tears(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_TEARS].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_TEARS, bMode);

	if (bMode)
	{
        g_bInfiniteAmmo = true;
        g_pChatMsgs->AddMessage("RAWWR!!!",kMsgCheatConfirm);
	}
	else
	{
        g_bInfiniteAmmo = false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::RemoveAI()
//
//	PURPOSE:	Remove all AI in the level
//
// ----------------------------------------------------------------------- //

void CCheatMgr::RemoveAI(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_REMOVEAI].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_REMOVEAI, bMode);

	if (bMode)
	{
        g_pChatMsgs->AddMessage("Cheaters never prosper...",kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("Sheeze, you really ARE pathetic...",kMsgCheatConfirm);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::TriggerBox()
//
//	PURPOSE:	Toggle trigger boxes on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::TriggerBox(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_TRIGGERBOX].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_TRIGGERBOX, bMode);

	if (bMode)
	{
        g_pChatMsgs->AddMessage("Ah shucks, that takes all the fun out of it...",kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("That's better sport!",kMsgCheatConfirm);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Pos1stCam()
//
//	PURPOSE:	Toggle adjusting 1st person camera on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Pos1stCam(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_POS1STCAM].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_POS1STCAM, bMode);

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_POS1STCAM);
	}

	if (bMode)
	{
        g_pChatMsgs->AddMessage("Adjust 1st person camera offset: ON",kMsgCheatConfirm);
	}
	else
	{
        g_pChatMsgs->AddMessage("Adjust 1st person camera offset: OFF",kMsgCheatConfirm);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Snowmobile()
//
//	PURPOSE:	Spawn in a snowmobile
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Snowmobile(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_SNOWMOBILE].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_SNOWMOBILE, bMode);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::ChaseToggle()
//
//	PURPOSE:	Toggle 3rd person camera on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::ChaseToggle()
{
	// Tell the server

	if (g_pGameClientShell)
	{
		g_pGameClientShell->ToggleDebugCheat(CHEAT_CHASETOGGLE);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Version()
//
//	PURPOSE:	Display version info
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Version()
{
	// Display the version info...

	g_pChatMsgs->AddMessage((char*)g_pVersionMgr->GetBuild(),kMsgCheatConfirm);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::BuildGuid()
//
//	PURPOSE:	Display build guid.  This is a way to identify a build sent
//				to specific parties.
//
// ----------------------------------------------------------------------- //

void CCheatMgr::BuildGuid()
{
	LTGUID buildGuid;
	LTGUID const* pGuidEncoded = g_pVersionMgr->GetBuildGuid( );

	buildGuid.guid.a = pGuidEncoded->guid.a - GAMEGUID.guid.a;
	buildGuid.guid.b = pGuidEncoded->guid.b - GAMEGUID.guid.b;
	buildGuid.guid.c = pGuidEncoded->guid.c - GAMEGUID.guid.c;
	for( int i = 0; i < 8; i++ )
		buildGuid.guid.d[i] = pGuidEncoded->guid.d[i] - GAMEGUID.guid.d[i];

	char szGuid[256] = "";
	sprintf( szGuid, "{ 0x%X, 0x%X, 0x%X, { 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X, 0x%X }}",
		buildGuid.guid.a, buildGuid.guid.b, buildGuid.guid.c, buildGuid.guid.d[0], 
		buildGuid.guid.d[1], buildGuid.guid.d[2], buildGuid.guid.d[3], buildGuid.guid.d[4], 
		buildGuid.guid.d[5], buildGuid.guid.d[6], buildGuid.guid.d[7] );

	g_pChatMsgs->AddMessage( szGuid, kMsgCheatConfirm );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetConsole
//
//	PURPOSE:	Enables the console.
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetConsole( LTBOOL bMode )
{
	s_CheatInfo[CHEAT_CONSOLE].bActive = bMode;

	WriteConsoleInt( "ConsoleEnable", bMode ? 1 : 0 );

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::GimmeGun()
//
//	PURPOSE:	Give the specific gun to the player
//
// ----------------------------------------------------------------------- //

void CCheatMgr::GimmeGun( CParsedMsg &cMsg )
{
	if( !g_pWeaponMgr || cMsg.GetArgCount() < 2 )
		return;

	// The full name of the weapon might be split between several 
	// arguments of the message so build the name from all arguments
	// except the name of the actual cheat (Arg 1).
	
	char szWeaponName[WMGR_MAX_NAME_LENGTH] = {0};
	cMsg.ReCreateMsg( szWeaponName, sizeof( szWeaponName ), 1 );

	const WEAPON *pWeapon = LTNULL;

	if( (szWeaponName[0] >= '0' && szWeaponName[0] <= '9') && strlen(szWeaponName) < 4 )
	{
		pWeapon = g_pWeaponMgr->GetWeapon( atoi( szWeaponName ));
	}
	else
	{
		pWeapon = g_pWeaponMgr->GetWeapon( szWeaponName );
	}

	char szMessage[256] = {0};

	if( pWeapon )
	{
		SendCheatMessage( CHEAT_GIMMEGUN, pWeapon->nId );

		sprintf( szMessage, "Giving weapon '%s' ID: %i", pWeapon->szName, pWeapon->nId );
		g_pChatMsgs->AddMessage( szMessage, kMsgCheatConfirm );
	}
	else
	{
		sprintf( szMessage, "Weapon '%s' does not exist!", szWeaponName );
		g_pChatMsgs->AddMessage( szMessage, kMsgCheatConfirm );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Gimmemod()
//
//	PURPOSE:	Give the specific mod to the player
//
// ----------------------------------------------------------------------- //

void CCheatMgr::GimmeMod( CParsedMsg &cMsg )
{
	if( !g_pWeaponMgr || cMsg.GetArgCount() < 2 )
		return;

	// The full name of the mod might be split between several 
	// arguments of the message so build the name from all arguments
	// except the name of the actual cheat (Arg 1).
	
	char szModName[WMGR_MAX_NAME_LENGTH] = {0};
	cMsg.ReCreateMsg( szModName, sizeof( szModName ), 1 );

	const MOD *pMod = LTNULL;

	if( (szModName[0] >= '0' && szModName[0] <= '9') && strlen(szModName) < 4 )
	{
		pMod = g_pWeaponMgr->GetMod( atoi( szModName ));
	}
	else
	{
		pMod = g_pWeaponMgr->GetMod( szModName );
	}

	char szMessage[256] = {0};

	if( pMod )
	{
		SendCheatMessage( CHEAT_GIMMEMOD, pMod->nId );

		sprintf( szMessage, "Giving mod '%s' ID: %i", pMod->szName, pMod->nId );
		g_pChatMsgs->AddMessage( szMessage, kMsgCheatConfirm );
	}
	else
	{
		sprintf( szMessage, "Mod '%s' does not exist!", szModName );
		g_pChatMsgs->AddMessage( szMessage, kMsgCheatConfirm );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::GimmeGear()
//
//	PURPOSE:	Give the specific gear to the player
//
// ----------------------------------------------------------------------- //

void CCheatMgr::GimmeGear( CParsedMsg &cMsg )
{
	if( !g_pWeaponMgr || cMsg.GetArgCount() < 2 )
		return;

	// The full name of the gear might be split between several 
	// arguments of the message so build the name from all arguments
	// except the name of the actual cheat (Arg 1).
	
	char szGearName[WMGR_MAX_NAME_LENGTH] = {0};
	cMsg.ReCreateMsg( szGearName, sizeof( szGearName ), 1 );

	const GEAR *pGear = LTNULL;

	if( (szGearName[0] >= '0' && szGearName[0] <= '9') && strlen(szGearName) < 4 )
	{
		pGear = g_pWeaponMgr->GetGear( atoi( szGearName ));
	}
	else
	{
		pGear = g_pWeaponMgr->GetGear( szGearName );
	}

	char szMessage[256] = {0};

	if( pGear )
	{
		SendCheatMessage( CHEAT_GIMMEGEAR, pGear->nId );

		sprintf( szMessage, "Giving gear '%s' ID: %i", pGear->szName, pGear->nId );
		g_pChatMsgs->AddMessage( szMessage, kMsgCheatConfirm );
	}
	else
	{
		sprintf( szMessage, "Gear '%s' does not exist!", szGearName );
		g_pChatMsgs->AddMessage( szMessage, kMsgCheatConfirm );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::GimmeAmmo()
//
//	PURPOSE:	Give the specific ammo to the player
//
// ----------------------------------------------------------------------- //

void CCheatMgr::GimmeAmmo( CParsedMsg &cMsg )
{
	if( !g_pWeaponMgr || cMsg.GetArgCount() < 2 )
		return;

	// The full name of the ammo might be split between several 
	// arguments of the message so build the name from all arguments
	// except the name of the actual cheat (Arg 1).
	
	char szAmmoName[WMGR_MAX_NAME_LENGTH] = {0};
	cMsg.ReCreateMsg( szAmmoName, sizeof( szAmmoName ), 1 );

	const AMMO *pAmmo = LTNULL;

	if( (szAmmoName[0] >= '0' && szAmmoName[0] <= '9') && strlen(szAmmoName) < 4 )
	{
		pAmmo = g_pWeaponMgr->GetAmmo( atoi( szAmmoName ));
	}
	else
	{
		pAmmo = g_pWeaponMgr->GetAmmo( szAmmoName );
	}

	char szMessage[256] = {0};

	if( pAmmo )
	{
		SendCheatMessage( CHEAT_GIMMEAMMO, pAmmo->nId );

		sprintf( szMessage, "Giving ammo '%s' ID: %i", pAmmo->szName, pAmmo->nId );
		g_pChatMsgs->AddMessage( szMessage, kMsgCheatConfirm );
	}
	else
	{
		sprintf( szMessage, "Ammo '%s' does not exist!", szAmmoName );
		g_pChatMsgs->AddMessage( szMessage, kMsgCheatConfirm );
	}
}	




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::ToggleEndgameFlag()
//
//	PURPOSE:	Toggle the registry entry indicating the player has completed the game...
//
// ----------------------------------------------------------------------- //

void CCheatMgr::ToggleEndgameFlag()
{
	
	CRegMgr* pRegMgr = g_pVersionMgr->GetRegMgr();
	if (pRegMgr->IsValid())
	{
		uint32 nDone = pRegMgr->Get("EndGame",0);
		pRegMgr->Set("EndGame",!nDone);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::BodyGolfing()
//
//	PURPOSE:	Toggle bodygolfing cheat on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::BodyGolfing(LTBOOL bMode)
{
	s_CheatInfo[CHEAT_BODYGOLFING].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_BODYGOLFING, bMode);

	if (bMode)
	{
        g_pChatMsgs->AddMessage("FORE!",kMsgCheatConfirm);
	}
}
