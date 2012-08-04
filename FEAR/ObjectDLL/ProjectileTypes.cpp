// ----------------------------------------------------------------------- //
//
// MODULE  : ProjectileTypes.cpp
//
// PURPOSE : Projectile classs - implementation
//
// CREATED : 10/3/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "ProjectileTypes.h"
#include "ServerUtilities.h"
#include "DamageTypes.h"
#include "SurfaceFunctions.h"
#include "SoundMgr.h"
#include "VarTrack.h"
#include "Spawner.h"
#include "Character.h"
#include "PlayerObj.h"
#include "CharacterHitBox.h"
#include "ObjectMsgs.h"
#include "ServerSoundMgr.h"
#include "GameServerShell.h"
#include "AIStimulusMgr.h"
#include "WeaponFireInfo.h"
#include "AIUtils.h"
#include "CharacterMgr.h"
#include "AI.h"
#include "AISoundMgr.h"
#include "WeaponItems.h"
#include "iltphysicssim.h"
#include "FXDB.h"
#include "SoundDB.h"
#include "FxDefs.h"
#include "PhysicsUtilities.h"
#include "CollisionsDB.h"
#include "WorldModel.h"
#include "GameModeMgr.h"
#include "ServerPhysicsCollisionMgr.h"
#include "CharacterDB.h"
#include "BroadcastDB.h"
#include "ltintersect.h"
#include "ServerConnectionMgr.h"

LINKFROM_MODULE( ProjectileTypes );


VarTrack g_vtDebugRemote;

CTList<CGrenade*> g_lstGrenades;


BEGIN_CLASS(CGrenade)
END_CLASS_FLAGS(CGrenade, CProjectile, CF_HIDDEN, "")

CMDMGR_BEGIN_REGISTER_CLASS( CGrenade )
CMDMGR_END_REGISTER_CLASS( CGrenade, CProjectile )

BEGIN_CLASS(CGrenadeProximity)
END_CLASS_FLAGS(CGrenadeProximity, CGrenade, CF_HIDDEN, "")

CMDMGR_BEGIN_REGISTER_CLASS( CGrenadeProximity )
	ADD_MESSAGE( ACTIVATE,	1,	NULL,	MSG_HANDLER( CGrenadeProximity, HandleActivateMsg ),	"ACTIVATE", "When sent by a player, attempts to put the grenade into the players inventory.", "TODO:CMDEXP" )
CMDMGR_END_REGISTER_CLASS( CGrenadeProximity, CGrenade )

BEGIN_CLASS(CSpear)
END_CLASS_FLAGS(CSpear, CProjectile, CF_HIDDEN, "")

CMDMGR_BEGIN_REGISTER_CLASS( CSpear )
CMDMGR_END_REGISTER_CLASS( CSpear, CProjectile )

BEGIN_CLASS(CRemoteCharge)
END_CLASS_FLAGS(CRemoteCharge, CGrenade, CF_HIDDEN, "")

CMDMGR_BEGIN_REGISTER_CLASS( CRemoteCharge )
	ADD_MESSAGE( ACTIVATE,	1,	NULL,	MSG_HANDLER( CRemoteCharge, HandleActivateMsg ),	"ACTIVATE", "When sent by a player, attempts to put the grenade into the players inventory.", "TODO:CMDEXP" )
CMDMGR_END_REGISTER_CLASS( CRemoteCharge, CGrenade )

// Hide this object in Dark.
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_PROXIMITYGRENADE CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	// JSC this has to be set to a value or else the parameter will be invalid
	#define CF_HIDDEN_PROXIMITYGRENADE 0

#endif

BEGIN_CLASS(ProximityGrenade)
END_CLASS_FLAGS(ProximityGrenade, GameBase, CF_HIDDEN_PROXIMITYGRENADE, "An armed proximity grenade")

CMDMGR_BEGIN_REGISTER_CLASS( ProximityGrenade )
CMDMGR_END_REGISTER_CLASS( ProximityGrenade, GameBase )


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	GrenadeFilterFn()
//
//	PURPOSE:	Filter function used by grenade intersect calls.
// ----------------------------------------------------------------------- //

