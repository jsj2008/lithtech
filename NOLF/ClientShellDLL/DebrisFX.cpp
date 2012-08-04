// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisFX.cpp
//
// PURPOSE : Debris - Implementation
//
// CREATED : 5/31/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DebrisFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"
#include "WeaponFXTypes.h"
#include "GameClientShell.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_vtDebrisBounce;

CDebrisFX::CDebrisFX() : 
	m_bFirstUpdate(LTTRUE), 
	m_fLastTime(-1.0f), 
	m_fStartTime(-1.0f)
{
	memset(m_DebrisList, 0, sizeof(m_DebrisList));
}

CDebrisFX::~CDebrisFX()
{
	ClearDebrisList();
}

CDebrisFX::DebrisTracker::DebrisTracker() : 
	m_bActiveEmitter(LTFALSE), 
	m_BounceCount(0), 
	m_hDebris(0),
	m_fDebrisLife(0.0f),
	m_fPitch(0.0f),
	m_fYaw(0.0f),
	m_fRoll(0.0f),
	m_fPitchVel(0.0f),
	m_fYawVel(0.0f),
	m_fRollVel(0.0f)
{
}

CDebrisFX::DebrisTracker::~DebrisTracker()
{
	if (m_hDebris)
	{
		ASSERT(!"Undeleted debris handle!  DebrisTracker can't delete its handles!");
	}
}

void CDebrisFX::ClearDebrisList()
{
	for (int i=0; i < MAX_DEBRIS; i++)
	{
		if (m_DebrisList[i])
		{
			RemoveDebris(i);
		}
	}
}

