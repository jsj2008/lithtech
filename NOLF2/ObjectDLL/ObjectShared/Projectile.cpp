// ----------------------------------------------------------------------- //
//
// MODULE  : Projectile.cpp
//
// PURPOSE : Projectile class - implementation
//
// CREATED : 9/25/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Projectile.h"
#include "ltengineobjects.h"
#include "iltserver.h"
#include "MsgIds.h"
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
#include "Body.h"
#include "AI.h"
#include "CVarTrack.h"
#include "iltmodel.h"
#include "ilttransform.h"
#include "iltphysics.h"
#include "ObjectMsgs.h"
#include "CharacterHitBox.h"
#include "GameServerShell.h"
#include "Camera.h"
#include "AIStimulusMgr.h"
#include "WeaponFireInfo.h"
#include "Weapon.h"
#include "AIUtils.h"
#include "VersionMgr.h"

LINKFROM_MODULE( Projectile );

#pragma force_active on
BEGIN_CLASS(CProjectile)
END_CLASS_DEFAULT_FLAGS(CProjectile, GameBase, NULL, NULL, CF_HIDDEN)
#pragma force_active off

extern uint16 g_wIgnoreFX;
extern uint8  g_nRandomWeaponSeed;
extern CAIStimulusMgr* g_pAIStimulusMgr;

static bool DoVectorFilterFn(HOBJECT hObj, void *pUserData);
static bool CanCharacterHitCharacter( CProjectile* pProjectile, HOBJECT hImpacted );

#define MAX_MODEL_NODES         9999
#define MAX_VECTOR_LOOP         20

static CVarTrack g_vtInvisibleMaxThickness;
static CVarTrack g_vtDeflectAngleRange;

CVarTrack g_vtNetFriendlyFire;


namespace
{
	void ProjectileInitFileGlobals()
	{
		if (!g_vtInvisibleMaxThickness.IsInitted())
		{
			g_vtInvisibleMaxThickness.Init(g_pLTServer, "InvisibleMaxThickness", LTNULL, 33.0f);
		}
		if (!g_vtNetFriendlyFire.IsInitted())
		{
			g_vtNetFriendlyFire.Init(g_pLTServer, "NetFriendlyFire", LTNULL, 0.0f);
		}
		if (!g_vtDeflectAngleRange.IsInitted())
		{
			g_vtDeflectAngleRange.Init(g_pLTServer, "DeflectAngleRange", LTNULL, 30.0f );
		}
	}
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
{
	AddAggregate(&m_damage);
	MakeTransitionable();

	m_vFlashPos.Init();
	m_vFirePos.Init();
	m_vDir.Init();

    m_hObject               = LTNULL;
	m_fVelocity				= 0.0f;
	m_fInstDamage			= 0.0f;
	m_fProgDamage			= 0.0f;
	m_fMass					= INFINITE_MASS; //0.0f;
	m_fLifeTime				= 5.0f;
	m_fRange				= 10000.0f;
    m_bSilenced             = LTFALSE;
	m_bSetup				= LTFALSE;

	m_bObjectRemoved        = LTFALSE;
	m_bDetonated            = LTFALSE;

	m_nWeaponId             = 0;
	m_nAmmoId               = 0;

	m_bCanHitSameProjectileKind = LTFALSE;
	m_bCanTouchFiredFromObj		= LTFALSE;

	m_eInstDamageType       = DT_BULLET;
	m_eProgDamageType       = DT_UNSPECIFIED;
	m_fInstDamage           = 0.0f;
	m_fProgDamage           = 0.0f;

	m_fRadius               = 0.0f;
	m_fStartTime            = 0.0f;
	m_fLifeTime             = 5.0f;
	m_fVelocity             = 0.0f;
	m_fMass                 = INFINITE_MASS; //0.0f;
	m_fRange                = 10000.0f;

	m_hFiredFrom            = LTNULL;

	m_bNumCallsToAddImpact = 0;

	m_bProcessInvImpact = LTFALSE;
	m_vInvisVel.Init();
	m_vInvisNewPos.Init();

	m_vDims.Init(1.0f, 1.0f, 1.0f);

	m_dwFlags = FLAG_FORCECLIENTUPDATE | FLAG_POINTCOLLIDE | FLAG_NOSLIDING |
	    FLAG_TOUCH_NOTIFY | FLAG_NOLIGHT | FLAG_RAYHIT;

	static DamageFlags nCantDamageFlags = DamageTypeToFlag(DT_BLEEDING) 
	        | DamageTypeToFlag(DT_BURN) 
	        | DamageTypeToFlag(DT_CHOKE) 
	        | DamageTypeToFlag(DT_ELECTROCUTE) 
	        | DamageTypeToFlag(DT_FREEZE) 
	        | DamageTypeToFlag(DT_POISON) 
	        | DamageTypeToFlag(DT_ENDLESS_FALL)
			| DamageTypeToFlag(DT_SLEEPING)
			| DamageTypeToFlag(DT_STUN)
			| DamageTypeToFlag(DT_CAMERA_DISABLER)
			| DamageTypeToFlag(DT_GLUE)
			| DamageTypeToFlag(DT_BEAR_TRAP)
			| DamageTypeToFlag(DT_LAUGHING)
			| DamageTypeToFlag(DT_ASSS)
			| DamageTypeToFlag(DT_SLIPPERY);

	// Make sure we can't be damaged by any of the progressive/volumebrush
	// damage types...
	m_nCantDamageFlags     = nCantDamageFlags;

	m_pWeaponData       = LTNULL;
	m_pAmmoData         = LTNULL;

	m_nTeamId = INVALID_TEAM;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CProjectile::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
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
					SAFE_STRCPY(pStruct->m_Filename, "Models\\Default.ltb");

					pStruct->m_NextUpdate = UPDATE_NEXT_FRAME;
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
				Detonate(LTNULL);
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

LTBOOL CProjectile::Setup(CWeapon const* pWeapon, WeaponFireInfo const & info)
{
	if (!pWeapon || !info.hFiredFrom || !g_pWeaponMgr) return LTFALSE;

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
	m_hFiredFrom        = info.hFiredFrom;

	// If this is a team game, get any team id from the firedfrom object.
	if( IsTeamGameType( ))
	{
		SetTeamId( GetPlayerTeamId( m_hFiredFrom ));
	}

#ifdef _DEBUG

	HCLASS hOwnerClass = g_pLTServer->GetObjectClass( m_hFiredFrom );
	char szOwnerClassName[ 128 ];
	LTRESULT ltResult;
	ltResult = g_pLTServer->GetClassName( hOwnerClass, szOwnerClassName, 128 );
	ASSERT( LT_OK == ltResult );

	if( LT_INSIDE != g_pLTServer->Common()->GetPointStatus( &m_vFirePos ) )
	{
		g_pLTServer->CPrint( "Fire Pos from '%s' was outside world", szOwnerClassName );
	}
	
	if( LT_INSIDE != g_pLTServer->Common()->GetPointStatus( &m_vFlashPos ) )
	{
		g_pLTServer->CPrint( "Flash Pos from '%s' was outside world", szOwnerClassName );
	}

#endif

	// get the weapon/ammo ids
	m_nWeaponId         = pWeapon->GetId();
	m_nAmmoId           = pWeapon->GetAmmoId();

	// determine if this projectile can impact
	// projectiles of the same kind
	if ( WMGR_INVALID_ID != m_nAmmoId )
	{
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo( m_nAmmoId );
		if ( ( 0 != pAmmo ) && ( 0 != pAmmo->pProjectileFX ) )
		{
			m_bCanHitSameProjectileKind = ( 0 != pAmmo->pProjectileFX->nCanImpactSameKind );
		}
	}

	// set the lifetime of the projectile
	m_fLifeTime         = pWeapon->GetLifeTime();

	m_hFiredFrom		= info.hFiredFrom;
	m_nWeaponId			= pWeapon->GetId();
	m_nAmmoId			= pWeapon->GetAmmoId();
	m_fLifeTime			= pWeapon->GetLifeTime();
	
	m_pWeaponData		= pWeapon->GetWeaponData();
	if (!m_pWeaponData) return LTFALSE;

	m_pAmmoData			= pWeapon->GetAmmoData();
	if (!m_pAmmoData) return LTFALSE;

	// setup the damage info
	m_fInstDamage       = pWeapon->GetInstDamage();
	m_fProgDamage       = pWeapon->GetProgDamage();
	m_eInstDamageType	= m_pAmmoData->eInstDamageType;
	m_eProgDamageType	= m_pAmmoData->eProgDamageType;

	// determine the velocity
	if (info.bOverrideVelocity)
	{
		m_fVelocity = info.fOverrideVelocity;
	}
	else if (info.bAltFire)  
	{
		m_fVelocity = (LTFLOAT) (m_pAmmoData->pProjectileFX ? m_pAmmoData->pProjectileFX->nAltVelocity : 0);
	}
	else
	{
		m_fVelocity = (LTFLOAT) (m_pAmmoData->pProjectileFX ? m_pAmmoData->pProjectileFX->nVelocity : 0);
	}

	// determine the projectile's range
	m_fRange            = (LTFLOAT) m_pWeaponData->nRange;

	// get the special case stuff
	m_bSilenced         = !!(pWeapon->GetSilencer());

	// determine ammo type
	AmmoType eAmmoType  = m_pAmmoData->eType;

	// no calls to add impact yet
	m_bNumCallsToAddImpact = 0;

	// Client-side hit detection
	m_vClientObjImpactPos	= info.vClientObjImpactPos;
	m_hClientObjImpact		= info.hClientObjImpact;

	// [RP] - Beyond this point nothing else can justify a bad setup so set a successful 
	//		  setup here.  This way calls that rely on a successful setup will have accurate info.

	m_bSetup = LTTRUE;

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
			DoVector();
		}
		else if (eAmmoType == GADGET)
		{
			; // Do nothing for now
		}
	}

