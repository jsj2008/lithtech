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
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "HUDMessageQueue.h"
#include "HUDDebug.h"
#include "sys/win/mpstrconv.h"
#include "ltprofileutils.h"

CCheatMgr*      g_pCheatMgr     = NULL;
bool             g_bInfiniteAmmo = false;

void CheatFn(int argc, char **argv)
{
	// Track the current execution shell scope for proper SEM behavior
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	if (argc < 1 || !g_pCheatMgr) return;

	// We need to store converted strings.
	std::vector< std::wstring > aStrings;
	aStrings.resize( argc );

	CParsedMsgW cMsg;
	cMsg.Init( argc, NULL );

	for( int i = 0; i < argc; i++ )
	{
		aStrings[i] = MPA2W( argv[i] ).c_str( );
		CParsedMsgW::CToken token = aStrings[i].c_str( );
		cMsg.SetArg( i, token );
	}

	if (g_pCheatMgr->Check( cMsg ))
	{
//		g_pClientSoundMgr->PlayInterfaceDBSound("Cheat");
	}
}


CCheatMgr::CheatInfo CCheatMgr::s_CheatInfo[] = {

#ifdef _FINAL  // Retail cheat names (what the world sees)

	{ L"FMC",			false },	// god				- can't die
	{ L"@KJK",			false },	// ammo				- full ammo
	{ L"@RJK_",			false },	// armor			- full armor
	{ L"ICNJQT",			false },	// health			- full health
	{ L"QMKR@^@MPY[",	false },	// poltergeist		- AI won't see you (can still hear you)
	{ L"QMP",			false },	// pos				- give world position
	{ L"JFN",			false },	// kfa				- god, all weapons
	{ L"FSM_",			false },	// guns				- all weapons
	{ L"UCNP^",			false },	// tears			- All weapons, infinite ammo
	{ L"LMC_",			false },	// mods				- get all of the mods for currently held weapons
	{ L"FCNP",			false },	// gear				- All gear
	{ L"LO_VJHN",		false },	// maphole			- exit the current level
	{ L"OCgRHS\\[PUU",	false },	// nextmission		- exit the current mission
	{ L"CMLR",			false },	// boot				- boot a player
	{ L"CSVJA",			false },	// build			- build info
	{ L"FWJI@A^H",		false },	// gimmegun <gun>	- give specific weapon
	{ L"DUHWNUVL",		false },	// gimmemod <mod>	- give specific mod
	{ L"FWJI@WHN",		false },	// gimmegear <gear> - give specific gear
	{ L"FWJI@KVUV",		false },	// gimmeammo <ammo> - give specific ammo
	{ L"NBSQ^O",			false },	// obtuse			- enables the console in release builds (not supported currently)
	{ L"DNCCLWN",		false },	// endgame			- unlock the chapter list as if you had completed the game
	{ L"COCBLNHWT",		false },	// baddaboom		- bodies fly far with explosion damage
	{ L"@QPOLW",		false },	// asscam			- toggle chase view

#else  // Development cheat names (what we're used to)...

	{ L"LPDKA",				false },	// mpgod
	{ L"LPNIHI",				false },	// mpammo
	{ L"LPNPHI]",			false },	// mparmor
	{ L"LPWALH_R",			false },	// mphealth
	{ L"LP_KIPN\\NK^gY",		false },	// mppoltergeist
	{ L"LP_K^",				false },	// mppos
	{ L"LPHDL",				false },	// mpkfa
	{ L"LPDQK]",				false },	// mpguns			- all weapons
	{ L"LPSAL^\\",			false },	// mptears			- All weapons, infinite ammo
	{ L"LPJKA]",				false },	// mpmods			- get all of the mods for currently held weapons
	{ L"LPDAL^",				false },	// mpgear			- All gear
	{ L"LPJM]THVL",			false },	// mpmaphole		- exit the current level
	{ L"LPMAePVQZY^SS",		false },	// mpnextmission	- exit the current mission
	{ L"LPAKJP",				false },	// mpboot			- boot a player
	{ L"LPAQTHO",			false },	// mpbuild			- build info
	{ L"LPDUHWNO\\V",		false },	// mpgimmegun <gun> - give specific weapon
	{ L"LPDUHWNUVL",			false },	// mpgimmemod <mod> - give specific mod
	{ L"LPDUHWNOLWY",		false },	// mpgimmegear <gear> - give specific gear
	{ L"LPDUHWNITST",		false },	// mpgimmeammo <ammo> - give specific ammo
	{ L"LPL@Q_\\M",			false },	// mpobtuse			- enables the console in release builds
	{ L"LPBLAAJUL",			false },	// mpendgame		- unlock the chapter list as if you had completed the game
	{ L"LPAMA@JLVUR",		false },	// mpbaddaboom		- bodies fly far with explosion damage
	{ L"LPN_^MJU",			false },	// mpasscam			- toggle chase view

#endif // _FINAL

// Development only cheats...
	{ L"LP@JT\\",			false },	// mpclip			- clip mode
	{ L"LPSAIO[W[\\",		false },	// mpteleport		- teleport to level start
	{ L"LPT^J]",				false },	// mpwpos			- toggle position weapon adjustment
	{ L"LPAKdS\\]JQ",		false },	// mpboyisuck		- Remove all AI in the level
	{ L"LPSPTA@M[Y",			false },	// mptriggers		- Toggle trigger boxes on/off
	{ L"LP@MHO]I",			false },	// mpcamera			- Toggle 1st person camera adjust on/off
	{ L"LP@MH\\H[",			false },	// mpcampos			- toggle camera pos/rot
	{ L"LPPMSOQMQ_HRH",		false	},	// mpsavevehicle	- save the vehicle tweaks to the bute file
};


