// ----------------------------------------------------------------------- //
//
// MODULE  : BodyState.cpp
//
// PURPOSE : Body State - Implementation
//
// CREATED : 1999
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "StdAfx.h"
#include "BodyState.h"
#include "Body.h"
#include "AIVolume.h"
#include "AIVolumeMgr.h"
#include "GameServerShell.h"
#include "MsgIDs.h"
#include "Attachments.h"
#include "AIUtils.h"
#include "ServerMissionMgr.h"
#include "PlayerObj.h"

static CVarTrack s_RemovePowerups;
static CVarTrack s_LifetimeTrack;
extern CVarTrack g_BodyStickDist;
extern CVarTrack g_BodyStateTimeout;
extern CVarTrack g_vtBodyExplosionVelMultiplier;
extern float g_kfDefaultBodyExplosionVelocity;

DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateNormal, kState_BodyNormal);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateExplode, kState_BodyExplode);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateStairs, kState_BodyStairs);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateLedge, kState_BodyLedge);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateLadder, kState_BodyLadder);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateUnderwater, kState_BodyUnderwater);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateLaser, kState_BodyLaser);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateDecay, kState_BodyDecay);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateCrush, kState_BodyCrush);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateChair, kState_BodyChair);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStatePoison, kState_BodyPoison);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateAcid, kState_BodyAcid);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateArrow, kState_BodyArrow);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateFade, kState_BodyFade);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateCarried, kState_BodyCarried);
DEFINE_AI_FACTORY_CLASS_SPECIFIC(State, CBodyStateDropped, kState_BodyDropped);

extern char* g_szArrowFrontDeath;
extern char* g_szArrowBackDeath;

// CBodyState

CBodyState::CBodyState()
{
    m_pBody = LTNULL;
}

void CBodyState::Init(Body* pBody)
{
	m_pBody = pBody;

	// Turn off gravity, solid, and animtransition
	// and turn on gothruworld, touch notify, and rayhit
	g_pCommonLT->SetObjectFlags(m_pBody->m_hObject, OFT_Flags, 
		FLAG_GOTHRUWORLD | FLAG_TOUCH_NOTIFY | FLAG_RAYHIT, 
		FLAG_SOLID | FLAG_GRAVITY | 
		FLAG_GOTHRUWORLD | FLAG_TOUCH_NOTIFY | FLAG_RAYHIT);

    g_pPhysicsLT->SetVelocity(m_pBody->m_hObject, &LTVector(0,0,0));
    g_pPhysicsLT->SetAcceleration(m_pBody->m_hObject, &LTVector(0,0,0));

	if (!s_RemovePowerups.IsInitted())
	{
        s_RemovePowerups.Init(g_pLTServer, "BodyRemovePowerups", NULL, 1.0f);
	}
}

void CBodyState::InitLoad(Body* pBody)
{
	m_pBody = pBody;
}

// CBodyStateNormal

void CBodyStateNormal::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	if(!s_LifetimeTrack.IsInitted())
	{
        s_LifetimeTrack.Init(g_pLTServer, "BodyLifetime", NULL, 30.0f);
	}

	//activate hit box (might have been deactivated by previous state)
	if (m_pBody->GetHitBox())
	{
		g_pCommonLT->SetObjectFlags(m_pBody->GetHitBox(), OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT);
	}
	
	// Make sure it's on the ground...
	
	MoveObjectToFloor( m_pBody->m_hObject );
}

void CBodyStateNormal::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

	if(!s_LifetimeTrack.IsInitted())
	{
        s_LifetimeTrack.Init(g_pLTServer, "BodyLifetime", NULL, 30.0f);
	}
}

void CBodyStateNormal::Update()
{
	CBodyState::Update();

	// [kml] 3/19/02
	// m_pBody->GetLifetime() should ONLY get set via a model string key
	// "LIFETIME 3.00" for example. So if the artists aren't doing that,
	// then you shouldn't ever have to deal with this next block of code.

	// [jrg] 6/30/02
	// re-implementing NOLF body fadeout after life time expires, the body will track which method to use
	// based on how it received the lifetime. If it received the lifetime from a modelkey it will skip the fade
	// and just go away.
	LTFLOAT fTime = g_pLTServer->GetTime();
	if((m_pBody->GetLifetime() > 0.0f) && (fTime > m_pBody->GetStarttime() + m_pBody->GetLifetime()))
	{

		if (m_pBody->GetFadeAfterLifetime())
		{
			//if we are supposed to fade, go to the appropriate state
			m_pBody->SetState(kState_BodyFade);
			return;
		}

		// m_pBody->ReleasePowerups() handles dropping weapons & spears, etc...
		// Since we don't want that, we use DropInventoryObject() for general inventory.
		// If you're not using the general inventory, then just ignore this.
		m_pBody->DropInventoryObject();
		g_pLTServer->RemoveObject( m_pBody->m_hObject );
		m_pBody = NULL;
		return;
	}
}

LTBOOL CBodyStateNormal::CanDeactivate()
{
	// CBody::Update calls CBodyState::Update which can remove the object
	// in some cases. Since this gets called at the end of hte CBody::Update
	// loop, we need to check the pointer here.
	if(!m_pBody)
		return LTTRUE;

	// If we have > 0 lifetime, see if we should remove ourselves

	if ( m_pBody && m_pBody->GetLifetime() > 0 )
	{
		return LTFALSE;
	}
	else
	{
		return LTTRUE;
	}
}

