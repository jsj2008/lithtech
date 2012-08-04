// ----------------------------------------------------------------------- //
//
// MODULE  : DamageFXMgr.cpp
//
// PURPOSE : Damage FX Manager class - Implementation
//
// CREATED : 1/20/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "stdafx.h"
	#include "DamageFXMgr.h"
	#include "VarTrack.h"
	#include "SoundMgr.h"
	#include "HUDMgr.h"
	#include "GameClientShell.h"
	#include "CommandIDs.h"
	#include "MsgIDs.h"
	#include "CMoveMgr.h"
	#include "VehicleMgr.h"
	#include "ClientWeaponMgr.h"
	#include "PlayerCamera.h"
	#include "bindmgr.h"
	#include "LadderMgr.h"
	#include "DamageFxDB.h"

//
// Defines...
//

#define KEY_DAMAGEFX							"DAMAGE_FX_KEY"

//
// Globals...
//

	CDamageFXMgr	*g_pDamageFXMgr = NULL;

	VarTrack		g_vtEnableDamageFX;


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::CDamageFXMgr
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CDamageFXMgr::CDamageFXMgr()
:	CGameDatabaseMgr				( ),
	m_bAllowMovement			( true ),
	m_nDisableWeaponCounts		( 0 ),
	m_hDamageFXCat				( NULL )
{
    m_lstDamageFx.Init( true );
	m_lstActiveDmgFx.Init( false );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::~CDamageFXMgr
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CDamageFXMgr::~CDamageFXMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::Term
//
//  PURPOSE:	Clean up after ourselfs
//
// ----------------------------------------------------------------------- //

void CDamageFXMgr::Term()
{
	g_pDamageFXMgr = NULL;

	m_lstDamageFx.Clear();
	m_lstActiveDmgFx.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDamageFXMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

bool CDamageFXMgr::Init(const char *szDatabaseFile /* = DB_Default_File  */)
{
	if( !OpenDatabase( szDatabaseFile ))
		return false;

	// Set the global ptr

	g_pDamageFXMgr = this;

	// Get handles to all of the categories in the database...
	m_hDamageFXCat = DATABASE_CATEGORY( DamageFxDB ).GetCategory();

	// Read in the properties for each Damage FX type...
	uint32 nNumRec = g_pLTDatabase->GetNumRecords(m_hDamageFXCat);

	for (uint32 n = 0; n < nNumRec; n++)
	{
		DAMAGEFX	*pDamageFX = debug_new( DAMAGEFX );
		if( pDamageFX && pDamageFX->Init( g_pLTDatabase->GetRecordByIndex(m_hDamageFXCat,n) ))
		{
			m_lstDamageFx.AddTail( pDamageFX );
		}
		else
		{
			debug_delete( pDamageFX );
			return false;
		}
	}

	g_vtEnableDamageFX.Init( g_pLTClient, "EnableDamageFX", NULL, 1.0f );

    return true;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CDamageFXMgr::GetDamageFX
//
//  PURPOSE:	Get the specified DamageFX record
//
// ----------------------------------------------------------------------- //

DAMAGEFX *CDamageFXMgr::GetDamageFX( uint32 nID )
{
	DAMAGEFX **pCur = NULL;

	pCur = m_lstDamageFx.GetItem( TLIT_FIRST );

	while( pCur )
	{
		if( *pCur && (*pCur)->m_nID == nID )
		{
			return *pCur;
		}

		pCur = m_lstDamageFx.GetItem( TLIT_NEXT );
	}

	return NULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	*CDamageFXMgr::GetDamageFX
//
//  PURPOSE:	Get the specified DamageFX record
//
// ----------------------------------------------------------------------- //

DAMAGEFX *CDamageFXMgr::GetDamageFX( char *pName )
{
	DAMAGEFX **pCur = NULL;

	pCur = m_lstDamageFx.GetItem( TLIT_FIRST );

	while( pCur )
	{
		if( *pCur && (*pCur)->m_szName[0] && (!LTStrICmp( (*pCur)->m_szName, pName )) )
		{
			return *pCur;
		}

		pCur = m_lstDamageFx.GetItem( TLIT_NEXT );
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::Update
//
//  PURPOSE:	Update all active damage fx
//
// ----------------------------------------------------------------------- //

void CDamageFXMgr::Update()
{
	if( g_vtEnableDamageFX.GetFloat() < 1.0f )
	{
		DAMAGEFX *pDamageFX = GetFirstDamageFX();
		while( pDamageFX )
		{
			pDamageFX->m_vtTestFX.SetFloat( 0.0f );

			pDamageFX = GetNextDamageFX();
		}

		Clear();
		return;
	}

	//if we are using an external camera, make sure to clear anything that might be causing rendering
	//issues
	if( g_pPlayerMgr->GetPlayerCamera()->GetCameraMode() == CPlayerCamera::kCM_Cinematic )
	{
		//and we don't need to update
		return;
	}

	//don't bother updating if we are paused, or using an alternate camera
	if(!g_pInterfaceMgr->IsInGame( ) ||g_pGameClientShell->IsGamePaused())
		return;

	m_bAllowMovement	= true;
	m_bAllowInput		= true;
	bool bHadDisableWeaponCounts = ( m_nDisableWeaponCounts > 0 );
	m_nDisableWeaponCounts = 0;

	// Update all active Damage FX...
	
	DAMAGEFX *pDamageFX = GetFirstActiveFX();
	float fFrameTime = ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS();
	while( pDamageFX )
	{
		pDamageFX->Update(fFrameTime);
		
		pDamageFX = GetNextActiveFX();
	}

	// Check if we should try to enable our weapons.
	if( m_nDisableWeaponCounts == 0 && bHadDisableWeaponCounts )
	{
		g_pPlayerMgr->GetClientWeaponMgr()->EnableWeapons();
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::Clear
//
//  PURPOSE:	Stop every active damage fx
//
// ----------------------------------------------------------------------- //

void CDamageFXMgr::Clear()
{
	// Stop all Damage FX...
	
	DAMAGEFX *pDamageFX = GetFirstDamageFX();
	while( pDamageFX )
	{
		pDamageFX->Stop( false );
	
		pDamageFX = GetNextDamageFX();
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::IsDamageActive
//
//  PURPOSE:	Given damage flags, find if we are taking damage of that type
//
// ----------------------------------------------------------------------- //

bool CDamageFXMgr::IsDamageActive( DamageFlags nDmgFlag )
{
	// Detrimine if any damage fx related to the passed in flags are active...
	
	DAMAGEFX *pDamageFX = GetFirstActiveFX();
	while( pDamageFX )
	{
		if( nDmgFlag & pDamageFX->m_nDamageFlag )
		{
			return true;
		}

		pDamageFX = GetNextActiveFX();
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::SetDamageFXAllowMovement
//
//  PURPOSE:	Sets weather or not any FX doesn't allow movement.
//
// ----------------------------------------------------------------------- //

void CDamageFXMgr::SetDamageFXMovementAndInput( bool bMove, bool bInput, bool bAllowWeapons )
	{
	m_bAllowMovement = bMove;
	m_bAllowInput = bInput;
	if( !bAllowWeapons )
		m_nDisableWeaponCounts++;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CDamageFXMgr::OnModelKey
//
//  PURPOSE:	Handle model key
//
// ----------------------------------------------------------------------- //

bool CDamageFXMgr::OnModelKey( HLOCALOBJ hObj, ArgList *pArgs )
{
	if (!hObj || !pArgs || !pArgs->argv || pArgs->argc == 0) return false;

	char* pKey = pArgs->argv[0];
	if (!pKey) return false;

	if( LTStrICmp( pKey, KEY_DAMAGEFX ) == 0 )
	{
		// Start or Stop FX if they are controlled by the animation...

		if( (pArgs->argc > 1) && pArgs->argv[1] )
		{
			if( LTStrICmp( pArgs->argv[1], "START") == 0 )
			{
				DAMAGEFX *pDamageFX = GetFirstActiveFX();
				while( pDamageFX )
				{
					if( pDamageFX->m_bAnimationControlsFX )
					{
						pDamageFX->StartSoundAndVisuals();
					}

					pDamageFX = GetNextActiveFX();
				}
			}
			else if( LTStrICmp( pArgs->argv[1], "STOP" ) == 0 )
			{
				DAMAGEFX *pDamageFX = GetFirstActiveFX();
				while( pDamageFX )
				{
					if( pDamageFX->m_bAnimationControlsFX )
					{
						pDamageFX->StopSoundAndVisuals();
					}

					pDamageFX = GetNextActiveFX();
				}
			}
		}

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::DAMAGEFX
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DAMAGEFX::DAMAGEFX()
:	m_nID					( INVALID_GAME_DATABASE_INDEX ),
	m_nDamageFlag			( 0 ),
	m_szName				( NULL ),
	m_szStartSound			( NULL ),
	m_szLoopSound			( NULL ),
	m_szFXName				( NULL ),
	m_szTakingHealthFXName	( NULL ),
	m_szTakingArmorFXName	( NULL ),
	m_sz3rdPersonFXName		( NULL ),
	m_sz1stPersonInstFXName	( NULL ),
	m_sz3rdPersonInstFXName	( NULL ),
	m_sz1stPersonDeathFXName( NULL ),
	m_sz3rdPersonDeathFXName( NULL ),
	m_szBodyFXName			( NULL ),
	m_fRotMax				( 0.0f ),
	m_fRotSpeed				( 0.0f ),
	m_fRotDir				( 0.0f ),
	m_fOffsetRot			( 0.0f ),
	m_fMaxRot				( 0.0f ),
	m_fMinRot				( 0.0f ),
	m_fMoveMult				( 0.0f ),
	m_fMinFXPercent			( 0.0f ),
	m_bActive				( false ),
	m_bFade					( false ),
	m_fFadeTm				( 0.0f ),
	m_bAllowMovement		( true ),
	m_bAllowInput			( true ),
	m_bAllowWeapons			( true ),
	m_hLoopSound			( NULL ),
	m_bJumpRequested		( false ),
	m_nNumJumps				( 0 ),
	m_nJumpsToEscape		( -1 ),
	m_bAttachCameraToAni	( false ),
	m_bShowClientModel		( false ),
	m_bAnimationControlsFX	( false ),
	m_bUpdateSoundAndVisuals( false ),
	m_fElapsedTime			( 0.0f ),
	m_fEndTime				( 0.0f )
{
	m_szVarTrackName[0] = '\0';
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::~DAMAGEFX
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

DAMAGEFX::~DAMAGEFX()
{
	if( m_hLoopSound )
	{
		g_pLTClient->SoundMgr()->KillSound( m_hLoopSound );
		m_hLoopSound = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::Init
//
//  PURPOSE:	Build the DamageFX struct
//
// ----------------------------------------------------------------------- //

bool DAMAGEFX::Init( HDAMAGEFX hDX )
{
	if( !hDX ) return false;

	m_szName					= DATABASE_CATEGORY( DamageFxDB ).GetRecordName(hDX);
	m_szStartSound				= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,StartSound);
	m_szLoopSound				= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX, LoopSound);
	m_szFXName					= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX, FirstPersonFX);
	m_szTakingHealthFXName		= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX, TakingHealthFXName);
	m_szTakingArmorFXName		= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX, TakingArmorFXName);
	m_sz3rdPersonFXName			= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX, ThirdPersonFX);
	m_sz1stPersonInstFXName		= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX, FirstPersonInstantFX);
	m_sz3rdPersonInstFXName		= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX, ThirdPersonInstantFX);
	m_sz1stPersonDeathFXName	= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX, FirstPersonDeathFX);
	m_sz3rdPersonDeathFXName	= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX, ThirdPersonDeathFX);
	m_szBodyFXName				= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX, BodyFX);

	m_fFadeTm			= (float)DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,FadeTime);
	

	m_fRotMax			= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,RotationMax);
	m_fRotSpeed			= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,RotationSpeed);

	m_fMinFXPercent		= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,MinFXPercent);
	m_fMinFXPercent		= LTCLAMP( m_fMinFXPercent, 0.0f, 1.0f );

	m_bAllowMovement	= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,AllowMovement);
	m_bAllowInput		= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,AllowInput);

	m_nJumpsToEscape	= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,NumJumpsToEscape);

	m_bInstantEffect	= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,InstantEffect);

	m_bAttachCameraToAni= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,AttachCameraToAni);
	m_bShowClientModel	= DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,ShowClientModel);

	m_bAnimationControlsFX = DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,AnimationControlsFX);

	m_bRemoveOnNextInstantDamage = DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,RemoveOnNextInstantDamage);
	m_bRenewOnNextInstantDamage = DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,RenewOnNextInstantDamage);

	// Set up the flag for testing against this DamageFX...
	HRECORD hDT = DATABASE_CATEGORY( DamageFxDB ).GETRECORDATTRIB(hDX,DamageType);
	m_nDamageFlag = g_pDTDB->GetDamageFlag(hDT);

	// Init the VarTrack for easy testing...

	LTSNPrintF( m_szVarTrackName, LTARRAYSIZE( m_szVarTrackName ), "Test%sFX", m_szName );

	m_vtTestFX.Init( g_pLTClient, m_szVarTrackName, NULL, 0.0f );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::Start
