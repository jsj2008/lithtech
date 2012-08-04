// ----------------------------------------------------------------------- //
//
// MODULE  : Weapon.cpp
//
// PURPOSE : Weapon class - implementation
//
// CREATED : 9/25/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Weapon.h"
#include "ServerUtilities.h"
#include "iltserver.h"
#include "ltengineobjects.h"
#include "MsgIDs.h"
#include "HHWeaponModel.h"
#include "Character.h"
#include "WeaponFXTypes.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "Arsenal.h"
#include "AIStimulusMgr.h"
#include "WeaponFireInfo.h"
#include "AIUtils.h"
#include "Projectile.h"
#include "FXDB.h"
#include "SkillDefs.h"
#include "AIUtils.h"
#include "WeaponPath.h"
#include "ServerSoundMgr.h"
#include "FxDefs.h"
#include "ServerDB.h"

extern CGameServerShell* g_pGameServerShell;
extern CAIStimulusMgr* g_pAIStimulusMgr;

// Variable to enable AIs to have an infinite amount of ammo for debug purposes.
VarTrack g_AIInfiniteAmmo;

#define INFINITE_AMMO_AMOUNT 1000

uint16 g_wIgnoreFX = 0;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	InfiniteAmmo
//
//	PURPOSE:	Simple utility function to determine if the passed in 
//				object has infinite ammo for the passed in weapon.
//
// ----------------------------------------------------------------------- //


