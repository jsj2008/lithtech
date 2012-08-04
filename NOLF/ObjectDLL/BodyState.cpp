// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#include "StdAfx.h"
#include "BodyState.h"
#include "Body.h"
#include "AIVolumeMgr.h"
#include "GameServerShell.h"
#include "MsgIDs.h"
#include "Attachments.h"

static CVarTrack s_LifetimeTrack;
extern CVarTrack g_BodyStickDist;
extern CVarTrack g_BodyStateTimeout;

IMPLEMENT_FACTORY(CBodyStateNormal, 0)
IMPLEMENT_FACTORY(CBodyStateExplode, 0)
IMPLEMENT_FACTORY(CBodyStateStairs, 0)
IMPLEMENT_FACTORY(CBodyStateLedge, 0)
IMPLEMENT_FACTORY(CBodyStateUnderwater, 0)
IMPLEMENT_FACTORY(CBodyStateLaser, 0)
IMPLEMENT_FACTORY(CBodyStateDecay, 0)
IMPLEMENT_FACTORY(CBodyStateCrush, 0)
IMPLEMENT_FACTORY(CBodyStateChair, 0)
IMPLEMENT_FACTORY(CBodyStatePoison, 0)
IMPLEMENT_FACTORY(CBodyStateAcid, 0)
IMPLEMENT_FACTORY(CBodyStateArrow, 0)
IMPLEMENT_FACTORY(CBodyStateFade, 0)

// CBodyState

void CBodyState::Constructor()
{
    m_pBody = LTNULL;
}

void CBodyState::Destructor()
{
}

void CBodyState::Init(Body* pBody)
{
	m_pBody = pBody;

	// Turn off gravity

	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_pBody->m_hObject);

	dwFlags &= ~FLAG_SOLID;
	dwFlags &= ~FLAG_ANIMTRANSITION;
	dwFlags &= ~FLAG_GRAVITY;
	//dwFlags &= ~FLAG_ENVIRONMENTMAP;

	dwFlags |= FLAG_GOTHRUWORLD;
	dwFlags |= FLAG_TOUCH_NOTIFY;
	dwFlags |= FLAG_RAYHIT;

    g_pLTServer->SetObjectFlags(m_pBody->m_hObject, dwFlags);

    g_pLTServer->SetVelocity(m_pBody->m_hObject, &LTVector(0,0,0));
    g_pLTServer->SetAcceleration(m_pBody->m_hObject, &LTVector(0,0,0));
}

void CBodyState::InitLoad(Body* pBody)
{
	m_pBody = pBody;
}

// CBodyStateNormal

void CBodyStateNormal::Constructor()
{
	CBodyState::Constructor();
}

void CBodyStateNormal::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateNormal::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	if(!s_LifetimeTrack.IsInitted())
	{
        s_LifetimeTrack.Init(g_pLTServer, "BodyLifetime", NULL, 30.0f);
	}
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

	if ( g_pGameServerShell->GetFadeBodies() )
	{
		m_pBody->SetState(eBodyStateFade);
		return;
	}

    LTFLOAT fTime = g_pLTServer->GetTime();

	if (!m_pBody->m_hObject || !g_pGameServerShell) return;

	if (g_pGameServerShell->GetGameType() == SINGLE)
	{
		// If we have > 0 lifetime, see if we should remove ourselves

		if ( (m_pBody->GetLifetime() > 0) && (fTime > m_pBody->GetStarttime() + m_pBody->GetLifetime()) )
		{
			m_pBody->SetState(eBodyStateFade);
			return;
		}
	}
	else if (fTime >= m_pBody->GetStarttime() + s_LifetimeTrack.GetFloat(30.0f))
	{
		m_pBody->SetState(eBodyStateFade);
		return;
	}
}

LTBOOL CBodyStateNormal::CanDeactivate()
{
	if (g_pGameServerShell->GetGameType() == SINGLE)
	{
		// If we have > 0 lifetime, see if we should remove ourselves

		if ( m_pBody->GetLifetime() > 0 )
		{
			return LTFALSE;
		}
		else
		{
			return LTTRUE;
		}
	}
	else
	{
		return LTFALSE;
	}
}

