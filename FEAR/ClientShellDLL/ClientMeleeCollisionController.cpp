// ----------------------------------------------------------------------- //
//
// MODULE  : ClientMeleeCollisionController.cpp
//
// PURPOSE : ClientMeleeCollisionController - Definition
//
// CREATED : 01/20/05
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ClientMeleeCollisionController.h"
#include "ClientServerShared.h"
#include "PlayerMgr.h"
#include "CMoveMgr.h"
#include "PlayerBodyMgr.h"
#include "CharacterFX.h"
#include "PhysicsUtilities.h"
#include "ClientWeapon.h"
#include "ClientWeaponMgr.h"

CClientMeleeCollisionController::MeleeCollider::MeleeCollider()
:	m_hAnimObject			(NULL)
,	m_nTrackerID			(MAIN_TRACKER)
,	m_hAnim					(INVALID_MODEL_ANIM)
,	m_fEndTime				(0.0f)
,	m_fScaledRadius			(0.0f)
,	m_hSocketParent			(NULL)
,	m_hSocket				(INVALID_MODEL_SOCKET)
,	m_hRigidBody			(INVALID_PHYSICS_RIGID_BODY)
,	m_hCollisionNotifier	(INVALID_PHYSICS_COLLISION_NOTIFIER)
,	m_pController			(NULL)
,	m_bDisableRequest		(false)
{
}

CClientMeleeCollisionController::MeleeCollider::~MeleeCollider()
{
	LTASSERT((m_hCollisionNotifier == INVALID_PHYSICS_COLLISION_NOTIFIER), "CollisionNotifier leak!");
}

// ----------------------------------------------------------------------- //

CClientMeleeCollisionController::CClientMeleeCollisionController()
:	m_bForcedBlocking		(false)
,	m_hObject				(NULL)
{
	// Set the backpointers.

	for ( int iEachCollider = 0; iEachCollider < LTARRAYSIZE(m_MeleeColliders); ++iEachCollider )
	{
		m_MeleeColliders[iEachCollider].m_pController = this;
	}
}

// ----------------------------------------------------------------------- //

CClientMeleeCollisionController::~CClientMeleeCollisionController()
{
	ResetCollisionData();

	// Clear the backpointers.

	for ( int iEachCollider = 0; iEachCollider < LTARRAYSIZE(m_MeleeColliders); ++iEachCollider )
	{
		m_MeleeColliders[iEachCollider].m_pController = NULL;
	}
}

// ----------------------------------------------------------------------- //

void CClientMeleeCollisionController::Init(HOBJECT hServerObject)
{
	m_hObject = hServerObject;
}

// ----------------------------------------------------------------------- //

void CClientMeleeCollisionController::HandleServerMessage(ILTMessage_Read* pMsg)
{
	MeleeControllerMsgId eMeleeMsg = (MeleeControllerMsgId)pMsg->Readuint8();

	switch (eMeleeMsg)
	{
	case MELEE_FORCE_BLOCKING_MSG:
		m_bForcedBlocking = pMsg->Readbool();
		break;
	}
}

// ----------------------------------------------------------------------- //

void CClientMeleeCollisionController::HandleModelKey(HLOCALOBJ hObj, ArgList *pArgs, ANIMTRACKERID nTrackerId)
{
	static CParsedMsg::CToken s_cTok_Attack("ATTACK");
	static CParsedMsg::CToken s_cTok_Block("BLOCK");

	// Pre-parse the message...
	CParsedMsg cParsedMsg( pArgs->argc, pArgs->argv );
	CParsedMsg::CToken cTok_Key = cParsedMsg.GetArg(1);

	bool bAttack = (cTok_Key == s_cTok_Attack);
	bool bBlock = (cTok_Key == s_cTok_Block);

	if (bAttack || bBlock)
	{
		if (pArgs->argc > 2)
		{
			// grab the collider record
			const char* pszColliderRecord = pArgs->argv[2];

			// grab an optional time limit
			float flDurationS = 0.0f;
			if (pArgs->argc > 3)
			{
				int nDurationMS = atoi(pArgs->argv[3]);
				flDurationS = (nDurationMS / 1000.f);
			}

			EnableCollisions(hObj, nTrackerId, pszColliderRecord, flDurationS, bBlock);
		}
	}
}

