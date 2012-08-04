// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerObj.cpp
//
// PURPOSE : Player object implementation
//
// CREATED : 9/18/97
//
// (c) 1997-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PlayerObj.h"
#include "iltserver.h"
#include "CommandIDs.h"
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
#include "Camera.h"
#include "AISounds.h"
#include "CharacterHitBox.h"
#include "CharacterDB.h"
#include "ServerSoundMgr.h"
#include "VersionMgr.h"
#include "GameStartPoint.h"
#include "AIStimulusMgr.h"
#include "PlayerLure.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "MissionDB.h"
#include "ServerMissionMgr.h"
#include "ServerSaveLoadMgr.h"
#include "AI.h"
#include "AINavMesh.h"
#include "AIBrain.h"
#include "AIGoalMgr.h"
#include "PickupItem.h"
#include "WeaponDB.h"
#include "SoundDB.h"
#include "ServerDB.h"
#include "SkillDefs.h"
#include <stdio.h>
#include "SlowMoDB.h"
#include "PhysicsUtilities.h"
#include "ServerConnectionMgr.h"
#include "SharedFXStructs.h"
#include "Turret.h"
#include "SonicIntoneHandlers.h"
#include "NavMarker.h"
#include "NavMarkerTypeDB.h"
#include "StringUtilities.h"
#include "PlayerNodeGoto.h"
#include "GameModeMgr.h"
#include "GearItems.h"
#include "StoryMode.h"
#include "AIQuadTree.h"
#include "ForensicObject.h"
#include "BroadcastDB.h"
#include "CharacterMgr.h"
#include "ObjectTransformHistory.h"
#include "LTEulerAngles.h"
#include "ObjectiveDB.h"
#include "GameStartPoint.h"
#include "GameStartPointMgr.h"

LINKFROM_MODULE( PlayerObj );

extern CGameServerShell* g_pGameServerShell;
extern CAIStimulusMgr* g_pAIStimulusMgr;

static VarTrack s_vtVehicleImpactDistMin;
static VarTrack s_vtVehicleImpactDistMax;

static VarTrack s_vtBaseHealRate;
static VarTrack s_vtWalkHealRate;

// Number of seconds between players handing their dying to death state.
// Spreads deaths out for multiplayer bandwidth utilization.
static VarTrack s_vtDyingTimeGap;

static VarTrack s_vtAlwaysForceClientToServerPos;

EventCaster CPlayerObj::PlayerScoredKillEvent;

BEGIN_CLASS(CPlayerObj)
END_CLASS_FLAGS(CPlayerObj, CCharacter, CF_HIDDEN, "CPlayerObj is the in-game representation of the player.")

CPlayerObj::PlayerObjList CPlayerObj::m_lstPlayerObjs;
StopWatchTimer CPlayerObj::s_RespawnMsgTimer;
CPlayerObj::PlayerObjList CPlayerObj::m_lstDyingPlayers;

static CParsedMsg::CToken s_cTok_Off("Off");
static CParsedMsg::CToken s_cTok_On("On");
static CParsedMsg::CToken s_cTok_Loop( "Loop" );
static CParsedMsg::CToken s_cTok_Linger( "Linger" );
static CParsedMsg::CToken s_cTok_Stop( "Stop" );
static CParsedMsg::CToken s_cTok_0( "0" );
static CParsedMsg::CToken s_cTok_1( "1" );

static float s_fRegenMovementVel = 100.0f;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateObjectiveMsg
//
//  PURPOSE:	Make sure the objective message is valid.
//
// ----------------------------------------------------------------------- //

static bool ValidateObjectiveMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
	{
	if( !pInterface ) return false;

	if( cpMsgParams.m_nArgs < 2 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateObjectiveMsg()" );
			pInterface->CPrint( "    MSG - OBJECTIVE - No action parameter listed." );
	}

		return false;
}

	if( cpMsgParams.m_nArgs == 2 && (LTStrIEquals( cpMsgParams.m_Args[1], "REMOVE" )))
	{
		return true;
	}
	if (LTStrIEquals( cpMsgParams.m_Args[1], "ADD" ) )
	{
		if (cpMsgParams.m_nArgs < 3)
	{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateObjectiveMsg()" );
			pInterface->CPrint( "    MSG - OBJECTIVE ADD - no objective specified." );
			return false;
	}
		HRECORD hRec = DATABASE_CATEGORY( Objective ).GetRecordByName( cpMsgParams.m_Args[2]);	
		if (hRec)
	{
			return true;
	}

		if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateObjectiveMsg()" );
			pInterface->CPrint( "    MSG - OBJECTIVE ADD - '%s' is not a valid objective.", cpMsgParams.m_Args[2] );
	}
	
		return false;


	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateObjectiveMsg()" );
		pInterface->CPrint( "    MSG - OBJECTIVE - Action '%s' is not valid!", cpMsgParams.m_Args[1] );
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateMissionMsg
//
//  PURPOSE:	Make sure the Mission message is valid.
//
// ----------------------------------------------------------------------- //

static bool ValidateMissionMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( cpMsgParams.m_nArgs > 3 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMissionMsg()" );
			pInterface->CPrint( "    MSG - MISSION - Too many parameters listed." );
		}
		
		return false;
	}
	if( cpMsgParams.m_nArgs < 2 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMissionMsg()" );
			pInterface->CPrint( "    MSG - MISSION - No string ID specified." );
		}

		return false;
		}

	if	(INVALID_STRINGEDIT_INDEX == IndexFromStringID(cpMsgParams.m_Args[1]))
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateMissionMsg()" );
			pInterface->CPrint( "    MSG - MISSION - Invalid string ID '%s' specified.",cpMsgParams.m_Args[1] );
		}
		return false;
	}

	if( cpMsgParams.m_nArgs == 2)
	{
		return true;
	}
	if (LTStrIEquals( cpMsgParams.m_Args[2], "0" )  ||
		LTStrIEquals( cpMsgParams.m_Args[2], "1" ))
	{
		return true;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateMissionMsg()" );
		pInterface->CPrint( "    MSG - MISSION - '%s' is not a valid flag", cpMsgParams.m_Args[2] );
	}
	
	return false;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateTransmissionMsg
//
//  PURPOSE:	Make sure Transmission message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateTransmissionMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( cpMsgParams.m_nArgs < 2 || cpMsgParams.m_nArgs > 5 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateTransmissionMsg()" );
			pInterface->CPrint( "    MSG - TRANSMISSION - Invalid number of parameters." );
		}
		
		return false;
	}

	if( cpMsgParams.m_nArgs == 3 )
	{
		if( !LTStrICmp( cpMsgParams.m_Args[2], "0" ) ||
			!LTStrICmp( cpMsgParams.m_Args[2], "1" ))
		{
			return true;
		}

		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateTransmissionMsg()" );
			pInterface->CPrint( "    MSG - TRANSMISSION - Invalid [ActivePlayerComm] parameter." );
		}
		
		return false;
	}


	if( cpMsgParams.m_nArgs == 4 )
	{
		if( !LTStrICmp( cpMsgParams.m_Args[3], "0" ) ||
			!LTStrICmp( cpMsgParams.m_Args[3], "1" ) ||
			!LTStrICmp( cpMsgParams.m_Args[3], "-1" ))
		{
			return true;
		}

		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateTransmissionMsg()" );
			pInterface->CPrint( "    MSG - TRANSMISSION - Invalid [TeamNumber] parameter." );
		}
		
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateCrosshairMsg
//
//  PURPOSE:	Make sure Crosshair message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateCrosshairMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( cpMsgParams.m_nArgs != 2)
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateCrosshairMsg()" );
			pInterface->CPrint( "    MSG - Crosshair - Invalid number of parameters." );
		}

		return false;
	}

	if( (LTStrICmp( cpMsgParams.m_Args[1], "on" ) != 0) &&
		(LTStrICmp( cpMsgParams.m_Args[1], "off" ) != 0) )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateCrosshairMsg()" );
			pInterface->CPrint( "    MSG - Crosshair - Invalid parameter: Should be 'ENABLE' or 'DISABLE' " );
		}

		return false;
	}
	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateFlashlightMsg
//
//  PURPOSE:	Make sure Flashlight message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateFlashlightMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( cpMsgParams.m_nArgs < 2 || cpMsgParams.m_nArgs > 3 )
	{
		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( true );
			pInterface->CPrint( "ERROR! - ValidateFlashlightMsg()" );
			pInterface->CPrint( "    MSG - Flashlight - Invalid number of parameters." );
		}

		return false;
	}

	if( cpMsgParams.m_nArgs >= 2 )
	{
		if( (LTStrICmp( cpMsgParams.m_Args[1], "enable" ) != 0) &&
			(LTStrICmp( cpMsgParams.m_Args[1], "disable" ) != 0) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - ValidateFlashlightMsg()" );
				pInterface->CPrint( "    MSG - Flashlight - Invalid parameter: Should be 'ENABLE' or 'DISABLE' " );
			}

			return false;
		}
	}
	return true;
}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateAcquireWeapon
//
//  PURPOSE:	Make sure weapon message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateAcquireWeapon( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pWeaponDB )
		return true;

	if( cpMsgParams.m_nArgs > 1 )
	{
		char buf[256] = {0};
		for( int32 i = 0; i < cpMsgParams.m_nArgs - 1; ++i )
		{
			if( i > 0 )
				LTStrCat( buf, " ", LTARRAYSIZE(buf) );

			LTStrCat( buf, cpMsgParams.m_Args[i+1], LTARRAYSIZE(buf) );
		}

		if( !g_pWeaponDB->GetWeaponRecord( buf ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - ValidateAcquireWeapon()" );
				pInterface->CPrint( "    MSG - ACQUIREWEAPON - Invalid weapon '%s'!", buf );
			}
			
			return false;
		}

		return true;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateAcquireWeapon()" );
		pInterface->CPrint( "    MSG - ACQUIREWEAPON - No weapon specified!" );
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateAcquireAmmo
//
//  PURPOSE:	Make sure ammo message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateAcquireAmmo( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pWeaponDB )
		return true;

	if( cpMsgParams.m_nArgs > 1 )
	{
		char buf[256] = {0};
		for( int32 i = 0; i < cpMsgParams.m_nArgs - 1; ++i )
		{
			if( i > 0 ) 
				LTStrCat( buf, " ", LTARRAYSIZE(buf) );
			
			LTStrCat( buf, cpMsgParams.m_Args[i+1], LTARRAYSIZE(buf) );
		}

		if( !g_pWeaponDB->GetAmmoRecord( buf ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - ValidateAcquireAmmo()" );
				pInterface->CPrint( "    MSG - ACQUIREAMMO - Invalid ammo '%s'!", buf );
			}
			
			return false;
		}

		return true;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateAcquireAmmo()" );
		pInterface->CPrint( "    MSG - ACQUIREAMMO - No ammo specified!" );
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateAcquireMod
//
//  PURPOSE:	Make sure mod message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateAcquireMod( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pWeaponDB )
		return true;

	if( cpMsgParams.m_nArgs > 1 )
	{
		char buf[256] = {0};
		for( int32 i = 0; i < cpMsgParams.m_nArgs - 1; ++i )
		{
			if( i > 0 )
				LTStrCat( buf, " ", LTARRAYSIZE(buf) );
			
			LTStrCat( buf, cpMsgParams.m_Args[i+1], LTARRAYSIZE(buf) );
		}

		if( !g_pWeaponDB->GetModRecord( buf ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - ValidateAcquireMod()" );
				pInterface->CPrint( "    MSG - ACQUIREMOD - Invalid mod '%s'!", buf );
			}
			
			return false;
		}

		return true;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateAcquireMod()" );
		pInterface->CPrint( "    MSG - ACQUIREMOD - No mod specified!" );
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateAcquireGear
//
//  PURPOSE:	Make sure gear message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateAcquireGear( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pWeaponDB )
		return true;

	if( cpMsgParams.m_nArgs > 1 )
	{
		char buf[256] = {0};
		for( int32 i = 0; i < cpMsgParams.m_nArgs - 1; ++i )
		{
			if( i > 0 )
				LTStrCat( buf, " ", LTARRAYSIZE(buf) );
			
			LTStrCat( buf, cpMsgParams.m_Args[i+1], LTARRAYSIZE(buf) );
		}

		if( !g_pWeaponDB->GetGearRecord( buf ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - ValidateAcquireGear()" );
				pInterface->CPrint( "    MSG - ACQUIREGEAR - Invalid gear '%s'!", buf );
			}
			
			return false;
		}

		return true;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateAcquireGear()" );
		pInterface->CPrint( "    MSG - ACQUIREGEAR - No gear specified!" );
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateChangeWeapon
//
//  PURPOSE:	Make sure weapon message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateChangeWeapon( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !g_pWeaponDB )
		return true;

	if( cpMsgParams.m_nArgs > 1 )
	{
		char buf[256] = {0};
		for( int32 i = 0; i < cpMsgParams.m_nArgs - 1; ++i )
		{
			if( i > 0 )
				LTStrCat( buf, " ", LTARRAYSIZE(buf) );
			
			LTStrCat( buf, cpMsgParams.m_Args[i+1], LTARRAYSIZE(buf) );
		}

		if( !g_pWeaponDB->GetWeaponRecord( buf ) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - ValidateChangeWeapon()" );
				pInterface->CPrint( "    MSG - CHANGEWEAPON - Invalid weapon '%s'!", buf );
			}
			
			return false;
		}

		return true;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateChangeWeapon()" );
		pInterface->CPrint( "    MSG - CHANGEWEAPON - No weapon specified!" );
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateSlowMo
//
//  PURPOSE:	Make sure slowmo message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateSlowMo( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_nArgs > 1 )
	{
		CParsedMsg::CToken token( cpMsgParams.m_Args[1] );
		if( token == s_cTok_On )
		{
			HRECORD hSlowMoRecord = NULL;
			if( cpMsgParams.m_nArgs > 2 )
				hSlowMoRecord = DATABASE_CATEGORY( SlowMo ).GetRecordByName( cpMsgParams.m_Args[2] );
			if( !hSlowMoRecord )
			{
				if( CCommandMgrPlugin::s_bShowMsgErrors )
				{
					pInterface->ShowDebugWindow( true );
					pInterface->CPrint( "ERROR! - %s", __FUNCTION__ );
					pInterface->CPrint( "    MSG - SLOWMO - Invalid slowmo record '%s'!", cpMsgParams.m_Args[2] );
				}

				return false;
			}

			return true;
		}
		else if( token == s_cTok_Off )
		{
			return true;
		}
		else
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - %s", __FUNCTION__ );
				pInterface->CPrint( "    MSG - SLOWMO - Invalid On/Off parameter '%s'!", cpMsgParams.m_Args[1] );
			}

			return false;
		}
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - %s", __FUNCTION__ );
		pInterface->CPrint( "    MSG - SLOWMO - Invalid number of arguments!" );
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateAnimate
//
//  PURPOSE:	Make sure animate message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateAnimate( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_nArgs > 2 )
	{
		CParsedMsg::CToken cTok_Arg3( cpMsgParams.m_Args[2] );
		if( (cTok_Arg3 != s_cTok_Loop) && (cTok_Arg3 != s_cTok_Linger) )
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - %s", __FUNCTION__ );
				pInterface->CPrint( "    MSG - ANIMATE - Invalid optional paramater '%s'!", cpMsgParams.m_Args[3] );
			}

			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateExitLevel
//
//  PURPOSE:	Make sure exitlevel message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateExitLevel( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( cpMsgParams.m_nArgs > 1 )
	{
		if( CParsedMsg::CToken( cpMsgParams.m_Args[1]) == s_cTok_0 ||
			CParsedMsg::CToken( cpMsgParams.m_Args[1]) == s_cTok_1 )
		{
			return true;
		}
		else
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( true );
				pInterface->CPrint( "ERROR! - %s", __FUNCTION__ );
				pInterface->CPrint( "    MSG - EXITLEVEL - Invalid exitmission parameter '%s'!", cpMsgParams.m_Args[1] );
			}

			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateGotoMsg
//
//  PURPOSE:	Make sure goto message is good
//
// ----------------------------------------------------------------------- //

static bool ValidateGotoMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	const char *pszNodeName = cpMsgParams.m_Args[1];
	if( !CCommandMgrPlugin::DoesObjectExist( pInterface, pszNodeName ))
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Could not find object '%s'!", cpMsgParams.m_Args[1] );
		return false;
	}
	
	if( !LTStrIEquals( CCommandMgrPlugin::GetObjectClass( pInterface, pszNodeName ), "PlayerNodeGoto" ) )
	{
		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Object '%s' is not of type 'PlayerGotoNode'!", cpMsgParams.m_Args[1] );
		return false;
	}

	return true;
}


//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( CPlayerObj )

//					Message		Num Params	Validation FnPtr		Syntax

	ADD_MESSAGE( MISSION,			-1,		ValidateMissionMsg,		MSG_HANDLER( CPlayerObj, HandleMissionMsg ),		"MISSION <string id> [1 or 0]", "Sets the mission status text to the specified sting id. Use 0 to clear the text. The optional parameter controls the display of the mission status popup. Leaving this blank or setting it to 1 will display the popup, setting it to 0 will change the text without displaying the popup.", "msg player (MISSION Briefing_Docks 0)" )
	ADD_MESSAGE( OBJECTIVE,			-1,		ValidateObjectiveMsg,	MSG_HANDLER( CPlayerObj, HandleObjectiveMsg ),		"OBJECTIVE <ADD objective id> or <REMOVE>", "Sets or clears the current objective for the player.", "msg player (OBJECTIVE ADD Objective_01)" )
	ADD_MESSAGE( TRANSMISSION,		-1,		ValidateTransmissionMsg,MSG_HANDLER( CPlayerObj, HandleTransmissionMsg ),	"TRANSMISSION <transmission id> [ActivePlayerComm] [TeamNumber]", "Sends a text message to every player in the game. The string database entry specified by <transmission id> will be displayed. If [ActivePlayerComm] is 1, the transmission is labeled as coming from the active player. The optional [TeamNumber] parameter specifes which team will receive the message (0 for Team 1,1 for Team 2, or -1 for all players).", "msg player (TRANSMISSION 13570 0 1)" )
	ADD_MESSAGE( FLASHLIGHT,		-1,		ValidateFlashlightMsg,	MSG_HANDLER( CPlayerObj, HandleFlashlightMsg ),		"FLASHLIGHT <Enable/Disable> [FlickerDuration]", "Enables or disables the players flashlight. If a [FlickerDuration] is specified, the flashlight will flicker for this many seconds before going out or coming back on.", "msg player (FLASHLIGHT disable 1.5)" )
	ADD_MESSAGE( CROSSHAIR,			2,		ValidateCrosshairMsg,	MSG_HANDLER( CPlayerObj, HandleCrosshairMsg ),		"CROSSHAIR <On/Off>", "Enables or disables the players crosshar.", "msg player (CROSSHAIR off)" )
	ADD_MESSAGE( SCORE,				2,		NULL,					MSG_HANDLER( CPlayerObj, HandleScoreMsg ),			"SCORE <+/-amount>", "Modifies the players score by the specified amount. Only relevant in multiplayer games.", "msg player (SCORE 50)<BR>msg player (SCORE -25)" )
	ADD_MESSAGE( HEARTBEAT,			2,		NULL,					MSG_HANDLER( CPlayerObj, HandleHeartbeatMsg ),		"HEARTBEAT <duration> or <On/Off>", "Turns on the Heatbeat sound effect for the specified duration or toggles it on or off. If the HEARTBEAT ON is sent, the Heartbeat sound will continue to play until the play receives a HEARTBEAT OFF message.", "msg player (HEARTBEAT 2.5)<BR>msg player (HEARTBEAT ON)<BR>msg player (HEARTBEAT OFF)" )
	ADD_MESSAGE( BREATH,			2,		NULL,					MSG_HANDLER( CPlayerObj, HandleBreathMsg ),			"BREATH <duration> or <On/Off>", "Turns on the heavy breathing sound effect for the specified duration or toggles it on or off. If the BREATH ON is sent, the breathing sound will continue to play until the play receives a BREATH OFF message.", "msg player (BREATH 2.5)<BR>msg player (BREATH ON)<BR>msg player (BREATH OFF)" )
	ADD_MESSAGE( SIGNAL,			2,		NULL,					MSG_HANDLER( CPlayerObj, HandleSignalMsg ),			"SIGNAL <duration>", "Triggers an interference signal which flickers the player's flashlight and HUD for the specified duration.", "msg player (SIGNAL 4.0)" )
	ADD_MESSAGE( TEXT,				2,		NULL,					MSG_HANDLER( CPlayerObj, HandleTextMsg ),			"TEXT <text sequence>", "Triggers the player's interface to play the specified text sequence. The <text sequence> is the name of game database record under the Interface\\Credits\\Order category.", "msg player (TEXT End_Credits)" )
	ADD_MESSAGE( FADEIN,			2,		NULL,					MSG_HANDLER( CPlayerObj, HandleScreenFadeMsg ),		"FADEIN <time>", "Fades the player's screen from black to the normal view over the given time.", "msg player (FADEIN 2.0)" )
	ADD_MESSAGE( FADEOUT,			2,		NULL,					MSG_HANDLER( CPlayerObj, HandleScreenFadeMsg ),		"FADEOUT <time>", "Fades the player's screen from the normal view to black over the given time.", "msg player (FADEOUT 2.0)" )
	ADD_MESSAGE( MISSIONFAILED,		2,		NULL,					MSG_HANDLER( CPlayerObj, HandleMissionFailedMsg ),	"MISSIONFAILED <text id>", "In single player levels, causes the player to mission to fail. The <text id> specifies the string database entry to display on the failure screen.", "msg player (MISSIONFAILED Failure_TimeExpired)" )
	ADD_MESSAGE( REMOVEBODIES,		1,		NULL,					MSG_HANDLER( CPlayerObj, HandleRemoveBodiesMsg ),	"REMOVEBODIES", "Removes the any dead bodies currently found in the level.", "msg player REMOVEBODIES" )
	ADD_MESSAGE( REMOVEALLBADAI,	1,		NULL,					MSG_HANDLER( CPlayerObj, HandleRemoveAllBadAIMsg ),	"REMOVEALLBADAI", "Removes from the game all enemy AI currently in the game. Does not prevent new AI from spawning in.", "msg player REMOVEALLBADAI" )
	ADD_MESSAGE( FACEOBJECT,		2,		NULL,					MSG_HANDLER( CPlayerObj, HandleFaceObjectMsg ),		"FACEOBJECT <object>", "Forces the player to rotate to face the named object.", "msg player (FACEOBJECT MonitorScreen01)" )
	ADD_MESSAGE( RESETINVENTORY,	1,		NULL,					MSG_HANDLER( CPlayerObj, HandleResetInventoryMsg ),	"RESETINVENTORY", "Removes the players weapons and ammunition.", "msg player RESETINVENTORY" )
	ADD_MESSAGE( ACQUIREWEAPON,		-1,		ValidateAcquireWeapon,	MSG_HANDLER( CPlayerObj, HandleAcquireWeaponMsg ),	"ACQUIREWEAPON <weapon>", "Gives the player the named weapon.", "msg player (ACQUIREWEAPON shotgun)" )	
	ADD_MESSAGE( ACQUIREAMMO,		-1,		ValidateAcquireAmmo,	MSG_HANDLER( CPlayerObj, HandleAcquireAmmoMsg ),	"ACQUIREAMMO <ammo>", "Gives the player a standard amount of the named ammuntion.", "msg player (ACQUIREAMMO shotgun)" )	
	ADD_MESSAGE( ACQUIREMOD,		-1,		ValidateAcquireMod,		MSG_HANDLER( CPlayerObj, HandleAcquireModMsg ),		"ACQUIREMOD <mod>", "Gives the player the named weapon modification. Not currently supported.", "Unsupported" )
	ADD_MESSAGE( ACQUIREGEAR,		-1,		ValidateAcquireGear,	MSG_HANDLER( CPlayerObj, HandleAcquireGearMsg ),	"ACQUIREGEAR <gear>", "Gives the player the named gear item.", "msg player (ACQUIREGEAR Armor Light" )
	ADD_MESSAGE( CHANGEWEAPON,		-1,		ValidateChangeWeapon,	MSG_HANDLER( CPlayerObj, HandleChangeWeaponMsg ),	"CHANGEWEAPON <weapon>", "Forces the player to select the named weapon.", "msg player (CHANGEWEAPON Unarmed)" )
	ADD_MESSAGE( LASTWEAPON,		1,		NULL,					MSG_HANDLER( CPlayerObj, HandleLastWeaponMsg ),		"LASTWEAPON", "Forces the player to select the weapon they had selected prior to their current weapon.", "msg player LASTWEAPON" )
	ADD_MESSAGE( FULLHEALTH,		1,		NULL,					MSG_HANDLER( CPlayerObj, HandleFullHealthMsg ),		"FULLHEALTH", "Completely heals the player.", "msg player FULLHEALTH" )
	ADD_MESSAGE( DISMOUNT,			1,		NULL,					MSG_HANDLER( CPlayerObj, HandleCancelLureMsg ),		"DISMOUNT", "The dismount command is obsolete and no longer supported.", "" )
	ADD_MESSAGE( FOLLOWLURE,		2,		NULL,					MSG_HANDLER( CPlayerObj, HandleFollowLureMsg ),		"FOLLOWLURE <PlayerLure>", "Attaches the player to a PlayerLure object.", "msg player (FOLLOWLURE CarLure)" )	
	ADD_MESSAGE( CANCELLURE,		1,		NULL,					MSG_HANDLER( CPlayerObj, HandleCancelLureMsg ),		"CANCELLURE", "Detaches the player from any PlayerLure object he is attached to.", "msg player CANCELLURE" )	
	ADD_MESSAGE( OVERLAY,			3,		NULL,					MSG_HANDLER( CPlayerObj, HandleOverlayMsg ),		"OVERLAY <overlay name> <ON | OFF>", "Displays or hides a mask over the game play. This is an outdated system to handle first person visual effects. The overlays are specified in the Game Database under the Client\\Overlay category.", "msg player (OVERLAY Binoculars ON)" )
	ADD_MESSAGE( CHANGEMODEL,		2,		NULL,					MSG_HANDLER( CPlayerObj, HandleChangeModelMsg ),	"ChangeModel <modelname>", "The change model command is obsolete and unsupported.", "" )
	ADD_MESSAGE( SIMULATIONTIMER,	3,		NULL,					MSG_HANDLER( CPlayerObj, HandleSimulationTimerMsg ),"SimulationTimer <numerator> <denominator>", "Set simulation timer relative to real time.", "Sets simulation timer to 50% of real time: msg player (simulationtimer 1 2)" )
	ADD_MESSAGE( PLAYERTIMER,		3,		NULL,					MSG_HANDLER( CPlayerObj, HandlePlayerTimerMsg ),	"PlayerTimer <numerator> <denominator>", "Set player timer relative to simulation timer.", "Sets player timer to 200% of simulation timer: msg player (playertimer 2 1)" )
	ADD_MESSAGE( SLOWMO,			-1,		ValidateSlowMo,			MSG_HANDLER( CPlayerObj, HandleSlowMoMsg ),			"SlowMo <<On <SlowMoRecordName>>|<Off>", "Puts player into slowmo mode using settings specified in record named SlowMoRecordName.", "Puts player into default slowmo mode:  msg player (slowmo default)" )
	ADD_MESSAGE( MIXER,				-1,		NULL,					MSG_HANDLER( CPlayerObj, HandleMixerMsg ),			"MIXER [KillTemp] <mixer name> [fadetime]", "Applies the named mixer to the sound system. If the optional KillTemp parameter is included, the named mixer is removed instead. The fadetime overrides the default crossfade time.", "msg player (MIXER CinematicMixer)<BR>msg player (MIXER KillTemp CinematicMixer)<BR>msg player (MIXER CinematicMixer 2700)" )
	ADD_MESSAGE( SOUNDFILTER,		-1,		NULL,					MSG_HANDLER( CPlayerObj, HandleSoundFilterMsg ),	"SOUNDFILTER <filter name/OFF>", "Turns on/off a sound filter. If a filter name is provied, that sound filter is turned on. If the OFF parameter is used, this will turn off the current sound filter.", "msg player (SOUNDFILTER Alley)<BR>msg player (SOUNDFILTER OFF)" )
	ADD_MESSAGE( RESTARTLEVEL,		1,		NULL,					MSG_HANDLER( CPlayerObj, HandleRestartLevelMsg ),	"RESTARTLEVEL", "Restarts the current level.", "Restartlevel" )
	ADD_MESSAGE_ARG_RANGE( ANIMATE,	2,	3,	ValidateAnimate,		MSG_HANDLER( CPlayerObj, HandleAnimateMsg ),		"ANIMATE <animation or stop> [Loop or Linger]", "Plays the specified animation on the player or stops the player from animating.", "msg Player (ANIMATE DSlumpB)" )
	ADD_MESSAGE( ANIMATECAMROT,		2,		NULL,					MSG_HANDLER( CPlayerObj, HandleAnimateCamRotMsg ),	"ANIMATECAMROT <1 or 0>", "If set to true this animates the PlayerCamera rotation with the PlayerBody animation", "msg Player (ANIMATECAMROT 1)" )
	ADD_MESSAGE( WEIGHTSET,			2,		NULL,					MSG_HANDLER( CPlayerObj, HandleWeightSetMsg ),		"WEIGHTSET <weight_set>", "Set the physics weight set for the player object.", "msg player (WEIGHTSET RigidBody)" )
	ADD_MESSAGE( EXITLEVEL,			-1,		ValidateExitLevel,		MSG_HANDLER( CPlayerObj, HandleExitLevelMsg ),		"EXITLEVEL [nextmission]", "Exits the level to the next level or optionally the next mission.", "msg player (exitlevel 1)" )
//	ADD_MESSAGE( SONIC,				-1,		NULL,					MSG_HANDLER( CPlayerObj, HandleSonicMsg ),			"SONIC [command] [value]", "Adjusts Sonic data for the player.", "msg Player (SONIC SkillInc Blast)" )
//	ADD_MESSAGE( CARRY,				2,		NULL,					MSG_HANDLER( CPlayerObj, HandleCarryMsg ),			"CARRY <prop | none>", "Set the player in the carrying state and set the object they are carrying.", "msg Player (CARRY Jin)" )
	ADD_MESSAGE( GOTO,				2,		ValidateGotoMsg,		MSG_HANDLER( CPlayerObj, HandleGotoMsg ),			"GOTO <PlayerGotoNode>", "Specifies a node the player should move to.", "msg Player (GOTO PlayerGotoNode00)" )
	ADD_MESSAGE( WEAPONEFFECT,		-1,		NULL,					MSG_HANDLER( CPlayerObj, HandleWeaponEffectMsg ),	"WEAPONEFFECT <ClientFX name> [<Socket name>,LEFT,RIGHT]", "Spawns the specified ClientFX attached to the player's current weapon using the optionally specified socket.  If no socket is specified, LEFT or RIGHT (default) can be used to specify which weapon to attach to.", "msg Player (WEAPONEFFECT SomeCoolFX MyFXSocket)" )
	ADD_MESSAGE( PLAYERMOVEMENT,	2,		NULL,					MSG_HANDLER( CPlayerObj, HandlePlayerMovementMsg ),	"PLAYERMOVEMENT <1 or 0>", "If you want to take the control of the movement away from the player set this to 0.  You MUST set this back to 1 in order to allow the player to move again.", "msg Player (PLAYERMOVEMENT 0)" )
	ADD_MESSAGE( KILLEARRINGEFFECT,	-1,		NULL,					MSG_HANDLER( CPlayerObj, HandlePlayerEarRingOffMsg ),	"KILLEARRINGEFFECT", "Terminates the ear-ringing effect if it is active.", "msg Player KILLEARRINGEFFECT" )
CMDMGR_END_REGISTER_CLASS( CPlayerObj, CCharacter )

// Defines...

#define MAX_AIR_LEVEL						100.0f
#define DEFAULT_FRICTION					5.0f


// How far off the player can be between the server and client
#define DEFAULT_LEASHLEN					16.0f

// How far out to let it interpolate the position
#define DEFAULT_LEASHSPRING					50.0f

// How fast to interpolate between the postions (higher = faster)
#define	DEFAULT_LEASHSPRINGRATE				0.3f

// scale factor for adjusting the leash length to account for the player's velocity
#define DEFAULT_LEASHSCALE					0.14f

#define CONSOLE_COMMAND_LEASH_LENGTH		"LeashLen"
#define CONSOLE_COMMAND_LEASH_SPRING		"LeashSpring"
#define CONSOLE_COMMAND_LEASH_SCALE			"LeashScale"
#define CONSOLE_COMMAND_LEASH_SPRING_RATE	"LeashSpringRate"
#define CONSOLE_COMMAND_MOVE_ACCEL			"RunAccel"
#define CONSOLE_COMMAND_JUMP_VEL			"JumpSpeed"
#define CONSOLE_COMMAND_SWIM_VEL			"SwimVel"
#define CONSOLE_COMMAND_LADDER_VEL			"LadderVel"
#define CONSOLE_COMMAND_RUNSPEEDMUL			"RunSpeedMul"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::CPlayerObj
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

CPlayerObj::CPlayerObj() 
:	CCharacter					( ),
	m_nMotionStatus				( MS_NONE ),
	m_nWeaponStatus				( WS_NONE ),
	m_ePlayerLean				( PL_CENTER ),
	m_bFirstUpdate				( true ),
	m_bNewMission				( true ),
	m_bForceUpdateInterface		( false ),
	m_nCurContainers			( 0 ),
	m_fOldHitPts				( -1 ),
	m_fOldArmor					( -1 ),
	m_fOldAirLevel				( MAX_AIR_LEVEL ),
	m_fAirLevel					( MAX_AIR_LEVEL ),
	m_hClient					( NULL ),
	m_fGravity					( DEFAULT_PLAYER_GRAVITY ),
	m_fLeashLen					( DEFAULT_LEASHLEN ),
	m_fLeashScale				( DEFAULT_LEASHSCALE ),
	m_ePlayerState				( ePlayerState_None ),
	m_bGodMode					( false ),
	m_bAllowInput				( true ),
	m_bCinematicInvulnerability	( false ),
	m_b3rdPersonView			( false ),
	m_eSpectatorMode			( eSpectatorMode_None ),
	m_bInvisible				( false ),
	m_PStateChangeFlags			( PSTATE_INITIAL ),
	m_ePPhysicsModel			( PPM_NORMAL ),
	m_dwLastLoadFlags			( LOAD_NEW_GAME ),
	m_pClientSaveData			( NULL ),
	m_pnOldAmmo					( NULL ),
	m_bChatting					( false ),
	m_bSlowMoCharge				( false ),
	m_bForceDuck				( false ),
	m_bFadeInitiated			( false ),
	m_bFadeIn					( true ),
	m_fFadeTimeRemaining		( 0.0f ),
	m_bUseLeash					( false ),
	m_nLastPositionForcedTime	( 0 ),
	m_eLeanVisibleStimID		( kStimID_Unset ),
	m_bSendStartLevelCommand	( false ),
	m_nControlFlags				( 0 ),
	m_tfCameraView				( ),
	m_tfTrueCameraView				( ),
	m_vSavedVelocity			( 0.0f, 0.0f, 0.0f ),
	m_dwSavedObjectFlags		( 0 ),
	m_nLoadout					( 0 ),
	m_vLastClientPos			( 0.0f, 0.0f, 0.0f ),
	m_vClientCameraOffset		( 0.0f, 0.0f, 0.0f ),
	m_nClientCameraOffsetTimeReceivedMS( 0 ),
	m_vHeadOffset				( 0.0f, 0.0f, 0.0f ),
	m_fWeaponMoveMultiplier		(1.0f),
	m_hPreviousWeapon			( NULL ),
	m_hRequestedWeapon			( NULL ),
	m_bSliding					( false ),
	m_hTurret					( NULL ),
	m_hGrabConstraint			( INVALID_PHYSICS_BREAKABLE ),
	m_fGrabEndTime				( 0.0f ),
	m_pPendingGrab				( NULL ),
	m_bIsCrouched				( false ),
	m_dwLastForensicTypeMask	( 0 ),
	m_PlayerRigidBody			( ),
	m_bLockSpectatorMode		(false)
{
	// CCharacter members...

	m_bShortRecoil			= true;
	m_eSoundPriority		= SOUNDPRIORITY_PLAYER_HIGH;
	m_fSoundOuterRadius		= g_pServerDB->GetPlayerFloat(PLAYER_BUTE_SoundRadius);
	m_fSoundInnerRadius		= g_pServerDB->GetPlayerFloat(PLAYER_BUTE_SoundInnerRadius);
	m_nMPModelIndex		    = (uint8)-1;

	m_nClientId = INVALID_CLIENT;

	m_dwFlags				|= FLAG_FORCECLIENTUPDATE | FLAG_YROTATION;
	m_dwFlags				&= ~FLAG_GRAVITY; // This is controlled by the client.
	
	// Turn off touching until we are spawned (placed correctly) to avoid 
	// touching objects at 0,0,0.  These flags are added back to m_dwFlags
	// in Respawn, and applied when the player becomes alive.
	m_dwFlags				&= ~(FLAG_SOLID | FLAG_TOUCH_NOTIFY); 

	m_fWalkVel				= g_pServerDB->GetPlayerFloat(PLAYER_BUTE_WALKSPEED);
	m_fRunVel				= g_pServerDB->GetPlayerFloat(PLAYER_BUTE_RUNSPEED);
	m_fJumpVel				= g_pServerDB->GetPlayerFloat(PLAYER_BUTE_JUMPSPEED);
	m_fLadderVel			= g_pServerDB->GetPlayerFloat(PLAYER_BUTE_LADDERSPEED);
	m_fSwimVel				= g_pServerDB->GetPlayerFloat(PLAYER_BUTE_SWIMSPEED);
	m_fCrawlVel				= g_pServerDB->GetPlayerFloat(PLAYER_BUTE_CRAWLSPEED);
	m_vHeadOffset.y			= g_pServerDB->GetPlayerFloat(PLAYER_BUTE_EYEOFFSET);
	m_fGravity				= g_pServerDB->GetPlayerFloat(PLAYER_BUTE_GRAVITY);

	uint8 nNumAmmoTypes		= g_pWeaponDB->GetNumAmmo( );
	if (nNumAmmoTypes > 0)
	{
		m_pnOldAmmo = debug_newa(int, nNumAmmoTypes);
		memset(m_pnOldAmmo, 0, nNumAmmoTypes * sizeof( int ));
	}

	if(!s_vtBaseHealRate.IsInitted())
	{
		s_vtBaseHealRate.Init(g_pLTServer, "BaseHealRate", NULL, 0.2f);
	}
	if(!s_vtWalkHealRate.IsInitted())
	{
		s_vtWalkHealRate.Init(g_pLTServer, "WalkHealRate", NULL, 0.1f);
	}

	if(!s_vtDyingTimeGap.IsInitted())
	{
		s_vtDyingTimeGap.Init(g_pLTServer, "DyingTimeGap", NULL, 1.0f / 3.0f );
	}

	if (!s_vtAlwaysForceClientToServerPos.IsInitted())
	{
		s_vtAlwaysForceClientToServerPos.Init(g_pLTServer, "AlwaysForceClientToServerPos", NULL, 0.0f);
	}

	m_ActivationData.Init();

	m_nWeaponSoundLoopType = PSI_INVALID;
	m_nWeaponSoundLoopWeapon = 0;
	
	// Dead timer uses the interface since it counts the real time
	// we should stay dead.
	m_DyingTimer = RealTimeTimer::Instance();

	// Add this instance to a list of all PlayerObj's.
	m_lstPlayerObjs.push_back( this );

	m_Inventory.Init(this);

	m_fNextClientRotationUpdateTime = RealTimeTimer::Instance().GetTimerAccumulatedS();
	m_rLastRotation.Init( );

	m_fLastRespawnTime = -1.0f;
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

	// Make sure any active weapon sounds aren't looping
	SetWeaponSoundLooping(PSI_INVALID, m_nWeaponSoundLoopWeapon);

	if (m_pnOldAmmo)
	{
		debug_deletea(m_pnOldAmmo);
	}

	SetClientSaveData( NULL );

	// Erase this instance from the list of all PlayerObj's.
	PlayerObjList::iterator it = std::find( m_lstPlayerObjs.begin( ), m_lstPlayerObjs.end( ), this );
	if( it != m_lstPlayerObjs.end( ))
	{
		m_lstPlayerObjs.erase( it );
	}

	// Erase this instance from the list of dying players if it's in there.
	it = std::find( m_lstDyingPlayers.begin( ), m_lstDyingPlayers.end( ), this );
	if( it != m_lstDyingPlayers.end( ))
	{
		m_lstDyingPlayers.erase( it );
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CPlayerObj::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
					pStruct->m_Flags2 |= FLAG2_PLAYERCOLLIDE | FLAG2_PLAYERSTAIRSTEP;
					// 08/05/03 - KEF - Server dims aren't used in multiplayer.
					if (!IsMultiplayerGameServer())
						pStruct->m_Flags2 |= FLAG2_SERVERDIMS;
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
			SetPlayerState( ePlayerState_None, true );
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
	// If the damager was an AI, notify them that they damaged a player.
	if ( IsAI( damage.hDamager ) )
	{
		CAI* pAI = (CAI*)g_pLTServer->HandleToObject( damage.hDamager );
		if ( pAI )
		{
			pAI->HandleDamagedPlayer( m_hObject );
		}
	}

	//we're dead don't bother going any farther...
	if( !m_damage.GetCanDamage( ))
		return;

	// Check for FriendlyFire...
	if( IsMultiplayerGameServer( ) && (IsPlayer( damage.hDamager ) && (damage.hDamager != m_hObject) ))
	{
		if(  GameModeMgr::Instance( ).m_grbUseTeams) 
		{
			if (AreSameTeam( damage.hDamager, m_hObject ))
			{
				if (!GameModeMgr::Instance( ).m_grbFriendlyFire)
				{
					return;
				}
				if (!m_DamageMsgTimer.IsStarted() || m_DamageMsgTimer.IsTimedOut()) 
				{
					HRECORD hRec = DATABASE_CATEGORY( Broadcast ).GetRecordByName( "TeamDamage");
					PlayerBroadcastInfo pbi;
					pbi.nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hRec );
					pbi.nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, Priority);
					pbi.bDamageBroadcast = true;

					HandleBroadcast( pbi );
					m_DamageMsgTimer.Start(DATABASE_CATEGORY( BroadcastGlobal ).GETRECORDATTRIB( DATABASE_CATEGORY( BroadcastGlobal ).GetGlobalRecord() , DamageTimeOut ));
				}
			}
			else
			{
				if (!m_DamageMsgTimer.IsStarted() || m_DamageMsgTimer.IsTimedOut()) 
				{
					HRECORD hRec = DATABASE_CATEGORY( Broadcast ).GetRecordByName( "TakingFire");
					PlayerBroadcastInfo pbi;
					pbi.nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hRec );
					pbi.nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, Priority);
					pbi.bDamageBroadcast = true;

					HandleBroadcast( pbi );
					m_DamageMsgTimer.Start(DATABASE_CATEGORY( BroadcastGlobal ).GETRECORDATTRIB( DATABASE_CATEGORY( BroadcastGlobal ).GetGlobalRecord() , DamageTimeOut ));
				}
			}
		}
	}

	if ( !m_damage.IsCantDamageType(damage.eType) )
	{
		HandleShortRecoil();
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
	case MID_ADDWEAPON:
	case MID_AMMOBOX:
	case MID_ADDMOD:
	case MID_ADDGEAR:
		m_Inventory.HandlePickupMsg(hSender,pMsg);
		break;
	}

	return CCharacter::ObjectMessageFn(hSender, pMsg);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleObjectiveMsg()