// CBodyStateStairs

void CBodyStateStairs::Constructor()
{
	CBodyState::Constructor();

    m_vDir = LTVector(0,0,0);
	m_iVolume = -1;
    m_bFell = LTFALSE;
}

void CBodyStateStairs::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateStairs::Init(Body* pBody)
{
	CBodyState::Init(pBody);

    LTVector vPos, vDims;
    g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);
    g_pLTServer->GetObjectDims(m_pBody->m_hObject, &vDims);

	CAIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(vPos, 64.0f/*vDims.y*/);
	_ASSERT(pVolume && pVolume->HasStairs());
	if ( !pVolume || !pVolume->HasStairs() )
	{
		m_pBody->SetState(eBodyStateNormal);
		return;
	}

	m_iVolume = pVolume->GetIndex();
	m_vDir = pVolume->GetStairsDir();

	// If we are more than 180' off the stairs dir, play the backwards falling animation

	LTRotation rRot; LTVector vNull, vForward;
	g_pLTServer->GetObjectRotation(m_pBody->m_hObject, &rRot);
	g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);

	if ( m_vDir.Dot(vForward) > 0.0f )
	{
	    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairsStart");
	}
	else
	{
	    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairsBack");
	}

    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairs");
    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairsStop");

	// Face down the stairs

	m_pBody->FaceDir(pVolume->GetStairsDir());
}

void CBodyStateStairs::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairsStart");
    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairs");
    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallDownStairsStop");
}

