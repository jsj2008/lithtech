// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleExplosionFX.cpp
//
// PURPOSE : Particle Explosion - Implementation
//
// CREATED : 5/22/98
//
// ----------------------------------------------------------------------- //

#include "ParticleExplosionFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"
#include "ClientServerShared.h"
#include "shareddefs.h"
#include "sfxmgr.h"
#include "bloodclientshell.h"
#include "bloodsplatfx.h"
#include "SoundTypes.h"

extern DVector g_vWorldWindVel;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::Init
//
//	PURPOSE:	Init the Particle trail segment
//
// ----------------------------------------------------------------------- //

DBOOL CParticleExplosionFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	PESCREATESTRUCT* pPE = (PESCREATESTRUCT*)psfxCreateStruct;
	ROT_COPY(m_rSurfaceRot, pPE->rSurfaceRot);
	VEC_COPY(m_vPos, pPE->vPos);
	VEC_COPY(m_vColor1, pPE->vColor1);
	VEC_COPY(m_vColor2, pPE->vColor2);
	VEC_COPY(m_vMinVel, pPE->vMinVel);
	VEC_COPY(m_vMaxVel, pPE->vMaxVel);
	VEC_COPY(m_vMinDriftOffset, pPE->vMinDriftOffset);
	VEC_COPY(m_vMaxDriftOffset, pPE->vMaxDriftOffset);
	m_bSmall			= pPE->bSmall;
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
	m_bBounce			= pPE->bBounce;

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

	if (m_bSmall)
	{
		m_fRadius /= 2.0f;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CParticleExplosionFX::CreateObject(CClientDE *pClientDE)
{
	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	// Initialize the Emitters velocity ranges based on our rotation...

	DVector vVelMin, vVelMax;
	VEC_COPY(vVelMin, m_vMinVel);
	VEC_COPY(vVelMax, m_vMaxVel);

	// Initialize our Emitters...

	DVector vStartVel;
	for (int i=0; i < m_nNumEmitters; i++)
	{
		if (m_bCreateDebris) 
		{
			m_hDebris[i] = CreateDebris();
		}

		m_ActiveEmitters[i] = DTRUE;
		m_BounceCount[i] = 2;

		VEC_SET(vStartVel, GetRandom(vVelMin.x, vVelMax.x), 
						   GetRandom(vVelMin.y, vVelMax.y), 
						   GetRandom(vVelMin.z, vVelMax.z));

		InitMovingObject(&(m_Emitters[i]), &m_vPos, &vStartVel);
		m_Emitters[i].m_PhysicsFlags |= m_nEmitterFlags;
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

DBOOL CParticleExplosionFX::Update()
{
	if (!m_hObject || !m_pClientDE) return DFALSE;

	if (!CBaseParticleSystemFX::Update()) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
		m_bFirstUpdate = DFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;
	}


	// Check to see if we should start fading the system...

	if (fTime > m_fStartTime + m_fFadeTime)
	{
		DFLOAT fEndTime = m_fStartTime + m_fLifeTime;
		if (fTime > fEndTime)
		{
			return DFALSE;
		}

		DFLOAT fScale = (fEndTime - fTime) / (m_fLifeTime - m_fFadeTime);

		DFLOAT r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);
		
		//for (int i=0; i < m_nNumEmitters; i++)
		//{
		//	if (m_hDebris[i])
		//	{
		//		m_pClientDE->GetObjectColor(m_hDebris[i], &r, &g, &b, &a);
		//		m_pClientDE->SetObjectColor(m_hDebris[i], r, g, b, fScale);
		//	}
		//}
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
			DBOOL bBounced = DFALSE;

			ClientIntersectInfo info;

			if (bBounced = UpdateEmmitter(&m_Emitters[i], &info))
			{
				if(m_nSurfaceType == SURFTYPE_FLESH)
				{
					CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
					if (!pShell) return DTRUE;
					CSFXMgr* psfxMgr = pShell->GetSFXMgr();
					if (!psfxMgr) return DTRUE;

					char szTemp[256];

					sprintf(szTemp, "sounds\\gibs\\flesh\\gib_impact%d.wav", GetRandom(0,NRES(5)));

					PlaySoundFromPos(&m_Emitters[i].m_Pos, szTemp, 300.0f,SOUNDPRIORITY_MISC_LOW);

					BSCREATESTRUCT splat;
					char* pSplatSprite = DNULL;

					switch(GetRandom(1,3))
					{
						case 1:		pSplatSprite = "sprites\\blood1.spr";	break;
						case 2:		pSplatSprite = "sprites\\blood2.spr";	break;
						case 3:		pSplatSprite = "sprites\\blood3.spr";	break;
						default:	pSplatSprite = "sprites\\blood1.spr";	break;
					}

					VEC_COPY(splat.m_Pos, m_Emitters[i].m_Pos);
					m_pClientDE->AlignRotation(&splat.m_Rotation, &info.m_Plane.m_Normal,&info.m_Plane.m_Normal);
					splat.m_fScale = 0.05f + GetRandom(-0.05f,0.05f);
					splat.m_hstrSprite = m_pClientDE->CreateString(pSplatSprite);
					
					if(info.m_Plane.m_Normal.y > 0.5f)
						splat.m_fGrowScale = 0.01f;

					psfxMgr->CreateSFX(SFX_BLOODSPLAT_ID, &splat, DFALSE, this);
					g_pClientDE->FreeString( splat.m_hstrSprite );
				}

				m_BounceCount[i]--;

				if (m_BounceCount[i] <= 0)
				{
					m_Emitters[i].m_PhysicsFlags |= MO_RESTING;
				}
			}

			if (m_Emitters[i].m_PhysicsFlags & MO_RESTING)
			{
				m_ActiveEmitters[i] = DFALSE;
				if (m_hDebris[i])
				{
					m_pClientDE->DeleteObject(m_hDebris[i]);
					m_hDebris[i] = DNULL;
				}
			}
			else if (m_hDebris[i])
			{
				m_pClientDE->SetObjectPos(m_hDebris[i], &(m_Emitters[i].m_Pos));

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
						DFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

						m_fPitch += m_fPitchVel * fDeltaTime;
						m_fYaw   += m_fYawVel * fDeltaTime;

						DRotation rRot;
						m_pClientDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
						m_pClientDE->SetObjectRotation(m_hDebris[i], &rRot);	
					}
				}
			}
		}
	}

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleExplosionFX::UpdateEmmitter
//
//	PURPOSE:	Update emmitter position
//
// ----------------------------------------------------------------------- //