	// register a Enemy Weapon Fire Sound AND a Ally WeaponFireSound

	// Get the Distance that fire noise carries	
	LTFLOAT fWeaponFireNoiseDistance = (LTFLOAT)pWeapon->GetWeaponData()->nAIFireSoundRadius;

	// If we're silenced use the radius specified by the silencer...
	if (m_bSilenced)
	{
		const MOD* pMod = pWeapon->GetSilencer();
		if (pMod)
		{
			fWeaponFireNoiseDistance = (LTFLOAT) pMod->nAISilencedFireSndRadius;
		}
	}

	if( fWeaponFireNoiseDistance > 0.f )
	{
		g_pAIStimulusMgr->RegisterStimulus(kStim_EnemyWeaponFireSound, m_hFiredFrom, m_vFirePos, fWeaponFireNoiseDistance, 1.f);
		g_pAIStimulusMgr->RegisterStimulus(kStim_AllyWeaponFireSound,  m_hFiredFrom, m_vFirePos, fWeaponFireNoiseDistance, 1.f);
		g_pAIStimulusMgr->RegisterStimulus(kStim_ProhibitedWeaponFireSound,  m_hFiredFrom, m_vFirePos, fWeaponFireNoiseDistance, 1.f);
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::TestInsideObject
//
//	PURPOSE:	Test to see if the projectile is inside the test object
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::TestInsideObject(HOBJECT hTestObj, AmmoType eAmmoType)
{
	// This appears to be for the AIs to test the object they are
	// shooting at, apparently for the case where the AI is standing
	// on top of its target.  I'm guessing that if the projectile
	// gets 1 update before checking if it hit anything, there is a
	// chance the projectile will go through the targe.  If so, this
	// function will see that it is already "inside" the object, and
	// hit it immediately.

	// return if there is no test object
	if (!hTestObj) return LTFALSE;

	// TO DO???
	// NOTE:  This code may need to be updated to use test the dims
	// of the CharacterHitBox instead of the dims of the object...
	// TO DO???

	// See if we are inside the test object...

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
		return LTFALSE;
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
				if (!pChar) return LTFALSE;

				ModelNode eModelNode = g_pModelButeMgr->GetSkeletonDefaultHitNode(pChar->GetModelSkeleton());

				pChar->SetModelNodeLastHit(eModelNode);

				//DANO: POTENTIAL PROBLEM!!
				// This variable m_fInstDamage gets changed here but
				// used by another function...in effect this variable
				// is getting passed "under the hood".  This can present
				// big problems with the Sequencer!
				AdjustDamage(pChar->ComputeDamageModifier(eModelNode));
			}

			ImpactDamageObject(m_hFiredFrom, hTestObj);
		}

		LTVector vNormal(0, 1, 0);
		AddImpact(hTestObj, m_vFlashPos, vTestPos, vNormal, GetSurfaceType(hTestObj));
	}

	RemoveObject();

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::AdjustDamage()
//
//	PURPOSE:	Adjust the instant damage by a modifier
//
// ----------------------------------------------------------------------- //