bool CCheatMgr::m_bPlayerCheated = false;



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
//	ROUTINE:	CCheatMgr::OnEnterWorld()
//
//	PURPOSE:	Resets cheat status that will change on entering the world...
//
// ----------------------------------------------------------------------- //

void CCheatMgr::OnEnterWorld()
{
	s_CheatInfo[CHEAT_GOD].bActive = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Check()
//
//	PURPOSE:	See if a string is a cheat code
//
// ----------------------------------------------------------------------- //

bool CCheatMgr::Check( CParsedMsgW const &cMsg )
{
	wchar_t buf[100];

	// copy their text
	LTStrCpy( buf, cMsg.GetArg(0).c_str(), ARRAY_LEN(buf) );

	// convert it to cheat compatible text
    unsigned int i;
    for ( i = 0; i < LTStrLen(cMsg.GetArg(0).c_str()); i++ )
		buf[i] = (((buf[i] ^ 38) + i) ^ 7) % 256;

	// then compare the converted text
	for ( i = 0; i < CHEAT_MAX; i++ )
	{
		if ( LTStrCmp( buf, s_CheatInfo[i].pzText ) == 0)
		{
			return Process( (CheatCode)i, cMsg );
		}
	}
    return false;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Process()
//
//	PURPOSE:	Calls the appropriate cheat function
//
// ----------------------------------------------------------------------- //

bool CCheatMgr::Process( CheatCode nCheatCode, CParsedMsgW const& cMsg )
{
	if ( nCheatCode <= CHEAT_NONE || nCheatCode >= CHEAT_MAX ) return false;

#ifdef _FINAL
	// Don't do cheats in multiplayer...
    if (IsMultiplayerGameClient())
	{
		// Well, okay, let them toggle between 1st and 3rd person ;)
		// and, well, blood is pretty cool...
		switch ( nCheatCode )
		{
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

		m_bPlayerCheated = true;
		return true;
	}

#else // _FINAL

	// Only allow cheats in MP if console variable turned on.
	if( IsMultiplayerGameClient( ))
	{
		float fVal = 0.0f;
		g_pLTClient->GetSConValueFloat( "AllowMPCheats", fVal );
		bool bAllowMPCheats = fVal != 0.0f;
		if( !bAllowMPCheats )
			return false;
	}

#endif // _FINAL

	// process cheat codes
	switch ( nCheatCode )
	{
		case CHEAT_GOD:			// god mode toggle
			SetGodMode(!s_CheatInfo[nCheatCode].bActive);
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

		case CHEAT_INVISIBLE:	// time to mess with the AI
			SetInvisible(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_BODYGOLFING:	// bodies fly far
			//currently unimplemented...
//			BodyGolfing(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_POS:			// show/hide player position
			SetPos(!s_CheatInfo[nCheatCode].bActive);
		break;

#ifndef _DEMO
		
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

		case CHEAT_REMOVEAI:	  // remove all ai
			RemoveAI(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_TRIGGERBOX:	  // toggle trigger boxes on/off
			TriggerBox(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_POS1STCAM:	  // toggle 1st person camera adjust on/off
			Pos1stCam(!s_CheatInfo[nCheatCode].bActive);
		break;

		case CHEAT_SAVEVEHICLE:		// save the vehicle tweaks to the bute file
			SaveVehicle( );
		break;

#endif  // _FINAL
#endif // _DEMO

		default:
			return false;			// skip setting global cheat indicator for unhandled cheats
	}

    m_bPlayerCheated = true;

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
//	ROUTINE:	CCheatMgr::SendCheatMessage()
//
//	PURPOSE:	sends a cheat to the server
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SendCheatMessageHRecord( CheatCode nCheatCode, HRECORD hRecord )
{
	// Send the Message to the server
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_CHEAT);
	cMsg.Writeuint8((uint8)nCheatCode);
	cMsg.WriteDatabaseRecord( g_pLTDatabase, hRecord );
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetGodMode()
//
//	PURPOSE:	Sets/resets God mode
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetGodMode(bool bMode)
{
	s_CheatInfo[CHEAT_GOD].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_GOD, (uint32)bMode);

	if (bMode)
	{
		g_pGameMsgs->AddMessage(L"God Mode: ON",kMsgCheatConfirm);
	}
	else
	{
 		g_pGameMsgs->AddMessage(L"God Mode: OFF",kMsgCheatConfirm);
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
    s_CheatInfo[CHEAT_AMMO].bActive = true;

	// Tell the server
    SendCheatMessage(CHEAT_AMMO, true);

//	g_pGameMsgs->AddMessage(L"You can never have too many bullets...",kMsgCheatConfirm);
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
    s_CheatInfo[CHEAT_ARMOR].bActive = true;

	// Tell the server
    SendCheatMessage(CHEAT_ARMOR, true);

//	g_pGameMsgs->AddMessage(L"We got def star, we got def star!",kMsgCheatConfirm);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetHealth()
//
//	PURPOSE:	Gives full health
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetHealth(bool bPlaySound)
{
    s_CheatInfo[CHEAT_HEALTH].bActive = true;

	// Tell the server
    SendCheatMessage(CHEAT_HEALTH, true);

//	g_pGameMsgs->AddMessage(L"Doctor Dentz!",kMsgCheatConfirm);

	if (bPlaySound)
	{
		g_pClientSoundMgr->PlayInterfaceDBSound("CheatHealth");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetClipMode()
//
//	PURPOSE:	Sets/resets Clip mode
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetClipMode(bool bMode)
{
#ifdef _FINAL
	// Can't do this in multiplayer.
	if(IsMultiplayerGameClient())
		return;
#endif // _FINAL

	if (!g_pPlayerMgr) return;

	s_CheatInfo[CHEAT_CLIP].bActive = bMode;

	if( !bMode )
	{
		// Force a send of our position to the server so they know where to put us.
		g_pPlayerMgr->UpdatePlayerInfo( true, true );
	}

	// Tell the server
	SendCheatMessage(CHEAT_CLIP, bMode ? eSpectatorMode_Clip : eSpectatorMode_None );

	if (bMode)
	{
        g_pGameMsgs->AddMessage(L"Clip mode enabled",kMsgCheatConfirm);
	}
	else
	{
        g_pGameMsgs->AddMessage(L"Clip mode disabled",kMsgCheatConfirm);
	}

	// If we were in invisible mode before changing to clip mode
	// change us back...

	if (!bMode && g_pPlayerMgr->IsInvisibleMode())
	{
		SetInvisible(true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetInvisible()
//
//	PURPOSE:	Sets/resets invisibility
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetInvisible(bool bMode)
{
#ifdef _FINAL
	// Can't do this in multiplayer.
	if(IsMultiplayerGameClient())
		return;
#endif // _FINAL

	// Don't ghost when in clip mode...

	if (!g_pPlayerMgr || g_pPlayerMgr->IsSpectating()) return;

	s_CheatInfo[CHEAT_INVISIBLE].bActive = bMode;

	g_pPlayerMgr->SetInvisibleMode(bMode);

	// Tell the server
	SendCheatMessage(CHEAT_INVISIBLE, bMode);

	if (bMode)
	{
        g_pGameMsgs->AddMessage(L"I see dead people...", kMsgCheatConfirm);
	}
	else
	{
        g_pGameMsgs->AddMessage(L"I live...again.", kMsgCheatConfirm);
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
    SendCheatMessage(CHEAT_TELEPORT, true);
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
    s_CheatInfo[CHEAT_FULL_WEAPONS].bActive = true;

	// Tell the server
    SendCheatMessage(CHEAT_FULL_WEAPONS, true);

//    g_pGameMsgs->AddMessage(L"Lots of guns...",kMsgCheatConfirm);
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
    s_CheatInfo[CHEAT_KFA].bActive = true;

	// Give us all weapons, ammo, armor, and health...
	SetFullWeapons(); // Gives us all ammo too
	SetHealth(false);
	SetArmor();

//    g_pGameMsgs->AddMessage(L"Knock em out the box Luke...",kMsgCheatConfirm);
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
    s_CheatInfo[CHEAT_EXITLEVEL].bActive = true;

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
    s_CheatInfo[CHEAT_NEXTMISSION].bActive = true;

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

void CCheatMgr::BootPlayer(CParsedMsgW const& cMsg)
{
	if (!IsMultiplayerGameClient()) return;

	CClientInfoMgr *pCIMgr = g_pGameClientShell->GetInterfaceMgr( )->GetClientInfoMgr();
	if (!pCIMgr) return;

	if( cMsg.GetArgCount() < 2 )
		return;

	// The full name of the player might be split between several 
	// arguments of the message so build the name from all arguments
	// except the name of the actual cheat (Arg 1).
	
	wchar_t szPlayerName[MAX_PLAYER_NAME] = {0};
	cMsg.ReCreateMsg( szPlayerName, LTARRAYSIZE( szPlayerName ), 1 );

	CLIENT_INFO* pInfo = pCIMgr->GetFirstClient();
	while (pInfo && LTStrICmp(pInfo->sName.c_str(),szPlayerName) != 0)
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
    s_CheatInfo[CHEAT_MODSQUAD].bActive = true;

//	g_pGameMsgs->AddMessage(L"Groovy",kMsgCheatConfirm);

	// Tell the server
    SendCheatMessage(CHEAT_MODSQUAD, true);
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
    s_CheatInfo[CHEAT_FULL_GEAR].bActive = true;

	// Tell the server
    SendCheatMessage(CHEAT_FULL_GEAR, true);

//    g_pGameMsgs->AddMessage(L"Gear! (as in 'Boy the Beatles sure are Gear!')",kMsgCheatConfirm);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetPos()
//
//	PURPOSE:	Toggle displaying of position on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetPos(bool bMode)
{
	s_CheatInfo[CHEAT_POS].bActive = bMode;

	if (g_pHUDDebug)
	{
		g_pHUDDebug->ShowPlayerPos((bMode==true));
	}

	if (bMode)
	{
        g_pGameMsgs->AddMessage(L"Show position enabled.",kMsgCheatConfirm);
	}
	else
	{
        g_pGameMsgs->AddMessage(L"Show position disabled.",kMsgCheatConfirm);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetCamPosRot()
//
//	PURPOSE:	Toggle displaying of camera pos/rot on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetCamPosRot(bool bMode)
{
	s_CheatInfo[CHEAT_CAM_POSROT].bActive = bMode;

	if (g_pHUDDebug)
	{
		g_pHUDDebug->ShowCamPosRot((bMode==true));
	}

	if (bMode)
	{
        g_pGameMsgs->AddMessage(L"Show Camera Pos/Rot enabled.",kMsgCheatConfirm);
	}
	else
	{
        g_pGameMsgs->AddMessage(L"Show Camera Pos/Rot disabled.",kMsgCheatConfirm);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::PosWeapon()
//
//	PURPOSE:	Toggle positioning of player view weapon on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::PosWeapon(bool bMode)
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
        g_pGameMsgs->AddMessage(L"Adjust player-view weapon position: ON",kMsgCheatConfirm);
	}
	else
	{
        g_pGameMsgs->AddMessage(L"Adjust player-view weapon position: OFF",kMsgCheatConfirm);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Tears()
//
//	PURPOSE:	Toggle tears cheat on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Tears(bool bMode)
{
	s_CheatInfo[CHEAT_TEARS].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_TEARS, bMode);

	if (bMode)
	{
        g_bInfiniteAmmo = true;
//        g_pGameMsgs->AddMessage(L"RAWWR!!!",kMsgCheatConfirm);
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

void CCheatMgr::RemoveAI(bool bMode)
{
	s_CheatInfo[CHEAT_REMOVEAI].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_REMOVEAI, bMode);

	if (bMode)
	{
//        g_pGameMsgs->AddMessage(L"Cheaters never prosper...",kMsgCheatConfirm);
	}
	else
	{
//        g_pGameMsgs->AddMessage(L"Sheeze, you really ARE pathetic...",kMsgCheatConfirm);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::TriggerBox()
//
//	PURPOSE:	Toggle trigger boxes on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::TriggerBox(bool bMode)
{
	s_CheatInfo[CHEAT_TRIGGERBOX].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_TRIGGERBOX, bMode);

	if (bMode)
	{
//        g_pGameMsgs->AddMessage(L"Ah shucks, that takes all the fun out of it...",kMsgCheatConfirm);
	}
	else
	{
//        g_pGameMsgs->AddMessage(L"That's better sport!",kMsgCheatConfirm);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Pos1stCam()
//
//	PURPOSE:	Toggle adjusting 1st person camera on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::Pos1stCam(bool bMode)
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
        g_pGameMsgs->AddMessage(L"Adjust 1st person camera offset: ON",kMsgCheatConfirm);
	}
	else
	{
        g_pGameMsgs->AddMessage(L"Adjust 1st person camera offset: OFF",kMsgCheatConfirm);
	}
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
	std::wstring sBuild;
	sBuild = MPA2W( g_pVersionMgr->GetBuild());
	g_pGameMsgs->AddMessage( sBuild.c_str( ),kMsgCheatConfirm);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SetConsole
//
//	PURPOSE:	Enables the console.
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SetConsole( bool bMode )
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

void CCheatMgr::GimmeGun( CParsedMsgW const& cMsg )
{
	if( !g_pWeaponDB || cMsg.GetArgCount() < 2 )
		return;

	// The full name of the weapon might be split between several 
	// arguments of the message so build the name from all arguments
	// except the name of the actual cheat (Arg 1).
	
	wchar_t szWeaponName[WMGR_MAX_NAME_LENGTH] = {0};
	cMsg.ReCreateMsg( szWeaponName, LTARRAYSIZE(szWeaponName), 1 );

	HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord( MPW2A(szWeaponName).c_str() );
	
	wchar_t wszMessage[256] = {0};

	if( hWeapon )
	{
		SendCheatMessageHRecord( CHEAT_GIMMEGUN, hWeapon );

		LTSNPrintF( wszMessage, ARRAY_LEN(wszMessage), L"Giving weapon '%s'", szWeaponName );
		g_pGameMsgs->AddMessage( wszMessage, kMsgCheatConfirm );
	}
	else
	{
		LTSNPrintF( wszMessage, ARRAY_LEN(wszMessage), L"Weapon '%s' does not exist!", szWeaponName );
		g_pGameMsgs->AddMessage( wszMessage, kMsgCheatConfirm );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::Gimmemod()
//
//	PURPOSE:	Give the specific mod to the player
//
// ----------------------------------------------------------------------- //

void CCheatMgr::GimmeMod( CParsedMsgW const& cMsg )
{
	if( !g_pWeaponDB || cMsg.GetArgCount() < 2 )
		return;

	// The full name of the mod might be split between several 
	// arguments of the message so build the name from all arguments
	// except the name of the actual cheat (Arg 1).
	
	wchar_t szModName[WMGR_MAX_NAME_LENGTH] = {0};
	cMsg.ReCreateMsg( szModName, LTARRAYSIZE( szModName ), 1 );

	HMOD hMod = g_pWeaponDB->GetModRecord( MPW2A(szModName).c_str() );

	wchar_t wszMessage[256] = L"";

	if( hMod )
	{
		SendCheatMessageHRecord( CHEAT_GIMMEMOD, hMod );

		LTSNPrintF( wszMessage, ARRAY_LEN(wszMessage), L"Giving mod '%s'", szModName );
		g_pGameMsgs->AddMessage( wszMessage, kMsgCheatConfirm );
	}
	else
	{
		LTSNPrintF( wszMessage, ARRAY_LEN(wszMessage), L"Mod '%s' does not exist!", szModName );
		g_pGameMsgs->AddMessage( wszMessage, kMsgCheatConfirm );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::GimmeGear()
//
//	PURPOSE:	Give the specific gear to the player
//
// ----------------------------------------------------------------------- //

void CCheatMgr::GimmeGear( CParsedMsgW const& cMsg )
{
	if( !g_pWeaponDB || cMsg.GetArgCount() < 2 )
		return;

	// The full name of the gear might be split between several 
	// arguments of the message so build the name from all arguments
	// except the name of the actual cheat (Arg 1).
	
	wchar_t szGearName[WMGR_MAX_NAME_LENGTH] = {0};
	cMsg.ReCreateMsg( szGearName, LTARRAYSIZE( szGearName ), 1 );

	HGEAR hGear = g_pWeaponDB->GetGearRecord( MPW2A(szGearName).c_str() );

	wchar_t wszMessage[256] = L"";

	if( hGear )
	{
		SendCheatMessageHRecord( CHEAT_GIMMEGEAR, hGear );

		LTSNPrintF( wszMessage, ARRAY_LEN(wszMessage), L"Giving gear '%s'", szGearName );
		g_pGameMsgs->AddMessage( wszMessage, kMsgCheatConfirm );
	}
	else
	{
		LTSNPrintF( wszMessage, ARRAY_LEN(wszMessage), L"Gear '%s' does not exist!", szGearName );
		g_pGameMsgs->AddMessage( wszMessage, kMsgCheatConfirm );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::GimmeAmmo()
//
//	PURPOSE:	Give the specific ammo to the player
//
// ----------------------------------------------------------------------- //

void CCheatMgr::GimmeAmmo( CParsedMsgW const& cMsg )
{
	if( !g_pWeaponDB || cMsg.GetArgCount() < 2 )
		return;

	// The full name of the ammo might be split between several 
	// arguments of the message so build the name from all arguments
	// except the name of the actual cheat (Arg 1).
	
	wchar_t szAmmoName[WMGR_MAX_NAME_LENGTH] = {0};
	cMsg.ReCreateMsg( szAmmoName, LTARRAYSIZE( szAmmoName ), 1 );

	HAMMO hAmmo = g_pWeaponDB->GetAmmoRecord( MPW2A(szAmmoName).c_str() );

	wchar_t wszMessage[256] = L"";

	if( hAmmo )
	{
		SendCheatMessageHRecord( CHEAT_GIMMEAMMO, hAmmo );

		LTSNPrintF( wszMessage, ARRAY_LEN(wszMessage), L"Giving ammo '%s'", szAmmoName );
		g_pGameMsgs->AddMessage( wszMessage, kMsgCheatConfirm );
	}
	else
	{
		LTSNPrintF( wszMessage, ARRAY_LEN(wszMessage), L"Ammo '%s' does not exist!", szAmmoName );
		g_pGameMsgs->AddMessage( wszMessage, kMsgCheatConfirm );
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
	uint32 nDone = LTProfileUtils::ReadUint32( "Game", "EndGame", 0, g_pVersionMgr->GetGameSystemIniFile());
	LTProfileUtils::WriteUint32( "Game", "EndGame", !nDone, g_pVersionMgr->GetGameSystemIniFile());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::BodyGolfing()
//
//	PURPOSE:	Toggle bodygolfing cheat on/off
//
// ----------------------------------------------------------------------- //

void CCheatMgr::BodyGolfing(bool bMode)
{
	s_CheatInfo[CHEAT_BODYGOLFING].bActive = bMode;

	// Tell the server
	SendCheatMessage(CHEAT_BODYGOLFING, bMode);

	if (bMode)
	{
        g_pGameMsgs->AddMessage(L"FORE!",kMsgCheatConfirm);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCheatMgr::SaveVehicle()
//
//	PURPOSE:	Save the info for the current vehicle being ridden...
//
// ----------------------------------------------------------------------- //

void CCheatMgr::SaveVehicle( )
{
	if( !g_pMoveMgr || !g_pMoveMgr->GetVehicleMgr() )
		return;

	g_pMoveMgr->GetVehicleMgr()->SaveVehicleInfo();
}
