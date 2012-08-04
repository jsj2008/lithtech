// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerObj.cpp
//
// PURPOSE : Player object implementation
//
// CREATED : 9/18/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerObj.h"
#include "iltserver.h"
#include "CommandIds.h"
#include "ServerUtilities.h"
#include "HHWeaponModel.h"
#include "GameServerShell.h"
#include "SurfaceFunctions.h"
#include "iltphysics.h"
#include "PlayerButes.h"
#include "TeleportPoint.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "Spawner.h"
#include "Attachments.h"
#include "VolumeBrush.h"
#include "SurfaceMgr.h"
#include "PlayerVehicle.h"
#include "Breakable.h"
#include "MusicMgr.h"
#include "Camera.h"
#include "AISounds.h"
#include "CharacterHitBox.h"
#include "AIVolume.h"
#include "RelationMgr.h"
#include "ObjectRelationMgr.h"
#include "ServerSoundMgr.h"
#include "VersionMgr.h"
#include "Body.h"
#include "GameStartPoint.h"
#include "AIStimulusMgr.h"
#include "PlayerLure.h"
#include "AIRegion.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "MissionButeMgr.h"
#include "ServerMissionMgr.h"
#include "ServerSaveLoadMgr.h"
#include "AIHuman.h"
#include "AIBrain.h"
#include "AIGoalMgr.h"
#include "AIGoalSpecialDamage.h"
#include "PickupItem.h"
#include "DoomsDayPiece.h"

#include <stdio.h>


LINKFROM_MODULE( PlayerObj );

extern CGameServerShell* g_pGameServerShell;
extern CServerButeMgr* g_pServerButeMgr;
extern CAIStimulusMgr* g_pAIStimulusMgr;
extern char s_FileBuffer[MAX_CS_FILENAME_LEN];
extern CVarTrack g_vtNetFriendlyFire;

static CVarTrack s_vtVehicleImpactDistMin;
static CVarTrack s_vtVehicleImpactDistMax;

static CVarTrack s_vtBaseHealRate;
static CVarTrack s_vtWalkHealRate;

BEGIN_CLASS(CPlayerObj)
END_CLASS_DEFAULT_FLAGS(CPlayerObj, CCharacter, NULL, NULL, CF_HIDDEN)

CPlayerObj::PlayerObjList CPlayerObj::m_lstPlayerObjs;

static char* s_pMPBodySounds[] = { "Chars\\Snd\\MaleBodyMoveCloth_1.WAV", "Chars\\Snd\\MaleBodyMoveLeather_1.WAV" };
static char* s_pBodySounds[] = { "Chars\\Snd\\BodyMoveCloth_1.WAV", "Chars\\Snd\\BodyMoveLeather_1.WAV" };


namespace
{
	typedef std::list<PickupItem*> PickupList;
}
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateMusicMsg
//
//  PURPOSE:	Make sure the music message is valid
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateMusicMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( cpMsgParams.m_nArgs < 2 || cpMsgParams.m_nArgs > 5 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMucisMsg()" );
			pInterface->CPrint( "    MSG - MUSIC - Invalid number of parameters." );
		}
		
		return LTFALSE;
	}

	int nTimingParam = -1;
	int nRequiredParams = -1;
	LTBOOL	bValid = LTFALSE;

	if( !_stricmp( cpMsgParams.m_Args[1], "I" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "INTENSITY" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "PS" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "SECONDARY" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "SS" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "STOPSECONDARY" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "SM" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "STOPMOTIF" ))
	{
		bValid = LTTRUE;
		nRequiredParams = 3;
		nTimingParam = 4;
	}
	else if( !_stricmp( cpMsgParams.m_Args[1], "PM" ) ||
			 !_stricmp( cpMsgParams.m_Args[1], "MOTIF" ))
	{
		bValid = LTTRUE;
		nRequiredParams = 4;
		nTimingParam = 5;
	}
	else if( !_stricmp( cpMsgParams.m_Args[1], "V" ) ||
			 !_stricmp( cpMsgParams.m_Args[1], "VOLUME" ))
	{
		bValid = LTTRUE;
		nRequiredParams = 3;
		nTimingParam = -1;
	}
	else if( !_stricmp( cpMsgParams.m_Args[1], "P" ) ||
			 !_stricmp( cpMsgParams.m_Args[1], "PLAY" ))
	{
		bValid = LTTRUE;
		nRequiredParams = 2;
		nTimingParam = 4;
	}
	else if( !_stricmp( cpMsgParams.m_Args[1], "S" ) ||
			 !_stricmp( cpMsgParams.m_Args[1], "STOP" ))
	{
		bValid = LTTRUE;
		nRequiredParams = 2;
		nTimingParam = 3;
	}
	
	if( !bValid )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMucisMsg()" );
			pInterface->CPrint( "    MSG - MUSIC - '%s' action is invalid. (see PlayerMessages.doc)", cpMsgParams.m_Args[1] );
		}
		
		return LTFALSE;
	}

	if( cpMsgParams.m_nArgs < nRequiredParams ||
		(nTimingParam != -1 ) && (cpMsgParams.m_nArgs > nTimingParam) ||
		(nTimingParam == -1) && (cpMsgParams.m_nArgs > nRequiredParams) )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateMucisMsg()" );
			pInterface->CPrint( "    MSG - MUSIC - Invalid parameters for action '%s'. (see PlayerMessages.doc)", cpMsgParams.m_Args[1] );
		}
		
		return LTFALSE;
	}

	if( (nTimingParam > 0) && (cpMsgParams.m_nArgs >= nTimingParam) && (cpMsgParams.m_Args[nTimingParam-1] != LTNULL) )
	{
		
		// We have a timing value so make sure it's valid...

		if( !_stricmp( cpMsgParams.m_Args[nTimingParam-1], "Invalid" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "Default" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "Immediately" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "Immediate" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "NextBeat" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "NextMeasure" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "NextGrid" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "NextSegment" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "Beat" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "Measure" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "Grid" ) ||
			!_stricmp( cpMsgParams.m_Args[nTimingParam-1], "Segment" ) )
		{
			bValid = LTTRUE;
		}
		else
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateMucisMsg()" );
				pInterface->CPrint( "    MSG - MUSIC - Timing value '%s' is invalid. (see PlayerMessages.doc)", cpMsgParams.m_Args[nTimingParam-1] );
			}
			
			return LTFALSE;
		}
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateObjectiveMsg
//
//  PURPOSE:	Make sure the objective message is valid.
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateObjectiveMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( cpMsgParams.m_nArgs < 2 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateObjectiveMsg()" );
			pInterface->CPrint( "    MSG - OBJECTIVE - No action parameter listed." );
		}
		
		return LTFALSE;
	}

	if( cpMsgParams.m_nArgs == 3 && (
		!_stricmp( cpMsgParams.m_Args[1], "ADD" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "REMOVE" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "COMPLETED" )))
	{
		return LTTRUE;
	}
	else if( !_stricmp( cpMsgParams.m_Args[1], "REMOVEALL" ) &&
			cpMsgParams.m_nArgs == 2 )
	{
		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateCrosshairMsg()" );
		pInterface->CPrint( "    MSG - OBJECTIVE - Action '%s' is not valid!", cpMsgParams.m_Args[1] );
	}
	
	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateKeyMsg
//
//  PURPOSE:	Make sure the objective message is valid.
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateKeyMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( cpMsgParams.m_nArgs < 2 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateKeyMsg()" );
			pInterface->CPrint( "    MSG - KEY - No action parameter listed." );
		}
		
		return LTFALSE;
	}

	if( cpMsgParams.m_nArgs == 3 && (
		!_stricmp( cpMsgParams.m_Args[1], "ADD" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "REMOVE" )))
	{
		return LTTRUE;
	}
	else if( !_stricmp( cpMsgParams.m_Args[1], "REMOVEALL" ) &&
			cpMsgParams.m_nArgs == 2 )
	{
		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateKeyMsg()" );
		pInterface->CPrint( "    MSG - KEY - Action '%s' is not valid!", cpMsgParams.m_Args[1] );
	}
	
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateCreditsMsg
//
//  PURPOSE:	Make sure Credits message is good
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateCreditsMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( !_stricmp( cpMsgParams.m_Args[1], "ON" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "OFF" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "INTRO" ))
	{
		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateCreditsMsg()" );
		pInterface->CPrint( "    MSG - CREDITS - Action '%s' is not valid!", cpMsgParams.m_Args[1] );
	}
	
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateIntelMsg
//
//  PURPOSE:	Make sure Intel message is good
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateIntelMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( cpMsgParams.m_nArgs < 3 || cpMsgParams.m_nArgs > 5 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateIntelMsg()" );
			pInterface->CPrint( "    MSG - INTEL - Invalid number of parameters." );
		}
		
		return LTFALSE;
	}

	if( cpMsgParams.m_nArgs > 3 )
	{
		if( !_stricmp( cpMsgParams.m_Args[3], "0" ) ||
			!_stricmp( cpMsgParams.m_Args[3], "1" ) ||
			!_stricmp( cpMsgParams.m_Args[4], "0" ) ||
			!_stricmp( cpMsgParams.m_Args[4], "1" ))
		{
			return LTTRUE;
		}

		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateIntelMsg()" );
			pInterface->CPrint( "    MSG - INTEL - Invalid [intel] or [show] parameters." );
		}
		
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateTransmissionMsg
//
//  PURPOSE:	Make sure Transmission message is good
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateTransmissionMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( cpMsgParams.m_nArgs < 2 || cpMsgParams.m_nArgs > 4 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateTransmissionMsg()" );
			pInterface->CPrint( "    MSG - TRANSMISSION - Invalid number of parameters." );
		}
		
		return LTFALSE;
	}

	if( cpMsgParams.m_nArgs == 3 )
	{
		if( !_stricmp( cpMsgParams.m_Args[2], "0" ) ||
			!_stricmp( cpMsgParams.m_Args[2], "1" ))
		{
			return LTTRUE;
		}

		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateTransmissionMsg()" );
			pInterface->CPrint( "    MSG - TRANSMISSION - Invalid [ActivePlayerComm] parameter." );
		}
		
		return LTFALSE;
	}


	if( cpMsgParams.m_nArgs == 4 )
	{
		if( !_stricmp( cpMsgParams.m_Args[3], "0" ) ||
			!_stricmp( cpMsgParams.m_Args[3], "1" ))
		{
			return LTTRUE;
		}

		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateTransmissionMsg()" );
			pInterface->CPrint( "    MSG - TRANSMISSION - Invalid [TeamNumber] parameter." );
		}
		
		return LTFALSE;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateAcquireWeapon
//
//  PURPOSE:	Make sure weapon message is good
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateAcquireWeapon( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pWeaponMgr )
		return LTTRUE;

	if( cpMsgParams.m_nArgs > 1 )
	{
		char buf[256];
		buf[0] = '\0';
		for( int32 i = 0; i < cpMsgParams.m_nArgs - 1; ++i )
		{
			if( i > 0 ) strcat(buf, " ");
			strcat( buf, cpMsgParams.m_Args[i+1] );
		}

		if( !g_pWeaponMgr->GetWeapon( buf ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateAcquireWeapon()" );
				pInterface->CPrint( "    MSG - ACQUIREWEAPON - Invalid weapon '%s'!", buf );
			}
			
			return LTFALSE;
		}

		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateAcquireWeapon()" );
		pInterface->CPrint( "    MSG - ACQUIREWEAPON - No weapon specified!" );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateAcquireAmmo
//
//  PURPOSE:	Make sure ammo message is good
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateAcquireAmmo( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pWeaponMgr )
		return LTTRUE;

	if( cpMsgParams.m_nArgs > 1 )
	{
		char buf[256];
		buf[0] = '\0';
		for( int32 i = 0; i < cpMsgParams.m_nArgs - 1; ++i )
		{
			if( i > 0 ) strcat(buf, " ");
			strcat( buf, cpMsgParams.m_Args[i+1] );
		}

		if( !g_pWeaponMgr->GetAmmo( buf ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateAcquireAmmo()" );
				pInterface->CPrint( "    MSG - ACQUIREAMMO - Invalid ammo '%s'!", buf );
			}
			
			return LTFALSE;
		}

		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateAcquireAmmo()" );
		pInterface->CPrint( "    MSG - ACQUIREAMMO - No ammo specified!" );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateAcquireMod
//
//  PURPOSE:	Make sure mod message is good
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateAcquireMod( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pWeaponMgr )
		return LTTRUE;

	if( cpMsgParams.m_nArgs > 1 )
	{
		char buf[256];
		buf[0] = '\0';
		for( int32 i = 0; i < cpMsgParams.m_nArgs - 1; ++i )
		{
			if( i > 0 ) strcat(buf, " ");
			strcat( buf, cpMsgParams.m_Args[i+1] );
		}

		if( !g_pWeaponMgr->GetMod( buf ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateAcquireMod()" );
				pInterface->CPrint( "    MSG - ACQUIREMOD - Invalid mod '%s'!", buf );
			}
			
			return LTFALSE;
		}

		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateAcquireMod()" );
		pInterface->CPrint( "    MSG - ACQUIREMOD - No mod specified!" );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateAcquireGear
//
//  PURPOSE:	Make sure gear message is good
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateAcquireGear( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pWeaponMgr )
		return LTTRUE;

	if( cpMsgParams.m_nArgs > 1 )
	{
		char buf[256];
		buf[0] = '\0';
		for( int32 i = 0; i < cpMsgParams.m_nArgs - 1; ++i )
		{
			if( i > 0 ) strcat(buf, " ");
			strcat( buf, cpMsgParams.m_Args[i+1] );
		}

		if( !g_pWeaponMgr->GetGear( buf ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateAcquireGear()" );
				pInterface->CPrint( "    MSG - ACQUIREGEAR - Invalid gear '%s'!", buf );
			}
			
			return LTFALSE;
		}

		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateAcquireGear()" );
		pInterface->CPrint( "    MSG - ACQUIREGEAR - No gear specified!" );
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateChangeWeapon
//
//  PURPOSE:	Make sure weapon message is good
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateChangeWeapon( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pWeaponMgr )
		return LTTRUE;

	if( cpMsgParams.m_nArgs > 1 )
	{
		char buf[256];
		buf[0] = '\0';
		for( int32 i = 0; i < cpMsgParams.m_nArgs - 1; ++i )
		{
			if( i > 0 ) strcat(buf, " ");
			strcat( buf, cpMsgParams.m_Args[i+1] );
		}

		if( !g_pWeaponMgr->GetWeapon( buf ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateChangeWeapon()" );
				pInterface->CPrint( "    MSG - CHANGEWEAPON - Invalid weapon '%s'!", buf );
			}
			
			return LTFALSE;
		}

		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateChangeWeapon()" );
		pInterface->CPrint( "    MSG - CHANGEWEAPON - No weapon specified!" );
	}

	return LTFALSE;
}

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( CPlayerObj )

//					Message		Num Params	Validation FnPtr		Syntax

	CMDMGR_ADD_MSG( MUSIC,			-1,		ValidateMusicMsg,		"MUSIC <action> <value1> <value2> [timing]" )
	CMDMGR_ADD_MSG( REWARD,			2,		NULL,					"REWARD <mission sensitive reward>" )
	CMDMGR_ADD_MSG( OBJECTIVE,		-1,		ValidateObjectiveMsg,	"OBJECTIVE <action> <objective id>" )
	CMDMGR_ADD_MSG( OPTION,			-1,		ValidateObjectiveMsg,	"OPTION <action> <objective id>" )
	CMDMGR_ADD_MSG( PARAMETER,		-1,		ValidateObjectiveMsg,	"PARAMETER <action> <objective id>" )
	CMDMGR_ADD_MSG( KEY,			-1,		ValidateKeyMsg,			"KEY <action> <key id>" )
	CMDMGR_ADD_MSG( TRANSMISSION,	-1,		ValidateTransmissionMsg,"TRANSMISSION <transmission id> [ActivePlayerComm] [TeamNumber]" )
	CMDMGR_ADD_MSG( CREDITS,		2,		ValidateCreditsMsg,		"CREDITS <action>" )
	CMDMGR_ADD_MSG( INTEL,			-1,		ValidateIntelMsg,		"INTEL <text id> <popup id> [intel] [show]" )
	CMDMGR_ADD_MSG( FADEIN,			2,		NULL,					"FADEIN <time>" )
	CMDMGR_ADD_MSG( FADEOUT,		2,		NULL,					"FADEOUT <time>" )
	CMDMGR_ADD_MSG( MISSIONTEXT,	2,		NULL,					"MISSIONTEXT <text id>" )
	CMDMGR_ADD_MSG( MISSIONFAILED,	2,		NULL,					"MISSIONFAILED <text id>" )
	CMDMGR_ADD_MSG( LOCKMUSIC,		1,		NULL,					"LOCKMUSIC" )
	CMDMGR_ADD_MSG( LOCKMOOD,		1,		NULL,					"LOCKMOOD" )
	CMDMGR_ADD_MSG( SETMOOD,		2,		NULL,					"SETMOOD <moodnum>" )
	CMDMGR_ADD_MSG( UNLOCKMUSIC,	1,		NULL,					"UNLOCKMUSIC" )
	CMDMGR_ADD_MSG( UNLOCKMOOD,		1,		NULL,					"UNLOCKMOOD" )
	CMDMGR_ADD_MSG( LOCKEVENT,		1,		NULL,					"LOCKEVENT" )
	CMDMGR_ADD_MSG( LOCKMOTIF,		1,		NULL,					"LOCKMOTIF" )
	CMDMGR_ADD_MSG( UNLOCKEVENT,	1,		NULL,					"UNLOCKEVENT" )
	CMDMGR_ADD_MSG( UNLOCKMOTIF,	1,		NULL,					"UNLOCKMOTIF" )
	CMDMGR_ADD_MSG( REMOVEBODIES,	1,		NULL,					"REMOVEBODIES" )
	CMDMGR_ADD_MSG( REMOVEALLBADAI, 1,		NULL,					"REMOVEALLBADAI" )
	CMDMGR_ADD_MSG( TRAITOR,		2,		NULL,					"TRAITOR <relation set" )
	CMDMGR_ADD_MSG( FACEOBJECT,		2,		NULL,					"FACEOBJECT <object>" )
	CMDMGR_ADD_MSG( RESETINVENTORY,	1,		NULL,					"RESETINVENTORY" )
	CMDMGR_ADD_MSG( ACQUIREWEAPON,	-1,		ValidateAcquireWeapon,	"ACQUIREWEAPON <weapon>" )	
	CMDMGR_ADD_MSG( ACQUIREAMMO,	-1,		ValidateAcquireAmmo,	"ACQUIREAMMO <ammo>" )
	CMDMGR_ADD_MSG( ACQUIREMOD,		-1,		ValidateAcquireMod,		"ACQUIREMOD <mod>" )
	CMDMGR_ADD_MSG( ACQUIREGEAR,	-1,		ValidateAcquireGear,	"ACQUIREGEAR <gear>" )
	CMDMGR_ADD_MSG( CHANGEWEAPON,	-1,		ValidateChangeWeapon,	"CHANGEWEAPON <weapon>" )
	CMDMGR_ADD_MSG( FULLHEALTH,		1,		NULL,					"FULLHEALTH" )
	CMDMGR_ADD_MSG( DISMOUNT,		1,		NULL,					"DISMOUNT" )
	CMDMGR_ADD_MSG( FOLLOWLURE,		2,		NULL,					"FOLLOWLURE <PlayerLure>" )	
	CMDMGR_ADD_MSG( CANCELLURE,		1,		NULL,					"CANCELLURE" )	
	CMDMGR_ADD_MSG( OVERLAY,		3,		NULL,					"OVERLAY <overlay id> <ON | OFF>" )
	CMDMGR_ADD_MSG( WAKEUP,			1,		NULL,					"WAKEUP" )
	CMDMGR_ADD_MSG( DROPBODY,		1,		NULL,					"DROPBODY" )
	CMDMGR_ADD_MSG( SCORE,			2,		NULL,					"SCORE <+/-amount>" )

CMDMGR_END_REGISTER_CLASS( CPlayerObj, CCharacter )

// Defines...

#define MAX_AIR_LEVEL						100.0f
#define FULL_AIR_LOSS_TIME					15.0f
#define FULL_AIR_REGEN_TIME					2.5f
#define DEFAULT_FRICTION					5.0f


// How far off the player can be between the server and client
#define DEFAULT_LEASHLEN					16.0f
// How far out to let it interpolate the position
#define DEFAULT_LEASHSPRING					50.0f
// How fast to interpolate between the postions (higher = faster)
#define	DEFAULT_LEASHSPRINGRATE				0.3f

#define CONSOLE_COMMAND_LEASH_LENGTH		"LeashLen"
#define CONSOLE_COMMAND_LEASH_SPRING		"LeashSpring"
#define CONSOLE_COMMAND_LEASH_SPRING_RATE	"LeashSpringRate"
#define CONSOLE_COMMAND_MOVE_VEL			"RunSpeed"
#define CONSOLE_COMMAND_JUMP_VEL			"JumpSpeed"
#define CONSOLE_COMMAND_SWIM_VEL			"SwimVel"
#define CONSOLE_COMMAND_LADDER_VEL			"LadderVel"


#define PLAYER_RESPAWN_SOUND				"Snd\\Powerups\\spawn_player.wav"

#define DEFAULT_PLAYERSOUND_RADIUS			512.0f

// Vehicle related console commands...

#define CONSOLE_COMMAND_VEHICLE				"Vehicle"

// BL 09/29/00 Added to fix falling off keyframed objects after loading game
int32 g_bPlayerUpdated = 10;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CPlayerObj
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CPlayerObj::CPlayerObj() 
	: CCharacter()
	, m_vClientCameraOffset( 0.0f, 0.0f, 0.0f )
	, m_nClientCameraOffsetTimeReceivedMS( 0 )
{
	m_bBlink				= LTFALSE;
	m_bShortRecoil			= LTTRUE;

	m_nMotionStatus			= MS_NONE;
	m_nWeaponStatus			= WS_NONE;
	m_ePlayerLean			= PL_CENTER;

	m_dwFlags			   |= FLAG_FORCECLIENTUPDATE | FLAG_YROTATION;
	m_dwFlags				&= ~FLAG_GRAVITY; // This is controlled by the client.

    m_pPlayerAttachments    = LTNULL;

	m_eModelId				= g_pModelButeMgr->GetModelId(DEFAULT_PLAYERNAME);
	m_eModelSkeleton		= g_pModelButeMgr->GetModelSkeleton(m_eModelId);

//	ResetObjectRelationMgr();

    m_bFirstUpdate          = true;
	m_bNewMission			= true;
	m_bForceUpdateInterface = false;
	m_ClientMoveCode		= 0;
	m_nCurContainers		= 0;
	m_fOldHitPts			= -1;
	m_fOldEnergy			= -1;
	m_fOldArmor				= -1;
	m_fOldAirLevel			= MAX_AIR_LEVEL;
	m_fAirLevel				= MAX_AIR_LEVEL;
	m_fOldModelAlpha		= 1.0f;
    m_hClient               = LTNULL;
	m_fLeashLen				= DEFAULT_LEASHLEN;
	m_eState				= PS_DEAD;
	m_bRespawnInvulnerability = false;
	m_bGriefProtected		 = false;
    m_bGodMode              = LTFALSE;
    m_bAllowInput           = LTTRUE;
	m_bCinematicInvulnerability = false;
    m_b3rdPersonView        = LTFALSE;

    m_bSpectatorMode        = LTFALSE;
	m_bInvisible			= false;

	m_PStateChangeFlags		= PSTATE_INITIAL;

	m_ePPhysicsModel		= PPM_NORMAL;
    m_hVehicleModel.SetReceiver( *this );

	m_vOldModelColor.Init();

	m_fWalkVel				= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_WALKSPEED);
	m_fRunVel				= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_RUNSPEED);
	m_fJumpVel				= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_JUMPSPEED);
	m_fSuperJumpVel			= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_SUPERJUMPSPEED);
	m_fLadderVel			= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_LADDERSPEED);
	m_fSwimVel				= g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_SWIMSPEED);

	m_dwLastLoadFlags		= LOAD_NEW_GAME;
    m_pClientSaveData       = LTNULL;

	m_nRespawnCount  = 0;

	m_bHasDoneHandshake = LTFALSE;

	m_eSoundPriority = SOUNDPRIORITY_PLAYER_HIGH;


    m_pnOldAmmo             = LTNULL;

    uint8 nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoIds();

	if (nNumAmmoTypes > 0)
	{
		m_pnOldAmmo = debug_newa(int, nNumAmmoTypes);
		memset(m_pnOldAmmo, 0, nNumAmmoTypes);
	}

    m_Cameras.Init(LTFALSE);
    m_bCameraListBuilt  = LTFALSE;

	m_Skills.Init();

	m_pAnimator				= &m_Animator;

	m_fSoundRadius			= DEFAULT_PLAYERSOUND_RADIUS;

	m_bChatting = LTFALSE;

	m_bReadyToExit = LTFALSE;

	m_bForceDuck = LTFALSE;
	m_bRespawnCalled = LTFALSE;

	m_vLastClientPos.Init();
	m_bUseLeash = LTFALSE;

	m_fHiddenTimer = 0.0f;
	m_fHideDuration = 3.0f;
	m_bHiding = LTFALSE;
	m_bHidden = LTFALSE;

	m_hCarriedObject.SetReceiver( *this );

	m_bCanDropBody = false;
	m_bCarryingAI = false;

	m_eLeanVisibleStimID = kStimID_Unset;

	m_hDeadBody = LTNULL;

	m_fLastVehicleFootstepTime = 0.0f;

	if(!s_vtBaseHealRate.IsInitted())
	{
		s_vtBaseHealRate.Init(g_pLTServer, "BaseHealRate", NULL, 0.2f);
	}
	if(!s_vtWalkHealRate.IsInitted())
	{
		s_vtWalkHealRate.Init(g_pLTServer, "WalkHealRate", NULL, 0.1f);
	}

	m_bVisibleToEnemyAI = false;

	// Add this instance to a list of all PlayerObj's.
	m_lstPlayerObjs.push_back( this );

	m_bSendStartLevelCommand = false;

	m_bClientLoaded = false;

	m_nControlFlags = 0;

	m_vLastGoodDropPos.Init( 0.0f, 0.0f, 0.0f );

	m_ActivationData.Init();
	m_rFullPlayerRot.Init();

	m_vSavedVelocity.Init();
	m_dwSavedObjectFlags = 0;

	m_nSaveVersion = 0;

	m_bUsedStartPoint = false;

	m_bCanBeRevived = false;

	m_nRequestedTeam = INVALID_TEAM;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::~CPlayerObj
//
//	PURPOSE:	deallocate object
//
// ----------------------------------------------------------------------- //