void CBodyStateStairs::Update()
{
	CBodyState::Update();

    LTVector vPos, vDims;
    g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);
    g_pLTServer->GetObjectDims(m_pBody->m_hObject, &vDims);

	CAIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(vPos, 64.0f/*vDims.y*2.0f*/, m_iVolume == -1 ? LTNULL : g_pAIVolumeMgr->GetVolumeByIndex(m_iVolume), LTFALSE);
    LTBOOL bInside = pVolume && pVolume->HasStairs();

	if ( !bInside )
	{
		CAIVolume* pLastVolume = g_pAIVolumeMgr->GetVolumeByIndex(m_iVolume);

        HMODELANIM hAni = g_pLTServer->GetModelAnimation(m_pBody->m_hObject);
		if ( hAni == m_hAniStop )
		{
            if ( g_pLTServer->GetModelPlaybackState(m_pBody->m_hObject) & MS_PLAYDONE )
			{
				m_pBody->SetState(eBodyStateNormal);
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
			LTVector vNewPos = vPos + pLastVolume->GetStairsDir()*g_pLTServer->GetFrameTime()*fStairsFallStopSpeed;
			g_pLTServer->MoveObject(m_pBody->m_hObject, &vNewPos);
		}
	}
	else
	{
		if ( pVolume->GetIndex() != m_iVolume )
		{
			m_iVolume = pVolume->GetIndex();
			m_pBody->FaceDir(pVolume->GetStairsDir());
		}

		if ( m_bFell )
		{
			const static LTFLOAT fStairsFallSpeed = g_pServerButeMgr->GetBodyStairsFallSpeed();
            LTVector vNewPos = vPos + pVolume->GetStairsDir()*g_pLTServer->GetFrameTime()*fStairsFallSpeed;

			IntersectQuery IQuery;
			IntersectInfo IInfo;

			VEC_COPY(IQuery.m_From, vNewPos);
            VEC_COPY(IQuery.m_To, vNewPos+LTVector(0,-256.0f,0));
			IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
			IQuery.m_FilterFn = GroundFilterFn;

			g_cIntersectSegmentCalls++;
            if (g_pLTServer->IntersectSegment(&IQuery, &IInfo) && ((IsMainWorld(IInfo.m_hObject)) || (OT_WORLDMODEL == g_pLTServer->GetObjectType(IInfo.m_hObject))))
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

void CBodyStateStairs::Save(HMESSAGEWRITE hWrite)
{
	CBodyState::Save(hWrite);

	SAVE_VECTOR(m_vDir);
	SAVE_INT(m_iVolume);
	SAVE_BOOL(m_bFell);
}

void CBodyStateStairs::Load(HMESSAGEREAD hRead)
{
	CBodyState::Load(hRead);

	LOAD_VECTOR(m_vDir);
	LOAD_INT(m_iVolume);
	LOAD_BOOL(m_bFell);
}

// CBodyStateLedge

void CBodyStateLedge::Constructor()
{
	CBodyState::Constructor();

	m_fTimer = 0.0f;
    m_vVelocity = LTVector(0,0,0);
	m_iVolume = -1;
	m_eState = eLeaning;
}

void CBodyStateLedge::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateLedge::Init(Body* pBody)
{
	CBodyState::Init(pBody);

    LTVector vPos, vDims;
    g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);
    g_pLTServer->GetObjectDims(m_pBody->m_hObject, &vDims);

	CAIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(vPos, 64.0f/*vDims.y*/);
	_ASSERT(pVolume && pVolume->HasLedge());
	if ( !pVolume || !pVolume->HasLedge() )
	{
		m_pBody->SetState(eBodyStateNormal);
		return;
	}

	m_iVolume = pVolume->GetIndex();

	// If we are more than 180' off the ledge dir, play the backwards falling animation

	LTRotation rRot; LTVector vNull, vForward;
	g_pLTServer->GetObjectRotation(m_pBody->m_hObject, &rRot);
	g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);

	if ( pVolume->GetLedgeDir().Dot(vForward) > 0.0f )
	{
		m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedgeStart");
	}
	else
	{
		m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedgeBack");
	}

    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedge");
    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedgeStop");

	// Face over the ledge

	m_pBody->FaceDir(pVolume->GetLedgeDir());

	// Start us leaning over the ledge

	m_eState = eLeaning;

    g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStart);
    g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	// Get the length of the animation

    LTAnimTracker* pat;
	g_pModelLT->GetMainTracker(m_pBody->m_hObject, pat);

    uint32 dwLength;
	g_pModelLT->GetCurAnimLength(pat, dwLength);
    m_fTimer = (LTFLOAT)dwLength/1000.0f;

	// Figure out the dims of the volume

	LTFLOAT fWidth = (LTFLOAT)fabs(pVolume->GetFrontTopLeft().x - pVolume->GetFrontTopRight().x);
    LTFLOAT fLength = (LTFLOAT)fabs(pVolume->GetFrontTopLeft().z - pVolume->GetBackTopLeft().z);

    m_vVelocity = pVolume->GetLedgeDir()*(24.0f + Min<LTFLOAT>(fWidth, fLength) + vDims.x*2.0f)/m_fTimer;

	// Teleport us to the edge of the volume

	LTVector vPosition;
	g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPosition);

	if ( pVolume->GetLedgeDir().x > MATH_EPSILON )
	{
		vPosition.x = pVolume->GetFrontTopRight().x;
	}
	else if ( pVolume->GetLedgeDir().x < -MATH_EPSILON )
	{
		vPosition.x = pVolume->GetFrontTopLeft().x;
	}

	if ( pVolume->GetLedgeDir().z > MATH_EPSILON )
	{
		vPosition.z = pVolume->GetFrontTopRight().z;
	}
	else if ( pVolume->GetLedgeDir().z < -MATH_EPSILON )
	{
		vPosition.z = pVolume->GetBackTopLeft().z;
	}

	g_pLTServer->MoveObject(m_pBody->m_hObject, &vPosition);
}

