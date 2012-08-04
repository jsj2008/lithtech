// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.cpp
//
// PURPOSE : Projectile class - implementation
//
// CREATED : 9/25/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Projectile.h"
#include "ltengineobjects.h"
#include "iltserver.h"
#include "MsgIDs.h"
#include "ServerUtilities.h"
#include "Explosion.h"
#include "Character.h"
#include "ClientWeaponSFX.h"
#include "WeaponFXTypes.h"
#include "VolumeBrush.h"
#include "VolumeBrushTypes.h"
#include "ClientServerShared.h"
#include "SurfaceFunctions.h"
#include "PlayerObj.h"
#include "AI.h"
#include "AIDB.h"
#include "VarTrack.h"
#include "iltmodel.h"
#include "iltphysics.h"
#include "ObjectMsgs.h"
#include "CharacterHitBox.h"
#include "CharacterDB.h"
#include "GameServerShell.h"
#include "Camera.h"
#include "AIStimulusMgr.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "AIUtils.h"
#include "VersionMgr.h"
#include "Attachments.h"
#include <algorithm>
#include "FXDB.h"
#include "PhysicsUtilities.h"
#include "GameModeMgr.h"
#include "TeamMgr.h"
#include "ServerConnectionMgr.h"
#include "ObjectTransformHistory.h"
#include "ltintersect.h"

LINKFROM_MODULE( Projectile );

BEGIN_CLASS(CProjectile)
END_CLASS_FLAGS(CProjectile, GameBase, CF_HIDDEN, "")

CMDMGR_BEGIN_REGISTER_CLASS( CProjectile )
CMDMGR_END_REGISTER_CLASS( CProjectile, GameBase )


extern uint16 g_wIgnoreFX;
extern CAIStimulusMgr* g_pAIStimulusMgr;

static bool DoVectorFilterFn(HOBJECT hObj, void *pUserData);
static bool CanCharacterHitCharacter( CProjectile* pProjectile, HOBJECT hImpacted );

#define PROJECTILE_DEFAULT_BULLET_FORCE	1200.0f
static VarTrack g_vtInvisibleMaxThickness;
static VarTrack g_vtPhysicsBulletForce;
static VarTrack g_vtUseHistoricalObjectTransforms;

namespace
{
	void ProjectileInitFileGlobals()
	{
		if (!g_vtInvisibleMaxThickness.IsInitted())
		{
			g_vtInvisibleMaxThickness.Init(g_pLTServer, "InvisibleMaxThickness", NULL, 33.0f);
		}
		if (!g_vtPhysicsBulletForce.IsInitted())
		{
			g_vtPhysicsBulletForce.Init(g_pLTServer, "PhysicsBulletForce", NULL, PROJECTILE_DEFAULT_BULLET_FORCE);
		}

		if( !g_vtUseHistoricalObjectTransforms.IsInitted( ))
		{
			g_vtUseHistoricalObjectTransforms.Init( g_pLTServer, "UseHistoricalObjectTransforms", NULL, 1.0f );
		}
	}

	// Period to calculate the direction of heat seeking projectiles.
	const float kHeatSeekingUpdatePeriod = 0.2f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageFilterFunctionHook()
//
//	PURPOSE:	Function that the m_destructable aggregate calls (if a
//				function is registered) when damage has been taken.
//
// ----------------------------------------------------------------------- //

static bool DamageFilterHook( GameBase *pObject, DamageStruct *pDamageStruct )
{
	// cast to a game base object
	CProjectile *pMyObj = dynamic_cast< CProjectile* >( pObject );

	// call the most derived class
	return pMyObj->FilterDamage( pDamageStruct );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CProjectile
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CProjectile::CProjectile() 
	: GameBase(OT_MODEL)
	, m_nTotalRicochets( 0 )
	, m_nUpdateNum( 0 )
	, m_nLastRicochetUpdateNum( -1 )
	, m_Shared( )
{
	AddAggregate(&m_damage);
	MakeTransitionable();

	// damage filtering
	m_damage.RegisterFilterFunction( DamageFilterHook, this );

	m_vFlashPos.Init();
	m_vFirePos.Init();
	m_vDir.Init();

    m_hObject				= NULL;
	m_fVelocity				= 0.0f;
	m_fInstDamage			= 0.0f;
	m_fInstPenetration		= 0.0f;
	m_fProgDamage			= 0.0f;
	m_fMass					= INFINITE_MASS; //0.0f;
	m_fLifeTime				= 5.0f;
	m_fRange				= 10000.0f;
    m_bSilenced				= false;
	m_bSetup				= false;

	m_bObjectRemoved		= false;
	m_bDetonated			= false;
	m_bRecoverable			= false;

	m_bCanHitSameProjectileKind = false;
	m_bDamagedByOwner 			= true;
	m_bCanTouchFiredFromObj		= false;

	m_eInstDamageType		= DT_BULLET;
	m_eProgDamageType		= DT_UNSPECIFIED;
	m_fInstDamage			= 0.0f;
	m_fInstPenetration		= 0.0f;
	m_fProgDamage			= 0.0f;

	m_fRadius				= 0.0f;
	m_fStartTime			= 0.0f;
				
	m_hFiringWeapon			= NULL;

	m_bNumCallsToAddImpact	= 0;

	m_bProcessInvImpact		= false;
	m_vInvisVel.Init();
	m_vInvisNewPos.Init();

	m_vDims.Init(1.0f, 1.0f, 1.0f);

	m_dwFlags = FLAG_FORCECLIENTUPDATE | FLAG_POINTCOLLIDE | FLAG_NOSLIDING |
				FLAG_TOUCH_NOTIFY | FLAG_NOLIGHT | FLAG_RAYHIT;

	static DamageFlags nCantDamageFlags = 
			  DamageTypeToFlag(DT_BURN) 
			| DamageTypeToFlag(DT_ELECTRICITY)
			| DamageTypeToFlag(DT_ENDLESS_FALL)
			| DamageTypeToFlag(DT_STUN);

	// Make sure we can't be damaged by any of the progressive/volumebrush
	// damage types...
	m_nCantDamageFlags		= nCantDamageFlags;

	m_bSendWeaponRecord		= true;
	m_bSendAmmoRecord		= true;

	m_bGuaranteedHit		= false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CProjectile::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			// keep track of the number of this update
			++m_nUpdateNum;

			Update();
		}
		break;

		case MID_PRECREATE:
		{
			if( fData != PRECREATE_SAVEGAME )
			{
				ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
				if (pStruct)
				{
					pStruct->m_Flags = m_dwFlags;

					// This will be set in Setup()...
					pStruct->SetFileName("Models\\Default." RESEXT_MODEL_PACKED);

					pStruct->m_NextUpdate = UPDATE_NEXT_FRAME;

					// We're not using rigidbodies, but just make
					// sure we are nonsolid.
					pStruct->m_eGroup = ePhysicsGroup_NonSolid;
				}
			}
		}
		break;

		case MID_INITIALUPDATE:
		{
			InitialUpdate((int)fData);
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			if( !m_bSetup ) break;
			HandleTouch((HOBJECT)pData);
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

		case MID_SAVESPECIALEFFECTMESSAGE:
		{
			SaveSFXMessage( static_cast<ILTMessage_Write*>( pData ), static_cast<uint32>( fData ) );
		}
		break;

		case MID_LOADSPECIALEFFECTMESSAGE:
		{
			LoadSFXMessage( static_cast<ILTMessage_Read*>( pData ), static_cast<uint32>( fData ) );
		}
		break;

		case MID_MODELSTRINGKEY :
		{
			HandleModelString( (ArgList*)pData );
		}
		break;


		default : break;
	}


	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CProjectile::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	uint32 dwRet = GameBase::ObjectMessageFn(hSender, pMsg);

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if (m_damage.IsDead())
			{
				if (IsPlayer(m_damage.GetLastDamager()))
				{
					m_Shared.m_hFiredFrom = m_damage.GetLastDamager();
				}


				// If we are getting damaged from and explosion, wait a little bit so that we
				// can also explode in an exciting chain reaction!
				if (m_damage.GetDeathType() == DT_EXPLODE)
				{
					const HRECORD hGlobalRec = g_pWeaponDB->GetGlobalRecord();
					const float fNewLifeTime = float(g_pWeaponDB->GetInt32(hGlobalRec,WDB_GLOBAL_tProjectileExplosionDelay)) / 1000.0f;
					const double fCurrentTime = SimulationTimer::Instance().GetTimerAccumulatedS();

					if( m_fLifeTime <= 0.0f )
					{
						// This is the first time we are hit, so just start our timer
						// before blowing up.
						m_fLifeTime = fNewLifeTime;
						m_fStartTime = fCurrentTime;
					}
					else
					{
						// We are already waiting to explode, only explode if our new life-time is less than our remaining life-time.
						const double fRemainingLifeTime = m_fLifeTime - (fCurrentTime - m_fStartTime);
	
						if( fNewLifeTime < fRemainingLifeTime )
						{
							m_fLifeTime = fNewLifeTime;
							m_fStartTime = fCurrentTime;
						}
					}
					
				}
				else
				{
					// Hmmm, some other type of damage.  We'll just blow up right away.
					Detonate(NULL);
				}
				
			}
		}
		break;

		default : break;
	}

	return dwRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Setup
//
//	PURPOSE:	Set up a projectile with the information needed
//
// ----------------------------------------------------------------------- //