// ----------------------------------------------------------------------- //

void CClientMeleeCollisionController::Update()
{
	for ( int iEachCollider = 0; iEachCollider < LTARRAYSIZE(m_MeleeColliders); ++iEachCollider )
	{
		UpdateCollider( m_MeleeColliders[iEachCollider] );
	}
}

void CClientMeleeCollisionController::UpdateCollider( MeleeCollider& rCollider )
{
	if (rCollider.m_hRigidBody != INVALID_PHYSICS_RIGID_BODY)
	{
		// Disable if endtime is up, if animation changes, or asked to.
		if (rCollider.m_bDisableRequest)
		{
			ResetCollider( rCollider );
			return;
		}
		else if (rCollider.m_fEndTime > 0.0f)
		{
			if (g_pLTClient->GetTime() >= rCollider.m_fEndTime)
			{
				ResetCollider( rCollider );
				return;
			}
		}
		else
		{
			HMODELANIM hAnim;
			g_pModelLT->GetCurAnim(rCollider.m_hAnimObject, rCollider.m_nTrackerID, hAnim);

			if (hAnim != rCollider.m_hAnim)
			{
				ResetCollider( rCollider );
				return;
			}
		}

		// Keep our rigidbody in sync with its parent socket.
		LTTransform tSocket;
		if (g_pModelLT->GetSocketTransform(rCollider.m_hSocketParent, rCollider.m_hSocket, tSocket, true) == LT_OK)
		{
			g_pLTBase->PhysicsSim()->KeyframeRigidBody(rCollider.m_hRigidBody, LTRigidTransform(tSocket.m_vPos, tSocket.m_rRot), ObjectContextTimer(rCollider.m_hAnimObject).GetTimerElapsedS());
		}
	}
}


// ----------------------------------------------------------------------- //

void CClientMeleeCollisionController::ResetCollisionData()
{
	for ( int iEachCollider = 0; iEachCollider < LTARRAYSIZE(m_MeleeColliders); ++iEachCollider )
	{
		ResetCollider( m_MeleeColliders[iEachCollider] );
	}
}

void CClientMeleeCollisionController::ResetCollider( MeleeCollider& rCollider )
{
	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	// Remove the old collision notifier...
	if (rCollider.m_hCollisionNotifier != INVALID_PHYSICS_COLLISION_NOTIFIER)
	{
		pILTPhysicsSim->ReleaseCollisionNotifier(rCollider.m_hCollisionNotifier);
		rCollider.m_hCollisionNotifier = INVALID_PHYSICS_COLLISION_NOTIFIER;
	}

	// Release our collider...
	if (rCollider.m_hRigidBody != INVALID_PHYSICS_RIGID_BODY)
	{
		pILTPhysicsSim->ReleaseRigidBody(rCollider.m_hRigidBody);
		rCollider.m_hRigidBody = INVALID_PHYSICS_RIGID_BODY;
	}

	rCollider.m_hCollisionList.resize(0);
	rCollider.m_fEndTime = 0.0f;
	rCollider.m_bDisableRequest = false;
}

// ----------------------------------------------------------------------- //