CPlayerObj::~CPlayerObj()
{

	if (m_pnOldAmmo)
	{
		debug_deletea(m_pnOldAmmo);
	}

	{ // BL 09/29/00 Added to fix falling off keyframed objects after loading game
		g_bPlayerUpdated = 10;
	}

	RemoveVehicleModel();

	DropCarriedObject( );

	RemoveBody();

	SetClientSaveData( NULL );

	// Erase this instance from the list of all PlayerObj's.
	PlayerObjList::iterator it = m_lstPlayerObjs.begin( );
	while( it != m_lstPlayerObjs.end( ))
	{
		if( *it == this )
		{
			m_lstPlayerObjs.erase( it );
			break;
		}

		it++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CPlayerObj::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_PRECREATE:
		{
			if( fData != PRECREATE_SAVEGAME )
			{
				// Turn on the cylinder physics
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				if (pStruct)
				{
					pStruct->m_Flags2 |= FLAG2_PLAYERCOLLIDE | FLAG2_SPECIALNONSOLID | FLAG2_SERVERDIMS;
				}

				uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);
				PostPropRead((ObjectCreateStruct*)pData);
				return dwRet;
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			// Hide ourselfs at first.  We will un-hide during respawn...
			SetClientLoaded( false );
		}
		break;

		case MID_LINKBROKEN:
		{
			HandleLinkBroken((HOBJECT)pData);
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		default : break;
	}

	return CCharacter::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleDamage
//
//	PURPOSE:	Handles getting damaged
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleDamage(const DamageStruct& damage)
{
	// Check for FriendlyFire...
	
	if( IsMultiplayerGame( ) && (1.0f > g_vtNetFriendlyFire.GetFloat( )) && IsPlayer( damage.hDamager ) && (damage.hDamager != m_hObject) )
	{
		if( IsCoopMultiplayerGameType( ) || (IsTeamGameType( ) && AreSameTeam( damage.hDamager, m_hObject )) ) 
		{
			return;
		}
	}

	// If hit by a tracking device just be completely oblivious to the actual damage and begin tracking... 

	if( damage.eType == DT_GADGET_TRACKER )
	{
		// Send message to begin tracking...
		SetTracking( true );
		return;
	}

	if ( !m_damage.IsCantDamageType(damage.eType) )
	{
		HandleShortRecoil();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleLinkBroken
//
//	PURPOSE:	Handle link broken message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleLinkBroken(HOBJECT hObj)
{
	if (!hObj) return;

	// See if this was one of our camera objects...

    LPBASECLASS* pCur = LTNULL;

	pCur = m_Cameras.GetItem(TLIT_FIRST);

	while (pCur && *pCur)
	{
		if ((*pCur)->m_hObject == hObj)
		{
            m_Cameras.Remove(g_pLTServer->HandleToObject(hObj));
			return;
		}

		pCur = m_Cameras.GetItem(TLIT_NEXT);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CPlayerObj::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch (messageID)
	{
		case MID_DAMAGE:
		{
			DamageStruct damage;
			damage.InitFromMessage(pMsg);

			HandleDamage(damage);
		}
		break;
	}

	return CCharacter::ObjectMessageFn(hSender, pMsg);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::OnTrigger()
//
//	PURPOSE:	Process a trigger message.
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Music("Music");
	static CParsedMsg::CToken s_cTok_Reward("Reward");
	static CParsedMsg::CToken s_cTok_Objective("Objective");
	static CParsedMsg::CToken s_cTok_Option("Option");
	static CParsedMsg::CToken s_cTok_Parameter("Parameter");
	static CParsedMsg::CToken s_cTok_Key("Key");
	static CParsedMsg::CToken s_cTok_Transmission("Transmission");
	static CParsedMsg::CToken s_cTok_Credits("Credits");
	static CParsedMsg::CToken s_cTok_Intel("Intel");
	static CParsedMsg::CToken s_cTok_FadeIn("FadeIn");
	static CParsedMsg::CToken s_cTok_FadeOut("FadeOut");
	static CParsedMsg::CToken s_cTok_MissionText("MissionText");
	static CParsedMsg::CToken s_cTok_MissionFailed("MissionFailed");
	static CParsedMsg::CToken s_cTok_LockMusic("LockMusic");
	static CParsedMsg::CToken s_cTok_LockMood("LockMood");
	static CParsedMsg::CToken s_cTok_SetMood("SetMood");
	static CParsedMsg::CToken s_cTok_UnlockMusic("UnlockMusic");
	static CParsedMsg::CToken s_cTok_UnlockMood("UnlockMood");
	static CParsedMsg::CToken s_cTok_LockEvent("LockEvent");
	static CParsedMsg::CToken s_cTok_LockMotif("LockMotif");
	static CParsedMsg::CToken s_cTok_UnlockEvent("UnlockEvent");
	static CParsedMsg::CToken s_cTok_UnlockMotif("UnlockMotif");
	static CParsedMsg::CToken s_cTok_RemoveBodies("RemoveBodies");
	static CParsedMsg::CToken s_cTok_RemoveAllBadAI("RemoveAllBadAi");
	static CParsedMsg::CToken s_cTok_Traitor("TRAITOR");
	static CParsedMsg::CToken s_cTok_FaceObject("FACEOBJECT");
	static CParsedMsg::CToken s_cTok_ResetInventory("RESETINVENTORY");
	static CParsedMsg::CToken s_cTok_AcquireWeapon("ACQUIREWEAPON");
	static CParsedMsg::CToken s_cTok_AcquireAmmo("ACQUIREAMMO");
	static CParsedMsg::CToken s_cTok_AcquireMod("ACQUIREMOD");
	static CParsedMsg::CToken s_cTok_AcquireGear("ACQUIREGEAR");
	static CParsedMsg::CToken s_cTok_ChangeWeapon("CHANGEWEAPON");
	static CParsedMsg::CToken s_cTok_FullHealth("FULLHEALTH");
	static CParsedMsg::CToken s_cTok_Dismount("DISMOUNT");
	static CParsedMsg::CToken s_cTok_FollowLure("FOLLOWLURE");
	static CParsedMsg::CToken s_cTok_CancelLure("CANCELLURE");
	static CParsedMsg::CToken s_cTok_Overlay("Overlay");
	static CParsedMsg::CToken s_cTok_WakeUp("WakeUp");
	static CParsedMsg::CToken s_cTok_DropBody( "DropBody" );
	static CParsedMsg::CToken s_cTok_Score( "Score" );


	// Default to not finding the trigger.
	bool bRet = false;

	if (cMsg.GetArg(0) == s_cTok_Music)
	{
		bRet = HandleMusicMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_Reward)
	{
		bRet = m_Skills.HandleRewardMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_Objective)
	{
		bRet = HandleObjectiveMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_Option)
	{
		bRet = HandleObjectiveMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_Parameter)
	{
		bRet = HandleObjectiveMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_Key)
	{
		bRet = HandleKeyMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_Transmission)
	{
		bRet = HandleTransmissionMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_Overlay)
	{
		bRet = HandleOverlayMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_Credits)
	{
		bRet = HandleCreditsMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_Intel)
	{
		bRet = HandleIntelMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_FadeIn)
	{
        bRet = HandleFadeScreenMessage(cMsg, LTTRUE);
	}
	else if (cMsg.GetArg(0) == s_cTok_FadeOut)
	{
        bRet = HandleFadeScreenMessage(cMsg, LTFALSE);
	}
	else if (cMsg.GetArg(0) == s_cTok_MissionText)
	{
		bRet = HandleMissionTextMessage(cMsg);
	}
	else if (cMsg.GetArg(0) == s_cTok_MissionFailed)
	{
		bRet = HandleMissionFailedMessage(cMsg);
	}
	else if ((cMsg.GetArg(0) == s_cTok_LockMusic) || (cMsg.GetArg(0) == s_cTok_LockMood))
	{
        g_pMusicMgr->LockMood();
	}
	else if( cMsg.GetArg(0) == s_cTok_SetMood && cMsg.GetArgCount( ) == 2 )
	{
        int nMood = atoi( cMsg.GetArg( 1 ));
		if( nMood < CMusicMgr::kNumMoods )
		{
			g_pMusicMgr->SetMood(( CMusicMgr::Mood )nMood );
		}
	}
	else if ((cMsg.GetArg(0) == s_cTok_UnlockMusic) || (cMsg.GetArg(0) == s_cTok_UnlockMood))
	{
		g_pMusicMgr->UnlockMood();
	}
	else if ((cMsg.GetArg(0) == s_cTok_LockEvent) || (cMsg.GetArg(0) == s_cTok_LockMotif))
	{
        g_pMusicMgr->LockEvent();
	}
	else if ((cMsg.GetArg(0) == s_cTok_UnlockEvent) || (cMsg.GetArg(0) == s_cTok_UnlockMotif))
	{
		g_pMusicMgr->UnlockEvent();
	}
	else if (cMsg.GetArg(0) == s_cTok_RemoveBodies)
	{
		HCLASS  hClass = g_pLTServer->GetClass("Body");
		HOBJECT hCurObject = LTNULL;
		while (hCurObject = g_pLTServer->GetNextObject(hCurObject))
		{
			if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hClass))
			{
				Body* pBody = (Body*)g_pLTServer->HandleToObject(hCurObject);
				g_pLTServer->RemoveObject( pBody->m_hObject );
            }
		}

		hCurObject = LTNULL;
		while (hCurObject = g_pLTServer->GetNextInactiveObject(hCurObject))
		{
			if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hCurObject), hClass))
			{
				Body* pBody = (Body*)g_pLTServer->HandleToObject(hCurObject);
				g_pLTServer->RemoveObject( pBody->m_hObject );
            }
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_RemoveAllBadAI)
	{
		bRet = HandleRemoveAllBadAIMessage( );
	}

	if (bRet)
		return true;

	if (CCharacter::OnTrigger(hSender, cMsg))
		return true;

	// See if we've turned to the dark side...

	if (cMsg.GetArg(0) == s_cTok_Traitor)
	{
		if (cMsg.GetArgCount() > 1)
		{
			// We've been a bad boy...
			ResetObjectRelationMgr(cMsg.GetArg(1));
		}
		else
		{
			UBER_ASSERT( 0, "Trigger Traitor now requires an argument -- supply a RelationSet/RelationData name" );
		}
		bRet = true;
	}
	else if (cMsg.GetArg(0) == s_cTok_FaceObject)
	{
		if (cMsg.GetArgCount() > 1)
		{
			const char* pObjName = cMsg.GetArg(1);
			if (pObjName)
			{
				ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
				int numObjects;

                g_pLTServer->FindNamedObjects(const_cast<char*>(pObjName), objArray);
				numObjects = objArray.NumObjects();

                if (!numObjects) return false;

                HOBJECT hObj = numObjects ? objArray.GetObject(0) : LTNULL;

				if (hObj)
				{
					// Look at the object...

                    LTVector vDir, vPos, vTargetPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPos);
					g_pLTServer->GetObjectPos(hObj, &vTargetPos);

					vTargetPos.y = vPos.y; // Don't look up/down.

					VEC_SUB(vDir, vTargetPos, vPos);
					VEC_NORM(vDir);

                    LTRotation rRot(vDir, LTVector(0.0f, 1.0f, 0.0f));
					g_pLTServer->SetObjectRotation(m_hObject, &rRot);
				}
			}
		}
		bRet = true;
	}
	else if (cMsg.GetArg(0) == s_cTok_ResetInventory)
	{
		ResetInventory(LTFALSE);
		bRet = true;
	}
	else if (cMsg.GetArg(0) == s_cTok_AcquireWeapon)
	{
		char buf[256];
		buf[0] = '\0';
		for (uint i=0; i < cMsg.GetArgCount()-1; i++)
		{
			if (i > 0) strcat(buf, " ");
			strcat(buf, cMsg.GetArg(i+1));
		}

		AcquireWeapon(buf);
		bRet = true;
	}
	else if (cMsg.GetArg(0) == s_cTok_AcquireAmmo)
	{
		char buf[256];
		buf[0] = '\0';
		for (uint i=0; i < cMsg.GetArgCount()-1; i++)
		{
			if (i > 0) strcat(buf, " ");
			strcat(buf, cMsg.GetArg(i+1));
		}

		AcquireAmmo(buf);
		bRet = true;
	}
	else if (cMsg.GetArg(0) == s_cTok_AcquireMod)
	{
		char buf[256];
		buf[0] = '\0';
		for (uint i=0; i < cMsg.GetArgCount()-1; i++)
		{
			if (i > 0) strcat(buf, " ");
			strcat(buf, cMsg.GetArg(i+1));
		}

		AcquireMod(buf);
		bRet = true;
	}
	else if (cMsg.GetArg(0) == s_cTok_AcquireGear)
	{
		char buf[64];
		buf[0] = '\0';
		for (uint i=0; i < cMsg.GetArgCount()-1; i++)
		{
			if (i > 0) strcat(buf, " ");
			strcat(buf, cMsg.GetArg(i+1));
		}

		AcquireGear(buf);
		bRet = true;
	}
	else if (cMsg.GetArg(0) == s_cTok_ChangeWeapon)
	{
		ChangeToWeapon(cMsg.GetArg(1));
		bRet = true;
	}
	else if (cMsg.GetArg(0) == s_cTok_FullHealth)
	{
		HealCheat();
		bRet = true;
	}
	else if (cMsg.GetArg(0) == s_cTok_Dismount)
	{
		if (m_ePPhysicsModel != PPM_NORMAL)
		{
			SetPhysicsModel(PPM_NORMAL);
			bRet = true;
		}
	}
	else if (cMsg.GetArg(0) == s_cTok_FollowLure)
	{
		bRet = ProcessCommandLure( cMsg );
	}
	else if (cMsg.GetArg(0) == s_cTok_CancelLure)
	{
		bRet = StopFollowingLure( );
	}
	else if( cMsg.GetArg(0) == s_cTok_WakeUp )
	{
		if( m_damage.IsTakingProgressiveDamage( DT_SLEEPING ))
		{
			m_damage.ClearProgressiveDamage( DT_SLEEPING );
		}
	}
	else if( cMsg.GetArg(0) == s_cTok_DropBody )
	{
		DropCarriedObject();
	}
	else if( cMsg.GetArg(0) == s_cTok_Score )
	{
		if( cMsg.GetArgCount( ) > 1 )
		{
			int nScore = atoi( cMsg.GetArg( 1 ));
			GetPlayerScore( )->AddBonus( nScore );
			bRet = true;
		}
	}

    return bRet;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CPlayerObj::ResetObjectRelationMgr()
//              
//	PURPOSE:	Sets the players ObjectRelationMgr to to the passed in string
//				if there is on, else to DEFAULT_PLAYERNAME
//
//----------------------------------------------------------------------------
void CPlayerObj::ResetObjectRelationMgr(const char* const pszNewRelation /*=NULL*/ )
{
	UBER_ASSERT( GetRelationMgr(), "ResetObjectRelationMgr: RelationMgr is NULL" );
	const char* pszAlignment = pszNewRelation!=NULL ? pszNewRelation : DEFAULT_PLAYERNAME;
	GetRelationMgr()->Init( m_hObject, pszAlignment ); 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FilterDamage()
//
//	PURPOSE:	Change the damage struct before damage is dealt to the player
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::FilterDamage( DamageStruct *pDamageStruct )
{
	//Stamina skill reduces duration of progressive damage in a single player game
	pDamageStruct->fDuration *= GetPlayerSkills()->GetSkillModifier(SKL_STAMINA,StaminaModifiers::eResistance);

	if ( 0.0f < pDamageStruct->fDuration )
	{
		// Check friendly fire.
		if( IsMultiplayerGame() && IsPlayer( pDamageStruct->hDamager) && pDamageStruct->hDamager != m_hObject && ( 1.0f > g_vtNetFriendlyFire.GetFloat() ))
		{
			if (IsCoopMultiplayerGameType( ))
			{
				return false;
			}
			if (IsTeamGameType( ) && AreSameTeam(pDamageStruct->hDamager, m_hObject) )
			{
				return false;
			}

		}
	}
	else
	{
		if ( IsPlayer( pDamageStruct->hDamager ) )
		{
			// Check friendly fire.
			if( IsMultiplayerGame() && ( 1.0f > g_vtNetFriendlyFire.GetFloat() ))
			{
				if (IsCoopMultiplayerGameType( ))
				{
					//pDamageStruct->fDamage = 0.0f;
					return false;
				}
				if (IsTeamGameType( ) && AreSameTeam(pDamageStruct->hDamager, m_hObject) && (pDamageStruct->hDamager != m_hObject))
				{
					//pDamageStruct->fDamage = 0.0f;
					return false;
				}

			}
			else
			{
				pDamageStruct->fDamage *= GetPlayerSkills()->GetSkillModifier(SKL_WEAPON,WeaponModifiers::eDamage);
			}
		}
	}
	
	if( m_ePPhysicsModel == PPM_SNOWMOBILE )
	{
		LTVector vDamageDir = pDamageStruct->vDir;
		vDamageDir.y = 0.0f;
		vDamageDir.Normalize();
		
		LTRotation rRot;
		g_pLTServer->GetObjectRotation( m_hObject, &rRot );
		
		LTVector vPlayerDir = rRot.Forward();
		vPlayerDir.y = 0.0f;
		vPlayerDir.Normalize();

		float fDot = vPlayerDir.Dot( vDamageDir );

		// Take less damage if coming from the front...
		
		if( fDot <= -0.75f )
		{
			pDamageStruct->fDamage *= 0.5f;
		}
	}

	if (IsGriefType(pDamageStruct->eType) && m_bGriefProtected)
	{
		return false;
	}

	return CCharacter::FilterDamage( pDamageStruct );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PreCreateSpecialFX()
//
//	PURPOSE:	Last chance to change our characterfx struct
//
// ----------------------------------------------------------------------- //
void CPlayerObj::PreCreateSpecialFX(CHARCREATESTRUCT& cs)
{
	CCharacter::PreCreateSpecialFX(cs);

    cs.bIsPlayer	= LTTRUE;
	cs.nTrackers	= m_Animator.GetNumTrackers();
	cs.nDimsTracker = m_Animator.GetDimsTracker();
	cs.nClientID	= (uint8) g_pLTServer->GetClientID(GetClient( ));

	cs.SetChatting(m_bChatting);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMusicMessage()
//
//	PURPOSE:	Process a music message
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::HandleMusicMessage(const CParsedMsg &cMsg)
{
	if (g_pMusicMgr->IsMoodLocked()) return true;

	// Send message to client containing the music trigger information
	char aTempBuffer[256];
	cMsg.ReCreateMsg(aTempBuffer, sizeof(aTempBuffer), 0);
	HSTRING hstrMusic = g_pLTServer->CreateString(aTempBuffer);
	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_MUSIC);
	cClientMsg.WriteHString(hstrMusic);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
	FREE_HSTRING(hstrMusic);

    return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleObjectiveMessage()
//
//	PURPOSE:	Process an objective message
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::HandleObjectiveMessage( const CParsedMsg &cMsg )
{
	ObjectiveMsgInfo omInfo;
	if( !g_pGameServerShell->ProcessObjectiveMessage( cMsg, &omInfo ))
	{
		return false;
	}

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8( MID_PLAYER_INFOCHANGE );
	cClientMsg.Writeuint8( omInfo.m_nType );
	cClientMsg.Writeuint8( omInfo.m_nRequest );
    cClientMsg.Writeuint8( 0 );
    cClientMsg.Writefloat( (LTFLOAT)omInfo.m_dwId );
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);

    return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleKeyMessage()
//
//	PURPOSE:	Process an objective message
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::HandleKeyMessage(const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Add("Add");
	static CParsedMsg::CToken s_cTok_Remove("Remove");
	static CParsedMsg::CToken s_cTok_RemoveAll("RemoveAll");
	
	// Update the client's keys...

	if (cMsg.GetArgCount() < 2) return false;

    uint8 nRequest = ITEM_ADD_ID;
	if (cMsg.GetArg(1) == s_cTok_Add)
	{
		nRequest = ITEM_ADD_ID;
	}
	else if (cMsg.GetArg(1) == s_cTok_Remove)
	{
		nRequest = ITEM_REMOVE_ID;
	}
	else if (cMsg.GetArg(1) == s_cTok_RemoveAll)
	{
		nRequest = ITEM_CLEAR_ID;
	}
	else
	{
        return false;
	}

    uint16 nId = 0;
	if (nRequest != ITEM_CLEAR_ID)
	{
        if (cMsg.GetArgCount() < 3) return false;
        nId = (uint16) atol(cMsg.GetArg(2));
	}

	// Update the local key list
	UpdateKeys(nRequest, nId);

	// Update the client's key list
	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cClientMsg.Writeuint8(IC_KEY_ID);
	cClientMsg.Writeuint8(nRequest);
    cClientMsg.Writeuint8(0);
    cClientMsg.Writefloat((LTFLOAT)nId);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);

    return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleTransmissionMessage()
//
//	PURPOSE:	Process an transmission message
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::HandleTransmissionMessage(const CParsedMsg &cMsg)
{
	if (cMsg.GetArgCount() < 2) return false;

    uint32 dwId = (uint32) atol(cMsg.GetArg(1));

	// See if this was a communication type of transmission.  This means we will put the 
	// active player's name in front of it.
	bool bActivePlayerComm = false;
	if( cMsg.GetArgCount( ) >= 3 )
	{
		bActivePlayerComm = !!atoi( cMsg.GetArg( 2 ));
	}
	uint8 nTeam = INVALID_TEAM;
	if( cMsg.GetArgCount( ) >= 4 )
	{
		nTeam= (uint8)atoi( cMsg.GetArg( 3 ));
	}

	uint32 nSound = 0;

	// If this is a communication transmission, get the activeplayer's client id and send it down.
	uint32 nActivePlayerClientId = -1;
	if( bActivePlayerComm )
	{
		HOBJECT hActivePlayer = g_pGameServerShell->GetActivePlayer( );
		CPlayerObj* pActivePlayer = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hActivePlayer ));
		if( pActivePlayer )
		{
			HCLIENT hClientActivePlayer = pActivePlayer->GetClient( );
			nActivePlayerClientId = g_pLTServer->GetClientID( hClientActivePlayer );
		}
	}

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_TRANSMISSION);
	cClientMsg.Writeuint32(nActivePlayerClientId);
	cClientMsg.Writeuint32(dwId);
    cClientMsg.Writeuint32(nSound);
	cClientMsg.Writeuint8(nTeam);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);

    return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleOverlayMessage()
//
//	PURPOSE:	Process an Overlay message
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::HandleOverlayMessage(const CParsedMsg &cMsg)
{
    if (IsMultiplayerGame( )) return false;

	if (cMsg.GetArgCount() < 3) return false;

    uint8  nId = (uint8) atol(cMsg.GetArg(1));
	uint8  nMsg = 255;
	static CParsedMsg::CToken s_cTok_Off("Off");
	static CParsedMsg::CToken s_cTok_On("On");

	if (cMsg.GetArg(2) == s_cTok_Off)
		nMsg = 0;
	else if (cMsg.GetArg(2) == s_cTok_On)
		nMsg = 1;
	

	if(nMsg < 255)
	{

		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_OVERLAY);
		cClientMsg.Writeuint8(nId);
		cClientMsg.Writeuint8(nMsg);
		g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
	}

    return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleCreditsMessage()
//
//	PURPOSE:	Process an Credits message
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::HandleCreditsMessage(const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Off("Off");
	static CParsedMsg::CToken s_cTok_On("On");
	static CParsedMsg::CToken s_cTok_Intro("Intro");

	if (cMsg.GetArgCount() < 2) return false;

	uint8 nMsg = 255;

	if (cMsg.GetArg(1) == s_cTok_Off)
		nMsg = 0;
	else if (cMsg.GetArg(1) == s_cTok_On)
		nMsg = 1;
	else if (cMsg.GetArg(1) == s_cTok_Intro)
		nMsg = 2;


	if (nMsg < 255)
	{
		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_CREDITS);
		cClientMsg.Writeuint8((uint8)nMsg);
		g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
	}

    return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleIntelMessage()
//
//	PURPOSE:	Process an Intel message
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::HandleIntelMessage(const CParsedMsg &cMsg)
{
	if (cMsg.GetArgCount() < 3) return false;

    uint32 nTextId = (uint32) atol(cMsg.GetArg(1));
    uint8 nPopupId = (uint8) atol(cMsg.GetArg(2));


	LTBOOL bIsIntel = LTFALSE;
	LTBOOL bShow = LTFALSE;
	LTBOOL bAdd = LTTRUE;

	if (cMsg.GetArgCount() >= 4)
	{
		bIsIntel = (LTBOOL)atol(cMsg.GetArg(3));

		if (cMsg.GetArgCount() >= 5)
		{
			bShow = (LTBOOL)atol(cMsg.GetArg(4));

			if (cMsg.GetArgCount() >= 6)
			{
				bAdd = (LTBOOL)atol(cMsg.GetArg(5));
			}

		}
	}



	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_INTEL_PICKEDUP);
    cClientMsg.Writeuint32(nTextId);
    cClientMsg.Writeuint8(nPopupId);
    cClientMsg.Writeuint8(bIsIntel);
    cClientMsg.Writeuint8(bShow);
    cClientMsg.Writeuint8(bAdd);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);


	if (bAdd)
		m_Skills.GainIntelBonus();



    return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleFadeScreenMessage()
//
//	PURPOSE:	Process a fade screen message
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::HandleFadeScreenMessage(const CParsedMsg &cMsg, bool bFadeIn)
{
	if (cMsg.GetArgCount() < 2) return false;

    float fFadeTime = (float)atof(cMsg.GetArg(1));
    if (fFadeTime < 0.1f) return false;

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cClientMsg.Writeuint8(IC_FADE_SCREEN_ID);
    cClientMsg.Writeuint8(bFadeIn ? 1 : 0);
	cClientMsg.Writeuint8(0);
    cClientMsg.Writefloat(fFadeTime);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);

    return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMissionTextMessage()
//
//	PURPOSE:	Process a mission text message
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::HandleMissionTextMessage(const CParsedMsg &cMsg)
{
	// Tell the client to draw the mission text

	if (cMsg.GetArgCount() < 2) return false;

    uint32 dwTextId = (uint32) atol(cMsg.GetArg(1));

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cClientMsg.Writeuint8(IC_MISSION_TEXT_ID);
    cClientMsg.Writeuint8(0);
    cClientMsg.Writeuint8(0);
    cClientMsg.Writefloat((float)dwTextId);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);

    return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMissionFailedMessage()