bool CProjectile::Setup(CWeapon const* pWeapon, WeaponFireInfo const & info)
{
	if (!pWeapon || !info.hFiredFrom || !g_pWeaponDB) return false;

	ProjectileInitFileGlobals();

	// get the fire position
	m_vFirePos          = info.vFirePos;
	
	// get the flash position
	m_vFlashPos         = info.vFlashPos;

	// get the direction
	m_vDir              = info.vPath;

	// make sure its normalized
	m_vDir.Normalize();

	// determined who fired the projectile
	SetFiredFrom( info.hFiredFrom );

	m_hFiringWeapon		= info.hFiringWeapon;

	// If this is a team game, get any team id from the firedfrom object.
	if( GameModeMgr::Instance( ).m_grbUseTeams)
	{
		SetTeamId( GetPlayerTeamId( m_Shared.m_hFiredFrom ));
	}

	

#ifdef _DEBUG

	HCLASS hOwnerClass = g_pLTServer->GetObjectClass( m_Shared.m_hFiredFrom );
	char szOwnerClassName[ 128 ];
	LTRESULT ltResult;
	ltResult = g_pLTServer->GetClassName( hOwnerClass, szOwnerClassName, 128 );
	ASSERT( LT_OK == ltResult );

	if( LT_INSIDE != g_pLTServer->Common()->GetPointStatus( &m_vFirePos ) )
	{
		DebugCPrint(1, "Fire Pos from '%s' was outside world", szOwnerClassName );
	}
	
	if( LT_INSIDE != g_pLTServer->Common()->GetPointStatus( &m_vFlashPos ) )
	{
		DebugCPrint(1, "Flash Pos from '%s' was outside world", szOwnerClassName );
	}

#endif

	// get the weapon/ammo ids
	m_Shared.m_hWeapon	= pWeapon->GetWeaponRecord();
	m_Shared.m_hAmmo	= pWeapon->GetAmmoRecord();
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));

	// Should the weapon and ammo records be sent to clients...
	m_bSendWeaponRecord	= info.bSendWeaponRecord;
	m_bSendAmmoRecord	= info.bSendAmmoRecord;

	m_bGuaranteedHit = info.bGuaranteedHit;

	m_bLeftHandWeapon = info.bLeftHandWeapon;


	// determine if this projectile can impact
	// projectiles of the same kind
	if( hAmmoData )
	{
		HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
		m_bCanHitSameProjectileKind = (hProjectileFX ? g_pFXDB->GetBool(hProjectileFX,FXDB_bCanHitSameKind) : false);
		m_bDamagedByOwner = (hProjectileFX ? g_pFXDB->GetBool(hProjectileFX,FXDB_bDamagedByOwner) : false);
	}

	// set the lifetime of the projectile
	m_fLifeTime			= pWeapon->GetLifeTime();

	// setup the damage info
	m_fInstDamage		= pWeapon->GetInstDamage();
	m_fProgDamage		= pWeapon->GetProgDamage();
	m_eInstDamageType	= g_pWeaponDB->GetAmmoInstDamageType( m_Shared.m_hAmmo, IsAI(m_Shared.m_hFiredFrom) );
	m_eProgDamageType	= g_pWeaponDB->GetAmmoProgDamageType( m_Shared.m_hAmmo, IsAI(m_Shared.m_hFiredFrom) );
	m_fInstPenetration	= g_pWeaponDB->GetFloat(hAmmoData,WDB_AMMO_fInstPenetration);

	// determine the velocity
	if (info.bOverrideVelocity)
	{
		m_fVelocity = info.fOverrideVelocity;
	}
	else
	{
		HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
		m_fVelocity = (float) (hProjectileFX ? g_pFXDB->GetInt32(hProjectileFX,FXDB_nVelocity) : 0);
	}

	// determine the projectile's range
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_Shared.m_hWeapon,IsAI(m_Shared.m_hFiredFrom));
		
	int32 nWeaponRange	= g_pWeaponDB->GetInt32( hWpnData, WDB_WEAPON_nRange );
	int32 nAmmoRange	= g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nRange );
	m_fRange            = (float)(nAmmoRange > 0 ? nAmmoRange : nWeaponRange);

	// get the special case stuff
	m_bSilenced         = !!(pWeapon->GetSilencer());

	// determine ammo type
	AmmoType eAmmoType  = (AmmoType)g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType );

	// no calls to add impact yet
	m_bNumCallsToAddImpact = 0;

	m_hNodeHit = info.hNodeHit;

	// [RP] - Beyond this point nothing else can justify a bad setup so set a successful 
	//		  setup here.  This way calls that rely on a successful setup will have accurate info.

	m_bSetup = true;

	// See if we start inside the test object...
	// If we are inside the test object, hit that
	// test object right away, and DON'T spawn
	// a projectile or a vector.
	if (!TestInsideObject(info.hTestObj, eAmmoType))
	{
		// the projectile is NOT inside the test object
		if (eAmmoType == PROJECTILE)
		{
			// 
			DoProjectile();
		}
		else if (eAmmoType == VECTOR)
		{
			DoVector( info );
		}
	}

	// register a Enemy Weapon Fire Sound AND a Ally WeaponFireSound

	// Get the Distance that fire noise carries	
	float fWeaponFireNoiseDistance = g_pWeaponDB->GetFloat(hWpnData,WDB_WEAPON_fAIFireSndRadius);

	// If we're silenced use the radius specified by the silencer...
	if( m_bSilenced )
	{
		HMOD hMod = pWeapon->GetSilencer();
		if( hMod )
		{
			fWeaponFireNoiseDistance = g_pWeaponDB->GetFloat( hMod, WDB_MOD_nAISilencedFireSndRadius );
		}
	}

	if( fWeaponFireNoiseDistance > 0.f && IsCharacter( m_Shared.m_hFiredFrom ))
	{
		// Register the AI stimulus.

		EnumCharacterAlignment eAlignment = kCharAlignment_Invalid;
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_Shared.m_hFiredFrom);
		if ( pChar )
		{
			eAlignment = pChar->GetAlignment();
		}

		StimulusRecordCreateStruct scs( kStim_WeaponFireSound, eAlignment, m_vFirePos, m_Shared.m_hFiredFrom );
		scs.m_flRadiusScalar = fWeaponFireNoiseDistance;
		g_pAIStimulusMgr->RegisterStimulus( scs );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Setup
//
//	PURPOSE:	Alternate setup approach, using just an ammo record
//
// ----------------------------------------------------------------------- //

bool CProjectile::Setup( HWEAPON hWeapon, HAMMO hAmmo, WeaponFireInfo const &wfi )
{
	if( !hWeapon || !hAmmo || !wfi.hFiredFrom || !g_pWeaponDB )
	{
		return false;
	}

	// Some general setup...
	ProjectileInitFileGlobals();

	// Copy over some weapon fire information
	m_vFirePos				= wfi.vFirePos;
	m_vFlashPos				= wfi.vFlashPos;
	m_vDir					= wfi.vPath.GetUnit();
	SetFiredFrom( wfi.hFiredFrom );
	m_hFiringWeapon			= wfi.hFiringWeapon;
	m_Shared.m_hWeapon		= hWeapon;
	m_Shared.m_hAmmo		= hAmmo;

	// Should the weapon and ammo records be sent to clients...
	m_bSendWeaponRecord	= wfi.bSendWeaponRecord;
	m_bSendAmmoRecord	= wfi.bSendAmmoRecord;

	m_bGuaranteedHit = wfi.bGuaranteedHit;

	m_bLeftHandWeapon = wfi.bLeftHandWeapon;

	// If this is a team game, get any team id from the firedfrom object.
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		SetTeamId( GetPlayerTeamId( m_Shared.m_hFiredFrom ) );
	}


	// Determine if this projectile can impact projectiles of the same kind
	HAMMODATA hAmmoData		= g_pWeaponDB->GetAmmoData( m_Shared.m_hAmmo, IsAI( m_Shared.m_hFiredFrom ) );
	HRECORD hProjectileFX	= g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX );
	HWEAPONDATA hWeaponData	= g_pWeaponDB->GetWeaponData( m_Shared.m_hWeapon, IsAI( m_Shared.m_hFiredFrom ) );

	if( hAmmoData )
	{
		m_bCanHitSameProjectileKind = ( hProjectileFX ? g_pFXDB->GetBool( hProjectileFX, FXDB_bCanHitSameKind ) : false );
		m_bDamagedByOwner = (hProjectileFX ? g_pFXDB->GetBool(hProjectileFX,FXDB_bDamagedByOwner) : false);
	}

	// Set the lifetime of the projectile
	m_fLifeTime				= g_pFXDB->GetFloat( hProjectileFX, FXDB_fLifetime );

	// Setup the damage info
	m_fInstDamage			= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fInstDamage );
	m_fProgDamage			= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamage );
	m_eInstDamageType		= g_pWeaponDB->GetAmmoInstDamageType( m_Shared.m_hAmmo, IsAI( m_Shared.m_hFiredFrom ) );
	m_eProgDamageType		= g_pWeaponDB->GetAmmoProgDamageType( m_Shared.m_hAmmo, IsAI( m_Shared.m_hFiredFrom ) );
	m_fInstPenetration		= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fInstPenetration );

	// Determine the velocity
	if( wfi.bOverrideVelocity )
	{
		m_fVelocity = wfi.fOverrideVelocity;
	}
	else
	{
		m_fVelocity = ( float )( hProjectileFX ? g_pFXDB->GetInt32( hProjectileFX, FXDB_nVelocity ) : 0 );
	}

	// Determine the projectile range
	int32 nWeaponRange		= g_pWeaponDB->GetInt32( hWeaponData, WDB_WEAPON_nRange );
	int32 nAmmoRange		= g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nRange );
	m_fRange				= ( float )( nAmmoRange > 0 ? nAmmoRange : nWeaponRange );

	// Determine ammo type
	AmmoType eAmmoType		= ( AmmoType )g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType );

	// No calls to add impact yet
	m_bNumCallsToAddImpact	= 0;

	m_hNodeHit = wfi.hNodeHit;

	// See if we start inside the test object...
	// If we are inside the test object, hit that test object right away, and DON'T spawn a projectile or a vector.
	if( !TestInsideObject( wfi.hTestObj, eAmmoType ) )
	{
		// the projectile is NOT inside the test object
		if( eAmmoType == PROJECTILE )
		{
			DoProjectile();
		}
		else if ( eAmmoType == VECTOR )
		{
			DoVector( wfi );
		}
	}

	// Get the Distance that fire noise carries	
	float fWeaponFireNoiseDistance = g_pWeaponDB->GetFloat( hWeaponData, WDB_WEAPON_fAIFireSndRadius );

	if( ( fWeaponFireNoiseDistance > 0.0f ) && IsCharacter( m_Shared.m_hFiredFrom ) )
	{
		// Register the AI stimulus.
		EnumCharacterAlignment eAlignment = kCharAlignment_Invalid;
		CCharacter* pChar = ( CCharacter* )g_pLTServer->HandleToObject( m_Shared.m_hFiredFrom );

		if( pChar )
		{
			eAlignment = pChar->GetAlignment();
		}

		StimulusRecordCreateStruct scs( kStim_WeaponFireSound, eAlignment, m_vFirePos, m_Shared.m_hFiredFrom );
		scs.m_flRadiusScalar = fWeaponFireNoiseDistance;
		g_pAIStimulusMgr->RegisterStimulus( scs );
	}


	m_bSetup = true;
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Setup
//
//	PURPOSE:	Set up a projectile with the minimum required info
//
// ----------------------------------------------------------------------- //

bool CProjectile::Setup(HAMMO hAmmo, LTRigidTransform const &trans)
{
	if (!g_pWeaponDB) return false;

	g_wIgnoreFX = WFX_SILENCED | WFX_ALTFIRESND;

	ProjectileInitFileGlobals();
	// get the fire position
	m_vFirePos          = trans.m_vPos;
	m_vFlashPos         = trans.m_vPos;
	m_vDir              = trans.m_rRot.Forward();

	// determined who fired the projectile
	SetFiredFrom( NULL );

	m_hFiringWeapon		= NULL;

	// get the weapon/ammo ids
	m_Shared.m_hWeapon	= g_pWeaponDB->GetWeaponFromAmmo(hAmmo,!USE_AI_DATA);
	m_Shared.m_hAmmo	= hAmmo;
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,!USE_AI_DATA);

	// determine if this projectile can impact
	// projectiles of the same kind
	if( !hAmmoData )
		return false;

	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
	m_bCanHitSameProjectileKind = (hProjectileFX ? g_pFXDB->GetBool(hProjectileFX,FXDB_bCanHitSameKind) : false);
	m_bDamagedByOwner = (hProjectileFX ? g_pFXDB->GetBool(hProjectileFX,FXDB_bDamagedByOwner) : false);

	// set the lifetime of the projectile
	m_fLifeTime			= g_pFXDB->GetFloat(hProjectileFX,FXDB_fLifetime);

	// setup the damage info
	m_fInstDamage		= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fInstDamage );
	m_fProgDamage		= g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamage );
	m_eInstDamageType	= g_pWeaponDB->GetAmmoInstDamageType( m_Shared.m_hAmmo, IsAI(m_Shared.m_hFiredFrom) );
	m_eProgDamageType	= g_pWeaponDB->GetAmmoProgDamageType( m_Shared.m_hAmmo, IsAI(m_Shared.m_hFiredFrom) );
	m_fInstPenetration	= g_pWeaponDB->GetFloat(hAmmoData,WDB_AMMO_fInstPenetration);

	m_fVelocity = 5.0f;

	// determine the projectile's range
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_Shared.m_hWeapon,IsAI(m_Shared.m_hFiredFrom));

	int32 nAmmoRange	= g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nRange );
	m_fRange            = (float)nAmmoRange;

	// determine ammo type
	AmmoType eAmmoType  = (AmmoType)g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType );
	if (eAmmoType != PROJECTILE)
		return false;

	// no calls to add impact yet
	m_bNumCallsToAddImpact = 0;

	m_hNodeHit = (HMODELNODE) NULL;

	// [RP] - Beyond this point nothing else can justify a bad setup so set a successful 
	//		  setup here.  This way calls that rely on a successful setup will have accurate info.

	m_bSetup = true;

	DoProjectile();
	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TestInsideObject