//
//	PURPOSE:	Handle OBJECTIVE, OPTION, PARAMATER messages...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleObjectiveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() < 2 )
		return;

	bool bSend = false;
	HRECORD hRec = NULL;

	if (LTStrICmp(crParsedMsg.GetArg(1),"REMOVE") == 0)
	{
		bSend = true;
	}
	else if (LTStrICmp(crParsedMsg.GetArg(1),"ADD") == 0)
	{
		bSend = true;
		const char* pszObj = crParsedMsg.GetArg(2);

		if (!pszObj || !pszObj[0])
		{
			LTERROR("MSG OBJECTIVE ADD - no objective specified.");
		}

		hRec = DATABASE_CATEGORY( Objective ).GetRecordByName( pszObj );	

		if (!hRec)
		{
			LTERROR_PARAM1("MSG OBJECTIVE ADD - '%s' is not a valid objective.", pszObj);
		}
		
	}

	if (bSend)
	{
		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_EVENT);
		cClientMsg.Writeuint8(kPEObjective);
		cClientMsg.WriteDatabaseRecord(g_pLTDatabase,hRec);
		g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
	}

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMissionMsg()
//
//	PURPOSE:	Handle MISSION messages...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleMissionMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() < 2 || crParsedMsg.GetArgCount() > 3 )
		return;

	bool bShow = true;
	HRECORD hRec = NULL;

	if (LTStrICmp(crParsedMsg.GetArg(2),"0") == 0)
	{
		bShow = false;
	}

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPEMission);
	cClientMsg.Writeuint32(IndexFromStringID(crParsedMsg.GetArg(1)));
	cClientMsg.Writebool(bShow);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);

}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleChangeModelMsg()
//
//	PURPOSE:	Process an ChangeModel message
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleChangeModelMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	ModelsDB::HMODEL hModel = g_pModelsDB->GetModelByRecordName(crParsedMsg.GetArg(1).c_str());
    if (!hModel )
	{
		g_pLTServer->CPrint("CPlayerObj::HandleChangeModelMsg() : %s is not a valid model name.",crParsedMsg.GetArg(1).c_str());
		return;
	}

	SetModel( hModel );
	ResetModel();
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMixerMsg()
//
//	PURPOSE:	Handle a MIXER message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleMixerMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Send message to client containing the mixer trigger information


	// okay, i'm not 100% sure about this 'recreatemsg' thing but
	// i've copied this from HandleMusicMsg .. if it seems weird
	// to you, point this out to TERRY (J)..
	char szMsgBuf[256] = {0};
	crParsedMsg.ReCreateMsg( szMsgBuf, ARRAY_LEN(szMsgBuf), 0 );

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_MIXER );
	cMsg.WriteString( szMsgBuf );
	g_pLTServer->SendToClient( cMsg.Read(), GetClient( ), MESSAGE_GUARANTEED );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleSoundFilterMsg()
//
//	PURPOSE:	Handle a sound filter message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleSoundFilterMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Send message to client containing the mixer trigger information

	char szMsgBuf[256] = {0};
	crParsedMsg.ReCreateMsg( szMsgBuf, ARRAY_LEN(szMsgBuf), 0 );

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SOUND_FILTER );
	cMsg.WriteString( szMsgBuf );
	g_pLTServer->SendToClient( cMsg.Read(), GetClient( ), MESSAGE_GUARANTEED );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleTransmissionMsg()
//
//	PURPOSE:	Handle a TRANSMISSION message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleTransmissionMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() < 2 )
		return;

	const char* pszStringID = crParsedMsg.GetArg(1).c_str();

	// See if this was a communication type of transmission.  This means we will put the 
	// active player's name in front of it.
	bool bActivePlayerComm = false;
	if( crParsedMsg.GetArgCount( ) >= 3 )
	{
		bActivePlayerComm = !!atoi( crParsedMsg.GetArg( 2 ));
	}
	uint8 nTeam = INVALID_TEAM;
	if( crParsedMsg.GetArgCount( ) >= 4 )
	{
		nTeam= (uint8)atoi( crParsedMsg.GetArg( 3 ));
	}

	uint32 nSound = 0;
	if( crParsedMsg.GetArgCount( ) >= 5 )
	{
		nSound = atoi( crParsedMsg.GetArg( 4 ));
	}

	// If this is a communication transmission, get the activeplayer's client id and send it down.
	uint32 nActivePlayerClientId = (uint32)-1;
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
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPETransmission);
	cClientMsg.Writeuint32(nActivePlayerClientId);
	cClientMsg.Writeuint8( 1 );
	cClientMsg.Writeuint32(IndexFromStringID(pszStringID));
	cClientMsg.Writeuint32(nSound);
	cClientMsg.Writeuint8(nTeam);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleFlashlightMsg()
//
//	PURPOSE:	Handle a Flashlight message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleFlashlightMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() < 2 )
		return;

	bool bEnable;

	if (LTStrICmp(crParsedMsg.GetArg(1),"Enable") == 0)
		bEnable = true;
	else if (LTStrICmp(crParsedMsg.GetArg(1),"Disable") == 0)
		bEnable = false;
	else
		return;

	float fFlickerDuration = 0.0f;

	if (crParsedMsg.GetArgCount() >= 3) 
	{
		fFlickerDuration = (float)atof(crParsedMsg.GetArg(2));
	}


	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPEFlashlight);
	cClientMsg.Writebool(bEnable);
	cClientMsg.Writefloat(fFlickerDuration);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleCrosshairMsg()
//
//	PURPOSE:	Handle a Crosshair message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleCrosshairMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() < 2 )
		return;

	bool bEnable;

	if (LTStrICmp(crParsedMsg.GetArg(1),"On") == 0)
		bEnable = true;
	else if (LTStrICmp(crParsedMsg.GetArg(1),"Off") == 0)
		bEnable = false;
	else
		return;



	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPECrosshair);
	cClientMsg.Writebool(bEnable);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleHeartbeatMsg()
//
//	PURPOSE:	Handle a Heartbeat message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleHeartbeatMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() != 2 )
		return;

	bool bEnable = true;
	float fDuration = 0.0f;

	if (LTStrICmp(crParsedMsg.GetArg(1),"On") == 0)
		bEnable = true;
	else if (LTStrICmp(crParsedMsg.GetArg(1),"Off") == 0)
		bEnable = false;
	else
	{
		fDuration = (float)atof(crParsedMsg.GetArg(1));
	}


	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPEHeartBeat);
	cClientMsg.Writebool(bEnable);
	cClientMsg.Writefloat(fDuration);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleBreathMsg()
//
//	PURPOSE:	Handle a Breath message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleBreathMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() != 2 )
		return;

	bool bEnable = true;
	float fDuration = 0.0f;

	if (LTStrICmp(crParsedMsg.GetArg(1),"On") == 0)
		bEnable = true;
	else if (LTStrICmp(crParsedMsg.GetArg(1),"Off") == 0)
		bEnable = false;
	else
	{
		fDuration = (float)atof(crParsedMsg.GetArg(1));
	}


	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPEBreathing);
	cClientMsg.Writebool(bEnable);
	cClientMsg.Writefloat(fDuration);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleSignalMsg()
//
//	PURPOSE:	Handle a SIGNAL message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleSignalMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() < 2 )
		return;

	float fDuration = (float)atof(crParsedMsg.GetArg(1));

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPESignal);
	cClientMsg.Writefloat(fDuration);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleOverlayMsg()
//
//	PURPOSE:	Handle a OVERLAY message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleOverlayMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( IsMultiplayerGameServer( ) )
		return;

	if( crParsedMsg.GetArgCount() < 3 )
		return;


	uint8  nMsg = 255;

	if (crParsedMsg.GetArg(2) == s_cTok_Off)
		nMsg = 0;
	else if (crParsedMsg.GetArg(2) == s_cTok_On)
		nMsg = 1;


	if(nMsg < 255)
	{
		//add or remove the overlay from our list
		if (nMsg)
		{
			m_ActiveOverlays.insert(crParsedMsg.GetArg(1).c_str());
		}
		else
		{
			m_ActiveOverlays.erase(crParsedMsg.GetArg(1).c_str());
		}
		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_EVENT);
		cClientMsg.Writeuint8(kPEOverlay);
		cClientMsg.WriteString(crParsedMsg.GetArg(1).c_str());
		cClientMsg.Writeuint8(nMsg);
		g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);

	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeightSetMsg()
//
//	PURPOSE:	Handle a WEIGHTSET message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleWeightSetMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount( ) == 2 )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PLAYER_BODY );
		cMsg.Writeuint8( kPlayerBodyWeightSet );
		cMsg.WriteString( crParsedMsg.GetArg( 1 ).c_str( ));
		g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleExitLevelMsg()
//
//	PURPOSE:	Handle a EXITLEVEL message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleExitLevelMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Get the optional exitmission parameter.
	bool bExitMission = false;
	if( crParsedMsg.GetArgCount( ) > 1 )
	{
		bExitMission = !!atoi( crParsedMsg.GetArg( 1 ).c_str( ));
	}

	// Go to next mission or next level in the same mission.
	if( bExitMission )
	{
		g_pServerMissionMgr->NextMission( );
	}
	else
	{
		g_pServerMissionMgr->ExitLevelSwitch( );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleSonicMsg()
//
//	PURPOSE:	Handle a SONIC message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleSonicMsg( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
	m_iSonicData.HandleMessage( crParsedMsg );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleCarryMsg()
//
//	PURPOSE:	Handle a CARRY message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleCarryMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_None( "None" );

	// Set the player up to carry the specified object...

	if( crParsedMsg.GetArgCount( ) < 2 )
		return;

	if( crParsedMsg.GetArg( 1 ) == s_cTok_None )
	{
		// Remove attachment and stop carrying object...

		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8( MID_PLAYER_EVENT );
		cClientMsg.Writeuint8( kPECarry );
		cClientMsg.WriteDatabaseRecord( g_pLTDatabase, NULL );
		g_pLTServer->SendToClient( cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED );
	}
	else
	{
		const char *pszCarryProp = crParsedMsg.GetArg( 1 );
		if( !pszCarryProp || !pszCarryProp[0] )
			return;

		PropsDB::HPROP hProp = g_pPropsDB->GetPropByRecordName( pszCarryProp );
		if( hProp )
		{
			// Send message to Client so they can begin carrying...

			CAutoMessage cClientMsg;
			cClientMsg.Writeuint8( MID_PLAYER_EVENT );
			cClientMsg.Writeuint8( kPECarry );
			cClientMsg.WriteDatabaseRecord( g_pLTDatabase, hProp );
			g_pLTServer->SendToClient( cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED );
		}
	}	
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleGotoMsg()
//
//	PURPOSE:	Handle a GOTO message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleGotoMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	HOBJECT hNode;
	if( LT_OK != FindNamedObject( crParsedMsg.GetArg(1), hNode ) )
	{
		LTERROR( "CPlayerObj::HandleGotoMsg: Could not find node" );
		return;
	}

	PlayerNodeGoto* pNode = dynamic_cast<PlayerNodeGoto*>(g_pLTServer->HandleToObject( hNode ));
	if( !pNode )
		return;

	// Send the node the player needs to visit...
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PLAYER_GOTO_NODE );
	cMsg.WriteObject( hNode );
	pNode->WriteNodeData( cMsg );
	g_pLTServer->SendToClient( cMsg.Read(), GetClient( ), MESSAGE_GUARANTEED );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandlePlayerMovementMsg()
//
//	PURPOSE:	Handle a PLAYERMOVEMENT message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandlePlayerMovementMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount( ) == 2 )
	{
		bool bAllowPlayerMovement = (crParsedMsg.GetArg( 1 ) == s_cTok_1);
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_CLIENT_PLAYER_UPDATE );
		cMsg.WriteBits( PSTATE_MOVEMENT, PSTATE_NUMBITS );
		cMsg.Writebool( bAllowPlayerMovement );
		g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandlePlayerEarRingOffMsg()
//
//	PURPOSE:	Handle a KILLEARRINGEFFECT message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandlePlayerEarRingOffMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SOUND_MISC );
	cMsg.Writeuint8( MID_SOUND_MISC_KILL_EARRING_EFFECT );
	g_pLTServer->SendToClient( cMsg.Read(), GetClient( ), MESSAGE_GUARANTEED );
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponEffectMsg()
//
//	PURPOSE:	Handle a WEAPONEFFECT message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponEffectMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if (crParsedMsg.GetArgCount() < 2)
		return;

	// Get the name of the ClientFX to spawn.
	const char *pszFX = crParsedMsg.GetArg(1);
	if (!pszFX || !pszFX[0])
		return;

	// Get the optional socket to attach to.
	const char *pszSocket = (crParsedMsg.GetArgCount() < 3) ? "RIGHT" : crParsedMsg.GetArg(2);

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPEWeaponEffect);
	cClientMsg.WriteString(pszFX);
	cClientMsg.WriteString(pszSocket);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::LoadOverlays()
//
//	PURPOSE:	
//
// --------------------------------------------------------------------------- //

void CPlayerObj::LoadOverlays(ILTMessage_Read *pMsg)
{
	char szName[128];
	m_ActiveOverlays.clear();
	uint16 num = pMsg->Readuint16();
	for (uint16 i = 0; i < num; i++)
	{

		pMsg->ReadString(szName,LTARRAYSIZE(szName));
		m_ActiveOverlays.insert(szName);

		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_EVENT);
		cClientMsg.Writeuint8(kPEOverlay);
		cClientMsg.WriteString(szName);
		cClientMsg.Writeuint8(1);
		g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);

	}
}

