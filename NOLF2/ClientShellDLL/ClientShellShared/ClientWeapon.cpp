// ----------------------------------------------------------------------- //
//
// MODULE  : ClientWeapon.cpp
//
// PURPOSE : Generic client-side weapon
//
// CREATED : 9/27/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientWeapon.h"
#include "VarTrack.h"
#include "LayoutMgr.h"
#include "PlayerStats.h"
#include "GameClientShell.h"
#include "ModelButeMgr.h"
#include "PlayerMgr.h"
#include "CharacterFX.h"
#include "BodyFX.h"
#include "MsgIDs.h"
#include "WeaponFXTypes.h"
#include "SurfaceFunctions.h"
#include "ClientWeaponUtils.h"
#include "CMoveMgr.h"
#include "ClientFXMgr.h"
#include "ClientMultiplayerMgr.h"
#include "ShellCasingFX.h"
#include "FXButeMgr.h"
#include "ClientResShared.h"
#include "PlayerViewAttachmentMgr.h"
#include "GadgetTargetFX.h"

//
// Externs
//
extern bool g_bInfiniteAmmo;


namespace
{
	HMODELANIM const INVALID_ANI = ( static_cast< HMODELANIM >( -1 ) );
	HMODELANIM const DEFAULT_ANI = ( static_cast< HMODELANIM >( 0 ) );
	int const INFINITE_AMMO_AMOUNT = 1000;

	// model animation names
	// I wish they could be const, but the engine's interface isn't
	char *ns_szSelectAnimationName = "Select";
	char *ns_szDeselectAnimationName = "Deselect";
	char *ns_szReloadAnimationName = "Reload";

	char *ns_szAltSelectAnimationName = "AltSelect";
	char *ns_szAltDeselectAnimationName = "AltDeselect";
	char *ns_szAltDeselect2AnimationName = "AltDeselect2";
	char *ns_szAltReloadAnimationName = "AltReload";

	char *ns_szPreFireAnimationName = "PreFire";
	char *ns_szPostFireAnimationName = "PostFire";

	char *ns_szIdleAnimationBasename = "Idle_";

	char *ns_szFireAnimationName = "Fire";
	char *ns_szFireAnimationBasename = "Fire";

	char *ns_szAltIdleAnimationBasename = "AltIdle_";

	char *ns_szAltFireAnimationName = "AltFire";
	char *ns_szAltFireAnimationBasename = "AltFire";

	// If -1.0f, the anmiation rates will be unchanged,
	// else they will run at the specified speed.
	static float nsfOverrideRate = -1.0f;

	bool        ns_bInited = false;
	VarTrack    ns_vtFastTurnRate;
	VarTrack    ns_vtPerturbRotationEffect;
	VarTrack    ns_vtPerturbIncreaseSpeed;
	VarTrack    ns_vtPerturbDecreaseSpeed;
	VarTrack    ns_vtPerturbWalkPercent;
	VarTrack    ns_vtCameraShutterSpeed;
	VarTrack	ns_vtFiringPerturbIncreaseSpeed;
	VarTrack	ns_vtFiringPerturbDecreaseSpeed;