//
//	PURPOSE:	Test to see if the projectile is inside the test object
//
// ----------------------------------------------------------------------- //

bool CProjectile::TestInsideObject(HOBJECT hTestObj, AmmoType eAmmoType)
{
	// This appears to be for the AIs to test the object they are
	// shooting at, apparently for the case where the AI is standing
	// on top of its target.  I'm guessing that if the projectile
	// gets 1 update before checking if it hit anything, there is a
	// chance the projectile will go through the targe.  If so, this
	// function will see that it is already "inside" the object, and
	// hit it immediately.

	// return if there is no test object
	if (!hTestObj) return false;

	if (hTestObj == m_Shared.m_hFiredFrom) 
	{
		return false;
	}

	// TO DO???
	// NOTE:  This code may need to be updated to use test the dims
	// of the CharacterHitBox instead of the dims of the object...
	// TO DO???

	// See if we are inside the test object...

	if (!m_bGuaranteedHit)	// only do the test if we're not sure we already hit something.
	{
		LTVector vTestPos, vTestDims;
		g_pLTServer->GetObjectPos(hTestObj, &vTestPos);
		g_pPhysicsLT->GetObjectDims(hTestObj, &vTestDims);

		if (m_vFirePos.x < vTestPos.x - vTestDims.x ||
			m_vFirePos.x > vTestPos.x + vTestDims.x ||
			m_vFirePos.y < vTestPos.y - vTestDims.y ||
			m_vFirePos.y > vTestPos.y + vTestDims.y ||
			m_vFirePos.z < vTestPos.z - vTestDims.z ||
			m_vFirePos.z > vTestPos.z + vTestDims.z)
		{
			// NOT inside the object, proceed as normal
			return false;
		}
	}

	// We're inside the object, so we automatically hit the object...

	if (eAmmoType == PROJECTILE)
	{
		// its a projectile, just detonate
		Detonate(hTestObj);
	}
	else
	{
		if (eAmmoType == VECTOR)
		{
			if (IsCharacter(hTestObj))
			{
				// If the test object is a character, add to the amount of
				// instant damage it will do based on the model node hit.

				CCharacter *pChar = (CCharacter*) g_pLTServer->HandleToObject(hTestObj);
				if (!pChar) return false;

				// Set the last node hit.  If a passed in node was specified, attempt to use it.
				// Otherwise, fall back to the skeletons default node.

				ModelsDB::HNODE hModelNode = NULL;
				if (m_hNodeHit != INVALID_MODEL_NODE)
				{
					char szName[64];
					if ( LT_OK == g_pLTServer->GetModelLT()->GetNodeName(hTestObj, m_hNodeHit, szName, LTARRAYSIZE(szName)) )
					{
						hModelNode = g_pModelsDB->GetSkeletonNode(pChar->GetModelSkeleton(), szName);
					}
				}

				if ( NULL == hModelNode )
				{
					hModelNode = g_pModelsDB->GetSkeletonDefaultHitNode(pChar->GetModelSkeleton());
				}

				pChar->SetModelNodeLastHit(hModelNode);

				//DANO: POTENTIAL PROBLEM!!
				// This variable m_fInstDamage gets changed here but
				// used by another function...in effect this variable
				// is getting passed "under the hood".  This can present
				// big problems with the Sequencer!
				AdjustDamage(pChar->ComputeDamageModifier(hModelNode));
			}

			ImpactDamageObject(m_Shared.m_hFiredFrom, hTestObj, m_vFlashPos, m_vFirePos, m_vDir);
		}

		AddImpact(hTestObj, m_vFlashPos, m_vFirePos, -m_vDir, GetSurfaceType(hTestObj), m_hNodeHit);
	}

	RemoveObject();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AdjustDamage()
//
//	PURPOSE:	Adjust the instant damage by a modifier
//
// ----------------------------------------------------------------------- //

void CProjectile::AdjustDamage(float fModifier)
{
	if( !m_Shared.m_hAmmo )
		return;

	// Only adjust the damage if we are using an adjustable damage type...
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	if( g_pWeaponDB->GetBool( hAmmoData, WDB_AMMO_bCanAdjustInstDamage ))
	{
		m_fInstDamage *= fModifier; 
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::InitialUpdate()
//
//	PURPOSE:	Do first update
//
// ----------------------------------------------------------------------- //

void CProjectile::InitialUpdate(int nInfo)
{
	// projectiles move alot, make the position updates unguaranteed
	// to help bandwidth
	g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED);

	//
	if (nInfo == INITIALUPDATE_SAVEGAME) return;

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_MOVEABLE, USRFLG_MOVEABLE);
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_SERVERDIMS, FLAG2_SERVERDIMS);

	// projectile visibility needs to be delayed on the client
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_DELAYCLIENTVISIBLE, FLAG_DELAYCLIENTVISIBLE);

	// setup the damage object
	m_damage.Init(m_hObject);
	m_damage.SetMass(m_fMass);
	m_damage.SetHitPoints(1.0f);
	m_damage.SetMaxHitPoints(1.0f);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
	m_damage.SetCanHeal(false);
	m_damage.SetCanRepair(false);

	// include our default "can't damage" types
	// in the damage object
	DamageFlags nDamageFlags = m_damage.GetCantDamageFlags();
	m_damage.SetCantDamageFlags(nDamageFlags | m_nCantDamageFlags);

	// prepare the next update
	SetNextUpdate(UPDATE_NEXT_FRAME);

	// Currently this is NOT set to the model's dimensions.
	// Should it be?
	g_pPhysicsLT->SetObjectDims(m_hObject, &m_vDims, 0);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Update()
//
//	PURPOSE:	Do update
//
// ----------------------------------------------------------------------- //

void CProjectile::Update()
{
	// Process moving us through an invisible object if necessary.  This is
	// done here since the position/velocity can't be updated in the touch
	// notify (since the engine is in the process of updating it)...
	if (m_bProcessInvImpact)
	{
		m_bProcessInvImpact = false;
		g_pPhysicsLT->SetVelocity(m_hObject, m_vInvisVel);
		g_pLTServer->SetObjectPos(m_hObject, m_vInvisNewPos);
	}

	// setup the next update
	SetNextUpdate(UPDATE_NEXT_FRAME);

	// Detonate if the life time has expired
	if ( (m_fLifeTime >= 0.0f) && SimulationTimer::Instance().GetTimerAccumulatedS() >= (m_fStartTime + m_fLifeTime))
	{
		Detonate(NULL);
	}

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	if( g_pWeaponDB->GetBool( hAmmoData, WDB_AMMO_bHeatSeeking ))
	{
		UpdateHeatSeeking( );
	}
}

// Structure to sort characters by.
struct ObjectDist
{
	GameBase*	m_pTarget;
	float		m_fDist;
	bool		m_bCharacter;

	bool operator()( ObjectDist const& a, ObjectDist const& b) const
	{
		// Make character's earlier in list than non-characters.
		if( a.m_bCharacter && !b.m_bCharacter )
			return true;
		if( !a.m_bCharacter && b.m_bCharacter )
			return false;

		// a should be considered "larger" than b if it's distance
		// is larger so that it goes at the end of the list.
		return ( a.m_fDist < b.m_fDist );
	}
};
typedef std::vector< ObjectDist > TObjectDistList;

