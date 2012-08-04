// ----------------------------------------------------------------------- //
//
// MODULE  : WarpGateFX.cpp
//
// PURPOSE : Warp gate (spawner) special FX - Implementation
//
// CREATED : 8/15/98
//
// ----------------------------------------------------------------------- //

#include "WarpGateFX.h"
#include "SFXMsgIds.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "BloodClientShell.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWarpGateSpriteFX::Init
//
//	PURPOSE:	Init the sprite
//
// ----------------------------------------------------------------------- //

DBOOL CWarpGateSpriteFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	WARPGATESPRITECS* pWGS = (WARPGATESPRITECS*)psfxCreateStruct;

	m_hObj			= pWGS->hObj;
	m_fRampUpTime	= pWGS->fRampUpTime;
	m_fRampDownTime	= pWGS->fRampDownTime;
	m_fMinScale		= pWGS->fMinScale;
	m_fMaxScale		= pWGS->fMaxScale;
	m_fAlpha		= pWGS->fAlpha;
	m_nRampUpType	= pWGS->nRampUpType;
	m_nRampDownType	= pWGS->nRampDownType;
	m_bAlign		= pWGS->bAlign;
	m_szSprite		= pWGS->szSprite;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWarpGateSpriteFX::CreateObject
//
//	PURPOSE:	Create sprite object
//
// ----------------------------------------------------------------------- //

DBOOL CWarpGateSpriteFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;
	m_pClientDE = pClientDE;

	ObjectCreateStruct	ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_SPRITE;
	ocStruct.m_Flags = FLAG_VISIBLE | FLAG_SPRITECHROMAKEY;

	m_pClientDE->GetObjectPos(m_hObj, &ocStruct.m_Pos);

	if(m_bAlign)
	{
		m_pClientDE->GetObjectRotation(m_hObj, &ocStruct.m_Rotation);
		ocStruct.m_Flags |= FLAG_ROTATEABLESPRITE;
	}

	if(m_szSprite)
		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)pClientDE->GetStringData(m_szSprite));
	else
		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)"Sprites\\rift.spr");

	m_hObject = pClientDE->CreateObject(&ocStruct);
	pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, m_fAlpha);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWarpGateSpriteFX::Update
//
//	PURPOSE:	Update the sprite
//
// ----------------------------------------------------------------------- //