void CBodyStateLedge::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

	m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedgeStart");
    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedge");
    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "FallOffLedgeStop");
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
				m_pBody->SetState(eBodyStateNormal);
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

			CAIVolume* pVolume = g_pAIVolumeMgr->GetVolumeByIndex(m_iVolume);

			LTVector vPosition;
			g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPosition);

			if ( pVolume->GetLedgeDir().x > MATH_EPSILON )
			{
				vPosition.x = pVolume->GetFrontTopRight().x + 48.0f;
			}
			else if ( pVolume->GetLedgeDir().x < -MATH_EPSILON )
			{
				vPosition.x = pVolume->GetFrontTopLeft().x - 48.0f;
			}

			if ( pVolume->GetLedgeDir().z > MATH_EPSILON )
			{
				vPosition.z = pVolume->GetFrontTopRight().z + 48.0f;
			}
			else if ( pVolume->GetLedgeDir().z < -MATH_EPSILON )
			{
				vPosition.z = pVolume->GetBackTopLeft().z - 48.0f;
			}

			g_pLTServer->MoveObject(m_pBody->m_hObject, &vPosition);

			// Turn all the goodness back on

            uint32 dwFlags = g_pLTServer->GetObjectFlags(m_pBody->m_hObject);
			dwFlags &= ~FLAG_GOTHRUWORLD;
			dwFlags |= FLAG_GRAVITY;
            g_pLTServer->SetObjectFlags(m_pBody->m_hObject, dwFlags);

			g_pLTServer->SetVelocity(m_pBody->m_hObject, &LTVector(pVolume->GetLedgeDir().x*100.0f,-500,pVolume->GetLedgeDir().y*100.0f));
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

void CBodyStateLedge::Save(HMESSAGEWRITE hWrite)
{
	CBodyState::Save(hWrite);

	SAVE_FLOAT(m_fTimer);
	SAVE_VECTOR(m_vVelocity);
	SAVE_INT(m_iVolume);
	SAVE_DWORD(m_eState);
}

void CBodyStateLedge::Load(HMESSAGEREAD hRead)
{
	CBodyState::Load(hRead);

	LOAD_FLOAT(m_fTimer);
	LOAD_VECTOR(m_vVelocity);
	LOAD_INT(m_iVolume);
	LOAD_DWORD_CAST(m_eState, State);
}

// CBodyStateUnderwater

void CBodyStateUnderwater::Constructor()
{
	CBodyState::Constructor();

    m_bStop = LTFALSE;
}

void CBodyStateUnderwater::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateUnderwater::Init(Body* pBody)
{
	CBodyState::Init(pBody);

    m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "SwimDeathStart");
    m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "SwimDeath");
    m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "SwimDeathStop");

	uint32 dwFlags = g_pLTServer->GetObjectFlags(m_pBody->m_hObject);
	dwFlags &= ~FLAG_GOTHRUWORLD;
//	dwFlags |= FLAG_GRAVITY;
	g_pLTServer->SetObjectFlags(m_pBody->m_hObject, dwFlags);
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
			m_pBody->SetState(eBodyStateNormal);
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

	if ( IsMainWorld(hObject) || (OT_WORLDMODEL == g_pLTServer->GetObjectType(hObject)) )
	{
        m_bStop = LTTRUE;
	}
}

void CBodyStateUnderwater::Save(HMESSAGEWRITE hWrite)
{
	CBodyState::Save(hWrite);

	SAVE_BOOL(m_bStop);
}

void CBodyStateUnderwater::Load(HMESSAGEREAD hRead)
{
	CBodyState::Load(hRead);

	LOAD_BOOL(m_bStop);
}

// CBodyStateLaser

void CBodyStateLaser::Constructor()
{
	CBodyState::Constructor();
}

void CBodyStateLaser::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateLaser::Init(Body* pBody)
{
	CBodyState::Init(pBody);
/*
	ObjectCreateStruct createstruct;
	createstruct.Clear();

	char szFileName[256], szSkinName[256];
    g_pLTServer->GetModelFilenames(pBody->m_hObject, szFileName, ARRAY_LEN(szFileName), szSkinName, ARRAY_LEN(szSkinName));

	SAFE_STRCPY(createstruct.m_Filename, szFileName);
	SAFE_STRCPY(createstruct.m_SkinNames[0], "SFX\\Debris\\Skins\\Feather.dtx");
	SAFE_STRCPY(createstruct.m_SkinNames[1], "SFX\\Debris\\Skins\\Feather.dtx");

    g_pLTServer->Common()->SetObjectFilenames(pBody->m_hObject, &createstruct);
*/
	static const char* aszDeathLasers[] = { "DLaser1", "DLaser2", "DLaser3", "DLaser4" };
    static const int cDeathLasers = sizeof(aszDeathLasers)/sizeof(const char*);

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, (char*)aszDeathLasers[GetRandom(0, cDeathLasers-1)]);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_fRemoveTime = g_pLTServer->GetTime() + 3.5f;

	m_pBody->SetState(eBodyStateFade);
}

