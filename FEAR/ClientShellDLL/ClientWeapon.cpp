// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeapon.cpp
//
// PURPOSE : Generic client-side weapon
//
// CREATED : 9/27/97
//
// (c) 1997-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeapon.h"
#include "VarTrack.h"
#include "PlayerStats.h"
#include "GameClientShell.h"
#include "ClientWeaponMgr.h"
#include "PlayerMgr.h"
#include "TargetMgr.h"
#include "CharacterFX.h"
#include "MsgIDs.h"
#include "WeaponFXTypes.h"
#include "SurfaceFunctions.h"
#include "ClientWeaponUtils.h"
#include "CMoveMgr.h"
#include "ClientFXMgr.h"
#include "ClientConnectionMgr.h"
#include "ShellCasingFX.h"
#include "FXDB.h"
#include "PlayerViewAttachmentMgr.h"
#include "AutoTargetMgr.h"
#include "HUDMessageQueue.h"
#include "PlayerCamera.h"
#include "PlayerBodyMgr.h"
#include "AnimationPropStrings.h"
#include "ClientDB.h"
#include "SkillDefs.h"
#include "AccuracyMgr.h"
#include "objectdetector.h"
#include "AimMgr.h"
#include "PhysicsUtilities.h"
#include "bindmgr.h"
#include "commandids.h"
#include "TurretFX.h"
#include "ForensicObjectFX.h"
#include "EntryToolLockFX.h"
#include "SpecialMoveMgr.h"
#include "HUDAmmoStack.h"
#include "ltfilewrite.h"
#include "lttimeutils.h"

#include "ILTRenderer.h"
extern ILTRenderer *g_pLTRenderer;

//
// Externs
//
extern bool g_bInfiniteAmmo;
extern StopWatchTimer TestFireTimer;
extern double fLastTestFireTime;
extern uint32 nTestFireShotsFired;
extern VarTrack g_vtEnableSimulationLog;

// PLAYER_BODY
extern VarTrack g_vtPlayerBodyWeapons;

// FOCUS DETECTOR
extern ObjectDetector g_iFocusObjectDetector;



void DebugPrintState(WeaponState eState)
{
	switch (eState)
	{
		case W_INACTIVE:
			DebugCPrint(0,"W_INACTIVE");
			break;
		case W_IDLE:
			DebugCPrint(0,"W_IDLE");
			break;
		case W_FIRING:
			DebugCPrint(0,"W_FIRING");
			break;
		case W_FIRED:
			DebugCPrint(0,"W_FIRED");
			break;
		case W_ALT_FIRING:
			DebugCPrint(0,"W_ALT_FIRING");
			break;
		case W_GREN_THROWING:
			DebugCPrint(0,"W_GREN_THROWING");
			break;
		case W_RELOADING:
			DebugCPrint(0,"W_RELOADING");
			break;
		case W_FIRING_NOAMMO:
			DebugCPrint(0,"W_FIRING_NOAMMO");
			break;
		case W_SELECT:
			DebugCPrint(0,"W_SELECT");
			break;
		case W_DESELECT:
			DebugCPrint(0,"W_DESELECT");
			break;
		case W_AUTO_SWITCH:
			DebugCPrint(0,"W_AUTO_SWITCH");
			break;
		case W_BLOCKING:
			DebugCPrint(0,"W_BLOCKING");
			break;
		case W_CHECKING_AMMO:
			DebugCPrint(0,"W_CHECKING_AMMO");
			break;
		default :
			DebugCPrint(0,"UNKNOWN WEAPON STATE!!!");
			break;
	}
}

namespace
{
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
	HMODELANIM const DEFAULT_ANI = ( static_cast< HMODELANIM >( 0 ) );
	int const INFINITE_AMMO_AMOUNT = 1000;

	const char *ns_szRighthand = "RightHand";
	const char *ns_szLeftHand = "LeftHand";

	// model animation names
	// I wish they could be const, but the engine's interface isn't
	char *ns_szSelectAnimationName = "Select";
	char *ns_szDeselectAnimationName = "Deselect";
	char *ns_szReloadAnimationName = "Reload";
	char *ns_szPlayerBaseAnimationName = "PlayerBase";

	char *ns_szPreFireAnimationName = "PreFire";
	char *ns_szPostFireAnimationName = "PostFire";

	char *ns_szIdleAnimationBasename = "Idle_";

	char *ns_szFireAnimationName = "Fire";
	char *ns_szFireAnimationBasename = "Fire";

    // If -1.0f, the anmiation rates will be unchanged,
	// else they will run at the specified speed.
	static float nsfOverrideRate = -1.0f;

	bool        ns_bInited = false;
	VarTrack    ns_vtCameraShutterSpeed;
	VarTrack	ns_vtPVModelScale;
	VarTrack	ns_vtClientWeaponStateTracking;
	VarTrack	ns_vtForensicDeselectDelay;
	VarTrack	ns_vtFastSwitchDelay;

