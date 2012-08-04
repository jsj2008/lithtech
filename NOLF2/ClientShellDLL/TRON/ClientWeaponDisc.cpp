// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeaponDisc.cpp
//
// PURPOSE : Tron specific client-side version of the disc
//
// CREATED : 4/12/02 (was WeaponModel.h)
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ClientWeaponDisc.h"
#include "MsgIDs.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "PlayerStats.h"
#include "RatingMgr.h"
#include "FXButeMgr.h"


//
// Externs
//
extern bool g_bInfiniteAmmo;


namespace
{
	// also defined in ClientWeapon.cpp;
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
	int const INFINITE_AMMO_AMOUNT = 1000;

	// defense animation names
	char *ns_szSwatDefenseAnimationName = "BlockSwat";
	char *ns_szPreHoldDefenseAnimationName = "PreBlock";
	char *ns_szHoldDefenseAnimationName = "Block";
	char *ns_szImpactHoldDefenseAnimationName = "BlockImpact";
	char *ns_szPostHoldDefenseAnimationName = "PostBlock";

	float const CLIENTWEAPONDISC_PROBE_LENGTH = 100000.0f;
};



// ---------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::CClientWeaponDisc()
//
//	PURPOSE:	Constructor
//
// ---------------------------------------------------------------------- //