bool VisibilityFilterFn(HOBJECT hObj, void *pUserData)
{
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);
	if (!(dwFlags & FLAG_SOLID))
	{
		return false;
	}
	else if (IsMainWorld(hObj) || (OT_WORLDMODEL == GetObjectType(hObj)))
	{
		return true;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::UpdateHeatSeeking
//
//	PURPOSE:	Updates the heat seeking of projectile
//
// ----------------------------------------------------------------------- //
void CProjectile::UpdateHeatSeeking( )
{
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	AmmoType eAmmoType  = (AmmoType)g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nType );
	if( eAmmoType != PROJECTILE )
		return;

	// Get our position and orientation for reference.
	LTVector vProjPos;
	LTRotation rotProj;
	g_pLTServer->GetObjectPos( m_hObject, &vProjPos );
	g_pLTServer->GetObjectRotation( m_hObject, &rotProj );
	LTVector vForward = rotProj.Forward( );

	TObjectDistList lstObjectDists;
	ObjectDist objectDist;
	LTVector vTargetPos;

	// Precalc some stuff for the main loop.
	float fHeatSeekingRangeSqr = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fHeatSeekingRange );
	fHeatSeekingRangeSqr *= fHeatSeekingRangeSqr;
	float fMaxAngle = MATH_DEGREES_TO_RADIANS( g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fHeatSeekingAngle ));

	CCharacter* pCharFiredFrom = dynamic_cast< CCharacter* >( g_pLTServer->HandleToObject( m_Shared.m_hFiredFrom ));

	CCharacter::CharacterList const& charList = CCharacter::GetCharacterList( );
	lstObjectDists.reserve( charList.size( ));

	// Iterate thru all the characters and see if we should add them
	// to our target list.
	CCharacter::CharacterList::const_iterator iter = charList.begin( );
	for( ; iter != charList.end( ); iter++ )
	{
		CCharacter* pChar = const_cast< CCharacter* >( *iter );

		// Don't seek ourselves.
		if( pCharFiredFrom == pChar )
			continue;

		// Ignore if dead.
		if( !pChar->IsAlive( ))
			continue;


		// Never target teammates.
		if( IsMultiplayerGameServer( ))
		{
			if( IsMyTeam( pChar->m_hObject ))
			{
				continue;
			}
		}

		// Check if we should be targeting this guy.
		if( !CanCharacterHitCharacter( this, pChar->m_hObject ))
			continue;

		// Find the distance to the object.
		g_pLTServer->GetObjectPos( pChar->GetHitBox(), &vTargetPos );
		objectDist.m_fDist = vTargetPos.DistSqr( vProjPos );

		// If out of range, skip it.
		if( objectDist.m_fDist > fHeatSeekingRangeSqr )
			continue;

		// If the object is too far off to one side, skip it.
		LTVector vDiff = vTargetPos - vProjPos;
		vDiff.Normalize( );
		float fAngle = ( float )acos( vDiff.Dot( vForward ));
		if( fAngle > fMaxAngle )
			continue;

		// Add him to the list.
		objectDist.m_pTarget = (GameBase*)g_pLTServer->HandleToObject(pChar->GetHitBox());
		objectDist.m_bCharacter = true;
		lstObjectDists.push_back( objectDist );
	}

	// Check if we have no targets.
	if( lstObjectDists.empty( ))
		return;

	// Sort them so we can choose the closest one.
	std::sort( lstObjectDists.begin( ), lstObjectDists.end( ), ObjectDist( ));

	// Setup the intersect segment information for call within the loop.
	IntersectInfo iInfo;
	IntersectQuery qInfo;
	qInfo.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
	qInfo.m_From	  = vProjPos;
	qInfo.m_FilterFn  = VisibilityFilterFn;

	float fHeatSeekingRateOfTurn = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fHeatSeekingRateOfTurn );


	// Will get filled in with final target.
	GameBase* pTarget = NULL;

	TObjectDistList::iterator objectDistIter = lstObjectDists.begin( );
	for( ; objectDistIter != lstObjectDists.end( ); objectDistIter++ )
	{
		GameBase* pObject = ( *( objectDistIter )).m_pTarget;

		g_pLTServer->GetObjectPos( pObject->m_hObject, &vTargetPos );

		// Check if we can really see the character.
		qInfo.m_To		  = vTargetPos;
		qInfo.m_pUserData = pObject->m_hObject;

		// Can't see him, go to the next closest.
		if( g_pLTServer->IntersectSegment( qInfo, &iInfo ))
			continue;

		pTarget = pObject;
		break;
	}

	// Check if we couldn't find a target to hit.
	if( !pTarget )
		return;

	// Get the direction to the target.
	LTVector vPos;
	g_pLTServer->GetObjectPos( pTarget->m_hObject, &vTargetPos );
	LTVector vDiff = vTargetPos - vProjPos;
	vDiff.Normalize( );
	float fDiffAngle = ( float )acos( vDiff.Dot( vForward ));

	// Get the maxium we can turn this frame.
		float fMaxTurn = MATH_DEGREES_TO_RADIANS( fHeatSeekingRateOfTurn ) * g_pLTServer->GetFrameTime( );

	// Turn it into a percent of the rotation we'll apply.
	float fPercent = Clamp( fMaxTurn / fDiffAngle, 0.0f, 1.0f );

	// Get the rotation to turn toward the target.
	LTRotation rTurn( vDiff, LTVector( 0.0f, 1.0f, 0.0f ));
	
	// Clamp the rotation.
	LTRotation rMaxTurn;
	rMaxTurn.Slerp( rotProj, rTurn, fPercent );
	m_vDir = rMaxTurn.Forward( );

	// Set our new rotation.
	g_pLTServer->SetObjectRotation( m_hObject, rMaxTurn );
	LTVector vVelocity;
	g_pPhysicsLT->GetVelocity( m_hObject, &vVelocity );
	float fSpeed = vVelocity.Mag( );
	vVelocity = m_vDir * fSpeed;
	g_pPhysicsLT->SetVelocity( m_hObject, vVelocity );

	// We need to update our firepos, because this is used to determine where the projectile
	// has come from for hit detection.
	m_vFirePos = vProjPos - ( m_vDir * 1000.0f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleTouch()
//
//	PURPOSE:	Handle touch notify message
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleTouch(HOBJECT hObj)
{
	if (!m_bSetup) return;
	if (m_bObjectRemoved) return;

	// Don't process any touches until this has been cleared...
	if (m_bProcessInvImpact) return;

	 // Let it get out of our bounding box...
	if( (hObj == m_Shared.m_hFiredFrom) && !m_bCanTouchFiredFromObj ) return;

	// If we've hit a character (or body), let its hit box take control...
	if (IsCharacter(hObj))
	{
		// convert the object from a characeter to its hitbox
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(hObj);
		if (pChar)
		{
			hObj = pChar->GetHitBox();
		}
	}


	// See if we want to impact on this object...
    uint32 dwUsrFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUsrFlags);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	// get the hitbox
	CCharacterHitBox* pHitBox = NULL;
	if (IsCharacterHitBox(hObj))
	{
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (!pHitBox) return;

		// can't hit ourselves
		if( (pHitBox->GetModelObject() == m_Shared.m_hFiredFrom) && !m_bCanTouchFiredFromObj ) return;
	}

	
	// Handle special AI alignment cases and multiplayer friendly fire...

	if( !CanCharacterHitCharacter( this, hObj ))
		return;


	// can we hit projectiles of the same kind?
	if ( false == m_bCanHitSameProjectileKind )
	{
		// Case where we don't hit our own type of projectiles 
		// (for multi-projectile weapons and projectiles 
		// that stick to objects)...
		if (IsKindOf(hObj, "CProjectile"))
		{
			CProjectile* pObj = (CProjectile*)g_pLTServer->HandleToObject(hObj);
			if ( !pObj->m_bSetup ) return;
			if ( pObj )
			{
				if (pObj->GetFiredFrom() == m_Shared.m_hFiredFrom)
				{
					return;
				}
			}
		}
	}

	bool bIsWorld = IsMainWorld(hObj);

	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);
	if (!bIsWorld && !(dwFlags & FLAG_SOLID))
	{
		if (pHitBox)
		{
			// AI can always hit the player in singleplayer.  You don't
			// need hit detection.
			bool bHitSomething = false;
			ModelsDB::HNODE hModelNode = NULL;
			if (!IsMultiplayerGameServer() && IsPlayer(pHitBox->GetModelObject()))
			{
				bHitSomething =  true;
			}
			else
			{
				hModelNode = g_pModelsDB->GetSkeletonNodeAlongPath( pHitBox->GetModelObject( ), pHitBox->GetSkeleton( ), m_vFirePos, m_vDir );
				if ( hModelNode )
				{
					bHitSomething =  true;
				}
			}

			// See if we really impacted on the box...
			if (bHitSomething)
			{
				// This is the object that we really hit...
				hObj = pHitBox->GetModelObject();

  				if (hModelNode )
  				{
  					pHitBox->SetModelNodeLastHit(hModelNode);
  					if( IsCharacter( hObj ))
  					{
						AdjustDamage( pHitBox->GetDamageModifier( hModelNode ));
  					}
  				}
			}
			else
			{
				return;
			}
		}
		else if (!(dwFlags & FLAG_RAYHIT))
		{
			// If we have ray hit set to true, projectiles should
			// impact on us too...
			return;
		}
	}

	// we hit the world, get details
	if (bIsWorld || (OT_WORLDMODEL == GetObjectType(hObj)))
	{
		CollisionInfo info;
		g_pLTServer->GetLastCollision(&info);

		// determine the surface type we hit
		SurfaceType eType = GetSurfaceType(info);

		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
		HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));

		if (eType == ST_SKY)
		{
			// we hit the sky
			HandleTouchSky( hObj );
			return;
		}
		else if (eType == ST_INVISIBLE)
		{
			// Ignore the impact if it's non-solid
			uint32 nObjFlags;
			g_pCommonLT->GetObjectFlags(info.m_hObject, OFT_Flags, nObjFlags);
			if ((nObjFlags & FLAG_SOLID) == 0)
			{
				return;
			}

			// we hit an invisible object
			// we have to handle this is the update function
			
			// set the flag so we know to process it later
			m_bProcessInvImpact = true;

			// prepare the info we need to process it
			g_pLTServer->GetObjectPos(m_hObject, &m_vInvisNewPos);
			g_pPhysicsLT->GetVelocity(m_hObject, &m_vInvisVel);
			m_vInvisNewPos += (m_vInvisVel * g_pLTServer->GetFrameTime());

			// Make sure this new position is inside the world
			if (LT_INSIDE == g_pCommonLT->GetPointStatus(&m_vInvisNewPos))
			{
				return;
			}
		}
	}

	// we hit something
	HandleImpact(hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleTouchSky()
//
//	PURPOSE:	Handle cases where projectile touches the sky
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleTouchSky(HOBJECT hObj)
{
	// just quietly get rid of the projectile
	RemoveObject();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleImpact()
//
//	PURPOSE:	Allow sub-classes to handle impacts...Default is to
//				go boom.
//
// ----------------------------------------------------------------------- //

void CProjectile::HandleImpact(HOBJECT hObj)
{
	Detonate(hObj);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Detonate()
//
//	PURPOSE:	Handle blowing up the projectile
//
// ----------------------------------------------------------------------- //

void CProjectile::Detonate(HOBJECT hObj)
{
	if (m_bDetonated) return;

	//DANO: a better place for this?
	// Make sure we don't detonate if a cinematic is playing (i.e.,
	// make sure the user doesn't disrupt the cinematic)...
	if (Camera::IsActive())
	{
		RemoveObject();
		return;
	}

	// mark the projectile as detonated
	m_bDetonated = true;

	// eventually we will determine the surface type
	SurfaceType eType = ST_UNKNOWN;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Determine the normal of the surface we are impacting on...
	LTVector vNormal = m_vFirePos - vPos;
	float fMag = vNormal.Mag( );
	if( fMag > 0.0f )
		vNormal /= fMag;
	else
		vNormal = -m_vDir;

	// if hObj in non-null, it is detonating on something
	if (hObj)
	{
		// projectile is detonating against something
		if (IsMainWorld(hObj) || GetObjectType(hObj) == OT_WORLDMODEL)
		{
			// get the collision info for this impact
			CollisionInfo info;
			g_pLTServer->GetLastCollision(&info);

			// check if we have a valid polygon
			if (info.m_hPoly != INVALID_HPOLY)
			{
				// get the surface type
				eType = GetSurfaceType(info.m_hPoly);
			}

			// get the normal of the plane we impacted with
			LTPlane plane = info.m_Plane;
			if (plane.m_Normal.MagSqr() == 0.0f)
			{
				LTERROR("CProjectile::Detonate() - Invalid impact plane");
			}
			else
			{
				vNormal = plane.m_Normal;
				//
				// Calculate where we really hit the plane
				// and make sure we don't tunnel through an object
				//

				LTVector vVel, vP0, vP1, vDir;
				// get the velocity of the projectile
				g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

				// get the direction of the projectile
				vDir = vVel;
				vDir.Normalize();

				// determine how much we've travelled this frame
				vVel *= g_pLTServer->GetFrameTime();

				// get a point just a little in front and behind the impact point
				vP0 = vPos - vVel;  // a little "behind" of the impact point
				vP1 = vPos + vVel;  // a littel "forward" of the impact point

				// throw an intersect segment to determine where we really hit
				IntersectInfo iInfo;
				IntersectQuery qInfo;

				// fill out the info for this test
				qInfo.m_Flags = INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID;
				qInfo.m_From	  = vP0;
				qInfo.m_To		  = vPos;
				qInfo.m_FilterFn  = SpecificObjectFilterFn;
				qInfo.m_pUserData = m_hObject;

				if (g_pLTServer->IntersectSegment(qInfo, &iInfo))
				{
					// we did hit the plane
					
					// get the intersect information
					vPos    = iInfo.m_Point - vDir;
					eType   = GetSurfaceType(iInfo);
					vNormal = iInfo.m_Plane.m_Normal;
				}
				else
				{
					// plane was NOT hit

					// fake the impact position
					float fDot1 = vNormal.Dot(vP0) - info.m_Plane.m_Dist;
					float fDot2 = vNormal.Dot(vP1) - info.m_Plane.m_Dist;

					if ( ( ( fDot1 < 0.0f ) && ( fDot2 < 0.0f ) ) ||
						( ( fDot1 > 0.0f ) && ( fDot2 > 0.0f ) ) )
					{
						vPos = vP1;
					}
					else
					{
						float fPercent = -fDot1 / (fDot2 - fDot1);
						vPos = vP0.Lerp(vP1, fPercent);
					}
				}

				// reset the projectile's rotation
				LTRotation rRot(vNormal, LTVector(0.0f, 1.0f, 0.0f));
				g_pLTServer->SetObjectRotation(m_hObject, rRot);
			}
		}
	}
	else
	{
		// Since hObj was null, this means the projectile's lifetime was up,
		// so we just blew-up in the air.
		eType = ST_AIR;
	}

	// if the surface type has not been determined, try one more time
	if (eType == ST_UNKNOWN)
	{
		// get the object's surface type
		eType = GetSurfaceType(hObj);
	}

	// do all the impact work
	AddImpact(hObj, m_vFlashPos, vPos, vNormal, eType);

	// Handle impact damage...
	if (hObj)
	{
		HOBJECT hDamager = m_Shared.m_hFiredFrom ? (HOBJECT)m_Shared.m_hFiredFrom : m_hObject;

		ImpactDamageObject(hDamager, hObj, m_vFlashPos, vPos, m_vDir);
	}

	// Remove projectile from world...
	RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddImpact()
//
//	PURPOSE:	Add an impact object.
//
//	USED:		both vectors and projectiles
//
// ----------------------------------------------------------------------- //

void CProjectile::AddImpact(	HOBJECT hObj,
								const LTVector &vFirePos,
								const LTVector &vImpactPos,
								const LTVector &vSurfaceNormal,
								SurfaceType eType,
								HMODELNODE hNodeHit )
{
	if ( !m_bSetup )
	{
		return;
	}

	// validate the pointers we will use
	ASSERT( 0 != g_pLTServer );
	ASSERT( 0 != g_pWeaponDB );
	ASSERT( 0 != g_pAIStimulusMgr );
	ASSERT( 0 != m_Shared.m_hWeapon );
	ASSERT( 0 != m_Shared.m_hAmmo );

	// Create the client side (impact) weapon fx...
	CLIENTWEAPONFX fxStruct;
	fxStruct.hFiredFrom			= m_Shared.m_hFiredFrom;
	fxStruct.vSurfaceNormal		= vSurfaceNormal;
	LTASSERT( !hObj || fxStruct.vSurfaceNormal.MagSqr() > 0, "Invalid surface normal specified." );
	fxStruct.vFirePos			= vFirePos;
	fxStruct.vPos				= vImpactPos + (m_vDir * -1.0f);
	fxStruct.hObj				= hObj;
	fxStruct.hWeapon			= m_Shared.m_hWeapon;
	fxStruct.bSendWeaponRecord	= true; //for impacts we always want to send the weapon record, because the client may have changed weapons
	fxStruct.hAmmo				= m_Shared.m_hAmmo;
	fxStruct.bSendAmmoRecord	= true; //for impacts we always want to send the ammo record, because the client may have changed weapons
	fxStruct.nSurfaceType		= eType;
	fxStruct.wIgnoreFX			= g_wIgnoreFX;
	fxStruct.hNodeHit			= hNodeHit;

	// Always use the flash position for the first call to AddImpact as it
	// can be called multiple times per vector so we want to make sure the
	// first call has the correct position...
	if (m_bNumCallsToAddImpact == 0)
	{
		fxStruct.vFirePos = m_vFlashPos;
	}

	// By default FX play at the position of the weapon's flash socket.
	// In some cases, we want the FX at some other socket (e.g. the character's LEFTHAND).
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_Shared.m_hWeapon, IsAI(m_Shared.m_hFiredFrom));
	fxStruct.bFXAtFlashSocket = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bFXAtFlashSocket );

	// If we do multiple calls to AddImpact, make sure we only do some
	// effects once :)
	g_wIgnoreFX |= WFX_SHELL | WFX_MUZZLE | WFX_FIRESOUND | WFX_ALTFIRESND | WFX_SILENCED;

	// Allow exit surface fx on the next call to AddImpact...
	g_wIgnoreFX &= ~WFX_EXITMARK;

	// limit the amount of exit marks
	if (IsMoveable(hObj))
	{
		// Well, don't do too many exit marks...The server will add one
		// if necessary...

		g_wIgnoreFX |= WFX_EXITMARK;
	}

	// If this is a player object, get the client id...
	if (IsPlayer(m_Shared.m_hFiredFrom))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_Shared.m_hFiredFrom);
		if (pPlayer)
		{
			fxStruct.nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}

	// do all server side fx activities
	// and send a message to the client
	CreateClientWeaponFX( fxStruct );

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	float	fAreaDamage = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamage );
	float	fProgDamageLifetime = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamageLifetime );

	// Do the area and progressive (over time) damage...
	if ((fAreaDamage > 0.0f && eType != ST_SKY) || fProgDamageLifetime > 0.0f)
	{
		AddExplosion(vImpactPos, vSurfaceNormal);
	}

	// if the projectile was fired from a character
	if(m_Shared.m_hFiredFrom && IsCharacter(m_Shared.m_hFiredFrom))
	{
		//
		// send a bunch of AI stimuli
		//

		if( !m_Shared.m_hWeapon || !m_Shared.m_hAmmo )
			return;

		// Register AI stimuli associated with an impact.
		RegisterImpactAIStimuli( m_Shared.m_hAmmo, 
			m_Shared.m_hFiredFrom, hObj,
			vImpactPos, m_vDir,
			eType );
	}

	// keep track of the number of times AddImpact is called
	m_bNumCallsToAddImpact++;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddExplosion()