// CBodyStateStairs

CBodyStateStairs::CBodyStateStairs()
{
    m_vDir = LTVector(0,0,0);
	m_pVolume = LTNULL;
    m_bFell = LTFALSE;
}

void CBodyStateStairs::Init(Body* pBody)
{
	CBodyState::Init(pBody);

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);

	// Use the animation dims so we don't clip into the world...

    LTVector	vDims;
	uint32		dwAni = g_pLTServer->GetModelAnimation( m_pBody->m_hObject );
	
	if (dwAni != INVALID_ANI)
	{
		g_pCommonLT->GetModelAnimUserDims( m_pBody->m_hObject, &vDims, dwAni );
	}
	else // use normal dims...
	{
		g_pPhysicsLT->GetObjectDims( m_pBody->m_hObject, &vDims );
	}

	AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, vPos, eAxisAll, 64.0f/*vDims.y*/);
	_ASSERT(pVolume && (pVolume->GetVolumeType() == AIVolume::kVolumeType_Stairs));
	if ( !pVolume || (pVolume->GetVolumeType() != AIVolume::kVolumeType_Stairs) )
	{
		m_pBody->SetState(kState_BodyNormal);
		return;
	}

	m_pVolume = pVolume;
	m_vDir = pVolume->GetForward();

	// If we are more than 180' off the stairs dir, play the backwards falling animation

	LTRotation rRot; 
	g_pLTServer->GetObjectRotation(m_pBody->m_hObject, &rRot);
	LTVector vForward = rRot.Forward();


	if ( m_vDir.Dot(vForward) > 0.0f )
	{
	    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairsStart");
		if (m_hAniStart == INVALID_ANI)
		{
			m_hAniStart = 0;
			g_pLTServer->CPrint("Body missing animation \"FallDownStairsStart\"");
		}
	}
	else
	{
	    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairsBack");
		if (m_hAniStart == INVALID_ANI)
		{
			m_hAniStart = 0;
			g_pLTServer->CPrint("Body missing animation \"FallDownStairsBack\"");
		}
	}

    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairs");
	if (m_hAniLoop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallDownStairs\"");
	}

    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairsStop");
	if (m_hAniStop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallDownStairsStop\"");
	}

	// Face down the stairs

	m_pBody->FaceDir(pVolume->GetForward());
}

void CBodyStateStairs::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairsStart");
	if (m_hAniStart == INVALID_ANI)
	{
		m_hAniStart = 0;
		g_pLTServer->CPrint("Body missing animation \"FallDownStairsStart\"");
	}

    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairs");
	if (m_hAniLoop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallDownStairs\"");
	}

    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairsStop");
	if (m_hAniStop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallDownStairsStop\"");
	}

}

void CBodyStateStairs::Update()
{
	CBodyState::Update();

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);
    
	// Use the animation dims so we don't clip into the world...

    LTVector	vDims;
	uint32		dwAni = g_pLTServer->GetModelAnimation( m_pBody->m_hObject );
	
	if (dwAni != INVALID_ANI)
	{
		g_pCommonLT->GetModelAnimUserDims( m_pBody->m_hObject, &vDims, dwAni );
	}
	else // use normal dims...
	{
		g_pPhysicsLT->GetObjectDims( m_pBody->m_hObject, &vDims );
	}

	AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, vPos, eAxisAll, 64.0f/*vDims.y*2.0f*/, m_pVolume, LTFALSE);
    LTBOOL bInside = pVolume && (pVolume->GetVolumeType() == AIVolume::kVolumeType_Stairs);

	if ( !bInside )
	{
		AIVolume* pLastVolume = m_pVolume;

        HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_pBody->m_hObject);
		if ( hAni == m_hAniStop )
		{
            if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
			{
				m_pBody->SetState(kState_BodyNormal);
				return;
			}
		}
		else
		{
            g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStop);
            g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);
		}

		if ( pVolume && pVolume->Inside2dLoose(vPos, 36.0f) )
		{
			const static LTFLOAT fStairsFallStopSpeed = g_pServerButeMgr->GetBodyStairsFallStopSpeed();
			LTVector vNewPos = vPos + pLastVolume->GetForward()*g_pLTServer->GetFrameTime()*fStairsFallStopSpeed;
			
			g_pLTServer->MoveObject(m_pBody->m_hObject, &vNewPos);
		}
	}
	else
	{
		if ( pVolume != m_pVolume )
		{
			m_pVolume = pVolume;
			m_pBody->FaceDir(pVolume->GetForward());
		}

		if ( m_bFell )
		{
			const static LTFLOAT fStairsFallSpeed = g_pServerButeMgr->GetBodyStairsFallSpeed();
            LTVector vNewPos = vPos + pVolume->GetForward()*g_pLTServer->GetFrameTime()*fStairsFallSpeed;

			IntersectQuery IQuery;
			IntersectInfo IInfo;

			VEC_COPY(IQuery.m_From, vNewPos);
            VEC_COPY(IQuery.m_To, vNewPos+LTVector(0,-256.0f,0));
			IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
			IQuery.m_FilterFn = GroundFilterFn;

			g_cIntersectSegmentCalls++;
            if (g_pLTServer->IntersectSegment(&IQuery, &IInfo) && ((IsMainWorld(IInfo.m_hObject)) || (OT_WORLDMODEL == GetObjectType(IInfo.m_hObject))))
			{
				vNewPos.y = IInfo.m_Point.y + vDims.y;
			}

			// TODO: for reasons unbeknownst to me, calling TeleportObject causes the Body to jerk around
			g_pLTServer->MoveObject(m_pBody->m_hObject, &vNewPos);
		}
		else
		{
            HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_pBody->m_hObject);
			if ( hAni == m_hAniStart )
			{
                if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
				{
                    m_bFell = LTTRUE;

                    g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniLoop);
                    g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTTRUE);
				}
			}
			else
			{
                g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStart);
                g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);
			}
		}
	}
}

