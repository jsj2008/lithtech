// ----------------------------------------------------------------------- //
//
// MODULE  : TurretFX.cpp
//
// PURPOSE : Client side representation on Turret objets, used for activation and weapons...
//
// CREATED : 07/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "TurretFX.h"
#include "CharacterFX.h"
#include "CMoveMgr.h"
#include "LTEulerAngles.h"
#include "PlayerCamera.h"
#include "ClientWeaponMgr.h"
#include "ActivationData.h"
#include "PlayerBodyMgr.h"
#include "AimMgr.h"
#include "FlashLight.h"
#include "SpecialFXNotifyMessageHandler.h"


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::CTurretFX
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

CTurretFX::CTurretFX( )
:	CSpecialFX		( ),
	m_csTurret		( ),
	m_fxLoop		( ),
	m_fxDamage		( )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::~CTurretFX
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

CTurretFX::~CTurretFX( )
{
	// If we were the player's turret, be sure the player is 
	// no longer using us!
	if( g_pPlayerMgr && this == g_pPlayerMgr->GetTurret() )
	{
		ReleasePlayer();
	}
	
	if (m_csTurret.m_hOperatingObject && g_pGameClientShell->GetSFXMgr())
	{
		CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(m_csTurret.m_hOperatingObject);
		if (pCharacter) 
		{
			pCharacter->SetOperatingTurret(NULL);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::Init
//
//  PURPOSE:	Initialize the client side turret class...
//
// ----------------------------------------------------------------------- //

bool CTurretFX::Init( HLOCALOBJ hServObj, ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::Init( hServObj, pMsg ))
		return false;

	uint32		dwInitialMsgPos = pMsg->Tell( ) - 8;

	m_csTurret.Read( pMsg );

	// Don't allow this message to be processed if the firedfrom is valid
	// but can't be read yet or if we don't have a local client yet.
	if(( m_csTurret.m_bOperatingObjectValid && m_csTurret.m_hOperatingObject == INVALID_HOBJECT ) ||
		( m_csTurret.m_bTurretWeaponValid && m_csTurret.m_hTurretWeapon == INVALID_HOBJECT ) ||
		( m_csTurret.m_bOperatingObjectValid && g_pLTClient->GetClientObject() == INVALID_HOBJECT ))
	{
		// The object depends another object but it is not yet available on the client.
		// Add the message to the dependent message list for object polling.
		uint32 nCurPos = pMsg->Tell( );
		pMsg->SeekTo( dwInitialMsgPos );

		SpecialFXNotifyMessageHandler::Instance().AddMessage( *pMsg, hServObj );

		pMsg->SeekTo( nCurPos );

		// Don't create the object until everything is valid.
		return false;
	}

	// This likely never happens, but if somehow the FX gets created with
	// the player as an operator we want to make sure that playermgr
	// is aware of the situation.
	if( m_csTurret.m_hOperatingObject)
	{
		if (m_csTurret.m_hOperatingObject == g_pLTClient->GetClientObject( ))
		{
			g_pPlayerMgr->SetOperatingTurret(this);
		}
		else
		{
			CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(m_csTurret.m_hOperatingObject);
			if (pCharacter) 
			{
				pCharacter->SetOperatingTurret(this);
			}
		}
		
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::Update
//
//  PURPOSE:	Update the weapon associated with the turret...
//
// ----------------------------------------------------------------------- //

bool CTurretFX::Update( )
{
	if( !CSpecialFX::Update( ))
		return false;

	// Is the local client operating the turret...
	if( m_csTurret.m_hOperatingObject == g_pLTClient->GetClientObject( ))
	{
		// Hide server weapon and make turret non-solid...
		g_pCommonLT->SetObjectFlags( m_csTurret.m_hTurretWeapon, OFT_Flags, 0, FLAG_VISIBLE );
		g_pCommonLT->SetObjectFlags( m_hServerObject, OFT_Flags, 0, FLAG_SOLID );

		g_pLTClient->SetObjectShadowLOD( m_csTurret.m_hTurretWeapon, eEngineLOD_Never );
		g_pLTClient->SetObjectShadowLOD( m_hServerObject, eEngineLOD_Never );

		// Establish the turret weapon's transform...
		CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon( );
		if( pWeapon && pWeapon->GetAnimationProperty( ) == kAP_WEAP_Turret )
		{
			HMODELSOCKET hTurretPivot = INVALID_MODEL_SOCKET;
			g_pModelLT->GetSocket( m_hServerObject, "Pivot", hTurretPivot );

			// Cache the player camera...
			CPlayerCamera *pPlayerCamera = g_pPlayerMgr->GetPlayerCamera( );

			if( hTurretPivot != INVALID_MODEL_SOCKET )
			{
				LTTransform tPivot;
				if( g_pModelLT->GetSocketTransform( m_hServerObject, hTurretPivot, tPivot, true ) == LT_OK )
				{
					// Clamp the rotation of the camera, and hence the turet weapon...
					LTVector2 v2PitchRange = g_pWeaponDB->GetVector2( m_csTurret.m_hTurret, WDB_TURRET_v2PitchRange );
					float fYawLimit = g_pWeaponDB->GetFloat( m_csTurret.m_hTurret, WDB_TURRET_fYawLimit );

					v2PitchRange.x = DEG2RAD( v2PitchRange.x );
					v2PitchRange.y = DEG2RAD( v2PitchRange.y );

					LTVector2 v2YawRange( DEG2RAD(fYawLimit), -DEG2RAD(fYawLimit) );

					pPlayerCamera->SetCameraClamping( tPivot.m_rRot, v2PitchRange, v2YawRange );

					tPivot.m_rRot = pPlayerCamera->GetCameraRotation( );					
					pWeapon->SetWeaponTransform( tPivot );
				}
			}

			// Start the zoom on the turret weapon...
			if( !AimMgr::Instance( ).IsAiming( ))
				AimMgr::Instance( ).BeginAim( );

			if( m_fxLoop.IsValid( ))
				m_fxLoop.GetInstance( )->Show( );
		
			// Update the camera so the position is accurate for this update...
			pPlayerCamera->SetTargetObject( pWeapon->GetRightHandModel( ));
			pPlayerCamera->Update( );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::OnServerMessage
//
//  PURPOSE:	Handle a message recieved from the server side Turret object...
//
// ----------------------------------------------------------------------- //

bool CTurretFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return false;

	uint8 nMsgId = pMsg->Readuint8( );
	switch( nMsgId )
	{
		case kTurretFXMsg_All:
		{
			const HOBJECT hOldOperator = m_csTurret.m_hOperatingObject;

			m_csTurret.Read( pMsg );

			if( m_csTurret.m_hOperatingObject != hOldOperator )
			{
				// Did the turret just release the player?
				if( hOldOperator )
				{
					if (hOldOperator == g_pLTClient->GetClientObject())
					{
						ReleasePlayer();
					}
					else
					{
						CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(hOldOperator);
						if (pCharacter) 
						{
							pCharacter->SetOperatingTurret(NULL);
						}
					}
				}
				
				// Did the turret just gain the player?
				// This will happen from loading a saved game.
				if( m_csTurret.m_hOperatingObject )
				{
					if (m_csTurret.m_hOperatingObject == g_pLTClient->GetClientObject())
					{
						g_pPlayerMgr->SetOperatingTurret(this);
					}
					else
					{
						CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(m_csTurret.m_hOperatingObject);
						if (pCharacter) 
						{
							pCharacter->SetOperatingTurret(this);
						}
					}
					
				}
			}
		}
		break;

		case kTurretFXMsg_Deactivate:
		{
			if( m_csTurret.m_hOperatingObject == g_pLTClient->GetClientObject( ))
			{
				ReleasePlayer();
			}
			else
			{
				CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(m_csTurret.m_hOperatingObject);
				if (pCharacter) 
				{
					pCharacter->SetOperatingTurret(NULL);
				}

			}

			m_csTurret.m_hOperatingObject = NULL;
		}
		break;

		case kTurretFXMsg_RemoteActivate:
		{
			Activate( );
		}
		break;

		case kTurretFXMsg_SwitchToTurret:
		{
			// Activate the next turret...
			HOBJECT hNextTurret = pMsg->ReadObject( );
			if( hNextTurret )
			{
				CTurretFX *pNextTurretFX = g_pGameClientShell->GetSFXMgr( )->GetTurretFX( hNextTurret );
				pNextTurretFX->Activate( );

				// Send deactivation message to server...
				CActivationData data;
				data.m_hTarget = hNextTurret;
				data.m_nType = MID_ACTIVATE_TURRET;

				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_PLAYER_ACTIVATE );
				data.Write( cMsg );
				g_pLTClient->SendToServer( cMsg.Read( ), MESSAGE_GUARANTEED );
			}

			// Deactivate this turret...
			if( m_csTurret.m_hOperatingObject == g_pLTClient->GetClientObject( ))
			{
				// Show server weapon and make solid again...
				g_pCommonLT->SetObjectFlags( m_csTurret.m_hTurretWeapon, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
				g_pCommonLT->SetObjectFlags( m_hServerObject, OFT_Flags, FLAG_SOLID, FLAG_SOLID );

				g_pLTClient->SetObjectShadowLOD( m_csTurret.m_hTurretWeapon, eEngineLOD_Medium );
				if( !g_pWeaponDB->GetBool( m_csTurret.m_hTurret, WDB_TURRET_bHideBase ))
					g_pLTClient->SetObjectShadowLOD( m_hServerObject, eEngineLOD_Medium );

				ShutdownClientFX( );
			}
			else
			{
				CCharacterFX* pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(m_csTurret.m_hOperatingObject);
				if (pCharacter) 
				{
					pCharacter->SetOperatingTurret(NULL);
				}

			}

			m_csTurret.m_hOperatingObject = NULL;

		}
		break;

		case kTurretFXMsg_Damage:
		{	
			m_csTurret.m_nDamageState = pMsg->Readuint32( );
			SetDamageState( );
		}
		break;

		default:
		break;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::CanActivate
//
//  PURPOSE:	Query to see if the turret can be activated...
//
// ----------------------------------------------------------------------- //

bool CTurretFX::CanActivate( )
{
	// If the turret is in use, or it's supposed to be activated remotely, don't allow activation...
	if( IsInUse( ) || m_csTurret.m_bRemoteActivation )
		return false;

	// Determine if the distance and angle are appropriate for activation...

	LTVector vPlayerPos;
	g_pLTClient->GetObjectPos( g_pMoveMgr->GetObject( ), &vPlayerPos );

	LTRigidTransform tTurret;
	g_pLTClient->GetObjectTransform( m_hServerObject, &tTurret );

	// Make sure the turret is within range...
	if( tTurret.m_vPos.Dist( vPlayerPos ) > g_pWeaponDB->GetFloat( m_csTurret.m_hTurret, WDB_TURRET_fActivateDistance ))
		return false;

	LTVector vDir = (vPlayerPos - tTurret.m_vPos);
	vDir.Normalize( );

	float fAngle = vDir.Dot( tTurret.m_rRot.Forward( ));

	// Make sure the player is at the correct angle...
	if( -fAngle < DEG2RAD( 90.0f - g_pWeaponDB->GetFloat( m_csTurret.m_hTurret, WDB_TURRET_fActivateApproachAngle )))
		return false;

	// The player can activate the turret...
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::CanActivate
//
//  PURPOSE:	Activates the turret and sets the player up for using it...
//
// ----------------------------------------------------------------------- //

bool CTurretFX::Activate( )
{
    if( IsInUse( ))
		return false;

	HOBJECT hPlayerObj = g_pMoveMgr->GetObject( );

	LTRigidTransform tTurret;
	g_pLTClient->GetObjectTransform( m_hServerObject, &tTurret );

	LTVector vZero(0.0f, 0.0f, 0.0f);
	g_pPhysicsLT->SetAcceleration( hPlayerObj, vZero );
	g_pPhysicsLT->SetVelocity( hPlayerObj, vZero );

	// Face the turret...
	g_pPlayerMgr->GetPlayerCamera( )->SetCameraRotation( tTurret.m_rRot );
	
	// If activating normally, position the palyer to use the turret...
	if( !m_csTurret.m_bRemoteActivation )
	{
		g_pLTClient->SetObjectRotation( hPlayerObj, tTurret.m_rRot );

		// Keep the players y value...
		LTVector vPlayerPos;
		g_pLTClient->GetObjectPos( hPlayerObj, &vPlayerPos );

		// Teleport the player into position...
		LTVector vPos = tTurret.m_rRot.RotateVector( g_pWeaponDB->GetVector3( m_csTurret.m_hTurret, WDB_TURRET_vPlayerOffset ));
		vPos.x += tTurret.m_vPos.x;
		vPos.y += vPlayerPos.y;
		vPos.z += tTurret.m_vPos.z;

		g_pLTClient->SetObjectPos( hPlayerObj, vPos );
	}

	CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();
	
	// Setup the Looping ClientFX...
	if( m_fxLoop.IsValid( ))
		pClientFXMgr->ShutdownClientFX( &m_fxLoop );

	const char *pszLoopFX = g_pWeaponDB->GetString( m_csTurret.m_hTurret, WDB_TURRET_sLoopFX );
	if( pszLoopFX && pszLoopFX[0] )
	{
		CLIENTFX_CREATESTRUCT fxCS( pszLoopFX, FXFLAG_LOOP );
		pClientFXMgr->CreateClientFX( &m_fxLoop, fxCS, false );
		m_fxLoop.GetInstance( )->Hide( );
	}

	// Make sure any damage is shown...
	SetDamageState( );

	// Turn off the flashlight...
	g_pPlayerMgr->GetFlashLight( )->TurnOff( );

	// Give the player a chance to do some setup...
	g_pPlayerMgr->SetOperatingTurret( this );
	m_csTurret.m_hOperatingObject = g_pLTClient->GetClientObject( );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::Deactivate
//
//  PURPOSE:	Deactivate the turret...
//
// ----------------------------------------------------------------------- //

bool CTurretFX::Deactivate( )
{
	if( !IsInUse( ))
		return false;

	HOBJECT hPlayerObj = g_pMoveMgr->GetObject( );
	
	// Face the camera in the direction the player is loking...
	LTRotation rRot;
	g_pLTClient->GetObjectRotation( hPlayerObj, &rRot );
	g_pPlayerMgr->GetPlayerCamera( )->SetCameraRotation( rRot );

	// Send deactivation message to server...
	CActivationData data;
	data.m_hTarget = m_hServerObject;
	data.m_nType = MID_ACTIVATE_TURRET;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_PLAYER_ACTIVATE );
	data.Write( cMsg );
	g_pLTClient->SendToServer( cMsg.Read( ), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::ReleasePlayer
//
//  PURPOSE:	Release the player from using this turret...
//
// ----------------------------------------------------------------------- //
void CTurretFX::ReleasePlayer()
{
	// PlayerMgr really can be null (when exitting game), the rest
	// are just for safety.
	if( !g_pPlayerMgr || !g_pCommonLT || !g_pWeaponDB || !g_pLTClient )
		return;

	// Clear the players' cached turret...
	g_pPlayerMgr->SetOperatingTurret( NULL );

	// Show server weapon and make solid again...
	g_pCommonLT->SetObjectFlags( m_csTurret.m_hTurretWeapon, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
	g_pCommonLT->SetObjectFlags( m_hServerObject, OFT_Flags, FLAG_SOLID, FLAG_SOLID );

	g_pLTClient->SetObjectShadowLOD( m_csTurret.m_hTurretWeapon, eEngineLOD_Medium );
	if( !g_pWeaponDB->GetBool( m_csTurret.m_hTurret, WDB_TURRET_bHideBase ))
		g_pLTClient->SetObjectShadowLOD( m_hServerObject, eEngineLOD_Medium );

	// Reset the player camera...
	g_pPlayerMgr->GetPlayerCamera( )->SetTargetObject( g_pMoveMgr->GetObject( ));
	g_pPlayerMgr->GetPlayerCamera( )->ClearCameraClamping( );

	// End the zoom on the turret weapon...
	if( AimMgr::Instance( ).IsAiming( ))
		AimMgr::Instance( ).EndAim( );

	ShutdownClientFX( );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::ShutdownClientFX
//
//  PURPOSE:	Shutdown any ClientFX currently playing...
//
// ----------------------------------------------------------------------- //

void CTurretFX::ShutdownClientFX( )
{
	CClientFXMgr *pClientFXMgr = &g_pGameClientShell->GetSimulationTimeClientFXMgr();

	if( m_fxLoop.IsValid( ))
		pClientFXMgr->ShutdownClientFX( &m_fxLoop );

	if( m_fxDamage.IsValid( ))
		pClientFXMgr->ShutdownClientFX( &m_fxDamage );

	for( CClientFXLinkNode *pCurFX = &m_DamageStateFX; pCurFX; pCurFX = pCurFX->m_pNext )
	{
		if( pCurFX->m_Link.IsValid( ))
			pClientFXMgr->ShutdownClientFX( &pCurFX->m_Link );
	}

	for( CClientFXLinkNode *pCurFX = &m_PersistentDamageStateFX; pCurFX; pCurFX = pCurFX->m_pNext )
	{
		if( pCurFX->m_Link.IsValid( ))
			pClientFXMgr->ShutdownClientFX( &pCurFX->m_Link );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CTurretFX::SetDamageState
//
//  PURPOSE:	Setup the correct damage FX based on the current damage state...
//
// ----------------------------------------------------------------------- //

void CTurretFX::SetDamageState( )
{
	HATTRIBUTE hDamageStateStruct = g_pWeaponDB->GetAttribute( m_csTurret.m_hTurret, WDB_TURRET_DamageState );
	uint32 nNumDamageStates = g_pWeaponDB->GetNumValues( hDamageStateStruct );

	// Play FX for this damage state if it's HealthPercent
	// is in the range (LastHealthPercent, CurHealthPercent]
	
	// Clear any previous damage state clientfx...
	for( CClientFXLinkNode* pCurr = &m_DamageStateFX; pCurr; pCurr = pCurr->m_pNext )
	{
		if( pCurr->m_Link.IsValid( ))
			g_pGameClientShell->GetSimulationTimeClientFXMgr( ).ShutdownClientFX( &pCurr->m_Link );
	}

	HATTRIBUTE hAttrib = g_pWeaponDB->GetStructAttribute( hDamageStateStruct, m_csTurret.m_nDamageState, WDB_TURRET_fxClientFX );
	const char *pszClientFXName = g_pWeaponDB->GetString( hAttrib );

	if( !LTStrEmpty( pszClientFXName ))
	{
		uint32 dwFlags = FXFLAG_LOOP;

		hAttrib = g_pWeaponDB->GetStructAttribute( hDamageStateStruct, m_csTurret.m_nDamageState, WDB_TURRET_bSmoothShutdown );
		if( !g_pWeaponDB->GetBool( hAttrib ))
			dwFlags |= FXFLAG_NOSMOOTHSHUTDOWN;

		CClientFXLinkNode* pNewFX = debug_new(CClientFXLinkNode);
		if( pNewFX )
		{
			CLIENTFX_CREATESTRUCT fxCS( pszClientFXName, dwFlags );
			g_pGameClientShell->GetSimulationTimeClientFXMgr( ).CreateClientFX( &pNewFX->m_Link, fxCS, true );
			if( pNewFX->m_Link.IsValid( ))
			{
				pNewFX->m_Link.GetInstance( )->Show( );

				// Add the new fx to the appropriate list, based on if it persists across states...
				hAttrib = g_pWeaponDB->GetStructAttribute( hDamageStateStruct, m_csTurret.m_nDamageState, WDB_TURRET_bPersistAcrossDamageStates );
				if( g_pWeaponDB->GetBool( hAttrib))
				{
					m_PersistentDamageStateFX.AddToEnd( pNewFX );
				}
				else
				{
					m_DamageStateFX.AddToEnd( pNewFX );
				}
			}
		}
	}


	// Setup the Damage ClientFX...
	if( !m_fxDamage.IsValid( ))
	{
		const char *pszDamageFX = g_pWeaponDB->GetString( m_csTurret.m_hTurret, WDB_TURRET_sDamageFX );
		if( pszDamageFX && pszDamageFX[0] )
		{
			CLIENTFX_CREATESTRUCT fxCS( pszDamageFX, 0 );
			g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_fxDamage, fxCS, true );
		}
	}
}

// EOF