static bool InfiniteAmmo( HOBJECT hObj, HWEAPONDATA hWpnData )
{
	if ( !g_AIInfiniteAmmo.IsInitted( ) )
	{
		g_AIInfiniteAmmo.Init( g_pLTServer, "AIInfiniteAmmo", NULL, 0.f );
	}

	if ( IsAI( hObj ) && g_AIInfiniteAmmo.GetFloat() != 0.0f )
	{
		return true;
	}

	if ( IsPlayer( hObj ) && g_pGameServerShell->IsInfiniteAmmo() )
	{
		return true;
	}

	return g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CWeapon
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CWeapon::CWeapon()
:	m_eState			( W_IDLE ),
	m_bHave				( false ),
	m_nAmmoInClip		( 0 ),
	m_fDamageFactor		( 1.0f ),
	m_fRangeAdjust		( 1.0f ),
	m_fLifeTime			( -1.0f ),
	m_pArsenal			( NULL ),
	m_pActiveWeapon		( NULL ),
	m_nCurTracer		( 0 ),
	m_nLastTimestamp	( 0 ),
	m_bFirstSelection	( true ),
	m_nHandAni			( INVALID_ANI ),
	m_nPreFireAni		( INVALID_ANI ),
	m_nFireAni			( INVALID_ANI ),
	m_nPostFireAni		( INVALID_ANI ),
	m_nReloadAni		( INVALID_ANI ),
	m_nPlayerAni		( INVALID_ANI ),
	m_bHidden			( false ),
	m_hWeapon			( NULL ),
	m_hAmmo				( NULL ),
	m_nHealth			( 0 ),
	m_nMaxHealth		( 0 ),
	m_nWarnHealth		( 0 )

{
	Drop();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::~CWeapon
//
//	PURPOSE:	Deallocate memory
//
// ----------------------------------------------------------------------- //

CWeapon::~CWeapon()
{
	KillLoopSound();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Init
//
//	PURPOSE:	Init object
//
// ----------------------------------------------------------------------- //

bool CWeapon::Init( CArsenal *pArsenal, HOBJECT hObj, HWEAPON hWeapon, HAMMO hAmmo )
{
	if( !pArsenal || !hObj )
		return false;

	KillLoopSound();

	m_pArsenal	= pArsenal;
	m_hObject	= hObj;
	m_hWeapon	= hWeapon;
	m_hAmmo		= hAmmo;

	// Use our parent enginetimer.
	m_LastFireTime.SetEngineTimer( ObjectContextTimer( m_hObject ));

	// breakable weapons
	if (hWeapon)
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, IsAI(m_hObject));
		m_nMaxHealth = g_pWeaponDB->GetInt32(hWpnData, WDB_WEAPON_nMaxHealth);
		m_nWarnHealth = g_pWeaponDB->GetInt32(hWpnData, WDB_WEAPON_nWarnHealth);
		m_nHealth = m_nMaxHealth;

		DisplayWeaponModelPieces(m_hModelObject, m_hWeapon, WDB_WEAPON_sShowPieces, true,  IsAI(m_hObject));
		DisplayWeaponModelPieces(m_hModelObject, m_hWeapon, WDB_WEAPON_sHidePieces, false, IsAI(m_hObject));
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Drop
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CWeapon::Drop()
{
	m_bHave = false;
	if( m_hWeapon )
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, IsAI(m_hObject));
		m_hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
	}
	else
	{
		m_hAmmo = NULL;
	}

	for( int i = 0; i < W_MAX_MODS; ++i )
	{
		m_Mods[i] = NULL;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::ReloadClip
//
//	PURPOSE:	Fill the clip
//
// ----------------------------------------------------------------------- //

void CWeapon::ReloadClip( bool bPlayReload, int32 nNewAmmo /*=-1*/, HAMMO hAmmo /*=NULL*/ )
{
	if( !m_pArsenal )
		return;

	// See if we want to reload with a different ammo type...
	
	if( (hAmmo != NULL) && (hAmmo != m_hAmmo) )
	{
		SetAmmo( hAmmo );
		m_nAmmoInClip = 0;
	}

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, IsAI(m_hObject));
	int32 nShotsPerClip = g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nShotsPerClip );

	// Determine how much ammo the the arsenal has.

	int32 nAmmo = 0;
	bool bInfiniteAmmo = InfiniteAmmo( m_hObject, hWpnData );
	if ( bInfiniteAmmo )
	{
		nAmmo = INFINITE_AMMO_AMOUNT;
	}
	else
	{
		nAmmo = nNewAmmo >= 0 ? nNewAmmo : m_pArsenal->GetAmmoCount( m_hAmmo );
	}

	// Clip is full...
	if (m_nAmmoInClip == nShotsPerClip && !bInfiniteAmmo )
	{
		return;
	}

	if (nAmmo > 0 && nShotsPerClip > 0)
	{
		m_nAmmoInClip = nAmmo < nShotsPerClip ? nAmmo : nShotsPerClip;
	}


	float	fRadiusMult = 1.0f;
	if (IsPlayer(m_hObject))
	{
		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hObject ));
        fRadiusMult = GetSkillValue(eStealthRadius);
	}
	
	// Add a sound stimulus for the reloading sound.
	if( IsCharacter( m_hObject ))
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hModelObject, &vPos);

		CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject(m_hObject);
		if ( pCharacter )
		{
			StimulusRecordCreateStruct scs( kStim_WeaponReloadSound, pCharacter->GetAlignment(), vPos, m_hObject );
			scs.m_hStimulusTarget = m_hModelObject;
			scs.m_flRadiusScalar = fRadiusMult;
			g_pAIStimulusMgr->RegisterStimulus( scs );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetWeaponData
//
//  PURPOSE:    Get the data record associated with the weapon
//
// ----------------------------------------------------------------------- //

HWEAPONDATA		CWeapon::GetWeaponData() const
{
	return g_pWeaponDB->GetWeaponData(m_hWeapon, IsAI(m_hObject));
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetAmmoDataRecord
//
//  PURPOSE:    Get the data record associated with the ammo
//
// ----------------------------------------------------------------------- //

HAMMODATA	CWeapon::GetAmmoDataRecord()	const	
{ 
	return g_pWeaponDB->GetAmmoData(m_hAmmo,IsAI(m_hObject)); 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::UpdateWeapon
//
//  PURPOSE:    Update the weapon (fire if bFire == true)
//
// ----------------------------------------------------------------------- //

WeaponState CWeapon::UpdateWeapon(WeaponFireInfo & fireInfo, bool bFire)
{
	WeaponState eRet = W_IDLE;

	if (bFire)
	{
		fireInfo.nSeed = GetRandom(2,255);
		fireInfo.nPerturbCount++;
		// Put in a timestamp since the AI's use this and don't send us a timestamp
		m_LastFireTime.Start();
		eRet = Fire(fireInfo);
	}

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Fire
//
//	PURPOSE:	Fire the weapon
//
// ----------------------------------------------------------------------- //

WeaponState CWeapon::Fire(WeaponFireInfo & info)
{
	if (!info.hFiredFrom || !g_pWeaponDB || !m_pArsenal) return W_IDLE;

	// Make sure we always have ammo if we should...
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, IsAI(m_hObject));

	bool bInfiniteAmmo = InfiniteAmmo( m_hObject, hWpnData );
	int32 nAmmo = bInfiniteAmmo ? INFINITE_AMMO_AMOUNT : m_pArsenal->GetAmmoCount( m_hAmmo );
	
	bool bInfiniteClip = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteClip );
	int32 nShotsPerClip = bInfiniteClip ? INFINITE_AMMO_AMOUNT : g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nShotsPerClip );

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo,IsAI(m_hObject));
	AmmoType eAmmoType = (AmmoType)g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType );


	// Restrict the rate of fire
	if( (info.nFireTimestamp - m_nLastTimestamp) < (uint32)g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nFireDelay ))
	{
		// Hey!  That was too fast!
		// Don't actually fire but decrement the ammo count.
		
		if( nAmmo > 0 )
		{
			if (m_nAmmoInClip > 0)
			{
				if (nShotsPerClip > 0 && !bInfiniteAmmo && !bInfiniteClip)
				{
					m_nAmmoInClip--;
				}

				if (!bInfiniteAmmo && !bInfiniteClip)
				{
					m_pArsenal->DecrementAmmo( m_hAmmo );
				}
			}

			// Check to see if we need to reload...

			if (nShotsPerClip > 0)
			{
				if (m_nAmmoInClip <= 0)
				{
					// Only automatically reload if we're the player... and don't play the animation.

					if ( IsPlayer(info.hFiredFrom) )
					{
						ReloadClip(false, nAmmo);
					}
				}
			}
		}

		return W_IDLE;
	}

	// Remember when this shot went off..
	m_nLastTimestamp = info.nFireTimestamp;

	WeaponState eRet = W_IDLE;

	if (nAmmo > 0)
	{
		// Ignore the exit surface, silenced, and alt-fire fx...

		g_wIgnoreFX = WFX_SILENCED | WFX_ALTFIRESND;

		// See if the weapon is silenced...

		if (GetSilencer())
		{
			g_wIgnoreFX &= ~WFX_SILENCED;	// Do silenced
			g_wIgnoreFX |= WFX_FIRESOUND;	// Don't do fire
		}
		else if (info.bAltFire)
		{
			g_wIgnoreFX &= ~WFX_ALTFIRESND;  // Do alt-fire
			g_wIgnoreFX |= WFX_FIRESOUND;	 // Don't do fire
		}

		// If the player fired this and it is the appropriate weapon type,
		// don't worry about playing the fire sound (the player already played it)...

		if (IsPlayer(info.hFiredFrom))
		{
			if (eAmmoType == PROJECTILE)
			{
				g_wIgnoreFX |= WFX_FIRESOUND | WFX_ALTFIRESND;
			}
		}

		// See if we should do tracers or not...

		HRECORD hTracerFX = g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sTracerFX );
		if( hTracerFX )
		{
			m_nCurTracer = ( m_nCurTracer + 1 ) % g_pFXDB->GetInt32(hTracerFX,FXDB_nFrequency);
			if (m_nCurTracer != 0)
			{
				g_wIgnoreFX |= WFX_TRACER;
			}
		}
		else
		{
			g_wIgnoreFX |= WFX_TRACER;
		}

		if (m_nAmmoInClip > 0)
		{
			LTRotation rRot;
			LTVector vU, vR, vF;

			g_pLTServer->GetObjectRotation(info.hFiredFrom, &rRot);
			vU = rRot.Up();
			vR = rRot.Right();
			vF = rRot.Forward();

			// Create a projectile for every vector...
			int32 nVectorsPerShot = g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nVectorsPerRound );

			CWeaponPath wp;
			LTVector vOriginalPath = info.vPath;

			wp.m_hWeapon		= GetWeaponRecord( );
			wp.m_hAmmo			= GetAmmoRecord( );
			wp.m_hFiredFrom		= info.hFiredFrom;
			wp.m_hFiringWeapon	= info.hFiringWeapon;
			wp.m_vFirePos		= info.vFirePos;
			wp.m_vFlashPos		= info.vFlashPos;
			wp.m_vPath			= info.vPath;
			wp.m_fInstDamage	= GetInstDamage( );
			wp.m_vU				= vU;
			wp.m_vR				= vR;
			wp.m_fPerturb		= info.fPerturb;
			wp.m_iRandomSeed	= info.nSeed;
			wp.m_iPerturbCount	= info.nPerturbCount;
			wp.m_nFireTimeStamp	= info.nFireTimestamp;

			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( wp.m_hWeapon, IsAI( wp.m_hFiredFrom ));
			HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData( wp.m_hAmmo, IsAI( wp.m_hFiredFrom ));
			int32 nWeaponRange	= g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nRange );
			int32 nAmmoRange	= g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nRange );
			wp.m_fRange			= (float)(nAmmoRange > 0 ? nAmmoRange : nWeaponRange);

			info.pWeaponPath = &wp;

			// Don't actually create an object, just use our
			// parent's projectile object to do vector calculations...
			CProjectile* pVectorProj = m_pArsenal->GetVecProjectile();
			if( pVectorProj )
				pVectorProj->ClearImpactPoints( );

			for (int i=0; i < nVectorsPerShot; i++)
			{

				// Shoot the weapon...
				

				if (eAmmoType == PROJECTILE)
				{
					info.vPath = wp.CalculatePath( vOriginalPath, IsAI(info.hFiredFrom) );

					if (!CreateProjectile(rRot, info))
					{
						return W_IDLE;
					}

					// If we are shooting multiple vectors ignore some special
					// fx after the first vector...

					g_wIgnoreFX |= WFX_FIRESOUND | WFX_ALTFIRESND | WFX_SHELL |
						WFX_SILENCED | WFX_MUZZLE | WFX_TRACER;
				}
				else if (eAmmoType == VECTOR)
				{
					//reset path for case of multiple vectors
					info.vPath = vOriginalPath;
					wp.m_vPath = info.vPath;
					if( pVectorProj )
					{
						pVectorProj->Setup(this, info);

					}
				}
			}

			if( eAmmoType == VECTOR && pVectorProj )
			{
				pVectorProj->SendVectorWeaponFireMsg( );
			}
			
			if (nShotsPerClip > 0 && !bInfiniteAmmo && !bInfiniteClip)
			{
				m_nAmmoInClip--;
			}

			if (!bInfiniteAmmo && !bInfiniteAmmo)
			{
				m_pArsenal->DecrementAmmo( m_hAmmo );
			}
		}
		else
		{
			// Play a dry fire sound if we are attempting to fire without any
			// ammo.  Generally, this is handled through model keys.  An AI may 
			// attempt to play an animation with 4 shots fired with only 3 rounds 
			// loaded.  This will handle playing a dryfire sound for the 4th shot

			HRECORD hSR = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rDryFireSnd );
			if ( hSR )
			{
				g_pServerSoundMgr->PlayDBSoundFromPos( info.vFirePos, hSR );
			}
		}

		eRet = W_FIRED;

		// Check to see if we need to reload...

		if (nShotsPerClip > 0)
		{
			if (m_nAmmoInClip <= 0)
			{
				// Only automatically reload if we're the player... and don't play the animation.

				if ( IsPlayer(info.hFiredFrom) )
				{
					ReloadClip(false, nAmmo);
				}
			}
		}
	}

	return eRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::CreateProjectile