void CBodyStateLaser::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

void CBodyStateLaser::Update()
{
	CBodyState::Update();
/*
	if ( g_pLTServer->GetTime() > m_fRemoveTime )
	{
		if ( (g_pGameServerShell->GetGameType() == SINGLE) && !!m_pBody->GetAttachments() )
		{
			m_pBody->GetAttachments()->DropAttachments();
		}

		m_pBody->RemoveObject();
	}
*/
}

void CBodyStateLaser::Save(HMESSAGEWRITE hWrite)
{
	CBodyState::Save(hWrite);

	SAVE_FLOAT(m_fRemoveTime);
}

void CBodyStateLaser::Load(HMESSAGEREAD hRead)
{
	CBodyState::Load(hRead);

	LOAD_FLOAT(m_fRemoveTime);
}

// CBodyStateDecay

void CBodyStateDecay::Constructor()
{
	CBodyState::Constructor();
}

void CBodyStateDecay::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateDecay::Init(Body* pBody)
{
	CBodyState::Init(pBody);
/*
	ObjectCreateStruct createstruct;
	createstruct.Clear();

	char szFileName[256], szSkinName[256];
    g_pLTServer->GetModelFilenames(pBody->m_hObject, szFileName, ARRAY_LEN(szFileName), szSkinName, ARRAY_LEN(szSkinName));

	SAFE_STRCPY(createstruct.m_Filename, szFileName);
	SAFE_STRCPY(createstruct.m_SkinNames[0], "SFX\\Debris\\Skins\\Feather.dtx");
	SAFE_STRCPY(createstruct.m_SkinNames[1], "SFX\\Debris\\Skins\\Feather.dtx");

    g_pLTServer->Common()->SetObjectFilenames(pBody->m_hObject, &createstruct);
*/
	m_pBody->SetState(eBodyStateFade);
/*
	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DDecay");

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_fRemoveTime = g_pLTServer->GetTime() + 3.5f;

	HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
	g_pLTServer->WriteToMessageByte(hMessage, SFX_BODY_ID);
	g_pLTServer->WriteToMessageObject(hMessage, m_pBody->m_hObject);
	g_pLTServer->WriteToMessageByte(hMessage, BFX_FADE_MSG);
	g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);
*/
}

void CBodyStateDecay::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

void CBodyStateDecay::Update()
{
	CBodyState::Update();
/*

	if ( g_pLTServer->GetTime() > m_fRemoveTime )
	{
		m_pBody->RemoveObject();
	}
*/
}

void CBodyStateDecay::Save(HMESSAGEWRITE hWrite)
{
	CBodyState::Save(hWrite);

	SAVE_FLOAT(m_fRemoveTime);
}

void CBodyStateDecay::Load(HMESSAGEREAD hRead)
{
	CBodyState::Load(hRead);

	LOAD_FLOAT(m_fRemoveTime);
}

// CBodyStateFade

void CBodyStateFade::Constructor()
{
	CBodyState::Constructor();

	m_fRemoveTime = -1.0f;
}

void CBodyStateFade::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateFade::Init(Body* pBody)
{
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
			m_pBody->ReleasePowerups();

			HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(LTNULL, MID_SFX_MESSAGE);
			g_pLTServer->WriteToMessageByte(hMessage, SFX_BODY_ID);
			g_pLTServer->WriteToMessageObject(hMessage, m_pBody->m_hObject);
			g_pLTServer->WriteToMessageByte(hMessage, BFX_FADE_MSG);
			g_pLTServer->EndMessage2(hMessage, MESSAGE_NAGGLEFAST);

			m_fRemoveTime = g_pLTServer->GetTime() + 3.5f;
		}

		if ( g_pLTServer->GetTime() > m_fRemoveTime )
		{
			m_pBody->RemoveObject();
		}
	}
}