void CBodyStateStairs::HandleTouch(HOBJECT hObject)
{
	CBodyState::HandleTouch(hObject);
}

void CBodyStateStairs::Save(ILTMessage_Write *pMsg)
{
	CBodyState::Save(pMsg);

	SAVE_VECTOR(m_vDir);
	SAVE_COBJECT(m_pVolume);
	SAVE_BOOL(m_bFell);
}

void CBodyStateStairs::Load(ILTMessage_Read *pMsg)
{
	CBodyState::Load(pMsg);

	LOAD_VECTOR(m_vDir);
	LOAD_COBJECT(m_pVolume, AIVolume);
	LOAD_BOOL(m_bFell);
}

// CBodyStateLedge

CBodyStateLedge::CBodyStateLedge()
{
	m_fTimer = 0.0f;
    m_vVelocity = LTVector(0,0,0);
	m_pVolume = LTNULL;
	m_eState = eLeaning;
}

void CBodyStateLedge::Init(Body* pBody)
{
	CBodyState::Init(pBody);

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);
    
	// Use the animation dims so we don't clip into the world...

    LTVector	vDims;
	uint32		dwAni = g_pLTServer->GetModelAnimation( m_pBody->m_hObject );
	
	if (dwAni != INVALID_ANI)
	{
		g_pCommonLT->GetModelAnimUserDims( m_pBody->m_hObject, &vDims, dwAni );
	}
	else // use normal dims...
	{
		g_pPhysicsLT->GetObjectDims( m_pBody->m_hObject, &vDims );
	}

	// Shrink the dims so the body won't collide with anything while falling.

	vDims.x = 1.f;
	vDims.z = 1.f;
	g_pPhysicsLT->SetObjectDims(m_pBody->m_hObject, &vDims, 0);


	AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, vPos, eAxisAll, 64.0f/*vDims.y*/);
	_ASSERT(pVolume && (pVolume->GetVolumeType() == AIVolume::kVolumeType_Ledge));
	if ( !pVolume || (pVolume->GetVolumeType() != AIVolume::kVolumeType_Ledge) )
	{
		m_pBody->SetState(kState_BodyNormal);
		return;
	}

	m_pVolume = pVolume;

	// If we are more than 180' off the ledge dir, play the backwards falling animation

	LTRotation rRot; 
	g_pLTServer->GetObjectRotation(m_pBody->m_hObject, &rRot);
	LTVector vForward = rRot.Forward();

	if ( pVolume->GetForward().Dot(vForward) > 0.0f )
	{
		m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedgeStart");
		if (m_hAniStart == INVALID_ANI)
		{
			m_hAniStart = 0;
			g_pLTServer->CPrint("Body missing animation \"FallOffLedgeStart\"");
		}

	}
	else
	{
		m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedgeBack");
		if (m_hAniStart == INVALID_ANI)
		{
			m_hAniStart = 0;
			g_pLTServer->CPrint("Body missing animation \"FallOffLedgeStart\"");
		}
	}


    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedge");
	if (m_hAniLoop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallOffLedge\"");
	}

    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedgeStop");
	if (m_hAniStop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallOffLedge\"");
	}

	// Face over the ledge

	m_pBody->FaceDir(pVolume->GetForward());

	// Start us leaning over the ledge

	m_eState = eLeaning;

    g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStart);
    g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	// Get the length of the animation

    ANIMTRACKERID pat;
	g_pModelLT->GetMainTracker(m_pBody->m_hObject, pat);

    uint32 dwLength;
	g_pModelLT->GetCurAnimLength(m_pBody->m_hObject, pat, dwLength);
    m_fTimer = (LTFLOAT)dwLength/1000.0f;

	// Figure out the dims of the volume

	LTFLOAT fWidth = (LTFLOAT)fabs(pVolume->GetFrontTopLeft().x - pVolume->GetFrontTopRight().x);
    LTFLOAT fLength = (LTFLOAT)fabs(pVolume->GetFrontTopLeft().z - pVolume->GetBackTopLeft().z);

    m_vVelocity = pVolume->GetForward()*(24.0f + Min<LTFLOAT>(fWidth, fLength) + vDims.x*2.0f)/m_fTimer;

	// Teleport us to the edge of the volume

	LTVector vPosition;
	g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPosition);

	if ( pVolume->GetForward().x > MATH_EPSILON )
	{
		vPosition.x = pVolume->GetFrontTopRight().x;
	}
	else if ( pVolume->GetForward().x < -MATH_EPSILON )
	{
		vPosition.x = pVolume->GetFrontTopLeft().x;
	}

	if ( pVolume->GetForward().z > MATH_EPSILON )
	{
		vPosition.z = pVolume->GetFrontTopRight().z;
	}
	else if ( pVolume->GetForward().z < -MATH_EPSILON )
	{
		vPosition.z = pVolume->GetBackTopLeft().z;
	}

	g_pLTServer->MoveObject(m_pBody->m_hObject, &vPosition);
}