//
//	PURPOSE:	Create the approprite projectile to fire.
//
// ----------------------------------------------------------------------- //

bool CWeapon::CreateProjectile(LTRotation & rRot, WeaponFireInfo & info)
{
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo,IsAI(info.hFiredFrom));
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
	if( !hProjectileFX || LTStrEmpty(g_pFXDB->GetString(hProjectileFX,FXDB_sClass)) )
		return false;

	ObjectCreateStruct theStruct;

	// set the starting rotation
    theStruct.m_Rotation = rRot;

	// move the start position along the line of fire a little
	if ( MATH_EPSILON < g_pFXDB->GetFloat(hProjectileFX,FXDB_fFireOffset ))
	{
		// determine the new point
		LTVector vNewFirePos;
		vNewFirePos = info.vPath;
		vNewFirePos *= g_pFXDB->GetFloat(hProjectileFX,FXDB_fFireOffset );
		vNewFirePos += info.vFirePos;

		// see if there is any geometry in the way
		IntersectInfo iInfo;
		IntersectQuery qInfo;
		qInfo.m_From = info.vFirePos;
		qInfo.m_To = vNewFirePos;
		qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
		qInfo.m_FilterFn  = SpecificObjectFilterFn;
		qInfo.m_PolyFilterFn = NULL;
		qInfo.m_pUserData = info.hFiredFrom;

		if ( true == g_pLTServer->IntersectSegment( qInfo, &iInfo ) )
		{
			// hit something, just put the fire position at the original
			theStruct.m_Pos = info.vFirePos;
		}
		else
		{
			// hit nothing, use the new fire position
			theStruct.m_Pos = vNewFirePos;
		}
	}
	else
	{
		theStruct.m_Pos = info.vFirePos;
	}

	HCLASS hClass = g_pLTServer->GetClass( g_pFXDB->GetString(hProjectileFX,FXDB_sClass) );

	LTASSERT_PARAM1( hClass, "Unable to retreive class: %s", g_pFXDB->GetString(hProjectileFX,FXDB_sClass) );

	if (hClass)
	{
        CProjectile* pProj = (CProjectile*)g_pLTServer->CreateObject(hClass, &theStruct);
		if (pProj)
		{
			if( !pProj->Setup(this, info) )
			{
				g_pLTServer->RemoveObject( pProj->m_hObject );
				return false;
			}

			return true;
		}
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::SetModelObject()
//
//	PURPOSE:	Set the object associated with the weapon model
//
// ----------------------------------------------------------------------- //

void CWeapon::SetModelObject(HOBJECT hObj)
{
	m_hModelObject = hObj;

	if (m_hModelObject)
	{
		// Init the model animations

		g_pModelLT->GetAnimIndex( m_hModelObject, "Hand",		m_nHandAni );
		g_pModelLT->GetAnimIndex( m_hModelObject, "PreFire",	m_nPreFireAni );
		g_pModelLT->GetAnimIndex( m_hModelObject, "Fire",		m_nFireAni );
		g_pModelLT->GetAnimIndex( m_hModelObject, "PostFire",	m_nPostFireAni );
		g_pModelLT->GetAnimIndex( m_hModelObject, "Reload",		m_nReloadAni );
		g_pModelLT->GetAnimIndex( m_hModelObject, "PlayerBase",	m_nPlayerAni );

        g_pModelLT->SetLooping( m_hModelObject, MAIN_TRACKER, false );
	}
}

void CWeapon::SetDualWeaponModelObject( HOBJECT hObject )
{
	m_hDualWeaponModel = hObject;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Select()
//
//	PURPOSE:	Select the weapon
//
// ----------------------------------------------------------------------- //

void CWeapon::Select()
{
	// We must have ammo in the clip the very first time the weapon is selected...

	if( m_bFirstSelection )
	{
		m_bFirstSelection = false;

		// Set the initial ammo count...

		int32 nAmmo = m_pArsenal->GetAmmoCount( m_hAmmo );
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, IsAI(m_hObject));
		int32 nShotsPerClip = g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nShotsPerClip );

		if (nAmmo > 0 && nShotsPerClip > 0)
		{
			m_nAmmoInClip = nAmmo < nShotsPerClip ? nAmmo : nShotsPerClip;
		}
	}

	if (IsPlayer(m_hObject))
	{
		PlayAnimation(m_nPlayerAni,true,false);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Deselect()
//
//	PURPOSE:	Deselect the weapon
//
// ----------------------------------------------------------------------- //

void CWeapon::Deselect()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CWeapon::Save(ILTMessage_Write *pMsg, uint8 nType)
{
	if (!pMsg) return;

	SAVE_HOBJECT( m_hObject );
    SAVE_HOBJECT( m_hModelObject );
	SAVE_HOBJECT( m_hDualWeaponModel );

    SAVE_FLOAT( m_fLifeTime );
    SAVE_INT( m_nCurTracer );
    SAVE_BOOL( m_bHave );
    SAVE_INT( m_nAmmoInClip );
    SAVE_BYTE( m_eState );
    SAVE_FLOAT( m_fDamageFactor );
    SAVE_FLOAT( m_fRangeAdjust );
	SAVE_HRECORD( m_hWeapon );
	SAVE_HRECORD( m_hAmmo );
	SAVE_BOOL( m_bFirstSelection );
	
	m_LastFireTime.Save( *pMsg );
	SAVE_DWORD( m_nLastTimestamp );

	for( uint8 i = 0; i < W_MAX_MODS; ++i )
	{
		SAVE_HRECORD( m_Mods[i] );
	}

	SAVE_bool( m_bHidden );

	SAVE_DWORD( m_nHealth );
	SAVE_DWORD( m_nMaxHealth );
	SAVE_DWORD( m_nWarnHealth );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CWeapon::Load(ILTMessage_Read *pMsg, uint8 nType)
{
	if (!pMsg) return;

    LOAD_HOBJECT( m_hObject );
    LOAD_HOBJECT( m_hModelObject );
	SetModelObject( m_hModelObject );

	LOAD_HOBJECT( m_hDualWeaponModel );

    LOAD_FLOAT( m_fLifeTime );
    LOAD_INT( m_nCurTracer );
    LOAD_BOOL( m_bHave );
    LOAD_INT( m_nAmmoInClip );
    LOAD_BYTE_CAST( m_eState, WeaponState );
    LOAD_FLOAT( m_fDamageFactor );
    LOAD_FLOAT( m_fRangeAdjust );
	LOAD_HRECORD( m_hWeapon, g_pWeaponDB->GetWeaponsCategory() );
	LOAD_HRECORD( m_hAmmo, g_pWeaponDB->GetAmmoCategory() );
	LOAD_BOOL( m_bFirstSelection );
		
	m_LastFireTime.Load( *pMsg );
	LOAD_DWORD( m_nLastTimestamp );

	for (int i=0; i < W_MAX_MODS; i++)
	{
		LOAD_HRECORD( m_Mods[i], g_pWeaponDB->GetModsCategory() );
	}

	LOAD_bool( m_bHidden );

	LOAD_DWORD( m_nHealth );
	LOAD_DWORD( m_nMaxHealth );
	LOAD_DWORD( m_nWarnHealth );
	CheckWeaponIsWeak();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetInstDamage()
//
//	PURPOSE:	Get the instant damage done by this weapon (value can vary)
//
// ----------------------------------------------------------------------- //

float CWeapon::GetInstDamage() const
{
	if( !m_hAmmo )
		return 0.0f;

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo,IsAI(m_hObject));
	float fDamage = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fInstDamage );
	//we don't want to randomize our damage, because it makes balancing the weapons more difficult
	//fDamage *= GetRandom(0.8f, 1.2f) * m_fDamageFactor;
	
	return fDamage;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::GetProgDamage()
//
//	PURPOSE:	Get the progressive damage done by this weapon
//
// ----------------------------------------------------------------------- //

float CWeapon::GetProgDamage() const
{
	if( !m_hAmmo )
		return 0.0f;

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo,IsAI(m_hObject));
	float fDamage = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamage );
	fDamage *= m_fDamageFactor;

	return fDamage;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeapon::AddMod
//
//	PURPOSE:	Add the specified mod to the weapon
//
// ----------------------------------------------------------------------- //

bool CWeapon::AddMod( HMOD hMod )
{
	if( !hMod )
		return false;

	for( int i = 0; i < W_MAX_MODS; ++i )
	{
		if( m_Mods[i] )
		{
			m_Mods[i] = hMod;
			return true;
		}
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::PlayAnimation
//
//  PURPOSE:	Plays the given animation of the model.
//
// ----------------------------------------------------------------------- //

bool CWeapon::PlayAnimation( HMODELANIM hAni, bool bForce, bool bLoop )
{
	if( hAni == INVALID_ANI	|| !m_hModelObject )
		return false;

	// See if we should brute force the animation or wait until the current ani is done...

	if( !bForce )
	{
		uint32 dwFlags;
		g_pModelLT->GetPlaybackState( m_hModelObject, MAIN_TRACKER, dwFlags );
		
		if( !(dwFlags & MS_PLAYDONE) )
			return false;
	}

	g_pModelLT->SetCurAnim( m_hModelObject, MAIN_TRACKER, hAni, true );
	g_pModelLT->SetLooping( m_hModelObject, MAIN_TRACKER, bLoop );
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::SetAnimRate
//
//  PURPOSE:	Set the weapons animation rate.
//
// ----------------------------------------------------------------------- //

void CWeapon::SetAnimRate( float flRate )
{
	g_pModelLT->SetAnimRate( m_hModelObject, MAIN_TRACKER, flRate );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::KillLoopSound
//
//  PURPOSE:	Call to the hand held weapon to kill any looping sound it may have.
//
// ----------------------------------------------------------------------- //

void CWeapon::KillLoopSound()
{
	if( !m_hModelObject )
		return;

	CHHWeaponModel *pHHWeapon = dynamic_cast< CHHWeaponModel* >( g_pLTServer->HandleToObject( m_hModelObject ));
	if( pHHWeapon )
	{
		pHHWeapon->KillLoopSound();
	}

	if( m_hDualWeaponModel )
	{
		pHHWeapon = dynamic_cast<CHHWeaponModel*>(g_pLTServer->HandleToObject( m_hDualWeaponModel ));
		if( pHHWeapon )
			pHHWeapon->KillLoopSound();
	}
}

float CWeapon::GetLifeTime() const
{
	if( !m_hAmmo )
		return 0.0f;

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_hAmmo,IsAI(m_hObject));
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
	if( !hProjectileFX )
		return 0.0f;

	return (m_fLifeTime < 0.0f ? g_pFXDB->GetFloat(hProjectileFX,FXDB_fLifetime) : m_fLifeTime);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::HideWeapon
//
//  PURPOSE:	Hide or show the weapon...
//
// ----------------------------------------------------------------------- //

void CWeapon::HideWeapon( bool bHidden )
{
	m_bHidden = bHidden;
	
	EEngineLOD eShadowLOD = eEngineLOD_Never;
	g_pLTServer->GetObjectShadowLOD( m_hObject, eShadowLOD );

	if( m_hModelObject )
	{
		g_pCommonLT->SetObjectFlags( m_hModelObject, OFT_Flags, (m_bHidden ? FLAG_FORCECLIENTUPDATE : FLAG_VISIBLE), FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );
		g_pLTServer->SetObjectShadowLOD( m_hModelObject, m_bHidden ? eEngineLOD_Never : eShadowLOD );
	}

	if( m_hDualWeaponModel )
	{
		g_pCommonLT->SetObjectFlags( m_hDualWeaponModel, OFT_Flags, (m_bHidden ? FLAG_FORCECLIENTUPDATE : FLAG_VISIBLE), FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );
		g_pLTServer->SetObjectShadowLOD( m_hDualWeaponModel, m_bHidden ? eEngineLOD_Never : eShadowLOD );
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::Reset
//
//  PURPOSE:	Handle reseting the weapon when the inventory is reset
//
// ----------------------------------------------------------------------- //

void CWeapon::Reset()
{
	// Reset the 'first selection' to insure the clip is loaded when it 
	// should be.  If this is not done, the clip may start empty even though
	// the user has ammo.  This occurs due to the arsenal giving the player
	// their default weapon without any ammo (which initializes the clip to 
	// empty (when the arsenal is reset, on spawning, this causes the clip
	// to be refilled as the next selection will be considered the first)

	m_bFirstSelection = true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::SetHealth
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CWeapon::SetHealth(int32 nNewHealth)
{
	m_nHealth = nNewHealth;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::CheckWeaponIsWeak
//
//  PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CWeapon::CheckWeaponIsWeak()
{
	if (m_nWarnHealth > 0 && m_nHealth <= m_nWarnHealth)
	{
		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hObject ));
		if (pPlayer)
			pPlayer->WarnWeaponWillBreak(m_hWeapon);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CWeapon::DamageWeapon
//
//  PURPOSE:	Applies an impact to the weapon itself.
//
// ----------------------------------------------------------------------- //

void CWeapon::DamageWeapon(int32 nAmount)
{
	int32 nOldHealth = m_nHealth;

	m_nHealth -= nAmount;

	//g_pLTServer->CPrint("Weapon health old/cur/max: %d / %d / %d", nOldHealth, m_nHealth, m_nMaxHealth);

	// if the weapon health just dipped below the warning health...
	if( nOldHealth > m_nWarnHealth )
	{
		CheckWeaponIsWeak();
	}
	// check if weapon just broke
	else if (m_nMaxHealth > 0 && m_nHealth <= 0)
	{
		// unacquire this weapon
		Drop();

		// spawn debris effect
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, IsAI(m_hObject));
		const char* pszDebrisFX = g_pWeaponDB->GetString(hWpnData, WDB_WEAPON_sDebrisFX);
		const char* pszSocket   = g_pWeaponDB->GetString(hWpnData, WDB_WEAPON_sDebrisSocket);

		if (!LTStrEmpty(pszDebrisFX) && !LTStrEmpty(pszSocket))
		{
			// find socket
			HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
			if( g_pModelLT->GetSocket( m_hModelObject, pszSocket, hSocket ) == LT_OK )
			{
				if( hSocket != INVALID_MODEL_SOCKET )
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8(SFX_CLIENTFXGROUPINSTANT);
					cMsg.WriteString(pszDebrisFX);
					cMsg.Writebool(false); // no looping
					cMsg.Writebool(false); // no smooth shutdown
					cMsg.Writebool(true);  // need special parent to specify socket
					cMsg.WriteObject(m_hModelObject);
					cMsg.Writeuint32(hSocket);
					cMsg.Writebool( false ); // No target info

					g_pLTServer->SendSFXMessage( cMsg.Read(), 0 );
				}
			}
		}

		// character specific behavior
		HWEAPON hReplacementWeapon = g_pWeaponDB->GetRecordLink(hWpnData, WDB_WEAPON_rBrokenWeapon);
		if (IsPlayer(m_hObject))
		{
			CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hObject ));
			if (pPlayer)
				pPlayer->HandleWeaponBroke(m_hWeapon, hReplacementWeapon);
		}
		else if (IsAI(m_hObject))
		{
			CAI* pAI = dynamic_cast<CAI*>(g_pLTServer->HandleToObject( m_hObject ));
			if (pAI)
				pAI->HandleWeaponBroke(m_hWeapon, hReplacementWeapon);
		}
	}
}