CBankedList<CDebrisFX::DebrisTracker> *CDebrisFX::GetDebrisBank()
{
	static CBankedList<DebrisTracker> theBank;

	return &theBank;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::Init
//
//	PURPOSE:	Init the Particle trail segment
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	DEBRISCREATESTRUCT* pDCS = (DEBRISCREATESTRUCT*)psfxCreateStruct;

	m_ds = *(pDCS);

	// If a valid debris id was specified, use it to determine the debris
	// values...

	DEBRIS* pDebris = g_pDebrisMgr->GetDebris(m_ds.nDebrisId);
	if (pDebris)
	{
		m_ds.bRotate		= pDebris->bRotate;
		m_ds.fMinLifeTime	= pDebris->fMinLifetime;
		m_ds.fMaxLifeTime	= pDebris->fMaxLifetime;
		m_ds.fFadeTime		= pDebris->fFadetime;
		m_ds.fMinScale		= pDebris->fMinScale;
		m_ds.fMaxScale		= pDebris->fMaxScale;
		m_ds.nNumDebris		= pDebris->nNumber;
		m_ds.vMaxVel		= pDebris->vMaxVel;
		m_ds.vMinVel		= pDebris->vMinVel;
		m_ds.vMinDOffset	= pDebris->vMinDOffset;
		m_ds.vMaxDOffset	= pDebris->vMaxDOffset;
		m_ds.nMaxBounce		= pDebris->nMaxBounce;
		m_ds.nMinBounce		= pDebris->nMinBounce;
		m_ds.fGravityScale	= pDebris->fGravityScale;
		m_ds.fAlpha			= pDebris->fAlpha;
	}

	m_ds.nNumDebris = LTMIN(m_ds.nNumDebris, MAX_DEBRIS);

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
    if (!pSettings) return LTFALSE;

    uint8 nDebrisLevel = GetConsoleInt("DebrisFXLevel", RS_HIGH);
	if (nDebrisLevel == RS_LOW)
	{
		m_ds.nNumDebris = int(float(m_ds.nNumDebris) * 0.333f);
		m_ds.fMinLifeTime *= 0.333f;
		m_ds.fMaxLifeTime *= 0.333f;
	}
	else if (nDebrisLevel == RS_MED)
	{
		m_ds.nNumDebris = int(float(m_ds.nNumDebris) * 0.666f);
		m_ds.fMinLifeTime *= 0.666f;
		m_ds.fMaxLifeTime *= 0.666f;
	}

	// Always make at least one piece ;)

	m_ds.nNumDebris = LTMAX(m_ds.nNumDebris, 1);

	// Allocate that many debris trackers
	ClearDebrisList();
	for (uint32 nCreateLoop = 0; nCreateLoop < m_ds.nNumDebris; ++nCreateLoop)
	{
		m_DebrisList[nCreateLoop] = GetDebrisBank()->New();
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	// Initialize the debris velocity ranges based on our rotation...

    LTVector vVelMin, vVelMax, vTemp, vU, vR, vF;
	vVelMin.Init(1.0f, 1.0f, 1.0f);
	vVelMax.Init(1.0f, 1.0f, 1.0f);

	m_pClientDE->GetRotationVectors(&m_ds.rRot, &vU, &vR, &vF);

	if (vF.y <= -0.95f || vF.y >= 0.95f)
	{
		vF.y = vF.y > 0.0f ? 1.0f : -1.0f;
		vR.Init(1.0f, 0.0f, 0.0f);
		vU.Init(0.0f, 0.0f, -1.0f * vF.y);
	}
	else if (vF.x <= -0.95f || vF.x >= 0.95f)
	{
		vF.x = vF.x > 0.0f ? 1.0f : -1.0f;
		vR.Init(0.0f, 0.0f, -1.0f * vF.x);
		vU.Init(0.0f, 1.0f, 0.0f);
	}
	else if (vF.z <= -0.95f || vF.z >= 0.95f)
	{
		vF.z = vF.z > 0.0f ? 1.0f : -1.0f;
		vR.Init(vF.z, 0.0f, 0.0f);
		vU.Init(0.0f, 1.0f, 0.0f);
	}

	vVelMin = vF * m_ds.vMinVel.y;
	vVelMax = vF * m_ds.vMaxVel.y;
	vVelMin += vR * m_ds.vMinVel.x;
	vVelMax += vR * m_ds.vMaxVel.x;
	vVelMin += vU * m_ds.vMinVel.z;
	vVelMax += vU * m_ds.vMaxVel.z;


	// Initialize our emitters...

    LTFLOAT fVal  = MATH_CIRCLE/2.0f;
    LTFLOAT fVal2 = MATH_CIRCLE;
    LTVector vVel;
	for (int i=0; i < m_ds.nNumDebris; i++)
	{
		// Get the tracker for this slot
		DebrisTracker *pTracker = m_DebrisList[i];
		if (!pTracker)
			continue;

		if (m_ds.bRotate)
		{
			pTracker->m_fPitchVel = GetRandom(-fVal, fVal);
			pTracker->m_fYawVel   = GetRandom(-fVal2, fVal2);
			pTracker->m_fRollVel  = GetRandom(-fVal2, fVal2);
		}

		pTracker->m_fDebrisLife = GetRandom(m_ds.fMinLifeTime, m_ds.fMaxLifeTime);

        pTracker->m_bActiveEmitter = LTTRUE;
		pTracker->m_BounceCount	= GetRandom(m_ds.nMinBounce, m_ds.nMaxBounce);

		vVel.Init(GetRandom(vVelMin.x, vVelMax.x),
				  GetRandom(vVelMin.y, vVelMax.y),
				  GetRandom(vVelMin.z, vVelMax.z));

		// Add a random offset to each debris item...

        LTVector vPos;
		if (m_ds.bDirOffsetOnly)
		{
			// Only offset in the direction the debris is traveling...

			vPos = vVel;
			vPos.Norm();

			// Only use y (forward) offset...

			vPos *= GetRandom(m_ds.vMinDOffset.y, m_ds.vMaxDOffset.y);
		}
		else
		{
			vPos.Init(GetRandom(m_ds.vMinDOffset.x, m_ds.vMaxDOffset.x),
					  GetRandom(m_ds.vMinDOffset.y, m_ds.vMaxDOffset.y),
					  GetRandom(m_ds.vMinDOffset.z, m_ds.vMaxDOffset.z));
		}

		vPos += m_ds.vPos;
		InitMovingObject(&(pTracker->m_Emitter), &vPos, &vVel);

		pTracker->m_Emitter.m_fGravityScale = m_ds.fGravityScale;

		CreateDebris(i, vPos);
	}


	// Play the explode sound...

	if (m_ds.bPlayExplodeSound)
	{
		char* pSound = g_pDebrisMgr->GetExplodeSound(m_ds.nDebrisId);
		if (pSound)
		{
			g_pClientSoundMgr->PlaySoundFromPos(m_ds.vPos, pSound, 1000.0f, SOUNDPRIORITY_MISC_LOW);
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::Update
//
//	PURPOSE:	Update the debris
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::Update()
{
    if (!m_pClientDE) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
        m_bFirstUpdate = LTFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;
	}


	// Check to see if we should start fading the debris...

    int i;
    for (i=0; i < m_ds.nNumDebris; i++)
	{
		// Get the tracker for this slot
		DebrisTracker *pTracker = m_DebrisList[i];
		if (!pTracker)
			continue;

		LTFLOAT fFadeStartTime = m_fStartTime + pTracker->m_fDebrisLife;
        LTFLOAT fFadeEndTime = fFadeStartTime + m_ds.fFadeTime;
		if (fTime > fFadeEndTime)
		{
			if (OkToRemoveDebris(i))
			{
				RemoveDebris(i);
			}
		}
		else if (fTime > fFadeStartTime && m_ds.fFadeTime > 0.1f)
		{
			LTFLOAT fScale = ((fFadeEndTime - fTime) / m_ds.fFadeTime);
			LTFLOAT r, g, b, a;
	
			if (pTracker->m_hDebris)
			{
				m_pClientDE->GetObjectColor(pTracker->m_hDebris, &r, &g, &b, &a);
				a = a < fScale ? a : fScale;
				m_pClientDE->SetObjectColor(pTracker->m_hDebris, r, g, b, a);
			}
		}
	}
	
	// See if all the debris have been removed or not...

	for (i=0; i < m_ds.nNumDebris; i++)
	{
		if (IsValidDebris(i)) break;
	}

	// All debris have been removed so remove us...

	if (i == m_ds.nNumDebris)
	{
		return LTFALSE;
	}
	

	// Loop over our list of emitters, updating the position of each

	for (i=0; i < m_ds.nNumDebris; i++)
	{
		// Get the tracker for this slot
		DebrisTracker *pTracker = m_DebrisList[i];
		if (!pTracker)
			continue;

		if (pTracker->m_bActiveEmitter)
		{
            LTBOOL bRemove = LTFALSE;
            LTBOOL bBounced = LTFALSE;
			LTBOOL bBouncedOnGround = LTFALSE;
			if (bBounced = UpdateEmitter(&(pTracker->m_Emitter), bRemove, bBouncedOnGround))
			{
				if (bBouncedOnGround)
				{
					pTracker->m_BounceCount--;
				}

				if (!(pTracker->m_Emitter.m_dwPhysicsFlags & MO_LIQUID) && (IsValidDebris(i)))
				{
					if (pTracker->m_BounceCount > 0)
					{
						if (m_ds.bPlayBounceSound && GetRandom(1, 4) == 1)
						{
							char* pSound = g_pDebrisMgr->GetBounceSound(m_ds.nDebrisId);

							// Play appropriate sound...

							if (pSound)
							{
								g_pClientSoundMgr->PlaySoundFromPos(pTracker->m_Emitter.m_vPos,
									pSound, 1000.0f, SOUNDPRIORITY_MISC_LOW);
							}
						}
					}
				}

				if (pTracker->m_BounceCount <= 0)
				{
					pTracker->m_Emitter.m_dwPhysicsFlags |= MO_RESTING;
				}
			}

			// Remove the debris if necessary...

			if (bRemove)
			{
				RemoveDebris(i);
                return LTTRUE;
			}

			if (pTracker->m_Emitter.m_dwPhysicsFlags & MO_RESTING)
			{
                pTracker->m_bActiveEmitter = LTFALSE;

				if (m_ds.bRotate && IsValidDebris(i))
				{
					RotateDebrisToRest(i);
				}
			}
			else if (IsValidDebris(i))
			{
				SetDebrisPos(i, pTracker->m_Emitter.m_vPos);

				if (m_ds.bRotate)
				{
					if (bBounced)
					{
						// Adjust due to the bounce...

                        LTFLOAT fVal    = MATH_CIRCLE/2.0f;
                        LTFLOAT fVal2   = MATH_CIRCLE;
						pTracker->m_fPitchVel = GetRandom(-fVal, fVal);
						pTracker->m_fYawVel   = GetRandom(-fVal2, fVal2);
						pTracker->m_fRollVel  = GetRandom(-fVal2, fVal2);
					}

					if (pTracker->m_fPitchVel != 0 || pTracker->m_fYawVel != 0 || pTracker->m_fRollVel != 0)
					{
                        LTFLOAT fDeltaTime = g_pGameClientShell->GetFrameTime();

						pTracker->m_fPitch += pTracker->m_fPitchVel * fDeltaTime;
						pTracker->m_fYaw   += pTracker->m_fYawVel * fDeltaTime;
						pTracker->m_fRoll  += pTracker->m_fRollVel * fDeltaTime;

                        LTRotation rRot;
						m_pClientDE->SetupEuler(&rRot, pTracker->m_fPitch, pTracker->m_fYaw, pTracker->m_fRoll);

						SetDebrisRot(i, rRot);
					}
				}
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::UpdateEmitter
//
//	PURPOSE:	Update Emitter position
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::UpdateEmitter(MovingObject* pObject, LTBOOL & bRemove,
								LTBOOL & bBounceOnGround)
{
    if (!m_pClientDE || !pObject || pObject->m_dwPhysicsFlags & MO_RESTING) return LTFALSE;

    bRemove = LTFALSE;

    LTBOOL bRet = LTFALSE;
    LTVector vNewPos;
    if (UpdateMovingObject(LTNULL, pObject, &vNewPos))
	{
		ClientIntersectInfo info;

		if (m_ds.bBounce)
		{
			uint dwFlags = (INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID);
			bRet = BounceMovingObject(LTNULL, pObject, &vNewPos, &info, 
				dwFlags, bBounceOnGround);
		}
		else
		{
			bRet = LTFALSE;
		}

		// If we hit the sky/invisible surface we're done...

		SurfaceType eType = GetSurfaceType(info);
		if (eType == ST_SKY || eType == ST_INVISIBLE)
		{
            bRemove = LTTRUE;
            return LTFALSE;
		}

		pObject->m_vLastPos = pObject->m_vPos;
		pObject->m_vPos = vNewPos;

        if (m_pClientDE->GetPointStatus(&vNewPos) == LT_OUTSIDE)
		{
			pObject->m_dwPhysicsFlags |= MO_RESTING;
			pObject->m_vPos = pObject->m_vLastPos;
		}
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::CreateDebris
//
//	PURPOSE:	Create the specified debris object
//
// ----------------------------------------------------------------------- //

void CDebrisFX::CreateDebris(int i, LTVector vPos)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;

	// Get the tracker for this slot
	DebrisTracker *pTracker = m_DebrisList[i];
	if (!pTracker)
		return;

	pTracker->m_hDebris = g_pDebrisMgr->CreateDebris(m_ds.nDebrisId, vPos);
	if (!pTracker->m_hDebris) return;

    LTVector vScale(1.0f, 1.0f, 1.0f);
	vScale *= GetRandom(0.8f, 1.2f);
	vScale *= GetRandom(m_ds.fMinScale, m_ds.fMaxScale);
	m_pClientDE->SetObjectScale(pTracker->m_hDebris, &vScale);

    LTFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(pTracker->m_hDebris, &r, &g, &b, &a);
	m_pClientDE->SetObjectColor(pTracker->m_hDebris, r, g, b, m_ds.fAlpha);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::OkToRemoveDebris
//
//	PURPOSE:	See if this particular model can be removed.
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::OkToRemoveDebris(int i)
{
 	// Debris sometimes doesn't get removed (and can keep piling up)
	// with the following code, so we'll just fade instead ;)

	return LTTRUE;

	if (i < 0 || i >= m_ds.nNumDebris) return LTTRUE;

    if (!m_pClientDE || !g_pGameClientShell || !IsValidDebris(i) || m_ds.bForceRemove) return LTTRUE;

	// The only constraint is that the client isn't currently looking
	// at the model...

	HOBJECT hCamera = g_pGameClientShell->GetCamera();
    if (!hCamera) return LTTRUE;

    LTVector vPos, vCamPos;
    if (!GetDebrisPos(i, vPos)) return LTTRUE;

	m_pClientDE->GetObjectPos(hCamera, &vCamPos);


	// Determine if the client can see us...

    LTVector vDir;
	VEC_SUB(vDir, vPos, vCamPos);

    LTRotation rRot;
    LTVector vU, vR, vF;
	m_pClientDE->GetObjectRotation(hCamera, &rRot);
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	vDir.Norm();
	vF.Norm();

    if (VEC_DOT(vDir, vF) <= 0.0f) return LTTRUE;


	// Client is looking our way, don't remove it yet...

    return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::RemoveDebris
//
//	PURPOSE:	Remove the specified debris object
//
// ----------------------------------------------------------------------- //

void CDebrisFX::RemoveDebris(int i)
{
	if (i < 0 || i >= MAX_DEBRIS) return;

	// Get the tracker for this slot
	DebrisTracker *pTracker = m_DebrisList[i];
	if (!pTracker)
		return;

	// Delete the handle if we can/need to
	if (pTracker->m_hDebris && m_pClientDE)
	{
		m_pClientDE->DeleteObject(pTracker->m_hDebris);
        pTracker->m_hDebris = LTNULL;
	}

	// Remove it from the list
	GetDebrisBank()->Delete(pTracker);
	m_DebrisList[i] = LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::IsValidDebris
//
//	PURPOSE:	Is the debris valid
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::IsValidDebris(int i)
{
    if (i < 0 || i >= m_ds.nNumDebris) return LTFALSE;

	// Get the tracker for this slot
	DebrisTracker *pTracker = m_DebrisList[i];
	if (!pTracker)
		return LTFALSE;

	return pTracker->m_hDebris != LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::RotateDebrisToRest
//
//	PURPOSE:	Rotate the debris to the rest position
//
// ----------------------------------------------------------------------- //

void CDebrisFX::RotateDebrisToRest(int i)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;

	// Get the tracker for this slot
	DebrisTracker *pTracker = m_DebrisList[i];
	if (!pTracker)
		return;

    LTRotation rRot;
	m_pClientDE->SetupEuler(&rRot, 0.0f, pTracker->m_fYaw, 0.0f);
	m_pClientDE->SetObjectRotation(pTracker->m_hDebris, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::SetDebrisPos
//
//	PURPOSE:	Set the debris position
//
// ----------------------------------------------------------------------- //

void CDebrisFX::SetDebrisPos(int i, LTVector vPos)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;

	// Get the tracker for this slot
	DebrisTracker *pTracker = m_DebrisList[i];
	if (!pTracker)
		return;

	m_pClientDE->SetObjectPos(pTracker->m_hDebris, &vPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::GetDebrisPos
//
//	PURPOSE:	Get the debris position
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::GetDebrisPos(int i, LTVector & vPos)
{
    if (i < 0 || i >= m_ds.nNumDebris) return LTFALSE;

	// Get the tracker for this slot
	DebrisTracker *pTracker = m_DebrisList[i];
	if (!pTracker)
		return LTFALSE;

	m_pClientDE->GetObjectPos(pTracker->m_hDebris, &vPos);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::SetDebrisRot
//
//	PURPOSE:	Set the debris rotation
//
// ----------------------------------------------------------------------- //

void CDebrisFX::SetDebrisRot(int i, LTRotation rRot)
{
	if (i < 0 || i >= m_ds.nNumDebris) return;

	// Get the tracker for this slot
	DebrisTracker *pTracker = m_DebrisList[i];
	if (!pTracker)
		return;

	m_pClientDE->SetObjectRotation(pTracker->m_hDebris, &rRot);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::GetDebrisPos
//
//	PURPOSE:	Get the lifetime of the debris
//
// ----------------------------------------------------------------------- //

LTFLOAT	CDebrisFX::GetDebrisLife(int i)
{
	if (i < 0 || i >= m_ds.nNumDebris) return 0.0f;

	// Get the tracker for this slot
	DebrisTracker *pTracker = m_DebrisList[i];
	if (!pTracker)
		return 0.0f;

	return pTracker->m_fDebrisLife;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::GetEmitter
//
//	PURPOSE:	Get the emitter of the debris
//
// ----------------------------------------------------------------------- //

MovingObject *CDebrisFX::GetEmitter(int i)
{
	if (i < 0 || i >= m_ds.nNumDebris) return LTNULL;

	// Get the tracker for this slot
	DebrisTracker *pTracker = m_DebrisList[i];
	if (!pTracker)
		return LTNULL;

	return &(pTracker->m_Emitter);
}