void CBodyStateLedge::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

	m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedgeStart");
	if (m_hAniStart == INVALID_ANI)
	{
		m_hAniStart = 0;
		g_pLTServer->CPrint("Body missing animation \"FallOffLedgeStart\"");
	}

    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedge");
	if (m_hAniLoop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallOffLedge\"");
	}

    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedgeStop");
	if (m_hAniStop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallOffLedge\"");
	}
}

void CBodyStateLedge::Update()
{
	CBodyState::Update();

	if ( m_eState == eLanding )
	{
        HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_pBody->m_hObject);
		if ( hAni == m_hAniStop )
		{
			// If the land ani is done, we're done with the whole state...

            if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
			{
				m_pBody->SetState(kState_BodyNormal);
				return;
			}
		}
	}
	else if ( m_eState == eFalling )
	{
		// Just fall... HandleTouch will kick us into the landing state
	}
	else if ( m_eState == eLeaning )
	{
        HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_pBody->m_hObject);
        if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
		{
			// If the start ani ends before we're done, do the falling ani.

            g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniLoop);
            g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTTRUE);
			m_eState = eFalling;

			// Teleport us over the edge

			AIVolume* pVolume = m_pVolume;

			LTVector vPosition;
			g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPosition);

			if ( pVolume->GetForward().x > MATH_EPSILON )
			{
				vPosition.x = pVolume->GetFrontTopRight().x + 48.0f;
			}
			else if ( pVolume->GetForward().x < -MATH_EPSILON )
			{
				vPosition.x = pVolume->GetFrontTopLeft().x - 48.0f;
			}

			if ( pVolume->GetForward().z > MATH_EPSILON )
			{
				vPosition.z = pVolume->GetFrontTopRight().z + 48.0f;
			}
			else if ( pVolume->GetForward().z < -MATH_EPSILON )
			{
				vPosition.z = pVolume->GetBackTopLeft().z - 48.0f;
			}

			g_pLTServer->MoveObject(m_pBody->m_hObject, &vPosition);

			// Turn all the goodness back on

			g_pCommonLT->SetObjectFlags(m_pBody->m_hObject, OFT_Flags, FLAG_GRAVITY, FLAG_GRAVITY | FLAG_GOTHRUWORLD);

			g_pPhysicsLT->SetVelocity(m_pBody->m_hObject, &LTVector(pVolume->GetForward().x*100.0f,-500,pVolume->GetForward().y*100.0f));
		}
	}
}

void CBodyStateLedge::HandleTouch(HOBJECT hObject)
{
	CBodyState::HandleTouch(hObject);

	if ( m_eState == eFalling )
	{
        g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStop);
        g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

		m_eState = eLanding;
	}
}

void CBodyStateLedge::Save(ILTMessage_Write *pMsg)
{
	CBodyState::Save(pMsg);

	SAVE_FLOAT(m_fTimer);
	SAVE_VECTOR(m_vVelocity);
	SAVE_COBJECT(m_pVolume);
	SAVE_DWORD(m_eState);
}

void CBodyStateLedge::Load(ILTMessage_Read *pMsg)
{
	CBodyState::Load(pMsg);

	LOAD_FLOAT(m_fTimer);
	LOAD_VECTOR(m_vVelocity);
	LOAD_COBJECT(m_pVolume, AIVolume);
	LOAD_DWORD_CAST(m_eState, State);
}

// CBodyStateLadder

CBodyStateLadder::CBodyStateLadder()
{
	m_fTimer = 0.0f;
	m_pVolume = LTNULL;
	m_eState = eLeaning;
}

void CBodyStateLadder::Init(Body* pBody)
{
	CBodyState::Init(pBody);

    LTVector vPos, vDims;
    g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);
    g_pPhysicsLT->GetObjectDims(m_pBody->m_hObject, &vDims);

	// Shrink the dims so the body won't collide with anything while falling.

	vDims.x = 1.f;
	vDims.z = 1.f;
	g_pPhysicsLT->SetObjectDims(m_pBody->m_hObject, &vDims, 0);

	AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, vPos, eAxisAll, 64.0f/*vDims.y*/);
	_ASSERT(pVolume && (pVolume->GetVolumeType() == AIVolume::kVolumeType_Ladder));
	if ( !pVolume || (pVolume->GetVolumeType() != AIVolume::kVolumeType_Ladder) )
	{
		m_pBody->SetState(kState_BodyNormal);
		return;
	}

	m_pVolume = pVolume;

	m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLadderStart");
	if (m_hAniStart == INVALID_ANI)
	{
		m_hAniStart = 0;
		g_pLTServer->CPrint("Body missing animation \"FallOffLadderStart\"");
	}

    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLadder");
	if (m_hAniLoop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallOffLadder\"");
	}

    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLadderStop");
	if (m_hAniStop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallOffLadderStop\"");
	}

	// Start us leaning over the ladder

	m_eState = eLeaning;

    g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStart);
    g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	// Get the length of the animation

    ANIMTRACKERID pat;
	g_pModelLT->GetMainTracker(m_pBody->m_hObject, pat);

    uint32 dwLength;
	g_pModelLT->GetCurAnimLength(m_pBody->m_hObject, pat, dwLength);
    m_fTimer = (LTFLOAT)dwLength/1000.0f;

	// Move away from the ladder.

	g_pPhysicsLT->SetVelocity(m_pBody->m_hObject, &LTVector( -pVolume->GetForward().x*10.f, 0.f, -pVolume->GetForward().z*10.f ) );
}