void CBodyStateFade::Save(HMESSAGEWRITE hWrite)
{
	CBodyState::Save(hWrite);

	SAVE_FLOAT(m_fRemoveTime);
}

void CBodyStateFade::Load(HMESSAGEREAD hRead)
{
	CBodyState::Load(hRead);

	LOAD_FLOAT(m_fRemoveTime);
}

// CBodyStateCrush

void CBodyStateCrush::Constructor()
{
	CBodyState::Constructor();
}

void CBodyStateCrush::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateCrush::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	static char* aszCrushDeaths[] = { "DCrush" };
    static int cCrushDeaths = sizeof(aszCrushDeaths)/sizeof(char*);
	char* szDeath = aszCrushDeaths[GetRandom(0, cCrushDeaths-1)];

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, szDeath);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_pBody->SetState(eBodyStateNormal);
}

void CBodyStateCrush::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

// CBodyStateChair

void CBodyStateChair::Constructor()
{
	CBodyState::Constructor();
}

void CBodyStateChair::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateChair::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	static char* aszChairDeaths[] = { "DSit2" /*, "DSitFall"*/ };
    static int cChairDeaths = sizeof(aszChairDeaths)/sizeof(char*);
	char* szDeath = aszChairDeaths[GetRandom(0, cChairDeaths-1)];

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, szDeath);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_pBody->SetState(eBodyStateNormal);
}

void CBodyStateChair::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

// CBodyStatePoison

void CBodyStatePoison::Constructor()
{
	CBodyState::Constructor();
}

void CBodyStatePoison::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStatePoison::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	static char* aszPoisonDeaths[] = { "DPoison" };
    static int cPoisonDeaths = sizeof(aszPoisonDeaths)/sizeof(char*);
	char* szDeath = aszPoisonDeaths[GetRandom(0, cPoisonDeaths-1)];

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, szDeath);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_pBody->SetState(eBodyStateNormal);
}

void CBodyStatePoison::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

// CBodyStateAcid

void CBodyStateAcid::Constructor()
{
	CBodyState::Constructor();
}

void CBodyStateAcid::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateAcid::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	static char* aszAcidDeaths[] = { "DAcid" };
    static int cAcidDeaths = sizeof(aszAcidDeaths)/sizeof(char*);
	char* szDeath = aszAcidDeaths[GetRandom(0, cAcidDeaths-1)];

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, szDeath);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	m_pBody->SetState(eBodyStateNormal);
}

void CBodyStateAcid::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

// CBodyStateArrow

void CBodyStateArrow::Constructor()
{
	CBodyState::Constructor();
}

void CBodyStateArrow::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateArrow::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	static char* aszArrowDeaths[] = { "DArrow" };
    static int cArrowDeaths = sizeof(aszArrowDeaths)/sizeof(char*);
	char* szDeath = aszArrowDeaths[GetRandom(0, cArrowDeaths-1)];

	HMODELANIM hAni = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, szDeath);

	g_pLTServer->SetModelAnimation(m_pBody->m_hObject, hAni);
	g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

	LTVector vPos;
	g_pLTServer->GetObjectPos(pBody->m_hObject, &vPos);

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(pBody->m_hObject, &rRot);

	LTVector vNull, vForward;
	g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	VEC_COPY(IQuery.m_From, vPos);
	VEC_COPY(IQuery.m_To, vPos - vForward*g_BodyStickDist.GetFloat());
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	IQuery.m_FilterFn = GroundFilterFn;
	IQuery.m_PolyFilterFn = LTNULL;

	if (!g_pLTServer->IntersectSegment(&IQuery, &IInfo) || !IsMainWorld(IInfo.m_hObject))
	{
		pBody->SetState(eBodyStateNormal);
	}
	else
	{
		//g_pLTServer->MoveObject(pBody->m_hObject, &IInfo.m_Point);
		LTVector vDims, vPos;
		g_pLTServer->GetObjectDims(pBody->m_hObject, &vDims);
		vPos = IInfo.m_Point;
		vPos += (vForward * vDims.x);
		g_pLTServer->TeleportObject(pBody->m_hObject, &vPos);
	}

	pBody->SetState(eBodyStateNormal);
}