	bool InitNamespaceVars( void )
	{
		if ( ns_bInited )
		{
			// bail if we've already inited
			return true;
		}

		ASSERT( 0 != g_pLayoutMgr );

		LTBOOL   bResult;
		LTFLOAT  fTemp;

		ns_bInited = true;

		ns_vtFastTurnRate.Init( g_pLTClient, "FastTurnRate", 0, 2.3f );

		fTemp = g_pLayoutMgr->GetPerturbRotationEffect();
		bResult = ns_vtPerturbRotationEffect.Init( g_pLTClient, "PerturbRotationEffect", 0, fTemp );
		ASSERT( LTTRUE == bResult );

		fTemp = g_pLayoutMgr->GetPerturbIncreaseSpeed();
		bResult = ns_vtPerturbIncreaseSpeed.Init( g_pLTClient, "PerturbIncreaseSpeed", 0, fTemp );
		ASSERT( LTTRUE == bResult );

		fTemp = g_pLayoutMgr->GetPerturbDecreaseSpeed();
		bResult = ns_vtPerturbDecreaseSpeed.Init( g_pLTClient, "PerturbDecreaseSpeed", 0, fTemp );
		ASSERT( LTTRUE == bResult );

		fTemp = g_pLayoutMgr->GetPerturbWalkPercent();
		bResult = ns_vtPerturbWalkPercent.Init( g_pLTClient, "PerturbWalkPercent", 0, fTemp );
		ASSERT( LTTRUE == bResult );

		bResult = ns_vtCameraShutterSpeed.Init( g_pLTClient, "CameraShutterSpeed", 0, 0.3f );
		ASSERT( LTTRUE == bResult );

		bResult = ns_vtFiringPerturbIncreaseSpeed.Init( g_pLTClient, "PerturbFiringIncreaseSpeed", 0, 4.0 );
		ASSERT( LTTRUE == bResult );

		bResult = ns_vtFiringPerturbDecreaseSpeed.Init( g_pLTClient, "PerturbFiringDecreaseSpeed", 0, 1.0 );
		ASSERT( LTTRUE == bResult );

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
	  m_hObject( 0 )
	, m_hBreachSocket( INVALID_MODEL_SOCKET )
	, m_hSilencerModel( 0 )
	, m_hScopeModel( 0 )
	, m_hSilencerSocket( INVALID_MODEL_SOCKET )
	, m_hScopeSocket( INVALID_MODEL_SOCKET )
	, m_bHaveSilencer( false )
	, m_bHaveScope( false )
	, m_nWeaponId( WMGR_INVALID_ID )
	, m_pWeapon( 0 )
	, m_nAmmoId( WMGR_INVALID_ID )
	, m_pAmmo( 0 )
	, m_vFlashPos( 0.0f, 0.0f, 0.0f )
	, m_vFlashOffset( 0.0f, 0.0f, 0.0f )
	, m_fFlashStartTime( 0.0f)
	, m_fBobHeight( 0.0f )
	, m_fBobWidth( 0.0f )
	, m_fMovementPerturb( 0.0f )
	, m_fFiringPerturb( 0.0f )
	, m_eLastFireType( FT_NORMAL_FIRE )
	, m_bCanSetLastFire( false )
	, m_fNextIdleTime( 0.0f )
	, m_bFire( false )
	, m_nAmmoInClip( 0 )
	, m_nNewAmmoInClip( 0 )
	, m_eState( W_INACTIVE )
	, m_nSelectAni( INVALID_ANI )
	, m_nDeselectAni( INVALID_ANI )
	, m_nReloadAni( INVALID_ANI )
	, m_nAltSelectAni( INVALID_ANI )
	, m_nAltDeselectAni( INVALID_ANI )
	, m_nAltDeselect2Ani( INVALID_ANI )
	, m_nAltReloadAni( INVALID_ANI )
	, m_nPreFireAni( INVALID_ANI )
	, m_nPostFireAni( INVALID_ANI )
	, m_bUsingAltFireAnis( false )
	, m_bFireKeyDownLastUpdate( false )
	, m_bWeaponDeselected( false )
	, m_wIgnoreFX( 0 )
	, m_bDisabled( true )
	, m_bVisible( false )
	, m_rCamRot( 0.0f, 0.0f, 0.0f )
	, m_vCamPos( 0.0f, 0.0f, 0.0f )
	, m_hLoopSound( LTNULL )
	, m_nLoopSoundId( PSI_INVALID )
	, m_bFirstSelection( true )
	, m_nTracerNumber( 0 )
	, m_bAutoSwitchEnabled( true )
	, m_bAutoSwitch( false )
	, m_fLastPitch( 0.0f )
	, m_fLastYaw( 0.0f )
	, m_KeyframedClientFX()
	, m_bControllingFlashLight( false )
	, m_bPaused( false )
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

	// clear alt-idle anims
	for ( i = 0; i < WM_MAX_ALTIDLE_ANIS; ++i )
	{
		m_nAltIdleAnis[i] = INVALID_ANI;
	}

	// clear alt-fire anims
	for ( i = 0; i < WM_MAX_ALTFIRE_ANIS; ++i )
	{
		m_nAltFireAnis[i] = INVALID_ANI;
	}

	for( i = 0; i < WM_MAX_PV_ATTACH_CLIENTFX; ++i )
	{
		m_bPVAttachClientFXHidden[i] = true;
	}
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
//	PURPOSE:	Handle animation commands
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::OnModelKey( HLOCALOBJ hObj, ArgList* pArgList )
{
	static CParsedMsg::CToken s_cTok_WeaponKeySound(WEAPON_KEY_SOUND);
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_FIRE( WEAPON_KEY_FIRE );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_SOUND( WEAPON_KEY_SOUND );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_BUTE_SOUND( WEAPON_KEY_BUTE_SOUND );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_LOOPSOUND( WEAPON_KEY_LOOPSOUND );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_FX( WEAPON_KEY_FX );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_FIREFX( WEAPON_KEY_FIREFX );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_HIDE_MODEL_PIECE( WEAPON_KEY_HIDE_MODEL_PIECE );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_SHOW_MODEL_PIECE( WEAPON_KEY_SHOW_MODEL_PIECE );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_FLASHLIGHT( WEAPON_KEY_FLASHLIGHT );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_DEFLECT( WEAPON_KEY_DEFLECT );
	static CParsedMsg::CToken s_cTok_RENDERSTYLE_MODEL_KEY( RENDERSTYLE_MODEL_KEY );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_SHELLCASING( WEAPON_KEY_SHELLCASING );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_HIDE_PVATTACHFX( WEAPON_KEY_HIDE_PVATTACHFX );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_SHOW_PVATTACHFX( WEAPON_KEY_SHOW_PVATTACHFX );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_HIDE_PVATTACHMENT( WEAPON_KEY_HIDE_PVATTACHMENT );
	static CParsedMsg::CToken s_cTok_WEAPON_KEY_SHOW_PVATTACHMENT( WEAPON_KEY_SHOW_PVATTACHMENT );

	if ( !hObj ||
	     !m_pWeapon ||
	     ( hObj != m_hObject ) ||
	     !pArgList || 
	     !pArgList->argv ||
	     ( pArgList->argc == 0 ) ||
		 m_bDisabled )
	{
		return false;
	}

	// make sure there is an argument
	char* pKey = pArgList->argv[0];
	if ( !pKey )
	{
		return false;
	}

	// Make a token to compare against.
	CParsedMsg::CToken tok( pKey );

	if( tok == s_cTok_WEAPON_KEY_FIRE )
	{
		//
		// Fire weapon
		//
		return HandleFireKey( hObj, pArgList );
	}
	else if( tok == s_cTok_WEAPON_KEY_SOUND )
	{
		//
		// Play a sound globally (everybody should hear it)
		//

		if ( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
		{
			char* pBuf = 0;

			PlayerSoundId nId = static_cast< PlayerSoundId >( atoi( pArgList->argv[ 1 ] ) );
			switch ( nId )
			{
				case PSI_RELOAD:
				case PSI_RELOAD2:
				case PSI_RELOAD3:
				{
					pBuf = m_pWeapon->szReloadSounds[ ( nId - PSI_RELOAD ) ];
				}
				break;

				case PSI_SELECT:
				{
					pBuf = m_pWeapon->szSelectSound;
				}
				break;

				case PSI_DESELECT:
				{
					pBuf = m_pWeapon->szDeselectSound;
				}
				break;

				case PSI_WEAPON_MISC1:
				case PSI_WEAPON_MISC2:
				case PSI_WEAPON_MISC3:
				case PSI_WEAPON_MISC4:
				case PSI_WEAPON_MISC5:
				{
					pBuf = m_pWeapon->szMiscSounds[nId - PSI_WEAPON_MISC1];
				}
				break; 

				case PSI_INVALID:
				default:
				{
				}
				break;
			}

			if ( pBuf && pBuf[ 0 ] )
			{
				g_pClientSoundMgr->PlaySoundLocal( pBuf, SOUNDPRIORITY_PLAYER_HIGH, 
					PLAYSOUND_REVERB, SMGR_DEFAULT_VOLUME, 1.0f, WEAPONS_SOUND_CLASS );

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
				cMsg.Writeuint8( m_nWeaponId );

				// write the client's id
				cMsg.Writeuint8( static_cast< uint8 >( dwId ) );

				// write the flash position (presumably this is where the sound comes from)
				cMsg.WriteLTVector( m_vFlashPos );

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
			g_pClientSoundMgr->PlaySoundLocal( pArgList->argv[1], SOUNDPRIORITY_PLAYER_HIGH,
					PLAYSOUND_REVERB, SMGR_DEFAULT_VOLUME, 1.0f, WEAPONS_SOUND_CLASS );
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_LOOPSOUND )
	{
		// Handle a looping sound key

		if( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
		{
			if( 0 == stricmp( pArgList->argv[1], "STOP" ))
			{
				// Stop the looping sound from playing...

				KillLoopSound();

				// Send message to server so all clients can stop the sound...
				// An id of invalid means stop

				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_WEAPON_SOUND_LOOP );
				cMsg.Writeuint8( PSI_INVALID );
				cMsg.Writeuint8( m_nWeaponId );
				g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
			
				return true;
			}

			char* pBuf = 0;

			PlayerSoundId nId = static_cast< PlayerSoundId >( atoi( pArgList->argv[ 1 ] ) );
			switch( nId )
			{
				case PSI_RELOAD:														// 1
				case PSI_RELOAD2:														// 2
				case PSI_RELOAD3:														// 3
				{
										pBuf = m_pWeapon->szReloadSounds[ ( nId - PSI_RELOAD ) ];
				}
				break;

				case PSI_SELECT:		pBuf = m_pWeapon->szSelectSound;		break;	// 4
				case PSI_DESELECT:		pBuf = m_pWeapon->szDeselectSound;		break;	// 5
				case PSI_FIRE:			pBuf = m_pWeapon->szFireSound;			break;	// 6
				case PSI_DRY_FIRE:		pBuf = m_pWeapon->szDryFireSound;		break;	// 7
				case PSI_ALT_FIRE:		pBuf = m_pWeapon->szAltFireSound;		break;	// 8
				case PSI_SILENCED_FIRE:	pBuf = m_pWeapon->szSilencedFireSound;	break;	// 9
				
				case PSI_WEAPON_MISC1:													// 10
				case PSI_WEAPON_MISC2:													// 11
				case PSI_WEAPON_MISC3:													// 12
				case PSI_WEAPON_MISC4:													// 13
				case PSI_WEAPON_MISC5:													// 14
				{
										pBuf = m_pWeapon->szMiscSounds[nId - PSI_WEAPON_MISC1];
				}
				break; 
				
				case PSI_INVALID:
				default:
				{
				}
				break;
			}

			if( pBuf && pBuf[0] )
			{
				if( !m_hLoopSound || (nId != m_nLoopSoundId) )
				{
					// Stop any previous looping sound...

					KillLoopSound();

					// Play the sound immediately localy 
					
					m_hLoopSound = g_pClientSoundMgr->PlaySoundLocal( pBuf, SOUNDPRIORITY_PLAYER_HIGH,
																	  PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE | PLAYSOUND_REVERB,
																	  SMGR_DEFAULT_VOLUME, 1.0f, WEAPONS_SOUND_CLASS );

					m_nLoopSoundId = nId;

					// Send message to server so all clients can start loop sound...

					CAutoMessage cMsg;
					cMsg.Writeuint8( MID_WEAPON_SOUND_LOOP );
					cMsg.Writeuint8( nId );
					cMsg.Writeuint8( m_nWeaponId );
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

		ASSERT( 0 != m_hObject );

		// Only allow fire keys if it is a fire animation...
		uint32 dwAni = g_pLTClient->GetModelAnimation( m_hObject );
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
			if( LT_OK == pModelLT->GetPiece( m_hObject, pArgList->argv[ i ], hPiece ) )
			{
				// hide it
				LTRESULT ltResult;
				ltResult = pModelLT->SetPieceHideStatus( m_hObject, hPiece, LTTRUE );
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
			if( LT_OK == pModelLT->GetPiece( m_hObject, pArgList->argv[ i ], hPiece ) )
			{
				// hide it
				LTRESULT ltResult;
				ltResult = pModelLT->SetPieceHideStatus( m_hObject, hPiece, LTFALSE );
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
			if( 0 == stricmp( pArgList->argv[1], "ON" ))
			{
				m_bControllingFlashLight = true;
				g_pPlayerMgr->GetFlashLight()->TurnOn();
			}
			else if( 0 == stricmp( pArgList->argv[1], "OFF" ))
			{
				m_bControllingFlashLight = false;
				g_pPlayerMgr->GetFlashLight()->TurnOff();
			}
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_DEFLECT )
	{
		if( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
		{
			// Tell the server we're deflecting.
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );
			cMsg.Writeuint8( CP_DEFLECT );
			cMsg.Writefloat(( float )atof( pArgList->argv[1] ));
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
		}
	}
	else if( tok == s_cTok_RENDERSTYLE_MODEL_KEY )
	{
		// start the index at the first argument
		int i = 0;
		int nRS;

		// while there are arguments
		while((i < pArgList->argc) && ('\0' != pArgList->argv[i][0]))
		{
			// Check for renderstyle model key
			if(stricmp(pArgList->argv[i],RENDERSTYLE_MODEL_KEY) == 0)
			{
				// Check params
				if(pArgList->argc >= i+3)
				{
					nRS = (atoi)(pArgList->argv[i+1]);
					SetObjectRenderStyle(m_hObject,nRS,pArgList->argv[i+2]);
				}
				else
				{
					// Not enough params
					g_pLTClient->CPrint("CClientWeapon::OnModelKey - ERROR - Not enough RS arguments! Syntax: RS <RSNum> <RSName>\n");
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
		if (!m_bVisible) return false;

		// Create a shell casing based on the model key...

		if (g_pPlayerMgr->IsFirstPerson())
		{
			SHELLCREATESTRUCT sc;
			sc.nWeaponId	= m_nWeaponId;
			sc.nAmmoId		= m_nAmmoId;
			sc.b3rdPerson	= LTFALSE;

			GetModelRot(&sc.rRot);
			GetShellEjectPos(&sc.vStartPos);

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

		// start the index at the first argument
		int i = 1;
		int nPVFX = 0;

		// while there are arguments
		while((i < pArgList->argc) && ('\0' != pArgList->argv[i][0]))
		{
			nPVFX = atoi( pArgList->argv[ i ] );
			if( nPVFX >= 0 && nPVFX < WM_MAX_PV_ATTACH_CLIENTFX )
			{
				if( m_PVAttachClientFX[ nPVFX ].IsValid() )
				{
					m_bPVAttachClientFXHidden[ nPVFX ] = true;
					m_PVAttachClientFX[ nPVFX ].GetInstance()->Hide();
				}
			}

			++i;
		}
	}
	else if( tok == s_cTok_WEAPON_KEY_SHOW_PVATTACHFX )
	{
		if (!m_bVisible) return false;

		// start the index at the first argument
		int i = 1;
		int nPVFX = 0;

		// while there are arguments
		while((i < pArgList->argc) && ('\0' != pArgList->argv[i][0]))
		{
			nPVFX = atoi( pArgList->argv[ i ] );
			if( nPVFX >= 0 && nPVFX < WM_MAX_PV_ATTACH_CLIENTFX )
			{
				if( m_PVAttachClientFX[ nPVFX ].IsValid() )
				{
					m_bPVAttachClientFXHidden[ nPVFX ] = false;
					m_PVAttachClientFX[ nPVFX ].GetInstance()->Show();
				}
			}

			++i;
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

void CClientWeapon::GetShellEjectPos( LTVector *vOriginalPos )
{
	LTVector vBreachOffset = GetBreachOffset();

	// Adjust the breach offset relative to the orientation of the weapon
	// model...

	LTVector vU = (vBreachOffset.y * m_rCamRot.Up());
	LTVector vR = (vBreachOffset.x * m_rCamRot.Right());
	LTVector vF = (vBreachOffset.z * m_rCamRot.Forward());

	*vOriginalPos = m_vFlashPos + vU + vR + vF;
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
	ASSERT( 0 != m_hObject );

	// Only allow fire keys if it is a fire animation...
	uint32 dwAni = g_pLTClient->GetModelAnimation( m_hObject );
	if ( IsFireAni( dwAni ) && GetState() == W_FIRING )
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
		bool bResult;
		CLIENTFX_LINK_NODE *pNewNode = debug_new(CLIENTFX_LINK_NODE);

		if(pNewNode)
		{
			CLIENTFX_CREATESTRUCT  fxCS( pArgList->argv[ i ],
										 FXFLAG_REALLYCLOSE,
										 m_hObject );

			bResult = g_pClientFXMgr->CreateClientFX( &pNewNode->m_Link,
													  fxCS,
													  LTTRUE );

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
	m_fNextIdleTime = 0;
	m_bFire = false;
	m_nAmmoInClip = 0;
	m_nNewAmmoInClip = 0;
	m_bWeaponDeselected = false;
	m_nTracerNumber = 0;
	m_fLastPitch = 0.0f;
	m_fLastYaw = 0.0f;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::Init()
//
//	PURPOSE:	Initialize perturb variables
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::Init( WEAPON const &rWeapon )
{
	// set up all the namespace variables
	InitNamespaceVars();

	// set the data
	m_pWeapon = &rWeapon;
	m_nWeaponId = m_pWeapon->nId;

	// ammo stays inited
	//DANO: temp use the default ammo
	m_pAmmo = g_pWeaponMgr->GetAmmo( m_pWeapon->nDefaultAmmoId );
	ASSERT( 0 != m_pAmmo );
	m_nAmmoId = m_pAmmo->nId;


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
	g_pPVAttachmentMgr->RemovePVAttachments();

	// stop any looping sound
	KillLoopSound();

	// remove all client fx
	RemoveMuzzleFlash();
	RemovePVAttachClientFX();

	// clear weapon info
	m_nWeaponId = WMGR_INVALID_ID;
	m_pWeapon = 0;

	// clear ammo info
	m_nAmmoId = WMGR_INVALID_ID;
	m_pAmmo = 0;

	// remove the weapon model...
	RemoveWeaponModel();

}

// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::RemoveWeaponModel()
//
//	PURPOSE:	Destroy the weapon model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::RemoveWeaponModel()
{
	if (m_hObject)
	{
		// get rid of the object
		LTRESULT ltResult = g_pLTClient->RemoveObject(m_hObject);
		ASSERT( LT_OK == ltResult );

		// clear the object pointer
		m_hObject = 0;
	}
	
	m_hBreachSocket = INVALID_MODEL_SOCKET;
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

WeaponState CClientWeapon::Update( bool bFire, FireType eFireType /*=FT_NORMAL_FIRE*/)
{
	ASSERT( 0 != m_hObject );

	// remove all keyframed FXEd effects that have expired
	RemoveFinishedKeyframedFX();

	//see if we are paused, if so we need to pause the animation
	bool bPaused = g_pGameClientShell->IsServerPaused() || m_bPaused;
	g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags, bPaused ? FLAG_PAUSED : 0, FLAG_PAUSED);

	// See if we are disabled...If so don't allow any weapon stuff...
	if ( m_bDisabled || bPaused)
	{
		return W_IDLE;
	}

	// If the player is trying to fire and
	// we are allowed to set the last fire
	// type ("normal" vs "alt"), do so
	if ( bFire && m_bCanSetLastFire )
	{
		m_eLastFireType = eFireType;
	}

	// See if we just started/stopped firing...
	if ( !m_bFireKeyDownLastUpdate && bFire )
	{
		//
		// fire key transition: off->on
		//

		// Currently, this just handles alt-firing, which is broken
		HandleFireKeyDownTransition();

		// check specaial cases that may prevent us from firing
		bFire = SpecialOverrideFire();
	}
	else if ( m_bFireKeyDownLastUpdate && !bFire )
	{
		//
		// fire key transition: on->off
		//

		HandleFireKeyUpTransition();
	}

	// Selecting Alt-fire does not fire the weapon if we are using
	// alt fire animations...
	if ( m_bUsingAltFireAnis && ( FT_ALT_FIRE == m_eLastFireType ) )
	{
		bFire = false;
	}

	if ( bFire )
	{
		HandleFireKeyDown();
	}
	else
	{
		HandleFireKeyUp();
	}

	// remember the state and fire key for future updates
	m_bFireKeyDownLastUpdate  = bFire;

	// Update the state of the model...
	WeaponState eState = UpdateModelState( bFire );

	LTVector vFireOffset( 0.0f, 0.0f, 0.0f );
	LTVector vFlashOffset( 0.0f, 0.0f, 0.0f );

	// NOTE: this is NOT the weapon state...this is the value
	// returned by the UpdateModelState function.
	LTBOOL bFiredWeapon = FiredWeapon(eState);

	if ( bFiredWeapon )
	{
		//
		// The weapon should now fire/activate.
		//

		// Get a random offset based on the recoil.
		LTVector vRecoil = m_pWeapon->vRecoil;
		vFireOffset.Init( GetRandom( -vRecoil.x, vRecoil.x ),
		                  GetRandom( -vRecoil.y, vRecoil.y ),
		                  GetRandom( -vRecoil.z, vRecoil.z ) );

		vFlashOffset = vFireOffset;

		// this does not need the muzzle position set,
		// that will be done later
		StartMuzzleFlash();

		// Send message to server telling player to fire...
		Fire( bFire );
	}

	// Update the weapon's position
	UpdateWeaponPosition( vFireOffset );

	// Update the muzzle flash...
	UpdateMuzzleFlash( eState, vFlashOffset );

	// Update the mods...
	UpdateMods();

	// Update the player-view attachments
	g_pPVAttachmentMgr->UpdatePVAttachments();

	// Update the weapon's perturb value based on the player's movement.
	UpdateMovementPerturb();

	// Update the weapon's perturb value based on current weapon firing.
	UpdateFiringPerturb(bFiredWeapon);

	// Handle Auto Ammo/Weapon switching...
	if ( m_bAutoSwitchEnabled && m_bAutoSwitch )
	{
		// We only auto-switch if we are out of ammo...
		if (g_pPlayerStats->GetAmmoCount(m_nAmmoId) <= 0)
		{
			// Tell the client weapon mgr to switch to a different weapon...
			eState = W_AUTO_SWITCH;
		}

		// clear the autoswitch flag
		m_bAutoSwitch = false;
	}

	return eState;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::ChangeAmmoWithReload()
//
//	PURPOSE:	Change to the specified ammo type
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ChangeAmmoWithReload( uint8 nNewAmmoId, bool bForce /*=false*/ )
{
	// Update the player's stats...
	if ( ( W_RELOADING == GetState() ) && !bForce )
	{
		return;
	}

	if ( CanChangeToAmmo( nNewAmmoId ) && ( nNewAmmoId != m_nAmmoId ) )
	{
		ASSERT( 0 != g_pWeaponMgr );
		m_nAmmoId   = nNewAmmoId;
		m_pAmmo     = g_pWeaponMgr->GetAmmo( m_nAmmoId );

		// Make sure we reset the anis (the ammo may override the 
		// weapon animations)...
		InitAnimations( true );

		if ( m_pAmmo->pAniOverrides )
		{
			// If we're not using the defaults play the new select ani...
			Select();
		}
		else
		{
			// Do normal reload...
			ReloadClip( true, -1, true, true );

			// Add a message so the user knows he switched ammo (sometimes it
			// isn't that obvious)...

			if (strlen(m_pAmmo->szShortName))
			{
				char szMsg[128];
				FormatString(IDS_CHANGING_AMMO, szMsg, sizeof(szMsg), m_pAmmo->szShortName);
				std::string icon = m_pAmmo->GetNormalIcon();
				g_pPickupMsgs->AddMessage(szMsg, icon.c_str());
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::ChangeAmmoImmediate()
//
//	PURPOSE:	Change to the specified ammo type
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ChangeAmmoImmediate( uint8 nNewAmmoId, int nAmmoAmount /*=-1*/, bool bForce /*=false*/ )
{
	// Update the player's stats...
	if ( ( W_RELOADING == GetState() ) && !bForce )
	{
		return;
	}

	if ( CanChangeToAmmo( nNewAmmoId ) && ( nNewAmmoId != m_nAmmoId ) )
	{
		ASSERT( 0 != g_pWeaponMgr );
		m_nAmmoId   = nNewAmmoId;
		m_pAmmo     = g_pWeaponMgr->GetAmmo( m_nAmmoId );

		// Make sure we reset the anis (the ammo may override the 
		// weapon animations)...
		InitAnimations( true );

		if ( m_pAmmo->pAniOverrides )
		{
			// If we're not using the defaults play the new select ani...
			Select();
		}
		else
		{
			// Do normal reload...
			ReloadClip( false, nAmmoAmount /*-1*/, true, true );
		}
		}
	else
	{
		// Update the hud to reflect the new ammo amount...
		g_pPlayerStats->UpdateAmmo( m_nWeaponId, m_nAmmoId, nAmmoAmount );
		g_pPlayerStats->UpdatePlayerWeapon( m_nWeaponId, m_nAmmoId );
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
                                int nNewAmmo /*=-1*/,
                                bool bForce /*=false*/,
								bool bNotifyServer /*=false*/)
{
	// Can't reload clip while deselecting the weapon...

	if ( W_DESELECT == GetState() ) return;

	// get all the ammo the player possesses
	int nAmmoCount = g_pPlayerStats->GetAmmoCount( m_nAmmoId );

	// Get an intermediate amount of ammo.  If the nNewAmmo has
	// been specified, use that value.  Otherwise use the total
	// amount of ammo on the player.
	int nAmmo = nNewAmmo >= 0 ? nNewAmmo : nAmmoCount;

	// Get how many shots are in a clip.
	int nShotsPerClip = m_pWeapon->nShotsPerClip;

	// Update the player's stats...
	// note: the ammo amount we pass may be too much but
	// these functions figure out what the max really is,
	// and then we do the same thing later

	// UpdateAmmo does a lot of stuff, one of those is passing in
	// how much ammo you have.  If you specify an amount that is
	// more or less, it will consider this the new amount of
	// ammo that you have on you and adjust things accordingly.
	g_pPlayerStats->UpdateAmmo( m_nWeaponId, m_nAmmoId, nAmmo );


	// This will set the player stats to the specified weapon and 
	// ammo id.  In this case, use the current ones.
	g_pPlayerStats->UpdatePlayerWeapon( m_nWeaponId, m_nAmmoId );

	// Make sure we can reload the clip...
	if ( !bForce )
	{
		// Already reloading...
		if ( m_hObject && ( W_RELOADING == GetState() ) )
		{
			return;
		}

		// Clip is full...
		if ( ( m_nAmmoInClip == nShotsPerClip ) || ( m_nAmmoInClip == nAmmoCount ) )
		{
			return;
		}
	}

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
			cMsg.Writeuint8( m_nAmmoId ); // We maybe switching ammo
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
		}
		

		// check for a valid reload animation
		if ( bPlayReload && ( INVALID_ANI != GetReloadAni() ) )
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
			m_nAmmoInClip = m_nNewAmmoInClip;
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

	int nShotsPerClip = m_pWeapon->nShotsPerClip;

	if ( 0 < m_nAmmoInClip )
	{
		if ( 0 < nShotsPerClip )
		{
			// decrease the ammo in the clip only if the clip
			// is non-zero
			--m_nAmmoInClip;
		}

		if ( !bInfiniteAmmo )
		{
			// we are not using infinite ammo, update the current amount
			--nAmmo;

			// Update our stats.  This will ensure that our stats are always
			// accurate (even in multiplayer)...
			g_pPlayerStats->UpdateAmmo( m_nWeaponId, m_nAmmoId, nAmmo, LTFALSE, LTFALSE );
		}
	}

	// Check to see if we need to reload...
	if ( 0 < nShotsPerClip )
	{
		if ( 0 >= m_nAmmoInClip )
		{
			ReloadClip( true, nAmmo );
		}
	}
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
//	PURPOSE:	Update weapon's skins.  This is done to reflect changes in
//				the base player model.  If the base player model changes
//				then the skins will probably have to change.  For instance,
//				the sleeve color of the PV weapon may change when the base
//				player model changes from a catsuit to a disco suit.
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ResetWeaponFilenames()
{
	ASSERT( 0 != g_pCommonLT );
	ASSERT( 0 != g_pLTClient );

	ASSERT( 0 != m_hObject );
	if( !m_hObject )
		return;

	// get the create struct
	ObjectCreateStruct createStruct;

	// put this weapon's model information in the create struct
	PopulateCreateStruct( &createStruct );

	// Set the filenames...
	g_pCommonLT->SetObjectFilenames( m_hObject, &createStruct );

	// Always put the weapon in the correct position after we reset the model...

	UpdateWeaponPosition( LTVector() );

	// Create Player-View attachments..
	// We do this here since the attachments rely on the model that the character is using
	// and when the weapon activates as the game loads we don't yet know the model.
	
	g_pPVAttachmentMgr->CreatePVAttachments( m_hObject );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateBob()
//
//	PURPOSE:	Update WeaponModel bob
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateBob(LTFLOAT fWidth, LTFLOAT fHeight)
{
	m_fBobWidth  = fWidth;
	m_fBobHeight = fHeight;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateMovementPerturb()
//
//	PURPOSE:	Update our weapon's movement perturb value
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateMovementPerturb()
{
	ASSERT( 0 != m_pWeapon );

	// Make sure the weapon has perturb...
	if ( m_pWeapon->nMaxPerturb == 0 )
	{
		m_fMovementPerturb = 0.0f;
		return;
	}

	LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	LTFLOAT fMovePerturb = g_pPlayerMgr->GetMoveMgr()->GetMovementPercent();

	if ( fMovePerturb > 1.0f )
	{
		fMovePerturb = 1.0f;
	}

	// If walking or zoomed
	if ( ( ! ( g_pPlayerMgr->GetMoveMgr()->GetControlFlags() & BC_CFLG_RUN ) ) ||
	     ( g_pPlayerMgr->IsZoomed() ) )
	{
		fMovePerturb *= ns_vtPerturbWalkPercent.GetFloat();
	}

	// Force greater perturb when damaged certain ways
	if( g_pDamageFXMgr->IsDamageActive( DamageTypeToFlag(DT_POISON) | DamageTypeToFlag(DT_STUN) ) )
	{
		fMovePerturb = 1.0f;
	}

	// [KLS 3/20/02] - Somehow the original working NOLF rotation perturb code was removed.  
	// Added it back in so rotation perturb will work correctly...

	LTVector vPlayerRot;
	g_pPlayerMgr->GetPlayerPitchYawRoll( vPlayerRot );

    LTFLOAT fPitchDiff = (LTFLOAT)fabs(vPlayerRot.x - m_fLastPitch);
    LTFLOAT fYawDiff = (LTFLOAT)fabs(vPlayerRot.y - m_fLastYaw);
	
	m_fLastPitch = vPlayerRot.x;
	m_fLastYaw = vPlayerRot.y;

	LTFLOAT fRotPerturb = ns_vtPerturbRotationEffect.GetFloat() * (fPitchDiff + fYawDiff) / (2.0f + ns_vtFastTurnRate.GetFloat() * fDelta);
	if (fRotPerturb > 1.0f)
	{
		fRotPerturb = 1.0f;
	}

	// Determine the maximum amount of perturb that was caused...
    LTFLOAT fAdjust = Max(fRotPerturb, fMovePerturb);
    LTFLOAT fDiff = (LTFLOAT)fabs(fAdjust - m_fMovementPerturb);

	if (fAdjust > m_fMovementPerturb)
	{
		fDelta *= ns_vtPerturbIncreaseSpeed.GetFloat();
		m_fMovementPerturb += Min( fDelta, fDiff );
	}
	else if (fAdjust < m_fMovementPerturb)
	{
		fDelta *= (ns_vtPerturbDecreaseSpeed.GetFloat() * g_pPlayerStats->GetSkillModifier(SKL_AIM,AimModifiers::eCorrection));
		m_fMovementPerturb -= Min( fDelta, fDiff );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateFiringPerturb()
//
//	PURPOSE:	Update our weapon's firing perturb value
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateFiringPerturb(LTBOOL bFiredWeapon)
{
	ASSERT(m_pWeapon);

	// Make sure the weapon has perturb...
	if (m_pWeapon->nMaxPerturb == 0)
	{
		m_fFiringPerturb = 0.0f;
		return;
	}

	LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();
	LTFLOAT fFirePerturb = bFiredWeapon ? 1.0f : 0.0f;
    LTFLOAT fDiff = (LTFLOAT)fabs(fFirePerturb - m_fFiringPerturb);

	if (fFirePerturb > m_fFiringPerturb)
	{
		fDelta *= ns_vtFiringPerturbIncreaseSpeed.GetFloat();
		m_fFiringPerturb += Min(fDelta, fDiff);
	}
	else if (fFirePerturb < m_fFiringPerturb)
	{
		fDelta *= (ns_vtFiringPerturbDecreaseSpeed.GetFloat() * g_pPlayerStats->GetSkillModifier(SKL_AIM,AimModifiers::eCorrection));
		m_fFiringPerturb -= Min(fDelta, fDiff);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SetVisible()
//
//	PURPOSE:	Hide/Show the weapon model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetVisible( bool bVis /*=true*/ )
{
	if ( !m_hObject ) return;

	// Set the visible/invisible data member even if we are disabled.
	// The Disabled() function will make sure the weapon is visible/invisible
	// if SetVisible() was called while the weapon was disabled...
	m_bVisible = bVis;

	if ( m_bDisabled )
	{
		return;
	}

	// setup the visiblity flag we'll use in our engine calls
	uint32 dwVisibleFlag;
	if ( bVis )
	{
		dwVisibleFlag = FLAG_VISIBLE;
	}
	else
	{
		dwVisibleFlag = 0;
	}

	// Hide/Show weapon model...
	LTRESULT ltResult = g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, dwVisibleFlag, FLAG_VISIBLE );

	// Always hide the flash (it will be shown when needed)...
	if( m_MuzzleFlashFX.IsValid() )
	{
		m_MuzzleFlashFX.GetInstance()->Hide();
	}

	// hide/show the pv FX
	if ( bVis )
	{
		ShowPVAttachClientFX();
	}
	else
	{
		HidePVAttachClientFX();
	}

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
	SetVisibleMods( bVis );

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
	m_hSilencerSocket = INVALID_MODEL_SOCKET;

	// Make sure we have the silencer...
	MOD const *pMod = g_pWeaponMgr->GetMod( 
	        static_cast< ModType >( g_pPlayerStats->GetSilencer(m_pWeapon) ) );
	if ( !pMod ||
	     !pMod->szSocket[ 0 ] ||
	     !g_pPlayerStats->HaveMod( pMod->nId ) )
	{
		if ( m_hSilencerModel )
		{
			ASSERT( 0 != g_pCommonLT );
			g_pCommonLT->SetObjectFlags( m_hSilencerModel, OFT_Flags, 0, FLAG_VISIBLE );
		}

		return;
	}

	// Make sure we have a socket for the silencer...
	if ( m_hObject )
	{
		if ( LT_OK != g_pModelLT->GetSocket( m_hObject, pMod->szSocket, m_hSilencerSocket ) )
		{
			if ( m_hSilencerModel )
			{
				ASSERT( 0 != g_pCommonLT );
				g_pCommonLT->SetObjectFlags( m_hSilencerModel, OFT_Flags, 0, FLAG_VISIBLE );
			}

			return;
		}
	}


	// Okay create/setup the model...
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT( createStruct );

	// copy the model filename
	SAFE_STRCPY( createStruct.m_Filename, pMod->szAttachModel );

	// add the skin names
	pMod->blrAttachSkins.CopyList( 0,
	                               createStruct.m_SkinNames[ 0 ],
	                               ( MAX_CS_FILENAME_LEN + 1 ) );

	// add the render styles
	pMod->blrAttachRenderStyles.CopyList( 0,
	                                      createStruct.m_RenderStyleNames[ 0 ],
	                                      ( MAX_CS_FILENAME_LEN + 1 ) );

	// create the model
	m_hSilencerModel = CreateModelObject( m_hSilencerModel, &createStruct );

	if ( m_hSilencerModel )
	{
		// make it visible
		g_pCommonLT->SetObjectFlags( m_hSilencerModel, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

		// The vector should be OK, do a const cast to match
		// the attach scale with the non-const engine.
		g_pLTClient->SetObjectScale( m_hSilencerModel, const_cast<LTVector *>( &(pMod->vAttachScale) ) );
	}

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
	// Update the silencer...
	if ( m_hSilencerModel )
	{
		LTVector vPos;
		GetModelPos( &vPos );

		LTRotation rRot;

		if ( m_bHaveSilencer && ( INVALID_MODEL_SOCKET != m_hSilencerSocket ) )
		{
			LTransform transform;
			if ( LT_OK == g_pModelLT->GetSocketTransform( m_hObject,
			                                              m_hSilencerSocket,
			                                              transform,
			                                              LTTRUE ) )
			{
				vPos = transform.m_Pos;
				rRot = transform.m_Rot;
				g_pLTClient->SetObjectPos( m_hSilencerModel, &vPos );
				g_pLTClient->SetObjectRotation( m_hSilencerModel, &rRot );
				if (transform.m_Scale.x != 1.0f || transform.m_Scale.y != 1.0f || transform.m_Scale.z != 1.0f)
					g_pLTClient->SetObjectScale(m_hSilencerModel, &transform.m_Scale);
			}
		}
		else
		{
			// Keep the model close to us...
			g_pLTClient->SetObjectPos( m_hSilencerModel, &vPos );

			// Hide model...
			g_pCommonLT->SetObjectFlags( m_hSilencerModel,
			                             OFT_Flags,
			                             0,
			                             FLAG_VISIBLE);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::RemoveSilencer
//
//	PURPOSE:	Remove the silencer mod
//
// ----------------------------------------------------------------------- //

void CClientWeapon::RemoveSilencer()
{
	if ( m_hSilencerModel )
	{
		g_pLTClient->RemoveObject( m_hSilencerModel );
		m_hSilencerModel  = 0;
		m_hSilencerSocket = INVALID_MODEL_SOCKET;
		m_bHaveSilencer   = false;
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
	m_hScopeSocket = INVALID_MODEL_SOCKET;

	// Make sure we have the scope...
	MOD const *pMod = g_pWeaponMgr->GetMod(
	        static_cast< ModType >( g_pPlayerStats->GetScope(m_pWeapon) ) );
	if ( !pMod ||
	     !pMod->szSocket[ 0 ] ||
	     !g_pPlayerStats->HaveMod( pMod->nId ) )
	{
		if ( m_hScopeModel )
		{
			ASSERT( 0 != g_pCommonLT );
			g_pCommonLT->SetObjectFlags( m_hScopeModel, OFT_Flags, 0, FLAG_VISIBLE );
		}

		return;
	}

	// Make sure we have a socket for the scope...
	if ( m_hObject )
	{
		if ( LT_OK != g_pModelLT->GetSocket( m_hObject, pMod->szSocket, m_hScopeSocket ) )
		{
			if ( m_hScopeModel )
			{
				ASSERT( 0 != g_pCommonLT );
				g_pCommonLT->SetObjectFlags( m_hScopeModel, OFT_Flags, 0, FLAG_VISIBLE );
			}

			return;
		}
	}


	// Okay create/setup the model...
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT( createStruct );

	// copy the model filename
	SAFE_STRCPY( createStruct.m_Filename, pMod->szAttachModel );

	// add the skin names
	pMod->blrAttachSkins.CopyList( 0,
	                               createStruct.m_SkinNames[ 0 ], 
	                               ( MAX_CS_FILENAME_LEN + 1 ) );

	// add the render styles
	pMod->blrAttachRenderStyles.CopyList( 0,
	                                      createStruct.m_RenderStyleNames[ 0 ],
	                                      ( MAX_CS_FILENAME_LEN + 1 ) );

	// create the model
	m_hScopeModel = CreateModelObject( m_hScopeModel, &createStruct );

	if ( m_hScopeModel )
	{
		// make it visible
		g_pCommonLT->SetObjectFlags( m_hScopeModel, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );

		// The vector should be OK, do a const cast to match
		// the attach scale with the non-const engine.
		g_pLTClient->SetObjectScale(m_hScopeModel, const_cast<LTVector *>( &(pMod->vAttachScale) ) );
	}

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
	// Update the scope...
	if ( m_hScopeModel )
	{
		LTVector vPos;
		GetModelPos( &vPos );

		LTRotation rRot;

		if ( m_bHaveScope && ( m_hScopeSocket != INVALID_MODEL_SOCKET ) )
		{
			LTransform transform;
			if ( LT_OK == g_pModelLT->GetSocketTransform( m_hObject,
			                                              m_hScopeSocket,
			                                              transform,
			                                              LTTRUE ) )
			{
				vPos = transform.m_Pos;
				rRot = transform.m_Rot;
				g_pLTClient->SetObjectPos( m_hScopeModel, &vPos );
				g_pLTClient->SetObjectRotation( m_hScopeModel, &rRot );
				if (transform.m_Scale.x != 1.0f || transform.m_Scale.y != 1.0f || transform.m_Scale.z != 1.0f)
					g_pLTClient->SetObjectScale(m_hScopeModel, &transform.m_Scale);
			}
		}
		else
		{
			// Keep the model close to us...
			g_pLTClient->SetObjectPos( m_hScopeModel, &vPos );

			// Hide model...
			g_pCommonLT->SetObjectFlags( m_hScopeModel, 
			                             OFT_Flags,
			                             0,
			                             FLAG_VISIBLE );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::RemoveScope
//
//	PURPOSE:	Remove the scope model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::RemoveScope()
{
	if ( m_hScopeModel )
	{
		g_pLTClient->RemoveObject( m_hScopeModel );
		m_hScopeModel  = 0;
		m_hScopeSocket = INVALID_MODEL_SOCKET;
		m_bHaveScope   = false;
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

		if (HasAmmo())
		{
			if (m_nAmmoInClip == 0)
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

void CClientWeapon::Select()
{
	if ( W_INACTIVE == GetState() )
	{
		// if its inactive, make it active and visible
		SetDisable( false );

		// [KLS 3/22/02] Only show the weapon in first-person
		SetVisible((g_pPlayerMgr->IsFirstPerson() ? true : false));
	}

	SetState( W_SELECT );

	
	uint32 dwSelectAni = GetSelectAni();

	if ( m_hObject && ( INVALID_ANI != dwSelectAni ) )
	{
		uint32 dwAni = g_pLTClient->GetModelAnimation( m_hObject );

		// the "default" animation is the select animation,
		// so this fails when we try to select it,
		// going to have to try to find a workaround
		if (!IsSelectAni( dwAni ) )
		{
			LTFLOAT fRate = 1.0f;
			if ( GADGET == m_pAmmo->eType )
			{
				fRate *= g_pPlayerStats->GetSkillModifier(SKL_GADGET,GadgetModifiers::eSelect);
			}
			else
			{
				fRate *= g_pPlayerStats->GetSkillModifier(SKL_WEAPON,WeaponModifiers::eReload);
			}

			// play select animation
			PlayAnimation( dwSelectAni, true, fRate );
			
			// [RP] 11/11/02 - Since we may need the camera position and/or the flash position
			//		after the selection ani starts but before the next update of this weapon, update
			//		the positions here.  They will get reset during the update to their accurate position.

			HOBJECT hCam = g_pPlayerMgr->GetCamera();
			if( hCam )
			{
				g_pLTClient->GetObjectPos( hCam, &m_vCamPos );
				UpdateMuzzleFlash( W_SELECT, LTVector(0,0,0) );
			}
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

	// Make sure there is ammo in the clip for the first selection...
	if( m_bFirstSelection )
	{
		m_bFirstSelection = false;
		ReloadClip( false );
	}
	else if ( 0 == m_nAmmoInClip )
	{
		// If there is no ammo in our clip, reload it...
		ReloadClip( false, -1, true, true );
	}

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

	// check the gadget special case
	bool bPlayDeselectAni = true;
	if ( ( GADGET == m_pAmmo->eType ) && !HasAmmo() )
	{
		bPlayDeselectAni = false;
	}

	if ( bPlayDeselectAni )
	{
		SetState( W_DESELECT );
		if ( PlayDeselectAnimation() )
		{
			// Tell the server we're playing the deselect animation...
			LTRESULT ltResult;
			CAutoMessage cMsg;

			cMsg.Writeuint8( MID_PLAYER_CLIENTMSG );

			// say "status change"
			cMsg.Writeuint8( CP_WEAPON_STATUS );

			// tell we are deselecting the weapon
			cMsg.Writeuint8( WS_DESELECT );

			// send the message
			ltResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
			ASSERT( LT_OK == ltResult );
		}
	}
	else
	{
		m_bWeaponDeselected = true;
	}

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
	m_hBreachSocket		= INVALID_MODEL_SOCKET;
	m_hSilencerSocket	= INVALID_MODEL_SOCKET;
	m_hScopeSocket		= INVALID_MODEL_SOCKET;

    m_bHaveSilencer     = false;
    m_bHaveScope        = false;

	m_fBobHeight		= 0.0f;
	m_fBobWidth			= 0.0f;
	m_fFlashStartTime	= 0.0f;

	m_vFlashPos.Init();
	m_vFlashOffset.Init();

    m_bFire                 = false;
	m_eLastFireType			= FT_NORMAL_FIRE;
    m_bCanSetLastFire       = false;
	m_nTracerNumber			= 0;
	m_fLastPitch			= 0.0f;
	m_fLastYaw				= 0.0f;

	m_nSelectAni			= INVALID_ANI;
	m_nDeselectAni			= INVALID_ANI;
	m_nReloadAni			= INVALID_ANI;

	m_nAltSelectAni			= INVALID_ANI;
	m_nAltDeselectAni		= INVALID_ANI;
	m_nAltDeselect2Ani		= INVALID_ANI;
	m_nAltReloadAni			= INVALID_ANI;

	m_wIgnoreFX				= 0;
    m_bWeaponDeselected     = false;

	m_nNewAmmoInClip		= 0;

    m_bFireKeyDownLastUpdate    = false;
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

	for (i=0; i < WM_MAX_ALTFIRE_ANIS; i++)
	{
		m_nAltFireAnis[i] = INVALID_ANI;
	}

	for (i=0; i < WM_MAX_ALTIDLE_ANIS; i++)
	{
		m_nAltIdleAnis[i] = INVALID_ANI;
	}
	
	for( i = 0; i < WM_MAX_PV_ATTACH_CLIENTFX; ++i )
	{
		m_bPVAttachClientFXHidden[i] = true;
	}
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
    if (!m_pWeapon || !m_pAmmo) return false;
	if( m_hObject ) return true;

	// reset any necessary data
	ResetData();

	// create the model
	bool result = CreateWeaponModel();
	if ( !result )
	{
		Term();
		return false;
	}

	// do one update of the weapon model position to put
	// it in the right place
	UpdateWeaponPosition( LTVector() );

	// initialize the animations for this model
	InitAnimations();

	// [RP 8/27/02] First play the select animation and THEN create the attachments
	// such as the ClientFX and any mods like the silencer THEN set the animation to 
	// an Idle animation.  We need to do this bit of wackiness to make sure the attachments
	// are created in the correct selected position and will render off screen when the
	// weapon becomes visible.

	uint32 dwSelectAni = GetSelectAni();

	PlayAnimation( dwSelectAni );

	// create all client fx
	CreatePVAttachClientFX();
	CreateMuzzleFlash();

	// create any mods
	CreateMods();

	// Create Player-View attachments..
	
	g_pPVAttachmentMgr->CreatePVAttachments( m_hObject );


	// Make sure the model doesn't start out on the select animation (if it does
	// it won't play the select animation when it is selected)...

	if ( INVALID_ANI != dwSelectAni )
	{
		uint32 dwAni = g_pLTClient->GetModelAnimation( m_hObject );

		if ( IsSelectAni( dwAni ) )
		{
			PlayAnimation(GetSubtleIdleAni());
		}
	}


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

	// hide the weapon
	SetVisible( false );

	// disable the weapon
	SetDisable( true );

	// turn off all keyframed ClientFX
	for ( CLIENTFX_LINK_NODE* pNode = m_KeyframedClientFX.m_pNext; pNode != NULL; pNode = pNode->m_pNext )
	{
		// turn off the effect
		g_pClientFXMgr->ShutdownClientFX( &pNode->m_Link );
	}

	// destroy the list of keyframed ClientFX
	m_KeyframedClientFX.DeleteList();

	// remove the mods
	RemoveMods();

	// remove player-view attachments
	g_pPVAttachmentMgr->RemovePVAttachments();

	// remove all client fx
	RemoveMuzzleFlash();
	RemovePVAttachClientFX();

	// remove the weapon model
	RemoveWeaponModel();

	KillLoopSound();
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
	ASSERT( 0 != m_pWeapon );

	if ( m_pWeapon->bInfiniteAmmo )
	{
		// infinite ammo, we have ammo
		return true;
	}
	else
	{
		for ( int i = 0; i < m_pWeapon->nNumAmmoIds; ++i )
		{
			ASSERT( 0 != g_pPlayerStats );
			if ( 0 < g_pPlayerStats->GetAmmoCount( m_pWeapon->aAmmoIds[ i ] ) )
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

bool CClientWeapon::CanUseAmmo( uint8 nAmmoId ) const
{
	if ( !m_pWeapon ||
	     !g_pWeaponMgr ||
	     !g_pWeaponMgr->IsValidAmmoId( nAmmoId ) )
	{
		return false;
	}

	for ( int i = 0; i < m_pWeapon->nNumAmmoIds; ++i)
	{
		if ( m_pWeapon->aAmmoIds[ i ] == nAmmoId )
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

bool CClientWeapon::CanChangeToAmmo( uint8 nAmmoId ) const
{
	int i;

	// Is the ID valid?
	ASSERT( 0 != g_pWeaponMgr );
	if ( !g_pWeaponMgr->IsValidAmmoId( nAmmoId ) )
	{
		return false;
	}

	// Is the ID valid for this weapon?  Do we have ammo?
	bool bFoundAndHasAmmo = false;
	for ( i = 0; ( !bFoundAndHasAmmo ) && ( i < m_pWeapon->nNumAmmoIds ); ++i )
	{
		if ( nAmmoId == m_pWeapon->aAmmoIds[ i ] )
		{
			if ( 0 < g_pPlayerStats->GetAmmoCount( nAmmoId ) )
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

uint8 CClientWeapon::GetNextAvailableAmmo( uint8 nGivenAmmoId )
{
	ASSERT( 0 != g_pWeaponMgr );

	uint8 nCurrAmmoId;

	// If the given type is invalid, use the current ammo ID
	if ( !g_pWeaponMgr->IsValidAmmoId( nGivenAmmoId ) )
	{
		nCurrAmmoId = m_nAmmoId;
	}
	else
	{
		nCurrAmmoId = nGivenAmmoId;
	}

	int nNewAmmoId = nCurrAmmoId;
	int nOriginalAmmoIndex = 0;
	int nCurAmmoIndex = 0;
	int nAmmoCount = 0;

	// Find the current ammo in the list of ammo 
	// supported by the current weapon
	ASSERT( 0 != m_pWeapon );
	for ( int i = 0; i < m_pWeapon->nNumAmmoIds; ++i )
	{
		if ( nCurrAmmoId == m_pWeapon->aAmmoIds[ i ] )
		{
			nOriginalAmmoIndex = i;
			nCurAmmoIndex = i;
			break;
		}
	}

	while ( 1 )
	{
		nCurAmmoIndex++;

		// check for wrap
		if ( nCurAmmoIndex >= m_pWeapon->nNumAmmoIds )
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
		nAmmoCount = g_pPlayerStats->GetAmmoCount( m_pWeapon->aAmmoIds[ nCurAmmoIndex ] );
		if (0 < nAmmoCount )
		{
			nNewAmmoId = m_pWeapon->aAmmoIds[ nCurAmmoIndex ];
			break;
		}
	}

	return nNewAmmoId;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetBestAvailableAmmoId()
//
//	PURPOSE:	Get the best available ammo id for this weapon.
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::GetBestAvailableAmmoId( int *nAmmoId ) const
{
	// start off assuming there is no ammo
	*nAmmoId = WMGR_INVALID_ID;

	if ( !g_pPlayerStats )
	{
		return LTFALSE;
	}

	// intermediate variables to keep track of the best ammo id found
	int nAmmoBest = WMGR_INVALID_ID;
	LTFLOAT fMaxPriority = -1.0f;

	// go through all the ammo ids
	for ( int i = 0; i < m_pWeapon->nNumAmmoIds; ++i )
	{
		if ( 0 < g_pPlayerStats->GetAmmoCount( m_pWeapon->aAmmoIds[ i ] ) )
		{
			// we do have this ammo

			// remember the ammo id
			int nAmmo = m_pWeapon->aAmmoIds[ i ];

			// get the ammo data
			ASSERT( 0 != g_pWeaponMgr );
			AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( nAmmo );
			ASSERT( 0 != pAmmo );

			// compare it to the previous priorities
			if ( pAmmo->fPriority > fMaxPriority )
			{
				// this is the best we've found so far, keep track of it
				nAmmoBest = nAmmo;
				fMaxPriority = pAmmo->fPriority;
			}
		}
	}

	if ( WMGR_INVALID_ID != nAmmoBest )
	{
		// we found the best ammo id
		*nAmmoId = nAmmoBest;
		return true;
	}

	// If we get to here (which we shouldn't), just use the default ammo
	// i if this weapon uses infinite ammo...
	if ( m_pWeapon->bInfiniteAmmo )
	{
		*nAmmoId = m_pWeapon->nDefaultAmmoId;
		return true;
	}

	// nothing found, admit failure
	*nAmmoId = WMGR_INVALID_ID;
	return false;
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
	SetVisibleMods( !bHideMods );
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

	m_bFirstSelection = pMsg->Readbool();
	m_nAmmoId	  =	pMsg->Readuint8();
	m_nAmmoInClip = pMsg->Readint32();
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

	pMsg->Writebool( m_bFirstSelection );
	pMsg->Writeuint8( m_nAmmoId );
	pMsg->Writeint32( m_nAmmoInClip );
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::CreateWeaponModel()
//
//	PURPOSE:	Create the weapon model itself.
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::CreateWeaponModel()
{
	ASSERT( 0 != g_pLTClient );
	ASSERT( 0 != m_pWeapon );
	ASSERT( 0 == m_hObject );

	ObjectCreateStruct createStruct;
	
	// put this weapon's model information in the create struct
	PopulateCreateStruct( &createStruct );

	m_hObject = CreateModelObject( m_hObject, &createStruct );
	if ( !m_hObject )
	{
		return false;
	}

	// needed for gadget
	DoSpecialCreateModel();

	// Setup Breach socket (if it exists)...
	m_hBreachSocket = INVALID_MODEL_SOCKET;
	if ( m_hObject )
	{
		if ( LT_OK != g_pModelLT->GetSocket(m_hObject, "Breach", m_hBreachSocket) )
		{
			m_hBreachSocket = INVALID_MODEL_SOCKET;
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::CreateModelObject
//
//	PURPOSE:	Create a weaponmodel model object
//
// ----------------------------------------------------------------------- //

HOBJECT CClientWeapon::CreateModelObject( HOBJECT hOldObj, ObjectCreateStruct *createStruct )
{
	ASSERT( 0 != m_pWeapon );
	if ( !m_pWeapon )
	{
		return static_cast< HOBJECT >( 0 );
	}

	HOBJECT hObj = hOldObj;

	if ( !hObj )
	{
		createStruct->m_ObjectType = OT_MODEL;
		createStruct->m_Flags     |= /*FLAG_VISIBLE |*/ FLAG_REALLYCLOSE;
		createStruct->m_Flags2    |= FLAG2_DYNAMICDIRLIGHT;

		hObj = g_pLTClient->CreateObject( createStruct );
		if ( !hObj ) return static_cast< HOBJECT >( 0 );
	}
	else
	{
		if ( LT_OK != g_pCommonLT->SetObjectFilenames( hObj, createStruct ) )
		{
			return static_cast< HOBJECT >( 0 );
		}
	}

	// we want to be notified for model keys
	LTRESULT ltResult = g_pCommonLT->SetObjectFlags(hObj, OFT_Client, CF_NOTIFYMODELKEYS, CF_NOTIFYMODELKEYS);
	ASSERT( LT_OK == ltResult );

	// Reset the model animation...

	g_pModelLT->SetLooping( hObj, MAIN_TRACKER, false );
	g_pModelLT->SetPlaying( hObj, MAIN_TRACKER, false );

	return hObj;
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
	ASSERT( 0 != g_pClientSoundMgr );
	ASSERT( 0 != g_pLTClient );
	ASSERT( 0 != g_pPlayerStats );

	WeaponState eRet = W_IDLE;

	// determine if we have infinite ammo, if if not, how much ammo we actually have
	int nAmmo;
	bool bInfiniteAmmo = ( g_bInfiniteAmmo || m_pWeapon->bInfiniteAmmo );
	if ( bInfiniteAmmo )
	{
		// dummy value for infinite ammo
		nAmmo = INFINITE_AMMO_AMOUNT;
	}
	else
	{
		// current amount of ammo
		nAmmo = g_pPlayerStats->GetAmmoCount( m_nAmmoId );
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

WeaponState CClientWeapon::UpdateModelState( bool bFire )
{
	WeaponState eRet = W_IDLE;

	// Determine what we should be doing...
	// (mostly updates animations, also
	// updates a couple supporting variables)
	if (bFire)
	{
		UpdateFiring();
	}
	else
	{
		UpdateNonFiring();
	}

	if ( m_bFire )
	{
		// gadgets...always the special case  :-(
		bool bGadgetSpecialCase = m_pAmmo->eType != GADGET;

		// doesn't actually fire, just updates the ammo 
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
//	FUNCTION:	CClientWeapon::UpdateWeaponPosition()
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateWeaponPosition( LTVector const &vOffset )
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

	// set the weapon model position
	g_pLTClient->SetObjectPos( m_hObject, &vNewPos );
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
	m_bCanSetLastFire = true;

	switch( GetState())
	{
		case W_RELOADING:
		{
			if ( !PlayReloadAnimation() )
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
				SetState( W_FIRING );
			}
		}
		break;

		case W_FIRING:
		case W_FIRING_NOAMMO:
		{
			if ( PlayFireAnimation( true ) )
			{
				m_bCanSetLastFire = false;
			}
		}
		break;
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
	m_bCanSetLastFire = true;

	switch( GetState())
	{
		case W_FIRING:
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
		break;

		case W_FIRING_NOAMMO:
		{
			SetState( W_IDLE );
		}
		break;

		case W_RELOADING:
		{
			if ( !PlayReloadAnimation() )
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
				SetState( W_IDLE );
			}
		}
		break;

		case W_IDLE:
		{
			PlayIdleAnimation();
		}
		break;
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
	ASSERT( 0 != m_hObject );

	m_nSelectAni        = g_pLTClient->GetAnimIndex( m_hObject, ns_szSelectAnimationName );
	m_nDeselectAni      = g_pLTClient->GetAnimIndex( m_hObject, ns_szDeselectAnimationName );
	m_nReloadAni        = g_pLTClient->GetAnimIndex( m_hObject, ns_szReloadAnimationName );

	m_nAltSelectAni     = g_pLTClient->GetAnimIndex(m_hObject, ns_szAltSelectAnimationName );
	m_nAltDeselectAni   = g_pLTClient->GetAnimIndex(m_hObject, ns_szAltDeselectAnimationName );
	m_nAltDeselect2Ani  = g_pLTClient->GetAnimIndex(m_hObject, ns_szAltDeselect2AnimationName );
	m_nAltReloadAni     = g_pLTClient->GetAnimIndex(m_hObject, ns_szAltReloadAnimationName );

	m_nPreFireAni       = g_pLTClient->GetAnimIndex(m_hObject, ns_szPreFireAnimationName );
	m_nPostFireAni      = g_pLTClient->GetAnimIndex(m_hObject, ns_szPostFireAnimationName );

	char buf[30];
	int i;

	for ( i = 0; i < WM_MAX_IDLE_ANIS; ++i )
	{
		sprintf( buf, "%s%d", ns_szIdleAnimationBasename, i );
		m_nIdleAnis[ i ] = g_pLTClient->GetAnimIndex( m_hObject, buf );
	}

	for ( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		if ( i > 0 )
		{
			sprintf( buf, "Fire%d", i );
		}
		else
		{
			sprintf( buf, "Fire" );
		}

		m_nFireAnis[ i ] = g_pLTClient->GetAnimIndex( m_hObject, buf );
	}

	for ( i = 0; i < WM_MAX_ALTIDLE_ANIS; ++i )
	{
		sprintf( buf, "%s%d", ns_szAltIdleAnimationBasename, i );
		m_nAltIdleAnis[ i ] = g_pLTClient->GetAnimIndex( m_hObject, buf );
	}

	for ( i = 0; i < WM_MAX_ALTFIRE_ANIS; ++i )
	{
		if ( i > 0 )
		{
			sprintf( buf, "%s%d", ns_szAltFireAnimationBasename, i );
		}
		else
		{
			sprintf( buf, ns_szAltFireAnimationName );
		}

		m_nAltFireAnis[ i ] = g_pLTClient->GetAnimIndex( m_hObject, buf );
	}

	// See if there are Ammo-override animations...
	if ( m_pAmmo->pAniOverrides )
	{
		// Set new animations...
		SetAmmoOverrideAnis( bAllowSelectOverride );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SetAmmoOverrideAnis
//
//	PURPOSE:	Set the ammo specific override animations...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetAmmoOverrideAnis( bool bAllowSelectOverride )
{
	if ( !m_hObject || !m_pAmmo || !m_pAmmo->pAniOverrides ) return;

	if ( bAllowSelectOverride && m_pAmmo->pAniOverrides->szSelectAni[ 0 ] )
	{
		m_nSelectAni = g_pLTClient->GetAnimIndex( m_hObject, m_pAmmo->pAniOverrides->szSelectAni );
	}

	if ( m_pAmmo->pAniOverrides->szDeselectAni[ 0 ] )
	{
		m_nDeselectAni = g_pLTClient->GetAnimIndex( m_hObject, m_pAmmo->pAniOverrides->szDeselectAni );
	}

	if ( m_pAmmo->pAniOverrides->szReloadAni[ 0 ] )
	{
		m_nReloadAni = g_pLTClient->GetAnimIndex( m_hObject, m_pAmmo->pAniOverrides->szReloadAni );
	}

	int i;
	for ( i = 0; i < WM_MAX_IDLE_ANIS; ++i )
	{
		if ( i < m_pAmmo->pAniOverrides->nNumIdleAnis )
		{
			if ( m_pAmmo->pAniOverrides->szIdleAnis[ i ][ 0 ] )
			{
				m_nIdleAnis[ i ] = g_pLTClient->GetAnimIndex( m_hObject, m_pAmmo->pAniOverrides->szIdleAnis[i] );
			}
		}
		else
		{
			m_nIdleAnis[ i ] = INVALID_ANI;
		}
	}

	for ( i = 0; i < WM_MAX_FIRE_ANIS; ++i )
	{
		if ( i < m_pAmmo->pAniOverrides->nNumFireAnis )
		{
			if ( m_pAmmo->pAniOverrides->szFireAnis[ i ][ 0 ] )
			{
				m_nFireAnis[ i ] = g_pLTClient->GetAnimIndex( m_hObject, m_pAmmo->pAniOverrides->szFireAnis[ i ] );
			}
		}
		else
		{
			m_nFireAnis[ i ] = INVALID_ANI;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::PlayAnimation
//
//	PURPOSE:	Play an animation
//
// ----------------------------------------------------------------------- //

void CClientWeapon::PlayAnimation( uint32 dwAni, bool bReset /*= true*/,
                                   float fRate /*=1.0f*/, bool bLooping /*=false*/ )
{
	if( !m_hObject )
		return;
	
	LTRESULT ltResult;

	if ( 0 < nsfOverrideRate )
	{
		fRate = nsfOverrideRate;
	}

	g_pModelLT->SetPlaying( m_hObject, MAIN_TRACKER, true );
	g_pModelLT->SetLooping( m_hObject, MAIN_TRACKER, bLooping );
	g_pModelLT->SetCurAnim( m_hObject, MAIN_TRACKER, dwAni );
		
	ltResult = g_pModelLT->SetAnimRate( m_hObject, MAIN_TRACKER, fRate );
	ASSERT( LT_OK == ltResult );

	if ( bReset )
	{
		// Start from beginning
		ltResult = g_pLTClient->ResetModelAnimation( m_hObject );
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
	ASSERT( 0 != g_pLTClient );
	ASSERT( 0 != g_pPlayerStats );

	uint32 dwSelectAni = GetSelectAni();

	if ( !m_hObject || ( dwSelectAni == INVALID_ANI ) )
	{
		// object or animation not valid
		return false;
	}

	uint32 dwAni	= g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState	= g_pLTClient->GetModelPlaybackState( m_hObject );

	bool bIsSelectAni = IsSelectAni( dwAni );
	if ( bIsSelectAni && ( dwState & MS_PLAYDONE ) )
	{
		// animation done
		return false;
	}
	
	if ( !bIsSelectAni )
	{
		// change to a select animation

		LTFLOAT fRate = 1.0f;
		if ( m_pAmmo->eType == GADGET )
		{
			// gadgets...always the special case
			fRate *= g_pPlayerStats->GetSkillModifier(SKL_GADGET,GadgetModifiers::eSelect);
		}
		else
		{
			fRate *= g_pPlayerStats->GetSkillModifier(SKL_WEAPON,WeaponModifiers::eReload);
		}

		PlayAnimation( dwSelectAni, false, fRate );
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
	uint32 dwDeselectAni = GetDeselectAni();

	if ( !m_hObject || ( dwDeselectAni == INVALID_ANI ) )
	{
		// model or animation invalid

		// mark "deselected" and handle details during next update
		m_bWeaponDeselected = true;
		return false;
	}

	uint32 dwAni	= g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState	= g_pLTClient->GetModelPlaybackState( m_hObject );

	bool bIsDeselectAni = IsDeselectAni( dwAni );

	if ( bIsDeselectAni && ( dwState & MS_PLAYDONE ) )
	{
		// animation is done

		// mark "deselected" and handle details during next update
		m_bWeaponDeselected = true;
		return false;
	}

	if ( !bIsDeselectAni )
	{
		// change to a deselect animation

		LTFLOAT fRate = 1.0f;
		if ( m_pAmmo->eType == GADGET )
		{
			// gadgets...always the special case
			fRate *= g_pPlayerStats->GetSkillModifier(SKL_GADGET,GadgetModifiers::eSelect);
		}
		else
		{
			fRate *= g_pPlayerStats->GetSkillModifier(SKL_WEAPON,WeaponModifiers::eReload);
		}

		PlayAnimation( dwDeselectAni, false, fRate );
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

bool CClientWeapon::PlayReloadAnimation()
{
	uint32 dwReloadAni = GetReloadAni();

	if ( !m_hObject || ( INVALID_ANI == dwReloadAni ) )
	{
		return false;
	}

	ASSERT( 0 != g_pLTClient );
	uint32 dwAni	= g_pLTClient->GetModelAnimation( m_hObject );
	uint32 dwState	= g_pLTClient->GetModelPlaybackState( m_hObject );

	bool bCurAniDone = !!( dwState & MS_PLAYDONE );
	bool bIsFireAni = IsFireAni(dwAni);

	bool bCanPlay  = ( !bIsFireAni || bCurAniDone ||
	                   g_pLTClient->GetModelLooping( m_hObject ) );

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
		m_nAmmoInClip = m_nNewAmmoInClip;

		// Update the player's stats...
		int nAmmo = g_pPlayerStats->GetAmmoCount( m_nAmmoId );

		g_pPlayerStats->UpdateAmmo( m_nWeaponId, m_nAmmoId, nAmmo );
		g_pPlayerStats->UpdatePlayerWeapon( m_nWeaponId, m_nAmmoId );

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
		LTFLOAT fRate = 1.0f;
		
		// Scale the animation rate...

		if( m_pWeapon )
		{
			fRate *= m_pWeapon->fReloadAnimRateScale;
		}

		// Scale the animation rate based on skill modifiers...
		
		if ( m_pAmmo->eType == GADGET )
		{
			fRate *= g_pPlayerStats->GetSkillModifier(SKL_GADGET,GadgetModifiers::eSelect);
		}
		else
		{
			fRate *= g_pPlayerStats->GetSkillModifier(SKL_WEAPON,WeaponModifiers::eReload);
		}

		PlayAnimation( dwReloadAni, false, fRate );

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

	if ( !m_hObject || g_pPlayerMgr->IsZoomed() )
	{
		// would be better to move the second check somewhere else
		return false;
	}

	// determine of the current animation is done
	bool bCurAniDone = !!( g_pLTClient->GetModelPlaybackState( m_hObject ) & MS_PLAYDONE );

	// Make sure idle animation is done if one is currently playing...
	uint32 dwAni = g_pLTClient->GetModelAnimation( m_hObject );
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
	LTFLOAT fTime = g_pLTClient->GetTime();
	bool bPlayIdle = false;
	if ( ( fTime > m_fNextIdleTime ) && bCurAniDone )
	{
		bPlayIdle = !bMoving;
		m_fNextIdleTime = GetNextIdleTime();
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

bool CClientWeapon::PlayFireAnimation( bool bResetAni )
{
	// Can only set the last fire type if a fire animation isn't playing
	// (i.e., we'll assume this function will return false)...

	uint32 dwAni			= g_pLTClient->GetModelAnimation(m_hObject);
	uint32 dwState			= g_pLTClient->GetModelPlaybackState(m_hObject);

	uint32 dwPreFireAni		= GetPreFireAni();
	uint32 dwFireAni		= GetFireAni( m_eLastFireType );
	uint32 dwPostFireAni	= GetPostFireAni();

	bool bHasPreFireAni		= !!(INVALID_ANI != dwPreFireAni);
	bool bHasFireAni		= !!(INVALID_ANI != dwFireAni);
	bool bHasPostFireAni	= !!(INVALID_ANI != dwPostFireAni);

	bool bIsFireAni			= IsFireAni( dwAni );

	bool bIsMainFireAni		= IsFireAni( dwAni, true );
	bool bIsPreFireAni		= IsPreFireAni( dwAni );
	bool bIsPostFireAni		= IsPostFireAni( dwAni );

	bool bIsCurAniDone		= !!(dwState & MS_PLAYDONE);

	
	// Scale the animation rate...
	
	float fRate = 1.0f;
	if( m_pWeapon )
	{
		fRate *= m_pWeapon->fFireAnimRateScale;
	}

	// KLS 4/26/02 - Re-wrote this logic to fix some bugs and to make things
	// a bit clearer.

	// First handle case of not currently playing any type of fire animation
	// but wanting to fire the weapon...

	if ( !bIsFireAni && bResetAni )
	{
		// If we have a pre-fire animation, play it...
		if ( bHasPreFireAni )
		{
			PlayAnimation( dwPreFireAni, true, fRate );
			return true;
		}

		// No pre-fire, so play the fire animation...
		if ( bHasFireAni )
		{
			PlayAnimation( dwFireAni, true, fRate );
			return true;
		}
		
		// Shouldn't really happen, but maybe we only have a post-fire ani...
		if ( bHasPostFireAni )
		{
			PlayAnimation( dwPostFireAni, true, fRate );
			return true;
		}

		// If we got here, it means we don't have any fire anis...
		return false;
	}


	// Now handle the case where we are already playing a fire animation...

	if ( bIsFireAni )
	{
		// Determine what animation to play if we're done firing...

		if ( !bIsCurAniDone )
		{
			return true;
		}


		// The current animation is done, so figure out what to do now...

		if ( bIsPreFireAni )
		{
			// We just finished playing the pre-fire ani, so try and play the main
			// fire ani, or the post-fire ani if we don't have main fire ani...

			if ( bHasFireAni )
			{
				PlayAnimation( dwFireAni, true, fRate );
				return true;
			}
			else if ( bHasPostFireAni )
			{
				PlayAnimation( dwPostFireAni, true, fRate );
				return true;
			}
		}
		else if ( bIsMainFireAni )
		{
			// We just finished playing the main fire ani, play it again if bResetAni
			// is true and we have ammo (i.e., they are holding down the fire key).  
			// Else, if we have a post-fire ani, play it...
		
			if ( bResetAni && g_pPlayerStats->GetAmmoCount(m_nAmmoId) > 0 )
			{
				PlayAnimation( dwFireAni, true, fRate );
				return true;
			}
			else if ( bHasPostFireAni )
			{
				PlayAnimation( dwPostFireAni, true, fRate );
				return true;
			}
		}
		else if( bIsPostFireAni )
		{
			// We just finished playing the post-fire ani, start the cycle again if bResetAni
			// is true and we have ammo (i.e., they are holding down the fire key).  
			
			if( bResetAni && g_pPlayerStats->GetAmmoCount(m_nAmmoId) > 0  )
			{
				if( bHasPreFireAni )
				{
					PlayAnimation( dwPreFireAni, true, fRate );
					return true;
				}
				else if( bHasFireAni )
				{
					PlayAnimation( dwFireAni, true, fRate );
					return true;
				}
			}

		}
	}

	// If we got here, it means we're done firing...

	DoSpecialEndFire();
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

	if ( ( dwAni == m_nSelectAni ) || ( dwAni == m_nAltSelectAni ) )
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
	if ( m_bUsingAltFireAnis )
	{
		return m_nAltSelectAni;
	}
	else
	{
		return m_nSelectAni;
	}
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

	if ( dwAni == m_nDeselectAni ||
	     dwAni == m_nAltDeselectAni ||
	     dwAni == m_nAltDeselect2Ani )
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
	uint32 dwAni = INVALID_ANI;

	if ( m_bUsingAltFireAnis )
	{
		// This is code from the WeaponModel which
		// did its own weapon switching.  Since alt-fire
		// is broken, its here for reference if anyone
		// wants to resurrect it.
		/*
		// If we're actually changing weapons make sure we use the
		// currect AltDeselect animation...
		if ( m_nRequestedWeaponId != WMGR_INVALID_ID &&
		     m_nRequestedWeaponId != m_nWeaponId )
		{
			dwAni = m_nAltDeselect2Ani;
		}
		else
		{
			dwAni = m_nAltDeselectAni;
		}
		*/
	}
	else
	{
		dwAni = m_nDeselectAni;
	}

	return dwAni;
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

	if ( ( dwAni == m_nReloadAni ) || ( dwAni == m_nAltReloadAni ) )
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
	if ( m_bUsingAltFireAnis )
	{
		return m_nAltReloadAni;
	}
	else
	{
		return m_nReloadAni;
	}
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

	// start at 1 because 0 is reserved for the subtle idle
	for ( i = 1; i < WM_MAX_ALTIDLE_ANIS; ++i )
	{
		if ( m_nAltIdleAnis[i] == dwAni )
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

	if ( m_bUsingAltFireAnis )
	{
		uint32 dwValidAltIdleAnis[ WM_MAX_ALTIDLE_ANIS ];

		// Note that we skip the first ani, this is reserved for
		// the subtle idle ani...
		for ( int i = 1; i < WM_MAX_ALTIDLE_ANIS; ++i )
		{
			if ( m_nAltIdleAnis[ i ] != INVALID_ANI )
			{
				dwValidAltIdleAnis[ nNumValid ] = m_nAltIdleAnis[ i ];
				++nNumValid;
			}
		}

		if ( 0 < nNumValid )
		{
			return dwValidAltIdleAnis[ GetRandom( 0, ( nNumValid - 1 ) ) ];
		}
	}
	else  // Normal idle anis
	{
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
	if ( m_bUsingAltFireAnis )
	{
		return m_nAltIdleAnis[ 0 ];
	}
	else
	{
		return m_nIdleAnis[ 0 ];
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetNextIdleTime()
//
//	PURPOSE:	Determine the next time we should play an idle animation
//
// ----------------------------------------------------------------------- //

LTFLOAT CClientWeapon::GetNextIdleTime() const
{
	return g_pLTClient->GetTime() + GetRandom( WEAPON_MIN_IDLE_TIME, WEAPON_MAX_IDLE_TIME );
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

	for ( i = 0; i < WM_MAX_ALTFIRE_ANIS; ++i )
	{
		if ( m_nAltFireAnis[ i ] == dwAni )
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

uint32 CClientWeapon::GetFireAni( FireType eFireType ) const
{
	int nNumValid = 0;

	if ( ( ( eFireType == FT_ALT_FIRE ) && CanUseAltFireAnis() ) ||
	     ( m_bUsingAltFireAnis && ( eFireType == FT_NORMAL_FIRE ) ) )
	{
		uint32 dwValidAltFireAnis[ WM_MAX_ALTFIRE_ANIS ];

		for ( int i = 0; i < WM_MAX_ALTFIRE_ANIS; ++i )
		{
			if ( INVALID_ANI != m_nAltFireAnis[ i ] )
			{
				dwValidAltFireAnis[ nNumValid ] = m_nAltFireAnis[ i ];
				++nNumValid;
			}
		}

		if ( nNumValid > 0 )
		{
			return dwValidAltFireAnis[ GetRandom( 0, ( nNumValid - 1 ) ) ];
		}
	}
	else if ( eFireType == FT_NORMAL_FIRE )
	{
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
//	ROUTINE:	CClientWeapon::CanUseAltFireAnis()
//
//	PURPOSE:	Can we use alt-fire anis?
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::CanUseAltFireAnis() const
{
	return ( INVALID_ANI != m_nAltSelectAni );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetModelPos()
//
//	PURPOSE:	Get the position of the weapon model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::GetModelPos( LTVector *vPos ) const
{
	ASSERT( 0 != m_hObject );
	ASSERT( 0 != vPos );

	g_pLTClient->GetObjectPos( m_hObject, vPos );
	*vPos += m_vCamPos;
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
	// Remove the silencer model...
	RemoveSilencer();

	// Remove the scope model...
	RemoveScope();
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::SetVisibleMods()
//
//	PURPOSE:	Show/hide the mods
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SetVisibleMods( bool bVis /*=true*/ )
{
	// setup the visiblity flag we'll use in our engine calls
	uint32 dwVisibleFlag;
	if ( bVis )
	{
		dwVisibleFlag = FLAG_VISIBLE;
	}
	else
	{
		dwVisibleFlag = 0;
	}

	// Hide/Show silencer...
	if ( m_hSilencerModel )
	{
		g_pCommonLT->SetObjectFlags( m_hSilencerModel, OFT_Flags, dwVisibleFlag, FLAG_VISIBLE );
	}

	// Hide/Show scope...
	if ( m_hScopeModel )
	{
		g_pCommonLT->SetObjectFlags( m_hScopeModel, OFT_Flags, dwVisibleFlag, FLAG_VISIBLE );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::GetModelRot()
//
//	PURPOSE:	Get the rotation of the weapon model
//
// ----------------------------------------------------------------------- //

void CClientWeapon::GetModelRot( LTRotation *rRot ) const
{
	ASSERT( 0 != rRot );

	// weapon rotation always matches the camera
	*rRot = m_rCamRot;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::CreateMuzzleFlash
//
//	PURPOSE:	Create the muzzle flash
//
// ----------------------------------------------------------------------- //

void CClientWeapon::CreateMuzzleFlash()
{
	ASSERT( 0 != g_pModelLT );
	ASSERT( 0 != g_pClientFXMgr );
	ASSERT( 0 != m_pWeapon );

	// Remove the old FX Instance
	RemoveMuzzleFlash();

	// If our FX Instance is not created do so now...
	if( !m_MuzzleFlashFX.IsValid() )
	{
		if ( '\0' != m_pWeapon->szPVMuzzleFxName[ 0 ] )
		{ 
			// This is always the player view FX
			CLIENTFX_CREATESTRUCT fxInit( m_pWeapon->szPVMuzzleFxName, FXFLAG_LOOP | FXFLAG_REALLYCLOSE, m_hObject ); 
			g_pClientFXMgr->CreateClientFX( &m_MuzzleFlashFX, fxInit, false );
			if( m_MuzzleFlashFX.IsValid() ) 
			{
				m_MuzzleFlashFX.GetInstance()->Hide();
				m_MuzzleFlashFX.GetInstance()->SetPos( LTVector(0.0f,0.0f,0.0f), LTVector(0.0f,0.0f,0.0f) );
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::RemoveMuzzleFlash
//
//	PURPOSE:	Destroys the muzzle flash
//
// ----------------------------------------------------------------------- //

void CClientWeapon::RemoveMuzzleFlash()
{
	if ( m_MuzzleFlashFX.IsValid() )
	{
		// The FX will run itself out and get deleted in the mgr but we should still null our ptr
		ASSERT( 0 != g_pClientFXMgr );
		g_pClientFXMgr->ShutdownClientFX( &m_MuzzleFlashFX );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::StartFlash()
//
//	PURPOSE:	Start the muzzle flash
//
// ----------------------------------------------------------------------- //

void CClientWeapon::StartMuzzleFlash()
{
	m_fFlashStartTime = g_pLTClient->GetTime();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::UpdateMuzzleFlash()
//
//	PURPOSE:	Update muzzle flash state
//
// ----------------------------------------------------------------------- //

void CClientWeapon::UpdateMuzzleFlash( WeaponState eState, LTVector const &vOffset )
{
	// [RP] Always update the flash position, otherwise weapons without
	// muzzleflashes or weapons with silencers will not fire correctly.
	
	// get weapon and muzzle offsets
	LTVector vWeaponOffset = GetWeaponOffset();
  	LTVector vMuzzleOffset = GetMuzzleOffset();

	// camera axes
	LTVector vCamU, vCamR, vCamF;
	vCamU = m_rCamRot.Up();
	vCamR = m_rCamRot.Right();
	vCamF = m_rCamRot.Forward();

	// set the flash offset to be a combination of 
	// the weapon, muzzle, and bobbing
	// NOTE: in camera space 
	m_vFlashOffset = vWeaponOffset + vMuzzleOffset;
	m_vFlashOffset.x += m_fBobWidth;
	m_vFlashOffset.y += m_fBobHeight;

	// add extra offset
	m_vFlashOffset += vOffset;

	// deterime the world space position of the flash,
	// offset using the camera's local axis
	LTVector vFlashCameraRelativeOffestInWorldSpace( 0.0f, 0.0f, 0.0f );
	vFlashCameraRelativeOffestInWorldSpace  = vCamR * m_vFlashOffset.x;
	vFlashCameraRelativeOffestInWorldSpace += vCamU * m_vFlashOffset.y;
	vFlashCameraRelativeOffestInWorldSpace += vCamF * m_vFlashOffset.z;

	m_vFlashPos = m_vCamPos + vFlashCameraRelativeOffestInWorldSpace;
	
	
	if( !m_MuzzleFlashFX.IsValid() )
	{
		return;
	}

	// skip everything if we are using the silencer
	if ( m_bHaveSilencer )
	{
		return;
	}

	// get the object flags
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hObject, OFT_Flags, dwFlags );

	// if the object is not visible hide it
	if ( ! ( dwFlags & FLAG_VISIBLE ) )
	{
		m_MuzzleFlashFX.GetInstance()->Hide();
		return;
	}

	LTFLOAT fCurTime = g_pLTClient->GetTime();
	LTFLOAT fFlashDuration = m_MuzzleFlashFX.GetInstance()->m_fDuration;


	if ( ( fCurTime >= ( m_fFlashStartTime + fFlashDuration ) ) ||
	     ( g_pPlayerMgr->GetPlayerState() != PS_ALIVE ) ||
	     ( IsLiquid( g_pPlayerMgr->GetCurContainerCode()) && !m_pWeapon->bUseUWMuzzleFX  ) )
	{
		m_MuzzleFlashFX.GetInstance()->Hide();
	}
	else
	{
		// Align the flash object to the direction the model is facing...
		m_MuzzleFlashFX.GetInstance()->Show();
		m_MuzzleFlashFX.GetInstance()->SetPos( m_vFlashPos, m_vFlashOffset );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::CreatePVAttachClientFX
//
//	PURPOSE:	Create all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CClientWeapon::CreatePVAttachClientFX()
{
	ASSERT( 0 != g_pClientFXMgr );
	ASSERT( 0 != m_pWeapon );

	RemovePVAttachClientFX();

	for ( int i = 0; i < WM_MAX_PV_ATTACH_CLIENTFX; ++i )
	{
		if ( '\0' != m_pWeapon->szPVAttachClientFX[ i ][ 0 ] )
		{
			CLIENTFX_CREATESTRUCT fxInit( m_pWeapon->szPVAttachClientFX[ i ], FXFLAG_LOOP | FXFLAG_REALLYCLOSE, m_hObject ); 
			g_pClientFXMgr->CreateClientFX( &m_PVAttachClientFX[ i ], fxInit, false );
			if ( m_PVAttachClientFX[ i ].IsValid() ) 
			{
				// start out hidden
				m_PVAttachClientFX[ i ].GetInstance()->Hide();
			}
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::ShowPVAttachClientFX
//
//	PURPOSE:	Show all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ShowPVAttachClientFX()
{
	ASSERT( 0 != g_pClientFXMgr );
	ASSERT( 0 != m_pWeapon );

	for ( int i = 0; i < WM_MAX_PV_ATTACH_CLIENTFX; ++i )
	{
		// Only show those FX that were visible before we hid them...

		if ( m_PVAttachClientFX[ i ].IsValid() && !m_bPVAttachClientFXHidden[i] )
		{
			m_PVAttachClientFX[ i ].GetInstance()->Show();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HidePVAttachClientFX
//
//	PURPOSE:	Hide all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CClientWeapon::HidePVAttachClientFX()
{
	ASSERT( 0 != g_pClientFXMgr );
	ASSERT( 0 != m_pWeapon );

	for ( int i = 0; i < WM_MAX_PV_ATTACH_CLIENTFX; ++i )
	{
		if ( m_PVAttachClientFX[ i ].IsValid() )
		{
			m_bPVAttachClientFXHidden[i] = !(m_PVAttachClientFX[i].GetInstance()->m_bShow);
			m_PVAttachClientFX[ i ].GetInstance()->Hide();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::RemovePVAttachClientFX
//
//	PURPOSE:	Destroys all the player view attach client fx
//
// ----------------------------------------------------------------------- //

void CClientWeapon::RemovePVAttachClientFX()
{
	ASSERT( 0 != g_pClientFXMgr );

	int i;

	for ( i = 0; i < WM_MAX_PV_ATTACH_CLIENTFX; ++i )
	{
		if ( m_PVAttachClientFX[ i ].IsValid() )
		{
			// The FX will run itself out and get deleted in the mgr but we should still null our ptr

			g_pClientFXMgr->ShutdownClientFX( &m_PVAttachClientFX[ i ] );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HandleFireKeyDown()
//
//	PURPOSE:	Handle fire key down.
//				NOTE: Currently the primary use of this function is to
//				support ALT-FIRE animations.  Unfortunately with the 
//				client weapon reshuffling, they are broken.
//				Use at your own risk.
//
// ----------------------------------------------------------------------- //

void CClientWeapon::HandleFireKeyDownTransition()
{
	// Only handle alt-fire case on weapons that have special
	// Alt-fire animations...
	if (m_eLastFireType != FT_ALT_FIRE || !CanUseAltFireAnis()) return;

	// If we aren't playing the select, deselect, or fire ani, it is
	// okay to toggle using Alt-Fire Anis on/off...
	uint32 dwAni = g_pLTClient->GetModelAnimation(m_hObject);

	if (IsSelectAni(dwAni) || IsDeselectAni(dwAni) || IsFireAni(dwAni))
	{
		return;
	}


	// Toggle use of Alt-Fire Anis on/off...

	// Alright we need to either select or deselect the alt-fire
	// aspect of the weapon.  This is a bit tricky since the
	// select/deselect code depends on the current value of
	// m_bUsingAltFireAnis, and we want to change that value here.
	//
	// So, for the select case (i.e., m_bUsingAltFireAni == TRUE AFTER
	// it is toggled), we'll go ahead and toggle it first...).
	//
	// However, for the deselect case (i.e., m_bUsingAltFireAni
	// == TRUE BEFORE it is toggled), we'll toggle it after we
	// call Deslect...


	// See if we need to call Select...
	if (!m_bUsingAltFireAnis)
	{
		// Toggle so Select knows the right ani to play...
//		m_bUsingAltFireAnis = !m_bUsingAltFireAnis;
//		Select();
	}
	else
	{
		// Call deselect, then toggle...
//		Deselect();
//		m_bUsingAltFireAnis = !m_bUsingAltFireAnis;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HandleFireKeyUp()
//
//	PURPOSE:	Handle fire key up
//
// ----------------------------------------------------------------------- //

void CClientWeapon::HandleFireKeyUpTransition()
{
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::Fire
//
//	PURPOSE:	Fire the weapon
//
// ----------------------------------------------------------------------- //

void CClientWeapon::Fire( bool bFire )
{
	ASSERT( 0 != m_hObject );
	ASSERT( 0 != m_pAmmo );

	// Special case, check for gadget modes that don't actually support firing...
	if ( DT_GADGET_INFRA_RED == m_pAmmo->eInstDamageType )
	{
		return;
	}

	// perturb based on player's movement/firing state
	LTFLOAT fPerturb = GetDynamicPerturb();

		// get the player's aim modifier
		LTFLOAT fPerturbX = g_pPlayerStats->GetSkillModifier(SKL_AIM,AimModifiers::eAccuracy);

		// factor the modifier into the perturb
		fPerturb *= fPerturbX;

	// fire position/direction information
	LTVector vU, vR, vF, vFirePos;

	// Get the fire pos/rot
	if ( !GetFireVectors( &vU, &vR, &vF, &vFirePos ) )
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
	WeaponPath wp;
	wp.nWeaponId  = m_nWeaponId;
	wp.vU         = vU;
	wp.vR         = vR;
	wp.fPerturbR  = fPerturb;
	wp.fPerturbU  = wp.fPerturbR;

	LTVector vObjectImpactPos;
	HOBJECT hObjectImpact = INVALID_HOBJECT;

	for (int i=0; i < m_pWeapon->nVectorsPerRound; i++)
	{
		wp.vPath = vF;

		g_pWeaponMgr->CalculateWeaponPath( wp );

		// Do client-side firing...
		ClientFire( wp.vPath, vFirePos, &vObjectImpactPos, &hObjectImpact );
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
	::PlayWeaponSound( m_pWeapon, vPos, eSoundId, LTTRUE );

	// send a fire message to the server
	SendFireMessage( bFire, fPerturb, vFirePos, vF, vObjectImpactPos, hObjectImpact );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SendFireMessage()
//
//	PURPOSE:	Send fire message to server
//
// ----------------------------------------------------------------------- //

void CClientWeapon::SendFireMessage( bool bFire,
                                     LTFLOAT fPerturb, 
                                     LTVector const &vFirePos,
                                     LTVector const &vDir,
									 LTVector const &vImpactPos,
                                     HOBJECT hObject )
{
	// sanity check
	ASSERT( 0 != g_pLTClient );
	ASSERT( 0 != m_hObject );
	ASSERT( 0 != m_pAmmo );

	// Send Fire message to server...

	// Calculate a random seed...(srand uses this value so it can't be 1, since
	// that has a special meaning for srand)
	uint8 nRandomSeed = GetRandom( 2, 255 );

	CAutoMessage cMsg;
	LTRESULT msgResult;

	cMsg.Writeuint8( MID_WEAPON_FIRE );

	// write the projectile type
	// NOTE: currently the server considers VECTOR and PROJECTILE the same
	cMsg.Writeuint8( GetFireMessageType() );

	// ID of the weapon that is firing
	cMsg.Writeuint8( m_nWeaponId );

	// ID of the ammo that is firing
	cMsg.Writeuint8( m_nAmmoId );

	// muzzle flash position
	cMsg.WriteLTVector( m_vFlashPos );

	// weapon fire position (point where the bullets come from)
	cMsg.WriteLTVector( vFirePos );

	// vector pointing in the direction of travel
	cMsg.WriteLTVector( vDir );

	// random seed
	cMsg.Writeuint8( nRandomSeed );

	/*
	// NOTE: alt-fire is broken, don't send this
	// determine if we are using the alt fire animation (TRUE == we ARE using the alt fire)
	msgResult = cMsg.WriteByte( static_cast< bool >( FT_ALT_FIRE == m_eLastFireType ) );
	ASSERT( LT_OK == msgResult );
	*/

	// perturb (random range from true center that bullet can travel)
	cMsg.Writeuint8( static_cast< uint8 >( fPerturb * 255.0f ) );

	// time the weapon fired, in microseconds
	cMsg.Writeint32( static_cast< int >( g_pLTClient->GetTime() * 1000.0f ) );

	// add any extra info needed for the fire message
	// (this function primarily for derived classes)
	AddExtraFireMessageInfo( bFire, cMsg );

	// Object that was hit (optional)
	bool bWriteObject = ( hObject != INVALID_HOBJECT );
	cMsg.Writebool( bWriteObject );
	if ( bWriteObject )
	{
		float fTimeOfImpact = vDir.Dot(vImpactPos - vFirePos);
		cMsg.Writefloat( fTimeOfImpact );
		cMsg.WriteObject( hObject );
	}

	// send the message
	msgResult = g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	ASSERT( LT_OK == msgResult );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::AddExtraFireMessageInfo()
//
//	PURPOSE:	For the derived weapons, add any additional info needed
//
// ----------------------------------------------------------------------- //

void CClientWeapon::AddExtraFireMessageInfo( bool bFire, ILTMessage_Write *pMessage )
{
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
	else if ( m_eLastFireType == FT_ALT_FIRE )
	{
		nFireType = PSI_ALT_FIRE;
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

bool CClientWeapon::GetFireVectors( LTVector *vU, LTVector *vR, LTVector *vF,
                                 LTVector *vFirePos ) const
{
	// Get the fire position / direction from the camera (so it lines
	// up correctly with the crosshairs)...
	LTRotation rRot;
	if ( g_pPlayerMgr->IsFirstPerson() &&
		( !g_pPlayerMgr->IsUsingExternalCamera() ) )
	{
		// we're in 1st person and not using an external camera,
		// the shot is coming from the middle of the camera

		// get the camera
		HOBJECT hCamera = g_pPlayerMgr->GetCamera();
		if ( !hCamera )
		{
			return false;
		}

		// get the camera's position, its the fire position
		g_pLTClient->GetObjectPos( hCamera, vFirePos );

		// get the axis orientation of the camera
		g_pLTClient->GetObjectRotation( hCamera, &rRot );
		*vU = rRot.Up();
		*vR = rRot.Right();
		*vF = rRot.Forward();
	}
	else
	{
		// external camera, the shot is coming from the model
		GetModelPos( vFirePos );
		GetModelRot( &rRot );
		*vU       = rRot.Up();
		*vR       = rRot.Right();
		*vF       = rRot.Forward();
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

WeaponState CClientWeapon::SetState( WeaponState eNewState )
{
	WeaponState eOldState = m_eState;

	m_eState = eNewState;

	if ( GetState() == W_IDLE )
	{
		// Earliest we can play a non-subtle idle ani...

		m_fNextIdleTime	= GetNextIdleTime();
	}

	return eOldState;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::ClientFire
//
//	PURPOSE:	Do client-side weapon firing
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ClientFire( LTVector const &vPath, LTVector const &vFirePos, LTVector *pObjectImpactPos, HOBJECT *pObjectImpact )
{
	// Always process gadget firing...
	if ( m_pAmmo->eType == GADGET )
	{
		DoGadget( vPath, vFirePos );
		return;
	}

	// Only process the rest of these if we're connected to a remote server.  We
	// need to do this to hide lag.
	if ( !g_pClientMultiplayerMgr->IsConnectedToRemoteServer( ))
	{
		return;
	}

	switch ( m_pAmmo->eType )
	{
		case PROJECTILE:
		{
			DoProjectile( vPath, vFirePos );
		}
		break;

		case VECTOR:
		{
			DoVector( vPath, vFirePos, pObjectImpactPos, pObjectImpact );
		}
		break;

		default:
		{
			g_pLTClient->CPrint( "ERROR in CClientWeapon::ClientFire().  Invalid Ammo Type!" );
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::DoProjectile
//
//	PURPOSE:	Do client-side projectile
//
// ----------------------------------------------------------------------- //

void CClientWeapon::DoProjectile( LTVector const &vPath, LTVector const &vFirePos )
{
	// projectiles are serverside
}

static bool ClientWeapon_PolyFilterFn(HPOLY hPoly, void *pUserData)
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
bool CheckVectorNodeIntersect(ModelSkeleton eModelSkeleton, HOBJECT hObject, const LTVector &vStartPos, const LTVector &vEndPos)
{
	// If they don't have a valid skeleton, consider it a hit
	if (eModelSkeleton == eModelSkeletonInvalid)
		return true;

	// Pre-calculate...
	LTVector vDir = (vEndPos - vStartPos);
	float fSegLength = vDir.Mag();
	if (fSegLength == 0.0f)
		return false;
	vDir /= fSegLength;

	// Run through the nodes looking for an intersection
	int cNodes = g_pModelButeMgr->GetSkeletonNumNodes(eModelSkeleton);
	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelNode eCurrentNode = (ModelNode)iNode;

		// Get the node radius
		LTFLOAT fNodeRadius = g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eCurrentNode);

		// Don't do transforms if we don't need to
		if (fNodeRadius <= 0.0f)
		{
			continue;
		}

		// Which node are you again?
		const char* szNodeName = g_pModelButeMgr->GetSkeletonNodeName(eModelSkeleton, eCurrentNode);
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
        LTransform transform;
        ltResult = g_pModelLT->GetNodeTransform(hObject, hNode, transform, LTTRUE);
		if ( ltResult != LT_OK )
		{
			continue;
		}

		// Distance along ray to point of closest approach to node point

        const LTVector vRelativeNodePos = transform.m_Pos - vStartPos;
		const float fRayDist = vDir.Dot(vRelativeNodePos);

		// Make sure the projection onto the ray is within range
		if ((fRayDist < -fNodeRadius) || (fRayDist > (fNodeRadius + fSegLength)))
			continue;
	
		// Ignore the node if it wasn't within the radius of the hit spot.
		const LTFLOAT fDistSqr = (vDir*fRayDist - vRelativeNodePos).MagSqr();
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

		// Is it attached to a body?
		CBodyFX *pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFromHitBox( hTest );
		if ( pBody )
		{
			return CheckVectorNodeIntersect(pBody->GetModelSkeleton(), pBody->GetServerObj(), pParams->m_vFirePos, pParams->m_vEndPos);
		}

		// It's not a player..  Go ahead and hit it.
		return true;
	}

	// Don't hit the characters, they've got hit-boxes..
	CCharacterFX *pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(hTest);
	if ( pCharacter )
		return false;

	// And the same with the bodies...
	CBodyFX *pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFX( hTest );
	if ( pBody )
		return false;

	// Ok we should be able to hit this...
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::DoVector
//
//	PURPOSE:	Do client-side vector
//
// ----------------------------------------------------------------------- //

void CClientWeapon::DoVector( LTVector const &vPath, LTVector const &vFirePos, LTVector *pObjectImpactPos, HOBJECT *pObjectImpact )
{
	ASSERT( 0 != m_hObject );
	ASSERT( 0 != m_pWeapon );

	LTVector vEndPos;

	IntersectInfo iInfo;
	IntersectQuery qInfo;
	qInfo.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;

	// compute the vector end points
	LTVector vTemp;
	VEC_MULSCALAR( vTemp, vPath, m_pWeapon->nRange );
	VEC_ADD( vEndPos, vFirePos, vTemp );

	// filter information
	CW_VOFF_Params cParams(this, vFirePos, vEndPos);
	qInfo.m_FilterFn  = ClientWeapon_VectorObjFilterFn;
	qInfo.m_pUserData = &cParams;
	qInfo.m_PolyFilterFn = ClientWeapon_PolyFilterFn;

	// vector end points
	qInfo.m_From = vFirePos;
	qInfo.m_To = vEndPos;

	// try to hit something
	if ( g_pLTClient->IntersectSegment( &qInfo, &iInfo ) )
	{
		// hit something, handle the impact
		HandleVectorImpact( vPath, &qInfo, &iInfo, pObjectImpactPos, pObjectImpact );
	}
	else
	{
		// hit nothing, pretend we hit the sky
		LTVector vUp;
		vUp.Init( 0.0f, 1.0f, 0.0f );
		AddImpact( LTNULL, vEndPos, vUp, vPath, ST_SKY );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HandleVectorImpact
//
//	PURPOSE:	Handle a vector hitting something
//
// ----------------------------------------------------------------------- //

void CClientWeapon::HandleVectorImpact( LTVector const &vPath, IntersectQuery *qInfo, IntersectInfo *iInfo, LTVector *pObjectImpactPos, HOBJECT *pObjectImpact )
{
	HOBJECT hImpactObj = iInfo->m_hObject;

	// Fill in the object impact information if we hit an object
	if ( pObjectImpact && pObjectImpactPos && ( iInfo->m_hPoly == INVALID_HPOLY ) && ( hImpactObj != INVALID_HOBJECT ) )
	{
		bool bUseNewImpact = true;

		// Don't use the new impact if the object's not a hitbox
		uint32 nObjUserFlags;
		g_pCommonLT->GetObjectFlags( iInfo->m_hObject, OFT_User, nObjUserFlags );
		bUseNewImpact &= ((nObjUserFlags & USRFLG_HITBOX) != 0);

		CCharacterFX *pCharacter = g_pGameClientShell->GetSFXMgr()->GetCharacterFromHitBox( hImpactObj );
		if ( pCharacter )
		{
			hImpactObj = pCharacter->GetServerObj();
		}
		else
		{
			CBodyFX *pBody = g_pGameClientShell->GetSFXMgr()->GetBodyFromHitBox( hImpactObj );
			if ( pBody )
			{
				hImpactObj = pBody->GetServerObj();
			}
		}

		// Only use the new object if it's closer than the original
		if ( bUseNewImpact && ( *pObjectImpact != INVALID_HOBJECT ) )
		{
			LTVector vObjectPos;
			g_pLTBase->GetObjectPos( hImpactObj, &vObjectPos );
			float fOldDistSqr;
			if ( *pObjectImpact != hImpactObj )
			{
				LTVector vOldObjectPos;
				g_pLTBase->GetObjectPos( *pObjectImpact, &vOldObjectPos );
				fOldDistSqr = pObjectImpactPos->DistSqr( vOldObjectPos );
			}
			else
				fOldDistSqr = pObjectImpactPos->DistSqr( vObjectPos );
			float fNewDistSqr = iInfo->m_Point.DistSqr( vObjectPos );
			bUseNewImpact = ( fOldDistSqr < fNewDistSqr );
		}

		if ( bUseNewImpact )
		{
			*pObjectImpact = hImpactObj;
			*pObjectImpactPos = iInfo->m_Point;
		}
	}

	// Get the surface type (check the poly first)...
	SurfaceType eType = GetSurfaceType( iInfo->m_hPoly );
	if ( eType == ST_UNKNOWN )
	{
		eType = GetSurfaceType( hImpactObj );
	}

	AddImpact( hImpactObj, iInfo->m_Point,
	           iInfo->m_Plane.m_Normal, vPath,
	           eType );

	// If we hit liquid, cast another ray that will go through the water...
	if ( ST_LIQUID == eType )
	{
		qInfo->m_FilterFn = AttackerLiquidFilterFn;

		if ( g_pLTClient->IntersectSegment( qInfo, iInfo ) )
		{
			// Get the surface type (check the poly first)...
			SurfaceType eType = GetSurfaceType( iInfo->m_hPoly );
			if ( ST_UNKNOWN == eType )
			{
				eType = GetSurfaceType( iInfo->m_hObject );
			}

			AddImpact( iInfo->m_hObject, iInfo->m_Point,
			           iInfo->m_Plane.m_Normal, vPath,
			           eType );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::DoGadget
//
//	PURPOSE:	Do client-side gadget
//
// ----------------------------------------------------------------------- //

void CClientWeapon::DoGadget( LTVector const &vPath, LTVector const &vFirePos )
{
	// Do Camera shutter fx...
	if ( DT_GADGET_CAMERA == m_pAmmo->eInstDamageType )
	{
		if (!g_pInterfaceMgr->FadingScreen())
		{
			g_pInterfaceMgr->StartScreenFadeIn( ns_vtCameraShutterSpeed.GetFloat() );
		}
	}

	DoVector( vPath, vFirePos, LTNULL, LTNULL );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::HandleGadgetImpact
//
//	PURPOSE:	Handle a gadget vector hitting an object
//
// ----------------------------------------------------------------------- //

void CClientWeapon::HandleGadgetImpact( HOBJECT hObj, LTVector vImpactPoint )
{
	// If the gadget can activate this type of object, Tell the server
	// that the gadget was activated on this object...
	LTVector vU, vR, vF, vFirePos;
	if ( !GetFireVectors( &vU, &vR, &vF, &vFirePos ) )
	{
		return;
	}

	uint32 dwUserFlags = 0;
	if ( hObj )
	{
		g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUserFlags);
	}

	DamageType eType = m_pAmmo->eInstDamageType;

	bool bTestDamageType = true;

	// Make sure the object isn't a character object (gadget and character
	// user flags overlap) unless this is a character specific gadget...
	if ( dwUserFlags & USRFLG_CHARACTER )
	{
		//no currently supported "character specific gadgets"
		return;
	}

	// Check the team identifier on the gadget to see if it matches our team...

	CGadgetTargetFX *pGadget = dynamic_cast<CGadgetTargetFX*>(g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_GADGETTARGET_ID, hObj ));
	if( !pGadget )
		return;

	if( IsTeamGameType() && pGadget->GetTeamID() != INVALID_TEAM )
	{
		CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
		if( !pLocalCI )
			return;

		if( pLocalCI->nTeamID != pGadget->GetTeamID() )
			return;
	}

	// Test the damage type if necessary...
	if ( bTestDamageType )
	{
		if ( DT_GADGET_CODE_DECIPHERER == eType )
		{
			// Make sure the object can be deciphered...
			if ( !( dwUserFlags & USRFLG_GADGET_CODE_DECIPHERER ) )
			{
				return;
			}
		}
		else if ( DT_GADGET_LOCK_PICK == eType )
		{
			// Make sure the object is "pickable"...
			if ( !(dwUserFlags & USRFLG_GADGET_LOCK_PICK ) )
			{
				return;
			}
		}
		else if ( DT_GADGET_WELDER == eType )
		{
			// Make sure the object is "weldable"...
			if ( !( dwUserFlags & USRFLG_GADGET_WELDER ) )
			{
				return;
			}
		}
		else if ( DT_GADGET_CAMERA == eType )
		{
			// Make sure the object is something we can photograph...
			if ( dwUserFlags & USRFLG_GADGET_CAMERA /*|USRFLG_GADGET_INTELLIGENCE*/ )
			{
				// Make sure we're in camera range...
				if ( !g_pPlayerMgr->InCameraGadgetRange( hObj ) )
				{
					return;
				}
			}
			else
			{
				return;
			}
		}
		else if( DT_GADGET_TIME_BOMB == eType )
		{
			// Make sure a bomb can be placed on the object...
			if( !(dwUserFlags & USRFLG_GADGET_BOMBABLE ) )
			{
				return;
			}
		}
		else if( DT_GADGET_INK_REAGENT == eType )
		{
			// Make sure the object is invisible ink...
			if( !( dwUserFlags & USRFLG_GADGET_INVISIBLE_INK ) )
			{
				return;
			}
		}
		else if( DT_GADGET_EAVESDROPBUG == eType )
		{
			// Make sure the object is able to recieve a bug...
			if( !( dwUserFlags & USRFLG_GADGET_EAVESDROPBUG ) )
			{
				return;
			}
		}
		else
		{
			return;
		}
	}

	// send the fire message to the server
	SendFireMessage( false, GetDynamicPerturb(), vFirePos, vF, vImpactPoint, hObj );

	// Do any special processing...
	DoSpecialFire();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::DoSpecialFire()
//
//	PURPOSE:	Do special case fire processing
//
// ----------------------------------------------------------------------- //

void CClientWeapon::DoSpecialFire()
{
	// Currently we need to check for gadget special cases...
	if ( GADGET == m_pAmmo->eType )
	{
		// Decrement ammo count here...
		DecrementAmmo();
	}
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

	if ( m_bAutoSwitchEnabled )
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
	ASSERT( 0 != m_pAmmo );

	// If we're out of ammo, keep hidden...
	if ( bShow && !bForce )
	{
		if ( g_pPlayerStats && ( g_pPlayerStats->GetAmmoCount(m_nAmmoId) < 1 ) )
		{
			bShow = false; // Hide the necessary pieces...
		}
	}

	ASSERT( 0 != g_pLTClient );
	ILTModel *pModelLT = g_pLTClient->GetModelLT();
	HMODELPIECE hPiece = 0;

	ASSERT( 0 != m_pWeapon );
	for( int i = 0; i < m_pWeapon->blrHiddenPieceNames.GetNumItems(); ++i )
	{
		// Do a const cast the ButeListReader's return value to work with the non-const engine.
		if( LT_OK == pModelLT->GetPiece( m_hObject, const_cast< char * >( m_pWeapon->blrHiddenPieceNames.GetItem(i) ), hPiece ) )
		{
			pModelLT->SetPieceHideStatus( m_hObject, hPiece, !bShow );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::SpecialCaseOverrideFire
//
//	PURPOSE:	Special cases weapons or gadgets that can only fire at certain objects
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::SpecialOverrideFire()
{
	if( !m_hObject )
	{
		return false;
	}

	// Currently  only the TimeBomb has specifc area it can be placed...
	if( DT_GADGET_TIME_BOMB == m_pAmmo->eInstDamageType )
	{
		IntersectInfo	iInfo;
		IntersectQuery	qInfo;
		qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

		LTVector vFirePos, vU, vR, vF;

		if( !GetFireVectors( &vU, &vR, &vF, &vFirePos ) )
		{
			return false;
		}

		HOBJECT hFilterList[] =
		{
			g_pLTClient->GetClientObject(),
			g_pPlayerMgr->GetMoveMgr()->GetObject(),
			LTNULL
		};

		qInfo.m_FilterFn  = ObjListFilterFn;
		qInfo.m_pUserData = hFilterList;

		qInfo.m_From = vFirePos;
		qInfo.m_To = qInfo.m_From + (vF * (LTFLOAT)m_pWeapon->nRange);

		if( g_pLTClient->IntersectSegment( &qInfo, &iInfo ) )
		{
			if( iInfo.m_hObject )
			{
				uint32 dwUsrFlgs;
				g_pCommonLT->GetObjectFlags( iInfo.m_hObject, OFT_User, dwUsrFlgs );

				// Check to see if the object we hit can have a bomb placed on it...
				if( dwUsrFlgs & USRFLG_GADGET_BOMBABLE )
				{
					// We can only place the bomb here if it belongs to our team...

					if( IsTeamGameType() )
					{
						// Check the team identifier on the gadget to see if it matches our team...

						CGadgetTargetFX *pGadget = dynamic_cast<CGadgetTargetFX*>(g_pGameClientShell->GetSFXMgr()->FindSpecialFX( SFX_GADGETTARGET_ID, iInfo.m_hObject ));
						if( !pGadget )
							return false;

						CLIENT_INFO *pLocalCI = g_pInterfaceMgr->GetClientInfoMgr()->GetLocalClient();
						if( !pLocalCI )
							return false;

						if( pGadget->GetTeamID() != INVALID_TEAM )
						{
							if( pLocalCI->nTeamID == pGadget->GetTeamID() )
								return true;
						}
					}
					else
					{
						return true;
					}
				}
			}
		}
	
		return false;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CClientWeapon::AddImpact
//
//	PURPOSE:	Add the weapon impact
//
// ----------------------------------------------------------------------- //

void CClientWeapon::AddImpact( HLOCALOBJ hObj, LTVector const &vImpactPoint,
                               LTVector const &vNormal, LTVector const &vPath,
                               SurfaceType eType )
{
	// Handle gadget special case...
	if ( GADGET == m_pAmmo->eType )
	{
		// No impact fx for gadgets...
		HandleGadgetImpact( hObj, vImpactPoint );
		return;
	}

	// See if we should do tracers or not...
	if ( m_pAmmo->pTracerFX )
	{
		// only 1 tracer for every few shots
		m_nTracerNumber = m_nTracerNumber + 1;
		m_nTracerNumber %= m_pAmmo->pTracerFX->nFrequency;
		if ( 0 != m_nTracerNumber )
		{
			m_wIgnoreFX |= WFX_TRACER;
		}
	}
	else
	{
		m_wIgnoreFX |= WFX_TRACER;
	}

	::AddLocalImpactFX( hObj, m_vFlashPos, vImpactPoint, vNormal, eType,
	                    vPath, m_nWeaponId, m_nAmmoId, m_wIgnoreFX );

	// If we do multiple calls to AddLocalImpact, make sure we only do some
	// effects once :)
	m_wIgnoreFX |= WFX_SILENCED | WFX_SHELL | WFX_LIGHT | WFX_MUZZLE | WFX_TRACER;
}


// ----------------------------------------------------------------------- //
//
//	FUNCTION:	CClientWeapon::PopulateCreateStruct()
//
//	PURPOSE:	Add all the basic information to the object create structure
//
// ----------------------------------------------------------------------- //

void CClientWeapon::PopulateCreateStruct( ObjectCreateStruct *pOCS ) const
{
	// sanity check parameters
	ASSERT( 0 != pOCS );

	// sanity check globals
	ASSERT( 0 != g_pModelButeMgr );
	ASSERT( 0 != g_pCommonLT );
	ASSERT( 0 != g_pLTClient );
	ASSERT( 0 != g_pPlayerMgr );

	// sanity check this object
	ASSERT( 0 != m_pWeapon );

	// get the model filename
	SAFE_STRCPY( pOCS->m_Filename, m_pWeapon->szPVModel );

	// add the skins
	m_pWeapon->blrPVSkins.CopyList( 0, pOCS->m_SkinNames[0], ( MAX_CS_FILENAME_LEN + 1 ) );

	// add the render styles
	m_pWeapon->blrPVRenderStyles.CopyList( 0, pOCS->m_RenderStyleNames[ 0 ], ( MAX_CS_FILENAME_LEN + 1 ) );

	// Figure out what hand skin to use...
	ASSERT( g_pPlayerMgr->GetMoveMgr() );
	CCharacterFX* pCharFX = g_pPlayerMgr->GetMoveMgr()->GetCharacterFX();

	if ( ( m_pWeapon->blrPVSkins.GetNumItems() ) &&
	     ( 0 == strcmp( m_pWeapon->blrPVSkins.GetItem( 0 ), "Hands" ) ) )
	{
		// This looks for the special case where "Hands" is specified as the first
		// player view skin.  If it is, then we take the skin from the model bute
		// instead of the weapon bute.
		ModelId nModelID = ( pCharFX ) ? pCharFX->GetModelId() : static_cast< ModelId >( 0 );
		SAFE_STRCPY( pOCS->m_SkinNames[ 0 ], g_pModelButeMgr->GetHandsSkinFilename( nModelID ) );
	}

	// Create the weapon at it's actual position...

	LTVector vWeaponOffset = GetWeaponOffset();

	pOCS->m_Pos.x = vWeaponOffset.x + m_fBobWidth;
	pOCS->m_Pos.y = vWeaponOffset.y + m_fBobHeight;
	pOCS->m_Pos.z = vWeaponOffset.z;
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
		m_hLoopSound = LTNULL;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::RemoveFinishedKeyframedFX()
//
//  PURPOSE:	Remove all keyframed FXEd effects that have finished playing
//
// ----------------------------------------------------------------------- //

bool CClientWeapon::RemoveFinishedKeyframedFX()
{
	bool bAtLeastOneFXRemoved = false;

	CLIENTFX_LINK_NODE*	pPrev = &m_KeyframedClientFX;

	for(CLIENTFX_LINK_NODE* pNode = m_KeyframedClientFX.m_pNext; pNode != NULL; )
	{
		CLIENTFX_LINK_NODE* pNext = pNode->m_pNext;

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
	if( !m_pWeapon )
		return;

	// Remove all mods and attachments and get rid of the weapon model...

	Deactivate();

	ResetData();
	m_nAmmoInClip = 0;
	m_bFirstSelection = true;
	m_nAmmoId = WMGR_INVALID_ID;
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

		KillLoopSound();

		// Send message to server so all clients can stop the sound...
		// An id of invalid means stop

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_WEAPON_SOUND_LOOP );
		cMsg.Writeuint8( PSI_INVALID );
		cMsg.Writeuint8( m_nWeaponId );
		g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CClientWeapon::ClearFiring()
//
//  PURPOSE:	Stop the weapon from firing and force it into an idle animation...
//
// ----------------------------------------------------------------------- //

void CClientWeapon::ClearFiring()
{
	// Clear any current fire flag we previously set...

	m_bFire = false;

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