void CBodyStateLadder::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

	m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLadderStart");
	if (m_hAniStart == INVALID_ANI)
	{
		m_hAniStart = 0;
		g_pLTServer->CPrint("Body missing animation \"FallOffLadderStart\"");
	}

    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLadder");
	if (m_hAniLoop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallOffLadder\"");
	}

    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLadderStop");
	if (m_hAniStop == INVALID_ANI)
	{
		g_pLTServer->CPrint("Body missing animation \"FallOffLadderStop\"");
	}
}

void CBodyStateLadder::Update()
{
	CBodyState::Update();

	if ( m_eState == eLanding )
	{
        HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_pBody->m_hObject);
		if ( hAni == m_hAniStop )
		{
			// If the land ani is done, we're done with the whole state...

            if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
			{
				m_pBody->SetState(kState_BodyNormal);
				return;
			}
		}
	}
	else if ( m_eState == eFalling )
	{
		// Just fall... HandleTouch will kick us into the landing state
	}
	else if ( m_eState == eLeaning )
	{
        HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_pBody->m_hObject);
        if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
		{
			// If the start ani ends before we're done, do the falling ani.

            g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniLoop);
            g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTTRUE);
			m_eState = eFalling;

			// Turn all the goodness back on

			g_pCommonLT->SetObjectFlags(m_pBody->m_hObject, OFT_Flags, FLAG_GRAVITY, FLAG_GRAVITY | FLAG_GOTHRUWORLD);

			g_pPhysicsLT->SetVelocity(m_pBody->m_hObject, &LTVector( 0.f, -500.f, 0.f ) );
		}
	}
}

void CBodyStateLadder::HandleTouch(HOBJECT hObject)
{
	CBodyState::HandleTouch(hObject);

	if ( m_eState == eFalling )
	{
        g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStop);
        g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

		m_eState = eLanding;
	}
}

void CBodyStateLadder::Save(ILTMessage_Write *pMsg)
{
	CBodyState::Save(pMsg);

	SAVE_FLOAT(m_fTimer);
	SAVE_COBJECT(m_pVolume);
	SAVE_DWORD(m_eState);
}

void CBodyStateLadder::Load(ILTMessage_Read *pMsg)
{
	CBodyState::Load(pMsg);

	LOAD_FLOAT(m_fTimer);
	LOAD_COBJECT(m_pVolume, AIVolume);
	LOAD_DWORD_CAST(m_eState, State);
}

// CBodyStateUnderwater

CBodyStateUnderwater::CBodyStateUnderwater()
{
    m_bStop = LTFALSE;
}

void CBodyStateUnderwater::Init(Body* pBody)
{
	CBodyState::Init(pBody);

    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "SwimDeathStart");
    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "SwimDeath");
    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "SwimDeathStop");

	g_pCommonLT->SetObjectFlags(m_pBody->m_hObject, OFT_Flags, 0, FLAG_GOTHRUWORLD);
}

void CBodyStateUnderwater::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "SwimDeathStart");
    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "SwimDeath");
    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "SwimDeathStop");
}

void CBodyStateUnderwater::Update()
{
	CBodyState::Update();

	// Update our animations

    HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_pBody->m_hObject);
	if ( hAni == m_hAniStart )
	{
        if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
		{
            g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniLoop);
            g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTTRUE);
		}
	}
	else if ( hAni == m_hAniLoop )
	{
		// Drift us downwards

        LTVector vPos;
		g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);
        vPos += LTVector(0,-36.0f,0)*g_pLTServer->GetFrameTime();
		g_pLTServer->MoveObject(m_pBody->m_hObject, &vPos);

		if ( m_bStop )
		{
            g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStop);
            g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);
		}
	}
	else if ( hAni == m_hAniStop )
	{
        if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
		{
			m_pBody->SetState(kState_BodyNormal);
		}
	}
	else
	{
        g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStart);
        g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);
	}
}

void CBodyStateUnderwater::HandleTouch(HOBJECT hObject)
{
	CBodyState::HandleTouch(hObject);

	if ( IsMainWorld(hObject) || (OT_WORLDMODEL == GetObjectType(hObject)) )
	{
        m_bStop = LTTRUE;
	}
}

void CBodyStateUnderwater::Save(ILTMessage_Write *pMsg)
{
	CBodyState::Save(pMsg);

	SAVE_BOOL(m_bStop);
}

void CBodyStateUnderwater::Load(ILTMessage_Read *pMsg)
{
	CBodyState::Load(pMsg);

	LOAD_BOOL(m_bStop);
}

// CBodyStateLaser

void CBodyStateLaser::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	static const char* aszDeathLasers[] = { "DLaser1", "DLaser2", "DLaser3", "DLaser4" };
    static const int cDeathLasers = sizeof(aszDeathLasers)/sizeof(const char*);

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, (char*)aszDeathLasers[GetRandom(0, cDeathLasers-1)]);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_fRemoveTime = g_pLTServer->GetTime() + 3.5f;

	m_pBody->SetState(kState_BodyFade);
}