//
//	PURPOSE:	Add an explosion
//
// ----------------------------------------------------------------------- //

void CProjectile::AddExplosion(const LTVector &vPos, const LTVector &vNormal)
{
	ASSERT( 0 != g_pLTServer );

	//
	// create an explosion object
	//

	// get the class
	HCLASS hClass = g_pLTServer->GetClass("Explosion");
	if (!hClass) return;

	// get an Object Create Struct
	ObjectCreateStruct theStruct;

	// set the position
	theStruct.m_Pos = vPos;

	// set the rotation to the normal
	theStruct.m_Rotation = LTRotation(vNormal, vNormal);

	// create the object
	Explosion* pExplosion = (Explosion*)g_pLTServer->CreateObject(hClass, &theStruct);

	// validate its creation
	if (pExplosion)
	{
		// setup the explosion
		pExplosion->Setup( m_Shared.m_hFiredFrom, m_Shared.m_hAmmo, IsAI( m_Shared.m_hFiredFrom ));
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AddSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //

void CProjectile::AddSpecialFX()
{
	ASSERT( 0 != g_pLTServer );

	// If this is a player object, get the client id...
	uint8 nShooterId = INVALID_TEAM; 
	if (IsPlayer(m_Shared.m_hFiredFrom))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_Shared.m_hFiredFrom);
		if (pPlayer)
		{
			nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}

	// Create a special fx...
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_PROJECTILE_ID);
	m_Shared.Write( cMsg );

	LTRESULT ltResult;
	ltResult = g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	ASSERT( LT_OK == ltResult );


	// Create a sound for firing the projectile...
	// NOTE: This is a PlayerSoundFX but this is used by AI as well.
	//		 Should really be a CharacterSoundFX anyways...

	CAutoMessage cSoundMsg;
	cSoundMsg.Writeuint8(SFX_PLAYERSOUND_ID);
	cSoundMsg.Writeuint8(PSI_FIRE);
	cSoundMsg.WriteDatabaseRecord(g_pLTDatabase, m_Shared.m_hWeapon);
	cSoundMsg.Writeuint8(nShooterId);
	cSoundMsg.WriteCompPos(m_vFirePos);
	g_pLTServer->SendSFXMessage(cSoundMsg.Read(), 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::OverrideClientFX()
//
//	PURPOSE:	Override the default ClientFX
//
// ----------------------------------------------------------------------- //

void CProjectile::OverrideClientFX(const char* szEffect,const char* szSameTeamEffect,const char* szOtherTeamEffect)
{
	ASSERT( 0 != g_pLTServer );

	m_Shared.m_sOverrideFX	= szEffect;
	m_Shared.m_sSameTeamFX	= szSameTeamEffect;
	m_Shared.m_sOtherTeamFX	= szOtherTeamEffect;

	// Update the SFX message for new players...
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_PROJECTILE_ID);
	m_Shared.Write( cMsg );

	LTRESULT ltResult;
	ltResult = g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	ASSERT( LT_OK == ltResult );

	// Update FX already playing...
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PROJECTILE_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_CLIENTFX );
	cMsg.WriteString( m_Shared.m_sOverrideFX.c_str( ) );
	cMsg.WriteString( m_Shared.m_sSameTeamFX.c_str( ) );
	cMsg.WriteString( m_Shared.m_sOtherTeamFX.c_str( ) );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::SetRecoverable()
//
//	PURPOSE:	Set whether the projectile is recoverable
//
// ----------------------------------------------------------------------- //

void CProjectile::SetRecoverable(bool bRecoverable)
{
	ASSERT( 0 != g_pLTServer );

	m_bRecoverable	= bRecoverable;

	// Update FX already playing...
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PROJECTILE_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_RECOVERABLE);
	cMsg.Writebool( m_bRecoverable );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::RemoveObject()
//
//	PURPOSE:	Remove the object, and do clean up (isle 4)
//
//	USED:		both vectors and projectiles
//
// ----------------------------------------------------------------------- //

void CProjectile::RemoveObject()
{
	if (m_bObjectRemoved) return;

	// NOTE: the actual removal doesn't happen until the
	// end of the frame
	g_pLTServer->RemoveObject(m_hObject);

	// make note that its been removed so it doesn't do anything silly
	m_bObjectRemoved = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoVectorFilterFn()
//
//	PURPOSE:	Filter the attacker out of IntersectSegment
//				calls (so you don't shot yourself).  Also handle
//				AIs of the same alignment not shooting eachother
//
//  SPECIAL NOTES: pUserData must be a NULL terminated HOBJECT array and 
//				   the first object in the array MUST be m_hFiredFrom!!!!!
//
// ----------------------------------------------------------------------- //

struct VectorFilterFnUserData
{
	CProjectile* m_pProjectile;
	HOBJECT* m_pFilterList;
};

bool DoVectorFilterFn(HOBJECT hObj, void *pUserData)
{
	HCLASS hCharacter		= g_pLTServer->GetClass( "CCharacter" );
	HCLASS hCharacterHitBox = g_pLTServer->GetClass( "CCharacterHitBox" );
	HCLASS hPickupItem		= g_pLTServer->GetClass( "PickupItem" );

	HCLASS hObjClass = g_pLTServer->GetObjectClass( hObj );

	// Filter out the specified objects...
	VectorFilterFnUserData* pVectFilterFnUserData = ( VectorFilterFnUserData* )pUserData;
	if (ObjListFilterFn(hObj, pVectFilterFnUserData->m_pFilterList))
	{
		// CharacterHitBox objects are used for vector impacts, don't
		// impact on the character or pickupitems...

		if( g_pLTServer->IsKindOf(hObjClass, hCharacter) || 
			g_pLTServer->IsKindOf(hObjClass, hPickupItem) )
		{
			return false;
		}

		// Check special character hit box cases...

		if( g_pLTServer->IsKindOf( hObjClass, hCharacterHitBox ))
		{
            CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(hObj);
			if (pCharHitBox)
			{
				// NOTE: The first object in the filter list MUST be m_hFiredFrom!!!!!

				HOBJECT hUs = pVectFilterFnUserData->m_pFilterList[0];

				HOBJECT hTestObj = pCharHitBox->GetModelObject();
                if (!hTestObj) return false;

				if (hTestObj == hUs)
				{
                    return false;
				}


				return CanCharacterHitCharacter( pVectFilterFnUserData->m_pProjectile, hTestObj );
			}
		}

        return true;
	}

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CheckCharacterAllignment
//
//	PURPOSE:	Checks the allignment of the characters to see if a projectile
//				from one character is allowed to impact on the other character.
//
// ----------------------------------------------------------------------- //

bool CanCharacterHitCharacter( CProjectile* pProjectile, HOBJECT hImpacted )
{
	HOBJECT hFiredFrom = pProjectile->GetFiredFrom( );

	// If we get hitboxes get the character objects...
	if( IsCharacterHitBox( hFiredFrom ))
	{
		CCharacterHitBox *pCharHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject( hFiredFrom ));
		if( !pCharHitBox )
			return true;

		hFiredFrom = pCharHitBox->GetModelObject();
	}

	if( IsCharacterHitBox( hImpacted ))
	{
		CCharacterHitBox *pCharHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject( hImpacted ));
		if( !pCharHitBox )
			return true;

		hImpacted = pCharHitBox->GetModelObject();
	}

	// Do special AI hitting AI case...
	if (IsAI(hFiredFrom) && IsAI(hImpacted))
	{
		CAI *pAI = (CAI*) g_pLTServer->HandleToObject(hFiredFrom);
		if (!pAI)
		{
			return false;
		}

		CCharacter* pB = (CCharacter*)g_pLTServer->HandleToObject(hImpacted);
		if (!pB)
		{
			return false;
		}

		// We can't hit guys we like, unless they're NEUTRAL to us

		EnumCharacterStance eStance = g_pCharacterDB->GetStance( pAI->GetAlignment(), pB->GetAlignment() );
		if( eStance != kCharStance_Tolerate )
		{
			// If they are NOT neutral to us, then find out if we like or
			// dislike them.
			// Return true if we don't like them, false if we do.

			return kCharStance_Like != eStance;
		}
	}

	// Check for friendly fire
	if( IsMultiplayerGameServer( ))
	{
		if( !GameModeMgr::Instance( ).m_grbFriendlyFire )
		{
			if( pProjectile->IsMyTeam( hImpacted ))
			{
				return false;
			}
		}
	}

	// Player-AI, AI-Player, and Player-Player with friendly fire on...

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoProjectile
//
//	PURPOSE:	Do projectile stuff...
//
//	USED:		only for projectiles
//
// ----------------------------------------------------------------------- //

void CProjectile::DoProjectile()
{ 
	if( !m_Shared.m_hAmmo ) 
		return;

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
	if( !hProjectileFX )
		return;

	// validate the pointers we will use
	ASSERT( 0 != g_pCommonLT );
	ASSERT( 0 != g_pPhysicsLT );

	// Set up the model...

	ObjectCreateStruct createStruct;
	createStruct.Clear();

	createStruct.m_eGroup = GetPhysicsGroup( );
	createStruct.SetFileName( g_pFXDB->GetString(hProjectileFX,FXDB_sModel));
	createStruct.SetMaterial(0, g_pFXDB->GetString(hProjectileFX,FXDB_sMaterial));

    g_pCommonLT->SetObjectFilenames(m_hObject, &createStruct);
    g_pLTServer->SetObjectScale(m_hObject, g_pFXDB->GetFloat(hProjectileFX,FXDB_fModelScale));

	// Set the dims based on the current animation...

    LTVector vDims;
    g_pModelLT->GetModelAnimUserDims(m_hObject, g_pLTServer->GetModelAnimation(m_hObject), &vDims);

	// Set object dims based on scale value...

    LTVector vNewDims = vDims * g_pFXDB->GetFloat(hProjectileFX,FXDB_fModelScale);
   	g_pPhysicsLT->SetObjectDims(m_hObject, &vNewDims, 0);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

	// Set the can't damage flags from the damage mask...
	HRECORD hDamageMask = g_pFXDB->GetRecordLink( hProjectileFX, FXDB_sDamageMask );
	if( hDamageMask )
	{
		DamageFlags df = g_pDTDB->GetDamageMaskFlags( hDamageMask );
		m_damage.SetCantDamageFlags( (df == 0 ? 0 : ~df) );
	}

	//setup our collision properties
	HRECORD hCollisionProperty = g_pFXDB->GetRecordLink( hProjectileFX, FXDB_rCollisionProperty );
	uint32 nUserFlags = CollisionPropertyRecordToUserFlag( hCollisionProperty );
	g_pLTServer->Common( )->SetObjectFlags( m_hObject, OFT_User, nUserFlags, USRFLG_COLLISIONPROPMASK );

	// Take us out of the physics simulation, we'll use our own rigid body
	PhysicsUtilities::SetPhysicsWeightSet(m_hObject, PhysicsUtilities::WEIGHTSET_NONE, false);

	// Register stimulus if one exists.

	HRECORD hLink = g_pFXDB->GetRecordLink( hProjectileFX, FXDB_sStimulus );
	if( hLink )
	{
		const char* pszStimulus = g_pLTDatabase->GetRecordName( hLink );
		EnumAIStimulusType eStimulus = (EnumAIStimulusType)g_pAIDB->String2BitFlag( pszStimulus, kStim_Count, s_aszStimulusTypes );

		float fRadius = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamageRadius );

		// Get the position of the firing object.

		LTVector vPos;
	   	g_pLTServer->GetObjectPos(m_Shared.m_hFiredFrom, &vPos);

		// Get the Alignment.  This assumes the source is a character.  If it 
		// is not, the alignment needs to be handled differently.

		EnumCharacterAlignment eAlignment = kCharAlignment_Invalid;
		if ( IsCharacter( m_Shared.m_hFiredFrom ) )
		{
			CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_Shared.m_hFiredFrom);
			if ( pChar )
			{
				eAlignment = pChar->GetAlignment();
			}
		}

		// Register the stimulus.

		StimulusRecordCreateStruct scs( eStimulus, eAlignment, vPos, m_Shared.m_hFiredFrom);
		scs.m_hStimulusTarget = m_hObject;
		scs.m_dwDynamicPosFlags |= CAIStimulusRecord::kDynamicPos_TrackTarget;
		scs.m_flRadiusScalar = fRadius;
		scs.m_flDurationScalar = 0.f;
		g_pAIStimulusMgr->RegisterStimulus( scs );
	}

	// Start your engines...

	m_fStartTime = SimulationTimer::Instance().GetTimerAccumulatedS();


	// Make the flash position the same as the fire position...

	m_vFlashPos	= m_vFirePos;


	// Set our force ignore limit and mass...

	g_pLTServer->SetBlockingPriority(m_hObject, 0);
	g_pPhysicsLT->SetForceIgnoreLimit(m_hObject, 0.0f);
	g_pPhysicsLT->SetMass(m_hObject, m_fMass);


	// Make sure we are pointing in the direction we are traveling...

	LTVector	vU, vR;

	if( (1.0f == m_vDir.y) || (-1.0f == m_vDir.y) )
	{
		vR = m_vDir.Cross( LTVector( 1.0f, 0.0f, 0.0f ));
	}
	else
	{
		vR = m_vDir.Cross( LTVector( 0.0f, 1.0f, 0.0f ));
	}

	vU = vR.Cross( m_vDir );
	vU.Normalize();
	
    LTRotation rRot(m_vDir, vU );
	g_pLTServer->SetObjectRotation(m_hObject, rRot);


	// Make sure we have the correct flags set...
	uint32 dwFlags = g_pFXDB->GetProjectileObjectFlags(hProjectileFX);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, dwFlags);


	// And away we go...

    LTVector vVel;
	vVel = m_vDir * m_fVelocity;
	g_pPhysicsLT->SetVelocity(m_hObject, vVel);


	// Special case of 0 life time...

	if ( LTNearlyEquals(m_fLifeTime,0.0f,0.01f) )
	{
        Detonate(NULL);
	}
	else
	{
		// Adds an initial special FX for this projectile?
		AddSpecialFX();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::DoVector
//
//	PURPOSE:	Do vector stuff
//
//	USED:		only for vectors
//
// ----------------------------------------------------------------------- //

void CProjectile::DoVector( WeaponFireInfo const & info )
{
	if( info.pWeaponPath )
	{
		// Add all objects associated with the character to the ignore list...
		if( IsCharacter( m_Shared.m_hFiredFrom ))
		{
			CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_Shared.m_hFiredFrom);
			if( pChar && pChar->GetAttachments( ))
			{
				ObjectList* pObjectList = g_pLTServer->CreateObjectList();
				if ( pObjectList )
				{
					pChar->GetAttachments()->AddToObjectList( pObjectList );

					ObjectLink* pLink = pObjectList->m_pFirstLink;
					for ( ; pLink != NULL; pLink = pLink->m_pNext )
					{
						info.pWeaponPath->IgnoreObject( pLink->m_hObject );
					}

					g_pLTServer->RelinquishList( pObjectList );
				}
			}
		}

		// Need to reset the damage each time DoVector is called otherwise multi vector
		// weapons won't do the appropriate amount of damage...
		info.pWeaponPath->m_fInstDamage	= m_fInstDamage;

		// Setup the impact and character hit callbacks...
		info.pWeaponPath->m_fnOnCharNodeHitFn = WeaponPath_OnCharNodeHitCB;
		info.pWeaponPath->m_fnOnImpactCB = WeaponPath_OnImpactCB;
		info.pWeaponPath->m_pImpactCBUserData = this;
		
		// Only do our own intersect segment in multiplayer games...
		if( IsMultiplayerGameServer( ))
			info.pWeaponPath->m_fnIntersectSegment = WeaponPath_IntersectSegment;

		// Perturb the weapon path each time before DoVector is called.
		// This needs to be consistent with the client so if one changes the other must as well...
		info.pWeaponPath->PerturbWeaponPath( IsAI(m_Shared.m_hFiredFrom) );

		info.pWeaponPath->DoVector( );

		// After each DoVector call the ignore list must be cleared out...
		info.pWeaponPath->ClearIgnoreList( );
	}

	// Okay, we're all done now...bye, bye...
	RemoveObject( );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ImpactDamageObject
//
//	PURPOSE:	Handle impact damage to an object
//
// ----------------------------------------------------------------------- //

void CProjectile::ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj, const LTVector& vFirePos,
									 const LTVector& vImpactPos, const LTVector &vDirection )
{
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo, IsAI(m_Shared.m_hFiredFrom));

	//Apply a physical force to the object that was hit...
	float fImpulse = g_pWeaponDB->GetFloat(hAmmoData, WDB_AMMO_fInstDamageImpulseForce);

