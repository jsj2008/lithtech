// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleExplosionFX.cpp
//
// PURPOSE : Particle Explosion - Implementation
//
// CREATED : 5/22/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ParticleExplosionFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"
#include "WeaponFXTypes.h"
#include "DebrisMgr.h"
#include "SoundMgr.h"
#include "GameClientShell.h"

extern LTVector g_vWorldWindVel;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::Init
//
//	PURPOSE:	Init the Particle trail segment
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleExplosionFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return LTFALSE;

	PESCREATESTRUCT* pPE = (PESCREATESTRUCT*)psfxCreateStruct;
    m_rSurfaceRot = pPE->rSurfaceRot;
	m_vPos				= pPE->vPos;
	m_vColor1			= pPE->vColor1;
	m_vColor2			= pPE->vColor2;
	m_vMinVel			= pPE->vMinVel;
	m_vMaxVel			= pPE->vMaxVel;
	m_vMinDriftVel		= pPE->vMinDriftVel;
	m_vMaxDriftVel		= pPE->vMaxDriftVel;
	m_fLifeTime			= pPE->fLifeTime;
	m_fFadeTime			= pPE->fFadeTime;
	m_fOffsetTime		= pPE->fOffsetTime;
	m_fRadius			= pPE->fRadius;
	m_fGravity			= pPE->fGravity;
	m_nNumPerPuff		= pPE->nNumPerPuff;
	m_nNumEmitters		= (pPE->nNumEmitters > MAX_EMITTERS ? MAX_EMITTERS : pPE->nNumEmitters);
	m_nEmitterFlags		= pPE->nEmitterFlags;
	m_pTextureName		= pPE->pFilename;
	m_bCreateDebris		= pPE->bCreateDebris;
	m_bRotateDebris		= pPE->bRotateDebris;
	m_nSurfaceType		= pPE->nSurfaceType;
	m_bIgnoreWind		= pPE->bIgnoreWind;
	m_nNumSteps			= pPE->nNumSteps;

	VEC_SET(m_vColorRange, m_vColor2.x - m_vColor1.x,
						   m_vColor2.y - m_vColor1.y,
						   m_vColor2.z - m_vColor1.z);

	if (m_vColorRange.x < 0.0f) m_vColorRange.x = 0.0f;
	if (m_vColorRange.y < 0.0f) m_vColorRange.y = 0.0f;
	if (m_vColorRange.z < 0.0f) m_vColorRange.z = 0.0f;


	if (m_bRotateDebris)
	{
		m_fPitchVel = GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
		m_fYawVel	= GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleExplosionFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	// Initialize the Emitters velocity ranges based on our rotation...

    LTVector vVelMin, vVelMax, vTemp, vU, vR, vF;
	VEC_SET(vVelMin, 1.0f, 1.0f, 1.0f);
	VEC_SET(vVelMax, 1.0f, 1.0f, 1.0f);

	vU = m_rSurfaceRot.Up();
	vR = m_rSurfaceRot.Right();
	vF = m_rSurfaceRot.Forward();

	if (vF.y <= -0.95f || vF.y >= 0.95f)
	{
		vF.y = vF.y > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, 1.0f);
	}
	else if (vF.x <= -0.95f || vF.x >= 0.95f)
	{
		vF.x = vF.x > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 0.0f, 1.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, 1.0f);
	}
	else if (vF.z <= -0.95f || vF.z >= 0.95f)
	{
		vF.z = vF.z > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 1.0f, 0.0f);
	}

	VEC_MULSCALAR(vVelMin, vF, m_vMinVel.y);
	VEC_MULSCALAR(vVelMax, vF, m_vMaxVel.y);

	VEC_MULSCALAR(vTemp, vR, m_vMinVel.x);
	VEC_ADD(vVelMin, vVelMin, vTemp);

	VEC_MULSCALAR(vTemp, vR, m_vMaxVel.x);
	VEC_ADD(vVelMax, vVelMax, vTemp);

	VEC_MULSCALAR(vTemp, vU, m_vMinVel.z);
	VEC_ADD(vVelMin, vVelMin, vTemp);

	VEC_MULSCALAR(vTemp, vU, m_vMaxVel.z);
	VEC_ADD(vVelMax, vVelMax, vTemp);


	// Initialize our Emitters...

    LTVector vStartVel;
	for (int i=0; i < m_nNumEmitters; i++)
	{
		if (m_bCreateDebris)
		{
			m_hDebris[i] = CreateDebris();
		}

        m_ActiveEmitters[i] = LTTRUE;
		m_BounceCount[i] = 2;

		VEC_SET(vStartVel, GetRandom(vVelMin.x, vVelMax.x),
						   GetRandom(vVelMin.y, vVelMax.y),
						   GetRandom(vVelMin.z, vVelMax.z));

		InitMovingObject(&(m_Emitters[i]), m_vPos, vStartVel);
		m_Emitters[i].m_dwPhysicsFlags |= m_nEmitterFlags;
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::Update
//
//	PURPOSE:	Update the Particle trail (add Particle)
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleExplosionFX::Update()
{
    if (!m_hObject || !m_pClientDE) return LTFALSE;

    if (!CBaseParticleSystemFX::Update()) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
        m_bFirstUpdate = LTFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;
	}


	// Check to see if we should start fading the system...

	if (fTime > m_fStartTime + m_fFadeTime)
	{
        LTFLOAT fEndTime = m_fStartTime + m_fLifeTime;
		if (fTime > fEndTime)
		{
            return LTFALSE;
		}

        LTFLOAT fScale = (fEndTime - fTime) / (m_fLifeTime - m_fFadeTime);

        LTFLOAT r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);
	}


	// See if it is time to create a new Particle puff...

	if (fTime >= m_fLastTime + m_fOffsetTime)
	{
		// Loop over our list of Emitters, creating new particles...

		for (int i=0; i < m_nNumEmitters; i++)
		{
			if (m_ActiveEmitters[i])
			{
				AddParticles(&m_Emitters[i]);
			}
		}

		m_fLastTime = fTime;
	}


	// Loop over our list of Emitters, updating the position of each

	for (int i=0; i < m_nNumEmitters; i++)
	{
		if (m_ActiveEmitters[i])
		{
            LTBOOL bBounced = LTFALSE;
			if (bBounced = UpdateEmitter(&m_Emitters[i]))
			{
				if (!(m_Emitters[i].m_dwPhysicsFlags & MO_LIQUID) && (m_hDebris[i]))
				{
					/*
					char* pSound = GetDebrisBounceSound(DBT_STONE_BIG);

					// Play appropriate sound...

					g_pClientSoundMgr->PlaySoundFromPos(m_Emitters[i].m_Pos, pSound,
						300.0f, SOUNDPRIORITY_MISC_LOW);
					*/
				}

				m_BounceCount[i]--;

				if (m_BounceCount[i] <= 0)
				{
					m_Emitters[i].m_dwPhysicsFlags |= MO_RESTING;
				}
			}

			if (m_Emitters[i].m_dwPhysicsFlags & MO_RESTING)
			{
                m_ActiveEmitters[i] = LTFALSE;
				if (m_hDebris[i])
				{
					m_pClientDE->RemoveObject(m_hDebris[i]);
                    m_hDebris[i] = LTNULL;
				}
			}
			else if (m_hDebris[i])
			{
				g_pLTClient->SetObjectPos(m_hDebris[i], &(m_Emitters[i].m_vPos));

				if (m_bRotateDebris)
				{
					if (bBounced)
					{
						// Adjust due to the bounce...

						m_fPitchVel = GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
						m_fYawVel	= GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
					}

					if (m_fPitchVel != 0 || m_fYawVel != 0)
					{
                        LTFLOAT fDeltaTime = g_pGameClientShell->GetFrameTime();

						m_fPitch += m_fPitchVel * fDeltaTime;
						m_fYaw   += m_fYawVel * fDeltaTime;

                        LTRotation rRot;
						rRot.Rotate(rRot.Up(), m_fYaw);
						rRot.Rotate(rRot.Right(), m_fPitch);
						g_pLTClient->SetObjectRotation(m_hDebris[i], &rRot);
					}
				}
			}
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::UpdateEmitter
//
//	PURPOSE:	Update emitter position
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleExplosionFX::UpdateEmitter(MovingObject* pObject)
{
    if (!m_pClientDE || !pObject || pObject->m_dwPhysicsFlags & MO_RESTING) return LTFALSE;

    LTBOOL bRet = LTFALSE;

    LTVector vNewPos;
    if (UpdateMovingObject(LTNULL, pObject, vNewPos))
	{
		ClientIntersectInfo info;
		LTBOOL bBouncedOnGround = LTFALSE;
        uint32 dwFlags = (INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID);

		bRet = BounceMovingObject(LTNULL, pObject, vNewPos, &info, 
			dwFlags, false, bBouncedOnGround);

		pObject->m_vLastPos	= pObject->m_vPos;
		pObject->m_vPos		= vNewPos;

        if (g_pCommonLT->GetPointStatus(&vNewPos) == LT_OUTSIDE)
		{
			pObject->m_dwPhysicsFlags |= MO_RESTING;
			pObject->m_vPos = pObject->m_vLastPos;
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::AddParticles
//
//	PURPOSE:	Add particles to system
//
// ----------------------------------------------------------------------- //

void CParticleExplosionFX::AddParticles(MovingObject* pObject)
{
	if (!m_hObject || !m_pClientDE || !pObject || pObject->m_dwPhysicsFlags & MO_RESTING) return;

    LTFLOAT fTime = m_pClientDE->GetTime();

    LTVector vCurPos, vLastPos, vPos, vDelta, vTemp, vDriftVel, vColor;

	vCurPos		= pObject->m_vPos;
	vLastPos	= pObject->m_vLastPos;

	// Calculate Particle puff positions...

	// Current position is relative to the particle system's postion (i.e.,
	// each puff of Particle is some distance away from the particle system's
	// position)...

	VEC_SUB(vCurPos, vCurPos, m_vPos);
	VEC_SUB(vLastPos, vLastPos, m_vPos);


	// How long has it been since the last Particle puff?

    LTFLOAT fTimeOffset = fTime - m_fLastTime;


	// Fill the distance between the last projectile position, and it's
	// current position with Particle puffs...

	VEC_SUB(vTemp, vCurPos, vLastPos);
	VEC_MULSCALAR(vDelta, vTemp, 1.0f/float(m_nNumSteps));

	VEC_COPY(vPos, vLastPos);

    LTFLOAT fCurLifeTime    = 10.0f;
    LTFLOAT fLifeTimeOffset = fTimeOffset / float(m_nNumSteps);

    LTFLOAT fOffset = 0.5f;

	int nNumPerPuff = GetNumParticles(m_nNumPerPuff);

	for (int i=0; i < m_nNumSteps; i++)
	{
		// Build the individual Particle puffs...

		for (int j=0; j < nNumPerPuff; j++)
		{
			VEC_COPY(vTemp, vPos);

			VEC_SET(vDriftVel, GetRandom(m_vMinDriftVel.x, m_vMaxDriftVel.x),
							   GetRandom(m_vMinDriftVel.y, m_vMaxDriftVel.y),
							   GetRandom(m_vMinDriftVel.z, m_vMaxDriftVel.z));

			if (!m_bIgnoreWind)
			{
				vDriftVel.x += g_vWorldWindVel.x;
				vDriftVel.y += g_vWorldWindVel.y;
				vDriftVel.z += g_vWorldWindVel.z;
			}

			vTemp.x += GetRandom(-fOffset, fOffset);
			vTemp.y += GetRandom(-fOffset, fOffset);
			vTemp.z += GetRandom(-fOffset, fOffset);

			GetRandomColorInRange(vColor);

			m_pClientDE->AddParticle(m_hObject, &vTemp, &vDriftVel, &vColor, fCurLifeTime);
		}

		VEC_ADD(vPos, vPos, vDelta);
		fCurLifeTime += fLifeTimeOffset;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::CreateDebris
//
//	PURPOSE:	Create a debris model
//
// ----------------------------------------------------------------------- //

HLOCALOBJ CParticleExplosionFX::CreateDebris()
{
    return LTNULL;
/*
    LTVector vScale;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	VEC_MULSCALAR(vScale, vScale, GetRandom(1.0f, 5.0f));

	char* pFilename = GetDebrisModel(DBT_STONE_BIG, vScale);
	char* pSkin     = GetDebrisSkin(DBT_STONE_BIG);

    if (!pFilename) return LTNULL;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	SAFE_STRCPY(createStruct.m_Filename, pFilename);
	SAFE_STRCPY(createStruct.m_SkinName, pSkin);
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
	VEC_COPY(createStruct.m_Pos, m_vPos);

	HLOCALOBJ hObj = m_pClientDE->CreateObject(&createStruct);

	m_pClientDE->SetObjectScale(hObj, &vScale);

	return hObj;
*/
}