//
//  PURPOSE:	Start the appropriate fx for either the local or non local characters...
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::Start( )
{
	if( m_bActive || !g_pMoveMgr || !g_pHUDMgr || !g_pDamageFXMgr || 
		!g_pPlayerMgr->IsPlayerAlive() || g_pPlayerMgr->IsSpectating())
		return;

	m_bActive = true;

	// Add ourselves to the global active list

	g_pDamageFXMgr->AddToActive( this );

	// Should we start sound and visual fx now or wait for the animation...

	m_bUpdateSoundAndVisuals = !m_bAnimationControlsFX || LadderMgr::Instance().IsClimbing();
	if( m_bUpdateSoundAndVisuals )
	{
		StartSoundAndVisuals();
	}

	// Clear our jump count

	m_nNumJumps = 0;

	m_bAllowWeapons = true;

	if( !g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() )
	{
		if( m_bShowClientModel )
			g_pCommonLT->SetObjectFlags( g_pLTClient->GetClientObject(), OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

		if( m_bAttachCameraToAni )
		{
			g_pPlayerMgr->GetPlayerCamera()->AttachToTarget( true, IsMultiplayerGameClient() );
			m_bAllowWeapons = false;
			g_pPlayerMgr->GetClientWeaponMgr()->DisableWeapons();
			
			// Send message to server so all clients can stop the sound...
			// An id of invalid means stop

			CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if( pWeapon )
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_WEAPON_SOUND_LOOP );
				cMsg.Writeuint8( PSI_INVALID );
				cMsg.WriteDatabaseRecord( g_pLTDatabase, pWeapon->GetWeaponRecord() );
				g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
			}
		}

		if (!m_bAllowInput)
			g_pPlayerMgr->AllowPlayerMovement(false);

	}

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::Stop
//
//  PURPOSE:	Stop the appropriate fx for either the local or non local characters...
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::Stop( bool bFade /* = true  */ )
{
	if( !m_bActive || !g_pPlayerMgr )
		return;

	m_bActive = false;
	m_bUpdateSoundAndVisuals = false;
	
	if( !m_bAnimationControlsFX || !g_pPlayerMgr->IsPlayerAlive() || !g_pPlayerMgr->IsSpectating() || 
		LadderMgr::Instance().IsClimbing() )
	{
		StopSoundAndVisuals( bFade );
	}

	// We no longer want to see the model

	if( m_bShowClientModel )
		g_pCommonLT->SetObjectFlags( g_pLTClient->GetClientObject(), OFT_Flags, 0, FLAG_VISIBLE );

	if( m_bAttachCameraToAni )
	{
		// Reset the player movement before detaching the camera from the head...

		if (!m_bAllowInput)
			g_pPlayerMgr->AllowPlayerMovement(true);

		g_pPlayerMgr->GetPlayerCamera()->AttachToTarget( false );

		if( !m_bAllowWeapons )
		g_pPlayerMgr->GetClientWeaponMgr()->EnableWeapons();
		
		CClientWeapon *pWeapon = g_pPlayerMgr->GetClientWeaponMgr()->GetCurrentClientWeapon();
		if( pWeapon )
		{
			pWeapon->ClearFiring();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::Update
//
//  PURPOSE:	Update an active DamageFX
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::Update( float fElapsedTime )
{
	if( !g_pPlayerMgr || !g_pMoveMgr || !g_pDamageFXMgr )
		return;
	
	HLOCALOBJ hCamera = g_pPlayerMgr->GetPlayerCamera()->GetCamera();
	if( !hCamera ) 
		return;

	//update our elapsed time
	m_fElapsedTime += fElapsedTime;

	// ABM 5/6/02 Quick check on non-looping FX to see if they need to terminate
	if (m_bInstantEffect && m_DamageFXInstance.IsValid())
	{
		if (m_DamageFXInstance.GetInstance()->IsDone())
		{
			Stop();
			return;
		}
	}

	// If we want to see the model make sure to continualy set it visible (it gets set invisible elsewhere)...

	if( m_bActive && m_bShowClientModel && !g_pMoveMgr->GetVehicleMgr()->IsVehiclePhysics() )
		g_pCommonLT->SetObjectFlags( g_pLTClient->GetClientObject(), OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

	// See if we can free ourselfs by jumping (Glue Bomb, Bear Trap)...

	if( (m_nJumpsToEscape > 0) && m_bActive )
	{
		if( CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_JUMP ) && !m_bJumpRequested )
		{
			// Tap

			m_bJumpRequested = true;
			++m_nNumJumps;
		}
		else if( !CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_JUMP ))
		{
			// Ok the player let go of the jump key... let them tap again

			m_bJumpRequested = false;
		}

		if( m_nNumJumps >= m_nJumpsToEscape )
		{
			// End the effect on the client

			Stop();

			// Send message to server to clear progressive damage.

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_CLEAR_PROGRESSIVE_DAMAGE );
			cMsg.WriteType( m_nDamageFlag );
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
			
		}
	}
	
	float fMove		= g_pPlayerMgr->GetMoveMgr()->GetMovementPercent();
	float fFrameTime	= ObjectContextTimer( g_pMoveMgr->GetServerObject( )).GetTimerElapsedS( );

	m_fMoveMult = m_fMinFXPercent + (fMove * (1.0f - m_fMinFXPercent));

	if( m_bActive && m_bUpdateSoundAndVisuals )
	{
		// ROTATION

		float fRotSpeed = m_fRotSpeed * fFrameTime * m_fRotDir;

		m_fOffsetRot += fRotSpeed;
		if( m_fOffsetRot >= m_fMaxRot )
		{
			m_fOffsetRot = m_fMaxRot;
			m_fRotDir = -1.0f;
			m_fMinRot = -m_fRotMax * GetRandom( 0.5f, 1.0f );
		}
		else if( m_fOffsetRot <= m_fMinRot )
		{
			m_fOffsetRot = m_fMinRot;
			m_fRotDir = 1.0f;
			m_fMaxRot = m_fRotMax * GetRandom( 0.5f, 1.0f );
		}

	}
	else if( m_bFade )
	{
		bool	bDone = false;
		
		if( m_fElapsedTime >= m_fEndTime )
		{
			bDone = true;
		}
		else
		{
			// ROTATION

			float fRotSpeed = m_fRotSpeed * fFrameTime;

			if( m_fOffsetRot < -fRotSpeed )
			{
				m_fOffsetRot += fRotSpeed;
				bDone = false;
			}
			else if( m_fOffsetRot > fRotSpeed )
			{
				m_fOffsetRot -= fRotSpeed;
				bDone = false;
			}
			else
			{
				m_fOffsetRot = 0.0f;
			}

		}
	
		if( bDone )
			m_bFade = false;
	}
	else
	{
		m_fOffsetRot	= 0.0f;
	}

	{
		g_pPlayerMgr->GetPlayerCamera()->ApplyLocalRotationOffset( LTVector( 0.0f, 0.0f, m_fOffsetRot * m_fMoveMult ));
	}

	// Set our movement and input...
	g_pDamageFXMgr->SetDamageFXMovementAndInput( m_bAllowMovement, m_bAllowInput, m_bAllowWeapons );

	if( !IsActive() )
	{
		g_pHUDMgr->QueueUpdate( kHUDDamage );
		
		// Remove from the Activelist

		g_pDamageFXMgr->RemoveFromActive( this );
	}
}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::StartSoundAndVisuals
//
//  PURPOSE:	Start fx for sound and visuals
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::StartSoundAndVisuals()
{
	if( !m_bActive ) 
		return;

	// We can now update the FX..

	m_bUpdateSoundAndVisuals = true;

	if( m_szStartSound[0] )
	{
		// Play the start sound

		g_pClientSoundMgr->PlaySoundLocal( m_szStartSound, SOUNDPRIORITY_PLAYER_MEDIUM );
	}

	if( m_szLoopSound[0] && !m_hLoopSound )
	{
		// Play the looping sound..

		uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_LOOP | PLAYSOUND_CLIENT;
		m_hLoopSound = g_pClientSoundMgr->PlaySoundLocal( m_szLoopSound, SOUNDPRIORITY_PLAYER_LOW, dwFlags );
	}

	m_fElapsedTime = 0.0f;

	// Create the FXEdit created FX...

	if( m_szFXName[0] )
	{
		CLIENTFX_CREATESTRUCT	fxInit( m_szFXName, FXFLAG_LOOP, g_pMoveMgr->GetObject() ); 
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(&m_DamageFXInstance, fxInit, true );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DAMAGEFX::StartSoundAndVisuals
//
//  PURPOSE:	Stop fx for sound and visuals
//
// ----------------------------------------------------------------------- //

void DAMAGEFX::StopSoundAndVisuals( bool bFade /* = true  */ )
{
	m_bFade = bFade;
	m_bUpdateSoundAndVisuals = false;

	m_fElapsedTime = 0.0f;
	m_fEndTime = m_fFadeTm;

	m_vtTestFX.SetFloat( 0.0f );

	if( m_hLoopSound )
	{
		g_pLTClient->SoundMgr()->KillSound( m_hLoopSound );
		m_hLoopSound = NULL;
	}

	if( m_DamageFXInstance.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_DamageFXInstance );
	}	
}

