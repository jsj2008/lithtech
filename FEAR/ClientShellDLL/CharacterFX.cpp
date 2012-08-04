// ----------------------------------------------------------------------- 
//
// MODULE  : CharacterFX.cpp
//
// PURPOSE : Character special FX - Implementation
//
// CREATED : 8/24/98
//
// (c) 1998-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "CharacterFX.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "SoundMgr.h"
#include "iltphysics.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"
#include "MsgIDs.h"
#include "VolumeBrushFX.h"
#include "CMoveMgr.h"
#include "VehicleMgr.h"
#include "ClientWeaponMgr.h"
#include "PlayerBodyMgr.h"
#include "HUDMgr.h"
#include "HUDNavMarker.h"
#include "NavMarkerTypeDB.h"
#include "HUDSubtitles.h"
#include "HUDNavMarkerMgr.h"
#include "HUDDialogue.h"
#include "sys/win/mpstrconv.h"
#include "PlayerCamera.h"
#include "SoundDB.h"
#include "ClientDB.h"
#include "ModelsDB.h"
#include "EngineTimer.h"
#include "PhysicsUtilities.h"
#include "LadderMgr.h"
#include "LadderFX.h"
#include "SpecialMoveMgr.h"
#include "GameModeMgr.h"
#include "CollisionsDB.h"
#include <algorithm>
#include "PropsDB.h"
#include "ForensicObjectFX.h"
#include "ltfileoperations.h"
#include "BroadcastDB.h"
#include "ClientMeleeCollisionController.h"
#include "LTEulerAngles.h"
#include "StateMachine.h"
#include "PolyGridFX.h"

static const float kFadeOutTime = 3.0f;

extern LTVector g_vPlayerCameraOffset;

#define INTERSECT_Y_OFFSET		50.0f
#define FOOTSTEP_SOUND_RADIUS	1000.0f
#define ALL_ATTACH_FX			-1

#define CHARACTERFX_KEY_FOOTSTEP		"FOOTSTEP_KEY"	// FOOTSTEP_KEY foot
#define CHARACTERFX_KEY_SOUNDBUTE		"BUTE_SOUND_KEY"// BUTE_SOUND_KEY soundbute
#define CHARACTERFX_KEY_CHARACTERSOUND	"CHARACTER_SOUND"
#define	CHARACTERFX_KEY_MOVE_LOUD		"MOVE_LOUD_KEY"
#define CHARACTERFX_KEY_MOVE_QUIET		"MOVE_QUIET_KEY"
#define CHARACTERFX_KEY_SHOW_ATTACHFX	"SHOW_ATTACHFX" // SHOW_ATTACHFX n
#define CHARACTERFX_KEY_HIDE_ATTACHFX	"HIDE_ATTACHFX" // HIDE_ATTACHFX n
#define CHARACTERFX_KEY_FX				"FX"			// HIDE_ATTACHFX n
#define CHARACTERFX_KEY_MATERIAL		"MATERIAL"		// MATERIAL n Name
#define CHARACTERFX_KEY_NOISE			"NOISE"
#define CHARACTERFX_KEY_DECAL			"DECAL"			// Create decal fx
#define CHARACTERFX_KEY_BLOCKWINDOW		"BLOCKWINDOW"
#define CHARACTERFX_KEY_DODGEWINDOW		"DODGEWINDOW"
#define CHARACTERFX_KEY_MELEECONTROL	"MELEE"

const char * const NMDB_TeamType = "Team";

#define SUBTITLE_STRINGID_OFFSET	0

SurfaceType g_eClientLastSurfaceType = ST_UNKNOWN;

//VarTrack g_vtBreathTime;
VarTrack g_vtModelKey;
VarTrack g_vtFootPrintBlend;
VarTrack g_vtDialogueCinematicSoundRadius;
VarTrack g_vtDialogueCinematicSoundInnerRadius;
VarTrack g_vtDingDelay;
VarTrack g_vtPlayerAimTracking;
VarTrack g_vtQuietMovementVolumeFactor;
VarTrack g_vtWallStickObjectForwardOffset;
VarTrack g_vtBodyLifetime;

CCharacterFX::CharFXList CCharacterFX::m_lstCharacters;

// Focus object detector
ObjectDetector		g_iFocusObjectDetector;
ObjectDetector		g_iAttackPredictionObjectDetector;

int32 CCharacterFX::m_nBroadcastPriority = -1;
float CCharacterFX::m_fBroadcastOuterRadius = -1.0f;
float CCharacterFX::m_fBroadcastInnerRadius = -1.0f;

LTVector2n g_vNameSz(128,32);



//
// RotationInterpolator
//
// Statemachine for controlling the interpolation of pitch
// and lean of non-local characters in multiplayer.
// This is needed because only Yaw is actually applied
// to the players using the object rotations.  Interpolation of yaw
// is handled by the engine, but not the pitch and leaning.
// Pitch and leaning come in through the Unguaranteed extra data.
//
class RotationInterpolator : public MacroStateMachine
{
public:

	RotationInterpolator( )
	{
		m_pCharacterFx = NULL;

		m_nTargetPitchRoll = 0;
		m_fStartTargetTime = 0.0f;
		m_fEndTargetTime = 0.0f;

		m_nNewPitchRoll = 0;
		m_fNewTime = 0.0f;

		m_nStartPitchRoll = 0;
		m_fStartRotateTime = 0.0f;
	}

	bool Init( CCharacterFX& characterFx );

private:

	enum EState
	{
		// State used when waiting not interpolating toward a rotation
		// and waiting for a new one.
		EState_Idle,
		// State object used when interpolating toward a target rotation.
		EState_Playing,
	};

	// Called to get the latest rotation if it has chnaged.
	void UpdateNewPitchRoll( )
	{
		// Check if the pitch/roll changed.
		uint32 nPitchRoll = 0;
		if( g_pLTBase->GetObjectUnguaranteedData( m_pCharacterFx->GetServerObj( ), &nPitchRoll ) != LT_OK )
			return;
		if( nPitchRoll != m_nNewPitchRoll)
		{
			// Record it as a new point in the future.
			m_nNewPitchRoll = nPitchRoll;
			// Take a fixed amount of time to get there.
			m_fNewTime = ( RealTimeTimer::Instance().GetTimerAccumulatedS( ) + m_vtPitchRollInterpTime.GetFloat( 0.1f ));
		}
	}

	bool Idle_OnUpdate( MacroStateMachine::EventParams& eventParams )
	{
		// Poll for new rotations.
		UpdateNewPitchRoll( );

		// Check if a new rotation came in.
		if( m_nNewPitchRoll == m_nTargetPitchRoll )
			return true;;

		// Switch to interpolating toward the target rotation.
		SetState( EState_Playing );

		return true;
	}

	bool Playing_OnEnter( MacroStateMachine::EventParams& eventParams )
	{
		// Record the target information.
		double fCurTime = RealTimeTimer::Instance().GetTimerAccumulatedS();
		m_nStartPitchRoll = m_nTargetPitchRoll;
		m_nTargetPitchRoll = m_nNewPitchRoll;
		m_fStartTargetTime = fCurTime;
		m_fEndTargetTime = m_fNewTime;
		m_fStartRotateTime = fCurTime;
		return true;
	}

	bool Playing_OnUpdate( MacroStateMachine::EventParams& eventParams );

	// Statemachine event handlers.
	MSM_BeginTable( RotationInterpolator )
		MSM_BeginState( EState_Idle )
			MSM_OnUpdate( Idle_OnUpdate )
		MSM_EndState( )
		MSM_BeginState( EState_Playing )
			MSM_OnEnter( Playing_OnEnter )
			MSM_OnUpdate( Playing_OnUpdate )
		MSM_EndState( )
	MSM_EndTable( )
			
private:

	CCharacterFX*		m_pCharacterFx;

	// The target rotation we are currently interpolating toward.
	uint32				m_nTargetPitchRoll;
	double				m_fStartTargetTime;
	double				m_fEndTargetTime;

	// A new point that is beyond the time of our current target rotation.
	uint32				m_nNewPitchRoll;
	double				m_fNewTime;

	// Information of when we started interpolating toward target.
	uint32				m_nStartPitchRoll;
	double				m_fStartRotateTime;

	// Console variable controlling the interpolation time.
	static VarTrack		m_vtPitchRollInterpTime;
};

bool RotationInterpolator::Playing_OnUpdate( MacroStateMachine::EventParams& eventParams )
{
	// Poll for new rotations.
	UpdateNewPitchRoll( );

	// Get parameterized value to interpolate to target rotation.
	double fCurTime = RealTimeTimer::Instance().GetTimerAccumulatedS();
	float fT = (float)(( fCurTime - m_fStartRotateTime ) / ( m_fEndTargetTime - m_fStartTargetTime ));

	// Check if we've reached or passed the target.
	bool bEndPlaying = false;
	if( fT >= 1.0f )
	{
		// If there's a new rotation past the target, then switch the target to the new rotation.
		// Re-evaluate the interpolatant parameter to the new target.
		if( m_nTargetPitchRoll != m_nNewPitchRoll )
		{
			m_fStartRotateTime = m_fEndTargetTime;
			m_nStartPitchRoll = m_nTargetPitchRoll;
			m_nTargetPitchRoll = m_nNewPitchRoll;
			m_fStartTargetTime = m_fEndTargetTime;
			m_fEndTargetTime = m_fNewTime;
			fT = (float)(( fCurTime - m_fStartRotateTime) / ( m_fEndTargetTime - m_fStartTargetTime));
		}
		// No new point, just go to the target.
		else
		{
			bEndPlaying = true;
			fT = 1.0f;
		}
	}

	// Get the interpolated pitch roll.
	LTVector2 v2Target;
	UncompressPitchRoll16( m_nTargetPitchRoll, v2Target );
	LTVector2 v2Start;
	UncompressPitchRoll16( m_nStartPitchRoll, v2Start );
	LTVector2 v2Interp = v2Start.Lerp( v2Target, fT );

	// Get the yaw from the character.
	LTRotation rCharacterFx;
	g_pLTBase->GetObjectRotation( m_pCharacterFx->GetServerObj(), &rCharacterFx );
	EulerAngles eaCharacterFx = Eul_FromQuat( rCharacterFx, EulOrdYXZr );

	// Combine the eulers into a rotation for pitch tracking.
	LTRotation rRot( v2Interp.x, eaCharacterFx.x, 0.0f );
	LTVector vPos;
	g_pLTClient->GetObjectPos( m_pCharacterFx->GetServerObj(), &vPos );
	m_pCharacterFx->GetNodeTrackerContext( ).SetTrackedTarget( kTrackerGroup_AimAt, vPos + (rRot.Forward() * 10000) );

	// Lean is the roll amount.
	m_pCharacterFx->GetLeanNodeController( ).SetLeanAngle( v2Interp.y );

	bool bEnable = g_vtPlayerAimTracking.GetFloat( ) >= 1.0f;
	m_pCharacterFx->EnablePlayerAimTracking( bEnable );	

	// Check if we reached the end of the changes.  Which means go back to idling.
	if( bEndPlaying )
	{
		SetState( EState_Idle );
	}

	return true;
}

VarTrack RotationInterpolator::m_vtPitchRollInterpTime;

bool RotationInterpolator::Init( CCharacterFX& characterFx )
{
	m_pCharacterFx = &characterFx;
	m_nTargetPitchRoll = 0;
	m_nNewPitchRoll = 0;

	if( GetState( ) == EState_Idle )
		return false;

	SetState( EState_Idle );

	// Initialize the global interp time.
	if( !m_vtPitchRollInterpTime.IsInitted( ))
	{
		m_vtPitchRollInterpTime.Init( g_pLTBase, "PitchRollInterpTime", NULL, 0.1f );
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Init
//
//	PURPOSE:	Init the character fx
//
// ----------------------------------------------------------------------- //

bool CCharacterFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::Init(hServObj, pMsg)) return false;
	if (!pMsg) return false;

	m_bSeverBody = false;
	m_hDeathFX = NULL;

	CHARCREATESTRUCT ch;

	ch.hServerObj = hServObj;
	ch.Read(pMsg);

	return Init(&ch);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Init
//
//	PURPOSE:	Init the character fx
//
// ----------------------------------------------------------------------- //

bool CCharacterFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return false;
	m_cs = *((CHARCREATESTRUCT*)psfxCreateStruct);

	// Register with some detectors
	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	bool bIsLocalClient = ( hPlayerObj == m_hServerObject );

	if( !bIsLocalClient )
	{
		g_iFocusObjectDetector.RegisterObject( m_iFocusLink, m_hServerObject, this );
		g_iAttackPredictionObjectDetector.RegisterObject( m_iAttackPredictionLink, m_hServerObject, this );
	}

	// Setup our mp model.
	if( m_cs.bIsPlayer && IsMultiplayerGameClient() )
	{
		ChangeMPModel( m_cs.nMPModelIndex );
	}

	// Init the node controller if we have a model.
	if ( GetModel( ) && !m_NodeController.Init(this) )
	{
		return false;
	}

	// Init some console vars that are reguired...
//	if (!g_vtBreathTime.IsInitted())
//	{
//		g_vtBreathTime.Init( g_pLTClient, "BreathTime", NULL, 5.0f);
//	}

	if (!g_vtModelKey.IsInitted())
	{
		g_vtModelKey.Init( g_pLTClient, "ModelKey", NULL, 0.0f);
	}

	if (!g_vtFootPrintBlend.IsInitted())
	{
		g_vtFootPrintBlend.Init( g_pLTClient, "FootPrintBlendMode", NULL, 2.0f);
	}

	if (!g_vtDialogueCinematicSoundRadius.IsInitted())
	{
		g_vtDialogueCinematicSoundRadius.Init( g_pLTClient, "DialogueCinematicSndRadius", NULL, 10000.0f);
	}
	if (!g_vtDialogueCinematicSoundInnerRadius.IsInitted())
	{
		g_vtDialogueCinematicSoundInnerRadius.Init( g_pLTClient, "DialogueCinematicSndRadius", NULL, g_vtDialogueCinematicSoundRadius.GetFloat() * 0.25f );
	}

	if (!g_vtDingDelay.IsInitted())
	{
		g_vtDingDelay.Init( g_pLTClient, "DingDelay", NULL, 0.5f);
	}

	if( !g_vtPlayerAimTracking.IsInitted() )
	{
		g_vtPlayerAimTracking.Init(g_pLTClient, "PlayerAimTracking", NULL, 1.0f);
	}

	if( !g_vtQuietMovementVolumeFactor.IsInitted() )
	{
		g_vtQuietMovementVolumeFactor.Init( g_pLTClient, "QuietMovementVolumeFactor", NULL, 0.65f );
	}

	if( !g_vtWallStickObjectForwardOffset.IsInitted( ))
	{
		g_vtWallStickObjectForwardOffset.Init( g_pLTClient, "WallStickObjectForwardOffset", NULL, -10.0f );
	}

	if( !g_vtBodyLifetime.IsInitted( ))
	{
		// Init bodylifetime to -1, infinite.
		g_vtBodyLifetime.Init( g_pLTClient, "BodyLifetime", NULL, -1.0f );
	}

	// Init our 3rd person flashlight fx (i.e., what other players see)...
	m_3rdPersonFlashlight.Initialize( m_hServerObject, "CharacterFX" );
	HOBJECT hLocalObj = g_pLTClient->GetClientObject();

	// Only do pitch node tracking if it's a non-local player in a multiplayer game...
   	if( m_cs.bIsPlayer && IsMultiplayerGameClient() )
	{
		if (m_hServerObject != hLocalObj)
		{
			// create the chat icon
			const char* const pszChatFXName = ClientDB::Instance().GetString(ClientDB::Instance().GetClientSharedRecord(), "ChatFX");
			CLIENTFX_CREATESTRUCT fxInit( pszChatFXName, FXFLAG_LOOP, m_hServerObject );

			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_linkChatFX, fxInit, true );
			if ( m_linkChatFX.IsValid() ) 
			{
				// start out hidden
				m_linkChatFX.GetInstance()->Hide();
			}

			m_PlayerRigidBody.Init( m_hServerObject );
		}
	}

	// Initially setup the node tracker context...
	if( GetModel( ))
	{
		ResetNodeTrackerContext();
		ResetNodeBlinkContext();
	}

	// Size the damageFX vector to the number of damagefx we have...

	//make sure that we don't have any old arrays lying around
	debug_deletea(m_p3rdPersonDamageFX);
	m_p3rdPersonDamageFX = NULL;
	m_nNum3rdPersonDamageFX = 0;

	if( g_pDamageFXMgr )
	{
		m_p3rdPersonDamageFX = debug_newa(CClientFXLink, g_pDamageFXMgr->GetNumDamageFX() );
		if(m_p3rdPersonDamageFX)
			m_nNum3rdPersonDamageFX = g_pDamageFXMgr->GetNumDamageFX();
	}

	// Setup our hitbox.
	if( m_cs.bUseDefaultHitboxDims )
	{
		g_pPhysicsLT->GetObjectDims( m_hServerObject, &m_cs.vHitBoxDims );
		m_cs.vHitBoxDims *= HB_DIMS_ENLARGE_PERCENT;
	}
	m_HitBox.Init( m_hServerObject, m_cs.vHitBoxDims, m_cs.vHitBoxOffset );

	UpdateAttachmentVisibility( m_hServerObject );

	m_nUniqueDialogueId = 0;
	m_bSubtitlePriority	= false;

	// Add this to the list of characters in game...
	m_lstCharacters.push_back( this );

	m_CharacterPhysics.Init( m_hServerObject );

	m_MeleeCollisionController.Init( m_hServerObject );

	if (m_fBroadcastOuterRadius < 0.0f)
	{
		m_fBroadcastOuterRadius = DATABASE_CATEGORY( BroadcastGlobal ).GETRECORDATTRIB( DATABASE_CATEGORY( BroadcastGlobal ).GetGlobalRecord() , SoundOuterRadius );
		m_fBroadcastInnerRadius = DATABASE_CATEGORY( BroadcastGlobal ).GETRECORDATTRIB( DATABASE_CATEGORY( BroadcastGlobal ).GetGlobalRecord() , SoundInnerRadius );
	}

	//make sure the body is hidden if it was severed and we are set to low-violence
	if (m_bSeverBody && !g_pProfileMgr->GetCurrentProfile()->m_bGore)
	{
		g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
		g_pLTClient->SetObjectShadowLOD( m_hServerObject, eEngineLOD_Never );
		m_bUpdateAttachments = true;
	}

	if (m_cs.HasSlowMoRecharge())
	{
		if( m_linkLoopFX.IsValid() )
		{
			g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkLoopFX );
		}

		HRECORD hGlobalRec = g_pWeaponDB->GetGlobalRecord();
		CLIENTFX_CREATESTRUCT fxInit( g_pWeaponDB->GetString(hGlobalRec,WDB_GLOBAL_sSlowMoRechargerFXName), FXFLAG_LOOP, GetLocalContextObject());
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_linkLoopFX, fxInit, true );
		
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::~CCharacterFX()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCharacterFX::~CCharacterFX()
{
	if( m_pRotationInterpolator )
	{
		delete m_pRotationInterpolator;
		m_pRotationInterpolator = NULL;
	}

	RemoveUnderwaterFX();
	RemoveFlashLightFX();
	RemoveAttachClientFX();
	
	if (m_hDialogueSnd)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hDialogueSnd);
	}

	if (m_hDialogueRadioSnd)
	{
		g_pLTClient->SoundMgr()->KillSound(m_hDialogueRadioSnd);
	}

	KillWeaponLoopSound();
	KillSlideSound();

	// Shutdown any damageFX...
	
	ShutdownDamageFX();
	RemoveSlowMoFX();

	//free all the 3rd person damage effects
	debug_deletea(m_p3rdPersonDamageFX);
	m_p3rdPersonDamageFX = NULL;
	
	if( m_linkChatFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkChatFX );
	}

	if( m_linkLoopFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkLoopFX );
	}

	CharFXList::iterator iter = std::find( m_lstCharacters.begin(), m_lstCharacters.end( ), this );
	if( iter != m_lstCharacters.end() )
	{
		m_lstCharacters.erase( iter );
	}

	if (m_pHUDItem)
	{
		debug_delete(m_pHUDItem);
		m_pHUDItem = NULL;
		m_hHUDType = NULL;
	}

	
	//remove any of our parts that we severed...
	ObjRefVector::iterator orvIt = m_hSeveredParts.begin();
	while (orvIt != m_hSeveredParts.end())
	{
		g_pLTClient->RemoveObject(*orvIt);
		orvIt++;
	}
	
	if( m_hWallStickObject )
	{
		g_pLTClient->RemoveObject( m_hWallStickObject );
		m_hWallStickObject = NULL;
	}

	if (m_pNameDisplay)
	{
		m_pNameDisplay->Term();
		debug_delete(m_pNameDisplay);
	}
	if (m_pInsigniaDisplay)
	{
		m_pInsigniaDisplay->Term();
		debug_delete(m_pInsigniaDisplay);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

bool CCharacterFX::CreateObject(ILTClient* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return false;

	//initialize our breath information
//	m_fBreathElapsedTime = 0.0f;
//	m_fBreathEndTime = g_vtBreathTime.GetFloat();
	
	// NOTE: Since we only use node control for the mouth now, we can safely use CF_INSIDERADIUS
	g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Client, CF_NOTIFYMODELKEYS, CF_NOTIFYMODELKEYS);

	// Set up MoveMgr's point to us, if applicable...

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();

	// Create a child timer for this object off the simulation timer.
	EngineTimer engineTimer;
	engineTimer.CreateChildTimer( SimulationTimer::Instance( ));
	engineTimer.ApplyTimerToObject( m_hServerObject );
	engineTimer.SetTimerTimeScale( m_cs.nTimeScaleNumerator, m_cs.nTimeScaleDenominator );

	// Check for local player.
	if( hPlayerObj == m_hServerObject)
	{
		InitLocalPlayer();
	}
	// Not local player.
	else 
	{
		// If this isn't a player, then make them solid.  Player's solid flag is controlled
		// by server.
		if( !m_cs.bIsPlayer )
		{
			uint32 dwFlags = FLAG_SOLID | (IsMultiplayerGameClient() ? FLAG_GRAVITY : 0);
			g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, dwFlags, dwFlags);

			if( g_pLTClient->PhysicsSim()->SetObjectPhysicsGroup( m_hServerObject, PhysicsUtilities::ePhysicsGroup_UserAI ) != LT_OK )
			{
				LTERROR( "Failed to set the physics group for AI characterfx." );
			}
		}

		if( IsMultiplayerGameClient())
		{
			g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags2, FLAG2_PLAYERSTAIRSTEP, FLAG2_PLAYERSTAIRSTEP);

		}
	}

	// Give us a rotation interpolator.
	if( IsMultiplayerGameClient())
	{
		if( !m_pRotationInterpolator )
		{
			m_pRotationInterpolator = new RotationInterpolator;
		}
		m_pRotationInterpolator->Init( *this );
	}

	m_nFontHeight = 16;
	m_Str.SetFont(CFontInfo(g_pLayoutDB->GetHUDFont(),m_nFontHeight));
	m_Str.SetColor(argbWhite);
	m_Str.SetAlignment(kCenter);
	m_Str.SetDropShadow(1);

	// create and show all client effects associated with this model
	CreateAttachClientFX();
	//ShowAttachClientFX();


	//create team HUD Markers in MP games for players other than the local client...
	if (GameModeMgr::Instance( ).m_grbUseTeams && m_cs.bIsPlayer && hPlayerObj != m_hServerObject)
	{
		if (!m_pHUDItem)
		{
			m_pHUDItem = debug_new(CHUDNavMarker);
			m_pHUDItem->SetScaling(true);
			m_hHUDType = g_pNavMarkerTypeDB->GetRecord(NMDB_TeamType);
			if ( g_pNavMarkerMgr )
			{
				g_pNavMarkerMgr->AddMarker(m_pHUDItem);
			}
		}
		UpdateHUDData();
	}


	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateCharacterModel
//
//	PURPOSE:	Change the character model if needed...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateCharacterModel( )
{
	if( IsMultiplayerGameClient( ))
	{
		if( !IsUsingMPModel( m_cs.nMPModelIndex ))
		{
			ChangeMPModel( m_cs.nMPModelIndex );
		}

		// Make sure the displays are setup.
		if( !m_pNameDisplay )
			InitDisplays();

		if (m_pNameDisplay)
		{
			m_pNameDisplay->SetMaterial();
		}
		if (m_pInsigniaDisplay)
		{
			m_pInsigniaDisplay->SetMaterial();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Update
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

bool CCharacterFX::Update()
{
	if (m_bWantRemove)
	{
		if ( g_pNavMarkerMgr )
		{
			g_pNavMarkerMgr->RemoveMarker(m_pHUDItem);
		}
		debug_delete(m_pHUDItem);
		m_pHUDItem = NULL;
	}

	if (!m_pClientDE || !m_hServerObject || m_bWantRemove) return false;


	UpdateCharacterModel( );


	HLOCALOBJ	hPlayerObj = g_pLTClient->GetClientObject();
	bool		bLocal = (hPlayerObj == m_hServerObject);

	// Update our hitbox...
	// We need to do this here incase of the early out when the server object is inactive. 
	// The hitbox should always be in the correct position.
	// Only need to update the hitbox if it's for someone else.
	if( !bLocal )
	{
		m_HitBox.Update();
	}

	// if this is a player, set the dims to the dims associated with the lower tracker
	if (m_cs.bIsPlayer)
	{
		uint32 nFlags;
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_Flags2, nFlags);

		if ((nFlags & FLAG2_SERVERDIMS) == 0)
		{
			HMODELANIM hAnim = INVALID_MODEL_ANIM;
			if( g_pModelLT->GetCurAnim(m_hServerObject, kAD_TRK_Lower, hAnim) == LT_OK && (hAnim != INVALID_MODEL_ANIM) )
			{
				LTVector vDims;
				if( g_pModelLT->GetModelAnimUserDims(m_hServerObject, hAnim, &vDims) == LT_OK )
					g_pPhysicsLT->SetObjectDims(m_hServerObject, &vDims, SETDIMS_PUSHOBJECTS);
			}
		}
	}

	// See if our server side object is active
	uint32 dwUserFlags(0);
	g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

	if ( !(dwUserFlags & USRFLG_GAMEBASE_ACTIVE) )
	{
		return true;
	}

	// Make us solid if our ai usrflg solid is set

	uint32 dwFlags(0);

	if ( dwUserFlags & USRFLG_AI_CLIENT_SOLID )
	{
		dwFlags |= FLAG_SOLID;
	}
	else if ( m_cs.bIsPlayer )
	{
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_Flags, dwFlags);
	}

	g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, dwFlags, FLAG_SOLID);

	// See if this character is the target of our follow mode.
	if( g_pPlayerMgr->GetSpectatorMode() == eSpectatorMode_Follow && g_pPlayerMgr->GetSpectatorFollowTarget( ) == m_hServerObject )
	{

		// Make sure our model is invisible if we're the target of a follow spectator.
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_Flags, dwFlags);
		if( dwFlags & FLAG_VISIBLE )
		{
			g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
			m_bUpdateAttachments = true;
		}
	}

	//make sure the body is hidden if it was severed and we are set to low-violence
	if (m_bSeverBody && !g_pProfileMgr->GetCurrentProfile()->m_bGore)
	{
		g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
		m_bUpdateAttachments = true;
	}


	// We need to update our physics solid flag.  Since we are set to ~FLAG_SOLID,
	// the engine automatically sets us to physics non-solid.
	uint32 nNumRigidBodies = 0;
	g_pLTBase->PhysicsSim( )->GetNumModelRigidBodies( m_hServerObject, nNumRigidBodies );
	for( uint32 nIndex = 0; nIndex < nNumRigidBodies; nIndex++ )
	{
		HPHYSICSRIGIDBODY hRigidBody;
		if (LT_OK == g_pLTBase->PhysicsSim( )->GetModelRigidBody( m_hServerObject, nIndex, hRigidBody ))
		{
			g_pLTBase->PhysicsSim( )->SetRigidBodySolid( hRigidBody, true );
			g_pLTBase->PhysicsSim( )->ReleaseRigidBody(hRigidBody);
		}
	}

	// Update

	uint32 dwUsrFlags;
	g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUsrFlags);

	if (dwUsrFlags & USRFLG_PLAYER_UNDERWATER)
	{
		UpdateUnderwaterFX();
	}
	else
	{
		RemoveUnderwaterFX();
	}

	// Update the character damage FX.  1st and 3rd person.
	UpdateDamageFX();

	// Update node tracker.
	if ( !m_cs.bIsDead )
	{
		// Update the aiming position of the player...
		UpdatePlayerAimPosition();

		// Update the node controllers.
	 	m_BlinkController.UpdateNodeBlink();
		m_NodeTrackerContext.UpdateNodeTrackers();
	}

	// Update various FX
	if ( m_cs.byFXFlags & CHARCREATESTRUCT::eFlashLight )
	{
		UpdateFlashLightFX();
	}
	else
	{
		RemoveFlashLightFX();
	}

	// Update node controller...

	m_NodeController.Update();


	// Update our sounds...

	UpdateSounds();

	if (!m_Str.IsEmpty())
	{
		LTVector vPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

		m_vPos = vPos;
		m_vPos.y += 64.0f;

		bool bOnScreen = false;
		LTVector pos = g_pInterfaceMgr->GetScreenFromWorldPos(m_vPos, g_pPlayerMgr->GetPlayerCamera()->GetCamera(),bOnScreen);

		float fMaxRange = GetConsoleFloat("MaxAIDebugRange",2000.0f);
		if (bOnScreen && pos.z > 0.0f && pos.z <= fMaxRange)
		{
			
			uint8 h = (uint8)(g_pInterfaceResMgr->GetXRatio() * (18.0f - (12.0f * pos.z / fMaxRange) )   );

			if (h != m_nFontHeight)
			{
				m_nFontHeight = h;
				m_Str.SetFontHeight(m_nFontHeight);
			}
			
			m_bStrVisible = true;
			m_Str.SetPos( LTVector2(pos.x, pos.y) );
		}
		else
		{
			m_bStrVisible = false;

		}
	}

	if( IsMultiplayerGameClient( ) && m_cs.bIsPlayer && !bLocal )
	{
		LTVector vPos;
		g_pLTClient->GetObjectPos( m_hServerObject, &vPos );
		m_PlayerRigidBody.Update( vPos );
	}

	if (m_cs.bIsDead)
		UpdateDead();

	if (m_bUpdateAttachments)
		UpdateAttachments();

	// Update any character specific physics...
	m_CharacterPhysics.Update( );

	if( m_hWallStickObject )
	{
		HPHYSICSRIGIDBODY hWallStickRigidBody = m_CharacterPhysics.GetWallStickRigidBody( );
		if( hWallStickRigidBody )
		{
			LTRigidTransform tWallStickTrans;
			g_pLTClient->PhysicsSim( )->GetRigidBodyTransform( hWallStickRigidBody, tWallStickTrans );
			
			LTRotation rWallStickObject;
			g_pLTClient->GetObjectRotation( m_hWallStickObject, &rWallStickObject );

			// Offset the object so it looks like its embedded in the wall...
			tWallStickTrans.m_vPos += rWallStickObject.Forward( ) * g_vtWallStickObjectForwardOffset.GetFloat( );
			g_pLTClient->SetObjectPos( m_hWallStickObject, tWallStickTrans.m_vPos );
		}
	}

	// Update our melee controller...
	m_MeleeCollisionController.Update();

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateDamageFX
//
//	PURPOSE:	Update our damage fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateDamageFX()
{
	if( !m_hServerObject )
		return;
		
	HLOCALOBJ	hPlayerObj = g_pLTClient->GetClientObject();
	bool		bLocal = (hPlayerObj == m_hServerObject);

	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) return;
	

	// No need to go further if our damageflags haven't changed...
	if( (m_nLastDamageFlags == m_cs.nDamageFlags) && (m_nInstantDamageFlags == 0) && !m_bWasPlayingSpecialDamageAni )
	{
		return;
	}

	m_nLastDamageFlags = m_cs.nDamageFlags;

	DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstDamageFX();
	while( pDamageFX )
	{
		// Test the damage flags against the DamageFX...

		if( m_nLastDamageFlags & pDamageFX->m_nDamageFlag || pDamageFX->m_vtTestFX.GetFloat() > 0.0f )
		{
			// Start this DamageFX if necessary

			if( bLocal )
			{
				// First person DamageFX for the local object...

				pDamageFX->Start();
			}
			else
			{
				// Play the 3rd person FX if this is not the local clients characterfx...

				if( pDamageFX->m_sz3rdPersonFXName[0] && 
					(pDamageFX->m_nID < m_nNum3rdPersonDamageFX) &&
					!m_p3rdPersonDamageFX[pDamageFX->m_nID].IsValid() )
				{
					CLIENTFX_CREATESTRUCT	fxInit( pDamageFX->m_sz3rdPersonFXName, FXFLAG_LOOP, GetLocalContextObject()); 
					g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(&m_p3rdPersonDamageFX[pDamageFX->m_nID], fxInit, true );

				}
			}
		}
		else
		{
			// Stop any current DamageFX
			if( bLocal )
			{
				pDamageFX->Stop();
			}
			else
			{
				if( (pDamageFX->m_nID < m_nNum3rdPersonDamageFX) && m_p3rdPersonDamageFX[pDamageFX->m_nID].IsValid() )
				{
					g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_p3rdPersonDamageFX[pDamageFX->m_nID] );
				}
			}
		}	

		// Check if we are taking any instant damage...

		if( m_nInstantDamageFlags & pDamageFX->m_nDamageFlag )
		{
			if( bLocal && pDamageFX->m_sz1stPersonInstFXName[0] )
			{
				if( !m_link1stPersonInstFX.IsValid() )
				{
					m_psz1stPersonInstFXName = pDamageFX->m_sz1stPersonInstFXName;

					CLIENTFX_CREATESTRUCT fxInit( m_psz1stPersonInstFXName, 0, GetLocalContextObject());
					g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_link1stPersonInstFX, fxInit, true );
				}
				else if( m_psz1stPersonInstFXName == pDamageFX->m_sz1stPersonInstFXName )
				{
					if( pDamageFX->m_bRemoveOnNextInstantDamage )
					{
						g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_link1stPersonInstFX );
					}
					else if( pDamageFX->m_bRenewOnNextInstantDamage )
					{
						g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_link1stPersonInstFX );

						m_psz1stPersonInstFXName = pDamageFX->m_sz1stPersonInstFXName;

						CLIENTFX_CREATESTRUCT fxInit( m_psz1stPersonInstFXName, 0, GetLocalContextObject() );
						g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_link1stPersonInstFX, fxInit, true );
					}
				}
			}
			if(!bLocal && pDamageFX->m_sz3rdPersonInstFXName[0] )
			{
				if( !m_link3rdPersonInstFX.IsValid() )
				{
					m_psz3rdPersonInstFXName = pDamageFX->m_sz3rdPersonInstFXName;

					CLIENTFX_CREATESTRUCT fxInit( m_psz3rdPersonInstFXName, 0, GetLocalContextObject());
					g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_link3rdPersonInstFX, fxInit, true );
				}
				else if( m_psz3rdPersonInstFXName == pDamageFX->m_sz3rdPersonInstFXName )
				{
					if( pDamageFX->m_bRemoveOnNextInstantDamage )
					{
						g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_link3rdPersonInstFX );
					}
					else if( pDamageFX->m_bRenewOnNextInstantDamage )
					{
						g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_link3rdPersonInstFX );

						m_psz3rdPersonInstFXName = pDamageFX->m_sz3rdPersonInstFXName;

						CLIENTFX_CREATESTRUCT fxInit( m_psz3rdPersonInstFXName, 0, GetLocalContextObject());
						g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_link3rdPersonInstFX, fxInit, true );

					}
				}
			}
		}

		pDamageFX = g_pDamageFXMgr->GetNextDamageFX();
	}

	// Clear the instant damage...
	
	m_nInstantDamageFlags = 0;
	m_bWasPlayingSpecialDamageAni = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleDialogueMsg
//
//	PURPOSE:	Start/Stop a dialogue sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleDialogueMsg(ILTMessage_Read *pMsg)
{
	// -1 is used as a signal to kill the current sound.  This is an 
	// invalid stringdatabase index; don't attempt to get a stringid
	// if this index is read.

	uint32 nSoundId = pMsg->Readuint32();
	const char* pszSound = "";
	if ( nSoundId != -1 )
	{
		pszSound = StringIDFromIndex( nSoundId );
	}

	float fOuterRadius = pMsg->Readfloat();
	float fInnerRadius = pMsg->Readfloat();
	m_nUniqueDialogueId = pMsg->Readuint8( );
	CharacterSoundType cst = (CharacterSoundType)pMsg->Readuint8( );
	int16 nMixChannel = pMsg->Readint16();
	bool bUseRadioSound = pMsg->Readbool();
	
	m_hDialogueRadioSnd = NULL;
	
	char szIcon[128] = "";
	pMsg->ReadString(szIcon,LTARRAYSIZE(szIcon));

	m_bSubtitlePriority = (cst == CST_DIALOG);
	bool bCensor = (cst == CST_DEATH) || (cst == CST_DAMAGE);

	if (bCensor && g_pVersionMgr->IsLowViolence())
	{
		//don't play the sound, but if it is a pain sound, do notify the server that we're done with it
		// if it's a death sound, don't bother because it's not waiting for notification
		if (cst == CST_DAMAGE)
			KillLipSyncSound( true );
		return;
	}


	if (m_hDialogueSnd)
	{
		KillLipSyncSound( false );
	}


	if (*pszSound && fOuterRadius > 0.0f)
	{
		m_bSubtitle = false;
		m_sDialogueIcon = szIcon ? szIcon : "";

		//don't bother getting the handle if it's a death sound, because we don't track them
		bool bGetHandle = (cst != CST_DEATH);
		m_hDialogueSnd = PlayLipSyncSound(pszSound, fOuterRadius, fInnerRadius, m_bSubtitle, m_bSubtitlePriority, 
			nMixChannel, bGetHandle, szIcon, bUseRadioSound, &m_hDialogueRadioSnd);

		//if we didn't get a handle (and we did ask for one) then tell the server we're done with the sound
		if( !m_hDialogueSnd && bGetHandle)
		{
			KillLipSyncSound( true );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleBroadcastMsg
//
//	PURPOSE:	Start a broadcast sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleBroadcastMsg(ILTMessage_Read *pMsg)
{
	uint32 nBroadcastID = pMsg->Readuint16();
	uint32 nClientID = pMsg->Readuint8();
	uint32 nTeam = pMsg->Readuint8();
	bool	bForceLocal  = pMsg->Readbool();
	int8	nPriority = pMsg->Readint8();
	bool	bTakingDamage = pMsg->Readbool();
	bool	bPlayerInitiated = pMsg->Readbool();

	if (!g_pProfileMgr->GetCurrentProfile()->m_bAllowBroadcast)
	{
		return;
	}

	if (GameModeMgr::Instance( ).m_grbUseTeams && nTeam != INVALID_TEAM)
	{
		CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if( !pLocalCI )
			return;

		if( pLocalCI->nTeamID != nTeam )
			return;
	}

	if (g_pInterfaceMgr->IsPlayerMuted(nClientID))
	{
		return;
	}

	if (m_pHUDItem && !m_cs.bIsDead)
	{
		if (bTakingDamage)
		{
			m_pHUDItem->Flash("Damage");
		}
		else
		{
			m_pHUDItem->Flash("Chat");
		}
		
	}

	PlayBroadcast(nBroadcastID, bForceLocal, nClientID, nPriority, false);

	const char* pszSoundID = StringIDFromIndex(nBroadcastID);
	const wchar_t* pwsMsg = LoadString(pszSoundID);

	uint32 nLocalID;
	g_pLTClient->GetLocalClientID( &nLocalID );

	if(!LTStrEmpty(pwsMsg) && m_cs.nClientID != nLocalID && bPlayerInitiated)
	{
		g_pInterfaceMgr->HandleChat(pwsMsg,nClientID,nTeam);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleBodyStateMsg
//
//	PURPOSE:	Set a new BodyState.
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleBodyStateMsg(ILTMessage_Read *pMsg)
{
	m_eBodyState = (BodyState)pMsg->Readuint32();
	m_eBodyCondition = (EnumAnimDesc)pMsg->Readuint32();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateSounds
//
//	PURPOSE:	Update our sounds
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateSounds()
{
	if (m_hDialogueSnd)
	{
		if (g_pLTClient->IsDone(m_hDialogueSnd))
		{
			KillLipSyncSound( true );
		}
	}

	// See if we should play a ding sound...

	if (IsMultiplayerGameClient())
	{
		for (int i=0; i < MAX_DINGS; i++)
		{
			if( m_DingTimers[i].IsStarted( ) && m_DingTimers[i].IsTimedOut( ))
			{
				m_DingTimers[i].Stop();
				HRECORD hSoundRec = g_pSoundDB->GetSoundDBRecord("MPTimeOutDing");
				if (hSoundRec)
				{
					g_pClientSoundMgr->PlayDBSoundLocal(hSoundRec, SOUNDPRIORITY_PLAYER_HIGH);
				}
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateUnderwaterFX
//
//	PURPOSE:	Update the underwater fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateUnderwaterFX()
{
	if (!m_pClientDE || !m_hServerObject) return;

	LTVector vPos;
	if( !IsLocalClient() || 
		!(g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_FirstPerson || 
		g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_Follow) )
	{
        LTVector vDims;
	    LTRotation rRot;

		g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
		g_pPhysicsLT->GetObjectDims(m_hServerObject, &vDims);
		g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

		vPos += rRot.Forward() * 20.0f;
		vPos.y += vDims.y * 0.4f;
	}
	else
	{
		LTRotation const& rRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation( );
		vPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );
		vPos += rRot.Forward() * 20.0f;
		vPos.y -= 20.0f;

		vPos += rRot.Up() * -20.0f;
	}

	if( !m_bIsUnderWater )
	{
		m_bIsUnderWater = true;
		CreateUnderwaterFX(vPos);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateUnderwaterFX
//
//	PURPOSE:	Create underwater special fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateUnderwaterFX(const LTVector & vPos)
{
	if (!m_pClientDE || !g_pGameClientShell || !m_hServerObject) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;


	// Play a splash if they just entered the water and it's not the local client.
	if( !IsLocalClient() || 
		!(g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_FirstPerson || 
		g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_Follow) )
	{
		LTVector vSplashPos;
		g_pLTClient->GetObjectPos( GetServerObj(), &vSplashPos );
		CPolyGridFX* pPolyGridFX = CPolyGridFX::FindSplashInPolyGrid( GetServerObj(), vSplashPos );
		if( pPolyGridFX )
		{
			pPolyGridFX->DoPolyGridSplash( GetServerObj(), vSplashPos, g_pModelsDB->GetSplashesJumpingDeepImpulse( m_cs.hModel ));
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveUnderwaterFX
//
//	PURPOSE:	Remove the underwater fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveUnderwaterFX()
{
	m_bIsUnderWater = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateFlashLightFX
//
//	PURPOSE:	Update the flash light fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateFlashLightFX()
{
	// See if this is the local player...

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_cs.bIsPlayer && hPlayerObj == m_hServerObject)
	{
		// If we're the local player don't show the 3rd person fx

		m_3rdPersonFlashlight.TurnOff();
		return;
	}
	
	m_3rdPersonFlashlight.TurnOn();
	m_3rdPersonFlashlight.Update( g_pLTClient->GetFrameTime() );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveFlashLightFX
//
//	PURPOSE:	Remove the flash light fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveFlashLightFX()
{
	m_3rdPersonFlashlight.TurnOff();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::OnModelKey
//
//	PURPOSE:	Handle model key
//
// ----------------------------------------------------------------------- //

void CCharacterFX::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs, ANIMTRACKERID nTrackerId )
{
	static CParsedMsg::CToken s_cTok_CharacterFXKeyFootstep( CHARACTERFX_KEY_FOOTSTEP );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyMaterial( CHARACTERFX_KEY_MATERIAL );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyMoveLoud( CHARACTERFX_KEY_MOVE_LOUD );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyMoveQuiet( CHARACTERFX_KEY_MOVE_QUIET );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyShowAttachFX( CHARACTERFX_KEY_SHOW_ATTACHFX );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyHideAttachFX( CHARACTERFX_KEY_HIDE_ATTACHFX );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyFX( CHARACTERFX_KEY_FX );
	static CParsedMsg::CToken s_cTok_CharacterFXKeySoundBute( CHARACTERFX_KEY_SOUNDBUTE );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyCharacterSound( CHARACTERFX_KEY_CHARACTERSOUND );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyNoise( CHARACTERFX_KEY_NOISE );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyDecal( CHARACTERFX_KEY_DECAL );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyBlockWindow( CHARACTERFX_KEY_BLOCKWINDOW );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyDodgeWindow( CHARACTERFX_KEY_DODGEWINDOW );
	static CParsedMsg::CToken s_cTok_CharacterFXKeyMeleeControl( CHARACTERFX_KEY_MELEECONTROL );

	static CParsedMsg::CToken s_cTok_Left( "LEFT" );
	static CParsedMsg::CToken s_cTok_2( "2" );

	if( !hObj || !pArgs || !pArgs->argv || pArgs->argc == 0 )
		return;

	// For now, ignore any modelstrings coming from the blend tracker.  In 
	// the future, we may want some strings to be allowed.  If so, 
	// selectively enable strings.

	if ( nTrackerId == kAD_TRK_Blend )
	{
		return;
	}

	// See if the damagefx take care of it...
	if( g_pDamageFXMgr->OnModelKey( hObj, pArgs ) )
		return;

	// Pre-parse the message...
	CParsedMsg cParsedMsg( pArgs->argc, pArgs->argv );
	CParsedMsg::CToken cTok_Key = cParsedMsg.GetArg(0);
	
	if (g_vtModelKey.GetFloat() > 0.0f)
	{
		g_pLTClient->CPrint("%s ModelKey: '%s'", (m_cs.bIsPlayer ? "Player" : "AI"), cTok_Key.c_str());
	}
		
    if( cTok_Key == s_cTok_CharacterFXKeyFootstep )
	{
		// Footsteps are handled by the local client in the playerbody.
		HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		bool bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == hObj);
		if( bIsLocalClient && !IsFootStepException(hObj))
			return;

		if( cParsedMsg.GetArgCount() > 1 )
		{
			// See if this is the left (2) or right (1) foot...

			if( (cParsedMsg.GetArg(1) == s_cTok_2) || (cParsedMsg.GetArg(1) == s_cTok_Left) )
			{
				m_bLeftFoot = true;
			}
			else
			{
				m_bLeftFoot = false;
			}
		}
		else
		{
			// Alternate feet...
			m_bLeftFoot = !m_bLeftFoot;
		}

		DoFootStepKey(hObj);
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyMaterial )
	{
		if( cParsedMsg.GetArgCount() > 2  )
		{
			// MATERIAL <index> <new material name>
			SetObjectMaterial( m_hServerObject, atoi(cParsedMsg.GetArg(1).c_str()), cParsedMsg.GetArg(2).c_str() );
		}
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyMoveLoud )
	{
		g_pClientSoundMgr->PlayDBSoundFromObject( GetLocalContextObject(), g_pModelsDB->GetModelLoudMovementSnd( m_cs.hModel ),
			SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, 0, 
			SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
			DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_FOOTSTEPS_NONPLAYER );
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyMoveQuiet )
	{
		g_pClientSoundMgr->PlayDBSoundFromObject( GetLocalContextObject(), g_pModelsDB->GetModelQuietMovementSnd( m_cs.hModel ),
			SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, 0, 
			SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
			DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_FOOTSTEPS_NONPLAYER );
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyShowAttachFX )
	{
		if( cParsedMsg.GetArgCount() > 1 )
		{
			uint32 nElement = atoi( cParsedMsg.GetArg(1).c_str() );
			ShowAttachClientFX( nElement );
		}
		else
		{
			ShowAttachClientFX( ALL_ATTACH_FX );
		}
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyHideAttachFX )
	{
		if( cParsedMsg.GetArgCount() > 1 )
		{
			uint32 nElement = atoi( cParsedMsg.GetArg(1).c_str() );
			HideAttachClientFX( nElement );
		}
		else
		{
			HideAttachClientFX( ALL_ATTACH_FX );
		}
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyFX )
	{
		if( cParsedMsg.GetArgCount() > 1 )
		{
			CLIENTFX_CREATESTRUCT fxInit( cParsedMsg.GetArg(1).c_str(), 0, GetLocalContextObject());

			// Fill in an optional socket.  This allows an effect to be 
			// spawned from this socket without following it/being relative to it.

			if ( cParsedMsg.GetArgCount() > 2 )
			{
				HMODELSOCKET hAttachSocket = INVALID_MODEL_SOCKET;
				if (LT_OK == g_pModelLT->GetSocket( GetLocalContextObject(), cParsedMsg.GetArg(2).c_str(), hAttachSocket) )
				{
					fxInit.m_hSocket = hAttachSocket;
				}
				else
				{
					g_pLTBase->CPrint( "Model String %s: Failed to find socket specified: %s", s_cTok_CharacterFXKeyFX.c_str(), cParsedMsg.GetArg(2).c_str() );
				}
			}

			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );
		}
	}
	else if( cTok_Key == s_cTok_CharacterFXKeySoundBute )
	{
		// Don't play the sound on the local clients CharacterFX, the PlayerBodyMgr should have already handled that...
		if( m_hServerObject != g_pLTClient->GetClientObject() )
		{
			if( cParsedMsg.GetArgCount() > 1 )
			{
				g_pClientSoundMgr->PlayDBSoundFromObject( m_hServerObject, g_pSoundDB->GetSoundDBRecord(cParsedMsg.GetArg(1).c_str()),
					SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, 0, 
					SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
					DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_FOOTSTEPS_NONPLAYER );
			}
		}
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyCharacterSound )
	{
		// Don't play the sound on the local clients CharacterFX, the PlayerBodyMgr should have already handled that...
		if( m_hServerObject != g_pLTClient->GetClientObject() )
		{
			if( cParsedMsg.GetArgCount() > 1 )
			{
				g_pClientSoundMgr->PlayDBSoundFromObject( m_hServerObject, g_pSoundDB->GetCharacterSoundDBRecord(GetModel(), cParsedMsg.GetArg(1).c_str()),
					SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, 0, 
					SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
					DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_FOOTSTEPS_NONPLAYER );
			}
		}
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyNoise )
	{
		// Hitting the ground noise

		SurfaceType eSurface = ST_UNKNOWN;

		CollisionInfo Info;
		g_pPhysicsLT->GetStandingOn(GetLocalContextObject(), &Info);

		if (Info.m_hPoly != INVALID_HPOLY)
		{
			eSurface = (SurfaceType)GetSurfaceType(Info.m_hPoly);
		}
		else if (Info.m_hObject) // Get the texture flags from the object...
		{
			eSurface = (SurfaceType)GetSurfaceType(Info.m_hObject);
		}

		// Play the noise

		LTVector vPos;
		g_pLTClient->GetObjectPos(GetLocalContextObject(), &vPos);

		HSURFACE hSurf = g_pSurfaceDB->GetSurface(eSurface);
		LTASSERT(hSurf,"Invalid surface.");
		if (hSurf)
		{
			HRECORD hFallSnd = g_pSurfaceDB->GetRecordLink(hSurf,SrfDB_Srf_rBodyFallSnd);
			if (hFallSnd)
			{
				g_pClientSoundMgr->PlayDBSoundFromPos(vPos, hFallSnd,
					SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, 0, 
					SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
					DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_FOOTSTEPS_NONPLAYER);
			}
		}
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyDecal )
	{
		if( cParsedMsg.GetArgCount() > 2 )
		{
			// Do an intersect segment to determine where to put the decal
			// fx...
			IntersectQuery iQuery;
			IntersectInfo  iInfo;
			iQuery.m_Flags = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;

			ILTModel* pModelLT   = g_pLTClient->GetModelLT();
			HMODELSOCKET hSocket;

			if (pModelLT->GetSocket(hObj, cParsedMsg.GetArg(1).c_str(), hSocket) != LT_OK)
			{
				return;
			}
				
			LTTransform transform;
			if (pModelLT->GetSocketTransform(hObj, hSocket, transform, true) != LT_OK)
			{
				return;
			}

			LTVector vR, vU, vF;
			transform.m_rRot.GetVectors(vR, vU, vF);
			vU.Normalize();

			iQuery.m_From = transform.m_vPos;
			iQuery.m_From += (vU * INTERSECT_Y_OFFSET);

			iQuery.m_To	= iQuery.m_From;
			iQuery.m_To -= (vU * INTERSECT_Y_OFFSET * 2.0f);

			// Don't hit ourself...
			HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(), g_pPlayerMgr->GetMoveMgr()->GetObject(), NULL};
			iQuery.m_pUserData	= hFilterList;
			iQuery.m_FilterFn	= WorldOnlyFilterFn;

			if (!m_pClientDE->IntersectSegment(iQuery, &iInfo))
			{
				return;
			}

			// Create the decal fx...
			LTRotation rRot;
			g_pLTClient->GetObjectRotation(hObj, &rRot);
			LTRotation rFootprintRot(iInfo.m_Plane.m_Normal, rRot.Forward());
			LTVector vFootprintPos = iInfo.m_Point + iInfo.m_Plane.m_Normal*1.0f;

			CLIENTFX_CREATESTRUCT fxCS( cParsedMsg.GetArg(2).c_str(), 0, LTRigidTransform(vFootprintPos, rFootprintRot));
			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
		}
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyBlockWindow )
	{
		if( pArgs->argc > 1 )
		{
			int nDurationMS = atoi( pArgs->argv[ 1 ] );
			float flDurationS = nDurationMS / 1000.f;

			// Notify the server, so the AI can make decisions.

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_OBJECT_MESSAGE );
			cMsg.WriteObject( m_hServerObject );
			cMsg.Writeuint32( MID_SFX_MESSAGE );
			cMsg.Writeuint8( SFX_CHARACTER_ID );
			cMsg.WriteBits(CFX_BLOCKWINDOW_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			cMsg.Writefloat( flDurationS );
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

			// Update the local state for player decisions.

			m_flBlockWindowEndTime = g_pLTClient->GetTime() + flDurationS;
		}
	}
	else if( cTok_Key == s_cTok_CharacterFXKeyDodgeWindow )
	{
		if( pArgs->argc > 1 )
		{
			int nDurationMS = atoi( pArgs->argv[ 1 ] );
			float flDurationS = nDurationMS / 1000.f;

			// Notify the server, so the AI can make decisions.

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_OBJECT_MESSAGE );
			cMsg.WriteObject( m_hServerObject );
			cMsg.Writeuint32( MID_SFX_MESSAGE );
			cMsg.Writeuint8( SFX_CHARACTER_ID );
			cMsg.WriteBits(CFX_DODGEWINDOW_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			cMsg.Writefloat( flDurationS );
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
		}
	}
	else if ( cTok_Key == s_cTok_CharacterFXKeyMeleeControl )
	{
		HandleMeleeModelKey(hObj, pArgs, nTrackerId);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::DoFootStepKey
//
//	PURPOSE:	Handle model foot step key
//
// ----------------------------------------------------------------------- //

void CCharacterFX::DoFootStepKey(HLOCALOBJ hObj, bool bForceSound)
{
	if (!hObj) return;

	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) return;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	bool bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == hObj);

	m_eLastSurface = ST_UNKNOWN;
	LTVector vPos;

	if (bIsLocalClient)
	{
		if (g_pPlayerMgr->IsSpectating() || !pMoveMgr->CanDoFootstep())
		{
			return;
		}

		// Use our current standing on surface if we still don't know what
		// we're standing on...

		m_eLastSurface = pMoveMgr->GetStandingOnSurface();

		g_pLTClient->GetObjectPos(pMoveMgr->GetObject(), &vPos);
	}
	else
	{
		g_pLTClient->GetObjectPos(hObj, &vPos);
	}

	IntersectQuery iQuery;
	IntersectInfo  iInfo;
	LTVector vFootPos;

	// Do an intersect segment to determine where to put the footprint
	// sprites...

	iQuery.m_Flags = IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	iQuery.m_From  = vPos;
	vFootPos = vPos;

	// If the object has Left/RightFoot sockets, use them to determine
	// the location for the InteresectSegment...

	ILTModel* pModelLT   = g_pLTClient->GetModelLT();

	char* pSocketName = (char *)(m_bLeftFoot ? "LeftFoot" : "RightFoot");
	HMODELSOCKET hSocket;

	if (pModelLT->GetSocket(hObj, pSocketName, hSocket) == LT_OK)
	{
		LTTransform transform;
		if (pModelLT->GetSocketTransform(hObj, hSocket, transform, true) == LT_OK)
		{
			iQuery.m_From = transform.m_vPos;
			vFootPos = transform.m_vPos;

			// Testing...
			iQuery.m_From.y += INTERSECT_Y_OFFSET;
		}
	}

	iQuery.m_To	= iQuery.m_From;
	iQuery.m_To.y -= (INTERSECT_Y_OFFSET * 2.0f);

	// Don't hit ourself...

	HOBJECT hFilterList[] = {g_pLTClient->GetClientObject(), g_pPlayerMgr->GetMoveMgr()->GetObject(), NULL};

	if (bIsLocalClient)
	{
		iQuery.m_FilterFn	= WorldOnlyFilterFn;
		iQuery.m_pUserData	= hFilterList;
	}

	if (m_pClientDE->IntersectSegment(iQuery, &iInfo))
	{
		if (IsMainWorld(iInfo.m_hObject) ||
			GetObjectType(iInfo.m_hObject) == OT_WORLDMODEL)
		{
			if (m_eLastSurface == ST_UNKNOWN)
			{
				m_eLastSurface = GetSurfaceType(iInfo);
			}
		}
	}


	// Play a footstep sound if this isn't the local client (or we're in
	// 3rd person).  The local client's footsteps are tied to the head bob
	// in 1st person...

	bool bPlaySound = true;

	if( !bForceSound && bIsLocalClient )
	{
		bPlaySound = (g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() != CPlayerCamera::kCM_FirstPerson) || LadderMgr::Instance().IsClimbing() || SpecialMoveMgr::Instance().IsActive();
	}

	if( bPlaySound )
	{
		PlayFootstepSound( vPos, m_eLastSurface, m_bLeftFoot, m_cs.ePlayerPhysicsModel );
	}


	// Leave footprints on the appropriate surfaces...

	if (ShowsTracks(m_eLastSurface))
	{
		// Use intersect position for footprint sprite...

		CreateFootprint(m_eLastSurface, iInfo);
	}

	CPolyGridFX* pPolyGridFX = CPolyGridFX::FindSplashInPolyGrid( GetServerObj(), vFootPos );
	if( pPolyGridFX )
	{
		pPolyGridFX->DoPolyGridSplash( GetServerObj(), vFootPos, g_pModelsDB->GetSplashesFootstepImpulse( m_cs.hModel ));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::IsFootStepException
//
//	PURPOSE:	Check if this is a footstep exception (most player footsteps
//				will be handled by HeadBob, except for these)
//
// ----------------------------------------------------------------------- //

bool CCharacterFX::IsFootStepException(HLOCALOBJ hObj)
{
	if (!hObj) 
		return false;

	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) 
		return false;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	bool bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == hObj);

	bool bIsException = false;

	if (bIsLocalClient )
	{
		bIsException = (g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() != CPlayerCamera::kCM_FirstPerson) || LadderMgr::Instance().IsClimbing() || SpecialMoveMgr::Instance().IsActive();
	}

	return bIsException;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayFootstepSound
//
//	PURPOSE:	Play footstep sound
//
// ----------------------------------------------------------------------- //

//callback function that is used to determine if we are overlapping with any ladder objects, and if we
//are, we will fill in the first encountered ladder volume and then stop looking. The user data is
//a pointer to a CLadderFX* that we will fill in with the ladder object
static void FindOverlappingLaddersCB(HOBJECT hObject, void* pUser)
{
	//we should always have a user parameter provided
	LTASSERT(pUser, "Error: Invalid user data passed into FindOverlappingLaddersCB");

	//convert our user pointer over to something more usable
	CLadderFX** ppLadder = (CLadderFX**)pUser;

	//now check the user data, and if we already have found a result, don't bother to keep looking
	if(*ppLadder)
		return;

	//we haven't found a ladder yet, so see if this object is a ladder
	*ppLadder = (CLadderFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_LADDER_ID, hObject);
}

void CCharacterFX::PlayFootstepSound( LTVector const& vPos, SurfaceType eSurface, bool bLeftFoot, PlayerPhysicsModel ePPModel )
{
	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) return;

	// Don't do movement sounds if in the menu...

	if ( !g_pInterfaceMgr->IsInGame( )) return;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	bool bIsLocalClient = (m_cs.bIsPlayer && hPlayerObj == m_hServerObject);

	// Dead men don't make movement sounds...

	if (bIsLocalClient && !g_pPlayerMgr->IsPlayerAlive()) return;

	// Dead men don't make movement sounds in multiplayer either...

	if (m_cs.bIsPlayer && IsMultiplayerGameClient() && m_cs.bIsDead) return;


	// If we're on a ladder, make sure it plays sounds and see if there is a surface override...

	if (eSurface == ST_LADDER)
	{
		// Find the ladder we're in...
		CLadderFX *pLadder = NULL;

		// Use the Ladder container from the server if we are the local client...
		if( bIsLocalClient )
		{
			pLadder = LadderMgr::Instance().GetLadder();
		}
		
		// If we aren't the local client or failed to get the ladder object use the server objects position...
		if( !pLadder )
		{
			LTVector vDims;
			g_pPhysicsLT->GetObjectDims( GetLocalContextObject(), &vDims );

			g_pLTClient->FindObjectsInBox( vPos, vDims, FindOverlappingLaddersCB, &pLadder );
		}

		if( pLadder )
		{
			// If there is a valid override surface use it for the footstep sounds...
			if( pLadder->GetSurfaceOverride() != ST_UNKNOWN )
			{
				eSurface = pLadder->GetSurfaceOverride();
			}
		}
		

	}

	// Determine the character type for the footstep call.
	MovementSound::ECharacterType eCharacterType = MovementSound::eCharacterType_AI;
	if( m_cs.bIsPlayer )
	{
		if( bIsLocalClient )
			eCharacterType = MovementSound::eCharacterType_PlayerFirstPerson;
		else
			eCharacterType = MovementSound::eCharacterType_PlayerThirdPerson;
	}

	HRECORD hSR = MovementSound::GetSoundRecord( eSurface, bLeftFoot ? MovementSound::eFootSide_Left : MovementSound::eFootSide_Right,
		eCharacterType );
	uint32 dwFlags = ( bIsLocalClient ? PLAYSOUND_CLIENTLOCAL : 0 );
	SoundPriority ePriority = ( m_cs.bIsPlayer ? SOUNDPRIORITY_PLAYER_HIGH : SOUNDPRIORITY_AI_HIGH );

	if( hSR )
	{
		float fStealth = ( ePPModel == PPM_NORMAL ? m_cs.fStealthPercent : 1.0f );

		int nVolume;
		SoundRecord sr;
		g_pSoundDB->FillSoundRecord( hSR,sr );

		// If we're specifying a movement volume externally... use that
		if( m_fFootstepVolume != -1.0f )
		{
			nVolume = ( int )( ( ( float )sr.m_nVolume * m_fFootstepVolume ) + 0.5f );
		}
		else
		{
			// Ok, it's a valid sound record, get the volume...
			nVolume = ( int )( ( ( float )sr.m_nVolume * fStealth ) + 0.5f );

			// Adjust the volume if we are walking or ducking...
			if( bIsLocalClient && pMoveMgr->IsMovingQuietly() )
			{
				nVolume = ( int )( ( ( float )nVolume * g_vtQuietMovementVolumeFactor.GetFloat() ) + 0.5f );
			}
		}

		g_pClientSoundMgr->PlayDBSoundFromPos( vPos, hSR, SMGR_INVALID_RADIUS, ePriority, dwFlags, nVolume,
			1.0f, SMGR_INVALID_RADIUS, DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_FOOTSTEPS_NONPLAYER );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayMovementSound
//
//	PURPOSE:	Play movement sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::PlayMovementSound()
{
	// Don't do movement sounds if in the menu...
	if( !g_pInterfaceMgr->IsInGame() ) return;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	bool bIsLocalClient = ( m_cs.bIsPlayer && ( hPlayerObj == m_hServerObject ) );

	// Dead men don't make movement sounds...
	if( bIsLocalClient && !g_pPlayerMgr->IsPlayerAlive() ) return;

	// Dead men don't make movement sounds in multiplayer either...
	if( m_cs.bIsPlayer && IsMultiplayerGameClient() && m_cs.bIsDead ) return;

	// Play the movement sound
	if( m_hBodyMovementSound )
	{
		uint32 dwFlags = ( bIsLocalClient ? PLAYSOUND_CLIENTLOCAL : 0 );
		SoundPriority ePriority = ( m_cs.bIsPlayer ? SOUNDPRIORITY_PLAYER_HIGH : SOUNDPRIORITY_AI_HIGH );

		g_pClientSoundMgr->PlayDBSoundLocal( m_hBodyMovementSound, ePriority, dwFlags, 100, 1.0f,
			DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_FOOTSTEPS_NONPLAYER );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateFootprint
//
//	PURPOSE:	Create a footprint sprite at the specified location
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateFootprint(SurfaceType eType, IntersectInfo & iInfo)
{
	HSURFACE hSurf = g_pSurfaceDB->GetSurface(eType);
	if (!hSurf) return;

	const char* pszLeft = g_pSurfaceDB->GetString(hSurf,SrfDB_Srf_sLtFootPrintFX);
	const char* pszRight = g_pSurfaceDB->GetString(hSurf,SrfDB_Srf_sRtFootPrintFX);

	if (!pszLeft || !pszRight) return;

	// Dead men don't make footprints...

	if (m_cs.bIsPlayer && IsMultiplayerGameClient() && m_cs.bIsDead) return;

	LTRotation rRot;
	g_pLTClient->GetObjectRotation(GetLocalContextObject(), &rRot);
	LTRotation rFootprintRot(iInfo.m_Plane.m_Normal, rRot.Forward());
	LTVector vFootprintPos = iInfo.m_Point + iInfo.m_Plane.m_Normal*1.0f;

	const char* pszFootPrintFX = m_bLeftFoot ? pszRight : pszLeft;
	CLIENTFX_CREATESTRUCT fxCS( pszFootPrintFX, 0, LTRigidTransform(vFootprintPos, rFootprintRot));
	g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxCS, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

bool CCharacterFX::OnServerMessage(ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::OnServerMessage(pMsg)) return false;

	uint8 nMsgId = pMsg->ReadBits( FNumBitsExclusive<CFX_COUNT>::k_nValue );

	switch(nMsgId)
	{
		case CFX_CROSSHAIR_MSG:
		{
			m_cs.eCrosshairPlayerStance = (EnumCharacterStance)pMsg->ReadBits( FNumBitsExclusive<kCharStance_Count>::k_nValue );
		}
		break;

		//sent down when a new spear is attached so we can hide them as necessary
		case CFX_UPDATE_ATTACHMENTS:
		{
			m_bUpdateAttachments = true;
		}
		break;

		case CFX_NODECONTROL_LIP_SYNC:
		{
			m_NodeController.HandleNodeControlMessage(nMsgId, pMsg);
		}
		break;

		case CFX_DIALOGUE_MSG:
		{
			HandleDialogueMsg(pMsg);
		}
		break;

		case CFX_BROADCAST_MSG:
		{
			HandleBroadcastMsg(pMsg);
		}
		break;

		case CFX_BODYSTATE_MSG:
		{
			HandleBodyStateMsg(pMsg);
		}
		break;

		case CFX_RESET_TRACKER:
		{
			uint8 iTracker = pMsg->Readuint8();
			if ( iTracker == 0 )
			{
				g_pModelLT->ResetAnim(m_hServerObject, MAIN_TRACKER);
			}
			else
			{
				g_pModelLT->ResetAnim(m_hServerObject, iTracker);
			}
		}
		break;

		case CFX_DMGFLAGS_MSG:
		{
			if( !pMsg->Readbool( ))
			{
				m_cs.nDamageFlags = 0;
			}
			else
			{
				m_cs.nDamageFlags = pMsg->ReadBits( kNumDamageTypes );
			}
		}
		break;

		case CFX_INSTANTDMGFLAGS_MSG:
		{
			m_nInstantDamageFlags = pMsg->ReadBits( kNumDamageTypes );
		}
		break;

		case CFX_STEALTH_MSG:
		{
			m_cs.fStealthPercent = pMsg->Readfloat();
		}
		break;

		case CFX_CLIENTID_MSG:
		{
			m_cs.nClientID = pMsg->ReadBits( FNumBitsExclusive<MAX_MULTI_PLAYERS*2>::k_nValue );
			if( m_cs.nClientID == ( 1 << FNumBitsExclusive<MAX_MULTI_PLAYERS*2>::k_nValue ) - 1 )
				m_cs.nClientID = INVALID_CLIENT;

			UpdateHUDData();
		}
		break;

		case CFX_CHAT_MSG:
		{
			m_cs.SetChatting(!!pMsg->Readuint8());
			if ( m_linkChatFX.IsValid() ) 
			{
				if (m_cs.IsChatting())
					m_linkChatFX.GetInstance()->Show();
				else
					m_linkChatFX.GetInstance()->Hide();
			}

		}
		break;

		case CFX_SLIDE:
			{
				bool bSliding = pMsg->Readbool();
				bool bIsLocalClient = (m_cs.bIsPlayer && g_pLTClient->GetClientObject() == m_hServerObject);
				if (!bIsLocalClient)
				{
					if (bSliding && !m_cs.IsSliding())
					{
						HRECORD hSR = g_pSoundDB->GetSoundDBRecord("Ladder_Slide");

						if (hSR)
						{
							// Play the sound from the character
							m_hSlideSound = g_pClientSoundMgr->PlayDBSoundFromObject( m_hServerObject, hSR, SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, (PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP) );
						}

					}
					else if (!bSliding && m_cs.IsSliding())
					{
						KillSlideSound();
					}

				}
				m_cs.SetSliding(bSliding);

			}
			break;


		case CFX_FLASHLIGHT_CREATE_MSG:
		{
			LTASSERT(!(m_cs.byFXFlags & CHARCREATESTRUCT::eFlashLight), "");
			m_cs.byFXFlags |= CHARCREATESTRUCT::eFlashLight;
		}
		break;

		case CFX_FLASHLIGHT_DESTROY_MSG:
		{
			LTASSERT(m_cs.byFXFlags & CHARCREATESTRUCT::eFlashLight, "");
			m_cs.byFXFlags &= ~CHARCREATESTRUCT::eFlashLight;
		}
		break;

		case CFX_ALLFX_MSG:
		{
			// Re-init our data...

			m_cs.Read(pMsg);

			InitLocalPlayer();

			if (m_cs.bIsDead)
			{
				InitDead();
			}
			else
			{
				ClearDead();
			}
		}
		break;

		case CFX_INFO_STRING:
		{
			char szTmp[kMaxInfoStringLength];
			pMsg->ReadString(szTmp,kMaxInfoStringLength);

			m_wsInfoString = MPA2WEX<kMaxInfoStringLength>(szTmp).c_str();

			char *pTok = strtok(szTmp,"\n");
			if (pTok)
			{
				m_Str.SetText( MPA2WEX<kMaxInfoStringLength>(pTok).c_str() );
			}
			else
			{
				m_Str.SetText(NULL);
			}

		}
		break;

		case CFX_WEAPON_SOUND_MSG :
		{
			HandleWeaponSoundMsg( pMsg );
		}
		break;

		case CFX_WEAPON_SOUND_LOOP_MSG :
		{
			// Only play sounds that did not originate from us...

			if( g_pLTClient->GetClientObject() != m_hServerObject )
			{
				HandleWeaponSoundLoopMsg( pMsg );				
			}
		}
		break;

		case CFX_TRACKER_TARGET_MSG:
		{			
			m_NodeTrackerContext.HandleServerMessage( pMsg );
		}
		break;

		case CFX_MELEE_CONTROLLER_MSG:
		{
			m_MeleeCollisionController.HandleServerMessage( pMsg );
		}
		break;

		case CFX_HITBOX_MSG:
		{
			LTVector vDims, vOffset;
						
			// Update the hitbox dims.  If can't use defaults, characterfx will fix it up in its init.
			bool bUseDefaultHitboxDims = !pMsg->Readbool();
			if( bUseDefaultHitboxDims )
			{
				g_pPhysicsLT->GetObjectDims( m_hServerObject, &vDims );
				vDims *= HB_DIMS_ENLARGE_PERCENT;
			}
			else
			{
				vDims		= pMsg->ReadCompLTVector();
			}

			if( !pMsg->Readbool())
			{
				vOffset = LTVector::GetIdentity();
			}
			else
			{
				vOffset	= pMsg->ReadCompLTVector();
			}

			// Set the dims and offset on the hitbox

			m_HitBox.SetDims( vDims );
			m_HitBox.SetOffset( vOffset );
		}
		break;

		case CFX_DEAD:
		{
			m_cs.bIsDead = true;
			m_cs.eDeathDamageType = static_cast<DamageType>(static_cast<DamageType>(pMsg->ReadBits( FNumBitsExclusive<kNumDamageTypes>::k_nValue )));
			m_cs.hDeathAmmo = (HAMMO)pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory( ));
			InitDead();
		}
		break;

		case CFX_PLAYER_RESPAWN:
		{
			ClearDead();

			// Play respawn sound.
			if( IsMultiplayerGameClient( ))
			{
				HRECORD hSound = ClientDB::Instance().GetRecordLink(ClientDB::Instance().GetClientSharedRecord(), "PlayerRespawnSound");
				if( hSound )
				{
					LTVector vPos;
					g_pLTClient->GetObjectPos( GetServerObj( ), &vPos );
					g_pClientSoundMgr->PlayDBSoundFromPos( vPos, hSound );
				}
			}
		}
		break;

		case CFX_FADE_MSG:
		{
			if (m_cs.bIsDead)
			{
				if( !m_FadeTimer.IsStarted() )
					m_FadeTimer.Start(kFadeOutTime);
			}
		}
		break;
		
		case CFX_CINEMATICAI_MSG:
		{
			m_cs.bIsCinematicAI = pMsg->Readbool();
		}
		break;

		case CFX_PLAYER_PHYSICS_MODEL:
		{
			PlayerPhysicsModel ppm = (PlayerPhysicsModel)pMsg->Readuint8();
			m_cs.ePlayerPhysicsModel = ppm;
		}
		break;

		case CFX_PLAYER_TIMER:
			{
				// Read the time scale sent from the server.
				bool bSimulationTime = pMsg->Readbool();
				uint16 nNumerator = 1;
				uint16 nDenominator = 1;
				if( !bSimulationTime )
				{
					nNumerator = pMsg->Readuint16();
					nDenominator = pMsg->Readuint16();
				}

				// Set our timer scale.
				EngineTimer engineTimer = m_hServerObject;
				if( engineTimer.IsValid( ))
				{
					engineTimer.SetTimerTimeScale( nNumerator, nDenominator );
				}
				else
				{
					LTERROR( "Player must have object specific timer." );
				}
			}
			break;

		case CFX_UPDATE_FORENSIC_MASK:
		{
			CForensicObjectFX* pFX=NULL;

			uint32 dwForensicTypeMask = pMsg->Readuint32();
			if (dwForensicTypeMask)
			{
				CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_FORENSICOBJECT_ID);
				if (pList)
				{
					int nNumForensics = pList->GetSize();
					for (int i=0; i < nNumForensics; i++)
					{
						CForensicObjectFX* pCurFX = (CForensicObjectFX*)(*pList)[i];
						if (pCurFX && pCurFX->m_cs.m_bPrimary && (pCurFX->m_cs.m_dwForensicTypeMask & dwForensicTypeMask))
						{
							pFX = pCurFX;
							break;	// assume there will only ever be one active forensic object at a time on a given navmesh poly.
						}
					}
				}
			}

			g_pPlayerMgr->SetForensicObject(pFX);
		}
		break;

		case CFX_SEVER:
		{
			HandleSeverMsg(pMsg);
		}
		break;

		case CFX_CREATE_LOOP_FX_MSG:
		{
			if( m_linkLoopFX.IsValid() )
			{
				g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkLoopFX );
			}

			char szFXName[64];
			pMsg->ReadString( szFXName, LTARRAYSIZE(szFXName) );

			CLIENTFX_CREATESTRUCT fxInit( szFXName, FXFLAG_LOOP, GetLocalContextObject());
			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_linkLoopFX, fxInit, true );
		}
		break;
		
		case CFX_KILL_LOOP_FX_MSG:
		{
			if( m_linkLoopFX.IsValid() )
			{
				g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkLoopFX );
			}
		}
		break;

		case CFX_RAGDOLL:
		{
			m_cs.vDeathDir = pMsg->ReadCompLTPolarCoord();
			m_cs.fDeathImpulseForce = pMsg->Readfloat();
			m_cs.hModelNodeLastHit = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pModelsDB->GetNodesCategory( ));
			m_cs.fDeathNodeImpulseForceScale = pMsg->Readfloat();
	
			m_bInitRagDoll = true;
		}
		break;

		case CFX_CHANGE_WEAPON:
		{
			m_cs.hCurWeaponRecord = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory( ));
		}
		break;

		case CFX_SPECTATE:
		{
			m_cs.bIsSpectating = pMsg->Readbool();
		}
		break;

		case CFX_USE_GEAR:
		{
			HGEAR hGear = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetGearCategory() );
			GearMsgType eMsgType = static_cast<GearMsgType>(pMsg->Readuint8());
			uint8 nCount = pMsg->Readuint8();

			// Play an clientfx for everyone.
			if( eMsgType == kGearUse )
			{
				char const* pszActivateFX = g_pWeaponDB->GetString( hGear, WDB_GEAR_sActivateFX );
				if( pszActivateFX && pszActivateFX[0] )
				{
					CLIENTFX_CREATESTRUCT fxInit( pszActivateFX, 0, GetLocalContextObject());
					g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );
				}
			}

			// If this is our local client, then update our inventory.
			bool bIsLocalClient = ( m_cs.bIsPlayer && (g_pLTClient->GetClientObject() == m_hServerObject));
			if( bIsLocalClient )
			{
				//this updates player stats....
				g_pInterfaceMgr->GetPlayerStats( )->UpdateGear( hGear, eMsgType, nCount );
				g_pHUDMgr->QueueUpdate(kHUDGear);
			}
		}
		break;

		case CFX_FX_MSG:
		{
			char szClientFX[MAX_CLIENTFX_NAME_LEN + 1] = {0};
			pMsg->ReadString( szClientFX, LTARRAYSIZE( szClientFX ));

			CLIENTFX_CREATESTRUCT fxInit( szClientFX, 0, GetLocalContextObject());

			HMODELSOCKET hSocket = pMsg->Readuint32( );
			fxInit.m_hSocket = hSocket;

			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );
		}
		break;

		case CFX_SHOW_ATTACH_FX:
		{
			uint32 nElement = pMsg->Readuint32();
			ShowAttachClientFX( nElement );
		}
		break;

		case CFX_HIDE_ATTACH_FX:
		{
			uint32 nElement = pMsg->Readuint32();
			HideAttachClientFX( nElement );
		}
		break;

		case CFX_MOTION_STATUS_FX:
		{
			uint8 nMotionStatus = pMsg->ReadBits( FNumBitsExclusive<MS_COUNT>::k_nValue );
			switch( nMotionStatus )
			{
			case MS_JUMPED:	
				{
					HRECORD hSoundRec = g_pSoundDB->GetSoundDBRecord("Jump3D");
					if (hSoundRec)
					{
						LTVector vPos;
						g_pLTClient->GetObjectPos( GetServerObj(), &vPos );
						g_pClientSoundMgr->PlayDBSoundFromPos(vPos, hSoundRec );
					}
				}
				break;
			case MS_LANDED:	
				{
					LTVector vPos;
					g_pLTClient->GetObjectPos( GetServerObj(), &vPos );

					HRECORD hLandSound = pMsg->ReadDatabaseRecord(g_pLTDatabase, g_pSoundDB->GetSoundCategory());
					if( hLandSound )
					{
						g_pClientSoundMgr->PlayDBSoundFromPos(vPos, hLandSound );
					}

					HOBJECT hPolyGrid = pMsg->ReadObject();
					if( hPolyGrid )
					{
						CPolyGridFX* pPolyGridFx = (CPolyGridFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_POLYGRID_ID, hPolyGrid );
						if( pPolyGridFx )
						{
							// Make the footstep splash.
							pPolyGridFx->DoPolyGridSplash( GetServerObj( ), vPos, g_pModelsDB->GetSplashesJumpingShallowImpulse( m_cs.hModel ));
						}
					}
				}
				break;
			}
		}
		break;

		default : break;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayLipSyncSound
//
//	PURPOSE:	Play a lip synced sound.
//
// ----------------------------------------------------------------------- //

HLTSOUND CCharacterFX::PlayLipSyncSound(const char* pszSoundID, float fOuterRadius, float fInnerRadius, bool & bSubtitle, 
										bool bSubtitlePriority, int16 nMixChannel, bool bGetHandle /*= true */, 
										const char *szIcon /* = NULL */, const bool bUseRadioSound /* = false */,
										HLTSOUND* pRadioSoundHandle /* = NULL */)
{

	bSubtitle = false;

	if (!pszSoundID || !pszSoundID[0] || fOuterRadius <= 0.0f) return NULL;

	uint32 dwFlags = 0;
	if (bGetHandle)
	{
		dwFlags = PLAYSOUND_GETHANDLE;
	}

	bool bIsLocalClient = false;
	SoundPriority ePriority = SOUNDPRIORITY_AI_HIGH;
	if (m_cs.bIsPlayer)
	{
		ePriority = SOUNDPRIORITY_PLAYER_HIGH;

		bIsLocalClient = (g_pLTClient->GetClientObject() == m_hServerObject);
		dwFlags |= bIsLocalClient ? PLAYSOUND_CLIENTLOCAL : 0;
	}

	// throw an assert if the string ID was not found
	if( !g_pLTIStringEdit->DoesIDExist(g_pLTDBStringEdit, pszSoundID) )
	{
		g_pLTBase->CPrint( "[PlayLipSyncSound] - The String ID '%s' does not exist!", pszSoundID );
		LTASSERT_PARAM1( 0, "The String ID '%s' does not exist!", pszSoundID );
	}

	// Show subtitles? (Dialogue sounds only)...

	LTVector vPos;
	g_pLTClient->GetObjectPos(GetLocalContextObject(), &vPos);

	if( pszSoundID && (pszSoundID[0] != '\0') )
	{
		// If we're in a cinematic use the cinematic radius, else
		// use the conversation radius...

		// KLS - 5/2/04 - Updated so LD can use the bIsCinematicAI flag to force playing
		// AI dialogue in the player's head even when not in a cinematic...
		if (( g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_Cinematic ) ||
				m_cs.bIsCinematicAI)
		{
			// Okay this dialogue is being played during a cinematic so make sure
			// we are either the player or a cinematic ai.  If not, don't play the
			// sound.
			
			if (bIsLocalClient || m_cs.bIsCinematicAI)
			{
				fOuterRadius = g_vtDialogueCinematicSoundRadius.GetFloat();
				fInnerRadius = g_vtDialogueCinematicSoundInnerRadius.GetFloat();

				// Since we're in a cinematic, force the dialogue to played
				// in the player's head (so it feels like a movie)
				dwFlags |= PLAYSOUND_CLIENTLOCAL;

				// Unload cinematic dialogue sounds after they are played...
				// NOTE: This will only unload dialogue sounds in cinematics,
				// to unload the conversation dialogue sounds we need to come
				// up with another approach...
				dwFlags |= PLAYSOUND_ONCE;
			}
			else
			{
				return NULL;  // Don't play this sound
			}
		}

		if (dwFlags & PLAYSOUND_CLIENTLOCAL)
		{
			// Force subtitle to be shown...
			vPos.Init();
		}

		//nStringId += SUBTITLE_STRINGID_OFFSET;
		bSubtitle = true;
	}

	// extract the voice path
	const char* pVoicePath;

#ifdef PLATFORM_XENON

	// for Xenon, we can just use the ID string as an XAct cue.
	pVoicePath = pszSoundID;

#else

	// for Win32/Linux, we have to get the voice path out of StringEdit,
	// and then convert it into a single-byte char string.
	const char* szVoicePath = g_pLTIStringEdit->GetVoicePath( g_pLTDBStringEdit, pszSoundID );

#ifndef _FINAL
	if( !g_pLTIStringEdit->DoesVoicePathExist(g_pLTDBStringEdit, pszSoundID) || LTStrEmpty(szVoicePath) )
	{
		g_pLTBase->CPrint( "[PlayLipSyncSound] - The String ID '%s' does have a valid voice path!", pszSoundID );
		LTASSERT_PARAM1( 0, "The String ID '%s' does have a valid voice path!", pszSoundID );
	}
#endif

	pVoicePath = szVoicePath;

#endif // PLATFORM_XENON

	HLTSOUND hSound = NULL;
	HLTSOUND hRadio = NULL;


	// Add radio sound stuff here -- Terry
	char pszRadioSoundPath[MAX_PATH];
	if (bUseRadioSound)
	{
		char pszRadioSoundExt[MAX_PATH];
		int32 nExtIndex;

		LTStrCpy(pszRadioSoundPath, pVoicePath, MAX_PATH);

		nExtIndex = CResExtUtil::GetExtensionIndex(pszRadioSoundPath);
		if( nExtIndex != -1 )
		{
			LTStrCpy(pszRadioSoundExt, &(pszRadioSoundPath[nExtIndex]), MAX_PATH);
			if (nExtIndex > 0)
			{
				nExtIndex--;	// strip off the '.'
			}
			pszRadioSoundPath[nExtIndex] = 0;
			LTStrCat(pszRadioSoundPath, "R.", MAX_PATH);
			LTStrCat(pszRadioSoundPath, pszRadioSoundExt, MAX_PATH);
		}
	}
	else
	{
		LTStrCpy(pszRadioSoundPath, "", MAX_PATH);
	}

	// Determine if the sound is a sound record...
	//HRECORD hSoundRec = g_pSoundDB->GetSoundDBRecord( pszVoicePath.c_str() );
	
	if (bGetHandle)
	{
		//if( hSoundRec )
		//{
		//	hSound = g_pClientSoundMgr->PlayDBSoundFromObject( m_hServerObject, hSoundRec, SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, dwFlags,
		//		SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
		//		SPEECH_SOUND_CLASS, nMixChannel );
		//}
		//else
		{
		hSound = g_pClientSoundMgr->PlaySoundFromObject(GetLocalContextObject(),
				pVoicePath, NULL, fOuterRadius, ePriority, dwFlags | PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 
				1.0f, fInnerRadius, SPEECH_SOUND_CLASS, nMixChannel );
		}
		if (bUseRadioSound)
		{
			
			hRadio = g_pClientSoundMgr->PlaySoundFromObject(GetLocalContextObject(),
				pszRadioSoundPath, NULL, fOuterRadius, ePriority, dwFlags |PLAYSOUND_CLIENTLOCAL, 
				SMGR_DEFAULT_VOLUME, 
				1.0f, fInnerRadius, SPEECH_SOUND_CLASS, nMixChannel );

			if (pRadioSoundHandle)
			{
				*pRadioSoundHandle = hRadio;
			}
		}
	}
	else
	{
		//if( hSoundRec )
		//{
		//	hSound = g_pClientSoundMgr->PlayDBSoundFromPos( vPos, hSoundRec, SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, dwFlags,
		//		SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
		//		SPEECH_SOUND_CLASS, nMixChannel );
		//}
		//else
		{
			g_pClientSoundMgr->PlaySoundFromPos(vPos, pVoicePath, NULL, fOuterRadius, ePriority,  PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, fInnerRadius,
				SPEECH_SOUND_CLASS, nMixChannel);
		}
		if (bUseRadioSound)
		{
			g_pClientSoundMgr->PlaySoundFromPos(vPos, pszRadioSoundPath, NULL, fOuterRadius, ePriority, PLAYSOUND_CLIENTLOCAL, SMGR_DEFAULT_VOLUME, 1.0f, fInnerRadius,
				SPEECH_SOUND_CLASS, nMixChannel);
		}
	}
	
	if (bSubtitle && (hSound || hRadio))
	{
		float fDuration = -1.0f;
		if (hRadio)
		{
			g_pLTClient->SoundMgr()->GetSoundDuration(hRadio, fDuration);

			//the radio sounds are non-positional, so the subtitles should be as well...
			vPos.Init();
		}
		else
		{
			g_pLTClient->SoundMgr()->GetSoundDuration(hSound, fDuration);
		}
		
		bSubtitle = g_pSubtitles->Show(pszSoundID, vPos, fOuterRadius, fDuration, bSubtitlePriority);
	}

	g_pHUDDialogue->Show(szIcon,INVALID_CLIENT);

	return hSound;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayBroadcast
//
//	PURPOSE:	Play broadcast sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::PlayBroadcast(uint32 nBroadcastId, bool bForce, uint32 nClientID, int8 nPriority, bool bPlayerControlled)
{
	if (!m_cs.bIsPlayer) return;

	bool bIsLocalClient = (g_pLTClient->GetClientObject() == m_hServerObject);
	if (bIsLocalClient && !bForce) return;

	const char* pszSoundID = StringIDFromIndex(nBroadcastId);
	// throw an assert if the string ID was not found
	if( !g_pLTIStringEdit->DoesIDExist(g_pLTDBStringEdit, pszSoundID) )
	{
		g_pLTBase->CPrint( "[PlayBroadcast] - The String ID '%s' does not exist!", pszSoundID );
		LTASSERT_PARAM1( 0, "The String ID '%s' does not exist!", pszSoundID );
		return;
	}

	if (m_nBroadcastPriority >= nPriority && !bPlayerControlled)
		return;

	
	if (m_hDialogueSnd)
	{
		KillLipSyncSound( false );
	}

	m_bPlayerBroadcast = bPlayerControlled;
	m_nBroadcastPriority = nPriority;

	// extract the voice path
	const char* szVoicePath = g_pLTIStringEdit->GetVoicePath( g_pLTDBStringEdit, pszSoundID );

	// throw an assert if the voice path was not found
	if( LTStrEmpty(szVoicePath) || !g_pLTIStringEdit->DoesVoicePathExist(g_pLTDBStringEdit, pszSoundID) )
	{
		g_pLTBase->CPrint( "[PlayBroadcast] - The String ID '%s' does not have a valid voice path!", pszSoundID );
		LTASSERT_PARAM1( 0, "The String ID '%s' does not have a valid voice path!", pszSoundID );
		return;
	}

	SoundPriority ePriority = SOUNDPRIORITY_PLAYER_HIGH;

	HLTSOUND hSound = NULL;

	// Add radio sound stuff here -- Terry
	char pszRadioSoundPath[MAX_PATH];
	char pszRadioSoundExt[MAX_PATH];
	int32 nExtIndex;

	LTStrCpy(pszRadioSoundPath, szVoicePath, MAX_PATH);

	nExtIndex = CResExtUtil::GetExtensionIndex(pszRadioSoundPath);
	if( nExtIndex != -1 )
	{
		LTStrCpy(pszRadioSoundExt, &(pszRadioSoundPath[nExtIndex]), MAX_PATH);
		if (nExtIndex > 0)
		{
			nExtIndex--;	// strip off the '.'
		}
		pszRadioSoundPath[nExtIndex] = 0;
		LTStrCat(pszRadioSoundPath, "R.", MAX_PATH);
		LTStrCat(pszRadioSoundPath, pszRadioSoundExt, MAX_PATH);
	}

	if (bIsLocalClient)
	{
		//if it's our dialogue, play it in our head only
		hSound = g_pClientSoundMgr->PlaySoundFromObject(GetLocalContextObject(),
			szVoicePath, NULL, m_fBroadcastOuterRadius, ePriority, PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENTLOCAL, SMGR_DEFAULT_VOLUME, 
			1.0f, m_fBroadcastInnerRadius, SPEECH_SOUND_CLASS, PLAYSOUND_MIX_SPEECH );
	}
	else
	{
		//if it's someone else's dialogue, play it at their position
		hSound = g_pClientSoundMgr->PlaySoundFromObject(GetLocalContextObject(),
			szVoicePath, NULL, m_fBroadcastOuterRadius, ePriority, PLAYSOUND_GETHANDLE | PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 
			1.0f, m_fBroadcastInnerRadius, SPEECH_SOUND_CLASS, PLAYSOUND_MIX_SPEECH );

		//and play the radio sound in our head
		m_hDialogueRadioSnd = g_pClientSoundMgr->PlaySoundFromObject(GetLocalContextObject(),
			pszRadioSoundPath, NULL, m_fBroadcastOuterRadius, ePriority, PLAYSOUND_GETHANDLE | PLAYSOUND_CLIENTLOCAL, SMGR_DEFAULT_VOLUME, 
			1.0f, m_fBroadcastInnerRadius, SPEECH_SOUND_CLASS, PLAYSOUND_MIX_SPEECH );
	}


	if (!bIsLocalClient)
	{
		HRECORD hIcon = DATABASE_CATEGORY( BroadcastGlobal ).GETRECORDATTRIB( DATABASE_CATEGORY( BroadcastGlobal ).GetGlobalRecord() , Icon );
		if (hIcon)
		{
			const char* pszIcon = g_pLTDatabase->GetRecordName(hIcon);
			g_pHUDDialogue->Show(pszIcon,nClientID);
			m_sDialogueIcon = pszIcon;
		}
	}


	m_hDialogueSnd = hSound;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ShowAttachClientFX
//
//	PURPOSE:	Show all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::ShowAttachClientFX( uint32 nElement )
{
	// Show a specifically indexed clientFX.

	if( nElement != ALL_ATTACH_FX )
	{
		//note that the +1 is to compensate for our dummy head
		CClientFXLink* pLink = m_AttachClientFX.GetElement(nElement + 1);

		if (pLink && pLink->IsValid())
		{
			pLink->GetInstance()->Show();
		}

		return;
	}

	// Show all clientFX.

	for ( CClientFXLinkNode* pCurr = m_AttachClientFX.m_pNext; pCurr; pCurr = pCurr->m_pNext )
	{
		if ( pCurr->m_Link.IsValid() )
		{
			pCurr->m_Link.GetInstance()->Show();
		}
		else
		{
			// when we hit 0, there are no more
			return;
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HideAttachClientFX
//
//	PURPOSE:	Hide all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HideAttachClientFX( uint32 nElement )
{
	// Hide a specifically indexed clientFX.

	if( nElement != ALL_ATTACH_FX )
	{
		//note that the +1 is to compensate for our dummy head
		CClientFXLink* pLink = m_AttachClientFX.GetElement(nElement + 1);

		if (pLink && pLink->IsValid())
		{
			pLink->GetInstance()->Hide();
		}

		return;
	}

	// Hide all clientFX.

	for ( CClientFXLinkNode* pCurr = m_AttachClientFX.m_pNext; pCurr; pCurr = pCurr->m_pNext )
	{
		if ( pCurr->m_Link.IsValid() )
		{
			pCurr->m_Link.GetInstance()->Hide();
		}
		else
		{
			// when we hit 0, there are no more
			return;
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateAttachClientFX
//
//	PURPOSE:	Create all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::CreateAttachClientFX()
{
	if( m_cs.bIsDead )
	{
		return;
	}

	RemoveAttachClientFX();
	
	ModelsDB::HMODEL hModel = GetModel( );

	// Don't do attachments if the model is bad.  The Speaker object
	// can have an invalid model.
	if( !hModel )
		return;

	int nNumClientFX = g_pModelsDB->GetNumPersistentClientFX( hModel );
	for ( int i = 0; i < nNumClientFX ; ++i )
	{
		char const *pClientFXName = g_pModelsDB->GetPersistentClientFXName( hModel, i );
		if ( pClientFXName && ( '\0' != pClientFXName[ 0 ] ) )
		{
			uint32 dwFlags = 0;

			if( g_pModelsDB->GetPersistentClientFXLoop( hModel, i ))
				dwFlags |= FXFLAG_LOOP;

			if( !g_pModelsDB->GetPersistentClientFXSmoothShutdown( hModel, i ))
				dwFlags |= FXFLAG_NOSMOOTHSHUTDOWN;

			CLIENTFX_CREATESTRUCT fxInit( pClientFXName, dwFlags, GetLocalContextObject());

			CClientFXLinkNode* pNewNode = debug_new(CClientFXLinkNode);

			if(pNewNode)
			{
				g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &pNewNode->m_Link, fxInit, true );
				if ( pNewNode->m_Link.IsValid() ) 
				{
					if( g_pModelsDB->GetPersistentClientFXVisible( hModel, i ))
					{
						pNewNode->m_Link.GetInstance()->Show();
					}
					else
					{
						pNewNode->m_Link.GetInstance()->Hide();
					}
					
					// add the link to the list
					m_AttachClientFX.AddToEnd(pNewNode);
				}
			}
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::RemoveAttachClientFX
//
//	PURPOSE:	Destroys all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CCharacterFX::RemoveAttachClientFX()
{
	for ( CClientFXLinkNode* pCurr = m_AttachClientFX.m_pNext; pCurr; pCurr = pCurr->m_pNext )
	{
		if ( pCurr->m_Link.IsValid() )
		{
			g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &pCurr->m_Link );
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::PlayDingSound
//
//	PURPOSE:	Play an impact ding sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::PlayDingSound()
{
	if (IsMultiplayerGameClient())
	{
		for (int i=0; i < MAX_DINGS; i++)
		{
			if( !m_DingTimers[i].IsStarted( ))
			{
				m_DingTimers[i].Start( g_vtDingDelay.GetFloat());
				break;
			}
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::InitLocalPlayer
//
//	PURPOSE:	Initialize the local player
//
// ----------------------------------------------------------------------- //

void CCharacterFX::InitLocalPlayer()
{
	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (m_cs.bIsPlayer && hPlayerObj == m_hServerObject)
	{
		g_pPlayerMgr->InitLocalPlayer( *this );

		if( g_pLTClient->PhysicsSim()->SetObjectPhysicsGroup( m_hServerObject, PhysicsUtilities::ePhysicsGroup_UserPlayer ) != LT_OK )
		{
			LTERROR( "Failed to set the physics group for the Server Player." );
		}

	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacterFX::HandleWeaponSoundMsg
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleWeaponSoundMsg( ILTMessage_Read *pMsg )
{
	uint8 nType			= pMsg->Readuint8();
	HWEAPON hWeaponID	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	HOBJECT hWeapon		= pMsg->ReadObject();
	LTVector vPos( 0, 0, 0 );
	::PlayWeaponSound( hWeaponID, !m_cs.bIsPlayer, vPos, (PlayerSoundId)nType, false, hWeapon );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacterFX::HandleWeaponSoundLoopMsg
//
//  PURPOSE:	Play or stop a looping weapon sound...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleWeaponSoundLoopMsg( ILTMessage_Read *pMsg )
{
	uint8 nType		= pMsg->Readuint8();
	HWEAPON hWeaponID	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	HWEAPONDATA hWeapon = g_pWeaponDB->GetWeaponData(hWeaponID, !m_cs.bIsPlayer);

	if( !hWeapon )
		return;

	HATTRIBUTE hWeaponSoundStruct;
	HATTRIBUTE   hWeaponSound = NULL;
	uint32 nValueIndex = 0;
	HRECORD hSR = NULL;

	if (m_cs.bIsPlayer)
	{
		// play the local one if it's the player...
		hWeaponSoundStruct = g_pWeaponDB->GetAttribute(hWeapon, WDB_WEAPON_LocalSoundInfo);
	}
	else
	{
		hWeaponSoundStruct = g_pWeaponDB->GetAttribute(hWeapon, WDB_WEAPON_NonLocalSoundInfo);
	}

	switch( nType )
	{
	case PSI_RELOAD:														// 1
	case PSI_RELOAD2:														// 2
	case PSI_RELOAD3:														// 3
		{
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct,  0,  WDB_WEAPON_rReloadSnd );
			nValueIndex = nType - PSI_RELOAD;
		}
		break;

	case PSI_SELECT:		hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rSelectSnd );			break;	// 4
	case PSI_DESELECT:		hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rDeselectSnd );		break;	// 5
	case PSI_FIRE:			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0,WDB_WEAPON_rFireSnd );			break;	// 6
	case PSI_DRY_FIRE:		hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0,WDB_WEAPON_rDryFireSnd );		break;	// 7
	case PSI_ALT_FIRE:		hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0,WDB_WEAPON_rAltFireSnd );		break;	// 8
	case PSI_SILENCED_FIRE:	hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0,WDB_WEAPON_rSilencedFireSnd );	break;	// 9

	case PSI_WEAPON_MISC1:													// 10
	case PSI_WEAPON_MISC2:													// 11
	case PSI_WEAPON_MISC3:													// 12
	case PSI_WEAPON_MISC4:													// 13
	case PSI_WEAPON_MISC5:													// 14
		{
			hWeaponSound = g_pWeaponDB->GetStructAttribute(hWeaponSoundStruct, 0, WDB_WEAPON_rMiscSnd );
			nValueIndex = nType - PSI_WEAPON_MISC1;
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
			KillWeaponLoopSound();
		}
		break;
	}
	if (hWeaponSound != NULL)
	{
		hSR = g_pWeaponDB->GetRecordLink(hWeaponSound, nValueIndex);
	}
	
	if( hSR )
	{
		// Stop any previous looping sound...

		KillWeaponLoopSound();

		LTVector	vPos;
		g_pLTClient->GetObjectPos( GetLocalContextObject(), &vPos );

		// Play the sound from the character

		m_hWeaponLoopSound = g_pClientSoundMgr->PlayDBSoundFromObject( GetLocalContextObject(), hSR, SMGR_INVALID_RADIUS,
																	 SOUNDPRIORITY_PLAYER_MEDIUM, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE, 
																	 SMGR_DEFAULT_VOLUME, 1.0f, SMGR_INVALID_RADIUS, WEAPONS_SOUND_CLASS,
																	 PLAYSOUND_MIX_WEAPONS_NONPLAYER);
	}

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacterFX::KillWeaponLoopSound
//
//  PURPOSE:	Kill any looping sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::KillWeaponLoopSound()
{
	if( m_hWeaponLoopSound )
	{
		g_pLTClient->SoundMgr()->KillSound( m_hWeaponLoopSound );
		m_hWeaponLoopSound = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacterFX::KillSlideSound
//
//  PURPOSE:	Kill any looping sound
//
// ----------------------------------------------------------------------- //

void CCharacterFX::KillSlideSound()
{
	if( m_hSlideSound )
	{
		g_pLTClient->SoundMgr()->KillSound( m_hSlideSound );
		m_hSlideSound = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacterFX::UpdatePlayerAimPosition
//
//  PURPOSE:	Update the aim at node tracker with the correct position...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdatePlayerAimPosition( )
{
	// Check if we don't have a rotation interpolator.
	if( !m_pRotationInterpolator ) 
		return;

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if( m_cs.bIsPlayer && hPlayerObj != m_hServerObject )
		m_pRotationInterpolator->Update( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ResetNodeTrackerContext
//
//	PURPOSE:	Reset the node tracking context...
//
// ----------------------------------------------------------------------- //
void CCharacterFX::ResetNodeTrackerContext()
{
	bool bIsLocalClient = (m_cs.bIsPlayer && g_pLTClient->GetClientObject() == m_hServerObject);
		
	// Local clients track through the player body...
	if( bIsLocalClient )
		return;
	
	// Don't do tracking if the skeleton is bad.  The Speaker object
	// can have an invalid skeleton.
	ModelsDB::HSKELETON hSkeleton = g_pModelsDB->GetModelSkeleton(m_cs.hModel );
	if( hSkeleton )
	{
		m_NodeTrackerContext.Init( m_hServerObject, hSkeleton );

		// Players should always try to aim?
		if( m_cs.bIsPlayer )
			EnablePlayerAimTracking( true );
	}

	ModelsDB::HLEAN hLean = g_pModelsDB->GetModelLeanRecord( m_cs.hModel );
	m_LeanNodeController.Init( m_hServerObject, hLean );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ResetNodeBlinkContext
//
//	PURPOSE:	Reset the node blink context...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::ResetNodeBlinkContext()
{
	ModelsDB::HBLINKNODEGROUP hBlinkGroup = g_pModelsDB->GetSkeletonBlinkNodeGroup(g_pModelsDB->GetModelSkeleton( m_cs.hModel ));
 	m_BlinkController.Init(m_hServerObject, hBlinkGroup);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::EnablePlayerAimTracking
//
//	PURPOSE:	Enable / Disable the aim at node tracker...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::EnablePlayerAimTracking( bool bEnable )
{
	if( !m_cs.bIsPlayer )
		return;

	if( m_NodeTrackerContext.IsTrackerGroupActive( kTrackerGroup_AimAt ) == bEnable )
		return;

	if( bEnable )
	{
		m_NodeTrackerContext.EnableTrackerGroup( kTrackerGroup_AimAt );
	}
	else
	{
		m_NodeTrackerContext.DisableTrackerGroup( kTrackerGroup_AimAt, false );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::Render
//
//	PURPOSE:	Render the Character debug info.
//
// ----------------------------------------------------------------------- //
void CCharacterFX::Render(HOBJECT hCamera)
{
	// Draw the name.
	if (!m_Str.IsEmpty() && m_bStrVisible)
	{
		m_Str.Render();
	}
}

// Gets the model record for this index.
ModelsDB::HMODEL CCharacterFX::MPModelIndexToModel( uint8 nMPModelIndex ) const
{
	// Get the model used by this player.
	ModelsDB::HMODEL hModel = NULL;
	// For team games, need to find relationship to local player.
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		// Find the model based on relationship.
		CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if( !pLocalCI )
			return false;
		CLIENT_INFO *pThisCI = g_pInterfaceMgr->GetClientInfoMgr()->GetClientByID( m_cs.nClientID );
		if( !pThisCI )
			return false;
		if( pLocalCI->nTeamID != pThisCI->nTeamID )
		{
			hModel = g_pModelsDB->GetEnemyTeamModel( nMPModelIndex );
		}
		else
		{
			hModel = g_pModelsDB->GetFriendlyTeamModel( nMPModelIndex );
		}
	}
	// For DM games, get dm model.
	else
	{
		hModel = g_pModelsDB->GetDMModel( nMPModelIndex );
	}

	if( !hModel )
	{
		LTERROR( "Invalid model." );
		return NULL;
	}

	return hModel;
}


bool CCharacterFX::IsUsingMPModel( uint8 nMPModelIndex ) const
{
	// Get the model used by this player.
	ModelsDB::HMODEL hModel = MPModelIndexToModel( nMPModelIndex );
	if( !hModel )
		return false;

	char szServerModelName[MAX_PATH];
	const char* szModelName = NULL;
	g_pModelLT->GetModelFilename( GetServerObj(), szServerModelName, LTARRAYSIZE(szServerModelName) );
	if (m_bSeverBody)
	{
		ModelsDB::HSEVERBODY hBody = g_pModelsDB->GetSeverBodyRecord(hModel);
		//we have to force the model change here so that the sockets are correct...
		szModelName = g_pModelsDB->GetBodyModelFile(hBody);
	}
	else
	{
		szModelName = g_pModelsDB->GetModelFilename( hModel );
	}

	// Only change the model if it's different than the team we think it should be...
	if( m_cs.hModel && LTStrIEquals( szServerModelName, szModelName ))
	{
		return true;
	}

	return false;
}

void CCharacterFX::ChangeMPModel( uint8 nMPModelIndex )
{
	// Get the model used by this player.
	ModelsDB::HMODEL hModel = MPModelIndexToModel( nMPModelIndex );
	if( !hModel )
		return;

	m_cs.nMPModelIndex = nMPModelIndex;

	char szServerModelName[MAX_PATH];
	const char* szModelName = NULL;
	g_pModelLT->GetModelFilename( GetServerObj(), szServerModelName, LTARRAYSIZE(szServerModelName) );
	if (m_bSeverBody)
	{
		ModelsDB::HSEVERBODY hBody = g_pModelsDB->GetSeverBodyRecord(hModel);
		//we have to force the model change here so that the sockets are correct...
		szModelName = g_pModelsDB->GetBodyModelFile(hBody);
	}
	else
	{
		szModelName = g_pModelsDB->GetModelFilename( hModel );
	}

	// Only change the model if it's different than the team we think it should be...
	if( m_cs.hModel && LTStrIEquals( szServerModelName, szModelName ))
	{
		return;
	}

	m_cs.hModel = hModel;

	uint8 nTrk[2] = {kAD_TRK_Upper,kAD_TRK_Lower};
	HMODELANIM hAni[2] = {INVALID_MODEL_ANIM,INVALID_MODEL_ANIM };
	HMODELWEIGHTSET hWt[2] = {INVALID_MODEL_WEIGHTSET,INVALID_MODEL_WEIGHTSET };
	uint32 nTime[2] = { 0, 0 };

	ObjectCreateStruct theStruct;
	if (m_hDeathFX)
	{
		const char* pFilename = g_pModelsDB->GetDeathFXModelFilename(m_hDeathFX);
		if( pFilename )
		{
			theStruct.SetFileName(pFilename);
		}


		if( g_pModelsDB->GetDeathFXNumMaterials(m_hDeathFX) )
		{
			g_pModelsDB->CopyDeathFXMaterialFilenames(m_hDeathFX, theStruct.m_Materials[0], LTARRAYSIZE( theStruct.m_Materials ), 
				LTARRAYSIZE( theStruct.m_Materials[0] ));
		}

	}
	else if (m_bSeverBody)
	{
		if (m_cs.bIsPlayer)
		{
			for (uint8 n = 0; n < 2; ++n)
			{
				g_pModelLT->GetWeightSet(GetServerObj(),nTrk[n],hWt[n]);
				g_pModelLT->GetCurAnim(GetServerObj(), nTrk[n], hAni[n]);
				if (hAni[n] != INVALID_MODEL_ANIM)
				{
					g_pModelLT->GetCurAnimTime(GetServerObj(), nTrk[n], nTime[n] );
				}
			}

		}

		ModelsDB::HSEVERBODY hBody = g_pModelsDB->GetSeverBodyRecord(hModel);
		theStruct.SetFileName(szModelName);
		g_pModelsDB->CopyBodyMaterialFilenames(hBody, theStruct.m_Materials[0], LTARRAYSIZE( theStruct.m_Materials ), 
			LTARRAYSIZE( theStruct.m_Materials[0] ));


	}
	else
	{
		theStruct.SetFileName(szModelName);
		g_pModelsDB->CopyMaterialFilenames( hModel, theStruct.m_Materials[0], LTARRAYSIZE( theStruct.m_Materials ), 
			LTARRAYSIZE( theStruct.m_Materials[0] ));
	}


	g_pCommonLT->SetObjectFilenames(GetServerObj(), &theStruct);


	if (m_bSeverBody && m_cs.bIsPlayer)
	{
		for (uint8 n = 0; n < 2; ++n)
		{
			g_pModelLT->AddTracker(GetServerObj(),nTrk[n], true);
			g_pModelLT->SetWeightSet(GetServerObj(),nTrk[n],hWt[n]);
			g_pModelLT->SetCurAnim(GetServerObj(), nTrk[n], hAni[n],false);
			if (hAni[n] != INVALID_MODEL_ANIM)
			{
				g_pModelLT->SetCurAnimTime(GetServerObj(), nTrk[n], nTime[n] );
			}
			g_pModelLT->SetPlaying(GetServerObj(), nTrk[n], true);
		}
	}
	// Reset our group, since we lost it with the model change.
	if( m_cs.bIsDead )
	{
		g_pLTBase->PhysicsSim( )->SetObjectPhysicsGroup( GetServerObj(), PhysicsUtilities::ePhysicsGroup_UserRagDoll );
	}
	else
	{
		g_pLTBase->PhysicsSim( )->SetObjectPhysicsGroup( GetServerObj(), PhysicsUtilities::ePhysicsGroup_UserPlayer );
	}

	CreateAttachClientFX();

	// The model skeleton may have changed so reset the node tracker context...
	ResetNodeTrackerContext();
	ResetNodeBlinkContext();

	InitDisplays();
}

//step through the things attached to us and see if we should hide any of them
void CCharacterFX::UpdateAttachments()
{
	HLOCALOBJ attachList[20];
	uint32 dwListSize = 0;
	uint32 dwNumAttach = 0;

	g_pCommonLT->GetAttachments(m_hServerObject, attachList, 20, dwListSize, dwNumAttach);
	uint32 nCharFlags = 0;
	g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_Flags, nCharFlags);
	bool bCharVisible = !!( nCharFlags & FLAG_VISIBLE );
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;

	EEngineLOD eShadowLOD = eEngineLOD_Low;
	g_pLTClient->GetObjectShadowLOD( m_hServerObject, eShadowLOD );

	for (int i=0; i < nNum; i++)
	{
		uint32 dwUsrFlags;
		g_pCommonLT->GetObjectFlags(attachList[i], OFT_User, dwUsrFlags);
		
		if( !bCharVisible || (g_pVersionMgr->IsLowViolence() && (dwUsrFlags & USRFLG_GORE)) )
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, 0, FLAG_VISIBLE);
			g_pLTClient->SetObjectShadowLOD( attachList[i], eEngineLOD_Never );
		}
		else if( bCharVisible )
		{
			g_pCommonLT->SetObjectFlags(attachList[i], OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			g_pLTClient->SetObjectShadowLOD( attachList[i], eShadowLOD );
		}
	}

	m_bUpdateAttachments = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::KillLipSyncSound
//
//	PURPOSE:	Kill the lipsync sound.
//
// ----------------------------------------------------------------------- //

void CCharacterFX::KillLipSyncSound( bool bSendNotification )
{
	if( m_hDialogueSnd )
	{
		g_pLTClient->SoundMgr()->KillSound(m_hDialogueSnd);
		m_hDialogueSnd = NULL;
	}

	if( m_hDialogueRadioSnd )
	{
		g_pLTClient->SoundMgr()->KillSound(m_hDialogueRadioSnd);
		m_hDialogueRadioSnd = NULL;
	}

	if (m_bSubtitle)
	{
		g_pSubtitles->Clear();
	}

	if (!m_sDialogueIcon.empty())
	{
		g_pHUDDialogue->Hide(m_sDialogueIcon.c_str());
		m_sDialogueIcon = "";
	}

	if( bSendNotification )
	{
		// Tell the server that the sound finished.
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_OBJECT_MESSAGE );
		cMsg.WriteObject( m_hServerObject );
		cMsg.Writeuint32( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_CHARACTER_ID );
		cMsg.WriteBits(CFX_DIALOGUE_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint8( m_nUniqueDialogueId );
		cMsg.Writeint16( 0 );	// mix channel 
		cMsg.Writebool( false ); // UseRadioSound;
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	}

	m_nBroadcastPriority = -1;

	m_bPlayerBroadcast = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ShutdownDamageFX
//
//	PURPOSE:	Shutdown any damageFX on this character...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::ShutdownDamageFX()
{
	// Shutdown any damageFX...

	if( m_link1stPersonInstFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_link1stPersonInstFX );
		m_psz1stPersonInstFXName = NULL;
	}

	if( m_link3rdPersonInstFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_link3rdPersonInstFX );
		m_psz3rdPersonInstFXName = NULL;
	}

	if( !m_p3rdPersonDamageFX )
		return;
	
	for( uint32 nCurrDamage = 0; nCurrDamage < m_nNum3rdPersonDamageFX; ++nCurrDamage )
	{
		if( m_p3rdPersonDamageFX[nCurrDamage].IsValid() )
		{
			g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_p3rdPersonDamageFX[nCurrDamage] );
		}
	}	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::OnObjectRotate
//
//	PURPOSE:	Gaive the characterFX a chance to modify the rotation...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::OnObjectRotate( LTRotation *pRot )
{
	if( !pRot )
		return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateHUDData
//
//	PURPOSE:	Update the navmarker assoicated with this character, if there is one..
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateHUDData()
{
	//no hud marker, nothing to do
	uint8 nId = m_cs.nClientID;
	if( nId == INVALID_CLIENT )
		return;

	CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
	CLIENT_INFO* pCI = pCIMgr->GetClientByID(nId);

	if (pCI)
	{
		if (m_pHUDItem)
		{
			HUDNavMarker_create HUDData;
			HUDData.m_bIsActive = true;
			HUDData.m_hTarget = m_hServerObject;
			HUDData.m_hType = m_hHUDType;
			HUDData.m_nTeamId = pCI->nTeamID;
			HUDData.m_pText = pCI->sName.c_str();
			HUDData.m_pCharFx = this;
			HUDData.m_pOverrideTex = pCI->sInsignia.c_str();

			m_pHUDItem->UpdateData(&HUDData);

		}
		if (m_pNameDisplay) 
		{
			m_pNameDisplay->SetText(pCI->sName.c_str());
			m_pNameDisplay->UpdateDisplay(false);
		}
		if (m_pInsigniaDisplay) 
		{
			m_pInsigniaDisplay->SetTexture(pCI->sInsignia.c_str());
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleChat
//
//	PURPOSE:	Flash the navmarker assoicated with this character, if there is one...
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleChat()
{
	//no hud marker, nothing to do
	if (!m_pHUDItem) return;

	m_pHUDItem->Flash("Chat");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::InitDead
//
//	PURPOSE:	Set the FX up as a dead char
//
// ----------------------------------------------------------------------- //

void CCharacterFX::InitDead()
{
	// Remove use from some detectors
	ObjectDetector::ReleaseLink( m_iFocusLink );
	ObjectDetector::ReleaseLink( m_iAttackPredictionLink );

	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	bool bIsLocalClient = ( m_cs.bIsPlayer && hPlayerObj == GetServerObj() );
	if( bIsLocalClient )
	{
		// Move the serverobj over to the client's object since it will do the ragdoll.
		LTRigidTransform tfPlayerTransform;
		g_pLTClient->GetObjectTransform(g_pMoveMgr->GetObject(), &tfPlayerTransform);
		g_pLTClient->SetObjectTransform(m_hServerObject, tfPlayerTransform);
	}

	if (m_pHUDItem)
	{
		m_pHUDItem->Flash("Death");
	}

	m_wsInfoString = L"";
	m_Str.SetText(NULL);

	m_cs.bIsDead = true;
	if( m_cs.bIsPlayer && IsMultiplayerGameClient()  )
	{
		g_pHUDMgr->QueueUpdate( kHUDPlayers );
	}

	ShutdownDamageFX();
	RemoveSlowMoFX();
	RemoveAttachClientFX();

	if( m_linkLoopFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkLoopFX );
	}

	// Play a deathfx on the body based on the damage type that killed the character... 

	DamageFlags	nDeathDamageFlag = DamageTypeToFlag(m_cs.eDeathDamageType);
	DAMAGEFX *pDamageFX = g_pDamageFXMgr->GetFirstDamageFX();

	while( pDamageFX )
	{
		if( pDamageFX->m_nDamageFlag & nDeathDamageFlag )
		{
			if( pDamageFX->m_sz3rdPersonDeathFXName[0] )
			{
				CLIENTFX_CREATESTRUCT fxInit( pDamageFX->m_sz3rdPersonDeathFXName, 0, m_hServerObject );
				g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );
			}
		}

		pDamageFX = g_pDamageFXMgr->GetNextDamageFX();
	}

	uint32 nNumDFX = g_pModelsDB->GetNumDeathFX(m_cs.hModel);
	HRECORD hDFX = NULL;
	//step through our death FX until we get a match or we run out
	for (uint32 nDFX = 0; hDFX == NULL && nDFX < nNumDFX; ++nDFX )
	{
		hDFX = g_pModelsDB->GetDeathFXRecord(m_cs.hModel,nDFX);
		//if the damage type doesn't match the kind that killed us, clear the record so we keep looking
		if (hDFX && g_pModelsDB->GetDeathFXDamageType(hDFX) != m_cs.eDeathDamageType )
		{
			hDFX = NULL;
		}
	}

	if (hDFX)
	{
		bool bApply = false;


		ObjectCreateStruct createstruct;
		const char* pFilename = g_pModelsDB->GetDeathFXModelFilename(hDFX);
		if( pFilename )
		{
			createstruct.SetFileName(pFilename);
			bApply = true;
		}


		if( g_pModelsDB->GetDeathFXNumMaterials(hDFX) )
		{
			g_pModelsDB->CopyDeathFXMaterialFilenames(hDFX, createstruct.m_Materials[0], LTARRAYSIZE( createstruct.m_Materials ), 
				LTARRAYSIZE( createstruct.m_Materials[0] ));
			bApply = true;
		}

		if (bApply)
		{
			m_hDeathFX = hDFX;
			g_pCommonLT->SetObjectFilenames(m_hServerObject, &createstruct);
		}
	}

	// Check if we're fading bodies on death.
	if( g_vtBodyLifetime.GetFloat( ) >= 0.0f && !m_cs.bPermanentBody )
	{
		// Take a little bit of time to get there.
		float fBodyLifetime = LTMAX( g_vtBodyLifetime.GetFloat( ), 0.01f );
		m_FadeTimer.Start( fBodyLifetime );
	}

	// Now that we're fully dead, make sure the scoreboard is updated correctly.
	if( IsMultiplayerGameClient( ))
	{
		CLIENT_INFO *pThisCI = g_pInterfaceMgr->GetClientInfoMgr()->GetClientByID( m_cs.nClientID );
		g_pInterfaceMgr->GetClientInfoMgr()->UpdateClientSort( pThisCI );
		g_pHUDMgr->QueueUpdate( kHUDScores );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ClearDead
//
//	PURPOSE:	Clear any states set by becoming dead
//
// ----------------------------------------------------------------------- //

void CCharacterFX::ClearDead()
{
	// Register with some detectors
	HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	bool bIsLocalClient = ( m_cs.bIsPlayer && hPlayerObj == GetServerObj() );

	if( !bIsLocalClient )
	{
		g_iFocusObjectDetector.RegisterObject( m_iFocusLink, GetServerObj(), this );
		g_iAttackPredictionObjectDetector.RegisterObject( m_iAttackPredictionLink, GetServerObj(), this );
	}

	const char* pPhysicsWeightSetName = g_pModelsDB->GetDefaultPhysicsWeightSet( m_cs.hModel );
	if( pPhysicsWeightSetName )
	{
		if( m_cs.bIsPlayer )
		{
			m_CharacterPhysics.Reset( pPhysicsWeightSetName, PhysicsUtilities::ePhysicsGroup_UserPlayer, IsMultiplayerGameClient( ));
		}
		else
		{
			m_CharacterPhysics.Reset( pPhysicsWeightSetName, PhysicsUtilities::ePhysicsGroup_UserAI, false );
		}
	}

	if( m_hWallStickObject )
	{
		g_pLTClient->RemoveObject( m_hWallStickObject );
		m_hWallStickObject = NULL;
	}


	if (m_pHUDItem)
	{
		m_pHUDItem->Flash("Respawn");
	}

	m_cs.bIsDead = false;
	m_FadeTimer.Stop();
	if( m_cs.bIsPlayer && IsMultiplayerGameClient()  )
	{
		g_pHUDMgr->QueueUpdate( kHUDPlayers );
	}

	//remove any of our parts that we severed...
	ObjRefVector::iterator orvIt = m_hSeveredParts.begin();
	while (orvIt != m_hSeveredParts.end())
	{
		g_pLTClient->RemoveObject(*orvIt);
		orvIt++;
	}

	m_bSeverBody = false;
	EnablePlayerAimTracking( true );	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::UpdateDead
//
//	PURPOSE:	Update any thing related to being dead
//
// ----------------------------------------------------------------------- //

void CCharacterFX::UpdateDead()
{

	if( m_bInitRagDoll )
	{
		// Should we ragdoll?
		if( m_CharacterPhysics.CanRagDoll( ))
		{
			m_bInitRagDoll = false;

			m_CharacterPhysics.RagDollDeath( m_cs.vDeathDir, m_cs.fDeathImpulseForce, m_cs.hModelNodeLastHit,
											 m_cs.fDeathNodeImpulseForceScale, true, false);

			if( g_pModelsDB->GetModelCanWallStick(m_cs.hModel) && 
				m_CharacterPhysics.StickToGeometry( m_cs.hModelNodeLastHit, m_cs.vDeathDir, m_cs.hDeathAmmo ))
			{
				HAMMODATA hDeathAmmoData = g_pWeaponDB->GetAmmoData( m_cs.hDeathAmmo, !m_cs.bIsPlayer );
				HPHYSICSRIGIDBODY hWallStickRigidBody = m_CharacterPhysics.GetWallStickRigidBody( );
				if( hDeathAmmoData && hWallStickRigidBody )
				{
					PropsDB::HPROP hProp = (PropsDB::HPROP)g_pWeaponDB->GetRecordLink( hDeathAmmoData, WDB_AMMO_rWallStickProp );
					if( hProp )
					{
						// Create stuck to geoemtry model...
						ObjectCreateStruct ocs;
						ocs.m_ObjectType = OT_MODEL;

						ocs.SetFileName( g_pPropsDB->GetPropFilename( hProp ));
						ocs.m_NextUpdate = (0.001f);

						LTRigidTransform tWallStickTrans;
						g_pLTClient->PhysicsSim( )->GetRigidBodyTransform( hWallStickRigidBody, tWallStickTrans );
                        ocs.m_Pos = tWallStickTrans.m_vPos;
						
						LTVector vF = m_cs.vDeathDir;
						if( vF != LTVector::GetIdentity( ))
							vF.Normalize( );

						ocs.m_Rotation = LTRotation( -vF, LTVector(0.0f, 1.0f, 0.0f));
						ocs.m_Flags =  FLAG_DONTFOLLOWSTANDING | FLAG_REMOVEIFOUTSIDE | FLAG_VISIBLE | FLAG_POINTCOLLIDE | FLAG_NOSLIDING | FLAG_TOUCH_NOTIFY;
						g_pPropsDB->CopyMaterialFilenames( hProp, ocs.m_Materials[0], LTARRAYSIZE( ocs.m_Materials ), LTARRAYSIZE( ocs.m_Materials[0] ));
						ocs.m_Scale = 1.0f;

						// Allocate an object...
						m_hWallStickObject = g_pLTClient->CreateObject( &ocs );
					}
				}
			}
		}
	}

	//update fading
	if (m_FadeTimer.IsStarted())
	{
		if (m_FadeTimer.IsTimedOut()) 
		{
			g_pLTClient->Common()->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
			g_pLTClient->SetObjectShadowLOD( m_hServerObject, eEngineLOD_Never );
			m_bUpdateAttachments = true;
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::HandleSeverMsg
//
//	PURPOSE:	Handle a message from the server telling us we've lost a limb
//
// ----------------------------------------------------------------------- //

void CCharacterFX::HandleSeverMsg(ILTMessage_Read *pMsg)
{
	// Our nodetracker is no longer valid since it was for the previous unsevered body.
	m_NodeTrackerContext.Term();

	HitLocation eHitLoc = (HitLocation)pMsg->Readint8();
	LTVector vDeathDir = pMsg->ReadCompLTPolarCoord();
	float fForce = pMsg->Readfloat();


	ModelsDB::HMODEL hModel = m_cs.hModel;

	//low violence means we down want to see severing either...
	if (!g_pProfileMgr->GetCurrentProfile()->m_bGore)
	{
		//fire the GibFX off... (the FX system will handle playing a low violence version)
		const char* pszGibFX = g_pModelsDB->GetGibFX(hModel);
		if ( !LTStrEmpty(pszGibFX))
		{
			CLIENTFX_CREATESTRUCT	fxInit( pszGibFX, NULL, m_hServerObject );
			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, fxInit, true );
		}

		//hide the body, since we don't want to see the severing at all
		g_pCommonLT->SetObjectFlags(m_hServerObject, OFT_Flags, 0, FLAG_VISIBLE);
		g_pLTClient->SetObjectShadowLOD( m_hServerObject, eEngineLOD_Never );
		m_bUpdateAttachments = true;
		m_bSeverBody = true;
		return;
	}


	ModelsDB::HSEVERBODY hBody = g_pModelsDB->GetSeverBodyRecord(hModel);
	uint32 nNumPieces = g_pModelsDB->GetBodyNumPieces(hBody);
	ModelsDB::HSEVERPIECE hPiece = NULL;
	//iterate through all of our pieces to see if any were severed...
	for (uint32 nPiece = 0; nPiece < nNumPieces && !hPiece; ++nPiece)
	{
		//get the piece
		ModelsDB::HSEVERPIECE hTestPiece = g_pModelsDB->GetBodyPiece(hBody,nPiece);
		if (hTestPiece)
		{
			//get the associated location
			HitLocation eTestLoc = g_pModelsDB->GetSPLocation(hTestPiece);
			if (eTestLoc == eHitLoc) 
			{
				hPiece = hTestPiece;
			}
		}
	}


	//spawn a model for the severed piece...
	const char *pszModel = g_pModelsDB->GetSPModelFile(hPiece);
	if (!LTStrEmpty(pszModel))
	{
		LTTransform tf;
		g_pLTClient->GetObjectTransform(m_hServerObject,&tf);

		ModelsDB::HNODE hNode = g_pModelsDB->GetSPSourceNode(hPiece);
		if (hNode)
		{
			const char* szNodeName = g_pModelsDB->GetNodeName( hNode );
			if( szNodeName )
			{
				LTRESULT ltResult;
				HMODELNODE hModelNode;
				ltResult = g_pModelLT->GetNode(m_hServerObject, szNodeName, hModelNode);
				if ( ltResult == LT_OK )
				{
					g_pModelLT->GetNodeTransform(m_hServerObject, hModelNode, tf, true);
					tf.m_vPos.y += 50.0f;
				}

			}
		}

		ObjectCreateStruct ocs;
		ocs.m_ObjectType = OT_MODEL;
		ocs.SetFileName(pszModel);
		ocs.m_Pos = tf.m_vPos;
		ocs.m_NextUpdate = (0.001f);
		ocs.m_Rotation = tf.m_rRot;
		ocs.m_Flags =  FLAG_GRAVITY | FLAG_DONTFOLLOWSTANDING | FLAG_REMOVEIFOUTSIDE | FLAG_VISIBLE | FLAG_POINTCOLLIDE | FLAG_NOSLIDING | FLAG_TOUCH_NOTIFY;
		g_pModelsDB->CopySPMaterialFilenames( hPiece, ocs.m_Materials[0], LTARRAYSIZE( ocs.m_Materials ), LTARRAYSIZE( ocs.m_Materials[0] ));

		// Allocate an object...
		HOBJECT hObj = g_pLTClient->CreateObject(&ocs);

		if (hObj)
		{
			m_hSeveredParts.push_back(hObj);

			PhysicsUtilities::SetPhysicsWeightSet(hObj, PhysicsUtilities::WEIGHTSET_RIGID_BODY, true);

			//setup our collision properties
			HRECORD hCollisionProperty = g_pLTDatabase->GetRecord( DATABASE_CATEGORY( CollisionProperty ).GetCategory(), "Gore" );
			uint32 nUserFlags = CollisionPropertyRecordToUserFlag( hCollisionProperty );
			g_pLTClient->Common( )->SetObjectFlags( hObj, OFT_User, nUserFlags, USRFLG_COLLISIONPROPMASK );


			uint32 nNumBodies = 0;
			if(g_pLTBase->PhysicsSim()->GetNumModelRigidBodies(hObj, nNumBodies) != LT_OK)
				return;

			LTVector vAng( GetRandom(-20.0f,20.0f),GetRandom(-20.0f,20.0f),GetRandom(-30.0f,30.0f));

			for(uint32 nCurrBody = 0; nCurrBody < nNumBodies; nCurrBody++)
			{
				HPHYSICSRIGIDBODY hRigidBody;
				if(g_pLTBase->PhysicsSim()->GetModelRigidBody(hObj, nCurrBody, hRigidBody) != LT_OK)
					continue;

				//now apply the force to it
				LTVector vImpulse = (vDeathDir * fForce);
				if( LTIsNaN( vImpulse ) || vImpulse.MagSqr() > 1000000.0f * 1000000.0f )
				{
					LTERROR( "Invalid impulse detected." );
					vImpulse.Init( 0.0f, 10.0f, 0.0f );
				}
				g_pLTBase->PhysicsSim()->ApplyRigidBodyImpulseWorldSpace(hRigidBody, tf.m_vPos, vImpulse);


				//apply a little spin to each body
				g_pLTBase->PhysicsSim()->SetRigidBodyAngularVelocity(hRigidBody,vAng);

				//and make sure it's solid
				g_pLTBase->PhysicsSim()->SetRigidBodySolid(hRigidBody,true);

				//and release our rigid body handle
				g_pLTBase->PhysicsSim()->ReleaseRigidBody(hRigidBody);
			}		



			g_pCommonLT->SetObjectFlags(hObj, OFT_User, SurfaceToUserFlag(g_pModelsDB->GetFleshSurfaceType(m_cs.hModel)), 0XFF000000);


			const char *pszPartFX = g_pModelsDB->GetSPPartFX(hPiece);
			if (!LTStrEmpty(pszPartFX))
			{
				CLIENTFX_CREATESTRUCT	fxInit( pszPartFX, NULL, hObj );

				HMODELSOCKET hAttachSocket = INVALID_MODEL_SOCKET;
				if (LT_OK == g_pModelLT->GetSocket(m_hServerObject,g_pModelsDB->GetSPPartFXSocket(hPiece),hAttachSocket))
				{
					fxInit.m_hSocket = hAttachSocket;
				}

				g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, fxInit, true );

			}


		}

	}

	
	//we have to force the model change here so that the sockets are correct...
	if (!m_bSeverBody)
	{
		m_bSeverBody = true;


		ObjectCreateStruct createstruct;
		const char* pFilename = g_pModelsDB->GetBodyModelFile(hBody);
		if( !pFilename )
		{
			LTERROR( "CCharacter::PrepareToSever: Invalid model." );
			return;
		}

		createstruct.SetFileName(pFilename);
		g_pModelsDB->CopyBodyMaterialFilenames(hBody, createstruct.m_Materials[0], LTARRAYSIZE( createstruct.m_Materials ), 
			LTARRAYSIZE( createstruct.m_Materials[0] ));

		g_pCommonLT->SetObjectFilenames(m_hServerObject, &createstruct);


		// Reset our group, since we lost it with the model change.
		if( m_cs.bIsDead )
		{
			g_pLTBase->PhysicsSim( )->SetObjectPhysicsGroup( m_hServerObject, PhysicsUtilities::ePhysicsGroup_UserRagDoll );
		}
		else
		{
			g_pLTBase->PhysicsSim( )->SetObjectPhysicsGroup( m_hServerObject, PhysicsUtilities::ePhysicsGroup_UserPlayer );
		}


		// The model skeleton may have changed so reset the node tracker context...
		EnablePlayerAimTracking( false );
		ResetNodeBlinkContext();


		if (m_cs.bIsPlayer)
		{
			uint8 nNumTrk = pMsg->Readuint8();
			for (uint8 n = 0; n < nNumTrk; ++n)
			{
				ANIMTRACKERID trkID = pMsg->Readuint8();
				HMODELWEIGHTSET hWeightSet = pMsg->Readuint32();
				HMODELANIM hAnim = pMsg->Readuint32();
				uint32 nTime = pMsg->Readuint32();

				// Add the tracker to the player model...
				g_pModelLT->AddTracker( m_hServerObject, trkID, true );
				g_pModelLT->SetWeightSet( m_hServerObject, trkID, hWeightSet );
				g_pModelLT->SetCurAnim(m_hServerObject, trkID, hAnim, false);
				g_pModelLT->SetCurAnimTime(m_hServerObject, trkID, nTime);

			}
		}
		

	}

	const char *pszBodyFX = g_pModelsDB->GetSPBodyFX(hPiece);
	if (!LTStrEmpty(pszBodyFX))
	{
		CLIENTFX_CREATESTRUCT	fxInit( pszBodyFX, NULL, m_hServerObject );

		HMODELSOCKET hAttachSocket = INVALID_MODEL_SOCKET;
		if (LT_OK == g_pModelLT->GetSocket(m_hServerObject,g_pModelsDB->GetSPBodyFXSocket(hPiece),hAttachSocket))
		{
			fxInit.m_hSocket = hAttachSocket;
		}

		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, fxInit, true );

	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::InitDisplays
//
//	PURPOSE:	initialize the custom displays
//
// ----------------------------------------------------------------------- //

void CCharacterFX::InitDisplays()
{
	if( !IsMultiplayerGameClient( )) return;

	uint32 nId = m_cs.nClientID;
	CClientInfoMgr* pCIMgr = g_pInterfaceMgr->GetClientInfoMgr();
	CLIENT_INFO* pCI = pCIMgr->GetClientByID(nId);
	if (!pCI)
	{
		return;
	}


	if (!m_pNameDisplay)
	{
		m_pNameDisplay = debug_new(CharacterDisplay);
	}
	if (m_pNameDisplay) 
	{
		m_pNameDisplay->Init(this,g_pModelsDB->GetModelNameDisplay(GetModel()),g_vNameSz,&(pCI->sDisplayData));
		if (pCI)
		{
			m_pNameDisplay->SetText(pCI->sName.c_str());
			m_pNameDisplay->UpdateDisplay(false);
		}
	}

	if (!m_pInsigniaDisplay) 
	{
		m_pInsigniaDisplay = debug_new(CharacterDisplaySimple);
	}
	if (m_pInsigniaDisplay) 
	{
		m_pInsigniaDisplay->Init(this,g_pModelsDB->GetModelInsigniaDisplay(GetModel()),&(pCI->sDisplayData));

		if (pCI)
		{
			if (!LTStrEmpty(pCI->sInsignia.c_str()))
			{
				m_pInsigniaDisplay->SetTexture(pCI->sInsignia.c_str());
			}
			else
			{
				m_pInsigniaDisplay->SetTexture(ClientDB::Instance( ).GetString( ClientDB::Instance( ).GetClientSharedRecord( ), CDB_sClientShared_DefaultPatch ));
			}
		}
	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::ResetDisplays
//
//	PURPOSE:	Reset the custom displays
//
// ----------------------------------------------------------------------- //

void CCharacterFX::ResetDisplays()
{
	if (m_pNameDisplay) 
	{
		m_pNameDisplay->UpdateDisplay(false);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::IsLocalClient
//
//	PURPOSE:	Does this FX belong to the local client...
//
// ----------------------------------------------------------------------- //

bool CCharacterFX::IsLocalClient() const
{ 
	if (!m_cs.bIsPlayer) return false;

	uint32 nLocalID;
	g_pLTClient->GetLocalClientID( &nLocalID );

	return ( m_cs.nClientID == nLocalID );

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::GetLocalContextObject
//
//	PURPOSE:	This will get the object to use for the local context.  So, if the characterfx
//				is the local player, it will return the playerbody object, otherwise, 
//				it will return the server's hobject of the characterfx.
// ----------------------------------------------------------------------- //
HOBJECT CCharacterFX::GetLocalContextObject( )
{
	if( IsLocalClient())
	{
		return g_pMoveMgr->GetObject();
	}
	else
	{
		return GetServerObj( );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::IsPlayingBroadcast
//
//	PURPOSE:	Are we broadcasting a message...
//
// ----------------------------------------------------------------------- //

bool CCharacterFX::IsPlayingBroadcast() const
{ 
	return (m_hDialogueSnd != NULL && m_bPlayerBroadcast);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateSlowMoFX()
//
//	PURPOSE:	Create the 3rd person client fx for being in slow mo
//
// ----------------------------------------------------------------------- //
void CCharacterFX::CreateSlowMoFX(const char *pszFX)
{
	if( m_linkSlowMoFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkSlowMoFX );
	}

	CLIENTFX_CREATESTRUCT fxInit( pszFX, FXFLAG_LOOP, GetLocalContextObject());
	g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_linkSlowMoFX, fxInit, true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacterFX::CreateSlowMoFX()
//
//	PURPOSE:	Remove the 3rd person client fx for being in slow mo
//
// ----------------------------------------------------------------------- //
void CCharacterFX::RemoveSlowMoFX()
{
	if( m_linkSlowMoFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkSlowMoFX );
	}
}


// EOF