#ifndef _FINAL
	// In non-final builds check and see if someone is tweaking the bullet force...
	if (g_vtPhysicsBulletForce.GetFloat(PROJECTILE_DEFAULT_BULLET_FORCE) != PROJECTILE_DEFAULT_BULLET_FORCE)
	{
		fImpulse = g_vtPhysicsBulletForce.GetFloat();
	}
#endif // _FINAL

	// Adjust the amount of force applied based on distance away...
	float fDist = (vImpactPos - vFirePos).Mag( );
	float fDamageFactor = g_pWeaponDB->GetEffectiveVectorRangeDamageFactor(m_Shared.m_hWeapon, fDist, IsAI(m_Shared.m_hFiredFrom));
	
// g_pLTServer->CPrint("ImpactDamageObject() fDamageFactor = %.4f", fDamageFactor);

	fImpulse *= fDamageFactor;

	PhysicsUtilities::ApplyPhysicsImpulseForce(hObj, fImpulse, vDirection, vImpactPos, false);

	DamageStruct damage;
	damage.hDamager		 = hDamager;
	damage.hAmmo		 = m_Shared.m_hAmmo;
	damage.fImpulseForce = fImpulse;
	damage.SetPositionalInfo(vImpactPos, vDirection);

	// Do Instant damage...
	if ( m_eInstDamageType != DT_UNSPECIFIED )
	{
		damage.eType	= m_eInstDamageType;
		damage.fDamage	= m_fInstDamage;
		damage.fPenetration = m_fInstPenetration;

		damage.DoDamage(m_hObject, hObj);
	}


	// Do Progressive damage...(if the progressive damage is supposed to
	// happen over time, it will be done in the explosion object)....
	if ( m_eProgDamageType != DT_UNSPECIFIED && g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamageLifetime ) <= 0.0f)
	{
		damage.eType	 = m_eProgDamageType;
		damage.fDamage	 = m_fProgDamage;
		damage.fDuration = g_pWeaponDB->GetFloat( hAmmoData,WDB_AMMO_fProgDamageDuration );

		damage.DoDamage(m_hObject, hObj);
	}



	// Update player summary info...
	CPlayerObj* pPlayer;
	if (IsPlayer(hDamager) && IsCharacter(hObj) && IsAccuracyType(m_eInstDamageType))
	{
        CCharacter* pChar = (CCharacter*) g_pLTServer->HandleToObject(hObj);
        pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hDamager);
		if (pPlayer && pChar)
		{
			pPlayer->GetMissionStats()->dwNumHits++;

			ModelsDB::HNODE hModelNode = pChar->GetModelNodeLastHit();

			if( hModelNode )
			{
				HitLocation eLoc = g_pModelsDB->GetNodeLocation( hModelNode );
				pPlayer->GetMissionStats()->dwHitLocations[eLoc]++;
			}
			else
			{
				pPlayer->GetMissionStats()->dwHitLocations[HL_UNKNOWN]++;
			}
		}
	}

	if (IsPlayer(hObj))
	{
        pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(hObj);
		if (pPlayer)
		{
			pPlayer->GetMissionStats()->dwNumTimesHit++;
		}
	}

	// Send notification to the player that they damaged another player...
	if( IsMultiplayerGameServer( ) && IsPlayer( hDamager ) && IsPlayer( hObj ))
	{
		uint8 nDamagedId;
		CPlayerObj *pDamaged = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( hObj ));
		CPlayerObj *pDamager = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( hDamager ));
		
		if( pDamaged && pDamager )
		{
			nDamagedId = (uint8)g_pLTServer->GetClientID( pDamaged->GetClient( ));

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_WEAPON_DAMAGE_PLAYER );
			cMsg.Writeuint8( nDamagedId );
			g_pLTServer->SendToClient( cMsg.Read( ), pDamager->GetClient( ), 0 );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CProjectile::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	m_Shared.Write( pMsg );

	SAVE_HOBJECT(m_hFiringWeapon);
	SAVE_VECTOR(m_vFlashPos);
	SAVE_VECTOR(m_vFirePos);
	SAVE_VECTOR(m_vDir);
	SAVE_BOOL(m_bSilenced);
	SAVE_BOOL(m_bObjectRemoved);
	SAVE_BOOL(m_bDetonated);
	SAVE_BYTE(m_eInstDamageType);
	SAVE_BYTE(m_eProgDamageType);
	SAVE_FLOAT(m_fInstDamage);
	SAVE_FLOAT(m_fProgDamage);
	SAVE_TIME(m_fStartTime);
	SAVE_FLOAT(m_fLifeTime);
	SAVE_FLOAT(m_fVelocity);
	SAVE_FLOAT(m_fRange);
	SAVE_BOOL(m_bProcessInvImpact);
	SAVE_VECTOR(m_vInvisVel);
	SAVE_VECTOR(m_vInvisNewPos);
	SAVE_BOOL(m_bSetup);
	SAVE_BOOL(m_bCanHitSameProjectileKind);
	SAVE_BOOL(m_bCanTouchFiredFromObj);
	SAVE_BOOL(m_bDamagedByOwner);

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	SAVE_DWORD(dwFlags);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CProjectile::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_Shared.Read( pMsg );

	LOAD_HOBJECT(m_hFiringWeapon);
	LOAD_VECTOR(m_vFlashPos);
	LOAD_VECTOR(m_vFirePos);
	LOAD_VECTOR(m_vDir);
	LOAD_BOOL(m_bSilenced);
	LOAD_BOOL(m_bObjectRemoved);
	LOAD_BOOL(m_bDetonated);
	LOAD_BYTE_CAST(m_eInstDamageType, DamageType);
	LOAD_BYTE_CAST(m_eProgDamageType, DamageType);
	LOAD_FLOAT(m_fInstDamage);
	LOAD_FLOAT(m_fProgDamage);
	LOAD_TIME(m_fStartTime);
	LOAD_FLOAT(m_fLifeTime);
	LOAD_FLOAT(m_fVelocity);
	LOAD_FLOAT(m_fRange);
	LOAD_BOOL(m_bProcessInvImpact);
	LOAD_VECTOR(m_vInvisVel);
	LOAD_VECTOR(m_vInvisNewPos);
	LOAD_BOOL(m_bSetup);
	LOAD_BOOL(m_bCanHitSameProjectileKind);
	LOAD_BOOL(m_bCanTouchFiredFromObj);
	LOAD_BOOL(m_bDamagedByOwner);

	uint32 dwFlags;
	LOAD_DWORD(dwFlags);
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::SaveSFXMessage
//
//	PURPOSE:	Save the object special effect message
//
// ----------------------------------------------------------------------- //

