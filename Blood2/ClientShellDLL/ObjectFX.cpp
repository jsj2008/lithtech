// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectFX.cpp
//
// PURPOSE : General object special FX - Implementation
//
// CREATED : 8/25/98
//
// ----------------------------------------------------------------------- //

#include "ObjectFX.h"
#include "SFXMsgIds.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "BloodClientShell.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectParticleFX::Init
//
//	PURPOSE:	Init the system
//
// ----------------------------------------------------------------------- //

DBOOL CObjectParticleFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CBaseParticleSystemFX::Init(psfxCreateStruct);

	OBJECTPARTICLEFXCS* pPS = (OBJECTPARTICLEFXCS*)psfxCreateStruct;

	m_hObj				= pPS->hObj;
	VEC_COPY(m_vOffset, pPS->vOffset);
	m_fRampUpTime		= pPS->fRampUpTime;
	m_fRampDownTime		= pPS->fRampDownTime;
	m_fDuration			= pPS->fDuration;
	m_fAddDelay			= pPS->fAddDelay;
	m_fRadius			= pPS->fRadius;
	m_fGravity			= pPS->fGravity;
	VEC_COPY(m_vRotations, pPS->vRotations);
	m_fPosRadius		= pPS->fPosRadius;
	m_nEmitType			= pPS->nEmitType;
	m_nNumParticles		= pPS->nNumParticles;
	m_fDensity			= pPS->fDensity;
	m_fAlpha			= pPS->fAlpha;
	VEC_COPY(m_vColor1, pPS->vMinColor);
	VEC_COPY(m_vColor2, pPS->vMaxColor);
	m_fMinVelocity		= pPS->fMinVelocity;
	m_fMaxVelocity		= pPS->fMaxVelocity;
	m_fMinLifetime		= pPS->fMinLifetime;
	m_fMaxLifetime		= pPS->fMaxLifetime;
	m_nRampUpType		= pPS->nRampUpType;
	m_nRampDownType		= pPS->nRampDownType;
	m_nFXFlags			= pPS->nFXFlags;
	m_szParticle		= pPS->szParticle;
	VEC_INIT(m_vPos);

	m_bMove				= DTRUE;
	m_bFirstStop		= DTRUE;
	m_fStopTime			= 0.0f;
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectParticleFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CObjectParticleFX::CreateObject(CClientDE *pClientDE)
{
	if(!pClientDE || !m_hObj) return DFALSE;

	if(m_szParticle)	m_pTextureName = pClientDE->GetStringData(m_szParticle);
		else			m_pTextureName = "SpriteTextures\\smoke64_2.dtx";

	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

//	pClientDE->SetObjectClientFlags(m_hObj, pClientDE->GetObjectClientFlags(m_hObj)|CF_NOTIFYREMOVE);

	// Setup the initial position variables for the object
	m_pClientDE->GetObjectPos(m_hObj, &m_vPos);
	m_pClientDE->SetObjectPos(m_hObject, &m_vPos);

//	m_pClientDE->GetObjectRotation(m_hObj, &m_rLastRot);

	VEC_COPY(m_vLastPos, m_vPos);
	VEC_COPY(m_vLastObjPos, m_vPos);
	VEC_COPY(m_vOrigin, m_vPos);

	m_fStartTime = m_pClientDE->GetTime();
	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectParticleFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CObjectParticleFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hObj || !m_hServerObject)
		return DFALSE;

	DFLOAT		fTime = m_pClientDE->GetTime() - m_fStartTime;
	DVector		vU, vR, vF;
	DRotation	rRot;

	// Initialize some data during the first update...
	if(m_bFirstUpdate)
	{
		m_fLastAddTime = 0.0f;
		m_bFirstUpdate = 0;
		return DTRUE;
	}

	// See if we should delete the FX
	if(m_fDuration)
	{
		if(m_fDuration < fTime)
			return DFALSE;
	}
	else
	{
		if(!g_pBloodClientShell->IsMultiplayerGame())
		{
			DVector	vPos;
			m_pClientDE->GetObjectPos(m_hObj, &vPos);

			if(m_nFXFlags & OBJPSFX_REMOVESTOPPED)
			{
				DVector	vVel;
				VEC_SUB(vVel, vPos, m_vLastObjPos);

				if((vVel.x == 0.0f) && (vVel.y == 0.0f) && (vVel.z == 0.0f))
				{
					m_bMove = DFALSE;

					if(m_nFXFlags & OBJPSFX_RAMPDOWNSTOPPED)
						m_fDuration = fTime + m_fRampDownTime + m_fMaxLifetime;
					else
						return DFALSE;
				}
			}

			VEC_COPY(m_vLastObjPos, vPos);
		}
		else
		{
			DVector	vPos;
			m_pClientDE->GetObjectPos(m_hObj, &vPos);

			if(m_nFXFlags & OBJPSFX_REMOVESTOPPED)
			{
				DVector	vVel;
				VEC_SUB(vVel, vPos, m_vLastObjPos);

				if((vVel.x == 0.0f) && (vVel.y == 0.0f) && (vVel.z == 0.0f))
				{
					m_bMove = DFALSE;

					if(m_bFirstStop)
					{
						m_fStopTime = m_pClientDE->GetTime();
						m_bFirstStop = DFALSE;
					}

					if(m_pClientDE->GetTime() > m_fStopTime + 1.0f)
					{
						if(m_nFXFlags & OBJPSFX_RAMPDOWNSTOPPED)
							m_fDuration = fTime + m_fRampDownTime + m_fMaxLifetime;
						else
							return DFALSE;
					}
				}
				else
				{
					m_bFirstStop = DTRUE;
					m_bMove = DTRUE;
				}
			}

			VEC_COPY(m_vLastObjPos, vPos);
		}
	}

	//*********************************************************************
	// Set the ramp state and ratio
	//*********************************************************************

	if(m_nRampUpType && m_fDuration && (fTime <= m_fRampUpTime))
	{
		m_fRampRatio = fTime / m_fRampUpTime;
		m_bRampState = OBJPSFX_RAMPUP;
		if(m_fRampRatio > 1.0f)		m_fRampRatio = 1.0f;
	}
	else if(m_nRampDownType && m_fDuration && (fTime >= m_fDuration - m_fRampDownTime))
	{
		m_fRampRatio = (m_fDuration - fTime) / m_fRampDownTime;
		m_bRampState = OBJPSFX_RAMPDOWN;
		if(m_fRampRatio < 0.0f)		m_fRampRatio = 0.0f;
	}
	else
	{
		m_fRampRatio = 1.0f;
		m_bRampState = 0;
	}

	//*********************************************************************
	// Handle the location and rotation of the system
	//*********************************************************************

	if(m_bMove)
	{
		m_pClientDE->GetObjectPos(m_hObj, &m_vPos);
		m_pClientDE->GetObjectRotation(m_hObj, &rRot);
		m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		VEC_MULSCALAR(vU, vU, m_vOffset.x);
		VEC_MULSCALAR(vR, vR, m_vOffset.y);
		VEC_MULSCALAR(vF, vF, m_vOffset.z);
		VEC_ADD(m_vPos, m_vPos, vU);
		VEC_ADD(m_vPos, m_vPos, vR);
		VEC_ADD(m_vPos, m_vPos, vF);
	}

	// Update the position of the object if we should
	if(!(m_nFXFlags & OBJPSFX_MOVINGSOURCE))
		m_pClientDE->SetObjectPos(m_hObject, &m_vPos);

	// Update the rotation of the system with the object
	if(m_nFXFlags & OBJPSFX_ALIGNROTATION)
	{
		m_pClientDE->GetObjectRotation(m_hObj, &m_rRot);
		m_pClientDE->GetRotationVectors(&m_rRot, &vU, &vR, &vF);
		m_pClientDE->GetObjectRotation(m_hObject, &m_rRot);
		m_pClientDE->AlignRotation(&m_rRot, &vF, &vU);
	}
	else
		m_pClientDE->GetObjectRotation(m_hObject, &m_rRot);

	// Rotate the object if it has rotation values
	DVector		vRotate;
	VEC_COPY(vRotate, m_vRotations);

	if(((m_bRampState == OBJPSFX_RAMPUP) && (m_nRampUpType & OBJPSFX_RAMPROTATION)) ||
	  ((m_bRampState == OBJPSFX_RAMPDOWN) && (m_nRampDownType & OBJPSFX_RAMPROTATION)))
		VEC_MULSCALAR(vRotate, vRotate, m_fRampRatio);

	if(m_vRotations.x || m_vRotations.y || m_vRotations.z)
	{
		if(m_vRotations.x)	m_pClientDE->EulerRotateX(&m_rRot, vRotate.x * MATH_PI);
		if(m_vRotations.y)	m_pClientDE->EulerRotateY(&m_rRot, vRotate.y * MATH_PI);
		if(m_vRotations.z)	m_pClientDE->EulerRotateZ(&m_rRot, vRotate.z * MATH_PI);
	}

	if(!(m_nFXFlags & OBJPSFX_ROTATINGSOURCE))
		m_pClientDE->SetObjectRotation(m_hObject, &m_rRot);

	//*********************************************************************
	// Handle the alpha ramp values and add more particles
	//*********************************************************************

	if(((m_bRampState == OBJPSFX_RAMPUP) && (m_nRampUpType & OBJPSFX_RAMPALPHA)) ||
	  ((m_bRampState == OBJPSFX_RAMPDOWN) && (m_nRampDownType & OBJPSFX_RAMPALPHA)))
	{
		// Fade in the particles
		DFLOAT		r, g, b, a;
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		a = m_fAlpha * m_fRampRatio;
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, a);
	}

	// Add more particles if it's time to
	if(m_pClientDE->GetTime() - m_fLastAddTime >= m_fAddDelay)
	{
		m_fLastAddTime = m_pClientDE->GetTime();
		AddParticles();

		VEC_COPY(m_vLastPos, m_vPos);
		ROT_COPY(m_rLastRot, m_rRot);
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectParticleFX::AddParticles
//
//	PURPOSE:	Add a new batch of particles
//
// ----------------------------------------------------------------------- //

void CObjectParticleFX::AddParticles()
{
	if(!m_pClientDE) return;

	// Add the particles to the system
	DDWORD		i, j = (DDWORD)m_fDensity;
	DFLOAT		life;
	DVector		start, vel, color, vUp, vU, vR, vF;
	DRotation	rRot;

	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
	ROT_COPY(rRot, m_rRot);

	switch(m_nEmitType)
	{
		case	OBJPSFX_EMITLOCATION:
		{
			m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

			VEC_SET(start, 0.0f, 0.0f, 0.0f);
			VEC_NORM(vF);

			if(m_nFXFlags & OBJPSFX_MOVINGSOURCE)
			{
				VEC_SUB(color, m_vPos, m_vOrigin);
				VEC_ADD(start, start, color);
			}

			if(m_nFXFlags & OBJPSFX_MOVINGSTREAM)
			{
				DVector		dif;
				DFLOAT		mag;
				DDWORD		count, num;

				VEC_SUB(dif, m_vLastPos, m_vPos);
				mag = VEC_MAG(dif);
				num = (DDWORD)(mag / (m_fDensity * m_fRadius / 1000.0f));
				VEC_DIVSCALAR(dif, dif, (DFLOAT)num);

				for(count = 0; count < num; count++)
				{
					VEC_MULSCALAR(vel, vF, GetRandom(m_fMinVelocity, m_fMaxVelocity));
					GetRandomColor(color);
					life = GetRandom(m_fMinLifetime, m_fMaxLifetime);
					m_pClientDE->AddParticle(m_hObject, &start, &vel, &color, life);
					VEC_ADD(start, start, dif);
				}
			}
			else
			{
				VEC_MULSCALAR(vel, vF, GetRandom(m_fMinVelocity, m_fMaxVelocity));
				GetRandomColor(color);
				life = GetRandom(m_fMinLifetime, m_fMaxLifetime);
				m_pClientDE->AddParticle(m_hObject, &start, &vel, &color, life);
			}
			break;
		}

		case	OBJPSFX_EMITRING:
		case	OBJPSFX_EMITFILLEDRING:
		case	OBJPSFX_EMITSPHERE:
		case	OBJPSFX_EMITFILLEDSPHERE:
		{
			DVector		dif, offset;
			DFLOAT		mag;
			DDWORD		count, num = 1;

			VEC_SET(offset, 0.0f, 0.0f, 0.0f);

			if(m_nFXFlags & OBJPSFX_MOVINGSTREAM)
			{
				VEC_SUB(dif, m_vLastPos, m_vPos);
				mag = VEC_MAG(dif);
				num = (DDWORD)(mag / (m_fDensity * m_fRadius / 1000.0f));
				VEC_DIVSCALAR(dif, dif, (DFLOAT)num);
			}

			for(count = 0; count < num; count++)
			{
				for(i = 0; i < m_nNumParticles; i++)
				{
					m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

					if((m_nEmitType == OBJPSFX_EMITSPHERE) || (m_nEmitType == OBJPSFX_EMITFILLEDSPHERE))
					{
						m_pClientDE->RotateAroundAxis(&rRot, &vF, GetRandom(-MATH_PI, MATH_PI));
						m_pClientDE->RotateAroundAxis(&rRot, &vR, GetRandom(-MATH_PI, MATH_PI));
					}
					else
						m_pClientDE->RotateAroundAxis(&rRot, &vF, GetRandom(-MATH_PI, MATH_PI));

					VEC_NORM(vR);
					VEC_COPY(start, vR);
					VEC_COPY(vel, vR);
					VEC_MULSCALAR(vel, vel, GetRandom(m_fMinVelocity, m_fMaxVelocity));

					if((m_nEmitType == OBJPSFX_EMITFILLEDRING) || (m_nEmitType == OBJPSFX_EMITFILLEDSPHERE))
						{ VEC_MULSCALAR(start, start, GetRandom(0.0f, m_fPosRadius)); }
					else
						{ VEC_MULSCALAR(start, start, m_fPosRadius); }

					if(m_nFXFlags & OBJPSFX_MOVINGSOURCE)
					{
						VEC_SUB(color, m_vPos, m_vOrigin);
						VEC_ADD(start, start, color);
					}

					if(m_nFXFlags & OBJPSFX_MOVINGSTREAM)
					{
						VEC_ADD(start, start, offset);
					}

					GetRandomColor(color);
					life = GetRandom(m_fMinLifetime, m_fMaxLifetime);

					m_pClientDE->AddParticle(m_hObject, &start, &vel, &color, life);
				}

				VEC_ADD(offset, offset, dif);
			}
			break;
		}

		case	OBJPSFX_EMITCYLINDER:
		case	OBJPSFX_EMITFILLEDCYLINDER:
		{
			DVector		dif, offset, up;
			DRotation	tempRot;
			DFLOAT		mag;
			DDWORD		count, num = 1;

			VEC_SET(up, 0.0f, 1.0f, 0.0f);
			VEC_SET(offset, 0.0f, 0.0f, 0.0f);

			m_pClientDE->GetObjectPos(m_hObj, &dif);
			VEC_SUB(dif, dif, m_vPos);
			mag = VEC_MAG(dif) * 2.0f;
			num = (DDWORD)(mag / (m_fDensity * m_fRadius / 1000.0f));
			if(num < 1) num = 1;

			m_pClientDE->AlignRotation(&tempRot, &dif, &up);
			m_pClientDE->GetRotationVectors(&tempRot, &vU, &vR, &dif);
			VEC_MULSCALAR(dif, dif, mag / (DFLOAT)num);

			for(count = 0; count < num; count++)
			{
				for(i = 0; i < m_nNumParticles; i++)
				{
					m_pClientDE->GetRotationVectors(&tempRot, &vU, &vR, &vF);
					m_pClientDE->RotateAroundAxis(&tempRot, &vF, GetRandom(-MATH_PI, MATH_PI));

					VEC_NORM(vR);
					VEC_COPY(start, vR);
					VEC_COPY(vel, vR);
					VEC_MULSCALAR(vel, vel, GetRandom(m_fMinVelocity, m_fMaxVelocity));

					if(m_nEmitType == OBJPSFX_EMITFILLEDCYLINDER)
						{ VEC_MULSCALAR(start, start, GetRandom(0.0f, m_fPosRadius)); }
					else
						{ VEC_MULSCALAR(start, start, m_fPosRadius); }

					if(m_nFXFlags & OBJPSFX_MOVINGSOURCE)
					{
						VEC_SUB(color, m_vPos, m_vOrigin);
						VEC_ADD(start, start, color);
					}

					VEC_ADD(start, start, offset);

					GetRandomColor(color);
					life = GetRandom(m_fMinLifetime, m_fMaxLifetime);

					m_pClientDE->AddParticle(m_hObject, &start, &vel, &color, life);
				}

				VEC_ADD(offset, offset, dif);
			}
			break;
		}

		case	OBJPSFX_EMITCOMETTAIL:
		{
			DVector		dif, offset;
			DFLOAT		mag;
			DDWORD		count, num = 1;

			VEC_SET(offset, 0.0f, 0.0f, 0.0f);

			if(m_nFXFlags & OBJPSFX_MOVINGSTREAM)
			{
				VEC_SUB(dif, m_vLastPos, m_vPos);
				mag = VEC_MAG(dif);
				num = (DDWORD)(mag / (m_fDensity * m_fRadius / 1000.0f));
				VEC_DIVSCALAR(dif, dif, (DFLOAT)num);
			}

			for(count = 0; count < num; count++)
			{
				for(i = 0; i < m_nNumParticles; i++)
				{
					m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

					m_pClientDE->RotateAroundAxis(&rRot, &vF, GetRandom(-MATH_PI, MATH_PI));
					m_pClientDE->RotateAroundAxis(&rRot, &vR, GetRandom(-MATH_PI, MATH_PI));

					VEC_NORM(vR);
					VEC_COPY(start, vR);
					VEC_COPY(vel, vR);
					VEC_MULSCALAR(vel, vel, GetRandom(m_fMinVelocity, m_fMaxVelocity));

					mag = GetRandom(0.0f, m_fPosRadius);
					VEC_MULSCALAR(start, start, mag);

					if(m_nFXFlags & OBJPSFX_MOVINGSOURCE)
					{
						VEC_SUB(color, m_vPos, m_vOrigin);
						VEC_ADD(start, start, color);
					}

					if(m_nFXFlags & OBJPSFX_MOVINGSTREAM)
					{
						VEC_ADD(start, start, offset);
					}

					GetRandomColor(color);
					life = m_fMaxLifetime - ((m_fMaxLifetime - m_fMinLifetime) * (mag / m_fPosRadius));

					m_pClientDE->AddParticle(m_hObject, &start, &vel, &color, life);
				}

				VEC_ADD(offset, offset, dif);
			}
			break;
		}

		case	OBJPSFX_EMITPOWERRING:
		{
			DVector		dif, offset;
			DFLOAT		mag;
			DDWORD		count, num = 1;
			DFLOAT		rotate = (MATH_PI * 2.0f) / (float)m_nNumParticles;

			VEC_SET(offset, 0.0f, 0.0f, 0.0f);

			if(m_nFXFlags & OBJPSFX_MOVINGSTREAM)
			{
				VEC_SUB(dif, m_vLastPos, m_vPos);
				mag = VEC_MAG(dif);
				num = (DDWORD)(mag / (m_fDensity * m_fRadius / 1000.0f));
				VEC_DIVSCALAR(dif, dif, (DFLOAT)num);
			}

			if(m_nFXFlags & OBJPSFX_ALIGNROTATION)
			{
				DVector	forward;
				VEC_SET(forward, 0.0f, 0.0f, 1.0f);
				m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
				m_pClientDE->AlignRotation(&rRot, &forward, &vU);
//				ROT_INIT(rRot);
			}

			for(count = 0; count < num; count++)
			{
				for(i = 0; i < m_nNumParticles; i++)
				{
					m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
					m_pClientDE->RotateAroundAxis(&rRot, &vF, rotate);

					VEC_NORM(vR);
					VEC_MULSCALAR(start, vR, m_fPosRadius);
					VEC_MULSCALAR(vel, vF, m_fMaxVelocity);
					VEC_MULSCALAR(vR, vR, m_fMinVelocity);
					VEC_ADD(vel, vel, vR);

					if(m_nFXFlags & OBJPSFX_MOVINGSOURCE)
					{
						VEC_SUB(color, m_vPos, m_vOrigin);
						VEC_ADD(start, start, color);
					}

					if(m_nFXFlags & OBJPSFX_MOVINGSTREAM)
					{
						VEC_ADD(start, start, offset);
					}

					GetRandomColor(color);
					life = GetRandom(m_fMinLifetime, m_fMaxLifetime);

					m_pClientDE->AddParticle(m_hObject, &start, &vel, &color, life);
				}

				VEC_ADD(offset, offset, dif);
			}
			break;
		}

		default:
		{
			for(i = 0; i < m_nNumParticles; i++)
			{
				m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
				m_pClientDE->RotateAroundAxis(&rRot, &vF, GetRandom(-MATH_PI, MATH_PI));

				VEC_NORM(vR);
				VEC_COPY(start, vR);
				VEC_COPY(vel, vR);
				VEC_MULSCALAR(vel, vel, GetRandom(m_fMinVelocity, m_fMaxVelocity));

				VEC_MULSCALAR(start, start, m_fPosRadius);

				if(m_nFXFlags & OBJPSFX_MOVINGSOURCE)
				{
					VEC_SUB(color, m_vPos, m_vOrigin);
					VEC_ADD(start, start, color);
				}

				GetRandomColor(color);
				life = GetRandom(m_fMinLifetime, m_fMaxLifetime);

				m_pClientDE->AddParticle(m_hObject, &start, &vel, &color, life);
			}
			break;
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectParticleFX::GetRandomColor
//
//	PURPOSE:	Get a random color from Min to Max
//
// ----------------------------------------------------------------------- //

void CObjectParticleFX::GetRandomColor(DVector &vector)
{
	DFLOAT	ratio = GetRandom(0.0f, 1.0f);

	vector.x = m_vColor1.x + ((m_vColor2.x - m_vColor1.x) * ratio);
	vector.y = m_vColor1.y + ((m_vColor2.y - m_vColor1.y) * ratio);
	vector.z = m_vColor1.z + ((m_vColor2.z - m_vColor1.z) * ratio);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectGeneralFX::Init
//
//	PURPOSE:	Init the FX location and type
//
// ----------------------------------------------------------------------- //

DBOOL CObjectGeneralFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	OBJECTFXCS* pOFXCS = (OBJECTFXCS*)psfxCreateStruct;

	m_hObj			= pOFXCS->hObj;
	VEC_COPY(m_vOffset, pOFXCS->vOffset);
	m_fScale		= pOFXCS->fScale;
	m_nScaleFlags	= pOFXCS->nScaleFlags;
	m_nFXType		= pOFXCS->nFXType;
	m_nFXFlags		= pOFXCS->nFXFlags;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CObjectGeneralFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CObjectGeneralFX::CreateObject(CClientDE *pClientDE)
{
	CBloodClientShell *pShell = (CBloodClientShell*)pClientDE->GetClientShell();
	if (!pClientDE || !pShell) return DFALSE;

	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return DFALSE;

	OBJECTPARTICLEFXCS	ops;

	HCONSOLEVAR hVar = pClientDE->GetConsoleVar("ObjectParticles");
	DBYTE	detail = (DBYTE)pClientDE->GetVarValueFloat(hVar);

	pClientDE->SetObjectClientFlags(m_hObj, 0);

	if(detail >= 2)
	{
		switch(m_nFXType)
		{
  			case	OBJFX_SMOKETRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 2000.0f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.nNumParticles	= 12;
				ops.fDensity		= 1.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 96.0f, 96.0f, 96.0f);
				VEC_SET(ops.vMaxColor, 192.0f, 192.0f, 192.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_ALL;

				ops.szParticle = pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_SMOKETRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.05f;
				ops.fRadius			= 1200.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 8;
				ops.fDensity		= 3.5f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 96.0f, 96.0f, 96.0f);
				VEC_SET(ops.vMaxColor, 192.0f, 192.0f, 192.0f);
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_ALL;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_FIRETRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 200.0f;
				ops.fPosRadius		= 6.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 1;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 255.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_FIRETRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 250.0f;
				ops.fPosRadius		= 6.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 1;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_FIRETRAIL_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 250.0f;
				ops.fPosRadius		= 6.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 1;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 196.0f, 196.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SPARKS_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 350.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nNumParticles	= 4;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SPARKS_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 500.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nNumParticles	= 7;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 35.0f;
				ops.fMaxVelocity	= 60.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SPARKS_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 400.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 50.0f;
				ops.fMaxVelocity	= 75.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SINGULARITY_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.35f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 1.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= GetRandom(75.0f,100.0f);
				VEC_SET(ops.vRotations, GetRandom(0.0f,0.01f), GetRandom(0.0f,0.015f), GetRandom(0.0f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 12;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 128.0f, 255.0f);
				ops.fMinVelocity	= -125.0f;
				ops.fMaxVelocity	= -175.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SINGULARITY_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.5f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 10.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 150.0f;
				VEC_SET(ops.vRotations, GetRandom(0.0f,0.01f), GetRandom(0.0f,0.015f), GetRandom(0.0f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 32;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 128.0f);
				VEC_SET(ops.vMaxColor, 128.0f, 64.0f, 255.0f);
				ops.fMinVelocity	= -125.0f;
				ops.fMaxVelocity	= -175.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SINGULARITY_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 1.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.5f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 10.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 6000.0f;
				ops.fPosRadius		= 350.0f;
				VEC_SET(ops.vRotations, 0.0f, -0.015f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITFILLEDCYLINDER;
				ops.nNumParticles	= 32;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 64.0f, 0.0f, 64.0f);
				VEC_SET(ops.vMaxColor, 164.0f, 0.0f, 255.0f);
				ops.fMinVelocity	= -50.0f;
				ops.fMaxVelocity	= -75.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_VOODOO_1:		// Chest - Normal (red)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 5.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 12;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 0.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_2:		// Nut - Double (red)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 8;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 0.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_3:		// Eye - Blind (blue)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 8;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 0.0f, 0.0f, 255.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_4:		// Arm - Weapon (white)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 8;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_5:		// Leg - Slow (green)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 8;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 0.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_6:		// Wonkey Vision (purple)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 8;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 0.0f, 255.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_HEAL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, -25.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 2.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 500.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= 150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITRING;
				ops.nNumParticles	= 18;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.75f;
				ops.fMaxLifetime	= 0.9f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_SHIELD_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 5.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 500.0f;
				ops.fPosRadius		= 45.0f;
				VEC_SET(ops.vRotations, GetRandom(0.0f,0.01f), GetRandom(0.0f,0.015f), GetRandom(0.0f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 24;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_SMOKING_1:
			{
//				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 2.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= 150.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 5;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.nFXFlags		= OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_SMOKING_2:
			{
//				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 2.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 2500.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 125.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 3;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.nFXFlags		= OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_SMOKING_3:
			{
//				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 2.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 5000.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 100.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 2;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.nFXFlags		= OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_FLAMING_1:
			{
				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.05f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= 300.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 7;
				ops.fDensity		= 7.5f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_FLAMING_2:
			{
				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.05f;
				ops.fRadius			= 2500.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 275.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 5;
				ops.fDensity		= 7.5f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_FLAMING_3:
			{
				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.05f;
				ops.fRadius			= 4000.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 225.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 3;
				ops.fDensity		= 7.5f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_GROUNDFLAME_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 3000.0f;
				ops.fPosRadius		= 15.0f;
				ops.fGravity		= 500.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 8;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 255.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.4f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");
				break;
			}

			case	OBJFX_GROUNDFLAME_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 2000.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 12;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 255.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.2f;
				ops.fMaxLifetime	= 0.4f;
				ops.nFXFlags		= OBJPSFX_STREAMING | OBJPSFX_SMOOTHSTOP;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");
				break;
			}

			case	OBJFX_STAFFTRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 350.0f;
				ops.fPosRadius		= 10.0f;
				ops.fGravity		= 50.0f;
				ops.nNumParticles	= 3;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.fDensity		= 6.5f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 255.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= -10.0f;
				ops.fMaxVelocity	= -20.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.5f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_STAFFTRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 350.0f;
				ops.fPosRadius		= 10.0f;
				ops.fGravity		= 50.0f;
				ops.nNumParticles	= 3;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.fDensity		= 6.5f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= -10.0f;
				ops.fMaxVelocity	= -20.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.5f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_STAFFTRAIL_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 350.0f;
				ops.fPosRadius		= 10.0f;
				ops.fGravity		= 50.0f;
				ops.nNumParticles	= 3;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.fDensity		= 6.5f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= -10.0f;
				ops.fMaxVelocity	= -20.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.5f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_BUGSPRAY_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 400.0f;
				ops.fPosRadius		= 3.0f;
				ops.fGravity		= -75.0f;
				ops.nNumParticles	= 3;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 7.5f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 32.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 128.0f, 196.0f, 0.0f);
				ops.fMinVelocity	= -5.0f;
				ops.fMaxVelocity	= 5.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_BUGSPRAY_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 3.0f;
				ops.fGravity		= 100.0f;
				ops.nNumParticles	= 1;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 7.5f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.35f;
				ops.fMaxLifetime	= 0.5f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");
				break;
			}

			case	OBJFX_ORBTRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 200.0f;
				ops.fPosRadius		= 3.0f;
				ops.nNumParticles	= 3;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 7.5f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 128.0f, 128.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= -5.0f;
				ops.fMaxVelocity	= 5.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_ORBTRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 200.0f;
				ops.fPosRadius		= 3.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 128.0f, 128.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 40.0f;
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.20f;
				ops.nFXFlags		= OBJPSFX_ROTATINGSOURCE | OBJPSFX_ALIGNROTATION | OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_ENERGYWALL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 2.5f;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 250.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITRING;
				ops.nNumParticles	= 18;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.75f;
				ops.fMaxLifetime	= 1.0f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_POWERUP_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, -25.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 2.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1750.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITRING;
				ops.nNumParticles	= 18;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 196.0f, 196.0f, 255.0f);
				ops.fMinLifetime	= 0.75f;
				ops.fMaxLifetime	= 1.0f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_DEATHRAY_RING_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_COPY(ops.vOffset, m_vOffset);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 1.5f;
				ops.fAddDelay		= 1.5f;
				ops.fRadius			= 4000.0f;
				ops.fPosRadius		= 2.5f;
				VEC_SET(ops.vRotations, 0.0f, 0.0f, GetRandom(-0.01f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITPOWERRING;
				ops.nNumParticles	= 24;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 0.1f;
				VEC_SET(ops.vMinColor, 0.0f, 196.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 64.0f, 255.0f, 64.0f);
				ops.fMinVelocity	= 150.0f;
				ops.fMaxVelocity	= 450.0f;
				ops.fMinLifetime	= 0.85f;
				ops.fMaxLifetime	= 0.85f;
				ops.nFXFlags		= OBJPSFX_ALIGNROTATION;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_DEATHRAY_RING_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_COPY(ops.vOffset, m_vOffset);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 1.5f;
				ops.fAddDelay		= 1.5f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 2.5f;
				VEC_SET(ops.vRotations, 0.0f, 0.0f, GetRandom(-0.01f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITPOWERRING;
				ops.nNumParticles	= 32;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 196.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 85.0f;
				ops.fMaxVelocity	= 450.0f;
				ops.fMinLifetime	= 1.0f;
				ops.fMaxLifetime	= 1.0f;
				ops.nFXFlags		= OBJPSFX_ALIGNROTATION;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_ELECTRIC_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.75f;
				ops.fRadius			= 750.0f;
				ops.fPosRadius		= 15.0f;
				ops.fGravity		= -250.0f;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 24;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 196.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 100.0f;
				ops.fMaxVelocity	= 300.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

#ifdef _ADD_ON

			case	OBJFX_GREEN_SMOKE:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 2000.0f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.nNumParticles	= 12;
				ops.fDensity		= 1.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 0.0f, 64.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 128.0f, 192.0f, 128.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_ALL;

				ops.szParticle = pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_BLOOD_TRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 150.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 6;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 32.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 96.0f, 0.0f, 0.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;

				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\drop32_1.dtx");
				break;
			}

#endif

			default:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 2000.0f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.nNumParticles	= 12;
				ops.fDensity		= 1.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 96.0f, 96.0f, 96.0f);
				VEC_SET(ops.vMaxColor, 192.0f, 192.0f, 192.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_ALL;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}
		}
	}
	else if(detail == 1)
	{
		switch(m_nFXType)
		{
  			case	OBJFX_SMOKETRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 2000.0f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.nNumParticles	= 12;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 96.0f, 96.0f, 96.0f);
				VEC_SET(ops.vMaxColor, 192.0f, 192.0f, 192.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.35f;
				ops.fMaxLifetime	= 0.5f;
				ops.nFXFlags		= OBJPSFX_ALL;

				ops.szParticle = pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_SMOKETRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.05f;
				ops.fRadius			= 1200.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 5;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 96.0f, 96.0f, 96.0f);
				VEC_SET(ops.vMaxColor, 192.0f, 192.0f, 192.0f);
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.3f;
				ops.nFXFlags		= OBJPSFX_ALL;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_FIRETRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 200.0f;
				ops.fPosRadius		= 6.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 1;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 255.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.5f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_FIRETRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 250.0f;
				ops.fPosRadius		= 6.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 1;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.5f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_FIRETRAIL_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 250.0f;
				ops.fPosRadius		= 6.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 1;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 196.0f, 196.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.5f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SPARKS_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 350.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nNumParticles	= 4;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SPARKS_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 500.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nNumParticles	= 6;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 35.0f;
				ops.fMaxVelocity	= 60.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SPARKS_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 400.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 50.0f;
				ops.fMaxVelocity	= 75.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SINGULARITY_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.35f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 1.5f;
				ops.fAddDelay		= 0.15f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= GetRandom(75.0f,100.0f);
				VEC_SET(ops.vRotations, GetRandom(0.0f,0.01f), GetRandom(0.0f,0.015f), GetRandom(0.0f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 9;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 128.0f, 255.0f);
				ops.fMinVelocity	= -125.0f;
				ops.fMaxVelocity	= -175.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SINGULARITY_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.5f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 10.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 150.0f;
				VEC_SET(ops.vRotations, GetRandom(0.0f,0.01f), GetRandom(0.0f,0.015f), GetRandom(0.0f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 24;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 128.0f);
				VEC_SET(ops.vMaxColor, 128.0f, 64.0f, 255.0f);
				ops.fMinVelocity	= -125.0f;
				ops.fMaxVelocity	= -175.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SINGULARITY_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 1.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.5f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 10.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 6000.0f;
				ops.fPosRadius		= 350.0f;
				VEC_SET(ops.vRotations, 0.0f, -0.015f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITFILLEDCYLINDER;
				ops.nNumParticles	= 24;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 64.0f, 0.0f, 64.0f);
				VEC_SET(ops.vMaxColor, 164.0f, 0.0f, 255.0f);
				ops.fMinVelocity	= -50.0f;
				ops.fMaxVelocity	= -75.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_VOODOO_1:		// Chest - Normal (red)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 5.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 9;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 0.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_2:		// Nut - Double (red)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 6;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 0.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_3:		// Eye - Blind (blue)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 6;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 0.0f, 0.0f, 255.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_4:		// Arm - Weapon (white)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 6;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_5:		// Leg - Slow (green)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 6;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 0.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_6:		// Wonkey Vision (purple)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 6;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 0.0f, 255.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_HEAL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, -25.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 2.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 500.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= 150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITRING;
				ops.nNumParticles	= 12;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.75f;
				ops.fMaxLifetime	= 0.9f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_SHIELD_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 5.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 500.0f;
				ops.fPosRadius		= 45.0f;
				VEC_SET(ops.vRotations, GetRandom(0.0f,0.01f), GetRandom(0.0f,0.015f), GetRandom(0.0f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 18;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_SMOKING_1:
			{
//				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= 150.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 3;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.nFXFlags		= OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_SMOKING_2:
			{
//				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 2500.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 125.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 2;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.nFXFlags		= OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_SMOKING_3:
			{
//				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 5000.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 100.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 1;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.nFXFlags		= OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_FLAMING_1:
			{
				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= 300.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 5;
				ops.fDensity		= 7.5f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_FLAMING_2:
			{
				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 2500.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 275.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 2;
				ops.fDensity		= 7.5f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_FLAMING_3:
			{
				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 4000.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 225.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 1;
				ops.fDensity		= 7.5f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_GROUNDFLAME_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 3000.0f;
				ops.fPosRadius		= 15.0f;
				ops.fGravity		= 500.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 6;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 255.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.4f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");
				break;
			}

			case	OBJFX_GROUNDFLAME_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 2000.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 9;
				ops.fDensity		= 7.5f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 255.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.2f;
				ops.fMaxLifetime	= 0.4f;
				ops.nFXFlags		= OBJPSFX_STREAMING | OBJPSFX_SMOOTHSTOP;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");
				break;
			}

			case	OBJFX_STAFFTRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 450.0f;
				ops.fPosRadius		= 10.0f;
				ops.fGravity		= 50.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 255.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= -10.0f;
				ops.fMaxVelocity	= -20.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.45f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_STAFFTRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 450.0f;
				ops.fPosRadius		= 10.0f;
				ops.fGravity		= 50.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= -10.0f;
				ops.fMaxVelocity	= -20.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.45f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_STAFFTRAIL_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 450.0f;
				ops.fPosRadius		= 10.0f;
				ops.fGravity		= 50.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= -10.0f;
				ops.fMaxVelocity	= -20.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.45f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_BUGSPRAY_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 400.0f;
				ops.fPosRadius		= 3.0f;
				ops.fGravity		= -75.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 32.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 128.0f, 196.0f, 0.0f);
				ops.fMinVelocity	= -5.0f;
				ops.fMaxVelocity	= 5.0f;
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.25f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_BUGSPRAY_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 3.0f;
				ops.fGravity		= 100.0f;
				ops.nNumParticles	= 1;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.4f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");
				break;
			}

			case	OBJFX_ORBTRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 200.0f;
				ops.fPosRadius		= 3.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 128.0f, 128.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= -5.0f;
				ops.fMaxVelocity	= 5.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_ORBTRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 200.0f;
				ops.fPosRadius		= 3.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 128.0f, 128.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 40.0f;
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.20f;
				ops.nFXFlags		= OBJPSFX_ROTATINGSOURCE | OBJPSFX_ALIGNROTATION | OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_ENERGYWALL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 2.5f;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 250.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITRING;
				ops.nNumParticles	= 12;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.75f;
				ops.fMaxLifetime	= 1.0f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_POWERUP_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, -25.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 2.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1750.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITRING;
				ops.nNumParticles	= 12;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 196.0f, 196.0f, 255.0f);
				ops.fMinLifetime	= 0.75f;
				ops.fMaxLifetime	= 1.0f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_DEATHRAY_RING_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_COPY(ops.vOffset, m_vOffset);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 1.5f;
				ops.fAddDelay		= 1.5f;
				ops.fRadius			= 4500.0f;
				ops.fPosRadius		= 2.5f;
				VEC_SET(ops.vRotations, 0.0f, 0.0f, GetRandom(-0.01f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITPOWERRING;
				ops.nNumParticles	= 18;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 0.1f;
				VEC_SET(ops.vMinColor, 0.0f, 196.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 64.0f, 255.0f, 64.0f);
				ops.fMinVelocity	= 150.0f;
				ops.fMaxVelocity	= 450.0f;
				ops.fMinLifetime	= 0.85f;
				ops.fMaxLifetime	= 0.85f;
				ops.nFXFlags		= OBJPSFX_ALIGNROTATION;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_DEATHRAY_RING_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_COPY(ops.vOffset, m_vOffset);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 1.5f;
				ops.fAddDelay		= 1.5f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 2.5f;
				VEC_SET(ops.vRotations, 0.0f, 0.0f, GetRandom(-0.01f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITPOWERRING;
				ops.nNumParticles	= 24;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 196.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 85.0f;
				ops.fMaxVelocity	= 450.0f;
				ops.fMinLifetime	= 1.0f;
				ops.fMaxLifetime	= 1.0f;
				ops.nFXFlags		= OBJPSFX_ALIGNROTATION;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_ELECTRIC_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.75f;
				ops.fRadius			= 750.0f;
				ops.fPosRadius		= 15.0f;
				ops.fGravity		= -250.0f;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 18;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 196.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 100.0f;
				ops.fMaxVelocity	= 300.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

#ifdef _ADD_ON

			case	OBJFX_GREEN_SMOKE:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 2000.0f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.nNumParticles	= 12;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 0.0f, 64.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 128.0f, 192.0f, 128.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.35f;
				ops.fMaxLifetime	= 0.5f;
				ops.nFXFlags		= OBJPSFX_ALL;

				ops.szParticle = pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_BLOOD_TRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 150.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 4;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 32.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 96.0f, 0.0f, 0.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.35f;
				ops.fMaxLifetime	= 0.5f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;

				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\drop32_1.dtx");
				break;
			}

#endif

			default:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 2000.0f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.nNumParticles	= 9;
				ops.fDensity		= 1.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 96.0f, 96.0f, 96.0f);
				VEC_SET(ops.vMaxColor, 192.0f, 192.0f, 192.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_ALL;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}
		}
	}
	else
	{
		switch(m_nFXType)
		{
  			case	OBJFX_SMOKETRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 2000.0f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.nNumParticles	= 12;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 96.0f, 96.0f, 96.0f);
				VEC_SET(ops.vMaxColor, 192.0f, 192.0f, 192.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_ALL;

				ops.szParticle = pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_SMOKETRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.05f;
				ops.fRadius			= 1200.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 3;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 96.0f, 96.0f, 96.0f);
				VEC_SET(ops.vMaxColor, 192.0f, 192.0f, 192.0f);
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.3f;
				ops.nFXFlags		= OBJPSFX_ALL;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_FIRETRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 200.0f;
				ops.fPosRadius		= 6.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 1;
				ops.fDensity		= 20.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 255.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_FIRETRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 250.0f;
				ops.fPosRadius		= 6.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 1;
				ops.fDensity		= 20.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_FIRETRAIL_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 250.0f;
				ops.fPosRadius		= 6.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 1;
				ops.fDensity		= 20.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 196.0f, 196.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SPARKS_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 350.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nNumParticles	= 4;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SPARKS_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 500.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nNumParticles	= 5;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 35.0f;
				ops.fMaxVelocity	= 60.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SPARKS_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fAddDelay		= 0.05f;
				ops.fRadius			= 400.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 50.0f;
				ops.fMaxVelocity	= 75.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SINGULARITY_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.35f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 1.5f;
				ops.fAddDelay		= 0.2f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= GetRandom(75.0f,100.0f);
				VEC_SET(ops.vRotations, GetRandom(0.0f,0.01f), GetRandom(0.0f,0.015f), GetRandom(0.0f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 6;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 128.0f, 255.0f);
				ops.fMinVelocity	= -125.0f;
				ops.fMaxVelocity	= -175.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SINGULARITY_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.5f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 10.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 150.0f;
				VEC_SET(ops.vRotations, GetRandom(0.0f,0.01f), GetRandom(0.0f,0.015f), GetRandom(0.0f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 16;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 128.0f);
				VEC_SET(ops.vMaxColor, 128.0f, 64.0f, 255.0f);
				ops.fMinVelocity	= -125.0f;
				ops.fMaxVelocity	= -175.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_SINGULARITY_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 1.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.5f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 10.5f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 6000.0f;
				ops.fPosRadius		= 350.0f;
				VEC_SET(ops.vRotations, 0.0f, -0.015f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITFILLEDCYLINDER;
				ops.nNumParticles	= 16;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 64.0f, 0.0f, 64.0f);
				VEC_SET(ops.vMaxColor, 164.0f, 0.0f, 255.0f);
				ops.fMinVelocity	= -50.0f;
				ops.fMaxVelocity	= -75.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_VOODOO_1:		// Chest - Normal (red)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 5.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 6;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 0.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_2:		// Nut - Double (red)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 4;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 0.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_3:		// Eye - Blind (blue)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 4;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 0.0f, 0.0f, 255.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_4:		// Arm - Weapon (white)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 4;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_5:		// Leg - Slow (green)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 4;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 0.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_VOODOO_6:		// Wonkey Vision (purple)
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, 20.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= -150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITCYLINDER;
				ops.nNumParticles	= 4;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 0.0f, 255.0f);
				ops.fMinVelocity	= 25.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_HEAL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, -25.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 2.5f;
				ops.fAddDelay		= 0.15f;
				ops.fRadius			= 500.0f;
				ops.fPosRadius		= 20.0f;
				ops.fGravity		= 150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITRING;
				ops.nNumParticles	= 9;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.75f;
				ops.fMaxLifetime	= 0.9f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_SHIELD_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 5.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 500.0f;
				ops.fPosRadius		= 45.0f;
				VEC_SET(ops.vRotations, GetRandom(0.0f,0.01f), GetRandom(0.0f,0.015f), GetRandom(0.0f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 12;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_SMOKING_1:
			{
//				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.15f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= 150.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 3;
				ops.fDensity		= 20.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_SMOKING_2:
			{
//				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.15f;
				ops.fRadius			= 2500.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 125.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 2;
				ops.fDensity		= 20.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_SMOKING_3:
			{
//				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.15f;
				ops.fRadius			= 5000.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 100.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 1;
				ops.fDensity		= 20.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 192.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 1.5f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_FLAMING_1:
			{
				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.1f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= 300.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 3;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_FLAMING_2:
			{
				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.15f;
				ops.fRadius			= 2500.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 275.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 2;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_FLAMING_3:
			{
				pClientDE->SetObjectClientFlags(m_hObj, CF_NOTIFYREMOVE);

				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fDuration		= 0.0f;
				ops.fAddDelay		= 0.15f;
				ops.fRadius			= 4000.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 225.0f;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.nNumParticles	= 1;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.75f;
				VEC_SET(ops.vMinColor, 192.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_MOVINGSOURCE;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");
				break;
			}

			case	OBJFX_GROUNDFLAME_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 3000.0f;
				ops.fPosRadius		= 15.0f;
				ops.fGravity		= 500.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 4;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 255.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.4f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");
				break;
			}

			case	OBJFX_GROUNDFLAME_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 2000.0f;
				ops.fPosRadius		= 12.5f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 6;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 255.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinLifetime	= 0.2f;
				ops.fMaxLifetime	= 0.4f;
				ops.nFXFlags		= OBJPSFX_STREAMING | OBJPSFX_SMOOTHSTOP;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");
				break;
			}

			case	OBJFX_STAFFTRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 450.0f;
				ops.fPosRadius		= 10.0f;
				ops.fGravity		= 50.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 255.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= -10.0f;
				ops.fMaxVelocity	= -20.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_STAFFTRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 450.0f;
				ops.fPosRadius		= 10.0f;
				ops.fGravity		= 50.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= -10.0f;
				ops.fMaxVelocity	= -20.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_STAFFTRAIL_3:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 450.0f;
				ops.fPosRadius		= 10.0f;
				ops.fGravity		= 50.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 0.0f);
				ops.fMinVelocity	= -10.0f;
				ops.fMaxVelocity	= -20.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_BUGSPRAY_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 400.0f;
				ops.fPosRadius		= 3.0f;
				ops.fGravity		= -75.0f;
				ops.nNumParticles	= 1;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 32.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 128.0f, 196.0f, 0.0f);
				ops.fMinVelocity	= -5.0f;
				ops.fMaxVelocity	= 5.0f;
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.25f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_BUGSPRAY_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 3.0f;
				ops.fGravity		= 100.0f;
				ops.nNumParticles	= 1;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 192.0f, 192.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.3f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");
				break;
			}

			case	OBJFX_ORBTRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 200.0f;
				ops.fPosRadius		= 3.0f;
				ops.nNumParticles	= 2;
				ops.nEmitType		= OBJPSFX_EMITFILLEDSPHERE;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 128.0f, 128.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= -5.0f;
				ops.fMaxVelocity	= 5.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_SMOOTHSTOP | OBJPSFX_STREAMING;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_ORBTRAIL_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 200.0f;
				ops.fPosRadius		= 3.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.fDensity		= 15.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 128.0f, 128.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 40.0f;
				ops.fMinLifetime	= 0.15f;
				ops.fMaxLifetime	= 0.20f;
				ops.nFXFlags		= OBJPSFX_ROTATINGSOURCE | OBJPSFX_ALIGNROTATION | OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare32_1.dtx");
				break;
			}

			case	OBJFX_ENERGYWALL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 2.5f;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 1500.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 250.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITRING;
				ops.nNumParticles	= 9;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinLifetime	= 0.75f;
				ops.fMaxLifetime	= 1.0f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_POWERUP_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_SET(ops.vOffset, -25.0f, 0.0f, 0.0f);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 2.5f;
				ops.fAddDelay		= 0.15f;
				ops.fRadius			= 1750.0f;
				ops.fPosRadius		= 25.0f;
				ops.fGravity		= 150.0f;
				VEC_SET(ops.vRotations, 0.0f, 0.025f, 0.0f);
				ops.nEmitType		= OBJPSFX_EMITRING;
				ops.nNumParticles	= 9;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 255.0f);
				VEC_SET(ops.vMaxColor, 196.0f, 196.0f, 255.0f);
				ops.fMinLifetime	= 0.75f;
				ops.fMaxLifetime	= 1.0f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_DEATHRAY_RING_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_COPY(ops.vOffset, m_vOffset);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 1.5f;
				ops.fAddDelay		= 1.5f;
				ops.fRadius			= 5000.0f;
				ops.fPosRadius		= 2.5f;
				VEC_SET(ops.vRotations, 0.0f, 0.0f, GetRandom(-0.01f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITPOWERRING;
				ops.nNumParticles	= 12;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 0.1f;
				VEC_SET(ops.vMinColor, 0.0f, 196.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 64.0f, 255.0f, 64.0f);
				ops.fMinVelocity	= 150.0f;
				ops.fMaxVelocity	= 450.0f;
				ops.fMinLifetime	= 0.85f;
				ops.fMaxLifetime	= 0.85f;
				ops.nFXFlags		= OBJPSFX_ALIGNROTATION;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_DEATHRAY_RING_2:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				VEC_COPY(ops.vOffset, m_vOffset);
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 1.5f;
				ops.fAddDelay		= 1.5f;
				ops.fRadius			= 1000.0f;
				ops.fPosRadius		= 2.5f;
				VEC_SET(ops.vRotations, 0.0f, 0.0f, GetRandom(-0.01f,0.01f));
				ops.nEmitType		= OBJPSFX_EMITPOWERRING;
				ops.nNumParticles	= 16;
				ops.fDensity		= 2.5f;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 196.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 85.0f;
				ops.fMaxVelocity	= 450.0f;
				ops.fMinLifetime	= 1.0f;
				ops.fMaxLifetime	= 1.0f;
				ops.nFXFlags		= OBJPSFX_ALIGNROTATION;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

			case	OBJFX_ELECTRIC_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fRampDownTime	= 0.25f;
				ops.nRampDownType	= OBJPSFX_RAMPALPHA;
				ops.fDuration		= 0.75f;
				ops.fAddDelay		= 0.75f;
				ops.fRadius			= 750.0f;
				ops.fPosRadius		= 15.0f;
				ops.fGravity		= -250.0f;
				ops.nEmitType		= OBJPSFX_EMITSPHERE;
				ops.nNumParticles	= 12;
				ops.fAlpha			= 0.5f;
				VEC_SET(ops.vMinColor, 0.0f, 0.0f, 196.0f);
				VEC_SET(ops.vMaxColor, 255.0f, 255.0f, 255.0f);
				ops.fMinVelocity	= 100.0f;
				ops.fMaxVelocity	= 300.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\Lensflare_1.dtx");
				break;
			}

#ifdef _ADD_ON

			case	OBJFX_GREEN_SMOKE:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 2000.0f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.nNumParticles	= 12;
				ops.fDensity		= 5.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 0.0f, 64.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 128.0f, 192.0f, 128.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_ALL;

				ops.szParticle = pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}

			case	OBJFX_BLOOD_TRAIL_1:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.025f;
				ops.fRadius			= 150.0f;
				ops.fPosRadius		= 5.0f;
				ops.fGravity		= -250.0f;
				ops.nEmitType		= OBJPSFX_EMITCOMETTAIL;
				ops.nNumParticles	= 2;
				ops.fDensity		= 10.0f;
				ops.fAlpha			= 1.0f;
				VEC_SET(ops.vMinColor, 32.0f, 0.0f, 0.0f);
				VEC_SET(ops.vMaxColor, 96.0f, 0.0f, 0.0f);
				ops.fMinVelocity	= 0.0f;
				ops.fMaxVelocity	= 25.0f;
				ops.fMinLifetime	= 0.25f;
				ops.fMaxLifetime	= 0.35f;
				ops.nFXFlags		= OBJPSFX_REMOVESTOPPED | OBJPSFX_RAMPDOWNSTOPPED | OBJPSFX_MOVINGSOURCE | OBJPSFX_MOVINGSTREAM;

				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\drop32_1.dtx");
				break;
			}

#endif

			default:
			{
				ops.hServerObj		= m_hObj;
				ops.hObj			= m_hObj;
				ops.fAddDelay		= 0.01f;
				ops.fRadius			= 2000.0f;
				ops.fGravity		= 25.0f;
				ops.nEmitType		= OBJPSFX_EMITLOCATION;
				ops.nNumParticles	= 6;
				ops.fDensity		= 1.0f;
				ops.fAlpha			= 0.25f;
				VEC_SET(ops.vMinColor, 96.0f, 96.0f, 96.0f);
				VEC_SET(ops.vMaxColor, 192.0f, 192.0f, 192.0f);
				ops.fMinVelocity	= 20.0f;
				ops.fMaxVelocity	= 50.0f;
				ops.fMinLifetime	= 0.5f;
				ops.fMaxLifetime	= 0.75f;
				ops.nFXFlags		= OBJPSFX_ALL;
				ops.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");
				break;
			}
		}
	}

	// Should we override the default flags?
	if(m_nFXFlags)	ops.nFXFlags = m_nFXFlags;

	// Scale values...
	if(m_nScaleFlags & OBJFX_SCALERADIUS)
		ops.fRadius *= m_fScale;

	if(m_nScaleFlags & OBJFX_SCALEPOSRADIUS)
		ops.fPosRadius *= m_fScale;

	if(m_nScaleFlags & OBJFX_SCALEOFFSET)
		{ VEC_MULSCALAR(ops.vOffset, ops.vOffset, m_fScale); }

	if(m_nScaleFlags & OBJFX_SCALERAMPTIMES)
	{
		ops.fRampUpTime *= m_fScale;
		ops.fRampDownTime *= m_fScale;
	}

	if(m_nScaleFlags & OBJFX_SCALEDURATION)
		ops.fDuration *= m_fScale;

	if(m_nScaleFlags & OBJFX_SCALENUMPARTICLES)
		ops.nNumParticles = (DDWORD)((float)ops.nNumParticles * m_fScale);

	if(m_nScaleFlags & OBJFX_SCALEDENSITY)
		ops.fDensity *= m_fScale;

	if(m_nScaleFlags & OBJFX_SCALEVELOCITY)
	{
		ops.fMinVelocity *= m_fScale;
		ops.fMaxVelocity *= m_fScale;
	}

	if(m_nScaleFlags & OBJFX_SCALELIFETIME)
	{
		ops.fMinLifetime *= m_fScale;
		ops.fMaxLifetime *= m_fScale;
	}

	if(m_nScaleFlags & OBJFX_SCALEROTATIONS)
		{ VEC_MULSCALAR(ops.vRotations, ops.vRotations, m_fScale); }

	if(m_nScaleFlags & OBJFX_SCALEGRAVITY)
		ops.fGravity *= m_fScale;

	if(m_nScaleFlags & OBJFX_SCALEADDDELAY)
		ops.fAddDelay *= m_fScale;

	// Create the special effect
	psfxMgr->CreateSFX(SFX_OBJECTPARTICLES_ID, &ops, DFALSE, this);
	return DTRUE;
}