void CClientMeleeCollisionController::EnableCollisions(HOBJECT hObj, ANIMTRACKERID nTrackerId, const char* pszColliderRecord, float flDurationS, bool bBlocking)
{
	// Find an unused collider.  If there isn't one, assert.  We are currently
	// assuming we only need two max (one of each of an AIs dual weapons). If
	// we validly need more, we may want to either increase the count or 
	// make these dynamic.

	MeleeCollider* pCollider = NULL;
	for ( int iEachCollider = 0; iEachCollider < LTARRAYSIZE(m_MeleeColliders); ++iEachCollider )
	{
		if ( INVALID_PHYSICS_COLLISION_NOTIFIER == m_MeleeColliders[iEachCollider].m_hCollisionNotifier )
		{
			pCollider = &m_MeleeColliders[iEachCollider];
			break;
		}
	}
	if ( NULL == pCollider )
	{
		char szError[256];
		LTSNPrintF(szError, LTARRAYSIZE(szError), "CClientMeleeCollisionController::EnableCollisions: Too many attacks and blocks at once.  '%s' will be ignored.", pszColliderRecord );
		LTERROR(szError);
		return;
	}

	// Clear out the collider, just in case any state remained.

	ResetCollider( *pCollider );

	bool bLocallyControlled = (hObj == g_pPlayerMgr->GetMoveMgr()->GetObject());

	// Grab the desired collision specifications from the database.
	HRECORD hMeleeCollider = g_pWeaponDB->GetMeleeColliderRecord(pszColliderRecord);
	if (!hMeleeCollider)
	{
		char szError[256];
		LTSNPrintF(szError, LTARRAYSIZE(szError), "ColliderRecord not found! '%s'", pszColliderRecord);
		LTERROR(szError);
		return;
	}

	// Keep track of our current animation so we know when it changes (see Update).
	pCollider->m_hAnimObject = hObj;
	pCollider->m_nTrackerID = nTrackerId;
	g_pModelLT->GetCurAnim(pCollider->m_hAnimObject, pCollider->m_nTrackerID, pCollider->m_hAnim);

	// Keep track of an optional end time (see Update).
	pCollider->m_fEndTime = (flDurationS > 0.0f) ? g_pLTClient->GetTime() + flDurationS : 0.0f;

	// Find the specified socket.
	pCollider->m_hSocketParent = NULL;
	pCollider->m_hSocket = INVALID_MODEL_SOCKET;
	const char* pszNodeName = g_pWeaponDB->GetString(hMeleeCollider, "NodeName");

	// Check attachments first...
	HOBJECT hParent = NULL;
	if (FindAttachmentSocket(hObj, pszNodeName, &hParent, &pCollider->m_hSocket))
	{
		pCollider->m_hSocketParent = hParent;
	}

	// Then check the model...
	else if (g_pModelLT->GetSocket(hObj, pszNodeName, pCollider->m_hSocket) == LT_OK)
	{
		pCollider->m_hSocketParent = hObj;
	}

	if (pCollider->m_hSocket == INVALID_MODEL_SOCKET)
	{
		char szError[256];
		LTSNPrintF(szError, LTARRAYSIZE(szError), "Socket not found! '%s'", pszNodeName);
		LTERROR(szError);
		return;
	}
	LTASSERT(pCollider->m_hSocketParent, "Socket assigned with no Parent!");

	LTTransform tSocket;
	if (LT_OK != g_pModelLT->GetSocketTransform(pCollider->m_hSocketParent, pCollider->m_hSocket, tSocket, true))
		return;

	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	// Create shape dynamically
	float fLengthUp		= g_pWeaponDB->GetFloat(hMeleeCollider, "LengthUp");
	float fLengthDown	= g_pWeaponDB->GetFloat(hMeleeCollider, "LengthDown");
	float fRadius		= g_pWeaponDB->GetFloat(hMeleeCollider, "Radius");

	HPHYSICSSHAPE hShape = pILTPhysicsSim->CreateCapsuleShape(
		LTVector(0.0f, -fLengthDown, 0.0f),
		LTVector(0.0f, fLengthUp, 0.0f),
		fRadius, 1.0f, 1000.0f);

	if (hShape == INVALID_PHYSICS_SHAPE)
	{
		LTERROR("Failed to create shape!");
		return;
	}

	EPhysicsGroup ePhysicsGroup =
		bBlocking ?
			PhysicsUtilities::ePhysicsGroup_UserBlockMelee :
		bLocallyControlled ?
			PhysicsUtilities::ePhysicsGroup_UserLocalMelee :
			PhysicsUtilities::ePhysicsGroup_UserRemoteMelee;

	pCollider->m_hRigidBody = pILTPhysicsSim->CreateRigidBody(hShape, LTRigidTransform(tSocket.m_vPos, tSocket.m_rRot), true, ePhysicsGroup, 0, 0.5f, 0.0f);

	pILTPhysicsSim->ReleaseShape(hShape);

	if (pCollider->m_hRigidBody == INVALID_PHYSICS_RIGID_BODY)
	{
		LTERROR("Failed to create RigidBody!");
		return;
	}

	// Calculate the scaled radius...
	pCollider->m_fScaledRadius = fRadius * g_pWeaponDB->GetFloat(hMeleeCollider, "CollisionRadiusScale");

	// Create collision notifier
	LTASSERT((pCollider->m_hCollisionNotifier == INVALID_PHYSICS_COLLISION_NOTIFIER), "CollisionNotifier leak!");

	if (!bBlocking)	// blocking will be handled by the attacker, and therefore will not require a collider.
	{
		pCollider->m_hCollisionNotifier =
			pILTPhysicsSim->RegisterCollisionNotifier(pCollider->m_hRigidBody, CollisionNotifier, pCollider);
		pILTPhysicsSim->EnableRigidBodyPinnedCollisions(pCollider->m_hRigidBody, true);
	}
}