//
//	PURPOSE:	Process a mission failed message
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::HandleMissionFailedMessage(const CParsedMsg &cMsg)
{
	// Tell the clients to draw the mission failed screen

	if (cMsg.GetArgCount() < 2) return false;

    uint32 dwTextId = (uint32) atol(cMsg.GetArg(1));

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cClientMsg.Writeuint8(IC_MISSION_FAILED_ID);
    cClientMsg.Writeuint8(0);
    cClientMsg.Writeuint8(0);
    cClientMsg.Writefloat((LTFLOAT)dwTextId);
	g_pLTServer->SendToClient(cClientMsg.Read(), NULL, MESSAGE_GUARANTEED);


	//Tell the MissionMgr we failed (this pauses the game)
	g_pServerMissionMgr->SetMissionFailed(true);

    return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::HandleRemoveAllBadAIMessage
//
//  PURPOSE:	Remove all the bad ai in the game...
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::HandleRemoveAllBadAIMessage( )
{
	
	HOBJECT hObj		= g_pLTServer->GetNextObject(LTNULL);
    HOBJECT hRemoveObj	= LTNULL;

	// Remove all the active bad ai objects...
    
	while( hObj )
	{
        if( IsAI( hObj ))
		{
			hRemoveObj = hObj;
		}

        hObj = g_pLTServer->GetNextObject( hObj );

		if( hRemoveObj )
		{
            CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hRemoveObj );
			if( pChar && GetAlignment( this->GetRelationSet(), pChar->GetRelationData() ) == HATE )
			{
				SendTriggerMsgToObject( this, pChar->m_hObject, LTFALSE, "REMOVE" );
			}

            hRemoveObj = LTNULL;
		}
	}

	// Remove all the inactive bad ai objects...

    hObj		= g_pLTServer->GetNextInactiveObject(LTNULL);
    hRemoveObj	= LTNULL;

	while( hObj )
	{
        if( IsAI( hObj ))
		{
			hRemoveObj = hObj;
		}

        hObj = g_pLTServer->GetNextInactiveObject( hObj );

		if( hRemoveObj )
		{
            CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject( hRemoveObj );
			if( pChar && GetAlignment( this->GetRelationSet(), pChar->GetRelationData() ) == HATE )
			{
				SendTriggerMsgToObject( this, pChar->m_hObject, LTFALSE, "REMOVE" );
			}

            hRemoveObj = LTNULL;
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PostPropRead
//
//	PURPOSE:	Handle post-property initialization
//
// ----------------------------------------------------------------------- //

void CPlayerObj::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (pStruct && g_pServerButeMgr && g_pModelButeMgr)
	{
		if( pStruct->m_UserData == eModelIdInvalid )
		{
			m_eModelId = g_pModelButeMgr->GetModelId( DEFAULT_PLAYERNAME );
		}
		else
		{
			m_eModelId = (ModelId)pStruct->m_UserData;
		}
		m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);

		const char* pFilename = g_pModelButeMgr->GetModelFilename(m_eModelId);
		SAFE_STRCPY(pStruct->m_Filename, pFilename);

		g_pModelButeMgr->CopySkinFilenames(m_eModelId, 0, pStruct->m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
		g_pModelButeMgr->CopyRenderStyleFilenames(m_eModelId, pStruct);


		SAFE_STRCPY(pStruct->m_Name, DEFAULT_PLAYERNAME);

	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::InitialUpdate(int nInfo)
{
	m_Skills.SetOwner(m_hObject);

    g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

	// Set up console vars used to tweak player movement...

    m_LeashLenTrack.Init(g_pLTServer, CONSOLE_COMMAND_LEASH_LENGTH, LTNULL, DEFAULT_LEASHLEN);
    m_LeashSpringTrack.Init(g_pLTServer, CONSOLE_COMMAND_LEASH_SPRING, LTNULL, DEFAULT_LEASHSPRING);
    m_LeashSpringRateTrack.Init(g_pLTServer, CONSOLE_COMMAND_LEASH_SPRING_RATE, LTNULL, DEFAULT_LEASHSPRINGRATE);
    m_MoveVelMulTrack.Init(g_pLTServer, CONSOLE_COMMAND_MOVE_VEL, LTNULL, 1.0f);
    m_JumpVelMulTrack.Init(g_pLTServer, CONSOLE_COMMAND_JUMP_VEL, LTNULL, 0.81f);
    m_SwimVelTrack.Init(g_pLTServer, CONSOLE_COMMAND_SWIM_VEL, LTNULL, m_fSwimVel);
    m_LadderVelTrack.Init(g_pLTServer, CONSOLE_COMMAND_LADDER_VEL, LTNULL, m_fLadderVel);

    if (nInfo == INITIALUPDATE_SAVEGAME) return LTTRUE;


    SetNextUpdate(UPDATE_NEXT_FRAME);
    g_pLTServer->SetModelLooping(m_hObject, LTTRUE);

	m_damage.SetMass(g_pModelButeMgr->GetModelMass(m_eModelId));

	// Player bodies should not get removed during density capacity limiting.
	// (it's OK to remove them in Deathmatch)
	m_bPermanentBody = IsRevivePlayerGameType( );

	ResetHealth();

	ResetObjectRelationMgr();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::Update()
{
    if (!g_pGameServerShell) return LTFALSE;

	{ // BL 09/29/00 Added to fix falling off keyframed objects after loading game
		g_bPlayerUpdated--;
	}

	// We need to do this before checking our HCLIENT data member since
	// there are cases (load/save) where HCLIENT gets set to null, but
	// the player object is still valid (and needs to get update called
	// until the data member is set back to a valid value)...

    SetNextUpdate(UPDATE_NEXT_FRAME);

    if( !GetClient( ))
	{
		// If we don't have a client, make sure we don't have a body.  This can
		// happen in mp when a client doesn't rejoin a loaded save game.  If an exit
		// trigger or transition occurred, the player will have a client
		// before it gets here.
		
		// [RP] 9/14/02 - Force to be able to drop the body at the player pos since the
		//		last good pos may be from a previous level...

		m_bCanDropBody = true;
		DropCarriedObject( );

		return LTFALSE;
	}
	
	// [RP] 9/14/02 Don't officially update if the player hasn't respawned or his client hasn't loaded yet...

	if( !m_bRespawnCalled || !m_bClientLoaded ) 
		return LTFALSE;

	if( m_bFirstUpdate )
	{
		if (IsMultiplayerGame( ))
		{
			SendIDToClients();
		}

		g_pLTServer->GetObjectPos(m_hObject, &m_vLastPos);
		m_Skills.UpdateClient();

		// Check to see if we just reloaded a saved game...
		if (m_dwLastLoadFlags == LOAD_RESTORE_GAME || m_dwLastLoadFlags == LOAD_NEW_LEVEL || m_dwLastLoadFlags == LOAD_TRANSITION )
		{
			HandleGameRestore();
		}
		m_dwLastLoadFlags = 0;
	}


	if (m_bFirstUpdate)
	{
		UpdateClientPhysics(); // (to make sure they have their model around)
		TeleportClientToServerPos();
		SendStartLevelCommand();
	}
	else if (m_bSendStartLevelCommand)
	{
		SendStartLevelCommand();
	}

	// Ensure that there is always some minimum music mood playing,
	// regardless of any AI being alive.
	// Minimum mood is set per level in world properties.

	if( m_bFirstUpdate )
	{
		m_eMusicMoodMin = g_pGameServerShell->GetMinMusicMood();
	}
	else {
		g_pMusicMgr->DoMood( m_eMusicMoodMin );
	}

	// Keep the client updated....

	UpdateClientPhysics();


	// Update the movement flags...

	UpdateCommands();


	// Update our movement...

	UpdateMovement();


	// Update air level...

	UpdateAirLevel();

	// Update our Health...(Only in easy mode)

	if (g_pGameServerShell->GetDifficulty() == GD_EASY)
	{
		UpdateHealth();
	}

	// Update Interface...

	UpdateInterface( false );


	// Update any client-side special fx...

	UpdateSpecialFX();


	// Let the client know our position...

	UpdateClientViewPos();


	// Update our console vars (have they changed?)...

	UpdateConsoleVars();


	// If we're outside the world (and not in spectator mode)...wake-up,
	// time to die...

	if (!m_bSpectatorMode && m_eState == PS_ALIVE)
	{
        LTVector vPos, vMin, vMax;
        g_pLTServer->GetWorldBox(vMin, vMax);
        g_pLTServer->GetObjectPos(m_hObject, &vPos);

		if (vPos.x < vMin.x || vPos.y < vMin.y || vPos.z < vMin.z ||
			vPos.x > vMax.x || vPos.y > vMax.y || vPos.z > vMax.z)
		{
			DamageStruct damage;

			damage.eType	= DT_EXPLODE;
			damage.fDamage	= damage.kInfiniteDamage;
			damage.hDamager = m_hObject;
			damage.vDir.Init(0, 1, 0);

			damage.DoDamage(this, m_hObject);
		}
	}


	if ( (m_eState == PS_ALIVE) && m_bForceDuck )
	{
		m_Animator.UpdateForceDucking();
	}

	// See if we're invulnerable due to a respawn.
	if( m_bRespawnInvulnerability && m_tmrRespawnInvulnerabilityTime.Stopped( ))
	{
		m_bRespawnInvulnerability = false;
		m_bGodMode = false;
		m_damage.SetCanDamage( true );
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, 0, USRFLG_PLAYER_ALPHACYCLE );
	}


	// Make sure the next update isn't considered the first.
	m_bFirstUpdate = false;
	
	// Make sure we aren't considered visibe to AI next frame.  If we are this will get set by the sense recorder.
	m_bVisibleToEnemyAI	= false;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateCommands
//
//	PURPOSE:	Set the properties on our animator
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateCommands()
{
	if (!GetClient( ) ) return;

	if ( m_bAllowInput && !m_bChatting )
	{
		// Posture
		if (IsCarryingHeavyObject())
		{
			m_Animator.SetPosture(CAnimatorPlayer::eCarry);
		}
		else if ( (WS_RELOADING == m_nWeaponStatus) || m_Animator.IsAnimatingPosture(CAnimatorPlayer::eReload) )
		{
			if ( m_Animator.IsAnimatingPostureDone(CAnimatorPlayer::eReload) )
			{
				m_nWeaponStatus = WS_NONE;
			}
			else
			{
				m_Animator.SetPosture(CAnimatorPlayer::eReload);
			}
		}
		else if ( (WS_SELECT == m_nWeaponStatus) || m_Animator.IsAnimatingPosture(CAnimatorPlayer::eSelect) )
		{
			if ( m_Animator.IsAnimatingPostureDone(CAnimatorPlayer::eSelect) )
			{
				m_nWeaponStatus = WS_NONE;
			}
			else
			{
				m_Animator.SetPosture(CAnimatorPlayer::eSelect);
			}
		}
		else if ( (WS_DESELECT == m_nWeaponStatus) || m_Animator.IsAnimatingPosture(CAnimatorPlayer::eDeselect) )
		{
			if ( m_Animator.IsAnimatingPostureDone(CAnimatorPlayer::eDeselect) )
			{
				m_nWeaponStatus = WS_NONE;
			}
			else
			{
				m_Animator.SetPosture(CAnimatorPlayer::eDeselect);
			}
		}

		if (!IsCarryingHeavyObject() && WS_NONE == m_nWeaponStatus )
		{
			if ( ((GetControlFlags() & BC_CFLG_FIRING) != 0) || 
				m_Animator.IsAnimatingPosture( CAnimatorPlayer::eFire ) ||
				m_Animator.IsAnimatingPosture( CAnimatorPlayer::eFire2 ) ||
				m_Animator.IsAnimatingPosture( CAnimatorPlayer::eFire3 ))
			{
				ClearHiding();
				CAnimatorPlayer::Posture ePosture = m_Animator.GetLastPosture();
				if( m_Animator.IsAnimatingPostureDone( ePosture ) || ePosture == CAnimatorPlayer::eAim )
				{
					ePosture = static_cast<CAnimatorPlayer::Posture>(GetRandom( CAnimatorPlayer::eFire, CAnimatorPlayer::eFire3 ));
				}
				
				m_Animator.SetPosture( ePosture );
			}
			else
			{
				m_Animator.SetPosture(CAnimatorPlayer::eAim);
			}
		}

		// Weapon

		CAnimatorPlayer::Weapon eWeapon = CAnimatorPlayer::ePistol;

		CWeapon* pWeapon = m_pAttachments ? ((CPlayerAttachments*)m_pAttachments)->GetWeapon() : LTNULL;
		if ( pWeapon )
		{
			WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
			if (pWeaponData)
			{
				eWeapon = (CAnimatorPlayer::Weapon)pWeaponData->nAniType;
			}
		}

		m_Animator.SetWeapon(eWeapon);

		// Movement and Direction

		if ( (MS_JUMPED == m_nMotionStatus) && m_Animator.IsAnimatingDirectionDone(CAnimatorPlayer::eJump) )
		{
			// We finished jumping

			m_nMotionStatus = MS_FALLING;
		}

		if ( (MS_LANDED == m_nMotionStatus) && m_Animator.IsAnimatingDirectionDone(CAnimatorPlayer::eLand) )
		{
			// We finished landing

			m_nMotionStatus = MS_NONE;
		}

		if ( m_ePPhysicsModel == PPM_SNOWMOBILE )
		{
			m_Animator.SetMain(CAnimatorPlayer::eSnowmobile);
		}
		else if ( m_nMotionStatus != MS_NONE && ((GetControlFlags() & BC_CFLG_DUCK) == 0) )
		{
			// Only do jumping stuff if we're not ducking

			if ( m_nMotionStatus == MS_JUMPED )
			{
				m_Animator.SetMovement(CAnimatorPlayer::eJumping);
				m_Animator.SetDirection(CAnimatorPlayer::eJump);
			}
			else if ( m_nMotionStatus == MS_FALLING )
			{
				m_Animator.SetMovement(CAnimatorPlayer::eJumping);
				m_Animator.SetDirection(CAnimatorPlayer::eTuck);
			}
			else if ( m_nMotionStatus == MS_LANDED )
			{
				m_Animator.SetMovement(CAnimatorPlayer::eJumping);
				m_Animator.SetDirection(CAnimatorPlayer::eLand);
			}
		}
		else
		{
			m_nMotionStatus = MS_NONE;

			// Movement

			if (!IsCarryingHeavyObject() && ((GetControlFlags() & BC_CFLG_RUN) != 0))
			{
				m_Animator.SetMovement(CAnimatorPlayer::eRunning);
			}
			else
			{
				m_Animator.SetMovement(CAnimatorPlayer::eWalking);
			}

			// Can only duck in certain situations...

			if (!IsCarryingHeavyObject() && !m_bBodyOnLadder && !m_bSpectatorMode && !IsLiquid(m_eContainerCode) )
			{
				if (((GetControlFlags() & BC_CFLG_DUCK) != 0) || m_bForceDuck)
				{
					m_Animator.SetMovement(CAnimatorPlayer::eCrouching);
				}
			}


			// See if we should be swimming...

			LTBOOL bSwimming = IsLiquid(m_eContainerCode);
			if (!bSwimming)
			{
				bSwimming = (m_bBodyInLiquid ? !m_bOnGround : LTFALSE);
			}

			if (bSwimming)
			{
				m_Animator.SetMovement(CAnimatorPlayer::eSwimming);
			}

			// Direction

			if ((GetControlFlags() & BC_CFLG_FORWARD) != 0)
			{
				m_Animator.SetDirection(CAnimatorPlayer::eForward);
			}

			if ((GetControlFlags() & BC_CFLG_REVERSE) != 0)
			{
				m_Animator.SetDirection(CAnimatorPlayer::eBackward);
			}

			if ((GetControlFlags() & BC_CFLG_STRAFE) != 0)
			{
				if ((GetControlFlags() & BC_CFLG_LEFT) != 0)
				{
					m_Animator.SetDirection(CAnimatorPlayer::eStrafeLeft);
				}

				if ((GetControlFlags() & BC_CFLG_RIGHT) != 0)
				{
					m_Animator.SetDirection(CAnimatorPlayer::eStrafeRight);
				}
			}

			if ((GetControlFlags() & BC_CFLG_STRAFE_RIGHT) != 0)
			{
				m_Animator.SetDirection(CAnimatorPlayer::eStrafeRight);
			}

			if ((GetControlFlags() & BC_CFLG_STRAFE_LEFT) != 0)
			{
				m_Animator.SetDirection(CAnimatorPlayer::eStrafeLeft);
			}

			// Lean
			
			bool bDirection = !(m_Animator.GetDirection() == CAnimatorPlayer::eNone);
			if( ((GetControlFlags() & BC_CFLG_LEAN_LEFT) != 0) && m_ePlayerLean == PL_LEFT && !bDirection )
			{
				m_Animator.SetLean(CAnimatorPlayer::eLeft);
			}

			if( ((GetControlFlags() & BC_CFLG_LEAN_RIGHT) != 0) && m_ePlayerLean == PL_RIGHT && !bDirection )
			{
				m_Animator.SetLean(CAnimatorPlayer::eRight);
			}
        
		}

		if ( m_bBodyOnLadder && (m_Animator.GetLastMain() != CAnimatorPlayer::eDT_Sleeping) && 
								(m_Animator.GetLastMain() != CAnimatorPlayer::eDT_Slippery))
		{
			if (((GetControlFlags() & BC_CFLG_FORWARD) != 0) || ((GetControlFlags() & BC_CFLG_JUMP) != 0))
			{
				m_Animator.SetMain(CAnimatorPlayer::eClimbingUp);
			}
			else if (((GetControlFlags() & BC_CFLG_REVERSE) != 0) || ((GetControlFlags() & BC_CFLG_DUCK) != 0))
			{
				m_Animator.SetMain(CAnimatorPlayer::eClimbingDown);
			}
			else
			{
				m_Animator.SetMain(CAnimatorPlayer::eClimbing);
			}
		}

	}
	
	if( m_nDamageFlags != 0 )
	{
		if( UpdateDamageAnimations() )
			return;
	}

	if (m_bGriefProtected)
	{
		if ( m_Animator.IsAnimatingMain(CAnimatorPlayer::eDT_Slippery) && IsGriefType(DT_SLIPPERY) )
			return;
		if ( m_Animator.IsAnimatingMain(CAnimatorPlayer::eDT_Sleeping) && IsGriefType(DT_SLEEPING) )
			return;
		if ( m_Animator.IsAnimatingMain(CAnimatorPlayer::eDT_BearTrap) && IsGriefType(DT_BEAR_TRAP) )
			return;
		if ( m_Animator.IsAnimatingMain(CAnimatorPlayer::eDT_Glue) && IsGriefType(DT_GLUE) )
			return;
		if ( m_Animator.IsAnimatingMain(CAnimatorPlayer::eDT_Stun) && IsGriefType(DT_STUN) )
			return;
		if ( m_Animator.IsAnimatingMain(CAnimatorPlayer::eDT_Laughing) && IsGriefType(DT_LAUGHING) )
			return;

		if (m_tmrGriefTime.On())
		{
			if (m_tmrGriefTime.Stopped())
			{
				m_bGriefProtected = false;
				m_tmrGriefTime.Stop();
			}
		}
		else
		{
			m_tmrGriefTime.Start( GetConsoleFloat("GriefTime",2.0f) );
		}

	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateDamageAnimations
//
//	PURPOSE:	Update our animatior based on any damage we are taking
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::UpdateDamageAnimations()
{
	// Don't do special damage anis if on a ladder or a vehicle...

	if( (m_ePPhysicsModel != PPM_NORMAL) )
		return true;

	// The DT functions loop through all damage types every time
	// you do anything.  Get the one's we care about now, so it
	// doesn't loop every frame.
	static DTINFO const* pDTSlippery = GetDTINFO( DT_SLIPPERY );
	static DTINFO const* pDTSleeping = GetDTINFO( DT_SLEEPING );
	static DTINFO const* pDTBearTrap = GetDTINFO( DT_BEAR_TRAP );
	static DTINFO const* pDTGlue = GetDTINFO( DT_GLUE );
	static DTINFO const* pDTStun = GetDTINFO( DT_STUN );
	static DTINFO const* pDTLaughing = GetDTINFO( DT_LAUGHING );
	static DTINFO const* pDTChoke = GetDTINFO( DT_CHOKE );
	static DTINFO const* pDTBurn = GetDTINFO( DT_BURN );
	static DTINFO const* pDTPoison = GetDTINFO( DT_POISON );
	static DTINFO const* pDTAsss = GetDTINFO( DT_ASSS );
	static DTINFO const* pDTElectrocute = GetDTINFO( DT_ELECTROCUTE );


	if( m_nDamageFlags & pDTLaughing->nDamageFlag )
	{
		// Uncontrollable laughter causes us to drop a carried object.
		DropCarriedObject( );
	}

	// Set the fullbody animation acording to the damage being done...
	
	// Set the damage types that have no movement first...
	
	if( (m_nDamageFlags & pDTSlippery->nDamageFlag) )
	{
		// We slipped, which causes us to drop what we're carrying.
		DropCarriedObject( );

		if( m_bBodyOnLadder )
			return true;

		// Fix prediction issues, since we can't move while this is happening...
		LTVector vZero(0.0f, 0.0f, 0.0f);
		g_pPhysicsLT->SetVelocity(m_hObject, &vZero);
		m_Animator.SetMain( CAnimatorPlayer::eDT_Slippery );
		if (pDTSlippery->bGrief)
		{
			m_bGriefProtected = true;
		}
		return true;
	}
	
	if( (m_nDamageFlags & pDTSleeping->nDamageFlag) && !m_bBodyOnLadder )
	{
		// We fell asleep, which causes us to drop what we're carrying.
		DropCarriedObject( );

		if( m_bBodyOnLadder )
			return true;

		// Fix prediction issues, since we can't move while this is happening...
		LTVector vZero(0.0f, 0.0f, 0.0f);
		g_pPhysicsLT->SetVelocity(m_hObject, &vZero);
		m_Animator.SetMain( CAnimatorPlayer::eDT_Sleeping );
		if (pDTSleeping->bGrief)
		{
			m_bGriefProtected = true;
		}
		return true;
	}

	if( m_nDamageFlags & pDTBearTrap->nDamageFlag )
	{
		m_Animator.SetMain( CAnimatorPlayer::eDT_BearTrap );
		if (pDTBearTrap->bGrief)
		{
			m_bGriefProtected = true;
		}
		return true;
	}
	
	if( m_nDamageFlags & pDTGlue->nDamageFlag )
	{
		m_Animator.SetMain( CAnimatorPlayer::eDT_Glue );
		if (pDTSlippery->bGrief)
		{
			m_bGriefProtected = true;
		}
		return true;
	}
	
	// Only play the hurt anis if we are aimming...

	if( m_Animator.GetPosture() == CAnimatorPlayer::eAim && !m_bBodyOnLadder )
	{
		// If we are moving at all play the generic walk hurt animation...

		if( m_Animator.GetDirection() != CAnimatorPlayer::eNone )
		{
			m_Animator.SetMain( CAnimatorPlayer::eDT_HurtWalk );
			return true;
		}

		// We are not moving so play the animation based on damage type...

		if( m_nDamageFlags & pDTStun->nDamageFlag )
		{
			m_Animator.SetMain( CAnimatorPlayer::eDT_Stun );
			if (pDTStun->bGrief)
			{
				m_bGriefProtected = true;
			}
			return true;
		}
	
		if( m_nDamageFlags & pDTLaughing->nDamageFlag )
		{
			m_Animator.SetMain( CAnimatorPlayer::eDT_Laughing );
			if (pDTLaughing->bGrief)
			{
				m_bGriefProtected = true;
			}
			return true;
		}
	
		// The rest just use a generic hurt animation while at rest...
		static DamageFlags nHurtDamageFlags = GetDamageFlagsWithHurtAnim( );
		if( m_nDamageFlags & nHurtDamageFlags )
		{
			m_Animator.SetMain( CAnimatorPlayer::eDT_Hurt );
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetDeathAni
//
//	PURPOSE:	Override for death animation handling
//
// ----------------------------------------------------------------------- //

HMODELANIM CPlayerObj::GetDeathAni(LTBOOL bFront)
{
	HMODELANIM hDeathAnim = INVALID_MODEL_ANIM;

	// Handle crouching death
	if (m_Animator.GetLastMovement() == CAnimatorPlayer::eCrouching)
	{
		hDeathAnim = GetCrouchDeathAni(bFront);
	}

	if (hDeathAnim == INVALID_MODEL_ANIM)
	{
		return CCharacter::GetDeathAni(bFront);
	}
	else
	{
		LTVector vDims;
		g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, hDeathAnim);
		ASSERT(vDims.y < 32.0f);
		return hDeathAnim;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetCrouchDeathAni
//
//	PURPOSE:	Get a crouching death animation
//
// ----------------------------------------------------------------------- //

HMODELANIM CPlayerObj::GetCrouchDeathAni(LTBOOL bFront)
{
	static const char* aszDeathCrouches[] = { "DCr1", "DCr2", "DCr3" };
    static const int cDeathCrouches = sizeof(aszDeathCrouches)/sizeof(const char*);
	return g_pLTServer->GetAnimIndex(m_hObject, aszDeathCrouches[GetRandom(0, cDeathCrouches-1)]);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateMovement
//
//	PURPOSE:	Update player movement
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateMovement()
{
	if (!GetClient( )) return;
	
	// If we're in multiplayer, handle leashing
	if ( IsMultiplayerGame( ) && m_bUseLeash)
	{
		// Get the amount we've moved away from where the client says we are
		LTVector curPos;
		g_pLTServer->GetObjectPos(m_hObject, &curPos);
		float fMoveAmountSqr = curPos.DistSqr(m_vLastClientPos);
		// Remember how fast we're going
		LTVector vVelocity;
		g_pPhysicsLT->GetVelocity(m_hObject, &vVelocity);
		// If we're beyond our leash, move the server object where the client says it is
		if (fMoveAmountSqr > m_fLeashLen*m_fLeashLen)
		{
			// If we're beyond the interpolation point, force us to the new destination
			if (fMoveAmountSqr > m_fLeashSpring*m_fLeashSpring)
			{
				// Move it first so we get collisions and stuff
				g_pLTServer->MoveObject(m_hObject, &m_vLastClientPos);
				// Then just teleport it there in case it didn't make it for some reason
				g_pLTServer->SetObjectPos(m_hObject, &m_vLastClientPos);
				// Turn off the leash if we're not moving
				m_bUseLeash = vVelocity.Mag() > 0.001f;
			}
			// Otherwise interpolate our way there
			else
			{
				LTVector vLerpPos;
				VEC_LERP(vLerpPos, curPos, m_vLastClientPos, m_fLeashSpringRate);
				// Move where we think we want to go  (If we don't make it, the leash will snap later)
				g_pLTServer->MoveObject(m_hObject, &vLerpPos);
			}
		}
		// Turn off the leash, we're close enough
		else if (vVelocity.Mag() < 0.001f)
		{
			m_bUseLeash = LTFALSE;
		}
	}

	if( m_ePPhysicsModel != PPM_NORMAL )
	{
		UpdateVehicleMovement();
	}

    CCharacter::UpdateMovement(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateVehicleMovement
//
//	PURPOSE:	Update movement while on a vehicle
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateVehicleMovement()
{
	switch( m_ePPhysicsModel )
	{
		case PPM_SNOWMOBILE:
		{
			if( g_pLTServer->GetTime() - m_fLastVehicleFootstepTime > 0.1f )
			{
				// Register EnemyFootstepSound stimulus.
				LTVector vMovementPos;
				g_pLTServer->GetObjectPos(m_hObject, &vMovementPos);

				float fVolume = g_pServerButeMgr->GetPlayerAttributeFloat( PLAYER_BUTE_SNOWMOBILEVOLUME );

				g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyFootstepSound, m_hObject, vMovementPos, fVolume );
				m_fLastVehicleFootstepTime = g_pLTServer->GetTime();
			}
		}
		break;

		default: break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsObjectInArray
//
//	PURPOSE:	See if the object is in the list
//
// ----------------------------------------------------------------------- //

static LTBOOL IsObjectInArray(LTObjRef* theList, uint32 listSize, HOBJECT hTest)
{
    uint32 i;
	for(i=0; i < listSize; i++)
	{
		if(theList[i] == hTest)
            return LTTRUE;
	}
    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsObjectInArray
//
//	PURPOSE:	See if the object is in the list
//
// ----------------------------------------------------------------------- //

static LTBOOL IsObjectInArray(HOBJECT* theList, uint32 listSize, HOBJECT hTest)
{
    uint32 i;
	for(i=0; i < listSize; i++)
	{
		if(theList[i] == hTest)
            return LTTRUE;
	}
    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateClientPhysics
//
//	PURPOSE:	Determine what physics related messages to send to the client
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateClientPhysics()
{
	if (!GetClient( ) || !m_hObject) return;

	// Update hiding...
	UpdateHiding();


	//can we drop a body here?
	bool bCanDrop = !!HasCurrentVolume();
	if (bCanDrop != m_bCanDropBody)
	{
		m_bCanDropBody = bCanDrop;
		m_PStateChangeFlags |= PSTATE_INTERFACE;
	}

	if( m_hCarriedObject && CanDropObject( ))
	{
		// Get a valid position to use as a last resort drop pos...

		g_pLTServer->GetObjectPos( m_hObject, &m_vLastGoodDropPos );
	}

	// Did our container states change?

	HOBJECT objContainers[MAX_TRACKED_CONTAINERS];
    uint32 nContainers = 0;

	UpdateInContainerState(objContainers, nContainers);

	if (!m_PStateChangeFlags) return;


	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_CLIENT_PLAYER_UPDATE);

	cMsg.Writeuint16((uint16)m_PStateChangeFlags);

	// Note : Container updates are only sent on local clients, which is done to avoid extra container intersection testing
	if (m_PStateChangeFlags & PSTATE_CONTAINERTYPE)
	{
        LTFLOAT fFrictionPercent = 1.0f;

		// Send the new container info.

        cMsg.Writeuint8((uint8)nContainers);

        for (uint32 i=0; i < nContainers && i < MAX_TRACKED_CONTAINERS; i++)
		{
			// Send container code...

            uint16 nCode;
            if (!g_pLTServer->GetContainerCode(objContainers[i], &nCode))
			{
				nCode = CC_NO_CONTAINER;
			}

            cMsg.Writeuint8((uint8)nCode);

			// Send current and gravity...

			fFrictionPercent = 1.0f;
            LTFLOAT fViscosity = 0.0f, fGravity = 0.0f;

            LTVector vCurrent;
			vCurrent.Init();

            LTBOOL	bHidden = LTFALSE;
			HOBJECT	hVolumeObject = LTNULL;
			PlayerPhysicsModel ePPM = PPM_NORMAL;

            HCLASS hVolClass = g_pLTServer->GetClass("VolumeBrush");

			if (hVolClass)
			{
                if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(objContainers[i]), hVolClass))
				{
                    VolumeBrush* pVolBrush = (VolumeBrush*)g_pLTServer->HandleToObject(objContainers[i]);
					if (pVolBrush)
					{
						vCurrent		 = pVolBrush->GetCurrent();
						fGravity		 = pVolBrush->GetGravity();
						fViscosity		 = pVolBrush->GetViscosity();
						bHidden			 = pVolBrush->GetHidden();
						fFrictionPercent = pVolBrush->GetFriction();
						hVolumeObject	 = objContainers[i];
						ePPM			 = pVolBrush->GetPhysicsModel();
					}
				}
			}

            cMsg.WriteLTVector(vCurrent);
            cMsg.Writefloat(fGravity);
            cMsg.Writefloat(fViscosity);
            cMsg.Writeuint8((uint8)bHidden);
			cMsg.WriteObject(hVolumeObject);
			cMsg.Writeuint8(ePPM);
		}

		// Set our friction based on the container's values...

        LTFLOAT fFrictionCoeff = DEFAULT_FRICTION * fFrictionPercent;
        g_pPhysicsLT->SetFrictionCoefficient(m_hObject, fFrictionCoeff);
        cMsg.Writefloat(fFrictionCoeff);


		// Remember what we sent last...

		for (uint32 nIterContainer = 0; nIterContainer < nContainers; nIterContainer++)
		{
			m_CurContainers[nIterContainer] = objContainers[nIterContainer];
		}
		m_nCurContainers = nContainers;
	}

	if (m_PStateChangeFlags & PSTATE_MODELFILENAMES)
	{
		char fileName[256], skinName[256];

		g_pModelLT->GetModelDBFilename(m_hObject, fileName, ARRAY_LEN(skinName));

        cMsg.WriteString(fileName);
		
		// write the texture names.
		uint8 cSkins = g_pModelButeMgr->GetNumSkins(m_eModelId);
		cMsg.Writeuint8(cSkins);
		for (uint iSkin=0; iSkin < cSkins; iSkin++)
		{
			g_pModelLT->GetSkinFilename(m_hObject, iSkin, skinName, ARRAY_LEN(skinName));
			cMsg.WriteString(skinName);
		}
	}

	if (m_PStateChangeFlags & PSTATE_GRAVITY)
	{
        LTVector vGravity;
        g_pPhysicsLT->GetGlobalForce(vGravity);
        cMsg.WriteLTVector(vGravity);
	}

	if (m_PStateChangeFlags & PSTATE_SPEEDS)
	{
        cMsg.Writefloat(m_fWalkVel);
        cMsg.Writefloat(m_fRunVel);
        cMsg.Writefloat(m_fSwimVel);
        cMsg.Writefloat(m_fJumpVel);
		cMsg.Writefloat(m_fSuperJumpVel);
        
		// RunSpeed lets you run as fast as you want.. the client
		// treats this as a 'max speed' and acceleration multiplier...

        cMsg.Writefloat(m_fMoveMultiplier);
        cMsg.Writefloat(m_fBaseMoveAccel);
        cMsg.Writefloat(m_fJumpMultiplier);
        cMsg.Writefloat(m_fLadderVel);

        LTFLOAT fFrictionCoeff = 0.0f;
        g_pPhysicsLT->GetFrictionCoefficient(m_hObject, &fFrictionCoeff);
        cMsg.Writefloat(fFrictionCoeff);
	}

	if (m_PStateChangeFlags & PSTATE_PHYSICS_MODEL)
	{
		WriteVehicleMessage(cMsg);
	}

	if (m_PStateChangeFlags & PSTATE_INTERFACE)
	{
		WriteInterfaceMessage(cMsg);
	}

	g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
	m_PStateChangeFlags = 0;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateInContainerState
//
//	PURPOSE:	Determine if we're in any containers...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateInContainerState(HOBJECT* objContainers, uint32 & nContainers)
{
	// Make sure container updates only get sent for non-local clients
	// This is an optimization to avoid a second container intersection query on hosting servers
	uint32 nClientFlags = g_pLTServer->GetClientInfoFlags(GetClient());
	if ((nClientFlags & CIF_LOCAL) == 0)
		return;

    nContainers = g_pLTServer->GetObjectContainers(m_hObject, objContainers, MAX_TRACKED_CONTAINERS);
	nContainers = LTMIN(nContainers, MAX_TRACKED_CONTAINERS);

    if (nContainers != m_nCurContainers)
	{
		m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
	}
	else
	{
		// Did we enter a container?
        uint32 i;
        for (i=0; i < nContainers; i++)
		{
			if(!IsObjectInArray(m_CurContainers, m_nCurContainers, objContainers[i]))
			{
				m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
				break;
			}
		}

		// Did we exit a container?

		if (!(m_PStateChangeFlags & PSTATE_CONTAINERTYPE))
		{
			for(i=0; i < m_nCurContainers; i++)
			{
				if(!IsObjectInArray(objContainers, nContainers, m_CurContainers[i]))
				{
					m_PStateChangeFlags |= PSTATE_CONTAINERTYPE;
					break;
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::WriteVehicleMessage
//
//	PURPOSE:	Write the info about our current vehicle
//
// ----------------------------------------------------------------------- //

void CPlayerObj::WriteVehicleMessage(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	pMsg->Writeuint8(m_ePPhysicsModel);

	// Check if we need to send the lure object down.
	if( m_ePPhysicsModel == PPM_LURE )
	{
		ASSERT( m_hPlayerLure );
	    PlayerLure* pPlayerLure = dynamic_cast< PlayerLure* >( g_pLTServer->HandleToObject( m_hPlayerLure ));
		if( pPlayerLure )
		{
			pPlayerLure->WriteVehicleMessage( *pMsg );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::WriteInterfaceMessage
//
//	PURPOSE:	Write the info needed for the interface
//
// ----------------------------------------------------------------------- //

void CPlayerObj::WriteInterfaceMessage(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	uint8 nFlags = 0;
	
	if (m_bHiding)
	{
		nFlags |= PSTATE_INT_HIDING;
	}
	
	if (m_bHidden)
	{
		nFlags |= PSTATE_INT_HIDDEN;
	}
	
	if( m_bVisibleToEnemyAI )
	{
		nFlags |= PSTATE_INT_CANTHIDE;
	}

	if (m_hCarriedObject)
	{
		nFlags |= PSTATE_INT_CARRYING;
	}

	if( CanDropObject() )
	{
		nFlags |= PSTATE_INT_CAN_DROP;
	}

	if( m_bCanBeRevived )
	{
		nFlags |= PSTATE_INT_CANREVIVE;
	}

	pMsg->Writeuint8(nFlags);

	pMsg->Writefloat(m_fHideDuration);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetSpectatorMode
//
//	PURPOSE:	Turn on/off spectator mode
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetSpectatorMode(LTBOOL bOn)
{
	// Toggle spectator mode...

	m_bSpectatorMode = bOn;

	if (m_bSpectatorMode)
	{
		// Clear the flags...(make sure we still tell the client to update)...

		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, m_nSavedFlags);
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAGMASK_ALL);

		// Go invisible to AI.
		BecomeInvisible();
	}
	else
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_nSavedFlags, FLAGMASK_ALL);
		ResetPlayer();

		// Become visible again to AI.
		BecomeVisible();

		// Force us to the floor...
		MoveObjectToFloor( m_hObject );

		UpdateClientPhysics();
		TeleportClientToServerPos();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetInvisibleMode
//
//	PURPOSE:	Turn on/off invisible mode
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetInvisibleMode(LTBOOL bSetInvisible)
{
	// Don't mess with invisibility if in spectator mode...

	if (!m_bSpectatorMode)
	{
		if (bSetInvisible)
		{
			BecomeInvisible();
		}
		else
		{
			BecomeVisible();
		}
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CPlayerObj::BecomeInvisible()
//              
//	PURPOSE:	Removes all of the Visibility based stimuli that represent the
//				player from the StimulusMgr.
//              
//----------------------------------------------------------------------------
void CPlayerObj::BecomeInvisible(void)
{
	if (m_bInvisible) return;

	m_bInvisible = true;

	ASSERT(m_eEnemyVisibleStimID != kStimID_Unset);
	g_pAIStimulusMgr->RemoveStimulus(m_eEnemyVisibleStimID);
	m_eEnemyVisibleStimID = kStimID_Unset;

	// This stimulus only exists in TRON.

	if( m_eUndeterminedVisibleStimID != kStimID_Invalid )
	{
		ASSERT(m_eUndeterminedVisibleStimID != kStimID_Unset);
		g_pAIStimulusMgr->RemoveStimulus(m_eUndeterminedVisibleStimID);
	}
	m_eUndeterminedVisibleStimID = kStimID_Unset;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CPlayerObj::BecomeInvisible()
//              
//	PURPOSE:	Adds to the stimulus mgr all visibility based stimuli for the
//				player.
//              
//----------------------------------------------------------------------------
void CPlayerObj::BecomeVisible(void)
{
	if (!m_bInvisible) return;

	m_bInvisible = false;

	ASSERT(m_eEnemyVisibleStimID == kStimID_Unset);
	m_eEnemyVisibleStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyVisible, m_hObject, LTNULL, CAIStimulusRecord::kDynamicPos_TrackSource, 1.f );

	ASSERT(m_eUndeterminedVisibleStimID == kStimID_Unset);
	m_eUndeterminedVisibleStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_UndeterminedVisible, m_hObject, LTNULL, CAIStimulusRecord::kDynamicPos_TrackSource, 1.f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ToggleGodMode()
//
//	PURPOSE:	Turns god mode on and off
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ToggleGodMode()
{
	m_bGodMode = !m_bGodMode;
	m_damage.SetCanDamage(!m_bGodMode);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetRespawnInvulnerability
//
//	PURPOSE:	Become invulnerable during respawn for a period of time.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetRespawnInvulnerability()
{
	m_damage.SetCanDamage( false );
	float fRespawnInvulnerabilityTime = GetConsoleFloat( "RespawnInvulnerabilityTime", 3.0f );
	m_tmrRespawnInvulnerabilityTime.Start( fRespawnInvulnerabilityTime );
	m_bRespawnInvulnerability = true;
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, USRFLG_PLAYER_ALPHACYCLE, USRFLG_PLAYER_ALPHACYCLE );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ClientLoaded
//
//	PURPOSE:	Sets whether the client is loaded and ready for the level.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetClientLoaded( bool bLoaded )
{
	m_bClientLoaded = bLoaded;

	if( bLoaded )
	{
		// [RP] 9/14/02 - Reset the players velocity to the saved velocity...

		if( m_dwLastLoadFlags == LOAD_RESTORE_GAME ||
			m_dwLastLoadFlags == LOAD_TRANSITION )
		{
			g_pPhysicsLT->SetVelocity( m_hObject, &m_vSavedVelocity );

			if( m_dwSavedObjectFlags & FLAG_GRAVITY )
			{
				// Add gravity back in if it was set when the player was saved...

				g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_GRAVITY, FLAG_GRAVITY );
			}
		}


		if( m_eState == PS_ALIVE )
		{
			// Hide the character when the client isn't loaded, and unhide
			// it when they become loaded.
			HideCharacter( false );

			// Make us solid again.
			g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_SOLID, FLAG_SOLID );

			// If this is COOP, then make us invulnerable for a period of time, while cycling.
			// the model's alpha.
			if( g_pGameServerShell->GetGameType() == eGameTypeCooperative )
			{
				SetRespawnInvulnerability( );
			}
			// For sp, just make us vulnerable again without the timeout and fancy-dancy
			// model alpha thingy.
			else
			{
				// Need to be invulnerable until we're loaded.
				m_damage.SetCanDamage( true );
			}

			// Add stimuli back now that we are revived, so that AI can see us again.
			AITRACE( AIShowCharacters, ( m_hObject, "Player '%s' is respawning", GetNetUniqueName() ) );
			RegisterPersistentStimuli();

			// Restore all our flags so we make sure we didn't miss any.
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags, FLAGMASK_ALL);

			// Re-enable our hitbox.
			g_pCommonLT->SetObjectFlags( m_hHitBox, OFT_Flags, FLAG_RAYHIT | FLAG_TOUCHABLE, FLAG_RAYHIT | FLAG_TOUCHABLE );

			// Teleport to our same position so we put ourselves back into containers.
			LTVector vPos;
			g_pLTServer->GetObjectPos( m_hObject, &vPos );
			g_pLTServer->TeleportObject( m_hObject, &vPos );
		}
		else
		{
			// Unhide our dead body.  We can't just call hidecharacter, because this is used
			// to hide/unhide everything about the character.
			if( m_hDeadBody )
			{
				Body* pBody = dynamic_cast< Body* >( g_pLTServer->HandleToObject( m_hDeadBody ));
				if( pBody )
				{
					pBody->HideBody( false );
				}
				else
				{
					ASSERT( !"CPlayerObj::SetClientLoaded:  Invalid body." );
				}
			}
		}
	}
	else
	{
		// Hide the character when the client isn't loaded, and unhide
		// it when they become loaded.
		HideCharacter( true );
		
		if( IsMultiplayerGame( ))
		{
			// Force us into normal mode (this fixes a bug with switching models
			// when riding vehicles)...
			SetPhysicsModel(PPM_NORMAL);
		}

		// Make us non-solid until we're loaded.
		// [RP] 1/28/03 - Touch Notify is needed for propper physics collision.

		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_SOLID | FLAG_TOUCH_NOTIFY );

		// Disable our hitbox.
		g_pCommonLT->SetObjectFlags( m_hHitBox, OFT_Flags, 0, FLAG_RAYHIT | FLAG_TOUCHABLE );

		// Teleport to our same position so we remove ourselves from containers.
		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
		g_pLTServer->TeleportObject( m_hObject, &vPos );

		// Need to be invulnerable until we're loaded.
		m_damage.SetCanDamage( false );

		RemovePersistentStimuli( );
	}

	// Don't show up on radar until we're loaded.
	bool bRadarVisible = ( bLoaded && ( g_pGameServerShell->GetGameType() != eGameTypeDeathmatch && 
		g_pGameServerShell->GetGameType() != eGameTypeSingle ));
	SetRadarVisible( bRadarVisible );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HealCheat()
//
//	PURPOSE:	Increase hit points
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HealCheat()
{
	if (m_damage.IsDead()) return;

	m_damage.Heal(m_damage.GetMaxHitPoints());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RepairArmorCheat()
//
//	PURPOSE:	Repair our armor
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RepairArmorCheat()
{
	if (m_damage.IsDead()) return;

	m_damage.Repair(m_damage.GetMaxArmorPoints());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullAmmoCheat()
//
//	PURPOSE:	Give us all ammo
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullAmmoCheat()
{
	if (m_damage.IsDead()) return;
	if (!m_pPlayerAttachments) return;

	m_pPlayerAttachments->HandleCheatFullAmmo();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullWeaponCheat()
//
//	PURPOSE:	Give us all weapons
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullWeaponCheat()
{
	if (m_damage.IsDead()) return;
	if (!m_pPlayerAttachments) return;

	m_pPlayerAttachments->HandleCheatFullWeapon();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullModsCheat()
//
//	PURPOSE:	Give us all mods for currently carried weapons
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullModsCheat()
{
	if (m_damage.IsDead()) return;
	if (!m_pPlayerAttachments) return;

	m_pPlayerAttachments->HandleCheatFullMods();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullGearCheat()
//
//	PURPOSE:	Give us all gear...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::FullGearCheat()
{
	if (m_damage.IsDead() || !GetClient( )) return;

    uint8 nNumGearTypes = g_pWeaponMgr->GetNumGearIds();

    for (uint8 i=0; i < nNumGearTypes; i++)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint32(MID_ADDGEAR);
		cMsg.Writeuint8(i);
		g_pLTServer->SendToObject(cMsg.Read(), m_hObject, m_hObject, MESSAGE_GUARANTEED);
		cMsg.Writeuint32(MID_GEAR_PICKEDUP);
		cMsg.Writeuint8(i);
		g_pLTServer->SendToClient(cMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FullGearCheat()
//
//	PURPOSE:	Give a specific weapon...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::GimmeGunCheat( uint8 nId )
{
	if( m_damage.IsDead() || !m_pPlayerAttachments )
		return;

	if( m_pPlayerAttachments->AcquireWeapon( nId ))
		ChangeWeapon( g_pWeaponMgr->GetCommandId( nId ), LTTRUE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GimmeModCheat()
//
//	PURPOSE:	Give a specific mod...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::GimmeModCheat( uint8 nId )
{
	if( m_damage.IsDead() || !m_pPlayerAttachments )
		return;

	m_pPlayerAttachments->AcquireMod( nId );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GimmeGearCheat()
//
//	PURPOSE:	Give a specific gear...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::GimmeGearCheat( uint8 nId )
{
	if( m_damage.IsDead() || !GetClient() )
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint32( MID_ADDGEAR );
	cMsg.Writeuint8( nId );
	g_pLTServer->SendToObject( cMsg.Read( ), m_hObject, m_hObject, MESSAGE_GUARANTEED );
	cMsg.Writeuint32( MID_GEAR_PICKEDUP );
	cMsg.Writeuint8( nId );
	g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GimmeAmmoCheat()
//
//	PURPOSE:	Give a specific ammo...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::GimmeAmmoCheat( uint8 nId )
{
	if( m_damage.IsDead() || !m_pPlayerAttachments )
		return;

	m_pPlayerAttachments->AcquireAmmo( nId );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SkillsCheat()
//
//	PURPOSE:	Give skill points...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SkillsCheat( )
{
	m_Skills.SkillsCheat();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireGear()
//
//	PURPOSE:	Give us the specified gear
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireGear(char* pGearName)
{
	if (!pGearName || !*pGearName || !GetClient( )) return;

	GEAR const *pGear = g_pWeaponMgr->GetGear(pGearName);
	if (!pGear || pGear->bServerRestricted ) return;

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_ADDGEAR);
	cMsg.Writeuint8((uint8)pGear->nId);
	g_pLTServer->SendToObject(cMsg.Read( ), m_hObject, m_hObject, MESSAGE_GUARANTEED);
	cMsg.Writeuint32(MID_GEAR_PICKEDUP);
	cMsg.Writeuint8((uint8)pGear->nId);
	g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Respawn()
//
//	PURPOSE:	Respawn the player
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Respawn(uint8 nServerLoadGameFlags /*=LOAD_NEW_LEVEL*/, bool bGetStartPt /*=true*/)
{
	m_bRespawnCalled = LTTRUE;

    if ( !GetClient( ) )
	{
		ASSERT( !"CPlayerObj::Respawn:  Respawning without a client." );
		return;
	}

	// Check if we have client save data to tell the client.
	if( m_pClientSaveData )
	{
		CAutoMessage cLoadMsg;
		cLoadMsg.Writeuint8( MID_PLAYER_LOADCLIENT );
		cLoadMsg.Writeuint32( m_nSaveVersion );
		cLoadMsg.WriteMessage( m_pClientSaveData );
		SetClientSaveData( NULL );
		g_pLTServer->SendToClient(cLoadMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
	}

	// Turn off leashing..
	m_bUseLeash = LTFALSE;
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);

	// Respawning from a save game.  Load will do all the work.
	if( m_dwLastLoadFlags == LOAD_RESTORE_GAME )
	{
		if (IsMultiplayerGame())
		{
			//our current skill set might be different than what's been saved...
			ResetMultiplayerSkills();
		}
		return;
	}
	
	// Respawning from a transition.  Load will do all the work.
	if( m_dwLastLoadFlags == LOAD_TRANSITION )
	{
		CreateSpecialFX( LTTRUE );
		AcquireCurrentLevelDefaults();
		return;
	}

    if (!g_pWeaponMgr) 
	{
		ASSERT( !"CPlayerObj::Respawn:  Respawning without a weaponmgr." );
		return;
	}

	//if the player requested a team switch, do it now
	if (RequestedTeamChange())
		SwitchToTeam(m_nRequestedTeam);

	// When respawning within the same level we must clear out our weapons.  This
	// flag will be zero after the first update.
	bool bRespawningAfterDeath = ( m_dwLastLoadFlags == 0 );
	if( bRespawningAfterDeath )
	{
		// Switch to non-revivable body if we have a revivable one in tow.
		if( IsRevivePlayerGameType( ) && m_hDeadBody )
			CreateRespawnBody( );

		if( m_pPlayerAttachments )
			m_pPlayerAttachments->DeselectWeapon();
		
		RemoveWeapons();
		RemoveAttachments( true );
	}

	if ( !m_pAttachments )
	{
		CreateAttachments();
		if ( m_pAttachments )
		{
			AddAggregate(m_pAttachments);
			m_pAttachments->Init(m_hObject);
		}

	}

	// Make sure our attachments are still hidden.
	HideAttachments( LTTRUE );

	// Reset our alignment.  Everybody deserves a second chance....

	ResetObjectRelationMgr();

	// Get a start point...

	GameStartPoint* pStartPt = bGetStartPt ? g_pGameServerShell->FindStartPoint(this) : LTNULL;

    LTVector vPos(0, 0, 0);

	if (pStartPt)
	{
		// Set our starting values...

		g_pLTServer->GetObjectPos(pStartPt->m_hObject, &vPos);

		// If in single-player make sure we're using the correct player model...

		if (!IsMultiplayerGame())
		{
			m_eModelId = pStartPt->GetPlayerModelId();
			m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);
		}

		SetPhysicsModel(pStartPt->GetPlayerPhysicsModel());
	}
	else
	{
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
	}

	if (IsMultiplayerGame( ))
	{
		TeleFragObjects(vPos);
	}


	if (pStartPt)
	{
		// Inform the client of the correct camera/player orientation...

        LTVector vVec = pStartPt->GetPitchYawRoll();
		
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PLAYER_ORIENTATION );
		cMsg.Writeuint8( MID_ORIENTATION_ALL );
		cMsg.WriteLTVector( vVec );
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);

		// Get start point command string...

		HSTRING hstrCommand = pStartPt->GetCommand();

		if (hstrCommand)
		{
            m_sStartLevelCommand = g_pLTServer->GetStringData(hstrCommand);

			if (IsMultiplayerGame() && pStartPt->SendCommandOnRespawn())
			{
				m_bSendStartLevelCommand = true;
			}
		}
	}

	g_pLTServer->SetObjectPos(m_hObject, &vPos);


	// Make sure we start on the ground...

	MoveObjectToFloor(m_hObject);


	// Play the respawn sound if multiplayer...

	if (IsMultiplayerGame( ))
	{
		g_pServerSoundMgr->PlaySoundFromPos(vPos, PLAYER_RESPAWN_SOUND,
			600.0f, SOUNDPRIORITY_MISC_HIGH);
	}


	// Update our special fx message...(and tell the client about it as
	// well ;).  We need to do this before aquiring our default weapon...

	CreateSpecialFX(LTTRUE);


	ResetPlayer();
	if (GetState() == PS_DEAD)
	{
		// Always make sure we respawn to the live state.
		// Must become alive before changing weapons.
		ResetAfterDeath( );
	}


	if( !bRespawningAfterDeath )
	{
		if( m_bNewMission )
		{
			m_bNewMission = false;
			SetupForNewMission( );
		}
		else
		{
			AcquireCurrentLevelDefaults();
		}
	}
	// We are respawning within the same level.
	else
	{
		ResetInventory();

		// Get the mission and level defaults.
		AcquireCurrentLevelDefaults();

		// Consider our client already loaded.
		SetClientLoaded( true );
	}


    m_bForceUpdateInterface = true;

	// (To make sure they have their model around)

	UpdateClientPhysics();
	TeleportClientToServerPos();

	// For now, set the permission set to 0 (no permissions) till we have
	// a method for setting it properly.
	m_iPermissionSet = 0;

	// Tell clients we have been respawned...

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( CFX_PLAYER_RESPAWN );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Revive()
//
//	PURPOSE:	Revive the player (Multiplayer only?)
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Revive()
{
	if( !IsMultiplayerGame() || !m_hClient )
		return;

	// Make us invulnerable for a bit.
	if( g_pGameServerShell->GetGameType() == eGameTypeCooperative )
		SetRespawnInvulnerability( );

	// not needed?
	m_bRespawnCalled = LTTRUE;

	HideCharacter( LTFALSE );

	if( m_pAttachments )
	{
		m_pAttachments->HideAttachments( LTFALSE );
	}

	// Turn off leashing..
	m_bUseLeash = LTFALSE;
	g_pLTServer->SetObjectState( m_hObject, OBJSTATE_ACTIVE );

	// Additional multiplayer flags...

	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, m_dwFlags, FLAGMASK_ALL );
	
	CreateSpecialFX( LTTRUE );

	// Bring us back from the dead...

	ResetAfterDeath();
	ResetModel();

	// Duck, so we know we can fit here..
	m_Animator.SetMovement(CAnimatorPlayer::eCrouching);
	
	// Center our lean...
	m_Animator.SetLean( CAnimatorPlayer::eCenter );
	
	m_Animator.Update();

	// Reset the main tracker to make sure we lose any special animations we were playing when we died...
	m_Animator.SetMainToBase();
	
	// Then update the commands, in case we actually can stand up
	UpdateCommands();

	switch (g_pGameServerShell->GetGameType() )
	{
	case eGameTypeDoomsDay:
		ResetHealth( LTFALSE, g_pServerButeMgr->GetPlayerAttributeFloat( PLAYER_BUTE_DOOMREVIVEHEALTH ),
						  g_pServerButeMgr->GetPlayerAttributeFloat( PLAYER_BUTE_DOOMREVIVEARMOR ));
		break;
	case eGameTypeCooperative:
	default:
		ResetHealth( LTFALSE, g_pServerButeMgr->GetPlayerAttributeFloat( PLAYER_BUTE_COOPREVIVEHEALTH ),
						  g_pServerButeMgr->GetPlayerAttributeFloat( PLAYER_BUTE_COOPREVIVEARMOR ));
		break;

	};

	ResetObjectRelationMgr();
	ChangeState( PS_ALIVE );

	m_bForceUpdateInterface = true;

	// (To make sure they have their model around)

	UpdateClientPhysics();

	// Revive us at the position we died...

    LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );


	// Use our dead body's position if it moved...

	if (m_hDeadBody)
	{
		LTVector vBodyPos;
		g_pLTServer->GetObjectPos( m_hDeadBody, &vBodyPos );

		// Only move our object if our body actually moved.  We'll worry
		// about changes in x/z only and ignore changes in y...

		LTVector vTestPos1 = vBodyPos;
		LTVector vTestPos2 = vPos;
		vTestPos1.y = vTestPos2.y = 0.0f;
		if (vTestPos1.NearlyEquals(vTestPos2, 1.0f) == false)
		{
			LTVector vDims;
			g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);

			// Note: since the Body is above the ground this will actually
			// move us up a little too high...MoveObjectToFloor below will
			// move us back down...

			vBodyPos.y += vDims.y; 
			g_pLTServer->SetObjectPos(m_hObject, &vBodyPos);
		}
	}

	// Filter out our dead body if we have one...
	HOBJECT hFilter[] = { m_hObject, m_hDeadBody, LTNULL };

	MoveObjectToFloor(m_hObject, hFilter);
	TeleportClientToServerPos();

	// Play the respawn sound...

	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pServerSoundMgr->PlaySoundFromPos(vPos, PLAYER_RESPAWN_SOUND,
			600.0f, SOUNDPRIORITY_MISC_HIGH);
	
	// Tell clients we have been revived...

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( CFX_PLAYER_REVIVED );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, 0 );

	// Add stimuli back now that we are revived, so that AI can see us again.

	AITRACE( AIShowCharacters, ( m_hObject, "Player '%s' is revived", GetNetUniqueName() ) );
	RegisterPersistentStimuli();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Teleport()
//
//	PURPOSE:	Teleport the player to the specified position with the
//				specified pitch/yaw/roll
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Teleport(const LTVector & vPos, const LTVector & vPitchYawRoll)
{
	// Inform the client of the correct camera/player orientation...

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PLAYER_ORIENTATION );
	cMsg.Writeuint8( MID_ORIENTATION_ALL );
	cMsg.WriteLTVector( vPitchYawRoll );
	g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);

	// Turn off the leash
	m_bUseLeash = LTFALSE;

	// Set our starting position...

	LTVector vNewPos = vPos;
	g_pLTServer->SetObjectPos(m_hObject, &vNewPos);
    g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);

	// Make sure we start on the ground...

	MoveObjectToFloor(m_hObject);

	UpdateClientPhysics();
	TeleportClientToServerPos();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleTeleport()
//
//	PURPOSE:	Teleport the player to the specified point
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleTeleport(TeleportPoint* pTeleportPoint)
{
	if (!pTeleportPoint) return;

	// Get the position and orientation from the teleport point...

    LTVector vPos;
	g_pLTServer->GetObjectPos(pTeleportPoint->m_hObject, &vPos);

    LTVector vPitchYawRoll = pTeleportPoint->GetPitchYawRoll();

	Teleport(vPos, vPitchYawRoll);
}

void CPlayerObj::HandleTeleport(const LTVector& vPos)
{
	CCharacter::HandleTeleport( vPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetAfterDeath()
//
//	PURPOSE:	ResetAfterDeath
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetAfterDeath()
{
	CCharacter::ResetAfterDeath();

	ChangeState(PS_ALIVE);

	g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, FLAG_RAYHIT | FLAG_TOUCHABLE, FLAG_RAYHIT | FLAG_TOUCHABLE );

    LTFLOAT fHealthX = 1.0f;
    LTFLOAT fArmorX  = 1.0f;

	fHealthX = m_Skills.GetSkillModifier(SKL_STAMINA,StaminaModifiers::eMaxHealth);
	fArmorX = m_Skills.GetSkillModifier(SKL_ARMOR,ArmorModifiers::eMaxArmor);

	// make sure progressive damage is reset
	m_damage.ClearProgressiveDamage();

	m_damage.Reset(fHealthX * g_pModelButeMgr->GetModelHitPoints(m_eModelId),
		fArmorX * g_pModelButeMgr->GetModelArmor(m_eModelId), g_pModelButeMgr->GetModelEnergy(m_eModelId));

	m_fAirLevel	= MAX_AIR_LEVEL;

	// Make sure we don't have any bodies.
	DropCarriedObject( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetupForNewMission()
//
//	PURPOSE:	Setup the player for a new mission.
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::SetupForNewMission( )
{
	// Clear our current weapons...

	if ( !m_pPlayerAttachments )
	{
		ASSERT( !"CPlayerObj::SetupForNewMission: Invalid attachments object." );
		return false;
	}

	//reset mission stats
	m_Stats.Init();
	m_Score.Init( g_pLTServer->GetClientID( GetClient() ) );

	// We'll need a updateinterface after all of this.
	m_bForceUpdateInterface = true;

	ResetPlayer();

	if( g_pServerMissionMgr->IsCustomLevel( ))
	{
		ResetHealth();
		ResetInventory();
	}
	else
	{
		// Get the current mission.
		int nCurMission = g_pServerMissionMgr->GetCurrentMission( );
		MISSION* pMission = g_pMissionButeMgr->GetMission( nCurMission );
		if( !pMission )
			return false;

		// Reset the player if told or if this is a new game.
		if( pMission->bResetPlayer || g_pGameServerShell->GetLGFlags( ) == LOAD_NEW_GAME )
		{
			ResetHealth();
			ResetInventory();
			RemoveCarriedObject();

			if( m_hVehicleModel )
			{
				g_pLTServer->RemoveObject( m_hVehicleModel );
				m_hVehicleModel = NULL;
			}
		}

		// Get the mission and lefel defaults...
		if( !AcquireCurrentLevelDefaults() )
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireDefaultWeapon()
//
//	PURPOSE:	Give us the default weapons
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireDefaultWeapon()
{
	char szWeaponName[256];
	szWeaponName[0] = '\0';
	WEAPON const *pWeaponData = LTNULL;
	int nNewWeapon = -1;

	g_pServerButeMgr->GetPlayerAttributeString(PLAYER_BUTE_DEFAULTWEAPON,
		szWeaponName, ARRAY_LEN(szWeaponName));

	if( !szWeaponName[0] )
		return;

	pWeaponData = g_pWeaponMgr->GetWeapon( szWeaponName );
	if( !pWeaponData )
		return;

	AMMO const *pAmmoData = g_pWeaponMgr->GetAmmo(pWeaponData->nDefaultAmmoId);
	if( !pAmmoData )
		return;

	if( !m_pPlayerAttachments )
		return;

	m_pPlayerAttachments->ObtainWeapon(pWeaponData->nId, pAmmoData->nId, pAmmoData->nSpawnedAmount, LTTRUE);
	nNewWeapon = g_pWeaponMgr->GetCommandId(pWeaponData->nId);
	if( nNewWeapon < 0)
		return;

	ChangeWeapon(nNewWeapon, LTTRUE);

	// Tell the client this is their default weapon.
	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cClientMsg.Writeuint8(IC_DEFAULTWEAPON_ID);
	cClientMsg.Writeuint8(pWeaponData->nId);
	cClientMsg.Writeuint8(0);
	cClientMsg.Writefloat(0.0f);
	g_pLTServer->SendToClient(cClientMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireWeapon()
//
//	PURPOSE:	Give us the specified weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireWeapon(char* pWeaponName)
{
	if (!pWeaponName || !*pWeaponName) return;

    uint8 nWeaponID = WMGR_INVALID_ID;
    uint8 nAmmoID = WMGR_INVALID_ID;
	char szWeaponName[128];
	strcpy(szWeaponName, pWeaponName);
	g_pWeaponMgr->ReadWeapon(szWeaponName, nWeaponID, nAmmoID);

	WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponID);

	if (pWeaponData && !pWeaponData->bServerRestricted )
	{
		AMMO const *pAmmoData = g_pWeaponMgr->GetAmmo(nAmmoID);
		if (pAmmoData && !pAmmoData->bServerRestricted && m_pPlayerAttachments)
		{
            m_pPlayerAttachments->ObtainWeapon(pWeaponData->nId, pAmmoData->nId, pAmmoData->nSpawnedAmount, LTTRUE);
			ChangeWeapon(g_pWeaponMgr->GetCommandId(pWeaponData->nId));
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireAmmo()
//
//	PURPOSE:	Give us the specified ammo
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireAmmo(char* pAmmoName)
{
	if (!pAmmoName || !*pAmmoName) return;


	AMMO const *pAmmoData = g_pWeaponMgr->GetAmmo(pAmmoName);

	if( pAmmoData && !pAmmoData->bServerRestricted )
	{
		if (pAmmoData && m_pPlayerAttachments)
		{
            m_pPlayerAttachments->AcquireAmmo(pAmmoData->nId );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireMod()
//
//	PURPOSE:	Give us the specified Mod
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireMod(char* pModName)
{
	if (!pModName || !*pModName) return;


	MOD const *pModData = g_pWeaponMgr->GetMod(pModName);
	if( pModData )
	{
		if (pModData && m_pPlayerAttachments)
		{
            m_pPlayerAttachments->AcquireMod(pModData->nId, false);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeToWeapon()
//
//	PURPOSE:	Change to the specified weapon (if we have it)
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeToWeapon(const char* pWeaponName)
{
	if (!pWeaponName || !*pWeaponName) return;

	WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon(pWeaponName);
	if (!pWeaponData || !m_pPlayerAttachments) return;

	CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon(pWeaponData->nId);
	if (pWeapon && pWeapon->Have())
	{
		ChangeWeapon(g_pWeaponMgr->GetCommandId(pWeaponData->nId));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleDead()
//
//	PURPOSE:	Tell client I died
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleDead(LTBOOL)
{
	if (!m_hObject)
		return;

	if (m_eState == PS_DEAD)
		return;

	// The player just died.  The body we make is revivable.
	if( IsRevivePlayerGameType( ))
	{
		m_bCanBeRevived = true;
		m_PStateChangeFlags |= PSTATE_INTERFACE;
	}

	if( m_pAttachments )
	{
		m_pAttachments->HideAttachments( LTTRUE );
	}

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_TOUCH_NOTIFY);
	g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, 0, FLAG_TOUCHABLE );

    CCharacter::HandleDead(LTFALSE);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE | FLAG_RAYHIT);
	g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, 0, FLAG_RAYHIT);


	// make sure progressive damage is reset
	m_damage.ClearProgressiveDamage();

	ChangeState(PS_DEAD);

	// Tell clients we are dead...

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( CFX_PLAYER_DEAD );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, 0 );

	SetTracking( false );

	// Remove stimuli so that AI do not treat a dead player as an enemy.

	AITRACE( AIShowCharacters, ( m_hObject, "Player '%s' is dead.", GetNetUniqueName() ) );
	RemovePersistentStimuli();

	//if the player killed himself, to switch teams, respawn immediately
	if (RequestedTeamChange() && CanRespawn())
		Respawn();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::StartDeath()
//
//	PURPOSE:	Tell client I'm dying
//
// ----------------------------------------------------------------------- //

void CPlayerObj::StartDeath()
{
	// If we're on a vehicle, detach ourselves and spawn a
	// vehicle powerup...

	if (m_ePPhysicsModel != PPM_NORMAL)
	{
		char* pSound;
		if (GetRandom(0, 1) == 1)
		{
			pSound = "Snd\\Vehicle\\vehiclecrash1.wav";
		}
		else
		{
			pSound = "Snd\\Vehicle\\vehiclecrash2.wav";
		}
		PlaySound(pSound, m_fSoundRadius, LTTRUE);

		SetPhysicsModel(PPM_NORMAL);
	}

	// Drop any body we might be carrying.
	DropCarriedObject( );
	
	CCharacter::StartDeath();
	ChangeState(PS_DYING);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeState()
//
//	PURPOSE:	Notify Client of changed state
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeState(PlayerState eState)
{
	if (!GetClient( )) return;

	m_eState = eState;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_STATE_CHANGE);
	cMsg.Writeuint8((uint8)m_eState);
	g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetPlayer()
//
//	PURPOSE:	Reset the player values
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetPlayer()
{
	// Reset the model...

	ResetModel();

	// Calculate the friction...

    g_pPhysicsLT->SetFrictionCoefficient(m_hObject, DEFAULT_FRICTION);
	m_PStateChangeFlags |= PSTATE_SPEEDS; // Resend friction..
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetHealth()
//
//	PURPOSE:	Reset the health and armor
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetHealth(LTBOOL bMaxOnly, float fHealthPercent, float fArmorPercent )
{

    LTFLOAT fHealthX = 1.0f;
    LTFLOAT fArmorX = 1.0f;
	LTFLOAT fEnergyX = 1.0f;

	fHealthX = m_Skills.GetSkillModifier(SKL_STAMINA,StaminaModifiers::eMaxHealth);
	fArmorX = m_Skills.GetSkillModifier(SKL_ARMOR,ArmorModifiers::eMaxArmor);

	m_damage.SetMaxHitPoints(fHealthX * g_pModelButeMgr->GetModelMaxHitPoints(m_eModelId));
	m_damage.SetMaxArmorPoints(fArmorX * g_pModelButeMgr->GetModelMaxArmor(m_eModelId));
	m_damage.SetMaxEnergy(fEnergyX * g_pModelButeMgr->GetModelMaxEnergy(m_eModelId));

	if (!bMaxOnly)
	{
		m_damage.SetHitPoints( fHealthPercent * fHealthX * g_pModelButeMgr->GetModelHitPoints(m_eModelId));
		m_damage.SetArmorPoints( fArmorPercent * fArmorX * g_pModelButeMgr->GetModelArmor(m_eModelId));
		m_damage.SetEnergy(fEnergyX * g_pModelButeMgr->GetModelEnergy(m_eModelId));
	}

	if (GetClient( ))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_MAX_HEALTH_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_damage.GetMaxHitPoints());
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);

		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_MAX_ARMOR_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_damage.GetMaxArmorPoints());
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);

		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_MAX_ENERGY_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_damage.GetMaxHitPoints());
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetInventory()
//
//	PURPOSE:	Reset our inventory
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetInventory(LTBOOL bRemoveGear)
{
	if (!m_pPlayerAttachments) return;

	// Clear our weapons/ammo...

	m_pPlayerAttachments->ResetAllWeapons();

	// Clear our gear...

	if (bRemoveGear)
	{
		m_damage.RemoveAllGear();
	}

	// Tell the client to clear out all our inventory...

	if (GetClient( ))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_RESET_INVENTORY_ID);
		cMsg.Writeuint8(bRemoveGear);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(0.0f);
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
	}

	// Well...give us at least *one* weapon ;)

	AcquireDefaultWeapon();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetModel()
//
//	PURPOSE:	Reset the model
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetModel()
{
	ObjectCreateStruct createstruct;
	createstruct.Clear();

	const char* pFilename = g_pModelButeMgr->GetModelFilename(m_eModelId);
	SAFE_STRCPY(createstruct.m_Filename, pFilename);

	g_pModelButeMgr->CopySkinFilenames(m_eModelId, 0, createstruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	g_pModelButeMgr->CopyRenderStyleFilenames(m_eModelId, &createstruct);

    g_pCommonLT->SetObjectFilenames(m_hObject, &createstruct);

	// Make sure the client knows about any changes...

	m_PStateChangeFlags |= PSTATE_MODELFILENAMES;

	m_damage.SetMass(g_pModelButeMgr->GetModelMass(m_eModelId));

	// Reset the animations...

	m_Animator.Reset(m_hObject);
	
	// Reset the main tracker to make sure we lose any special animations we were playing when we died...

	m_Animator.SetMainToBase();

	m_bInitializedAnimation = LTFALSE;

	if (m_bShortRecoil)
	{
	 	g_pModelLT->RemoveTracker(m_hObject, m_RecoilAnimTracker);
	}

	if (m_bBlink)
	{
 	 	g_pModelLT->RemoveTracker(m_hObject, m_BlinkAnimTracker);
	} 

	if( m_pPlayerAttachments )
	{
		// Reattach all of our attachments in case we have changed models.

		m_pPlayerAttachments->RemoveAndRecreateAttachments();

		// Detach any prop or object attachments...
		
		m_pPlayerAttachments->DetachAttachmentsOfType( ATTACHMENT_TYPE_PROP );
		m_pPlayerAttachments->DetachAttachmentsOfType( ATTACHMENT_TYPE_OBJECT );
	}
	
	// Run throught the list of default attachments for the model and attach them...

	const char* pszAttachmentPos;
	const char* pszAttachment;
	char szTrigger[128];

	uint32 cAttachments = g_pModelButeMgr->GetNumDefaultAttachments( m_eModelId );
	for( uint32 iAttachment = 0; iAttachment < cAttachments; ++iAttachment )
	{
		g_pModelButeMgr->GetDefaultAttachment( m_eModelId, iAttachment, pszAttachmentPos, pszAttachment );

		sprintf( szTrigger, "%s %s %s", KEY_ATTACH, pszAttachmentPos, pszAttachment );
		SendTriggerMsgToObject( this, m_hObject, LTFALSE, szTrigger );
	}

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, dwFlags );

	HideAttachments( !(dwFlags & FLAG_VISIBLE) );

	/*
	if (IsMultiplayerGame( ))
	{
		if ( m_pPlayerAttachments )
		{
			m_pPlayerAttachments->ResetRequirements();
			m_pPlayerAttachments->AddRequirements(m_eModelId);
		}
	}
	*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeWeapon
//
//	PURPOSE:	Tell the client to change the weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeWeapon(uint8 nCommandId, LTBOOL bAuto, int32 nAmmoId)
{
	if (!GetClient( )) return;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_WEAPON_CHANGE);
	cMsg.Writeuint8(nCommandId);
	cMsg.Writeuint8(bAuto);
	cMsg.Writefloat((LTFLOAT)nAmmoId);
	g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoWeaponChange
//
//	PURPOSE:	Change our weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoWeaponChange(uint8 nWeaponId,uint8 nAmmoId)
{
	if ( !m_pPlayerAttachments ) return;

	m_pPlayerAttachments->ChangeWeapon(nWeaponId);

	CWeapon *pWeapon = m_pPlayerAttachments->GetWeapon();
	if( pWeapon )
	{
		pWeapon->SetAmmoId( nAmmoId );
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateInterface
//
//	PURPOSE:	Tell the client of about any changes
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateInterface(bool bForceUpdate)
{
	if ( !m_pPlayerAttachments ) return;
	if (!GetClient( ) || !g_pWeaponMgr) return;

	if( m_bForceUpdateInterface )
	{
		m_bForceUpdateInterface = false;
		bForceUpdate = true;
	}


	// See if the ammo has changed...

    uint8 nNumAmmoTypes = g_pWeaponMgr->GetNumAmmoIds();

	for (int i=0; i < nNumAmmoTypes; i++)
	{
		int nAmmo = m_pPlayerAttachments->GetAmmoCount(i);

		if (m_pnOldAmmo)
		{
			if (m_pnOldAmmo[i] != nAmmo || bForceUpdate)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
				cMsg.Writeuint8(IC_AMMO_ID);
				cMsg.Writeuint8(WMGR_INVALID_ID);
				cMsg.Writeuint8(i);
				cMsg.Writefloat((LTFLOAT)nAmmo);
				g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
			}

			m_pnOldAmmo[i] = nAmmo;
		}
	}


	// See if health has changed...

	if ( ( abs( (int)m_fOldHitPts - (int)m_damage.GetHitPoints() )  >= 1) || bForceUpdate)
	{
		m_fOldHitPts = m_damage.GetHitPoints();

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_HEALTH_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_fOldHitPts);
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
	}


	// See if armor has changed...

	if (m_fOldArmor != m_damage.GetArmorPoints() || bForceUpdate)
	{
		m_fOldArmor = m_damage.GetArmorPoints();

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_ARMOR_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_fOldArmor);
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
	}

	// [kml] 2/8/02
	// See if energy has changed...

	if (m_fOldEnergy != m_damage.GetEnergy() || bForceUpdate)
	{
		m_fOldEnergy = m_damage.GetEnergy();

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_ENERGY_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(m_fOldEnergy);
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
	}


	// See if air level has changed...

	if (m_fOldAirLevel != m_fAirLevel || bForceUpdate)
	{
		m_fOldAirLevel = m_fAirLevel;
        LTFLOAT fPercent = m_fAirLevel / MAX_AIR_LEVEL;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_AIRLEVEL_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(fPercent);
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetDamageSound
//
//	PURPOSE:	Determine what damage sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerObj::GetDamageSound(DamageType eType)
{
	const char* pDir = g_pModelButeMgr->GetModelPlayerPainSndDir( m_eModelId );
	if( !pDir )
	{
		return LTNULL;
	}

	LTSNPrintF( s_FileBuffer, ARRAY_LEN( s_FileBuffer ), "Chars\\Snd\\%s\\", pDir );

	if (IsLiquid(m_eContainerCode))
	{
		strcat(s_FileBuffer, "underwaterpain.wav");
		return s_FileBuffer;
	}

	DTINFO const* pInfo = GetDTINFO( eType );
	if( !pInfo )
		return NULL;

	if( pInfo->saPlayerDamageSounds.size( ) == 0 )
	{
		char const* PainSounds[] =  { "pain01.wav", "pain02.wav", "pain03.wav", "pain04.wav", "pain05.wav" };

		int nSize = ARRAY_LEN(PainSounds) - 1;
		strcat(s_FileBuffer, PainSounds[GetRandom(0, nSize)]);
	}
	else
	{
		uint32 nRandSelection = GetRandom( 0, pInfo->saPlayerDamageSounds.size( ) - 1 );
		char const* pszDamageSound = pInfo->saPlayerDamageSounds[nRandSelection].c_str( );
		strcat( s_FileBuffer, pszDamageSound );
	}

	return s_FileBuffer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetDeathSound
//
//	PURPOSE:	Determine what death sound to play
//
// ----------------------------------------------------------------------- //

char* CPlayerObj::GetDeathSound()
{
/*
	if (g_pVersionMgr->IsLowViolence())
	{
		return LTNULL;
	}
*/

	DamageType eType = m_damage.GetDeathType();

	const char* pDir = g_pModelButeMgr->GetModelPlayerPainSndDir( m_eModelId );
	if( !pDir )
	{
		return LTNULL;
	}

	LTSNPrintF( s_FileBuffer, ARRAY_LEN( s_FileBuffer ), "Chars\\Snd\\%s\\", pDir );

	if (IsLiquid(m_eContainerCode))
	{
		strcat(s_FileBuffer, "underwaterdeath.wav");
		return s_FileBuffer;
	}


	switch (eType)
	{
		case DT_ENDLESS_FALL:
		{
			strcat(s_FileBuffer, "endless_fall.wav");
		}
		break;

		default:
		{
			char* DeathSounds[] =  { "death01.wav", "death02.wav", "death03.wav", "death04.wav", "death05.wav" };

			int nSize = (sizeof(DeathSounds)/sizeof(DeathSounds[0])) - 1;
			strcat(s_FileBuffer, DeathSounds[GetRandom(0, nSize)]);
		}
	}

	return s_FileBuffer;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateAirLevel()
//
//	PURPOSE:	Update our air usage
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateAirLevel()
{
    LTFLOAT fDeltaTime = g_pLTServer->GetFrameTime();

	// See if we are in a liquid...

	if (IsLiquid(m_eContainerCode) && !m_damage.HasAirSupply())
	{
		LTFLOAT fFullAirLostTime = g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_FULLAIRLOSTTIME);
        if (fFullAirLostTime <= 0.0f) return;

		LTFLOAT fDeltaAirLoss	 = (MAX_AIR_LEVEL/fFullAirLostTime);

		m_fAirLevel -= fDeltaTime*fDeltaAirLoss;

		if (m_fAirLevel < 0.0f)
		{
			m_fAirLevel = 0.0f;

			LTFLOAT fAirLostDamage = g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_AIRLOSSDAMAGE);
	        
			LTFLOAT fDamage = fAirLostDamage*fDeltaTime;

			DamageStruct damage;

			damage.eType	= DT_CHOKE;
			damage.fDamage	= fDamage;
			damage.hDamager = m_hObject;
			damage.vDir.Init(0.0, 0.0, 0.0);

			damage.DoDamage(this, m_hObject);
		}
	}
	else if (m_fAirLevel < MAX_AIR_LEVEL)
	{
 		LTFLOAT fFullAirRegenTime = g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_FULLAIRREGENTIME);
        if (fFullAirRegenTime <= 0.0f) return;

		LTFLOAT fDeltaAirRegen = (MAX_AIR_LEVEL/fFullAirRegenTime);
		m_fAirLevel += fDeltaTime*fDeltaAirRegen;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateHealth()
//
//	PURPOSE:	Update our health
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateHealth()
{
	if (m_damage.IsDead()) return;

	LTFLOAT fDeltaTime = g_pLTServer->GetFrameTime();

	LTVector vCurPos, vVel;
	g_pLTServer->GetObjectPos(m_hObject, &vCurPos);

	vVel = (vCurPos - m_vLastPos) / fDeltaTime;

	m_vLastPos = vCurPos;

	if (m_damage.GetHitPoints() >= m_damage.GetMaxHitPoints()) return;


	LTFLOAT fHeal = 0.0;
	if (vVel.Mag() > m_fWalkVel)
	{
		fHeal = s_vtWalkHealRate.GetFloat(0.1f);		
	}
	else
	{
		fHeal = s_vtBaseHealRate.GetFloat(0.2f);		
	}

	fHeal *= fDeltaTime;

	m_damage.Heal(fHeal);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateHiding()
//
//	PURPOSE:	Update our hiding state
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateHiding()
{
	if (CanHide())
	{
		if ( !m_bHiding )
		{
			StartHiding();
			m_fHideDuration = GetConsoleFloat("HideTime", 3.0f) * m_Skills.GetSkillModifier(SKL_STEALTH,StealthModifiers::eHideTime);
		}

		LTFLOAT fLast = m_fHiddenTimer;
		m_fHiddenTimer += g_pLTServer->GetFrameTime();

		// Reset interface if player was previously seen by an enemy.

		if( ( fLast < 0.f ) && ( m_fHiddenTimer >= 0.f ) )
		{
			m_PStateChangeFlags |= PSTATE_INTERFACE;
		}

		if ( m_fHiddenTimer > m_fHideDuration && !m_bHidden)
		{
			StartHidden();
		}
	}
	else
	{
		ClearHiding();
	}
}

LTBOOL CPlayerObj::CanHide() 
{ 
	if (HasCurrentInformationVolume())
	{
		AIInformationVolume* pVol = GetCurrentInformationVolume();
		if (!pVol) return LTFALSE;

		if (pVol->GetVolumeType() != AIInformationVolume::eTypePlayerInfo) return LTFALSE;
		AIVolumePlayerInfo* pPlayerInfoVol = (AIVolumePlayerInfo*)pVol;

		if ( pPlayerInfoVol->IsOn() 
			&& ( !IsFlashLightOn() )
			&& pPlayerInfoVol->IsOn()
			&& pPlayerInfoVol->IsHiding()
			&& ( !pPlayerInfoVol->IsHidingCrouchRequired() ||
				(m_Animator.GetLastMovement() == CAnimatorPlayer::eCrouching)))
		{
			return LTTRUE;
		}
	}

	return LTFALSE;
}

void CPlayerObj::StartHiding() 
{ 
	if ( !m_bHiding )
	{
		m_PStateChangeFlags |= PSTATE_INTERFACE;
	}

	m_bHiding = LTTRUE;
}

void CPlayerObj::StartHidden() 
{ 
	if (!m_bHidden)
	{
		m_PStateChangeFlags |= PSTATE_INTERFACE;
	}

	m_bHidden = LTTRUE; 
}

void CPlayerObj::EndHidden()
{
	if( m_bHidden || m_bHiding )
	{
		m_PStateChangeFlags |= PSTATE_INTERFACE;
	}
	
	m_bHidden = LTFALSE;
	m_fHiddenTimer = 0.0f;
}

void CPlayerObj::ClearHiding() 
{ 
	if (m_bHiding || m_bHidden)
	{
		m_PStateChangeFlags |= PSTATE_INTERFACE;

	}

	m_bHiding = LTFALSE; 
	m_bHidden = LTFALSE; 
	m_fHiddenTimer = 0.0f; 
	m_fHideDuration = 3.0f;
}

void CPlayerObj::SetVisibleToEnemyAI( CAI* pAI, bool bVis )
{
	if( pAI && pAI->GetBrain()->GetAIDataExist( kAIData_DoNotExposeHiding ) )
	{
		return;
	}

	m_bVisibleToEnemyAI = bVis;

	// Let the interface know we can't hide...
	
	if( m_bHiding || m_bHidden )
	{
		m_PStateChangeFlags |= PSTATE_INTERFACE;

		m_bHidden = LTFALSE;
		m_bHiding = LTFALSE; 
	}

	m_fHiddenTimer = -1.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoActivate()
//
//	PURPOSE:	Activate the object in front of us (return true if an
//				object was activated)
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::DoActivate(CActivationData* pData)
{
	if (m_eState != PS_ALIVE) return LTFALSE;

	// Not allowed to do any activating when paused.
	uint32 dwFlags = 0;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, dwFlags );
	if ( dwFlags & FLAG_PAUSED )
	{
		return LTFALSE;
	}

	// First see if a cinematic is running...If so, try and stop it...

    if (PlayingCinematic(LTTRUE))
	{
        return LTFALSE;
	}


	// If we're on a vehicle, time to get off...

	if( (m_ePPhysicsModel != PPM_NORMAL) && (m_ePPhysicsModel != PPM_LURE) )
	{
		SetPhysicsModel(PPM_NORMAL);
	    return LTFALSE;
	}

	// If we're carrying an object, drop it...

	if( m_hCarriedObject && CanDropObject( ) && pData->m_nType == MID_ACTIVATE_MOVE )
	{
		if( !DropCarriedObject( ) )
		{
			return LTFALSE;
		}

		return LTTRUE;
	}

	if (pData->m_hTarget)
	{
		HSTRING hStr = LTNULL;
		GameBase *pObj = (GameBase *)g_pLTServer->HandleToObject(pData->m_hTarget);
			
		switch (pData->m_nType)
		{
			case MID_ACTIVATE_SEARCH:
			{
				hStr = g_pLTServer->CreateString("SEARCH");
				if (hStr)
				{
					SendTriggerMsgToObject(this, pData->m_hTarget, hStr);
					g_pLTServer->FreeString(hStr);

					return LTTRUE;
				}
			}
			break;
		
			case MID_ACTIVATE_MOVE:
			{
				//you can't pick it up  if you're crouching so go away
				if ( m_Animator.GetLastMovement() == CAnimatorPlayer::eCrouching )
					return LTFALSE;

				uint32 dwFlags;
				g_pCommonLT->GetObjectFlags( pData->m_hTarget, OFT_User, dwFlags );

				//if the target isn't a body, we can't move it
				if (IsKindOf(pData->m_hTarget, "Body"))
				{
					Body* pBody = (Body*)g_pLTServer->HandleToObject(pData->m_hTarget);
					if (!pBody || pBody->GetBodyState() != kState_BodyNormal) 
					{
						return LTFALSE;
					}
				}
				else if (IsKindOf(pData->m_hTarget, "CAIHuman"))
				{
					CAIHuman* pAI = (CAIHuman*)g_pLTServer->HandleToObject(pData->m_hTarget);
					if (!pAI || !pAI->IsUnconscious()) 
					{
						return LTFALSE;
					}
				}
				else if( IsKindOf( pData->m_hTarget, "DoomsDayPiece" ) )
				{
					// Is there anything we need to check on the object??
				}
				else
				{
					return LTFALSE;
				}

				if( !PickupObject( pData->m_hTarget ))
				{
					return LTFALSE;
				}

				return LTTRUE;
			} 
			break;

			case MID_ACTIVATE_REVIVE:
			{
				// If the target isn't a body, we cant' revive it

				if (!IsKindOf(pData->m_hTarget, "Body"))
				{
					return LTFALSE;
				}

				Body *pBody = dynamic_cast<Body*>(g_pLTServer->HandleToObject( pData->m_hTarget ));
				if (!pBody || pBody->GetBodyState() != kState_BodyNormal) 
				{
						return LTFALSE;
				}

				hStr = g_pLTServer->CreateString( "REVIVE" );
				if( hStr )
				{
					SendTriggerMsgToObject( this, pData->m_hTarget, hStr );
					g_pLTServer->FreeString( hStr );

					return LTTRUE;
				}
			}
			break;
			
			case MID_ACTIVATE_WAKEUP:
			{
				if( !IsCharacter( pData->m_hTarget ))
				{
					return LTFALSE;
				}

				HSTRING hstr = g_pLTServer->CreateString( "WAKEUP" );
				if( hstr )
				{
					SendTriggerMsgToObject( this, pData->m_hTarget, hstr );
					g_pLTServer->FreeString( hstr );

					return LTTRUE;
				}
			}
			break;

			case MID_ACTIVATE_NORMAL:
			default:
			{
				hStr = g_pLTServer->CreateString("ACTIVATE");

				if (hStr)
				{
					SendTriggerMsgToObject(this, pData->m_hTarget, hStr);
					g_pLTServer->FreeString(hStr);

					return LTTRUE;
				}
			}
			break;
		}
    }

	if (pData->m_nSurfaceType != ST_UNKNOWN)
	{
		SURFACE *pSurf = g_pSurfaceMgr->GetSurface((SurfaceType)pData->m_nSurfaceType);

		// See if the surface we tried to activate has an activation
		// sound...If so, play it...
		if (pSurf && pSurf->szActivationSnd[0] && pSurf->fActivationSndRadius > 0)
		{
			g_pServerSoundMgr->PlaySoundFromPos(pData->m_vIntersect, pSurf->szActivationSnd,
						pSurf->fActivationSndRadius, SOUNDPRIORITY_PLAYER_LOW);
		}

		return LTTRUE;
	}
	

    return LTFALSE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PlayingCinematic()
//
//	PURPOSE:	See if we are playing a cinematic (and stop it if specified)
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::PlayingCinematic(LTBOOL bStopCinematic)
{
	// Search for cinematic object if we need to stop it...

	if (Camera::IsActive() && bStopCinematic)
	{
        HOBJECT hObj = g_pLTServer->GetNextObject(LTNULL);

		// Stop all the Cameras that are currently active...

		while (hObj)
		{
			if (IsKindOf(hObj, "Camera"))
			{
                Camera* pCam = (Camera*) g_pLTServer->HandleToObject(hObj);
				if( pCam && pCam->IsOn() )
				{
					SendTriggerMsgToObject(this, hObj, LTFALSE, "SKIP");
				}
			}

            hObj = g_pLTServer->GetNextObject(hObj);
		}
		
		// Stop the music...
		/* [KLS 9/6/02] Cinematic music now uses .wav files so it
		   be paused, so we don't need to stop it (in fact we shouldn't :)
		ConParse parse;
		parse.Init("MUSIC STOP IMMEDIATELY");
		g_pCommonLT->Parse(&parse);
		CParsedMsg cpMsg(parse.m_nArgs, parse.m_Args);
		HandleMusicMessage(cpMsg);
		*/
	}

	return Camera::IsActive();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessDamageMsg()
//
//	PURPOSE:	Process a damage message.
//
// --------------------------------------------------------------------------- //

void CPlayerObj::ProcessDamageMsg(ILTMessage_Read *pMsg)
{
	if (!pMsg || !GetClient( )) return;

	DamageStruct damage;
	damage.InitFromMessage(pMsg);

	// Check for FriendlyFire...
	
	if( IsMultiplayerGame( ) && (1.0f > g_vtNetFriendlyFire.GetFloat( )) && IsPlayer( damage.hDamager ) && (damage.hDamager != m_hObject) )
	{
		if( IsCoopMultiplayerGameType( ) || (IsTeamGameType( ) && AreSameTeam( damage.hDamager, m_hObject )) ) 
		{
			return;
		}
	}

	CCharacter::ProcessDamageMsg(pMsg);

	// Tell the client about the damage...

    LTFLOAT fDamage = m_damage.GetLastDamage();
    LTFLOAT fArmorAbsorb = m_damage.GetLastArmorAbsorb();

	if (fDamage > 0.0f || fArmorAbsorb > 0.0f)
	{
		LTBOOL bTookHealth = (fDamage > 0.0f) ? LTTRUE : LTFALSE;
		LTFLOAT fVal = fArmorAbsorb + fDamage;

        LTFLOAT fPercent = fVal / m_damage.GetMaxHitPoints();
		if (fPercent > 1.0f) fPercent = 1.0f;

        LTVector vDir = m_damage.GetLastDamageDir();
		vDir.Normalize();
		VEC_MULSCALAR(vDir, vDir, fPercent);

		CAutoMessage cDamageMsg;
		cDamageMsg.Writeuint8(MID_PLAYER_DAMAGE);
		cDamageMsg.WriteLTVector(vDir);
		cDamageMsg.Writeuint8(m_damage.GetLastDamageType());
		cDamageMsg.Writeuint8((uint8)bTookHealth);
		g_pLTServer->SendToClient(cDamageMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
	}

}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::MultiplayerInit
//
//	PURPOSE:	Init multiplayer values
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::MultiplayerInit(ILTMessage_Read *pMsg)
{
    if (!g_pGameServerShell || !GetClient( ))
		return LTFALSE;

	m_eModelId = ( ModelId )pMsg->Readuint8( );
	m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);

	uint8 nTeam = pMsg->Readuint8( );

	// If this is a team game, then we take our model info from the team.
	if( IsTeamGameType( ))
	{
		CTeamMgr::Instance().AddPlayer( g_pLTServer->GetClientID(GetClient( )), nTeam );

		CTeamMgr::Instance().UpdateClient();

		// Don't reset the model yet, since that will happen agian when we respawn.
		if( !SetupTeamModel( false ))
		{
			return LTFALSE;
		}
	}

	m_Skills.MultiplayerInit(pMsg);

	g_pGameServerShell->SetUpdateGameServ();

	// Force us into normal mode (this fixes a bug with switching models
	// when riding vehicles)...
	SetPhysicsModel(PPM_NORMAL);

    return LTTRUE;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::MultiplayerUpdate
//
//	PURPOSE:	Update multiplayer values
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::MultiplayerUpdate(ILTMessage_Read *pMsg)
{
    if (!g_pGameServerShell || !GetClient( ))
		return LTFALSE;

	// Get our current clientdata so we can overwrite it.
	NetClientData ncd;
	int nNcdSize = sizeof( ncd );
	g_pLTServer->GetClientData( GetClient( ), ( uint8* )&ncd, nNcdSize );

	// Get the name the client would like.
	pMsg->ReadString( ncd.m_szName, ARRAY_LEN( ncd.m_szName ));

	// Update the players unique name.
	char szUniqueName[MAX_PLAYER_NAME];
	GenerateUniqueName(ncd.m_szName, szUniqueName, ARRAY_LEN( szUniqueName ));
	m_sNetNameUnique = szUniqueName;
	ModelId eModelId = ( ModelId )pMsg->Readuint8( );

	uint8 nTeam = pMsg->Readuint8( );

	
	// Don't allow model changes in team games.
	if( IsTeamGameType( ))
	{
		if (nTeam == INVALID_TEAM)
		{
			if (GetTeamID() == INVALID_TEAM)
			{
				CTeam* pTeam = CTeamMgr::Instance().GetTeamWithLeastPlayers();
				if (pTeam)
					nTeam = pTeam->GetID();
			}
			else
				nTeam = GetTeamID();
		}

		//don't allow team switches if you can't respawn
		if (nTeam != GetTeamID() && g_pServerMissionMgr->CanRespawn( *this ))
		{

			m_nRequestedTeam = nTeam;

			//we have to be dead to respawn on new team properly
			if (GetState() == PS_ALIVE)
			{
				// if we're not loaded, temporarily turn off invulnerability
				if (!m_bClientLoaded)
					m_damage.SetCanDamage( true );

				DamageStruct damage;

				damage.eType	= DT_UNSPECIFIED;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = NULL;
				damage.vDir.Init(0, 1, 0);

				damage.DoDamage(this, m_hObject);

				// if we're not loaded turn invulnerability back on
				if (!m_bClientLoaded)
					m_damage.SetCanDamage( false );

			}
			else if (GetState() == PS_DEAD)
			{
				// tell the client they'll switch when they respawn
				uint8 nClientID = (uint8) g_pLTServer->GetClientID(GetClient( ));
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_TEAM);
//				cMsg.Writeuint8(nClientID);
//				cMsg.Writeuint8(m_nRequestedTeam);
//				cMsg.Writebool(true);
				g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);

				CancelRevive();
			}
	
			
		}
	}
	else
	{
		m_eModelId = eModelId;
		m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton(m_eModelId);
	}

	// Tell the server about the changes we made.
	ncd.m_ePlayerModelId = m_eModelId;
	g_pLTServer->SetClientData( GetClient( ), ( uint8* )&ncd, sizeof( ncd ));

	g_pGameServerShell->SetUpdateGameServ();

	ResetModel();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;
	
	// Players will save the objectives stored in the server shell so they will 
	// properly carry across levels.  

	g_pGameServerShell->SaveObjectives( pMsg, dwSaveFlags );

	// Save the command mgrs' global vars here so we bring them across levels

	g_pCmdMgr->SaveGlobalVars( pMsg, LTFALSE );

	// Save animator

	m_Animator.Save(pMsg);

	// Make sure the player skills gets saved...

    m_Skills.Save(pMsg);
    m_Stats.WriteData(pMsg);

	// Save PlayerObj data...

	SAVE_HOBJECT(m_hVehicleModel);

	SAVE_CHARSTRING(m_sStartLevelCommand);

	SAVE_FLOAT(m_fOldHitPts);
	SAVE_FLOAT(m_fOldEnergy);
	SAVE_FLOAT(m_fOldArmor);
	SAVE_FLOAT(m_fOldAirLevel);
	SAVE_FLOAT(m_fAirLevel);
	SAVE_FLOAT(m_fOldModelAlpha);
	SAVE_BYTE(m_eState);
	SAVE_BOOL(m_b3rdPersonView);
	SAVE_DWORD(m_nSavedFlags);
	SAVE_BOOL(m_bGodMode);
	SAVE_BOOL(m_bAllowInput);
	SAVE_bool( m_bCinematicInvulnerability );
	SAVE_BYTE(m_ePPhysicsModel);
	SAVE_bool(m_bNewMission);

	SAVE_WORD(m_nClientChangeFlags);
	SAVE_VECTOR(m_vOldModelColor);

	SAVE_BYTE(m_nMotionStatus);
	SAVE_BYTE(m_nWeaponStatus);
	SAVE_BYTE(m_ePlayerLean);

	SAVE_FLOAT(m_fHiddenTimer);
	SAVE_FLOAT(m_fHideDuration);
	SAVE_BOOL(m_bHiding);
	SAVE_BOOL(m_bHidden);

	SAVE_HOBJECT(m_hCarriedObject);
	SAVE_bool(m_bCarryingAI);

	SAVE_HOBJECT( m_hPlayerLure ); 
	SAVE_BOOL( m_bForceDuck );

	SAVE_HOBJECT( m_hDeadBody );
	SAVE_TIME( m_fLastVehicleFootstepTime );

	SAVE_VECTOR( m_vLastGoodDropPos );
	SAVE_bool( m_bCanDropBody );

	SAVE_ROTATION( m_rFullPlayerRot );

	// [RP] 9/14/02 - Save off the velocity so we can zero out the players vel when they load
	//		and then reset the saved vel when they actually respawn in the world.

	g_pPhysicsLT->GetVelocity( m_hObject, &m_vSavedVelocity );
	SAVE_VECTOR( m_vSavedVelocity );

	// Save client data associated with this player...

	if (m_pClientSaveData)
	{
		SAVE_BOOL(LTTRUE);
		pMsg->WriteMessage(m_pClientSaveData);

		SetClientSaveData( NULL );
	}
	else
	{
		SAVE_BOOL(LTFALSE);
		// This should NEVER happen!
		_ASSERT(LTFALSE);
	}

	SAVE_bool( m_bCanBeRevived );

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_nSaveVersion = g_pVersionMgr->GetCurrentSaveVersion( );

	m_dwLastLoadFlags = dwLoadFlags;

	// Players will load the objectives to the server shell so they will 
	// properly carry across levels.  

	g_pGameServerShell->LoadObjectives( pMsg, dwLoadFlags );

	// Load the command mgrs' global vars here so we bring them across levels

	g_pCmdMgr->LoadGlobalVars( pMsg, LTFALSE );

	// Load animator

	m_Animator.Load(pMsg);

	// Make sure the player skills get loaded...

    m_Skills.Load(pMsg);
    m_Stats.ReadData(pMsg);

	// Load PlayerObj data...

	LOAD_HOBJECT(m_hVehicleModel);

	LOAD_CHARSTRING(m_sStartLevelCommand.GetBuffer( 1024 ), 1024 );
	m_sStartLevelCommand.ReleaseBuffer( );

	LOAD_FLOAT(m_fOldHitPts);
	LOAD_FLOAT(m_fOldEnergy);
	LOAD_FLOAT(m_fOldArmor);
	LOAD_FLOAT(m_fOldAirLevel);
	LOAD_FLOAT(m_fAirLevel);
	LOAD_FLOAT(m_fOldModelAlpha);
	LOAD_BYTE_CAST(m_eState, PlayerState);
	LOAD_BOOL(m_b3rdPersonView);
	LOAD_DWORD(m_nSavedFlags);
	LOAD_BOOL(m_bGodMode);
	LOAD_BOOL(m_bAllowInput);
	LOAD_bool( m_bCinematicInvulnerability );
	LOAD_BYTE_CAST(m_ePPhysicsModel, PlayerPhysicsModel);
	LOAD_bool( m_bNewMission );

    LOAD_WORD(m_nClientChangeFlags);
	LOAD_VECTOR(m_vOldModelColor);

	LOAD_BYTE(m_nMotionStatus);
	LOAD_BYTE(m_nWeaponStatus);
	LOAD_BYTE_CAST(m_ePlayerLean, CPPlayerLeanTypes);

	LOAD_FLOAT(m_fHiddenTimer);
	LOAD_FLOAT(m_fHideDuration);

	//don't load hidden and hiding directly, 
	//  because we want to force an update to be sent down to the client
	LTBOOL bHiding;
	LTBOOL bHidden;
	LOAD_BOOL(bHiding);
	LOAD_BOOL(bHidden);
	if (bHiding)
		StartHiding();
	if (bHidden)
		StartHidden();

	LOAD_HOBJECT(m_hCarriedObject);
	LOAD_bool(m_bCarryingAI);

	LOAD_HOBJECT( m_hPlayerLure );
	LOAD_BOOL( m_bForceDuck );

	LOAD_HOBJECT( m_hDeadBody );
	LOAD_TIME( m_fLastVehicleFootstepTime );

	LOAD_VECTOR( m_vLastGoodDropPos );
	LOAD_bool( m_bCanDropBody );

	LOAD_ROTATION( m_rFullPlayerRot );

	// [RP] 9/14/02 - Load the saved velocity and zero out the objects velocity.  We'll reset it
	//		to the saved velocity later.

	LOAD_VECTOR( m_vSavedVelocity );
	LTVector vZero(0.0f, 0.0f, 0.0f);
	g_pPhysicsLT->SetVelocity( m_hObject, &vZero );
	g_pPhysicsLT->SetAcceleration( m_hObject, &vZero );

	// Get rid of the gravity flag so the physics won't pull us down if the playr was jumping when saved...

	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, m_dwSavedObjectFlags );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_GRAVITY );
	
	// Load client data associated with this player...

	LTBOOL bClientSaveData;
	LOAD_BOOL(bClientSaveData);
	if (bClientSaveData)
	{
		SetClientSaveData( pMsg->ReadMessage() );
	}

	if( g_pVersionMgr->GetCurrentSaveVersion( ) < CVersionMgr::kSaveVersion__1_3 )
	{
		m_bCanBeRevived = false;

		if (m_hCarriedObject)
		{
			SetCarrying(CFX_CARRY_BODY);
		}
		else
		{
			SetCarrying(CFX_CARRY_NONE);
		}
	}
	else
	{
		LOAD_bool( m_bCanBeRevived );
	}

	//need to initialize this with the client id
	m_Score.Init( g_pLTServer->GetClientID( GetClient() ) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleGameRestore
//
//	PURPOSE:	Setup the object
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleGameRestore()
{
	if (!g_pWeaponMgr) return;


	// Make sure we are using the correct model/skin...

	ResetModel();


	// Let the client know what state we are in...

	ChangeState(m_eState);


	// Make sure the interface is accurate...

    m_bForceUpdateInterface = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::BuildKeepAlives
//
//	PURPOSE:	Add the objects that should be keep alive
//				between levels to this list.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::BuildKeepAlives(ObjectList* pList)
{
	if (!pList || !m_hObject) return;

	// Since we must be loading a level....Hide and make non-solid...(but
	// make sure we still update the client)...
	
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAGMASK_ALL);

	g_pPhysicsLT->SetVelocity(m_hObject, &(LTVector(0, 0, 0)) );

	{ ///////////////////////////////////////////////////////////////////////////

		// Clear any player/character data that we don't want to save
		// between levels.  NOTE:  This data will still be saved in normal
		// saved games

		m_fLastPainTime				= -(float)INT_MAX;
		m_fLastPainVolume			= 0.0f;

		m_LastFireInfo.Clear();

		m_pLastVolume				= LTNULL;
		m_pCurrentVolume			= LTNULL;
		m_vLastVolumePos            = LTVector(0,0,0);

		for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
		{
			HATTACHMENT hAttachment;
			HOBJECT hSpear = m_aSpears[iSpear].hObject;
			if ( hSpear )
			{
				if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSpear, &hAttachment) )
				{
					g_pLTServer->RemoveAttachment(hAttachment);
				}

				g_pLTServer->RemoveObject(hSpear);
				m_aSpears[iSpear].hObject = LTNULL;
			}
		}

	} ///////////////////////////////////////////////////////////////////////////

	// Build keep alives...
	AddToObjectList( pList );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::BuildCameraList
//
//	PURPOSE:	Build a list of all the camera in the level...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::BuildCameraList()
{

// I actually don't think the cameras even need to be added to
// this list anymore due to the changes to the client-side vis
// code (i.e., all objects staying around on the client after they
// have been created)...

	if (m_bCameraListBuilt) return;

	StartTimingCounter();

	// Add all the camera's in the world to the list...

    HOBJECT hObj    = g_pLTServer->GetNextObject(LTNULL);
    HCLASS  hCamera = g_pLTServer->GetClass("Camera");

	// Add all the active ones...

	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hCamera))
		{
            m_Cameras.AddTail(g_pLTServer->HandleToObject(hObj));
            g_pLTServer->CreateInterObjectLink(m_hObject, hObj);
		}

        hObj = g_pLTServer->GetNextObject(hObj);
	}

	// Add all the inactive ones...

    hObj = g_pLTServer->GetNextInactiveObject(LTNULL);
	while (hObj)
	{
        if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hCamera))
		{
            m_Cameras.AddTail(g_pLTServer->HandleToObject(hObj));
            g_pLTServer->CreateInterObjectLink(m_hObject, hObj);
		}

        hObj = g_pLTServer->GetNextInactiveObject(hObj);
	}

	EndTimingCounter("CPlayerObj::BuildCameraList()");

    m_bCameraListBuilt = LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ClientUpdate
//
//	PURPOSE:	Handle client update
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::ClientUpdate(ILTMessage_Read *pMsg)
{
    if (!g_pGameServerShell || !GetClient( )) return LTFALSE;

    LTBOOL bRet = LTTRUE;

    LTBOOL bOld3rdPersonView = m_b3rdPersonView;
	m_nClientChangeFlags = 0;

    m_nClientChangeFlags = pMsg->Readuint16();

	if (m_nClientChangeFlags & CLIENTUPDATE_PLAYERROT)
	{
		ASSERT( !( m_nClientChangeFlags & CLIENTUPDATE_ACCURATEPLAYERROT ) );
		ASSERT( !( m_nClientChangeFlags & CLIENTUPDATE_FULLPLAYERROT ) );

        LTRotation rRot;
        uint8 byteRotation = pMsg->Readuint8();
		UncompressRotationByte(byteRotation, &rRot);

		g_pLTServer->SetObjectRotation(m_hObject, &rRot);
		
		m_rFullPlayerRot.Identity();
	}
	else if ( m_nClientChangeFlags & CLIENTUPDATE_ACCURATEPLAYERROT )
	{
		ASSERT( !( m_nClientChangeFlags & CLIENTUPDATE_PLAYERROT ) );
		ASSERT( !( m_nClientChangeFlags & CLIENTUPDATE_FULLPLAYERROT ) );

		LTRESULT ltResult;
		LTRotation rRot;

		// get the compressed rotation
		uint16 wRotation = pMsg->Readuint16();

		// uncompress the rotation
		UncompressRotationShort( wRotation, &rRot );

		// set the object's rotation
		ltResult = g_pLTServer->SetObjectRotation( m_hObject, &rRot );
		ASSERT( LT_OK == ltResult );

		m_rFullPlayerRot.Identity();
	}
	else if( m_nClientChangeFlags & CLIENTUPDATE_FULLPLAYERROT )
	{
		ASSERT( !( m_nClientChangeFlags & CLIENTUPDATE_ACCURATEPLAYERROT ) );
		ASSERT( !( m_nClientChangeFlags & CLIENTUPDATE_PLAYERROT ) );

		// Read the rotation in and save it so we can use it later...

		m_rFullPlayerRot = pMsg->ReadCompLTRotation();

		// Set the object's rotation
		// We're setting the full rotation but FLAG_YROTATION will only send the Yaw...

		g_pLTServer->SetObjectRotation( m_hObject, &m_rFullPlayerRot );
	}

	// grab the pitch, if its there
	if ( m_nClientChangeFlags & ( CLIENTUPDATE_PLAYERROT | CLIENTUPDATE_ACCURATEPLAYERROT ) )
	{
		uint8 ucPitch = pMsg->Readuint8();

		// get the compressed pitch
		float fPitch;

		// uncompress the pitch
		UncompressAngleFromByte( ucPitch, &fPitch );

		// set the pitch (this fn will notify all the clients)
		SetPitch( fPitch );
	}
		
	if ( m_nClientChangeFlags & CLIENTUPDATE_CAMERAOFFSET )
	{
		// the camera offset has been sent

		// the offset comes up compressed
		TVector3< short > vCompressedCameraOffset;

		// read the vector
		vCompressedCameraOffset.x = pMsg->Readint16();
		vCompressedCameraOffset.y = pMsg->Readint16();
		vCompressedCameraOffset.z = pMsg->Readint16();

		// uncompress the vector
		UncompressOffset( &m_vClientCameraOffset, vCompressedCameraOffset, 100 );

		// record the time this was sent
		m_nClientCameraOffsetTimeReceivedMS = static_cast< int >( g_pLTServer->GetTime() * 1000.0f );
	}

	if (m_nClientChangeFlags & CLIENTUPDATE_3RDPERSON)
	{
        m_b3rdPersonView = (m_nClientChangeFlags & CLIENTUPDATE_3RDPERVAL) ? LTTRUE : LTFALSE;
	}
	if (m_nClientChangeFlags & CLIENTUPDATE_ALLOWINPUT)
	{
        m_bAllowInput = (LTBOOL)pMsg->Readuint8();
        bRet = LTFALSE;
	}

	// Read the control flags
	SetControlFlags(pMsg->Readuint32());

	// Only change the client flags in multiplayer...

	if (IsMultiplayerGame( ))
	{
		if (m_b3rdPersonView != bOld3rdPersonView)
		{
            uint32 dwFlags = g_pLTServer->GetClientInfoFlags(GetClient( ));

			if (m_b3rdPersonView)
			{
				// Make sure the object's rotation is sent (needed for 3rd person view)...

				dwFlags |= CIF_SENDCOBJROTATION;
			}
			else
			{
				dwFlags &= ~CIF_SENDCOBJROTATION;
			}

            g_pLTServer->SetClientInfoFlags(GetClient( ), dwFlags);
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateSpecialFX()
//
//	PURPOSE:	Update the client-side special fx
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateSpecialFX()
{
    uint32 dwUserFlags = 0;

	// See if we're under water...

	if (IsLiquid(m_eContainerCode))
	{
		dwUserFlags |= USRFLG_PLAYER_UNDERWATER;
	}


    // Update our "duck" usrflag.  This is used on the client to see if we
	// are *really* ducked...

	if ( m_Animator.GetLastMovement() == CAnimatorPlayer::eCrouching )
	{
		dwUserFlags |= USRFLG_PLAYER_DUCK;
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwUserFlags, USRFLG_PLAYER_UNDERWATER | USRFLG_PLAYER_DUCK);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ProcessCommandLure()
//
//	PURPOSE:	Process a lure command
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::ProcessCommandLure( const CParsedMsg &cMsg )
{
	// Check if they are cancelling the lure.
	if( cMsg.GetArgCount() < 2 )
	{
		ASSERT( !"CPlayerObj::ProcessCommandLure: No lure name specified." );
		return true;
	}

	const char* pObjName = cMsg.GetArg(1);
	if( !pObjName )
	{
		ASSERT( !"CPlayerObj::ProcessCommandLure: Invalid lure name specified." );
		return false;
	}

	// Find the lure objects.
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(const_cast<char *>(pObjName), objArray);
	int numObjects = objArray.NumObjects();

	// Check if the lure is missing.
    if( !numObjects )
	{
		ASSERT( !"CPlayerObj::ProcessCommandLure: Lure object not found." );
		return false;
	}

	// Get the first object.
    HOBJECT hLure = objArray.GetObject(0);
	if( !hLure )
	{
		ASSERT( !"CPlayerObj::ProcessCommandLure: Corrupt lure object." );
		return false;
	}

	// Convert the hobject to a playerlure.
    PlayerLure* pPlayerLure = dynamic_cast< PlayerLure* >( g_pLTServer->HandleToObject( hLure ));
	if( !pPlayerLure )
	{
		ASSERT( !"CPlayerObj::ProcessCommandLure: Lure specified is not a PlayerLure." );
		return false;
	}

	return FollowLure( *pPlayerLure );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateConsoleVars()
//
//	PURPOSE:	Check console commands that pertain to the player
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateConsoleVars()
{
	// Check var trackers...

	SetLeashLen(m_LeashLenTrack.GetFloat());
	SetLeashSpring(m_LeashSpringTrack.GetFloat());
	
	// Clamp the spring rate to [0.0,1.0]
	float fSpringRate = m_LeashSpringRateTrack.GetFloat();
	fSpringRate = LTCLAMP(fSpringRate, 0.0f, 1.0f);
	SetLeashSpringRate(fSpringRate);

	SetMoveMul(m_MoveVelMulTrack.GetFloat());
	SetJumpVelMul(m_JumpVelMulTrack.GetFloat());
	SetLadderVel(m_LadderVelTrack.GetFloat());
	SetSwimVel(m_SwimVelTrack.GetFloat());
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateClientViewPos()
//
//	PURPOSE:	Update where the client's view is
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateClientViewPos()
{
	if (!GetClient( )) return;

	if (Camera::IsActive())
	{
		// If we're in a cinematic don't allow the player to be damaged...
        m_damage.SetCanDamage(LTFALSE);
		m_bCinematicInvulnerability = true;

		// Make sure we aren't moving...

		if (!m_bAllowInput)
		{
			// Don't cancel Y Vel/Accel so we can be moved to the ground...

            LTVector vVec;
			g_pPhysicsLT->GetVelocity(m_hObject, &vVec);
			vVec.x = vVec.z = 0.0f;
			g_pPhysicsLT->SetVelocity(m_hObject, &vVec);
		}
	}
	else
	{
		// Check if we were set to invulnerable by a cinematic camera.
		if( m_bCinematicInvulnerability )
		{
	        m_damage.SetCanDamage(LTTRUE);
			m_bCinematicInvulnerability = false;
		}

        LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
        g_pLTServer->SetClientViewPos(GetClient( ), &vPos);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SendStartLevelCommand()
//
//	PURPOSE:	Trigger any beginning of level events...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SendStartLevelCommand()
{
	if (!m_sStartLevelCommand.IsEmpty( ))
	{
		if( g_pCmdMgr->IsValidCmd( m_sStartLevelCommand ) )
		{
			g_pCmdMgr->Process( m_sStartLevelCommand, m_hObject, m_hObject );
		}

		// Okay, this is a one-time only trigger, so remove the messages...
        m_sStartLevelCommand.Empty( );
	}

	m_bSendStartLevelCommand = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandlePlayerPositionMessage()
//
//	PURPOSE:	Process new position message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandlePlayerPositionMessage(ILTMessage_Read *pMsg)
{
    LTVector newPos, curPos, curVel, newVel;
    uint8 moveCode;
    LTBOOL bOnGround;

    moveCode = pMsg->Readuint8();

    newPos = pMsg->ReadLTVector();

	newVel = pMsg->ReadLTVector();

    bOnGround = pMsg->Readuint8();
    m_eStandingOnSurface = (SurfaceType) pMsg->Readuint8();

    HPOLY hStandingOnPoly;
	pMsg->ReadType(&hStandingOnPoly);

	if (moveCode == m_ClientMoveCode )
	{
		SetOnGround(bOnGround);

		g_pLTServer->GetObjectPos(m_hObject, &curPos);

		float fMinMove = 0.1f;
		if (IsMultiplayerGame( ))
		{
			// For client-side prediction...
			g_pPhysicsLT->SetVelocity(m_hObject, &newVel);

			fMinMove = m_fLeashLen;
		}

			
        // If we are walking or crouching we can remain hidden...
		
		if( (m_Animator.GetLastMovement() != CAnimatorPlayer::eWalking) &&
			(m_Animator.GetLastMovement() != CAnimatorPlayer::eCrouching) )
		{
			if (!curPos.NearlyEquals(newPos, fMinMove))
			{
				// Clear out our hidden state
				EndHidden();
			}
		}

		// Just teleport the player for singleplayer game.  Use leash for mp game.
		if (!g_pGameServerShell->IsPaused( )) 
		{
			if( !IsMultiplayerGame() )
			{
				if (!curPos.NearlyEquals(newPos, 0.1f))
				{
					// Move it first so we get collisions and stuff
					g_pLTServer->MoveObject(m_hObject, &newPos);

					LTVector vResultPos;
					g_pLTServer->GetObjectPos( m_hObject, &vResultPos );

					if( vResultPos != newPos )
					{
						// Then just teleport it there in case it didn't make it for some reason
						g_pLTServer->SetObjectPos(m_hObject, &newPos);
					}
				}
			}
			else
			{
				// Change the gravity on this object..
				g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, (newVel.y*newVel.y > 0.01f) ? FLAG_GRAVITY : 0, FLAG_GRAVITY);
				// Turn on the leash
				m_bUseLeash = LTTRUE;
			}
		}
	}

	// Remember where the client last said we were
	m_vLastClientPos = newPos;


	// See if we're standing on a Breakable object...

	if (hStandingOnPoly != INVALID_HPOLY)
	{
        HOBJECT hObj = LTNULL;
        if (g_pLTServer->GetHPolyObject(hStandingOnPoly, hObj) == LT_OK)
		{
			if (hObj && IsKindOf(hObj, "Breakable"))
			{
                Breakable* pBreak = (Breakable*)g_pLTServer->HandleToObject(hObj);
				if (pBreak)
				{
					pBreak->Break(m_hObject);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TeleportClientToServerPos()
//
//	PURPOSE:	This sends a message to the client telling it to move to
//				our position.  Used when loading games and respawning.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::TeleportClientToServerPos()
{
	if (!GetClient( )) return;

    LTVector myPos;
	g_pLTServer->GetObjectPos(m_hObject, &myPos);

	// Tell the player about the new move code.
	++m_ClientMoveCode;
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENT_PLAYER_UPDATE);
	cMsg.Writeuint16(PSTATE_POSITION);
	cMsg.Writeuint8(m_ClientMoveCode);
	// Need to send the position to the client in the message because
	// they client may not get our position normally for a few frames since
	// our position is unguaranteed.
	cMsg.WriteLTVector(myPos);

	// [RP] - Sending the dims because the client will begin using the dims from
	// the last game which may or may not be the same as the newly loaded dims.

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);
	cMsg.WriteLTVector(vDims);

	g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponFireMessage
//
//	PURPOSE:	Handle player firing weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponFireMessage(ILTMessage_Read *pMsg)
{
	if (!g_pWeaponMgr || !GetClient( )) return;

	// If we aren't dead, and we aren't in spectator mode, let us fire.

	if (m_damage.IsDead() || m_bSpectatorMode)
	{
		return;
	}

	if ( !m_pPlayerAttachments )
	{
		return;
	}

	// holding value for bytes
	uint8 cTemp;

	// get the message type (vector, projectile, special, etc)
	// NOTE: currently VECTOR and PROJECTILE types are handled the same
	uint8 nFireType = pMsg->Readuint8();

	// Get the weapon id
	uint8 nWeaponId = pMsg->Readuint8();

	// get the ammo id
	uint8 nAmmoId = pMsg->Readuint8();

	// get the object weapon
	CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon();
	
	// Make sure everything's kosher...
	if (!pWeapon || nWeaponId != pWeapon->GetId())
	{
		g_pLTServer->CPrint("Client Weapon (%d) != Server Weapon (%d)",nWeaponId, pWeapon ? pWeapon->GetId() : -1 );
		_ASSERT(!"ERROR - Client WeaponId doesn't match server WeaponId");
		return;
	}
	
	if (nAmmoId != pWeapon->GetAmmoId())
	{
		g_pLTServer->CPrint("Client Ammo (%d) != Server Ammo (%d)",nAmmoId,pWeapon->GetAmmoId());
		_ASSERT(!"ERROR - Client AmmoId doesn't match server AmmoId");
		return;
	}

	// get the weapon and ammo data
	WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon(nWeaponId);
	AMMO const*pAmmoData = g_pWeaponMgr->GetAmmo(nAmmoId);
	if (!pWeaponData || !pAmmoData) return;


	// Set the ammo type in the weapon...

	pWeapon->SetAmmoId(nAmmoId);


	//
	// create the fire info and fill it with info from the message
	//
	WeaponFireInfo weaponFireInfo;

	// type of weapon
	uint8 nWeaponType = nFireType;

	// who fired the shot
	weaponFireInfo.hFiredFrom = m_hObject;

	// get the flash position
	weaponFireInfo.vFlashPos = pMsg->ReadLTVector();

	// get the fire position
	weaponFireInfo.vFirePos = pMsg->ReadLTVector();

	// get the direction of travel
	weaponFireInfo.vPath = pMsg->ReadLTVector();

	// get the random number seed
	weaponFireInfo.nSeed = pMsg->Readuint8();

	// NOTE: alt firing is BROKEN
	/*
	// determine if we are alt firing
	cTemp = pMsg->Readuint8();
	bool bAltFire = ( 0 < cTemp );
	*/
	bool bAltFire = false;
	weaponFireInfo.bAltFire = bAltFire;

	// get the perturb (note: R and U (up&down and side-to-side) are the same)
	cTemp = pMsg->Readuint8();
	weaponFireInfo.fPerturbR       = /*same as next line*/
	weaponFireInfo.fPerturbU       = static_cast< LTFLOAT >( cTemp ) / 255.0f;

	// get the timestamp when the shot was fired
	weaponFireInfo.nFireTimestamp = pMsg->Readuint32();

	// extract the extra (weapon specific) data from the message
	GetExtraFireMessageInfo( nWeaponType, pMsg, &weaponFireInfo );


	// Read out the object impact info
	weaponFireInfo.hClientObjImpact = INVALID_HOBJECT;
	if ( pMsg->Readbool() )
	{
		weaponFireInfo.vClientObjImpactPos = weaponFireInfo.vFirePos + weaponFireInfo.vPath * pMsg->Readfloat();
		weaponFireInfo.hClientObjImpact = pMsg->ReadObject();
	}

	// If this is a gadget, tell the activated object it was activated...

	bool bChangedAmmoType = false;

	if (pAmmoData->eType == GADGET)
	{
 		if (weaponFireInfo.hClientObjImpact)
 		{
 			char buf[100];
 			sprintf(buf, "Gadget %d", pAmmoData->nId);
 			SendTriggerMsgToObject(this, weaponFireInfo.hClientObjImpact, LTFALSE, buf);
 		}
 		else
 		{
 			// [KLS 8/13/02] If we're using the welder, change the ammo to welder 
 			// burn so we can damage other objects...

 			if (pAmmoData->eInstDamageType == DT_GADGET_WELDER)
 			{
 				bChangedAmmoType = true;
				AMMO const* pTempAmmoData = g_pWeaponMgr->GetAmmo("WelderBurn");
 				if (pTempAmmoData)
 				{
 					pWeapon->SetAmmoId(pTempAmmoData->nId);
 				}
 			}
 			else
 			{
 				return;
 			}
 		}
	}


	// If we're in 3rd person view, use the hand held weapon fire pos.

	if (m_b3rdPersonView)
	{
		weaponFireInfo.vFirePos  = HandHeldWeaponFirePos(pWeapon);
		weaponFireInfo.vFlashPos = weaponFireInfo.vFirePos;
	}


	pWeapon->Fire(weaponFireInfo);

 	// [KLS 8/13/02] Change ammo type back to correct ammo type...
 
 	if (bChangedAmmoType)
 	{
 		pWeapon->SetAmmoId(nAmmoId);
 	}


	ClearHiding();

	// Update number of shots fired...

	if (IsAccuracyType(pAmmoData->eInstDamageType))
	{
		m_Stats.dwNumShotsFired += pWeaponData->nVectorsPerRound;
	}



	// If this is a projectile weapon, tell clients to play the fire sound (vector
	// weapons do this in the weapon fx message)...

	if (pAmmoData->eType == PROJECTILE)
	{
        uint8 nClientID = (uint8) g_pLTServer->GetClientID(GetClient( ));

		CAutoMessage cSoundMsg;
		cSoundMsg.Writeuint8(SFX_PLAYERSOUND_ID);
		cSoundMsg.Writeuint8(bAltFire ? PSI_ALT_FIRE : PSI_FIRE);
        cSoundMsg.Writeuint8(nWeaponId);
        cSoundMsg.Writeuint8(nClientID);
		cSoundMsg.WriteCompPos(weaponFireInfo.vFirePos);
		g_pLTServer->SendSFXMessage(cSoundMsg.Read(), weaponFireInfo.vFirePos, 0);
	}


	// Reset the weapon state to none, firing takes priority

	m_nWeaponStatus = WS_NONE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponSoundMessage
//
//	PURPOSE:	Handle weapon sound message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponSoundMessage(ILTMessage_Read *pMsg)
{
	if (!pMsg || !GetClient( )) return;

    uint8 nType     = pMsg->Readuint8();
    uint8 nWeaponId = pMsg->Readuint8();
    uint8 nId       = pMsg->Readuint8();
	LTVector vPos	= pMsg->ReadLTVector();

    uint8 nClientID = (uint8) g_pLTServer->GetClientID(GetClient( ));

	CAutoMessage cSoundMsg;
	cSoundMsg.Writeuint8(SFX_PLAYERSOUND_ID);
    cSoundMsg.Writeuint8(nType);
    cSoundMsg.Writeuint8(nWeaponId);
    cSoundMsg.Writeuint8(nClientID);
	cSoundMsg.WriteCompPos(vPos);
	g_pLTServer->SendSFXMessage(cSoundMsg.Read(), vPos, 0);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::HandleWeaponSoundLoopMessage
//
//  PURPOSE:	Tell all the clients to handle the looping sound message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponSoundLoopMessage(ILTMessage_Read *pMsg)
{
	if( !pMsg || !GetClient( ) ) return;

	uint8	nType		= pMsg->Readuint8();
	uint8	nWeaponId	= pMsg->Readuint8();

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( CFX_WEAPON_SOUND_LOOP_MSG );
	cMsg.Writeuint8( nType );
	cMsg.Writeuint8( nWeaponId );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, 0 );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleActivateMessage
//
//	PURPOSE:	Handle player activation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleActivateMessage(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	// Cache our activation data for called methods to use and clear when we're done...
	
	m_ActivationData.Read(pMsg);
	DoActivate(&m_ActivationData);
	m_ActivationData.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleTeleportMsg
//
//	PURPOSE:	Handle player teleport
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleTeleportMsg(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	LTVector vPos = pMsg->ReadLTVector();
	LTVector vPitchYawRoll = pMsg->ReadLTVector();

	Teleport(vPos, vPitchYawRoll);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleClientMsg
//
//	PURPOSE:	Handle message from our client (CMoveMgr)
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleClientMsg(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	uint8 byMessage = pMsg->Readuint8();

    switch (byMessage)
	{
		case CP_FLASHLIGHT:
		{
            uint8 byFlashlight = pMsg->Readuint8();

			if ( byFlashlight == FL_ON )
			{
 				CreateFlashLight();
			}
			else if ( byFlashlight == FL_OFF )
			{
                DestroyFlashLight();
			}
		}
		break;
		
		case CP_PLAYER_LEAN:
		{
			CPPlayerLeanTypes eLean = ( CPPlayerLeanTypes )pMsg->Readuint8();
			LTVector	vPos = pMsg->ReadLTVector();

			SetLean( eLean, vPos );
		}
		break;

		case CP_MOTION_STATUS :
		{
            m_nMotionStatus = pMsg->Readuint8();

			if (MS_JUMPED == m_nMotionStatus)
			{
				LTBOOL bPlaySound = LTFALSE;
				if (!IsLiquid(m_eContainerCode))
				{
					bPlaySound = (m_bBodyInLiquid ? m_bOnGround : (m_eStandingOnSurface != ST_INVISIBLE));
				}

				if (bPlaySound)
				{
					uint8 nClientID = (uint8) g_pLTServer->GetClientID(GetClient( ));

					LTVector vPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPos);

					CAutoMessage cSoundMsg;
					cSoundMsg.Writeuint8(SFX_PLAYERSOUND_ID);
					cSoundMsg.Writeuint8(PSI_JUMP);
					cSoundMsg.Writeuint8(0);
					cSoundMsg.Writeuint8(nClientID);
					cSoundMsg.WriteCompPos(vPos);
					g_pLTServer->SendSFXMessage(cSoundMsg.Read(), vPos, 0);
				}
			}
		}
		break;

		case CP_WEAPON_STATUS :
		{
            m_nWeaponStatus = pMsg->Readuint8();
		}
		break;

		case CP_DAMAGE_VEHICLE_IMPACT :
		case CP_DAMAGE :
		{
			DamageStruct damage;

			damage.eType	= (DamageType) pMsg->Readuint8();
	        damage.fDamage  = pMsg->Readfloat();
			damage.vDir		= pMsg->ReadLTVector();

			// BL 10/30/00 - if this is vehicle impact damage, read out the position of the impact
			LTVector vReportedPos;
			if ( byMessage == CP_DAMAGE_VEHICLE_IMPACT )
			{
				vReportedPos = pMsg->ReadLTVector();
			}

			if (pMsg->Readuint8())
			{
				damage.fDuration = pMsg->Readfloat();
			}

			HOBJECT hObj = pMsg->ReadObject();

			if (hObj)
			{
				// BL 10/30/00 - if this is vehicle impact damage, make it fall off by the distance from the impact
				if (byMessage == CP_DAMAGE_VEHICLE_IMPACT)
				{
					if (hObj != m_hObject)
					{
						LTVector vPos;
						g_pLTServer->GetObjectPos(hObj, &vPos);

						LTFLOAT fDistance = vPos.Dist(vReportedPos);
						LTFLOAT fDamageModifier = 1.0f;

						if(!s_vtVehicleImpactDistMin.IsInitted())
							s_vtVehicleImpactDistMin.Init(g_pLTServer, "VehicleImpactDistMin", NULL, 100.0f);
						if(!s_vtVehicleImpactDistMax.IsInitted())
							s_vtVehicleImpactDistMax.Init(g_pLTServer, "VehicleImpactDistMax", NULL, 300.0f);

						LTFLOAT fMinDistance = s_vtVehicleImpactDistMin.GetFloat(100.0f);
						LTFLOAT fMaxDistance = s_vtVehicleImpactDistMax.GetFloat(300.0f);

						if ( fDistance <= fMinDistance )
						{
							fDamageModifier = 1.0f;
						}
						else if ( fDistance > fMinDistance && fDistance < fMaxDistance )
						{
							fDamageModifier = 1.0f - (fDistance - fMinDistance)/(fMaxDistance - fMinDistance);
						}
						else if ( fDistance >= fMaxDistance )
						{
							// Don't do the damage

							return;
						}

						damage.fDamage *= fDamageModifier;
					}
				}

				damage.hDamager = m_hObject;
				damage.DoDamage(this, hObj);
			}
		}
		break;

		case CP_PHYSICSMODEL :
		{
            PlayerPhysicsModel eModel = (PlayerPhysicsModel)pMsg->Readuint8();
			SetPhysicsModel(eModel, LTFALSE);
		}
		break;

		case CP_DEFLECT :
		{
			float fDuration = pMsg->Readfloat( );
			SetDeflecting( fDuration );
		}
		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RideVehicle
//
//	PURPOSE:	Handle vehicle activation
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RideVehicle(PlayerVehicle* pVehicle)
{
	if (!pVehicle) return;

	// Can't ride vehicles when underwater...

	if (IsLiquid(m_eContainerCode))
	{
		g_pLTServer->CPrint("Can't ride vehicles underwater!!!");
		return;
	}

	if (IsCarryingHeavyObject())
	{
		g_pLTServer->CPrint("Can't ride vehicles while carrying a something heavy");
		return;
	}


	PlayerPhysicsModel eModel = pVehicle->GetPhysicsModel();

	// Set our vehicle model.  SetVehiclePhysicsModel() will set the
	// appropriate links and attach the model to us...

	if (eModel != PPM_NORMAL)
	{
		m_hVehicleModel = pVehicle->m_hObject;
	}

	SetPhysicsModel(eModel);


	if (m_hVehicleModel)
	{
		pVehicle->SetRidden(true);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::FollowLure
//
//	PURPOSE:	Sets us to follow a lure.
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::FollowLure( PlayerLure& playerLure )
{
	// Record the lure to follow.
	m_hPlayerLure = playerLure.m_hObject;

	// SetPhysicsModel doesn't allow setting model to the same model.  Trick
	// it by changing the model here.
	if( m_ePPhysicsModel == PPM_LURE )
		m_ePPhysicsModel = PPM_NORMAL;
	SetPhysicsModel( PPM_LURE );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::StopFollowingLure
//
//	PURPOSE:	No longer follow the lure.
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::StopFollowingLure( )
{
	// Make sure we have a lure.
	if( !m_hPlayerLure )
	{
		ASSERT( !"CPlayerObj::StopFollowingLure: Lure missing." );
		return false;
	}

	// Clear the lure object.
	m_hPlayerLure = NULL;

	// Go to normal physics model.
	SetPhysicsModel( PPM_NORMAL );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PushCharacter
//
//	PURPOSE:	Push character from some position
//
// ----------------------------------------------------------------------- //

void CPlayerObj::PushCharacter(const LTVector &vPos, LTFLOAT fRadius, LTFLOAT fStartDelay, LTFLOAT fDuration, LTFLOAT fStrength)
{
	if( GetClient( ) )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_ADD_PUSHER);
		cMsg.WriteCompLTVector(vPos);
		cMsg.Writefloat(fRadius);
		cMsg.Writefloat(fStartDelay);
		cMsg.Writefloat(fDuration);
		cMsg.Writefloat(fStrength);
		g_pLTServer->SendToClient(cMsg.Read(), GetClient( ), 0);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TeleFragObjects
//
//	PURPOSE:	TeleFrag any player object's at this position
//
// ----------------------------------------------------------------------- //

void CPlayerObj::TeleFragObjects(LTVector & vPos)
{
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
    g_pLTServer->FindNamedObjects(DEFAULT_PLAYERNAME, objArray);
	int numObjects = objArray.NumObjects();

	if (!numObjects) return;

	for (int i = 0; i < numObjects; i++)
	{
		HOBJECT hObject = objArray.GetObject(i);

		if (hObject != m_hObject)
		{
            LTVector vObjPos, vDims;
			g_pLTServer->GetObjectPos(hObject, &vObjPos);
			g_pPhysicsLT->GetObjectDims(hObject, &vDims);

			// Increase the size of the dims to account for the players
			// dims overlapping...

			vDims *= 2.0f;

			if (vObjPos.x - vDims.x < vPos.x && vPos.x < vObjPos.x + vDims.x &&
				vObjPos.y - vDims.y < vPos.y && vPos.y < vObjPos.y + vDims.y &&
				vObjPos.z - vDims.z < vPos.z && vPos.z < vObjPos.z + vDims.z)
			{
				DamageStruct damage;

				damage.eType	= DT_ELECTROCUTE;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = m_hObject;
				damage.vDir.Init(0, 1, 0);

				damage.DoDamage(this, hObject);
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CPlayerObj::CreateAttachments()
{
	if (!m_pAttachments)
	{
		m_pAttachments = m_pPlayerAttachments = static_cast<CPlayerAttachments*>(CAttachments::Create(ATTACHMENTS_TYPE_PLAYER));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TransferAttachments
//
//	PURPOSE:	Transfer our attachments aggregate (i.e., clear our
//				attachments aggregate, but don't remove it)
//
// ----------------------------------------------------------------------- //

CAttachments* CPlayerObj::TransferAttachments( bool bRemove )
{
	if( bRemove )
	{
		m_pPlayerAttachments = LTNULL;
	}

	return CCharacter::TransferAttachments( bRemove );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TransferWeapons
//
//	PURPOSE:	Transfers weapons to body.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::TransferWeapons(Body *pBody, bool bRemove )
{
	// Check if we are transferring weapons to a revivable body.
	if( IsRevivePlayerGameType( ) && m_bCanBeRevived )
	{
		CCharacter::TransferWeapons(pBody, bRemove );
		return;
	}

	// Get rid of powerups we already had on the body.
	pBody->RemovePowerupObjects( );

	//for deathmatch transefer all the weapons, not just the one in hand
	if (m_pPlayerAttachments)
	{
		// get first and last weapon indices
		int nFirstWeaponCommandId =g_pWeaponMgr->GetFirstWeaponCommandId();
		int nLastWeaponCommandId = g_pWeaponMgr->GetLastWeaponCommandId();
		char szSpawn[1024] = "";

		PickupList PUList;
		PickupList::iterator iter;

		// note: add 1 because the first and last are inclusive
		int nMaxWeapons = nLastWeaponCommandId - nFirstWeaponCommandId + 1;

		//step through each potential player weapon...
		int i, nCommandId;
		for (i = 0, nCommandId = nFirstWeaponCommandId;
			  i < nMaxWeapons;
			  ++i, ++nCommandId )
		{
			// get the weapon data 
			uint8 nWeaponId = g_pWeaponMgr->GetWeaponId(nCommandId);
			CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon(nWeaponId);

			//if the player has the weapon...
			if (pWeapon && pWeapon->Have())
			{

				WEAPON const *pWeaponStruct = g_pWeaponMgr->GetWeapon(nWeaponId);
				AMMO const *pAmmo     = g_pWeaponMgr->GetAmmo(pWeaponStruct->nDefaultAmmoId);		

				//if we have valid data for the weapon and ammo, create one...
				if (pAmmo && !pAmmo->bServerRestricted && pWeaponStruct && !pWeaponStruct->bServerRestricted && 
					pWeaponStruct->szHHModel[0])
				{
					sprintf(szSpawn, "WeaponItem Gravity 0;AmmoAmount 0;WeaponType %s;AmmoType %s;MPRespawn 0, DMTouchPickup 0",
						pWeaponStruct->szName, pAmmo->szName);

					BaseClass* pObj = SpawnObject(szSpawn, LTVector(-10000,-10000,-10000), LTRotation());

					//if we succeeded in creating a WeaponItem, hide it and add it to the body...
					if ( pObj && pObj->m_hObject )
					{

						g_pCommonLT->SetObjectFlags( pObj->m_hObject, OFT_Flags, 0, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY );

						PickupItem *pItem = dynamic_cast<PickupItem *>(g_pLTServer->HandleToObject(pObj->m_hObject ));
						int n = 0;
						int sz = PUList.size();
						if (sz > 1)
							n = GetRandom(0,sz-1);
						iter = PUList.begin();
						for (int i = 0; i < n; i++)
							iter++;

						PUList.insert(iter, pItem);
					
						//pBody->AddPickupItem(pObj->m_hObject);
					}
				}

			}
		}

  		//step through each potential player ammo...
		for (i = 0; i < g_pWeaponMgr->GetNumAmmoIds(); ++i )
		{
			// get the ammo data 
			AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(i);
			int nAmmoCount = m_pPlayerAttachments->GetAmmoCount(i);

			if (pAmmo && !pAmmo->bServerRestricted && nAmmoCount > 0)
			{

				sprintf(szSpawn, "AmmoBox MPRespawn 0;AmmoType1 %s;AmmoCount1 %d", pAmmo->szName, nAmmoCount);
				BaseClass* pObj = SpawnObject(szSpawn, LTVector(-10000,-10000,-10000), LTRotation());

				//if we succeeded in creating a WeaponItem, hide it and add it to the body...
				if ( pObj && pObj->m_hObject )
				{

					g_pCommonLT->SetObjectFlags( pObj->m_hObject, OFT_Flags, 0, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY );

					PickupItem *pItem = dynamic_cast<PickupItem *>(g_pLTServer->HandleToObject(pObj->m_hObject ));
					int n = 0;
					int sz = PUList.size();
					if (sz > 1)
						n = GetRandom(0,sz-1);
					
					iter = PUList.begin();
					for (int i = 0; i < n; i++)
						iter++;

					PUList.insert(iter, pItem);

				
					//pBody->AddPickupItem(pObj->m_hObject);
				}

			}
		}

		iter = PUList.begin();
		while (iter != PUList.end())
		{
			pBody->AddPickupItem((*iter)->m_hObject);
			iter++;
		}

	}

	RemoveWeapons();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetPhysicsModel
//
//	PURPOSE:	Set the physics model
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetPhysicsModel(PlayerPhysicsModel eModel, LTBOOL bUpdateClient)
{
	if (m_ePPhysicsModel == eModel) return;

	if (bUpdateClient)
	{
		m_PStateChangeFlags |= PSTATE_PHYSICS_MODEL;
	}

	switch (eModel)
	{
		case PPM_SNOWMOBILE:
		case PPM_LIGHTCYCLE:
			SetVehiclePhysicsModel(eModel);
		break;

		default :
		case PPM_NORMAL :
			SetNormalPhysicsModel();
		break;
	}

	m_ePPhysicsModel = eModel;


    // Set our usr flags as necessary...

    uint32 dwUserFlags = 0;

	if (m_ePPhysicsModel == PPM_SNOWMOBILE)
	{
		dwUserFlags |= USRFLG_PLAYER_SNOWMOBILE;
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwUserFlags, USRFLG_PLAYER_SNOWMOBILE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetVehiclePhysicsModel
//
//	PURPOSE:	Set the vehicle physics model
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetVehiclePhysicsModel(PlayerPhysicsModel eModel)
{
	// Create the vehicle model...

	if (!m_hVehicleModel)
	{
		// Use the objects actual rotation since the full player rotation may 
		// not be accurate untill you are actually on the vehicle...
		
        LTVector vPos;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);
		
		LTRotation rPlayerRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rPlayerRot);

		LTRotation rRot = m_rFullPlayerRot.IsIdentity() ? rPlayerRot : m_rFullPlayerRot;

		char buff[256];
		char* pPropName = GetPropertyNameFromPlayerPhysicsModel(eModel);
		sprintf(buff, "PlayerVehicle VehicleType %s;Gravity 1", pPropName);

		BaseClass* pModel = SpawnObject(buff, vPos, rRot);
		if (!pModel) return;

		m_hVehicleModel = pModel->m_hObject;
		if (!m_hVehicleModel) return;
	}

	// Attach the vehicle model to us...

    LTVector vOffset(0, 0, 0);
    LTRotation rOffset;

    const char* pSocket = "Snowmobile";

	HATTACHMENT hAttachment;
    if( LT_OK != g_pLTServer->CreateAttachment(m_hObject, m_hVehicleModel, (char *)pSocket,
												&vOffset, &rOffset, &hAttachment) )
	{
		ASSERT( !"CreateAttachment() failed." );
		g_pLTServer->CPrint( "Failed to attach Snowmobile.  Check the socket on the player model!" );		
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetNormalPhysicsModel
//
//	PURPOSE:	Set the normal physics model
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetNormalPhysicsModel()
{
	// Remove any vehicle models...

	RemoveVehicleModel();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RemoveVehicleModel
//
//	PURPOSE:	Remove our vehicle attachment
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RemoveVehicleModel()
{
	if (!m_hVehicleModel) return;

	HATTACHMENT hAttachment;
    if (g_pLTServer->FindAttachment(m_hObject, m_hVehicleModel, &hAttachment) == LT_OK)
	{
        g_pLTServer->RemoveAttachment(hAttachment);

	}

	// Respawn the vehicle...

    PlayerVehicle* pVehicle = (PlayerVehicle*)g_pLTServer->HandleToObject(m_hVehicleModel);
	if (pVehicle)
	{
        LTVector vVec;
		g_pLTServer->GetObjectPos(m_hObject, &vVec);
		g_pLTServer->SetObjectPos(m_hVehicleModel, &vVec);

		LTRotation rPlayerRot;
		g_pLTServer->GetObjectRotation( m_hObject, &rPlayerRot );

		LTRotation rRot = m_rFullPlayerRot.IsIdentity() ? rPlayerRot : m_rFullPlayerRot;
		g_pLTServer->SetObjectRotation( m_hVehicleModel, &rRot );

        // g_pLTServer->GetVelocity(m_hObject, &vVec);

		vVec.Init(0, 0, 0);
		g_pPhysicsLT->SetVelocity(m_hVehicleModel, &vVec);

		g_pPhysicsLT->GetObjectDims(m_hObject, &vVec);
		g_pPhysicsLT->SetObjectDims(m_hVehicleModel, &vVec, 0);

		pVehicle->SetRidden(false);
        
	}

    m_hVehicleModel = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HasDangerousWeapon
//
//	PURPOSE:	Determine if the weapon we are holding is dangerous
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::HasDangerousWeapon()
{
    CWeapon* pWeapon = m_pPlayerAttachments ? m_pPlayerAttachments->GetWeapon() : LTNULL;
	if (!pWeapon)
	{
        return LTFALSE;
	}
	else
	{
		WEAPON const *pW = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
		if (!pW) return LTFALSE;

		return pW->bLooksDangerous;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HasMeleeWeapon
//
//	PURPOSE:	Determine if the weapon we are holding is a melee weapon
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerObj::HasMeleeWeapon()
{
    CWeapon* pWeapon = m_pPlayerAttachments ? m_pPlayerAttachments->GetWeapon() : LTNULL;
	if (!pWeapon)
	{
        return LTFALSE;
	}
	else
	{
		WEAPON const *pW = g_pWeaponMgr->GetWeapon(pWeapon->GetId());
		if (!pW) return LTFALSE;

		return (pW->nAniType == CAnimatorPlayer::eMelee);
	}

    return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetFootstepVolume
//
//	PURPOSE:	Determines our footstep volume
//
// ----------------------------------------------------------------------- //

LTFLOAT	CPlayerObj::GetFootstepVolume()
{
	switch (m_Animator.GetLastMovement())
	{
		case CAnimatorPlayer::eWalking:
		{
			return g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_WALKVOLUME);
		}
		break;

		case CAnimatorPlayer::eJumping:
		case CAnimatorPlayer::eRunning:
		{
			return g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_RUNVOLUME);
		}
		break;

		case CAnimatorPlayer::eCrouching:
		{
			return g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_CROUCHVOLUME);
		}
		break;

		default:
		{
			return g_pServerButeMgr->GetPlayerAttributeFloat(PLAYER_BUTE_WALKVOLUME);
		}
		break;
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SendIDToClients()
//
//	PURPOSE:	Send our client id to clients
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SendIDToClients()
{
	// Update clients with new info...
	uint8 nClientID = (uint8) g_pLTServer->GetClientID(GetClient( ));

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(CFX_CLIENTID_MSG);
	cMsg.Writeuint8(nClientID);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetChatting()
//
//	PURPOSE:	handle player chat mode
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetChatting(LTBOOL bChatting)
{
	m_bChatting = bChatting;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(CFX_CHAT_MSG);
	cMsg.Writeuint8(bChatting);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, 0);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ComputeDamageModifier
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //


LTFLOAT CPlayerObj::ComputeDamageModifier(ModelNode eModelNode)
{

	if( IsCoopMultiplayerGameType() )
		return 1.0f;

    LTFLOAT fModifier = CCharacter::ComputeDamageModifier(eModelNode);

	float fMaxModifier = GetConsoleFloat( "MaxDamageModifier", 3.0f );
	fModifier = Min<LTFLOAT>(fMaxModifier, fModifier);

	return fModifier;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleSkillUpdate
//
//	PURPOSE:	Handle any changes due to skill being updated
//
// ----------------------------------------------------------------------- //
void CPlayerObj::HandleSkillUpdate(ILTMessage_Read *pMsg)
{
	m_Skills.HandleSkillUpdate(pMsg);

	//max armor or max health might have changed
	ResetHealth(LTTRUE);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleVolumeEnter
//
//	PURPOSE:	Handle any changes what occur when the ;player changes
//				volumes such as region change transmission
//
// ----------------------------------------------------------------------- //
void CPlayerObj::HandleVolumeEnter(AIInformationVolume* pNewVolume)
{
	AIInformationVolume* pOldVolume = GetLastInformationVolume();
	
	// If the new volume has a region, then we either will be sending the 
	// update if it is the same, or sending one if there is no old region.
	if ( !pNewVolume || ( pNewVolume && !pNewVolume->HasRegion() )) 
		return;

	AIRegion* pRegion = NULL;
	if ( pOldVolume )
	{
		if ( pOldVolume->HasRegion() )
		{
			pRegion = pOldVolume->GetRegion();
		}
	}

	if ( pRegion != pNewVolume->GetRegion() )
	{
		// send
		uint8 u8Pset = pNewVolume->GetRegion()->GetPsetByte();

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_REGION_CHANGE);
		cMsg.Writeuint8(u8Pset);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(0.0f);
		g_pLTServer->SendToClient(cMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CPlayerObj::GetVerticalThreshold()
//              
//	PURPOSE:	Returns the verical dims used to calculate information for what
//				AIVolume the player is in.
//              
//----------------------------------------------------------------------------
float CPlayerObj::GetVerticalThreshold() const
{
	LTVector vDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);
	return vDims.y*2.0f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CPlayerObj::GetInfoVerticalThreshold()
//              
//	PURPOSE:	Returns the verical dims used to calculate information for what
//				Information Volume the player is in.
//              
//----------------------------------------------------------------------------
float CPlayerObj::GetInfoVerticalThreshold() const
{
	// The player should only use his actual position when 
	// searching for Information volumes.  Otherwise, she thinks
	// she's in Hiding Volumes on the floor below.

	return 0.0f;
}


void CPlayerObj::SetCarriedObject( HOBJECT hObject, bool bTransition /* = false  */ )
{
	
	if (m_hCarriedObject && !hObject)
	{
		// remove body attachment
		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, m_hCarriedObject, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}
		g_pCommonLT->SetObjectFlags(m_hCarriedObject, OFT_Flags, 0, FLAG_FORCECLIENTUPDATE);


		if (m_bCarryingAI)
		{
			HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hCarriedObject, "knockout_in");
			if (hAni != INVALID_ANI)
			{
				g_pLTServer->SetModelAnimation(m_hCarriedObject, hAni);
				g_pLTServer->SetModelLooping(m_hCarriedObject, LTFALSE);
			}

			CCharacter *pChar = (CCharacter *)g_pLTServer->HandleToObject(m_hCarriedObject);
			if (pChar)
			{
				LTVector vPos, vForward, vUp(0.0f,1.0f,0.0f);
				LTRotation rRot;

				// Use our position if we are allowed to drop an object...
				// Use the last good drop pos if we are not allowed (not in an AI volume :)

				if( CanDropObject( ))
				{
					g_pLTServer->GetObjectPos(m_hObject, &vPos);
				}
				else
				{
					vPos = m_vLastGoodDropPos;
				}

				g_pLTServer->GetObjectRotation(m_hObject, &rRot);

				vForward = rRot.Forward();
				g_pLTServer->SetObjectPos(pChar->m_hObject, &vPos);
				g_pLTServer->SetObjectPos(pChar->GetHitBox(), &vPos);
				g_pLTServer->SetObjectRotation(pChar->m_hObject, &rRot);
				g_pLTServer->SetObjectRotation(pChar->GetHitBox(), &rRot);

				LTVector vDims;
				g_pPhysicsLT->GetObjectDims(pChar->m_hObject, &vDims);
				g_pPhysicsLT->SetObjectDims(pChar->GetHitBox(), &vDims,0);

				// [RP] 9/15/02 - If the player is transitioning with Cate do a "special" teleport
				//		to make sure she is in the correct position when the new level loads...

				bool bNormalTeleport = true;
				if( bTransition && IsMultiplayerGame() )
				{
					if( IsKindOf( m_hCarriedObject, "CAIHuman" ))
					{
						CAIHuman *pCate = dynamic_cast<CAIHuman*>(g_pLTServer->HandleToObject( m_hCarriedObject ));
						if( pCate )
						{
							if( !_stricmp( pCate->GetName(), "Cate" ))
							{
								bNormalTeleport = false;

								pCate->TransitioningInMultiplayerWhileCarryingCateTeleportHack( vPos );
							}
						}
					}
				}

				if( bNormalTeleport )
				{
					//send the telport message so that the AI's position is updated immediately
					char szMsg[128];
					sprintf(szMsg,"TELEPORT %f,%f,%f",vPos.x,vPos.y,vPos.z);
					SendTriggerMsgToObject(this, m_hCarriedObject, 0, szMsg);
				}

				pChar->HideAttachments(LTFALSE);

			}

		}
		else
		{
			HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hCarriedObject, "dropbody");
			if (hAni != INVALID_ANI)
			{
				g_pLTServer->SetModelAnimation(m_hCarriedObject, hAni);
				g_pLTServer->SetModelLooping(m_hCarriedObject, LTFALSE);
			}

		}

		m_hCarriedObject = LTNULL;
		m_bCanDropBody = false;
		m_bCarryingAI = false;
		SetCarrying(CFX_CARRY_NONE);

	}

	if (hObject && !m_hCarriedObject)
	{
		m_bCarryingAI = !!(IsKindOf(hObject,"CAIHuman"));

		if (IsKindOf(hObject,"Body") || m_bCarryingAI)
		{
			SetCarrying(CFX_CARRY_BODY);
		}
		else
		{
			DoomsDayPiece* pDoomsDayPiece = dynamic_cast< DoomsDayPiece* >( g_pLTServer->HandleToObject( hObject ));
			//we only know about carrying doomsday pieces right now
			ASSERT(pDoomsDayPiece);
			if( pDoomsDayPiece )
			{
				switch (pDoomsDayPiece->GetDoomsDayPieceType())
				{
				case kDoomsDay_transmitter:
					SetCarrying(CFX_CARRY_DD_TRAN);
					break;
				case kDoomsDay_battery:
					SetCarrying(CFX_CARRY_DD_BATT);
					break;
				case kDoomsDay_Core:
					SetCarrying(CFX_CARRY_DD_CORE);
					break;
				}
			}
		}

		m_hCarriedObject = hObject;


		g_pCommonLT->SetObjectFlags(m_hCarriedObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE);

		HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_hCarriedObject, "carrybody");
		if (hAni != INVALID_ANI)
		{
			g_pLTServer->SetModelAnimation(m_hCarriedObject, hAni);
			g_pLTServer->SetModelLooping(m_hCarriedObject, LTFALSE);
		}

		

		// Attach body to shoulder
		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		static char* pSocket = "Body";

		HATTACHMENT hAttachment;
		if (LT_OK != g_pLTServer->CreateAttachment(m_hObject, m_hCarriedObject, pSocket, &LTVector(0,0,0), &LTRotation(), &hAttachment))
		{
			ASSERT(!"CreateAttachment() failed.");
			g_pLTServer->CPrint("Failed to attach body.");
		}
		
		
		if (IsMultiplayerGame())
		{
			
			PlaySound(s_pMPBodySounds[GetRandom(0,1)], m_fSoundRadius, LTTRUE);
		}
		else
		{
			PlaySound(s_pBodySounds[GetRandom(0,1)], m_fSoundRadius, LTTRUE);
		}
		
		
		
		
		// Set a valid drop position.  We know this is good otherwise we wouldn't be able to pick it up...
		
		g_pLTServer->GetObjectPos( m_hObject, &m_vLastGoodDropPos );
	}

}


bool CPlayerObj::IsCarryingHeavyObject() 
{
	return ( IsCarrying() && (m_pcs->nCarrying == CFX_CARRY_BODY || m_pcs->nCarrying == CFX_CARRY_DD_CORE));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetClientSaveData
//
//	PURPOSE:	Returns the permission sets of the player in one byte
//
// ----------------------------------------------------------------------- //
void CPlayerObj::SetClientSaveData(ILTMessage_Read *pMsg)
{
	m_pClientSaveData = pMsg;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::DoWeaponReload
//
//  PURPOSE:	Reload the clip of our current weapon with the specified ammo...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoWeaponReload( uint8 nAmmoId )
{
	if( !m_pPlayerAttachments ) return;

	CWeapon *pWeapon = m_pPlayerAttachments->GetWeapon();
	if( pWeapon )
	{
		pWeapon->ReloadClip( LTTRUE, -1, nAmmoId );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::HandleExit
//
//  PURPOSE:	Handle our exit of the level.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleExit( bool bNewMission )
{
	// Save whether we should reset for the new mission.
	m_bNewMission = bNewMission;

	// Only drop our keys between missions, NOT between levels.

	if( m_bNewMission )
	{
		// Drop our keys.
		m_Keys.Clear();
		m_Stats.Init();
	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::HandlePreExit
//
//  PURPOSE:	Get ready to exit the level.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandlePreExit()
		{
	HCLIENT hClient = GetClient( );
	if( !hClient )
		return;

	uint32  nClientID   = g_pLTServer->GetClientID(hClient);

	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_PLAYER_SUMMARY);
	cMsg.Writeuint32(nClientID);    

	m_Stats.WriteData(cMsg);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::AddToObjectList
//
//  PURPOSE:	Add any objects attached to us to the list...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AddToObjectList( ObjectList *pObjList, eObjListControl eControl /* = eObjListNODuplicates  */)
{
	if( !pObjList ) return;

	// Send to base first...

	CCharacter::AddToObjectList( pObjList, eControl );

	// If we are carring a body, add it...

	GameBase* pCarriedObject = dynamic_cast< GameBase* >( g_pLTServer->HandleToObject( m_hCarriedObject ));
	if( pCarriedObject )
	{
		pCarriedObject->AddToObjectList( pObjList, eControl );
	}

	GameBase* pVehicle = dynamic_cast< GameBase* >( g_pLTServer->HandleToObject( m_hVehicleModel ));
	if( pVehicle )
	{
		pVehicle->AddToObjectList( pObjList, eControl );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RemoveAttachments
//
//	PURPOSE:	Remove the attachments aggregate with the option of destroying it.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RemoveAttachments( bool bDestroyAttachments )
{
	m_pPlayerAttachments = LTNULL;

	CCharacter::RemoveAttachments( bDestroyAttachments );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::DropCarriedObject
//
//  PURPOSE:	Drop the object we're carrying.
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::DropCarriedObject( bool bTransition /* = false  */ )
{
	// Make sure we have a body and we can drop it.
	if( !m_hCarriedObject )
		return false;

	// Make sure the object is visible.  It may have been hidden
	// due to a reload of a save game in mp.
	g_pCommonLT->SetObjectFlags( m_hCarriedObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hCarriedObject, OFT_User, dwFlags );

	if (IsKindOf(m_hCarriedObject,"Body"))
	{
		Body *pBody = dynamic_cast<Body*>(g_pLTServer->HandleToObject( m_hCarriedObject ));
		if( pBody )
		{
			pBody->SetBeingCarried( false, NULL );
		}

		if( !CanDropObject( ))
		{
			// Send message to body to drop at the last good drop position...

			char szMsg[128] = {0};
			LTSNPrintF( szMsg, ARRAY_LEN( szMsg ), "DROPATPOS %.4f %.4f %.4f", m_vLastGoodDropPos.x, m_vLastGoodDropPos.y, m_vLastGoodDropPos.z );
			
			SendTriggerMsgToObject( this, m_hCarriedObject, 0, szMsg );
		}
		else
		{
			// Send trigger to body to toggle move.
			SendTriggerMsgToObject(this, m_hCarriedObject, 0, "CARRY");
		}	
	}
	else if (IsKindOf(m_hCarriedObject,"CAIHuman"))
	{
		HOBJECT hAI = m_hCarriedObject;
	    g_pCommonLT->SetObjectFlags(hAI, OFT_User, 0, USRFLG_ATTACH_HIDE1SHOW3);
		ShowObjectAttachments(hAI,true);

		SetCarriedObject( NULL, bTransition );

		g_pCommonLT->SetObjectFlags(hAI, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT);
		CAIHuman *pAI = dynamic_cast<CAIHuman*>(g_pLTServer->HandleToObject( hAI ));
		if( pAI )
		{
			CAIGoalSpecialDamage* pGoal = (CAIGoalSpecialDamage*)pAI->GetGoalMgr()->FindGoalByType( kGoal_SpecialDamage );
			if( pGoal )
			{
				pGoal->PauseSpecialDamage( LTFALSE );
			}

			pAI->SetBeingCarried( false );

			if( pAI->GetHitBox() )
			{
				g_pCommonLT->SetObjectFlags(pAI->GetHitBox(), OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT);
			}

			// NOTE: Right now we don't care if we are not allowed to drop the body.  SetCarriedBody()
			// will move the AI to the correct position if we are not allowed to drop the body.
		}
	}
	else if( IsKindOf( m_hCarriedObject, "DoomsDayPiece" ))
	{
		HOBJECT hObj = m_hCarriedObject;
		g_pCommonLT->SetObjectFlags( hObj, OFT_User, 0, USRFLG_ATTACH_HIDE1SHOW3 );
		
		ShowObjectAttachments( hObj, true );
		SetCarriedObject( NULL, bTransition );

		// Send message to the object letting it know it's been dropped so it can handle things
		// like going solid again or other special handling...
		
		SendTriggerMsgToObject( this, hObj, 0, "DROP" );
	}
	else
		return false;

	m_PStateChangeFlags |= PSTATE_INTERFACE;

	m_hCarriedObject = NULL;
	m_bCanDropBody = false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::PickupObject
//
//  PURPOSE:	Pickup up an object.
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::PickupObject( HOBJECT hObject )
{
	// Make sure we don't already have a carried object...
	if( m_hCarriedObject || !hObject )
		return false;

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( hObject, OFT_User, dwFlags );

	if (IsKindOf(hObject,"Body"))
	{
		// Send trigger to body to toggle move.
		SendTriggerMsgToObject(this, hObject, 0, "CARRY");

		Body *pBody = dynamic_cast<Body*>(g_pLTServer->HandleToObject( hObject ));
		if( pBody )
		{
			pBody->SetBeingCarried( true, m_hObject );
		}

	}
	else if (IsKindOf(hObject,"CAIHuman"))
	{
	    CAIHuman *pAI = dynamic_cast<CAIHuman*>(g_pLTServer->HandleToObject(hObject));
		if( pAI )
		{
			// Don't try and carry a body that is already being carried...

			if( pAI->BeingCarried() )
				return false;						

			g_pCommonLT->SetObjectFlags(hObject, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3);

			ShowObjectAttachments(hObject,false);
			SetCarriedObject(hObject);

			CAIGoalSpecialDamage* pGoal = (CAIGoalSpecialDamage*)pAI->GetGoalMgr()->FindGoalByType( kGoal_SpecialDamage );
			if( pGoal )
			{
				pGoal->PauseSpecialDamage( LTTRUE );
			}

			pAI->SetBeingCarried( true );

			if( pAI->GetHitBox() )
			{
				g_pCommonLT->SetObjectFlags(pAI->GetHitBox(), OFT_Flags, 0, FLAG_RAYHIT);
			}
		}
	}
	else if( IsKindOf( hObject, "DoomsDayPiece" ))
	{
		// Send message to the object letting it know the player wants to pick it up.
		// The object can then decide if it is ok to be picked up or not.

		SendTriggerMsgToObject( this, hObject, 0, "CARRY" );
	}
	else
		return false;

	m_PStateChangeFlags |= PSTATE_INTERFACE;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::CanDropObject
//
//  PURPOSE:	Are we allowed to drop our carried object or not...
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::CanDropObject( )
{
	// If we are carrying a dead body or knocked out AI make sure we are in an appropriate volume...

	if( !m_hCarriedObject || IsKindOf( m_hCarriedObject, "Body") || IsKindOf( m_hCarriedObject, "CAIHuman" ))
	{
		return m_bCanDropBody;
	}

	// All other objects don't care where they are droped...

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::AcquireLevelDefaultWeapons
//
//  PURPOSE:	Obtaian the mission and level default weapons...
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::AcquireLevelDefaultWeapons( int nMissionId, int nLevelId )
{
	// Make sure the mission and level are valid...
	
	MISSION* pMission = g_pMissionButeMgr->GetMission( nMissionId );
	if( !pMission )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Invalid mission." );
		return false;
	}

	LEVEL *pLevel = g_pMissionButeMgr->GetLevel( nMissionId, nLevelId );
	if( !pLevel )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Invalid level." );
		return false;
	}

	if( !m_pPlayerAttachments )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Invalid attachments object." );
		return false;
	}

	// Add the mission specific defaults...
/*BEGIN HACK - for DM testing 
	if( !IsCoopMultiplayerGameType( ))
	{
		int aDefaultWeapons[2] = { g_pWeaponMgr->GetWeapon("Katana")->nId,
									g_pWeaponMgr->GetWeapon("Beretta")->nId,
								};

		for( int i = 0; i < 2; ++i )
		{
			int nWeaponId = aDefaultWeapons[i];
		
			// Check if we already have this weapon.
			CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon( nWeaponId );
			if (pWeapon)
			{
				continue;
			}

			WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon( nWeaponId );
			if( !pWeaponData || pWeaponData->bServerRestricted )
			{
				ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Invalid weapon data" );
				continue;
			}

			AMMO const *pAmmoData = g_pWeaponMgr->GetAmmo(pWeaponData->nDefaultAmmoId);
			if( !pAmmoData || pAmmoData->bServerRestricted )
			{
				ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Invalid ammoid" );
				continue;
			}

			m_pPlayerAttachments->ObtainWeapon( nWeaponId, pWeaponData->nDefaultAmmoId, 
				pAmmoData->nSelectionAmount, TRUE );
			pWeapon = m_pPlayerAttachments->GetWeapon( nWeaponId );
			if( !pWeapon )
			{
				ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Could not obtain weapon." );
				continue;
			}
		}

		return true;

	}
END HACK - for DM testing */

	int i;
	for( i = 0; i < pMission->nNumDefaultWeapons; ++i )
	{
		int nWeaponId = pMission->aDefaultWeapons[i];
	
		// Check if we already have this weapon.
		CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon( nWeaponId );
		if (pWeapon)
		{
			continue;
		}

		WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon( nWeaponId );
		if( !pWeaponData || pWeaponData->bServerRestricted )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Invalid weapon data" );
			continue;
		}

		AMMO const *pAmmoData = g_pWeaponMgr->GetAmmo(pWeaponData->nDefaultAmmoId);
		if( !pAmmoData || pAmmoData->bServerRestricted )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Invalid ammoid" );
			continue;
		}

		m_pPlayerAttachments->ObtainWeapon( nWeaponId, pWeaponData->nDefaultAmmoId, 
			pAmmoData->nSelectionAmount, TRUE );
		pWeapon = m_pPlayerAttachments->GetWeapon( nWeaponId );
		if( !pWeapon )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Could not obtain weapon." );
			continue;
		}
	}

	// Add the level specific defaults...

	for( i = 0; i < pLevel->nNumDefaultWeapons; ++i )
	{
		int nWeaponId = pLevel->aDefaultWeapons[i];

		// Check if we already have this weapon.
		CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon( nWeaponId );
		if (pWeapon)
		{
			continue;
		}

		WEAPON const *pWeaponData = g_pWeaponMgr->GetWeapon( nWeaponId );
		if( !pWeaponData || pWeaponData->bServerRestricted )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Invalid weapon data" );
			continue;
		}

		AMMO const *pAmmoData = g_pWeaponMgr->GetAmmo( pWeaponData->nDefaultAmmoId );
		if( !pAmmoData || pAmmoData->bServerRestricted )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Invalid ammoid" );
			continue;
		}

		m_pPlayerAttachments->ObtainWeapon( nWeaponId, pWeaponData->nDefaultAmmoId, 
			pAmmoData->nSelectionAmount, TRUE );
		pWeapon = m_pPlayerAttachments->GetWeapon( nWeaponId );
		if( !pWeapon )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultWeapons: Could not obtain weapon." );
			continue;
		}
	}


	// If specified, switch to the selected weapon for this mission...

	if (pMission->nSelectedWeapon != ( signed char )WMGR_INVALID_ID && nLevelId == 0)
	{
		ChangeWeapon( g_pWeaponMgr->GetCommandId( pMission->nSelectedWeapon ), LTTRUE );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::AcquireLevelDefaultMods
//
//  PURPOSE:	Obtaian the mission and level default mods...
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::AcquireLevelDefaultMods( int nMissionId, int nLevelId )
{
	// Make sure the mission and level are valid...

	MISSION* pMission = g_pMissionButeMgr->GetMission( nMissionId );
	if( !pMission )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid mission." );
		return false;
	}

	LEVEL *pLevel = g_pMissionButeMgr->GetLevel( nMissionId, nLevelId );
	if( !pLevel )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid level." );
		return false;
	}

	if( !m_pPlayerAttachments )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid attachments object." );
		return false;
	}
		
	// Add the mission specific defaults...

	const MOD *pMod = LTNULL;
	for( int i = 0; i < pMission->nNumDefaultMods; ++i )
	{
		int nModId = pMission->aDefaultMods[i];

		pMod = g_pWeaponMgr->GetMod( nModId );
		if( !pMod )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid modid" );
			continue;
		}

        uint8 nWeaponId = pMod->GetWeaponId();
		CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon(nWeaponId);
		if( !pWeapon )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Don't have weapon." );
			continue;
		}

		// Don't add the mod again.
		if( pWeapon->HaveMod( nModId ))
			continue;

		m_pPlayerAttachments->AcquireMod( nModId, false );
	}
	

	// Add the level specific defaults...

	for(int i = 0; i < pLevel->nNumDefaultMods; ++i )
	{
		int nModId = pLevel->aDefaultMods[i];
		
		pMod = g_pWeaponMgr->GetMod( nModId );
		if( !pMod )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid modid" );
			continue;
		}

        uint8 nWeaponId = pMod->GetWeaponId();
		CWeapon* pWeapon = m_pPlayerAttachments->GetWeapon( nWeaponId );
		if( !pWeapon )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Don't have weapon." );
			continue;
		}

		// Don't add the mod again.
		if( pWeapon->HaveMod( nModId ))
			continue;

		m_pPlayerAttachments->AcquireMod( nModId, false );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::AcquireLevelDefaultAmmo
//
//  PURPOSE:	Obtaian the mission and level default ammo...
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::AcquireLevelDefaultAmmo( int nMissionId, int nLevelId )
{
	// Make sure the mission and level are valid...
	
	MISSION* pMission = g_pMissionButeMgr->GetMission( nMissionId );
	if( !pMission )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultAmmo: Invalid mission." );
		return false;
	}

	LEVEL *pLevel = g_pMissionButeMgr->GetLevel( nMissionId, nLevelId );
	if( !pLevel )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultAmmo: Invalid level." );
		return false;
	}

	if( !m_pPlayerAttachments )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultAmmo: Invalid attachments object." );
		return false;
	}

	// Add the mission specific defaults...

	const AMMO *pAmmo = LTNULL;
	int i;
	for( i = 0; i < pMission->nNumDefaultAmmo; ++i )
	{
		int nAmmoId = pMission->aDefaultAmmo[i];

		pAmmo = g_pWeaponMgr->GetAmmo( nAmmoId );
		if( !pAmmo || pAmmo->bServerRestricted )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid ammoid" );
			continue;
		}

	    if( m_pPlayerAttachments->GetAmmoCount( nAmmoId ) <= 0 )
			m_pPlayerAttachments->AcquireAmmo( nAmmoId );
	}

	// Add the level specific defaults...

	for( i = 0; i < pLevel->nNumDefaultAmmo; ++i )
	{
		int nAmmoId = pLevel->aDefaultAmmo[i];

		pAmmo = g_pWeaponMgr->GetAmmo( nAmmoId );
		if( !pAmmo || pAmmo->bServerRestricted )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid ammoid" );
			continue;
		}

		if( m_pPlayerAttachments->GetAmmoCount( nAmmoId ) <= 0 )
			m_pPlayerAttachments->AcquireAmmo( nAmmoId );
	}
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::AcquireCurrentLevelDefaults
//
//  PURPOSE:	Obtaian the mission and level defaults...
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::AcquireCurrentLevelDefaults()
{
	// Get the mission and lefel defaults...

	int nMissionId	= g_pServerMissionMgr->GetCurrentMission();
	int nLevelId	= g_pServerMissionMgr->GetCurrentLevel();
	
	if( !AcquireLevelDefaultWeapons( nMissionId, nLevelId ))
		return false;

	if( !AcquireLevelDefaultMods( nMissionId, nLevelId ))
		return false;

	if( !AcquireLevelDefaultAmmo( nMissionId, nLevelId ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::SetNetClientData
//
//  PURPOSE:	Set the client data when client joins.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetNetClientData( NetClientData const& netClientData )
{
	// Give the player a unique name.
	char szUniqueName[MAX_PLAYER_NAME];
	GenerateUniqueName( netClientData.m_szName, szUniqueName, ARRAY_LEN( szUniqueName ));
	m_sNetNameUnique = szUniqueName;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::GetNetUniqueName
//
//  PURPOSE:	Get the unique player name.  This could be different than
//				then name the client gave us since display names must be unique.
//
// ----------------------------------------------------------------------- //

char const* CPlayerObj::GetNetUniqueName( )
{
	return m_sNetNameUnique.c_str( );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::GenerateUniqueName
//
//  PURPOSE:	Generates a unqiue name for all players given a base name.
//
// ----------------------------------------------------------------------- //

static bool IsNameTaken( char const* pszName, CPlayerObj* pSkipPlayer )
{
	for( CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( ); 
		iter != CPlayerObj::GetPlayerObjList( ).end( ); iter++ )
	{
		CPlayerObj* pPlayer = *iter;

		// Skip target player.
		if( pPlayer == pSkipPlayer )
			continue;

		// Check if our name is different than this player
		if( stricmp( pszName, pPlayer->GetNetUniqueName( )) == 0 )
			return true;
	}

	return false;
}

void CPlayerObj::GenerateUniqueName( char const* pszNameBase, char* pszUniqueName, int nUniqueNameSize )
{
	CString sUniqueName = pszNameBase;
	bool bFound = false;

	// If the base name is taken, then generate a new name with a number on the 
	// end of the base name.
	if( IsNameTaken( sUniqueName, this ))
	{
		int nCount = 0;
		while( 1 )
		{
			sUniqueName.Format( "%s%d", pszNameBase, nCount );
			if( !IsNameTaken( sUniqueName, this ))
				break;

			nCount++;
		}
	}

	// Copy the string.
	strncpy( pszUniqueName, sUniqueName, nUniqueNameSize );
	pszUniqueName[nUniqueNameSize-1] = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::OnLinkBroken
//
//	PURPOSE:	Handle attached object getting removed.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	if( pRef == &m_hVehicleModel ||
		pRef == &m_hCarriedObject )
	{
		if( pRef == &m_hCarriedObject )
		{
			m_PStateChangeFlags |= PSTATE_INTERFACE;

			m_hCarriedObject = NULL;
			m_bCanDropBody = false;
		}

		HATTACHMENT hAttachment;
		if ( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment(hAttachment);
		}
	}

	CCharacter::OnLinkBroken( pRef, hObj );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetupBody
//
//	PURPOSE:	Assign our dead body handle...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetupBody( Body* pBody )
{
	if( !pBody )
		return;

	// See if this body is revivable or if it's the respawn body.
	bool bRevivableBody = IsRevivePlayerGameType( ) && m_bCanBeRevived;
	float fLifetime = m_fBodyLifetime;

	// If it's revivable, we'll need to reference it for later removal.
	if( bRevivableBody )
	{
		if( m_hDeadBody )
			g_pLTServer->RemoveObject( m_hDeadBody );

		m_hDeadBody = pBody->m_hObject;
		fLifetime = -1;

	}

	BODYINITSTRUCT bi;
	bi.eBodyState = GetBodyState();
	bi.pCharacter = this;
	bi.bPermanentBody = bRevivableBody;
	bi.fBodyLifetime = fLifetime;
	bi.bCanRevive = bRevivableBody;

	pBody->Init(bi);

// Don't allow killing of revivable body.
/*
	// If the body is revivable, we need to give it a few hitpoints,
	// so that it can be later killed.
	if( bRevivableBody )
	{
		// Give the body some hitpoints, but take away the amount of hitpoints our player is below
		// zero.  This allows bodies to be instantly killed if they are hit with a big weapon.
		CDestructible* pDestructible = pBody->GetDestructible( );
		pDestructible->SetCanDamage( LTTRUE );
		float fReviveBodyHitPoints = g_pServerButeMgr->GetPlayerAttributeFloat( PLAYER_BUTE_REVIVEBODYHEALTH );
		pDestructible->SetHitPoints( fReviveBodyHitPoints );

		DamageStruct damage;
		damage.fDamage = -m_damage.GetHitPoints( );
		damage.DoDamage( pBody, pBody->m_hObject );
	}
*/
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RemoveBody
//
//	PURPOSE:	Get rid of the dead body.  No pigs required.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RemoveBody()
{
	if( !m_hDeadBody)
		return;

	Body* pBody = dynamic_cast<Body*>(g_pLTServer->HandleToObject( m_hDeadBody ));
	if( !pBody )
		return;

	// Make sure our body is not permanent when we try to fade it away...

	pBody->SetPermanentBody( false );
	pBody->SetState( kState_BodyFade );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CreateRespawnBody
//
//	PURPOSE:	Change body to represent we have respawned.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::CreateRespawnBody( )
{
	// Can no longer be revived.
	m_bCanBeRevived = false;
	m_PStateChangeFlags |= PSTATE_INTERFACE;

	Body* pBody = dynamic_cast< Body* >( g_pLTServer->HandleToObject( m_hDeadBody ));
	if( !pBody )
		return;

	// Tell body it's now a body of someone that has respawned.
	pBody->SetPermanentBody( false );
	pBody->SetCanRevive( false );
	pBody->ResetLifetime( GetConsoleFloat("BodyLifetime",30.0f) );
	TransferWeapons( pBody, false );

// Don't allow killing of revivable body.
/*
	CDestructible* pDestructible = pBody->GetDestructible( );
	pDestructible->SetCanDamage( LTFALSE );
*/

	// Forget about our body.  It will remove itself.
	m_hDeadBody = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RemoveCarriedObject
//
//	PURPOSE:	Get rid of the object we are carrying...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RemoveCarriedObject()
{
	if ( m_hCarriedObject )
	{
		g_pLTServer->RemoveObject( m_hCarriedObject );
		m_hCarriedObject = NULL;
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetNumberPlayersWithClients
//
//	PURPOSE:	Counts the number of players that have clients.
//				After a load of a save game, some players may not
//				get possessed by clients if those clients don't come back.
//
// ----------------------------------------------------------------------- //

uint32 CPlayerObj::GetNumberPlayersWithClients( )
{
	uint32 nNumberPlayersWithClients = 0;

	// Count the number of players with clients that are loaded.
	CPlayerObj::PlayerObjList::const_iterator iter = CPlayerObj::GetPlayerObjList( ).begin( );
	while( iter != CPlayerObj::GetPlayerObjList( ).end( ))
	{
		CPlayerObj* pPlayerObj = *iter;
		if( pPlayerObj->GetClient( ))
			nNumberPlayersWithClients++;

		iter++;
	}

	return nNumberPlayersWithClients;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HideCharacter
//
//	PURPOSE:	Hide/Show character.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HideCharacter(LTBOOL bHide)
{
	CCharacter::HideCharacter( bHide );

	// Hide/unhide our carried object.
	if( m_hCarriedObject )
	{
		g_pCommonLT->SetObjectFlags( m_hCarriedObject, OFT_Flags, bHide ? 0 : FLAG_VISIBLE, FLAG_VISIBLE );
	}

	// Hide/unhide our dead body.
	if( m_hDeadBody )
	{
		Body* pBody = dynamic_cast< Body* >( g_pLTServer->HandleToObject( m_hDeadBody ));
		if( pBody )
		{
			pBody->HideBody( bHide ? true : false );
		}
		else
		{
			ASSERT( !"CPlayerObj::HideCharacter:  Invalid body." );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetMultiplayerSkills()
//
//	PURPOSE:	Client's skill preferences might be different than those
//				 in the saved game, reset everything based on the client's
//				 current preferences
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetMultiplayerSkills()
{

    float fHealthX = m_Skills.GetSkillModifier(SKL_STAMINA,StaminaModifiers::eMaxHealth);
    float fArmorX  = m_Skills.GetSkillModifier(SKL_ARMOR,ArmorModifiers::eMaxArmor);

	float fHealthMax = fHealthX * g_pModelButeMgr->GetModelMaxHitPoints(m_eModelId);
	float fArmorMax = fArmorX * g_pModelButeMgr->GetModelMaxArmor(m_eModelId);

	m_damage.SetMaxHitPoints(fHealthMax);
	m_damage.SetMaxArmorPoints(fArmorMax);

	if (m_damage.GetHitPoints() > fHealthMax)
		m_damage.SetHitPoints(fHealthMax);

	if (m_damage.GetArmorPoints() > fArmorMax)
		m_damage.SetArmorPoints(fArmorMax);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cMsg.Writeuint8(IC_MAX_HEALTH_ID);
	cMsg.Writeuint8(0);
	cMsg.Writeuint8(0);
	cMsg.Writefloat(m_damage.GetMaxHitPoints());
	g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);

	cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cMsg.Writeuint8(IC_MAX_ARMOR_ID);
	cMsg.Writeuint8(0);
	cMsg.Writeuint8(0);
	cMsg.Writefloat(m_damage.GetMaxArmorPoints());
	g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);


	
	for (int a = 0; a < g_pWeaponMgr->GetNumAmmoIds(); ++a)
	{
		const AMMO* pAmmo = g_pWeaponMgr->GetAmmo(a);
		int nCurAmount = m_pPlayerAttachments->GetAmmoCount(a);
		int nMax = pAmmo->GetMaxAmount(m_hObject);
		if (nCurAmount > nMax)
		{
			m_pPlayerAttachments->SetAmmo(a,nMax);

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
			cMsg.Writeuint8(IC_AMMO_ID);
			cMsg.Writeuint8(WMGR_INVALID_ID);
			cMsg.Writeuint8(a);
			cMsg.Writefloat((LTFLOAT)nCurAmount);
			g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);

		}
	}

}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetLean
//
//	PURPOSE:	Sets the lean value of the player and updates other clients.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetLean( CPPlayerLeanTypes eLean, LTVector const& vPos )
{
	if( eLean == m_ePlayerLean )
		return;

	m_ePlayerLean = eLean;

	uint32 nUsrFlags = ( m_ePlayerLean != PL_CENTER ) ? USRFLG_PLAYER_LEANING : 0;

    g_pCommonLT->SetObjectFlags( m_hObject, OFT_User, nUsrFlags, USRFLG_PLAYER_LEANING );

	if( m_ePlayerLean != PL_CENTER )
	{
		// If there was already a lean stimuls remove it

		if( m_eLeanVisibleStimID != kStimID_Unset )
		{
			g_pAIStimulusMgr->RemoveStimulus( m_eLeanVisibleStimID );
		}
		
		// Register the new lean stimulus position...

		LTVector vObjPos;
		g_pLTServer->GetObjectPos( m_hObject, &vObjPos );

		// Register it as a Dynamic position with an offset.
		m_eLeanVisibleStimID = g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyLeanVisible, m_hObject, NULL, CAIStimulusRecord::kDynamicPos_TrackSource, vPos - vObjPos, 1.f );
	}
	else
	{
		if( m_eLeanVisibleStimID != kStimID_Unset )
		{
			g_pAIStimulusMgr->RemoveStimulus( m_eLeanVisibleStimID );
			m_eLeanVisibleStimID = kStimID_Unset;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetupTeamModel
//
//	PURPOSE:	Setup our team model.
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::SetupTeamModel( bool bResetModel )
{
	uint8 nTeamId = GetTeamID( );
	if( nTeamId == INVALID_TEAM )
		return false;

	CTeam* pTeam = CTeamMgr::Instance( ).GetTeam( nTeamId );
	if( !pTeam )
		return false;

	// Setup our team model.
	m_eModelId = g_pModelButeMgr->GetTeamModel( pTeam->GetModel( ));
	m_eModelSkeleton = g_pModelButeMgr->GetModelSkeleton( m_eModelId );

	// Check if we should reset the model now.
	if( bResetModel )
	{
		ResetModel();
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetTeamID
//
//	PURPOSE:	Setup or team model.
//
// ----------------------------------------------------------------------- //
uint8 CPlayerObj::GetTeamID( ) const
{
	uint8 nTeamId = CTeamMgr::Instance( ).GetTeamIdOfPlayer( g_pLTServer->GetClientID(GetClient( )));
	return nTeamId;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CanRespawn
//
//	PURPOSE:	Check if player is allowed to respawn.
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::CanRespawn( )
{
	// We need to be dead.
	if( GetState() != PS_DEAD )
		return false;

	// Don't allow respawning if the gamemode doesn't allow it.
	if( !g_pServerMissionMgr->CanRespawn( *this ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CanRevive
//
//	PURPOSE:	Check if player is allowed to revive.
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::CanRevive( )
{
	// We need to be dead.
	if( GetState() != PS_DEAD )
		return false;

	// Don't allow respawning if the gamemode doesn't allow it.
	if( !g_pServerMissionMgr->CanRespawn( *this ))
		return false;

	return true;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SwitchToTeam
//
//	PURPOSE:	handle player's request to change teams
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SwitchToTeam(uint8 nTeam )
{
	if( !IsTeamGameType( )) return;
	if (nTeam == GetTeamID() || nTeam > CTeamMgr::Instance().GetNumTeams() ) return;

	CTeamMgr::Instance().AddPlayer( g_pLTServer->GetClientID(GetClient( )), nTeam );
	CTeamMgr::Instance().UpdateClient();
	SetupTeamModel(false);

	g_pGameServerShell->SendPlayerInfoMsgToClients(NULL, this, MID_PI_UPDATE);

	m_nRequestedTeam = INVALID_TEAM;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CancelRevive
//
//	PURPOSE:	Cancels ability to revive body by removing the revivable body.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::CancelRevive( )
{
	if( IsRevivePlayerGameType( ) && m_hDeadBody )
		CreateRespawnBody( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetClient
//
//	PURPOSE:	Set the client and make sure the specialFX message is updated...
//
//	NOTE:		ONLY call this from within CGameServerShell::RespawnPlayer()!
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetClient( HCLIENT hClient )
{
	m_hClient = hClient;

	CreateSpecialFX();
}