	bool InitNamespaceVars( void )
	{
		if ( ns_bInited )
		{
			// bail if we've already inited
			return true;
		}

		LTASSERT( 0 != g_pWeaponDB, "InitNamespaceVars : WeaponDB does not exist." );

		bool   bResult;

		ns_bInited = true;

		HRECORD hGlobalRec = g_pWeaponDB->GetGlobalRecord();

		bResult = ns_vtCameraShutterSpeed.Init( g_pLTClient, "CameraShutterSpeed", 0, 0.3f );
		ASSERT( true == bResult );

		bResult = ns_vtPVModelScale.Init( g_pLTClient, "PVModelScale", 0, 10.0f );
		ASSERT( true == bResult );

		bResult = ns_vtClientWeaponStateTracking.Init( g_pLTClient, "ClientWeaponStateTracking", 0, 0.0f );
		ASSERT( true == bResult );

		bResult = ns_vtForensicDeselectDelay.Init( g_pLTClient, "ForensicDeselectDelay", 0, 1.2f );
		ASSERT( true == bResult );

		bResult = ns_vtFastSwitchDelay.Init( g_pLTClient, "FastSwitchDelay", 0, 0.1f );
		ASSERT( true == bResult );

		return true;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::CClientWeapon()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

CClientWeapon::CClientWeapon() :
	  m_RightHandWeapon( )
	, m_LeftHandWeapon( )
	, m_bHaveSilencer( false )
	, m_bHaveScope( false )
	, m_hWeapon( NULL )
	, m_hAmmo( NULL )
	, m_hComplimentaryWeapon( NULL )
	, m_fBobHeight( 0.0f )
	, m_fBobWidth( 0.0f )
	, m_bFire( false )
	, m_bFireLeftHand( false )
	, m_nNewAmmoInClip( 0 )
	, m_eState( W_INACTIVE )
	, m_nSelectAni( INVALID_ANI )
	, m_nDeselectAni( INVALID_ANI )
	, m_nReloadAni( INVALID_ANI )
	, m_nPlayerAni( INVALID_ANI )
	, m_nPreFireAni( INVALID_ANI )
	, m_nPostFireAni( INVALID_ANI )
	, m_bWeaponDeselected( false )
	, m_wIgnoreFX( 0 )
	, m_bDisabled( true )
	, m_bVisible( false )
	, m_rCamRot( 0.0f, 0.0f, 0.0f )
	, m_vCamPos( 0.0f, 0.0f, 0.0f )
	, m_hLoopSound( NULL )
	, m_nLoopSoundId( PSI_INVALID )
	, m_bFirstSelection( true )
	, m_nTracerNumber( 0 )
	, m_bAutoSwitchEnabled( true )
	, m_bAutoSwitch( false )
	, m_KeyframedClientFX()
	, m_bControllingFlashLight( false )
	, m_bPaused( false )
	, m_eWeaponAnimProp( kAP_None )
	, m_bAltFireRequested( false )
	, m_bGrenadeRequested( false )
	, m_bReloadInterrupted( false )
	, m_bSemiAuto( false )
	, m_bSemiAutoLock( false )
	, m_bSupportsFinishingMoves( false )
	, m_bQueuedFireAnimation( false )
	, m_bQueuedBlockAnimation( false )
	, m_sCriticalHitSocket( "" )
	, m_sCriticalHitImpactSocket( "" )
	, m_fCriticalHitDistanceSquared( 0.0f )
	, m_fCriticalHitAngleCos( 0.0f )
	, m_fCriticalHitViewAngleCos( 0.0f )
	, m_bWeaponTransformSet( false )
	, m_cbDeselect( NULL )
	, m_pcbData( NULL )
	, m_fDeselectToolTime( 0.0f )
	, m_eLastSpecialAim( kAP_None )
	, m_nRandomSeed( 0 )
	, m_bFireHandled( false )
	, m_bDryFireHandled( false )
	, m_vMinProximity( 0.0f, 0.0f )
	, m_bWeaponDamageThresholdExceeded( false )
{
	int i;

	// clear idle anims
	for ( i = 0; i < WM_MAX_IDLE_ANIS; ++i )
	{
		m_nIdleAnis[i] = INVALID_ANI;
	}

	// clear fire anims
	for ( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		m_nFireAnis[i] = INVALID_ANI;
	}

	m_nPerturbCount = GetRandom(0,255);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::~CClientWeapon()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CClientWeapon::~CClientWeapon()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::OnModelKey()
//
//	PURPOSE:	Handle animation commands recieved from the engine...
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::OnModelKey( HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList* pArgList )
{
	// Only accept keys for the left or right weapon model...
	if( !hObj || ((hObj != m_RightHandWeapon.m_hObject) && (hObj != m_LeftHandWeapon.m_hObject)) )
	{
		return false;
	}

	return HandleModelKey( hObj, hTrackerID, pArgList );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HandleModelKeyFromPlayerBody()
//
//	PURPOSE:	The PlayerBody needs to relay some model keys to the weapon...
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::HandleModelKeyFromPlayerBody( HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList* pArgList )
{
	// Only accept keys from the player body...
	if( !hObj || (hObj != CPlayerBodyMgr::Instance( ).GetObject( )) )
	{
		return false;
	}

	return HandleModelKey( hObj, hTrackerID, pArgList );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::QueueFireAnimation
//
//  PURPOSE:	Queue up another fire animation...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::QueueFireAnimation()
{
	CPlayerBodyMgr &PlayerBodyMgr = CPlayerBodyMgr::Instance();
	HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData( m_hWeapon, !USE_AI_DATA );
	bool bUsePostFire = g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bUsePostFire );

	if( bUsePostFire )
	{
		if( PlayerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_PostFire ) )
		{
			m_bQueuedFireAnimation = true;
		}
	}
	else
	{
		if( PlayerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_Fire ) )
		{
			m_bQueuedFireAnimation = true;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HandleModelKey()
//
//	PURPOSE:	Actually handle the model keys recieved...
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::HandleModelKey( HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList* pArgList )
{
	static CParsedMsg::CToken s_cTok_WeaponKeySound(WEAPON_KEY_SOUND);
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_FIRE( WEAPON_KEY_FIRE );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_FINISH( WEAPON_KEY_FINISH );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_FINISH_RAGDOLL( WEAPON_KEY_FINISH_RAGDOLL );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_SOUND( WEAPON_KEY_SOUND );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_BUTE_SOUND( WEAPON_KEY_BUTE_SOUND );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_LOOPSOUND( WEAPON_KEY_LOOPSOUND );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_FX( WEAPON_KEY_FX );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_FIREFX( WEAPON_KEY_FIREFX );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_HIDE_MODEL_PIECE( WEAPON_KEY_HIDE_MODEL_PIECE );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_SHOW_MODEL_PIECE( WEAPON_KEY_SHOW_MODEL_PIECE );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_FLASHLIGHT( WEAPON_KEY_FLASHLIGHT );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_MATERIAL( WEAPON_KEY_MATERIAL );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_SHELLCASING( WEAPON_KEY_SHELLCASING );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_HIDE_PVATTACHFX( WEAPON_KEY_HIDE_PVATTACHFX );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_SHOW_PVATTACHFX( WEAPON_KEY_SHOW_PVATTACHFX );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_HIDE_PVATTACHMENT( WEAPON_KEY_HIDE_PVATTACHMENT );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_SHOW_PVATTACHMENT( WEAPON_KEY_SHOW_PVATTACHMENT );
	static CParsedMsg::CToken s_cTok_Right( "RIGHT" );
	static CParsedMsg::CToken s_cTok_Left( "LEFT" );
	
	if( !hObj || m_bDisabled || !pArgList || !pArgList->argv || (pArgList->argc == 0) )
		return false;

	// make sure there is an argument
	char* pKey = pArgList->argv[0];
	if ( !pKey )
	{
		return false;
	}

	if (SkipModelKeys())
		return true;


	// Make a token to compare against.
	CParsedMsg::CToken tok( pKey );

	if( tok == s_cTok_WEAPON_KEY_FIRE )
	{
		//
		// Fire weapon
		//
		return HandleFireKey( hObj, pArgList );
	}
	else if( tok == s_cTok_WEAPON_KEY_FINISH )
	{
		//
		// Finish off the target
		//
		HOBJECT hTarget = g_pPlayerMgr->GetTargetMgr()->GetTargetObject();
		if (hTarget)
		{
			// Get impulse parameter
			float fImpulse = PhysicsUtilities::DEFAULT_IMPULSE_FORCE;
			if ( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
			{
				fImpulse = (float)atof( pArgList->argv[ 1 ] );
			}

			// Get impact position and direction.
			LTVector vImpactPos, vDir;
			HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
			if (m_RightHandWeapon.m_hObject &&					// check the weapon for an impact socket
				(LT_OK == g_pModelLT->GetSocket( m_RightHandWeapon.m_hObject, m_sCriticalHitImpactSocket.c_str(), hSocket )))
			{
				LTTransform tSocket;
				g_pModelLT->GetSocketTransform( m_RightHandWeapon.m_hObject, hSocket, tSocket, true );
				vImpactPos = tSocket.m_vPos;					// use impact socket's location for damage
				vDir = tSocket.m_rRot.Forward().GetUnit();		// and also use its orientation for momentum transfer
			}
			else
			{													// use the target's critical hit socket
				if (LT_OK == g_pModelLT->GetSocket( hTarget, m_sCriticalHitSocket.c_str(), hSocket ))
				{
					LTTransform tSocket;
					g_pModelLT->GetSocketTransform( hTarget, hSocket, tSocket, true );
					vImpactPos = tSocket.m_vPos;
				}
				else											// fallback to using the camera (should never happen)
				{
					vImpactPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos();
				}

				LTRotation rRot;								// create a good (enough) momentum direction (based on player rotation)
				g_pLTClient->GetObjectRotation( CPlayerBodyMgr::Instance( ).GetObject(), &rRot );
				vDir = rRot.Forward().GetUnit();
				vDir.z += 0.5f;
				vDir.Normalize();
			}

			SendFinishMessage(hTarget, fImpulse, vDir, vImpactPos);
		}
		return true;
	}
	else if( tok == s_cTok_WEAPON_KEY_FINISH_RAGDOLL )
	{
		//
		// Finish off the target (using ragdoll)
		//
		HOBJECT hTarget = g_pPlayerMgr->GetTargetMgr()->GetTargetObject();
		if (hTarget)
		{
			// Get grab duration
			float fDuration = 1.0f;
			if ( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
			{
				fDuration = (float)atof( pArgList->argv[ 1 ] ) / 1000.0f;	// convert from milliseconds.
			}

			// Get break force
			float fBreakForce = PhysicsUtilities::DEFAULT_BREAK_FORCE;
			if ( ( pArgList->argc > 2 ) && pArgList->argv[ 2 ] )
			{
				fBreakForce = (float)atof( pArgList->argv[ 2 ] );
			}

			// Get impact position.
			HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
			if (LT_OK == g_pModelLT->GetSocket( CPlayerBodyMgr::Instance().GetObject(), m_sCriticalHitSocket.c_str(), hSocket ))
			{
				LTTransform tSocket;
				g_pModelLT->GetSocketTransform( CPlayerBodyMgr::Instance().GetObject(), hSocket, tSocket, true );
				LTVector vImpactPos = tSocket.m_vPos;

				SendRagdollFinishMessage(hTarget, fDuration, fBreakForce, vImpactPos);
			}
			else if (LT_OK == g_pModelLT->GetSocket( hTarget, m_sCriticalHitSocket.c_str(), hSocket ))
			{
				LTTransform tSocket;
				g_pModelLT->GetSocketTransform( hTarget, hSocket, tSocket, true );
				LTVector vImpactPos = tSocket.m_vPos;

				SendRagdollFinishMessage(hTarget, fDuration, fBreakForce, vImpactPos);
			}
		}
		return true;
	}
	else if( tok == s_cTok_WEAPON_KEY_SOUND )
	{
		//
		// Play a sound globally (everybody should hear it)
		//
		HWEAPONDATA hWeapon = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);

		if ( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
		{
			HATTRIBUTE hWeaponSoundStruct;
			HATTRIBUTE   hWeaponSound = NULL;
			uint32 nValueIndex = 0;
			HRECORD hSR = NULL;

			if (g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_FirstPerson)
			{
				// play the local one if it's in first person mode...
				hWeaponSoundStruct = g_pWeaponDB->GetAttribute(hWeapon, WDB_WEAPON_LocalSoundInfo);
			}
			else
			{
				hWeaponSoundStruct = g_pWeaponDB->GetAttribute(hWeapon, WDB_WEAPON_NonLocalSoundInfo);
			}


			PlayerSoundId nId = static_cast< PlayerSoundId >( atoi( pArgList->argv[ 1 ] ) );
			switch ( nId )
			{
				case PSI_RELOAD:
				case PSI_RELOAD2:
				case PSI_RELOAD3:
				{
					hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rReloadSnd );
					nValueIndex = nId - PSI_RELOAD;
				}
				break;

				case PSI_SELECT:
				{
					hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rSelectSnd );
				}
				break;

				case PSI_DESELECT:
				{
					hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rDeselectSnd );
				}
				break;

				case PSI_WEAPON_MISC1:
				case PSI_WEAPON_MISC2:
				case PSI_WEAPON_MISC3:
				case PSI_WEAPON_MISC4:
				case PSI_WEAPON_MISC5:
				{
					hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rMiscSnd );
					nValueIndex = nId - PSI_WEAPON_MISC1;
				}
				break; 
				case PSI_FIRE:			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireSnd );			break;	// 6
				case PSI_DRY_FIRE:		hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rDryFireSnd );		break;	// 7
				case PSI_ALT_FIRE:		hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rAltFireSnd );		break;	// 8
				case PSI_SILENCED_FIRE:	hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rSilencedFireSnd );	break;	// 9
				case PSI_INVALID:
				default:
				{
				}
				break;
				case PSI_FIRE_LOOP:
					hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireLoopSnd );
					break;
				case PSI_FIRE_LOOP_END:
					hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireLoopEndSnd );
					break;
			}
			if (hWeaponSound != NULL)
			{
				hSR = g_pWeaponDB->GetRecordLink(hWeaponSound,nValueIndex);
			}

			if ( hSR )
			{
				g_pClientSoundMgr->PlayDBSoundLocal( hSR, SOUNDPRIORITY_INVALID, 0, SMGR_INVALID_VOLUME, 1.0f, WEAPONS_SOUND_CLASS, PLAYSOUND_MIX_WEAPONS_NONPLAYER );

				// Send message to Server so that other clients can hear this sound...
				uint32 dwId;
				LTRESULT ltResult;
				CAutoMessage cMsg;

				cMsg.Writeuint8( MID_WEAPON_SOUND );

				// get this client's ID
				ltResult = g_pLTClient->GetLocalClientID( &dwId );
				ASSERT( LT_OK == ltResult );

				// write the sound to play
				cMsg.Writeuint8( nId );

				// write the weapon's id
				cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hWeapon );

				// write the client's id
				cMsg.Writeuint8( static_cast< uint8 >( dwId ) );

				// write the flash position (presumably this is where the sound comes from)
				cMsg.WriteLTVector( GetFlashPos( ));

				// send the message
				ltResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
				ASSERT( LT_OK == ltResult );
			}
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_BUTE_SOUND )
	{
		//
		// Play a sound locally
		//

		// Play a sound bute...
		if( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
		{
			g_pClientSoundMgr->PlayDBSoundLocal( g_pSoundDB->GetSoundDBRecord(pArgList->argv[1]), SOUNDPRIORITY_INVALID,
				PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_REVERB,
				SMGR_INVALID_VOLUME, 1.0f, WEAPONS_SOUND_CLASS, PLAYSOUND_MIX_WEAPONS_NONPLAYER);
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_LOOPSOUND )
	{
		// Handle a looping sound key

		if( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
		{
			if( 0 == LTStrICmp( pArgList->argv[1], "STOP" ))
			{
				// Stop the looping sound from playing...
				StopLoopSound();
			
				return true;
			}

			HWEAPONDATA hWeapon = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);
			HATTRIBUTE hWeaponSoundStruct;
			HRECORD hSR = NULL;
			HATTRIBUTE   hWeaponSound = NULL;
			uint32 nValueIndex = 0;

			if (g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_FirstPerson)
			{
				// play the local one if it's in first person mode...
				hWeaponSoundStruct = g_pWeaponDB->GetAttribute(hWeapon, WDB_WEAPON_LocalSoundInfo);
			}
			else
			{
				hWeaponSoundStruct = g_pWeaponDB->GetAttribute(hWeapon, WDB_WEAPON_NonLocalSoundInfo);
			}

			PlayerSoundId nId = static_cast< PlayerSoundId >( atoi( pArgList->argv[ 1 ] ) );


			switch( nId )
			{
				case PSI_RELOAD:														// 1
				case PSI_RELOAD2:														// 2
				case PSI_RELOAD3:														// 3
				{
						hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rReloadSnd );
						nValueIndex = nId - PSI_RELOAD;
				}
				break;

				case PSI_SELECT:		hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rSelectSnd );		break;	// 4
				case PSI_DESELECT:		hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rDeselectSnd );		break;	// 5
				case PSI_FIRE:			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireSnd );			break;	// 6
				case PSI_DRY_FIRE:		hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rDryFireSnd );		break;	// 7
				case PSI_ALT_FIRE:		hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rAltFireSnd );		break;	// 8
				case PSI_SILENCED_FIRE:	hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rSilencedFireSnd );	break;	// 9
				
				case PSI_WEAPON_MISC1:													// 10
				case PSI_WEAPON_MISC2:													// 11
				case PSI_WEAPON_MISC3:													// 12
				case PSI_WEAPON_MISC4:													// 13
				case PSI_WEAPON_MISC5:													// 14
				{
						hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rMiscSnd );
						nValueIndex =  nId - PSI_WEAPON_MISC1;
				}
				break; 
				case PSI_FIRE_LOOP:
					hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireLoopSnd );
					break;
				case PSI_FIRE_LOOP_END:
					hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rFireLoopEndSnd );
					break;
				
				case PSI_INVALID:
				default:
				{
				}
				break;
			}
			if (hWeaponSound != NULL)
			{
				hSR = g_pWeaponDB->GetRecordLink(hWeaponSound);
			}
			if( hSR )
			{
				if( !m_hLoopSound || (nId != m_nLoopSoundId) )
				{
					// Stop any previous looping sound...

					KillLoopSound();

					// Play the sound immediately localy 
					
					m_hLoopSound = g_pClientSoundMgr->PlaySoundLocal( g_pSoundDB->GetRandomSoundFileWeighted(hSR), SOUNDPRIORITY_PLAYER_HIGH,
																	  PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_REVERB,
																	  SMGR_DEFAULT_VOLUME, 1.0f, WEAPONS_SOUND_CLASS );

					m_nLoopSoundId = nId;

					// Send message to server so all clients can start loop sound...
					CAutoMessage cMsg;
					cMsg.Writeuint8( MID_WEAPON_SOUND_LOOP );
					cMsg.Writeuint8( nId );
					cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hWeapon );
					g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
				}
				
			}

		}
		
	}
	else if( tok == s_cTok_WEAPON_KEY_FX )
	{
		//
		// Special FX key
		//
		return HandleFXKey( hObj, pArgList );
	}
	else if( tok == s_cTok_WEAPON_KEY_FIREFX )
	{
		//
		// Fire weapon & spawn special effects
		//

		// Only allow fire keys if it is a fire animation...
		uint32 dwAni = g_pLTClient->GetModelAnimation( hObj );
		if ( IsFireAni( dwAni ) && GetState() == W_FIRING )
		{
			m_bFire = true;
		}

		// handle the fx key
		return HandleFXKey( hObj, pArgList );
	}
	else if( tok == s_cTok_WEAPON_KEY_HIDE_MODEL_PIECE )
	{
		//
		// Hide some model pieces
		//

		//
		// loop through the rest of the arguments and spawn each FX
		//

		// start the index at the first argument
		int i = 1;

		// get the model interface
		ILTModel *pModelLT = g_pLTClient->GetModelLT();
		ASSERT( 0 != pModelLT );

		// prepare the piece
		HMODELPIECE hPiece = 0;

		// while there are arguments
		while ( ( i < pArgList->argc ) && ( '\0' != pArgList->argv[ i ][ 0 ] ) )
		{
			// reset the piece
			hPiece = 0;

			// if we find the model's piece...
			if( LT_OK == pModelLT->GetPiece( hObj, pArgList->argv[ i ], hPiece ) )
			{
				// hide it
				LTRESULT ltResult;
				ltResult = pModelLT->SetPieceHideStatus( hObj, hPiece, true );
				ASSERT( ( LT_OK == ltResult) || ( LT_NOCHANGE == ltResult ) );
			}

			// increment the index
			++i;
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_SHOW_MODEL_PIECE )
	{
		//
		// Show one of the model pieces
		//

		//
		// loop through the rest of the arguments and spawn each FX
		//

		// start the index at the first argument
		int i = 1;

		// get the model interface
		ILTModel *pModelLT = g_pLTClient->GetModelLT();
		ASSERT( 0 != pModelLT );

		// prepare the piece
		HMODELPIECE hPiece = 0;

		// while there are arguments
		while ( ( i < pArgList->argc ) && ( '\0' != pArgList->argv[ i ][ 0 ] ) )
		{
			// reset the piece
			hPiece = 0;

			// if we find the model's piece...
			if( LT_OK == pModelLT->GetPiece( hObj, pArgList->argv[ i ], hPiece ) )
			{
				// hide it
				LTRESULT ltResult;
				ltResult = pModelLT->SetPieceHideStatus( hObj, hPiece, false );
				ASSERT( ( LT_OK == ltResult) || ( LT_NOCHANGE == ltResult ) );
			}

			// increment the index
			++i;
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_FLASHLIGHT )
	{
		if (!m_bVisible) return false;

		if( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
		{
			if( 0 == LTStrICmp( pArgList->argv[1], "ON" ))
			{
				m_bControllingFlashLight = true;
				g_pPlayerMgr->GetFlashLight()->TurnOn();
			}
			else if( 0 == LTStrICmp( pArgList->argv[1], "OFF" ))
			{
				m_bControllingFlashLight = false;
				g_pPlayerMgr->GetFlashLight()->TurnOff();
			}
			else
			{
				struct Helper
				{
					static void CheckFlashlights(CWeaponModelData& model, CParsedMsg::CToken tok, ArgList* pArgList)
					{
						if (!model.m_hObject)
							return;

						if (!(( pArgList->argc > 2 ) && pArgList->argv[ 2 ]))
							return;

						CWeaponModelData::TFlashlightData::iterator iCurFlashlight = model.m_aFlashlights.begin();
						for (; iCurFlashlight != model.m_aFlashlights.end(); ++iCurFlashlight)
						{
							if (tok == iCurFlashlight->tok)
							{
								if( 0 == LTStrICmp( pArgList->argv[2], "ON" ))
								{
									iCurFlashlight->light->TurnOn();
								}
								else if( 0 == LTStrICmp( pArgList->argv[2], "OFF" ))
								{
									iCurFlashlight->light->TurnOff();
								}
								break;	// assume unique names.
							}
						}
					}
				};

				Helper::CheckFlashlights(m_RightHandWeapon, pArgList->argv[1], pArgList);
				Helper::CheckFlashlights(m_LeftHandWeapon, pArgList->argv[1], pArgList);
			}
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_MATERIAL )
	{
		// start the index at the first argument
		int i = 0;

		// while there are arguments
		while((i < pArgList->argc) && ('\0' != pArgList->argv[i][0]))
		{
			// Check for material model key
			if( LTStrIEquals( pArgList->argv[i], WEAPON_KEY_MATERIAL ))
			{
				// Check params
				if(pArgList->argc >= i+3)
				{
					SetObjectMaterial( hObj, atoi(pArgList->argv[i+1]), pArgList->argv[i+2] );
				}
				else
				{
					// Not enough params
					DebugCPrint(1,"CClientWeapon::OnModelKey - ERROR - Not enough arguments! Syntax: MATERIAL <Piece> <Material>\n");
				}

				// Move past all arguments of this key
				i += 3;
			}
			else
			{
				// Go to the next string
				i++;
			}
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_SHELLCASING )
	{
		if (!m_bVisible || !GetConsoleInt("ShellCasings", 1)) 
			return false;

		// Create a shell casing based on the model key...

		if( g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_FirstPerson )
		{
			SHELLCREATESTRUCT sc;
			sc.hWeapon		= m_hWeapon;
			sc.hAmmo		= m_hAmmo;
			sc.b3rdPerson	= false;

			LTRigidTransform rBreachTransform;
			GetBreachTransform( rBreachTransform );
			
			sc.vStartPos = rBreachTransform.m_vPos;
			sc.rRot = rBreachTransform.m_rRot;

			// Add on the player's velocity...

			HOBJECT hObj = g_pPlayerMgr->GetMoveMgr()->GetObject();
			if (hObj)
			{
				g_pPhysicsLT->GetVelocity(hObj, &sc.vStartVel);
			}

			g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_SHELLCASING_ID, &sc);
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_HIDE_PVATTACHFX )
	{
		if (!m_bVisible) return false;

		uint32 nFX = 0;

		// Retrieve the index of the FX to show...
		if( pArgList->argc > 1 && !LTStrEmpty( pArgList->argv[1] ))
		{
			// Subtract one to get the actual index...
			nFX = atoi( pArgList->argv[1] ) - 1;
		}

		if( pArgList->argc == 3 )
		{
			CParsedMsg::CToken tokHand( pArgList->argv[2] );
			if( tokHand == s_cTok_Right )
			{
				m_RightHandWeapon.HidePersistentClientFX( nFX );
			}
			else if( tokHand == s_cTok_Left )
			{
				m_LeftHandWeapon.HidePersistentClientFX( nFX );
			}
		}
		else
		{
			// Default to the right hand...
			m_RightHandWeapon.HidePersistentClientFX( nFX );
		}

	}
	else if( tok == s_cTok_WEAPON_KEY_SHOW_PVATTACHFX )
	{
		if (!m_bVisible) return false;

		uint32 nFX = 0;

		// Retrieve the index of the FX to show...
		if( pArgList->argc > 1 && !LTStrEmpty( pArgList->argv[1] ))
		{
			// Subtract one to get the actual index...
			nFX = atoi( pArgList->argv[1] ) - 1;
		}

		if( pArgList->argc == 3 )
		{
			CParsedMsg::CToken tokHand( pArgList->argv[2] );
            if( tokHand == s_cTok_Right )
			{
				m_RightHandWeapon.ShowPersistentClientFX( nFX );
			}
			else if( tokHand == s_cTok_Left )
			{
				m_LeftHandWeapon.ShowPersistentClientFX( nFX );
			}
		}
		else
		{
			// Default to the right hand...
			m_RightHandWeapon.ShowPersistentClientFX( nFX );
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_HIDE_PVATTACHMENT	)
	{
		if( !m_bVisible ) return false;

		// start the index at the first argument
		int i = 1;
		uint32 nPVAttachment = 0;

		// while there are arguments
		while((i < pArgList->argc) && ('\0' != pArgList->argv[i][0]))
		{
			nPVAttachment = atoi( pArgList->argv[ i ] );

			g_pPVAttachmentMgr->ShowPVAttachment( nPVAttachment, false );

			++i;
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_SHOW_PVATTACHMENT )
	{
		if( !m_bVisible ) return false;

		// start the index at the first argument
		int i = 1;
		uint32 nPVAttachment = 0;

		// while there are arguments
		while((i < pArgList->argc) && ('\0' != pArgList->argv[i][0]))
		{
			nPVAttachment = atoi( pArgList->argv[ i ] );

			g_pPVAttachmentMgr->ShowPVAttachment( nPVAttachment, true );

			++i;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::HandleFireKey()
//
//	PURPOSE:	Handle a fire key
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::HandleFireKey( HLOCALOBJ hObj, ArgList* pArgList )
{
	static CParsedMsg::CToken s_cTok_Right( "RIGHT" );
	static CParsedMsg::CToken s_cTok_Left( "LEFT" );

	if( !pArgList )
		return false;

	// Remember which hand to fire from if specified...
	m_bFireLeftHand = false;
	if( pArgList->argc == 2 )
	{
		char* pKey = pArgList->argv[1];
		if( !pKey )
			return false;
		
		// Make a token to compare against.
		CParsedMsg::CToken tok( pKey );

		if( tok == s_cTok_Left )
		{
			m_bFireLeftHand = true;
		}
	}

/*
	// Only allow fire keys if it is a fire animation...
	uint32 dwAni = g_pLTClient->GetModelAnimation( m_hObject );
	if ( IsFireAni( dwAni ) && GetState() == W_FIRING )
	{
//		DebugCPrint(0,"Handle Fire Key: %0.3f (%0.3f) <%0.3f>",m_FireTimer.GetElapseTime(),RealTimeTimer::Instance().GetTimerAccumulatedS(),SimulationTimer::Instance().GetTimerAccumulatedS());
		m_bFire = true;
	}
*/
	if (GetConsoleBool("TestFiring",false) || GetConsoleBool("TestSAFiring",false))
	{
		double fElapse = TestFireTimer.GetElapseTime() - fLastTestFireTime;
		fLastTestFireTime = TestFireTimer.GetElapseTime();
		nTestFireShotsFired++;
		DebugCPrint(0,"Handle Fire Key: %0.3f (%0.3f)",fLastTestFireTime,fElapse);
		if (GetState() != W_FIRING)
			DebugCPrint(0,"Ignore fire key: Not in W_FIRING fire state");
	}

// PLAYER_BODY
	if( CPlayerBodyMgr::Instance( ).IsEnabled() && GetState() == W_FIRING )
	{
		m_bFire = true;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::HandleFXKey()
//
//	PURPOSE:	Handle a fire key
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::HandleFXKey( HLOCALOBJ hObj, ArgList* pArgList )
{
	// assume success
	bool bOverallResult = true;

	//
	// loop through the rest of the arguments and spawn each FX
	//

	// start the index at the first argument
	int i = 1;

	// while there are arguments
	while ( ( i < pArgList->argc ) &&
	        ( '\0' != pArgList->argv[ i ][ 0 ] ) )
	{
		// create the effect
		CClientFXLinkNode *pNewNode = debug_new(CClientFXLinkNode);

		if(pNewNode)
		{
			CLIENTFX_CREATESTRUCT  fxCS( pArgList->argv[ i ], 0, hObj );
			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &pNewNode->m_Link, fxCS, true );

			if ( pNewNode->m_Link.IsValid() )
			{
				// effect spawned successfully, keep track of it
				m_KeyframedClientFX.AddToEnd(pNewNode);
			}
			else
			{
				debug_delete(pNewNode);

				// at least 1 FX failed to spawn
				bOverallResult = false;
			}
		}
		else
		{
			bOverallResult = false;
		}

		// increment the index
		++i;
	}

	return bOverallResult;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::OnEnterWorld()
//
//	PURPOSE:	Do the weapon setup when a level starts
//
// ----------------------------------------------------------------------- //

void CClientWeapon::OnEnterWorld()
{
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::OnExitWorld()
//
//	PURPOSE:	Do what's necessary upon exit of the world
//
// ----------------------------------------------------------------------- //

void CClientWeapon::OnExitWorld()
{
	// turn this weapon "off"
	Deactivate();

	// set some of the variables to 0
	m_NextIdleTime.Stop( );
	m_bFire = false;
	
	m_RightHandWeapon.m_nAmmoInClip = 0;
	m_LeftHandWeapon.m_nAmmoInClip = 0;
	m_nNewAmmoInClip = 0;

	m_bWeaponDeselected = false;
	m_nTracerNumber = 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::Init()
//
//	PURPOSE:	Initialize variables
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::Init( HWEAPON hWeapon )
{
	// set up all the namespace variables
	InitNamespaceVars();

	m_hWeapon = hWeapon;
	LTASSERT( m_hWeapon, "CClientWeapon::Init() ERROR - Trying to init invalid weapon record." );
	if( !m_hWeapon )
		return false;

	// Set the weapons default ammo to begin with...
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);
	m_hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
	LTASSERT( m_hAmmo, "CClientWeapon::Init() ERROR - Weapon has invalid ammo record." );
	if( !m_hAmmo )
		return false;

	// Get the weapon we want to switch to in case of losing or gaining ammo
	m_hComplimentaryWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rComplimentaryWeapon );

	// Get the name of the animation property used to specify what animations to play on the PlayerBody...
	const char* pszWeaponAnimProp = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sAnimationProperty );
	m_eWeaponAnimProp = AnimPropUtils::Enum(pszWeaponAnimProp);
	LTASSERT( m_eWeaponAnimProp < AnimPropUtils::Count(), "CClientWeapon::Init() ERROR - Weapon has invalid animation property." );
	if( m_eWeaponAnimProp == AnimPropUtils::Count() )
		return false;

	m_bSemiAuto = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bSemiAuto);

	m_bAutoSwitchEnabled = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bAutoSwitchEnabled);

	m_bSupportsFinishingMoves = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bSupportsFinishingMoves);

	m_sCriticalHitSocket = g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_sCriticalHitSocket);
	m_sCriticalHitImpactSocket = g_pWeaponDB->GetString(hWpnData,WDB_WEAPON_sCriticalHitImpactSocket);

	float fCriticalHitDistance = g_pWeaponDB->GetFloat(hWpnData,WDB_WEAPON_fCriticalHitDistance);
	m_fCriticalHitDistanceSquared = (fCriticalHitDistance * fCriticalHitDistance);

	float fCriticalHitAngle = g_pWeaponDB->GetFloat(hWpnData,WDB_WEAPON_fCriticalHitAngle);
	m_fCriticalHitAngleCos = LTCos(DEG2RAD(fCriticalHitAngle) * 0.5f);	// half-angle from center

	float fCriticalHitViewAngle = g_pWeaponDB->GetFloat(hWpnData,WDB_WEAPON_fCriticalHitViewAngle);
	m_fCriticalHitViewAngleCos = LTCos(DEG2RAD(fCriticalHitViewAngle) * 0.5f);	// half-angle from center

	m_nActivationType = g_pWeaponDB->GetInt32(hWpnData, WDB_WEAPON_nActivationType);

	// Initialize the right hand weapon model...
	HATTRIBUTE hRightWeaponModel = g_pWeaponDB->GetAttribute( hWpnData, WDB_WEAPON_RightHandWeapon );
	m_RightHandWeapon.Init( m_hWeapon, hRightWeaponModel );
	
	// Initialize the left hand weapon model...
	HATTRIBUTE hLeftWeaponModel = g_pWeaponDB->GetAttribute( hWpnData, WDB_WEAPON_LeftHandWeapon );
	m_LeftHandWeapon.Init( m_hWeapon, hLeftWeaponModel );
	
	m_vMinProximity = g_pWeaponDB->GetVector2(hWpnData, WDB_WEAPON_v2MinProximity);

	// setup animation controllers...
	ltstd::reset_vector(m_aAnimControllers);
	HATTRIBUTE hAnimControllersAtt = g_pWeaponDB->GetAttribute(hWpnData, WDB_WEAPON_rAnimControllers);
	if (hAnimControllersAtt)
	{
		uint32 nNumAnimControllersValues = g_pLTDatabase->GetNumValues(hAnimControllersAtt);
		for (uint32 nAnimControllersIndex = 0; nAnimControllersIndex < nNumAnimControllersValues; nAnimControllersIndex++)
		{
			HRECORD hAnimController = g_pWeaponDB->GetRecordLink(hAnimControllersAtt, nAnimControllersIndex);
			if (hAnimController)
			{
				// allocate and initialize in place...
				m_aAnimControllers.push_back(ConditionalAnimationController());
				new(&m_aAnimControllers.back())ConditionalAnimationController(hAnimController);
			}
		}
	}

	// successful
	return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::Term()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeapon::Term()
{
	// remove the mods
	RemoveMods();

	// remove player-view attachments
	if( g_pPVAttachmentMgr )
		g_pPVAttachmentMgr->RemovePVAttachments();

	// stop any looping sound
	KillLoopSound();

	// Clear weapon info...

	m_hWeapon	= NULL;
	m_hAmmo		= NULL;

	// remove the weapon models...
	m_RightHandWeapon.Term( );
	m_LeftHandWeapon.Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::Update()
//
//	PURPOSE:	Update the WeaponModel state
//				NOTE: the return value is a WeaponState but is not necessarily
//				this weapon's current state.
//
// ----------------------------------------------------------------------- //

WeaponState CClientWeapon::Update( )
{
	LTASSERT( m_RightHandWeapon.m_hObject || m_LeftHandWeapon.m_hObject, "No valid weapon objects" );

	// remove all keyframed FXEdit effects that have expired
	RemoveFinishedKeyframedFX();

	//see if we are paused, if so we need to pause the animation
	bool bPaused = g_pGameClientShell->IsServerPaused() || m_bPaused;

	// See if we are disabled...If so don't allow any weapon stuff...
	if ( m_bDisabled || bPaused)
	{
		return W_IDLE;
	}

	// Update the state of the model...
	WeaponState eState = UpdateModelState( );

	// NOTE: this is NOT the weapon state...this is the value
	// returned by the UpdateModelState function.
	bool bFiredWeapon = FiredWeapon(eState);

	if( bFiredWeapon )
	{
		//
		// The weapon should now fire/activate.
		//

		// this does not need the muzzle position set,
		// that will be done later
		if( m_bFireLeftHand )
		{
			m_LeftHandWeapon.StartMuzzleFlash( );
		}
		else
		{
			m_RightHandWeapon.StartMuzzleFlash( );
		}

		// Send message to server telling player to fire...
		Fire( );
	}

	LTVector vFireOffset( 0.0f, 0.0f, 0.0f );
	// Update the weapon's position
	UpdateWeaponPosition( vFireOffset );

	// Update the muzzle flash...
	m_RightHandWeapon.UpdateMuzzleFlash( !m_bHaveSilencer );
	m_LeftHandWeapon.UpdateMuzzleFlash( !m_bHaveSilencer );

	// Update the mods...
	UpdateMods();

	// Update the player-view attachments
	g_pPVAttachmentMgr->UpdatePVAttachments();

	// Handle Auto Ammo/Weapon switching...
	if( m_bAutoSwitchEnabled && m_bAutoSwitch )
	{
		// We only auto-switch if we are out of ammo...
		if( g_pPlayerStats->GetAmmoCount( m_hAmmo ) <= 0 )
		{
			// Tell the client weapon mgr to switch to a different weapon...
			eState = W_AUTO_SWITCH;
		}

		// clear the autoswitch flag
		m_bAutoSwitch = false;
	}

	// Special handling for forensic tools...
	if (IS_ACTIVATE_FORENSIC(m_nActivationType))
	{
		g_pPlayerMgr->GetForensicObjectDetector().SetTransform( g_pPlayerMgr->GetPlayerCamera()->GetCamera() );
		g_pPlayerMgr->GetForensicObjectDetector().AcquireObject( false );

		CForensicObjectFX* pForensicObject = g_pPlayerMgr->GetForensicObject();

		// Switch away from forensic tools if we leave our forensic area.
		if( !IsSwapping() )
		{
			if (pForensicObject == NULL)
			{
				if (m_fDeselectToolTime == 0.0f)	// only set once
				{
					m_fDeselectToolTime = g_pLTClient->GetTime() + ns_vtForensicDeselectDelay.GetFloat();
				}
			}
			else
			{
				m_fDeselectToolTime = 0.0f;			// reset when we go back in
			}
		}

		// Deselect after an appropriate amount of warning time.
		if ((m_fDeselectToolTime > 0.0f) && (g_pLTClient->GetTime() >= m_fDeselectToolTime))
		{
			m_fDeselectToolTime = 0.0f;
			g_pClientWeaponMgr->LastWeapon();
			g_pClientWeaponMgr->DeselectCustomWeapon();
		}

		// Check if we're close enough to bring out the collection tool.
		if (pForensicObject && !IsSwapping())
		{
			HOBJECT hForensicObject = g_pPlayerMgr->GetForensicObjectDetector().GetObject();
			CForensicObjectFX* pFX = (CForensicObjectFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_FORENSICOBJECT_ID, hForensicObject);
			HRECORD hTool = (pFX && pFX->m_bOn) ?	// Check if we are within range of evidence
				pFX->m_cs.m_rCollectionTool : 
				g_pPlayerMgr->GetClientWeaponMgr()->GetDefaultCollectionTool();

			if (hTool && hTool != GetWeaponRecord())
			{
				// Then select it.
				g_pClientWeaponMgr->ChangeWeapon(hTool);
			}
		}
	}

	// Special handling for entry tools...
	else if (IS_ACTIVATE_ENTRY(m_nActivationType))
	{
		// Don't switch to aiming or not until after transitions have finished.
		//!!ARL: There's still an issue with crossing boundaries while the select animation is playing.  Checking for (W_SELECT != GetState()) doesn't seem to fix this.
		CPlayerBodyMgr& PlayerBodyMgr = CPlayerBodyMgr::Instance();
		if (!PlayerBodyMgr.GetAnimationContext(PlayerBodyMgr.GetWeaponContext())->IsTransitioning()
			&& !SpecialMoveMgr::Instance().IsActive())
		{
			m_eLastSpecialAim = kAP_None;

			CActivationData data = g_pPlayerMgr->GetTargetMgr()->GetActivationData();
			if (data.m_nType == MID_ACTIVATE_SPECIALMOVE)
			{
				CSpecialMoveFX *pSpecialMove = g_pGameClientShell->GetSFXMgr()->GetSpecialMoveFX(data.m_hTarget);
				if (pSpecialMove && (pSpecialMove->GetSFXID() == SFX_ENTRYTOOLLOCK_ID)
					&& (GetWeaponRecord() == ((CEntryToolLockFX*)pSpecialMove)->m_cs.m_rEntryTool))
				{
					m_eLastSpecialAim = kAP_SF_SpecialAim;
				}
			}
		}

		PlayerBodyMgr.SetAnimProp( kAPG_SpecialFire, m_eLastSpecialAim, CPlayerBodyMgr::kUnlocked );
	}

	UpdateWeaponDisplay();

	return eState;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HandleAnimationStimulus
//
//	PURPOSE:	Dispatch animation stimulus to conditional animation controllers.
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::HandleAnimationStimulus( const char* pszStimulus )
{
	bool bHasController = false;

	for (AnimControllers::iterator it = m_aAnimControllers.begin();
		it != m_aAnimControllers.end(); ++it)
	{
		it->HandleStimulus(pszStimulus);
		bHasController = true;
	}

	return bHasController;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HandlingAnimationStimulus
//
//	PURPOSE:	See if any of our anim controllers are currently handling the specified stimulus.
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::HandlingAnimationStimulus( const char* pszStimulus ) const
{
	for (AnimControllers::const_iterator it = m_aAnimControllers.begin();
		it != m_aAnimControllers.end(); ++it)
	{
		if (it->HandlingStimulus(pszStimulus))
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HandlingAnimationStimulusGroup
//
//	PURPOSE:	See if any of our anim controllers are currently handling the specified stimulus (prefix).
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::HandlingAnimationStimulusGroup( const char* pszStimulus ) const
{
	for (AnimControllers::const_iterator it = m_aAnimControllers.begin();
		it != m_aAnimControllers.end(); ++it)
	{
		if (it->HandlingStimulusGroup(pszStimulus))
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::ActiveAnimationStimulus
//
//	PURPOSE:	Check to see if any action is currently playing on our
//				conditional animation controllers.
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::ActiveAnimationStimulus() const
{
	for (AnimControllers::const_iterator it = m_aAnimControllers.begin();
		it != m_aAnimControllers.end(); ++it)
	{
		if (it->ActiveStimulus())
			return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateAnimControllers()
{
	for (AnimControllers::iterator it = m_aAnimControllers.begin();
		it != m_aAnimControllers.end(); ++it)
	{
		it->Update();
	}
}

// ----------------------------------------------------------------------- //

void CClientWeapon::ResetAnimControllers()
{
	for (AnimControllers::iterator it = m_aAnimControllers.begin();
		it != m_aAnimControllers.end(); ++it)
	{
		it->Reset();
	}
}

//-----------------------------------------------------------------------------

void CClientWeapon::UpdateWeaponDisplay(bool bFiredWeapon)
{
	if (m_RightHandWeapon.m_pDisplay)
	{
		m_RightHandWeapon.m_pDisplay->UpdateDisplay(bFiredWeapon);
	}

	if (m_LeftHandWeapon.m_pDisplay)
	{
		m_LeftHandWeapon.m_pDisplay->UpdateDisplay(bFiredWeapon);
	}
}

//-----------------------------------------------------------------------------

void CClientWeapon::ShowCustomWeapon()
{
	CreateWeaponModels();
	SetDisable(false);
	SetVisible(true);
}

//-----------------------------------------------------------------------------

void CClientWeapon::HideCustomWeapon()
{
	m_RightHandWeapon.Term( );
	m_LeftHandWeapon.Term( );
	SetVisible(false);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::ChangeAmmoWithReload()
//
//	PURPOSE:	Change to the specified ammo type
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ChangeAmmoWithReload( HAMMO hNewAmmo, bool bForce /*=false*/ )
{
	// Update the player's stats...
	if ( W_RELOADING == GetState() )
	{
		if( !bForce )
			return;

		// Restart the animation in idle.
		PlayIdleAnimation( );
	}

	if ( CanChangeToAmmo( hNewAmmo ) && (hNewAmmo != m_hAmmo) )
	{
		m_hAmmo	= hNewAmmo;

		// Make sure we reset the anis (the ammo may override the 
		// weapon animations)...
		InitAnimations( true );

		// Do normal reload...
		ReloadClip( true, -1, true, true );
		
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::ChangeAmmoImmediate()
//
//	PURPOSE:	Change to the specified ammo type
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ChangeAmmoImmediate( HAMMO hNewAmmo, int nAmmoAmount /*=-1*/, bool bForce /*=false*/ )
{
	// Update the player's stats...
	if ( ( W_RELOADING == GetState() ) && !bForce )
	{
		return;
	}

	if( !CanChangeToAmmo( hNewAmmo ))
		return;

	if( hNewAmmo != m_hAmmo )
	{
		m_hAmmo	= hNewAmmo;

		// Make sure we reset the anis (the ammo may override the 
		// weapon animations)...
		InitAnimations( true );

		// Do normal reload...
		ReloadClip( false, nAmmoAmount /*-1*/, true, true );
	}
	else
	{
		// If default value, then just use the current ammo.
		uint32 nNewAmmoAmount = nAmmoAmount;
		if( nAmmoAmount == -1 )
		{
			nNewAmmoAmount = g_pPlayerStats->GetAmmoCount( m_hAmmo );
		}

		// Update the hud to reflect the new ammo amount...
		g_pPlayerStats->UpdateAmmo( m_hWeapon, m_hAmmo, nNewAmmoAmount, false, true, WDB_INVALID_WEAPON_INDEX );
		g_pPlayerStats->UpdatePlayerWeapon( m_hWeapon, m_hAmmo );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::ReloadClip
//
//	PURPOSE:	Fill the clip
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ReloadClip( bool bPlayReload /*=true*/,
								int32 nNewAmmo /*=-1*/,
                                bool bForce /*=false*/,
								bool bNotifyServer /*=false*/)
{

	// Can't reload clip while deselecting the weapon...

	if ( W_DESELECT == GetState() ) return;

	if ( !m_hWeapon )
		return;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);
	if( !m_hAmmo )
	{
		// No current ammo, use the default...
		m_hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
		if( !m_hAmmo )
			return;
	}

	// get all the ammo the player possesses
	int32 nAmmoCount = g_pPlayerStats->GetAmmoCount( m_hAmmo );

	// Get an intermediate amount of ammo.  If the nNewAmmo has
	// been specified, use that value.  Otherwise use the total
	// amount of ammo on the player.
	int32 nAmmo = nNewAmmo >= 0 ? nNewAmmo : nAmmoCount;

	// Get how many shots are in a clip.
	int32 nShotsPerClip = g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nShotsPerClip );

	// Update the player's stats...
	// note: the ammo amount we pass may be too much but
	// these functions figure out what the max really is,
	// and then we do the same thing later

	// UpdateAmmo does a lot of stuff, one of those is passing in
	// how much ammo you have.  If you specify an amount that is
	// more or less, it will consider this the new amount of
	// ammo that you have on you and adjust things accordingly.
	g_pPlayerStats->UpdateAmmo( m_hWeapon, m_hAmmo, nAmmo, false, true, WDB_INVALID_WEAPON_INDEX );


	if (g_pClientWeaponMgr->GetCurrentClientWeapon() == this)
	{
		// This will set the player stats to the specified weapon and 
		// ammo id.  In this case, use the current ones.
		g_pPlayerStats->UpdatePlayerWeapon( m_hWeapon, m_hAmmo );
	}

	int32 nTotalAmmoInClips = GetAmmoInClips( );

	// Make sure we can reload the clip...
	if ( !bForce )
	{
		// Already reloading...
		if ( m_RightHandWeapon.m_hObject && ( W_RELOADING == GetState() ) )
		{
			return;
		}

		// Clip is full...
		if ( ( nTotalAmmoInClips == nShotsPerClip ) || ( nTotalAmmoInClips == nAmmoCount ) )
		{
			return;
		}
	}

	AimMgr::Instance().EndAim();

	if ( ( nAmmo > 0 ) && ( nShotsPerClip > 0 ) )
	{
		// The amount of ammo we give the player due
		// of the reload is tracked with m_nNewAmmoInClip.
		// Set the new ammo to the lesser vaule of
		// either the max clip size of the amount of ammo
		// on the player.
		if ( nAmmo < nShotsPerClip )
		{
			m_nNewAmmoInClip = nAmmo;
		}
		else
		{
			m_nNewAmmoInClip = nShotsPerClip;
		}
		
		if( bNotifyServer )
		{
			// Let the server know we are reloading the clip...

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_WEAPON_RELOAD );
			cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hWeapon );
			cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hAmmo ); // We maybe switching ammo
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
		}
		
// PLAYER_BODY - Need to get reload anis on weapons, but should still reload even if they arent there...
		// check for a valid reload animation
		if ( bPlayReload /*&& ( INVALID_ANI != GetReloadAni() )*/ )
		{
			// setting the state will "queue" the animation to
			// start playing on the next update
			SetState( W_RELOADING );
			return;
		}
		else
		{
			// there is no reload animation, so just put
			// the right amount in the clip directly
			SplitAmmoBetweenClips( );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::DecrementAmmo
//
//	PURPOSE:	Decrement the weapon's ammo count
//
// ----------------------------------------------------------------------- //

void CClientWeapon::DecrementAmmo()
{
	// Hide the necessary pieces...
	SpecialShowPieces(false);

	int32 nAmmo;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);

	bool bInfiniteAmmo = ( g_bInfiniteAmmo || g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo ));
	if ( bInfiniteAmmo )
	{
		nAmmo = INFINITE_AMMO_AMOUNT;
	}
	else
	{
		nAmmo = g_pPlayerStats->GetAmmoCount( m_hAmmo );
	}

	bool bInfiniteClip = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteClip );
	int32 nShotsPerClip = bInfiniteClip ? INFINITE_AMMO_AMOUNT : g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nShotsPerClip );

	if ( 0 < GetAmmoInClips( ))
	{
		if ( 0 < nShotsPerClip && !bInfiniteClip )
		{
			// decrease the ammo in the clip only if the clip
			// is non-zero

			if( m_bFireLeftHand )
			{
				--m_LeftHandWeapon.m_nAmmoInClip;
			}
			else
			{
				--m_RightHandWeapon.m_nAmmoInClip;
			}
		}

		if ( !bInfiniteAmmo && !bInfiniteClip )
		{
			// we are not using infinite ammo, update the current amount
			--nAmmo;

			// Update our stats.  This will ensure that our stats are always
			// accurate (even in multiplayer)...
			g_pPlayerStats->UpdateAmmo( m_hWeapon, m_hAmmo, nAmmo, false, false, WDB_INVALID_WEAPON_INDEX );
		}
	}

	// Check to see if we need to reload...
	if ( 0 < nShotsPerClip )
	{
		if ( 0 >= GetAmmoInClips( ))
		{
			ReloadClip( true, nAmmo );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::Block()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::Block( HOBJECT hFrom )
{
	HandleAnimationStimulus("CS_Block");
	if (ActiveAnimationStimulus())
		return true;

	// Gotta have a weapon... I suppose
	if( !m_hWeapon )
	{
		return false;
	}

	// Can only start a block from a small number of other states
	WeaponState eState = GetState();

	if (eState == W_BLOCKING)
	{
		m_bQueuedBlockAnimation = true;
		return true;
	}

	if( ( eState != W_IDLE ) && ( eState != W_SELECT ) )
	{
		return false;
	}

	// I assume this causes aiming to stop, and it's used elsewhere... so whatever
	AimMgr::Instance().EndAim();

	// Setting the state will "queue" the animation to start playing on the next update
	SetState( W_BLOCKING );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::CheckAmmo()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::CheckAmmo()
{
	// Gotta have a weapon... I suppose
	if( !m_hWeapon )
	{
		return false;
	}

	// Make sure this weapon supports the required animations
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	const char* pszIcon = g_pWeaponDB->GetString( hAmmoData, WDB_AMMO_sIcon );
	if (!pszIcon || !pszIcon[0])
	{
		return false;
	}

	// Can only check ammo from a small number of other states
	WeaponState eState = GetState();
	if( ( eState != W_IDLE ) && ( eState != W_SELECT ) )
	{
		return false;
	}

	// I assume this causes aiming to stop, and it's used elsewhere... so whatever
	AimMgr::Instance().EndAim();

	// Setting the state will "queue" the animation to start playing on the next update
	SetState( W_CHECKING_AMMO );

	return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::Stun()
//
//	PURPOSE:	Support for stunning enemies with taser while holding another weapon.
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::Stun()
{
	// Can only stun from a small number of other states
	WeaponState eState = GetState();
	if( ( eState != W_IDLE ) && ( eState != W_SELECT ) )
	{
		return false;
	}

	// I assume this causes aiming to stop, and it's used elsewhere... so whatever
	AimMgr::Instance().EndAim();

	// Play an appropriate animation...
	HandleAnimationStimulus("CS_GrenadeStart");
	if(ActiveAnimationStimulus())
	{
		// Attach the weapon model
		g_pPlayerMgr->GetClientWeaponMgr()->ShowCustomWeapon(CPlayerBodyMgr::Instance().GetGrenadeWeapon(), "CS_Grenade");
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::SetCameraInfo()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetCameraInfo( LTRotation const &rCamRot, LTVector const &vCamPos )
{
	// Store current camera pos/rot...
	m_rCamRot = rCamRot;
	m_vCamPos = vCamPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::ResetWeaponFilenames()
//
//	PURPOSE:	Update the weapon model filenames...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ResetWeaponFilenames( )
{
	// Set the filenames for the right handed weapon...
	if( m_RightHandWeapon.m_hObject )
	{
		ObjectCreateStruct ocs;
		m_RightHandWeapon.PopulateCreateStruct( ocs );
		g_pCommonLT->SetObjectFilenames( m_RightHandWeapon.m_hObject, &ocs );
		if (m_RightHandWeapon.m_pDisplay)
		{
			//restore the display material
			m_RightHandWeapon.m_pDisplay->SetMaterial();
		}
	}

	// Set the filenames for the left handed weapon...
	if( m_LeftHandWeapon.m_hObject )
	{
		ObjectCreateStruct ocs;
		m_LeftHandWeapon.PopulateCreateStruct( ocs );
        g_pCommonLT->SetObjectFilenames( m_LeftHandWeapon.m_hObject, &ocs );
		if (m_LeftHandWeapon.m_pDisplay)
		{
			//restore the display material
			m_LeftHandWeapon.m_pDisplay->SetMaterial();
		}
	}

	// Create Player-View attachments..
	// We do this here since the attachments rely on the model that the character is using
	// and when the weapon activates as the game loads we don't yet know the model.
	g_pPVAttachmentMgr->CreatePVAttachments( m_RightHandWeapon.m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SetWeaponLODDistanceBias()
//
//	PURPOSE:	Set the LOD distance bias on the weapon models...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetWeaponLODDistanceBias( float fLODDistBias )
{
	if( m_RightHandWeapon.m_hObject )
	{
		g_pModelLT->SetLODDistanceBias( m_RightHandWeapon.m_hObject, fLODDistBias );
	}

	if( m_LeftHandWeapon.m_hObject )
	{
		g_pModelLT->SetLODDistanceBias( m_LeftHandWeapon.m_hObject, fLODDistBias );
	}
}

void CClientWeapon::SetWeaponDepthBiasTableIndex( ERenderLayer eRenderLayer )
{
	if( m_RightHandWeapon.m_hObject )
	{
		g_pLTRenderer->SetObjectDepthBiasTableIndex( m_RightHandWeapon.m_hObject, eRenderLayer );
	}

	if( m_LeftHandWeapon.m_hObject )
	{
		g_pLTRenderer->SetObjectDepthBiasTableIndex( m_LeftHandWeapon.m_hObject, eRenderLayer );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateBob()
//
//	PURPOSE:	Update WeaponModel bob
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateBob(float fWidth, float fHeight)
{
	m_fBobWidth  = fWidth;
	m_fBobHeight = fHeight;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SetVisible()
//
//	PURPOSE:	Hide/Show the weapon model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetVisible( bool bVis /* = true */, bool bShadow /* = true  */ )
{
	if( !m_RightHandWeapon.m_hObject && !m_LeftHandWeapon.m_hObject )
		return;

	// Set the visible/invisible data member even if we are disabled.
	// The Disabled() function will make sure the weapon is visible/invisible
	// if SetVisible() was called while the weapon was disabled...
	m_bVisible = bVis;

	if ( m_bDisabled )
	{
		return;
	}

	// Hide/Show weapon models...
	m_RightHandWeapon.SetVisibility( bVis, bShadow );
	m_LeftHandWeapon.SetVisibility( bVis, bShadow );

	// Make sure flashlight is on/off as appropriate if we're hiding/showing
	// the weapon...
	if (m_bControllingFlashLight)
	{
		if (bVis)
		{
			g_pPlayerMgr->GetFlashLight()->TurnOn();
		}
		else
		{
			g_pPlayerMgr->GetFlashLight()->TurnOff();
		}
	}

	// set the visibility of the mods
	SetModVisibility( bVis, bShadow );

	// Set the visibility of the player-view attachments...

	g_pPVAttachmentMgr->ShowPVAttachments( bVis );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::CreateSilencer
//
//	PURPOSE:	Create the silencer mod
//
// ----------------------------------------------------------------------- //

void CClientWeapon::CreateSilencer()
{
	// Start without a silencer...
	m_bHaveSilencer = false;
	
	// Make sure we have a silencer...
	HMOD hSilencer = g_pPlayerStats->GetSilencer( m_hWeapon );
	if( !hSilencer )
		return;

	// Try creating silencers for both the left and right weapons...
	if( !m_RightHandWeapon.CreateSilencer( hSilencer ) && !m_LeftHandWeapon.CreateSilencer( hSilencer ))
		return;

	// if we've reached this point, we have the mod
	m_bHaveSilencer = true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateSilencer
//
//	PURPOSE:	Update the silencer mod
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateSilencer()
{
	if( m_bHaveSilencer )
	{
		m_RightHandWeapon.UpdateSilencer( );
		m_LeftHandWeapon.UpdateSilencer( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::CreateScope
//
//	PURPOSE:	Create the scope model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::CreateScope()
{
	// Start without a silencer...
	m_bHaveScope = false;

	// Make sure we have the silencer...
	HMOD hScope = g_pPlayerStats->GetScope( m_hWeapon );
	if( !hScope )
		return;

	// Try creating silencers for both the left and right weapons...
	if( !m_RightHandWeapon.CreateScope( hScope ) && !m_LeftHandWeapon.CreateScope( hScope ))
		return;

	// if we've reached this point, we have the mod
	m_bHaveScope = true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateScope
//
//	PURPOSE:	Update the scope model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateScope()
{
	if( m_bHaveScope )
	{
		m_RightHandWeapon.UpdateScope( );
		m_LeftHandWeapon.UpdateScope( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SetDisable()
//
//	PURPOSE:	Disable/Enable the weapon
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetDisable( bool bDisable /*=true*/)
{
	bool bOldVisibility = m_bVisible;

	// Let the client shell handle the weapon being disabled...

	g_pPlayerMgr->HandleWeaponDisable( ( true == bDisable ) );

	if ( bDisable )
	{
		m_bReloadInterrupted = false;

		//save our clip info...
		g_pPlayerStats->UpdateAmmoInClip(m_hWeapon,GetAmmoInClips());

		// Force weapon invisible...
		SetVisible( false );

		// Reset our data member for when the weapon is re-enabled...
		m_bVisible = bOldVisibility;

		// Must set this AFTER call to SetVisible()
		m_bDisabled = true;

		SetPaused( true );
	}
	else
	{
		// Must set this BEFORE the call to SetVisible()
		m_bDisabled = false;

		// Set the visibility back to whatever it was...
		SetVisible( m_bVisible );

		//save our clip info...
		m_nNewAmmoInClip = g_pPlayerStats->GetAmmoInClip(m_hWeapon);
		SplitAmmoBetweenClips();

		if (HasAmmo())
		{
			if (GetAmmoInClips() == 0)
			{
				ReloadClip(false);
			}

			// Unhide any hidden pieces...
			SpecialShowPieces(true,true);

		}

		SetPaused( false );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::Select()
//
//	PURPOSE:	Select the weapon
//
// ----------------------------------------------------------------------- //

void CClientWeapon::Select(bool bImmediate)
{
	if ( W_INACTIVE == GetState() )
	{
		// if its inactive, make it active and visible
		SetDisable( false );

		// [KLS 3/22/02] Only show the weapon in first-person
		SetVisible( ((g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_FirstPerson) ? true : false) );
	}

	if (bImmediate)
	{
		SetState( W_IDLE );
		uint32 dwPlayerAni = GetPlayerAni();

		if ( m_RightHandWeapon.m_hObject && ( INVALID_ANI != dwPlayerAni ) )
		{
			PlayAnimation( dwPlayerAni );
		}
				
	}
	else
	{
		SetState( W_SELECT );

		uint32 dwSelectAni = GetSelectAni();

		if ( m_RightHandWeapon.m_hObject && ( INVALID_ANI != dwSelectAni ) )
		{
			uint32 dwAni = g_pLTClient->GetModelAnimation( m_RightHandWeapon.m_hObject );

			// the "default" animation is the select animation,
			// so this fails when we try to select it,
			// going to have to try to find a workaround
			if (!IsSelectAni( dwAni ) )
			{
				float fRate = GetSkillValue(eWeaponReloadSpeed);

				// play select animation
				PlayAnimation( dwSelectAni, fRate );

				// Since we may need the camera position and/or the flash position
				// after the selection ani starts but before the next update of this weapon, update
				// the positions here.  They will get reset during the update to their accurate position.

				m_vCamPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );

				// Update the muzzle flash...
				m_RightHandWeapon.UpdateMuzzleFlash( !m_bHaveSilencer );
				m_LeftHandWeapon.UpdateMuzzleFlash( !m_bHaveSilencer );
			}

			// Tell the server we're playing the select animation...
			LTRESULT ltResult;
			CAutoMessage cMsg;

			cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );

			// say "status change"
			cMsg.Writeuint8( CP_WEAPON_STATUS );

			// tell the server we're selecting a weapon
			cMsg.Writeuint8( WS_SELECT );

			// send the message
			ltResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
			ASSERT( LT_OK == ltResult );
		}

	}

	// Make sure there is ammo in the clip for the first selection...
	if( m_bFirstSelection )
	{
		m_bFirstSelection = false;
		ReloadClip( false );
	}
	else if ( 0 == GetAmmoInClips( ))
	{
		// If there is no ammo in our clip, reload it...
		ReloadClip( false, -1, true, true );
	}

	UpdateWeaponDisplay();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::Deselect()
//
//	PURPOSE:	Deselect the weapon (with callback
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::Deselect( ClientWeaponCallBackFn cbFn, void *pData )
{
	ASSERT( 0 != cbFn );

	// setup the callback
	m_cbDeselect = cbFn;
	m_pcbData = pData;

	SetState( W_DESELECT );
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::ResetData
//
//	PURPOSE:	Reset weapon specific data
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ResetData()
{
    m_bHaveSilencer     = false;
    m_bHaveScope        = false;

	m_fBobHeight		= 0.0f;
	m_fBobWidth			= 0.0f;
	
	m_NextIdleTime.Stop( );

    m_bFire                 = false;
	m_nTracerNumber			= 0;

	m_nSelectAni			= INVALID_ANI;
	m_nDeselectAni			= INVALID_ANI;
	m_nReloadAni			= INVALID_ANI;

	m_wIgnoreFX				= 0;
    m_bWeaponDeselected     = false;

	m_nNewAmmoInClip		= 0;

	m_bAutoSwitch				= false;

	m_bControllingFlashLight	= false;

    int i;
    for (i=0; i < WM_MAX_FIRE_ANIS; i++)
	{
		m_nFireAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_IDLE_ANIS; i++)
	{
		m_nIdleAnis[i] = INVALID_ANI;
	}

	m_FireTimer.Stop();

	m_RightHandWeapon.ResetData( );
	m_LeftHandWeapon.ResetData( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::Activate()
//
//	PURPOSE:	Activate the weapon creating any necessary resources.
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::Activate()
{
	if( !m_hWeapon /*|| !m_hAmmo*/ )
		return false;

// PLAYER_BODY prototype...
	if( g_vtPlayerBodyWeapons.GetFloat() < 1.0f )
	{
		if( m_hWeapon == g_pWeaponDB->GetUnarmedRecord( ))
		{
			// Turn the player body on when holstering...
			CPlayerBodyMgr::Instance( ).Enable();
		}
	}

	if( m_RightHandWeapon.m_hObject || m_LeftHandWeapon.m_hObject )
		return true;

	// reset any necessary data
	ResetData();

	// create the model
	if( !CreateWeaponModels() )
	{
		Term();
		return false;
	}

	// Set our timers that are based on the client object.
	EngineTimer engineTimer = g_pMoveMgr->GetServerObject( );
	if (!engineTimer.IsValid())
	{
		Term();
		return false;
	}
	m_NextIdleTime.SetEngineTimer( engineTimer );
	m_FireTimer.SetEngineTimer( engineTimer );

	m_tmrFastSwitchDelay.SetEngineTimer( engineTimer );

	m_RightHandWeapon.m_FlashTime.SetEngineTimer( engineTimer );
	m_LeftHandWeapon.m_FlashTime.SetEngineTimer( engineTimer );

	// do one update of the weapon model position to put
	// it in the right place
	UpdateWeaponPosition( LTVector::GetIdentity() );

	// initialize the animations for this model
	InitAnimations();

	// [RP 8/27/02] First play the select animation and THEN create the attachments
	// such as the ClientFX and any mods like the silencer THEN set the animation to 
	// an Idle animation.  We need to do this bit of wackiness to make sure the attachments
	// are created in the correct selected position and will render off screen when the
	// weapon becomes visible.

	uint32 dwSelectAni = GetSelectAni();

	PlayAnimation( dwSelectAni );

	m_RightHandWeapon.CreateMuzzleFlash( );
	m_LeftHandWeapon.CreateMuzzleFlash( );

	m_RightHandWeapon.CreatePersistentClientFX( );
	m_LeftHandWeapon.CreatePersistentClientFX( );

	// create any mods
	CreateMods();

	// Create Player-View attachments..
	
	g_pPVAttachmentMgr->CreatePVAttachments( m_RightHandWeapon.m_hObject );


	// Make sure the model doesn't start out on the select animation (if it does
	// it won't play the select animation when it is selected)...

	if ( INVALID_ANI != dwSelectAni )
	{
		uint32 dwAni = g_pLTClient->GetModelAnimation( m_RightHandWeapon.m_hObject );

		if ( IsSelectAni( dwAni ) )
		{
			PlayAnimation(GetSubtleIdleAni());
		}
	}

	// show the broken version
	DisplayWeaponModelPieces(m_hWeapon, WDB_WEAPON_sHidePieces, false);
	DisplayWeaponModelPieces(m_hWeapon, WDB_WEAPON_sShowPieces, true);

    return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::Deactivate()
//
//	PURPOSE:	Put the weapon into an inactive state
//
// ----------------------------------------------------------------------- //

void CClientWeapon::Deactivate()
{
	// set the state to inactave
	SetState( W_INACTIVE );
	
	// disable the weapon
	SetDisable( true );

	// turn off all keyframed ClientFX
	for ( CClientFXLinkNode* pNode = m_KeyframedClientFX.m_pNext; pNode != NULL; pNode = pNode->m_pNext )
	{
		// turn off the effect
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &pNode->m_Link );
	}

	// turn off the ammo indicator on the hud
	if( g_pHUDAmmoStack )
		g_pHUDAmmoStack->SetAmmoType(NULL);

	// destroy the list of keyframed ClientFX
	m_KeyframedClientFX.DeleteList();

	// remove the mods
	RemoveMods();

	// remove player-view attachments
	g_pPVAttachmentMgr->RemovePVAttachments();

	// remove the weapon models
	m_RightHandWeapon.Term( );
	m_LeftHandWeapon.Term( );

	KillLoopSound();

// PLAYER_BODY prototype...
	if( g_vtPlayerBodyWeapons.GetFloat() < 1.0f )
	{
		if( m_hWeapon == g_pWeaponDB->GetUnarmedRecord( ))
		{
			// Go back to normal when selecting a weapon...
			CPlayerBodyMgr::Instance( ).Disable();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HasAmmo()
//
//	PURPOSE:	Do we have any ammo for this weapon
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::HasAmmo() const
{
	ASSERT( m_hWeapon );
	if( !m_hWeapon )
		return true;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);

	if( g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo ))
	{
		// infinite ammo, we have ammo
		return true;
	}
	else
	{
		uint8 nNumAmmo = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rAmmoName );
		for( uint8 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
		{
			HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, nAmmo );
			if( 0 < g_pPlayerStats->GetAmmoCount( hAmmo ))
			{
				// we have ammo
				return true;
			}
		}
	}

	// couldn't find any ammo, we're empty
	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::CanUseAmmo()
//
//	PURPOSE:	Determine if the current weapon can use the specified ammo type
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::CanUseAmmo( HAMMO hAmmo ) const
{
	if ( !m_hWeapon || !hAmmo )
	{
		return false;
	}
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);

	uint8 nNumAmmo = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rAmmoName );
	for( uint8 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
	{
		HAMMO hCurAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, nAmmo );
		if( hCurAmmo == hAmmo )
		{
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CanChangeToAmmo()
//
//	PURPOSE:	See if we can change to the specified ammo
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::CanChangeToAmmo( HAMMO hAmmo ) const
{
	// Is the ID valid?
	
	if( !hAmmo || !m_hWeapon )
	{
		return false;
	}

	// Is the ID valid for this weapon?  Do we have ammo?
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);

	uint8 nNumAmmo = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rAmmoName );
	bool bFoundAndHasAmmo = false;
	for( uint8 nAmmo = 0; (!bFoundAndHasAmmo) && (nAmmo < nNumAmmo); ++nAmmo )
	{
		if( hAmmo == g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, nAmmo ))
		{
			if( 0 < g_pPlayerStats->GetAmmoCount( hAmmo ))
			{
				bFoundAndHasAmmo = true;
			}
		}
	}

	if ( !bFoundAndHasAmmo )
	{
		return false;
	}

	// if me made it this far, this ammo is useable
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetNextAvailableAmmo()
//
//	PURPOSE:	Determine the next available ammo type
//
// ----------------------------------------------------------------------- //

HAMMO CClientWeapon::GetNextAvailableAmmo( HAMMO hAmmo /* = NULL */ )
{
	HAMMO hCurAmmo = m_hAmmo;

	// If the given ammo is valid, use it...

	if( hAmmo )
	{
		hCurAmmo = hAmmo;
	}


	HAMMO hNewAmmo = hCurAmmo;
	int32 nOriginalAmmoIndex = 0;
	int32 nCurAmmoIndex = 0;
	int32 nAmmoCount = 0;

	// Find the current ammo in the list of ammo 
	// supported by the current weapon
	ASSERT( 0 != m_hWeapon );
	if ( !m_hWeapon )
		return 0;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);

	uint8 nNumAmmo = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rAmmoName );
	for( uint8 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
	{
		if( hCurAmmo == g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, nAmmo ))
		{
			nOriginalAmmoIndex = nAmmo;
			nCurAmmoIndex = nAmmo;
			break;
		}
	}

	while ( 1 )
	{
		nCurAmmoIndex++;

		// check for wrap
		if( nCurAmmoIndex >= nNumAmmo )
		{
			nCurAmmoIndex = 0;
		}

		// have we checked all the ammo?
		if ( nCurAmmoIndex == nOriginalAmmoIndex )
		{
			break;
		}

		// get the ammo count
		ASSERT( 0 != g_pPlayerStats );
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);

		hCurAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, nCurAmmoIndex );
		nAmmoCount = g_pPlayerStats->GetAmmoCount( hCurAmmo );
		if (0 < nAmmoCount )
		{
			hNewAmmo = hCurAmmo;
			break;
		}
	}

	return hNewAmmo;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetBestAvailableAmmoId()
//
//	PURPOSE:	Get the best available ammo id for this weapon.
//
// ----------------------------------------------------------------------- //

HAMMO CClientWeapon::GetBestAvailableAmmo( ) const
{
	if ( !g_pPlayerStats )
	{
		return false;
	}

	// intermediate variables to keep track of the best ammo id found
	HAMMO hBestAmmo = NULL;
	int32 nMaxPriority = -1;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);

	// go through all the ammo ids
	uint8 nNumAmmo = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rAmmoName );
	for( uint8 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
	{
		HAMMO hCurAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, nAmmo );
		if( 0 < g_pPlayerStats->GetAmmoCount( hCurAmmo ) )
		{
			// we do have this ammo

			// compare it to the previous priorities
			HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hCurAmmo);
			int32 nPriority = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nPriority );
			if( nPriority > nMaxPriority )
			{
				// this is the best we've found so far, keep track of it
				hBestAmmo = hCurAmmo;
				nMaxPriority = nPriority;
			}
		}
	}

	if( hBestAmmo != NULL )
	{
		// we found the best ammo id
		return hBestAmmo;
	}

	// If we get to here (which we shouldn't), just use the default ammo
	// i if this weapon uses infinite ammo...
	if( g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo ))
	{
		return g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
	}

	// nothing found, admit failure
	
	return NULL;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::GetActivationType
//
//	PURPOSE:	Handle any activation specific functionality here.
//
// ----------------------------------------------------------------------- //

uint8 CClientWeapon::GetActivationType( HOBJECT hTarget, float fTargetRange ) const
{
	// Handle default (non-activatable) case.
	if (m_nActivationType == MID_ACTIVATE_NORMAL)
		return MID_ACTIVATE_NORMAL;

	// Handle default case.
	return m_nActivationType;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::IsSwapping()
//
//	PURPOSE:	Are we currently in the middle of swapping weapons?
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::IsSwapping() const
{
	return ( g_pClientWeaponMgr->GetRequestedWeaponRecord() != NULL
		&& g_pClientWeaponMgr->GetRequestedWeaponRecord() != GetWeaponRecord() );
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::CanPerformFinishingMove()
//
//	PURPOSE:	Can this weapon perform a finishing move on the give
//				target using the player's current orientation?
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::CanPerformFinishingMove(HOBJECT hTarget) const
{
	// Can finish?
	if (!m_bSupportsFinishingMoves)
		return false;

	// Has socket?
	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	if (LT_OK != g_pModelLT->GetSocket( hTarget, m_sCriticalHitSocket.c_str(), hSocket ))
		return false;

	LTTransform tSocket;
	g_pModelLT->GetSocketTransform( hTarget, hSocket, tSocket, true );

	// Close enough?
	LTVector const& vPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos();
	if (vPos.DistSqr(tSocket.m_vPos) > m_fCriticalHitDistanceSquared)
		return false;

	// Correctly positioned relative to the target?
	LTVector vDir = (vPos - tSocket.m_vPos).GetUnit();
	if (tSocket.m_rRot.Forward().GetUnit().Dot(vDir) < m_fCriticalHitAngleCos)
		return false;

	// Looking in the right direction?
	LTRotation const& rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation();
	if (rRot.Forward().GetUnit().Dot(-vDir) < m_fCriticalHitViewAngleCos)
		return false;

	// Just right.
	return true;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::CreateMods()
//
//	PURPOSE:	Create the mods for this weapon
//
// ----------------------------------------------------------------------- //

void CClientWeapon::CreateMods()
{
	// Create the available mods...
	CreateSilencer();
	CreateScope();

	// Put the mods in their starting pos/rot...
	UpdateMods();

	// set the starting of the mods
	bool bHideMods = (m_bDisabled || !m_bVisible);
	SetModVisibility( !bHideMods, true );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::Load
//
//  PURPOSE:	Load data for this weapon
//
// ----------------------------------------------------------------------- //

void CClientWeapon::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg ) return;

	m_RightHandWeapon.Load( pMsg );
	m_LeftHandWeapon.Load( pMsg );

	m_bFirstSelection = pMsg->Readbool();
	m_hAmmo				= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::Save
//
//  PURPOSE:	Save data for this weapon
//
// ----------------------------------------------------------------------- //

void CClientWeapon::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg ) return;

	m_RightHandWeapon.Save( pMsg );
	m_LeftHandWeapon.Save( pMsg );

	pMsg->Writebool( m_bFirstSelection );
	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hAmmo );
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::CreateWeaponModels()
//
//	PURPOSE:	Create the weapon model itself.
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::CreateWeaponModels()
{
	LTASSERT( NULL != m_hWeapon, "Trying to create a weapon model without a weapon record." );
		
	if( !m_hWeapon )
		return true;

	// Create the right and left handed weapon models...
	bool bRightCreated = m_RightHandWeapon.CreateWeaponModel( ns_szRighthand );
	bool bLeftCreated = m_LeftHandWeapon.CreateWeaponModel( ns_szLeftHand );

#if !defined(PROJECT_DARK)

	// The Dark project has legitimate weapons that do not have models.
	// Allow proper initialization of these weapons.
	if( !bRightCreated && !bLeftCreated )
	{
		LTERROR( "Failed to create a weapon model in either hand." );
		return false;
	}

#endif

	// Hide special pieces if out of ammo...
	DoSpecialCreateModel();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SwitchToComplimentaryWeapon
//
//	PURPOSE:	Switch between different versions of the weapon.
//				(useful for switching from ranged to melee for example)
// ----------------------------------------------------------------------- //

void CClientWeapon::SwitchToComplimentaryWeapon()
{
	if (!m_hComplimentaryWeapon)
		return;

	// Make sure we stop firing!
	ClearFiring(false);

	// Tell the server side player to switch weapons
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_WEAPON_SWAP );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hWeapon ); // From weapon
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hComplimentaryWeapon ); // To weapon
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateAmmoFromFire
//
//	PURPOSE:	Decrement the ammo, clear the fire boolean
//
// ----------------------------------------------------------------------- //

WeaponState CClientWeapon::UpdateAmmoFromFire( bool bDecrementAmmo /*= true*/)
{
	WeaponState eRet = W_IDLE;

	// determine if we have infinite ammo, if if not, how much ammo we actually have
	int nAmmo;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);

	bool bInfiniteAmmo = ( g_bInfiniteAmmo || g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo ));
	if ( bInfiniteAmmo )
	{
		// dummy value for infinite ammo
		nAmmo = INFINITE_AMMO_AMOUNT;
	}
	else
	{
		// current amount of ammo
		nAmmo = g_pPlayerStats->GetAmmoCount( m_hAmmo );
	}

	// If this weapon uses ammo, make sure we have ammo...
	if ( nAmmo > 0 )
	{
		// by changing the state to "fired", the update function
		// will see this and fire the weapon
		eRet = W_FIRED;

		if ( bDecrementAmmo )
		{
			DecrementAmmo();
		}
	}
	else  // NO AMMO
	{

/* [KLS 5/8/02] Removed support for dry-fire....

		// Play dry-fire sound...
		if ( m_pWeapon->szDryFireSound[ 0 ] )
		{
			g_pClientSoundMgr->PlaySoundLocal( m_pWeapon->szDryFireSound, SOUNDPRIORITY_PLAYER_HIGH,
												0, SMGR_DEFAULT_VOLUME, 1.0f, WEAPONS_SOUND_CLASS );
		}

		// Send message to Server so that other clients can hear this sound...
		uint32 dwId;
		LTRESULT ltResult;
		CAutoMessage cMsg;

		cMsg.Writeuint8( MID_WEAPON_SOUND );

		// get the local client id
		ltResult = g_pLTClient->GetLocalClientID( &dwId );
		ASSERT( LT_OK == ltResult );

		// the dry fire sound
		cMsg.Writeuint8( PSI_DRY_FIRE );

		// the weapon id
		cMsg.Writeuint8( m_nWeaponId );

		// client id
		cMsg.Writeuint8( static_cast< uint8 >( dwId ) );

		// flash position (presumably where the sound comes from)
		cMsg.WriteLTVector( m_vFlashPos );

		// send the message
		ltResult = g_pLTClient->SendToServer( cMsg.Read(), 0 );
		ASSERT( LT_OK == ltResult );
*/
	}

	return eRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateModelState
//
//	PURPOSE:	Update the model's state (fire if bFire == true)
//				NOTE: the return value is a WeaponState but is not necessarily
//				this weapon's current state.
//
// ----------------------------------------------------------------------- //

WeaponState CClientWeapon::UpdateModelState( )
{
	WeaponState eRet = W_IDLE;

	bool bFire = g_pPlayerMgr->CanFireWeapon( ) && !m_bSemiAutoLock && !m_bReloadInterrupted;
	bool bAltFire = g_pPlayerMgr->CanAltFireWeapon( ) | m_bAltFireRequested;
	bool bGrenade = g_pPlayerMgr->CanThrowGrenade( ) | m_bGrenadeRequested;

	// hack to track the press fire event, not whether the fire button is still being held down.
	if (!bFire)
		m_bFireHandled = false;

	if (bFire)
	{
		if (!m_bFireHandled)
		{
			HandleAnimationStimulus("CS_Attack");
			m_bFireHandled = ActiveAnimationStimulus();
		}
		bFire = !m_bFireHandled;
	}

	// Don't let weapon be fired if out of ammo, instead play a dry-fire sound
	// and switch to a complimentary weapon (i.e. melee - if set)
	if (bFire)
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);
		bool bInfiniteAmmo = ( g_bInfiniteAmmo || g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo ));
		bool bHasAmmo = (g_pPlayerStats->GetAmmoCount( m_hAmmo ) > 0);
		bool bInfiniteClip = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteClip );
		if( bHasAmmo && !bInfiniteClip && (GetAmmoInClips( ) == 0) )
		{
			bFire = false;

			// we don't want the dry fire to play multiple times if they hold
			// the button down while reloading, so i'm debouncing it -- Terry
			if (!m_bDryFireHandled)
			{
				::PlayWeaponSound( m_hWeapon, !USE_AI_DATA, LTVector::GetIdentity( ), PSI_DRY_FIRE, true );
				m_bDryFireHandled= true;
			}

			// Just need to reload the clips...
			ReloadClip( );
		}
		else if ( !bInfiniteAmmo && !bHasAmmo )
		{
			bFire = false;

			if( GetState( ) != W_DESELECT )
			{
				StopLoopSound( );
				::PlayWeaponSound( m_hWeapon, !USE_AI_DATA, LTVector::GetIdentity( ), PSI_DRY_FIRE, true );
				
				SwitchToComplimentaryWeapon();

				// If allowed to auto switch when empty, do so...
				m_bAutoSwitch = m_bAutoSwitchEnabled;
			}
		}
		else if( g_pPlayerMgr->IsUnderwater( ) && !g_pPlayerMgr->IsSwimmingAllowed( ))
		{
			bFire = false;
			if( GetState( ) != W_DESELECT )
			{
				StopLoopSound( );
				::PlayWeaponSound( m_hWeapon, !USE_AI_DATA, LTVector::GetIdentity( ), PSI_DRY_FIRE, true );
			}
		}
	}

	// Determine what we should be doing...
	// (mostly updates animations, also
	// updates a couple supporting variables)

	if( bGrenade )
	{
		UpdateGrenadeThrow( );
	}
	else if( bAltFire )
	{
		UpdateAltFiring( );
	}
	else if( bFire )
	{
//g_pLTClient->CPrint("BEFORE UpdateFiring");
//DebugPrintState(m_eState);

		// actually able to fire again, so wipe the DryFire flag.. -- Terry
		m_bDryFireHandled = false;
		UpdateFiring( );

//g_pLTClient->CPrint("AFTER UpdateFiring");
//DebugPrintState(m_eState);
	}
	else
	{
		UpdateNonFiring( );
	}

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	if ( m_bFire )
	{
		// doesn't actually fire, just updates the ammo 
		eRet = UpdateAmmoFromFire( true );

		if( m_bFireLeftHand )
		{
			++m_LeftHandWeapon.m_nNumFiresInARow;
			m_RightHandWeapon.m_nNumFiresInARow = 0;
		}
		else
		{
			++m_RightHandWeapon.m_nNumFiresInARow;
			m_LeftHandWeapon.m_nNumFiresInARow = 0;
		}

		// clear the fire flag
		m_bFire = false;
	}


	// See if we just finished deselecting the weapon...
	if ( m_bWeaponDeselected )
	{
		m_bWeaponDeselected = false;

		if ( m_cbDeselect )
		{
			// call the deselect callback
			m_cbDeselect( m_hWeapon, m_pcbData );

			// clear out the callback data
			m_cbDeselect = 0;
			m_pcbData = 0;
		}

		// deactivate the current weapon, its not used anymore
		Deactivate();
	}

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::UpdateWeaponPosition()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateWeaponPosition( LTVector const &vOffset )
{
// PLAYER_BODY
	if( CPlayerBodyMgr::Instance( ).IsEnabled() )
	{
		if( !m_bWeaponTransformSet )
		{
			m_RightHandWeapon.UpdateWeaponPosition( );
			m_LeftHandWeapon.UpdateWeaponPosition( );
		}

		// Don't ignore the hand socket next update... (unless transform is manually set again)
		m_bWeaponTransformSet = false;
	}
	else
	{
		LTVector vNewPos;
		LTVector vWeaponOffset = GetWeaponOffset();

		vNewPos.x = vWeaponOffset.x + m_fBobWidth;
		vNewPos.y = vWeaponOffset.y + m_fBobHeight;
		vNewPos.z = vWeaponOffset.z;

		// use the extra offset for the weapon placement
		vNewPos += vOffset.x;
		vNewPos += vOffset.y;
		vNewPos += vOffset.z;

		// KLS Updated so weapon model is in world space, not camera space

		// Translate the position based on the camera's orientation...
		
		vNewPos = m_rCamRot * vNewPos;
		
		// Now add in the camera's position...

		vNewPos += m_vCamPos;

		// set the weapon model position
		g_pLTClient->SetObjectTransform( m_RightHandWeapon.m_hObject, LTRigidTransform(vNewPos, m_rCamRot) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SetWeaponTransform
//
//	PURPOSE:	Manually set the weapons position and rotation...
//
//	NOTES:		This needs to be done everyframe or UpdateWeaponPosition() will override the value...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetWeaponTransform( const LTTransform &rWeaponTrans )
{
	if( m_RightHandWeapon.m_hObject )
		g_pLTClient->SetObjectTransform( m_RightHandWeapon.m_hObject, rWeaponTrans );
	
	if( m_LeftHandWeapon.m_hObject )
		g_pLTClient->SetObjectTransform( m_LeftHandWeapon.m_hObject, rWeaponTrans );
	
	// Ignore the hand socket transform...
	m_bWeaponTransformSet = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateFiring
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateFiring()
{
	switch( GetState())
	{
		case W_RELOADING:
		{
			if ( !PlayReloadAnimation(false) )
			{
				SetState( W_FIRING );
			}
		}
		break;

		case W_BLOCKING:
		{
			if( !PlayBlockAnimation() )
			{
				SetState( W_FIRING );
			}
		}
		break;

		case W_CHECKING_AMMO:
		{
			if( !PlayCheckAmmoAnimation() )
			{
				SetState( W_FIRING );
			}
		}
		break;

		case W_IDLE:
		{
			SetState( W_FIRING );
		}
		break;
	
		case W_SELECT:
		{
			if ( !PlaySelectAnimation() )
			{
				SetState( W_FIRING );
			}
		}
		break;

		case W_DESELECT:
		{
			if ( !PlayDeselectAnimation() )
			{
				SetState( W_INACTIVE );
			}
		}
		break;

		case W_ALT_FIRING:
		{
			if( !PlayAltFireAnimation( false ))
			{
				SetState( W_FIRING );
			}
		}
		break;

		case W_GREN_THROWING:
		{
			if( !PlayGrenadeAnimation( false ))
			{
				SetState( W_FIRING );
			}
		}
		break;
	}

	// KLS - 5/4/04 - Moved this outside the above switch statement so we can 
	// start the fire animation on the same update if we changed the firing state above...
	// This makes sure we never miss firing the weapon due to low framerates...
		switch( GetState() )
		{
			case W_FIRING:
			case W_FIRING_NOAMMO:
			{
				PlayFireAnimation( true );
			}
			break;
		}
	}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateAltFiring
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateAltFiring( )
{
	switch( GetState())
	{
		case W_RELOADING:
		{
			if ( !PlayReloadAnimation(true) )
			{
				SetState( W_ALT_FIRING );
			}
		}
		break;

		case W_BLOCKING:
		{
			if( !PlayBlockAnimation() )
			{
				SetState( W_ALT_FIRING );
			}
		}
		break;

		case W_CHECKING_AMMO:
		{
			if( !PlayCheckAmmoAnimation() )
			{
				SetState( W_ALT_FIRING );
			}
		}
		break;

		case W_IDLE:
		{
			SetState( W_ALT_FIRING );
		}
		break;

		case W_SELECT:
		{
			// Shouldn't fulfill an Alt-Fire request after a reload... 
			m_bAltFireRequested = false;

			if ( !PlaySelectAnimation() )
			{
				SetState( W_ALT_FIRING );
			}
		}
		break;

		case W_DESELECT:
		{
			// Shouldn't fulfill an Alt-Fire request after a reload... 
			m_bAltFireRequested = false;

			if ( !PlayDeselectAnimation() )
			{
				SetState( W_INACTIVE );
			}
		}
		break;

        case W_FIRING:
		case W_FIRING_NOAMMO:
		{
			// Remember Alt-Fire was requested while firing...
            m_bAltFireRequested = true;
			
			if( !PlayFireAnimation( false ) )
			{
				SetState( W_ALT_FIRING );
			}
		}
		break;

		case W_GREN_THROWING:
		{
			if( !PlayGrenadeAnimation( false ) )
			{
				SetState( W_ALT_FIRING );
			}
		}
		break;
	}

	// KLS - 5/4/04 - Moved this outside the above switch statement so we can 
	// start the fire animation on the same update if we changed the firing state above...
	// This makes sure we never miss firing the weapon due to low framerates...
	if( GetState() == W_ALT_FIRING )
	{
		// Request fulfilled...
		m_bAltFireRequested = false;
		PlayAltFireAnimation( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateGrenadeThrow
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateGrenadeThrow( )
{
	// Be sure we are not aiming.
	if( AimMgr::Instance().IsAiming() )
	{
		AimMgr::Instance().EndAim();
	}

	switch( GetState())
	{
		case W_RELOADING:
		{

			if ( !PlayReloadAnimation(true) )
			{
				SetState( W_GREN_THROWING );
			}
		}
		break;

		case W_BLOCKING:
		{
			if( !PlayBlockAnimation() )
			{
				SetState( W_GREN_THROWING );
			}
		}
		break;

		case W_CHECKING_AMMO:
		{
			if( !PlayCheckAmmoAnimation() )
			{
				SetState( W_GREN_THROWING );
			}
		}
		break;

		case W_IDLE:
		{
			SetState( W_GREN_THROWING );
		}
		break;

		case W_SELECT:
		{
			// Shouldn't fulfill a grenade request after weapon selection 
			m_bGrenadeRequested = false;

			if ( !PlaySelectAnimation() )
			{
				SetState( W_GREN_THROWING );
			}
		}
		break;

		case W_DESELECT:
		{
			// Shouldn't fulfill a grenade request after weapon de-selection 
			m_bGrenadeRequested = false;

			if ( !PlayDeselectAnimation() )
			{
				SetState( W_INACTIVE );
			}
		}
		break;

		case W_FIRING:
		case W_FIRING_NOAMMO:
		{
			// Remember grenade was requested while firing...
			m_bGrenadeRequested = true;

			if( !PlayFireAnimation( false ) )
			{
				SetState( W_GREN_THROWING );
			}
		}
		break;

		case W_ALT_FIRING:
		{
			// Remember grenade was requested while firing...
			m_bGrenadeRequested = true;

			if( !PlayAltFireAnimation( false ) )
			{
				SetState( W_GREN_THROWING );
			}
		}
		break;
	}

	// KLS - 5/4/04 - Moved this outside the above switch statement so we can 
	// start the fire animation on the same update if we changed the firing state above...
	// This makes sure we never miss firing the weapon due to low framerates...
	if( GetState() == W_GREN_THROWING )
	{
		// Request fulfilled...
		m_bGrenadeRequested = false;
		PlayGrenadeAnimation( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateNonFiring
//
//	PURPOSE:	Update the non-firing animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateNonFiring()
{
	switch( GetState())
	{
		case W_FIRING:
		{
			if ( !PlayFireAnimation( false ) )
			{
				SetState( W_IDLE );
			}
		}
		break;

		case W_ALT_FIRING:
		{
			if( !PlayAltFireAnimation( false ) )
			{
				//the alt-fire may have interrupted a reload, so reset the flag here
				m_bReloadInterrupted = false;
				SetState( W_IDLE );
			}
		}
		break;

		case W_GREN_THROWING:
		{
			if( !PlayGrenadeAnimation( false ) )
			{
				//the grenade may have interrupted a reload, so reset the flag here
				m_bReloadInterrupted = false;
				SetState( W_IDLE );
			}
		}
		break;

		case W_FIRING_NOAMMO:
		{
			SetState( W_IDLE );
		}
		break;

		case W_RELOADING:
		{
			if ( !PlayReloadAnimation(false) )
			{
				SetState( W_IDLE );
			}
		}
		break;

		case W_BLOCKING:
		{
			if( !PlayBlockAnimation() )
			{
				SetState( W_IDLE );
			}
		}
		break;

		case W_CHECKING_AMMO:
		{
			if( !PlayCheckAmmoAnimation() )
			{
				SetState( W_IDLE );
			}
		}
		break;

		case W_SELECT:
		{
			if ( !PlaySelectAnimation() )
			{
				SetState( W_IDLE );
			}
		}
		break;

		case W_DESELECT:
		{
			if ( !PlayDeselectAnimation() )
			{
                                m_bWeaponDeselected = true;
				SetState( W_INACTIVE );
			}
		}
		break;
	}

	// KLS - 5/4/04 - Moved this outside the above switch statement so we can 
	// start the idle animation on the same update if we changed the idle state above...
	// This makes sure we never miss changing to the correct state due to low framerates...
	if( GetState() == W_IDLE )
	{
		PlayIdleAnimation();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::InitAnimations
//
//	PURPOSE:	Set the animations
//
// ----------------------------------------------------------------------- //

void CClientWeapon::InitAnimations( bool bAllowSelectOverride )
{
	m_nSelectAni        = g_pLTClient->GetAnimIndex( m_RightHandWeapon.m_hObject, ns_szSelectAnimationName );
	m_nDeselectAni      = g_pLTClient->GetAnimIndex( m_RightHandWeapon.m_hObject, ns_szDeselectAnimationName );
	m_nReloadAni        = g_pLTClient->GetAnimIndex( m_RightHandWeapon.m_hObject, ns_szReloadAnimationName );
	m_nPlayerAni        = g_pLTClient->GetAnimIndex( m_RightHandWeapon.m_hObject, ns_szPlayerBaseAnimationName );

	m_nPreFireAni       = g_pLTClient->GetAnimIndex(m_RightHandWeapon.m_hObject, ns_szPreFireAnimationName );
	m_nPostFireAni      = g_pLTClient->GetAnimIndex(m_RightHandWeapon.m_hObject, ns_szPostFireAnimationName );

	char buf[30];
	int i;

	for ( i = 0; i < WM_MAX_IDLE_ANIS; ++i )
	{
		LTSNPrintF( buf, LTARRAYSIZE( buf ), "%s%d", ns_szIdleAnimationBasename, i );

		m_nIdleAnis[ i ] = g_pLTClient->GetAnimIndex( m_RightHandWeapon.m_hObject, buf );
	}

	for ( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		if ( i > 0 )
		{
			LTSNPrintF( buf, LTARRAYSIZE( buf ), "Fire%d", i );
		}
		else
		{
			LTSNPrintF( buf, LTARRAYSIZE( buf ), "Fire" );
		}

		m_nFireAnis[ i ] = g_pLTClient->GetAnimIndex( m_RightHandWeapon.m_hObject, buf );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayAnimation
//
//	PURPOSE:	Play an animation
//
// ----------------------------------------------------------------------- //

void CClientWeapon::PlayAnimation( uint32 dwAni, float fRate /*=1.0f*/, bool bLooping /*=false*/ )
{
	// The player body will notify the weapon of which animation to play...
	if( CPlayerBodyMgr::Instance( ).IsEnabled( ))
		return;

	if( m_RightHandWeapon.m_hObject )
	{
		if ( 0 < nsfOverrideRate )
		{
			fRate = nsfOverrideRate;
		}

		g_pModelLT->SetPlaying( m_RightHandWeapon.m_hObject, MAIN_TRACKER, true );
		g_pModelLT->SetLooping( m_RightHandWeapon.m_hObject, MAIN_TRACKER, bLooping );
		g_pModelLT->SetCurAnim( m_RightHandWeapon.m_hObject, MAIN_TRACKER, dwAni, true );
			
		LTRESULT ltResult = g_pModelLT->SetAnimRate( m_RightHandWeapon.m_hObject, MAIN_TRACKER, fRate );
		ASSERT( LT_OK == ltResult );
	}

	if( m_LeftHandWeapon.m_hObject )
	{
		if ( 0 < nsfOverrideRate )
		{
			fRate = nsfOverrideRate;
		}

		g_pModelLT->SetPlaying( m_LeftHandWeapon.m_hObject, MAIN_TRACKER, true );
		g_pModelLT->SetLooping( m_LeftHandWeapon.m_hObject, MAIN_TRACKER, bLooping );
		g_pModelLT->SetCurAnim( m_LeftHandWeapon.m_hObject, MAIN_TRACKER, dwAni, true );

		LTRESULT ltResult = g_pModelLT->SetAnimRate( m_LeftHandWeapon.m_hObject, MAIN_TRACKER, fRate );
		ASSERT( LT_OK == ltResult );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlaySelectAnimation()
//
//	PURPOSE:	Set model to select animation
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::PlaySelectAnimation()
{
// PLAYER_BODY...
	CPlayerBodyMgr &PlayerBodyMgr = CPlayerBodyMgr::Instance( );
	if( PlayerBodyMgr.IsEnabled( ) )
	{
		// See if the animation playing on the weapon context is a select animation...
		CPlayerBodyMgr::PlayerBodyContext eWeaponContext = PlayerBodyMgr.GetWeaponContext( );
		EnumAnimProp eActionProp = PlayerBodyMgr.GetCurrentAnimProp( eWeaponContext, kAPG_Action );

		// Begin the select animation if we haven't already...
		if( eActionProp != kAP_ACT_Select )
		{
			m_tmrFastSwitchDelay.Start(ns_vtFastSwitchDelay.GetFloat(0.1f));
			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_Select, CPlayerBodyMgr::kLocked );
			return true;
		}

		// Check if we're still playing the select animation...
		if( PlayerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_Select ))
			return true;

		// Animation is done playing...
		m_tmrFastSwitchDelay.Stop();
		return false;
	}

	uint32 dwSelectAni = GetSelectAni();

	if ( !m_RightHandWeapon.m_hObject || ( dwSelectAni == INVALID_ANI ) )
	{
		// object or animation not valid
		return false;
	}

	uint32 dwAni	= g_pLTClient->GetModelAnimation( m_RightHandWeapon.m_hObject );

	bool bIsSelectAni = IsSelectAni( dwAni );
	if ( bIsSelectAni && IsAnimationDone() )
	{
		// animation done
		return false;
	}
	
	if ( !bIsSelectAni )
	{
		// change to a select animation

		float fRate = GetSkillValue(eWeaponReloadSpeed);
		PlayAnimation( dwSelectAni, fRate );
	}

	return true;	// Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayDeselectAnimation()
//
//	PURPOSE:	Set model to deselect animation
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::PlayDeselectAnimation()
{
// PLAYER_BODY...
	CPlayerBodyMgr &PlayerBodyMgr = CPlayerBodyMgr::Instance( );
	if( PlayerBodyMgr.IsEnabled( ))
	{
		// Don't deselect until we are done attacking
		if (HandlingAnimationStimulusGroup("CS_Attack"))
			return true;

		// See if the animation playing on the weapon context is a fire animation...
		CPlayerBodyMgr::PlayerBodyContext eWeaponContext = PlayerBodyMgr.GetWeaponContext( );
		EnumAnimProp eActionProp = PlayerBodyMgr.GetCurrentAnimProp( eWeaponContext, kAPG_Action );

		// if we're playing a select anim, and we've been playing it long enough to see...
		if( eActionProp == kAP_ACT_Select && m_tmrFastSwitchDelay.IsStarted() && m_tmrFastSwitchDelay.IsTimedOut())
		{
			m_tmrFastSwitchDelay.Stop();

			//cancel the selection...
			CPlayerBodyMgr::Instance( ).ClearLockedProperties( kAPG_Action );
			CAnimationContext *pContext = CPlayerBodyMgr::Instance().GetAnimationContext(eWeaponContext);
			if (pContext)
			{
				pContext->ClearLock();
			}
			// Mark "deselected" and handle details during next update...
			m_bWeaponDeselected = true;
			return false;
		}


		// Begin the deselect animation if we haven't already...
		if( eActionProp != kAP_ACT_Deselect )
		{
			PlayerBodyMgr.SetAnimProp( kAPG_Action, kAP_ACT_Deselect, CPlayerBodyMgr::kLocked );
			return true;
		}

		// Check if we're still playing the deselect animation...
		if( PlayerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_Deselect ))
			return true;

		// Also make sure our custom weapon has finished deselecting.
		if (HandlingAnimationStimulusGroup(g_pClientWeaponMgr->GetCustomWeaponDeselectStimulus()))
			return true;

		// Animation is done...

		// Mark "deselected" and handle details during next update...
		m_bWeaponDeselected = true;
		return false;
	}

	uint32 dwDeselectAni = GetDeselectAni();

	if ( !m_RightHandWeapon.m_hObject || ( dwDeselectAni == INVALID_ANI ) )
	{
		// model or animation invalid

		// mark "deselected" and handle details during next update
		m_bWeaponDeselected = true;
		return false;
	}

	uint32 dwAni	= g_pLTClient->GetModelAnimation( m_RightHandWeapon.m_hObject );

	bool bIsDeselectAni = IsDeselectAni( dwAni );

	if ( bIsDeselectAni && IsAnimationDone() )
	{
		// animation is done

		// mark "deselected" and handle details during next update
		m_bWeaponDeselected = true;
		return false;
	}

	if ( !bIsDeselectAni )
	{
		// change to a deselect animation

		float fRate = GetSkillValue(eWeaponReloadSpeed);
		PlayAnimation( dwDeselectAni, fRate );
	}

	return true;	// Animation playing
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayReloadAnimation()
//
//	PURPOSE:	Set model to reloading animation
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::PlayReloadAnimation(bool bInterrupt)
{
	
// PLAYER_BODY...
	if( CPlayerBodyMgr::Instance( ).IsEnabled() )
	{
		// See if the animation playing on the weapon context is a fire animation...
		CPlayerBodyMgr::PlayerBodyContext eWeaponContext = CPlayerBodyMgr::Instance( ).GetWeaponContext( );
		EnumAnimProp eActionProp = CPlayerBodyMgr::Instance( ).GetCurrentAnimProp( eWeaponContext, kAPG_Action );

		ANIMTRACKERID nWeaponTrackerId = CPlayerBodyMgr::Instance( ).GetContextTrackerID( eWeaponContext );
		HOBJECT hPlayerBody = CPlayerBodyMgr::Instance( ).GetObject( );

		uint32 dwPlayback = 0;
		g_pModelLT->GetPlaybackState( hPlayerBody, nWeaponTrackerId, dwPlayback );


		bool bIsMainFireAni		= (eActionProp == kAP_ACT_Fire);
		bool bIsPreFireAni		= (eActionProp == kAP_ACT_PreFire);
		bool bIsPostFireAni		= (eActionProp == kAP_ACT_PostFire);
		bool bIsFireAni			= (bIsMainFireAni || bIsPreFireAni || bIsPostFireAni);
		bool bIsReloadAni		= (eActionProp == kAP_ACT_Reload);
		bool bCurAniDone		= (dwPlayback & MS_PLAYDONE);
		bool bCanPlay			= (!bIsFireAni || bCurAniDone);

		// Make sure the *entire* fire animation sequence has finished playing...
		if( bIsFireAni )
		{
			bCanPlay = false;

			if( bIsPostFireAni && !CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, kAP_ACT_PostFire ))
			{ 
				bCanPlay = true;
			}
			else if( (bIsPreFireAni && !CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, kAP_ACT_PreFire )) ||
					 (bIsMainFireAni && !CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, kAP_ACT_Fire )) )
			{
				PlayFireAnimation( false );
			}
		}

		if (bInterrupt)
		{
			m_bReloadInterrupted = true;
			if (bIsReloadAni)
			{
				CPlayerBodyMgr::Instance( ).ClearLockedProperties(kAPG_Action);
				CAnimationContext* pWeaponContext = CPlayerBodyMgr::Instance( ).GetAnimationContext( eWeaponContext );
				pWeaponContext->ClearLock();
				
			}
			return false;
		}

		if( bIsReloadAni && !CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, kAP_ACT_Reload ))
		{
			// Set ammo in clip amount...
			SplitAmmoBetweenClips( );

			// Update the player's stats...
			uint32 nAmmo = g_pPlayerStats->GetAmmoCount( m_hAmmo );

			g_pPlayerStats->UpdateAmmo( m_hWeapon, m_hAmmo, nAmmo, false, true, WDB_INVALID_WEAPON_INDEX );
			g_pPlayerStats->UpdatePlayerWeapon( m_hWeapon, m_hAmmo );
			m_bReloadInterrupted = false;

			return false;
		}
		else if( !bIsReloadAni && bCanPlay )
		{
			// Make sure we do special end fire before changing animations...
			if( bIsFireAni && bCurAniDone )
			{
				DoSpecialEndFire();
			}

			ASSERT( 0 != g_pPlayerStats );
			float fRate = 1.0f;

			// Scale the animation rate...

			if( m_hWeapon )
			{
				HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);
				fRate *= g_pWeaponDB->GetFloat( hWpnData, WDB_WEAPON_fReloadAnimRateScale );
			}

			// Scale the animation rate based on skill modifiers...
				fRate *= GetSkillValue(eWeaponReloadSpeed);

			CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_Reload, CPlayerBodyMgr::kLocked );

			// Tell the server we're playing the reload ani...
			CAutoMessage cMsg;
			LTRESULT ltResult;

			cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
			cMsg.Writeuint8( CP_WEAPON_STATUS );
			cMsg.Writeuint8( WS_RELOADING );

			ltResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
			ASSERT( LT_OK == ltResult );
		}

		// Animation playing
		return true;
	}

	uint32 dwReloadAni = GetReloadAni();

	if ( !m_RightHandWeapon.m_hObject || ( INVALID_ANI == dwReloadAni ) )
	{
/*
		// Set ammo in clip amount...
		m_nAmmoInClip = m_nNewAmmoInClip;

		// Update the player's stats...
		uint32 nAmmo = g_pPlayerStats->GetAmmoCount( m_hAmmo );

		g_pPlayerStats->UpdateAmmo( m_hWeapon, m_hAmmo, nAmmo );
		g_pPlayerStats->UpdatePlayerWeapon( m_hWeapon, m_hAmmo );
		*/
		return false;
	}

	uint32 dwAni	= g_pLTClient->GetModelAnimation( m_RightHandWeapon.m_hObject );

	bool bCurAniDone = IsAnimationDone();
	bool bIsFireAni = IsFireAni(dwAni);

	bool bCanPlay  = ( !bIsFireAni || bCurAniDone ||
	                   g_pLTClient->GetModelLooping( m_RightHandWeapon.m_hObject ) );

	// Make sure the *entire* fire animation sequence has finished playing...
	
	if( bIsFireAni && (GetPostFireAni() != INVALID_ANI) )
	{
		bCanPlay = false;

		if( IsPostFireAni( dwAni ) && bCurAniDone )
		{ 
			bCanPlay = true;
		}
		else if( bCurAniDone )
		{
			PlayFireAnimation( false );
		}
	}

	bool bIsReloadAni = IsReloadAni( dwAni );

	if ( bIsReloadAni && bCurAniDone )
	{
		// Set ammo in clip amount...
		SplitAmmoBetweenClips( );

		// Update the player's stats...
		uint32 nAmmo = g_pPlayerStats->GetAmmoCount( m_hAmmo );

		g_pPlayerStats->UpdateAmmo( m_hWeapon, m_hAmmo, nAmmo, false, true, WDB_INVALID_WEAPON_INDEX );
		g_pPlayerStats->UpdatePlayerWeapon( m_hWeapon, m_hAmmo );

		return false;
	}
	else if ( !bIsReloadAni && bCanPlay )
	{
		// Make sure we do special end fire before changing animations...
		if ( bIsFireAni && bCurAniDone )
		{
			DoSpecialEndFire();
		}

		ASSERT( 0 != g_pPlayerStats );
		float fRate = 1.0f;
		
		// Scale the animation rate...

		if( m_hWeapon )
		{	
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);
			fRate *= g_pWeaponDB->GetFloat( hWpnData, WDB_WEAPON_fReloadAnimRateScale );
		}

		// Scale the animation rate based on skill modifiers...
			fRate *= GetSkillValue(eWeaponReloadSpeed);
		PlayAnimation( dwReloadAni, fRate );

		// Tell the server we're playing the reload ani...
		CAutoMessage cMsg;
		LTRESULT ltResult;

		cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
		cMsg.Writeuint8( CP_WEAPON_STATUS );
		cMsg.Writeuint8( WS_RELOADING );

		ltResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
		ASSERT( LT_OK == ltResult );
	}

	// Animation playing
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayIdleAnimation()
//
//	PURPOSE:	Set model to Idle animation
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::PlayIdleAnimation()
{
	ASSERT( 0 != g_pGameClientShell );
	ASSERT( 0 != g_pLTClient );

	// Don't play idle animations with the PlayerBody...
	if( CPlayerBodyMgr::Instance().IsEnabled( ))
		return false;

	if ( !m_RightHandWeapon.m_hObject )
		return false;

	// determine of the current animation is done
	bool bCurAniDone = IsAnimationDone();

	// Make sure idle animation is done if one is currently playing...
	uint32 dwAni = g_pLTClient->GetModelAnimation( m_RightHandWeapon.m_hObject );
	if ( IsIdleAni( dwAni ) )
	{
		if ( !bCurAniDone )
		{
			return true;
		}
	}

	// See if the player is moving...Don't do normal idles when player is
	// moving...
	bool bMoving = false;
	if ( 0.1f < g_pPlayerMgr->GetMoveMgr()->GetVelocity().Mag() )
	{
		bMoving = !!( g_pPlayerMgr->GetPlayerFlags() & BC_CFLG_MOVING );
	}

	// Play idle if it is time...(and not moving)...
	// Don't play fidget animation if we're zoomed or moving.
	bool bPlayIdle = false;
	if ( m_NextIdleTime.IsTimedOut( ) && bCurAniDone )
	{
		bPlayIdle = ( !bMoving && !g_pPlayerMgr->GetPlayerCamera()->IsZoomed());
		m_NextIdleTime.Start( GetNextIdlePeriod( ));
	}

	// get a subtle animation in case we will be playing it
	uint32 nSubtleIdleAni = GetSubtleIdleAni();

	// determine which kind of idle to play (if any)
	if ( bPlayIdle )
	{
		//
		// play normal idle animation
		//

		// the the animation
		uint32 nAni = GetIdleAni();

		// make sure if its valid
		if ( nAni == INVALID_ANI )
		{
			nAni = DEFAULT_ANI;
		}

		// play it
		PlayAnimation( nAni, false );

		// return true because vaild idle playing
		return true;
	}
	else if ( nSubtleIdleAni != INVALID_ANI )
	{
		// Play subtle idle...
		if ( ( dwAni != nSubtleIdleAni ) || bCurAniDone )
		{
			PlayAnimation( nSubtleIdleAni );
		}

		// return true because vaild idle playing
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayFireAnimation()
//
//	PURPOSE:	Set model to firing animation.  If the model has a PreFire animation
//				we will play that first and then play the Fire animation.  If the model
//				has a PostFire animation we will play that as soon as the Fire ani is done. 
//
// ----------------------------------------------------------------------- //

static uint32 GetFireAnimationRandomSeed( CPlayerBodyMgr* pPBM, CAnimationContext* pContext )
{
	CAnimationProps iProps = pPBM->GetAnimationProps();

	for( uint32 i = 0; i < kAPG_Count; ++i )
	{
		EnumAnimProp eProp = iProps.Get( ( EnumAnimPropGroup )i );

		if( eProp == kAP_None )
		{
			iProps.Set( ( EnumAnimPropGroup )i, kAP_Any );
		}
	}

	uint32 nAnims = pContext->CountAnimations( iProps );
	return GetRandom( 0, ( nAnims - 1 ) );
}

// ----------------------------------------------------------------------- //

bool CClientWeapon::PlayFireAnimation( bool bResetAni, bool bPlayOnce /*=false*/ )
{
	CPlayerBodyMgr &PlayerBodyMgr = CPlayerBodyMgr::Instance( );

	if( !CPlayerBodyMgr::Instance( ).IsEnabled() )
	{
		return false;
	}

	// See if the animation playing on the weapon context is a fire animation...
	CPlayerBodyMgr::PlayerBodyContext eWeaponContext = PlayerBodyMgr.GetWeaponContext();
	EnumAnimProp eActionProp = PlayerBodyMgr.GetCurrentAnimProp( eWeaponContext, kAPG_Action );
	CAnimationContext* pWeaponContext = PlayerBodyMgr.GetAnimationContext( eWeaponContext );

	bool bIsMainFireAni		= (eActionProp == kAP_ACT_Fire);
	bool bIsPreFireAni		= (eActionProp == kAP_ACT_PreFire);
	bool bIsPostFireAni		= (eActionProp == kAP_ACT_PostFire);
	bool bIsFireAni			= (bIsMainFireAni || bIsPreFireAni || bIsPostFireAni);

	HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData( m_hWeapon, !USE_AI_DATA);
	bool bUsePreFire = g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bUsePreFire );
	bool bUsePostFire = g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bUsePostFire );
	bool bLoopPrePostFire = g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bLoopPrePostFire );

	// Semi-Auto weapons should not loop through a frame...
	bool bLoopThroughFrame = !g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bSemiAuto );
	static uint32 nRandomSeed = -1;

	if( bLoopPrePostFire && pWeaponContext )
	{
		pWeaponContext->SetRandomSeed( nRandomSeed );
		pWeaponContext->LoopThroughFrame( false );
		bLoopThroughFrame = false;
	}


	bool bDualWeapon = !!g_pWeaponDB->GetRecordLink( hWeaponData, WDB_WEAPON_rSingleWeapon );

	if( bDualWeapon )
	{
		// Can't loop dual weapons since they use more than one fire animation...
		bLoopThroughFrame = false;

		// Determine which weapons can fire...
		bool bLeftCanFire = m_LeftHandWeapon.CanFire( );
		bool bRightCanFire = m_RightHandWeapon.CanFire( );

		if( bLeftCanFire && bRightCanFire )
		{
			// Both weapons can fire so randomly choose one...
			EnumAnimProp eWeaponLimb = (GetRandom( 0, 1 ) == 1 ? kAP_WHAND_Left : kAP_WHAND_Right);
			PlayerBodyMgr.SetAnimProp( kAPG_WeaponHand, eWeaponLimb );
		}
		else if( !bRightCanFire && (m_LeftHandWeapon.GetAmmoInClip() > 0) )
		{
			// Right can't fire so use the left as long as it has ammo...
			PlayerBodyMgr.SetAnimProp( kAPG_WeaponHand, kAP_WHAND_Left );
		}
		else if( !bLeftCanFire && (m_RightHandWeapon.GetAmmoInClip() > 0) )
		{
			// Left can't fire so use the right as long as it has ammo...
			PlayerBodyMgr.SetAnimProp( kAPG_WeaponHand, kAP_WHAND_Right );
		}
	}

	// First handle case of not currently playing any type of fire animation
	// but wanting to fire the weapon...
	if( !bIsFireAni && (bResetAni || bPlayOnce) && bUsePreFire )
	{
		// Try to play a pre-fire animation...
		// If no pre-fire animation exists a regular fire animation will try to play...
		PlayerBodyMgr.SetAnimProp( kAPG_Action, kAP_ACT_PreFire, CPlayerBodyMgr::kLocked );

		if( bLoopPrePostFire && pWeaponContext && ( nRandomSeed == -1 ) )
		{
			nRandomSeed = GetFireAnimationRandomSeed( &PlayerBodyMgr, pWeaponContext );
			pWeaponContext->SetRandomSeed( nRandomSeed );
		}

		return true;
	}

	// Now handle the case where we are already playing a fire animation...
	if( bIsFireAni || !bUsePreFire )
	{
		// The current animation is done, so figure out what to do now...
		if ( bIsPreFireAni || (!bUsePreFire && !bIsMainFireAni) )
		{
			if( PlayerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_PreFire ))
				return true;

			PlayerBodyMgr.SetAnimProp( kAPG_Action, kAP_ACT_Fire, CPlayerBodyMgr::kLocked );
			pWeaponContext->LoopThroughFrame( bLoopThroughFrame );

			return true;
		}
		else if ( bIsMainFireAni )
		{
			if( PlayerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_Fire ))
			{
				// Kill the loop if the player no longer holding fire down...
				if( bLoopThroughFrame && !(g_pPlayerMgr->GetPlayerFlags() & BC_CFLG_FIRING) )
					pWeaponContext->LoopThroughFrame( false );
							
				return true;
			}

			// We just finished playing the main fire ani, play it again if bResetAni
			// is true and we have ammo (i.e., they are holding down the fire key).  
			// Else, if we have a post-fire ani, play it...

			if( bResetAni && g_pPlayerStats->GetAmmoCount( m_hAmmo ) > 0 )
			{
				if( bLoopPrePostFire )
				{
					if( !bUsePostFire )
						return false;

					PlayerBodyMgr.SetAnimProp( kAPG_Action, kAP_ACT_PostFire, CPlayerBodyMgr::kLocked );
				}
				else
				{
					PlayerBodyMgr.SetAnimProp( kAPG_Action, kAP_ACT_Fire, CPlayerBodyMgr::kLocked );
					pWeaponContext->LoopThroughFrame( bLoopThroughFrame );
				}

				return true;
			}
			else if( bUsePostFire )
			{
				PlayerBodyMgr.SetAnimProp( kAPG_Action, kAP_ACT_PostFire, CPlayerBodyMgr::kLocked );
				return true;
			}

		}
		else if( bIsPostFireAni )
		{
			if( PlayerBodyMgr.IsLocked( kAPG_Action, kAP_ACT_PostFire ))
				return true;

			// We just finished playing the post-fire ani, start the cycle again if bResetAni
			// is true and we have ammo (i.e., they are holding down the fire key).  

			if( ( m_bQueuedFireAnimation || bResetAni ) && g_pPlayerStats->GetAmmoCount( m_hAmmo ) > 0  )
			{
				m_bQueuedFireAnimation = false;

				if( bUsePreFire )
				{
					PlayerBodyMgr.SetAnimProp( kAPG_Action, kAP_ACT_PreFire, CPlayerBodyMgr::kLocked );

					if( bLoopPrePostFire && pWeaponContext )
					{
						nRandomSeed = GetFireAnimationRandomSeed( &PlayerBodyMgr, pWeaponContext );
						pWeaponContext->SetRandomSeed( nRandomSeed );
					}
				}
				else
				{
					PlayerBodyMgr.SetAnimProp( kAPG_Action, kAP_ACT_Fire, CPlayerBodyMgr::kLocked );
					pWeaponContext->LoopThroughFrame( bLoopThroughFrame );
				}

				return true;
			}

		}
	}

	nRandomSeed = -1;
	DoSpecialEndFire();

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayAltFireAnimation()
//
//	PURPOSE:	Set model to alt-firing animation...
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::PlayAltFireAnimation( bool bResetAni, bool bPlayOnce /*=false*/ )
{
	if( CPlayerBodyMgr::Instance( ).IsEnabled() )
	{
		// See if the animation playing on the weapon context is an alt-fire animation...
		CPlayerBodyMgr::PlayerBodyContext eWeaponContext = CPlayerBodyMgr::Instance( ).GetWeaponContext( );
		EnumAnimProp eActionProp = CPlayerBodyMgr::Instance( ).GetCurrentAnimProp( eWeaponContext, kAPG_Action );

		bool bIsAltFireAni = (eActionProp == kAP_ACT_FireSecondary);
		
		// First handle case of not currently playing any type of alt-fire animation
		// but wanting to alt-fire the weapon...
		if( !bIsAltFireAni && (bResetAni || bPlayOnce))
		{
			return CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_FireSecondary, CPlayerBodyMgr::kLocked );
		}

		if( bIsAltFireAni )
		{
			if( CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, kAP_ACT_FireSecondary ))
				return true;

			if( bResetAni )
			{
				return CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, kAP_ACT_FireSecondary, CPlayerBodyMgr::kLocked );
			}
		}

		// Animation done playing...
		return false;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayThrowAni()
//
//	PURPOSE:	Helper for handling throw animations...
//				(Factored out to support additional animation restrictions.)
//
// ----------------------------------------------------------------------- //

void CClientWeapon::PlayThrowAni( EnumAnimProp eProp, bool bLocked ) const
{
	CPlayerBodyMgr::AnimPlay ePlay = bLocked ? CPlayerBodyMgr::kLocked : CPlayerBodyMgr::kUnlocked;

	bool bSet = CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_Action, eProp, ePlay );
	if (!bSet)
	{
		LTERROR("failed to play thow ani");
	}

	switch (g_pPlayerMgr->GetSonicType())
	{
	case eSonicType_Blast:
		switch (g_pPlayerMgr->GetSonicData().GetSkillLevel( "Blast" ))
		{
		case 1: CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_SonicType, kAP_SONIC_Blast, ePlay ); break;
		case 2: CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_SonicType, kAP_SONIC_Blast2, ePlay ); break;
		}
		break;
	case eSonicType_Guide:
		switch (g_pPlayerMgr->GetSonicData().GetSkillLevel( "Guide" ))
		{
		case 1: CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_SonicType, kAP_SONIC_Guide, ePlay ); break;
		case 2: CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_SonicType, kAP_SONIC_Guide2, ePlay ); break;
		}
		break;
	case eSonicType_Alter:
		switch (g_pPlayerMgr->GetSonicData().GetSkillLevel( "Alter" ))
		{
		case 1: CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_SonicType, kAP_SONIC_Alter, ePlay ); break;
		case 2: CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_SonicType, kAP_SONIC_Alter2, ePlay ); break;
		}
		break;
	case eSonicType_Disabled:
		CPlayerBodyMgr::Instance( ).SetAnimProp( kAPG_SonicType, kAP_SONIC_Disabled, ePlay );
		break;
	default:
		break;
	};
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayGrenadeAnimation()
//
//	PURPOSE:	Set model to grenade throwing animation...
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::PlayGrenadeAnimation( bool bResetAni, bool bPlayOnce /*=false*/ )
{
	if( CPlayerBodyMgr::Instance( ).IsEnabled() )
	{
		// See if the animation playing on the weapon context is a fire animation...
		CPlayerBodyMgr::PlayerBodyContext eWeaponContext = CPlayerBodyMgr::Instance( ).GetWeaponContext( );
		EnumAnimProp eActionProp = CPlayerBodyMgr::Instance( ).GetCurrentAnimProp( eWeaponContext, kAPG_Action );

		ANIMTRACKERID nWeaponTrackerId = CPlayerBodyMgr::Instance( ).GetContextTrackerID( eWeaponContext );
		HOBJECT hPlayerBody = CPlayerBodyMgr::Instance( ).GetObject( );

		bool bIsThrowStartAni	= (eActionProp == kAP_ACT_ThrowStart);
		bool bIsThrowHoldAni	= (eActionProp == kAP_ACT_ThrowHold);
		bool bIsMainThrowAni	= (eActionProp == kAP_ACT_Throw);
		bool bIsThrowAni		= (bIsMainThrowAni || bIsThrowHoldAni || bIsThrowStartAni);

		// First handle case of not currently playing any type of throw animation
		// but wanting to throw a grenade...
		if( !bIsThrowAni && (bResetAni || bPlayOnce) )
		{
			// Try to play a pre-fire animation...
			// If no pre-fire animation exists a regular fire animation will try to play...
			PlayThrowAni(kAP_ACT_ThrowStart, true);
			return true;
		}

		// Now handle the case where we are already playing a throw animation...
		if( bIsThrowAni )
		{
			// The current animation is done, so figure out what to do now...
			if ( bIsThrowStartAni )
			{
				if( CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, kAP_ACT_ThrowStart ))
					return true;

				// We just finished playing the start ani, check to see if we are holding or throwing...
				if( bResetAni || bPlayOnce )
				{
					// Play the hold animation unlocked so the throw animation is immediate...
					PlayThrowAni(kAP_ACT_ThrowHold, false);
					if( bPlayOnce )
						return false;
				}
				else
				{
					PlayThrowAni(kAP_ACT_Throw, true);
				}
				return true;				
			}
			else if ( bIsThrowHoldAni )
			{
				// We just finished playing the hold ani, play it again if bResetAni is set, otherwise release the grenade
				if( (bResetAni) )//!!ARL && !CPlayerBodyMgr::Instance( ).GetGrenadeWeapon( )->m_bSemiAuto )
				{
					// Play the hold animation unlocked so the throw animation is immediate...
					PlayThrowAni(kAP_ACT_ThrowHold, false);
					return true;
				}
				else
				{
					PlayThrowAni(kAP_ACT_Throw, true);
					return true;
				}

			}
			else if( bIsMainThrowAni )
			{
				if( CPlayerBodyMgr::Instance( ).IsLocked( kAPG_Action, kAP_ACT_Throw ))
					return true;

				// We just finished playing the throw ani, start the cycle again if bResetAni
				// is true and we have ammo (i.e., they are holding down the throw key again).  

				if( bResetAni && g_pPlayerStats->GetAmmoCount( m_hAmmo ) > 0  )
				{
					PlayThrowAni(kAP_ACT_ThrowStart, false);
					return true;
				}

			}
		}

		// If we got here, it means we're done firing...
		return false;

	}
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayBlockAnimation()
//
//	PURPOSE:	Set model to blocking animation
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::PlayBlockAnimation()
{
	if( CPlayerBodyMgr::Instance().IsEnabled() )
	{
		// See if the animation playing on the weapon context is a fire animation...
		CPlayerBodyMgr::PlayerBodyContext eWeaponContext = CPlayerBodyMgr::Instance().GetWeaponContext( );
		EnumAnimProp eActionProp = CPlayerBodyMgr::Instance().GetCurrentAnimProp( eWeaponContext, kAPG_Action );

		if ( eActionProp != kAP_ACT_Block )
		{
			// Start out by playing the block animation if we're not currently blocking...
			CPlayerBodyMgr::Instance().SetAnimProp( kAPG_Action, kAP_ACT_Block, CPlayerBodyMgr::kLocked );

			// Tell the server we're playing the block ani...
			CAutoMessage cMsg;
			LTRESULT ltResult;

			cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
			cMsg.Writeuint8( CP_WEAPON_STATUS );
			cMsg.Writeuint8( WS_BLOCKING );

			ltResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
			ASSERT( LT_OK == ltResult );

			return true;
		}

		// If we're still playing the animation... just continue
		if( CPlayerBodyMgr::Instance().IsLocked( kAPG_Action, kAP_ACT_Block ) )
		{
			return true;
		}
		
		// Play it again if we've got one queued up.
		if (m_bQueuedBlockAnimation)
		{
			m_bQueuedBlockAnimation = false;
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayCheckAmmoAnimation()
//
//	PURPOSE:	Set model to checking ammo animation
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::PlayCheckAmmoAnimation()
{
	if( CPlayerBodyMgr::Instance().IsEnabled() )
	{
		// See if the animation playing on the weapon context is a checking ammo animation...
		CPlayerBodyMgr::PlayerBodyContext eWeaponContext = CPlayerBodyMgr::Instance().GetWeaponContext( );
		EnumAnimProp eActionProp = CPlayerBodyMgr::Instance().GetCurrentAnimProp( eWeaponContext, kAPG_Action );

		if ( eActionProp != kAP_ACT_CheckAmmo )
		{
			// Start out by playing the check ammo animation if we're not currently checking ammo...
			CPlayerBodyMgr::Instance().SetAnimProp( kAPG_Action, kAP_ACT_CheckAmmo, CPlayerBodyMgr::kLocked );

			// Notify the hud that we've begun checking ammo.
			if( g_pHUDAmmoStack )
				g_pHUDAmmoStack->SetAmmoType(m_hAmmo);

			return true;
		}

		// If we're still playing the animation... just continue
		if( CPlayerBodyMgr::Instance().IsLocked( kAPG_Action, kAP_ACT_CheckAmmo ) )
		{
			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::IsSelectAni()
//
//	PURPOSE:	Is this a valid Select ani
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::IsSelectAni( uint32 dwAni ) const
{
	if ( INVALID_ANI == dwAni )
	{
		return false;
	}

	if( dwAni == m_nSelectAni )
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetSelectAni()
//
//	PURPOSE:	Get a select animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeapon::GetSelectAni() const
{
		return m_nSelectAni;
	}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::IsDeselectAni()
//
//	PURPOSE:	Is this a valid deselect ani
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::IsDeselectAni( uint32 dwAni ) const
{
	if ( INVALID_ANI == dwAni )
	{
		return false;
	}

	if ( dwAni == m_nDeselectAni )
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetDeselectAni()
//
//	PURPOSE:	Get a deselect animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeapon::GetDeselectAni() const
{
	return m_nDeselectAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::IsReloadAni()
//
//	PURPOSE:	Is this a valid Reload ani
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::IsReloadAni( uint32 dwAni ) const
{
	if ( INVALID_ANI == dwAni )
	{
		return false;
	}

	if ( dwAni == m_nReloadAni )
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetReloadAni()
//
//	PURPOSE:	Get a reload animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeapon::GetReloadAni() const
{
		return m_nReloadAni;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetPlayerAni()
//
//	PURPOSE:	Get player base animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeapon::GetPlayerAni() const
{
	return m_nPlayerAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::IsIdleAni()
//
//	PURPOSE:	Is the passed in animation an idle animation (NOTE this
//				will return false if the passed in animation is a subtle
//				idle animation).
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::IsIdleAni( uint32 dwAni ) const
{
	if ( INVALID_ANI == dwAni )
	{
		return false;
	}

	int i;

	// start at 1 because 0 is reserved for the subtle idle
	for ( i = 1; i < WM_MAX_IDLE_ANIS; ++i )
	{
		if ( m_nIdleAnis[ i ] == dwAni )
		{
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetIdleAni()
//
//	PURPOSE:	Get an idle animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeapon::GetIdleAni() const
{
	int nNumValid = 0;
		uint32 dwValidIdleAnis[ WM_MAX_IDLE_ANIS ];

		// Note that we skip the first ani, this is reserved for
		// the subtle idle ani...
		for ( int i = 1; i < WM_MAX_IDLE_ANIS; ++i )
		{
			if ( m_nIdleAnis[ i ] != INVALID_ANI )
			{
				dwValidIdleAnis[ nNumValid ] = m_nIdleAnis[ i ];
				++nNumValid;
			}
		}

		if ( 0 < nNumValid )
		{
			return dwValidIdleAnis[ GetRandom( 0, ( nNumValid - 1 ) ) ];
		}

	return INVALID_ANI;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetSubtleIdleAni()
//
//	PURPOSE:	Get a sutble idle animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeapon::GetSubtleIdleAni() const
{
		return m_nIdleAnis[ 0 ];
	}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetNextIdlePeriod()
//
//	PURPOSE:	Determine the period of time before we should play an idle animation
//
// ----------------------------------------------------------------------- //

float CClientWeapon::GetNextIdlePeriod() const
{
	return GetRandom( WEAPON_MIN_IDLE_TIME, WEAPON_MAX_IDLE_TIME );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::IsPreFireAni
//
//  PURPOSE:	Is the passed in animation a pre-fire animation
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::IsPreFireAni( uint32 dwAni ) const
{
	if( INVALID_ANI == dwAni )
	{
		return false;
	}

	if( dwAni == m_nPreFireAni )
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::GetPreFireAni
//
//  PURPOSE:	Get the pre-fire animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeapon::GetPreFireAni( ) const
{
	return m_nPreFireAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::IsFireAni()
//
//	PURPOSE:	Is the passed in animation any one of the fire animations
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::IsFireAni( uint32 dwAni, bool bCheckNormalOnly /*= false*/) const
{
	if ( INVALID_ANI == dwAni )
	{
		return false;
	}

	int i;
	for ( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		if ( m_nFireAnis[ i ] == dwAni )
		{
			return true;
		}
	}

	// We want to see if the animation is a PreFire ani or PostFire ani because
	// they can be thought of as part of the entire Fire animation sequence.
	if ( !bCheckNormalOnly && (IsPreFireAni( dwAni ) || IsPostFireAni( dwAni )) )
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetFireAni()
//
//	PURPOSE:	Get the fire animation based on the fire type
//
// ----------------------------------------------------------------------- //

uint32 CClientWeapon::GetFireAni( ) const
{
	int nNumValid = 0;
		uint32 dwValidFireAnis[ WM_MAX_FIRE_ANIS ];

		for ( int i = 0; i < WM_MAX_FIRE_ANIS; ++i )
		{
			if ( INVALID_ANI != m_nFireAnis[ i ] )
			{
				dwValidFireAnis[ nNumValid ] = m_nFireAnis[ i ];
				++nNumValid;
			}
		}

		if ( nNumValid > 0 )
		{
			return dwValidFireAnis[ GetRandom( 0, ( nNumValid - 1 ) ) ];
		}

	return INVALID_ANI;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::IsPostFireAni
//
//  PURPOSE:	Is the passed in animation a post-fire animation
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::IsPostFireAni( uint32 dwAni ) const
{
	if( INVALID_ANI == dwAni )
	{
		return false;
	}

	if( dwAni == m_nPostFireAni )
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::GetPostFireAni
//
//  PURPOSE:	get the post-fire animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeapon::GetPostFireAni() const
{
	return m_nPostFireAni;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::UpdateMods()
//
//	PURPOSE:	Update the mods for this weapon
//
// ----------------------------------------------------------------------- //
void CClientWeapon::UpdateMods()
{
	// Update the silencer...
	UpdateSilencer();

	// Update the scope...
	UpdateScope();
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::RemoveMods()
//
//	PURPOSE:	Remove the mods for this weapon
//
// ----------------------------------------------------------------------- //
void CClientWeapon::RemoveMods()
{
	// Remove the silencer models...
	m_RightHandWeapon.RemoveSilencer( );
	m_LeftHandWeapon.RemoveSilencer( );

	m_bHaveSilencer = false;

	// Remove the scope model...
	m_RightHandWeapon.RemoveScope( );
	m_LeftHandWeapon.RemoveScope( );

	m_bHaveScope   = false;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::SetModVisibility()
//
//	PURPOSE:	Show/hide the mods
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetModVisibility(bool bVisible, bool bShadow ) 
{
	// Hide/Show silencer...
	m_RightHandWeapon.SetSilencerVisibility( bVisible, bShadow );
	m_LeftHandWeapon.SetSilencerVisibility( bVisible, bShadow );

	// Hide/Show scope...
	m_RightHandWeapon.SetScopeVisibility( bVisible, bShadow );
	m_LeftHandWeapon.SetScopeVisibility( bVisible, bShadow );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::Fire
//
//	PURPOSE:	Fire the weapon
//
// ----------------------------------------------------------------------- //

void CClientWeapon::Fire( )
{
	LTASSERT( 0 != m_hWeapon, "Trying to fire with an invalid weapon record." );
	LTASSERT( 0 != m_hAmmo, "Trying to fire with an invalid ammo record." );

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_hWeapon, !USE_AI_DATA );
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData( m_hAmmo, !USE_AI_DATA );

	if( (AmmoType)g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType ) == TRIGGER )
	{
		// send a fire message to the server
		SendTriggerFireMessage();
	}
	else
	{
		// perturb based on player's movement/firing state
		float fPerturb = CAccuracyMgr::Instance().GetCurrentPerturb();

		// get the player's aim modifier
		float fPerturbX = GetSkillValue(eAimAccuracy);

		// factor the modifier into the perturb
		fPerturb *= fPerturbX;

		// fire position/direction information
		LTVector vU, vR, vF, vFirePos;

		// Get the fire pos/rot
		if ( !GetFireVectors( vR, vU, vF, vFirePos ) )
		{
			return;
		}

		// Make sure we always ignore the fire sounds...
		m_wIgnoreFX = WFX_FIRESOUND | WFX_ALTFIRESND;

		if ( !m_bHaveSilencer )
		{
			m_wIgnoreFX |= WFX_SILENCED;
		}

		// Create a client-side projectile for every vector...
		CWeaponPath wp;
		wp.m_hWeapon	= m_hWeapon;
		wp.m_fPerturb	= fPerturb;
		wp.m_hAmmo		= m_hAmmo;
		wp.m_hFiredFrom	= g_pLTClient->GetClientObject( );
		wp.m_vFirePos	= vFirePos;
        wp.m_vFlashPos	= GetFlashPos( );
		wp.m_nFireTimeStamp = g_pGameClientShell->GetServerRealTimeMS( );
		
		int32 nWeaponRange	= g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nRange );
		int32 nAmmoRange	= g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nRange );
		wp.m_fRange			= (float)(nAmmoRange > 0 ? nAmmoRange : nWeaponRange);

		wp.IgnoreObject( CPlayerBodyMgr::Instance( ).GetObject( ));

		// Don't hit any turret object currently being controlled...
		if( g_pPlayerMgr->IsOperatingTurret( ) && g_pPlayerMgr->GetTurret( ))
		{
			wp.IgnoreObject( g_pPlayerMgr->GetTurret( )->GetServerObj( ));
		}

		wp.m_fnOnImpactCB = WeaponPath_OnImpactCB;
		wp.m_pImpactCBUserData = this;

		

		// Calculate a random seed...(srand uses this value so it can't be 1, since
		// that has a special meaning for srand)
		m_nRandomSeed = GetRandom( 2, 255 );
		wp.m_iRandomSeed = m_nRandomSeed;
		wp.m_iPerturbCount = m_nPerturbCount;


		LTVector vObjectImpactPos;
		HOBJECT hObjectImpact = INVALID_HOBJECT;
		
		uint8 nVectorsPerRound = g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nVectorsPerRound );
		for( uint8 nVector = 0; nVector < nVectorsPerRound; ++nVector )
		{
			if( (AmmoType)g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType ) == VECTOR )
			{
				wp.m_vPath = vF;
				wp.PerturbWeaponPath( !USE_AI_DATA );
				wp.DoVector( );
			}
		}

		// send a fire message to the server
		SendFireMessage( fPerturb, vFirePos, vF );

		m_nPerturbCount += nVectorsPerRound;
	}

	

	// Play Fire sound...
	uint8 nFireType = GetLastSndFireType();

	PlayerSoundId eSoundId = PSI_FIRE;
	if ( nFireType == PSI_SILENCED_FIRE )
	{
		eSoundId = PSI_SILENCED_FIRE;
	}
	else if ( nFireType == PSI_ALT_FIRE )
	{
		eSoundId = PSI_ALT_FIRE;
	}

	LTVector vPos( 0, 0, 0 );
	::PlayWeaponSound( m_hWeapon, !USE_AI_DATA, vPos, eSoundId, true );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SendFireMessage()
//
//	PURPOSE:	Send fire message to server
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SendFireMessage( float fPerturb, 
                                     LTVector const &vFirePos,
                                     LTVector const &vDir )
{
	CAutoMessage cMsg;
	LTRESULT msgResult;

	cMsg.Writeuint8( MID_WEAPON_FIRE );

	// The server knows the weapon the client is currently using so that doesn't
	// need to be sent.  Grenades and melee are handled differently, so they will be sent...
	
	bool bSendWeaponRecord = false;
	HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData( m_hWeapon, !USE_AI_DATA );
	if( g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bIsGrenade ) || (m_hWeapon == g_pWeaponDB->GetUnarmedRecord( )) )
		bSendWeaponRecord = true;

	cMsg.Writebool( bSendWeaponRecord );
	if( bSendWeaponRecord )
	{
		// Record of the weapon that is firing
		cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hWeapon );
	}

	// The ammo should only be required to be sent if the weapon has multiple ammo types...
	bool bSendAmmoRecord = false;
	if( g_pWeaponDB->GetNumValues( hWeaponData, WDB_WEAPON_rAmmoName ) > 1 )
		bSendAmmoRecord = true;
	
	cMsg.Writebool( bSendAmmoRecord );
	if( bSendAmmoRecord )
	{
		// Record of the ammo that is firing
		cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hAmmo );
	}

	// weapon fire position (point where the bullets come from)
	cMsg.WriteLTVector( vFirePos );

	// vector pointing in the direction of travel
	cMsg.WriteLTVector( vDir );

	// random seed
	cMsg.Writeuint8( m_nRandomSeed );

	//perturb count
	cMsg.Writeuint8( m_nPerturbCount );


	// perturb (random range from true center that bullet can travel)
	cMsg.Writeuint8( static_cast< uint8 >( fPerturb * 255.0f ) );

	// time the weapon fired, in milliseconds
	uint32 nFireTime = g_pGameClientShell->GetServerRealTimeMS( );
	cMsg.Writeuint32( nFireTime );

	//which hand did the firing... needed to sync animation and FX
	cMsg.Writebool(m_bFireLeftHand);

	// send the message
	msgResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	ASSERT( LT_OK == msgResult );

#ifndef _FINAL
	if (g_vtEnableSimulationLog.GetFloat())
	{
		WriteFireMessageToSimulationLog(vFirePos, vDir, m_nRandomSeed, m_nPerturbCount, fPerturb, nFireTime);
	}
#endif

}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::SendFinishMessage()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SendFinishMessage( HOBJECT hTarget, float fImpulse, LTVector vDir, LTVector vImpactPos )
{
	CAutoMessage cMsg;
	LTRESULT msgResult;

	cMsg.Writeuint8( MID_WEAPON_FINISH );
	cMsg.WriteObject( hTarget );
	cMsg.Writefloat( fImpulse );
	cMsg.WriteLTVector( vDir );
	cMsg.WriteLTVector( vImpactPos );

	// send the message
	msgResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	ASSERT( LT_OK == msgResult );
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::SendFinishMessage()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SendRagdollFinishMessage( HOBJECT hTarget, float fDuration, float fBreakForce, LTVector vImpactPos )
{
	CAutoMessage cMsg;
	LTRESULT msgResult;

	cMsg.Writeuint8( MID_WEAPON_FINISH_RAGDOLL );
	cMsg.WriteObject( hTarget );
	cMsg.Writefloat( fDuration );
	cMsg.Writefloat( fBreakForce );
	cMsg.WriteLTVector( vImpactPos );

	// send the message
	msgResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	ASSERT( LT_OK == msgResult );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SendTriggerFireMessage()
//
//	PURPOSE:	Send fire message for TRIGGER weapons to server
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SendTriggerFireMessage( )
{
	CAutoMessage cMsg;
	LTRESULT msgResult;

	cMsg.Writeuint8( MID_WEAPON_FIRE );

	// The server knows the weapon the client is currently using so that doesn't
	// need to be sent.  Grenades and melee are handled differently, so they will be sent...

	bool bSendWeaponRecord = false;
	HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData( m_hWeapon, !USE_AI_DATA );
	if( g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bIsGrenade ) || (m_hWeapon == g_pWeaponDB->GetUnarmedRecord( )) )
		bSendWeaponRecord = true;

	cMsg.Writebool( bSendWeaponRecord );
	if( bSendWeaponRecord )
	{
		// Record of the weapon that is firing
		cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hWeapon );
	}

	// The ammo should only be required to be sent if the weapon has multiple ammo types...
	bool bSendAmmoRecord = false;
	if( g_pWeaponDB->GetNumValues( hWeaponData, WDB_WEAPON_rAmmoName ) > 1 )
		bSendAmmoRecord = true;

	cMsg.Writebool( bSendAmmoRecord );
	if( bSendAmmoRecord )
	{
		// Record of the ammo that is firing
		cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hAmmo );
	}

	// time the weapon fired, in milliseconds
	cMsg.Writeint32( g_pGameClientShell->GetServerRealTimeMS( ));

	// send the message
	msgResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	ASSERT( LT_OK == msgResult );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SendDropGrenadeMessage()
//
//	PURPOSE:	Send all relevant drop information to the server...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SendDropGrenadeMessage( float fPerturb, LTVector const &vFirePos, LTVector const &vDir )
{
	CAutoMessage cMsg;

	cMsg.Writeuint8( MID_DROP_GRENADE );

	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hWeapon );

	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hAmmo );

	// muzzle flash position
	cMsg.WriteLTVector( GetFlashPos( ));

	// weapon fire position (point where the bullets come from)
	cMsg.WriteLTVector( vFirePos );

	// vector pointing in the direction of travel
	cMsg.WriteLTVector( vDir );

	// perturb (random range from true center that bullet can travel)
	cMsg.Writeuint8( static_cast< uint8 >( fPerturb * 255.0f ) );

	// time the weapon fired, in milliseconds
	cMsg.Writeint32( g_pGameClientShell->GetServerRealTimeMS( ));

	// send the message
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetLastSndFireType
//
//	PURPOSE:	Get the last fire snd type
//
// ----------------------------------------------------------------------- //

uint8 CClientWeapon::GetLastSndFireType() const
{
	// Determine the fire snd type...
	uint8 nFireType = PSI_FIRE;

	if ( m_bHaveSilencer )
	{
		nFireType = PSI_SILENCED_FIRE;
	}

	return nFireType;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetFireVectors
//
//	PURPOSE:	Get the fire pos/rot
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::GetFireVectors( LTVector &vRight, LTVector &vUp, LTVector &vForward, LTVector &vFirePos ) const
{
	// Get the fire position / direction from the camera (so it lines
	// up correctly with the crosshairs)...
	LTRotation rRot;
	if( g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_FirstPerson )
	{
		// we're in 1st person and not using an external camera,
		// the shot is coming from the middle of the camera

		if( !m_sFireNode.empty( ))
		{
			HOBJECT hMoveObj = g_pMoveMgr->GetObject( );
			
			HMODELNODE hNull = INVALID_MODEL_NODE;
			g_pModelLT->GetNode( hMoveObj, m_sFireNode.c_str( ), hNull );

			LTTransform tNull;
			g_pModelLT->GetNodeTransform( hMoveObj, hNull, tNull, true );
			vFirePos = tNull.m_vPos;
		
			//aim where the body is aimed
			g_pLTClient->GetObjectRotation( hMoveObj, &rRot );
			//rRot = tNull.m_rRot;
		}
		else
		{
			// get the camera's position, its the fire position
			// Set the final result.
			g_pLTClient->GetObjectPos( g_pPlayerMgr->GetPlayerCamera()->GetCamera( ), &vFirePos );
			g_pLTClient->GetObjectRotation( g_pPlayerMgr->GetPlayerCamera()->GetCamera( ), &rRot );
		}

		//make corrections for grenade trajectory...
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, !USE_AI_DATA);
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
		if (PROJECTILE == (AmmoType)g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType ))
		{
			HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
			float fAngle = 0.0f;
			if (hProjectileFX)
			{
				fAngle = g_pFXDB->GetFloat(hProjectileFX,FXDB_fFireElevation);
			}

			if (fAngle > 0.0f)
			{
				float fElev = -(DEG2RAD(fAngle));
				//aim at 10m out
				rRot.Rotate(rRot.Right(),fElev);
			}

		}

		if( CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_FOCUS ) && g_iFocusObjectDetector.GetObject() )
		{
			LTVector vFocusPos = g_iFocusObjectDetector.GetObjectSpatialPosition();

			vForward	= ( vFocusPos - vFirePos ).GetUnit();
			vRight		= vForward.Cross( rRot.Up() );
			vUp			= vRight.Cross( vForward );

			vUp.Normalize();
		}
		else if (CAutoTargetMgr::Instance().IsLockedOn())
		{
			// Fire at the closest node
			vForward	= CAutoTargetMgr::Instance().GetTargetVector();
			vRight		= vForward.Cross( rRot.Up() );
			vUp			= vRight.Cross( vForward );

			vUp.Normalize();
		}
		else
		{
			// get the axis orientation of the camera
			rRot.GetVectors( vRight, vUp, vForward );
		}
	}
	else
	{
		// external camera, the shot is coming from the model
		if( m_bFireLeftHand )
		{
			g_pLTClient->GetObjectPos( m_LeftHandWeapon.m_hObject, &vFirePos );
			g_pLTClient->GetObjectRotation( m_LeftHandWeapon.m_hObject, &rRot );
			rRot.GetVectors( vRight, vUp, vForward );
		}
		else
		{
			g_pLTClient->GetObjectPos( m_RightHandWeapon.m_hObject, &vFirePos );
			g_pLTClient->GetObjectRotation( m_RightHandWeapon.m_hObject, &rRot );
			rRot.GetVectors( vRight, vUp, vForward );
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SetState()
//
//	PURPOSE:	Set our m_eState data member, and do any special tasks
//				related to entering a state.
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetState( WeaponState eNewState )
{
	if( m_eState == eNewState )
		return;

	m_eState = eNewState;

	// Debug output!
	if (ns_vtClientWeaponStateTracking.GetFloat() >= 1.0f )
	{
		const char* pName = NULL;

		if( m_hWeapon )
		{
			pName = g_pWeaponDB->GetRecordName( m_hWeapon );
		}

		switch (m_eState)
		{
		case W_INACTIVE:
			DebugCPrint(0,"CClientWeapon::SetState(W_INACTIVE) %s",pName );
			break;
		case W_IDLE:
			DebugCPrint(0,"CClientWeapon::SetState(W_IDLE) %s",pName );
			break;
		case W_FIRING:
			DebugCPrint(0,"CClientWeapon::SetState(W_FIRING) %s",pName );
			break;
		case W_FIRED:
			DebugCPrint(0,"CClientWeapon::SetState(W_FIRED) %s",pName );
			break;
		case W_ALT_FIRING:
			DebugCPrint(0,"CClientWeapon::SetState(W_ALT_FIRING) %s",pName );
			break;
		case W_GREN_THROWING:
			DebugCPrint(0,"CClientWeapon::SetState(W_GREN_THROWING) %s",pName );
			break;
		case W_RELOADING:
			DebugCPrint(0,"CClientWeapon::SetState(W_RELOADING) %s",pName );
			break;
		case W_FIRING_NOAMMO:
			DebugCPrint(0,"CClientWeapon::SetState(W_FIRING_NOAMMO) %s",pName );
			break;
		case W_SELECT:
			DebugCPrint(0,"CClientWeapon::SetState(W_SELECT) %s",pName );
			break;
		case W_DESELECT:
			DebugCPrint(0,"CClientWeapon::SetState(W_DESELECT) %s",pName );
			break;
		case W_AUTO_SWITCH:
			DebugCPrint(0,"CClientWeapon::SetState(W_AUTO_SWITCH) %s",pName );
			break;
		case W_BLOCKING:
			DebugCPrint(0,"CClientWeapon::SetState(W_BLOCKING) %s",pName );
			break;
		case W_CHECKING_AMMO:
			DebugCPrint(0,"CClientWeapon::SetState(W_CHECKING_AMMO) %s",pName );
			break;
		default :
			DebugCPrint(0,"CClientWeapon::SetState(UNKNOWN!!!) %s",pName );
		break;
		}
	}

	//if we're semi auto, and we started firing, don't fire again until we've hit some other state
	m_bSemiAutoLock = (m_bSemiAuto && ( GetState() == W_FIRING ));

	if ( GetState() == W_IDLE || GetState() == W_DESELECT )
	{
		//this is a safety net hack. there is a low repro bug where looping fire sounds can get stuck on
		//if we're idle or switching away from a weapon,
		// make sure we haven't accidentally left a looping fire sound playing...
		StopLoopSound();
	}

	if ( GetState() == W_IDLE )
		{

		// Earliest we can play a non-subtle idle ani...

		m_NextIdleTime.Start( GetNextIdlePeriod( ));
	}

	return;
}


#ifndef _FINAL

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::WriteFireMessageToSimulationLog()
//
//	PURPOSE:	Write the fire message to the simulation log.
//
// ----------------------------------------------------------------------- //

void CClientWeapon::WriteFireMessageToSimulationLog(const LTVector& vFirePos, const LTVector& vDir, const uint8 nRandomSeed, const uint8 nPerturbCount, const float fPerturb, const int32 nFireTime)
{
	CLTFileWrite& cFileWrite = g_pGameClientShell->GetSimulationLogFile();

	// timestamp
	uint32 nTimeStamp = LTTimeUtils::GetTimeMS();
	cFileWrite.Write(&nTimeStamp, sizeof(uint32));

	// action code
	uint8 nAction = eSimulationActionFire;
	cFileWrite.Write(&nAction, sizeof(uint8));

	// weapon and ammo
	//uint32 nWeaponIndex = g_pLTDatabase->GetRecordIndex( m_hWeapon );
	//uint32 nAmmoIndex = g_pLTDatabase->GetRecordIndex( m_hAmmo );
	//cFileWrite.Write(&nWeaponIndex, sizeof(uint32));
	//cFileWrite.Write(&nAmmoIndex, sizeof(uint32));


	// weapon
	const char* pName = g_pLTDatabase->GetCategoryName(g_pLTDatabase->GetRecordParent(m_hWeapon));
	uint32 nLength = LTStrLen(pName);
	cFileWrite.Write(&nLength, sizeof(uint32));
	cFileWrite.Write(pName, nLength);

	pName   = g_pLTDatabase->GetRecordName(m_hWeapon);
	nLength = LTStrLen(pName);
	cFileWrite.Write(&nLength, sizeof(uint32));
	cFileWrite.Write(pName, nLength);

	// ammo
	pName = g_pLTDatabase->GetCategoryName(g_pLTDatabase->GetRecordParent(m_hAmmo));
	nLength = LTStrLen(pName);
	cFileWrite.Write(&nLength, sizeof(uint32));
	cFileWrite.Write(pName, nLength);

	pName   = g_pLTDatabase->GetRecordName(m_hAmmo);
	nLength = LTStrLen(pName);
	cFileWrite.Write(&nLength, sizeof(uint32));
	cFileWrite.Write(pName, nLength);

	// flashpos
	LTVector vFlashPos = GetFlashPos();
	cFileWrite.Write(&vFlashPos.x, sizeof(float));
	cFileWrite.Write(&vFlashPos.y, sizeof(float));
	cFileWrite.Write(&vFlashPos.z, sizeof(float));

	// firepos
	cFileWrite.Write(&vFirePos.x, sizeof(float));
	cFileWrite.Write(&vFirePos.y, sizeof(float));
	cFileWrite.Write(&vFirePos.z, sizeof(float));

	// direction
	cFileWrite.Write(&vDir.x, sizeof(float));
	cFileWrite.Write(&vDir.y, sizeof(float));
	cFileWrite.Write(&vDir.z, sizeof(float));

	// randomseed
	cFileWrite.Write(&nRandomSeed, sizeof(uint8));

	// perturb count
	cFileWrite.Write(&nPerturbCount, sizeof(uint8));

	// perturb
	uint8 nPerturb = static_cast< uint8 >( fPerturb * 255.0f );
	cFileWrite.Write(&nPerturb, sizeof(uint8));

	// fire time
	cFileWrite.Write(&nFireTime, sizeof(int));

	// object (always 0)
	uint8 nWriteObject = 0;
	cFileWrite.Write(&nWriteObject, sizeof(uint8));
}

#endif

static bool ClientWeapon_PolyFilterFn(HPOLY hPoly, void *pUserData, const LTVector& vIntersectPoint)
{
	// Make sure we hit a surface type we care about...

	SurfaceType eSurfType = GetSurfaceType(hPoly);

	if (eSurfType == ST_INVISIBLE)
	{
        return false;
	}

    return true;
}

// Function for determining if an intersection test intersected a character
// Most of this code was swiped from CCharacterHitBox::FindHitNode
static bool CheckVectorNodeIntersect(ModelsDB::HSKELETON hModelSkeleton, HOBJECT hObject, const LTVector &vStartPos, const LTVector &vEndPos)
{
	// If they don't have a valid skeleton, consider it a hit
	if (!hModelSkeleton)
		return true;

	// Pre-calculate...
	LTVector vDir = (vEndPos - vStartPos);
	float fSegLength = vDir.Mag();
	if (fSegLength == 0.0f)
		return false;
	vDir /= fSegLength;

	// Run through the nodes looking for an intersection
	int cNodes = g_pModelsDB->GetSkeletonNumNodes(hModelSkeleton);
	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelsDB::HNODE hCurNode = g_pModelsDB->GetSkeletonNode( hModelSkeleton, iNode );

		// Get the node radius
		float fNodeRadius = g_pModelsDB->GetNodeRadius( hCurNode );

		// Don't do transforms if we don't need to
		if (fNodeRadius <= 0.0f)
		{
			continue;
		}

		// Which node are you again?
		const char* szNodeName = g_pModelsDB->GetNodeName( hCurNode );
		if( !szNodeName )
		{
			continue;
		}

		LTRESULT ltResult;
		HMODELNODE hNode;
		ltResult = g_pModelLT->GetNode(hObject, const_cast<char*>(szNodeName), hNode);
		if ( ltResult != LT_OK )
		{
			continue;
		}

		// Where are you?
        LTTransform transform;
        ltResult = g_pModelLT->GetNodeTransform(hObject, hNode, transform, true);
		if ( ltResult != LT_OK )
		{
			continue;
		}

		// Distance along ray to point of closest approach to node point

        const LTVector vRelativeNodePos = transform.m_vPos - vStartPos;
		const float fRayDist = vDir.Dot(vRelativeNodePos);

		// Make sure the projection onto the ray is within range
		if ((fRayDist < -fNodeRadius) || (fRayDist > (fNodeRadius + fSegLength)))
			continue;
	
		// Ignore the node if it wasn't within the radius of the hit spot.
		const float fDistSqr = (vDir*fRayDist - vRelativeNodePos).MagSqr();
		if( fDistSqr > fNodeRadius*fNodeRadius )
		{
			continue;
		}

		// We have a winner!
		return true;
	}

	// We didn't hit any nodes..  :(
	return false;
}

// ClientWeapon_VectorObjFilterFn parameter structure
struct CW_VOFF_Params 
{
	CW_VOFF_Params(CClientWeapon *pWeapon, const LTVector &vFirePos, const LTVector &vEndPos) :
		m_pWeapon(pWeapon),
		m_vFirePos(vFirePos),
		m_vEndPos(vEndPos)
	{
	}

	CClientWeapon *m_pWeapon;
	LTVector m_vFirePos, m_vEndPos;
};

bool ClientWeapon_VectorObjFilterFn(HOBJECT hTest, void *pUserData)
{
	// Don't hit the local player.
	HOBJECT hLocalPlayer = g_pPlayerMgr->GetMoveMgr()->GetObject( );
	if ( hTest == hLocalPlayer )
		return false;

	// Don't hit the server player.
	HOBJECT hServerPlayer = g_pLTClient->GetClientObject();
	if ( hTest == hServerPlayer )
		return false;

	// Check for a hit-box
	uint32 nObjUserFlags;
	g_pCommonLT->GetObjectFlags( hTest, OFT_User, nObjUserFlags );
	if ( (nObjUserFlags & USRFLG_HITBOX) != 0)
	{
		CW_VOFF_Params *pParams = reinterpret_cast<CW_VOFF_Params *>(pUserData);

		// Is it attached to a character?
		CCharacterFX *pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox( hTest );
		if ( pCharacter )
		{
			HOBJECT hCharacterObj = pCharacter->GetServerObj( );
			if ( hCharacterObj == hServerPlayer ) // Don't hit self
			{
				return false;
			}

			return CheckVectorNodeIntersect(pCharacter->GetModelSkeleton(), pCharacter->GetServerObj(), pParams->m_vFirePos, pParams->m_vEndPos);
		}

		// It's not a player..  Go ahead and hit it.
		return true;
	}

	// Don't hit the characters, they've got hit-boxes..
	CCharacterFX *pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(hTest);
	if ( pCharacter )
		return false;

	// Ok we should be able to hit this...
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::DoSpecialEndFire()
//
//	PURPOSE:	Do special case end of fire animation processing
//
// ----------------------------------------------------------------------- //

void CClientWeapon::DoSpecialEndFire()
{
	// Unhide any hidden pieces...
	SpecialShowPieces(true);

	if ( m_bAutoSwitchEnabled)
	{
		// Set the m_bAutoSwitch flag which will handle the ammo/weapon auto-switching...
		// NOTE, we may or may not auto-switch at this time.  
		// See the bottom of CClientWeapon::Update().

		m_bAutoSwitch = true;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::DoSpecialCreateModel()
//
//	PURPOSE:	Do special case create model processing...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::DoSpecialCreateModel()
{
	// If we're out of ammo, hide the necessary pieces...
	if ( !HasAmmo() )
	{
		// Hide the necessary pieces...
		SpecialShowPieces( false );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SpecialShowPieces()
//
//	PURPOSE:	Do special case create model processing...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SpecialShowPieces( bool bShow /*= true*/, bool bForce /*= false*/ )
{
	// If we're out of ammo, keep hidden...

	if( m_hAmmo && bShow && !bForce )
	{
		if ( g_pPlayerStats && ( g_pPlayerStats->GetAmmoCount(m_hAmmo) < 1 ) )
		{
			bShow = false; // Hide the necessary pieces...
		}
	}

	ASSERT( 0 != m_hWeapon );
	if ( !m_hWeapon )
		return;

	DisplayWeaponModelPieces(m_hWeapon, WDB_WEAPON_sHiddenPieceName, bShow);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::DisplayWeaponModelPieces()
//
//	PURPOSE:	Helper function to show or hide a list of model pieces...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::DisplayWeaponModelPieces( HWEAPON hWeapon, const char* pszWeaponPiecesAttr, bool bShow )
{
	ASSERT( 0 != g_pLTClient );
	ILTModel *pModelLT = g_pLTClient->GetModelLT();
	HMODELPIECE hPiece = 0;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	uint8 nNumHiddenPices = g_pWeaponDB->GetNumValues( hWpnData, pszWeaponPiecesAttr );
	for( uint8 nHiddenPiece = 0; nHiddenPiece < nNumHiddenPices; ++nHiddenPiece)
	{
		const char *pszHiddenPieceName = g_pWeaponDB->GetString( hWpnData, pszWeaponPiecesAttr, nHiddenPiece );
		
		if(  m_RightHandWeapon.m_hObject )
		{
			if( LT_OK == pModelLT->GetPiece( m_RightHandWeapon.m_hObject, pszHiddenPieceName, hPiece ) )
			{
				pModelLT->SetPieceHideStatus( m_RightHandWeapon.m_hObject, hPiece, !bShow );
			}
		}

		if( m_LeftHandWeapon.m_hObject )
		{
			if( LT_OK == pModelLT->GetPiece( m_LeftHandWeapon.m_hObject, pszHiddenPieceName, hPiece ) )
			{
				pModelLT->SetPieceHideStatus( m_LeftHandWeapon.m_hObject, hPiece, !bShow );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::AddImpact
//
//	PURPOSE:	Add the weapon impact
//
// ----------------------------------------------------------------------- //

void CClientWeapon::AddImpact( HLOCALOBJ hObj,LTVector const &vFrom, LTVector const &vImpactPoint,
                               LTVector const &vNormal, LTVector const &vPath, SurfaceType eType,
							   HMODELNODE hHitNode )
{
	// See if we should do tracers or not...
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo);
	HRECORD hTracerFX = g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sTracerFX, 0 );
	if( hTracerFX )
	{
		// only 1 tracer for every few shots
		m_nTracerNumber = m_nTracerNumber + 1;
		m_nTracerNumber %= g_pFXDB->GetInt32(hTracerFX,FXDB_nFrequency);
		if ( 0 != m_nTracerNumber )
		{
			m_wIgnoreFX |= WFX_TRACER;
		}
	}
	else
	{
		m_wIgnoreFX |= WFX_TRACER;
	}

	bool bFXAtFlashSocket = true;
	if( m_wIgnoreFX & WFX_MUZZLE )
	{
		bFXAtFlashSocket = false;
	}

	if( GetConsoleFloat( "LocalWeaponFX", 1.0f ))
	{
		::AddLocalImpactFX( hObj, vFrom, vImpactPoint, vNormal, eType,
			vPath, m_hWeapon, m_hAmmo, m_wIgnoreFX, bFXAtFlashSocket, hHitNode );
	}

	// If we do multiple calls to AddLocalImpact, make sure we only do some
	// effects once :)
	m_wIgnoreFX |= WFX_SILENCED | WFX_SHELL | WFX_MUZZLE | WFX_TRACER;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::KillLoopSound
//
//  PURPOSE:	Stop the looping sound from playing...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::KillLoopSound( )
{
	m_nLoopSoundId = PSI_INVALID;
	if( m_hLoopSound )
	{
		g_pLTClient->SoundMgr()->KillSound( m_hLoopSound );
		m_hLoopSound = NULL;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::StopLoopSound
//
//  PURPOSE:	Stop the looping sound from playing and notify the server...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::StopLoopSound()
{
	KillLoopSound();

	// Send message to server so all clients can stop the sound...
	// An id of invalid means stop

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_WEAPON_SOUND_LOOP );
	cMsg.Writeuint8( PSI_INVALID );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hWeapon );
	g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::RemoveFinishedKeyframedFX()
//
//  PURPOSE:	Remove all keyframed FXEdit effects that have finished playing
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::RemoveFinishedKeyframedFX()
{
	bool bAtLeastOneFXRemoved = false;

	CClientFXLinkNode*	pPrev = &m_KeyframedClientFX;

	for(CClientFXLinkNode* pNode = m_KeyframedClientFX.m_pNext; pNode != NULL; )
	{
		CClientFXLinkNode* pNext = pNode->m_pNext;

		if ( !pNode->m_Link.IsValid() || pNode->m_Link.GetInstance()->IsDone() )
		{
			//remove this node
			pPrev->m_pNext = pNode->m_pNext;
			pNode->m_pNext = NULL;
			debug_delete(pNode);

			bAtLeastOneFXRemoved = true;
		}
		else
		{
			//update our previous pointer
			pPrev = pNode;			
		}

		pNode = pNext;
	}

	return bAtLeastOneFXRemoved;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::ResetWeapon()
//
//  PURPOSE:	Reset the weapon data and empty the clip...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ResetWeapon()
{
	if( !m_hWeapon )
		return;

	// Remove all mods and attachments and get rid of the weapon model...

	Deactivate();

	ResetData();
	m_bFirstSelection = true;
	m_hAmmo = NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::SetPaused()
//
//  PURPOSE:	Pause/UnPause the weapon and do any cleanup associated with pausing/unpausing...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetPaused( bool bPaused )
{
	if( m_bPaused == bPaused )
		return;

	m_bPaused = bPaused;
	
	if( m_bPaused )
	{
		// Stop the looping sound from playing...
		StopLoopSound();
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::ClearFiring()
//
//  PURPOSE:	Stop the weapon from firing and force it into an idle animation...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ClearFiring(bool bReset)
{
	// Clear any current fire flag we previously set...

	m_bFire = false;

	//!!ARL: Sometimes we don't want to reset because it'll put us back into the
	// base pose for a split second before the server has a chance to get back to
	// us with what animation to really play (like a Deselect)

	if (bReset)
	{
		uint32 nAni = GetIdleAni();

		// Make sure it's a valid ani

		if( nAni == INVALID_ANI )
		{
			nAni = DEFAULT_ANI;
		}

		// Play it and reset the model animation...

		PlayAnimation( nAni, true );
		SetState( W_IDLE );
	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::IsAnimationDone()
//
//  PURPOSE:	Is the Current Animation done playing
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::IsAnimationDone() const
{
	uint32 dwAni	= g_pLTClient->GetModelAnimation( m_RightHandWeapon.m_hObject );
	
	if ( IsFireAni( dwAni ) && m_FireTimer.IsStarted())
	{
		return  (m_FireTimer.IsTimedOut());
	}
	else
	{
		uint32 dwState	= g_pLTClient->GetModelPlaybackState( m_RightHandWeapon.m_hObject );
		return ( dwState & MS_PLAYDONE );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::SkipModelKeys()
//
//  PURPOSE:	checks state of weapon to determine if model keys should be ignored.
//
// ----------------------------------------------------------------------- //
// 
bool CClientWeapon::SkipModelKeys() const
{
	uint32 dwAni = g_pLTClient->GetModelAnimation( m_RightHandWeapon.m_hObject );

	//if this is a fire animation
	//	and the current loop is done playing
	//  and we're not continuing to fire
	if (IsFireAni(dwAni,true) && m_FireTimer.IsTimedOut() && !(g_pPlayerMgr->GetPlayerFlags() & BC_CFLG_FIRING ) )
	{
		DebugCPrint(0,"Skipping model keys in fire ani. (%0.3f)",RealTimeTimer::Instance().GetTimerAccumulatedS());
		return true;
	}

	return false;

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::SetWeaponModelAnimation
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetWeaponModelAnimation( const char *pszAnimationName )
{
	if( !pszAnimationName || !pszAnimationName[0] )
		return;

	uint32 dwWeaponAnim = INVALID_MODEL_ANIM;

	if( m_RightHandWeapon.m_hObject )
	{
		if( g_pModelLT->GetAnimIndex( m_RightHandWeapon.m_hObject, pszAnimationName, dwWeaponAnim ) == LT_OK ) 
		{
			g_pModelLT->SetCurAnim( m_RightHandWeapon.m_hObject, MAIN_TRACKER, dwWeaponAnim, true );
			g_pModelLT->SetPlaying( m_RightHandWeapon.m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_RightHandWeapon.m_hObject, MAIN_TRACKER, !m_bSemiAuto );
		}
		else if( g_pModelLT->GetAnimIndex( 
				m_RightHandWeapon.m_hObject, 
				g_pWeaponDB->GetString( g_pWeaponDB->GetWeaponData( m_hWeapon, !USE_AI_DATA ), WDB_GLOBAL_sDefaultAnimationName ),
				dwWeaponAnim ) == LT_OK )
		{
			g_pModelLT->SetCurAnim( m_RightHandWeapon.m_hObject, MAIN_TRACKER, dwWeaponAnim, true );
			g_pModelLT->SetPlaying( m_RightHandWeapon.m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_RightHandWeapon.m_hObject, MAIN_TRACKER, !m_bSemiAuto );
		}
	}

	if( m_LeftHandWeapon.m_hObject )
	{
		if( g_pModelLT->GetAnimIndex( m_LeftHandWeapon.m_hObject, pszAnimationName, dwWeaponAnim ) == LT_OK ) 
		{
			g_pModelLT->SetCurAnim( m_LeftHandWeapon.m_hObject, MAIN_TRACKER, dwWeaponAnim, true );
			g_pModelLT->SetPlaying( m_LeftHandWeapon.m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_LeftHandWeapon.m_hObject, MAIN_TRACKER, !m_bSemiAuto );
		}
		else if( g_pModelLT->GetAnimIndex( 
			m_LeftHandWeapon.m_hObject,
			g_pWeaponDB->GetString( g_pWeaponDB->GetWeaponData( m_hWeapon, !USE_AI_DATA ), WDB_GLOBAL_sDefaultAnimationName ),
			dwWeaponAnim ) == LT_OK )
		{
			g_pModelLT->SetCurAnim( m_LeftHandWeapon.m_hObject, MAIN_TRACKER, dwWeaponAnim, true );
			g_pModelLT->SetPlaying( m_LeftHandWeapon.m_hObject, MAIN_TRACKER, true );
			g_pModelLT->SetLooping( m_LeftHandWeapon.m_hObject, MAIN_TRACKER, !m_bSemiAuto );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::SplitAmmoBetweenClips()
//
//	PURPOSE:	Correctly distributes the new ammount of ammo between the 
//				left and right clips for dual and sinlge weapons...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SplitAmmoBetweenClips( )
{
	// Make sure there is ammo to distribute...
	if( m_nNewAmmoInClip <= 0 )
		return;

	bool bHasRightClip	= !!m_RightHandWeapon.m_hObject;
	bool bHasLeftClip	= !!m_LeftHandWeapon.m_hObject;

	if( bHasRightClip && bHasLeftClip )
	{
		// Split the ammo between the two...

		float fSplit = m_nNewAmmoInClip * 0.5f;

		m_RightHandWeapon.m_nAmmoInClip = (int32)fSplit;
		m_LeftHandWeapon.m_nAmmoInClip = (int32)fSplit;

		// If there is an uneven amount give the right weapon one more...
		if( m_nNewAmmoInClip % 2 != 0 )
			m_RightHandWeapon.m_nAmmoInClip += 1;
	}
	else if( bHasRightClip )
	{
		// All ammo goes to right weapon clip...
		m_RightHandWeapon.m_nAmmoInClip = m_nNewAmmoInClip;
		m_LeftHandWeapon.m_nAmmoInClip = 0;
	}
	else if( bHasLeftClip )
	{
		// All ammo goes to left weapon clip...
		m_LeftHandWeapon.m_nAmmoInClip = m_nNewAmmoInClip;
		m_RightHandWeapon.m_nAmmoInClip = 0;
	}

}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::DropGrenade()
//
//	PURPOSE:	Drop a live grenade right at the players position. (Used on player death)
//
// ----------------------------------------------------------------------- //

void CClientWeapon::DropGrenade( )
{
	LTASSERT( 0 != m_hWeapon, "Trying to fire with an invalid weapon record." );
	LTASSERT( 0 != m_hAmmo, "Trying to fire with an invalid ammo record." );

	// Make sure we're actually a grenade...
	HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData( m_hWeapon, !USE_AI_DATA );
	if( !g_pWeaponDB->GetBool( hWeaponData, WDB_WEAPON_bIsGrenade ))
		return;

	// perturb based on player's movement/firing state
	float fPerturb = CAccuracyMgr::Instance().GetCurrentPerturb();

	// get the player's aim modifier
	float fPerturbX = GetSkillValue(eAimAccuracy);

	// factor the modifier into the perturb
	fPerturb *= fPerturbX;

	// fire position/direction information
	LTVector vU, vR, vF, vFirePos;

	// Get the fire pos/rot
	if ( !GetFireVectors( vR, vU, vF, vFirePos ) )
	{
		return;
	}

	// Make sure we always ignore the fire sounds...
	m_wIgnoreFX = WFX_FIRESOUND | WFX_ALTFIRESND;

	if ( !m_bHaveSilencer )
	{
		m_wIgnoreFX |= WFX_SILENCED;
	}

	// send a fire message to the server
	SendDropGrenadeMessage( fPerturb, vFirePos, vF );
}


bool CClientWeapon::WeaponPath_OnImpactCB( CWeaponPath::COnImpactCBData &rImpactData, void *pUserData )
{
	if( !pUserData )
		return false;

	CClientWeapon *pClientWeapon = reinterpret_cast<CClientWeapon*>(pUserData);
	pClientWeapon->AddImpact( rImpactData.m_hObjectHit, rImpactData.m_vFirePos, rImpactData.m_vImpactPos,
							  rImpactData.m_vImpactNormal, rImpactData.m_vDir, rImpactData.m_eSurfaceType,
							  rImpactData.m_hHitNode );

	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CWeaponModelData
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CWeaponModelData::~CWeaponModelData
//
//	PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CWeaponModelData::~CWeaponModelData( )
{
	if (m_pDisplay)
	{
		debug_delete(m_pDisplay);
		m_pDisplay = NULL;
	}

	if( m_pPersistentClientFX )
	{
		debug_deletea( m_pPersistentClientFX );
		m_pPersistentClientFX = NULL;
	}

	m_nNumPersistentClientFX = 0;

	if( m_pbLoadPersistentFXVisible )
	{
		debug_deletea( m_pbLoadPersistentFXVisible );
		m_pbLoadPersistentFXVisible = NULL;
	}

	Term( );
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CWeaponModelData::PopulateCreateStruct()
//
//	PURPOSE:	Used to fill in information realating to the weapon model...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::PopulateCreateStruct( ObjectCreateStruct &rOCS ) const
{
	HATTRIBUTE hAttrib = NULL;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_hWeaponRecord, !USE_AI_DATA );
	
	if( g_vtPlayerBodyWeapons.GetFloat() >= 1.0f )
	{
		hAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_sHHModel );
		rOCS.SetFileName( g_pWeaponDB->GetString( hAttrib ));

		hAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_fHHScale );
		rOCS.m_Scale = g_pWeaponDB->GetFloat( hAttrib );
	}

	const char *pszWeaponModelStruct = g_pLTDatabase->GetAttributeName( m_hWeaponModelStruct );
	if( LTStrEmpty( pszWeaponModelStruct ))
		return;

	// Get all specified materials...
	char szCompleteName[256];
	LTSNPrintF( szCompleteName, LTARRAYSIZE( szCompleteName ), "%s.0.%s", pszWeaponModelStruct, WDB_WEAPON_sHHMaterial );
	g_pWeaponDB->CopyStringValues( hWpnData, szCompleteName, rOCS.m_Materials[0],
								   LTARRAYSIZE(rOCS.m_Materials), LTARRAYSIZE(rOCS.m_Materials[0]) );

}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CWeaponModelData::Init()
//
//	PURPOSE:	pre-create display
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::Init( HWEAPON hWeapon, HATTRIBUTE hWeaponModelStruct )
{
	if( !hWeapon || !hWeaponModelStruct )
	{
		LTERROR( "Failed to initialize weapon model." );	
		return;
	}

	m_hWeaponRecord = hWeapon;
	m_hWeaponModelStruct = hWeaponModelStruct;
	

	HATTRIBUTE hAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_rCustomDisplay );
	HRECORD hDisplay = g_pWeaponDB->GetRecordLink(hAttrib,0);
	if (hDisplay)
	{
		m_pDisplay = WeaponDisplay::CreateDisplay(this,hDisplay);
	}

	// Allocate the bank of PersistentFX...
	if( !m_pPersistentClientFX )
	{
		HATTRIBUTE hPersistentFXAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_aPersistentClientFX );
		m_nNumPersistentClientFX = g_pLTDatabase->GetNumValues( hPersistentFXAttrib );

		m_pPersistentClientFX = debug_newa( CPersistentClientFX, m_nNumPersistentClientFX );
	}
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CWeaponModelData::Term()
//
//	PURPOSE:	Remove weapon models and other objects...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::Term( )
{
	RemoveMuzzleFlash( );
	RemoveScope( );
	RemoveSilencer( );
	RemoveWeaponModel( );
	ShutdownPersistentClientFX( );
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CWeaponModelData::SetVisibility
//
//	PURPOSE:	Set the visibility of the weapon model and any attached mods...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::SetVisibility(bool bVisible, bool bShadow  )
{
	if( m_hObject )
	{
		g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, (bVisible ? FLAG_VISIBLE : 0), FLAG_VISIBLE );
	}

	if (bShadow)
	{
		if (bVisible)
		{
			EEngineLOD eShadowLOD = eEngineLOD_Never;
			g_pLTClient->GetObjectShadowLOD( CPlayerBodyMgr::Instance( ).GetObject( ), eShadowLOD );
			g_pLTClient->SetObjectShadowLOD( m_hObject, eShadowLOD );
		}
		else
		{
			g_pLTClient->SetObjectShadowLOD(m_hObject, eEngineLOD_Never);
		}
	}

	// Always hide the flash (it will be shown when needed)...
	if( m_MuzzleFlashFX.IsValid( ))
	{
		m_MuzzleFlashFX.GetInstance( )->Hide( );
	}

	SetSilencerVisibility( bVisible, bShadow );
	SetScopeVisibility( bVisible, bShadow );
	SetPersistentClientFXVisibility( bVisible );
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CWeaponModelData::CreateSilencer
//
//	PURPOSE:	Create the silencer model object...
//
// ----------------------------------------------------------------------- //

bool CWeaponModelData::CreateSilencer( HMOD hSilencer )
{
	// Start without a silencer...
	m_hSilencerSocket = INVALID_MODEL_SOCKET;

	if(	!hSilencer ||
		!g_pWeaponDB->GetNumValues( hSilencer, WDB_MOD_sSocket ) ||
		!g_pPlayerStats->HaveMod( hSilencer ) )
	{
		if( m_hSilencerModel )
			RemoveSilencer( );
		
		return false;
	}

	// Make sure we have a socket for the silencer...
	if( m_hObject )
	{
		const char *pszSocket = g_pWeaponDB->GetString( hSilencer, WDB_MOD_sSocket );
		if( g_pModelLT->GetSocket( m_hObject, pszSocket, m_hSilencerSocket ) != LT_OK )
		{
			if( m_hSilencerModel )
				RemoveSilencer( );

			return false;
		}

		// Don't try to create the model if none was specified...
		char const* pszAttachModel = g_pWeaponDB->GetString( hSilencer, WDB_MOD_sAttachModel );
		if( pszAttachModel[0] )
		{
			// Okay create/setup the model...
			ObjectCreateStruct ocs;

			// copy the model filename
			ocs.SetFileName( g_pWeaponDB->GetString( hSilencer, WDB_MOD_sAttachModel ));

			// add the material names
			g_pWeaponDB->CopyStringValues( hSilencer, WDB_MOD_sAttachMaterial,
				ocs.m_Materials[0], LTARRAYSIZE(ocs.m_Materials),
				LTARRAYSIZE( ocs.m_Materials[0] ));

			// create the model
			m_hSilencerModel = CreateModelObject( m_hSilencerModel, ocs );
			if( !m_hSilencerModel )
				return false;

			// make it visible
			g_pCommonLT->SetObjectFlags( m_hSilencerModel, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

			float fScale = g_pWeaponDB->GetFloat( hSilencer, WDB_MOD_fAttachScale );
			g_pLTClient->SetObjectScale( m_hSilencerModel, fScale );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CWeaponModelData::RemoveSilencer()
//
//	PURPOSE:	Remove the silencer model object...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::RemoveSilencer( )
{
	if( m_hSilencerModel )
	{
		g_pLTClient->RemoveObject( m_hSilencerModel );
		m_hSilencerModel  = NULL;
		m_hSilencerSocket = INVALID_MODEL_SOCKET;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::UpdateSilencer
//
//	PURPOSE:	Update the silencer model...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::UpdateSilencer( )
{
	// Update the silencer transform...
	if( m_hSilencerModel && m_hSilencerSocket )
	{
		LTTransform tSilencerTansform;
		if( g_pModelLT->GetSocketTransform( m_hObject, m_hSilencerSocket, tSilencerTansform, true ) == LT_OK )
		{
			g_pLTClient->SetObjectTransform( m_hSilencerModel, tSilencerTansform );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::SetSilencerVisibility
//
//	PURPOSE:	Update the visibility of the silencer model...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::SetSilencerVisibility( bool bVisible, bool bShadow )
{
	if( m_hSilencerModel )
	{
		g_pCommonLT->SetObjectFlags( m_hSilencerModel, OFT_Flags, (bVisible ? FLAG_VISIBLE : 0), FLAG_VISIBLE );
	}

	if (bShadow)
	{
		if (bVisible)
		{
			EEngineLOD eShadowLOD = eEngineLOD_Never;
			g_pLTClient->GetObjectShadowLOD( CPlayerBodyMgr::Instance( ).GetObject( ), eShadowLOD );
			g_pLTClient->SetObjectShadowLOD( m_hSilencerModel, eShadowLOD );
		}
		else
		{
			g_pLTClient->SetObjectShadowLOD(m_hSilencerModel, eEngineLOD_Never);
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::CreateScope
//
//	PURPOSE:	Create the scope model object...
//
// ----------------------------------------------------------------------- //

bool CWeaponModelData::CreateScope( HMOD hScope )
{
	// Start without a scope...
	m_hScopeSocket = INVALID_MODEL_SOCKET;

	if(	!hScope ||
		!g_pWeaponDB->GetNumValues( hScope, WDB_MOD_sSocket ) ||
		!g_pPlayerStats->HaveMod( hScope ) )
	{
		if( m_hScopeModel )
			RemoveScope( );

		return false;
	}

	// Make sure we have a socket for the scope...
	if( m_hObject )
	{
		const char *pszSocket = g_pWeaponDB->GetString( hScope, WDB_MOD_sSocket );
		if( g_pModelLT->GetSocket( m_hObject, pszSocket, m_hScopeSocket ) != LT_OK )
		{
			if( m_hScopeModel )
				RemoveScope( );

			return false;
		}

		// Don't try to create the model if none was specified...
		char const* pszAttachModel = g_pWeaponDB->GetString( hScope, WDB_MOD_sAttachModel );
		if( pszAttachModel[0] )
		{
			// Okay create/setup the model...
			ObjectCreateStruct ocs;

			// copy the model filename
			ocs.SetFileName( g_pWeaponDB->GetString( hScope, WDB_MOD_sAttachModel ));

			// add the material names
			g_pWeaponDB->CopyStringValues( hScope, WDB_MOD_sAttachMaterial,
				ocs.m_Materials[0], LTARRAYSIZE(ocs.m_Materials),
				LTARRAYSIZE( ocs.m_Materials[0] ));

			// create the model
			m_hScopeModel = CreateModelObject( m_hScopeModel, ocs );

			if ( m_hScopeModel )
			{
				// make it visible
				g_pCommonLT->SetObjectFlags( m_hScopeModel, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

				float fScale = g_pWeaponDB->GetFloat( hScope, WDB_MOD_fAttachScale );
				g_pLTClient->SetObjectScale( m_hScopeModel, fScale );
			}
		}
	}
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CWeaponModelData::RemoveScope()
//
//	PURPOSE:	Remove the scope model object...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::RemoveScope( )
{
	if( m_hScopeModel )
	{
		g_pLTClient->RemoveObject( m_hScopeModel );
		m_hScopeModel  = NULL;
		m_hScopeSocket = INVALID_MODEL_SOCKET;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::UpdateScope
//
//	PURPOSE:	Update the scope model...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::UpdateScope( )
{
	// Update the scope transform...
	if( m_hScopeModel && m_hScopeSocket )
	{
		LTTransform tScopeTansform;
		if( g_pModelLT->GetSocketTransform( m_hObject, m_hScopeSocket, tScopeTansform, true ) == LT_OK )
		{
			g_pLTClient->SetObjectTransform( m_hScopeModel, tScopeTansform );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::SetScopeVisibility
//
//	PURPOSE:	Update the visibility of the scope model...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::SetScopeVisibility(bool bVisible, bool bShadow  )
{
	if( m_hScopeModel )
	{
		g_pCommonLT->SetObjectFlags( m_hScopeModel, OFT_Flags, (bVisible ? FLAG_VISIBLE : 0), FLAG_VISIBLE );
	}

	if (bShadow)
	{
		if (bVisible)
		{
			EEngineLOD eShadowLOD = eEngineLOD_Never;
			g_pLTClient->GetObjectShadowLOD( CPlayerBodyMgr::Instance( ).GetObject( ), eShadowLOD );
			g_pLTClient->SetObjectShadowLOD( m_hScopeModel, eShadowLOD );
		}
		else
		{
			g_pLTClient->SetObjectShadowLOD(m_hScopeModel, eEngineLOD_Never);
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::CreateModelObject
//
//	PURPOSE:	Create a model object
//
// ----------------------------------------------------------------------- //

HOBJECT CWeaponModelData::CreateModelObject( HOBJECT hOldObj, ObjectCreateStruct &rOCS )
{
	HOBJECT hObj = hOldObj;

	if( !hObj )
	{
		if( !rOCS.m_Filename[0] )
			return NULL;

		rOCS.m_ObjectType = OT_MODEL;
		rOCS.m_Flags     |= 0;

		hObj = g_pLTClient->CreateObject( &rOCS );
		if( !hObj )
			return NULL;

		// Have new object use the player's timer.
		ObjectContextTimer( g_pMoveMgr->GetServerObject( )).ApplyTimerToObject( hObj );
	}
	else
	{
		if ( LT_OK != g_pCommonLT->SetObjectFilenames( hObj, &rOCS ) )
		{
			return NULL;
		}
	}

	// we want to be notified for model keys
	LTRESULT ltResult = g_pCommonLT->SetObjectFlags( hObj, OFT_Client, CF_NOTIFYMODELKEYS, CF_NOTIFYMODELKEYS );
	ASSERT( LT_OK == ltResult );

	// Reset the model animation...
	g_pModelLT->SetLooping( hObj, MAIN_TRACKER, false );
	g_pModelLT->SetPlaying( hObj, MAIN_TRACKER, false );

	// Set the shadow for HH weapons...
	EEngineLOD eShadowLOD = eEngineLOD_Never;
	g_pLTClient->GetObjectShadowLOD( CPlayerBodyMgr::Instance( ).GetObject( ), eShadowLOD );
	g_pLTClient->SetObjectShadowLOD( hObj, eShadowLOD );

	// Scale the model if necessary...
	// PLAYER_BODY
	if( g_vtPlayerBodyWeapons.GetFloat() < 1.0f )
	{
		if (ns_vtPVModelScale.GetFloat() != 1.0f)
		{
			g_pLTClient->SetObjectScale(hObj, ns_vtPVModelScale.GetFloat());
		}

		// No shadows for pv weapons...
		g_pLTClient->SetObjectShadowLOD( hObj, eEngineLOD_Never );

	}

	return hObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::RemoveWeaponModel
//
//	PURPOSE:	Remove the model used as the weapon...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::RemoveWeaponModel( )
{
	TFlashlightData::iterator iCurFlashlight = m_aFlashlights.begin();
	for (; iCurFlashlight != m_aFlashlights.end(); ++iCurFlashlight)
	{
		debug_delete( iCurFlashlight->light );
	}
	m_aFlashlights.clear();

	if( m_hObject )
	{
		g_pLTClient->RemoveObject( m_hObject );
		m_hObject = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::CreateWeaponModel
//
//	PURPOSE:	Create the model used as the weapon...
//
// ----------------------------------------------------------------------- //

bool CWeaponModelData::CreateWeaponModel( const char *pszSocket )
{
	if( m_hObject )
	{
		LTERROR( "Trying to create a weapon model that already exists." );
		return false;
	}

	if( !m_hWeaponRecord )
	{
		LTERROR( "Trying to create a weapon model without a weapon record." );
		return false;
	}

	if( !m_hWeaponModelStruct )
	{
		LTERROR( "Trying to create a weapon model without a weapon model struct attribute." );
		return false;
	}

	ObjectCreateStruct ocs;
	PopulateCreateStruct( ocs );
	
	m_hObject = CreateModelObject( m_hObject, ocs );
	if( !m_hObject )
	{
		if (m_pDisplay)
		{
			m_pDisplay->Term();
			debug_delete(m_pDisplay);
			m_pDisplay = NULL;
		}

		return false;
	}

	HOBJECT hPlayerBody = CPlayerBodyMgr::Instance( ).GetObject( );
	
	// Cache the hand socket used for positioning the weapon model...
	m_hHandSocket = INVALID_MODEL_SOCKET;
	if( g_pModelLT->GetSocket( hPlayerBody, pszSocket, m_hHandSocket ) != LT_OK )
	{
		LTERROR( "Failed to find socket on player model." );
		return false;
	}

	HATTRIBUTE hAttrib = NULL;
	
	// Setup Breach socket (if it exists)...
	m_hBreachSocket = INVALID_MODEL_SOCKET;
	hAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_sBreachSocket );
	if( g_pModelLT->GetSocket( m_hObject, g_pWeaponDB->GetString( hAttrib ), m_hBreachSocket ) != LT_OK )
	{
		m_hBreachSocket = INVALID_MODEL_SOCKET;
	}

	// Setup the Flash socket (if it exists)...
	m_hMuzzleSocket = INVALID_MODEL_SOCKET;
	hAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_sMuzzleSocket );
	if( g_pModelLT->GetSocket( m_hObject, g_pWeaponDB->GetString( hAttrib ), m_hMuzzleSocket ) != LT_OK )
	{
		m_hMuzzleSocket = INVALID_MODEL_SOCKET;
	}

	hAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_fHHScale );
	g_pLTClient->SetObjectScale( m_hObject, g_pWeaponDB->GetFloat( hAttrib ));

	// Initialize the offests now so they can be modified (through dev cheats) later...
	// And so we don't access the database every frame for positioning the weapon :)

	hAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_vPos );
	m_vWeaponOffset	= g_pWeaponDB->GetVector3( hAttrib );

	if (m_pDisplay)
	{
		m_pDisplay->Reset();
	}


	hAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_rFlashlights );
	uint32 nFlashlights = g_pWeaponDB->GetNumValues( hAttrib );
	for (uint32 nFlashlight=0; nFlashlight<nFlashlights; nFlashlight++)
	{
		HRECORD rFlashlight = g_pWeaponDB->GetRecordLink(hAttrib, nFlashlight);
		if (!rFlashlight)
		{
			continue;
		}

		int i=m_aFlashlights.size();
		m_aFlashlights.resize(i+1);

		m_aFlashlights[i].light = debug_new( Flashlight ); LTASSERT( m_aFlashlights[i].light, "Failed to allocate weapon flashlight!" );
		m_aFlashlights[i].light->Initialize(m_hObject, rFlashlight);

		LTStrCpy(m_aFlashlights[i].cachedName, g_pLTDatabase->GetRecordName(rFlashlight), LTARRAYSIZE(m_aFlashlights[i].cachedName));
	}
	// update the tokens afterward since we're resizing our arrays above and they'll get moved around in memory (assumes flashlights don't get added after this point).
	TFlashlightData::iterator iCurFlashlight = m_aFlashlights.begin();
	for (; iCurFlashlight != m_aFlashlights.end(); ++iCurFlashlight)
	{
		iCurFlashlight->tok = CParsedMsg::CToken(iCurFlashlight->cachedName);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::UpdateWeaponPosition
//
//	PURPOSE:	Move the weapon model to the appropriate socket...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::UpdateWeaponPosition( )
{
	if( !m_hObject || (m_hHandSocket == INVALID_MODEL_SOCKET) )
		return;

	HOBJECT hPlayerBody = CPlayerBodyMgr::Instance( ).GetObject();

	LTTransform tHand;
	if( g_pModelLT->GetSocketTransform( hPlayerBody, m_hHandSocket, tHand, true ) == LT_OK )
	{
		g_pLTClient->SetObjectTransform( m_hObject, LTRigidTransform( tHand.m_vPos, tHand.m_rRot ));
	}

	float fElapsedS = ObjectContextTimer( g_pMoveMgr->GetServerObject() ).GetTimerElapsedS();
	TFlashlightData::iterator iCurFlashlight = m_aFlashlights.begin();
	for (; iCurFlashlight != m_aFlashlights.end(); ++iCurFlashlight)
	{
		iCurFlashlight->light->Update(fElapsedS);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::CreateMuzzleFlash
//
//	PURPOSE:	Create the muzzle flash
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::CreateMuzzleFlash( )
{
	if( !m_hWeaponRecord || !m_hObject )
		return;

	// Remove the old FX Instance
	RemoveMuzzleFlash( );

	// If our FX Instance is not created do so now...
	if( !m_MuzzleFlashFX.IsValid() )
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_hWeaponRecord, !USE_AI_DATA );
		const char *pszMuzzleFxName = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sPVMuzzleFX );
		
		if( pszMuzzleFxName && pszMuzzleFxName[0] )
		{ 
			// This is always the player view FX
			CLIENTFX_CREATESTRUCT fxInit( pszMuzzleFxName, FXFLAG_LOOP, m_hObject ); 
			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_MuzzleFlashFX, fxInit, false );
			if( m_MuzzleFlashFX.IsValid() ) 
			{
				m_MuzzleFlashFX.GetInstance()->Hide();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::RemoveMuzzleFlash
//
//	PURPOSE:	Destroys the muzzle flash
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::RemoveMuzzleFlash()
{
	if( m_MuzzleFlashFX.IsValid() )
	{
		// Shut it down gracefully...
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_MuzzleFlashFX );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::StartFlash()
//
//	PURPOSE:	Start the muzzle flash
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::StartMuzzleFlash()
{
	if( m_MuzzleFlashFX.IsValid() )
	{
		m_FlashTime.Start( m_MuzzleFlashFX.GetInstance()->m_fDuration );
	}

	// Spawn a "fire and forget" muzzleflash (if one is specified).
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_hWeaponRecord, !USE_AI_DATA );
	const char *pszMuzzleFxName = g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sPVMuzzleFX2 );

	if( pszMuzzleFxName && pszMuzzleFxName[0] )
	{ 
		CLIENTFX_CREATESTRUCT fxInit( pszMuzzleFxName, 0, m_hObject ); 
		if( m_hMuzzleSocket != INVALID_MODEL_SOCKET )
			fxInit.m_hSocket = m_hMuzzleSocket;
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::UpdateMuzzleFlash()
//
//	PURPOSE:	Update muzzle flash state
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::UpdateMuzzleFlash( bool bVisible )
{
	// Always update the flash position even if the FX is invalid...
	UpdateMuzzleFlashPos( );

	if( !m_hObject || !m_MuzzleFlashFX.IsValid( ))
		return;

	// get the object flags
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, dwFlags );

	if(	m_FlashTime.IsTimedOut( ) ||
		(g_pPlayerMgr->GetPlayerState() != ePlayerState_Alive) ||
		!(dwFlags & FLAG_VISIBLE) ||
		!bVisible )
	{
		m_MuzzleFlashFX.GetInstance()->Hide();
	}
	else
	{
		m_MuzzleFlashFX.GetInstance()->Show();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::UpdateMuzzleFlashPos()
//
//	PURPOSE:	Update muzzle flash position
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::UpdateMuzzleFlashPos( )
{	
	// Get the Flash socket position and cache that pos
	if( m_hMuzzleSocket != INVALID_MODEL_SOCKET )
	{
		LTTransform transform;
		if( g_pModelLT->GetSocketTransform( m_hObject, m_hMuzzleSocket, transform, true ) == LT_OK)
		{
			m_vFlashPos = transform.m_vPos;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::CreatePersistentClientFX
//
//	PURPOSE:	Create the Persistent ClientFX for the weapon model...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::CreatePersistentClientFX( )
{
	if(	!m_hWeaponRecord || !m_hObject || !m_pPersistentClientFX )
		return;

	// Destroy all old persistent ClientFX...
	ShutdownPersistentClientFX( );

	HATTRIBUTE hPersistentFXAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_aPersistentClientFX );
	HATTRIBUTE hAttrib = NULL;

	// Read each individual client fx...
	for( uint32 nFX = 0; nFX < m_nNumPersistentClientFX; ++nFX )
	{
		hAttrib = g_pWeaponDB->GetStructAttribute( hPersistentFXAttrib, nFX, WDB_WEAPON_sClientFX );
		const char *pszClientFX = g_pWeaponDB->GetString( hAttrib );

		if( !LTStrEmpty( pszClientFX ))
		{
			CPersistentClientFX *pPersistentFX = &m_pPersistentClientFX[nFX];

			uint32 dwFlags = FXFLAG_LOOP;

			// BJL - Persistent effects are deleted when the player changes 
			// weapons; this is the only time smoothshutdown is applied. 
			// Unfortunately, before deleting the effect, we hide it. Hiding 
			// the effect causes the effect to become suspended. After it is
			// hidden, we shut it down. 
			//
			// As the effect is suspended, it no longer is updated. As it is 
			// no longer updated, we don't query for the effect having ended.
			// This results in these effects 'leaking' -- nothing will 
			// unsuspend them, and they will never be deleted as they are 
			// waiting to be shut down. This manifested itself with polytrails
			// immediately before beta. We should reexamine this system and 
			// determine how to handle smoothshutdown if needed.

			#pragma MESSAGE("SmoothShutdown on persistent ClientFX is disabled (Suspended/SmoothShutdown conflicts)")
//			hAttrib = g_pWeaponDB->GetStructAttribute( hPersistentFXAttrib, nFX, WDB_WEAPON_bSmoothShutdown );
//			if( !g_pWeaponDB->GetBool( hAttrib ))
//				dwFlags |= FXFLAG_NOSMOOTHSHUTDOWN;
			
			CLIENTFX_CREATESTRUCT fxInit( pszClientFX, dwFlags, m_hObject );
			if( g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &pPersistentFX->m_ClientFXLink, fxInit, true ) &&
				pPersistentFX->m_ClientFXLink.IsValid( ))
			{
				// Determine if the FX should be visible on start up...
				hAttrib = g_pWeaponDB->GetStructAttribute( hPersistentFXAttrib, nFX, WDB_WEAPON_bVisible );
				if( m_pbLoadPersistentFXVisible )
				{
					if( m_pbLoadPersistentFXVisible[nFX] )
					{
						pPersistentFX->m_ClientFXLink.GetInstance( )->Show( );
						pPersistentFX->m_bVisible = true;
					}
					else
					{
						pPersistentFX->m_ClientFXLink.GetInstance( )->Hide( );
						pPersistentFX->m_bVisible = false;
					}

					debug_deletea( m_pbLoadPersistentFXVisible );
					m_pbLoadPersistentFXVisible = NULL;
				}
				else if( g_pWeaponDB->GetBool( hAttrib ) )
				{
					pPersistentFX->m_ClientFXLink.GetInstance( )->Show( );
					pPersistentFX->m_bVisible = true;
				}
				else
				{
					pPersistentFX->m_ClientFXLink.GetInstance( )->Hide( );
					pPersistentFX->m_bVisible = false;
				}
			}
		}
	}
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::ShutdownPersistentClientFX
//
//	PURPOSE:	Destroys the Persistent ClientFX for the weapon model...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::ShutdownPersistentClientFX( )
{
	if( !m_pPersistentClientFX )
		return;

	// Run through each FX and shutdown each one...
	for( uint32 nFX = 0; nFX < m_nNumPersistentClientFX; ++nFX  )
	{
		CPersistentClientFX *pPersistentFX = &m_pPersistentClientFX[nFX];
		if( pPersistentFX && pPersistentFX->m_ClientFXLink.IsValid( ))
		{
			g_pGameClientShell->GetSimulationTimeClientFXMgr( ).ShutdownClientFX( &pPersistentFX->m_ClientFXLink );
			
			// Reset the visibility so the FX will be properly recreated...
			pPersistentFX->m_bVisible = false;
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::ShowPersistentClientFX
//
//	PURPOSE:	Set the specified persistent FX visible...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::ShowPersistentClientFX( uint32 nFX )
{
	if( nFX > m_nNumPersistentClientFX || m_nNumPersistentClientFX == 0 || !m_pPersistentClientFX )
		return;

	m_pPersistentClientFX[nFX].m_ClientFXLink.GetInstance( )->Show( );
	m_pPersistentClientFX[nFX].m_bVisible = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::HidePersistentClientFX
//
//	PURPOSE:	Set the specified persistent FX invisible...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::HidePersistentClientFX( uint32 nFX )
{
	if( nFX > m_nNumPersistentClientFX || m_nNumPersistentClientFX == 0 || !m_pPersistentClientFX )
		return;

	m_pPersistentClientFX[nFX].m_ClientFXLink.GetInstance( )->Hide( );
	m_pPersistentClientFX[nFX].m_bVisible = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::SetPersistentClientFXVisibility
//
//	PURPOSE:	Show/Hide all of the persistent FX...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::SetPersistentClientFXVisibility( bool bVisible )
{
	if( !m_pPersistentClientFX )
		return;

	// Run through each FX and set the visibility of each one...
	for( uint32 nFX = 0; nFX < m_nNumPersistentClientFX; ++nFX )
	{
		CPersistentClientFX *pPersistentFX = &m_pPersistentClientFX[nFX];
		if( pPersistentFX && pPersistentFX->m_ClientFXLink.IsValid( ))
		{
			// Hide/Show the ClientFX on a global scope.
			// Don't change the individual visible flag since that specifies if the FX
			// should be visible when the global visibility is true...
			if( bVisible && pPersistentFX->m_bVisible )
			{
				pPersistentFX->m_ClientFXLink.GetInstance( )->Show( );
			}
			else
			{
				pPersistentFX->m_ClientFXLink.GetInstance( )->Hide( );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::GetBreachTransform()
//
//	PURPOSE:	Reset some members...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::GetBreachTransform( LTRigidTransform &rBreachTransform )
{
    if( m_hObject && (m_hBreachSocket != INVALID_MODEL_SOCKET) )
	{
		LTTransform trans;
		if( g_pModelLT->GetSocketTransform( m_hObject, m_hBreachSocket, trans, true ) == LT_OK )
		{
			rBreachTransform.Init( trans.m_vPos, trans.m_rRot );		
			return;
		}
	}

	// Failed to get the breach socket transform... use the object transform?
	g_pLTClient->GetObjectTransform( m_hObject, &rBreachTransform );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::ResetData()
//
//	PURPOSE:	Reset some members...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::ResetData( )
{
	m_FlashTime.Stop( );

	m_nAmmoInClip = 0;
	m_nNumFiresInARow = 0;

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::CanFire()
//
//	PURPOSE:	Reset some members...
//
// ----------------------------------------------------------------------- //

bool CWeaponModelData::CanFire( )
{
	// Check for valid weapon model and check ammo amount...
	if( !m_hObject || (m_nAmmoInClip <= 0) )
		return false;

	HATTRIBUTE hAttrib = NULL;
	hAttrib = g_pWeaponDB->GetStructAttribute( m_hWeaponModelStruct, 0, WDB_WEAPON_nMaxFireFrequency );
	
	// Check allowed frequency...
	int32 nMaxFireFrequency = g_pWeaponDB->GetInt32( hAttrib );

	// No frequency specified so just allow the weapon to fire...
	if( nMaxFireFrequency <= 0 )
		return true;

	// If we haven't hit the max this weapon can fire...
	if( m_nNumFiresInARow < nMaxFireFrequency )
		return true;

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::Save()
//
//	PURPOSE:	Save object...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	pMsg->Writeint32( m_nAmmoInClip );

	bool bCurrentWeapon = g_pPlayerStats->GetCurrentWeaponRecord( ) == m_hWeaponRecord;
	pMsg->Writebool( bCurrentWeapon );

	if( bCurrentWeapon )
	{
		// Save the visibility of all persistent FX so they will be created with the
		// proper visibility on load...
		pMsg->Writeint32( m_nNumPersistentClientFX );

		for( uint32 nFX = 0; nFX < m_nNumPersistentClientFX; ++nFX )
		{
			pMsg->Writebool( m_pPersistentClientFX[nFX].m_bVisible );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponModelData::Load()
//
//	PURPOSE:	Load object...
//
// ----------------------------------------------------------------------- //

void CWeaponModelData::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	m_nAmmoInClip = pMsg->Readint32( );

	// If this was the current weapon, load the visibility of the persistent FX...
	if( pMsg->Readbool( ))
	{
		m_nNumPersistentClientFX = pMsg->Readuint32( );
	
		// Allocate the visibility on load flags...
		if( !m_pbLoadPersistentFXVisible && m_nNumPersistentClientFX > 0 )
			m_pbLoadPersistentFXVisible = debug_newa( bool, m_nNumPersistentClientFX );
		
		// Load the visibility of each persistent client FX...
		for( uint32 nFX = 0; nFX < m_nNumPersistentClientFX; ++nFX )
		{
			m_pbLoadPersistentFXVisible[nFX] = pMsg->Readbool( );
		}
	}
}

// EOF