// ----------------------------------------------------------------------- //

void CClientMeleeCollisionController::DisableCollisions()
{
	for ( int iEachCollider = 0; iEachCollider < LTARRAYSIZE(m_MeleeColliders); ++iEachCollider )
	{
		m_MeleeColliders[iEachCollider].m_bDisableRequest = true;
	}
}

// ----------------------------------------------------------------------- //

void CClientMeleeCollisionController::CollisionNotifier(	HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
															const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
															float fVelocity, bool& bIgnoreCollision, void* pUser	)
{
	CGameClientShell::CClientShellScopeTracker cScopeTracker;

	bIgnoreCollision = true;	// let it pass thru

	MeleeCollider* pCollider = (MeleeCollider*)pUser;
	
	if ( !pCollider )
	{
		LTERROR("Invalid user data!");
		return;
	}

	CClientMeleeCollisionController* pController = pCollider->m_pController;

	if ( !pController )
	{
		LTERROR("Invalid user data!");
		return;
	}

	if (hBody1 == NULL || hBody2 == NULL)
	{
		LTERROR("Invalid rigidbody!");
		return;
	}

	if (pCollider->m_bDisableRequest)
		return;

	ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();

	LTVector vDir;
	HPHYSICSRIGIDBODY hMeleeBody = pCollider->m_hRigidBody;
	HPHYSICSRIGIDBODY hTargetBody = INVALID_PHYSICS_RIGID_BODY;
	if (hBody1 == hMeleeBody)
	{
		hTargetBody = hBody2;
		vDir = -vCollisionNormal;	// flip the normal so it's always coming from the melee body.
	}
	else if (hBody2 == hMeleeBody)
	{
		hTargetBody = hBody1;
		vDir = vCollisionNormal;
	}
	else
	{
		LTERROR("Collision using unknown melee object!  Possibly a collision notifier leak?");
		return;
	}

	HOBJECT hTarget = NULL;
	pILTPhysicsSim->GetRigidBodyObject(hTargetBody, hTarget);	// hTarget will be NULL for blocking rigidbodies

	// Filter out hitting yourself (AIs need to be able to hit other AIs, 
	// which enables them to hit themselves).  This is an easy to detect 
	// case, so filter it out here to minimize the overhead.

	if ( hTarget == pController->m_hObject )
	{
		return;
	}

	// Fix up client player handles to use their server handle instead.
	if (hTarget == g_pPlayerMgr->GetMoveMgr()->GetObject())
	{
		hTarget = g_pLTClient->GetClientObject();
	}

	//!!ARL: Filter out non-server objects since we can't damange them anyway.
	//(ATT needs to expose IsServerObject to game code first).

	// Only collide with an object once.  Animations that do multiple impacts
	// will require multiple MELEEATTACK keys.  This avoids worrying about
	// constant notifications due to interpenetration.
	ObjRefVector::iterator iBegin = pCollider->m_hCollisionList.begin();
	ObjRefVector::iterator iEnd = pCollider->m_hCollisionList.end();
	if (std::find(iBegin, iEnd, hTarget) != iEnd)
	{
		return;
	}
	pCollider->m_hCollisionList.push_back(hTarget);

	HMODELNODE hNodeHit = PhysicsUtilities::GetRigidBodyModelNode(hTarget, hTargetBody);

	EPhysicsGroup eHitPhysics;
	uint32 nHitSystem;
	pILTPhysicsSim->GetRigidBodyCollisionInfo(hTargetBody, eHitPhysics, nHitSystem);

	LTVector vPos = vCollisionPt;
#if 1
	// Pull the collision point in based on the collision record's radius scale.
	LTRigidTransform tMeleeBody;
	if (pILTPhysicsSim->GetRigidBodyTransform(hMeleeBody, tMeleeBody) == LT_OK)
	{
		// translate into meleebody space.
		vPos = tMeleeBody.GetInverse() * vPos;

		// project a unit vector into the plane of the capsule
		LTVector vFlatDir = LTVector(vPos.x, 0.0f, vPos.z).GetUnit();

		// then scale to the appropriate distance from the center
		vPos.x = vFlatDir.x * pCollider->m_fScaledRadius;
		vPos.z = vFlatDir.z * pCollider->m_fScaledRadius;

		// translate back into world space.
		vPos = tMeleeBody * vPos;
	}
#elif 0
	//!!ARL: Alternate solution: find closest point on the axis line and interpolate.
#endif

	//!!ARL: hTarget is always a CharacterFX isn't it?  - since collision is
	// only enabled for characters and not the player body.  I think for the
	// local player we want to enable collision for the player body instead of
	// its characterfx.  Search for SetRigidBodyCollisionGroup in CharacterFX.cpp
	// If we do that we'll likely want to fix up hTarget here to refer back to
	// the original character hobject.  We'll also need to ensure that we don't
	// get double collisions (one from the player body and a second from its
	// associated characterfx).

	// Pass on to the controller for handling.
	pController->HandleCollision(hTarget, hNodeHit, eHitPhysics, vPos, vDir);
}