void CProjectile::AdjustDamage(LTFLOAT fModifier)
{
	// Only adjust the damage if we are using an adjustable damage type...

	if (m_pAmmoData->bCanAdjustInstDamage)
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
	g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED);

	//
	if (nInfo == INITIALUPDATE_SAVEGAME) return;

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_MOVEABLE, USRFLG_MOVEABLE);
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_SERVERDIMS, FLAG2_SERVERDIMS);

	// setup the damage object
	m_damage.Init(m_hObject);
	m_damage.SetMass(m_fMass);
	m_damage.SetHitPoints(1.0f);
	m_damage.SetMaxHitPoints(1.0f);
	m_damage.SetArmorPoints(0.0f);
	m_damage.SetMaxArmorPoints(0.0f);
	m_damage.SetCanHeal(LTFALSE);
	m_damage.SetCanRepair(LTFALSE);
	m_damage.SetApplyDamagePhysics(LTFALSE);

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
		m_bProcessInvImpact = LTFALSE;
		g_pPhysicsLT->SetVelocity(m_hObject, &m_vInvisVel);
		g_pLTServer->SetObjectPos(m_hObject, &m_vInvisNewPos);
	}

	// setup the next update
	SetNextUpdate(UPDATE_NEXT_FRAME);

	// Detonate if the life time has expired
	if (g_pLTServer->GetTime() >= (m_fStartTime + m_fLifeTime))
	{
		Detonate(LTNULL);
	}
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
	if( (hObj == m_hFiredFrom) && !m_bCanTouchFiredFromObj ) return;

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
	else if (IsBody(hObj))
	{
		// convert the object from a body to its hitbox
		Body* pBody = (Body*)g_pLTServer->HandleToObject(hObj);
		if (pBody)
		{
			hObj = pBody->GetHitBox();
		}
	}

	// See if we want to impact on this object...
    uint32 dwUsrFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_User, dwUsrFlags);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES) return;

	// get the hitbox
	CCharacterHitBox* pHitBox = LTNULL;
	if (IsCharacterHitBox(hObj))
	{
		pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
		if (!pHitBox) return;

		// can't hit ourselves
		if( (pHitBox->GetModelObject() == m_hFiredFrom) && !m_bCanTouchFiredFromObj ) return;
	}

	
	// Handle special AI alignment cases and multiplayer friendly fire...

	if( !CanCharacterHitCharacter( this, hObj ))
		return;


	// can we hit projectiles of the same kind?
	if ( LTFALSE == m_bCanHitSameProjectileKind )
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
				if (pObj->GetFiredFrom() == m_hFiredFrom)
				{
					return;
				}
			}
		}
	}

	LTBOOL bIsWorld = IsMainWorld(hObj);

	// Don't impact on non-solid objects...unless it is a CharacterHitBox
	// object...
	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);
	if (!bIsWorld && !(dwFlags & FLAG_SOLID))
	{
		if (pHitBox)
		{
			// See if we really impacted on the box...
			if (pHitBox->DidProjectileImpact(this))
			{
				// This is the object that we really hit...
				hObj = pHitBox->GetModelObject();
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
			m_bProcessInvImpact = LTTRUE;

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
		else if ( ( m_pAmmoData->pProjectileFX->fMaxRicochetAngle > 0.0f ) &&
		          ( m_pAmmoData->pProjectileFX->nMaxRicochets > m_nTotalRicochets ) )
		{
			// check if we have ricocheted this frame
			if ( m_nLastRicochetUpdateNum == m_nUpdateNum ) 
			{
				// we have ricocheted, don't do it again
				return;
			}

			//
			// check for a ricochet
			//

			// TODO: this duplicates the code in HandleImpact,
			// set it up so this stuff doesn't have to be done twice.
			// (Its not the cheapest in the world).


			LTRESULT ltResult;

			// get the collision info for this impact
			CollisionInfo colInfo;
			ltResult = g_pLTServer->GetLastCollision(&colInfo);
			ASSERT( LT_OK == ltResult );

			// get the normal of the plane we impacted with
			LTPlane plane = colInfo.m_Plane;
			LTVector vNormal = plane.m_Normal;

			//
			// Calculate where we really hit the plane
			// and make sure we don't tunnel through an object
			//

			LTVector vVel, vP0, vP1, vDir;
			// get the velocity of the projectile
			ltResult = g_pPhysicsLT->GetVelocity(m_hObject, &vVel);
			ASSERT( LT_OK == ltResult );

			// get the direction of the projectile
			vDir = vVel;
			vDir.Normalize();

			// determine how much we've travelled this frame
			vVel *= g_pLTServer->GetFrameTime();

			// get the position
			LTVector vPos;
			ltResult = g_pLTServer->GetObjectPos(m_hObject, &vPos);
			ASSERT( LT_OK == ltResult );

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

			if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
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
				LTFLOAT fDot1 = VEC_DOT(vNormal, vP0) - colInfo.m_Plane.m_Dist;
				LTFLOAT fDot2 = VEC_DOT(vNormal, vP1) - colInfo.m_Plane.m_Dist;

				if ( ( ( fDot1 < 0.0f ) && ( fDot2 < 0.0f ) ) ||
				     ( ( fDot1 > 0.0f ) && ( fDot2 > 0.0f ) ) )
				{
					vPos = vP1;
				}
				else
				{
					LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
					VEC_LERP(vPos, vP0, vP1, fPercent);
				}
			}

			LTFLOAT fAngleFromNormal;
			LTVector vReversedNormal = -vNormal;
			fAngleFromNormal = ltacosf( VEC_DOT( vReversedNormal, vDir ) );
			
			if ( ( MATH_HALFPI - fAngleFromNormal ) < m_pAmmoData->pProjectileFX->fMaxRicochetAngle )
			{
				// ricochet the projectile

				// increase the ricochet count
				m_nTotalRicochets += 1;

				// reflect the direction
				LTVector vReflectedDir;
				vReflectedDir = vDir - 2 * vNormal * ( VEC_DOT( vDir, vNormal ) );

				// normalize and scale to velocity
				LTVector vNewVel = vReflectedDir;
				vNewVel *= static_cast< LTFLOAT >( m_pAmmoData->pProjectileFX->nVelocity );

				// keep track of the update num to prevent multiple ricochets in the same frame
				m_nLastRicochetUpdateNum = m_nUpdateNum;

				// set the new speed
				ltResult = g_pPhysicsLT->SetVelocity(m_hObject, &vNewVel);
				ASSERT( LT_OK == ltResult );

				// do all the impact work
				AddImpact( hObj, m_vFlashPos, vPos, vNormal, eType, IMPACT_TYPE_RICOCHET );

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
	m_bDetonated = LTTRUE;

	// eventually we will determine the surface type
	SurfaceType eType = ST_UNKNOWN;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// Determine the normal of the surface we are impacting on...
	LTVector vNormal(0.0f, 1.0f, 0.0f);

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

			if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
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
				LTFLOAT fDot1 = VEC_DOT(vNormal, vP0) - info.m_Plane.m_Dist;
				LTFLOAT fDot2 = VEC_DOT(vNormal, vP1) - info.m_Plane.m_Dist;

				if ( ( ( fDot1 < 0.0f ) && ( fDot2 < 0.0f ) ) ||
				     ( ( fDot1 > 0.0f ) && ( fDot2 > 0.0f ) ) )
				{
					vPos = vP1;
				}
				else
				{
					LTFLOAT fPercent = -fDot1 / (fDot2 - fDot1);
					VEC_LERP(vPos, vP0, vP1, fPercent);
				}
			}

			// reset the projectile's rotation
			LTRotation rRot(vNormal, LTVector(0.0f, 1.0f, 0.0f));
			g_pLTServer->SetObjectRotation(m_hObject, &rRot);
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

	// Check if we got deflected.
	if( Deflect( hObj, m_vFlashPos, vPos, vNormal ))
	{
		// Remove projectile from world...
		RemoveObject();
		return;
	}


	// do all the impact work
	AddImpact(hObj, m_vFlashPos, vPos, vNormal, eType);

	// Handle impact damage...
	if (hObj)
	{
		HOBJECT hDamager = m_hFiredFrom ? m_hFiredFrom : m_hObject;

		ImpactDamageObject(hDamager, hObj);
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

void CProjectile::AddImpact( HOBJECT hObj,
                             const LTVector &vFirePos,
                             const LTVector &vImpactPos,
                             const LTVector &vSurfaceNormal,
                             SurfaceType eType,
                             IMPACT_TYPE eImpactType,
							 bool bDeflected)
{
	if ( !m_bSetup )
	{
		return;
	}

	// validate the pointers we will use
	ASSERT( 0 != g_pSurfaceMgr );
	ASSERT( 0 != g_pLTServer );
	ASSERT( 0 != g_pWeaponMgr );
	ASSERT( 0 != g_pAIStimulusMgr );
	ASSERT( 0 != m_pWeaponData );
	ASSERT( 0 != m_pAmmoData );

	// Create the client side (impact) weapon fx...
	CLIENTWEAPONFX fxStruct;
	fxStruct.hFiredFrom     = m_hFiredFrom;
	fxStruct.vSurfaceNormal = vSurfaceNormal;
	fxStruct.vFirePos       = vFirePos;
	fxStruct.vPos           = vImpactPos + (m_vDir * -1.0f);
	fxStruct.hObj           = hObj;
	fxStruct.nWeaponId      = m_pWeaponData->nId;
	fxStruct.nAmmoId        = m_pAmmoData->nId;
	fxStruct.nSurfaceType   = eType;
	fxStruct.wIgnoreFX      = g_wIgnoreFX;
	fxStruct.eImpactType    = eImpactType;

	if (bDeflected)
		fxStruct.wIgnoreFX |= (WFX_MARK | WFX_EXITMARK);

	// Always use the flash position for the first call to AddImpact...
	//DANO: why?
	if (m_bNumCallsToAddImpact == 0)
	{
		fxStruct.vFirePos = m_vFlashPos;
	}

	// If we do multiple calls to AddImpact, make sure we only do some
	// effects once :)
	//DANO: mission an "if" statement?
	g_wIgnoreFX |= WFX_SHELL | WFX_LIGHT | WFX_MUZZLE;


	// Allow exit surface fx on the next call to AddImpact...
	//DANO: missing an "if" statement?
	g_wIgnoreFX &= ~WFX_EXITSURFACE;

	// limit the amount of exit marks
	if (IsMoveable(hObj))
	{
		// Well, don't do too many exit marks...The server will add one
		// if necessary...

		g_wIgnoreFX |= WFX_EXITMARK;
	}

	// If this is a player object, get the client id...
	if (IsPlayer(m_hFiredFrom))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pPlayer)
		{
			fxStruct.nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}

	// do all server side fx activities
	// and send a message to the client
	CreateClientWeaponFX(fxStruct);

	// Do the area and progressive (over time) damage...
	if ((m_pAmmoData->nAreaDamage > 0.0f && eType != ST_SKY) ||
		 m_pAmmoData->fProgDamageLifetime > 0.0f)
	{
		AddExplosion(vImpactPos, vSurfaceNormal);
	}

	// Update Character fire info...
	if (m_hFiredFrom && IsCharacter(m_hFiredFrom) && CanSetLastFireInfo())
	{
		CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pChar)
		{
			CharFireInfo info;
			info.hObject    = hObj;
			info.vFiredPos  = m_vFlashPos;  // Use initial flash pos
			info.vImpactPos = vImpactPos;
			info.nWeaponId  = m_pWeaponData->nId;
			info.nAmmoId    = m_pAmmoData->nId;
			info.fTime      = g_pLTServer->GetTime();
			info.bSilenced  = m_bSilenced;
			info.eSurface   = eType;

			pChar->SetLastFireInfo(&info);
		}
	}

	// if the projectile was fired from a character
	if(m_hFiredFrom && IsCharacter(m_hFiredFrom))
	{
		//
		// send a bunch of AI stimuli
		//

		WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon(m_pWeaponData->nId);
		ASSERT( 0 != pWeapon );

		// Send a HearEnemyWeaponImpact stimulus.
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(m_pAmmoData->nId);
		if ( (!pAmmo) || (!pAmmo->pImpactFX) ) return;
		ASSERT(pAmmo && pAmmo->pImpactFX);

		// Get the Distance that the impact noise carries
		LTFLOAT fWeaponImpactNoiseDistance = (LTFLOAT) pAmmo->pImpactFX->nAISoundRadius;
	
		// Scale based on surface types
		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eType);
		ASSERT(pSurf && "Surface type not found" );
		if( !pSurf ) return;

		if ( !pAmmo->pImpactFX->bAIIgnoreSurface )
		{
			fWeaponImpactNoiseDistance *= pSurf->fImpactNoiseModifier;
		}

		// note: the enemy part of kStim_EnemyWeaponImpactSound
		// is largly irrelevant, this will just register a weapon impact sound
		// and the AI can react however it wants

		if( fWeaponImpactNoiseDistance > 0.f )
		{
			g_pAIStimulusMgr->RegisterStimulus( (EnumAIStimulusType)pAmmo->pImpactFX->nAIStimulusType, pAmmo->pImpactFX->nAIAlarmLevel, m_hFiredFrom, hObj, vImpactPos, fWeaponImpactNoiseDistance);
			g_pAIStimulusMgr->RegisterStimulus(kStim_ProhibitedWeaponImpactSound, m_hFiredFrom, hObj, vImpactPos, fWeaponImpactNoiseDistance, 1.f);
		}

		// Send a SeeEnemyWeaponImpact stimulus, if it hit a character.
		if(IsCharacter(hObj))
		{
			// [RP] 8/08/02 - I really don't like special casing the damage types like this 
			//		but since we are so close to gold this is the safest way of excluding the
			//		tracking damage from registering a visible stimulus.
			// [JO] 8/14/02 - added skipping Unspecified, so neutral AI do not freak out when a coin comes near them.
			// [JO] 9/07/02 - added skipping camera disabler.

			if( (pAmmo->eInstDamageType != DT_GADGET_TRACKER) && 
				(pAmmo->eProgDamageType != DT_GADGET_TRACKER) && 
				(pAmmo->eInstDamageType != DT_CAMERA_DISABLER) && 
				(pAmmo->eProgDamageType != DT_CAMERA_DISABLER) && 
				( ( pAmmo->eInstDamageType != DT_UNSPECIFIED ) || ( pAmmo->eProgDamageType != DT_UNSPECIFIED ) ) )
			{
				g_pAIStimulusMgr->RegisterStimulus(kStim_EnemyWeaponImpactVisible, m_hFiredFrom, hObj, vImpactPos, 1.f, 1.f);
				g_pAIStimulusMgr->RegisterStimulus(kStim_ProhibitedWeaponImpactVisible, m_hFiredFrom, hObj, vImpactPos, 1.f, 1.f);
			}
		}
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
	INIT_OBJECTCREATESTRUCT(theStruct);

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
		pExplosion->Setup(m_hFiredFrom, m_nAmmoId);
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

	//
	// This apparently only creates the special FX related the the firing
	// of the weapon.
	//

	if (!g_pWeaponMgr) return;

	// If this is a player object, get the client id...
	uint8 nShooterId = -1; 
	if (IsPlayer(m_hFiredFrom))
	{
		CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hFiredFrom);
		if (pPlayer)
		{
			nShooterId = (uint8) g_pLTServer->GetClientID(pPlayer->GetClient());
		}
	}


		// Create a special fx...
		CAutoMessage cMsg;

		cMsg.Writeuint8(SFX_PROJECTILE_ID);

		cMsg.Writeuint8(m_pWeaponData->nId);

		cMsg.Writeuint8(m_pAmmoData->nId);

		cMsg.Writeuint8(nShooterId);

		LTRESULT ltResult;
		ltResult = g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
		ASSERT( LT_OK == ltResult );


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
	m_bObjectRemoved = LTTRUE;
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DoVectorPolyFilterFn()
//
//	PURPOSE:	Handle filtering out unwanted polies
//
//	USED:		only for vectors
//
// ----------------------------------------------------------------------- //

bool DoVectorPolyFilterFn(HPOLY hPoly, void *pUserData)
{
	// Don't filter for now...
	//DANO: this was removed in the PS2 version, should it
	// be removed here?
	return true;

	// Make sure we hit a surface type we care about...
	SurfaceType eSurfType = GetSurfaceType(hPoly);

	if (eSurfType == ST_INVISIBLE)
	{
		// ignore invisible surfaces
		return false;
	}

	return true;
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
	// Filter out the specified objects...
	VectorFilterFnUserData* pVectFilterFnUserData = ( VectorFilterFnUserData* )pUserData;
	if (ObjListFilterFn(hObj, pVectFilterFnUserData->m_pFilterList))
	{
		// CharacterHitBox objects are used for vector impacts, don't
		// impact on the character/body prop object itself....

		if (IsCharacter(hObj) || IsBody(hObj) || IsKindOf(hObj, "Intelligence"))
		{
            return false;
		}

		// Check special character hit box cases...

		if (IsCharacterHitBox(hObj))
		{
            CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(hObj);
			if (pCharHitBox)
			{
				// NOTE: The first object in the filter list MUST be m_hFiredFrom!!!!!

				HOBJECT hUs = pVectFilterFnUserData->m_pFilterList[0];

				HOBJECT hTestObj = pCharHitBox->GetModelObject();
                if (!hTestObj) return LTFALSE;

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

		if ( NEUTRAL != GetRelativeAlignment( pAI->GetRelationSet(),
			pB->GetRelationData()) )
		{
			// If they are NOT neutral to us, then find out if we like or
			// dislike them.
			CharacterAlignment eAlignment = GetAlignment(pAI->GetRelationSet(), 
				pB->GetRelationData() );
			
			// Return true if we don't like them, false if we do.
			return LIKE != eAlignment;
		}
	}

	// Check for friendly fire
	if( IsMultiplayerGame( ))
	{
		if( g_vtNetFriendlyFire.GetFloat() < 1.0f)
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
	if (!m_pAmmoData || !m_pAmmoData->pProjectileFX) return;

	// validate the pointers we will use
	ASSERT( 0 != g_pSurfaceMgr );
	ASSERT( 0 != g_pCommonLT );
	ASSERT( 0 != g_pPhysicsLT );

	// Set up the model...

	ObjectCreateStruct createStruct;
	createStruct.Clear();

	SAFE_STRCPY(createStruct.m_Filename, m_pAmmoData->pProjectileFX->szModel);
	SAFE_STRCPY(createStruct.m_SkinNames[0], m_pAmmoData->pProjectileFX->szSkin);

    g_pCommonLT->SetObjectFilenames(m_hObject, &createStruct);
    g_pLTServer->ScaleObject(m_hObject, &(m_pAmmoData->pProjectileFX->vModelScale));

	// Set the dims based on the current animation...

    LTVector vDims;
    g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));

	// Set object dims based on scale value...

    LTVector vNewDims = vDims * m_pAmmoData->pProjectileFX->vModelScale;
   	g_pPhysicsLT->SetObjectDims(m_hObject, &vNewDims, 0);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);


	// Start your engines...

    m_fStartTime = g_pLTServer->GetTime();


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
	g_pLTServer->SetObjectRotation(m_hObject, &rRot);


	// Make sure we have the correct flags set...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_pAmmoData->pProjectileFX->dwObjectFlags, m_pAmmoData->pProjectileFX->dwObjectFlags);


	// And away we go...

    LTVector vVel;
	vVel = m_vDir * m_fVelocity;
	g_pPhysicsLT->SetVelocity(m_hObject, &vVel);


	// Special case of 0 life time...

	if (m_fLifeTime <= 0.0f)
	{
        Detonate(LTNULL);
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

void CProjectile::DoVector()
{
	// Check to see if we should use the client-side intersection
	if (HandleClientVectorImpact())
	{
		return;
	}

	IntersectInfo iInfo;
	IntersectQuery qInfo;

	LTVector vTo, vFrom, vOriginalFrom;
	vFrom = m_vFirePos;
	vTo	= vFrom + (m_vDir * m_fRange);

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	LTBOOL bHitSomething = LTFALSE;
	LTBOOL bDone         = LTFALSE;

	int nLoopCount = 0; // No infinite loops thanks.


	const int cMaxNumFilterObjects = 11;
	HOBJECT hFilterList[cMaxNumFilterObjects];
	
	// Make sure we don't hit the object that fired the weapon...

	hFilterList[0] = m_hFiredFrom;
	int nNumFilteredObjects = 1;

	for (int i=1; i < cMaxNumFilterObjects; i++)
	{
		hFilterList[i] = LTNULL;
	}

	qInfo.m_PolyFilterFn = DoVectorPolyFilterFn;
	qInfo.m_FilterFn	 = DoVectorFilterFn;
	VectorFilterFnUserData userdata;
	userdata.m_pFilterList = hFilterList;
	userdata.m_pProjectile = this;
	qInfo.m_pUserData	 = &userdata;

	while (!bDone)
	{
		qInfo.m_From = vFrom;
		qInfo.m_To   = vTo;

		if (g_pLTServer->IntersectSegment(&qInfo, &iInfo))
		{
			if (IsCharacterHitBox(iInfo.m_hObject))
			{
				vOriginalFrom = vFrom;

				if (HandlePotentialHitBoxImpact(iInfo, vFrom))
				{
					HandleVectorImpact(iInfo, vOriginalFrom, vTo);
					return;
				}
			}
			else
			{
				if (HandleVectorImpact(iInfo, vFrom, vTo))
				{
					return;
				}
			}

			// Filter out the object we just hit from the next call
			// to intersect segment...

			if (iInfo.m_hObject && !IsMainWorld(iInfo.m_hObject))
			{
				if (nNumFilteredObjects < cMaxNumFilterObjects)
				{
					// Filter out the object we just hit...
		
					hFilterList[nNumFilteredObjects] = iInfo.m_hObject;
					nNumFilteredObjects++;
				}
				else
				{
					// Hit too many objects...
					g_pLTServer->CPrint("ERROR in CProjectile::DoVector() - Tried to filter too many objects!!!");
					bDone = LTTRUE;
				}
			}
		}
		else // Didn't hit anything...
		{
            bDone = LTTRUE;
		}


		// Melee weapons can't shoot through objects...
		ASSERT( 0 != m_pAmmoData );
		if (m_pAmmoData->eInstDamageType == DT_MELEE)
		{
			bDone = LTTRUE;
		}


		// Make sure we don't loop forever...

		if (++nLoopCount > MAX_VECTOR_LOOP)
		{
            g_pLTServer->CPrint("ERROR in CProjectile::DoVector() - Infinite loop encountered!!!");
            bDone = LTTRUE;
		}
	}


	// Didn't hit anything so just impact at the end pos...

    LTVector vUp(0.0f, 1.0f, 0.0f);
    AddImpact(LTNULL, m_vFlashPos, vTo, vUp, ST_SKY);


	// Okay, we're all done now...bye, bye...

	RemoveObject();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleClientVectorImpact
//
//	PURPOSE:	Handle a client-side vector hitting something
//
//	USED:		only for vectors
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::HandleClientVectorImpact()
{
	if (!m_hClientObjImpact) return LTFALSE;

	// Filter the object they told us about, because we know stuff they don't
	HOBJECT hFilterList[] = { m_hFiredFrom, LTNULL };
	VectorFilterFnUserData userdata;
	userdata.m_pFilterList = hFilterList;
	userdata.m_pProjectile = this;
	if (!DoVectorFilterFn(m_hClientObjImpact, (void*)&userdata))
	{
		return LTFALSE;
	}

	LTVector vCurObjPos;
	g_pLTServer->GetObjectPos(m_hClientObjImpact, &vCurObjPos);
	LTVector vCurObjDims;
	g_pPhysicsLT->GetObjectDims(m_hClientObjImpact, &vCurObjDims);

	// Check the impact position against an expanded version of the dims so we know we're not entirely off base...
	LTVector vCurObjMin = vCurObjPos - (vCurObjDims * 2.0f);
	LTVector vCurObjMax = vCurObjPos + (vCurObjDims * 2.0f);
	if ((vCurObjMin.x > m_vClientObjImpactPos.x) ||
		(vCurObjMin.y > m_vClientObjImpactPos.y) ||
		(vCurObjMin.z > m_vClientObjImpactPos.z) ||
		(vCurObjMax.x < m_vClientObjImpactPos.x) ||
		(vCurObjMax.y < m_vClientObjImpactPos.y) ||
		(vCurObjMax.z < m_vClientObjImpactPos.z))
	{
		return LTFALSE;
	}

	// Ok, act like we hit them
	IntersectInfo iInfo;
	iInfo.m_hObject = m_hClientObjImpact;
	iInfo.m_hPoly = INVALID_HPOLY;
	iInfo.m_Point = m_vClientObjImpactPos;

	HandleVectorImpact(iInfo, m_vFirePos, m_vClientObjImpactPos);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandleVectorImpact
//
//	PURPOSE:	Handle a vector hitting something
//
//	USED:		only for vectors
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::HandleVectorImpact(IntersectInfo & iInfo, LTVector & vFrom,
                                     LTVector & vTo)
{
	// Get the surface type...

	SurfaceType eSurfType = GetSurfaceType(iInfo);


	// See if we hit an invisible surface...

	if (eSurfType == ST_INVISIBLE)
	{
		if (!CalcInvisibleImpact(iInfo, eSurfType))
		{
			SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
            if (!pSurf) return LTTRUE;

			return UpdateDoVectorValues(*pSurf, 0, iInfo.m_Point, vFrom, vTo);
		}
	}

	// If the fire position is the initial fire position, use the flash
	// position when building the impact special fx...

    LTVector vFirePos = (vFrom.NearlyEquals(m_vFirePos, 0.0f) ? m_vFlashPos : vFrom);

	// Check if we got deflected.
	if( Deflect( iInfo.m_hObject, vFirePos, iInfo.m_Point, iInfo.m_Plane.m_Normal ))
	{
		return LTTRUE;
	}

	// See if we hit an object that should be damaged...

    LTBOOL bHitWorld = IsMainWorld(iInfo.m_hObject);

	if (!bHitWorld && eSurfType != ST_LIQUID)
	{
		ImpactDamageObject(m_hFiredFrom, iInfo.m_hObject);
	}

	AddImpact(iInfo.m_hObject, vFirePos, iInfo.m_Point, iInfo.m_Plane.m_Normal, eSurfType);


	// See if we can shoot through the surface...

	SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurfType);
    if (!pSurf) return LTTRUE;  // Done.

	if (pSurf->bCanShootThrough)
	{
		int nMaxThickness = pSurf->nMaxShootThroughThickness;
		if (nMaxThickness == 0)
		{
			// Special case of always being able to shoot through surface...

			// Calculate new values for next DoVector iteration...

			return UpdateDoVectorValues(*pSurf, 0, iInfo.m_Point, vFrom, vTo);
		}

		// Test if object/wall intersected is thin enough to be shot
		// through...

		// Test object case first...
		// Check if this is a worldmodel.
		bool bHitWorldModel = ( OT_WORLDMODEL == GetObjectType( iInfo.m_hObject ));
		if (!bHitWorldModel && !bHitWorld && iInfo.m_hObject)
		{
			// Test to see if we can shoot through the object...

            LTVector vDims;
			g_pPhysicsLT->GetObjectDims(iInfo.m_hObject, &vDims);

			if (vDims.x*2.0f >= nMaxThickness &&  vDims.y*2.0f >= nMaxThickness &&
				vDims.z*2.0f >= nMaxThickness)
			{
				// Can't shoot through this object...
                return LTTRUE;
			}
		}

		// Determine if we shot through the wall/object...

		HOBJECT hFilterList[] = { m_hFiredFrom, LTNULL };

		IntersectInfo iTestInfo;
		IntersectQuery qTestInfo;

        qTestInfo.m_From = iInfo.m_Point + (m_vDir * (LTFLOAT)(nMaxThickness + 1));
		qTestInfo.m_To   = iInfo.m_Point - m_vDir;

		qTestInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

		qTestInfo.m_FilterFn  = DoVectorFilterFn;
		VectorFilterFnUserData userdata;
		userdata.m_pFilterList = hFilterList;
		userdata.m_pProjectile = this;
		qTestInfo.m_pUserData = &userdata;

        if (g_pLTServer->IntersectSegment(&qTestInfo, &iTestInfo))
		{
			// Calculate new values for next DoVector iteration...

            LTVector vThickness = iTestInfo.m_Point - iInfo.m_Point;
			return UpdateDoVectorValues(*pSurf, vThickness.Mag(), iTestInfo.m_Point, vFrom, vTo);
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::UpdateVectorValues
//
//	PURPOSE:	Update our DoVector values
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::UpdateDoVectorValues(SURFACE & surf, LTFLOAT fThickness,
                                        LTVector vImpactPos, LTVector & vFrom, LTVector & vTo)
{
	// See if we've traveled the distance...

    LTVector vDistTraveled = vImpactPos - m_vFirePos;
    LTFLOAT fDist = m_fRange - vDistTraveled.Mag();
    if (fDist < 1.0f) return LTTRUE;

	// Just dampen based on the bute file values, don't worry about the
	// surface thinkness...

	//DANO: POTENTIAL PROBLEM!!
	// This variable m_fInstDamage gets changed here but
	// used by another function...in effect this variable
	// is getting passed "under the hood".  This can present
	// big problems with the Sequencer!
	AdjustDamage(surf.fBulletDamageDampen);

	fDist *= surf.fBulletRangeDampen;

	int nPerturb = surf.nMaxShootThroughPerturb;

	if (nPerturb)
	{
        LTRotation rRot(m_vDir, LTVector(0.0f, 1.0f, 0.0f));

        LTVector vU, vR, vF;
		vU = rRot.Up();
		vR = rRot.Right();
		vF = rRot.Forward();

        LTFLOAT fRPerturb = ((LTFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;
        LTFLOAT fUPerturb = ((LTFLOAT)GetRandom(-nPerturb, nPerturb))/1000.0f;

		m_vDir += (vR * fRPerturb);
		m_vDir += (vU * fUPerturb);

		m_vDir.Normalize();
	}

	// Make sure we move the from position...

    if (vFrom.NearlyEquals(vImpactPos, 1.0f))
	{
		vFrom += m_vDir;
	}
	else
	{
		vFrom = vImpactPos;
	}

	vTo = vFrom + (m_vDir * fDist);

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::CalcInvisibleImpact
//
//	PURPOSE:	Update the impact value so it ignores invisible surfaces
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::CalcInvisibleImpact(IntersectInfo & iInfo, SurfaceType & eSurfType)
{
	// Since we hit an invisible surface try and find a solid surface that
	// is the real surface of impact.  NOTE:  We assume that the solid
	// surface will have a normal facing basically the opposite direction...

	HOBJECT hFilterList[] = { m_hFiredFrom, LTNULL };

	IntersectInfo iTestInfo;
	IntersectQuery qTestInfo;

	qTestInfo.m_From = iInfo.m_Point + (m_vDir * g_vtInvisibleMaxThickness.GetFloat());
	qTestInfo.m_To   = iInfo.m_Point - m_vDir;

	qTestInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;

	qTestInfo.m_FilterFn  = DoVectorFilterFn;
	VectorFilterFnUserData userdata;
	userdata.m_pFilterList = hFilterList;
	userdata.m_pProjectile = this;
	qTestInfo.m_pUserData = &userdata;

    if (g_pLTServer->IntersectSegment(&qTestInfo, &iTestInfo))
	{
		eSurfType = GetSurfaceType(iTestInfo);

		// If we hit another invisible surface, we're done...

		if (eSurfType != ST_INVISIBLE)
		{
			iInfo = iTestInfo;
            return LTTRUE;
		}
	}

    return LTFALSE;
}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::HandlePotentialHitBoxImpact
//
//	PURPOSE:	Handle a vector hitting a Character's hit box
//
// ----------------------------------------------------------------------- //

LTBOOL CProjectile::HandlePotentialHitBoxImpact(IntersectInfo & iInfo,
                                               LTVector & vFrom)
{
    CCharacterHitBox *pCharHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(iInfo.m_hObject);
    if (!pCharHitBox) return LTFALSE;

	return pCharHitBox->HandleImpact(this, iInfo, m_vDir, vFrom);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::Deflect
//
//	PURPOSE:	Check if we got deflected.
//
// ----------------------------------------------------------------------- //

bool CProjectile::Deflect( HOBJECT hTarget, const LTVector &vFirePos, const LTVector &vImpactPos, const LTVector &vNormal )
{
	// Check if this ammo can be deflected.
	if( !m_pAmmoData->bCanBeDeflected )
		return false;

	// Only character's can deflect.
	CCharacter* pCharacter = dynamic_cast< CCharacter* >( g_pLTServer->HandleToObject( hTarget ));
	if( !pCharacter )
		return false;

	// Check if character is deflecting.
	if( !pCharacter->IsDeflecting( ))
		return false;

	// Get the direction the character is pointing.
	LTRotation rCharacterRot = pCharacter->GetRotationWithPitch( );
	LTVector vCharacterForward = rCharacterRot.Forward( );

	// Get the direction to the projectile from the character.
	LTVector vCharacterPos;

	g_pLTServer->GetObjectPos( hTarget, &vCharacterPos );
	LTVector vDiff = vFirePos - vCharacterPos;
	vDiff.Normalize( );

	// See if the character is pointing in the direction of the projectile.
	float fDot = vCharacterForward.Dot( vDiff );
	float fAngleInRadians = g_vtDeflectAngleRange.GetFloat( ) * MATH_PI / 180.0f;
	float fMinimumDot = ( float )cos( fAngleInRadians );
	if( fDot < fMinimumDot )
		return false;

	SURFACE* pSurface = g_pSurfaceMgr->GetSurface( const_cast< char* >( m_pAmmoData->sDeflectSurfaceType.c_str( )));
	if( pSurface )
		AddImpact( NULL, vFirePos, vImpactPos, vNormal, pSurface->eType, IMPACT_TYPE_IMPACT, true );

	// Projectile deflected.
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CProjectile::ImpactDamageObject
//
//	PURPOSE:	Handle impact damage to an object
//
// ----------------------------------------------------------------------- //

void CProjectile::ImpactDamageObject(HOBJECT hDamager, HOBJECT hObj)
{
	DamageStruct damage;

	damage.hDamager = hDamager;
	damage.vDir		= m_vDir;
	damage.nAmmoId	= m_nAmmoId;

	// Do Instant damage...

	if ( m_eInstDamageType != DT_UNSPECIFIED )
	{
		damage.eType	= m_eInstDamageType;
		damage.fDamage	= m_fInstDamage;

		damage.DoDamage(this, hObj);
	}

	// Do Progressive damage...(if the progressive damage is supposed to
	// happen over time, it will be done in the explosion object)....

	if ( m_eProgDamageType != DT_UNSPECIFIED && m_pAmmoData->fProgDamageLifetime <= 0.0f)
	{
		damage.eType	 = m_eProgDamageType;
		damage.fDamage	 = m_fProgDamage;
		damage.fDuration = m_pAmmoData->fProgDamageDuration;

		damage.DoDamage(this, hObj);
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


			ModelNode eModelNode = pChar->GetModelNodeLastHit();

			if (eModelNode != eModelNodeInvalid)
			{
				HitLocation eLoc = g_pModelButeMgr->GetSkeletonNodeLocation(pChar->GetModelSkeleton(),eModelNode);
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

	SAVE_HOBJECT(m_hFiredFrom);

	SAVE_VECTOR(m_vFlashPos);
    SAVE_VECTOR(m_vFirePos);
    SAVE_VECTOR(m_vDir);
	SAVE_BOOL(m_bSilenced);
    SAVE_BOOL(m_bObjectRemoved);
    SAVE_BOOL(m_bDetonated);
    SAVE_BYTE(m_nWeaponId);
    SAVE_BYTE(m_nAmmoId);
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

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	SAVE_DWORD(dwFlags);

	SAVE_BYTE(m_nTeamId);
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

	LOAD_HOBJECT(m_hFiredFrom);

    LOAD_VECTOR(m_vFlashPos);
    LOAD_VECTOR(m_vFirePos);
    LOAD_VECTOR(m_vDir);
    LOAD_BOOL(m_bSilenced);
    LOAD_BOOL(m_bObjectRemoved);
    LOAD_BOOL(m_bDetonated);
    LOAD_BYTE(m_nWeaponId);
    LOAD_BYTE(m_nAmmoId);
    LOAD_BYTE_CAST(m_eInstDamageType, DamageType);
    LOAD_BYTE_CAST(m_eProgDamageType, DamageType);
    LOAD_FLOAT(m_fInstDamage);
    LOAD_FLOAT(m_fProgDamage);
    LOAD_TIME(m_fStartTime);
    LOAD_FLOAT(m_fLifeTime);
    LOAD_FLOAT(m_fVelocity);
    LOAD_FLOAT(m_fRange);

	m_pWeaponData = g_pWeaponMgr->GetWeapon(m_nWeaponId);
	m_pAmmoData	  = g_pWeaponMgr->GetAmmo(m_nAmmoId);

    LOAD_BOOL(m_bProcessInvImpact);
    LOAD_VECTOR(m_vInvisVel);
	LOAD_VECTOR(m_vInvisNewPos);
	LOAD_BOOL(m_bSetup);
	LOAD_BOOL(m_bCanHitSameProjectileKind);
	LOAD_BOOL(m_bCanTouchFiredFromObj);

	uint32 dwFlags;
	LOAD_DWORD(dwFlags);
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL );

	if( g_pVersionMgr->GetCurrentSaveVersion( ) > CVersionMgr::kSaveVersion__1_2 )
	{
		LOAD_BYTE( m_nTeamId );
	}
}

bool CProjectile::IsMyTeam( HOBJECT hPlayer ) const
{
	bool bMyTeam = false;

	//if projectile wasn't fired by a player, teams are irrelevant...
	if (!IsPlayer(m_hFiredFrom))
		return false;

	// If this is a coop game, then all other players are on my team, everything
	// else isn't.
	if( IsCoopMultiplayerGameType( ))
	{
		bMyTeam = !!IsPlayer( hPlayer );
	}
	// If it's a team game, then check our internal teamid.
	else if( IsTeamGameType( ))
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