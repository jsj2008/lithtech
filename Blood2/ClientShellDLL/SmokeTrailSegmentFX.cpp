// ----------------------------------------------------------------------- //
//
// MODULE  : SmokeTrailSegmentFX.cpp
//
// PURPOSE : SmokeTrail segment special FX - Implementation
//
// CREATED : 3/1/98
//
// ----------------------------------------------------------------------- //

#include "SmokeTrailSegmentFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ContainerCodes.h"

DVector g_vWorldWindVel;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeTrailSegmentFX::Init
//
//	PURPOSE:	Init the smoke trail segment
//
// ----------------------------------------------------------------------- //

DBOOL CSmokeTrailSegmentFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseParticleSystemFX::Init(psfxCreateStruct)) return DFALSE;

	STSCREATESTRUCT* pSTS = (STSCREATESTRUCT*)psfxCreateStruct;
	VEC_COPY(m_vVel, pSTS->vVel);
	VEC_COPY(m_vColor1, pSTS->vColor1);
	VEC_COPY(m_vColor2, pSTS->vColor2);
	m_bSmall		= pSTS->bSmall;
	m_fLifeTime		= pSTS->fLifeTime;
	m_fFadeTime		= pSTS->fFadeTime;
	m_fOffsetTime	= pSTS->fOffsetTime;
	m_fRadius		= pSTS->fRadius;
	m_fGravity		= pSTS->fGravity;
	m_nNumPerPuff	= pSTS->nNumPerPuff;

	m_bIgnoreWind	= DFALSE;

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

DBOOL CSmokeTrailSegmentFX::CreateObject(CClientDE *pClientDE)
{
	if(!pClientDE || !m_hServerObject) return DFALSE;

	m_pTextureName	= "SpriteTextures\\smoke32_2.dtx";

	// Determine if we are in a liquid...
	DVector vPos;
	pClientDE->GetObjectPos(m_hServerObject, &vPos);

	HLOCALOBJ objList[1];
	DDWORD dwNum = pClientDE->GetPointContainers(&vPos, objList, 1);

	if(dwNum > 0 && objList[0])
	{
		D_WORD dwCode;
		if(pClientDE->GetContainerCode(objList[0], &dwCode))
		{
			if(IsLiquid((ContainerCode)dwCode))
			{
				m_pTextureName = "SpriteTextures\\ParticleTextures\\particlebubble.dtx";
				m_fRadius *= 0.25f;
				m_fGravity = 5.0f;
				m_nNumPerPuff *= 3;
				m_bIgnoreWind = DTRUE;
			}
		}
	}

	int ret = CBaseParticleSystemFX::CreateObject(pClientDE);

	if(ret)
	{
		DFLOAT r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, 0.5f);
	}

	return ret;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSmokeTrailSegmentFX::Update
//
//	PURPOSE:	Update the smoke trail (add smoke)
//
// ----------------------------------------------------------------------- //