void CPlayerObj::SaveOverlays(ILTMessage_Write *pMsg)
{
	pMsg->Writeuint16(m_ActiveOverlays.size());
	
	StringSet::iterator iter = m_ActiveOverlays.begin();
	while (iter != m_ActiveOverlays.end())
	{
		pMsg->WriteString(iter->c_str());
		iter++;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleTextMsg()
//
//	PURPOSE:	Handle a TEXT message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleTextMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() < 2 )
		return;

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_TEXT);
	cClientMsg.WriteString(crParsedMsg.GetArg(1));
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleScreenFadeMsg()
//
//	PURPOSE:	Handle FADEIN and FADEOUT messages...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleScreenFadeMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_FadeIn( "FADEIN" );

	if( crParsedMsg.GetArgCount() < 2 )
		return;

	// Fading in or out?
	m_bFadeIn = (crParsedMsg.GetArg(0) == s_cTok_FadeIn );
	
	m_fFadeTimeRemaining = (float)atof(crParsedMsg.GetArg(1));
	if( m_fFadeTimeRemaining < 0.1f )
		return;

	m_bFadeInitiated = true;

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cClientMsg.Writeuint8(IC_FADE_SCREEN_ID);
	cClientMsg.Writeuint8(m_bFadeIn ? 1 : 0);
	cClientMsg.Writebool(false);
	cClientMsg.Writefloat(m_fFadeTimeRemaining);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleMissionFailedMsg()
//
//	PURPOSE:	Handle a MISSIONFAILED messages...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleMissionFailedMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Tell the clients to draw the mission failed screen

	if( crParsedMsg.GetArgCount() < 2 )
		return;

	uint32 dwTextId = (uint32) atol(crParsedMsg.GetArg(1));

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cClientMsg.Writeuint8(IC_MISSION_FAILED_ID);
	cClientMsg.Writeuint8(0);
	cClientMsg.Writeuint8(0);
	cClientMsg.Writeuint32( dwTextId );
	g_pLTServer->SendToClient(cClientMsg.Read(), NULL, MESSAGE_GUARANTEED);


	//Tell the MissionMgr we failed (this pauses the game)
	g_pServerMissionMgr->SetMissionFailed(true);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleRemoveBodiesMsg()
//
//	PURPOSE:	Handle a REMOVEBODIES message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleRemoveBodiesMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	CCharacter::CharacterList::const_iterator iter;
	for( iter = CCharacter::GetBodyList().begin(); iter != CCharacter::GetBodyList().end(); ++iter )
	{
		CCharacter *pBody = *iter;
		g_pLTServer->RemoveObject( pBody->m_hObject );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleRemoveAllBadAIMsg()
//
//	PURPOSE:	Handle a REMOVEALLBADAI message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleRemoveAllBadAIMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	CAI::AIList::const_iterator iter;
	for( iter = CAI::GetAIList().begin(); iter != CAI::GetAIList().end(); ++iter )
	{
		CAI *pAI = *iter;
		if( g_pCharacterDB->GetStance( m_eAlignment, pAI->GetAlignment() ) == kCharStance_Hate )
		{
			g_pCmdMgr->QueueMessage( this, pAI, "REMOVE" );
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleFaceObjectMsg()
//
//	PURPOSE:	Handle a FACEOBJECT message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleFaceObjectMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		const char* pObjName = crParsedMsg.GetArg(1);
		if( pObjName )
		{
			ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
			int numObjects;

			g_pLTServer->FindNamedObjects(const_cast<char*>(pObjName), objArray);
			numObjects = objArray.NumObjects();

			if( numObjects == 0 )
				return;

			HOBJECT hObj = objArray.GetObject( 0 );
			if( hObj )
			{
				// Look at the object...

				LTVector vDir, vPos, vTargetPos;
				g_pLTServer->GetObjectPos(m_hObject, &vPos);
				g_pLTServer->GetObjectPos(hObj, &vTargetPos);

				vTargetPos.y = vPos.y; // Don't look up/down.

				vDir = (vTargetPos - vPos).GetUnit();

				LTRotation rRot(vDir, LTVector(0.0f, 1.0f, 0.0f));
				g_pLTServer->SetObjectRotation(m_hObject, rRot);
				m_tfCameraView.m_rRot = rRot;
				m_tfTrueCameraView.m_rRot = rRot;
			}
		}
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleResetInventoryMsg()
//
//	PURPOSE:	Handle a RESETINVENTORY message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleResetInventoryMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	ResetInventory( false );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleAcquireWeaponMsg()
//
//	PURPOSE:	Handle a ACQUIREWEAPON message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleAcquireWeaponMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	char szWeaponName[32] = {0};
	
	crParsedMsg.ReCreateMsg( szWeaponName, ARRAY_LEN(szWeaponName), 1 );
	AcquireWeapon( szWeaponName );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleAcquireAmmoMsg()
//
//	PURPOSE:	Handle a ACQUIREAMMO message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleAcquireAmmoMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	char szAmmoName[32] = {0};

	crParsedMsg.ReCreateMsg( szAmmoName, ARRAY_LEN(szAmmoName), 1 );
	AcquireAmmo( szAmmoName );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleAcquireModMsg()
//
//	PURPOSE:	Handle a ACQUIREMOD message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleAcquireModMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	char szModName[32] = {0};

	crParsedMsg.ReCreateMsg( szModName, ARRAY_LEN(szModName), 1 );
	AcquireMod( szModName );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleAcquireGearMsg()
//
//	PURPOSE:	Handle a ACQUIREGEAR message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleAcquireGearMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	char szGearName[32] = {0};

	crParsedMsg.ReCreateMsg( szGearName, ARRAY_LEN(szGearName), 1 );
	AcquireGear( szGearName );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleChangeWeaponMsg()
//
//	PURPOSE:	Handle a CHANGEWEAPON message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleChangeWeaponMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	ChangeToWeapon( crParsedMsg.GetArg( 1 ));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleLastWeaponMsg()
//
//	PURPOSE:	Handle a LASTWEAPON message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleLastWeaponMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	ChangeToLastWeapon();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleFullHealthMsg()
//
//	PURPOSE:	Handle a FULLHEALTH message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleFullHealthMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	HealCheat();
}	

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleDismountMsg()
//
//	PURPOSE:	Handle a DISMOUNT message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleDismountMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( m_ePPhysicsModel != PPM_NORMAL )
	{
		SetPhysicsModel( PPM_NORMAL );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleFollowLureMsg()
//
//	PURPOSE:	Handle a FOLLOWLURE message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleFollowLureMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Check if they are cancelling the lure.
	if( crParsedMsg.GetArgCount() < 2 )
	{
		LTERROR( "CPlayerObj::ProcessCommandLure: No lure name specified." );
		return;
	}

	const char* pObjName = crParsedMsg.GetArg(1);
	if( !pObjName )
	{
		LTERROR( "CPlayerObj::ProcessCommandLure: Invalid lure name specified." );
		return;
	}

	// Find the lure objects.
	ObjArray <HOBJECT, MAX_OBJECT_ARRAY_SIZE> objArray;
	g_pLTServer->FindNamedObjects(const_cast<char *>(pObjName), objArray);
	int numObjects = objArray.NumObjects();

	// Check if the lure is missing.
	if( !numObjects )
	{
		LTERROR( "CPlayerObj::ProcessCommandLure: Lure object not found." );
		return;
	}

	// Get the first object.
	HOBJECT hLure = objArray.GetObject(0);
	if( !hLure )
	{
		LTERROR( "CPlayerObj::ProcessCommandLure: Corrupt lure object." );
		return;
	}

	// Convert the hobject to a playerlure.
	PlayerLure* pPlayerLure = dynamic_cast< PlayerLure* >( g_pLTServer->HandleToObject( hLure ));
	if( !pPlayerLure )
	{
		LTERROR( "CPlayerObj::ProcessCommandLure: Lure specified is not a PlayerLure." );
		return;
	}

	FollowLure( *pPlayerLure );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleCancelLureMsg()
//
//	PURPOSE:	Handle a CANCELLURE message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleCancelLureMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	StopFollowingLure( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleScoreMsg()
//
//	PURPOSE:	Handle a SCORE message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleScoreMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount( ) > 1 )
	{
		int32 nScore = atoi( crParsedMsg.GetArg( 1 ));
			
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
		if( pGameClientData )
			pGameClientData->GetPlayerScore( )->AddScore( nScore );
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleAnimateMsg()
//
//	PURPOSE:	Handle an ANIMATE message...
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleAnimateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Need to inform the client of the animation to play...
	if( crParsedMsg.GetArgCount( ) >= 2 )
	{
		if( crParsedMsg.GetArg( 1 ) == s_cTok_Stop )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_PLAYER_BODY );
			cMsg.Writeuint8( kPlayerBodyAnimateStop );
			g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );

			return;
		}

		// Make sure the animation actually exists on the model and just send the index if it does...
		HMODELANIM hAnim = INVALID_MODEL_ANIM;
		if( g_pModelLT->GetAnimIndex( m_hObject, crParsedMsg.GetArg( 1 ).c_str( ), hAnim ) == LT_OK )
		{
			PlayerBodyMessage eMsg = kPlayerBodyAnimate;
			if( crParsedMsg.GetArgCount( ) == 3 )
			{
				if( crParsedMsg.GetArg( 2 ) == s_cTok_Loop )
				{
					eMsg = kPlayerBodyAnimateLoop;
				}
				else if( crParsedMsg.GetArg( 2 ) == s_cTok_Linger )
				{
					eMsg = kPlayerBodyAnimateLinger;
				}
			}
			
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_PLAYER_BODY );
			cMsg.Writeuint8( eMsg );
			cMsg.Writeuint32( hAnim );
			g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleAnimateCamRotMsg()
//
//	PURPOSE:	Handle an ANIMATECAMROT message...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleAnimateCamRotMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount( ) == 2 )
	{
		bool bAnimateCamRot = !(crParsedMsg.GetArg( 1 ) == s_cTok_0);
		
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_PLAYER_BODY );
		cMsg.Writeuint8( kPlayerBodyAnimateCamRot );
		cMsg.Writebool( bAnimateCamRot );
		g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
	}
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
	pDamageStruct->fDuration *= GetSkillValue(eStaminaResistance);

	if ( 0.0f < pDamageStruct->fDuration )
	{
		// Check friendly fire.
		if( IsMultiplayerGameServer() && IsPlayer( pDamageStruct->hDamager) && pDamageStruct->hDamager != m_hObject && ( GameModeMgr::Instance( ).m_grbFriendlyFire ))
		{
			if (GameModeMgr::Instance( ).m_grbUseTeams && AreSameTeam(pDamageStruct->hDamager, m_hObject) )
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
			if( IsMultiplayerGameServer() && 
				GameModeMgr::Instance( ).m_grbUseTeams && 
				AreSameTeam(pDamageStruct->hDamager, m_hObject) && 
				(pDamageStruct->hDamager != m_hObject))
			{
				if (!GameModeMgr::Instance( ).m_grbFriendlyFire )
				{
					return false;
				}
				else
				{
					// Check if we should reflect some damage back on the damager.  Don't bother
					// if we can't take damage, which can happen if we're a dead body.
					if( m_damage.GetCanDamage( ) )
					{
						if (GameModeMgr::Instance( ).m_grfTeamReflectDamage > 0.0f)
						{
							// Reflect back a percentage of damage on the damager.  If the percentage is greater than 200, then do infinite damage.
							float fDamage = ( GameModeMgr::Instance( ).m_grfTeamReflectDamage > 2.0f ) ? DamageStruct::kInfiniteDamage : 
							( pDamageStruct->fDamage * GameModeMgr::Instance( ).m_grfTeamReflectDamage );
							DamageStruct damage = *pDamageStruct;
							damage.fDamage = fDamage;
							damage.hDamager = ( HOBJECT )pDamageStruct->hDamager;
							LTVector vDamagerPos, vVictimPos;
							g_pLTServer->GetObjectPos( pDamageStruct->hDamager, &vDamagerPos );
							g_pLTServer->GetObjectPos( m_hObject, &vVictimPos );
							LTVector vDir = vDamagerPos - vVictimPos;
							vDir.Normalize();
							damage.SetPositionalInfo( vDamagerPos, vDir );
							damage.DoDamage( pDamageStruct->hDamager, pDamageStruct->hDamager );
						}


					}
					//now adjust the damage based on the team damage percent
					pDamageStruct->fDamage *= GameModeMgr::Instance( ).m_grfTeamDamagePercent;
			}
			}

		}
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

	cs.bIsPlayer	= true;
	cs.nClientID	= m_nClientId;
	cs.ePlayerPhysicsModel	= m_ePPhysicsModel;
	cs.nMPModelIndex = m_nMPModelIndex;

	cs.SetChatting(m_bChatting);
	cs.SetSliding(m_bSliding);
	cs.SetHasSlowMoRecharge(m_bSlowMoCharge);
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
	if (!pStruct || !g_pServerDB || !g_pModelsDB)
		return;

	GameClientData* pGameClientData = ( GameClientData * )pStruct->m_UserData;
	m_nClientId = (uint8)g_pLTServer->GetClientID( pGameClientData->GetClient( ));

	if( !IsMultiplayerGameServer( ))
	{
		// Use the sp model.
		SetModel( g_pModelsDB->GetModelByRecordName( DEFAULT_PLAYERNAME ));
	}
	else
	{
		// Ask gameclientdata what model to use.
		SetModel( pGameClientData->GetMPModel( ));
		m_nMPModelIndex = pGameClientData->GetMPModelIndex( );
	}

	const char* pFilename = g_pModelsDB->GetModelFilename(GetModel( ));
	pStruct->SetFileName(pFilename);

	g_pModelsDB->CopyMaterialFilenames(GetModel( ), pStruct->m_Materials[0], LTARRAYSIZE( pStruct->m_Materials ), 
		LTARRAYSIZE( pStruct->m_Materials[0] ));

	pStruct->SetName(DEFAULT_PLAYERNAME);

	// Use the player group.
	pStruct->m_eGroup = PhysicsUtilities::ePhysicsGroup_UserPlayer;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::InitialUpdate
//
//	PURPOSE:	Handle initial Update
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::InitialUpdate(int nInfo)
{
	g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

	// Set up console vars used to tweak player movement...

	m_LeashLenTrack.Init(g_pLTServer, CONSOLE_COMMAND_LEASH_LENGTH, NULL, DEFAULT_LEASHLEN);
	m_LeashSpringTrack.Init(g_pLTServer, CONSOLE_COMMAND_LEASH_SPRING, NULL, DEFAULT_LEASHSPRING);
	m_LeashScaleTrack.Init(g_pLTServer, CONSOLE_COMMAND_LEASH_SCALE, NULL, DEFAULT_LEASHSCALE);
	m_LeashSpringRateTrack.Init(g_pLTServer, CONSOLE_COMMAND_LEASH_SPRING_RATE, NULL, DEFAULT_LEASHSPRINGRATE);
	m_BaseMoveAccelTrack.Init(g_pLTServer, CONSOLE_COMMAND_MOVE_ACCEL, NULL, 3000.0f);
	m_JumpVelMulTrack.Init(g_pLTServer, CONSOLE_COMMAND_JUMP_VEL, NULL, 0.81f);
	m_SwimVelTrack.Init(g_pLTServer, CONSOLE_COMMAND_SWIM_VEL, NULL, m_fSwimVel);
	m_LadderVelTrack.Init(g_pLTServer, CONSOLE_COMMAND_LADDER_VEL, NULL, m_fLadderVel);
	m_vtRunSpeedMul.Init(g_pLTServer, CONSOLE_COMMAND_RUNSPEEDMUL, NULL, 1.0f);

	m_tmrSlowMoHoldCharger.SetEngineTimer( RealTimeTimer::Instance( ));

	m_PlayerRigidBody.Init( m_hObject );

	if (nInfo == INITIALUPDATE_SAVEGAME) return true;

	// Create a child timer for this object off the simulation timer.
	EngineTimer engineTimer;
	engineTimer.CreateChildTimer( SimulationTimer::Instance( ));
	engineTimer.ApplyTimerToObject( m_hObject );

	SetNextUpdate(UPDATE_NEXT_FRAME);
	g_pLTServer->SetModelLooping(m_hObject, true);

	m_damage.SetMass(g_pModelsDB->GetModelMass(GetModel( )));

	SetPlayerAlignment();

	// If we can't respawn, then just keep the body around.
	m_bPermanentBody = !GameModeMgr::Instance().m_grbAllowRespawnFromDeath;

	ResetHealth();

	m_Inventory.SetCapacity( g_pWeaponDB->GetWeaponCapacity() );

	m_DamageMsgTimer.SetEngineTimer( RealTimeTimer::Instance() );
	s_RespawnMsgTimer.SetEngineTimer( RealTimeTimer::Instance() );

    return true;
}

// ----------------------------------------------------------------------- //
//	
//	ROUTINE:	CPlayerObj::SetPlayerAlignment
//
//	PURPOSE:	Set the alignment of the player based on eth AIName.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetPlayerAlignment()
{
	const char* pszAIName = g_pModelsDB->GetModelAIName( GetModel( ));
	SetAIAttributes( pszAIName );

	if ( m_pAIAttributes )
	{
		m_eAlignment = g_pCharacterDB->String2Alignment( m_pAIAttributes->strAlignment.c_str() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Update
//
//	PURPOSE:	Handle Update
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::Update()
{
	if (!g_pGameServerShell) return false;

#ifndef _FINAL
	if (m_cs.bIsDead)
	{
		LTVector vPos, vMin, vMax;
		g_pLTServer->GetWorldBox(vMin, vMax);
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		if (vPos.x < vMin.x || vPos.y < vMin.y || vPos.z < vMin.z ||
			vPos.x > vMax.x || vPos.y > vMax.y || vPos.z > vMax.z)
		{
			DebugCPrint(0,"%s - body outside of world at: (%0.2f,%0.2f,%0.2f)",__FUNCTION__,vPos.x,vPos.y,vPos.z);
		}

	}
#endif // _FINAL
	

	// We need to do this before checking our HCLIENT data member since
	// there are cases (load/save) where HCLIENT gets set to null, but
	// the player object is still valid (and needs to get update called
	// until the data member is set back to a valid value)...

	SetNextUpdate(UPDATE_NEXT_FRAME);

	if( !GetClient( ))
	{
		return false;
	}

	// [RP] 9/14/02 Don't officially update if the player hasn't respawned or his client hasn't loaded yet...
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
	if( !pGameClientData || !pGameClientData->IsClientInWorld() || GetPlayerState( ) == ePlayerState_None )
		return false;

	if( m_bFirstUpdate )
	{
		if (IsMultiplayerGameServer( ))
		{
			SendIDToClients();
		}

		// Check to see if we just reloaded a saved game...
		if (m_dwLastLoadFlags == LOAD_RESTORE_GAME || m_dwLastLoadFlags == LOAD_NEW_LEVEL || m_dwLastLoadFlags == LOAD_TRANSITION )
		{
			HandleGameRestore();
		}

		if	(m_hRequestedWeapon)
		{
			ChangeWeapon( m_hRequestedWeapon, true, NULL, true, false );
			m_hRequestedWeapon = NULL;
		}

		// Reset any ladder the player was on after loading a saved game.
		ActivateLadderOnLoad( );
		
		m_dwLastLoadFlags = 0;
	}


	if (m_bFirstUpdate)
	{
		UpdateClientPhysics(); // (to make sure they have their model around)
		TeleportClientToServerPos( true );
		SendStartLevelCommand();
	}
	else if (m_bSendStartLevelCommand)
	{
		SendStartLevelCommand();
	}

	// check to see if our weapon movement multiplier has changed
	HWEAPON hWeapon = m_Arsenal.GetCurWeaponRecord();
	float fWeaponMoveMult = 1.0f;
	if( hWeapon )
	{
		HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData( hWeapon, !USE_AI_DATA );

		fWeaponMoveMult = g_pWeaponDB->GetFloat( hWeaponData,WDB_WEAPON_fMovementMultiplier );
	}

	//factor in flag movement penalty, if applicable
	if (GetInventory()->GetCTFFlag())
	{
		float fFlagMult = GameModeMgr::Instance().m_grfCTFFlagMovementLimit;
		fWeaponMoveMult = min(fFlagMult,fWeaponMoveMult);
	}

		if( fWeaponMoveMult != m_fWeaponMoveMultiplier )
		{
			m_fWeaponMoveMultiplier = fWeaponMoveMult;
			m_PStateChangeFlags |= PSTATE_SPEEDS;
		}



	// Update our console vars (have they changed?)...
	UpdateConsoleVars();

	// Keep the client updated....
	UpdateClientPhysics();

	// Update the movement flags...
	UpdateCommands();

	// Update our movement...
	UpdateMovement();

	// Update air level...
	UpdateAirLevel();

	// Update our Health...
	UpdateHealth();

	// Update Interface...
	UpdateInterface( false );

	// Update any client-side special fx...
	UpdateSpecialFX();

	// Let the client know our position...
	UpdateClientViewPos();

	// Keep track of the current state of the fade on the client...
	UpdateClientFadeTime();

	// Update our sonic data
	m_iSonicData.Update( EngineTimer( m_hObject ).GetTimerElapsedS() );

	// Apply the constraint only after the dude starts ragdolling.
	if (m_pPendingGrab && m_pPendingGrab->IsRagdolling())
	{
		g_pLTBase->PhysicsSim()->AddBreakableToSimulation(m_hGrabConstraint);
		m_pPendingGrab = NULL;
	}

	// Release our grabbed ragdoll if time is up.
	if ((m_hGrabConstraint != INVALID_PHYSICS_BREAKABLE) && (g_pLTServer->GetTime() >= m_fGrabEndTime))
	{
		g_pLTBase->PhysicsSim()->RemoveBreakableFromSimulation(m_hGrabConstraint);
		g_pLTBase->PhysicsSim()->ReleaseBreakable(m_hGrabConstraint);
		m_hGrabConstraint = INVALID_PHYSICS_BREAKABLE;
	}

#ifdef PROJECT_DARK
	//!!ARL: This should *not* be updated every single frame.
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	ENUM_NMPolyID ePolyID = g_pAIQuadTree->GetContainingNMPoly(vPos, ForensicObject::GetForensicTypeMask(), kNMPoly_Invalid);
	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePolyID );
	if ( pPoly )
	{
		// This distance is used to perform vertical checks between the players 
		// position and the containing poly.  As the AIQuadTree does 2D containment
		// tests, we need to distinguish between the player standing in a forensics 
		// poly and the player a story above or below a poly.

		const float kMaxVerticalDistanceFromPoly = 100;

		bool bVerticallyInPoly = 
			( vPos.y < pPoly->GetNMPolyAABB()->vMax.y + kMaxVerticalDistanceFromPoly ) 
			&& ( vPos.y > pPoly->GetNMPolyAABB()->vMin.y - kMaxVerticalDistanceFromPoly );

		if ( !bVerticallyInPoly )
		{
			pPoly = NULL;
		}
	}

	uint32 dwCharTypeMask = pPoly ? (pPoly->GetNMCharTypeMask() & ForensicObject::GetForensicTypeMask()) : 0;
	if (dwCharTypeMask != m_dwLastForensicTypeMask)
	{
		m_dwLastForensicTypeMask = dwCharTypeMask;

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_CHARACTER_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.WriteBits(CFX_UPDATE_FORENSIC_MASK, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint32( m_dwLastForensicTypeMask );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
	}
#endif

	// If we're outside the world (and not in spectator mode)...wake-up,
	// time to die...

	if (!IsSpectating( ) && GetPlayerState( ) == ePlayerState_Alive)
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

			damage.DoDamage(m_hObject, m_hObject);
		}
	}


	if ( (GetPlayerState( ) == ePlayerState_Alive) && m_bForceDuck )
	{
		if( !m_Animator.IsDisabled()) 
			m_Animator.UpdateForceDucking();
	}

	m_Inventory.Update();

	uint32 nFlags; 
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, nFlags);
	if (m_ePlayerState == ePlayerState_Alive && !(nFlags & FLAG_VISIBLE) && !IsSpectating())
	{
		LTERROR("shouldn't be alive and hidden");
	}

	// Update the playerstate data.
	UpdatePlayerState( );

	float fDeltaTime = EngineTimer( m_hObject ).GetTimerElapsedS();
	m_NodeTrackerContext.UpdateNodeTrackers( fDeltaTime );

	// Check if it's time to update our pitch/roll rotation.
	// We do this at a periodic rate to reduce the network traffic down to clients.
	if( IsMultiplayerGameServer() && RealTimeTimer::Instance().GetTimerAccumulatedS() > m_fNextClientRotationUpdateTime )
	{
		// Check if the rotation changed since the last time we set it.
		LTRigidTransform tfCamera;
		GetViewTransform( tfCamera );
		if( m_rLastRotation != tfCamera.m_rRot )
		{
			// Remove the yaw from the hitbox.  Just store the pitch and roll.  The
			// client will use this to indicate how much node tracking to do for pitch and lean.
			EulerAngles eaCamera = Eul_FromQuat( tfCamera.m_rRot, EulOrdYXZr );
			m_rLastRotation = tfCamera.m_rRot;

			uint32 nPitchRoll = CompressPitchRoll16( eaCamera.y, eaCamera.z );
			g_pLTServer->SetObjectUnguaranteedData( m_hObject, nPitchRoll );

			// Update the next rotation time.
			float fUpdateRate = GetConsoleFloat( "ClientRotUpdateRate", 15.0f );
			m_fNextClientRotationUpdateTime = RealTimeTimer::Instance().GetTimerAccumulatedS() + (( fUpdateRate > 0.0f ) ? ( 1.0f / fUpdateRate ) : 0.0f );
		}
	}

	// Check if player has slowmo charger and should get points.
	if( m_bSlowMoCharge && m_tmrSlowMoHoldCharger.IsStarted( ) && m_tmrSlowMoHoldCharger.IsTimedOut( ))
	{
		// Get the amount of time we've gone over to roll it over into the new period.
		double fOverTime = LTMAX( m_tmrSlowMoHoldCharger.GetElapseTime() - m_tmrSlowMoHoldCharger.GetDuration( ), 0.0f );

		// Make sure we're still getting score for holding slowmo.
		float fSlowMoHoldScorePeriod = (float)GameModeMgr::Instance().m_grnSlowMoHoldScorePeriod;
		uint32 nSlowMoHoldScorePlayer = GameModeMgr::Instance().m_grnSlowMoHoldScorePlayer;
		uint32 nSlowMoHoldScoreTeam = GameModeMgr::Instance().m_grbUseTeams ? GameModeMgr::Instance().m_grnSlowMoHoldScoreTeam : 0;
		if( fSlowMoHoldScorePeriod > 0.0f && ( nSlowMoHoldScorePlayer || nSlowMoHoldScoreTeam ))
		{
			m_tmrSlowMoHoldCharger.Start( fSlowMoHoldScorePeriod - fOverTime );
			if( nSlowMoHoldScorePlayer )
		{
			GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
			if( pGameClientData )
					pGameClientData->GetPlayerScore( )->AddObjectiveScore( nSlowMoHoldScorePlayer );
			}
			if( nSlowMoHoldScoreTeam )
			{
				CTeamMgr::Instance().AddToScore( GetTeamID(), nSlowMoHoldScoreTeam );
			}
		}
		else
		{
			m_tmrSlowMoHoldCharger.Stop( );
		}
	}

	// Make sure the next update isn't considered the first.
	m_bFirstUpdate = false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateDead()
//
//	PURPOSE:	Override of the Charater UpdateDead function.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateDead( bool bCanBeRemoved )
{
	bool bPlayerCanBeRemoved = bCanBeRemoved;
	if( GetPlayerState( ) != ePlayerState_Dead )
		bPlayerCanBeRemoved = false;

	CCharacter::UpdateDead( bPlayerCanBeRemoved );
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
	if (!GetClient( )) return;

// PLAYER_BODY
	if( m_Animator.IsDisabled() )
		return;

	if ( m_bAllowInput /*&& !m_bChatting*/ )
	{

		if ( (WS_RELOADING == m_nWeaponStatus) || m_Animator.IsAnimatingPosture(CAnimatorPlayer::eReload) )
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

		if ( WS_NONE == m_nWeaponStatus )
		{
			if ( ((GetControlFlags() & BC_CFLG_FIRING) != 0) || 
				m_Animator.IsAnimatingPosture( CAnimatorPlayer::eFire ) ||
				m_Animator.IsAnimatingPosture( CAnimatorPlayer::eFire2 ) ||
				m_Animator.IsAnimatingPosture( CAnimatorPlayer::eFire3 ))
			{
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

		CWeapon* pWeapon = m_Arsenal.GetCurWeapon();
		if ( pWeapon )
		{
			HWEAPON hWeapon = pWeapon->GetWeaponRecord();
			if( hWeapon )
			{
				HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
				eWeapon = (CAnimatorPlayer::Weapon)g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nAniType );
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

		if ( m_nMotionStatus != MS_NONE && ((GetControlFlags() & BC_CFLG_DUCK) == 0) )
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

			if( (GetControlFlags() & BC_CFLG_RUN) != 0 )
			{
				m_Animator.SetMovement(CAnimatorPlayer::eRunning);
			}
			else
			{
				m_Animator.SetMovement(CAnimatorPlayer::eWalking);
			}

			// Can only duck in certain situations...

			if ( !IsOnLadder( ) && !m_bBodySpecialMove && !IsSpectating( ) && !IsLiquid(m_eContainerCode) )
			{
				if (((GetControlFlags() & BC_CFLG_DUCK) != 0) || m_bForceDuck)
				{
					m_Animator.SetMovement(CAnimatorPlayer::eCrouching);
				}
			}


			// See if we should be swimming...

			bool bSwimming = IsLiquid(m_eContainerCode);
			if (!bSwimming)
			{
				bSwimming = (m_bBodyInLiquid ? !m_bOnGround : false);
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

		if ( IsOnLadder( ) && (m_Animator.GetLastMain() != CAnimatorPlayer::eDT_Sleeping) && 
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
	if( m_Animator.IsDisabled())
		return false;

	// Don't do special damage anis if on a ladder or a vehicle...

	if( (m_ePPhysicsModel != PPM_NORMAL) )
		return true;

	// The DT functions loop through all damage types every time
	// you do anything.  Get the one's we care about now, so it
	// doesn't loop every frame.
	static DamageFlags nStunFlag = DamageTypeToFlag(DT_STUN);
	static DamageFlags nBurnFlag = DamageTypeToFlag(DT_BURN);


	// Set the fullbody animation acording to the damage being done...

	// Set the damage types that have no movement first...


	// Only play the hurt anis if we are aimming...

	if( m_Animator.GetPosture() == CAnimatorPlayer::eAim && !( IsOnLadder( ) || m_bBodySpecialMove) )
	{
		// If we are moving at all play the generic walk hurt animation...

		if( m_Animator.GetDirection() != CAnimatorPlayer::eNone )
		{
			m_Animator.SetMain( CAnimatorPlayer::eDT_HurtWalk );
			return true;
		}

		// We are not moving so play the animation based on damage type...

		if( m_nDamageFlags & nStunFlag )
		{
			m_Animator.SetMain( CAnimatorPlayer::eDT_Stun );
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

HMODELANIM CPlayerObj::GetDeathAni(bool bFront)
{
	HMODELANIM hDeathAnim = INVALID_MODEL_ANIM;

	// Handle crouching death
	if ( !m_Animator.IsDisabled() && m_Animator.GetLastMovement() == CAnimatorPlayer::eCrouching)
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
		g_pModelLT->GetModelAnimUserDims(m_hObject, hDeathAnim, &vDims);
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

HMODELANIM CPlayerObj::GetCrouchDeathAni(bool bFront)
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
	if ( IsMultiplayerGameServer( ) && m_bUseLeash)
	{
		// calculate the difference between the server's position and client's position of the player object
		LTVector curPos;
		g_pLTServer->GetObjectPos(m_hObject, &curPos);
		float fDistSqr = curPos.DistSqr(m_vLastClientPos);
		
		// get the velocity - this is used to scale the leash length
		LTVector vVelocity;
		g_pPhysicsLT->GetVelocity(m_hObject, &vVelocity);

		// this check determines if the object has moved outside of the movement dead-zone, which
		// is the allowable distance between the client's position for the player object and the
		// position of the object on the server
		if (fDistSqr > m_fLeashLen*m_fLeashLen)
		{
			// we're outside of the dead-zone, so interpolate toward the client's reported position
			LTVector vNewPos = curPos.Lerp(m_vLastClientPos, m_fLeashSpringRate);

			// use physics to move the object toward the target point
			g_pLTServer->Physics()->MoveObject(m_hObject, vNewPos, 0);

			// determine the maximum allowed distance by scaling the velocity, but don't
			// reduce it below the minimum threshold
			float fMaxDistance = LTMAX(vVelocity.Mag() * m_fLeashScale, m_fLeashSpring);

			// recalculate the distance from the client's last reported position
			fDistSqr = vNewPos.DistSqr(m_vLastClientPos);

			// check to see if we've exceeded the maximum allowed distance
			if (fDistSqr > fMaxDistance*fMaxDistance)
			{
				if (s_vtAlwaysForceClientToServerPos.GetFloat())
				{
					// force the client to our position
					TeleportClientToServerPos(false);

					// set the velocity to zero
					LTVector vVel(0.0f, 0.0f, 0.0f);
					g_pPhysicsLT->SetVelocity(m_hObject, vVel);

					// store the current time, which will be used to disregard player updates
					// that were sent by the client prior to the teleport event
					m_nLastPositionForcedTime = g_pLTServer->GetRealTimeMS( );

					// turn off the leash
					m_bUseLeash = false;
				}
				else
				{
				// use physics to move the object with collisions
				g_pLTServer->Physics()->MoveObject(m_hObject, m_vLastClientPos, 0);
				
				// force the position in case the object was blocked
				g_pLTServer->SetObjectPos(m_hObject, m_vLastClientPos);
				
				// turn off the leash if the object is not moving
				m_bUseLeash = vVelocity.Mag() > 0.001f;
			}
		}
		}
		else if (vVelocity.Mag() < 0.001f)
		{
			// Turn off the leash, we're close enough
			m_bUseLeash = false;
		}
	}

	CCharacter::UpdateMovement(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsObjectInArray
//
//	PURPOSE:	See if the object is in the list
//
// ----------------------------------------------------------------------- //

static bool IsObjectInArray(LTObjRef* theList, uint32 listSize, HOBJECT hTest)
{
	uint32 i;
	for(i=0; i < listSize; i++)
	{
		if(theList[i] == hTest)
			return true;
	}
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IsObjectInArray
//
//	PURPOSE:	See if the object is in the list
//
// ----------------------------------------------------------------------- //

static bool IsObjectInArray(HOBJECT* theList, uint32 listSize, HOBJECT hTest)
{
	uint32 i;
	for(i=0; i < listSize; i++)
	{
		if(theList[i] == hTest)
			return true;
	}
	return false;
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

	// Did our container states change?

	HOBJECT objContainers[MAX_TRACKED_CONTAINERS];
	uint32 nContainers = 0;

	UpdateInContainerState(objContainers, nContainers);

	if (!m_PStateChangeFlags) return;


	CAutoMessage cMsg;

	cMsg.Writeuint8(MID_CLIENT_PLAYER_UPDATE);

	cMsg.WriteBits(m_PStateChangeFlags, PSTATE_NUMBITS);

	// Note : Container updates are only sent on local clients, which is done to avoid extra container intersection testing
	if (m_PStateChangeFlags & PSTATE_CONTAINERTYPE)
	{
		float fFrictionPercent = 1.0f;

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
			float fViscosity = 0.0f, fGravity = 0.0f;

			LTVector vCurrent;
			vCurrent.Init();

			bool	bHidden = false;
			HOBJECT	hVolumeObject = NULL;
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

		float fFrictionCoeff = DEFAULT_FRICTION * fFrictionPercent;
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
		cMsg.WriteDatabaseRecord( g_pLTDatabase, GetModel( ));
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
		cMsg.Writefloat(m_fCrawlVel);

		// RunSpeed lets you run as fast as you want.. the client
		// treats this as a 'max speed' and acceleration multiplier...

		cMsg.Writefloat(m_fMoveMultiplier * m_fWeaponMoveMultiplier);
		cMsg.Writefloat(m_fBaseMoveAccel);
		cMsg.Writefloat(m_fJumpMultiplier);
		cMsg.Writefloat(m_fLadderVel);
		cMsg.Writefloat(m_fGravity);

		float fFrictionCoeff = 0.0f;
		g_pPhysicsLT->GetFrictionCoefficient(m_hObject, &fFrictionCoeff);
		cMsg.Writefloat(fFrictionCoeff);
	}

	if (m_PStateChangeFlags & PSTATE_PHYSICS_MODEL)
	{
		WriteVehicleMessage(cMsg);
	}

	if (m_PStateChangeFlags & PSTATE_CAMERA)
	{
		// For now just send the eye offset...
		cMsg.WriteLTVector(m_vHeadOffset);
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

	//Send a message to the character FX so that it knows we're on a vehicle
	// need a separate message because vehicle info is tracked in multiple places on the client
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits(CFX_PLAYER_PHYSICS_MODEL, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.Writeuint8( m_ePPhysicsModel );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, 0 );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetSpectatorMode
//
//	PURPOSE:	Turn on/off spectator mode
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetSpectatorMode( SpectatorMode eSpectatorMode, bool bForce )
{
/*
	switch( eSpectatorMode)
	{
	case eSpectatorMode_Clip:
		DebugCPrint(0,"CPlayerObj::SetSpectatorMode () : eSpectatorMode_Clip - %s",MPW2A(GetNetUniqueName()).c_str());
		break;
	case eSpectatorMode_Fly:
		DebugCPrint(0,"CPlayerObj::SetSpectatorMode () : eSpectatorMode_Fly - %s",MPW2A(GetNetUniqueName()).c_str());
		break;
	case eSpectatorMode_Follow:
		DebugCPrint(0,"CPlayerObj::SetSpectatorMode () : eSpectatorMode_Follow - %s",MPW2A(GetNetUniqueName()).c_str());
		break;
	case eSpectatorMode_Fixed:
		DebugCPrint(0,"CPlayerObj::SetSpectatorMode () : eSpectatorMode_Fixed - %s",MPW2A(GetNetUniqueName()).c_str());
		break;
	case eSpectatorMode_Tracking:
		DebugCPrint(0,"CPlayerObj::SetSpectatorMode () : eSpectatorMode_Tracking - %s",MPW2A(GetNetUniqueName()).c_str());
		break;
	case eSpectatorMode_None:
		DebugCPrint(0,"CPlayerObj::SetSpectatorMode () : eSpectatorMode_None - %s",MPW2A(GetNetUniqueName()).c_str());
		break;

	};
*/
	// Only allow entering spectator mode while we're alive if force to from a cheat.
	if( m_ePlayerState == ePlayerState_Alive && !IsSpectating( ) && !bForce && eSpectatorMode != eSpectatorMode_None )
	{
		return;
	}

	// If we're not alive, just go to the spectating playerstate.
	if( !IsAlive() && !IsSpectating())
	{
		SetPlayerState( ePlayerState_Spectator, false );
	}


	if( m_eSpectatorMode == eSpectatorMode )
		return;

	SpectatorMode eOldSpectatorMode = m_eSpectatorMode;
	m_eSpectatorMode = eSpectatorMode;

	// Tell our client about our spectator mode.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PLAYER_SPECTATORMODE );
	cMsg.Writeuint8(( uint8 )m_eSpectatorMode );
	g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);

	// Check if we're entering spectator mode for the first time.
	if( eOldSpectatorMode == eSpectatorMode_None )
	{
		// Clear the flags...(make sure we still tell the client to update)...

		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, m_nSavedFlags);
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAGMASK_ALL);
		g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, 0, FLAG_VISIBLE | FLAG_RAYHIT | FLAG_TOUCHABLE );

		// Go invisible to AI.
		BecomeInvisible();
		RemovePersistentStimuli( );

		HideCharacter(true);
	}
	// Check if we're leaving spectator mode.
	else if( m_eSpectatorMode == eSpectatorMode_None )
	{
		if( m_ePlayerState == ePlayerState_Alive )
		{
			SetPlayerState( ePlayerState_Alive, true );

			// Add stimuli back now that we are alive again, so that AI can see us again.
			BecomeVisible();
			RegisterPersistentStimuli();

			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_nSavedFlags, FLAGMASK_ALL);


			// move us to where the client left us..,
			g_pLTServer->SetObjectPos(m_hObject,m_vLastClientPos);

			// Force us to the floor...
			MoveObjectToFloor( m_hObject );
			UpdateClientPhysics();
			TeleportClientToServerPos( false );

			// then reset our client position so that leashing doesn't move us back...
			LTVector myPos;
			g_pLTServer->GetObjectPos(m_hObject, &myPos);
			m_vLastClientPos = myPos;

		}
	}

	// Update our special fx message so new clients will get the updated
	// info as well...
	CreateSpecialFX();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetInvisibleMode
//
//	PURPOSE:	Turn on/off invisible mode
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetInvisibleMode(bool bSetInvisible)
{
	// Don't mess with invisibility if in spectator mode...

	if (!IsSpectating( ))
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

	//ASSERT(m_eEnemyVisibleStimID != kStimID_Unset);
	if (m_eEnemyVisibleStimID != kStimID_Unset)
	{
		g_pAIStimulusMgr->RemoveStimulus(m_eEnemyVisibleStimID);
	}
	m_eEnemyVisibleStimID = kStimID_Unset;
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

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos);

	StimulusRecordCreateStruct scs( kStim_CharacterVisible, GetAlignment(), vPos, m_hObject );
	scs.m_dwDynamicPosFlags |= CAIStimulusRecord::kDynamicPos_TrackSource;
	m_eEnemyVisibleStimID = g_pAIStimulusMgr->RegisterStimulus( scs );
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
//	ROUTINE:	CPlayerObj::HealCheat()
//
//	PURPOSE:	Increase hit points
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HealCheat()
{
	if (!IsAlive()) return;

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
	if (!IsAlive()) return;

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
	if (!IsAlive()) return;

	m_Inventory.HandleCheatFullAmmo();
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
	if (!IsAlive()) return;

	m_Inventory.HandleCheatFullWeapon();
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
	if (!IsAlive()) return;

	m_Inventory.HandleCheatFullMods();
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
	if (!IsAlive()) return;

	m_Inventory.HandleCheatFullGear();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GimmeGunCheat()
//
//	PURPOSE:	Give a specific weapon...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::GimmeGunCheat( HWEAPON hWeapon )
{
	if( !IsAlive( ))
		return;

	m_Inventory.HandleCheatWeapon( hWeapon );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GimmeModCheat()
//
//	PURPOSE:	Give a specific mod...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::GimmeModCheat( HMOD hMod )
{
	if( !IsAlive())
		return;

	m_Inventory.AcquireMod( hMod );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GimmeGearCheat()
//
//	PURPOSE:	Give a specific gear...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::GimmeGearCheat( HGEAR hGear )
{
	if( !IsAlive())
		return;

	m_Inventory.AcquireGear( hGear );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GimmeAmmoCheat()
//
//	PURPOSE:	Give a specific ammo...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::GimmeAmmoCheat( HAMMO hAmmo )
{
	if( !IsAlive())
		return;

	m_Inventory.AcquireAmmo( hAmmo );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireGear()
//
//	PURPOSE:	Give us the specified gear
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireGear( const char *pszGearName )
{
	if( !pszGearName || !*pszGearName || !GetClient( ))
		return;

	HGEAR hGear = g_pWeaponDB->GetGearRecord( pszGearName );
	if( hGear && !g_pWeaponDB->IsRestricted( hGear ))
		m_Inventory.AcquireGear(hGear);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Respawn()
//
//	PURPOSE:	Respawn the player
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Respawn( )
{
	if ( !GetClient( ) )
	{
		LTERROR( "Respawning without a client." );
		return;
	}

	SetLadderObject( INVALID_HOBJECT );
	SetSpecialMove(false);

	// Turn off leashing..
	m_bUseLeash = false;
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);

	//make sure we're still updating
	SetNextUpdate(UPDATE_NEXT_FRAME);

	// Record when we last respawned.
	m_fLastRespawnTime = GameTimeTimer::Instance( ).GetTimerAccumulatedS( );

	// Respawning from a save game.  Load will do all the work.
	if( m_dwLastLoadFlags == LOAD_RESTORE_GAME )
	{
		SetPlayerState( ePlayerState_Alive, false );
		return;
	}

	// Respawning from a transition.  Load will do all the work.
	if( m_dwLastLoadFlags == LOAD_TRANSITION )
	{
		SetPlayerState( ePlayerState_Alive, false );
		CreateSpecialFX( true );
		AcquireCurrentLevelDefaults();
		return;
	}

	//if the player requested a team switch, do it now
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
	if( pGameClientData && pGameClientData->RequestedTeamChange())
	{
		SwitchToTeam(pGameClientData->GetRequestedTeam());
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
	HideAttachments( true );

	// Get a start point...

	GameStartPoint* pStartPt = GameStartPointMgr::Instance( ).FindStartPoint(this);


	LTVector vPos(0, 0, 0);

	if (pStartPt)
	{
		// Save our last start point.
		pGameClientData->SetLastStartPoint( pStartPt->m_hObject );

		// Set our starting values...

		g_pLTServer->GetObjectPos(pStartPt->m_hObject, &vPos);

		// If in single-player make sure we're using the correct player model...

		if (!IsMultiplayerGameServer())
		{
			SetModel( pStartPt->GetPlayerModel());
		}

		SetPhysicsModel(pStartPt->GetPlayerPhysicsModel());
	}
	else
	{
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
	}

	if (IsMultiplayerGameServer( ))
	{
		TeleFragObjects(vPos);
	}

	LTRotation rRot;

	if (pStartPt)
	{
		// Inform the client of the correct camera/player orientation...

		g_pLTServer->GetObjectRotation( pStartPt->m_hObject, &rRot );

		// Get start point command string...

		m_sStartLevelCommand = pStartPt->GetCommand();

		if (!m_sStartLevelCommand.empty())
		{
			if (IsMultiplayerGameServer() && pStartPt->SendCommandOnRespawn())
			{
				m_bSendStartLevelCommand = true;
			}
		}
	}

	// Make the player solid and touchable (they are created without these
	// flags to avoid touching objects until they are spawned). The next time we
	// apply m_dwFlags (in ePlayerState_Alive) we should be capable of touching 
	// objects.  This is a workaround for the fact that we create the player 
	// initially at 0,0,0 (potentially touching any objects that happen to be 
	// at that location).
	m_dwFlags |= FLAG_SOLID | FLAG_TOUCH_NOTIFY;

	// Place the object and character hit box at the start point...
	Teleport( vPos, rRot );


	if (IsMultiplayerGameServer( ) && GameModeMgr::Instance( ).m_grbUseTeams)
	{
		if (!s_RespawnMsgTimer.IsStarted() || s_RespawnMsgTimer.IsTimedOut())
		{
			HRECORD hRec = DATABASE_CATEGORY( Broadcast ).GetRecordByName( "Respawn");
			PlayerBroadcastInfo pbi;
			pbi.nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hRec );
			pbi.nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, Priority);

			HandleBroadcast( pbi );
			s_RespawnMsgTimer.Start(10.0f);
		}
	}

	if (!IsMultiplayerGameServer( ))
	{
	// Update our special fx message...(and tell the client about it as
	// well ;).  We need to do this before aquiring our default weapon...

		// 08/04/03 - KEF - Reset player also does this, so I don't believe this is needed any more.
		// It works properly under network testing, but because it has not yet been tested under
		// singleplayer, it's still sent in that situation.

		CreateSpecialFX(true);
	}

	// Clear the spectator mode.
	SetSpectatorMode( eSpectatorMode_None, true );

	PlayerState eOldState = m_ePlayerState;

	// We are now alive.
	SetPlayerState( ePlayerState_Alive, false );

	// Check if we're currenlty dead.
	if( m_damage.IsDead())
		ResetAfterDeath();

	if( m_bNewMission )
	{
		m_bNewMission = false;
		SetupForNewMission( );
	}
	else
	{
		AcquireCurrentLevelDefaults();
	}

	if( IsMultiplayerGameServer( ))
	{
		// Track the players transform history...
		CObjectTransformHistoryMgr::Instance( ).AddObject( m_hObject );
	}

	m_bForceUpdateInterface = true;

	// Tell clients we have been respawned...
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits(CFX_PLAYER_RESPAWN, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::Teleport()
//
//	PURPOSE:	Teleport the player to the specified position with the
//				specified pitch/yaw/roll
//
// ----------------------------------------------------------------------- //

void CPlayerObj::Teleport(const LTVector & vPos, const LTRotation& rRot)
{
	// Turn off the leash
	m_bUseLeash = false;

	// Set our starting position...

	LTVector vNewPos = vPos;
	g_pLTServer->SetObjectPos(m_hObject, vNewPos);
	g_pLTServer->SetObjectState(m_hObject, OBJSTATE_ACTIVE);

	m_tfCameraView.m_rRot = rRot;
	m_tfTrueCameraView.m_rRot = rRot;
	g_pLTServer->SetObjectRotation(m_hObject, rRot);

	// Teleport the character hit box...
	g_pLTServer->SetObjectPos( GetHitBox( ), vNewPos );

	// Make sure we start on the ground...

	MoveObjectToFloor(m_hObject);

	UpdateClientPhysics();
	TeleportClientToServerPos( true );
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

	LTRotation rRot;
	g_pLTServer->GetObjectRotation( pTeleportPoint->m_hObject, &rRot );

	Teleport( vPos, rRot );
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

	// Calculate the friction...
	g_pPhysicsLT->SetFrictionCoefficient(m_hObject, DEFAULT_FRICTION);
	m_PStateChangeFlags |= PSTATE_SPEEDS; // Resend friction..

	g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, FLAG_VISIBLE | FLAG_RAYHIT | FLAG_TOUCHABLE, FLAG_VISIBLE | FLAG_RAYHIT | FLAG_TOUCHABLE );

	float fHealthX	= 1.0f;
	float fArmorX	= 1.0f;

	fHealthX	= GetSkillValue(eHealthMax);
	fArmorX		= GetSkillValue(eArmorMax);

	// make sure progressive damage is reset
	m_damage.ClearProgressiveDamage();

	m_damage.Reset( fHealthX * g_pModelsDB->GetModelHitPoints( GetModel( )),
		fArmorX * g_pModelsDB->GetModelArmor(GetModel( )) );

	m_fAirLevel	= MAX_AIR_LEVEL;

	ResetPlayer( );

	ResetInventory(true);
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

	if ( !m_pAttachments )
	{
		ASSERT( !"CPlayerObj::SetupForNewMission: Invalid attachments object." );
		return false;
	}

	//reset mission stats
	m_Stats.Init();

	// We'll need a updateinterface after all of this.
	m_bForceUpdateInterface = true;

	if( g_pServerMissionMgr->IsCustomLevel( ))
	{
		ResetHealth();
		ResetInventory();
	}
	else
	{
		// Get the current mission.
		int nCurMission = g_pServerMissionMgr->GetCurrentMission( );
		HRECORD hMission = g_pMissionDB->GetMission( nCurMission );
		if( !hMission )
			return false;

		// Reset the player if told or if this is a new game.
		if( g_pMissionDB->GetBool(hMission,MDB_ResetPlayer) || g_pGameServerShell->GetLGFlags( ) == LOAD_NEW_GAME )
		{
			ResetHealth();
			ResetInventory();
		}

		// Get the mission and lefel defaults...
		if( !AcquireCurrentLevelDefaults() )
			return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireDefaultWeapons()
//
//	PURPOSE:	Give us the default weapons
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireDefaultWeapons()
{
	HRECORD hGlobalRec = g_pWeaponDB->GetGlobalRecord();
	HATTRIBUTE hDefsAtt = g_pLTDatabase->GetAttribute(hGlobalRec,WDB_GLOBAL_rDefaultWeapons);
	uint32 nNumDefs = g_pLTDatabase->GetNumValues(hDefsAtt);

	//step through the list of weapons
	for (uint32 n = 0; n < nNumDefs; ++n)
	{
		HWEAPON hWeapon = g_pLTDatabase->GetRecordLink(hDefsAtt,n,NULL);
		if( !hWeapon )
			continue;

		// Check if we already have this weapon.
		CWeapon* pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( pWeapon )
		{
			continue;
		}

		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		if( g_pWeaponDB->IsRestricted( hWpnData ))
		{
			continue;
		}
		
		HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA); 
		if( !hAmmoData || g_pWeaponDB->IsRestricted( hAmmoData ))
		{
			continue;
		}

		uint32 nSelectionAmmount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nSelectionAmount );
		m_Inventory.AcquireWeapon( hWeapon, hAmmo, nSelectionAmmount, true );
		pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( !pWeapon )
		{
			LTERROR( "CPlayerObj::AcquireDefaultWeapons: Could not obtain weapon." );
			continue;
		}
	}

	AcquireDefaultWeapon();
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
	HWEAPON hWeapon = g_pServerDB->GetPlayerDefaultWeapon();

	if( !hWeapon )
		return;

	m_Inventory.AcquireDefaultWeapon( hWeapon);
	ChangeWeapon( hWeapon, true, NULL, false, false );


	// Tell the client this is their default weapon.
	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
	cClientMsg.Writeuint8(IC_DEFAULTWEAPON_ID);
	cClientMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
	g_pLTServer->SendToClient(cClientMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireWeapon()
//
//	PURPOSE:	Give us the specified weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireWeapon( const char *pszWeaponName )
{
	if( !pszWeaponName || !*pszWeaponName )
		return;

	HWEAPON	hWeapon = NULL;
	HAMMO	hAmmo = NULL;

	g_pWeaponDB->ReadWeapon( pszWeaponName, hWeapon, hAmmo, !USE_AI_DATA );
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	if( hWeapon && !g_pWeaponDB->IsRestricted( hWpnData ))
	{
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA); 
		if( hAmmo && !g_pWeaponDB->IsRestricted( hAmmoData ))
		{
			HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,false);
			uint32 nAmount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nSelectionAmount );
			m_Inventory.AcquireWeapon( hWeapon, hAmmo, nAmount, true );

			
			if (!g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bIsGrenade))
				ChangeWeapon( hWeapon, true, NULL, true, true );
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

void CPlayerObj::AcquireAmmo( const char *pszAmmoName )
{
	if( !pszAmmoName || !*pszAmmoName )
		return;

	HAMMO hAmmo = g_pWeaponDB->GetAmmoRecord( pszAmmoName );
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA); 
	if( hAmmo && !g_pWeaponDB->IsRestricted( hAmmoData ))
	{
		m_Inventory.AcquireAmmo( hAmmo );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::AcquireMod()
//
//	PURPOSE:	Give us the specified Mod
//
// ----------------------------------------------------------------------- //

void CPlayerObj::AcquireMod( const char *pszModName)
{
	if( !pszModName || !*pszModName )
		return;

	HMOD hMod = g_pWeaponDB->GetModRecord( pszModName );
	if( hMod )
	{
		m_Inventory.AcquireMod( hMod, false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeToWeapon()
//
//	PURPOSE:	Change to the specified weapon (if we have it)
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeToWeapon( const char* pszWeaponName )
{
	if( !pszWeaponName || !*pszWeaponName )
		return;

	HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord( pszWeaponName );
	if( !hWeapon )
		return;

	CWeapon* pWeapon = m_Arsenal.GetWeapon( hWeapon );
	if( pWeapon && pWeapon->Have() )
	{
		ChangeWeapon( hWeapon, true, NULL, true, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeToLastWeapon()
//
//	PURPOSE:	Change back to the previously held weapon.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeToLastWeapon()
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_WEAPON_LAST );
	g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleDead()
//
//	PURPOSE:	Tell client I died
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleDead()
{
	if (!m_hObject)
		return;

	GameClientData* pGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( GetClient( ));
	if( !pGameClientData )
		return;

	//if we died and we're not switching teams... spawn a death marker
	if (GameModeMgr::Instance( ).m_grbUseTeams && !pGameClientData->RequestedTeamChange())
	{
		NavMarkerCreator nmc;
		//see what kind of marker we're supposed to use
		nmc.m_hType = g_pServerDB->GetPlayerRecordLink(SrvDB_rDeathMarker);

		//create the marker, if a type was specified
		if (nmc.m_hType)
		{
			nmc.m_nTeamId = GetTeamID();
			g_pLTServer->GetObjectPos(m_hObject,&nmc.m_vPos);
			nmc.m_bInstant = true;

			NavMarker* pNM = nmc.SpawnMarker();
		}

		//look for nearby chars
		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );

		CTList<CCharacter*>	lstChars;
		CCharacter			**ppChar = NULL;

		bool bFound = false;
		if( g_pCharacterMgr->FindCharactersWithinRadius( &lstChars, vPos, 2000.0f, NULL ))
		{
			ppChar = lstChars.GetItem( TLIT_FIRST );
			while ( ppChar && !bFound )
			{
				CCharacter	*pTest = *ppChar;
				ppChar = lstChars.GetItem( TLIT_NEXT );

				// Make sure the witness can actually see the character...
				IntersectQuery	IQuery;
				IntersectInfo	IInfo;

				IQuery.m_Flags		= CHECK_FROM_POINT_INSIDE_OBJECTS | INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;

				HOBJECT hFilterList[] = { m_hObject, NULL };
				IQuery.m_FilterFn	= ObjListFilterFn;
				IQuery.m_pUserData	= hFilterList;

				IQuery.m_To		= vPos;
				g_pLTServer->GetObjectPos( pTest->m_hObject, &IQuery.m_From );

				if( g_pLTServer->IntersectSegment( IQuery, &IInfo ))
				{
					if( IInfo.m_hObject != pTest->m_hObject &&
						IInfo.m_hObject != pTest->GetHitBox() )
					{
						// The witness can't actually see the character so don't broadcast yet...
						continue;
					}
				}

				if (m_hObject != pTest->m_hObject && IsPlayer(pTest->m_hObject) && pTest->IsAlive())
				{
					CPlayerObj* pWitness = (CPlayerObj*)g_pLTServer->HandleToObject(pTest->m_hObject);
					if (pWitness->GetTeamID() == GetTeamID()) 
					{
						HRECORD hRec = DATABASE_CATEGORY( Broadcast ).GetRecordByName( "ManDown");
						PlayerBroadcastInfo pbi;
						pbi.nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hRec );
						pbi.nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, Priority);
						pbi.bForceClient = true;
						pWitness->HandleBroadcast( pbi );
						bFound = true;
					}
				}
			}
		}

	}

	m_Inventory.RemoveSlowMoNavMarker( );

	
	if( m_pAttachments )
	{
		m_pAttachments->HideAttachments( true );
	}

	m_Arsenal.HideWeapons( true );

	// If we're on the turret, then remove all our weapons, otherwise we'll drop
	// the turret weapon.
	if( GetTurret( ))
	{
		m_Arsenal.RemoveAllActiveWeapons( );
	}

	// Make sure any active weapon sounds aren't looping
	SetWeaponSoundLooping(PSI_INVALID, m_nWeaponSoundLoopWeapon);

	SetPlayerState(ePlayerState_Dying_Stage1, false);

	CCharacter::HandleDead();

	// Players should not be shootable.  Ai can be so you can get blood from them, but 
	// players in mp should not block projectiles or vectors.
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_RAYHIT | FLAG_TOUCH_NOTIFY );
	g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, 0, FLAG_RAYHIT | FLAG_TOUCHABLE );

	g_pLTServer->SetObjectShadowLOD( m_hObject, eEngineLOD_Never );


	// make sure progressive damage is reset
	m_damage.ClearProgressiveDamage();

	// Remove stimuli so that AI do not treat a dead player as an enemy.

	AITRACE( AIShowCharacters, ( m_hObject, "Player '%s' is dead.", GetNetUniqueName() ) );
	RemovePersistentStimuli();

	// See if the death caused the end of the round.
	g_pServerMissionMgr->CheckEliminationWin();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PrepareToSever
//
//	PURPOSE:	Switch to the sever body...
//
// ----------------------------------------------------------------------- //
void CPlayerObj::PrepareToSever()
{
	if (!m_bStartedDeath) return;

	//save off the old anims
	TrackerList::iterator iter = m_Trackers.begin();
	while (iter != m_Trackers.end())
	{
		g_pModelLT->GetCurAnim(m_hObject, (*iter).m_trkID, (*iter).m_hAni);
		if ((*iter).m_hAni != INVALID_MODEL_ANIM)
		{
			g_pModelLT->GetCurAnimTime(m_hObject, (*iter).m_trkID, (*iter).m_nTime );
		}
		iter++;
	}

	CCharacter::PrepareToSever();

	iter = m_Trackers.begin();
	while (iter != m_Trackers.end())
	{
		// Add the tracker to the player model...
		g_pModelLT->AddTracker( m_hObject, (*iter).m_trkID, true );

		// Set the initial weightset...
		LTRESULT res = g_pModelLT->SetWeightSet( m_hObject, (*iter).m_trkID, (*iter).m_hWeightSet );
		if (LT_OK != res)
		{
			LTERROR_PARAM2("failed to reset weight set %d after sever code: %d",(*iter).m_hAni,res);
			DebugCPrint(0,"%s failed to reset weight set %d after sever code: %d",__FUNCTION__, (*iter).m_hAni,res);
		}

		if ((*iter).m_hAni != INVALID_MODEL_ANIM)
		{
			LTRESULT res = g_pModelLT->SetCurAnim(m_hObject, (*iter).m_trkID, (*iter).m_hAni,false);
			if (LT_OK != res)
			{
				LTERROR_PARAM2("failed to reset anim %d after sever code: %d",(*iter).m_hAni,res);
				DebugCPrint(0,"%s failed to reset anim %d after sever code: %d",__FUNCTION__, (*iter).m_hAni,res);
			}
			g_pModelLT->SetCurAnimTime(m_hObject, (*iter).m_trkID, (*iter).m_hAni);
			g_pModelLT->SetPlaying( m_hObject, (*iter).m_trkID, true );

		}
		iter++;

	}

	LTVector vDims;
	g_pModelLT->GetModelAnimUserDims(m_hObject, g_pLTServer->GetModelAnimation(m_hObject), &vDims);
	SetDims(&vDims);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::WriteAnimInfo
//
//	PURPOSE:	Write the info about our current animatons and trackers
//
// ----------------------------------------------------------------------- //

void CPlayerObj::WriteAnimInfo(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	pMsg->Writeuint8( (uint8)m_Trackers.size() );

	TrackerList::iterator iter = m_Trackers.begin();
	while (iter != m_Trackers.end())
	{
		g_pModelLT->GetCurAnim(m_hObject, (*iter).m_trkID, (*iter).m_hAni);
		uint32 nTime = 0;
		if ((*iter).m_hAni != INVALID_MODEL_ANIM)
		{
			g_pModelLT->GetCurAnimTime(m_hObject, (*iter).m_trkID, nTime );
		}

		pMsg->Writeuint8((*iter).m_trkID);
		pMsg->Writeuint32((*iter).m_hWeightSet);
		pMsg->Writeuint32((*iter).m_hAni);
		pMsg->Writeuint32(nTime);

		iter++;
	}
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
		PlayDBSound("VehicleCrash", m_fSoundOuterRadius, true);

		SetPhysicsModel(PPM_NORMAL);
	}

	if( GetTurret( ))
	{
		// Cannot use normal activate message because turrets may only be activated from certain angles...
		g_pCmdMgr->QueueMessage( this, g_pLTServer->HandleToObject( GetTurret( )), "ACTIVATE_TURRET" );
	}

	CCharacter::StartDeath();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetPlayerState()
//
//	PURPOSE:	Do playerstate changes.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetPlayerState(PlayerState eState, bool bForceReset )
{
	// Check if we're already at this state.
	if( !bForceReset && eState == m_ePlayerState )
		return;

	PlayerState eOldPlayerState = m_ePlayerState;
	m_ePlayerState = eState;

/*
	switch( m_ePlayerState)
	{
	case ePlayerState_Dying_Stage1:
		DebugCPrint(0,"%s : ePlayerState_Dying_Stage1 - %s", __FUNCTION__, MPW2A(GetNetUniqueName()).c_str());
		break;
	case ePlayerState_Dying_Stage2:
		DebugCPrint(0,"%s : ePlayerState_Dying_Stage2 - %s", __FUNCTION__, MPW2A(GetNetUniqueName()).c_str());
		break;
	case ePlayerState_Dead:
		DebugCPrint(0,"%s : ePlayerState_Dead - %s",__FUNCTION__, MPW2A(GetNetUniqueName()).c_str());
		break;
	case ePlayerState_Alive:
		DebugCPrint(0,"%s : ePlayerState_Alive - %s",__FUNCTION__, MPW2A(GetNetUniqueName()).c_str());
		break;
	case ePlayerState_Spectator:
		DebugCPrint(0,"%s : ePlayerState_Spectator - %s",__FUNCTION__, MPW2A(GetNetUniqueName()).c_str());
		break;

	};
*/

	// Need to include damage type info in the state change, so we need to precalc that now.
	bool bSendDamageType = false;
	bool bSendTeamKillCount = false;
	DamageType dt = DT_INVALID;
	if (ePlayerState_Dying_Stage1 == m_ePlayerState)
	{
		dt = m_damage.GetDeathType();
		if (DT_INVALID != dt)
		{
			bSendDamageType =  g_pGameServerShell->ShouldSendDeathDamageToClient( DamageTypeToFlag(dt) );
		}

		//determine if we need to send the number of team kills since the last death
		if (IsMultiplayerGameServer() && GameModeMgr::Instance().m_grbUseTeams && GameModeMgr::Instance().m_grbFriendlyFire)
		{
			bSendTeamKillCount = true;
		}
		
	}

	// Tell our client about our state before sending other data in the specific state handling.
	if( GetClient( ))
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_STATE_CHANGE);
		cMsg.Writeuint8((uint8)m_ePlayerState);
		cMsg.Writebool(bSendDamageType);
		if (bSendDamageType)
		{
			cMsg.WriteBits(DamageTypeToFlag(dt),kNumDamageTypes);
		}
		cMsg.Writebool(bSendTeamKillCount);
		if (bSendTeamKillCount)
		{
			uint8 nTK = 0;
			GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
			if( pGameClientData )
			{
				nTK = pGameClientData->GetPlayerScore( )->GetNumTeamKills();
				pGameClientData->GetPlayerScore( )->ClearTeamKills();
				
			}
			cMsg.Writeuint8(nTK);
		}
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
	}

	switch( m_ePlayerState )
	{
		case ePlayerState_None:
			{
				// Not solid.
				g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_SOLID );

				// Hide the player while they are still joining.
				HideCharacter( true );

				// Teleport to our same position so we remove ourselves from containers.
				g_pLTServer->ForceCurrentObjectPos( m_hObject );

				// Need to be invulnerable until we're loaded.
				m_damage.SetCanDamage( false );

				m_DyingTimer.Stop( );

				RemovePersistentStimuli( );
			}
			break;
		case ePlayerState_Alive:
			{
				// Reset the players velocity to the saved velocity.
				if( m_dwLastLoadFlags == LOAD_RESTORE_GAME ||
					m_dwLastLoadFlags == LOAD_TRANSITION )
				{
					g_pPhysicsLT->SetVelocity( m_hObject, m_vSavedVelocity );

					if( m_dwSavedObjectFlags & FLAG_GRAVITY )
					{
						// Add gravity back in if it was set when the player was saved...

						g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, FLAG_GRAVITY, FLAG_GRAVITY );
					}
				}

				m_DyingTimer.Stop( );

				ResetPlayer();

				// Unhide the player.
				HideCharacter( false );

				// Make us vulnerable to damage again.
				if( !m_bGodMode )
					m_damage.SetCanDamage( true );

				// Become visible again to AI.
				BecomeVisible();

				// Add stimuli back now that we are alive again, so that AI can see us again.
				AITRACE( AIShowCharacters, ( m_hObject, "Player '%s' is respawning", GetNetUniqueName() ) );
				RegisterPersistentStimuli();

				// Restore all our flags so we make sure we didn't miss any.
				g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags, FLAGMASK_ALL);

				// Re-enable our hitbox.
				g_pCommonLT->SetObjectFlags( m_hHitBox, OFT_Flags, FLAG_RAYHIT | FLAG_TOUCHABLE, FLAG_RAYHIT | FLAG_TOUCHABLE );

				// Teleport to our same position so we put ourselves back into containers.
				g_pLTServer->ForceCurrentObjectPos( m_hObject );

			}
			break;
		case ePlayerState_Dead:
			{
				if( IsMultiplayerGameServer())
				{
					GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
					if( pGameClientData )
					{
						pGameClientData->SwitchToStandbyPlayer();

						//if the player killed himself, to switch teams, respawn immediately
						if( GameModeMgr::Instance( ).m_grbUseTeams && pGameClientData->RequestedTeamChange() && 
							GameModeMgr::Instance( ).m_grbAllowRespawnFromDeath )
						{
							CPlayerObj* pPlayerObj = ( CPlayerObj* )g_pLTServer->HandleToObject( pGameClientData->GetPlayer( ));
							pPlayerObj->Respawn();
						}
					}
					else
					{
						// No client, so remove the object.
						g_pLTServer->RemoveObject( m_hObject );
					}
				}
			}
			break;
		case ePlayerState_Dying_Stage1:
			{
				// Start a timer that sets the minimum amount of time we must remain dead.
				if (IsMultiplayerGameServer())
				{
					m_DyingTimer.Start( g_pServerDB->GetPlayerFloat( SrvDB_fMultiplayerDeathDelay ));
				}
				else
				{
					m_DyingTimer.Start( g_pServerDB->GetPlayerFloat( SrvDB_fDeathDelay ));
				}

				// Put us on the back of the dying list.
				m_lstDyingPlayers.push_back( this );
			}
			break;
		case ePlayerState_Dying_Stage2:
			{
				// Record this last death time.
				g_pGameServerShell->SetLastPlayerDeathTime( RealTimeTimer::Instance().GetTimerAccumulatedS());

				// Erase this instance from the list of dying players.
				PlayerObjList::iterator it = std::find( m_lstDyingPlayers.begin( ), m_lstDyingPlayers.end( ), this );
				if( it != m_lstDyingPlayers.end( ))
				{
					m_lstDyingPlayers.erase( it );
				}

				// If this is multiplayer, we're going to need a standby player to switch to when we respawn.
				// Create it now, so it has a chance to get to the client before we need to use it.
				if( IsMultiplayerGameServer())
				{
					GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
					if( pGameClientData )
					{
						pGameClientData->CreateStandbyPlayer();
					}
				}

				DropWeapons();
				SpawnGearItemsOnDeath( );
				DropInventoryEvent.DoNotify();
			}
			break;
		case ePlayerState_Spectator:
			{
				// Handle any team change they may have.
				GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
				if( pGameClientData && pGameClientData->RequestedTeamChange())
				{
					SwitchToTeam(pGameClientData->GetRequestedTeam());
				}

				// Reset us after our death.
				if( m_damage.IsDead())
				{
					ResetAfterDeath();
				}
				// At least give the model info.
				else
				{
					ResetModel( );
				}

				UpdateClientPhysics();
			}
			break;
		default:
			break;
	}

	// Notify the client about spectating.
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.WriteBits(CFX_SPECTATE, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writebool( IsSpectating( ));
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}

	CreateSpecialFX();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdatePlayerState()
//
//	PURPOSE:	Updates the player state data.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdatePlayerState( )
{
	switch( m_ePlayerState)
	{
	case ePlayerState_None:
		break;
	case ePlayerState_Dying_Stage1:
		{
			// Wait our turn to complete our dying.  This allows death to be spread out over a few
			// frames which helps with sending new objects down to the clients in mp.
			// If we're at the front, then it's our turn.
			if( m_lstDyingPlayers.front( ) == this )
			{
				// Check if we've waited long enough since the last death.
				float fDyingTimeGap = s_vtDyingTimeGap.GetFloat( );
				if( RealTimeTimer::Instance().GetTimerAccumulatedS() >= g_pGameServerShell->GetLastPlayerDeathTime() + fDyingTimeGap )
				{
					// If our death timer already ran out, then give it a little more time, so don't switch to Dead too
					// fast.  This gives our standby player some time to get to the clients.
					if( m_DyingTimer.GetTimeLeft() < fDyingTimeGap )
					{
						// Add a little more time to the timer.
						m_DyingTimer.Start( 0.1f );
					}

					// Switch to stage 2.
					SetPlayerState( ePlayerState_Dying_Stage2, false );
				}
			}
		}
		break;
	case ePlayerState_Dying_Stage2:
		{
			// See if we're done dying and we can go to the dead state.
			// Need to stay in this state for at least a frame so that the standby player
			// can make it down to the client.
			if( m_DyingTimer.IsTimedOut( ))
			{
				m_DyingTimer.Stop( );
				SetPlayerState( ePlayerState_Dead, false );
			}
		}
		break;
	case ePlayerState_Dead:
		break;
	case ePlayerState_Alive:
		break;
	case ePlayerState_Spectator:
		break;
	};
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

	CCharacter::CharacterList::const_iterator iter;
	for( iter = CCharacter::GetCharacterList().begin(); iter != CCharacter::GetCharacterList().end(); ++iter )
	{
		CCharacter *pChar = *iter;
		pChar->ResetCrosshair(kCharStance_Undetermined);
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetHealth()
//
//	PURPOSE:	Reset the health and armor
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetHealth(bool bMaxOnly, float fHealthPercent, float fArmorPercent )
{

	float fHealthX = 1.0f;
	float fArmorX = 1.0f;

	fHealthX	= GetSkillValue(eHealthMax);
	fArmorX		= GetSkillValue(eArmorMax);

	m_damage.SetMaxHitPoints(fHealthX * g_pModelsDB->GetModelMaxHitPoints(GetModel( )));
	m_damage.SetMaxArmorPoints(fArmorX * g_pModelsDB->GetModelMaxArmor(GetModel( )));

	if (!bMaxOnly)
	{
		m_damage.SetHitPoints( fHealthPercent * fHealthX * g_pModelsDB->GetModelHitPoints(GetModel( )));
		m_damage.SetArmorPoints( fArmorPercent * fArmorX * g_pModelsDB->GetModelArmor(GetModel( )));
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
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ResetInventory()
//
//	PURPOSE:	Reset our inventory
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ResetInventory(bool bRemoveGear)
{
	if( !m_pAttachments )
		return;

	// Clear our weapons/ammo...

	m_Inventory.Reset();

	// Clear our gear...

	if (bRemoveGear)
	{
		m_Inventory.RemoveAllGear();
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

	AcquireDefaultWeapons();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetModel()
//
//	PURPOSE:	Sets the current model.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetModel( ModelsDB::HMODEL hModel )
{
	CCharacter::SetModel( hModel );
	m_hModelSkeleton = g_pModelsDB->GetModelSkeleton( hModel );
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
	CCharacter::ResetModel();

	ObjectCreateStruct createstruct;
	createstruct.m_eGroup = PhysicsUtilities::ePhysicsGroup_UserPlayer;

	const char* pFilename = g_pModelsDB->GetModelFilename(GetModel( ));
	if( !pFilename )
	{
		ASSERT( !"CPlayerObj::ResetModel: Invalid model." );
		return;
	}

	createstruct.SetFileName(pFilename);
	g_pModelsDB->CopyMaterialFilenames(GetModel( ), createstruct.m_Materials[0], LTARRAYSIZE( createstruct.m_Materials ), 
		LTARRAYSIZE( createstruct.m_Materials[0] ));

	g_pCommonLT->SetObjectFilenames(m_hObject, &createstruct);

	// Make sure we're still using the player group.
	g_pLTServer->PhysicsSim()->SetObjectPhysicsGroup(m_hObject, PhysicsUtilities::ePhysicsGroup_UserPlayer);

	// Make sure the client knows about any changes...

	m_PStateChangeFlags |= PSTATE_MODELFILENAMES;

	m_damage.SetMass(g_pModelsDB->GetModelMass(GetModel( )));

	// Reset the animations...
	if( !m_Animator.IsDisabled())
	{
		m_Animator.Reset(m_hObject);

		// Reset the main tracker to make sure we lose any special animations we were playing when we died...
		m_Animator.SetMainToBase();
	}


	if( m_pAttachments )
	{
		// Re-attach all of our attachments in case we have changed models.

		m_pAttachments->RemoveAndRecreateAttachments();

		// Detach any prop or object attachments...

		m_pAttachments->DetachAttachmentsOfType( ATTACHMENT_TYPE_PROP );
		m_pAttachments->DetachAttachmentsOfType( ATTACHMENT_TYPE_OBJECT );
	}

	// Run thorough the list of default attachments for the model and attach them...

	const char *pszAttachment;
	char szMsg[128] = {0};

	uint32 cAttachments = g_pModelsDB->GetNumDefaultAttachments( GetModel( ));
	for( uint32 iAttachment = 0; iAttachment < cAttachments; ++iAttachment )
	{
		pszAttachment = g_pModelsDB->GetDefaultAttachment( GetModel( ), iAttachment );

		if( pszAttachment && pszAttachment[0] )
		{
			LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "%s %s", KEY_ATTACH, pszAttachment );
			g_pCmdMgr->QueueMessage( this, this, szMsg );
		}
	}

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, dwFlags );

	HideAttachments( !(dwFlags & FLAG_VISIBLE) );

	HRECORD hLeanRecord = g_pModelsDB->GetModelLeanRecord( GetModel( ));
	m_LeanNodeController.Init( m_hObject, hLeanRecord );

	if( m_hModelSkeleton != NULL )
	{
		m_NodeTrackerContext.Init( m_hObject, m_hModelSkeleton );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ChangeWeapon
//
//	PURPOSE:	Tell the client to change the weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ChangeWeapon( HWEAPON hWeapon, bool bForce, HAMMO hAmmo, bool bPlaySelect, bool bPlayDeselect )
{
	if( !GetClient( ))
		return;

//	const char* pszName = g_pLTDatabase->GetRecordName(hWeapon);
//	DebugCPrint(0,"%s - %s",__FUNCTION__,pszName);

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_WEAPON_CHANGE );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
	cMsg.Writebool( bForce );
	cMsg.Writebool( bPlaySelect );
	cMsg.Writebool( bPlayDeselect );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, hAmmo );
	g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoWeaponChange
//
//	PURPOSE:	Change our weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoWeaponChange( HWEAPON hWeapon, HAMMO hAmmo )
{
	m_Arsenal.ChangeWeapon( hWeapon, hAmmo );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::WarnWeaponWillBreak
//
//	PURPOSE:
//
// ----------------------------------------------------------------------- //

void CPlayerObj::WarnWeaponWillBreak(HWEAPON hWeapon)
{
	// inform client that the weapon is about to break soon
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_WEAPON_BREAK_WARN );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
	cMsg.Writebool(true);
	g_pLTServer->SendToClient( cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponBroke
//
//	PURPOSE:	The specified weapon has taken to much damage and 
//				needs to be replaced.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponBroke(HWEAPON hBrokenWeapon, HWEAPON hReplacementWeapon)
{
	if (!hReplacementWeapon)
		hReplacementWeapon = g_pServerDB->GetPlayerDefaultWeapon();

	m_Inventory.HandleWeaponBroke(hBrokenWeapon, hReplacementWeapon);
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
	if (!GetClient( ) || !g_pWeaponDB) return;

	if( m_bForceUpdateInterface )
	{
		m_bForceUpdateInterface = false;
		bForceUpdate = true;
	}

	bool bSendSomething = false;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_PLAYER_INTERFACE);

	// See if the ammo has changed...
	uint8 nNumAmmo = g_pWeaponDB->GetNumAmmo();

	// Write the ammo ammounts into a details message and collect the number
	// of ammo's we write out.  Need to do this so we can send the count at the beginning of the
	// message.
	CAutoMessage cAmmoDetailMsg;
	uint8 nNumAmmoWritten = 0;
	if( m_pnOldAmmo )
	{
		for( uint8 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
		{
			HAMMO hAmmo = g_pWeaponDB->GetAmmoRecord( nAmmo );
			int32 nAmmoCount = m_Arsenal.GetAmmoCount( hAmmo );

			// If not a forced update, only send if it's changed.  If it is a forced update, only send if it's not zero.
			if(( !bForceUpdate && m_pnOldAmmo[nAmmo] != nAmmoCount ) || ( bForceUpdate && nAmmoCount != 0 ))
			{
				cAmmoDetailMsg.Writeuint8( nAmmo );
				LTASSERT( nAmmoCount == ( uint16 )nAmmoCount, "Ammo amount exceeds maximum" );
				bool bSendAmmoCount = nAmmoCount != 0;
				cAmmoDetailMsg.Writebool( bSendAmmoCount );
				if( bSendAmmoCount )
					cAmmoDetailMsg.Writeuint16(nAmmoCount);
				nNumAmmoWritten++;
			}

			m_pnOldAmmo[nAmmo] = nAmmoCount;
		}
	}

	// Send the total ammo message.
	cMsg.Writebool( nNumAmmoWritten > 0 );
	if( nNumAmmoWritten > 0 )
	{
		bSendSomething = true;
		cMsg.Writebool( bForceUpdate );
		cMsg.Writeuint8( nNumAmmoWritten );
		cMsg.WriteMessageRaw( cAmmoDetailMsg.Read( ));
	}

	// See if health has changed...
	bool bSendHealth = ( ( abs( (int)m_fOldHitPts - (int)m_damage.GetHitPoints() )  >= 1) || bForceUpdate);
	cMsg.Writebool( bSendHealth );
	if( bSendHealth )
	{
		bSendSomething = true;
		m_fOldHitPts = m_damage.GetHitPoints();
		cMsg.Writeuint16(( uint16 )LTCLAMP(( uint16 )m_fOldHitPts, 0, SHRT_MAX ));
	}


	// See if armor has changed...

	bool bSendArmor = (m_fOldArmor != m_damage.GetArmorPoints() || bForceUpdate);
	cMsg.Writebool( bSendArmor );
	if( bSendArmor )
	{
		bSendSomething = true;
		m_fOldArmor = m_damage.GetArmorPoints();
		cMsg.Writeuint16(( uint16 )LTCLAMP(( uint16 )m_fOldArmor, 0, SHRT_MAX ));
	}

	// See if air level has changed...
	bool bSendAirLevel = (m_fOldAirLevel != m_fAirLevel || bForceUpdate);
	cMsg.Writebool( bSendAirLevel );
	if( bSendAirLevel )
	{
		bSendSomething = true;
		m_fOldAirLevel = m_fAirLevel;
		float fPercent = m_fAirLevel / MAX_AIR_LEVEL;
		cMsg.Writeuint8(( uint8 )LTCLAMP( fPercent * 255.0f, 0.0f, 255.0f ));
	}

	// Tell the client if we actually have something to send.
	if( bSendSomething )
		g_pLTServer->SendToClient(cMsg.Read( ), GetClient( ), MESSAGE_GUARANTEED);
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
	float fDeltaTime = EngineTimer( m_hObject ).GetTimerElapsedS();

	// See if we are in a liquid...

	if (IsLiquid(m_eContainerCode) && !m_Inventory.HasAirSupply())
	{
		float fFullAirLostTime = g_pServerDB->GetPlayerFloat(PLAYER_BUTE_FULLAIRLOSTTIME);
		if (fFullAirLostTime <= 0.0f) return;

		float fDeltaAirLoss	 = (MAX_AIR_LEVEL/fFullAirLostTime);

		m_fAirLevel -= fDeltaTime*fDeltaAirLoss;

		if (m_fAirLevel < 0.0f)
		{

		}
	}
	else if (m_fAirLevel < MAX_AIR_LEVEL)
	{
		float fFullAirRegenTime = g_pServerDB->GetPlayerFloat(PLAYER_BUTE_FULLAIRREGENTIME);
		if (fFullAirRegenTime <= 0.0f) return;

		float fDeltaAirRegen = (MAX_AIR_LEVEL/fFullAirRegenTime);
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

	RegenerationData regen;
	
	if (IsMultiplayerGameServer())
	{
		regen = g_pServerDB->GetRegenerationMP();
	}
	else
	{
		regen = g_pServerDB->GetRegeneration(g_pGameServerShell->GetDifficulty());
	}
	
	if (m_damage.GetHitPoints() >= regen.m_fThreshold) return;

	if (g_pLTServer->GetTime() < (m_damage.GetLastDamageTime() + regen.m_fDelay)) return;


	float fDeltaTime = EngineTimer( m_hObject ).GetTimerElapsedS();


	float fHeal = 0.0;
	if (m_vLastVelocity.MagSqr() < s_fRegenMovementVel)
	{
		fHeal = regen.m_fBaseRegeneration;		
	}
	else
	{
		fHeal = regen.m_fMoveRegeneration;		
	}

	fHeal *= fDeltaTime;

	m_damage.Heal(fHeal);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DoActivate()
//
//	PURPOSE:	Activate the object in front of us (return true if an
//				object was activated)
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::DoActivate(CActivationData* pData)
{
	if (GetPlayerState( ) != ePlayerState_Alive) return false;

	// Not allowed to do any activating when paused.
	if ( g_pGameServerShell->IsPaused())
	{
		return false;
	}

	//special case for releasing from ladder...
	if (pData->m_nType == MID_ACTIVATE_LADDER && !pData->m_hTarget)
	{
		SetLadderObject( INVALID_HOBJECT );
		return true;
	}

	//ditto for special move objects (I guess)...
	if (pData->m_nType == MID_ACTIVATE_SPECIALMOVE && !pData->m_hTarget)
	{
		SetSpecialMove(false);
		return true;
	}

	if (pData->m_hTarget)
	{
		ILTBaseClass *pTarget = g_pLTServer->HandleToObject(pData->m_hTarget);

		switch (pData->m_nType)
		{

			case MID_ACTIVATE_WAKEUP:
			{
				if( !IsCharacter( pData->m_hTarget ))
				{
					return false;
				}

				if( g_pCmdMgr->QueueMessage( this, pTarget, "WAKEUP" ))
					return true;
			}
			break;

			case MID_ACTIVATE_LADDER:
			{
				SetLadderObject( pData->m_hTarget );
				return true;
			}
			break;

			case MID_ACTIVATE_SPECIALMOVE:
			{
				SetSpecialMove(true);
				return true;
			}
			break;

			case MID_ACTIVATE_TURRET:
			{
				// Cannot use normal activate message because turrets may only be activated from certain angles...
				if( g_pCmdMgr->QueueMessage( this, pTarget, "ACTIVATE_TURRET" ))
					return true;
			}
			break;

			case MID_ACTIVATE_CHOP:
			case MID_ACTIVATE_PRY:
			case MID_ACTIVATE_BREAK:
			case MID_ACTIVATE_EXTINGUISH:
			case MID_ACTIVATE_DIG:
			{
				if( g_pCmdMgr->QueueMessage( this, pTarget, "ACTIVATE" ))
					return true;
			}
			break;

			case MID_ACTIVATE_NORMAL:
			default:
			{
				// First see if a cinematic is running...If so, try and stop it...
				if (PlayingCinematic(true))
				{
					return false;
				}

				if( g_pCmdMgr->QueueMessage( this, pTarget, "ACTIVATE" ))
					return true;
			}
			break;
		}
	}

	if (pData->m_nSurfaceType != ST_UNKNOWN)
	{
		HSURFACE hSurf = g_pSurfaceDB->GetSurface((SurfaceType)pData->m_nSurfaceType);

		// See if the surface we tried to activate has an activation
		// sound...If so, play it...
		if (hSurf)
		{
			HRECORD hActSnd = g_pSurfaceDB->GetRecordLink(hSurf,SrfDB_Srf_rActivationSnd);
			if (hActSnd && g_pSoundDB->GetFloat(hActSnd,SndDB_fOuterRadius) > 0) 
			{
				g_pServerSoundMgr->PlaySoundFromPos(pData->m_vIntersect, g_pSoundDB->GetRecordName(hActSnd), NULL,
					-1.0f, SOUNDPRIORITY_PLAYER_LOW, PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
					DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
			}
		}

		return true;
	}

	// First see if a cinematic is running...If so, try and stop it...
	if (PlayingCinematic(true))
	{
		return false;
	}

	return false;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::PlayingCinematic()
//
//	PURPOSE:	See if we are playing a cinematic (and stop it if specified)
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::PlayingCinematic(bool bStopCinematic)
{
	// Search for cinematic object if we need to stop it...

	if (Camera::IsActive() && bStopCinematic)
	{
		HOBJECT hObj = g_pLTServer->GetNextObject(NULL);

		// Stop all the Cameras that are currently active...

		while (hObj)
		{
			if (IsKindOf(hObj, "Camera"))
			{
				Camera* pCam = (Camera*) g_pLTServer->HandleToObject(hObj);
				if( pCam && pCam->IsOn() )
				{
					// Send a message to the camera to skip...
					g_pCmdMgr->QueueMessage( this, pCam, "SKIP" );
				}
			}

			hObj = g_pLTServer->GetNextObject(hObj);
		}
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

void CPlayerObj::ProcessDamageMsg( DamageStruct &rDamage )
{
	if( !GetClient( ))
		return;

	if (rDamage.eType == DT_WORLDONLY) 
	{
		return;
	}

	// Check for FriendlyFire...
	if( IsMultiplayerGameServer( ) && IsPlayer( rDamage.hDamager ) && (rDamage.hDamager != m_hObject) )
	{
		if( GameModeMgr::Instance( ).m_grbUseTeams && AreSameTeam( rDamage.hDamager, m_hObject ) ) 
		{
			// Check if ff is off.
			if( !GameModeMgr::Instance( ).m_grbFriendlyFire )
				return;
		}
	}

	CCharacter::ProcessDamageMsg( rDamage );

	HRECORD hSlowMo = GetSlowMoType(rDamage.eType);
	if (hSlowMo)
	{
		g_pGameServerShell->EnterSlowMo( hSlowMo, -1.0f, GetClient( ), kTransition | kUsePlayerTimeScale );
	}
	
	// Tell the client about the damage...

	float fDamage = m_damage.GetLastDamage();
	float fArmorAbsorb = m_damage.GetLastArmorAbsorb();

	if (fDamage > 0.0f || fArmorAbsorb > 0.0f)
	{
		bool bTookHealth = (fDamage > 0.0f) ? true : false;
		float fVal = fArmorAbsorb + fDamage;

		float fPercent = fVal / m_damage.GetMaxHitPoints();
		if (fPercent > 1.0f) fPercent = 1.0f;

		LTVector vDir = m_damage.GetLastDamageDir().GetUnit();

		// Update our sent health and armor so it doesn't get resent in the UpdateInterface polling.
		m_fOldHitPts = m_damage.GetHitPoints();
		m_fOldArmor = m_damage.GetArmorPoints();

		ModelsDB::HNODE hModelNode = GetModelNodeLastHit();
		HitLocation eLoc = HL_UNKNOWN;
		if( hModelNode )
		{
			eLoc = g_pModelsDB->GetNodeLocation( hModelNode );
		}


		CAutoMessage cDamageMsg;
		cDamageMsg.Writeuint8(MID_PLAYER_DAMAGE);
		cDamageMsg.WriteCompLTPolarCoord( vDir );
		cDamageMsg.Writeuint8(( uint8 )( fPercent * 255 ));
		cDamageMsg.Writeuint8(m_damage.GetLastDamageType());
		cDamageMsg.Writebool(bTookHealth);
		cDamageMsg.Writeuint16(LTCLAMP(( uint16 )m_fOldHitPts, 0, SHRT_MAX ));
		cDamageMsg.Writeuint16(LTCLAMP(( uint16 )m_fOldArmor, 0, SHRT_MAX ));
		cDamageMsg.Writebool( (eLoc == HL_HEAD) );
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

bool CPlayerObj::ClientInit( )
{
	if (!g_pGameServerShell || !GetClient( ))
		return false;

	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
	if( !pGameClientData )
		return false;

	if( !IsMultiplayerGameServer( ))
	{
		// Use the sp model.
		SetModel( g_pModelsDB->GetModelByRecordName( DEFAULT_PLAYERNAME ));
	}
	else
	{
		// Ask gameclientdata what model to use.
		SetModel( pGameClientData->GetMPModel( ));
		m_nMPModelIndex = pGameClientData->GetMPModelIndex( );
	}

	m_nLoadout = pGameClientData->GetLoadout();

	g_pGameServerShell->SetUpdateGameServ();

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::MultiplayerUpdate
//
//	PURPOSE:	Update multiplayer values
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::MultiplayerUpdate(ILTMessage_Read *pMsg)
{
	if (!g_pGameServerShell || !GetClient( ))
		return false;

	// Get our current clientdata so we can overwrite it.
	NetClientData ncd;
	GameClientData* pGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( GetClient( ));
	if( !pGameClientData )
		return false;
	if( !pGameClientData->GetNetClientData( ncd ))
		return false;
	// Get the name the client would like.
	pMsg->ReadWString( ncd.m_szName, LTARRAYSIZE( ncd.m_szName ));
	// Set our new changed netclientdata.
	pGameClientData->SetNetClientData( ncd );

	pGameClientData->SetMPModelIndex( pMsg->Readuint8( ));

	uint8 nTeam = pMsg->Readuint8( );
	uint8 nLoadout = pMsg->Readuint8( );

	// Get the insignia.  Only sent as a file title to reduce bandwidth, so we have to recreate the full path.
	char szPatchTitle[MAX_PATH] = "";
	pMsg->ReadString(szPatchTitle,LTARRAYSIZE(szPatchTitle));
	char szPatchDir[MAX_PATH] = "";
	g_pModelsDB->GetInsigniaFolder( szPatchDir, LTARRAYSIZE( szPatchDir ));
	char szPatchPath[MAX_PATH*2];
	LTSNPrintF( szPatchPath, LTARRAYSIZE( szPatchPath ), "%s%s.dds", szPatchDir, szPatchTitle );
	pGameClientData->SetInsignia(szPatchPath);

	// Don't allow model changes in team games.
	if( GameModeMgr::Instance( ).m_grbUseTeams)
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
		if (nTeam != pGameClientData->GetLastTeamId())
		{
			pGameClientData->SetRequestedTeam( nTeam );
			HandleTeamSwitchRequest();
		}
	}

	SetModel( pGameClientData->GetMPModel( ));
	m_nMPModelIndex = pGameClientData->GetMPModelIndex();

	if (nLoadout != pGameClientData->GetLoadout() && GameModeMgr::Instance( ).m_grbUseLoadout )
	{
		m_nLoadout = nLoadout;
		pGameClientData->SetLoadout( m_nLoadout );

		if (GetPlayerState( ) == ePlayerState_None)
		{
			//if they aren't already in the game we can change weapons now...
			ResetInventory(true);
			AcquireMPLoadoutWeapons();
		}
	}

	g_pGameServerShell->SetUpdateGameServ();

	ResetModel();

	// Let missionmgr know that we're ready.
	g_pServerMissionMgr->OnPlayerInWorld( *this );

	return true;
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

	// Save the command mgrs' global vars here so we bring them across levels

	g_pCmdMgr->SaveGlobalVars( pMsg, false );

	// Save animator

	m_Animator.Save(pMsg);

	m_Stats.WriteData(pMsg);
	m_Inventory.Save(pMsg);

	// Save PlayerObj data...

	SAVE_STDSTRING(m_sStartLevelCommand);

	SAVE_FLOAT(m_fOldHitPts);
	SAVE_FLOAT(m_fOldArmor);
	SAVE_FLOAT(m_fOldAirLevel);
	SAVE_FLOAT(m_fAirLevel);
	SAVE_BYTE(GetPlayerState( ));
	SAVE_BOOL(m_b3rdPersonView);
	SAVE_DWORD(m_nSavedFlags);
	SAVE_BOOL(m_bGodMode);
	SAVE_BOOL(m_bAllowInput);
	SAVE_bool( m_bCinematicInvulnerability );
	SAVE_BYTE(m_ePPhysicsModel);
	SAVE_bool(m_bNewMission);

	SAVE_WORD(m_nClientChangeFlags);

	SAVE_BYTE(m_nMotionStatus);
	SAVE_BYTE(m_nWeaponStatus);
	SAVE_BYTE(m_ePlayerLean);

	SAVE_HOBJECT( m_hPlayerLure ); 
	SAVE_BOOL( m_bForceDuck );


	SAVE_RIGIDTRANSFORM( m_tfCameraView );
	SAVE_RIGIDTRANSFORM( m_tfTrueCameraView );

	// [RP] 9/14/02 - Save off the velocity so we can zero out the players vel when they load
	//		and then reset the saved vel when they actually respawn in the world.

	g_pPhysicsLT->GetVelocity( m_hObject, &m_vSavedVelocity );
	SAVE_VECTOR( m_vSavedVelocity );

	// Save client data associated with this player...

	if (m_pClientSaveData)
	{
		SAVE_BOOL(true);
		pMsg->WriteMessage(m_pClientSaveData);

		SetClientSaveData( NULL );
	}
	else
	{
		SAVE_BOOL(false);
		// This should NEVER happen!
		LTASSERT(false, "TODO: Add description here");
	}


	// Save the weapon sound loop state
	SAVE_BYTE( m_nWeaponSoundLoopType );
	SAVE_BYTE( m_nWeaponSoundLoopWeapon );

	SaveOverlays(pMsg);

	// Save the sonic data
	m_iSonicData.Save( pMsg );

	SAVE_HRECORD( m_hPreviousWeapon );
	SAVE_HRECORD( m_hRequestedWeapon );

	SAVE_HOBJECT( m_StoryModeObject );
	SAVE_BOOL( m_bCanSkipStory );

	SAVE_BOOL(m_bFadeInitiated);
	if( m_bFadeInitiated )
	{
		SAVE_BOOL( m_bFadeIn );
		SAVE_FLOAT( LTMAX( m_fFadeTimeRemaining, 0.0f ) );
	}

	SAVE_HOBJECT( m_hTurret );

	// The Player saves the GameServerShell's UniqueObjectID 
	// so that it remains unique across TransAms.
	SAVE_DWORD( g_pGameServerShell->GetUniqueObjectID() );
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

	m_dwLastLoadFlags = dwLoadFlags;

	// Load the command mgrs' global vars here so we bring them across levels

	g_pCmdMgr->LoadGlobalVars( pMsg, false );

	// Load animator

	m_Animator.Load(pMsg);

	m_Stats.ReadData(pMsg);
	m_Inventory.Load(pMsg);

	// Load PlayerObj data...

	LOAD_STDSTRING( m_sStartLevelCommand );

	LOAD_FLOAT(m_fOldHitPts);
	LOAD_FLOAT(m_fOldArmor);
	LOAD_FLOAT(m_fOldAirLevel);
	LOAD_FLOAT(m_fAirLevel);
	LOAD_BYTE_CAST(m_ePlayerState, PlayerState);
	LOAD_BOOL(m_b3rdPersonView);
	LOAD_DWORD(m_nSavedFlags);
	LOAD_BOOL(m_bGodMode);
	LOAD_BOOL(m_bAllowInput);
	LOAD_bool( m_bCinematicInvulnerability );
	LOAD_BYTE_CAST(m_ePPhysicsModel, PlayerPhysicsModel);
	LOAD_bool( m_bNewMission );

	LOAD_WORD(m_nClientChangeFlags);

	LOAD_BYTE(m_nMotionStatus);
	LOAD_BYTE(m_nWeaponStatus);
	LOAD_BYTE_CAST(m_ePlayerLean, CPPlayerLeanTypes);

	LOAD_HOBJECT( m_hPlayerLure );
	LOAD_BOOL( m_bForceDuck );

	LOAD_RIGIDTRANSFORM( m_tfCameraView );
	LOAD_RIGIDTRANSFORM( m_tfTrueCameraView );

	// [RP] 9/14/02 - Load the saved velocity and zero out the objects velocity.  We'll reset it
	//		to the saved velocity later.

	LOAD_VECTOR( m_vSavedVelocity );
	LTVector vZero(0.0f, 0.0f, 0.0f);
	g_pPhysicsLT->SetVelocity( m_hObject, vZero );
	g_pPhysicsLT->SetAcceleration( m_hObject, vZero );

	// Get rid of the gravity flag so the physics won't pull us down if the playr was jumping when saved...

	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, m_dwSavedObjectFlags );
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, 0, FLAG_GRAVITY );

	// Load client data associated with this player...

	bool bClientSaveData;
	LOAD_BOOL(bClientSaveData);
	if (bClientSaveData)
	{
		SetClientSaveData( pMsg->ReadMessage() );
	}

	uint32 nWeaponSoundLoopType, nWeaponSoundLoopWeapon;
	LOAD_BYTE( nWeaponSoundLoopType );
	LOAD_BYTE( nWeaponSoundLoopWeapon );
	SetWeaponSoundLooping(nWeaponSoundLoopType, nWeaponSoundLoopWeapon);

	LoadOverlays(pMsg);

	HRECORD hSlowMoRecord = g_pGameServerShell->GetSlowMoRecord( );
	if( hSlowMoRecord )
	{
		// Enter slowmo without any transitions.
		EnterSlowMo( hSlowMoRecord, GetClient( ), INVALID_TEAM, kUsePlayerTimeScale | (m_Inventory.IsSlowMoPlayerControlled() ? kPlayerControlled : 0) );
	}

	// Load the sonic data
	m_iSonicData.Load( pMsg );

	LOAD_HRECORD( m_hPreviousWeapon, g_pWeaponDB->GetWeaponsCategory( ));
	LOAD_HRECORD( m_hRequestedWeapon, g_pWeaponDB->GetWeaponsCategory( ));

	LOAD_HOBJECT(m_StoryModeObject);
	LOAD_BOOL( m_bCanSkipStory );

	LOAD_BOOL( m_bFadeInitiated );
	if( m_bFadeInitiated )
	{
		LOAD_BOOL( m_bFadeIn );
		LOAD_FLOAT( m_fFadeTimeRemaining );
	}

	if( m_bFadeInitiated )
	{
		CAutoMessage cClientMsg;
		cClientMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cClientMsg.Writeuint8(IC_FADE_SCREEN_ID);
		cClientMsg.Writeuint8(m_bFadeIn ? 1 : 0);
		cClientMsg.Writebool(true);
		cClientMsg.Writefloat(m_fFadeTimeRemaining);
		g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
	}

	LOAD_HOBJECT( m_hTurret );

	// The Player loads the GameServerShell's UniqueObjectID 
	// so that it remains unique across TransAms.
	uint32 nUniqueObjectID;
	LOAD_DWORD( nUniqueObjectID );
	g_pGameServerShell->SetUniqueObjectID( nUniqueObjectID );
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
	if (!g_pWeaponDB) return;


	// Make sure we are using the correct model/material...

	ResetModel();

	// Let the client know what state we are in...
	SetPlayerState(GetPlayerState( ), true);

	// Make sure the interface is accurate...
	m_bForceUpdateInterface = true;

	//if we were in story mode make sure the client knows...
	if (InStoryMode())
	{
		StartStoryMode(m_StoryModeObject, m_bCanSkipStory, true);
	}

	if( m_dwLastLoadFlags == LOAD_NEW_LEVEL || m_dwLastLoadFlags == LOAD_TRANSITION )
	{
		// Don't allow slow-mo across level boundaries.
		SetTimerScale( 1, 1 );
	}
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

	{
		LTVector cEmptyVector(0,0,0);
		g_pPhysicsLT->SetVelocity(m_hObject, cEmptyVector);
	}

	{ ///////////////////////////////////////////////////////////////////////////

		// Clear any player/character data that we don't want to save
		// between levels.  NOTE:  This data will still be saved in normal
		// saved games

		m_fLastPainVolume			= 0.0f;

		m_eLastNavMeshPoly			= kNMPoly_Invalid;
		m_eCurrentNavMeshPoly		= kNMPoly_Invalid;
		m_vLastNavMeshPos			= LTVector(0,0,0);

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
				m_aSpears[iSpear].hObject = NULL;
			}
		}

	} ///////////////////////////////////////////////////////////////////////////

	// Build keep alives...
	AddToObjectList( pList );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ClientUpdate
//
//	PURPOSE:	Handle client update
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::ClientUpdate(ILTMessage_Read *pMsg)
{
	if (!g_pGameServerShell || !GetClient( )) return false;

	// read the timestamp
	uint32 nTimestamp = pMsg->Readuint32();
	if (s_vtAlwaysForceClientToServerPos.GetFloat() && (nTimestamp < m_nLastPositionForcedTime))
	{
		// this message was generated before we forced the client's position
		return false;
	}

	bool bRet = true;
	bool bOld3rdPersonView = m_b3rdPersonView;

    m_nClientChangeFlags = pMsg->Readuint8();

	// Update the camera data from the client...
	ClientCameraInfoUpdate( pMsg, m_nClientChangeFlags );

	if (m_nClientChangeFlags & CLIENTUPDATE_3RDPERSON)
	{
		m_b3rdPersonView = (m_nClientChangeFlags & CLIENTUPDATE_3RDPERVAL) ? true : false;
	}
	if (m_nClientChangeFlags & CLIENTUPDATE_ALLOWINPUT)
	{
		m_bAllowInput = pMsg->Readbool();
		bRet = false;
	}

	// Handle updating of animation trackers if needed...
	if( m_nClientChangeFlags & CLIENTUPDATE_ANIMATION )
	{
		// Read the animation message...
		ClientAnimationUpdate( pMsg );
	}

	m_bIsCrouched = pMsg->Readbool();

	// NOTE: Position info is handled outside of this ClientUpdate.
	//		 Perhaps it should be within this function?

	// Only change the client flags in multiplayer...

	if (IsMultiplayerGameServer( ))
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
	// See if we're under water...

	uint32 dwUserFlags = 0;
	if (IsLiquid(m_eContainerCode))
	{
		dwUserFlags |= USRFLG_PLAYER_UNDERWATER;
	}

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, dwUserFlags, USRFLG_PLAYER_UNDERWATER);
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
	SetLeashScale(m_LeashScaleTrack.GetFloat());
	SetLeashSpring(m_LeashSpringTrack.GetFloat());

	// Clamp the spring rate to [0.0,1.0]
	float fSpringRate = m_LeashSpringRateTrack.GetFloat();
	fSpringRate = LTCLAMP(fSpringRate, 0.0f, 1.0f);
	SetLeashSpringRate(fSpringRate);

	ChangeSpeedsVar(m_fBaseMoveAccel, m_BaseMoveAccelTrack.GetFloat());
	SetMoveMul( GameModeMgr::Instance( ).m_grfRunSpeed * m_vtRunSpeedMul.GetFloat( 1.0f ));
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
		m_damage.SetCanDamage(false);
		m_bCinematicInvulnerability = true;

		// Make sure we aren't moving...

		if (!m_bAllowInput)
		{
			// Don't cancel Y Vel/Accel so we can be moved to the ground...

			LTVector vVec;
			g_pPhysicsLT->GetVelocity(m_hObject, &vVec);
			vVec.x = vVec.z = 0.0f;
			g_pPhysicsLT->SetVelocity(m_hObject, vVec);
		}
	}
	else
	{
		// Check if we were set to invulnerable by a cinematic camera.
		if( m_bCinematicInvulnerability )
		{
			m_damage.SetCanDamage(true);
			m_bCinematicInvulnerability = false;
		}

		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		g_pLTServer->SetClientViewPos(GetClient( ), &vPos);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateClientFadeTime()
//
//	PURPOSE:	Update the time left on the current fade.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateClientFadeTime()
{
	if( m_fFadeTimeRemaining > 0.0f )
	{
		m_fFadeTimeRemaining -= EngineTimer( m_hObject ).GetTimerElapsedS();

		// If the fade is over.

		if( m_fFadeTimeRemaining <= 0.0f )
		{
			// Reset the timer.
			m_fFadeTimeRemaining = 0.0f;

			// Mark there being no fade in progress.
			if( m_bFadeIn )
			{
				m_bFadeInitiated = false;
			}
		}
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
	if(	!m_sStartLevelCommand.empty( ))
	{
		g_pCmdMgr->QueueCommand( m_sStartLevelCommand.c_str(), m_hObject, m_hObject );
		
		// Okay, this is a one-time only trigger, so remove the messages...
		m_sStartLevelCommand.clear( );
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
	// Skip out if it's not included
	if (!pMsg->Readbool())
		return;

	uint8  moveCode = pMsg->Readuint8();
	LTVector newPos = pMsg->ReadLTVector();
	LTVector newVel = pMsg->ReadLTVector();
	bool  bOnGround = pMsg->Readbool();

	m_eStandingOnSurface = (SurfaceType) pMsg->Readuint8();

	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
	if( !pGameClientData )
	{
		LTERROR( "Invalid gameclientdata" );
		return;
	}

	if (moveCode == pGameClientData->GetClientMoveCode())
	{
		SetOnGround(bOnGround);

		LTVector curPos;
		g_pLTServer->GetObjectPos(m_hObject, &curPos);

		if (IsMultiplayerGameServer( ))
		{
			// For client-side prediction...
			g_pPhysicsLT->SetVelocity(m_hObject, newVel);
		}

		m_vLastVelocity = newVel;


		// Don't move the player if the server is paused...
		if (!g_pGameServerShell->IsPaused( )) 
		{ 
			m_PlayerRigidBody.Update( newPos );

			if( !IsMultiplayerGameServer() )
			{
				if (!curPos.NearlyEquals(newPos, 0.1f))
				{
					// Move it first so we get collisions and stuff
					g_pLTServer->Physics()->MoveObject(m_hObject, newPos, 0);

					LTVector vResultPos;
					g_pLTServer->GetObjectPos( m_hObject, &vResultPos );

					if ( vResultPos != newPos )
					{
						// Then just teleport it there in case it didn't make it for some reason
						g_pLTServer->SetObjectPos(m_hObject, newPos);
					}
				}
			}
			else
			{
				// Turn gravity on if they are moving in the y dir and the object is solid.  Otherwise
				// make sure it's off.
				bool bGravityOn = ( newVel.y*newVel.y > 0.01f );
				uint32 nFlags = 0;
				g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, nFlags );
				bGravityOn = bGravityOn && ( nFlags & FLAG_SOLID );

				g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, bGravityOn ? FLAG_GRAVITY : 0, FLAG_GRAVITY);
				m_bUseLeash = true;
			}
		}
	}
	else
	{
		DebugCPrint(1,"Invalid client movement code received.");
	}

	// Remember where the client last said we were
	m_vLastClientPos = newPos;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::TeleportClientToServerPos()
//
//	PURPOSE:	This sends a message to the client telling it to move to
//				our position.  Used when loading games and respawning.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::TeleportClientToServerPos( bool bWithRotation )
{
	if (!GetClient( )) return;

	LTVector myPos;
	g_pLTServer->GetObjectPos(m_hObject, &myPos);

	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
	if( !pGameClientData )
	{
		LTERROR( "Invalid gameclientdata" );
		return;
	}

	// Advance the move code.
	pGameClientData->SetClientMoveCode( pGameClientData->GetClientMoveCode() + 1 );

	// Tell the player about the new move code.
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_CLIENT_PLAYER_UPDATE);
	cMsg.WriteBits(PSTATE_POSITION, PSTATE_NUMBITS);
	cMsg.Writeuint8(pGameClientData->GetClientMoveCode());
	// Need to send the position to the client in the message because
	// they client may not get our position normally for a few frames since
	// our position is unguaranteed.
	cMsg.WriteLTVector(myPos);

	// [RP] - Sending the dims because the client will begin using the dims from
	// the last game which may or may not be the same as the newly loaded dims.

	LTVector vDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);
	cMsg.WriteLTVector(vDims);

	cMsg.Writebool( bWithRotation );
	if( bWithRotation )
	{
		cMsg.WriteCompLTPolarCoord( m_tfTrueCameraView.m_rRot.Forward( ));
	}

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
	if (!g_pWeaponDB || !GetClient( )) return;

	// If we aren't dead, and we aren't in spectator mode, let us fire.

	if (m_damage.IsDead() || IsSpectating( ))
	{
		return;
	}

	// holding value for bytes
	uint8 cTemp;

	// get the object weapon
	CWeapon *pCurWeapon = m_Arsenal.GetCurWeapon();
	CWeapon *pWeapon = pCurWeapon;

	// Use the weapon known by the arsenal...
	HWEAPON hWeapon = m_Arsenal.GetCurWeaponRecord( );
	
	bool bReadWeaponRecord = pMsg->Readbool( );
	if( bReadWeaponRecord )
	{
		// The weapon record was sent from the client so retrieve it...
		hWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory( ));
	}

	// Ammo should initially be the weapons default, this maybe overridden if specified by the client...
	HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData( hWeapon, !USE_AI_DATA );
	HAMMO hAmmo  = g_pWeaponDB->GetRecordLink( hWeaponData, WDB_WEAPON_rAmmoName );
	
	bool bReadAmmoRecord = pMsg->Readbool( );
	if( bReadAmmoRecord )
	{
		// The ammo record was sent from the client so retrieve it...
		hAmmo = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
	}

	if( bReadWeaponRecord )
	{
		// Setup the weapon to be used for this fire message...
		m_Arsenal.ChangeWeapon( hWeapon );
		pWeapon = m_Arsenal.GetCurWeapon( );
		pWeapon->ReloadClip( false, 1, hAmmo );
	}

	
	if( m_hTurret )
	{
		Turret *pTurret = dynamic_cast<Turret*>(g_pLTServer->HandleToObject( m_hTurret ));
		if( pTurret )
		{
			pWeapon = pTurret->GetTurretWeapon( );
		}
	}

	// Make sure everything's kosher...
	if( !pWeapon || hWeapon != pWeapon->GetWeaponRecord() )
	{
		return;
	}

	if( hAmmo != pWeapon->GetAmmoRecord() )
	{
		const char *pszClientAmmo = g_pWeaponDB->GetRecordName( hAmmo );
		const char *pszServerAmmo = "NULL";
		if( pWeapon )
			pszServerAmmo = g_pWeaponDB->GetRecordName( pWeapon->GetAmmoRecord() );

		g_pLTServer->CPrint("Client Ammo (%s) != Server Ammo (%s)", pszClientAmmo, pszServerAmmo );
		LTERROR( "ERROR - Client AmmoId doesn't match server AmmoId" );
		return;
	}

	// Set the ammo type in the weapon...
	pWeapon->SetAmmo( hAmmo );

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA);
	if( g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType ) == TRIGGER )
	{
		const char* pszMsg = g_pWeaponDB->GetString(hAmmoData,WDB_AMMO_sTriggerMessage);
		g_pCmdMgr->QueueMessage( this, this, pszMsg );
	}
	else
	{
		//
		// create the fire info and fill it with info from the message
		//
		WeaponFireInfo weaponFireInfo;

		// who fired the shot
		weaponFireInfo.hFiredFrom = m_hObject;
		weaponFireInfo.hFiringWeapon = ( GetTurret( ) ? GetTurret( ) : pWeapon->GetModelObject());

		// get the fire position
		weaponFireInfo.vFirePos = pMsg->ReadLTVector();
		
		// get the direction of travel
		weaponFireInfo.vPath = pMsg->ReadLTVector();

		// get the random number seed
		weaponFireInfo.nSeed = pMsg->Readuint8();

		// get the perturb count
		weaponFireInfo.nPerturbCount = pMsg->Readuint8();

		/*
		// AltFire is not currently implemented
		*/
		weaponFireInfo.bAltFire = false;

		// get the perturb
		cTemp = pMsg->Readuint8();
		weaponFireInfo.fPerturb	= static_cast< float >( cTemp ) / 255.0f;

		// get the time stamp when the shot was fired
		weaponFireInfo.nFireTimestamp = pMsg->Readuint32();

		//which hand did the firing
		weaponFireInfo.bLeftHandWeapon = pMsg->Readbool();

		// If we're in 3rd person view, use the hand held weapon fire pos.
		if (m_b3rdPersonView)
		{
			weaponFireInfo.vFirePos  = HandHeldWeaponFirePos(pWeapon);
			weaponFireInfo.vFlashPos = weaponFireInfo.vFirePos;
		}

		// If the weapon or ammo record was read from the client message it needs to be sent to other clients...
		weaponFireInfo.bSendWeaponRecord = bReadWeaponRecord;
		weaponFireInfo.bSendAmmoRecord = bReadAmmoRecord;

		// Find out how much ammo we use for this fire to update our interface data.
		int32 nAmmoCount = m_Arsenal.GetAmmoCount( hAmmo );

		pWeapon->Fire(weaponFireInfo);	

		// Update the client stats so we don't end
		// up sending ammo messages for what the client already knows.
		int32 nAmmoUsed = nAmmoCount - m_Arsenal.GetAmmoCount( hAmmo );
		uint32 nAmmoIndex = g_pWeaponDB->GetRecordIndex( hAmmo );
		m_pnOldAmmo[nAmmoIndex] -= nAmmoUsed;
	}

	// Switch back to weapon used before switching to weapon sent from client...
	if( bReadWeaponRecord )
	{
		if( pCurWeapon )
            m_Arsenal.ChangeWeapon( pCurWeapon->GetWeaponRecord( ), pCurWeapon->GetAmmoRecord( ));
		else
			m_Arsenal.DeselectCurWeapon();
	}

	// Update number of shots fired...

	if( IsAccuracyType( g_pWeaponDB->GetAmmoInstDamageType( hAmmo ) ))
	{
		m_Stats.dwNumShotsFired += g_pWeaponDB->GetInt32( hWeaponData, WDB_WEAPON_nVectorsPerRound );
	}

	// Reset the weapon state to none, firing takes priority

	m_nWeaponStatus = WS_NONE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponFinishMessage
//
//	PURPOSE:	Handle player finishing move damage / momentum
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponFinishMessage(ILTMessage_Read *pMsg)
{
	if (!g_pWeaponDB || !GetClient( )) return;

	// If we aren't dead, and we aren't in spectator mode, let us fire.

	if (m_damage.IsDead() || IsSpectating( ))
	{
		return;
	}

	HOBJECT hTarget = pMsg->ReadObject();
	float fImpulse = pMsg->Readfloat();
	LTVector vDir = pMsg->ReadLTVector();
	LTVector vImpactPos = pMsg->ReadLTVector();

	PhysicsUtilities::ApplyPhysicsImpulseForce(hTarget, fImpulse, vDir, vImpactPos, false);

	DamageStruct damage;
	damage.hDamager			= hTarget;
	damage.fImpulseForce	= fImpulse;
	damage.SetPositionalInfo(vImpactPos, vDir);
	damage.eType			= DT_MELEE;
	damage.fDamage			= damage.kInfiniteDamage;
	damage.DoDamage(m_hObject, hTarget);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleWeaponFinishRagdollMessage
//
//	PURPOSE:	Handle player finishing move damage / momentum
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleWeaponFinishRagdollMessage(ILTMessage_Read *pMsg)
{
	if (!g_pWeaponDB || !GetClient( )) return;

	// If we aren't dead, and we aren't in spectator mode, let us fire.

	if (m_damage.IsDead() || IsSpectating( ))
	{
		return;
	}

	// Can't grab two things at once!
	if (m_hGrabConstraint != INVALID_PHYSICS_BREAKABLE)
	{
		return;
	}

	HOBJECT hTarget = pMsg->ReadObject();
	float fDuration = pMsg->Readfloat();
	float fBreakForce = pMsg->Readfloat();
	LTVector vImpactPos = pMsg->ReadLTVector();

	DamageStruct damage;
	damage.hDamager			= hTarget;
	damage.SetPositionalInfo(vImpactPos, LTVector(0,-1,0));
	damage.eType			= DT_MELEE;
	damage.fDamage			= damage.kInfiniteDamage;
	damage.DoDamage(m_hObject, hTarget);

	m_fGrabEndTime = g_pLTServer->GetTime() + fDuration;

	ILTPhysicsSim* pPhysicsSim = g_pLTBase->PhysicsSim();
	HPHYSICSRIGIDBODY hPlayerBody = PhysicsUtilities::GetClosestRigidBody(m_hObject, vImpactPos);
	if (hPlayerBody != INVALID_PHYSICS_RIGID_BODY)
	{
		HPHYSICSRIGIDBODY hTargetBody = PhysicsUtilities::GetClosestRigidBody(hTarget, vImpactPos);
		if (hTargetBody != INVALID_PHYSICS_RIGID_BODY)
		{
			LTRigidTransform tPlayerBody;
			if (pPhysicsSim->GetRigidBodyTransform(hPlayerBody, tPlayerBody) == LT_OK)
			{
				LTRigidTransform tTargetBody;
				if (pPhysicsSim->GetRigidBodyTransform(hTargetBody, tTargetBody) == LT_OK)
				{
					LTVector vPlayerPos = tPlayerBody.GetInverse() * vImpactPos;
					LTVector vTargetPos = tTargetBody.GetInverse() * vImpactPos;

					HPHYSICSCONSTRAINT hConstraint = pPhysicsSim->CreateBallAndSocket(hPlayerBody, hTargetBody, vPlayerPos, vTargetPos);
					if (hConstraint != INVALID_PHYSICS_CONSTRAINT)
					{
						m_hGrabConstraint = pPhysicsSim->CreateBreakable(hConstraint, fBreakForce);
						if (m_hGrabConstraint != INVALID_PHYSICS_BREAKABLE)
						{
//							pPhysicsSim->AddBreakableToSimulation(m_hGrabConstraint);
							m_pPendingGrab = CCharacter::DynamicCast(hTarget);
						}
						pPhysicsSim->ReleaseConstraint(hConstraint);
					}
				}
			}
			pPhysicsSim->ReleaseRigidBody(hTargetBody);
		}
		pPhysicsSim->ReleaseRigidBody(hPlayerBody);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleDropGrenadeMessage
//
//	PURPOSE:	Handle drop grenade message
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleDropGrenadeMessage( ILTMessage_Read *pMsg )
{
	if( !pMsg || !GetClient( ))
		return;

	// Get the weapon id
	HWEAPON hWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );

	// get the ammo id
	HAMMO hAmmo = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );

	HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData( hWeapon, !USE_AI_DATA );
	
	// Make sure it's actually a grenade being dropped...
	if( !g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bIsGrenade ))
		return;
	
	CWeapon *pCurWeapon = m_Arsenal.GetCurWeapon();

	m_Arsenal.ObtainWeapon( hWeapon, hAmmo, 1 );
	m_Arsenal.ChangeWeapon( hWeapon );
	CWeapon *pWeapon = m_Arsenal.GetCurWeapon( );
	pWeapon->ReloadClip( false, 1, hAmmo );

	//
	// create the fire info and fill it with info from the message
	//
	WeaponFireInfo weaponFireInfo;
	static uint8 s_nCount = GetRandom( 0, 255 );
	s_nCount++;

	// who fired the shot
	weaponFireInfo.hFiredFrom = m_hObject;
	weaponFireInfo.hFiringWeapon = ( GetTurret( ) ? GetTurret( ) : pWeapon->GetModelObject());

	// get the flash position
	weaponFireInfo.vFlashPos = pMsg->ReadLTVector();

	// get the fire position
	weaponFireInfo.vFirePos = pMsg->ReadLTVector();

	// get the direction of travel
	weaponFireInfo.vPath = pMsg->ReadLTVector();

	// Override the velocity...
	weaponFireInfo.bOverrideVelocity = true;
	weaponFireInfo.fOverrideVelocity = 10.0f;

	weaponFireInfo.nSeed = (uint8)GetRandom( 2, 255 );
	weaponFireInfo.nPerturbCount	= s_nCount;

	/*
	// AltFire is not currently implemented
	*/
	weaponFireInfo.bAltFire = false;

	// get the perturb (note: R and U (up&down and side-to-side) are the same)
	uint8 cTemp = pMsg->Readuint8();
	weaponFireInfo.fPerturb	= static_cast< float >( cTemp ) / 255.0f;

	// get the timestamp when the shot was fired
	weaponFireInfo.nFireTimestamp = pMsg->Readuint32();

	pWeapon->Fire(weaponFireInfo);

	if( pCurWeapon )
		m_Arsenal.ChangeWeapon( pCurWeapon->GetWeaponRecord( ), pCurWeapon->GetAmmoRecord( ));
	else
		m_Arsenal.DeselectCurWeapon();

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

	uint8 nType		= pMsg->Readuint8();
	HWEAPON hWeapon	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	uint8 nId		= pMsg->Readuint8();
	LTVector vPos	= pMsg->ReadLTVector();

	uint8 nClientID	= (uint8) g_pLTServer->GetClientID(GetClient( ));

	CAutoMessage cSoundMsg;
	cSoundMsg.Writeuint8(SFX_PLAYERSOUND_ID);
	cSoundMsg.Writeuint8(nType);
	cSoundMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
	cSoundMsg.Writeuint8(nClientID);
	cSoundMsg.WriteCompPos(vPos);
	g_pLTServer->SendSFXMessage(cSoundMsg.Read(), 0);
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

	SetWeaponSoundLooping( nType, nWeaponId );
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
	LTRotation rRot = pMsg->ReadLTRotation();

	Teleport(vPos, rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandlePlayerEventMsg
//
//	PURPOSE:	Handle player events
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandlePlayerEventMsg(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	static uint32 nStartPoint = 0;

	PlayerEventMsgType eType = static_cast<PlayerEventMsgType>(pMsg->Readuint8());

	switch(eType)
	{
	case kPEDropWeapon:
		{
			DropCurrentWeapon();
		}
		break;
	case kPENextSpawnPoint:
		{
			nStartPoint++;
			if (nStartPoint >= GameStartPoint::GetStartPointList().size())
			{
				nStartPoint = 0;
			}
			GameStartPoint* pGSP = GameStartPoint::GetStartPointList()[nStartPoint];
			if (pGSP)
			{
				char szName[64];
				szName[0] = '\0';
				g_pLTServer->GetObjectName(pGSP->m_hObject, szName, sizeof(szName));
				g_pLTBase->CPrint("Teleport to: %s", szName );

				LTRigidTransform trans;
				g_pLTServer->GetObjectTransform(pGSP->m_hObject, &trans);

				// Place the object and character hit box at the start point...
				Teleport( trans.m_vPos,trans.m_rRot );

			}
		}
		break;
	case kPEPrevSpawnPoint:
		{
			if (nStartPoint == 0)
			{
				nStartPoint = GameStartPoint::GetStartPointList().size() - 1;
			}
			else
			{
				nStartPoint--;

			}
			GameStartPoint* pGSP = GameStartPoint::GetStartPointList()[nStartPoint];
			if (pGSP)
			{
				char szName[64];
				szName[0] = '\0';
				g_pLTServer->GetObjectName(pGSP->m_hObject, szName, sizeof(szName));
				g_pLTBase->CPrint("Teleport to: %s", szName );

				LTRigidTransform trans;
				g_pLTServer->GetObjectTransform(pGSP->m_hObject, &trans);

				// Place the object and character hit box at the start point...
				Teleport( trans.m_vPos,trans.m_rRot );

			}
		}
		break;
	}

};


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
			m_nMotionStatus = pMsg->ReadBits( FNumBitsExclusive<MS_COUNT>::k_nValue );
			HRECORD hLandSound = NULL;
			HOBJECT hPolyGrid = NULL;
			if( m_nMotionStatus == MS_LANDED )
			{
				hLandSound = pMsg->ReadDatabaseRecord(g_pLTDatabase, g_pSoundDB->GetSoundCategory());
				// Read any polygrid they splashed into.
				if( pMsg->Readbool( ))
				{
					hPolyGrid = pMsg->ReadObject();
				}
				}

			// Relay this to all other clients.
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_SFX_MESSAGE);
			cMsg.Writeuint8(SFX_CHARACTER_ID);
			cMsg.WriteObject(m_hObject);
			cMsg.WriteBits(CFX_MOTION_STATUS_FX, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			cMsg.WriteBits(m_nMotionStatus, FNumBitsExclusive<MS_COUNT>::k_nValue );
			if( m_nMotionStatus == MS_LANDED )
				{
				cMsg.WriteDatabaseRecord( g_pLTDatabase, hLandSound );
				cMsg.WriteObject( hPolyGrid );
			}
			SendToClientsExcept( *cMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
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

			damage.eType	= static_cast<DamageType> (pMsg->Readuint8());
			damage.fDamage  = pMsg->Readfloat();

			// BL 10/30/00 - if this is vehicle impact damage, read out the position of the impact
			LTVector vReportedPos;
			if ( byMessage == CP_DAMAGE_VEHICLE_IMPACT )
			{
				LTVector vDir	= pMsg->ReadLTVector();
				vReportedPos	= pMsg->ReadLTVector();
				damage.SetPositionalInfo(vReportedPos, vDir);
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

						float fDistance = vPos.Dist(vReportedPos);
						float fDamageModifier = 1.0f;

						if(!s_vtVehicleImpactDistMin.IsInitted())
							s_vtVehicleImpactDistMin.Init(g_pLTServer, "VehicleImpactDistMin", NULL, 100.0f);
						if(!s_vtVehicleImpactDistMax.IsInitted())
							s_vtVehicleImpactDistMax.Init(g_pLTServer, "VehicleImpactDistMax", NULL, 300.0f);

						float fMinDistance = s_vtVehicleImpactDistMin.GetFloat(100.0f);
						float fMaxDistance = s_vtVehicleImpactDistMax.GetFloat(300.0f);

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
				damage.DoDamage(m_hObject, hObj);
			}
		}
		break;

	case CP_PHYSICSMODEL :
		{
			PlayerPhysicsModel eModel = (PlayerPhysicsModel)pMsg->Readuint8();
			ModelsDB::HMODEL hModel = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pModelsDB->GetModelsCategory());

			SetPhysicsModel( eModel, hModel, false );
		}
		break;
	case CP_LADDER_SLIDE:
		{
			m_bSliding = pMsg->Readbool();

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_SFX_MESSAGE);
			cMsg.Writeuint8(SFX_CHARACTER_ID);
			cMsg.WriteObject(m_hObject);
			cMsg.WriteBits(CFX_SLIDE, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			cMsg.Writebool(m_bSliding);
			g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

			// Update our special fx message so new clients will get the updated
			// info as well...

			CreateSpecialFX();

		}
		break;
	case CP_STORY_CANCEL:
		{
			if (InStoryMode())
			{
				StoryMode* pSM = dynamic_cast< StoryMode* >( g_pLTServer->HandleToObject( m_StoryModeObject ));
				pSM->End(true);
				EndStoryMode();
			}
		}
		break;

	default : break;
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

void CPlayerObj::PushCharacter(const LTVector &vPos, float fRadius, float fStartDelay, float fDuration, float fStrength)
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
		g_pLTServer->SendToClient(cMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
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

				damage.eType	= DT_EXPLODE;
				damage.fDamage	= damage.kInfiniteDamage;
				damage.hDamager = m_hObject;

				damage.DoDamage(m_hObject, hObject);
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
	if ( !m_pAttachments )
	{
		m_pAttachments = static_cast<CAttachments*>(CAttachments::Create());
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
//	if( bRemove )
//	{
//		m_pAttachments = NULL;
//	}

	return CCharacter::TransferAttachments( bRemove );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DropCurrentWeapon()
//
//	PURPOSE:	Drop our current weapon
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DropCurrentWeapon()
{
	if( !m_pAttachments )
		return;

	//we only want to drop our current weapon
	CWeapon* pWeapon = m_Arsenal.GetCurWeapon();

	//if the player has the weapon...
	if( !pWeapon || !pWeapon->GetActiveWeapon( ))
		return;

	LTVector vPos = m_tfTrueCameraView.m_vPos;
	vPos += (m_tfTrueCameraView.m_rRot.Up()*-20.0f);
	



	m_Inventory.DropWeapon(pWeapon->GetWeaponRecord(),m_tfTrueCameraView.m_vPos, m_tfTrueCameraView.m_rRot, m_vLastVelocity,true);

	}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::DropWeapons
//
//	PURPOSE:	Drop our weapons
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DropWeapons()
{
	DropCurrentWeapon();
	RemoveWeapons();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetPhysicsModel
//
//	PURPOSE:	Set the physics model
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::SetPhysicsModel( PlayerPhysicsModel eModel /* = PPM_NORMAL */, 
								 ModelsDB::HMODEL hModel /* = NULL */,
								 bool bUpdateClient /* = true  */ )
{
	if (m_ePPhysicsModel == eModel)
		return false;

	if (bUpdateClient)
	{
		m_PStateChangeFlags |= PSTATE_PHYSICS_MODEL;
	}

	switch (eModel)
	{
	default :
	case PPM_NORMAL :
		SetNormalPhysicsModel();
		break;
	}

	m_ePPhysicsModel = eModel;


	//even if we don't update the state change flags, we still want to update the CharacterFX
	if (!bUpdateClient)
	{
		// Send the new player physics model to the clients so they can perform vehicle dependent
		// actions and play sounds based on the specific vehicle...

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_CHARACTER_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.WriteBits(CFX_PLAYER_PHYSICS_MODEL, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint8( m_ePPhysicsModel );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
	}

	CreateSpecialFX();

	return true;
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

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::GetFootstepVolume
//
//	PURPOSE:	Determines our footstep volume
//
// ----------------------------------------------------------------------- //

float	CPlayerObj::GetFootstepVolume()
{
	// BRIANL TEMP HACK.  6/21/04
	//
	// Currently, m_Animator is always disabled to address a playerbody 
	// issue.  This resulted in no footstep sounds, as this function was
	// returning 0.  As this made it difficult for our level designers to
	// evaluate the AIs behavior, the players run volume will be returned 
	// for now.  This can be removed once the playerbody issues are 
	// addressed.
	if( m_Animator.IsDisabled())
		return g_pServerDB->GetPlayerFloat(PLAYER_BUTE_RUNVOLUME);

	switch (m_Animator.GetLastMovement())
	{
	case CAnimatorPlayer::eWalking:
		{
			return g_pServerDB->GetPlayerFloat(PLAYER_BUTE_WALKVOLUME);
		}
		break;

	case CAnimatorPlayer::eJumping:
	case CAnimatorPlayer::eRunning:
		{
			return g_pServerDB->GetPlayerFloat(PLAYER_BUTE_RUNVOLUME);
		}
		break;

	case CAnimatorPlayer::eCrouching:
		{
			return g_pServerDB->GetPlayerFloat(PLAYER_BUTE_CROUCHVOLUME);
		}
		break;

	default:
		{
			return g_pServerDB->GetPlayerFloat(PLAYER_BUTE_WALKVOLUME);
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
	uint8 nClientID = GetClient( ) ? (uint8)g_pLTServer->GetClientID(GetClient( )) : INVALID_TEAM;

	// Check if already set.
	if( nClientID == m_cs.nClientID )
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.WriteBits(CFX_CLIENTID_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.WriteBits(nClientID, FNumBitsExclusive<MAX_MULTI_PLAYERS*2>::k_nValue);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

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

void CPlayerObj::SetChatting(bool bChatting)
{
	m_bChatting = bChatting;

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.WriteBits(CFX_CHAT_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.Writeuint8(bChatting);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetHasSlowMoRecharge()
//
//	PURPOSE:	handle player chat mode
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetHasSlowMoRecharge(bool bSlowMoRecharge)
{
	// Check if we already know we have it.
	if( m_bSlowMoCharge == bSlowMoRecharge )
		return;

	m_bSlowMoCharge = bSlowMoRecharge;
	HRECORD hGlobalRec = g_pWeaponDB->GetGlobalRecord();

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);

	if (bSlowMoRecharge)
	{
		cMsg.WriteBits(CFX_CREATE_LOOP_FX_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.WriteString( g_pWeaponDB->GetString(hGlobalRec,WDB_GLOBAL_sSlowMoRechargerFXName) );
	}
	else
	{
		cMsg.WriteBits(CFX_KILL_LOOP_FX_MSG , FNumBitsExclusive<CFX_COUNT>::k_nValue );
	}

	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	if( bSlowMoRecharge )
	{
		float fSlowMoHoldScorePeriod = (float)GameModeMgr::Instance().m_grnSlowMoHoldScorePeriod;
		if( fSlowMoHoldScorePeriod > 0.0f )
		{
			m_tmrSlowMoHoldCharger.Start( fSlowMoHoldScorePeriod );
		}
	}
	else
	{
		m_tmrSlowMoHoldCharger.Stop( );
	}

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


float CPlayerObj::ComputeDamageModifier(ModelsDB::HNODE hModelNode)
{

	if( !IsMultiplayerGameServer() )
		return 1.0f;

	float fModifier = CCharacter::ComputeDamageModifier(hModelNode);

	float fMaxModifier = GetConsoleFloat( "MaxDamageModifier", 3.0f );
	fModifier = LTMIN( fMaxModifier, fModifier );

	return fModifier;
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

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetClientSaveData
//
//	PURPOSE:	
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

void CPlayerObj::DoWeaponReload( HWEAPON hWeapon, HAMMO hAmmo )
{
	CWeapon *pWeapon = m_Arsenal.GetWeapon( hWeapon );
	if( pWeapon )
	{
		pWeapon->ReloadClip( true, -1, hAmmo );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::DoWeaponSwap
//
//  PURPOSE:	Swap one weapon for another
//
// ----------------------------------------------------------------------- //

void CPlayerObj::DoWeaponSwap( HWEAPON hFromWeapon, HWEAPON hToWeapon )
{
	m_Inventory.SwapWeapon( hFromWeapon, hToWeapon, true );
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

	// Don't preserve slowmo across levels.
	ExitSlowMo( false );

	// Don't preserve fade status between levels.
	m_bFadeInitiated = false;
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
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

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
	CCharacter::RemoveAttachments( bDestroyAttachments );
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

	HRECORD hMission = g_pMissionDB->GetMission( nMissionId );
	if( !hMission )
	{
		LTERROR( "CPlayerObj::AcquireLevelDefaultWeapons: Invalid mission." );
		return false;
	}

	HRECORD hLevel = g_pMissionDB->GetLevel( hMission, nLevelId );
	if( !hLevel )
	{
		LTERROR( "CPlayerObj::AcquireLevelDefaultWeapons: Invalid level." );
		return false;
	}

	if( !m_pAttachments )
	{
		LTERROR( "CPlayerObj::AcquireLevelDefaultWeapons: Invalid attachments object." );
		return false;
	}

	for( uint32 i = 0; i < g_pMissionDB->GetNumValues(hMission,MDB_DefaultWeapons); ++i )
	{
		HWEAPON hWeapon = g_pMissionDB->GetRecordLink(hMission,MDB_DefaultWeapons,i);
		if( !hWeapon )
			continue;

		// Check if we already have this weapon.
		CWeapon* pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( pWeapon )
		{
			continue;
		}

		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		if( g_pWeaponDB->IsRestricted( hWpnData ))
		{
			continue;
		}
		HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName ) ;
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA); 
		if( !hAmmo || g_pWeaponDB->IsRestricted( hAmmoData ))
		{
			continue;
		}

		uint32 nSelectionAmmount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nSelectionAmount );
		if( !m_Inventory.AcquireWeapon( hWeapon, hAmmo, nSelectionAmmount, true ))
			continue;
		pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( !pWeapon )
		{
			LTERROR( "CPlayerObj::AcquireLevelDefaultWeapons: Could not obtain weapon." );
			continue;
		}
	}

	// Add the level specific defaults...

	for( uint32 i = 0; i < g_pMissionDB->GetNumValues(hLevel,MDB_DefaultWeapons); ++i )
	{
		HWEAPON hWeapon = g_pMissionDB->GetRecordLink(hLevel,MDB_DefaultWeapons,i);
		if( !hWeapon )
			continue;

		// Check if we already have this weapon.
		CWeapon* pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( pWeapon )
		{
			continue;
		}

		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		if( g_pWeaponDB->IsRestricted( hWpnData ))
		{
			continue;
		}
		
		HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName ) ;
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA); 
		if( !hAmmo || g_pWeaponDB->IsRestricted( hAmmoData ))
		{
			continue;
		}

		uint32 nSelectionAmmount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nSelectionAmount );
		if( !m_Inventory.AcquireWeapon( hWeapon, hAmmo, nSelectionAmmount, true ))
			continue;
		pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( !pWeapon )
		{
			LTERROR( "CPlayerObj::AcquireLevelDefaultWeapons: Could not obtain weapon." );
			continue;
		}
	}


	// If specified, switch to the selected weapon for this mission...
	if (nLevelId == 0)
	{
		HWEAPON hWeapon = g_pMissionDB->GetRecordLink(hMission,MDB_SelectedWeapon);
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		if( hWeapon && !g_pWeaponDB->IsRestricted( hWpnData ))
		{
			m_hRequestedWeapon = hWeapon;
		}
		//	ChangeWeapon( hWeapon, true, NULL, false );
		else
		{
			// Don't change to the default weapon if the player is not to be reset...

			if( g_pMissionDB->GetBool(hMission, MDB_ResetPlayer) || g_pGameServerShell->GetLGFlags( ) == LOAD_NEW_GAME )
			{
				// No selected weapon so make sure we have the default weapon selected...

				HWEAPON hWeapon = g_pServerDB->GetPlayerDefaultWeapon();
				if( hWeapon )
				{
					m_hRequestedWeapon = hWeapon;
//					ChangeWeapon( hWeapon, true, NULL, false );
				}
			}
		}
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

	HRECORD hMission = g_pMissionDB->GetMission( nMissionId );
	if( !hMission )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid mission." );
		return false;
	}

	HRECORD hLevel = g_pMissionDB->GetLevel( hMission, nLevelId );
	if( !hLevel )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid level." );
		return false;
	}

	if( !m_pAttachments )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid attachments object." );
		return false;
	}

	// Add the mission specific defaults...

	for( uint32 i = 0; i < g_pMissionDB->GetNumValues(hMission,MDB_DefaultMods); ++i )
	{
		HWEAPON hMod = g_pMissionDB->GetRecordLink(hMission,MDB_DefaultMods,i);
		if( !hMod )
		{
			continue;
		}

		HWEAPON hWeapon = g_pWeaponDB->GetWeaponFromMod( hMod, !USE_AI_DATA );
		CWeapon* pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( !pWeapon )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Don't have weapon." );
			continue;
		}

		// Don't add the mod again.
		if( pWeapon->HaveMod( hMod ))
			continue;

		m_Inventory.AcquireMod( hMod, false );
	}


	// Add the level specific defaults...
	for( uint32 i = 0; i < g_pMissionDB->GetNumValues(hLevel,MDB_DefaultMods); ++i )
	{
		HWEAPON hMod = g_pMissionDB->GetRecordLink(hLevel,MDB_DefaultMods,i);
		if( !hMod )
		{
			continue;
		}

		HWEAPON hWeapon = g_pWeaponDB->GetWeaponFromMod( hMod, !USE_AI_DATA );
		CWeapon* pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( !pWeapon )
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Don't have weapon." );
			continue;
		}

		// Don't add the mod again.
		if( pWeapon->HaveMod( hMod ))
			continue;

		m_Inventory.AcquireMod( hMod, false );
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
	ASSERT( !"CPlayerObj::AcquireLevelDefaultAmmo() function disabled." );
	return false;
/*
	// Make sure the mission and level are valid...

	HRECORD hMission = g_pMissionDB->GetMission( nMissionId );
	if( !pMission )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultAmmo: Invalid mission." );
		return false;
	}

	LEVEL *pLevel = g_pMissionDB->GetLevel( nMissionId, nLevelId );
	if( !pLevel )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultAmmo: Invalid level." );
		return false;
	}

	if( !m_pAttachments )
	{
		ASSERT( !"CPlayerObj::AcquireLevelDefaultAmmo: Invalid attachments object." );
		return false;
	}

	// Add the mission specific defaults...

	const AMMO *pAmmo = NULL;
	for( int i = 0; i < pMission->nNumDefaultAmmo; ++i )
	{
		HAMMO hAmmo = pMission->ahDefaultAmmo[i];
		if( !hAmmo || g_pWeaponDB->IsRestricted( hAmmo ))
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid ammoid" );
			continue;
		}

		if( m_Arsenal.GetAmmoCount( hAmmo ) <= 0 )
			m_Inventory.AcquireAmmo( hAmmo );
	}

	// Add the level specific defaults...

	for( i = 0; i < pLevel->nNumDefaultAmmo; ++i )
	{
		HAMMO hAmmo = pLevel->ahDefaultAmmo[i];
		if( !hAmmo || g_pWeaponDB->IsRestricted( hAmmo ))
		{
			ASSERT( !"CPlayerObj::AcquireLevelDefaultMods: Invalid ammoid" );
			continue;
		}

		if( m_Arsenal.GetAmmoCount( hAmmo ) <= 0 )
			m_Inventory.AcquireAmmo( hAmmo );
	}

	return true;
*/
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
	if (IsMultiplayerGameServer())
	{
		if ( GameModeMgr::Instance( ).m_grbUseLoadout )
		{
			return AcquireMPLoadoutWeapons();
		}
		else
		{
		return AcquireMPDefaultWeapons();
	}
	}
	else
	{
		// Get the mission and lefel defaults...

		int nMissionId	= g_pServerMissionMgr->GetCurrentMission();
		int nLevelId	= g_pServerMissionMgr->GetCurrentLevel();


		if( !AcquireLevelDefaultWeapons( nMissionId, nLevelId ))
			return false;

		if( !AcquireLevelDefaultMods( nMissionId, nLevelId ))
			return false;
		/*
		if( !AcquireLevelDefaultAmmo( nMissionId, nLevelId ))
		return false;
		*/
		return true;

	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::AcquireMPDefaultWeapons
//
//  PURPOSE:	Obtaian the multiplayer default weapons...
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::AcquireMPDefaultWeapons()
{
	if( !m_pAttachments )
	{
		LTERROR( "CPlayerObj::AcquireMPDefaultWeapons: Invalid attachments object." );
		return false;
	}

	HRECORD hGlobalRec = g_pWeaponDB->GetGlobalRecord();
	HATTRIBUTE hMPDefsAtt = g_pLTDatabase->GetAttribute(hGlobalRec,WDB_GLOBAL_rMPDefaultWeapons);
	uint32 nNumDefs = g_pLTDatabase->GetNumValues(hMPDefsAtt);

	//keep track of the best weapon so we can selected it later
	HWEAPON hBest = NULL;

	//step through the list of weapons
	for (uint32 n = 0; n < nNumDefs; ++n)
	{
		HWEAPON hWeapon = g_pLTDatabase->GetRecordLink(hMPDefsAtt,n,NULL);
		if( !hWeapon )
			continue;

		//compare this weapon with the best we've gotten so far
		if (!hBest || IsPreferredWeapon(hWeapon,hBest))
		{
			hBest = hWeapon;
		}


		// Check if we already have this weapon.
		CWeapon* pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( pWeapon )
		{
			continue;
		}

		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		if( g_pWeaponDB->IsRestricted( hWpnData ))
		{
			continue;
		}
		
		HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA); 
		if( !hAmmo || g_pWeaponDB->IsRestricted( hAmmo ))
		{
			continue;
		}

		uint32 nSelectionAmmount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nSelectionAmount );
		m_Inventory.AcquireWeapon( hWeapon, hAmmo, nSelectionAmmount, true );
		pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( !pWeapon )
		{
			LTERROR( "CPlayerObj::AcquireMPDefaultWeapons: Could not obtain weapon." );
			continue;
		}



	}

	if (hBest)
	{
		ChangeWeapon(hBest,true, NULL, false, false );
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::AcquireMPLoadoutWeapons
//
//  PURPOSE:	Obtain the weapons based on loadout...
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::AcquireMPLoadoutWeapons()
{
	if( !m_pAttachments )
	{
		LTERROR( "CPlayerObj::AcquireMPLoadoutWeapons: Invalid attachments object." ); 
		return false;
	}

	if (m_nLoadout > g_pWeaponDB->GetNumLoadouts())
	{
		//should only get here if all the loadout weapons are restricted...
		return true;
	}

	HRECORD hLoadout = NULL;
	if (m_nLoadout == g_pWeaponDB->GetNumLoadouts())
	{
		hLoadout = g_pWeaponDB->GetFallbackLoadout();
	}
	else
	{
		hLoadout = g_pWeaponDB->GetLoadout(m_nLoadout);
	}
	
	if (!hLoadout) return false;

	const char *szNm = g_pWeaponDB->GetRecordName(hLoadout);

	HATTRIBUTE hWpnAtt = g_pLTDatabase->GetAttribute(hLoadout,WDB_LOADOUT_rWeapons);
	uint32 nNumWpns = g_pLTDatabase->GetNumValues(hWpnAtt);

	//keep track of the best weapon so we can selected it later
	HWEAPON hBest = NULL;

	//step through the list of weapons
	for (uint32 n = 0; n < nNumWpns; ++n)
	{
		HWEAPON hWeapon = g_pLTDatabase->GetRecordLink(hWpnAtt,n,NULL);
		if( !hWeapon )
			continue;

		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		if( g_pWeaponDB->IsRestricted( hWpnData ))
		{
			continue;
		}

		//compare this weapon with the best we've gotten so far
		if ((!hBest || IsPreferredWeapon(hWeapon,hBest)) && !g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bIsGrenade) )
		{
			hBest = hWeapon;
		}


		// Check if we already have this weapon.
		CWeapon* pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( pWeapon )
		{
			continue;
		}

		HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA); 
		if( !hAmmo || g_pWeaponDB->IsRestricted( hAmmo ))
		{
			continue;
		}

		uint32 nSelectionAmmount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nSelectionAmount );
		m_Inventory.AcquireWeapon( hWeapon, hAmmo, nSelectionAmmount, true );
		pWeapon = m_Arsenal.GetWeapon( hWeapon );
		if( !pWeapon )
		{
			LTERROR( "CPlayerObj::AcquireMPDefaultWeapons: Could not obtain weapon." );
			continue;
		}


	}

	if (hBest)
	{
		ChangeWeapon(hBest,true, NULL, true, false );
	}

	HATTRIBUTE hGearAtt = g_pLTDatabase->GetAttribute(hLoadout,WDB_LOADOUT_rGear);
	uint32 nNumGear = g_pLTDatabase->GetNumValues(hGearAtt);

	//step through the list of weapons
	for (uint32 n = 0; n < nNumGear; ++n)
	{
		HGEAR hGear = g_pLTDatabase->GetRecordLink(hGearAtt,n,NULL);
		if( !hGear )
			continue;

		if( !g_pWeaponDB->IsRestricted( hGear ))
			m_Inventory.AcquireGear(hGear, 1);

	}


	return true;
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::GetNetUniqueName
//
//  PURPOSE:	Get the unique player name.  This could be different than
//				then name the client gave us since display names must be unique.
//
// ----------------------------------------------------------------------- //

wchar_t const* CPlayerObj::GetNetUniqueName( )
{
	if( !GetClient( ))
	{
		LTERROR( "Invalid client." );
		return L"";
	}

	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
	if( !pGameClientData )
	{
		LTERROR( "Invalid client." );
		return L"";
	}

	return pGameClientData->GetUniqueName();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::GetPatch
//
//  PURPOSE:	Get the insignia patch chosen by the player
//
// ----------------------------------------------------------------------- //

char const* CPlayerObj::GetPatch( )
{
	if( !GetClient( ))
	{
		LTERROR( "Invalid client." );
		return "";
	}

	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
	if( !pGameClientData )
	{
		LTERROR( "Invalid client." );
		return "";
	}

	return pGameClientData->GetInsignia();
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
	CCharacter::OnLinkBroken( pRef, hObj );
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

void CPlayerObj::HideCharacter(bool bHide)
{
	CCharacter::HideCharacter( bHide );
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
		StimulusRecordCreateStruct scs(kStim_LeanVisible, GetAlignment(), vObjPos, m_hObject);
		scs.m_dwDynamicPosFlags |= CAIStimulusRecord::kDynamicPos_TrackSource;
		scs.m_vOffset = vPos - vObjPos;
		m_eLeanVisibleStimID = g_pAIStimulusMgr->RegisterStimulus( scs );
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
	// don't allow respawn if we're locked into spectator mode
	if (m_bLockSpectatorMode)
	{
		return false;
	}

	// We need to be dead.
	if( GetPlayerState() == ePlayerState_Alive )
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
	// Remember what team we're on.
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
	if( pGameClientData )
	{
		pGameClientData->SetLastTeamId( nTeam );
	}

	if( !GameModeMgr::Instance( ).m_grbUseTeams) return;

	if (nTeam == GetTeamID() || nTeam > CTeamMgr::Instance().GetNumTeams() ) return;

	CTeamMgr::Instance().AddPlayer( g_pLTServer->GetClientID(GetClient( )), nTeam );

	// Set the alignment of the player to match the team so that AI will respond appropriately.
	//static EnumCharacterAlignment eTeamAlignments[2] = { (EnumCharacterAlignment)5, (EnumCharacterAlignment)3 };
    this->SetAlignment( g_pCharacterDB->GetTeamAlignment( nTeam ) );

	CTeamMgr::Instance().UpdateClient();

	g_pGameServerShell->SendPlayerInfoMsgToClients(NULL, this, MID_PI_UPDATE);

	g_pServerMissionMgr->OnPlayerInWorld( *this );
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
	m_nClientId = (uint8)g_pLTServer->GetClientID(m_hClient);
	SendIDToClients();

	m_Inventory.OnObtainClient( );

	m_iSonicData.SetClient( hClient );
	m_iSonicData.SetBook( "Player" );
	m_iSonicData.SetIntoneHandler( SonicIntoneHandlerDefault::Instance() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ClientAnimationUpdate
//
//	PURPOSE:	Handle a message from the client to update animations...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ClientAnimationUpdate( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	bool bPlayerBody = pMsg->Readbool();
	if( bPlayerBody )
	{
		m_Animator.Disable();

		ANIMTRACKERID trkID = MAIN_TRACKER;
		HMODELANIM hAnim = INVALID_MODEL_ANIM;
		HMODELWEIGHTSET hWeightSet = INVALID_MODEL_WEIGHTSET;
		HMODELWEIGHTSET hCurWeightSet = INVALID_MODEL_WEIGHTSET;

		// Read the total number of trackers...
		uint8 nNumTrackers = pMsg->Readuint8();

		// Read the info for each tracker...
		for( uint8 nTracker = 0; nTracker < nNumTrackers; ++nTracker )
		{
			// Read the tracker...
			trkID = pMsg->Readuint8();

			// The main tracker can't have set it's weightset and wasn't even written to the message...
			if( trkID != MAIN_TRACKER )
			{
				// Get the current weightset...
				g_pModelLT->GetWeightSet( m_hObject, trkID, hCurWeightSet );

				// Read the weightset and set it if different than the current...
				hWeightSet = pMsg->Readuint32();


				if( hCurWeightSet != hWeightSet )
				{
					g_pModelLT->SetWeightSet( m_hObject, trkID, hWeightSet );

					TrackerList::iterator iter = FindTracker(trkID);
					if (iter != m_Trackers.end())
					{
						(*iter).m_hWeightSet = hWeightSet;
					}
				}
			}

			// Read the enabled flag...
			bool bEnabled = pMsg->Readbool();
			g_pModelLT->SetPlaying( m_hObject, trkID, bEnabled );

			if( bEnabled )
			{
				// Set the animation recieved from the client...
				hAnim = pMsg->Readuint32();

				bool bSetTime = pMsg->Readbool();

				bool bInterpolate = !bSetTime;
				g_pModelLT->SetCurAnim( m_hObject, trkID, hAnim, bInterpolate );

				if( bSetTime )
				{
					// Set the time for the animaion so it remains in synch...
					uint32 nAnimTime = pMsg->Readuint32();
					g_pModelLT->SetCurAnimTime( m_hObject, trkID, nAnimTime );
				}

				bool bLooping = pMsg->Readbool();
				g_pModelLT->SetLooping( m_hObject, trkID, bLooping );
				
				bool bSetRate = pMsg->Readbool();
				float fRate = 1.0f;
				if (bSetRate)
				{
					fRate = pMsg->Readfloat();
				}

				g_pModelLT->SetAnimRate(m_hObject,trkID,fRate);
			}
		}

		// Read the Dims tracker and update the dims...
		trkID = pMsg->Readuint8();
		g_pModelLT->GetCurAnim( m_hObject, trkID, hAnim );

		LTVector vDims;
		g_pModelLT->GetModelAnimUserDims( m_hObject, hAnim, &vDims );
		SetDims( &vDims );
	}
	else
	{
		ANIMTRACKERID trkID = MAIN_TRACKER;
		HMODELWEIGHTSET hWeightSet = INVALID_MODEL_WEIGHTSET;
		HMODELWEIGHTSET hCurWeightSet = INVALID_MODEL_WEIGHTSET;

		// Read the total number of trackers...
		uint8 nNumTrackers = pMsg->Readuint8();

		// Read the info for each tracker...
		for( uint8 nTracker = 0; nTracker < nNumTrackers; ++nTracker )
		{
			// Read the tracker...
			trkID = pMsg->Readuint8();

			// The main tracker can't have set it's weightset and wasn't even written to the message...
			if( trkID != MAIN_TRACKER )
			{
				// Get the current weightset...
				g_pModelLT->GetWeightSet( m_hObject, trkID, hCurWeightSet );

				// Read the weightset and set it if different than the current...
				hWeightSet = pMsg->Readuint32();
				if( hCurWeightSet != hWeightSet )
				{
					g_pModelLT->SetWeightSet( m_hObject, trkID, hWeightSet );
					TrackerList::iterator iter = FindTracker(trkID);
					if (iter != m_Trackers.end())
					{
						(*iter).m_hWeightSet = hWeightSet;
					}

				}
			}

			// Read the enabled flag...
			bool bEnabled = pMsg->Readbool();
			g_pModelLT->SetPlaying( m_hObject, trkID, bEnabled );
		}

		m_Animator.Enable( m_hObject );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleAnimTrackerMsg
//
//	PURPOSE:	Handle a message from the client for animation trackers...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleAnimTrackerMsg( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	uint8 nAnimTrackerMsgId = pMsg->Readuint8();

	switch( nAnimTrackerMsgId )
	{
		case MID_ANIMTRACKERS_ADD:
		{
			ClientAnimationAddTrackers( pMsg );	
		}
		break;

		case MID_ANIMTRACKERS_REMOVE:
		{
			ClientAnimationRemoveTrackers( pMsg );
		}
		break;

		default:
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleSonicMsg
//
//	PURPOSE:	Handle a message from the client for sonics
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleSonicMsg( ILTMessage_Read* pMsg )
{
	m_iSonicData.HandleMessage( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ClientAnimationAddTrackers
//
//	PURPOSE:	Handle a message from the client to add animation trackers...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ClientAnimationAddTrackers( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	m_Trackers.clear();

	ANIMTRACKERID trkID = MAIN_TRACKER;
	HMODELWEIGHTSET hWeightSet = INVALID_MODEL_WEIGHTSET;

	// Read the total number of trackers...
	uint8 nNumTrackers = pMsg->Readuint8();

	// Read the info for each tracker...
	for( uint8 nTracker = 0; nTracker < nNumTrackers; ++nTracker )
	{
		// Read the tracker the client added...
		trkID = pMsg->Readuint8();

		// Add the new trackers, the main is always around and doesn't need to be added...
		if( trkID != MAIN_TRACKER )
		{
			// Add the tracker to the player model...
			g_pModelLT->AddTracker( m_hObject, trkID, true );

			// Set the initial weightset...
			hWeightSet = pMsg->Readuint32();
			g_pModelLT->SetWeightSet( m_hObject, trkID, hWeightSet );

			m_Trackers.push_back(PlayerTrackerInfo(trkID,hWeightSet));
		}
	}

	// Read the tracker that is responsible for footsteps...
	m_nFootstepTrackerId = pMsg->Readuint8();

	// Initialize the recoil tracker if the model supports one...
	// This needs to be done last since the recoil animations use additive blending which
	// is dependent on the order the trackers are added...
	AddRecoilTracker( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ClientAnimationRemoveTrackers
//
//	PURPOSE:	Handle message from the client to remove animation trackers...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ClientAnimationRemoveTrackers( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	ANIMTRACKERID trkID = MAIN_TRACKER;
	
	// Read the total number of trackers...
	uint8 nNumTrackers = pMsg->Readuint8();

	// Read the info for each tracker...
	for( uint8 nTracker = 0; nTracker < nNumTrackers; ++nTracker )
	{
		// Read the tracker the client removed...
		trkID = pMsg->Readuint8();

		// Remove the old trackers, the main is always around and can't be removed...
		if( trkID != MAIN_TRACKER )
		{
			g_pModelLT->RemoveTracker( m_hObject, trkID );

			TrackerList::iterator iter = FindTracker(trkID);
			if (iter != m_Trackers.end())
			{
				m_Trackers.erase(iter);
			}
			
		}
	}

	// Remove any previously added recoil tracker...
	RemoveRecoilTracker( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::RemoveWeapons
//
//	PURPOSE:	Remove all the weapons attached to this character.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::RemoveWeapons()
{
	m_Inventory.Reset();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::OnClientEnterWorld
//
//	PURPOSE:	Notification that a client has entered the world
//
// ----------------------------------------------------------------------- //

void CPlayerObj::OnClientEnterWorld(HCLIENT hClient, CPlayerObj *pPlayer)
{
	// Ignore ourselves
	if (hClient == m_hClient)
		return;

	// Tell them about a weapon looping sound if one is playing
	if (IsWeaponSoundLooping())
		UpdateWeaponSoundLooping(hClient);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetWeaponSoundLooping
//
//	PURPOSE:	Changes the weapon sound looping state for the given player
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetWeaponSoundLooping( uint32 nType, uint32 nWeapon )
{
	// Filter out duplicates
	if ((nType == m_nWeaponSoundLoopType) && (nWeapon == m_nWeaponSoundLoopWeapon))
		return;

	// Remember the state
	m_nWeaponSoundLoopType = nType;
	m_nWeaponSoundLoopWeapon = nWeapon;

	// Broadcast the state to the clients
	UpdateWeaponSoundLooping(NULL);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::UpdateWeaponSoundLooping
//
//	PURPOSE:	Updates the weapon sound looping state for the given client
//
// ----------------------------------------------------------------------- //

void CPlayerObj::UpdateWeaponSoundLooping(HCLIENT hClient)
{
	// Send them the weapon sound looping state
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits(CFX_WEAPON_SOUND_LOOP_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.Writeuint8( m_nWeaponSoundLoopType );
	cMsg.Writeuint8( m_nWeaponSoundLoopWeapon );
	g_pLTServer->SendToClient( cMsg.Read(), hClient, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ClientCameraInfoUpdate
//
//	PURPOSE:	Handle a message from the client to update the camera data...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ClientCameraInfoUpdate( ILTMessage_Read *pMsg, uint32 dwClientFlags  )
{
	if( !( dwClientFlags & CLIENTUPDATE_CAMERAINFO ))
		return;

	LTRotation rTrueCameraRot = pMsg->ReadCompLTRotation();
	LTRotation rCameraRot = rTrueCameraRot;

	EulerAngles eaCamera = Eul_FromQuat( rCameraRot, EulOrdYXZr );

	bool bSeparateCameraFromBody = pMsg->Readbool();
	if (bSeparateCameraFromBody) 
	{
		// Adjust the yaw to be this separate value.
		eaCamera.x = pMsg->Readfloat();
		rCameraRot = LTRotation(0.0f, eaCamera.x, 0.0f);
	}

	LTVector vCameraPos;
	vCameraPos = pMsg->ReadCompLTVector();

	// Don't do camera stuff until after the player has done his first update.
	if( m_bFirstUpdate )
		return;

	// Remove the pitch and roll from the player rotation.  Just store the yaw.
	// The characterhitbox will update its own rotation at a specified update rate in its update.
	LTRotation rPlayer(0.0f, eaCamera.x, 0.0f);
	g_pLTServer->RotateObject(m_hObject, rPlayer);

	// The roll is the lean amount.
	m_LeanNodeController.SetLeanAngle( eaCamera.z );

	m_NodeTrackerContext.EnableTrackerGroup( kTrackerGroup_AimAt );
	m_NodeTrackerContext.SetTrackedTarget( kTrackerGroup_AimAt, vCameraPos + ( rCameraRot.Forward() * 10000.0f ) );

	// Set the direction the player is actually looking...
	m_tfTrueCameraView.m_rRot = rTrueCameraRot;
	m_tfTrueCameraView.m_vPos = vCameraPos;
	m_tfCameraView.m_rRot = rCameraRot;
	m_tfCameraView.m_vPos = vCameraPos;

}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleSimulationTimerMsg()
//
//	PURPOSE:	Process an simulationtimer message
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleSimulationTimerMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() != 3 )
	{
		LTERROR( "Invalid simulation timer message." );
		return;
	}

	uint32 nNumerator = atoi( crParsedMsg.GetArg( 1 ));
	uint32 nDenominator = atoi( crParsedMsg.GetArg( 2 ));
	g_pGameServerShell->SetSimulationTimerScale( nNumerator, nDenominator );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandlePlayerTimerMsg()
//
//	PURPOSE:	Process an playertimer message
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandlePlayerTimerMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() < 3 )
	{
		LTERROR( "Invalid player timer message." );
		return;
	}

	uint32 nNumerator = atoi( crParsedMsg.GetArg( 1 ));
	uint32 nDenominator = atoi( crParsedMsg.GetArg( 2 ));
	SetTimerScale( nNumerator, nDenominator );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::SetTimerScale
//
//  PURPOSE:	Accessor the player timer scale.  All player timer changes should go through these
//				accessores rather than the engine directly.
// ----------------------------------------------------------------------- //
bool CPlayerObj::SetTimerScale( uint32 nNumerator, uint32 nDenominator )
{
	uint32 nAdjustedNumerator = LTMAX( nNumerator, 0 );
	uint32 nAdjustedDenominator = LTMAX( nDenominator, 0 );

	// Make sure the numerator is greater than the denominator so we can only be faster than 
	// the simulation time.
	nAdjustedNumerator = LTMAX( nAdjustedNumerator, nAdjustedDenominator );

	if( !EngineTimer( m_hObject ).SetTimerTimeScale( nAdjustedNumerator, nAdjustedDenominator ))
		return false;

	// Tell the current clients about the change.
	if( !SendTimerScale( ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SendTimerScale
//
//	PURPOSE:	Sends the timer scale to the client.
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::SendTimerScale( )
{
	uint32 nNumerator;
	uint32 nDenominator;
	if( !EngineTimer( m_hObject ).GetTimerTimeScale( nNumerator, nDenominator ))
		return false;

	// Check if the scale is set to real time, which means we don't have to 
	// send the exact data down.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits(CFX_PLAYER_TIMER, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	bool bRealTime = ( nNumerator == nDenominator );
	cMsg.Writebool( bRealTime );
	if( !bRealTime )
	{
		LTASSERT( nNumerator == ( uint16 )nNumerator, "Invalid timer scale." );
		LTASSERT( nDenominator == ( uint16 )nDenominator, "Invalid timer scale." );
		cMsg.Writeuint16(( uint16 )nNumerator );
		cMsg.Writeuint16(( uint16 )nDenominator );
	}
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	return true;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::EnterSlowMo()
//
//	PURPOSE:	Go into slowmo mode.
//
// --------------------------------------------------------------------------- //

bool CPlayerObj::EnterSlowMo( HRECORD hSlowMoRecord, HCLIENT hActivator, uint8 nSlowMoActivatorTeamId, uint32 dwFlags )
{
	// See if a valid slowmo was specified.
	if( !hSlowMoRecord )
	{
		return false;
	}

	// Don't enter slow-mo if the game mode doesn't use it...
	if( !GameModeMgr::Instance( ).m_grbUseSlowMo )
		return false;

	// Check if we're doing special multiplayer slowmo.
	if( IsMultiplayerGameServer( ))
	{
		uint8 nTeamId = GetPlayerTeamId( m_hObject );

		// In team games all teammates of the activating player are at an advantage, non-team games, only the activating player...
		if( (GameModeMgr::Instance( ).m_grbUseTeams && ( nTeamId == nSlowMoActivatorTeamId )) ||
			( hActivator == GetClient()) )
		{
			dwFlags = kTransition | kPlayerControlled | kUsePlayerTimeScale;
			if( hActivator != GetClient() )
				dwFlags |= kDontUpdateCharge;
		}
		else
		{
			// The other team is stuck at the simulation time as a disadvantage...
			dwFlags = kTransition | kPlayerControlled | kDontUpdateCharge;
		}
	}

	bool bDoTransition = !!(dwFlags & kTransition);
	bool bPlayerControlled = !!(dwFlags & kPlayerControlled);
	bool bUsePlayerTimeScale = !!(dwFlags & kUsePlayerTimeScale);
	bool bUpdateCharge = !(dwFlags & kDontUpdateCharge);
	
	if( bPlayerControlled && InStoryMode( ))
	{
		return false;
	}

	// Store the record while were in it.
	m_Inventory.SetSlowMoPlayerControl( bPlayerControlled );
	m_Inventory.SetSlowMoUpdateCharge( bUpdateCharge );

	LTVector2 v2PlayerTimeScale = GETCATRECORDATTRIB( SlowMo, g_pGameServerShell->GetSlowMoRecord(), PlayerTimeScale );

	if( !bUsePlayerTimeScale )
	{
		// Use simulation timer...
		v2PlayerTimeScale.x = v2PlayerTimeScale.y = 1.0f;
	}

	// Change the time scales.
	SetTimerScale(( uint32 )v2PlayerTimeScale.x, ( uint32 )v2PlayerTimeScale.y );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::ExitSlowMo()
//
//	PURPOSE:	Exit slowmo state.
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ExitSlowMo( bool bDoTransition )
{
	// Check if we're not in slowmo.
	if( !g_pGameServerShell->IsInSlowMo( ))
		return;

	// Change the time scales.
	SetTimerScale( 1, 1 );

	m_Inventory.ExitSlowMo( );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleSlowMoMsg()
//
//	PURPOSE:	Process an slowmo message
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleSlowMoMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() < 2 )
	{
		LTERROR( "Invalid slowmo message." );
		return;
	}

	if( crParsedMsg.GetArg( 1 ) == s_cTok_On )
	{
		HRECORD hSlowMoRecord = NULL;
		if( crParsedMsg.GetArgCount() > 2 )
			hSlowMoRecord = DATABASE_CATEGORY( SlowMo ).GetRecordByName( crParsedMsg.GetArg( 2 ));
		if( !hSlowMoRecord )
		{
			LTERROR( " - Invalid slowmo record." );
			return;
		}

		g_pGameServerShell->EnterSlowMo( hSlowMoRecord, -1.0f, GetClient( ), kTransition | kUsePlayerTimeScale );
	}
	else if( crParsedMsg.GetArg( 1 ) == s_cTok_Off )
	{
		g_pGameServerShell->ExitSlowMo(true, m_Inventory.GetSlowMoCharge( ));
	}
	else
	{
		LTERROR( "Invalid slowmo message." );
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandlePlayerSlowMoMsg()
//
//	PURPOSE:	Process an slowmo message
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandlePlayerSlowMoMsg(ILTMessage_Read *pMsg)
{
	SlowMoMsgType eType = (SlowMoMsgType)(pMsg->Readuint8());

	if (eType == kSlowMoStart)
	{
		if (g_pGameServerShell->IsInSlowMo())
		{
			return;
		}

		if( !g_pGameServerShell->EnterSlowMo( m_Inventory.GetSlowMoRecord(), m_Inventory.GetSlowMoCharge( ), 
			GetClient( ), kTransition | kPlayerControlled | kUsePlayerTimeScale ))
			return;
	}
	else if (eType == kSlowMoEnd)
	{
		if (!g_pGameServerShell->IsInSlowMo() || !m_Inventory.IsSlowMoPlayerControlled() || IsMultiplayerGameServer( ))
		{
			return;
		}
		g_pGameServerShell->ExitSlowMo(true, m_Inventory.GetSlowMoCharge( ));
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::HandleRestartLevelMsg()
//
//	PURPOSE:	Process an restartlevel message
//
// --------------------------------------------------------------------------- //

void CPlayerObj::HandleRestartLevelMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	SetPlayerState(ePlayerState_None, true);
	if (g_pServerSaveLoadMgr->ReloadSaveExists())
	{
		g_pServerSaveLoadMgr->ReloadLevel();
	}
	else
	{
		g_pServerMissionMgr->ExitLevelToLevel( g_pGameServerShell->GetCurLevel( ));
	}	

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::IsPreferredWeapon( )
//
//	PURPOSE:	Checks if WeaponA is preferred to WeaponB.
//
// ----------------------------------------------------------------------- //

bool CPlayerObj::IsPreferredWeapon( HWEAPON hWeaponA, HWEAPON hWeaponB ) const
{
	return m_Inventory.IsPreferredWeapon(hWeaponA,hWeaponB);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerObj::SetOperatingTurret( )
//
//	PURPOSE:	Set or clear the turret the player is using...
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SetOperatingTurret( const Turret &rTurret, bool bUsing )
{
	if( bUsing ) // Activating...
	{
		// Cache current weapon so it can be switched to once the player is done using the turret...
        m_hPreviousWeapon = m_Arsenal.GetCurWeaponRecord( );
		
		// Switch to the Turrets weapon...
		HWEAPON hTurretWeapon = (HWEAPON)g_pWeaponDB->GetRecordLink( rTurret.GetTurretRecord( ), WDB_TURRET_rWeapon );
		HWEAPONDATA hTurretWeaponData = g_pWeaponDB->GetWeaponData( hTurretWeapon, !USE_AI_DATA );
		bool bInfiniteAmmo = g_pWeaponDB->GetBool( hTurretWeaponData, WDB_WEAPON_bInfiniteAmmo );
		int32 nAmmo = bInfiniteAmmo ? 1000 : -1;
		
		m_Inventory.AcquireWeapon( hTurretWeapon, NULL, nAmmo, true );
		ChangeWeapon( hTurretWeapon, true, NULL, false, false );

		m_hTurret = rTurret.GetHOBJECT( );
	}
	else // Deactivating...
	{
		// Make sure we have the turret.
		if( GetTurret( ))
		{
			// Remove the turret weapon from the inventory...
			HWEAPON hTurretWeapon = (HWEAPON)g_pWeaponDB->GetRecordLink( rTurret.GetTurretRecord( ), WDB_TURRET_rWeapon );
			m_Inventory.RemoveWeapon( hTurretWeapon, false );
			
			// Switch back to the weapon used prior to using the turret...
			ChangeWeapon( m_hPreviousWeapon, true, NULL, false, false );
			m_hPreviousWeapon = NULL;

			m_hTurret = NULL;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayer::BerserkerAttack
//
//  PURPOSE:	Handle the AI grabbing the player, holding them in place and 
//				beating them.
//
// ----------------------------------------------------------------------- //
void CPlayerObj::BerserkerAttack( HOBJECT hAI )
{
	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPESyncAction);
	cClientMsg.WriteDatabaseRecord(g_pLTDatabase, g_pModelsDB->GetSyncActionRecord("BerserkAttack"));
	cClientMsg.WriteObject(hAI);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayer::BerserkerAbort
//
//  PURPOSE:	Handle the AI aborting the above attack.
//
// ----------------------------------------------------------------------- //
void CPlayerObj::BerserkerAbort( HOBJECT hAI )
{
	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPESyncAction);
	cClientMsg.WriteDatabaseRecord(g_pLTDatabase, g_pModelsDB->GetSyncActionRecord("BerserkAbort"));
	cClientMsg.WriteObject(hAI);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayer::AddRemoteCharge
//
//  PURPOSE:	We set a remote charge and need to keep track of it...
//
// ----------------------------------------------------------------------- //
void CPlayerObj::AddRemoteCharge(HOBJECT hRemote)
{
	while (m_ActiveRemoteCharges.size() >= g_pServerDB->GetRemoteChargeLimit())
	{

		HOBJECT hObj = *(m_ActiveRemoteCharges.begin());
		if( hObj )
		{
			g_pLTServer->RemoveObject(hObj);	
		}
		m_ActiveRemoteCharges.erase(m_ActiveRemoteCharges.begin());
	}

	CCharacter::AddRemoteCharge(hRemote);

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayer::SpawnGearItemsOnDeath
//
//  PURPOSE:	Drops gear items when the character dies... 
//
// ----------------------------------------------------------------------- //

void CPlayerObj::SpawnGearItemsOnDeath( )
{
	// Only spawn gear in multiplayer when dead...
	if( !IsMultiplayerGameServer( ) && (GetPlayerState( ) != ePlayerState_Alive) )
		return;
	
	LTVector vPos;
	LTRotation rRot;
	g_pLTServer->GetObjectPos(m_hObject,&vPos);
	g_pLTServer->GetObjectRotation(m_hObject,&rRot);

	HMODELSOCKET hSocket;

	if(g_pModelLT->GetSocket(m_hObject, "Back", hSocket) == LT_OK)
	{
		LTTransform transform;

		if( LT_OK == g_pModelLT->GetSocketTransform(m_hObject, hSocket, transform, true))
		{
			vPos = transform.m_vPos;
			rRot = transform.m_rRot;
		}
	}

	LTVector vSpawnPos = vPos;

	HRECORD hGlobal = g_pWeaponDB->GetGlobalRecord( );

	float fLifetime = float(g_pWeaponDB->GetInt32( hGlobal, WDB_GLOBAL_tDroppedGearLifeTime )) / 1000.0f;
	
	uint32 nNumGearItems = g_pWeaponDB->GetNumValues( hGlobal, WDB_GLOBAL_rDroppedGearItems );
	for( uint32 nGear = 0; nGear < nNumGearItems; ++nGear )
	{
		HGEAR hGear = g_pWeaponDB->GetRecordLink( hGlobal, WDB_GLOBAL_rDroppedGearItems, nGear );
		if( hGear )
		{
			char szSpawn[1024] = "";
			LTSNPrintF( szSpawn, ARRAY_LEN(szSpawn), "GearItem Gravity 0;MoveToFloor 1;GearType (%s);MPRespawn 0; DMTouchPickup 1;LifeTime %0.2f;Placed 0",
													g_pWeaponDB->GetRecordName( hGear ), fLifetime);

			BaseClass* pObj = SpawnObject( szSpawn, vSpawnPos, rRot );
			if (!pObj)
			{
				LTASSERT_PARAM1(0, "CPlayerObj::SpawnGearItemsOnDeath : Failed to Spawn: %s", szSpawn);
				return;
			}

			GearItem* pGearItem = dynamic_cast< GearItem* >( pObj );
			if( pGearItem )
			{
				LTVector vImpulse = rRot.Forward() + rRot.Up();
				vImpulse *= GetConsoleFloat("DropImpulse",1000.0f);
				if( GetDestructible()->IsDead() && GetDestructible()->GetDeathType() == DT_EXPLODE)
				{
					vImpulse += GetDestructible()->GetDeathDir() * GetDestructible()->GetDeathImpulseForce();
				}

				LTVector vAng( GetRandom(-10.0f,10.0f),GetRandom(-10.0f,10.0f),GetRandom(-20.0f,20.0f));
				pGearItem->DropItem( vImpulse, LTVector::GetIdentity(), vAng, m_hObject );		
			}

			// Randomize the position for the next spawn...
			LTVector vDiff( GetRandom( -100.0f, 100.0f ), 0, GetRandom( -100.0f, 100.0f ));
			vSpawnPos = (vPos + vDiff);
		}
	}

	// Drop slow-mo gear if we have one...
	if( m_Inventory.GetSlowMoGearRecord( ))
	{
		// Spawn the gear item..
		char szSpawn[1024] = "";
		LTSNPrintF( szSpawn, ARRAY_LEN(szSpawn), "GearItem Gravity 0;MoveToFloor 1;GearType (%s);MPRespawn 0; DMTouchPickup 1;LifeTime %0.2f;Placed 0",
					g_pWeaponDB->GetRecordName( m_Inventory.GetSlowMoGearRecord( )), fLifetime);

		BaseClass* pObj = SpawnObject( szSpawn, vSpawnPos, rRot );
		if (!pObj)
		{
			LTASSERT_PARAM1(0, "CPlayerObj::SpawnGearItemsOnDeath : Failed to Spawn: %s", szSpawn);
			return;
		}

		GearItem* pGearItem = dynamic_cast< GearItem* >( pObj );
		if( pGearItem )
		{
			// Keep connection with original gear item...
			pGearItem->SetOriginalPickupObject( m_Inventory.GetSlowMoGearObject( ));

			LTVector vImpulse = rRot.Forward() + rRot.Up();
			vImpulse *= GetConsoleFloat("DropImpulse",1000.0f);
			if( GetDestructible()->IsDead() && GetDestructible()->GetDeathType() == DT_EXPLODE)
			{
				vImpulse += GetDestructible()->GetDeathDir() * GetDestructible()->GetDeathImpulseForce();
			}

			LTVector vAng( GetRandom(-10.0f,10.0f),GetRandom(-10.0f,10.0f),GetRandom(-20.0f,20.0f));
			pGearItem->DropItem( vImpulse, LTVector::GetIdentity(), vAng, m_hObject );

		}

		m_Inventory.ClearSlowMoGearObject( );
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::StartStoryMode
//
//  PURPOSE:	enter story mode...
//
// ----------------------------------------------------------------------- //
bool CPlayerObj::StartStoryMode(HOBJECT hStoryMode, bool bCanSkip, bool bFromRestore)
{
	if (!hStoryMode)
		return false;

	if (InStoryMode() && hStoryMode != m_StoryModeObject)
	{
		LTERROR("CPlayerObj::StartStoryMode() - trying to enter storymode while already in story mode");
		DebugCPrint(0,"CPlayerObj::StartStoryMode() - trying to enter storymode while already in story mode");
		return false;
	}

	m_StoryModeObject = hStoryMode;
	m_bCanSkipStory	  = bCanSkip;

	if( !bFromRestore )
		g_pGameServerShell->ExitSlowMo( true, m_Inventory.GetSlowMoCharge( ));

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPEStoryMode);
	cClientMsg.Writebool(true);
	cClientMsg.Writebool(bCanSkip);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);

	return true;
}


bool CPlayerObj::InStoryMode()
{
	return ( (HOBJECT)m_StoryModeObject != NULL);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::EndStoryMode
//
//  PURPOSE:	exit story mode...
//
// ----------------------------------------------------------------------- //
bool CPlayerObj::EndStoryMode()
{
	if (!InStoryMode())
	{
		LTERROR("CPlayerObj::EndStoryMode() - trying to exit storymode while not in story mode");
		DebugCPrint(0,"CPlayerObj::EndStoryMode() - trying to exit storymode while not in story mode");
		return false;
	}

	m_StoryModeObject = NULL;

	CAutoMessage cClientMsg;
	cClientMsg.Writeuint8(MID_PLAYER_EVENT);
	cClientMsg.Writeuint8(kPEStoryMode);
	cClientMsg.Writebool(false);
	cClientMsg.Writebool(true);
	g_pLTServer->SendToClient(cClientMsg.Read(), GetClient( ), MESSAGE_GUARANTEED);

	return true;

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::HandleBroadcast
//
//  PURPOSE:	handle broadcasting a message to in MP
//
// ----------------------------------------------------------------------- //
void CPlayerObj::HandleBroadcastMsg(ILTMessage_Read *pMsg)
{
	if (!IsMultiplayerGameServer())
		return;

	PlayerBroadcastInfo pbi;
	HRECORD hRec = pMsg->ReadDatabaseRecord( g_pLTDatabase, DATABASE_CATEGORY( Broadcast ).GetCategory() );

	pbi.nBroadcastID = pMsg->Readuint32();
	pbi.nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, Priority);
	pbi.bPlayerInitiated = true;

	bool bLocationSent = pMsg->Readbool();
	LTVector vLoc;
	if (bLocationSent)
	{
		vLoc = pMsg->ReadCompLTVector();
	}

	if (GameModeMgr::Instance( ).m_grbUseTeams)
	{
		HRECORD hNavMarker = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, NavMarker );
		if (hNavMarker)
		{
			eNavMarkerPlacement ePlacement = DATABASE_CATEGORY( Broadcast ).GetPlacement(hRec);
			GameClientData* pGameClientData = ServerConnectionMgr::Instance( ).GetGameClientData( GetClient( ));
			if( pGameClientData )
			{

				NavMarkerCreator nmc;
				//see what kind of marker we're supposed to use
				nmc.m_hType = hNavMarker;
				nmc.m_bBroadcast = true;
				nmc.m_bInstant = true;

				//create the marker, if a type was specified
				bool bCreate = true;
				if (nmc.m_hType)
				{
					nmc.m_nTeamId = GetTeamID();
					switch(ePlacement)
					{
					case kNavMarkerAttached:
						//attach it to the player
						nmc.m_hTarget = m_hObject;
						break;
					case kNavMarkerProjected:
						{
							if( bLocationSent)
							{
								//if we hit something place the marker there
								nmc.m_vPos = vLoc;
							}
							else
							{
								bCreate = false;
							}
						} break;
					default:
						//place it where we are at
						g_pLTServer->GetObjectPos(m_hObject,&nmc.m_vPos);
					}

					if (bCreate)
					{
						NavMarker* pNM = nmc.SpawnMarker();
					}
				}


			}
		}
	}

	HandleBroadcast( pbi );

}

	
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::HandleBroadcast
//
//  PURPOSE:	handle broadcasting a message to players in MP
//
// ----------------------------------------------------------------------- //
void CPlayerObj::HandleBroadcast(const PlayerBroadcastInfo& pbi)
{
	if (!IsMultiplayerGameServer())
		return;

	CAutoMessage cBroadcastMsg;
	cBroadcastMsg.Writeuint8(MID_SFX_MESSAGE);
	cBroadcastMsg.Writeuint8(SFX_CHARACTER_ID);
	cBroadcastMsg.WriteObject(m_hObject);
	cBroadcastMsg.WriteBits(CFX_BROADCAST_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cBroadcastMsg.Writeuint16(pbi.nBroadcastID);
	cBroadcastMsg.Writeuint8( (uint8)g_pLTServer->GetClientID(GetClient( )));
	cBroadcastMsg.Writeuint8( (pbi.bSendToTeam ? GetTeamID() : INVALID_TEAM) );
	cBroadcastMsg.Writebool( pbi.bForceClient );
	cBroadcastMsg.Writeint8( pbi.nPriority );
	cBroadcastMsg.Writebool( pbi.bDamageBroadcast );
	cBroadcastMsg.Writebool( pbi.bPlayerInitiated );
	g_pLTServer->SendToClient(cBroadcastMsg.Read(), pbi.hTarget, MESSAGE_GUARANTEED);
}

TrackerList::iterator CPlayerObj::FindTracker( ANIMTRACKERID trkID )
{
	TrackerList::iterator iter = m_Trackers.begin();
	while (iter != m_Trackers.end())
	{

		if ((*iter).m_trkID == trkID)
		{
			return iter;
		}
		iter++;
	}

	return m_Trackers.end();
}

void CPlayerObj::HandleGib()
{
	CCharacter::HandleGib();
	HRECORD hRec = DATABASE_CATEGORY( Broadcast ).GetRecordByName( "Messy");
	CPlayerObj* pPlayerKiller = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_damage.GetLastDamager() ));
	if( pPlayerKiller && pPlayerKiller->GetClient() && hRec)
	{
		PlayerBroadcastInfo pbi;
		pbi.nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hRec );
		pbi.nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, Priority);
		pbi.bForceClient = true;
		pbi.hTarget = pPlayerKiller->GetClient();

		pPlayerKiller->HandleBroadcast( pbi );
	}

}
void CPlayerObj::HandleSever(ModelsDB::HSEVERPIECE hPiece)
{
	CCharacter::HandleSever(hPiece);
	HRECORD hRec = DATABASE_CATEGORY( Broadcast ).GetRecordByName( "Messy");
	CPlayerObj* pPlayerKiller = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_damage.GetLastDamager() ));
	if( pPlayerKiller && pPlayerKiller->GetClient() && hRec)
	{
		PlayerBroadcastInfo pbi;
		pbi.nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hRec );
		pbi.nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, Priority);
		pbi.bForceClient = true;
		pbi.hTarget = pPlayerKiller->GetClient();

		pPlayerKiller->HandleBroadcast( pbi );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::ActivateLadderOnLoad
//
//  PURPOSE:	After loading a saved game activate the ladder if the character has one
//
// ----------------------------------------------------------------------- //

void CPlayerObj::ActivateLadderOnLoad( )
{
	if( m_hLoadLadderObject == INVALID_HOBJECT )
		return;

	SetLadderObject( m_hLoadLadderObject );

	// Send message to the client to reactivate the ladder...
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PLAYER_LADDER );
	cMsg.WriteObject( m_hLoadLadderObject );
	g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );

	m_hLoadLadderObject = INVALID_HOBJECT;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CPlayerObj::HandleTeamSwitchRequest
//
//  PURPOSE:	Check to see if a team switch was requested
//
// ----------------------------------------------------------------------- //

void CPlayerObj::HandleTeamSwitchRequest()
{
	GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( GetClient( ));
	if (!pGameClientData)
	{
		return;
	}

	uint8 nTeam = pGameClientData->GetRequestedTeam();

	enum ETeamSwitchResponse
	{
		eKillCurrentPlayer,
		eSwitchOnNextDeath,
		eSwitchNow,
	};

	// Figure out what our response to the team change is.
	ETeamSwitchResponse eTeamSwitchResponse = eSwitchNow;
	switch( g_pServerMissionMgr->GetServerGameState())
	{
		// These states allow the player to switch immediately.
	case EServerGameState_None:
	case EServerGameState_Loading:
	case EServerGameState_GracePeriod:
		eTeamSwitchResponse = eSwitchNow;
		break;

	case EServerGameState_EndingRound:
	case EServerGameState_ShowScore:
	case EServerGameState_ExitingLevel:
		eTeamSwitchResponse = eSwitchNow;
		break;

	case EServerGameState_Playing:
	case EServerGameState_PlayingSuddenDeath:
		{
			if( IsAlive( ))
			{
				// Check if we're allowed to respawn manually.  Or, if we haven't reached
				// the minimum to play, then go ahead and switch while being alive.
				if( GameModeMgr::Instance( ).m_grbAllowRespawnFromDeath || !g_pServerMissionMgr->CheckMinimumPlayersToPlay())
				{
					eTeamSwitchResponse = eKillCurrentPlayer;
				}
				// Not allowed to respawn, don't quite switch teams yet.
				else
				{
					eTeamSwitchResponse = eSwitchOnNextDeath;
				}
			}
			else
			{
				eTeamSwitchResponse = eSwitchNow;
			}
		}
		break;
	}

	// Now do the response.
	switch( eTeamSwitchResponse )
	{
	case eKillCurrentPlayer:
		{
			// Kill the player and they will respawn on the new team.
			DamageStruct damage;
			damage.eType	= DT_UNSPECIFIED;
			damage.fDamage	= damage.kInfiniteDamage;
			damage.hDamager = NULL;
			damage.DoDamage(m_hObject, m_hObject);
		}
		break;
	case eSwitchOnNextDeath:
		{
			// tell the client they'll switch when they respawn
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_TEAM);
			g_pLTServer->SendToClient(cMsg.Read(), GetClient(), MESSAGE_GUARANTEED);
		}
		break;
	case eSwitchNow:
		{
			// Switch teams.
			SwitchToTeam(pGameClientData->GetRequestedTeam());
		}
		break;
	}
}

// EOF