DBOOL CWarpGateSpriteFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hObj)	return DFALSE;
	DFLOAT	fTime = m_pClientDE->GetTime() - m_fTriggerTime;

	// Update the position and rotation of the object
	DVector		vPos;
	m_pClientDE->GetObjectPos(m_hObj, &vPos);
	m_pClientDE->SetObjectPos(m_hObject, &vPos);

	if(m_bAlign)
	{
		DRotation	rRot;
		m_pClientDE->GetObjectRotation(m_hObj, &rRot);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}

	// Get the user flags to see if the FX should be on or off
	DDWORD dwFlags;
	m_pClientDE->GetObjectUserFlags(m_hObj, &dwFlags);

	if(dwFlags & USRFLG_VISIBLE)
	{

		if(m_bFirstUpdate)
		{
			m_fTriggerTime = m_pClientDE->GetTime();
			fTime = 0.0f;
			m_bFirstUpdate = 0;
			m_bStarted = 1;
		}

		if(fTime <= m_fRampUpTime)
		{
			// Scale the sprite
			if(m_nRampUpType & 0x0001)
			{
				DFLOAT	scale = m_fMinScale + ((m_fMaxScale - m_fMinScale) * (fTime / m_fRampUpTime));
				VEC_SET(m_vScale, scale, scale, 0.0f);
			}
			else
				{ VEC_SET(m_vScale, m_fMaxScale, m_fMaxScale, 0.0f); }

			m_pClientDE->SetObjectScale(m_hObject, &m_vScale);
			m_pClientDE->CPrint("Scale: %f %f %f", m_vScale.x, m_vScale.y, m_vScale.z);

			// Fade in the sprite
			DFLOAT		r, g, b, a;
			m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
			
			if(m_nRampUpType & 0x0002)
			{
				a = m_fAlpha * (fTime / m_fRampUpTime);
				if(a > m_fAlpha)	a = m_fAlpha;
			}

			m_pClientDE->SetObjectColor(m_hObject, r, g, b, a);
			m_pClientDE->CPrint("Alpha: %f %f %f %f", m_fAlpha, fTime, m_fRampUpTime, a);
		}
		else
		{
			VEC_SET(m_vScale, m_fMaxScale, m_fMaxScale, 0.0f);
			m_pClientDE->SetObjectScale(m_hObject, &m_vScale);

			DFLOAT		r, g, b, a;
			m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
			m_pClientDE->SetObjectColor(m_hObject, r, g, b, m_fAlpha);
		}
	}
	else if(m_bStarted)
	{
		if(!m_bFirstUpdate)
		{
			m_fTriggerTime = m_pClientDE->GetTime();
			fTime = 0.0f;
			m_bFirstUpdate = 1;
		}

		if(fTime <= m_fRampDownTime)
		{
			// Scale the sprite
			if(m_nRampDownType & 0x0001)
			{
				DFLOAT	scale = m_fMaxScale - ((m_fMaxScale - m_fMinScale) * (fTime / m_fRampUpTime));
				VEC_SET(m_vScale, scale, scale, 0.0f);
			}
			else
				{ VEC_SET(m_vScale, m_fMaxScale, m_fMaxScale, 0.0f); }

			m_pClientDE->SetObjectScale(m_hObject, &m_vScale);
			m_pClientDE->CPrint("Scale: %f %f %f", m_vScale.x, m_vScale.y, m_vScale.z);

			// Fade out the sprite
			DFLOAT		r, g, b, a;
			m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
			
			if(m_nRampDownType & 0x0002)
			{
				a = m_fAlpha - (m_fAlpha * (fTime / m_fRampUpTime));
				if(a < 0.0f)	a = 0.0f;
			}

			m_pClientDE->SetObjectColor(m_hObject, r, g, b, a);
			m_pClientDE->CPrint("Alpha: %f", a);
		}
		else
			m_bStarted = 0;
	}
	else
	{
		VEC_SET(m_vScale, 0.0f, 0.0f, 0.0f);
		m_pClientDE->SetObjectScale(m_hObject, &m_vScale);
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWarpGateParticleFX::Init
//
//	PURPOSE:	Init the system
//
// ----------------------------------------------------------------------- //

DBOOL CWarpGateParticleFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CBaseParticleSystemFX::Init(psfxCreateStruct);

	WARPGATEPARTICLECS* pWGPS = (WARPGATEPARTICLECS*)psfxCreateStruct;

	m_hObj				= pWGPS->hObj;
	m_fRampUpTime		= pWGPS->fRampUpTime;
	m_fRampDownTime		= pWGPS->fRampDownTime;

	m_fRadius			= pWGPS->fSystemRadius;
	m_fPosRadius		= pWGPS->fPosRadius;
	VEC_COPY(m_vOffset, pWGPS->vOffset);
	VEC_COPY(m_vRotations, pWGPS->vRotations);
	m_fMinVelocity		= pWGPS->fMinVelocity;
	m_fMaxVelocity		= pWGPS->fMaxVelocity;
	m_nNumParticles		= pWGPS->nNumParticles;
	m_nEmitType			= pWGPS->nEmitType;
	VEC_COPY(m_vColor1, pWGPS->vMinColor);
	VEC_COPY(m_vColor2, pWGPS->vMaxColor);
	m_fAlpha			= pWGPS->fAlpha;
	m_fMinLifetime		= pWGPS->fMinLifetime;
	m_fMaxLifetime		= pWGPS->fMaxLifetime;
	m_fAddDelay			= pWGPS->fAddDelay;
	m_fGravity			= pWGPS->fGravity;
	m_nRampUpType		= pWGPS->nRampUpType;
	m_nRampDownType		= pWGPS->nRampDownType;
	m_bAlign			= pWGPS->bAlign;
	m_szParticle		= pWGPS->szParticle;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWarpGateParticleFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CWarpGateParticleFX::CreateObject(CClientDE *pClientDE)
{
	if(!pClientDE) return DFALSE;

	if(m_szParticle)
		m_pTextureName = pClientDE->GetStringData(m_szParticle);
	else
		m_pTextureName = "SpriteTextures\\smoke64_2.dtx";

	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	// Update the position and rotation of the object
	DVector		vPos;
	m_pClientDE->GetObjectPos(m_hObj, &vPos);
	m_pClientDE->SetObjectPos(m_hObject, &vPos);

	if(m_bAlign)
	{
		DRotation	rRot;
		m_pClientDE->GetObjectRotation(m_hObj, &rRot);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}
	else
	{
		DRotation	rRot;
		ROT_INIT(rRot);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWarpGateParticleFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CWarpGateParticleFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hObj) return DFALSE;
	DFLOAT		fTime = m_pClientDE->GetTime() - m_fTriggerTime;

	// Update the position of the object
	DVector		vPos;
	m_pClientDE->GetObjectPos(m_hObj, &vPos);
	VEC_ADD(vPos, vPos, m_vOffset);
	m_pClientDE->SetObjectPos(m_hObject, &vPos);

	// Rotate the object if it has rotation values
	DRotation	rRot;
	m_pClientDE->GetObjectRotation(m_hObject, &rRot);
	if(m_vRotations.x)	m_pClientDE->EulerRotateX(&rRot, m_vRotations.x * MATH_PI);
	if(m_vRotations.y)	m_pClientDE->EulerRotateY(&rRot, m_vRotations.y * MATH_PI);
	if(m_vRotations.z)	m_pClientDE->EulerRotateZ(&rRot, m_vRotations.z * MATH_PI);
	m_pClientDE->SetObjectRotation(m_hObject, &rRot);

	// Get the user flags to see if the FX should be on or off
	DDWORD dwFlags;
	m_pClientDE->GetObjectUserFlags(m_hObj, &dwFlags);

	if(dwFlags & USRFLG_VISIBLE)
	{
		if(m_bFirstUpdate)
		{
			m_fTriggerTime = m_pClientDE->GetTime();
			fTime = 0.0f;
			m_bFirstUpdate = 0;
			m_bStarted = 1;
		}

		if(fTime <= m_fRampUpTime)
		{
			// Fade in the particles
			DFLOAT		r, g, b, a;
			m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
			
			if(m_nRampUpType & 0x0001)
			{
				a = m_fAlpha * (fTime / m_fRampUpTime);
				if(a > m_fAlpha)	a = m_fAlpha;
			}

			m_pClientDE->SetObjectColor(m_hObject, r, g, b, a);
		}

		// Add more particles if it's time to
		if(fTime >= m_fLastAddTime + m_fAddDelay)
		{
			m_fLastAddTime = fTime;
			AddParticles();
		}
	}
	else if(m_bStarted)
	{
		if(!m_bFirstUpdate)
		{
			m_fTriggerTime = m_pClientDE->GetTime();
			fTime = 0.0f;
			m_bFirstUpdate = 1;
		}

		if(fTime <= m_fRampDownTime)
		{
			// Fade out the particles
			DFLOAT		r, g, b, a;
			m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
			
			if(m_nRampDownType & 0x0001)
			{
				a = m_fAlpha - (m_fAlpha * (fTime / m_fRampUpTime));
				if(a < 0.0f)	a = 0.0f;
			}

			m_pClientDE->SetObjectColor(m_hObject, r, g, b, a);
		}
		else
			m_bStarted = 0;

		// Add more particles if it's time to
		if(fTime >= m_fLastAddTime + m_fAddDelay)
		{
			m_fLastAddTime = fTime;
			AddParticles();
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //

void CWarpGateParticleFX::AddParticles()
{
	// Add the particles to the system
	DDWORD		i;
	DFLOAT		life;
	DVector		start, vel, color, vUp, vU, vR, vF;
	DRotation	rRot;

	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	if(m_bAlign)	m_pClientDE->GetObjectRotation(m_hObject, &rRot);
		else		m_pClientDE->AlignRotation(&rRot, &vUp, &vUp);

	for(i = 0; i < m_nNumParticles; i++)
	{
		m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		if((m_nEmitType == 2) || (m_nEmitType == 3))
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

		if((m_nEmitType == 1) || (m_nEmitType == 3))
			{ VEC_MULSCALAR(start, start, GetRandom(0.0f, m_fPosRadius)); }
		else
			{ VEC_MULSCALAR(start, start, m_fPosRadius); }

		GetRandomColorInRange(color);
		life = GetRandom(m_fMinLifetime, m_fMaxLifetime);

		m_pClientDE->AddParticle(m_hObject, &start, &vel, &color, life);
	}
}