static inline bool GrenadeFilterFn(HOBJECT hObj, void *pUserData)
{
	if (!hObj) return false;

	uint32 nFlags = 0;
	g_pLTServer->Common()->GetObjectFlags( hObj, OFT_Flags, nFlags );
	if( nFlags & FLAG_SOLID )
		return true;
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::CGrenade
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGrenade::CGrenade() : CProjectile()
	, m_hCollisionNotifier( INVALID_PHYSICS_COLLISION_NOTIFIER )
	, m_bDetonateOnImpact ( true )
	, m_bRotatedToRest ( false )
	, m_bAddToGrenadeList ( true )
	, m_hRigidBody ( INVALID_PHYSICS_RIGID_BODY)
	, m_fReflection (0.3f)
{
	m_vDims.Init(5.0f, 5.0f, 5.0f);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::~CGrenade
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGrenade::~CGrenade()
{
	// Just to be safe...
	if( m_bAddToGrenadeList )
		g_lstGrenades.Remove(this);

	if( m_hCollisionNotifier != INVALID_PHYSICS_COLLISION_NOTIFIER )
	{
		g_pLTServer->PhysicsSim()->ReleaseCollisionNotifier( m_hCollisionNotifier );
		m_hCollisionNotifier = INVALID_PHYSICS_COLLISION_NOTIFIER;
	}

	ReleaseRigidBody();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::CreateRigidBody
//
//	PURPOSE:	create the shape and body for the physics sim...
//
// ----------------------------------------------------------------------- //

void CGrenade::CreateRigidBody()
{
	ReleaseRigidBody();

	EPhysicsGroup eGroup;
	uint32 nSystem;
	LTRigidTransform tBody;
	float fFriction, fCOR;

	HPHYSICSRIGIDBODY hRigidBody;
	HPHYSICSSHAPE hShape;
	if (LT_OK == g_pLTServer->PhysicsSim()->GetModelRigidBody( m_hObject, 0, hRigidBody ))
	{
		g_pLTServer->PhysicsSim()->GetRigidBodyTransform(hRigidBody,tBody);
		g_pLTServer->PhysicsSim()->GetRigidBodyCollisionInfo(hRigidBody, eGroup, nSystem);
		g_pLTServer->PhysicsSim()->GetRigidBodyFriction(hRigidBody,fFriction);
		g_pLTServer->PhysicsSim()->GetRigidBodyCOR(hRigidBody,fCOR);

		g_pLTServer->PhysicsSim()->GetRigidBodyShape(hRigidBody,hShape);

		m_hRigidBody = g_pLTServer->PhysicsSim()->CreateRigidBody(hShape,tBody,false,eGroup,nSystem,fFriction,fCOR);

		g_pLTServer->PhysicsSim()->ReleaseShape(hShape);
		g_pLTServer->PhysicsSim()->ReleaseRigidBody(hRigidBody);

	}


	//get a rigid transform of the game object
	LTRigidTransform tObject;
	g_pLTServer->GetObjectTransform(m_hObject,&tObject);
	tObject.m_rRot = tBody.m_rRot;

	//get the offest transform
	m_tBodyOffset.Difference(tBody,tObject);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::CreateRigidBody
//
//	PURPOSE:	free the shape and body for the physics sim...
//
// ----------------------------------------------------------------------- //

void CGrenade::ReleaseRigidBody()
{
	if (m_hRigidBody != INVALID_PHYSICS_RIGID_BODY)
	{
		g_pLTServer->PhysicsSim()->RemoveRigidBodyFromSimulation(m_hRigidBody);
		g_pLTServer->PhysicsSim()->ReleaseRigidBody(m_hRigidBody);
		m_hRigidBody = INVALID_PHYSICS_RIGID_BODY;
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Setup
//
//	PURPOSE:	Setup the grenade...
//
// ----------------------------------------------------------------------- //

bool CGrenade::Setup(CWeapon const* pWeapon, WeaponFireInfo const& info)
{
	// Initialize our parent...
	if (!CProjectile::Setup(pWeapon, info)) return false;

	if (IsPlayer(m_Shared.m_hFiredFrom) && m_bDetonateOnImpact && GameModeMgr::Instance( ).m_grbUseTeams)
	{
		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_Shared.m_hFiredFrom ));
		HRECORD hRec = DATABASE_CATEGORY( Broadcast ).GetRecordByName( "Grenade");

		PlayerBroadcastInfo pbi;
		pbi.nBroadcastID = DATABASE_CATEGORY( Broadcast ).GetRandomLineID( hRec );
		pbi.bForceClient = true;
		pbi.nPriority = DATABASE_CATEGORY( Broadcast ).GETRECORDATTRIB( hRec, Priority);

		pPlayer->HandleBroadcast( pbi );
	}

	return SetupRigidBody();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Setup
//
//	PURPOSE:	Set up a grenade with the minimum required info
//
// ----------------------------------------------------------------------- //

bool CGrenade::Setup(HAMMO hAmmo, LTRigidTransform const &trans)
{
	// Initialize our parent...
	if (!CProjectile::Setup(hAmmo, trans)) return false;

	return SetupRigidBody();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::SetupRigidbody
//
//	PURPOSE:	Setup the grenade...
//
// ----------------------------------------------------------------------- //

bool CGrenade::SetupRigidBody()
{
	CreateRigidBody();
	if (m_hRigidBody == INVALID_PHYSICS_RIGID_BODY)
		return false;

	// We don't want to be in the normal physics simulation so clear the
	// physics related flags...
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE | FLAG_GRAVITY | FLAG_TOUCH_NOTIFY | FLAG_SOLID);
	// Clear our velocity...
	LTVector vVel(0, 0, 0);
	g_pPhysicsLT->SetVelocity(m_hObject, vVel);

	// Calculate our original velocity...
	vVel = m_vDir * m_fVelocity;

	// Take us out of the physics simulation, we'll use our own rigid body
	PhysicsUtilities::SetPhysicsWeightSet(m_hObject, PhysicsUtilities::WEIGHTSET_NONE, false);

	g_pLTServer->PhysicsSim()->SetRigidBodySolid(m_hRigidBody,true);
	g_pLTServer->PhysicsSim()->SetRigidBodyVelocity(m_hRigidBody, vVel);


	LTVector vAng( GetRandom(-10.0f,10.0f),GetRandom(-10.0f,10.0f),GetRandom(-10.0f,10.0f));
	g_pLTServer->PhysicsSim()->SetRigidBodyAngularVelocity(m_hRigidBody,vAng);

	RegisterCollisionNotifier(m_hRigidBody);

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Detonate
//
//	PURPOSE:	Go boom
//
// ----------------------------------------------------------------------- //

void CGrenade::Detonate(HOBJECT hObj)
{
	// We're blowing up, take us out of the grenade list
	if( m_bAddToGrenadeList )
		g_lstGrenades.Remove(this);

	CProjectile::Detonate(hObj);
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGrenade::CollisionNotifier
//
//  PURPOSE:	Register class specific collision notifiers
//
// ----------------------------------------------------------------------- //
void CGrenade::RegisterCollisionNotifier(HPHYSICSRIGIDBODY hRigidBody)
{
	m_hCollisionNotifier = g_pLTServer->PhysicsSim()->RegisterCollisionNotifier(hRigidBody, CollisionNotifier, this );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGrenade::CollisionNotifier
//
//  PURPOSE:	Collision notifier for CGrenade class rigidbodies...
//
// ----------------------------------------------------------------------- //

void CGrenade::CollisionNotifier( HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
									  const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
									  float fVelocity, bool& bIgnoreCollision, void* pUser )
{

	// Let the individual grenade handle the collision...
	CGrenade *pGrenade = static_cast<CGrenade*>(pUser);

	HOBJECT hObj1 = NULL;
	HOBJECT hObj2 = NULL;
		
	g_pLTServer->PhysicsSim()->GetRigidBodyObject(hBody1,hObj1);
	g_pLTServer->PhysicsSim()->GetRigidBodyObject(hBody2,hObj2);

	if (hBody1 == pGrenade->GetRigidbody())
	{
		hObj1 = pGrenade->m_hObject;
		if (hObj2 == pGrenade->GetFiredFrom())
		{
			bIgnoreCollision = true;
			return;
		}
	}
	else if (hBody2 == pGrenade->GetRigidbody())
	{
		hObj2 = pGrenade->m_hObject;
		if (hObj1 == pGrenade->GetFiredFrom())
		{
			bIgnoreCollision = true;
			return;
		}
	}

	CollisionData collisionData;
	collisionData.hBodyA = hBody1;
	collisionData.hBodyB = hBody2;
	collisionData.hObjectA = hObj1;
	collisionData.hObjectB = hObj2;
	g_pLTBase->PhysicsSim()->IsRigidBodyPinned(hBody1, collisionData.bIsPinnedA);
	g_pLTBase->PhysicsSim()->IsRigidBodyPinned(hBody2, collisionData.bIsPinnedB);
	collisionData.vCollisionPt = vCollisionPt;
	collisionData.vCollisionNormal = vCollisionNormal;
	collisionData.fVelocity = fVelocity;
	ServerPhysicsCollisionMgr::Instance().HandleRigidBodyCollision( collisionData );

	if (hBody1 == pGrenade->GetRigidbody())
	{
		//the grenade is obj1 and it hit obj2
		pGrenade->HandleCollision(hObj2,hBody2,vCollisionPt,vCollisionNormal,fVelocity,bIgnoreCollision, NULL, false);
	}
	else if (hBody2 == pGrenade->GetRigidbody())
	{
		//the grenade is obj2 and it hit obj1
		pGrenade->HandleCollision(hObj1,hBody1,vCollisionPt,vCollisionNormal,fVelocity,bIgnoreCollision, NULL, false);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::HandleCollision
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CGrenade::HandleCollision(HOBJECT hObjHit, HPHYSICSRIGIDBODY hBodyHit, 
							   const LTVector& vCollisionPt, const LTVector& vCollisionNormal, 
							   float fVelocity, bool& bIgnoreCollision, IntersectInfo* pInfo,
							   bool bFakeBounce )
{

	if (!hObjHit) return;

	if (hObjHit == m_Shared.m_hFiredFrom && (SimulationTimer::Instance().GetTimerAccumulatedS() < (m_fStartTime + 0.25f)))
	{
		bIgnoreCollision = true;
		return;
	}

	// Check the ignore projectiles flag.

	uint32 dwUsrFlags;
	g_pCommonLT->GetObjectFlags(hObjHit, OFT_User, dwUsrFlags);
	if (dwUsrFlags & USRFLG_IGNORE_PROJECTILES)
	{
		bIgnoreCollision = true;
		return;
	}

	// If we hit a character detonate. 
	// (i.e., only act like a timed grenade if we don't hit someone)

	if (m_bDetonateOnImpact && ( IsCharacter(hObjHit) || IsCharacterHitBox(hObjHit) ))
	{

		if( GameModeMgr::Instance( ).m_grbFriendlyFire || !IsMyTeam( hObjHit ))
		{
			Detonate(hObjHit);
			return;
		}
	}

	// We hit the world...
	if( !IsMainWorld(hObjHit) && GetObjectType( hObjHit ) == OT_WORLDMODEL )
	{
		WorldModel* pWM = WorldModel::DynamicCast( hObjHit );
		if (!pWM || pWM->IsDestroyed()) 
		{
			bIgnoreCollision = true;
		}
	}

	if( !bIgnoreCollision && bFakeBounce )
	{
		LTVector vVel;
		g_pLTServer->PhysicsSim()->GetRigidBodyVelocity(m_hRigidBody,vVel);

		//reflect the velocity over the normal
		vVel -= vCollisionNormal * (2.0f * vVel.Dot(vCollisionNormal));

		vVel *= m_fReflection;

		LTRigidTransform tRTrans;
		tRTrans.m_vPos = vCollisionPt;
		tRTrans.m_rRot.Init();
		tRTrans *= m_tBodyOffset.GetInverse();
		g_pLTServer->PhysicsSim()->TeleportRigidBody(m_hRigidBody,tRTrans);

		g_pLTServer->Physics()->SetVelocity(m_hObject, vVel);
		g_pLTServer->PhysicsSim()->SetRigidBodyVelocity(m_hRigidBody,vVel);
		g_pLTServer->PhysicsSim()->SetRigidBodyAngularVelocity(m_hRigidBody,LTVector(0.0f,0.0f,0.0f));
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::UpdateGrenade()
//
//	PURPOSE:	Update the grenade...
//
// ----------------------------------------------------------------------- //

bool CGrenade::UpdateGrenade()
{
    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if (!m_bRotatedToRest)
	{
		bool bMoving = false;
		LTVector vVel;
		LTVector vAngVel;

		g_pLTServer->PhysicsSim()->GetRigidBodyVelocity(m_hRigidBody,vVel);
		g_pLTServer->PhysicsSim()->GetRigidBodyAngularVelocity(m_hRigidBody,vAngVel);

		bMoving = vVel.MagSqr() > 5.0f || vAngVel.MagSqr() > 0.5f;

		
		if (bMoving)
		{
			LTRigidTransform tRTrans;
			g_pLTServer->PhysicsSim()->GetRigidBodyTransform(m_hRigidBody,tRTrans);
			LTRigidTransform tNewPos = tRTrans * m_tBodyOffset;
			g_pLTServer->SetObjectRotation(m_hObject, tNewPos.m_rRot);

			g_pLTServer->Physics()->SetVelocity(m_hObject, vVel);
			g_pLTServer->Physics()->MoveObject(m_hObject, tNewPos.m_vPos, 0);

			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			// Check if the grenade object didn't make it to the rigidbody object.
			if (!vPos.NearlyEquals(tNewPos.m_vPos,10.0f))
			{
				IntersectQuery query;
				IntersectInfo info;
				query.m_From		= vPos;
				query.m_To			= tNewPos.m_vPos;
				query.m_Flags		= INTERSECT_HPOLY | IGNORE_NONSOLID | INTERSECT_OBJECTS;
				query.m_FilterFn	= GrenadeFilterFn;

				//test this segment against the world to see what we bounce against
				if (g_pLTBase->IntersectSegment(query, &info))
				{
					LTVector vNormal;
					// get the normal of the plane we impacted with
					if (info.m_Plane.m_Normal.MagSqr() == 0.0f)
					{
						vNormal = -vVel;
					}
					else
					{
						vNormal = info.m_Plane.m_Normal;
					}

					vNormal.Normalize();
					bool bIgnoreCollision = false;
					HandleCollision( info.m_hObject, NULL, info.m_Point, vNormal, vVel.Mag( ), bIgnoreCollision, &info, true );
				}
			}
		}
		else
		{
			RotateToRest();
		}
	}

	// Continue processing...
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CGrenade::RotateToRest()
{
	// Record this grenade for AI's to be wary of

	if ( !m_bRotatedToRest && m_bAddToGrenadeList )
	{
		g_lstGrenades.Add(this);
	}

	// At rest

	m_bRotatedToRest = true;

	// Turn off any animations that are playing (e.g., spin)...

	g_pModelLT->SetPlaying(m_hObject, MAIN_TRACKER, false);
	g_pLTServer->Physics()->SetVelocity(m_hObject, LTVector(0,0,0));

	
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CGrenade::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			// Only allow CProjectile to process the MID_UPDATE message
			// if specified...
			if (UpdateGrenade())
			{
				return CProjectile::EngineMessageFn(messageID, pData, fData);
			}
			else
			{
				return 0;
			}
		}
		break;

		case MID_PRECREATE:
			{
				uint32 nRet = CProjectile::EngineMessageFn(messageID, pData, fData);

				if( fData != PRECREATE_SAVEGAME )
				{
					ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;
					if (pStruct)
					{
						pStruct->m_eGroup = PhysicsUtilities::ePhysicsGroup_UserProjectile;
					}
				}

				return nRet;
			}
			break;

		default : break;
	}

	return CProjectile::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CGrenade::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	CProjectile::Save(pMsg, dwSaveFlags);

	SAVE_BOOL(m_bRotatedToRest);
	SAVE_BOOL(m_bAddToGrenadeList);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenade::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CGrenade::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	CProjectile::Load(pMsg, dwLoadFlags);

	LOAD_BOOL(m_bRotatedToRest);
	LOAD_BOOL(m_bAddToGrenadeList);

	if ( m_bRotatedToRest && m_bAddToGrenadeList )
	{
		g_lstGrenades.Add(this);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::CSpear
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CSpear::CSpear()
:	CProjectile		( ),
	m_hClassData	( NULL )
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::~CSpear
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CSpear::~CSpear()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CSpear::Setup
//
//  PURPOSE:	Setup the spear...
//
// ----------------------------------------------------------------------- //

bool CSpear::Setup( const CWeapon *pWeapon, const WeaponFireInfo &info )
{
	// Let the base class setup first...

	if( !CProjectile::Setup( pWeapon, info ))
		return false;

	if( !m_Shared.m_hAmmo )
		return false;

	
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
	// Get the class data...

	if (!hProjectileFX) return false;

	m_hClassData = g_pFXDB->GetSpearClassData(hProjectileFX);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSpear::HandleImpact
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CSpear::HandleImpact(HOBJECT hObj)
{

	// [KLS 8/18/02] - Get the object's position and rotation before calling
	// parent's HandleImpact as it may change these values...
    LTVector vPos, vVel;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);


	// [RP] 7/29/02 Call the parent projectile's HandleImapct() first so it will process the damage.
	//		This will enable us to see if the projectile has killed a character if it impacted on one.
	//		Calling this will remove the projectile object but since it won't actually get removed untill
	//		the next update getting the objects data is still valid...

	CProjectile::HandleImpact(hObj);

	if( !m_Shared.m_hAmmo )
		return;
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));

	if( !hProjectileFX || !m_hClassData )
	{
		return;
	}


	CollisionInfo info;
    g_pLTServer->GetLastCollision(&info);


	// Should we break the spear?

	enum SpearAction
	{
		eSpearActionBreak,
		eSpearActionStickWorld,
		eSpearActionStickAI,
		eSpearActionStickPlayer
	};

	SpearAction eSpearAction = eSpearActionBreak;


	// Optimization:  AI spears always break...
	if (IsPlayer(m_Shared.m_hFiredFrom))
	{

		bool bDoNormalChecks = true;

		// First see if we hit a character and killed them in a "wall stick able" way
		// If so we want to keep the spear stuck in them so it has a chance to stick
		// them to a wall..

		if ( g_pFXDB->GetBool(m_hClassData,FXDB_bCanWallStick) && IsCharacter(hObj))
		{
			CCharacter* pCharacter = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));
			if (pCharacter && !pCharacter->IsAlive())
			{
				ModelsDB::HNODE hModelNode = pCharacter->GetModelNodeLastHit();
				if (NODEFLAG_WALLSTICK & g_pModelsDB->GetNodeFlags( hModelNode ))
				{
					bDoNormalChecks = false;
					eSpearAction = (IsAI(hObj) ? eSpearActionStickAI : eSpearActionStickPlayer);
				}
			}
		}
		
		if (bDoNormalChecks)
		{
			if (GetRandom(0.0, 1.0f) > g_pFXDB->GetFloat(m_hClassData,FXDB_fStickPercent) )
			{
				// Randomly break even if we could sometimes stick...
		
				eSpearAction = eSpearActionBreak;
			}
			else if (IsMainWorld(hObj))
			{
 				// Calculate where we really hit the world...

				LTVector vCurVel, vP0, vP1;
				g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

				vP1 = vPos;
				vCurVel = vVel * g_pLTServer->GetFrameTime();
				vP0 = vP1 - vCurVel;
				vP1 += vCurVel;

				float fDot1 = info.m_Plane.DistTo(vP0);
				float fDot2 = info.m_Plane.DistTo(vP1);

				if (fDot1 < 0.0f && fDot2 < 0.0f || fDot1 > 0.0f && fDot2 > 0.0f)
				{
					vPos = vP1;
				}
				else
				{
					vPos = vP0.Lerp(vP1, -fDot1 / (fDot2 - fDot1));
				}

				// Set our new "real" pos...

				g_pLTServer->SetObjectPos(m_hObject, vPos);

				eSpearAction = eSpearActionStickWorld;
			}
			else if (IsMoveable(hObj))
			{
				if (IsAI(hObj))
				{
					// Attach to a AI
					eSpearAction = eSpearActionStickAI;
				}
				else if (IsPlayer(hObj))
				{
					// Attach to a Player
					eSpearAction = eSpearActionStickPlayer;
				}
				else
				{
					// Could probably come up with a way to attach to moveable
					// non-character objects (like doors), but it is much easier
					// to just break it ;)...

					eSpearAction = eSpearActionBreak;
				}
			}
		}
	}



	// If the surface is too hard, the spear will just break when
	// it hits it...

	const char *pszAmmoName = g_pWeaponDB->GetRecordName( m_Shared.m_hAmmo );

	SurfaceType eSurf = GetSurfaceType(info);
	HSURFACE hSurf = g_pSurfaceDB->GetSurface(eSurf);
	if ((eSpearActionBreak == eSpearAction) || ((eSpearActionStickWorld == eSpearAction) && hSurf && g_pSurfaceDB->GetFloat(hSurf,SrfDB_Srf_fHardness) > 0.5f))
	{
		return;
	}

	// Create the Spear powerup...

	char szSpawn[512];
	LTSNPrintF( szSpawn, ARRAY_LEN(szSpawn), "AmmoBox MPRespawn 0;AmmoType1 %s;AmmoCount1 1;Filename %s;Material %s",
		pszAmmoName, g_pFXDB->GetString(hProjectileFX,FXDB_sModel),	g_pFXDB->GetString(hProjectileFX,FXDB_sMaterial) );

	float fScale = g_pFXDB->GetFloat(hProjectileFX,FXDB_fModelScale);

	// Make sure the spear sticks out a little ways...

	vVel.Normalize();
	vPos -= (vVel * fScale / 2.0f);

	BaseClass* pClass = SpawnObject(szSpawn, LTVector(-10000,-10000,-10000), rRot);

	if (pClass)
	{
		g_pLTServer->SetObjectScale(pClass->m_hObject, fScale);

		LTVector vDims;
		g_pPhysicsLT->GetObjectDims(pClass->m_hObject, &vDims);
		vDims.x *= fScale;
		vDims.y *= fScale;
		vDims.z *= fScale;

		LTVector vDimsScale = g_pFXDB->GetVector3(m_hClassData,FXDB_vDimsScale);
		vDims.x *= vDimsScale.x;
		vDims.y *= vDimsScale.y;
		vDims.z *= vDimsScale.z;

		// Make sure our dims get propagated to the clients...
		g_pCommonLT->SetObjectFlags(pClass->m_hObject, OFT_Flags2, FLAG2_SERVERDIMS, FLAG2_SERVERDIMS);
		g_pPhysicsLT->SetObjectDims(pClass->m_hObject, &vDims, 0);

		// We don't want our pickups animating
		
		g_pModelLT->SetPlaying( pClass->m_hObject, MAIN_TRACKER, false );

		if ( eSpearActionStickAI == eSpearAction || eSpearActionStickPlayer == eSpearAction )
		{
			g_pCommonLT->SetObjectFlags(pClass->m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
			g_pCommonLT->SetObjectFlags(pClass->m_hObject, OFT_Flags, 0, FLAG_TOUCH_NOTIFY);

			if ( eSpearActionStickPlayer == eSpearAction )
			{
				g_pCommonLT->SetObjectFlags(pClass->m_hObject, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3);
			}

			// Try to attach it to the character

			CCharacter* pCharacter = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(hObj));
			if( pCharacter )
			{
				pCharacter->AddSpear(pClass->m_hObject, pCharacter->GetModelNodeLastHit(), rRot, g_pFXDB->GetBool(m_hClassData,FXDB_bCanWallStick));
			}
		}
		else // ( eSpearActionStickWorld == eSpearAction )
		{
			// Move it to the right position in the world
			g_pLTServer->SetObjectPos(pClass->m_hObject, vPos);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProximity::CGrenadeProximity
//
//	PURPOSE:	Static data of grenade list.
//
// ----------------------------------------------------------------------- //

CGrenadeProximity::CGrenadeProximityList CGrenadeProximity::m_lstGrenadeProximity;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProximity::CGrenadeProximity
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGrenadeProximity::CGrenadeProximity() : 
	CGrenade(),
	m_hClassData		( NULL ),
	m_eState			(eUnarmed),
	m_bPickedUp			(false)
	
{
	m_bDetonateOnImpact	 = false;
	m_dwFlags |= FLAG_MODELKEYS;
	m_bArmModelKeyString = false;

	m_lstGrenadeProximity.push_back( this );
}

CGrenadeProximity::~CGrenadeProximity()
{
	// Erase this instance from the list.
	for( CGrenadeProximityList::iterator iter = m_lstGrenadeProximity.begin( ); iter != m_lstGrenadeProximity.end( ); iter++ )
	{
		if( *iter == this )
		{
			m_lstGrenadeProximity.erase( iter );
			break;
		}
	}
}



// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGrenadeProximity::Setup
//
//  PURPOSE:	Setup the prox grenade...
//
// ----------------------------------------------------------------------- //

bool CGrenadeProximity::Setup( const CWeapon *pWeapon, const WeaponFireInfo &info )
{
	// Let the base class setup first...
	if( !CGrenade::Setup( pWeapon, info ))
		return false;

	return SetupProx();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGrenadeProximity::Setup
//
//  PURPOSE:	Setup the prox grenade...
//
// ----------------------------------------------------------------------- //

bool CGrenadeProximity::Setup( HAMMO hAmmo, LTRigidTransform const &trans )
{
	// Let the base class setup first...
	if( !CGrenade::Setup( hAmmo, trans ))
		return false;

	return SetupProx();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGrenadeProximity::Setup
//
//  PURPOSE:	Setup the prox grenade...
//
// ----------------------------------------------------------------------- //

bool CGrenadeProximity::SetupProx( )
{
	if( !m_Shared.m_hAmmo )
		return false;

	// Set us up to use the physics simulation...
	m_nFlipTries = 3;
	m_bFlipping = false;
	LTVector vAng( GetRandom(-10.0f,10.0f),60.0f,GetRandom(-10.0f,10.0f));
	g_pLTServer->PhysicsSim()->SetRigidBodyAngularVelocity(m_hRigidBody,vAng);


	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
	// Get the class data...

	if (!hProjectileFX) return false;

	m_hClassData = g_pFXDB->GetProximityClassData(hProjectileFX);

	if (IsPlayer(m_Shared.m_hFiredFrom))
	{
		CPlayerObj* pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_Shared.m_hFiredFrom ));
		if (pPlayer)
		{
			GameClientData* pGameClientData = ServerConnectionMgr::Instance().GetGameClientData( pPlayer->GetClient( ));
			if( pGameClientData)
			{
				pGameClientData->AddProximityMine(m_hObject);
			}
		}
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProximity::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CGrenadeProximity::RotateToRest()
{

	m_bFlipping = false;
	LTRigidTransform tRTrans;
	g_pLTServer->PhysicsSim()->GetRigidBodyTransform(m_hRigidBody,tRTrans);
	LTVector vUp = tRTrans.m_rRot.Up();

	//we're too turned and we haven't given up yet, try to straighten out
	if (vUp.y < 0.75f && m_nFlipTries > 0)
	{
		m_bFlipping = true;
		--m_nFlipTries;
		float fP =  GetConsoleFloat("FlipPop",300.0f);
		LTVector vPop(0.0f,fP,0.0f);
		vPop -= vUp * (fP / 2.0f);
		if( LTIsNaN( vPop ) || vPop.MagSqr() > 1000000.0f * 1000000.0f )
		{
			LTERROR( "Invalid impulse detected." );
			vPop.Init( 0.0f, 10.0f, 0.0f );
		}
		LTVector vPopPos = tRTrans.m_vPos + (vUp * 10.0f) + LTVector(10.0f,0.0f,0.0f);

		g_pLTServer->PhysicsSim()->ApplyRigidBodyImpulseWorldSpace(m_hRigidBody, vPopPos,vPop);
	}


	//we need to turn ourselves over, so we're not at rest yet
	if (m_bFlipping)
	{
		return;
	}

	//we're oriented OK, or we've given up, so take us out of the simulation...
	g_pLTServer->PhysicsSim()->GetRigidBodyTransform(m_hRigidBody,tRTrans);
	ReleaseRigidBody();

	LTRigidTransform tNewPos = tRTrans * m_tBodyOffset;
	g_pLTServer->SetObjectTransform(m_hObject,tNewPos);
	g_pLTServer->Physics()->SetVelocity(m_hObject, LTVector(0,0,0));

	// We want to be in the normal physics simulation so again so set the
	// physics related flags...
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_TOUCH_NOTIFY | FLAG_SOLID, FLAG_TOUCH_NOTIFY | FLAG_SOLID);
	
	// Record this grenade for AI's to be wary of
	if ( !m_bRotatedToRest && m_bAddToGrenadeList )
	{
		g_lstGrenades.Add(this);
	}

	// At rest
	m_bRotatedToRest = true;

	LTRESULT res;

	//play the arming animation
	HMODELANIM hAni = g_pLTBase->GetAnimIndex(m_hObject, g_pFXDB->GetString(m_hClassData,FXDB_sArmAnimation));
	
	res = g_pLTServer->GetModelLT()->SetCurAnim( m_hObject, MAIN_TRACKER, hAni, false);
	res = g_pLTServer->GetModelLT()->SetPlaying( m_hObject, MAIN_TRACKER, true );
	res = g_pLTServer->GetModelLT()->SetLooping( m_hObject, MAIN_TRACKER, false );

	// turn off the delay visible flag so that new clients will be able to see this proxy
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_DELAYCLIENTVISIBLE);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProximity::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CGrenadeProximity::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
	case MID_UPDATE:
		{
			if( m_bArmModelKeyString )
			{
				m_bArmModelKeyString = false;
				Arm( );
			}
		}
		break;
	}

	return CGrenade::EngineMessageFn(messageID, pData, fData);
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGrenadeProximity::HandleModelString
//
//  PURPOSE:	Handle reaching a frame string...
//
// ----------------------------------------------------------------------- //

void CGrenadeProximity::HandleModelString( ArgList *pArgList )
{
	static CParsedMsg::CToken s_cTok_KEY_BUTE_SOUND ( "BUTE_SOUND_KEY" );
	static CParsedMsg::CToken s_cTok_KEY_ARM ( "ARM" );
	static CParsedMsg::CToken s_cTok_KEY_DETONATE ( "FIRE" );
	static CParsedMsg::CToken s_cTok_KEY_LAUNCH ( "LAUNCH" );
	static CParsedMsg::CToken s_cTok_KEY_FX ( "FX" );


	if (!pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	CParsedMsg::CToken tok( pKey );

	if ( tok == s_cTok_KEY_BUTE_SOUND )
	{
		if( pArgList->argc > 1 && pArgList->argv[1] )
		{
			HRECORD hSR = g_pSoundDB->GetSoundDBRecord( pArgList->argv[1] );
			g_pServerSoundMgr->PlayDBSoundFromObject( m_hObject, hSR,
				-1.0f, SOUNDPRIORITY_MISC_LOW, 0,
				SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
				DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
		}
	}
	else if (tok == s_cTok_KEY_ARM) 
	{
		// Do the arm later since we can't change anims in a keystring callback.
		m_bArmModelKeyString = true;
	}
	else if (tok == s_cTok_KEY_LAUNCH) 
	{
		LTVector vVel(0.0f,g_pFXDB->GetFloat(m_hClassData,FXDB_fPopUpVelocity),0.0f);
		g_pPhysicsLT->SetVelocity(m_hObject, vVel);
	}
	else if (tok == s_cTok_KEY_FX) 
	{
		if( pArgList->argc > 1 && pArgList->argv[1] )
		{
			// We can turn on (create) non-looping fx as often as we want...
			LTRigidTransform tTransform;
			g_pLTServer->GetObjectTransform(m_hObject, &tTransform);

			CAutoMessage cMsg;
			cMsg.Writeuint8(SFX_CLIENTFXGROUPINSTANT);
			cMsg.WriteString(pArgList->argv[1]);
			cMsg.Writebool( false ); //not looping
			cMsg.Writebool( false ); //smooth shutdown
			cMsg.Writebool( false ); // No special parent.
			cMsg.WriteLTVector(tTransform.m_vPos);
			cMsg.WriteCompLTRotation(tTransform.m_rRot);
			cMsg.Writebool( false ); // No target.
			g_pLTServer->SendSFXMessage(cMsg.Read(), 0);
		}
	}
	else if (tok == s_cTok_KEY_DETONATE) 
	{
		Detonate(NULL);
	}
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGrenadeProximity::Arm
//
//  PURPOSE:	Arm the grenade
//
// ----------------------------------------------------------------------- //

void CGrenadeProximity::Arm()
{
	m_eState = eArmed;

	OverrideClientFX(	g_pFXDB->GetString(m_hClassData,FXDB_sArmedFX,0),
						g_pFXDB->GetString(m_hClassData,FXDB_sArmedFX,1),
						g_pFXDB->GetString(m_hClassData,FXDB_sArmedFX,2)
					);

	//play the armed animation
	HMODELANIM hAni = g_pLTBase->GetAnimIndex(m_hObject, g_pFXDB->GetString(m_hClassData,FXDB_sArmedAnimation));
	g_pLTServer->GetModelLT()->SetCurAnim( m_hObject, MAIN_TRACKER, hAni, true);
	g_pLTServer->GetModelLT()->SetPlaying( m_hObject, MAIN_TRACKER, true );
	g_pLTServer->GetModelLT()->SetLooping( m_hObject, MAIN_TRACKER, true );

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGrenadeProximity::Activate
//
//  PURPOSE:	Start the detonation...
//
// ----------------------------------------------------------------------- //

void CGrenadeProximity::Activate()
{
	m_eState = eActivated;

	//play the activation animation
	HMODELANIM hAni = g_pLTBase->GetAnimIndex(m_hObject, g_pFXDB->GetString(m_hClassData,FXDB_sActivateAnimation));
	g_pLTServer->GetModelLT()->SetCurAnim( m_hObject, MAIN_TRACKER, hAni, false);
	g_pLTServer->GetModelLT()->SetLooping( m_hObject, MAIN_TRACKER, false );

	LTVector vVel(0.0f,g_pFXDB->GetFloat(m_hClassData,FXDB_fPopUpVelocity),0.0f);
	g_pPhysicsLT->SetVelocity(m_hObject, vVel);


}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProximity::UpdateGrenade()
//
//	PURPOSE:	Move the grenade model based on the position of our
//				rigid body
//
// ----------------------------------------------------------------------- //

bool CGrenadeProximity::UpdateGrenade()
{
	if (m_bPickedUp)
	{
		RemoveObject();
		return false;
	}

	if (!CGrenade::UpdateGrenade())
		return false;

	if (!m_bRotatedToRest && !m_bFlipping)
	{
		// Set us up to use the physics simulation...
		LTRigidTransform tRTrans;
		g_pLTServer->PhysicsSim()->GetRigidBodyTransform(m_hRigidBody,tRTrans);
		LTVector vUp = tRTrans.m_rRot.Up();

		//we're too turned, try to straighten out
		if (vUp.y < 0.5f )
		{
			{

				float fP =  GetConsoleFloat("FlipTurn",50.0f);
				//pull up on top
				LTVector vPop(0.0f,fP,0.0f);
				if( LTIsNaN( vPop ) || vPop.MagSqr() > 1000000.0f * 1000000.0f )
				{
					LTERROR( "Invalid impulse detected." );
					vPop.Init( 0.0f, 10.0f, 0.0f );
				}
				LTVector vPopPos = tRTrans.m_vPos + (vUp * 20.0f);
				g_pLTServer->PhysicsSim()->ApplyRigidBodyImpulseWorldSpace(m_hRigidBody, vPopPos,vPop);

				//pull down on bottom
				vPop.y = -2.0f * fP;
				if( LTIsNaN( vPop ) || vPop.MagSqr() > 1000000.0f * 1000000.0f )
				{
					LTERROR( "Invalid impulse detected." );
					vPop.Init( 0.0f, 10.0f, 0.0f );
				}
				vPopPos = tRTrans.m_vPos + (vUp * -20.0f);
				g_pLTServer->PhysicsSim()->ApplyRigidBodyImpulseWorldSpace(m_hRigidBody, vPopPos,vPop);
			}

		}
	}


	// Are there any Characters close enough to go active?
	if( m_eState == eArmed)
	{
		if (!m_bRecoverable)
		{
			SetRecoverable(true);
		}

		CheckActivation( );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CGrenadeProximity::CheckActivation
//
//  PURPOSE:	Is there a character within our activation range...
//
// ----------------------------------------------------------------------- //

void CGrenadeProximity::CheckActivation( )
{
	float fRadius = g_pFXDB->GetFloat(m_hClassData,FXDB_fActivationRadius);

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	CTList<CCharacter*>	lstChars;
	CCharacter			**ppChar = NULL;

	if( g_pCharacterMgr->FindCharactersWithinRadius( &lstChars, vPos, fRadius, NULL ))
	{
		ppChar = lstChars.GetItem( TLIT_FIRST );
		while ( ppChar )
		{
			CCharacter	*pTest = *ppChar;
			ppChar = lstChars.GetItem( TLIT_NEXT );
			
			if (!pTest->IsAlive())
				continue;

			if( !IsEnemyOf( pTest->m_hObject ))
				continue;

			// Make sure the GrenadeProximity can actually see the character...

			IntersectQuery	IQuery;
			IntersectInfo	IInfo;

			IQuery.m_Flags		= CHECK_FROM_POINT_INSIDE_OBJECTS | INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;

			HOBJECT hFilterList[] = { m_hObject, NULL };
			IQuery.m_FilterFn	= ObjListFilterFn;
			IQuery.m_pUserData	= hFilterList;

			IQuery.m_From		= vPos;
			g_pLTServer->GetObjectPos( pTest->m_hObject, &IQuery.m_To );

			if( g_pLTServer->IntersectSegment( IQuery, &IInfo ))
			{
				if( IInfo.m_hObject != pTest->m_hObject &&
					IInfo.m_hObject != pTest->GetHitBox() )
				{
					// The GrenadeProximity can't actually see the character so don't activate yet...
					continue;
				}
			}

			Activate();
		}

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProximity::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CGrenadeProximity::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	CGrenade::Save(pMsg, dwSaveFlags);

	SAVE_BYTE(m_eState);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProximity::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CGrenadeProximity::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	CGrenade::Load(pMsg, dwLoadFlags);

	LOAD_BYTE_CAST(m_eState, ActivationState);

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
	// Get the class data...

	if (hProjectileFX)
	{
		m_hClassData = g_pFXDB->GetProximityClassData(hProjectileFX);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProximity::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

bool CGrenadeProximity::OccupiesPosition( LTVector const& vPos )
{
	// Ignore if not armed.
	if( m_eState != eArmed )
		return false;

	LTVector vGrenadePos;
	g_pLTServer->GetObjectPos( m_hObject, &vGrenadePos );
	float fRadius = g_pFXDB->GetFloat(m_hClassData,FXDB_fActivationRadius);

	return LTIntersect::Sphere_Point( LTSphere( vGrenadePos, fRadius ), vPos );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProximity::IsEnemyOf
//
//	PURPOSE:	Checks if grenade is enemy of character.
//
// ----------------------------------------------------------------------- //

bool CGrenadeProximity::IsEnemyOf( HOBJECT hCharacter ) const
{
	//you are safe from your own mines (unless it's a team game where you might have switched teams)
	if( !GameModeMgr::Instance( ).m_grbUseTeams)
	{
		if (hCharacter == m_Shared.m_hFiredFrom) 
		{
			return false;
		}
	}

	CAI* pAI = CAI::DynamicCast( hCharacter );
	if( pAI )
	{
		if (!m_Shared.m_hFiredFrom) 
		{
			//if we don't know who fired the mine, assume it's an enemy AI
			CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer();
			if (pAI && pPlayer && kCharStance_Hate == g_pCharacterDB->GetStance( pAI->GetAlignment(), pPlayer->GetAlignment() ))
			{
				return false;
			}
		}
		else if (IsAI(m_Shared.m_hFiredFrom))
		{
			CCharacter* pB = CCharacter::DynamicCast( hCharacter );
			if (pAI && pB && kCharStance_Like == g_pCharacterDB->GetStance( pAI->GetAlignment(), pB->GetAlignment() ))
			{
				return false;
			}
		}
		else if (IsPlayer(m_Shared.m_hFiredFrom)) 
		{
			CPlayerObj* pPlayer = CPlayerObj::DynamicCast( m_Shared.m_hFiredFrom );
			if (pAI && pPlayer && kCharStance_Like == g_pCharacterDB->GetStance( pAI->GetAlignment(), pPlayer->GetAlignment() ))
			{
				return false;
			}
		}


	}

	// Don't activate if it's our owner/team...
	if( GameModeMgr::Instance( ).m_grbUseTeams)
	{
		if( IsMyTeam( hCharacter ))
			return false;
	}

	return true;
}

void CGrenadeProximity::HandlePlayerChange()
{
	CGrenade::HandlePlayerChange();

	// Update the proxi on the client with the new firedfrom info.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PROJECTILE_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_FIREDFROM );
	bool bFiredFromValid = ( m_Shared.m_hFiredFrom != NULL );
	cMsg.Writebool( bFiredFromValid );
	if( bFiredFromValid )
		cMsg.WriteObject( m_Shared.m_hFiredFrom );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	OverrideClientFX(	g_pFXDB->GetString(m_hClassData,FXDB_sArmedFX,0),
		g_pFXDB->GetString(m_hClassData,FXDB_sArmedFX,1),
		g_pFXDB->GetString(m_hClassData,FXDB_sArmedFX,2)
		);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGrenadeProximity::HandleActivateMsg
//
//	PURPOSE:	Handle a ACTIVATE message...
//
// ----------------------------------------------------------------------- //

void CGrenadeProximity::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( !m_bRecoverable )
		return;

	// If the activating character is dead, he can't pick up stuff...
	if (!IsPlayer(hSender))
		return;

	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
	if( !pPlayer || !pPlayer->IsAlive() )
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_ADDWEAPON);
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_Shared.m_hWeapon );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_Shared.m_hAmmo );
	cMsg.Writeint32(1);
	cMsg.Writebool(false);
	cMsg.Writeuint32(0);
	cMsg.Writebool(false);
	cMsg.WriteObject(m_Shared.m_hFiredFrom);
	g_pLTServer->SendToObject(cMsg.Read(), m_hObject, hSender, MESSAGE_GUARANTEED);

	SetNextUpdate(UPDATE_NEXT_FRAME);

	// Consider ourselves picked up.
	m_bRecoverable = false;
	m_bPickedUp = true;


}





// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteCharge::CRemoteCharge
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CRemoteCharge::CRemoteCharge() : CGrenade()
	, m_bStuck		( false )
	, m_hClassData	( NULL )
	, m_bPickedUp	(false)

{
	m_bDetonateOnImpact	 = false;
	if (!g_vtDebugRemote.IsInitted())
	{
		g_vtDebugRemote.Init(g_pLTServer, "DebugRemote", NULL, 0.0f);
	}

	m_fReflection = -0.1f;

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CRemoteCharge::Setup
//
//  PURPOSE:	Set up the remote charge...
//
// ----------------------------------------------------------------------- //

bool CRemoteCharge::Setup( const CWeapon *pWeapon, const WeaponFireInfo &info )
{
	// Let the base class setup first...

	if( !CGrenade::Setup( pWeapon, info ))
		return false;

	CCharacter* pCharObj = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject( m_Shared.m_hFiredFrom ));

	if (pCharObj)
	{
		pCharObj->AddRemoteCharge(m_hObject);
	}

	g_pLTServer->GetObjectPos( m_hObject, &m_vLastPos );

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
	// Get the class data...

	if (!hProjectileFX) return false;

	m_hClassData = g_pFXDB->GetRemoteClassData(hProjectileFX);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteCharge::UpdateGrenade()
//
//	PURPOSE:	Check to see if we're stuck yet...
//
// ----------------------------------------------------------------------- //

bool CRemoteCharge::UpdateGrenade()
{
	if (m_bPickedUp)
	{
		RemoveObject();
		return false;
	}

	// Keep updating grenade's position if we're not stuck yet.
	if( !m_bStuck && !CGrenade::UpdateGrenade())
		return false;

	if (m_bStuck && !m_bRotatedToRest)
	{
		RotateToRest();
	}

	if (m_bRotatedToRest && !m_bRecoverable)
	{
		SetRecoverable(true);
	}

	// Update our last pos.
	g_pLTServer->GetObjectPos( m_hObject, &m_vLastPos );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteCharge::RotateToRest()
//
//	PURPOSE:	Rotate the grenade to its rest position...
//
// ----------------------------------------------------------------------- //

void CRemoteCharge::RotateToRest()
{

	//take us out of the simulation...
	ReleaseRigidBody();

	if (!m_hTarget)
	{
		g_pLTServer->SetObjectTransform(m_hObject,m_tAttachPoint);
	}

	g_pLTServer->Physics()->SetVelocity(m_hObject, LTVector(0,0,0));

	LTRigidTransform tRTrans;
	g_pLTServer->GetObjectTransform(m_hObject,&tRTrans);
	if ( g_vtDebugRemote.GetFloat(0.0f) == 1.0f )
	{
		DebugCPrint(0," -   final pos:  %0.2f,%0.2f,%0.2f",tRTrans.m_vPos.x,tRTrans.m_vPos.y,tRTrans.m_vPos.z);
	}

	// Record this grenade for AI's to be wary of
	if ( !m_bRotatedToRest && m_bAddToGrenadeList )
	{
		g_lstGrenades.Add(this);
	}

	// At rest
	m_bRotatedToRest = true;

	// turn off the delay visible flag so that new clients will be able to see the remote charge
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_DELAYCLIENTVISIBLE);

	//set up the client FX
	OverrideClientFX(	g_pFXDB->GetString(m_hClassData,FXDB_sOverrideFX,0),
		g_pFXDB->GetString(m_hClassData,FXDB_sOverrideFX,1),
		g_pFXDB->GetString(m_hClassData,FXDB_sOverrideFX,2)
		);


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteCharge::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CRemoteCharge::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	CGrenade::Load(pMsg, dwLoadFlags);


	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(m_Shared.m_hAmmo,IsAI(m_Shared.m_hFiredFrom));
	HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
	// Get the class data...

	if (hProjectileFX)
	{
		m_hClassData = g_pFXDB->GetRemoteClassData(hProjectileFX);
	}

}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteCharge::HandleCollision
//
//	PURPOSE:	Handle bouncing off of things
//
// ----------------------------------------------------------------------- //

void CRemoteCharge::HandleCollision(HOBJECT hObjHit, HPHYSICSRIGIDBODY hBodyHit,const LTVector& vCollisionPt, 
									const LTVector& vCollisionNormal, float fVelocity, 
									bool& bIgnoreCollision, IntersectInfo* pInfo, bool bFakeBounce )
{
	IntersectInfo* pInputInfo = pInfo;

	//see if we hit something that broke
	CGrenade::HandleCollision(hObjHit, hBodyHit, vCollisionPt,vCollisionNormal,fVelocity,
		bIgnoreCollision, pInputInfo, false);
	if( bIgnoreCollision )
		return;

	if (m_bStuck)
	{
		bIgnoreCollision = true;
		return;
	}

	//figure out where the charge is, and calculate the direction to use for impact detection...
	LTRigidTransform tRTrans;
	g_pLTServer->PhysicsSim( )->GetRigidBodyTransform(m_hRigidBody,tRTrans);

	// If we've hit a character try sticking to it...
	if (IsCharacter(hObjHit))
	{
		if ( g_vtDebugRemote.GetFloat(0.0f) == 1.0f )
		{
			DebugCPrint(0,"Hit character");
			DebugCPrint(0," -   init pos:  %0.2f,%0.2f,%0.2f",tRTrans.m_vPos.x,tRTrans.m_vPos.y,tRTrans.m_vPos.z);
		}

		tRTrans.m_vPos = vCollisionPt - vCollisionNormal;
		LTVector vTestDir = vCollisionNormal;
		LTVector vTestPos = tRTrans.m_vPos;
		vTestDir.Normalize();

		if ( g_vtDebugRemote.GetFloat(0.0f) == 1.0f )
		{
			DebugCPrint(0," -    adj pos:  %0.2f,%0.2f,%0.2f",tRTrans.m_vPos.x,tRTrans.m_vPos.y,tRTrans.m_vPos.z);
		}
		//align grenade
		LTVector	vU, vR;
		if( (1.0f == vTestDir.y) || (-1.0f == vTestDir.y) )
		{
			vR = vTestDir.Cross( LTVector( 1.0f, 0.0f, 0.0f ));
		}
		else
		{
			vR = vTestDir.Cross( LTVector( 0.0f, 1.0f, 0.0f ));
		}
		vU = vR.Cross( vTestDir );
		vU.Normalize();
		tRTrans.m_rRot = LTRotation( vU, vTestDir );
		LTRigidTransform tNewPos = tRTrans * m_tBodyOffset;
		g_pLTServer->SetObjectTransform(m_hObject,tNewPos);

		StickToCharacter(hObjHit,vTestPos,vTestDir);
		m_bStuck = true;

		return;
	}

	// Only allowed to stick to a character, the world or a worldmodel.  Character is handled above,
	// hObjHit == NULL is the world, otherwise, it's type should be worldmodel.
	if( !hObjHit || GetObjectType( hObjHit ) == OT_WORLDMODEL )
	{
		// If we don't have a surface type yet, then we can try to get one from
		// the polygons.  This only works if the HOBJECT is NULL, which means the
		// rigid body is the main world, or if the HOBJECT is a worldmodel.
		LTVector vTestDir = vCollisionPt - tRTrans.m_vPos;
		float fDist = 10.0f + vTestDir.Mag();
		vTestDir.Normalize();
		LTVector vTarget = tRTrans.m_vPos + (fDist * vTestDir);

		// Get the object of the rigid body.  If it doesn't have a rigidbody or an object, then
		// we'll assume it's the mainworld.
		HSURFACE hSurf = NULL;
		IntersectInfo iInfo;
		IntersectQuery qInfo;

		if( !pInfo )
		{
			// Find the polygon hit and get the surface info from that.
			qInfo.m_From		= tRTrans.m_vPos;
			qInfo.m_To			= vTarget;
			HOBJECT hIntersectObject = NULL;
			qInfo.m_Flags		= INTERSECT_HPOLY | INTERSECT_OBJECTS;

			// If we don't have a null, then that means we are hitting the world.
			if( hObjHit && GetObjectType( hObjHit ) == OT_WORLDMODEL )
			{
				hIntersectObject = hObjHit;
			}
			else
			{
				hIntersectObject = g_pLTBase->GetMainWorldModel();
			}

			// Try to hit the object going the first direction.
			if( !g_pLTBase->IntersectSegmentAgainst( qInfo, &iInfo, hIntersectObject ))
			{
				// Didn't find in the first direction, try the opposite way.
				qInfo.m_From		= qInfo.m_To;
				qInfo.m_To			= tRTrans.m_vPos;
				if( !g_pLTBase->IntersectSegmentAgainst( qInfo, &iInfo, hIntersectObject ))
				{
					return;
				}
			}

			// Use this intersectinfo.
			pInfo = &iInfo;
		}

		// Resolve the surfacetype to the HSURFACE.
		SurfaceType eSurfaceType = GetSurfaceType( *pInfo );
		if( eSurfaceType != ST_UNKNOWN )
		{
			hSurf = g_pSurfaceDB->GetSurface( eSurfaceType );
		}

		//test surface for hardness
		if (!hSurf || g_pSurfaceDB->GetFloat(hSurf,SrfDB_Srf_fHardness) < 0.2f)
		{
			return;
		}


		//align grenade to surface
		//we can now build up a rotation given the plane normal and the axis to build our transform
		LTVector vN = pInfo->m_Plane.Normal();

		LTVector	vU, vR;
		if( (1.0f == vN.y) || (-1.0f == vN.y) )
		{
			vR = vN.Cross( LTVector( 1.0f, 0.0f, 0.0f ));
		}
		else
		{
			vR = vN.Cross( LTVector( 0.0f, 1.0f, 0.0f ));
		}

		vU = vR.Cross( vN );
		vU.Normalize();

		LTRotation rRot( vU, vN );
		LTVector vNewPoint = pInfo->m_Point + vN * 3.0;

		m_tAttachPoint.Init(vNewPoint,rRot);
		m_bStuck = true;

		//we can't take us out of the sim in mid-collision, but flag us to do so on the next update...
		g_pLTServer->PhysicsSim()->TeleportRigidBody(m_hRigidBody,m_tAttachPoint);
		g_pLTServer->SetObjectTransform( m_hObject, m_tAttachPoint );

		// If it's a worldmodel object, we can make an attachment.
		WorldModel* pWorldModel = WorldModel::DynamicCast( hObjHit );
		if( pWorldModel )
		{
			pWorldModel->AttachObject( m_hObject );
		}

		return;
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteCharge::StickToCharacter
//
//	PURPOSE:	Handle sticking to a character...
//
// ----------------------------------------------------------------------- //

void CRemoteCharge::StickToCharacter(HOBJECT hObjHit, const LTVector& vPos, const LTVector& vHeading)
{

	CCharacter* pChar = CCharacter::DynamicCast(hObjHit);
	if (!pChar)
	{
		return;
	}

	ModelsDB::HNODE hModelNode = FindHitNode(pChar,vHeading,vPos);
	if ( !hModelNode )
	{
		return;
	}

	m_bStuck = pChar->AttachRemoteCharge(m_hObject,hModelNode);
	if (m_bStuck)
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3);

		// we need to clear the netflags as well
		uint32 nNetFlags = 0;
		g_pLTServer->GetNetFlags(m_hObject, nNetFlags);
		
		nNetFlags &= ~(NETFLAG_POSUNGUARANTEED | NETFLAG_ROTUNGUARANTEED);
		g_pLTServer->SetNetFlags(m_hObject, nNetFlags);

		m_hTarget = hObjHit;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteCharge::FindHitNode()
//
//	PURPOSE:	Find possible node hit.  If no hit, then eModelNodeInvalid
//				returned.
//
// ----------------------------------------------------------------------- //

ModelsDB::HNODE CRemoteCharge::FindHitNode(CCharacter const* pChar, LTVector const& vDir, LTVector const& vFrom ) const
{
	ModelsDB::HNODE hModelNode = NULL;
	ModelsDB::HSKELETON hModelSkeleton = pChar->GetModelSkeleton();

	// This algorithm may need to change since our dims are probably much
	// bigger than the model's actual dims...

	float fMinDistance = float(INT_MAX);

	if ( g_vtDebugRemote.GetFloat(0.0f) == 1.0f )
	{
		g_pLTServer->CPrint("Checking hit nodes..................");
	}

	int cNodes = g_pModelsDB->GetSkeletonNumNodes(hModelSkeleton);
	for (int iNode = 0; iNode < cNodes; iNode++)
	{
		ModelsDB::HNODE hCurNode = g_pModelsDB->GetSkeletonNode( hModelSkeleton, iNode );

		// Don't do transforms if we don't need to...
		float fNodeRadius = g_pModelsDB->GetNodeRadius( hCurNode );
		if (fNodeRadius <= 0.0f)
		{
			continue;
		}

		const char* szNodeName = g_pModelsDB->GetNodeName( hCurNode );
		if( !szNodeName )
		{
			//LTASSERT( 0, "CCharacterHitBox::HandleVectorImpact:  No node name in model skeleton." ); 
			continue;
		}

		LTTransform transform;
		LTRESULT ltResult;
		HMODELNODE hNode;
		ltResult = g_pModelLT->GetNode(pChar->m_hObject, const_cast<char*>(szNodeName), hNode);
		if ( ltResult != LT_OK )
		{
			continue;
		}

		ltResult = g_pModelLT->GetNodeTransform(pChar->m_hObject, hNode, transform, true);
		//		LTASSERT( ltResult == LT_OK, "" );
		if ( ltResult != LT_OK )
		{
			continue;
		}


		// Distance along ray to point of closest approach to node point

		const LTVector vRelativeNodePos = transform.m_vPos - vFrom;
		const float fRayDist = vDir.Dot(vRelativeNodePos);
		const float fDistSqr = (vDir*fRayDist - vRelativeNodePos).MagSqr();


		// Ignore the node if it wasn't within the radius of the hit spot.
		if( fDistSqr > fNodeRadius*fNodeRadius )
		{
			continue;
		}

		if ( g_vtDebugRemote.GetFloat(0.0f) == 1.0f )
		{
			g_pLTServer->CPrint("Hit ''%s'' node", szNodeName );
		}


		// Ignore if not closest
		if ( fDistSqr >= fMinDistance )
		{
			continue;
		}

		// Highest priority hit node so far.
		hModelNode = hCurNode;
		fMinDistance = fDistSqr;
	}

	return hModelNode;
}

void CRemoteCharge::HandlePlayerChange()
{
	CGrenade::HandlePlayerChange();

	// Update the remote on the client with the new firedfrom info.
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PROJECTILE_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_FIREDFROM );
	bool bFiredFromValid = ( m_Shared.m_hFiredFrom != NULL );
	cMsg.Writebool( bFiredFromValid );
	if( bFiredFromValid )
		cMsg.WriteObject( m_Shared.m_hFiredFrom );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	OverrideClientFX(	g_pFXDB->GetString(m_hClassData,FXDB_sOverrideFX,0),
		g_pFXDB->GetString(m_hClassData,FXDB_sOverrideFX,1),
		g_pFXDB->GetString(m_hClassData,FXDB_sOverrideFX,2)
		);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CRemoteCharge::HandleActivateMsg
//
//	PURPOSE:	Handle a ACTIVATE message...
//
// ----------------------------------------------------------------------- //

void CRemoteCharge::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( !m_bRecoverable )
		return;

	// If the activating character is dead, he can't pick up stuff...
	if (!IsPlayer(hSender))
		return;

	CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
	if( !pPlayer || !pPlayer->IsAlive() )
		return;

	CAutoMessage cMsg;
	cMsg.Writeuint32(MID_ADDWEAPON);
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_Shared.m_hWeapon );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_Shared.m_hAmmo );
	cMsg.Writeint32(1);
	cMsg.Writebool(false);
	cMsg.Writeuint32(0);
	cMsg.Writebool(false);
	cMsg.WriteObject(m_Shared.m_hFiredFrom);
	g_pLTServer->SendToObject(cMsg.Read(), m_hObject, hSender, MESSAGE_GUARANTEED);

	SetNextUpdate(UPDATE_NEXT_FRAME);

	// Consider ourselves picked up.
	m_bRecoverable = false;
	m_bPickedUp = true;

}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ProximityGrenade::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 ProximityGrenade::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
	case MID_UPDATE:
		{
			Update();
		}
		break;

	case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;
	default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ProximityGrenade::Update()
//
//	PURPOSE:	Handle update
//
// ----------------------------------------------------------------------- //

void ProximityGrenade::Update()
{
	if (m_bFirstUpdate)
	{
		SetNextUpdate(UPDATE_NEXT_FRAME);
		m_bFirstUpdate = false;

		HAMMO hAmmo = g_pWeaponDB->GetAmmoRecord("Proximity");
		HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,!USE_AI_DATA);
		HRECORD hProjectileFX = ( g_pWeaponDB->GetRecordLink( hAmmoData, WDB_AMMO_sProjectileFX ));
		if( !hProjectileFX || LTStrEmpty(g_pFXDB->GetString(hProjectileFX,FXDB_sClass)) )
			return;

		LTRigidTransform trans;
		g_pLTServer->GetObjectTransform(m_hObject,&trans);

		ObjectCreateStruct theStruct;
		// set the starting rotation
		theStruct.m_Rotation = trans.m_rRot;
		theStruct.m_Pos = trans.m_vPos;

		HCLASS hClass = g_pLTServer->GetClass( g_pFXDB->GetString(hProjectileFX,FXDB_sClass) );

		LTASSERT_PARAM1( hClass, "Unable to retreive class: %s", g_pFXDB->GetString(hProjectileFX,FXDB_sClass) );

		if (hClass)
		{
			CProjectile* pProj = (CProjectile*)g_pLTServer->CreateObject(hClass, &theStruct);
			if (pProj)
			{
				if( !pProj->Setup(hAmmo, trans) )
				{
					g_pLTServer->RemoveObject( pProj->m_hObject );
				}
			}
		}
	}
	else
	{
		g_pLTServer->RemoveObject(m_hObject);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ProximityGrenade::InitialUpdate()
//
//	PURPOSE:	Handle initial update
//
// ----------------------------------------------------------------------- //

void ProximityGrenade::InitialUpdate()
{
	SetNextUpdate(UPDATE_NEXT_FRAME);
	m_bFirstUpdate = true;

}