void CBodyStateLaser::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

void CBodyStateLaser::Update()
{
	CBodyState::Update();
}

void CBodyStateLaser::Save(ILTMessage_Write *pMsg)
{
	CBodyState::Save(pMsg);

	SAVE_TIME(m_fRemoveTime);
}

void CBodyStateLaser::Load(ILTMessage_Read *pMsg)
{
	CBodyState::Load(pMsg);

	LOAD_TIME(m_fRemoveTime);
}

// CBodyStateDecay

void CBodyStateDecay::Init(Body* pBody)
{
	CBodyState::Init(pBody);
	m_pBody->SetState(kState_BodyFade);
}

void CBodyStateDecay::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

void CBodyStateDecay::Update()
{
	CBodyState::Update();
}

void CBodyStateDecay::Save(ILTMessage_Write *pMsg)
{
	CBodyState::Save(pMsg);

	SAVE_TIME(m_fRemoveTime);
}

void CBodyStateDecay::Load(ILTMessage_Read *pMsg)
{
	CBodyState::Load(pMsg);

	LOAD_TIME(m_fRemoveTime);
}

// CBodyStateFade

CBodyStateFade::CBodyStateFade()
{
	m_fRemoveTime = -1.0f;
}

void CBodyStateFade::Init(Body* pBody)
{
	// Can't fade out permanent bodies...
	if (pBody->IsPermanentBody())
	{
		pBody->SetState(kState_BodyNormal);
		return;
	}

	CBodyState::Init(pBody);

	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);
}

void CBodyStateFade::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

void CBodyStateFade::Update()
{
	CBodyState::Update();

	if ( MS_PLAYDONE & g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) )
	{
		if ( m_fRemoveTime < 0.0f )
		{
			if (!s_RemovePowerups.GetFloat())
			{
				m_pBody->ReleasePowerups();
			}

			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_SFX_MESSAGE);
			cMsg.Writeuint8(SFX_BODY_ID);
			cMsg.WriteObject(m_pBody->m_hObject);
			cMsg.Writeuint8(BFX_FADE_MSG);
			g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

			m_fRemoveTime = g_pLTServer->GetTime() + 3.5f;
		}

		if ( g_pLTServer->GetTime() > m_fRemoveTime )
		{
			if (s_RemovePowerups.GetFloat())
			{
				m_pBody->RemovePowerupObjects();
			}

			g_pLTServer->RemoveObject( m_pBody->m_hObject );
			m_pBody = NULL;
		}
	}
}

void CBodyStateFade::Save(ILTMessage_Write *pMsg)
{
	CBodyState::Save(pMsg);

	SAVE_TIME(m_fRemoveTime);
}

void CBodyStateFade::Load(ILTMessage_Read *pMsg)
{
	CBodyState::Load(pMsg);

	LOAD_TIME(m_fRemoveTime);
}

// CBodyStateCrush

void CBodyStateCrush::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	static char* aszCrushDeaths[] = { "DCrush" };
    static int cCrushDeaths = sizeof(aszCrushDeaths)/sizeof(char*);
	char* szDeath = aszCrushDeaths[GetRandom(0, cCrushDeaths-1)];

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, szDeath);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_pBody->SetState(kState_BodyNormal);
}

void CBodyStateCrush::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

// CBodyStateChair

void CBodyStateChair::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	static char* aszChairDeaths[] = { "DSit2" /*, "DSitFall"*/ };
    static int cChairDeaths = sizeof(aszChairDeaths)/sizeof(char*);
	char* szDeath = aszChairDeaths[GetRandom(0, cChairDeaths-1)];

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, szDeath);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_pBody->SetState(kState_BodyNormal);
}

void CBodyStateChair::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

// CBodyStatePoison

void CBodyStatePoison::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	static char* aszPoisonDeaths[] = { "DPoison" };
    static int cPoisonDeaths = sizeof(aszPoisonDeaths)/sizeof(char*);
	char* szDeath = aszPoisonDeaths[GetRandom(0, cPoisonDeaths-1)];

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, szDeath);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_pBody->SetState(kState_BodyNormal);
}

void CBodyStatePoison::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

// CBodyStateAcid

void CBodyStateAcid::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	static char* aszAcidDeaths[] = { "DAcid" };
    static int cAcidDeaths = sizeof(aszAcidDeaths)/sizeof(char*);
	char* szDeath = aszAcidDeaths[GetRandom(0, cAcidDeaths-1)];

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, szDeath);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_pBody->SetState(kState_BodyNormal);
}

void CBodyStateAcid::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

// CBodyStateArrow

void CBodyStateArrow::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	// Determine if the arrow came from the front or the back...

	LTVector vDir = pBody->GetDeathDir();

    LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_pBody->m_hObject, &rRot);
	bool bFront = (vDir.Dot(rRot.Forward()) < 0.0f);

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, bFront ? g_szArrowFrontDeath : g_szArrowBackDeath);

	if (INVALID_ANI != hAni)
	{
		g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
		g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

		LTVector vPos;
		g_pLTServer->GetObjectPos(pBody->m_hObject, &vPos);

		float fDir = (bFront ? 1.0f : -1.0f);

		LTVector vForward = rRot.Forward();

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		IQuery.m_From = vPos;
		IQuery.m_To = (vPos - (fDir * vForward * g_BodyStickDist.GetFloat()));
		IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		IQuery.m_FilterFn = GroundFilterFn;
		IQuery.m_PolyFilterFn = LTNULL;

		if (g_pLTServer->IntersectSegment(&IQuery, &IInfo) && IsMainWorld(IInfo.m_hObject))
		{
			LTVector vDims, vPos;
			g_pPhysicsLT->GetObjectDims(pBody->m_hObject, &vDims);
			vPos = IInfo.m_Point;
			vPos += (fDir * vForward * vDims.x);
			g_pLTServer->TeleportObject(pBody->m_hObject, &vPos);
		}
	}

	pBody->SetState(kState_BodyNormal);
}