void CBodyStateArrow::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);
}

// CBodyStateExplode

void CBodyStateExplode::Constructor()
{
	CBodyState::Constructor();

	m_bLand = LTFALSE;
}

void CBodyStateExplode::Destructor()
{
	CBodyState::Destructor();
}

void CBodyStateExplode::Init(Body* pBody)
{
	CBodyState::Init(pBody);

	LTVector vDir = pBody->GetDeathDir();
	if ( vDir.y < 0.0 )
	{
		vDir.y = 0.01f;
	}

	LTRotation rRot; LTVector vNull, vForward;
	g_pLTServer->GetObjectRotation(m_pBody->m_hObject, &rRot);
	g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);

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

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_pBody->m_hObject);
	dwFlags |= FLAG_GRAVITY;
	dwFlags &= ~FLAG_GOTHRUWORLD;
    g_pLTServer->SetObjectFlags(m_pBody->m_hObject, dwFlags);

	// Give us some velocity

	static CVarTrack cvBodyExplodeVelocity;
	if ( !cvBodyExplodeVelocity.IsInitted() )
	{
        cvBodyExplodeVelocity.Init(g_pLTServer, "BodyExplodeVelocity", LTNULL, 200.0f);
	}

//	g_pLTServer->CPrint("vel = %f", cvBodyExplodeVelocity.GetFloat());
    LTVector vTemp(vDir*cvBodyExplodeVelocity.GetFloat());
    g_pLTServer->SetVelocity(m_pBody->m_hObject, &vTemp);
}

void CBodyStateExplode::InitLoad(Body* pBody)
{
	CBodyState::InitLoad(pBody);

	m_hAniStart = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXFStart");
	m_hAniLoop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXFLoop");
	m_hAniStop = g_pLTServer->GetAnimIndex(m_pBody->m_hObject, "DXFStop");
}

LTBOOL FlyingFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return LTFALSE;

	uint32 dwFlags;
	dwFlags = g_pLTServer->GetObjectFlags(hObj);

	return (dwFlags & FLAG_SOLID);
}

void CBodyStateExplode::HandleTouch(HOBJECT hObject)
{
	LTVector vVelocity;
	g_pLTServer->GetVelocity(m_pBody->m_hObject, &vVelocity);

	if (vVelocity.Mag() < 5.0f || vVelocity.y < 5.0f )
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_pBody->m_hObject, &vPos);

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		LTVector vDims;
		g_pLTServer->GetObjectDims(m_pBody->m_hObject, &vDims);

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

	if ( m_pBody && ((g_pLTServer->GetTime() - m_pBody->GetStarttime()) > g_BodyStateTimeout.GetFloat()) )
	{
		g_pLTServer->SetModelAnimation(m_pBody->m_hObject, m_hAniStop);
		g_pLTServer->SetModelLooping(m_pBody->m_hObject, LTFALSE);

		m_pBody->SetState(eBodyStateNormal);

		return;
	}

	if ( m_bLand )
	{
		g_pLTServer->TeleportObject(m_pBody->m_hObject, &m_vLandPos);
		m_pBody->SetState(eBodyStateNormal);
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

void CBodyStateExplode::Save(HMESSAGEWRITE hWrite)
{
	CBodyState::Save(hWrite);

	SAVE_BOOL(m_bLand);
	SAVE_VECTOR(m_vLandPos);
}

void CBodyStateExplode::Load(HMESSAGEREAD hRead)
{
	CBodyState::Load(hRead);

	LOAD_BOOL(m_bLand);
	LOAD_VECTOR(m_vLandPos);
}