// ----------------------------------------------------------------------- //

void CClientMeleeCollisionController::HandleCollision(HOBJECT hTarget, HMODELNODE hNodeHit, EPhysicsGroup eHitPhysics, const LTVector& vPos, const LTVector& vDir)
{
	// Check if the attack has been blocked via rigidbody...
	if (eHitPhysics == PhysicsUtilities::ePhysicsGroup_UserBlockMelee)
	{
		HandleBlocked(hTarget, vPos, vDir);
		return;
	}

	// Check if the attack has been blocked via forced blocking (what the AI does)...
	CCharacterFX* pTargetFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(hTarget);
	if (pTargetFX && pTargetFX->IsBlocking())
	{
		HandleBlocked(hTarget, vPos, vDir);
		return;
	}

	// Handle normal damage...
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_OBJECT_MESSAGE);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint32(MID_MELEEATTACK);
	cMsg.WriteObject(hTarget);
	cMsg.Writeuint32(hNodeHit);
	cMsg.WriteLTVector(vPos);
	cMsg.WriteLTVector(vDir);
	cMsg.Writeint32(g_pGameClientShell->GetServerRealTimeMS());
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	//!!ARL: Maybe DisableCollisions for hTarget if they are blocking?
	// (to avoid the weirdness of taking damage but still blocking the attack)

	// For local player targets, send an AttackRecoil stimulus so a proper animation can be played.
	if (hTarget == g_pPlayerMgr->GetMoveMgr()->GetObject())
	{
		CPlayerBodyMgr::Instance().HandleAnimationStimulus("CS_RecoilFromAttack");
	}
}

// ----------------------------------------------------------------------- //

void CClientMeleeCollisionController::HandleBlocked(HOBJECT hTarget, const LTVector& vPos, const LTVector& vDir)
{
	// Get the proper weapon record...
	CClientWeapon* pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	HWEAPON hWeapon = pClientWeapon ? pClientWeapon->GetWeaponRecord() : NULL;	//!!ARL: Use Attacker's weapon instead?  (will need to be sent from server - probably along with block info)
	HWEAPONDATA hWeaponData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	// Spawn a block effect for it...
	const char* pszBlockFX = g_pWeaponDB->GetString(hWeaponData, "BlockFX");
	CLIENTFX_CREATESTRUCT fxcs(pszBlockFX, 0, LTRigidTransform(vPos, LTRotation(vDir, LTVector(0,1,0))));
	g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, fxcs, true);

	// Let the server objects know they've blocked / been blocked.
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_OBJECT_MESSAGE);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint32(MID_MELEEBLOCK);
	cMsg.WriteObject(hTarget);
	g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);

	// Disable attacker's collision (i.e. stop attacking).
	DisableCollisions();

	// For local player attackers, send a BlockRecoil stimulus so a proper animation can be played.
	if (m_hObject == g_pPlayerMgr->GetMoveMgr()->GetObject())
	{
		CPlayerBodyMgr::Instance().HandleAnimationStimulus("CS_RecoilFromBlock");
	}
}