void CBodyStateArrow::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

// CBodyStateExplode

CBodyStateExplode::CBodyStateExplode()
{
	m_bLand = LTFALSE;
}

void CBodyStateExplode::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	LTVector vDir = pBody->GetDeathDir();
	if ( vDir.y < 0.0 )
	{
		vDir.y = 0.01f;
	}

	LTRotation rRot; 
	g_pLTServer->GetObjectRotation(m_pBody->m_hObject, &rRot);
	LTVector vForward = rRot.Forward();

	LTBOOL bForward = LTTRUE;

	if ( vForward.Dot(vDir) < 0.0 )
	{
		bForward = LTFALSE;
	}

	m_pBody->FaceDir(bForward ? vDir : -vDir);

	// DBnB, DBnF

	if ( bForward )
	{
		m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXFStart");
		m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXFLoop");
		m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXFStop");
	}
	else
	{
		m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXBStart");
		m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXBLoop");
		m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXBStop");
	}

    g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStart);
    g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	// Make us non solid

	g_pCommonLT->SetObjectFlags(m_pBody->m_hObject, OFT_Flags, FLAG_GRAVITY, FLAG_GRAVITY | FLAG_GOTHRUWORLD);

	// Give us some velocity

	static CVarTrack cvBodyExplodeVelocity;
	if ( !cvBodyExplodeVelocity.IsInitted() )
	{
        cvBodyExplodeVelocity.Init(g_pLTServer, "BodyExplodeVelocity", LTNULL, 200.0f);
	}

    LTVector vTemp(vDir*cvBodyExplodeVelocity.GetFloat());
	g_pPhysicsLT->SetVelocity(m_pBody->m_hObject, &vTemp);
	//g_pLTServer->CPrint("CBodyStateExplode Vel = %f, %f, %f", vTemp.x, vTemp.y, vTemp.z);
}

void CBodyStateExplode::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

	m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXFStart");
	m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXFLoop");
	m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXFStop");
}

bool FlyingFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return false;

	// We only care about solid non-player objects...

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags(hObj, OFT_Flags, dwFlags);

	if (dwFlags & FLAG_SOLID)
	{
		return (IsPlayer(hObj) ? false : true);
	}

	return false;
}

void CBodyStateExplode::HandleTouch(HOBJECT hObject)
{
	LTVector vVelocity;
	g_pPhysicsLT->GetVelocity(m_pBody->m_hObject, &vVelocity);

	if (vVelocity.Length() < 5.0f || vVelocity.y < 5.0f )
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		// Use the animation dims so we don't clip into the world...

		LTVector	vDims;
		uint32		dwAni = g_pLTServer->GetModelAnimation( m_pBody->m_hObject );
		
		if (dwAni != INVALID_ANI)
		{
			g_pCommonLT->GetModelAnimUserDims( m_pBody->m_hObject, &vDims, dwAni );
		}
		else // use normal dims...
		{
			g_pPhysicsLT->GetObjectDims( m_pBody->m_hObject, &vDims );
		}

		IQuery.m_From = vPos;
		IQuery.m_To = vPos - LTVector(0, vDims.y*2.0f, 0);
		IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
		IQuery.m_FilterFn = FlyingFilterFn;

		g_cIntersectSegmentCalls++;
		if ( g_pLTServer->IntersectSegment(&IQuery, &IInfo) )
		{
			g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStop);
			g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

			m_bLand = LTTRUE;
			m_vLandPos = IInfo.m_Point + LTVector(0, vDims.y, 0);
		}
	}
}

void CBodyStateExplode::Update()
{
	CBodyState::Update();

	bool bSetToRestingState = false;
	if ( m_pBody && ((g_pLTServer->GetTime() - m_pBody->GetStarttime()) > g_BodyStateTimeout.GetFloat()) )
	{
		// If we're in the air, Force us to the ground just in case...
		LTVector vVelocity(0, 0, 0);
		g_pPhysicsLT->SetVelocity(m_pBody->m_hObject, &vVelocity);
		HandleTouch(LTNULL);

		g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStop);
		g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

		bSetToRestingState = true;
	}

	if ( m_bLand || bSetToRestingState )
	{
		if ( m_bLand )
		{
			g_pLTServer->TeleportObject(m_pBody->m_hObject, &m_vLandPos);
		}

		SetRestingState();
		return;
	}

    if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
	{
		if ( g_pLTServer->GetModelAnimation(m_pBody->m_hObject) == m_hAniStart )
		{
			g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniLoop);
			g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTTRUE);
		}

		return;
	}
}