void CProjectile::SaveSFXMessage( ILTMessage_Write *pMsg, uint32 dwFlags )
{
	if( !pMsg )
		return;

	CAutoMessage cGetMsg;
	g_pLTServer->GetObjectSFXMessage( m_hObject, cGetMsg );
	CLTMsgRef_Read pSFXMsg = cGetMsg.Read( );

	if( pSFXMsg->Size( ) == 0 )
		return;

	pMsg->Writeuint8( pSFXMsg->Readuint8( ) );

	PROJECTILECREATESTRUCT ProjectileCS;
	ProjectileCS.Read( pSFXMsg );
	ProjectileCS.Write( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::LoadSFXMessage
//
//	PURPOSE:	Load the object special effect message.
//
// ----------------------------------------------------------------------- //

void CProjectile::LoadSFXMessage( ILTMessage_Read *pMsg, uint32 dwFlags )
{
	if( !pMsg || pMsg->Size( ) == 0 )
		return;

	CAutoMessage cSFXMsg;

	cSFXMsg.Writeuint8( pMsg->Readuint8( ) );

	PROJECTILECREATESTRUCT ProjectileCS;
	ProjectileCS.Read( pMsg );
	ProjectileCS.Write( cSFXMsg );
	g_pLTServer->SetObjectSFXMessage( m_hObject, cSFXMsg.Read( ) );
}

bool CProjectile::IsMyTeam( HOBJECT hPlayer ) const
{
	bool bMyTeam = false;

	if (!IsMultiplayerGameServer( ))
		return false;

	// If it's a team game, then check our internal teamid.
	if( GameModeMgr::Instance( ).m_grbUseTeams)
	{
		if( GetTeamId( ) != INVALID_TEAM )
		{
			bMyTeam = ( GetTeamId( ) == GetPlayerTeamId( hPlayer ));
		}
	}
	// Every other gametype has no team allegiance.
	else
	{
		bMyTeam = false;
	}

	return bMyTeam;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::FilterDamage()
//
//	PURPOSE:	Change the damage struct before damage is dealt
//
// ----------------------------------------------------------------------- //

bool CProjectile::FilterDamage( DamageStruct *pDamageStruct )
{
	//am I being damaged by the person who launched me?
	if (pDamageStruct->hDamager == m_Shared.m_hFiredFrom && !m_bDamagedByOwner) 
	{
		return false;
	}
	return true;
}

void CProjectile::SetFiredFrom( HOBJECT hFiredFrom )
{
	m_Shared.m_hFiredFrom = hFiredFrom;
	if( !m_Shared.m_hFiredFrom )
	{
		m_delegateRemoveClient.Detach();
		m_delegatePlayerSwitched.Detach();
		return;
	}

	// If the firing object is a player, then record the gameclientdata object so we
	// can properly give the client the kill.
	CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_Shared.m_hFiredFrom ));
	if( pPlayerObj )
	{
		GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayerObj->GetClient( ));
		if( pGameClientData )
		{
			m_delegateRemoveClient.Attach( this, pGameClientData, pGameClientData->RemoveClient );
			m_delegatePlayerSwitched.Attach( this, pGameClientData, pGameClientData->PlayerSwitched );
		}
	}
}

void CProjectile::HandlePlayerChange()
{
	// Update the SFX message on the client.  Need to send a special override message in case they are 
	// waiting to read the sfx message in a polling fashion.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE_OVERRIDE );
	cMsg.WriteObject( m_hObject );
	m_Shared.Write( cMsg );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

void CProjectile::OnPlayerSwitched( CProjectile* pProjectile, GameClientData* pGameClientData, EventCaster::NotifyParams& notifyParams )
{
	// Transfer our firedfrom to the client's new player.
	pProjectile->m_Shared.m_hFiredFrom = pGameClientData->GetPlayer( );
	pProjectile->HandlePlayerChange();
}

void CProjectile::OnRemoveClient( CProjectile* pProjectile, GameClientData* pGameClientData, EventCaster::NotifyParams& notifyParams )
{
	// Clear out our fired from, since our client is gone.
	pProjectile->m_Shared.m_hFiredFrom = NULL;
	pProjectile->HandlePlayerChange();
}

bool CProjectile::WeaponPath_OnImpactCB( CWeaponPath::COnImpactCBData &rImpactData, void *pUserData )
{
	if( !pUserData )
		return false;

	CProjectile *pProjectile = reinterpret_cast<CProjectile*>(pUserData);

	// Cache the list of impact points to send to clients...
	pProjectile->m_lstImpactPoints.push_back( rImpactData.m_vImpactPos );

	// Register AI stimuli associated with an impact.
	RegisterImpactAIStimuli( pProjectile->m_Shared.m_hAmmo, 
							 rImpactData.m_hObjectFired, rImpactData.m_hObjectHit,
							 rImpactData.m_vImpactPos, rImpactData.m_vDir,
							 rImpactData.m_eSurfaceType );

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(pProjectile->m_Shared.m_hAmmo,IsAI(rImpactData.m_hObjectFired));
	float	fAreaDamage = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fAreaDamage );
	float	fProgDamageLifetime = g_pWeaponDB->GetFloat( hAmmoData, WDB_AMMO_fProgDamageLifetime );

	// Do the area and progressive (over time) damage...
	if ((fAreaDamage > 0.0f && rImpactData.m_eSurfaceType != ST_SKY) || fProgDamageLifetime > 0.0f)
	{
		pProjectile->AddExplosion(rImpactData.m_vImpactPos, rImpactData.m_vImpactNormal);
	}

	// Should the object be damaged...
	if( rImpactData.m_hObjectHit && 
		(IsMainWorld( rImpactData.m_hObjectHit ) || (GetObjectType( rImpactData.m_hObjectHit ) == OT_CONTAINER)) )
	{
		return false;
	}

	// Set the adjusted damage value...
	pProjectile->m_fInstDamage = rImpactData.m_fInstDamage;
	pProjectile->ImpactDamageObject( rImpactData.m_hObjectFired, rImpactData.m_hObjectHit,
									 rImpactData.m_vFirePos, rImpactData.m_vImpactPos, rImpactData.m_vDir );

	return true;
}

bool CProjectile::WeaponPath_OnCharNodeHitCB( HOBJECT hCharacter, ModelsDB::HNODE hModelNode, float &rfDamageModifier )
{
	if( !IsCharacter( hCharacter ))
		return false;

	CCharacter *pCharacter = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( hCharacter ));
	if( !pCharacter )
		return false;

	// Let the character know where they last got hit...
	pCharacter->SetModelNodeLastHit( hModelNode );

	// Let the character adjust the damage applied to them based on the node hit...
	rfDamageModifier = pCharacter->ComputeDamageModifier( hModelNode );

	return true;
}