CClientWeaponDisc::CClientWeaponDisc() :
	  m_nSwatDefenseAni( INVALID_ANI )
	, m_nPreHoldDefenseAni( INVALID_ANI )
	, m_nHoldDefenseAni( INVALID_ANI )
	, m_nImpactHoldDefenseAni( INVALID_ANI )
	, m_nPostHoldDefenseAni( INVALID_ANI )
	, m_bIsDiscActive( false )
	, m_bFireMessageSent( false )
	, m_bDiscNeedsUpdates( false )
	, m_bIgnoreFireKeyframe( false )
	, m_fTimeDiscActive( -1.0f )
{
	// base class redefines
	m_bAutoSwitchEnabled = false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::~CClientWeaponDisc()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CClientWeaponDisc::~CClientWeaponDisc()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::OnMessage()
//
//	PURPOSE:	Process messages from the server
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::OnMessage( uint8 messageID, ILTMessage_Read *pMsg )
{
	switch ( messageID )
	{
		case MID_PROJECTILE:
		{
			// projectile message, send to the hanler routine
			bool bMessageHandled = HandleMessageProjectile( messageID, pMsg );
			if ( bMessageHandled )
			{
				return true;
			}
		}
		break;

		default:
		{
			// only projectile messages should be going to the weapon
			ASSERT( 0 );
		}
		break;
	}  // switch( messageID )

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::UpdateModelState()
//
//	PURPOSE:	Update the model state for the disc
//
// ----------------------------------------------------------------------- //

WeaponState CClientWeaponDisc::UpdateModelState( bool bFire )
{
	WeaponState eRet = W_IDLE;

	// Determine what we should be doing...
	// (mostly updates animations, also
	// updates a couple supporting variables)
	if ( bFire )
	{
		UpdateFiring();
	}
	else if ( g_pLTClient->IsCommandOn( COMMAND_ID_DEFEND ) &&
	          g_pLTClient->IsCommandOn( COMMAND_ID_DEFEND_MODIFIER ) )
	{
		UpdateSwatDefense();
	}
	else if ( g_pLTClient->IsCommandOn( COMMAND_ID_DEFEND ) &&
	          !g_pLTClient->IsCommandOn( COMMAND_ID_DEFEND_MODIFIER ) )
	{
		UpdateHoldDefense();
	}
	else
	{
		UpdateIdle();
	}

	// this is reset here just in case it was set while the hold
	// animation was not playing
	m_bPlayImpactHoldAnimation = false;

	if ( m_bFire )
	{
		// gadgets...always the special case  :-(
		bool bGadgetSpecialCase = m_pAmmo->eType != GADGET;

		// doesn't actually fire, just updates the
		// ammo and clears the m_bFire flag
		eRet = UpdateAmmoFromFire( bGadgetSpecialCase );

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
			m_cbDeselect( m_nWeaponId, m_pcbData );

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
//	ROUTINE:	CClientWeaponDisc::UpdateFiring()
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::UpdateFiring()
{
	m_bCanSetLastFire = true;

	if ( GetState() == W_RELOADING )
	{
		if ( !PlayReloadAnimation() )
		{
			SetState( W_FIRING );
		}
	}
	if ( GetState() == W_IDLE )
	{
		SetState( W_FIRING );
	}
	if ( GetState() == W_SELECT )
	{
		if ( !PlaySelectAnimation() )
		{
			SetState( W_FIRING );
		}
	}
	if ( GetState() == W_DESELECT )
	{
		if ( !PlayDeselectAnimation() )
		{
			SetState( W_FIRING );
		}
	}
	if ( GetState() == W_SWAT_DEFENSE )
	{
		if ( !PlaySwatDefenseAnimation( false ) )
		{
			SetState( W_FIRING );
		}
	}
	if ( GetState() == W_HOLD_DEFENSE )
	{
		if ( !PlayHoldDefenseAnimation( false ) )
		{
			SetState( W_FIRING );
		}
	}
	if ( ( GetState() == W_FIRING ) || ( GetState() == W_FIRING_NOAMMO ) )
	{
		if ( PlayFireAnimation( true ) )
		{
			m_bCanSetLastFire = false;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::UpdateSwatDefense()
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::UpdateSwatDefense()
{
	if ( GetState() == W_RELOADING )
	{
		if ( !PlayReloadAnimation() )
		{
			SetState( W_SWAT_DEFENSE );
		}
	}
	if ( GetState() == W_IDLE )
	{
		SetState( W_SWAT_DEFENSE );
	}
	if ( GetState() == W_SELECT )
	{
		if ( !PlaySelectAnimation() )
		{
			SetState( W_SWAT_DEFENSE );
		}
	}
	if ( GetState() == W_DESELECT )
	{
		if ( !PlayDeselectAnimation() )
		{
			// TODO: the transition from this state should
			// be to W_INACTIVE, which involves making sure
			// making that change here won't negatively
			// affect anything downline
			SetState( W_SWAT_DEFENSE );
		}
	}
	if ( GetState() == W_HOLD_DEFENSE )
	{
		if ( !PlayHoldDefenseAnimation( false ) )
		{
			SetState( W_SWAT_DEFENSE );
		}
	}
	if ( ( GetState() == W_FIRING ) || ( GetState() == W_FIRING_NOAMMO ) )
	{
		if ( !PlayFireAnimation( false ) )
		{
			SetState( W_SWAT_DEFENSE );
		}
	}
	if ( GetState() == W_SWAT_DEFENSE )
	{
		PlaySwatDefenseAnimation( true );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::UpdateHoldDefense()
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::UpdateHoldDefense()
{
	if ( GetState() == W_RELOADING )
	{
		if ( !PlayReloadAnimation() )
		{
			SetState( W_HOLD_DEFENSE );
		}
	}
	if ( GetState() == W_IDLE )
	{
		SetState( W_HOLD_DEFENSE );
	}
	if ( GetState() == W_SELECT )
	{
		if ( !PlaySelectAnimation() )
		{
			SetState( W_HOLD_DEFENSE );
		}
	}
	if ( GetState() == W_DESELECT )
	{
		if ( !PlayDeselectAnimation() )
		{
			// TODO: the transition from this state should
			// be to W_INACTIVE, which involves making sure
			// making that change here won't negatively
			// affect anything downline
			SetState( W_HOLD_DEFENSE );
		}
	}
	if ( ( GetState() == W_FIRING ) || ( GetState() == W_FIRING_NOAMMO ) )
	{
		if ( !PlayFireAnimation( false ) )
		{
			SetState( W_HOLD_DEFENSE );
		}
	}
	if ( GetState() == W_SWAT_DEFENSE )
	{
		PlaySwatDefenseAnimation( true );
	}
	if ( GetState() == W_HOLD_DEFENSE )
	{
		PlayHoldDefenseAnimation( true );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::UpdateIdle
//
//	PURPOSE:	Update the non-firing animation state of the model
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::UpdateIdle()
{
	m_bCanSetLastFire = true;

	if ( GetState() == W_FIRING )
	{
		if ( !PlayFireAnimation( false ) )
		{
			SetState( W_IDLE );
		}
		else
		{
			m_bCanSetLastFire = false;
		}
	}
	if ( GetState() == W_FIRING_NOAMMO )
	{
		SetState( W_IDLE );
	}
	if ( GetState() == W_RELOADING )
	{
		if ( !PlayReloadAnimation() )
		{
			SetState( W_IDLE );
		}
	}
	if ( GetState() == W_SELECT )
	{
		if ( !PlaySelectAnimation() )
		{
			SetState( W_IDLE );
		}
	}
	if ( GetState() == W_DESELECT )
	{
		if ( !PlayDeselectAnimation() )
		{
			m_bWeaponDeselected = true;
			SetState( W_IDLE );
		}
	}
	if ( GetState() == W_SWAT_DEFENSE )
	{
		if ( !PlaySwatDefenseAnimation( false ) )
		{
			SetState( W_IDLE );
		}
	}
	if ( GetState() == W_HOLD_DEFENSE )
	{
		if ( !PlayHoldDefenseAnimation( false ) )
		{
			SetState( W_IDLE );
		}
	}
	if ( GetState() == W_IDLE )
	{
		PlayIdleAnimation();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::InitAnimations()
//
//	PURPOSE:	Initialize the disc specific weapons
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::InitAnimations( bool bAllowSelectOverride )
{
	// Swat defense animations
	m_nSwatDefenseAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szSwatDefenseAnimationName );

	// Hold defense animations
	m_nPreHoldDefenseAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szPreHoldDefenseAnimationName );
	m_nHoldDefenseAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szHoldDefenseAnimationName );
	m_nImpactHoldDefenseAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szImpactHoldDefenseAnimationName );
	m_nPostHoldDefenseAni = g_pLTClient->GetAnimIndex( m_hObject, ns_szPostHoldDefenseAnimationName );

	// init the base class animations
	CClientWeapon::InitAnimations( bAllowSelectOverride );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::IsSwatDefenseAni()
//
//	PURPOSE:	Returns true if the passed animation is a swat defense animation
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::IsSwatDefenseAni( uint32 dwAni )
{
	ASSERT( INVALID_ANI != m_nPreHoldDefenseAni );
	if ( m_nSwatDefenseAni == dwAni )
	{
		return true;
	}
	else
	{
		return false;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::GetSwatDefenseAni()
//
//	PURPOSE:	Returns the swat defense animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeaponDisc::GetSwatDefenseAni() const
{
	ASSERT( INVALID_ANI != m_nSwatDefenseAni );
	return m_nSwatDefenseAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::IsHoldDefenseAni()
//
//	PURPOSE:	Returns true if the passed animation is any type of hold defense
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::IsHoldDefenseAni( uint32 dwAni )
{
	ASSERT( INVALID_ANI != m_nPreHoldDefenseAni );
	ASSERT( INVALID_ANI != m_nHoldDefenseAni );
	ASSERT( INVALID_ANI != m_nImpactHoldDefenseAni );
	ASSERT( INVALID_ANI != m_nPostHoldDefenseAni );

	if ( ( dwAni == m_nPreHoldDefenseAni ) ||
	     ( dwAni == m_nHoldDefenseAni ) ||
	     ( dwAni == m_nImpactHoldDefenseAni ) ||
	     ( dwAni == m_nPostHoldDefenseAni ) )
	{
		return true;
	}
	else
	{
		return false;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::GetPreHoldDefenseAni()
//
//	PURPOSE:	Returns the pre hold defense animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeaponDisc::GetPreHoldDefenseAni() const
{
	ASSERT( INVALID_ANI != m_nPreHoldDefenseAni );
	return m_nPreHoldDefenseAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::GetHoldDefenseAni()
//
//	PURPOSE:	Returns the hold defense animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeaponDisc::GetHoldDefenseAni() const
{
	ASSERT( INVALID_ANI != m_nHoldDefenseAni );
	return m_nHoldDefenseAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::GetImpactHoldDefenseAni()
//
//	PURPOSE:	Return the impact defense animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeaponDisc::GetImpactHoldDefenseAni() const
{
	ASSERT( INVALID_ANI != m_nImpactHoldDefenseAni );
	return m_nImpactHoldDefenseAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::GetPostHoldDefenseAni()
//
//	PURPOSE:	Return the post hold defense animation
//
// ----------------------------------------------------------------------- //

uint32 CClientWeaponDisc::GetPostHoldDefenseAni() const
{
	ASSERT( INVALID_ANI != m_nPostHoldDefenseAni );
	return m_nPostHoldDefenseAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::HandleMessageProjectile()
//
//	PURPOSE:	Handle projectile messages from the server
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::HandleMessageProjectile( uint8 messageID,
                                                 ILTMessage_Read *pMsg )
{
	// get the projectile message subtype
	uint8 nProjectileMessageType = pMsg->Readuint8( );

	switch ( nProjectileMessageType )
	{
		case MPROJ_RETURNING:
		{
			//
			// disc is returning
			//

			// stop sending updates to the disc
			m_bDiscNeedsUpdates = false;

			// we handled message, return true
			return true;
		}
		break;

		case MPROJ_RETURNING_DISTANCE_THRESHOLD:
		{
			// don't handle this currently
		}
		break;

		case MPROJ_RETURNED:
		{
			//
			// we now have the disc!
			//

			// get the extra information to be sure

			// get the weapon id
			uint8 nWeaponId = pMsg->Readuint8();

			// get the ammo id
			uint8 nAmmoId = pMsg->Readuint8();

			// get the number of ammos (not needed, cleaning out the message)
			uint8 nAmmoAmount = pMsg->Readuint8();

			// Since the weapon cannot switch until we get
			// the disc back, make sure the returned type
			// is the same weapon and ammo type
			ASSERT( m_nWeaponId == nWeaponId );
			ASSERT( m_nAmmoId == nAmmoId );

			// disc is no longer flying
			m_bIsDiscActive = false;

			// stop sending updates to the disc
			m_bDiscNeedsUpdates = false;

			// reset the active timer
			m_fTimeDiscActive = -1.0f;

			// give the disc back
			IncrementAmmo();

			// we handled message, return true
			return true;
		}
		break;

		case MPROJ_BLOCKED:
		{
			// play a clientfx based on the defense type

			// get the type of defense
			uint8 nDefendType = pMsg->Readuint8( );

			// get the damage percentage
			float fDefendPercentage = pMsg->Readfloat( );

			DISCCLASSDATA const *pDiscData = dynamic_cast< DISCCLASSDATA const * >( m_pAmmo->pProjectileFX->pClassData );

			// no arm defense using the disc
			ASSERT( MPROJ_START_ARM_BLOCK != nDefendType );

			CLIENTFX_LINK clientFXLink;
			uint32 dwFXFlags = FXFLAG_SMOOTHSHUTDOWN | FXFLAG_REALLYCLOSE;
			if ( MPROJ_START_SWAT_BLOCK == nDefendType )
			{
				if ( pDiscData->fSwatCriticalDefendThreshold <= fDefendPercentage )
				{
					// create the critical defense effect
					CLIENTFX_CREATESTRUCT fxInit( pDiscData->szSwatCriticalDefendPVFXName, dwFXFlags, m_hObject );

					// create the client fx
					g_pClientFXMgr->CreateClientFX( &clientFXLink, fxInit, LTTRUE );
					if ( clientFXLink.m_pInstance ) 
					{
						// setup the flags
						clientFXLink.m_pInstance->m_bPlayerView = LTFALSE;
					}
				}
				else
				{
					// create the normal defense effect
					CLIENTFX_CREATESTRUCT fxInit( pDiscData->szSwatDefendPVFXName, dwFXFlags, m_hObject );

					// create the client fx
					g_pClientFXMgr->CreateClientFX( &clientFXLink, fxInit, LTTRUE );
					if ( clientFXLink.m_pInstance ) 
					{
						// setup the flags
						clientFXLink.m_pInstance->m_bPlayerView = LTTRUE;
					}
				}
			}
			else
			{
				// create the hold defense effect
				CLIENTFX_CREATESTRUCT fxInit( pDiscData->szHoldDefendPVFXName, dwFXFlags, m_hObject );

				// set this flag and during the next animation update the hold impact
				// animation will play
				m_bPlayImpactHoldAnimation = true;

				// create the client fx
				g_pClientFXMgr->CreateClientFX( &clientFXLink, fxInit, LTTRUE );
				if ( clientFXLink.m_pInstance ) 
				{
					// setup the flags
					clientFXLink.m_pInstance->m_bPlayerView = LTTRUE;
				}
			}
		}
		break;

		default:
		{
			// The projectile message type is malformed...
			// perhaps the message was created improperly,
			// or perhaps the client weapon is not
			// designed to handle these messages.
			ASSERT( 0 );
		}
		break;
	};  // switch( nProjectileMessageType )

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::OnModelKey()
//
//	PURPOSE:	Update the animation state of the model
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::OnModelKey( HLOCALOBJ hObj, ArgList* pArgs )
{
	ASSERT( 0 != hObj );

	// get the keyframe intent
	char const *pKey = pArgs->argv[ 0 ];

	if ( 0 == stricmp( pKey, WEAPON_KEY_FIRE ) )
	{
		if ( !m_bIgnoreFireKeyframe )
		{
			// Only allow fire keys if it is a fire animation...
			uint32 dwAni = g_pLTClient->GetModelAnimation( m_hObject );
			if ( IsFireAni( dwAni ) )
			{
				// we've received a fire key, setup for throwing the disc
				m_bFire = true;

				// clear the fire message sent flag
				m_bFireMessageSent = false;

				// keep track of the total time disc is active
				m_fTimeDiscActive = 0.0f;
			}
		}

		// reset the ignore fire keyframe flag
		m_bIgnoreFireKeyframe = false;

		return true;
	}

	// if this point is reached the message was not
	// handled by the Disc, try another object
	return CClientWeapon::OnModelKey( hObj, pArgs );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::Update()
//
//	PURPOSE:	Override the main weapon update function.
//
// ----------------------------------------------------------------------- //

/// DEBUG CODE
bool g_bDisplayClientWeaponTarget = false;
/// DEBUG CODE

WeaponState CClientWeaponDisc::Update( bool bFire,
                                       FireType eFireType /*=FT_NORMAL_FIRE*/ )
{
/// DEBUG CODE
	if ( g_bDisplayClientWeaponTarget )
	{
		// find the point we're aiming at
		LTVector vTargetPoint;
		CalculatePointGuidedTarget( &vTargetPoint );
		static CLIENTFX_LINK s_pTargetPoint;
		if ( s_pTargetPoint.m_pInstance ) 
		{
			// back it off just a little
			LTVector vBackOff;
			CalculateControlDirection( &vBackOff );
			vBackOff *= -5.0f;
			vTargetPoint += vBackOff;
			s_pTargetPoint.m_pInstance->SetPos( vTargetPoint, vTargetPoint );
		}
		else
		{
			// create the target point
			CLIENTFX_CREATESTRUCT fxInit( "DISC_target", FXFLAG_LOOP , vTargetPoint ); 

			g_pClientFXMgr->CreateClientFX( &s_pTargetPoint, fxInit, LTTRUE );
			if ( s_pTargetPoint.m_pInstance ) 
			{
				s_pTargetPoint.m_pInstance->m_bPlayerView = LTFALSE;
			}
		}
	}
/// DEBUG CODE

	// 
	// See if we are disabled...If so don't allow any weapon stuff...
	if ( m_bDisabled )
	{
		return W_IDLE;
	}

	if ( m_bDiscNeedsUpdates )
	{
		CAutoMessage cMsg;
		LTRESULT ltResult;

		cMsg.Writeuint8( MID_PROJECTILE );

		// write the projectile message subtype
		cMsg.Writeuint8( MPROJ_UPDATE_CONTROL_LINE );

		// pass a control vector, which is a vector pointing
		// straight out in front of the player
		LTVector vControlDirection;
		CalculateControlDirection( &vControlDirection );

		// get the control position (in this case, the camera position)
		// get the camera
		HOBJECT hCamera = g_pPlayerMgr->GetCamera();
		ASSERT( 0 != hCamera );
		LTVector vControlPosition;
		ltResult = g_pLTClient->GetObjectPos( hCamera, &vControlPosition );
		ASSERT( LT_OK == ltResult );

		// write the control vector
		cMsg.WriteLTVector( vControlDirection );

		// write the control position
		cMsg.WriteLTVector( vControlPosition );

		// send the message
		ltResult = g_pLTClient->SendToServer(
				cMsg.Read(),
				MESSAGE_GUARANTEED
			);
		ASSERT( LT_OK == ltResult );
	}

	return CClientWeapon::Update( bFire, eFireType );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::PlayFireAnimation()
//
//	PURPOSE:	Set model to firing animation.  If the model has a PreFire animation
//				we will play that first and then play the Fire animation.  If the model
//				has a PostFire animation we will play that as soon as the Fire ani is done. 
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::PlayFireAnimation( bool bResetAni )
{
	// bResetAni is equivalant to saying "the fire key is down"

	// Can only set the last fire type if a fire animation isn't playing
	// (i.e., we'll assume this function will return false)...
	uint32 dwAni    = g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState  = g_pLTClient->GetModelPlaybackState( m_hObject );

	bool bIsFireAni = IsFireAni( dwAni );

	uint32 dwPreFireAni = GetPreFireAni();
	uint32 dwPostFireAni = GetPostFireAni();

	if ( !bIsFireAni )
	{
		// If we have a pre-fire animation, play it before the fire animation...
		if( ( INVALID_ANI != dwPreFireAni ) && !bIsFireAni )
		{
			PlayAnimation( dwPreFireAni );
			return true;
		}
	}
	else
	{
		if ( ( dwPreFireAni == dwAni ) )
		{
			// Since the disc firing sequence and the related variables
			// to esure we have a boomerang style weapon are pretty
			// sensitive to variation, force the entire prefire animation
			// to finish completely, the queue either a fire or a post
			// fire animation depending on what the user wants to do.

			if ( dwState & MS_PLAYDONE )
			{
				//
				// There are 2 ways to fire a disc.  Either click-to-fire
				// click-to-return, or hold-to-fire-release-to-return.
				// The logic here supports both versions.  If the user
				// is pressing the fire button when the fire key is
				// received, we are considering that hold-to-fire.  If
				// not, it is considered click-to-fire.
				//

				// if bResetAni is true, the user wants to fire
				if ( bResetAni || m_bIsDiscActive )
				{
					// we are done playing the prefire animation, play the
					// fire animation as looping
					uint32 dwFireAni = GetFireAni( m_eLastFireType );
					if ( INVALID_ANI == dwFireAni )
					{
						// no valid fire animation
						ASSERT( !"no valid fire animation" );
						return false;
					}

					PlayAnimation( dwFireAni, true, 1.0f, true );
				}
				else
				{
					//
					// the user dosen't want to fire
					//

					if ( !m_bIsDiscActive )
					{
						if ( INVALID_ANI == dwPostFireAni )
						{
							// no valid fire animation
							return false;
						}

						PlayAnimation( dwPostFireAni );
					}
				}
			}
			// else, don't let anything happen until the prefire is done
		}
		else if ( ( dwPostFireAni == dwAni ) && ( dwState & MS_PLAYDONE ) )
		{
			// postfire animation is finished,
			// handle the ending
			DoSpecialEndFire();

			if ( bResetAni )
			{
				// the user wants to start throwing again...NOW!
				// Without this here, there is no way for the weapon
				// to start firing again until the user lets the
				// mouse button up.

				PlayAnimation( dwPreFireAni );
				return true;
			}

			// no special animation playing
			return false;
		}
		else if ( ( dwPostFireAni != dwAni ) && 
		          ( !m_bIsDiscActive ) )
		{
			// play the postfire animation immediately
			if ( INVALID_ANI == dwPostFireAni )
			{
				// no valid postfire animation
				ASSERT( !"no valid postfire animation" );
				return false;
			}

			PlayAnimation( dwPostFireAni );
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::PlaySwatDefenseAnimation()
//
//	PURPOSE:	Set the model to the swat defense animation
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::PlaySwatDefenseAnimation( bool bResetAni )
{
	// get thde current animation and the current animation state
	uint32 dwAni    = g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState  = g_pLTClient->GetModelPlaybackState( m_hObject );

	if ( !IsSwatDefenseAni( dwAni ) )
	{
		// start the defense animation
		PlayAnimation( GetSwatDefenseAni(), false );

		// let the server know
		SendDefendMessage( MPROJ_START_SWAT_BLOCK , GetSwatDefenseAni() );

		// turn on accurate rotations on the server
		g_pPlayerMgr->StartServerAccurateRotation();

		// start sending the camera offset to the server
		g_pPlayerMgr->StartSendingCameraOffsetToServer();

		return true;
	}
	else
	{
		if ( dwState & MS_PLAYDONE )
		{
			// all done!
			if ( bResetAni )
			{
				// button held down, play again

				// start the defense animation
				PlayAnimation( GetSwatDefenseAni(), false );

				// let the server know
				SendDefendMessage( MPROJ_START_SWAT_BLOCK , GetSwatDefenseAni() );

				// turn on accurate rotations on the server
				g_pPlayerMgr->StartServerAccurateRotation();

				// start sending the camera offset to the server
				g_pPlayerMgr->StartSendingCameraOffsetToServer();

				return true;
			}
			else
			{
				// turn off accurate rotations on the server
				g_pPlayerMgr->EndServerAccurateRotation();

				// stop sending the camera offset to the server
				g_pPlayerMgr->EndSendingCameraOffsetToServer();

				return false;
			}
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::PlayHoldDefenseAnimation()
//
//	PURPOSE:	Set the model to the hold defense animation
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::PlayHoldDefenseAnimation( bool bResetAni )
{
	// get thde current animation and the current animation state
	uint32 dwAni    = g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState  = g_pLTClient->GetModelPlaybackState( m_hObject );

	if ( GetPreHoldDefenseAni() == dwAni )
	{
		if ( dwState & MS_PLAYDONE )
		{
			PlayAnimation( GetHoldDefenseAni(), false, 1.0f, true );
			return true;
		}
	}
	else if ( GetHoldDefenseAni() == dwAni )
	{
		if ( !bResetAni )
		{
			// play the hold "release" aniamtion
			PlayAnimation( GetPostHoldDefenseAni(), true );

			// let the server know
			SendDefendMessage( MPROJ_END_HOLD_BLOCK, GetPostHoldDefenseAni() );

			// turn off accurate rotations on the server
			g_pPlayerMgr->EndServerAccurateRotation();

			// stop sending the camera offset to the server
			g_pPlayerMgr->EndSendingCameraOffsetToServer();
		}
		else if ( m_bPlayImpactHoldAnimation )
		{
			// play the impact hold animation
			PlayAnimation( GetImpactHoldDefenseAni() );
		}

		return true;
	}
	else if ( GetImpactHoldDefenseAni() == dwAni )
	{
		if ( dwState & MS_PLAYDONE )
		{
			if ( bResetAni )
			{
				// keep playing the hold animation
				PlayAnimation( GetHoldDefenseAni(), false, 1.0f, true );
			}
			else
			{
				// play the hold "release" aniamtion
				PlayAnimation( GetPostHoldDefenseAni(), true );

				// let the server know
				SendDefendMessage( MPROJ_END_HOLD_BLOCK, GetPostHoldDefenseAni() );

				// turn off accurate rotations on the server
				g_pPlayerMgr->EndServerAccurateRotation();

				// stop sending the camera offset to the server
				g_pPlayerMgr->EndSendingCameraOffsetToServer();
			}
		}
	}
	else if ( GetPostHoldDefenseAni() == dwAni )
	{
		if ( dwState & MS_PLAYDONE )
		{
			// all done!
			if ( bResetAni )
			{
				// button held down, play again
				PlayAnimation( GetPreHoldDefenseAni(), false );

				// let the server know
				SendDefendMessage(
						MPROJ_START_HOLD_BLOCK,
						GetPreHoldDefenseAni()
					);

				// keep on accurate rotations on the server
				g_pPlayerMgr->StartServerAccurateRotation();

				// keep on the camera offset being sent to the server
				g_pPlayerMgr->StartSendingCameraOffsetToServer();

				return true;
			}
			else
			{
				return false;
			}
		}
	}
	else
	{
		ASSERT( !IsHoldDefenseAni( dwAni ) );

		// no hold animation playing, start the hold
		PlayAnimation( GetPreHoldDefenseAni(), false );

		// let the server know
		SendDefendMessage( MPROJ_START_HOLD_BLOCK, GetPreHoldDefenseAni() );

		// turn on accurate rotations on the server
		g_pPlayerMgr->StartServerAccurateRotation();

		// start sending the camera offset to the server
		g_pPlayerMgr->StartSendingCameraOffsetToServer();

		return true;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::AddExtraFireMessageInfo
//
//	PURPOSE:	Add disc firing information to the fire message
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMsg )
{
	ASSERT( pMsg );

	// get the projectile structure
	PROJECTILEFX const *pProjectileFX = m_pAmmo->pProjectileFX;
	ASSERT( 0 != pProjectileFX );

	// get the projectile class data
	DISCCLASSDATA *pDiscClassData =
		dynamic_cast< DISCCLASSDATA* >( pProjectileFX->pClassData );
	ASSERT( 0 != pDiscClassData );

	if ( bFire )
	{
		//
		// The player uses the control line method of guiding the disc.
		//

		// find the point we're aiming from
		LTVector vTargetStart;
		// get the camera
		HOBJECT hCamera = g_pPlayerMgr->GetCamera();
		ASSERT( 0 != hCamera );
		// get the camera's position, its the fire position
		g_pLTClient->GetObjectPos( hCamera, &vTargetStart );

		// find the point we're aiming at
	//	LTVector vTargetPoint;
	//	bResult = CalculatePointGuidedTarget( &vTargetPoint );
	//	ASSERT( true == bResult );

		// determine the angle offset (angle from center to release the disc)
		LTFLOAT fDiscReleaseAngle;
		fDiscReleaseAngle = InterpolateRating<LTFLOAT>(
				Rating_Accuracy,
				pDiscClassData->fInitialAngleMin,
				pDiscClassData->fInitialAngleMax,
				pDiscClassData->fInitialAngleSurge
			);

		// pass a control vector, which is a vector pointing
		// straight out in front of the player
		LTVector vControlDirection;
		CalculateControlDirection( &vControlDirection );

		// write the angle offset
		pMsg->Writefloat( fDiscReleaseAngle );

		// write the desired kind of disc tracking
		pMsg->Writeuint8(
				static_cast< uint8 >( MPROJ_DISC_TRACKING_CONTROL_LINE )
			);

		// write the control vector
		pMsg->WriteLTVector( vControlDirection );

		// write the control position (in this case, the camera)
		pMsg->WriteLTVector( vTargetStart );
	}
	else
	{
		//
		// The player is shooting the disc that flys straight with no control.
		//

		// write the angle offset
		pMsg->Writefloat( 0.0f );

		// write the desired kind of disc tracking
		pMsg->Writeuint8(
				static_cast< uint8 >( MPROJ_DISC_TRACKING_STEADY )
			);
	}

	// write if this is a cluster disc or not (blah)
	pMsg->Writeuint8( static_cast< uint8 >( pDiscClassData->bClusterDiscInfo ) );

	// the fire message will be sent shortly after this, so
	// set the flag indicating we've send the fire message
	m_bFireMessageSent = true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::SendDefendMessage()
//
//	PURPOSE:	Send a message telling the server the swat defense is starting
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::SendDefendMessage( uint8 cBlockMsgType, uint32 nAni ) const
{
	LTRESULT ltResult;
	CAutoMessage cMsg;

	cMsg.Writeuint8( MID_PROJECTILE );

	// write the projectile subtype
	cMsg.Writeuint8( cBlockMsgType );

	// write the weapon type for server validation
	cMsg.Writeuint8( m_nAmmoId );

	// write the current timestamp (in milliseconds)
	cMsg.Writeuint32(
		static_cast< int >( g_pLTClient->GetTime() * 1000.0f )
	);

	// get the swat defenese animation length
	uint32 nLength;
	ltResult =
		g_pModelLT->GetAnimLength(
			m_hObject,
			nAni,
			nLength
		);
	ASSERT( LT_OK == ltResult );

	// write the length
	cMsg.Writeuint32( nLength );

	// send the message
	ltResult =
		g_pLTClient->SendToServer(
			cMsg.Read(),
			MESSAGE_GUARANTEED
		);
	ASSERT( LT_OK == ltResult );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::CalculatePointGuidedTarget()
//
//	PURPOSE:	Shoot a line through the camera and return first valid point
//				it hits
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::CalculatePointGuidedTarget( LTVector *pvTargetPoint )
{
	//
	// safety check
	//

	// params
	ASSERT( 0 != pvTargetPoint );

	// globals
	ASSERT( 0 != g_pLTClient );
	ASSERT( 0 != g_pPlayerMgr );


	//
	// Cast a ray from the camera to determine where
	// the point guided weapon's target is.
	//

	// get the camera
	HOBJECT hCamera;
	hCamera = g_pPlayerMgr->GetCamera();

	// determine the start point of the segment
	LTVector vStartPos;
	g_pLTClient->GetObjectPos( hCamera, &vStartPos );

	// determine the orientation of the segment
	LTRotation rRot;
	g_pLTClient->GetObjectRotation( hCamera, &rRot );

	// determine the direction of the segment
	LTVector vForward;
	vForward = rRot.Forward();

	// scale the direction to get the distance we're going to check
	LTVector vDistance;
	VEC_MULSCALAR( vDistance, vForward, CLIENTWEAPONDISC_PROBE_LENGTH );

	// determine the end point of the segment
	LTVector vEndPos;
	VEC_ADD( vEndPos, vStartPos, vDistance );

	// setup the intersect segment
	IntersectInfo iInfo;
	IntersectQuery qInfo;
	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	// DON'T collide with these objects
	HOBJECT hFilterList[] = { 
			g_pLTClient->GetClientObject()  // the player
			, g_pPlayerMgr->GetMoveMgr()->GetObject()  // the player
			, LTNULL
		};

	// callback function when we hit something
	// ObjListFilterFn: just filter out objects specified in above list,
	// otherwise register a hit
	qInfo.m_FilterFn  = ObjListFilterFn;  
	qInfo.m_pUserData = hFilterList;

	// end points
	qInfo.m_From = vStartPos;
	qInfo.m_To = vEndPos;

	// do the intersect segment
	LTBOOL bResult;
	bResult = g_pLTClient->IntersectSegment( &qInfo, &iInfo );

	// if we hit something, set the point
	if ( bResult )
	{
		*pvTargetPoint = iInfo.m_Point;
	}

	return ( LTTRUE == bResult );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::CalculateControlDirection()
//
//	PURPOSE:	Calculate a vector to send to the disc to influence its control
//
// ----------------------------------------------------------------------- //

bool CClientWeaponDisc::CalculateControlDirection( LTVector *pvControlDirection )
{
	//
	// safety check
	//

	// params
	ASSERT( 0 != pvControlDirection );

	// globals
	ASSERT( 0 != g_pLTClient );
	ASSERT( 0 != g_pPlayerMgr );


	//
	// The control direction is the direction the player is facing
	//

	// get the camera
	HOBJECT hCamera;
	hCamera = g_pPlayerMgr->GetCamera();
	ASSERT( 0 != hCamera );

	// determine the orientation of the segment
	LTRotation rRot;
	g_pLTClient->GetObjectRotation( hCamera, &rRot );

	// determine the direction of the segment
	LTVector vForward;
	*pvControlDirection = rRot.Forward();

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::Fire()
//
//	PURPOSE:	Fire the weapon
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::Fire( bool bFire )
{
	//
	// update disc specific variables
	//

	// keep track locally that the disc has been thrown
	m_bIsDiscActive = true;

	// keep sending updates to the disc until it says stop
	// or we get it back
	m_bDiscNeedsUpdates = true;

	// now call the the parent's version
	CClientWeapon::Fire( bFire );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::HandleFireKeyDownTransition()
//
//	PURPOSE:	What do do when the fire key goes down
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::HandleFireKeyDownTransition()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::HandleFireKeyUpTransition()
//
//	PURPOSE:	What to do when the fire key goes up
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::HandleFireKeyUpTransition()
{
	//
	// There are 2 ways to fire a disc.  Either click-to-fire
	// click-to-return, or hold-to-fire-release-to-return.
	// The logic here supports both versions.  If the user
	// is pressing the fire button when the fire key is
	// received, we are considering that hold-to-fire.  If
	// not, it is considered click-to-fire.
	//

	if ( m_bDiscNeedsUpdates )
	{
		//
		// tell the disc to return
		//

		// reset the fire message sent flag
		m_bFireMessageSent = false;

		// stop sending updates
		m_bDiscNeedsUpdates = false;

		CAutoMessage cMsg;
		LTRESULT ltResult;

		cMsg.Writeuint8( MID_PROJECTILE );

		// tell the disc to return 
		cMsg.Writeuint8( MPROJ_RETURN );

		// send the message
		ltResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
		ASSERT( LT_OK == ltResult );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::HandleFireKeyDown()
//
//	PURPOSE:	What to do while the fire key is continually pressed
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::HandleFireKeyDown()
{
	// 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::HandleFireKeyUp()
//
//	PURPOSE:	What to do when the fire key goes up
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::HandleFireKeyUp()
{
	// 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::DoSpecialEndFire()
//
//	PURPOSE:	What to do when the fire key goes up
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::DoSpecialEndFire()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeaponDisc::IncrementAmmo
//
//	PURPOSE:	Increment the weapon's ammo count
//
// ----------------------------------------------------------------------- //

void CClientWeaponDisc::IncrementAmmo( int nAmount /*=1*/ )
{
	int nAmmo;
	bool bInfiniteAmmo = ( g_bInfiniteAmmo || ( !!( m_pWeapon->bInfiniteAmmo ) ) );
	if ( bInfiniteAmmo )
	{
		nAmmo = INFINITE_AMMO_AMOUNT;
	}
	else
	{
		nAmmo = g_pPlayerStats->GetAmmoCount( m_nAmmoId );
	}

	// add the specified amount of ammo
	nAmmo = nAmmo + nAmount;

	// Update our stats.  This will ensure that our stats are always
	// accurate (even in multiplayer).  This call will also ensure
	// it doesn't go over the max amount.
	g_pPlayerStats->UpdateAmmo( m_nWeaponId,
	                            m_nAmmoId,
	                            nAmmo,
	                            LTFALSE,
	                            LTFALSE );

	// check we have more ammo than we did a moment ago
	int nNewAmmoAmount = g_pPlayerStats->GetAmmoCount( m_nAmmoId );

	// if we have more ammo than before and the clip is not full
	if ( ( nAmmo < nNewAmmoAmount ) &&
	     ( m_nAmmoInClip < m_pWeapon->nShotsPerClip ) )
	{
		// add ammo to the clip
		m_nAmmoInClip += nAmount;

		// make sure we don't go beyond the maximum
		if ( m_nAmmoInClip > m_pWeapon->nShotsPerClip )
		{
			m_nAmmoInClip = m_pWeapon->nShotsPerClip;
		}
	}
}