void CBodyStateExplode::SetRestingState()
{
	// If the body is no longer in an AI volume we need to remove it..
	bool bInAIVolume = true;


	if (g_kfDefaultBodyExplosionVelocity < g_vtBodyExplosionVelMultiplier.GetFloat())
	{
		// If we're doing the cheat code our multiplier will be higher than the default
		// In this case, don't bother checking for body being in a volume...
	}
	else
	{
		if ( g_pAIVolumeMgr->IsInitialized() )
		{
			LTVector vPos;
			g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);

			const float fVerticalThreshold = 100.0f;  // Same as AI use
			AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, vPos, eAxisAll, fVerticalThreshold, LTNULL);
			bInAIVolume = !!(pVolume);
		}
	}

	// If this is a body of an AI and it gets out of aivolume, we need to fade it out.  If it's
	// a player don't fade it out, because it must transform to a backpack.  The backback
	// will fade on its own.
	CPlayerObj* pPlayer = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( m_pBody->GetCharacter( )));
	if( pPlayer || bInAIVolume )
	{
		m_pBody->SetState(kState_BodyNormal);
	}
	else
	{
		m_pBody->SetState(kState_BodyFade);
	}
}

void CBodyStateExplode::Save(ILTMessage_Write *pMsg)
{
	CBodyState::Save(pMsg);

	SAVE_BOOL(m_bLand);
	SAVE_VECTOR(m_vLandPos);
}

void CBodyStateExplode::Load(ILTMessage_Read *pMsg)
{
	CBodyState::Load(pMsg);

	LOAD_BOOL(m_bLand);
	LOAD_VECTOR(m_vLandPos);
}


// CBodyStateCarried

void CBodyStateCarried::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	//can't see it or touch it while being carried
	g_pCommonLT->SetObjectFlags(m_pBody->m_hObject, OFT_Flags, 
		FLAG_FORCECLIENTUPDATE, 	FLAG_TOUCH_NOTIFY | FLAG_RAYHIT | FLAG_FORCECLIENTUPDATE);

	g_pCommonLT->SetObjectFlags(m_pBody->m_hObject, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3);

	//deactivate hit box
	if (m_pBody->GetHitBox())
	{
		g_pCommonLT->SetObjectFlags(m_pBody->GetHitBox(), OFT_Flags, 0, FLAG_RAYHIT);
	}


	m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "carrybody");
	if (m_hAniStart == INVALID_ANI)
	{
		m_hAniStart = 0;
		g_pLTServer->CPrint("Body missing animation \"carrybody\"");
	}

}

void CBodyStateCarried::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "carrybody");
	if (m_hAniStart == INVALID_ANI)
	{
		m_hAniStart = 0;
		g_pLTServer->CPrint("Body missing animation \"carrybody\"");
	}
}

void CBodyStateCarried::Update()
{
	CBodyState::Update();

    HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_pBody->m_hObject);
	if ( hAni != m_hAniStart )
	{
        g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStart);
        g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);
		g_pLTServer->ResetModelAnimation(m_pBody->m_hObject);
	}
}


// CBodyStateDropped

void CBodyStateDropped::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	g_pCommonLT->SetObjectFlags(m_pBody->m_hObject, OFT_User, 0, USRFLG_ATTACH_HIDE1SHOW3);

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);

	AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, vPos, eAxisAll, 64.0f);
	_ASSERT(pVolume);
	if (pVolume)
	{
		if (pVolume->GetVolumeType() == AIVolume::kVolumeType_Ledge)
		{
			//turn it 180'
			LTRotation rRot; 
			g_pLTServer->GetObjectRotation(m_pBody->m_hObject, &rRot);
			rRot.Rotate(LTVector(0.0f, 1.0f, 0.0f), MATH_PI);
			g_pLTServer->SetObjectRotation(m_pBody->m_hObject, &rRot);

			m_pBody->SetState(kState_BodyLedge);
			return;
		}
		if (pVolume->GetVolumeType() == AIVolume::kVolumeType_Stairs)
		{
			LTRotation rRot; 
			g_pLTServer->GetObjectRotation(m_pBody->m_hObject, &rRot);
			rRot.Rotate(LTVector(0.0f, 1.0f, 0.0f), MATH_PI);
			g_pLTServer->SetObjectRotation(m_pBody->m_hObject, &rRot);

			m_pBody->SetState(kState_BodyStairs);
			return;
		}

	}


	m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "dropbody");
	if (m_hAniStart == INVALID_ANI)
	{
		m_hAniStart = 0;
		g_pLTServer->CPrint("Body missing animation \"dropbody\"");
	}
}

void CBodyStateDropped::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "dropbody");
	if (m_hAniStart == INVALID_ANI)
	{
		m_hAniStart = 0;
		g_pLTServer->CPrint("Body missing animation \"dropbody\"");
	}
}

void CBodyStateDropped::Update()
{
	CBodyState::Update();

    HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_pBody->m_hObject);
	if ( hAni == m_hAniStart )
	{
        if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
		{
//			if (!g_pServerMissionMgr->GetAllowBodies())
			{
				HOBJECT hFilterList[] = {m_pBody->m_hObject, m_pBody->GetHitBox(), LTNULL};
				MoveObjectToFloor(m_pBody->m_hObject,hFilterList);
			}
			m_pBody->SetState(kState_BodyNormal);
			return;
		}
	}
	else
	{
		g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStart);
		g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);
		g_pLTServer->ResetModelAnimation(m_pBody->m_hObject);

	}

}