DBOOL CSmokeTrailSegmentFX::Update()
{
	if (!m_hObject || !m_pClientDE) return DFALSE;

	DFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
		if (!m_hServerObject) return DFALSE;

		m_bFirstUpdate = DFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;

		// Where is the server (moving) object...
		DVector vPos, vTemp;
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
		
		// Current position is relative to the particle system's postion (i.e., 
		// each puff of smoke is some distance away from the particle system's 
		/// position)...
		m_pClientDE->GetObjectPos(m_hObject, &vTemp);
		VEC_SUB(vPos, vPos, vTemp);

		VEC_COPY(m_vLastPos, vPos);
	}


	// Check to see if we should just wait for last smoke puff to go away...
	if (m_bWantRemove || (fTime > m_fStartTime + m_fFadeTime))
	{
		if (fTime > m_fLastTime + m_fLifeTime)
		{
			return DFALSE;
		}

		DFLOAT fScale = (m_fLifeTime - (fTime - m_fLastTime)) / m_fLifeTime;

		// Adjust the alpha
		DFLOAT r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, fScale);

		return DTRUE;
	}

	// See if it is time to create a new smoke puff...
	if ((fTime > m_fLastTime + m_fOffsetTime) && m_hServerObject)
	{
		DVector vCurPos, vPos, vDelta, vTemp, vDriftVel, vColor;

		// Calculate smoke puff position...

		// Where is the server (moving) object...
		m_pClientDE->GetObjectPos(m_hServerObject, &vCurPos);
		
		// Current position is relative to the particle system's postion (i.e., 
		// each puff of smoke is some distance away from the particle system's 
		/// position)...
		m_pClientDE->GetObjectPos(m_hObject, &vTemp);
		VEC_SUB(vCurPos, vCurPos, vTemp);

		// How long has it been since the last smoke puff?
		DFLOAT fTimeOffset = fTime - m_fLastTime;

		// What is the range of colors?
		DFLOAT fRange = m_vColor2.x - m_vColor1.x;

		// Fill the distance between the last projectile position, and it's 
		// current position with smoke puffs...
		int nNumSteps = (m_fLastTime > 0) ? 8 : 1;

		VEC_SUB(vTemp, vCurPos, m_vLastPos);
		VEC_MULSCALAR(vDelta, vTemp, 1.0f/float(nNumSteps));

		VEC_COPY(vPos, m_vLastPos);

		DFLOAT fCurLifeTime    = 10.0f; //  m_fLifeTime - fTimeOffset;
		DFLOAT fLifeTimeOffset = fTimeOffset / float(nNumSteps);

		DFLOAT fOffset = 0.5f;
		DVector vDriftOffset;
		VEC_SET(vDriftOffset, 4.0f, 5.5f, 0.5f);

		if (m_bSmall)
		{
			VEC_SET(vDriftOffset, 5.0f, 50.0f, 5.0f);
		}
		else
		{
			VEC_SET(vDriftOffset, 10.0f, 50.0f, 10.0f);
		}

		for (int i=0; i < nNumSteps; i++)
		{
			// Build the individual smoke puffs...

			for (int j=0; j < m_nNumPerPuff; j++)
			{
				VEC_COPY(vTemp, vPos);

				if (m_bIgnoreWind)
				{
					VEC_SET(vDriftVel, GetRandom(-vDriftOffset.x, vDriftOffset.x), 
									   GetRandom(5.0f, 6.0f), 
									   GetRandom(-vDriftOffset.z, vDriftOffset.z));
				}
				else
				{
					VEC_SET(vDriftVel, g_vWorldWindVel.x + GetRandom(-vDriftOffset.x, vDriftOffset.x), 
									   g_vWorldWindVel.y + GetRandom(5.0f, 6.0f), 
									   g_vWorldWindVel.z + GetRandom(-vDriftOffset.z, vDriftOffset.z));
				}

				vTemp.x += GetRandom(-fOffset, fOffset);
				vTemp.y += GetRandom(-fOffset, fOffset);
				vTemp.z += GetRandom(-fOffset, fOffset);

				DFLOAT fOffset  = GetRandom(m_vColor1.x, m_vColor2.x);
				DFLOAT fPercent = 1.0f;

				if (fRange > 0.01)
				{
					fPercent = fOffset / fRange;
				}

				vColor.x = m_vColor1.x + fOffset;

				fOffset = fPercent * (m_vColor2.y - m_vColor1.y);
				vColor.y = m_vColor1.y + fOffset;

				fOffset = fPercent * (m_vColor2.z - m_vColor1.z);
				vColor.z = m_vColor1.z + fOffset;

				m_pClientDE->AddParticle(m_hObject, &vTemp, &vDriftVel, &vColor, fCurLifeTime);

			}
			VEC_ADD(vPos, vPos, vDelta);
			fCurLifeTime += fLifeTimeOffset;
		}

		m_fLastTime = fTime;

		VEC_COPY(m_vLastPos, vCurPos);
	}

	return DTRUE;
}
