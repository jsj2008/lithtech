// ----------------------------------------------------------------------- //
//
// MODULE  : Breakable.CPP
//
// PURPOSE : A Breakable object
//
// CREATED : 1/14/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Breakable.h"
#include "Trigger.h"
#include "ClientServerShared.h"
#include "ServerUtilities.h"
#include "ObjectMsgs.h"
#include "SoundMgr.h"
#include "PlayerObj.h"
#include "Spawner.h"

BEGIN_CLASS(Breakable)
	// New properties...
	ADD_REALPROP_FLAG(BreakTime, 2.0f, 0)
	ADD_STRINGPROP_FLAG(BreakSound, "Snd\\Event\\Break.wav", PF_FILENAME)
	ADD_REALPROP_FLAG(BreakSoundRadius, 500.0f, PF_RADIUS)
	ADD_STRINGPROP_FLAG(ImpactSound, "Snd\\Event\\BreakImpact.wav", PF_FILENAME)
	ADD_REALPROP_FLAG(ImpactSoundRadius, 1000.0f, PF_RADIUS)
    ADD_BOOLPROP(TouchActivate, LTTRUE)
    ADD_BOOLPROP(DestroyOnImpact, LTFALSE)
    ADD_BOOLPROP(DestroyAfterBreak, LTFALSE)
    ADD_BOOLPROP(CrushObjects, LTFALSE)
	ADD_VECTORPROP_VAL(ScreenShakeAmount, 1.5f, 1.5f, 1.5f)
	PROP_DEFINEGROUP(Physics, PF_GROUP4)
		ADD_REALPROP_FLAG(FallVelocity, -1000.0f, PF_GROUP4)
		ADD_REALPROP_FLAG(RotationTime, 15.0f, PF_GROUP4)
        ADD_BOOLPROP_FLAG(AdjustPitch, LTTRUE, PF_GROUP4)
		ADD_REALPROP_FLAG(PitchDelta, 4.0f, PF_GROUP4)
        ADD_BOOLPROP_FLAG(AdjustYaw, LTTRUE, PF_GROUP4)
		ADD_REALPROP_FLAG(YawDelta, 2.0f, PF_GROUP4)
        ADD_BOOLPROP_FLAG(AdjustRoll, LTTRUE, PF_GROUP4)
		ADD_REALPROP_FLAG(RollDelta, 5.5f, PF_GROUP4)

	// Property overrides...
    ADD_BOOLPROP_FLAG(NeverDestroy, LTFALSE, PF_GROUP1)
	PROP_DEFINEGROUP(StateFlags, PF_GROUP4 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(ActivateTrigger, LTTRUE, PF_GROUP4 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(StartOpen, LTFALSE, PF_GROUP4 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(TriggerClose, LTTRUE, PF_GROUP4 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(RemainsOpen, LTTRUE, PF_GROUP4 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(ForceMove, LTFALSE, PF_GROUP4 | PF_HIDDEN)
	ADD_VISIBLE_FLAG(1, PF_HIDDEN)
	ADD_SOLID_FLAG(1, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(BoxPhysics, LTFALSE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(Locked, LTTRUE, PF_HIDDEN)
    ADD_BOOLPROP_FLAG(IsKeyframed, LTTRUE, PF_HIDDEN) // Moves...
	ADD_BOOLPROP_FLAG(LoopSounds, LTTRUE, PF_HIDDEN)
	ADD_REALPROP_FLAG(Speed, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(MoveDelay, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(MoveDist, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(MoveDir, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(SoundPos, 0.0f, 0.0f, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(PortalName, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(OpenSound, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(CloseSound, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(LockedSound, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(OpenedCommand, "", PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ClosedCommand, "", PF_HIDDEN)
	ADD_REALPROP_FLAG(SoundRadius, 1000.0f, PF_RADIUS | PF_HIDDEN)
	ADD_REALPROP_FLAG(OpenWaitTime, 4.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(CloseWaitTime, 0.0f, PF_HIDDEN)
	ADD_REALPROP_FLAG(ClosingSpeed, 0.0f, PF_HIDDEN)
	ADD_BOOLPROP(AITriggerable, 0)
	PROP_DEFINEGROUP(Waveform, PF_GROUP5 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(Linear, LTTRUE, PF_GROUP5 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(Sine, LTFALSE, PF_GROUP5 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(SlowOff, LTFALSE, PF_GROUP5 | PF_HIDDEN)
        ADD_BOOLPROP_FLAG(SlowOn, LTFALSE, PF_GROUP5 | PF_HIDDEN)
	ADD_STRINGPROP_FLAG(Attachments, "", PF_HIDDEN)
    ADD_BOOLPROP_FLAG(RemoveAttachments, LTTRUE, PF_HIDDEN)
	ADD_VECTORPROP_VAL_FLAG(AttachDir, 0.0f, 200.0f, 0.0f, PF_HIDDEN)
	ADD_STRINGPROP_FLAG(ShadowLights, "", PF_OBJECTLINK | PF_HIDDEN)
END_CLASS_DEFAULT(Breakable, Door, NULL, NULL)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::Breakable()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

Breakable::Breakable() : Door()
{
	m_fRotVel			 = 0.0f;
	m_fBreakTime		 = 0.0f;
	m_fBreakSoundRadius	 = 0.0f;
	m_fImpactSoundRadius = 0.0f;
    m_hstrBreakSound     = LTNULL;
    m_hstrImpactSound    = LTNULL;
    m_bFalling           = LTFALSE;
    m_bStarted           = LTFALSE;
    m_bCrushObjects      = LTFALSE;
    m_bTouchActivate     = LTTRUE;

    m_bDestroyOnImpact   = LTFALSE;
    m_bDestroyAfterBreak = LTFALSE;

    m_hBreakObj          = LTNULL;

	m_vShakeAmount.Init();
	m_vFinalPos.Init();
	m_vTotalDelta.Init();
	m_vDelta.Init();
	m_vStartingPitchYawRoll.Init();
	m_vPitchYawRoll.Init();
	m_vSign.Init(1, 1, 1);
	m_vAdjust.Init(1, 1, 1);
	m_vVel.Init(0, -1000.0f, 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::~Breakable()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //

Breakable::~Breakable()
{
	FREE_HSTRING(m_hstrBreakSound);
	FREE_HSTRING(m_hstrImpactSound);

	if (m_hBreakObj)
	{
        g_pLTServer->BreakInterObjectLink(m_hObject, m_hBreakObj);
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::EngineMessageFn()
//
//	PURPOSE:	Handler for engine messages
//
// --------------------------------------------------------------------------- //

uint32 Breakable::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch (messageID)
	{
		case MID_UPDATE:
		{
			// Door changes this, so save original value...

            LTBOOL bFirstUpdate = m_bFirstUpdate;

            uint32 dwRet = Door::EngineMessageFn(messageID, pData, fData);

			// Ignore the first update...

			if (!bFirstUpdate)
			{
				Update();
			}

			return dwRet;
		}
		break;

		case MID_PRECREATE:
		{
			// Need to call base class to have the object name read in before
			// we call PostPropRead()

            uint32 dwRet = Door::EngineMessageFn(messageID, pData, fData);

			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp((ObjectCreateStruct*)pData);
			}

			return dwRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			CacheFiles();
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save((HMESSAGEWRITE)pData);
		}
		break;

		case MID_LOADOBJECT:
		{
			Load((HMESSAGEREAD)pData);
		}
		break;

		case MID_LINKBROKEN :
		{
			HOBJECT hLink = (HOBJECT)pData;
			if (hLink)
			{
				if (hLink == m_hBreakObj)
				{
                    m_hBreakObj = LTNULL;
				}
			}
		}
		break;

		default :
		break;
	}

	return Door::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::ReadProp()
//
//	PURPOSE:	Reads breakable properties
//
// --------------------------------------------------------------------------- //

void Breakable::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric("BreakTime", &genProp) == LT_OK)
	{
		m_fBreakTime = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("BreakSoundRadius", &genProp) == LT_OK)
	{
		m_fBreakSoundRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("ImpactSoundRadius", &genProp) == LT_OK)
	{
		m_fImpactSoundRadius = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("DestroyOnImpact", &genProp) == LT_OK)
	{
		m_bDestroyOnImpact = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("DestroyAfterBreak", &genProp) == LT_OK)
	{
		m_bDestroyAfterBreak = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("CrushObjects", &genProp) == LT_OK)
	{
		m_bCrushObjects = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("TouchActivate", &genProp) == LT_OK)
	{
		m_bTouchActivate = genProp.m_Bool;
	}

    if (g_pLTServer->GetPropGeneric("BreakSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrBreakSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("ImpactSound", &genProp) == LT_OK)
	{
		if (genProp.m_String[0])
		{
            m_hstrImpactSound = g_pLTServer->CreateString(genProp.m_String);
		}
	}

    if (g_pLTServer->GetPropGeneric("ScreenShakeAmount", &genProp ) == LT_OK)
	{
		m_vShakeAmount = genProp.m_Vec;
	}

    g_pLTServer->GetPropRotationEuler("Rotation", &m_vStartingPitchYawRoll);
	m_vPitchYawRoll = m_vStartingPitchYawRoll;

    if (g_pLTServer->GetPropGeneric("AdjustPitch", &genProp) == LT_OK)
	{
		m_vAdjust.x = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("PitchDelta", &genProp) == LT_OK)
	{
		m_vTotalDelta.x = MATH_DEGREES_TO_RADIANS(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("AdjustYaw", &genProp) == LT_OK)
	{
		m_vAdjust.y = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("YawDelta", &genProp) == LT_OK)
	{
		m_vTotalDelta.y = MATH_DEGREES_TO_RADIANS(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("AdjustRoll", &genProp) == LT_OK)
	{
		m_vAdjust.z = genProp.m_Float;
	}

    if (g_pLTServer->GetPropGeneric("RollDelta", &genProp) == LT_OK)
	{
		m_vTotalDelta.z = MATH_DEGREES_TO_RADIANS(genProp.m_Float);
	}

    if (g_pLTServer->GetPropGeneric("RotationTime", &genProp) == LT_OK)
	{
		if (genProp.m_Float > 0.0f)
		{
			m_fRotVel = MATH_CIRCLE / genProp.m_Float;
		}
		else
		{
			m_fRotVel = MATH_CIRCLE;
		}
	}

    if (g_pLTServer->GetPropGeneric("FallVelocity", &genProp) == LT_OK)
	{
		m_vVel.y = genProp.m_Float;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::Update()
//
//	PURPOSE:	Update the breakable
//
// ----------------------------------------------------------------------- //

void Breakable::Update()
{
    LTFLOAT fTime = g_pLTServer->GetTime();

    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);

	// See if it is time to stop the breaking...

	if (!m_BreakTimer.Stopped())
	{
		// Update the breaking...

		UpdateBreaking();
	}
	else if (!m_bFalling)
	{
		m_bFalling = StopBreak();
	}

	if (m_bFalling)
	{
		UpdateFalling();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::StartBreak()
//
//	PURPOSE:	Start the breaking...
//
// ----------------------------------------------------------------------- //

void Breakable::StartBreak(HOBJECT hObj)
{
	if (m_bStarted) return;

    m_bStarted = LTTRUE;

	// If an object caused us to break, give him a little upward
	// velocity...

	if (hObj)
	{
		m_hBreakObj = hObj;
        g_pLTServer->CreateInterObjectLink(m_hObject, m_hBreakObj);

		if (IsPlayer(hObj) && m_vShakeAmount.Mag() > 0.0f)
		{
            CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObj);
			if (pPlayer)
			{
				HCLIENT hClient = pPlayer->GetClient();
				if (hClient)
				{
                    HMESSAGEWRITE hMessage = g_pLTServer->StartMessage(hClient, MID_SHAKE_SCREEN);
                    g_pLTServer->WriteToMessageVector(hMessage, &m_vShakeAmount);
                    g_pLTServer->EndMessage(hMessage);
				}
			}
		}
	}


	// Play the breaking sound...

	if (m_hstrBreakSound)
	{
        LTVector vPos;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);

        char* pSnd = g_pLTServer->GetStringData(m_hstrBreakSound);

		if (pSnd)
		{
			g_pServerSoundMgr->PlaySoundFromPos(vPos, pSnd, m_fBreakSoundRadius,
				SOUNDPRIORITY_MISC_MEDIUM);
		}
	}


	// Use box physics (faster)...

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags |= FLAG_BOXPHYSICS;


	// Turn off the touch notify flag if we don't do crushing damage...

	if (!m_bCrushObjects)
	{
		dwFlags &= ~FLAG_TOUCH_NOTIFY;
	}

    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	m_BreakTimer.Start(m_fBreakTime);

	m_vDelta.Init();

    g_pLTServer->SetNextUpdate(m_hObject, 0.001f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::StopBreak()
//
//	PURPOSE:	Stop the breaking...
//
// ----------------------------------------------------------------------- //

LTBOOL Breakable::StopBreak()
{
	if (m_bDestroyAfterBreak)
	{
		Destroy();
        return LTFALSE;
	}

	// Determine how far we can fall before we'll hit the ground...

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From	= vPos;
	IQuery.m_To		= vPos;
	IQuery.m_To.y	-= 10000.0f;

	IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID;

    if (g_pLTServer->IntersectSegment(&IQuery, &IInfo))
	{
		m_vFinalPos = IInfo.m_Point;
	}
	else
	{
		m_vFinalPos = IQuery.m_To;
	}

	// Turn off our solid flag so anything on top of us will
	// fall normally... (i.e., they won't land on us while they
	// are falling)...

    uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
	dwFlags &= ~FLAG_SOLID;
    g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

	// If the player caused us to break, make sure the client
	// moves (this will make gravity pull the object down)...

	if (m_hBreakObj && IsPlayer(m_hBreakObj))
	{
        CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hBreakObj);
		if (pPlayer)
		{
			pPlayer->TeleportClientToServerPos();
		}

        g_pLTServer->BreakInterObjectLink(m_hObject, m_hBreakObj);
        m_hBreakObj = LTNULL;
	}


	// Create break fx...

	CreateBreakFX();


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::UpdateBreaking()
//
//	PURPOSE:	Update the breaking...
//
// ----------------------------------------------------------------------- //

void Breakable::UpdateBreaking()
{
    LTFLOAT fDeltaTime = g_pLTServer->GetFrameTime();
    LTFLOAT fDelta = 0.0f;

	if (m_vAdjust.x)
	{
		fDelta = ((m_vSign.x > 0.0f ? m_fRotVel : -m_fRotVel) * fDeltaTime);

		m_vPitchYawRoll.x += fDelta;
		m_vDelta.x += fDelta;

		if (fabs(m_vDelta.x) >= m_vTotalDelta.x)
		{
			m_vDelta.x = (m_vSign.x * m_vTotalDelta.x);
			m_vPitchYawRoll.x = m_vStartingPitchYawRoll.x + m_vDelta.x;
			m_vSign.x *= -1.0f;
		}
	}

	if (m_vAdjust.y)
	{
		fDelta = ((m_vSign.y > 0.0f ? m_fRotVel : -m_fRotVel) * fDeltaTime);

		m_vPitchYawRoll.y += fDelta;
		m_vDelta.y		  += fDelta;

		if (fabs(m_vDelta.y) >= m_vTotalDelta.y)
		{
			m_vDelta.y = (m_vSign.y * m_vTotalDelta.y);
			m_vPitchYawRoll.y = m_vStartingPitchYawRoll.y + m_vDelta.y;
			m_vSign.y *= -1.0f;
		}
	}

	if (m_vAdjust.z)
	{
		fDelta = ((m_vSign.z > 0.0f ? m_fRotVel : -m_fRotVel) * fDeltaTime);

		m_vPitchYawRoll.z += fDelta;
		m_vDelta.z		  += fDelta;

		if (fabs(m_vDelta.z) >= m_vTotalDelta.z)
		{
			m_vDelta.z = (m_vSign.z * m_vTotalDelta.z);
			m_vPitchYawRoll.z = m_vStartingPitchYawRoll.z + m_vDelta.z;
			m_vSign.z *= -1.0f;
		}
	}


	// Shake, rattle, and roll...

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);

    g_pLTServer->SetupEuler(&rRot, m_vPitchYawRoll.x, m_vPitchYawRoll.y, m_vPitchYawRoll.z);
    g_pLTServer->SetObjectRotation(m_hObject, &rRot);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::UpdateFalling()
//
//	PURPOSE:	Update the falling state...
//
// ----------------------------------------------------------------------- //

void Breakable::UpdateFalling()
{
    LTVector vOldPos;
    g_pLTServer->GetObjectPos(m_hObject, &vOldPos);

    LTVector vPos = vOldPos;
    vPos += (m_vVel * g_pLTServer->GetFrameTime());

    LTBOOL bDone = LTFALSE;

	if (vPos.y < m_vFinalPos.y)
	{
		vPos.y = m_vFinalPos.y;
        bDone = LTTRUE;
	}

    g_pLTServer->TeleportObject(m_hObject, &vPos);

	if (vPos.Equals(m_vFinalPos, 10.0f))
	{
        bDone = LTTRUE;
	}

	if (bDone)
	{
        m_bFalling = LTFALSE;

        uint32 dwFlags = g_pLTServer->GetObjectFlags(m_hObject);
		dwFlags |= FLAG_SOLID;
        g_pLTServer->SetObjectFlags(m_hObject, dwFlags);

        g_pLTServer->SetNextUpdate(m_hObject, 0.0f);


		// Create impact fx...

		CreateImpactFX();


		if (m_bDestroyOnImpact)
		{
			Destroy();
		}
		else
		{
			// Play the impact sound...

			if (m_hstrImpactSound)
			{
                LTVector vPos;
                g_pLTServer->GetObjectPos(m_hObject, &vPos);

                char* pSnd = g_pLTServer->GetStringData(m_hstrImpactSound);

				if (pSnd)
				{
					g_pServerSoundMgr->PlaySoundFromPos(vPos, pSnd, m_fImpactSoundRadius,
						SOUNDPRIORITY_MISC_MEDIUM);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::Destroy()
//
//	PURPOSE:	Die
//
// ----------------------------------------------------------------------- //

void Breakable::Destroy()
{
	// Destroy us...

    HOBJECT hObj = LTNULL;
    LTVector vDir(0, 0, 0);

	DamageStruct damage;

	damage.eType	= DT_EXPLODE;
	damage.fDamage	= damage.kInfiniteDamage;
	damage.hDamager = hObj;
	damage.vDir		= vDir;

	damage.DoDamage(this, m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::CrushObject()
//
//	PURPOSE:	Crush the specified object
//
// ----------------------------------------------------------------------- //

void Breakable::CrushObject(HOBJECT hObj)
{
	if (!hObj) return;

    LTVector vPos;
    LTVector vHisPos;

    g_pLTServer->GetObjectPos(m_hObject, &vPos);
    g_pLTServer->GetObjectPos(m_hObject, &vHisPos);

    LTVector vDir = vPos - vHisPos;
	vDir.Norm();

	DamageStruct damage;

	damage.eType	= DT_CRUSH;
	damage.fDamage	= damage.kInfiniteDamage;
	damage.hDamager = m_hObject;
	damage.vDir		= vDir;

	damage.DoDamage(this, hObj);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::CreateBreakFX()
//
//	PURPOSE:	Create break fx
//
// ----------------------------------------------------------------------- //

void Breakable::CreateBreakFX()
{
	STEAMCREATESTRUCT sc;

	sc.fRange			= 40.0f;
	sc.fCreateDelta		= 10.0f;
	sc.fEndAlpha		= 0.0f;
	sc.fEndScale		= 2.5f;
	sc.fParticleRadius	= 13000.0f;
	sc.fStartAlpha		= 0.5f;
	sc.fStartScale		= 1.0f;
	sc.fVel				= 20.0f;
    sc.hstrParticle     = g_pLTServer->CreateString("SFX\\Impact\\Spr\\Smoke.spr");
	sc.vMinDriftVel.Init(-15, -20, -15);
	sc.vMinDriftVel.Init(15, -5, 15);
	sc.vColor1.Init(255, 255, 255);
	sc.vColor2.Init(255, 255, 255);

    LTVector vDims;
    g_pLTServer->GetObjectDims(m_hObject, &vDims);
	sc.fVolumeRadius = Max(vDims.x, vDims.y);
	sc.fVolumeRadius = Max(sc.fVolumeRadius, vDims.z);

	sc.nNumParticles = int(sc.fVolumeRadius / 10.0f);
	sc.nNumParticles = sc.nNumParticles < 1 ? 1 : sc.nNumParticles;

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);

    LTVector vDown(0, -1, 0);
    g_pLTServer->AlignRotation(&rRot, &vDown, LTNULL);

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	Steam* pSteam = (Steam*)SpawnObject("Steam", vPos, rRot);
	if (pSteam)
	{
        pSteam->Setup(&sc, 3.0f, LTTRUE);
	}

	// Clear this so we don't try and delete it twice...

    sc.hstrParticle = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::CreateImpactFX()
//
//	PURPOSE:	Create impact fx
//
// ----------------------------------------------------------------------- //

void Breakable::CreateImpactFX()
{
	STEAMCREATESTRUCT sc;

	sc.fRange			= 75.0f;
	sc.fCreateDelta		= 10.0f;
	sc.fEndAlpha		= 0.0f;
	sc.fEndScale		= 3.0f;
	sc.fParticleRadius	= 20000.0f;
	sc.fStartAlpha		= 0.5f;
	sc.fStartScale		= 1.0f;
	sc.fVel				= 25.0f;
    sc.hstrParticle     = g_pLTServer->CreateString("SFX\\Impact\\Spr\\Smoke.spr");
	sc.vMinDriftVel.Init(-35, 5, -35);
	sc.vMinDriftVel.Init(35, 50, 35);
	sc.vColor1.Init(255, 255, 255);
	sc.vColor2.Init(255, 255, 255);

    LTVector vDims;
    g_pLTServer->GetObjectDims(m_hObject, &vDims);
	sc.fVolumeRadius = Max(vDims.x, vDims.y);
	sc.fVolumeRadius = Max(sc.fVolumeRadius, vDims.z);

	sc.nNumParticles = int(sc.fVolumeRadius / 10.0f);
	sc.nNumParticles = sc.nNumParticles < 1 ? 1 : sc.nNumParticles;

    LTRotation rRot;
    g_pLTServer->GetObjectRotation(m_hObject, &rRot);

    LTVector vDown(0, 1, 0);
    g_pLTServer->AlignRotation(&rRot, &vDown, LTNULL);

    LTVector vPos;
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	Steam* pSteam = (Steam*)SpawnObject("Steam", vPos, rRot);
	if (pSteam)
	{
        pSteam->Setup(&sc, 3.0f, LTTRUE);
	}

	// Clear this so we don't try and delete it twice...

    sc.hstrParticle = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::TouchNotify
//
//	PURPOSE:	Handle object touch notify
//
// ----------------------------------------------------------------------- //

void Breakable::TouchNotify(HOBJECT hObj)
{
	if (!hObj || (!m_bTouchActivate && !m_bCrushObjects)) return;

	// Determine if he can actually break us (i.e., is he standing on us, or
	// is he moving fast enough to break through)...

    LTBOOL bBreak = IsStandingOnMe(hObj);


	// Start the break...

	if (!m_bStarted && bBreak)
	{
		StartBreak(hObj);
	}
	else if (m_bFalling && m_bCrushObjects)
	{
		CrushObject(hObj);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::IsStandingOnMe
//
//	PURPOSE:	Determine if the object is standing on me...
//
// ----------------------------------------------------------------------- //

LTBOOL Breakable::IsStandingOnMe(HOBJECT hObj)
{
    LTVector vHisPos, vPos;
    g_pLTServer->GetObjectPos(hObj, &vHisPos);
    g_pLTServer->GetObjectPos(m_hObject, &vPos);

	// We're touching, he's above me, so...yes...

	return (vHisPos.y > vPos.y);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::TriggerMsg()
//
//	PURPOSE:	Handler for trigger messages.
//
// --------------------------------------------------------------------------- //

void Breakable::TriggerMsg(HOBJECT hSender, const char* szMsg)
{
	if (_stricmp(szMsg, "Break") == 0)
	{
		StartBreak(hSender);
		return;
	}

	Door::TriggerMsg(hSender, szMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::CacheFiles
//
//	PURPOSE:	Cache resources used by this object
//
// ----------------------------------------------------------------------- //

void Breakable::CacheFiles()
{
    char* pFile = LTNULL;
	if (m_hstrBreakSound)
	{
        pFile = g_pLTServer->GetStringData(m_hstrBreakSound);
		if (pFile)
		{
            g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}

	if (m_hstrImpactSound)
	{
        pFile = g_pLTServer->GetStringData(m_hstrImpactSound);
		if (pFile)
		{
            g_pLTServer->CacheFile(FT_SOUND, pFile);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void Breakable::Save(HMESSAGEWRITE hWrite)
{
	if (!hWrite) return;

	SAVE_HOBJECT(m_hBreakObj);
	SAVE_BOOL(m_bStarted);
	SAVE_BOOL(m_bFalling);
	SAVE_BOOL(m_bDestroyOnImpact);
	SAVE_BOOL(m_bDestroyAfterBreak);
	SAVE_BOOL(m_bCrushObjects);
	SAVE_BOOL(m_bTouchActivate);
	SAVE_FLOAT(m_fBreakTime);
	SAVE_FLOAT(m_fBreakSoundRadius);
	SAVE_FLOAT(m_fImpactSoundRadius);
	SAVE_FLOAT(m_fRotVel);
	SAVE_HSTRING(m_hstrBreakSound);
	SAVE_HSTRING(m_hstrImpactSound);
	SAVE_VECTOR(m_vStartingPitchYawRoll);
	SAVE_VECTOR(m_vPitchYawRoll);
	SAVE_VECTOR(m_vTotalDelta);
	SAVE_VECTOR(m_vDelta);
	SAVE_VECTOR(m_vSign);
	SAVE_VECTOR(m_vFinalPos);
	SAVE_VECTOR(m_vShakeAmount);
	SAVE_VECTOR(m_vAdjust);
	SAVE_VECTOR(m_vVel);

	m_BreakTimer.Save(hWrite);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	Breakable::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void Breakable::Load(HMESSAGEREAD hRead)
{
	if (!hRead) return;

	LOAD_HOBJECT(m_hBreakObj);
	LOAD_BOOL(m_bStarted);
	LOAD_BOOL(m_bFalling);
	LOAD_BOOL(m_bDestroyOnImpact);
	LOAD_BOOL(m_bDestroyAfterBreak);
	LOAD_BOOL(m_bCrushObjects);
	LOAD_BOOL(m_bTouchActivate);
	LOAD_FLOAT(m_fBreakTime);
	LOAD_FLOAT(m_fBreakSoundRadius);
	LOAD_FLOAT(m_fImpactSoundRadius);
	LOAD_FLOAT(m_fRotVel);
	LOAD_HSTRING(m_hstrBreakSound);
	LOAD_HSTRING(m_hstrImpactSound);
	LOAD_VECTOR(m_vStartingPitchYawRoll);
	LOAD_VECTOR(m_vPitchYawRoll);
	LOAD_VECTOR(m_vTotalDelta);
	LOAD_VECTOR(m_vDelta);
	LOAD_VECTOR(m_vSign);
	LOAD_VECTOR(m_vFinalPos);
	LOAD_VECTOR(m_vShakeAmount);
	LOAD_VECTOR(m_vAdjust);
	LOAD_VECTOR(m_vVel);

	m_BreakTimer.Load(hRead);
}