bool CProjectile::WeaponPath_IntersectSegment( CWeaponPath::CIntersectSegmentData &rISData )
{
	if( g_vtUseHistoricalObjectTransforms.GetFloat( ) > 0.0f )
	{
		CObjectTransformHistoryMgr &rObjectTransformHistory = CObjectTransformHistoryMgr::Instance( );
		CWeaponPath *pWeaponPath = rISData.m_pWeaponPath;

		// Check each object with a transform history and do an AABB segment test against the historic transform...
		
		HOBJECT hObject = rObjectTransformHistory.GetNextTrackedObject( NULL );
		for( ; hObject && pWeaponPath; hObject = rObjectTransformHistory.GetNextTrackedObject( hObject ))
		{
			if( ObjListFilterFn( hObject, pWeaponPath->m_hFilterList ))
			{
				// Ignore non-live characters...
				CCharacter *pChar = CCharacter::DynamicCast( hObject );
				if( pChar && !pChar->IsAlive( ))
				{
                    pWeaponPath->IgnoreObject( hObject );
					pWeaponPath->IgnoreObject( pChar->GetHitBox( ));
					
					continue;
				}

				// The transform needs to be from the past, since that is what the client saw...
				// Calculate the round-trip connection latency...
				uint32 nTimeMSInPast = g_pLTServer->GetRealTimeMS( ) - pWeaponPath->m_nFireTimeStamp;

				// There is already a "built-in" latency from the prediction system holding
				// updates to the clients.  This latency also needs to be factored in and can be dynamic
				// so calculate it and add it to the connection latency...
				uint32 nMinUpdateRate = GetConsoleInt( "NetMinUnguaranteedPeriod", 50 );
				uint32 nPredictionHistorySize = GetConsoleInt( "PredictionHistorySize", 2 );
				uint32 nProcessingConstant = GetConsoleInt( "PredictionProcessingConstant", 15 );

				// Factor in the prediction latency on top of the connection latency...
				nTimeMSInPast += ((nPredictionHistorySize * nMinUpdateRate) + (nPredictionHistorySize * nProcessingConstant));
									
				rObjectTransformHistory.GetHistoricalTransform( hObject, nTimeMSInPast, rISData.m_tObjectHitTrans );

				LTVector vDims = LTVector::GetIdentity( );
				if( pChar )
				{
					g_pPhysicsLT->GetObjectDims( pChar->GetHitBox( ), &vDims );
				}
				else
				{
					g_pPhysicsLT->GetObjectDims( hObject, &vDims );
				}
			
				LTRect3f rAABB;

				// Now get at the AABB of the object...
				rAABB.m_vMin = rISData.m_tObjectHitTrans.m_vPos - vDims;
				rAABB.m_vMax = rISData.m_tObjectHitTrans.m_vPos + vDims;

				float fCollisionTime = 0.0f;
				if( LTIntersect::AABB_Segment( rAABB, rISData.m_pIQuery->m_From, rISData.m_pIQuery->m_To, fCollisionTime ))
				{
					// Successful hit, fill out the collision information...
					if( pChar )
					{
						rISData.m_pIInfo->m_hObject = pChar->GetHitBox( );
					}
					else
					{
						rISData.m_pIInfo->m_hObject = hObject;
					}
				
					// Need to determine if something is in the way of the character...
					LTVector vDir = rISData.m_pIQuery->m_To - rISData.m_pIQuery->m_From;
					rISData.m_pIQuery->m_To = rISData.m_pIQuery->m_From + (vDir * fCollisionTime);

					if( !g_pLTServer->IntersectSegment( *rISData.m_pIQuery, rISData.m_pIInfo ))
					{
						// There was a clean line to the player so set the hit point...
						rISData.m_pIInfo->m_Point = rISData.m_pIQuery->m_To;
					}

					return true;
				}
				else
				{
					// The shot missed the object so don't hit it with subsequent IntersectSegment calls...
					pWeaponPath->IgnoreObject( hObject );
					if( pChar )
					{
						pWeaponPath->IgnoreObject( pChar->GetHitBox( ));
					}
				}
			}
		}
	}

	// Didn't hit any objects with a transform history.  Just do normal intersect test...
	if( g_pLTServer->IntersectSegment( *rISData.m_pIQuery, rISData.m_pIInfo ))
	{
		g_pLTServer->GetObjectTransform( rISData.m_pIInfo->m_hObject, &rISData.m_tObjectHitTrans );
		return true;
	}

	return false;
}

void CProjectile::SendVectorWeaponFireMsg( )
{
	// We already sent our effects through TestInsideObject's AddImpact (no need to send again).
	//!!ARL: Still have a bug since we'll get double impacts if the AI sticks their muzzle inside your chest and pulls the trigger.
	// (maybe check m_bNumCallsToAddImpact??)
	if (m_bGuaranteedHit)
		return;

	// Create the client side (impact) weapon fx...
	CLIENTWEAPONFX fxStruct;
	fxStruct.hFiredFrom			= m_Shared.m_hFiredFrom;
	fxStruct.hWeapon			= m_Shared.m_hWeapon;
	fxStruct.bSendWeaponRecord	= m_bSendWeaponRecord;
	fxStruct.hAmmo				= m_Shared.m_hAmmo;
	fxStruct.bSendAmmoRecord	= m_bSendAmmoRecord;
	fxStruct.wIgnoreFX			= g_wIgnoreFX;
	
	
	// By default FX play at the position of the weapon's flash socket.
	// In some cases, we want the FX at some other socket (e.g. the character's LEFTHAND).
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_Shared.m_hWeapon, IsAI(m_Shared.m_hFiredFrom));
	fxStruct.bFXAtFlashSocket = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bFXAtFlashSocket );

	// If this is a player object, get the client id...
	if( IsPlayer( m_Shared.m_hFiredFrom ))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_Shared.m_hFiredFrom);
		if (pPlayer)
		{
			fxStruct.nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}


	// If this is a movable object, set the flags of fx to ignore
	// marks and smoke...
	if( IsMoveable( fxStruct.hObj ))
	{
		fxStruct.wIgnoreFX |= WFX_MARK;
	}


	// Tell all the clients who can see this fx about the fx...

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_WEAPON_FIRE_FX );

	// If the object that fired is a player then just send the clientID since it is
	// smaller that the object handle...
	bool bSendClientID = IsPlayer( fxStruct.hFiredFrom );
	cMsg.Writebool( bSendClientID );
	if( bSendClientID )
	{
		cMsg.Writeuint8( fxStruct.nShooterId );
	}
	else
	{
		cMsg.WriteObject( fxStruct.hFiredFrom );
	}


	cMsg.Writebool( fxStruct.bSendWeaponRecord );
	if( fxStruct.bSendWeaponRecord )
		cMsg.WriteDatabaseRecord( g_pLTDatabase, fxStruct.hWeapon );

	cMsg.Writebool( fxStruct.bSendAmmoRecord );
	if( fxStruct.bSendAmmoRecord )
	{
		cMsg.WriteDatabaseRecord( g_pLTDatabase, fxStruct.hAmmo );
	}
	else
	{
		// Inform the clients if an AI fired the weapon so they can retrieve
		// the correct weapon data record...
		bool bIsAI = IsAI(fxStruct.hFiredFrom);
		cMsg.Writebool( bIsAI );
	}

	// Once the IMPACT_DING is removed from the FX this can be reduced to 8 bits....
	cMsg.Writeuint16(fxStruct.wIgnoreFX);

	cMsg.Writebool(m_bLeftHandWeapon);

	// Write out each impact point so the fx will play at the correct postionon the clients...
	uint8 nNumImpactPoints = (uint8)m_lstImpactPoints.size( );
	cMsg.Writeuint8( nNumImpactPoints );
	if( nNumImpactPoints > 0 )
	{
		for( uint8 nPoint = 0; nPoint < nNumImpactPoints; ++nPoint )
		{
			cMsg.WriteCompLTVector( m_lstImpactPoints[nPoint] );
		}
	}

	

	// If a player fired the weapon don't send the FX message to them as they already created FX locally...
	if( IsPlayer( fxStruct.hFiredFrom ))
	{
		CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( fxStruct.hFiredFrom ));
		if( pPlayer )
		{
			SendToClientsExcept( *cMsg.Read( ), pPlayer->GetClient( ), 0 );
		}
	}
	else
	{
		g_pLTServer->SendToClient( cMsg.Read( ), NULL, 0 );
	}
}

void CProjectile::RegisterImpactAIStimuli( HAMMO hAmmo, 
										   HOBJECT hFiredFrom,
										   HOBJECT hObjectHit,
										   const LTVector& vImpactPos, 
										   const LTVector& vImpactDir,
										   SurfaceType eSurfaceType )
{
	//
	// Send a WeaponImpactSound stimulus.
	//

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData( hAmmo,IsAI( hFiredFrom ) );
	HRECORD hImpactFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sImpactFX ));
	if( hImpactFX )
	{
		// Get the Distance that the impact noise carries
		float fWeaponImpactNoiseDistance = (float)g_pFXDB->GetInt32(hImpactFX,FXDB_nAISoundRadius);

		// Scale based on surface types
		HSURFACE hSurf = g_pSurfaceDB->GetSurface( eSurfaceType );
		LTASSERT(hSurf, "Surface type not found" );
		if( hSurf )
		{
			if ( !g_pFXDB->GetBool(hImpactFX,FXDB_bAIIgnoreSurface) )
			{
				fWeaponImpactNoiseDistance *= g_pSurfaceDB->GetFloat(hSurf,SrfDB_Srf_fImpactNoiseMod,0,1.0f);
			}
		}

		// Send a WeaponImpactSound stimulus, if it makes any noise.
		if( fWeaponImpactNoiseDistance > 0.f )
		{
			EnumAIStimulusType eStimulusType = CAIStimulusMgr::StimulusFromString( g_pFXDB->GetString(hImpactFX,FXDB_sAIStimulusType) );

			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject( hFiredFrom );
			if ( pCharacter )
			{
				StimulusRecordCreateStruct scs( eStimulusType, pCharacter->GetAlignment(), vImpactPos + (vImpactDir * -1.0f), hFiredFrom );
				scs.m_flAlarmScalar = (float)g_pFXDB->GetInt32(hImpactFX,FXDB_nAIAlarmLevel);
				scs.m_hStimulusTarget = hObjectHit;
				scs.m_flRadiusScalar = fWeaponImpactNoiseDistance;

				g_pAIStimulusMgr->RegisterStimulus( scs );
			}
		}
	}


	//
	// Send a SeeEnemyWeaponImpact stimulus, if it hit a character.
	//

	if(IsCharacter(hObjectHit))
	{
		DamageType eInstDamageType = g_pWeaponDB->GetAmmoInstDamageType( hAmmo, IsAI( hFiredFrom ) );
		DamageType eProgDamageType = g_pWeaponDB->GetAmmoProgDamageType( hAmmo, IsAI( hFiredFrom ) );

		if( (eInstDamageType != DT_UNSPECIFIED) ||
			(eProgDamageType != DT_UNSPECIFIED) )
		{
			CCharacter* pCharacter = (CCharacter*)g_pLTServer->HandleToObject( hFiredFrom );
			if ( pCharacter )
			{
				StimulusRecordCreateStruct scs( kStim_WeaponImpactVisible, pCharacter->GetAlignment(), vImpactPos + (vImpactDir * -1.0f), hFiredFrom );
				scs.m_hStimulusTarget = hObjectHit;
				g_pAIStimulusMgr->RegisterStimulus( scs );
			}
		}
	}
}