DBOOL CParticleExplosionFX::UpdateEmmitter(MovingObject* pObject, ClientIntersectInfo* pInfo)
{	
	if (!m_pClientDE || !pObject || pObject->m_PhysicsFlags & MO_RESTING) return DFALSE;

	DBOOL bRet = DFALSE;

	DVector vNewPos;
	if (UpdateMovingObject(DNULL, pObject, &vNewPos))
	{
		bRet = BounceMovingObject(DNULL, pObject, &vNewPos, pInfo);

		if(!m_bBounce && bRet)
			pObject->m_PhysicsFlags |= MO_RESTING;

		VEC_COPY(pObject->m_LastPos, pObject->m_Pos);
		VEC_COPY(pObject->m_Pos, vNewPos);

		if (m_pClientDE->GetPointStatus(&vNewPos) == DE_OUTSIDE)
		{
			pObject->m_PhysicsFlags |= MO_RESTING;
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
	if (!m_hObject || !m_pClientDE || !pObject || pObject->m_PhysicsFlags & MO_RESTING) return;

	DFLOAT fTime = m_pClientDE->GetTime();

	DVector vCurPos, vLastPos, vPos, vDelta, vTemp, vDriftVel, vColor;

	VEC_COPY(vCurPos, pObject->m_Pos);
	VEC_COPY(vLastPos, pObject->m_LastPos);

	// Calculate Particle puff positions...

	// Current position is relative to the particle system's postion (i.e., 
	// each puff of Particle is some distance away from the particle system's 
	// position)...

	VEC_SUB(vCurPos, vCurPos, m_vPos);
	VEC_SUB(vLastPos, vLastPos, m_vPos);


	// How long has it been since the last Particle puff?

	DFLOAT fTimeOffset = fTime - m_fLastTime;

	
	// Fill the distance between the last projectile position, and it's 
	// current position with Particle puffs...

	VEC_SUB(vTemp, vCurPos, vLastPos);
	VEC_MULSCALAR(vDelta, vTemp, 1.0f/float(m_nNumSteps));

	VEC_COPY(vPos, vLastPos);

	DFLOAT fCurLifeTime    = 10.0f;
	DFLOAT fLifeTimeOffset = fTimeOffset / float(m_nNumSteps);

	DFLOAT fOffset = 0.5f;

	if (m_bSmall)
	{
		fOffset /= 2.0f;
	}

	for (int i=0; i < m_nNumSteps; i++)
	{
		// Build the individual Particle puffs...

		for (int j=0; j < m_nNumPerPuff; j++)
		{
			VEC_COPY(vTemp, vPos);

			VEC_SET(vDriftVel, GetRandom(m_vMinDriftOffset.x, m_vMaxDriftOffset.x), 
							   GetRandom(m_vMinDriftOffset.y, m_vMaxDriftOffset.y), 
							   GetRandom(m_vMinDriftOffset.z, m_vMaxDriftOffset.z));

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
	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	char* debrisFilename[] =
	{
		"Models\\Props\\debris\\generic1.abc",
		"Models\\Props\\debris\\generic2.abc",
		"Models\\Props\\debris\\generic3.abc",
		"Models\\Props\\debris\\generic4.abc",
		"Models\\Props\\debris\\generic5.abc",
		"Models\\Props\\debris\\generic6.abc",
		"Models\\Props\\debris\\generic7.abc",
		"Models\\Props\\debris\\generic7.abc"
	};

	createStruct.m_ObjectType = OT_MODEL;
	_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)"models\\gibs\\flesh\\gib1.abc");
	_mbscpy((unsigned char*)createStruct.m_SkinName, (const unsigned char*)"skins\\gibs\\flesh\\gib1.abc");
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT; 
	VEC_COPY(createStruct.m_Pos, m_vPos);

	HLOCALOBJ hObj = m_pClientDE->CreateObject(&createStruct);

//	DFLOAT fVal = m_fRadius / 400.0f;

//	DVector vScale;
//	VEC_SET(vScale, fVal, fVal, fVal);
//	m_pClientDE->SetObjectScale(hObj, &vScale);

	//DFLOAT r, g, b, a;
	//m_pClientDE->GetObjectColor(hObj, &r, &g, &b, &a);
	//m_pClientDE->SetObjectColor(hObj, 1.0f, 0.4f, 0.0f, a);

	return hObj;
}