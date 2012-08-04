// ----------------------------------------------------------------------- //
//
// MODULE  : DebrisFX.cpp
//
// PURPOSE : Debris - Implementation
//
// CREATED : 5/31/98
//
// ----------------------------------------------------------------------- //

#include "DebrisFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"
#include "WeaponFXTypes.h"
#include "SurfaceTypes.h"
#include "RiotClientShell.h"
#include "ltobjectcreate.h"

extern CRiotClientShell* g_pRiotClientShell;

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

	DEBRISCREATESTRUCT* pDebris = (DEBRISCREATESTRUCT*)psfxCreateStruct;
	m_rRot = pDebris->rRot;
	VEC_COPY(m_vPos, pDebris->vPos);
	VEC_COPY(m_vMinVel, pDebris->vMinVel);
	VEC_COPY(m_vMaxVel, pDebris->vMaxVel);
	m_fLifeTime			= pDebris->fLifeTime;
	m_fFadeTime			= pDebris->fFadeTime;
	m_nNumDebris		= (pDebris->nNumDebris > MAX_DEBRIS ? MAX_DEBRIS : pDebris->nNumDebris);
	m_nDebrisFlags		= pDebris->nDebrisFlags;
	m_bRotate			= pDebris->bRotate;
	m_eDebrisType		= (DebrisType)pDebris->nDebrisType;
	m_fMinScale			= pDebris->fMinScale;
	m_fMaxScale			= pDebris->fMaxScale;
	m_bPlayBounceSound	= pDebris->bPlayBounceSound;
	m_bPlayExplodeSound = pDebris->bPlayExplodeSound;
	m_bForceRemove		= pDebris->bForceRemove;


	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return LTNULL;

	uint8 nDetailLevel = pSettings->SpecialFXSetting();
	if (nDetailLevel == RS_LOW)
	{
		m_nNumDebris = int(LTFLOAT(m_nNumDebris) * 0.333f);
		m_fLifeTime	 *= 0.333f;
	}
	else if (nDetailLevel == RS_MED)
	{
		m_nNumDebris = int(LTFLOAT(m_nNumDebris) * 0.666f);
		m_fLifeTime	 *= 0.666f;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::CreateObject
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
	VEC_SET(vVelMin, 1.0f, 1.0f, 1.0f);
	VEC_SET(vVelMax, 1.0f, 1.0f, 1.0f);

	m_pClientDE->Common()->GetRotationVectors(m_rRot, vU, vR, vF);

	if (vF.y <= -0.95f || vF.y >= 0.95f)
	{
		vF.y = vF.y > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, -1.0f * vF.y);
	}
	else if (vF.x <= -0.95f || vF.x >= 0.95f)
	{
		vF.x = vF.x > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 0.0f, 0.0f, -1.0f * vF.x);
		VEC_SET(vU, 0.0f, 1.0f, 0.0f);
	}
	else if (vF.z <= -0.95f || vF.z >= 0.95f)
	{
		vF.z = vF.z > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, vF.z, 0.0f, 0.0f);
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


	// Initialize our emmitters...

	LTFLOAT fVal	 = MATH_CIRCLE/2.0f;
	LTFLOAT fVal2 = MATH_CIRCLE;
	LTVector vVel;
	for (int i=0; i < m_nNumDebris; i++)
	{
		if (m_bRotate)
		{
			m_fPitchVel[i] = GetRandom(-fVal, fVal);
			m_fYawVel[i]   = GetRandom(-fVal2, fVal2);
			m_fRollVel[i]  = GetRandom(-fVal2, fVal2);
		}

		m_hDebris[i]	 = CreateDebris();
		m_fDebrisLife[i] = GetRandom(m_fLifeTime, m_fLifeTime * 2.0f);

		m_ActiveEmmitters[i] = LTTRUE;
		m_BounceCount[i]	 = GetRandom(2, 6);

		VEC_SET(vVel, GetRandom(vVelMin.x, vVelMax.x), 
					  GetRandom(50.0f, 150.0f) + GetRandom(vVelMin.y, vVelMax.y), 
					  GetRandom(vVelMin.z, vVelMax.z));

		InitMovingObject(&(m_Emmitters[i]), &m_vPos, &vVel);
		m_Emmitters[i].m_PhysicsFlags |= m_nDebrisFlags;
	}


	// Play the explode sound...

	if (m_bPlayExplodeSound)
	{
		char* pSound = GetDebrisExplodeSound(m_eDebrisType);
		if (pSound)
		{
			PlaySoundFromPos(&m_vPos, pSound, 1000.0f, SOUNDPRIORITY_MISC_LOW);
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::Update
//
//	PURPOSE:	Update the Particle trail (add Particle)
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

	if (fTime > m_fStartTime + m_fFadeTime)
	{
		for (int i=0; i < m_nNumDebris; i++)
		{
			LTFLOAT fEndTime = m_fStartTime + m_fDebrisLife[i];
			if (fTime > fEndTime)
			{
				if (OkToRemoveDebris(m_hDebris[i]))
				{
					if (m_hDebris[i])
					{
						m_pClientDE->RemoveObject(m_hDebris[i]);
						m_hDebris[i] = LTNULL;
					}
				}
			}
		}

		// See if all the debris have been removed or not...

		int i;
		for (i=0; i < m_nNumDebris; i++)
		{
			if (m_hDebris[i]) break;
		}

		// All debris have been removed so remove us...

		if (i == m_nNumDebris)
		{
			return LTFALSE;
		}

//#define FADING_DEBRIS
#ifdef FADING_DEBRIS
		LTFLOAT fScale = (fEndTime - fTime) / (m_fLifeTime - m_fFadeTime);

		LTFLOAT r, g, b, a;
		
		for (int i=0; i < m_nNumDebris; i++)
		{
			if (m_hDebris[i])
			{
				m_pClientDE->GetObjectColor(m_hDebris[i], &r, &g, &b, &a);
				m_pClientDE->SetObjectColor(m_hDebris[i], r, g, b, fScale);
			}
		}
#endif
	}


	// Loop over our list of emmitters, updating the position of each

	for (int i=0; i < m_nNumDebris; i++)
	{
		if (m_ActiveEmmitters[i])
		{
			LTBOOL bBounced = LTFALSE;
			if (bBounced = UpdateEmmitter(&m_Emmitters[i]))
			{
				if (!(m_Emmitters[i].m_PhysicsFlags & MO_LIQUID) && (m_hDebris[i]))
				{
					if (m_bPlayBounceSound && GetRandom(1, 4) == 1)
					{
						char* pSound = GetDebrisBounceSound(m_eDebrisType);
					
						// Play appropriate sound...
					
						if (pSound)
						{
							PlaySoundFromPos(&m_Emmitters[i].m_Pos, pSound, 1000.0f,
											 SOUNDPRIORITY_MISC_LOW);
						}
					}
				}

				m_BounceCount[i]--;

				if (m_BounceCount[i] <= 0)
				{
					m_Emmitters[i].m_PhysicsFlags |= MO_RESTING;
				}
			}

			if (m_Emmitters[i].m_PhysicsFlags & MO_RESTING)
			{
				m_ActiveEmmitters[i] = LTFALSE;

				if (m_bRotate && m_hDebris[i])
				{
					LTRotation rRot;
					m_pClientDE->Common()->SetupEuler(rRot, 0.0f, m_fYaw[i], 0.0f);
					m_pClientDE->SetObjectRotation(m_hDebris[i], &rRot);	
				}
			}
			else if (m_hDebris[i])
			{
				m_pClientDE->SetObjectPos(m_hDebris[i], &(m_Emmitters[i].m_Pos));

				if (m_bRotate)
				{
					if (bBounced)
					{
						// Adjust due to the bounce...

						LTFLOAT fVal    = MATH_CIRCLE/2.0f;
						LTFLOAT fVal2   = MATH_CIRCLE;
						m_fPitchVel[i] = GetRandom(-fVal, fVal);
						m_fYawVel[i]   = GetRandom(-fVal2, fVal2);
						m_fRollVel[i]  = GetRandom(-fVal2, fVal2);
					}

					if (m_fPitchVel[i] != 0 || m_fYawVel[i] != 0 || m_fRollVel[i] != 0)
					{
						LTFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

						m_fPitch[i] += m_fPitchVel[i] * fDeltaTime;
						m_fYaw[i]   += m_fYawVel[i] * fDeltaTime;
						m_fRoll[i]  += m_fRollVel[i] * fDeltaTime;

						LTRotation rRot;
						m_pClientDE->Common()->SetupEuler(rRot, m_fPitch[i], m_fYaw[i], m_fRoll[i]);
						m_pClientDE->SetObjectRotation(m_hDebris[i], &rRot);	
					}
				}
			}
		}
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::UpdateEmmitter
//
//	PURPOSE:	Update emmitter position
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::UpdateEmmitter(MovingObject* pObject)
{	
	if (!m_pClientDE || !pObject || pObject->m_PhysicsFlags & MO_RESTING) return LTFALSE;

	LTBOOL bRet = LTFALSE;

	LTVector vNewPos;
	if (UpdateMovingObject(LTNULL, pObject, &vNewPos))
	{
		ClientIntersectInfo info;
		bRet = BounceMovingObject(LTNULL, pObject, &vNewPos, &info);

		VEC_COPY(pObject->m_LastPos, pObject->m_Pos);
		VEC_COPY(pObject->m_Pos, vNewPos);

		if (m_pClientDE->Common()->GetPointStatus(&vNewPos) == LT_OUTSIDE)
		{
			pObject->m_PhysicsFlags |= MO_RESTING;
			VEC_COPY(pObject->m_Pos, pObject->m_LastPos);
		}
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::CreateDebris
//
//	PURPOSE:	Create a debris model
//
// ----------------------------------------------------------------------- //

HLOCALOBJ CDebrisFX::CreateDebris()
{
	LTVector vScale;
	VEC_SET(vScale, 1.0f, 1.0f, 1.0f);
	VEC_MULSCALAR(vScale, vScale, GetRandom(0.8f, 1.2f));

	char* pFilename = GetDebrisModel(m_eDebrisType, vScale);
	char* pSkin     = GetDebrisSkin(m_eDebrisType);

	if (!pFilename) return LTNULL;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	SAFE_STRCPY(createStruct.m_Filename, pFilename);
	SAFE_STRCPY(createStruct.m_SkinName, pSkin);
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT; 
	VEC_COPY(createStruct.m_Pos, m_vPos);

	HLOCALOBJ hObj = m_pClientDE->CreateObject(&createStruct);

	LTFLOAT fVal = (m_eDebrisType == DBT_CAR_PARTS) ? m_fMaxScale : GetRandom(m_fMinScale, m_fMaxScale);

	VEC_MULSCALAR(vScale, vScale, fVal);
	m_pClientDE->SetObjectScale(hObj, &vScale);

	// Adjust the alpha on glass debris...

	if (m_eDebrisType == DBT_GLASS_BIG || m_eDebrisType == DBT_GLASS_SMALL)
	{
		m_pClientDE->SetObjectColor(hObj, 1.0f, 1.0f, 1.0f, GetRandom(0.1f, 0.3f));
	}

	return hObj;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDebrisFX::OkToRemoveDebris
//
//	PURPOSE:	See if this particular model can be removed.
//
// ----------------------------------------------------------------------- //

LTBOOL CDebrisFX::OkToRemoveDebris(HLOCALOBJ hDebris)
{
	if (!m_pClientDE || !g_pRiotClientShell || !hDebris || m_bForceRemove) return LTTRUE;


	// The only constraint is that the client isn't currently looking
	// at the model...

	HLOCALOBJ hCamera = g_pRiotClientShell->GetCamera();
	if (!hCamera) return LTTRUE;

	LTVector vPos, vCamPos;
	m_pClientDE->GetObjectPos(hDebris, &vPos);
	m_pClientDE->GetObjectPos(hCamera, &vCamPos);


	// Determine if the client can see us...

	LTVector vDir;
	VEC_SUB(vDir, vPos, vCamPos);

	LTRotation rRot;
	LTVector vU, vR, vF;
	m_pClientDE->GetObjectRotation(hCamera, &rRot);
	m_pClientDE->Common()->GetRotationVectors(rRot, vU, vR, vF);

	VEC_NORM(vDir);
	VEC_NORM(vF);
	LTFLOAT fMul = VEC_DOT(vDir, vF);
	if (fMul <= 0.0f) return LTTRUE;


	// Client is looking our way, don't remove it yet...

	return LTFALSE;
}