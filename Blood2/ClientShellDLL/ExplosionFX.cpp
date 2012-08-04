// ----------------------------------------------------------------------- //
//
// MODULE  : ExplosionFX.cpp
//
// PURPOSE : Explosion special FX - Implementation
//
// CREATED : 7/1/98
//
// ----------------------------------------------------------------------- //

#include "ExplosionFX.h"
#include "SFXMsgIds.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "BloodClientShell.h"
#include "ObjectFX.h"
#include <mbstring.h>

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionModelFX::Init
//
//	PURPOSE:	Init the splash
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionModelFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	EXPLOSIONMODELCS* pEMCS = (EXPLOSIONMODELCS*)psfxCreateStruct;

	VEC_COPY(m_vPos, pEMCS->vPos);
	VEC_COPY(m_vNormal, pEMCS->vNormal);
	VEC_COPY(m_vScale1, pEMCS->vScale1);
	VEC_COPY(m_vScale2, pEMCS->vScale2);
	VEC_COPY(m_vRotations, pEMCS->vRotations);

	m_fDuration			= pEMCS->fDuration;
	m_fInitAlpha		= pEMCS->fAlpha;
	m_bWaveForm			= pEMCS->bWaveForm;
	m_bFadeType			= pEMCS->bFadeType;
	m_bRandomRot		= pEMCS->bRandomRot;

	if( m_szModel )
		g_pClientDE->FreeString( m_szModel );
	m_szModel			= g_pClientDE->CopyString( pEMCS->szModel );
	if( m_szSkin )
		g_pClientDE->FreeString( m_szSkin );
	m_szSkin			= g_pClientDE->CopyString( pEMCS->szSkin );

	if((m_vScale1.x != m_vScale2.x) || (m_vScale1.y != m_vScale2.y) || (m_vScale1.z != m_vScale2.z))
		m_bScale = 1;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionModelFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionModelFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;
	m_pClientDE = pClientDE;

	ObjectCreateStruct	ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_MODEL;
	ocStruct.m_Flags = FLAG_VISIBLE;
	VEC_COPY(ocStruct.m_Pos, m_vPos);

	if(m_bRandomRot == 1)
	{
		DFLOAT	fPitch, fYaw, fRoll;
		fPitch	= GetRandom(-MATH_PI, MATH_PI);
		fYaw	= GetRandom(-MATH_PI, MATH_PI);
		fRoll	= GetRandom(-MATH_PI, MATH_PI);
		pClientDE->SetupEuler(&ocStruct.m_Rotation, fPitch, fYaw, fRoll);
	}
	else if(m_bRandomRot == 2)
	{
		DVector		u, r, f;
		VEC_SET(u, 0.0f, 1.0f, 0.0f);

		//pClientDE->CPrint("Normal: %f %f %f", m_vNormal.x, m_vNormal.y, m_vNormal.z);

		pClientDE->AlignRotation(&ocStruct.m_Rotation, &m_vNormal, &u);
		pClientDE->GetRotationVectors(&ocStruct.m_Rotation, &u, &r, &f);
		pClientDE->RotateAroundAxis(&ocStruct.m_Rotation, &f, GetRandom(0.0f, MATH_PI));
	}

	if(m_szModel)	_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)pClientDE->GetStringData(m_szModel));
		else		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)"Models\\Explosions\\exp_sphere.abc");

	if(m_szSkin)	_mbscpy((unsigned char*)ocStruct.m_SkinName, (const unsigned char*)pClientDE->GetStringData(m_szSkin));
		else		_mbscpy((unsigned char*)ocStruct.m_SkinName, (const unsigned char*)"Skins\\Explosions\\Explosion_1.dtx");

	m_hObject = pClientDE->CreateObject(&ocStruct);

	// Gouraud shade and make full bright...
	DDWORD dwFlags = pClientDE->GetObjectFlags(m_hObject);
	pClientDE->SetObjectFlags(m_hObject, dwFlags | FLAG_MODELGOURAUDSHADE | FLAG_NOLIGHT);
	pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, m_fInitAlpha);
	pClientDE->SetObjectScale(m_hObject, &m_vScale1);

	m_fStartTime = pClientDE->GetTime();

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionModelFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionModelFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;
	DFLOAT		fTime = m_pClientDE->GetTime() - m_fStartTime;
	DFLOAT		fRatio = fTime / m_fDuration;

	if(m_bScale)
	{
		DFLOAT		fTemp = fRatio;
		DVector		vScale;
		VEC_SUB(vScale, m_vScale2, m_vScale1);

		switch(m_bWaveForm)
		{
			case	1:	fTemp = (DFLOAT)pow(fRatio, 0.5);	break;
			case	2:	fTemp = (DFLOAT)pow(fRatio, 2.0);	break;
			case	3:	if(fTime < (m_fDuration / 2))
							fTemp = fRatio * 2.0f;
						else
							fTemp = (1.0f - fRatio) * 2.0f;
						break;
			case	4:	if(fTime < (m_fDuration / 2))
							fTemp = (DFLOAT)pow(fRatio * 2.0f, 0.5);
						else
							fTemp = (DFLOAT)pow((1.0f - fRatio) * 2.0f, 0.5);
						break;
			case	5:	if(fTime < (m_fDuration / 2))
							fTemp = (DFLOAT)pow(fRatio * 2.0f, 2.0);
						else
							fTemp = (DFLOAT)pow((1.0f - fRatio) * 2.0f, 2.0);
						break;
		}

		VEC_MULSCALAR(vScale, vScale, fTemp);
		VEC_ADD(vScale, vScale, m_vScale1);
		m_pClientDE->SetObjectScale(m_hObject, &vScale);
	}

	if(m_vRotations.x || m_vRotations.y || m_vRotations.z)
	{
		DRotation	rot;
		DVector		u, r, f;

		m_pClientDE->GetObjectRotation(m_hObject, &rot);
		m_pClientDE->GetRotationVectors(&rot, &u, &r, &f);
		m_pClientDE->RotateAroundAxis(&rot, &u, m_vRotations.x);
		m_pClientDE->RotateAroundAxis(&rot, &r, m_vRotations.y);
		m_pClientDE->RotateAroundAxis(&rot, &f, m_vRotations.z);
		m_pClientDE->SetObjectRotation(m_hObject, &rot);
	}

	if(m_bFadeType)
	{
		DFLOAT		alpha;

		switch(m_bFadeType)
		{
			case	1:	alpha = m_fInitAlpha * (1.0f - fRatio);					break;	// InitAlpha to zero
			case	2:	alpha = 1.0f - ((1.0f - m_fInitAlpha) * fRatio);		break;	// Solid to InitAlpha
			case	3:	alpha = m_fInitAlpha * fRatio;							break;	// Zero to InitAlpha
			case	4:	alpha = m_fInitAlpha + ((1.0f - m_fInitAlpha) * fRatio);break;	// InitAlpha to Solid
			case	5:	if(fTime < (m_fDuration / 2))
							alpha = m_fInitAlpha * fRatio * 2.0f;
						else
							alpha = m_fInitAlpha * 2.0f * (1.0f - fRatio);		break;
		}

		m_pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, alpha);
	}

	return (fTime < m_fDuration);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionSpriteFX::Init
//
//	PURPOSE:	Init the splash
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionSpriteFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	EXPLOSIONSPRITECS* pESCS = (EXPLOSIONSPRITECS*)psfxCreateStruct;

	VEC_COPY(m_vPos, pESCS->vPos);
	VEC_COPY(m_vNormal, pESCS->vNormal);
	VEC_COPY(m_vScale1, pESCS->vScale1);
	VEC_COPY(m_vScale2, pESCS->vScale2);

	m_fDuration			= pESCS->fDuration;
	m_fInitAlpha		= pESCS->fAlpha;
	m_bWaveForm			= pESCS->bWaveForm;
	m_bFadeType			= pESCS->bFadeType;
	m_bAlign			= pESCS->bAlign;

	if( m_szSprite )
		g_pClientDE->FreeString( m_szSprite );
	m_szSprite			= g_pClientDE->CopyString( pESCS->szSprite );

	if((m_vScale1.x != m_vScale2.x) || (m_vScale1.y != m_vScale2.y) || (m_vScale1.z != m_vScale2.z))
		m_bScale = 1;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionSpriteFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionSpriteFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;
	m_pClientDE = pClientDE;

	ObjectCreateStruct	ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_SPRITE;
	ocStruct.m_Flags = FLAG_VISIBLE | FLAG_SPRITECHROMAKEY;
	VEC_COPY(ocStruct.m_Pos, m_vPos);

	if(m_bAlign)
	{
		DVector	vUp;
		VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
		m_pClientDE->AlignRotation(&(ocStruct.m_Rotation), &m_vNormal, &vUp);
		ocStruct.m_Flags |= FLAG_ROTATEABLESPRITE;

		if(m_bAlign == 2)
			m_pClientDE->RotateAroundAxis(&(ocStruct.m_Rotation), &m_vNormal, GetRandom(0.0f, 2 * MATH_PI));
	}

	if(m_szSprite)
		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)pClientDE->GetStringData(m_szSprite));
	else
		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)"Sprites\\Explosn.spr");

	m_hObject = pClientDE->CreateObject(&ocStruct);
	pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, m_fInitAlpha);
	pClientDE->SetObjectScale(m_hObject, &m_vScale1);

	m_fStartTime = pClientDE->GetTime();

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionSpriteFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionSpriteFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;
	DFLOAT		fTime = m_pClientDE->GetTime() - m_fStartTime;
	DFLOAT		fRatio = fTime / m_fDuration;

	if(m_bScale)
	{
		DFLOAT		fTemp = fRatio;
		DVector		vScale;
		VEC_SUB(vScale, m_vScale2, m_vScale1);

		if(m_bWaveForm == 1)		fTemp = (DFLOAT)pow(fRatio, 0.5);
		else if(m_bWaveForm == 2)	fTemp = (DFLOAT)pow(fRatio, 2.0);

		VEC_MULSCALAR(vScale, vScale, fTemp);
		VEC_ADD(vScale, vScale, m_vScale1);
		m_pClientDE->SetObjectScale(m_hObject, &vScale);
	}

	if(m_bFadeType)
	{
		DFLOAT		alpha;

		switch(m_bFadeType)
		{
			case	1:	alpha = m_fInitAlpha * (1.0f - fRatio);					break;	// InitAlpha to zero
			case	2:	alpha = 1.0f - ((1.0f - m_fInitAlpha) * fRatio);		break;	// Solid to InitAlpha
			case	3:	alpha = m_fInitAlpha * fRatio;							break;	// Zero to InitAlpha
			case	4:	alpha = m_fInitAlpha + ((1.0f - m_fInitAlpha) * fRatio);break;	// InitAlpha to Solid
			case	5:	if(fTime < (m_fDuration / 2))
							alpha = m_fInitAlpha * fRatio * 2.0f;
						else
							alpha = m_fInitAlpha * 2.0f * (1.0f - fRatio);		break;
		}

		m_pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, alpha);
	}

	return (fTime < m_fDuration);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionRingFX::Init
//
//	PURPOSE:	Init the splash
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionRingFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CBaseParticleSystemFX::Init(psfxCreateStruct);

	EXPLOSIONRINGCS* pERCS = (EXPLOSIONRINGCS*)psfxCreateStruct;

	VEC_COPY(m_vPos, pERCS->vPos);
	VEC_COPY(m_vNormal, pERCS->vNormal);

	VEC_COPY(m_vColor, pERCS->vColor);
	m_fRadius			= pERCS->fRadius;
	m_fPosRadius		= pERCS->fPosRadius;
	m_fVelocity			= pERCS->fVelocity;
	m_fGravity			= pERCS->fGravity;
	m_nParticles		= pERCS->nParticles;

	m_fDuration			= pERCS->fDuration;
	m_fRotation			= pERCS->fRotation;
	m_fInitAlpha		= pERCS->fAlpha;
	m_fDelay			= pERCS->fDelay;
	m_bFadeType			= pERCS->bFadeType;
	m_bRotateType		= pERCS->bRotateType;
	m_bAlign			= pERCS->bAlign;

	if( m_szParticle )
		g_pClientDE->FreeString( m_szParticle );
	m_szParticle		= g_pClientDE->CopyString( pERCS->szParticle );

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionRingFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionRingFX::CreateObject(CClientDE *pClientDE)
{
	if(!pClientDE ) return DFALSE;

	if(m_szParticle)
		m_pTextureName = pClientDE->GetStringData(m_szParticle);
	else
		m_pTextureName = "SpriteTextures\\smoke64_2.dtx";

	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if(bRet)
	{
		if(m_fDelay)
			pClientDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, 0.0f);
		else
		{
			pClientDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, m_fInitAlpha);
			AddParticles();
		}

		m_fStartTime = m_pClientDE->GetTime();
	}
	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionRingFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionRingFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;
	DFLOAT		fTime = m_pClientDE->GetTime() - m_fStartTime;
	DFLOAT		fRatio = fTime / m_fDuration;

	if(m_fDelay)
	{
		if(fTime >= m_fDelay)
		{
			if(!m_bFadeType)
				m_pClientDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, m_fInitAlpha);
			m_fStartTime = m_pClientDE->GetTime();
			m_fDelay = 0.0f;
			AddParticles();
		}
		return	DTRUE;
	}

	if(m_bRotateType)
	{
		DVector		vUp;
		DRotation	rRot;
		DFLOAT		rotate = 0.0f;

		VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

		switch(m_bRotateType)
		{
			case	1:	rotate = m_fRotation;									break;	// Constant rotation
			case	2:	rotate = m_fRotation * (1.0f - fRatio);					break;	// Slow down from initial
			case	3:	rotate = 1.0f - ((1.0f - m_fRotation) * fRatio);		break;	// Complete rotation to destination
			case	4:	rotate = m_fRotation * fRatio;							break;	// No rotation to destination
			case	5:	rotate = m_fRotation + ((1.0f - m_fRotation) * fRatio);	break;	// Speed up from initial
		}

		rotate *= MATH_PI;

		m_pClientDE->GetObjectRotation(m_hObject, &rRot);

		if(m_bAlign)	m_pClientDE->RotateAroundAxis(&rRot, &m_vNormal, rotate);
			else		m_pClientDE->RotateAroundAxis(&rRot, &vUp, rotate);

		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}

	if(m_bFadeType)
	{
		DFLOAT		alpha;

		switch(m_bFadeType)
		{
			case	1:	alpha = m_fInitAlpha * (1.0f - fRatio);					break;	// InitAlpha to zero
			case	2:	alpha = 1.0f - ((1.0f - m_fInitAlpha) * fRatio);		break;	// Solid to InitAlpha
			case	3:	alpha = m_fInitAlpha * fRatio;							break;	// Zero to InitAlpha
			case	4:	alpha = m_fInitAlpha + ((1.0f - m_fInitAlpha) * fRatio);break;	// InitAlpha to Solid
			case	5:	if(fTime < (m_fDuration / 2))
							alpha = m_fInitAlpha * fRatio * 2.0f;
						else
							alpha = m_fInitAlpha * 2.0f * (1.0f - fRatio);		break;

		}

		m_pClientDE->SetObjectColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z, alpha);
	}

	return (fTime < m_fDuration);
}

// ----------------------------------------------------------------------- //

void CExplosionRingFX::AddParticles()
{
	// Add the particles to the system
	DFLOAT		rotValue;
	DVector		start, vel, tempColor, vUp, vU, vR, vF;
	DRotation	rRot;

	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
	VEC_SET(tempColor, 255.0f, 255.0f, 255.0f);
	rotValue = 2 * MATH_PI / m_nParticles;

	if(m_bAlign)	m_pClientDE->AlignRotation(&rRot, &m_vNormal, &vUp);
		else		m_pClientDE->AlignRotation(&rRot, &vUp, &vUp);

	for(DDWORD i = 0; i < m_nParticles; i++)
	{
		m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
		m_pClientDE->RotateAroundAxis(&rRot, &vF, rotValue);

		VEC_NORM(vR);
		VEC_COPY(start, vR);
		VEC_COPY(vel, vR);
		VEC_MULSCALAR(start, start, m_fPosRadius);
		VEC_MULSCALAR(vel, vel, m_fVelocity);

		m_pClientDE->AddParticle(m_hObject, &start, &vel, &tempColor, m_fDuration);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFlameFX::Init
//
//	PURPOSE:	Init the splash
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFlameFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CBaseParticleSystemFX::Init(psfxCreateStruct);

	EXPLOSIONFLAMECS* pEFCS = (EXPLOSIONFLAMECS*)psfxCreateStruct;

	VEC_COPY(m_vPos, pEFCS->vPos);
	VEC_COPY(m_vNormal, pEFCS->vNormal);

	VEC_COPY(m_vColor1, pEFCS->vColor1);
	VEC_COPY(m_vColor2, pEFCS->vColor2);
	VEC_COPY(m_vColor3, pEFCS->vColor3);

	VEC_COPY(m_vLifeTimes, pEFCS->vLifeTimes);
	VEC_COPY(m_vLifeOffsets, pEFCS->vLifeOffsets);

	m_fRampUp = pEFCS->vFXTimes.x;
	m_fDuration = pEFCS->vFXTimes.y;
	m_fRampDown = pEFCS->vFXTimes.z;

	m_nParticles		= pEFCS->nParticles;
	m_fRadius			= pEFCS->fRadius;
	m_fPosRadius		= pEFCS->fPosRadius;
	m_fGravity			= pEFCS->fGravity;
	m_fVelocity			= pEFCS->fVelocity;
	m_fDelay			= pEFCS->fDelay;
	m_fAlpha			= pEFCS->fAlpha;
	m_bFadeType			= pEFCS->bFadeType;
	m_bRampFlags		= pEFCS->bRampFlags;

	m_szParticle		= pEFCS->szParticle;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFlameFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFlameFX::CreateObject(CClientDE *pClientDE)
{
	if(!pClientDE ) return DFALSE;

	if(m_szParticle)	m_pTextureName = pClientDE->GetStringData(m_szParticle);
		else			m_pTextureName = "SpriteTextures\\drop32_1.dtx";

	m_pClientDE = pClientDE;
	DBOOL bRet = CBaseParticleSystemFX::CreateObject(pClientDE);

	if(bRet)
	{
		if(!m_bFadeType)
			pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, m_fAlpha);

		m_fStartTime = m_pClientDE->GetTime();
		m_fAddTime = m_fDelay;
		AddParticles();
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFlameFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFlameFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;
	DFLOAT		fTime = m_pClientDE->GetTime() - m_fStartTime;
	DFLOAT		fRatio = fTime / m_fDuration;

	if(fTime >= m_fAddTime)
	{
		AddParticles();
		m_fAddTime += m_fDelay;
	}

	if(m_bFadeType)
	{
		DFLOAT		alpha;

		switch(m_bFadeType)
		{
			case	1:	alpha = m_fAlpha * (1.0f - fRatio);					break;	// InitAlpha to zero
			case	2:	alpha = 1.0f - ((1.0f - m_fAlpha) * fRatio);		break;	// Solid to InitAlpha
			case	3:	alpha = m_fAlpha * fRatio;							break;	// Zero to InitAlpha
			case	4:	alpha = m_fAlpha + ((1.0f - m_fAlpha) * fRatio);	break;	// InitAlpha to Solid
			case	5:	if(fTime < (m_fDuration / 2))
							alpha = m_fAlpha * fRatio * 2.0f;
						else
							alpha = m_fAlpha * 2.0f * (1.0f - fRatio);		break;

		}

		m_pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, alpha);
	}

	return (fTime < m_fDuration);
}

// ----------------------------------------------------------------------- //

void CExplosionFlameFX::AddParticles()
{
	// Add the particles to the system
	DVector		minOffset, maxOffset, minVel, maxVel;
	DFLOAT		minLifetime = 0, maxLifetime = 0;
	DFLOAT		radius1 = m_fPosRadius / 3, radius2 = radius1 * 2;

	VEC_SET(minVel, -m_fVelocity, -m_fVelocity, -m_fVelocity);
	VEC_SET(maxVel, m_fVelocity, m_fVelocity, m_fVelocity);

	VEC_SET(minOffset, -radius1, -radius1, -radius1);
	VEC_SET(maxOffset, radius1, radius1, radius1);
	minLifetime = m_vLifeTimes.x - m_vLifeOffsets.x;
	maxLifetime = m_vLifeTimes.x + m_vLifeOffsets.x;

	m_pClientDE->AddParticles(m_hObject, m_nParticles, &minOffset, &maxOffset, &minVel, &maxVel,
								&m_vColor1, &m_vColor1, minLifetime, maxLifetime);

	VEC_SET(minOffset, -radius2, -radius2, -radius2);
	VEC_SET(maxOffset, radius2, radius2, radius2);
	minLifetime = m_vLifeTimes.y - m_vLifeOffsets.y;
	maxLifetime = m_vLifeTimes.y + m_vLifeOffsets.y;

	m_pClientDE->AddParticles(m_hObject, m_nParticles * 2, &minOffset, &maxOffset, &minVel, &maxVel,
								&m_vColor2, &m_vColor2, minLifetime, maxLifetime);

	VEC_SET(minOffset, -m_fPosRadius, -m_fPosRadius, -m_fPosRadius);
	VEC_SET(maxOffset, m_fPosRadius, m_fPosRadius, m_fPosRadius);
	minLifetime = m_vLifeTimes.z - m_vLifeOffsets.z;
	maxLifetime = m_vLifeTimes.z + m_vLifeOffsets.z;

	m_pClientDE->AddParticles(m_hObject, m_nParticles * 3, &minOffset, &maxOffset, &minVel, &maxVel,
								&m_vColor3, &m_vColor3, minLifetime, maxLifetime);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionWaveFX::Init
//
//	PURPOSE:	Init the splash
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionWaveFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	EXPLOSIONWAVECS* pEWCS = (EXPLOSIONWAVECS*)psfxCreateStruct;

	VEC_COPY(m_vPos, pEWCS->vPos);
	VEC_COPY(m_vNormal, pEWCS->vNormal);
	VEC_COPY(m_vScale1, pEWCS->vScale1);
	VEC_COPY(m_vScale2, pEWCS->vScale2);

	m_fDuration			= pEWCS->fDuration;
	m_fInitAlpha		= pEWCS->fAlpha;
	m_fDelay			= pEWCS->fDelay;
	m_bWaveForm			= pEWCS->bWaveForm;
	m_bFadeType			= pEWCS->bFadeType;
	m_bAlign			= pEWCS->bAlign;

	m_szWave			= pEWCS->szWave;

	if((m_vScale1.x != m_vScale2.x) || (m_vScale1.y != m_vScale2.y) || (m_vScale1.z != m_vScale2.z))
		m_bScale = 1;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionWaveFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionWaveFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;
	m_pClientDE = pClientDE;

	ObjectCreateStruct	ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_SPRITE;
	ocStruct.m_Flags = FLAG_VISIBLE | FLAG_SPRITECHROMAKEY | FLAG_ROTATEABLESPRITE;
	VEC_COPY(ocStruct.m_Pos, m_vPos);

	DVector	vUp;
	VEC_SET(vUp, 0.0f, 1.0f, 0.0f);

	if(m_bAlign)
		m_pClientDE->AlignRotation(&(ocStruct.m_Rotation), &m_vNormal, &vUp);
	else
		m_pClientDE->AlignRotation(&(ocStruct.m_Rotation), &vUp, &vUp);

	if(m_szWave)
		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)pClientDE->GetStringData(m_szWave));
	else
		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)"Sprites\\Shockring.spr");

	m_hObject = pClientDE->CreateObject(&ocStruct);
	pClientDE->SetObjectScale(m_hObject, &m_vScale1);

	if(m_fDelay)
		pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, 0.0f);
	else
		pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, m_fInitAlpha);

	m_fStartTime = pClientDE->GetTime();

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionWaveFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionWaveFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;
	DFLOAT		fTime = m_pClientDE->GetTime() - m_fStartTime;
	DFLOAT		fRatio = fTime / m_fDuration;

	if(m_fDelay)
	{
		if(fTime >= m_fDelay)
		{
			m_pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, m_fInitAlpha);
			m_fStartTime = m_pClientDE->GetTime();
			m_fDelay = 0.0f;
		}
		return	DTRUE;
	}

	if(m_bScale)
	{
		DFLOAT		fTemp = fRatio;
		DVector		vScale;
		VEC_SUB(vScale, m_vScale2, m_vScale1);

		if(m_bWaveForm == 1)		fTemp = (DFLOAT)pow(fRatio, 0.5);
		else if(m_bWaveForm == 2)	fTemp = (DFLOAT)pow(fRatio, 2.0);

		VEC_MULSCALAR(vScale, vScale, fTemp);
		VEC_ADD(vScale, vScale, m_vScale1);
		m_pClientDE->SetObjectScale(m_hObject, &vScale);
	}

	if(m_bFadeType)
	{
		DFLOAT		alpha;

		switch(m_bFadeType)
		{
			case	1:	alpha = m_fInitAlpha * (1.0f - fRatio);					break;	// InitAlpha to zero
			case	2:	alpha = 1.0f - ((1.0f - m_fInitAlpha) * fRatio);		break;	// Solid to InitAlpha
			case	3:	alpha = m_fInitAlpha * fRatio;							break;	// Zero to InitAlpha
			case	4:	alpha = m_fInitAlpha + ((1.0f - m_fInitAlpha) * fRatio);break;	// InitAlpha to Solid
		}

		m_pClientDE->SetObjectColor(m_hObject, 1.0f, 1.0f, 1.0f, alpha);
	}

	return (fTime < m_fDuration);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionLightFX::Init
//
//	PURPOSE:	Init the splash
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionLightFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	EXPLOSIONLIGHTCS* pELCS = (EXPLOSIONLIGHTCS*)psfxCreateStruct;

	VEC_COPY(m_vPos, pELCS->vPos);
	VEC_COPY(m_vColor1, pELCS->vColor1);
	VEC_COPY(m_vColor2, pELCS->vColor2);

	m_fDuration			= pELCS->fDuration;
	m_fDelay			= pELCS->fDelay;
	m_fRadius1			= pELCS->fRadius1;
	m_fRadius2			= pELCS->fRadius2;

	if((m_vColor1.x != m_vColor2.x) || (m_vColor1.y != m_vColor2.y) || (m_vColor1.z != m_vColor2.z))
		m_bChangeColor = 1;

	if(m_fRadius1 != m_fRadius2)
		m_bScale = 1;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionLightFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionLightFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;
	m_pClientDE = pClientDE;

	ObjectCreateStruct	ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_LIGHT;
	ocStruct.m_Flags = FLAG_VISIBLE;
	VEC_COPY(ocStruct.m_Pos, m_vPos);

	m_hObject = pClientDE->CreateObject(&ocStruct);
	pClientDE->SetLightColor(m_hObject, m_vColor1.x, m_vColor1.y, m_vColor1.z);

	if(m_fDelay)
		pClientDE->SetLightRadius(m_hObject, 0.0f);
	else
		pClientDE->SetLightRadius(m_hObject, m_fRadius1);

	m_fStartTime = pClientDE->GetTime();

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionLightFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionLightFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;
	DFLOAT		fTime = m_pClientDE->GetTime() - m_fStartTime;
	DFLOAT		fRatio = fTime / m_fDuration;

	if(m_fDelay)
	{
		if(fTime >= m_fDelay)
		{
			m_pClientDE->SetLightRadius(m_hObject, m_fRadius1);
			m_fStartTime = m_pClientDE->GetTime();
			m_fDelay = 0.0f;
		}
		return	DTRUE;
	}

	if(m_bChangeColor)
	{
		DVector		vColor;
		VEC_SUB(vColor, m_vColor2, m_vColor1);
		VEC_MULSCALAR(vColor, vColor, fRatio);
		VEC_ADD(vColor, vColor, m_vColor1);
		m_pClientDE->SetLightColor(m_hObject, vColor.x, vColor.y, vColor.z);
	}

	if(m_bScale)
	{
		DFLOAT		radius;
		radius = m_fRadius1 + ((m_fRadius2 - m_fRadius1) * fRatio);
		m_pClientDE->SetLightRadius(m_hObject, radius);
	}

	return (fTime < m_fDuration);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFragFX::Init
//
//	PURPOSE:	Init the fragment
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFragFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	EXPLOSIONFRAGCS* pEFCS = (EXPLOSIONFRAGCS*)psfxCreateStruct;

	VEC_COPY(m_vPos, pEFCS->vPos);
	VEC_COPY(m_vLastPos, pEFCS->vPos);
	VEC_COPY(m_vNormal, pEFCS->vNormal);
	VEC_COPY(m_vScale, pEFCS->vScale);
	VEC_COPY(m_vRotateMax, pEFCS->vRotateMax);

	m_fSpread			= pEFCS->fSpread;
	m_fDuration			= pEFCS->fDuration;
	m_fVelocity			= pEFCS->fVelocity;
	m_fBounceMod		= pEFCS->fBounceMod;
	m_fGravity			= pEFCS->fGravity;
	m_fFadeTime			= pEFCS->fFadeTime;
	m_fInitAlpha		= pEFCS->fInitAlpha;
	m_bRandDir			= pEFCS->bRandDir;
	m_bSpawnExp			= pEFCS->bSpawnExp;
	m_nSpawnType		= pEFCS->nSpawnType;
	m_nTrailType		= pEFCS->nTrailType;

	if( m_szModel )
		g_pClientDE->FreeString( m_szModel );
	m_szModel			= g_pClientDE->CopyString( pEFCS->szModel );
	if( m_szSkin )
		g_pClientDE->FreeString( m_szSkin );
	m_szSkin			= g_pClientDE->CopyString( pEFCS->szSkin );

	if(m_vRotateMax.x || m_vRotateMax.y || m_vRotateMax.z)
		m_bRotate = 1;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFragFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFragFX::CreateObject(CClientDE *pClientDE)
{
	if (!pClientDE ) return DFALSE;
	m_pClientDE = pClientDE;

	ObjectCreateStruct	ocStruct;
	INIT_OBJECTCREATESTRUCT(ocStruct);

	ocStruct.m_ObjectType = OT_MODEL;
	ocStruct.m_Flags = FLAG_VISIBLE;

	VEC_COPY(ocStruct.m_Pos, m_vPos);

	if(m_bRandDir)
	{
		DVector vDir;
		VEC_SET(vDir, GetRandom(-10.0f, 10.0f), GetRandom(-10.0f, 10.0f), GetRandom(-10.0f, 10.0f));
		VEC_NORM(vDir);
		VEC_MULSCALAR(m_vVelocity, vDir, m_fVelocity);
	}
	else
	{
		DVector		vUp, vU, vR, vF;
		DRotation	rRot;

		VEC_SET(vUp, 0.0f, 1.0f, 0.0f);
		pClientDE->AlignRotation(&rRot, &m_vNormal, &vUp);
		pClientDE->RotateAroundAxis(&rRot, &m_vNormal, GetRandom(-MATH_PI, MATH_PI));
		pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		VEC_MULSCALAR(m_vVelocity, m_vNormal, m_fVelocity);
		VEC_MULSCALAR(vR, vR, m_fSpread);
		VEC_ADD(m_vVelocity, m_vVelocity, vR);
		VEC_NORM(m_vVelocity);
		VEC_MULSCALAR(m_vVelocity, m_vVelocity, m_fVelocity);
	}

	VEC_SET(m_vGravity, 0.0f, -1.0f, 0.0f);
	VEC_MULSCALAR(m_vGravity, m_vGravity, m_fGravity);

	if(m_bRotate)
	{
		fPitch = GetRandom(-m_vRotateMax.x, m_vRotateMax.x);
		fYaw = GetRandom(-m_vRotateMax.y, m_vRotateMax.y);
		fRoll = GetRandom(-m_vRotateMax.z, m_vRotateMax.z);
	}

	if(m_szModel)	_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)pClientDE->GetStringData(m_szModel));
		else		_mbscpy((unsigned char*)ocStruct.m_Filename, (const unsigned char*)"Models\\Explosions\\exp_sphere.abc");

	if(m_szSkin)	_mbscpy((unsigned char*)ocStruct.m_SkinName, (const unsigned char*)pClientDE->GetStringData(m_szSkin));
		else		_mbscpy((unsigned char*)ocStruct.m_SkinName, (const unsigned char*)"Skins\\Explosions\\Explosion_1.dtx");

	m_hObject = pClientDE->CreateObject(&ocStruct);

	DDWORD dwFlags = pClientDE->GetObjectFlags(m_hObject);
	pClientDE->SetObjectFlags(m_hObject, dwFlags | FLAG_MODELGOURAUDSHADE | FLAG_NOLIGHT);

	pClientDE->SetObjectScale(m_hObject, &m_vScale);

	DFLOAT	r, g, b, a;
	pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	pClientDE->SetObjectColor(m_hObject, r, g, b, m_fInitAlpha);

	//**************************************************************//

	if(m_nTrailType)
	{
		CBloodClientShell *pShell = (CBloodClientShell*)pClientDE->GetClientShell();
		if (!pClientDE || !pShell) return DFALSE;

		CSFXMgr* psfxMgr = pShell->GetSFXMgr();
		if (!psfxMgr) return DFALSE;

		OBJECTFXCS		ops;

		ops.hObj			= m_hObject;
		VEC_SET(ops.vOffset, 0.0f, 0.0f, 0.0f);
		ops.fScale			= 0.0f;
		ops.nScaleFlags		= 0;
		ops.nFXType			= m_nTrailType;
		ops.nFXFlags		= 0;

		psfxMgr->CreateSFX(SFX_OBJECTFX_ID, &ops, DFALSE, this);
	}

	//**************************************************************//

	m_fStartTime = pClientDE->GetTime();

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFragFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFragFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;
	DFLOAT		fTime = m_pClientDE->GetTime() - m_fStartTime;
	DBOOL		bMove = 1;
	
	if((VEC_MAG(m_vVelocity) < 1.0f) && (m_vLastNormal.y > 0.9f))
		bMove = 0;

	// Rotate the object if it has a rotation flag and is moving
	if(m_bRotate && bMove)
	{
		DRotation	rRot;
		m_fPitch += fPitch;
		m_fYaw += fYaw;
		m_fRoll += fRoll;
		m_pClientDE->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}

	// Fade out at the end of the duration if there is a fade out time
	if((fTime > m_fDuration - m_fFadeTime) && (fTime < m_fDuration))
	{
		DFLOAT	alpha = m_fInitAlpha - ((fTime - (m_fDuration - m_fFadeTime)) / m_fFadeTime);
		DFLOAT	r, g, b, a;

		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
		m_pClientDE->SetObjectColor(m_hObject, r, g, b, alpha);
	}

	// If the object is moving, check for collisions and set the position and velocity
	if(bMove)
	{
		// Adjust the last and current position data
		VEC_COPY(m_vLastPos, m_vPos);
		VEC_ADD(m_vVelocity, m_vVelocity, m_vGravity);
		VEC_ADD(m_vPos, m_vPos, m_vVelocity);

		// Setup the data to test for a collision
		ClientIntersectQuery	iq;
		ClientIntersectInfo		ii;
		iq.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
		VEC_COPY(iq.m_From, m_vLastPos);
		VEC_MULSCALAR(iq.m_To, m_vVelocity, 1.1f);
		VEC_ADD(iq.m_To, iq.m_To, iq.m_From);

		if(m_pClientDE->IntersectSegment(&iq, &ii))
		{
			DVector		vLift, vVel, vNorm;
			DFLOAT		r = VEC_MAG(m_vVelocity);
			DFLOAT		dot;

			// Make some temporary vectors for calculations
			VEC_COPY(vVel, m_vVelocity);
			VEC_COPY(vNorm, ii.m_Plane.m_Normal);
			VEC_COPY(m_vLastNormal, vNorm);
			VEC_NORM(vNorm);
			VEC_NORM(vVel);

			// Calculate a reflection vector
			dot = VEC_DOT(vNorm, vVel);
			VEC_MULSCALAR(vLift, vNorm, 2.0f * -dot);
			VEC_ADD(vLift, vVel, vLift);

			// Set the velocity of the reflection and decrease it a little
			VEC_MULSCALAR(m_vVelocity, vLift, r * m_fBounceMod);

			// Place the object at the collision point... a little off the surface
			VEC_ADD(m_vPos, ii.m_Point, vNorm);

			// Spawn and new explosion
			if(m_bSpawnExp >= 2)
			{
				CBloodClientShell *pShell = (CBloodClientShell*)m_pClientDE->GetClientShell();
				if (!pShell) return DFALSE;

				CSFXMgr* psfxMgr = pShell->GetSFXMgr();
				if (!psfxMgr) return DFALSE;

				EXPLOSIONFXCS		expCS;
				expCS.hServerObj = 0;
				VEC_COPY(expCS.vPos, m_vPos);
				VEC_COPY(expCS.vNormal, m_vLastNormal);
				expCS.nType = m_nSpawnType;

				psfxMgr->CreateSFX(SFX_EXPLOSIONFX_ID, &expCS, DFALSE, this);
				if(m_bSpawnExp == 2)	return	DFALSE;
			}

			// Slow the rotation for every impact
			fPitch *= 0.5f;
			fYaw *= 0.5f;
			fRoll *= 0.5f;
		}

		m_pClientDE->SetObjectPos(m_hObject, &m_vPos);
	}

	// Check to see if the object should continue to exist...
	if(fTime < m_fDuration)
		return	DTRUE;
	else
	{
		if((m_bSpawnExp == 1) || (m_bSpawnExp == 4))
		{
			CBloodClientShell *pShell = (CBloodClientShell*)m_pClientDE->GetClientShell();
			if (!pShell) return DFALSE;

			CSFXMgr* psfxMgr = pShell->GetSFXMgr();
			if (!psfxMgr) return DFALSE;

			EXPLOSIONFXCS		expCS;
			expCS.hServerObj = 0;
			VEC_COPY(expCS.vPos, m_vPos);
			VEC_COPY(expCS.vNormal, m_vLastNormal);
			expCS.nType = m_nSpawnType;

			psfxMgr->CreateSFX(SFX_EXPLOSIONFX_ID, &expCS, DFALSE, this);
		}
		return	DFALSE;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::Init
//
//	PURPOSE:	Init the FX location and type
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct)		return	DFALSE;
	CSpecialFX::Init(psfxCreateStruct);

	EXPLOSIONFXCS* pEFXCS = (EXPLOSIONFXCS*)psfxCreateStruct;

	VEC_COPY(m_vPos, pEFXCS->vPos);
	VEC_COPY(m_vNormal, pEFXCS->vNormal);
	VEC_NORM(m_vNormal);
	m_nExpID = pEFXCS->nType;

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

DBOOL CExplosionFX::CreateObject(CClientDE *pClientDE)
{
	CBloodClientShell *pShell = (CBloodClientShell*)pClientDE->GetClientShell();
	if (!pClientDE || !pShell) return DFALSE;

	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return DFALSE;

	HCONSOLEVAR hVar = pClientDE->GetConsoleVar("ExplosionsDetail");
	DBYTE	detail = (DBYTE)pClientDE->GetVarValueFloat(hVar);

	if(detail >= 2)
	{
		switch(m_nExpID)
		{
			case	EXP_DEFAULT_SMALL:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS, erCS2;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 25.0f, 25.0f, 25.0f);
				VEC_SET(emCS.vScale2, 200.0f, 200.0f, 200.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.25f, 0.25f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 1.0f);
				erCS.fRadius	= 4000.0f;
				erCS.fPosRadius	= 10.0f;
				erCS.fVelocity	= 400.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 0.5f;
				erCS.fAlpha		= 1.0f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 0;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius1	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.5f, 1.5f, 0.0f);
				esCS.fDuration	= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

			case	EXP_DEFAULT_MEDIUM:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS, erCS2;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 1.0f);
				erCS.fRadius	= 8000.0f;
				erCS.fPosRadius	= 10.0f;
				erCS.fVelocity	= 400.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 0.5f;
				erCS.fAlpha		= 1.0f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 0;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 200.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius1	= 75.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.0f, 2.0f, 0.0f);
				esCS.fDuration	= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_DEFAULT_LARGE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS, erCS2;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 600.0f, 600.0f, 600.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.75f, 0.75f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 1.0f);
				erCS.fRadius	= 12000.0f;
				erCS.fPosRadius	= 10.0f;
				erCS.fVelocity	= 400.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 0.5f;
				erCS.fAlpha		= 1.0f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 0;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration		= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_GRENADE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS, erCS2, erCS3, erCS4, erCS5;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= GetRandom(0,2);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= 0;
				emCS.szSkin			= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Fire Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 1.0f);
				erCS.fRadius		= 8000.0f;
				erCS.fPosRadius		= 10.0f;
				erCS.fVelocity		= 400.0f;
				erCS.fGravity		= 0.0f;
				erCS.nParticles		= 32;
				erCS.fDuration		= 0.5f;
				erCS.fAlpha			= 1.0f;
				erCS.fDelay			= 0.0f;
				erCS.bFadeType		= 1;
				erCS.bAlign			= 0;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Smoke Ring 1
				VEC_COPY(erCS2.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(erCS2.vPos, m_vPos, temp);
				VEC_SET(erCS2.vColor, 1.0f, 1.0f, 1.0f);
				erCS2.fRadius		= 5000.0f;
				erCS2.fPosRadius	= 10.0f;
				erCS2.fVelocity		= GetRandom(100.0f,150.0f);
				erCS2.fGravity		= GetRandom(0.0f,30.0f);
				erCS2.nParticles	= 32;
				erCS2.fDuration		= 1.5f;
				erCS2.fAlpha		= 0.5f;
				erCS2.fDelay		= GetRandom(0.0f,0.5f);
				erCS2.bFadeType		= 1;
				erCS2.bAlign		= 0;
				erCS2.szParticle	= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS2, DFALSE, this);
				g_pClientDE->FreeString( erCS2.szParticle );
				erCS2.szParticle = DNULL;

				// Smoke Ring 2
				VEC_COPY(erCS3.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(erCS3.vPos, m_vPos, temp);
				VEC_SET(erCS3.vColor, 1.0f, 1.0f, 1.0f);
				erCS3.fRadius		= 5000.0f;
				erCS3.fPosRadius	= 10.0f;
				erCS3.fVelocity		= GetRandom(100.0f,150.0f);
				erCS3.fGravity		= GetRandom(20.0f,50.0f);
				erCS3.nParticles	= 32;
				erCS3.fDuration		= 1.5f;
				erCS3.fAlpha		= 0.5f;
				erCS3.fDelay		= GetRandom(0.0f,0.5f);
				erCS3.bFadeType		= 1;
				erCS3.bAlign		= 0;
				erCS3.szParticle	= pClientDE->CreateString("SpriteTextures\\Smoke32_5.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS3, DFALSE, this);
				g_pClientDE->FreeString( erCS3.szParticle );
				erCS3.szParticle = DNULL;

				// Smoke Ring 3
				VEC_COPY(erCS4.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 40.0f);
				VEC_ADD(erCS4.vPos, m_vPos, temp);
				VEC_SET(erCS4.vColor, 1.0f, 1.0f, 1.0f);
				erCS4.fRadius		= 5000.0f;
				erCS4.fPosRadius	= 10.0f;
				erCS4.fVelocity		= GetRandom(100.0f,150.0f);
				erCS4.fGravity		= GetRandom(40.0f,70.0f);
				erCS4.nParticles	= 32;
				erCS4.fDuration		= 1.5f;
				erCS4.fAlpha		= 0.5f;
				erCS4.fDelay		= GetRandom(0.0f,0.5f);
				erCS4.bFadeType		= 1;
				erCS4.bAlign		= 0;
				erCS4.szParticle	= pClientDE->CreateString("SpriteTextures\\Smoke32_6.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS4, DFALSE, this);
				g_pClientDE->FreeString( erCS4.szParticle );
				erCS4.szParticle = DNULL;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 1.0f, 1.0f, 1.0f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.25f;
				efCS.fDuration		= 10.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= OBJFX_SMOKETRAIL_1;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < GetRandom(2,5); i++)
				{
					efCS.fSpread		= GetRandom(40.0f, 60.0f);
					efCS.fVelocity		= GetRandom(20.0f,40.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				efCS.szModel = DNULL;
				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration	= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_NAPALM_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Data for all models
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\Exp_sphere_2.abc");
				emCS.szSkin			= 0;

				// Model 1
				VEC_SET(emCS.vScale1, 20.0f, 20.0f, 20.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.75f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Model 2
				VEC_SET(emCS.vScale1, 35.0f, 35.0f, 35.0f);
				VEC_SET(emCS.vScale2, 350.0f, 350.0f, 350.0f);
				emCS.fDuration		= 0.75f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= 2;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Model 3
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\Explosion.dtx");
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 450.0f, 450.0f, 450.0f);
				emCS.fDuration		= 1.0f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;


				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.75f, 0.75f, 0.0f);
				VEC_COPY(esCS.vScale2, esCS.vScale1);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\napalm.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Fire Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 1.0f);
				erCS.fRadius	= 6000.0f;
				erCS.fPosRadius	= 10.0f;
				erCS.fVelocity	= 400.0f;
				erCS.fGravity	= 350.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 0.5f;
				erCS.fAlpha		= 1.0f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\Firepart64.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration		= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_NAPALM_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\Exp_sphere_2.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\Explosion.dtx");
				VEC_SET(emCS.vScale1, 25.0f, 25.0f, 25.0f);
				VEC_SET(emCS.vScale2, 250.0f, 250.0f, 250.0f);
				emCS.fDuration		= 1.0f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.4f, 0.4f, 0.0f);
				VEC_COPY(esCS.vScale2, esCS.vScale1);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration		= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_NAPALM_FIREBALL:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 0.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_COPY(esCS.vScale2, esCS.vScale1);
				esCS.fDuration		= 0.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 1;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\debrisimp.spr");
	//			esCS.szSprite		= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius1	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.5f, 1.5f, 0.0f);
				esCS.fDuration	= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_TESLA_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 20.0f, 20.0f, 20.0f);
				VEC_SET(emCS.vScale2, 100.0f, 100.0f, 100.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.99f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= 0;
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\eballsmall.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;


				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, GetRandom(0.25f,0.50f), GetRandom(0.25f,0.50f), 0.0f);
				VEC_SET(esCS.vScale2, GetRandom(0.50f,0.75f), GetRandom(0.50f,0.75f), 0.0f);
				esCS.fDuration		= 1.0f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 2;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\TeslaImp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 0.5f, 0.5f, 1.0f);
				erCS.fRadius	= 4000.0f;
				erCS.fPosRadius	= 10.0f;
				erCS.fVelocity	= GetRandom(150.0f,250.0f);
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 24;
				erCS.fDuration	= 0.75f;
				erCS.fRotation	= GetRandom(-0.08f,0.08f);
				erCS.fAlpha		= 0.5f;
				erCS.fDelay		= 0.15f;
				erCS.bFadeType	= 1;
				erCS.bRotateType= 2;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\Smoke32_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.5f, 0.5f, 1.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_TESLA_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONFLAMECS	efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				emCS.fDuration		= 3.0f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= 0;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\duel_sphere.abc");
	//			emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\eballbig.dtx");
				emCS.szSkin			= pClientDE->CreateString("Spritetextures\\waterblue.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;


				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.1f, 0.1f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 3.0f;
				esCS.fAlpha			= 0.75f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\TesAltLoop.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Flame
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(efCS.vPos, m_vPos, temp);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vColor1, 255.0f, 255.0f, 255.0f);
				VEC_SET(efCS.vColor2, 200.0f, 200.0f, 255.0f);
				VEC_SET(efCS.vColor3, 150.0f, 150.0f, 255.0f);
				VEC_SET(efCS.vLifeTimes, 0.75f, 0.875f, 1.0f);
				VEC_SET(efCS.vLifeOffsets, 0.25f, 0.25f, 0.25f);
				VEC_SET(efCS.vFXTimes, 0.5f, 2.5f, 0.0f);
				efCS.nParticles		= 4;
				efCS.fRadius		= 4000.0f;
				efCS.fPosRadius		= 0.0f;
				efCS.fGravity		= 0.0f;
				efCS.fVelocity		= 125.0f;
				efCS.fDelay			= 0.33f;
				efCS.fAlpha			= 0.85f;
				efCS.bFadeType		= 1;
				efCS.bRampFlags		= EXP_FLAME_RAMP_VEL;
				efCS.szParticle		= pClientDE->CreateString("SpriteTextures\\teslaprime_3.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONFLAME_ID, &efCS, DFALSE, this);
				g_pClientDE->FreeString( efCS.szParticle );
				efCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.75f, 0.75f, 1.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 3.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius2	= 350.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_HOWITZER_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS, erCS2;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= 1;
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\howitzerexp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 0.5f);
				erCS.fRadius	= 3000.0f;
				erCS.fPosRadius	= 10.0f;
				erCS.fVelocity	= 300.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 0.5f;
				erCS.fAlpha		= 1.0f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lightn32.dtx");

				for(int i = 0; i < 2; i++)
				{
					erCS.fDelay		+= 0.15f;
					erCS.fRadius	+= 1000.0f;
					psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				}

				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.75f, 0.75f, 1.0f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.25f;
				efCS.fDuration		= 10.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= OBJFX_SMOKETRAIL_1;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(i = 0; i < GetRandom(5,10); i++)
				{
					efCS.fSpread		= GetRandom(40.0f, 80.0f);
					efCS.fVelocity		= GetRandom(20.0f,40.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}

				g_pClientDE->FreeString( efCS.szModel );
				efCS.szModel = DNULL;
				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration		= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_HOWITZER_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= 1;
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, GetRandom(0.6f, 0.9f), GetRandom(0.6f, 0.9f), 0.0f);
				esCS.fDuration	= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\howitzerexp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration		= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_HOWITZER_MINI:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 10.0f, 10.0f, 10.0f);
				VEC_SET(emCS.vScale2, 150.0f, 150.0f, 150.0f);
				emCS.fDuration	= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.15f, 0.15f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration	= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\howitzerexp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius1	= 75.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 1.0f);
				erCS.fRadius		= 8000.0f;
				erCS.fPosRadius		= 5.0f;
				erCS.fVelocity		= 60.0f;
				erCS.fGravity		= 0.0f;
				erCS.nParticles		= 12;
				erCS.fDuration		= 1.5f;
				erCS.fAlpha			= 0.65f;
				erCS.fDelay			= 0.0f;
				erCS.bFadeType		= 1;
				erCS.bAlign			= 1;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Smoke64_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;
			}
			break;

			case	EXP_FLAME_SMALL:
			{
				EXPLOSIONFLAMECS	efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Flame
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(efCS.vPos, m_vPos, temp);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vColor1, 255.0f, 128.0f, 128.0f);
				VEC_SET(efCS.vColor2, 255.0f, 255.0f, 255.0f);
				VEC_SET(efCS.vColor3, 255.0f, 255.0f, 0.0f);
				VEC_SET(efCS.vLifeTimes, 0.35f, 0.65f, 1.0f);
				VEC_SET(efCS.vLifeOffsets, 0.15f, 0.25f, 0.35f);
				VEC_SET(efCS.vFXTimes, 1.0f, 8.0f, 2.0f);
				efCS.nParticles		= 2;
				efCS.fRadius		= 4000.0f;
				efCS.fPosRadius		= 12.0f;
				efCS.fGravity		= 50.0f;
				efCS.fVelocity		= 0.0f;
				efCS.fDelay			= 0.25f;
				efCS.fAlpha			= 0.75f;
				efCS.bFadeType		= 0;
				efCS.bRampFlags		= 0;
				efCS.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONFLAME_ID, &efCS, DFALSE, this);
				g_pClientDE->FreeString( efCS.szParticle );
				efCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 0.5f, 0.0f);
				elCS.fDuration	= 11.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 200.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 35.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius1	= 75.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLARE_FIZZLE:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				EXPLOSIONSPRITECS	esCS;
				DVector				temp;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius2	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Fizzle
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 0.75f, 0.0f);
				erCS.fRadius	= 2500.0f;
				erCS.fPosRadius	= 0.0f;
				erCS.fVelocity	= 50.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 8;
				erCS.fDuration	= GetRandom(0.5f,1.0f);
				erCS.fAlpha		= 1.0f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lensflare_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.0f, 1.0f, 0.0f);
				esCS.fDuration		= 3.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_FLARE_BURST:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.4f, 0.4f, 0.0f);
				esCS.fDuration		= 0.25f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\debrisimp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLARE_FRAG:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 0.5f;
				esCS.fAlpha			= 0.75f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\explodesmall.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.00f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
/*				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius1	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
*/
				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.0f, 1.0f, 0.0f);
				esCS.fDuration	= 3.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_LIFELEECH_PRIMARY:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 0.5f;
				esCS.fAlpha			= 0.75f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\explodesmall.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 0.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
/*				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius1	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
*/
				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.0f, 1.0f, 0.0f);
				esCS.fDuration	= 3.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_LIFELEECH_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\duel_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\leechball2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;


				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.5f, 1.5f, 0.0f);
				esCS.fDuration		= 0.75f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 2;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\explode128.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 0.85f, 0.0f);
				erCS.fRadius	= 7500.0f;
				erCS.fPosRadius	= 25.0f;
				erCS.fVelocity	= 1000.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 0.5f;
				erCS.fRotation	= 0.0;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bRotateType= 0;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lensflare_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 0.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 150.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_SINGULARITY_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 0.5f, 0.5f, 0.5f);
				VEC_SET(emCS.vScale2, 3.0f, 3.0f, 3.0f);
				VEC_SET(emCS.vRotations, 0.0f, 0.0f, -0.05f);
				emCS.fDuration		= 5.5f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 2;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\blackhole.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\blackhole.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 800.0f, 800.0f, 800.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\blackhole2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 0.5f, 0.0f, 0.65f);
				erCS.fRadius	= 12000.0f;
				erCS.fPosRadius	= 50.0f;
				erCS.fVelocity	= 50.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 24;
				erCS.fDuration	= 5.5f;
				erCS.fRotation	= -0.01f;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bRotateType= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.5f, 0.0f, 0.5f);
				VEC_SET(elCS.vColor2, 0.25f, 0.0f, 0.25f);
				elCS.fDuration	= 7.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 10.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_SHOCKBALL:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\zealotball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 0.0f, 0.0f, 1.0f);
				erCS.fRadius	= 7500.0f;
				erCS.fPosRadius	= 25.0f;
				erCS.fVelocity	= 1000.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 0.5f;
				erCS.fRotation	= 0.0;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bRotateType= 0;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lensflare_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.0f, 0.0f, 1.0f);
				VEC_SET(elCS.vColor2, 0.0f, 0.0f, 1.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 150.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_DIVINE_SHOCKBALL:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\divineball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 0.0f, 0.0f);
				erCS.fRadius	= 7500.0f;
				erCS.fPosRadius	= 25.0f;
				erCS.fVelocity	= 1000.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 0.5f;
				erCS.fRotation	= 0.0;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bRotateType= 0;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lensflare_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.0f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 0.0f, 0.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 150.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_SHOCKBALL_LARGE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 200.0f, 200.0f, 200.0f);
				VEC_SET(emCS.vScale2, 1500.0f, 1500.0f, 1500.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\bmothball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 1.0f, 1.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 400.0f;
				elCS.fRadius2	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_ORB_BREAK:
			{
				EXPLOSIONFRAGCS		efCS;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.25f, 0.25f, 0.25f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.5f;
				efCS.fDuration		= 7.0f;
				efCS.fGravity		= 0.125f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= OBJFX_ORBTRAIL_2;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < 10; i++)
				{
					efCS.fSpread		= GetRandom(2.0f, 15.0f);
					efCS.fVelocity		= GetRandom(3.0f,6.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				g_pClientDE->FreeString( efCS.szSkin );
			}
			break;

			case	EXP_RIFT_1:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 0.05f, 0.05f, 1.5f);
				VEC_SET(emCS.vScale2, 2.0f, 2.0f, 3.5f);
				VEC_SET(emCS.vRotations, 0.0f, 0.0f, -0.05f);
				emCS.fDuration		= 2.0f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 4;
				emCS.bFadeType		= 5;
				emCS.bRandomRot		= 2;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\blackhole.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\rift.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 20.0f, 20.0f, 20.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\riftball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 0.0f, 0.75f, 0.0f);
				erCS.fRadius	= 18000.0f;
				erCS.fPosRadius	= 150.0f;
				erCS.fVelocity	= 0.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 18;
				erCS.fDuration	= 2.75f;
				erCS.fRotation	= -0.01f;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 5;
				erCS.bRotateType= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.0f, 0.5f, 0.0f);
				elCS.fDuration	= 3.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_RIFT_2:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 0.5f, 0.5f, 1.5f);
				VEC_SET(emCS.vScale2, 25.0f, 25.0f, 15.0f);
				VEC_SET(emCS.vRotations, 0.0f, 0.0f, -0.05f);
				emCS.fDuration		= 8.0f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 4;
				emCS.bFadeType		= 5;
				emCS.bRandomRot		= 2;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\blackhole.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\leechball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 0.85f, 0.25f, 0.25f);
				erCS.fRadius	= 24000.0f;
				erCS.fPosRadius	= 750.0f;
				erCS.fVelocity	= 0.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 2.75f;
				erCS.fRotation	= -0.01f;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 5;
				erCS.bRotateType= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 1.0f, 0.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.0f, 0.0f);
				elCS.fDuration	= 3.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 500.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_DEATHRAY_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 1.0f, 1.0f, 1.0f);
				VEC_SET(emCS.vScale2, 35.0f, 35.0f, 35.0f);
				emCS.fDuration		= 0.65f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\rift.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.0f, 0.5f, 0.0f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_BUGSPRAY_PRIMARY:
			{
				EXPLOSIONFLAMECS	efCS;
				DVector				temp;

				// Flame
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(efCS.vPos, m_vPos, temp);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vColor1, 128.0f, 128.0f, 128.0f);
				VEC_SET(efCS.vColor2, 192.0f, 192.0f, 192.0f);
				VEC_SET(efCS.vColor3, 255.0f, 255.0f, 255.0f);
				VEC_SET(efCS.vLifeTimes, 0.7f, 0.85f, 1.0f);
				VEC_SET(efCS.vLifeOffsets, 0.15f, 0.25f, 0.35f);
				VEC_SET(efCS.vFXTimes, 0.0f, 1.25f, 0.25f);
				efCS.nParticles		= 3;
				efCS.fRadius		= 2500.0f;
				efCS.fPosRadius		= 7.5f;
				efCS.fGravity		= 200.0f;
				efCS.fVelocity		= 0.0f;
				efCS.fDelay			= 0.1f;
				efCS.fAlpha			= 0.25f;
				efCS.bFadeType		= 1;
				efCS.bRampFlags		= 0;
				efCS.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONFLAME_ID, &efCS, DFALSE, this);
				g_pClientDE->FreeString( efCS.szParticle );
				efCS.szParticle = DNULL;
			}
			break;

			case	EXP_BUGSPRAY_ALT:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 0.5f);
				erCS.fRadius	= 4500.0f;
				erCS.fPosRadius	= 1.0f;
				erCS.fVelocity	= 75.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 16;
				erCS.fDuration	= 0.5f;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 1;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");

				for(int i = 0; i < 2; i++)
				{
					erCS.fDelay		+= 0.1f;
					psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				}
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 15.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 25.0f;
				elCS.fRadius1	= 25.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_NAGA_EYE_BEAM:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;
				EXPLOSIONFLAMECS	efCS;
				DVector				temp;

				// Flame
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(efCS.vPos, m_vPos, temp);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vColor1, 128.0f, 128.0f, 128.0f);
				VEC_SET(efCS.vColor2, 192.0f, 192.0f, 192.0f);
				VEC_SET(efCS.vColor3, 255.0f, 255.0f, 255.0f);
				VEC_SET(efCS.vLifeTimes, 0.7f, 0.85f, 1.0f);
				VEC_SET(efCS.vLifeOffsets, 0.15f, 0.25f, 0.35f);
				VEC_SET(efCS.vFXTimes, 0.0f, 1.0f, 0.25f);
				efCS.nParticles		= 2;
				efCS.fRadius		= 1500.0f;
				efCS.fPosRadius		= 7.5f;
				efCS.fGravity		= 200.0f;
				efCS.fVelocity		= 0.0f;
				efCS.fDelay			= 0.1f;
				efCS.fAlpha			= 0.25f;
				efCS.bFadeType		= 1;
				efCS.bRampFlags		= 0;
				efCS.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONFLAME_ID, &efCS, DFALSE, this);

				g_pClientDE->FreeString( efCS.szParticle );
				efCS.szParticle = DNULL;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 10.0f, 10.0f, 10.0f);
				VEC_SET(emCS.vScale2, 100.0f, 100.0f, 100.0f);
				emCS.fDuration		= 0.65f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\divineball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 1.0f, 0.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_NAGA_SPIKE:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Smoke Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 1.0f, 0.9f, 0.75f);
				erCS.fRadius		= 6000.0f;
				erCS.fPosRadius		= 10.0f;
				erCS.fVelocity		= GetRandom(100.0f,125.0f);
				erCS.fGravity		= GetRandom(25.0f,50.0f);
				erCS.nParticles		= 24;
				erCS.fDuration		= 1.25f;
				erCS.fAlpha			= 0.75f;
				erCS.bFadeType		= 1;
				erCS.bAlign			= 1;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);

				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.75f, 0.75f, 1.0f);
				VEC_SET(efCS.vRotateMax, 0.5f, 0.5f, 0.5f);
				efCS.fBounceMod		= 0.25f;
				efCS.fDuration		= 5.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= 0;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < GetRandom(5,8); i++)
				{
					VEC_SET(efCS.vScale, GetRandom(2.0f, 5.0f), GetRandom(2.0f, 5.0f), GetRandom(2.0f, 3.0f));
					efCS.fSpread		= GetRandom(40.0f, 80.0f);
					efCS.fVelocity		= GetRandom(20.0f,40.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}

				g_pClientDE->FreeString( efCS.szModel );
				efCS.szModel = DNULL;
				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 125.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.5f, 1.5f, 0.0f);
				esCS.fDuration	= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

			case	EXP_NAGA_STONE_CHUNK:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONFRAGCS		efCS;
				DVector				temp;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.75f, 0.75f, 1.0f);
				VEC_SET(efCS.vRotateMax, 0.5f, 0.5f, 0.5f);
				efCS.fBounceMod		= 0.5f;
				efCS.fDuration		= 5.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= 0;

				char	string[64];
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < GetRandom(3,5); i++)
				{
					sprintf(string, "models\\gibs\\stone\\gib%d.abc", GetRandom(6,9));
					efCS.szModel		= pClientDE->CreateString(string);
//					sprintf(string, "spritetextures\\rock32_%d.dtx", GetRandom(1,3));
//					efCS.szSkin			= pClientDE->CreateString(string);

					VEC_SET(efCS.vScale, GetRandom(1.0f, 2.0f), GetRandom(1.0f, 2.0f), GetRandom(1.0f, 2.0f));
					efCS.fSpread		= GetRandom(40.0f, 80.0f);
					efCS.fVelocity		= GetRandom(15.0f,30.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);

					g_pClientDE->FreeString( efCS.szModel );
					efCS.szModel = DNULL;
				}

				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.75f, 1.75f, 0.0f);
				esCS.fDuration	= 2.5f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

			case	EXP_NAGA_POUND_GROUND:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONSPRITECS	esCS;
				DVector				temp;

				// Smoke Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 1.0f, 0.9f, 0.75f);
				erCS.fRadius		= 8000.0f;
				erCS.fPosRadius		= 10.0f;
				erCS.fVelocity		= GetRandom(100.0f,150.0f);
				erCS.fGravity		= GetRandom(25.0f,50.0f);
				erCS.nParticles		= 32;
				erCS.fDuration		= 1.75f;
				erCS.fAlpha			= 0.75f;
				erCS.bFadeType		= 1;
				erCS.bAlign			= 1;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);

				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Shock wave
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 4.0f, 4.0f, 0.0f);
				esCS.fDuration	= 1.25f;
				esCS.fAlpha		= 0.5f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 1;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\ripple.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.1f, 0.1f, 0.0f);
				VEC_SET(esCS.vScale2, 3.5f, 3.5f, 0.0f);
				esCS.fDuration	= 3.5f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

#ifdef _ADD_ON

			case	EXP_GAS_GRENADE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= GetRandom(0,2);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= 0;
				emCS.szSkin			= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Smoke Ring 1
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 0.0f, 0.5f, 0.0f);
				erCS.fRadius		= 6500.0f;
				erCS.fPosRadius		= 10.0f;
				erCS.fVelocity		= GetRandom(100.0f,150.0f);
				erCS.fGravity		= GetRandom(0.0f,30.0f);
				erCS.nParticles		= 32;
				erCS.fDuration		= 1.5f;
				erCS.fAlpha			= 0.5f;
				erCS.fDelay			= GetRandom(0.0f,0.5f);
				erCS.bFadeType		= 1;
				erCS.bAlign			= 0;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 3.0f, 3.0f, 0.0f);
				esCS.fDuration	= 15.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites_ao\\greensmoke.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

			case	EXP_FLAYER_PRIMARY:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.0f, 0.0f, 0.0f);
				VEC_SET(esCS.vScale2, 0.35f, 0.35f, 0.0f);
				esCS.fDuration		= 2.0f;
				esCS.fAlpha			= 0.85f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites_ao\\greenrift.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 25.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLAYER_ALT:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;

				// Sprite
				VEC_SET(esCS.vNormal, 0.0f, 1.0f, 0.0f);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.0f, 0.0f, 0.0f);
				VEC_SET(esCS.vScale2, 0.45f, 0.45f, 0.0f);
				esCS.fDuration		= 22.0f;
				esCS.fAlpha			= 0.85f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 2;
				esCS.szSprite		= pClientDE->CreateString("Sprites_ao\\greenrift.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 20.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 125.0f;
				elCS.fRadius2	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLAYER_RETRACT:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.0f, 0.0f, 0.0f);
				VEC_SET(esCS.vScale2, 0.2f, 0.2f, 0.0f);
				esCS.fDuration		= 11.0f;
				esCS.fAlpha			= 0.85f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites_ao\\greenrift.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 10.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 25.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLAYER_SHATTER:
			{
				EXPLOSIONFRAGCS		efCS;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 5.0f, 5.0f, 5.0f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.5f;
				efCS.fDuration		= 7.0f;
				efCS.fGravity		= 0.2f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.szModel		= pClientDE->CreateString("Models_ao\\Ammo_ao\\chainlink.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins_ao\\Ammo_ao\\chainlink.dtx");

				for(int i = 0; i < 20; i++)
				{
					efCS.fSpread		= GetRandom(2.0f, 15.0f);
					efCS.fVelocity		= GetRandom(3.0f,6.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				g_pClientDE->FreeString( efCS.szSkin );
			}
			break;
#endif
		}
	}
	else if(detail == 1)
	{
		switch(m_nExpID)
		{
			case	EXP_DEFAULT_SMALL:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 25.0f, 25.0f, 25.0f);
				VEC_SET(emCS.vScale2, 200.0f, 200.0f, 200.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.25f, 0.25f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius1	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.5f, 1.5f, 0.0f);
				esCS.fDuration	= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_DEFAULT_MEDIUM:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 200.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius1	= 75.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.0f, 2.0f, 0.0f);
				esCS.fDuration	= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_DEFAULT_LARGE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 600.0f, 600.0f, 600.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.75f, 0.75f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration		= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_GRENADE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS2, erCS3;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= GetRandom(0,2);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= 0;
				emCS.szSkin			= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Smoke Ring 1
				VEC_COPY(erCS2.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(erCS2.vPos, m_vPos, temp);
				VEC_SET(erCS2.vColor, 1.0f, 1.0f, 1.0f);
				erCS2.fRadius		= 5000.0f;
				erCS2.fPosRadius	= 10.0f;
				erCS2.fVelocity		= GetRandom(100.0f,150.0f);
				erCS2.fGravity		= GetRandom(0.0f,30.0f);
				erCS2.nParticles	= 32;
				erCS2.fDuration		= 1.5f;
				erCS2.fAlpha		= 0.5f;
				erCS2.fDelay		= GetRandom(0.0f,0.5f);
				erCS2.bFadeType		= 1;
				erCS2.bAlign		= 0;
				erCS2.szParticle	= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS2, DFALSE, this);
				g_pClientDE->FreeString( erCS2.szParticle );
				erCS2.szParticle = DNULL;

				// Smoke Ring 2
				VEC_COPY(erCS3.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(erCS3.vPos, m_vPos, temp);
				VEC_SET(erCS3.vColor, 1.0f, 1.0f, 1.0f);
				erCS3.fRadius		= 5000.0f;
				erCS3.fPosRadius	= 10.0f;
				erCS3.fVelocity		= GetRandom(100.0f,150.0f);
				erCS3.fGravity		= GetRandom(20.0f,50.0f);
				erCS3.nParticles	= 32;
				erCS3.fDuration		= 1.5f;
				erCS3.fAlpha		= 0.5f;
				erCS3.fDelay		= GetRandom(0.0f,0.5f);
				erCS3.bFadeType		= 1;
				erCS3.bAlign		= 0;
				erCS3.szParticle	= pClientDE->CreateString("SpriteTextures\\Smoke32_5.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS3, DFALSE, this);
				g_pClientDE->FreeString( erCS3.szParticle );
				erCS3.szParticle = DNULL;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 1.0f, 1.0f, 1.0f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.25f;
				efCS.fDuration		= 10.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= OBJFX_SMOKETRAIL_1;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < GetRandom(2,3); i++)
				{
					efCS.fSpread		= GetRandom(40.0f, 60.0f);
					efCS.fVelocity		= GetRandom(20.0f,40.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				efCS.szModel = DNULL;
				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration	= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

			case	EXP_NAPALM_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Data for all models
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\Exp_sphere_2.abc");
				emCS.szSkin			= 0;

				// Model 1
				VEC_SET(emCS.vScale1, 20.0f, 20.0f, 20.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.75f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Model 3
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\Explosion.dtx");
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 450.0f, 450.0f, 450.0f);
				emCS.fDuration		= 1.0f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.75f, 0.75f, 0.0f);
				VEC_COPY(esCS.vScale2, esCS.vScale1);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\napalm.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration		= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_NAPALM_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\Exp_sphere_2.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\Explosion.dtx");
				VEC_SET(emCS.vScale1, 25.0f, 25.0f, 25.0f);
				VEC_SET(emCS.vScale2, 250.0f, 250.0f, 250.0f);
				emCS.fDuration		= 1.0f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.4f, 0.4f, 0.0f);
				VEC_COPY(esCS.vScale2, esCS.vScale1);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration		= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_NAPALM_FIREBALL:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 0.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_COPY(esCS.vScale2, esCS.vScale1);
				esCS.fDuration		= 0.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 1;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\debrisimp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius1	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.5f, 1.5f, 0.0f);
				esCS.fDuration	= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_TESLA_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 20.0f, 20.0f, 20.0f);
				VEC_SET(emCS.vScale2, 100.0f, 100.0f, 100.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.99f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= 0;
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\eballsmall.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 0.5f, 0.5f, 1.0f);
				erCS.fRadius	= 4000.0f;
				erCS.fPosRadius	= 10.0f;
				erCS.fVelocity	= GetRandom(150.0f,250.0f);
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 18;
				erCS.fDuration	= 0.75f;
				erCS.fRotation	= GetRandom(-0.08f,0.08f);
				erCS.fAlpha		= 0.5f;
				erCS.fDelay		= 0.15f;
				erCS.bFadeType	= 1;
				erCS.bRotateType= 2;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\Smoke32_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.5f, 0.5f, 1.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_TESLA_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONFLAMECS	efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				emCS.fDuration		= 3.0f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= 0;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\duel_sphere.abc");
	//			emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\eballbig.dtx");
				emCS.szSkin			= pClientDE->CreateString("Spritetextures\\waterblue.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.1f, 0.1f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration		= 3.0f;
				esCS.fAlpha			= 0.75f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\TesAltLoop.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Flame
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(efCS.vPos, m_vPos, temp);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vColor1, 255.0f, 255.0f, 255.0f);
				VEC_SET(efCS.vColor2, 200.0f, 200.0f, 255.0f);
				VEC_SET(efCS.vColor3, 150.0f, 150.0f, 255.0f);
				VEC_SET(efCS.vLifeTimes, 0.75f, 0.875f, 1.0f);
				VEC_SET(efCS.vLifeOffsets, 0.25f, 0.25f, 0.25f);
				VEC_SET(efCS.vFXTimes, 0.5f, 2.5f, 0.0f);
				efCS.nParticles		= 2;
				efCS.fRadius		= 4000.0f;
				efCS.fPosRadius		= 0.0f;
				efCS.fGravity		= 0.0f;
				efCS.fVelocity		= 125.0f;
				efCS.fDelay			= 0.33f;
				efCS.fAlpha			= 0.85f;
				efCS.bFadeType		= 1;
				efCS.bRampFlags		= EXP_FLAME_RAMP_VEL;
				efCS.szParticle		= pClientDE->CreateString("SpriteTextures\\teslaprime_3.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONFLAME_ID, &efCS, DFALSE, this);
				g_pClientDE->FreeString( efCS.szParticle );
				efCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.75f, 0.75f, 1.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 3.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius2	= 350.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_HOWITZER_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS, erCS2;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= 1;
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.65f, 0.65f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\howitzerexp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 0.5f);
				erCS.fRadius	= 3000.0f;
				erCS.fPosRadius	= 10.0f;
				erCS.fVelocity	= 300.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 0.5f;
				erCS.fAlpha		= 1.0f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lightn32.dtx");

				for(int i = 0; i < 1; i++)
				{
					erCS.fDelay		+= 0.225f;
					erCS.fRadius	+= 1500.0f;
					psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				}
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.75f, 0.75f, 1.0f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.25f;
				efCS.fDuration		= 10.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= OBJFX_SMOKETRAIL_1;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(i = 0; i < GetRandom(3,6); i++)
				{
					efCS.fSpread		= GetRandom(40.0f, 80.0f);
					efCS.fVelocity		= GetRandom(20.0f,40.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				efCS.szModel = DNULL;
				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration		= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

			case	EXP_HOWITZER_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= 1;
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, GetRandom(0.6f, 0.9f), GetRandom(0.6f, 0.9f), 0.0f);
				esCS.fDuration	= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\howitzerexp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 2.5f, 2.5f, 0.0f);
				esCS.fDuration		= 5.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_HOWITZER_MINI:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 10.0f, 10.0f, 10.0f);
				VEC_SET(emCS.vScale2, 150.0f, 150.0f, 150.0f);
				emCS.fDuration	= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.15f, 0.15f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration	= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\howitzerexp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius1	= 75.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 1.0f);
				erCS.fRadius		= 8000.0f;
				erCS.fPosRadius		= 5.0f;
				erCS.fVelocity		= 60.0f;
				erCS.fGravity		= 0.0f;
				erCS.nParticles		= 12;
				erCS.fDuration		= 1.5f;
				erCS.fAlpha			= 0.65f;
				erCS.fDelay			= 0.0f;
				erCS.bFadeType		= 1;
				erCS.bAlign			= 1;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Smoke64_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;
			}
			break;

			case	EXP_FLAME_SMALL:
			{
				EXPLOSIONFLAMECS	efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Flame
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(efCS.vPos, m_vPos, temp);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vColor1, 255.0f, 128.0f, 128.0f);
				VEC_SET(efCS.vColor2, 255.0f, 255.0f, 255.0f);
				VEC_SET(efCS.vColor3, 255.0f, 255.0f, 0.0f);
				VEC_SET(efCS.vLifeTimes, 0.35f, 0.65f, 1.0f);
				VEC_SET(efCS.vLifeOffsets, 0.15f, 0.25f, 0.35f);
				VEC_SET(efCS.vFXTimes, 1.0f, 8.0f, 2.0f);
				efCS.nParticles		= 2;
				efCS.fRadius		= 4000.0f;
				efCS.fPosRadius		= 12.0f;
				efCS.fGravity		= 50.0f;
				efCS.fVelocity		= 0.0f;
				efCS.fDelay			= 0.35f;
				efCS.fAlpha			= 0.75f;
				efCS.bFadeType		= 0;
				efCS.bRampFlags		= 0;
				efCS.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONFLAME_ID, &efCS, DFALSE, this);
				g_pClientDE->FreeString( efCS.szParticle );
				efCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 0.5f, 0.0f);
				elCS.fDuration	= 11.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 200.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 35.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius1	= 75.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLARE_FIZZLE:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				EXPLOSIONSPRITECS	esCS;
				DVector				temp;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius2	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Fizzle
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 0.75f, 0.0f);
				erCS.fRadius	= 2500.0f;
				erCS.fPosRadius	= 0.0f;
				erCS.fVelocity	= 50.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 8;
				erCS.fDuration	= GetRandom(0.5f,1.0f);
				erCS.fAlpha		= 1.0f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lensflare_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.0f, 1.0f, 0.0f);
				esCS.fDuration		= 3.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_FLARE_BURST:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.4f, 0.4f, 0.0f);
				esCS.fDuration		= 0.25f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\debrisimp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLARE_FRAG:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 0.5f;
				esCS.fAlpha			= 0.75f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\explodesmall.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.00f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
/*				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius1	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
*/
				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.0f, 1.0f, 0.0f);
				esCS.fDuration	= 3.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_LIFELEECH_PRIMARY:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 0.5f;
				esCS.fAlpha			= 0.75f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\explodesmall.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 0.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
/*				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius1	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
*/
				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration	= 3.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

			}
			break;

			case	EXP_LIFELEECH_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\duel_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\leechball2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.25f, 1.25f, 0.0f);
				esCS.fDuration		= 0.75f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 2;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\explode128.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 0.85f, 0.0f);
				erCS.fRadius	= 7500.0f;
				erCS.fPosRadius	= 25.0f;
				erCS.fVelocity	= 1000.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 24;
				erCS.fDuration	= 0.5f;
				erCS.fRotation	= 0.0;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bRotateType= 0;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lensflare_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 0.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 150.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_SINGULARITY_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 0.5f, 0.5f, 0.5f);
				VEC_SET(emCS.vScale2, 3.0f, 3.0f, 3.0f);
				VEC_SET(emCS.vRotations, 0.0f, 0.0f, -0.05f);
				emCS.fDuration		= 5.5f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 2;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\blackhole.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\blackhole.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 0.5f, 0.0f, 0.65f);
				erCS.fRadius	= 12000.0f;
				erCS.fPosRadius	= 50.0f;
				erCS.fVelocity	= 50.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 18;
				erCS.fDuration	= 5.5f;
				erCS.fRotation	= -0.01f;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bRotateType= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.5f, 0.0f, 0.5f);
				VEC_SET(elCS.vColor2, 0.25f, 0.0f, 0.25f);
				elCS.fDuration	= 7.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 10.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_SHOCKBALL:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\zealotball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 0.0f, 0.0f, 1.0f);
				erCS.fRadius	= 7500.0f;
				erCS.fPosRadius	= 25.0f;
				erCS.fVelocity	= 1000.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 24;
				erCS.fDuration	= 0.5f;
				erCS.fRotation	= 0.0;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bRotateType= 0;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lensflare_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.0f, 0.0f, 1.0f);
				VEC_SET(elCS.vColor2, 0.0f, 0.0f, 1.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 150.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_DIVINE_SHOCKBALL:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\divineball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 0.0f, 0.0f);
				erCS.fRadius	= 7500.0f;
				erCS.fPosRadius	= 25.0f;
				erCS.fVelocity	= 1000.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 24;
				erCS.fDuration	= 0.5f;
				erCS.fRotation	= 0.0;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bRotateType= 0;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lensflare_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.0f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 0.0f, 0.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 150.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_SHOCKBALL_LARGE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 200.0f, 200.0f, 200.0f);
				VEC_SET(emCS.vScale2, 1500.0f, 1500.0f, 1500.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\bmothball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 1.0f, 1.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 400.0f;
				elCS.fRadius2	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_ORB_BREAK:
			{
				EXPLOSIONFRAGCS		efCS;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.25f, 0.25f, 0.25f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.5f;
				efCS.fDuration		= 7.0f;
				efCS.fGravity		= 0.125f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= OBJFX_ORBTRAIL_2;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < 8; i++)
				{
					efCS.fSpread		= GetRandom(2.0f, 15.0f);
					efCS.fVelocity		= GetRandom(3.0f,6.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				g_pClientDE->FreeString( efCS.szSkin );
			}
			break;

			case	EXP_RIFT_1:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 0.05f, 0.05f, 1.5f);
				VEC_SET(emCS.vScale2, 2.0f, 2.0f, 3.5f);
				VEC_SET(emCS.vRotations, 0.0f, 0.0f, -0.05f);
				emCS.fDuration		= 2.0f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 4;
				emCS.bFadeType		= 5;
				emCS.bRandomRot		= 2;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\blackhole.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\rift.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 0.0f, 0.75f, 0.0f);
				erCS.fRadius	= 18000.0f;
				erCS.fPosRadius	= 150.0f;
				erCS.fVelocity	= 0.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 12;
				erCS.fDuration	= 2.75f;
				erCS.fRotation	= -0.01f;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 5;
				erCS.bRotateType= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.0f, 0.5f, 0.0f);
				elCS.fDuration	= 3.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_RIFT_2:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 0.5f, 0.5f, 1.5f);
				VEC_SET(emCS.vScale2, 25.0f, 25.0f, 15.0f);
				VEC_SET(emCS.vRotations, 0.0f, 0.0f, -0.05f);
				emCS.fDuration		= 8.0f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 4;
				emCS.bFadeType		= 5;
				emCS.bRandomRot		= 2;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\blackhole.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\leechball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 0.85f, 0.25f, 0.25f);
				erCS.fRadius	= 24000.0f;
				erCS.fPosRadius	= 750.0f;
				erCS.fVelocity	= 0.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 32;
				erCS.fDuration	= 2.75f;
				erCS.fRotation	= -0.01f;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 5;
				erCS.bRotateType= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 1.0f, 0.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.0f, 0.0f);
				elCS.fDuration	= 3.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 500.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_DEATHRAY_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 1.0f, 1.0f, 1.0f);
				VEC_SET(emCS.vScale2, 35.0f, 35.0f, 35.0f);
				emCS.fDuration		= 0.65f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\rift.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.0f, 0.5f, 0.0f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_BUGSPRAY_PRIMARY:
			{
				EXPLOSIONFLAMECS	efCS;
				DVector				temp;

				// Flame
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(efCS.vPos, m_vPos, temp);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vColor1, 128.0f, 128.0f, 128.0f);
				VEC_SET(efCS.vColor2, 192.0f, 192.0f, 192.0f);
				VEC_SET(efCS.vColor3, 255.0f, 255.0f, 255.0f);
				VEC_SET(efCS.vLifeTimes, 0.7f, 0.85f, 1.0f);
				VEC_SET(efCS.vLifeOffsets, 0.15f, 0.25f, 0.35f);
				VEC_SET(efCS.vFXTimes, 0.0f, 1.25f, 0.25f);
				efCS.nParticles		= 3;
				efCS.fRadius		= 2500.0f;
				efCS.fPosRadius		= 7.5f;
				efCS.fGravity		= 200.0f;
				efCS.fVelocity		= 0.0f;
				efCS.fDelay			= 0.1f;
				efCS.fAlpha			= 0.25f;
				efCS.bFadeType		= 1;
				efCS.bRampFlags		= 0;
				efCS.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONFLAME_ID, &efCS, DFALSE, this);
				g_pClientDE->FreeString( efCS.szParticle );
				efCS.szParticle = DNULL;
			}
			break;

			case	EXP_BUGSPRAY_ALT:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 0.5f);
				erCS.fRadius	= 4500.0f;
				erCS.fPosRadius	= 1.0f;
				erCS.fVelocity	= 75.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 16;
				erCS.fDuration	= 0.5f;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 1;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");

				for(int i = 0; i < 1; i++)
				{
					erCS.fDelay		+= 0.15f;
					psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				}
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 15.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 25.0f;
				elCS.fRadius1	= 25.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_NAGA_EYE_BEAM:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 10.0f, 10.0f, 10.0f);
				VEC_SET(emCS.vScale2, 100.0f, 100.0f, 100.0f);
				emCS.fDuration		= 0.65f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\divineball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 1.0f, 0.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_NAGA_SPIKE:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Smoke Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 1.0f, 0.9f, 0.75f);
				erCS.fRadius		= 6000.0f;
				erCS.fPosRadius		= 10.0f;
				erCS.fVelocity		= GetRandom(100.0f,125.0f);
				erCS.fGravity		= GetRandom(25.0f,50.0f);
				erCS.nParticles		= 12;
				erCS.fDuration		= 1.25f;
				erCS.fAlpha			= 0.75f;
				erCS.bFadeType		= 1;
				erCS.bAlign			= 1;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);

				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.75f, 0.75f, 1.0f);
				VEC_SET(efCS.vRotateMax, 0.5f, 0.5f, 0.5f);
				efCS.fBounceMod		= 0.25f;
				efCS.fDuration		= 5.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= 0;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < GetRandom(2,4); i++)
				{
					VEC_SET(efCS.vScale, GetRandom(2.0f, 5.0f), GetRandom(2.0f, 5.0f), GetRandom(2.0f, 3.0f));
					efCS.fSpread		= GetRandom(40.0f, 80.0f);
					efCS.fVelocity		= GetRandom(20.0f,40.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}

				g_pClientDE->FreeString( efCS.szModel );
				efCS.szModel = DNULL;
				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 125.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.5f, 1.5f, 0.0f);
				esCS.fDuration	= 2.5f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

			case	EXP_NAGA_STONE_CHUNK:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONFRAGCS		efCS;
				DVector				temp;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.75f, 0.75f, 1.0f);
				VEC_SET(efCS.vRotateMax, 0.5f, 0.5f, 0.5f);
				efCS.fBounceMod		= 0.5f;
				efCS.fDuration		= 5.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= 0;

				char	string[64];
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < GetRandom(2,3); i++)
				{
					sprintf(string, "models\\gibs\\stone\\gib%d.abc", GetRandom(6,9));
					efCS.szModel		= pClientDE->CreateString(string);
//					sprintf(string, "spritetextures\\rock32_%d.dtx", GetRandom(1,3));
//					efCS.szSkin			= pClientDE->CreateString(string);

					VEC_SET(efCS.vScale, GetRandom(1.0f, 2.0f), GetRandom(1.0f, 2.0f), GetRandom(1.0f, 2.0f));
					efCS.fSpread		= GetRandom(40.0f, 80.0f);
					efCS.fVelocity		= GetRandom(15.0f,30.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);

					g_pClientDE->FreeString( efCS.szModel );
					efCS.szModel = DNULL;
				}

				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 1.75f, 1.75f, 0.0f);
				esCS.fDuration	= 1.5f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

			case	EXP_NAGA_POUND_GROUND:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONSPRITECS	esCS;
				DVector				temp;

				// Smoke Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 1.0f, 0.9f, 0.75f);
				erCS.fRadius		= 8000.0f;
				erCS.fPosRadius		= 10.0f;
				erCS.fVelocity		= GetRandom(100.0f,150.0f);
				erCS.fGravity		= GetRandom(25.0f,50.0f);
				erCS.nParticles		= 16;
				erCS.fDuration		= 1.75f;
				erCS.fAlpha			= 0.75f;
				erCS.bFadeType		= 1;
				erCS.bAlign			= 1;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);

				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Shock wave
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 4.0f, 4.0f, 0.0f);
				esCS.fDuration	= 1.25f;
				esCS.fAlpha		= 0.5f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 1;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\ripple.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.1f, 0.1f, 0.0f);
				VEC_SET(esCS.vScale2, 3.5f, 3.5f, 0.0f);
				esCS.fDuration	= 2.25f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\smokepuff2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

#ifdef _ADD_ON

			case	EXP_GAS_GRENADE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= GetRandom(0,2);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= 0;
				emCS.szSkin			= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Smoke Ring 1
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 0.0f, 0.5f, 0.0f);
				erCS.fRadius		= 6500.0f;
				erCS.fPosRadius		= 10.0f;
				erCS.fVelocity		= GetRandom(100.0f,150.0f);
				erCS.fGravity		= GetRandom(0.0f,30.0f);
				erCS.nParticles		= 32;
				erCS.fDuration		= 1.5f;
				erCS.fAlpha			= 0.5f;
				erCS.fDelay			= GetRandom(0.0f,0.5f);
				erCS.bFadeType		= 1;
				erCS.bAlign			= 0;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 25.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius1	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 3.0f, 3.0f, 0.0f);
				esCS.fDuration	= 15.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites_ao\\greensmoke.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

			case	EXP_FLAYER_PRIMARY:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.0f, 0.0f, 0.0f);
				VEC_SET(esCS.vScale2, 0.35f, 0.35f, 0.0f);
				esCS.fDuration		= 2.0f;
				esCS.fAlpha			= 0.85f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites_ao\\greenrift.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 25.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLAYER_ALT:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;

				// Sprite
				VEC_SET(esCS.vNormal, 0.0f, 1.0f, 0.0f);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.0f, 0.0f, 0.0f);
				VEC_SET(esCS.vScale2, 0.45f, 0.45f, 0.0f);
				esCS.fDuration		= 22.0f;
				esCS.fAlpha			= 0.85f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 2;
				esCS.szSprite		= pClientDE->CreateString("Sprites_ao\\greenrift.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 20.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 125.0f;
				elCS.fRadius2	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLAYER_RETRACT:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.0f, 0.0f, 0.0f);
				VEC_SET(esCS.vScale2, 0.2f, 0.2f, 0.0f);
				esCS.fDuration		= 11.0f;
				esCS.fAlpha			= 0.85f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites_ao\\greenrift.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 10.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 25.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLAYER_SHATTER:
			{
				EXPLOSIONFRAGCS		efCS;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 5.0f, 5.0f, 5.0f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.5f;
				efCS.fDuration		= 7.0f;
				efCS.fGravity		= 0.2f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.szModel		= pClientDE->CreateString("Models_ao\\Ammo_ao\\chainlink.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins_ao\\Ammo_ao\\chainlink.dtx");

				for(int i = 0; i < 12; i++)
				{
					efCS.fSpread		= GetRandom(2.0f, 15.0f);
					efCS.fVelocity		= GetRandom(3.0f,6.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				g_pClientDE->FreeString( efCS.szSkin );
			}
			break;
#endif
		}
	}
	else
	{
		switch(m_nExpID)
		{
			case	EXP_DEFAULT_SMALL:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 25.0f, 25.0f, 25.0f);
				VEC_SET(emCS.vScale2, 200.0f, 200.0f, 200.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.25f, 0.25f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_DEFAULT_MEDIUM:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 200.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_DEFAULT_LARGE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 600.0f, 600.0f, 600.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.75f, 0.75f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_GRENADE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS2, erCS3;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= GetRandom(0,2);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= 0;
				emCS.szSkin			= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Smoke Ring 1
				VEC_COPY(erCS2.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(erCS2.vPos, m_vPos, temp);
				VEC_SET(erCS2.vColor, 1.0f, 1.0f, 1.0f);
				erCS2.fRadius		= 5000.0f;
				erCS2.fPosRadius	= 10.0f;
				erCS2.fVelocity		= GetRandom(100.0f,150.0f);
				erCS2.fGravity		= GetRandom(0.0f,30.0f);
				erCS2.nParticles	= 32;
				erCS2.fDuration		= 1.5f;
				erCS2.fAlpha		= 0.5f;
				erCS2.fDelay		= GetRandom(0.0f,0.5f);
				erCS2.bFadeType		= 1;
				erCS2.bAlign		= 0;
				erCS2.szParticle	= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS2, DFALSE, this);
				g_pClientDE->FreeString( erCS2.szParticle );
				erCS2.szParticle = DNULL;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 1.0f, 1.0f, 1.0f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.25f;
				efCS.fDuration		= 10.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= OBJFX_SMOKETRAIL_1;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < GetRandom(0,3); i++)
				{
					efCS.fSpread		= GetRandom(40.0f, 60.0f);
					efCS.fVelocity		= GetRandom(20.0f,40.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				g_pClientDE->FreeString( efCS.szSkin );

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_NAPALM_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Data for all models
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\Exp_sphere_2.abc");
				emCS.szSkin			= 0;

				// Model 1
				VEC_SET(emCS.vScale1, 20.0f, 20.0f, 20.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.75f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Model 3
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\Explosion.dtx");
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 450.0f, 450.0f, 450.0f);
				emCS.fDuration		= 1.0f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.65f, 0.65f, 0.0f);
				VEC_COPY(esCS.vScale2, esCS.vScale1);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\napalm.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_NAPALM_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\Exp_sphere_2.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\Explosion.dtx");
				VEC_SET(emCS.vScale1, 25.0f, 25.0f, 25.0f);
				VEC_SET(emCS.vScale2, 250.0f, 250.0f, 250.0f);
				emCS.fDuration		= 1.0f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.4f, 0.4f, 0.0f);
				VEC_COPY(esCS.vScale2, esCS.vScale1);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_NAPALM_FIREBALL:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 0.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_COPY(esCS.vScale2, esCS.vScale1);
				esCS.fDuration		= 0.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 1;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\debrisimp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_TESLA_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 20.0f, 20.0f, 20.0f);
				VEC_SET(emCS.vScale2, 100.0f, 100.0f, 100.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.99f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= 0;
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\eballsmall.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.5f, 0.5f, 1.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_TESLA_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				emCS.fDuration		= 3.0f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= 0;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\duel_sphere.abc");
	//			emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\eballbig.dtx");
				emCS.szSkin			= pClientDE->CreateString("Spritetextures\\waterblue.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.1f, 0.1f, 0.0f);
				VEC_SET(esCS.vScale2, 0.4f, 0.4f, 0.0f);
				esCS.fDuration		= 3.0f;
				esCS.fAlpha			= 0.75f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\TesAltLoop.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 30.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.75f, 0.75f, 1.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 3.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius2	= 350.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_HOWITZER_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= 1;
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.65f, 0.65f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\howitzerexp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.75f, 0.75f, 1.0f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.25f;
				efCS.fDuration		= 10.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= OBJFX_SMOKETRAIL_1;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < GetRandom(3,6); i++)
				{
					efCS.fSpread		= GetRandom(40.0f, 80.0f);
					efCS.fVelocity		= GetRandom(20.0f,40.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				efCS.szModel = DNULL;
				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_HOWITZER_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= 1;
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, GetRandom(0.6f, 0.9f), GetRandom(0.6f, 0.9f), 0.0f);
				esCS.fDuration	= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\howitzerexp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_HOWITZER_MINI:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 10.0f, 10.0f, 10.0f);
				VEC_SET(emCS.vScale2, 150.0f, 150.0f, 150.0f);
				emCS.fDuration	= 0.5f;
				emCS.fAlpha		= 0.75f;
				emCS.bWaveForm	= GetRandom(0,2);
				emCS.bFadeType	= 1;
				emCS.bRandomRot	= 1;
				emCS.szModel	= 0;
				emCS.szSkin		= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.15f, 0.15f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration	= 1.5f;
				esCS.fAlpha		= 1.0f;
				esCS.bWaveForm	= 0;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\howitzerexp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLAME_SMALL:
			{
				EXPLOSIONFLAMECS	efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Flame
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(efCS.vPos, m_vPos, temp);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vColor1, 255.0f, 128.0f, 128.0f);
				VEC_SET(efCS.vColor2, 255.0f, 255.0f, 255.0f);
				VEC_SET(efCS.vColor3, 255.0f, 255.0f, 0.0f);
				VEC_SET(efCS.vLifeTimes, 0.35f, 0.65f, 1.0f);
				VEC_SET(efCS.vLifeOffsets, 0.15f, 0.25f, 0.35f);
				VEC_SET(efCS.vFXTimes, 1.0f, 8.0f, 2.0f);
				efCS.nParticles		= 2;
				efCS.fRadius		= 4000.0f;
				efCS.fPosRadius		= 12.0f;
				efCS.fGravity		= 50.0f;
				efCS.fVelocity		= 0.0f;
				efCS.fDelay			= 0.5f;
				efCS.fAlpha			= 0.75f;
				efCS.bFadeType		= 0;
				efCS.bRampFlags		= 0;
				efCS.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONFLAME_ID, &efCS, DFALSE, this);
				g_pClientDE->FreeString( efCS.szParticle );
				efCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 0.5f, 0.0f);
				elCS.fDuration	= 11.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 200.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Light  scorch
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.2f, 0.2f, 0.2f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 35.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius1	= 75.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLARE_FIZZLE:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				EXPLOSIONSPRITECS	esCS;
				DVector				temp;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 100.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Fizzle
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 0.75f, 0.0f);
				erCS.fRadius	= 2500.0f;
				erCS.fPosRadius	= 0.0f;
				erCS.fVelocity	= 50.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 8;
				erCS.fDuration	= GetRandom(0.5f,1.0f);
				erCS.fAlpha		= 1.0f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 1;
				erCS.szParticle	= pClientDE->CreateString("SpriteTextures\\lensflare_1.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;
			}
			break;

			case	EXP_FLARE_BURST:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.4f, 0.4f, 0.0f);
				esCS.fDuration		= 0.25f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\debrisimp.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 150.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLARE_FRAG:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 0.5f;
				esCS.fAlpha			= 0.75f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\explodesmall.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.00f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_LIFELEECH_PRIMARY:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.25f, 0.25f, 0.0f);
				VEC_SET(esCS.vScale2, 0.75f, 0.75f, 0.0f);
				esCS.fDuration		= 0.5f;
				esCS.fAlpha			= 0.75f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\explodesmall.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;


				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 0.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_LIFELEECH_ALT:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\duel_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\leechball2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 0.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 150.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_SINGULARITY_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 0.5f, 0.5f, 0.5f);
				VEC_SET(emCS.vScale2, 3.0f, 3.0f, 3.0f);
				VEC_SET(emCS.vRotations, 0.0f, 0.0f, -0.05f);
				emCS.fDuration		= 5.5f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 2;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\blackhole.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\blackhole.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.5f, 0.0f, 0.5f);
				VEC_SET(elCS.vColor2, 0.25f, 0.0f, 0.25f);
				elCS.fDuration	= 7.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 10.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_SHOCKBALL:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\zealotball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 0.0f, 0.0f, 1.0f);
				VEC_SET(elCS.vColor2, 0.0f, 0.0f, 1.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 150.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_DIVINE_SHOCKBALL:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 100.0f, 100.0f, 100.0f);
				VEC_SET(emCS.vScale2, 1000.0f, 1000.0f, 1000.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\divineball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.0f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 0.0f, 0.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 150.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_SHOCKBALL_LARGE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 200.0f, 200.0f, 200.0f);
				VEC_SET(emCS.vScale2, 1500.0f, 1500.0f, 1500.0f);
				VEC_SET(emCS.vRotations, 0.05f, 0.0f, 0.0f);
				emCS.fDuration		= 0.50f;
				emCS.fAlpha			= 0.5f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 0;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\bmothball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 1.0f, 1.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 400.0f;
				elCS.fRadius2	= 100.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_ORB_BREAK:
			{
				EXPLOSIONFRAGCS		efCS;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.25f, 0.25f, 0.25f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.5f;
				efCS.fDuration		= 7.0f;
				efCS.fGravity		= 0.125f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= OBJFX_ORBTRAIL_2;
				efCS.szModel		= pClientDE->CreateString("Models\\Gibs\\gibmetal2.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < 5; i++)
				{
					efCS.fSpread		= GetRandom(2.0f, 15.0f);
					efCS.fVelocity		= GetRandom(3.0f,6.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				efCS.szModel = DNULL;
				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;
			}
			break;

			case	EXP_RIFT_1:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 0.05f, 0.05f, 1.5f);
				VEC_SET(emCS.vScale2, 2.0f, 2.0f, 3.5f);
				VEC_SET(emCS.vRotations, 0.0f, 0.0f, -0.05f);
				emCS.fDuration		= 2.0f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 4;
				emCS.bFadeType		= 5;
				emCS.bRandomRot		= 2;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\blackhole.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\rift.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.0f, 0.5f, 0.0f);
				elCS.fDuration	= 3.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 300.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_RIFT_2:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 0.5f, 0.5f, 1.5f);
				VEC_SET(emCS.vScale2, 25.0f, 25.0f, 15.0f);
				VEC_SET(emCS.vRotations, 0.0f, 0.0f, -0.05f);
				emCS.fDuration		= 8.0f;
				emCS.fAlpha			= 1.0f;
				emCS.bWaveForm		= 4;
				emCS.bFadeType		= 5;
				emCS.bRandomRot		= 2;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\blackhole.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\leechball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 1.0f, 0.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.0f, 0.0f);
				elCS.fDuration	= 3.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 500.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_DEATHRAY_PRIMARY:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONLIGHTCS	elCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 1.0f, 1.0f, 1.0f);
				VEC_SET(emCS.vScale2, 35.0f, 35.0f, 35.0f);
				emCS.fDuration		= 0.65f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\rift.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 1.0f, 0.0f);
				VEC_SET(elCS.vColor2, 0.0f, 0.5f, 0.0f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius2	= 15.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_BUGSPRAY_PRIMARY:
			{
				EXPLOSIONFLAMECS	efCS;
				DVector				temp;

				// Flame
				VEC_MULSCALAR(temp, m_vNormal, 20.0f);
				VEC_ADD(efCS.vPos, m_vPos, temp);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vColor1, 128.0f, 128.0f, 128.0f);
				VEC_SET(efCS.vColor2, 192.0f, 192.0f, 192.0f);
				VEC_SET(efCS.vColor3, 255.0f, 255.0f, 255.0f);
				VEC_SET(efCS.vLifeTimes, 0.7f, 0.85f, 1.0f);
				VEC_SET(efCS.vLifeOffsets, 0.15f, 0.25f, 0.35f);
				VEC_SET(efCS.vFXTimes, 0.0f, 1.25f, 0.25f);
				efCS.nParticles		= 3;
				efCS.fRadius		= 2500.0f;
				efCS.fPosRadius		= 7.5f;
				efCS.fGravity		= 200.0f;
				efCS.fVelocity		= 0.0f;
				efCS.fDelay			= 0.1f;
				efCS.fAlpha			= 0.25f;
				efCS.bFadeType		= 1;
				efCS.bRampFlags		= 0;
				efCS.szParticle		= pClientDE->CreateString("SpriteTextures\\smoke64_2.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONFLAME_ID, &efCS, DFALSE, this);
				g_pClientDE->FreeString( efCS.szParticle );
				efCS.szParticle = DNULL;
			}
			break;

			case	EXP_BUGSPRAY_ALT:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 15.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 1.0f, 1.0f, 0.5f);
				erCS.fRadius	= 4500.0f;
				erCS.fPosRadius	= 1.0f;
				erCS.fVelocity	= 75.0f;
				erCS.fGravity	= 0.0f;
				erCS.nParticles	= 16;
				erCS.fDuration	= 0.5f;
				erCS.fAlpha		= 0.75f;
				erCS.fDelay		= 0.0f;
				erCS.bFadeType	= 1;
				erCS.bAlign		= 1;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\flames1_09.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 0.75f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 50.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_NAGA_EYE_BEAM:
			{
				EXPLOSIONMODELCS	emCS;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_SET(emCS.vScale1, 10.0f, 10.0f, 10.0f);
				VEC_SET(emCS.vScale2, 100.0f, 100.0f, 100.0f);
				emCS.fDuration		= 0.65f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= 1;
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= pClientDE->CreateString("Models\\Explosions\\exp_sphere.abc");
				emCS.szSkin			= pClientDE->CreateString("Skins\\Explosions\\divineball.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				g_pClientDE->FreeString( emCS.szModel );
				emCS.szModel = DNULL;
				g_pClientDE->FreeString( emCS.szSkin );
				emCS.szSkin = DNULL;
			}
			break;

			case	EXP_NAGA_SPIKE:
			{
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Smoke Ring
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_COPY(erCS.vPos, m_vPos);
				VEC_SET(erCS.vColor, 1.0f, 0.9f, 0.75f);
				erCS.fRadius		= 6000.0f;
				erCS.fPosRadius		= 10.0f;
				erCS.fVelocity		= GetRandom(100.0f,125.0f);
				erCS.fGravity		= GetRandom(25.0f,50.0f);
				erCS.nParticles		= 12;
				erCS.fDuration		= 1.25f;
				erCS.fAlpha			= 0.75f;
				erCS.bFadeType		= 1;
				erCS.bAlign			= 1;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);

				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 125.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_NAGA_STONE_CHUNK:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONFRAGCS		efCS;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 0.75f, 0.75f, 1.0f);
				VEC_SET(efCS.vRotateMax, 0.5f, 0.5f, 0.5f);
				efCS.fBounceMod		= 0.5f;
				efCS.fDuration		= 5.0f;
				efCS.fGravity		= 1.0f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.bRandDir		= 0;
				efCS.bSpawnExp		= 0;
				efCS.nSpawnType		= 0;
				efCS.nTrailType		= 0;

				char	string[64];
				efCS.szSkin			= pClientDE->CreateString("Skins\\Gibs\\gibmetal3.dtx");

				for(int i = 0; i < GetRandom(2,3); i++)
				{
					sprintf(string, "models\\gibs\\stone\\gib%d.abc", GetRandom(6,9));
					efCS.szModel		= pClientDE->CreateString(string);

					VEC_SET(efCS.vScale, GetRandom(1.0f, 2.0f), GetRandom(1.0f, 2.0f), GetRandom(1.0f, 2.0f));
					efCS.fSpread		= GetRandom(40.0f, 80.0f);
					efCS.fVelocity		= GetRandom(15.0f,30.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);

					g_pClientDE->FreeString( efCS.szModel );
					efCS.szModel = DNULL;
				}

				g_pClientDE->FreeString( efCS.szSkin );
				efCS.szSkin = DNULL;
			}
			break;

			case	EXP_NAGA_POUND_GROUND:
			{
				EXPLOSIONSPRITECS	esCS;

				// Shock wave
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 4.0f, 4.0f, 0.0f);
				esCS.fDuration	= 1.25f;
				esCS.fAlpha		= 0.5f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 1;
				esCS.szSprite	= pClientDE->CreateString("Sprites\\ripple.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

#ifdef _ADD_ON

			case	EXP_GAS_GRENADE:
			{
				EXPLOSIONMODELCS	emCS;
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONRINGCS		erCS;
				EXPLOSIONFRAGCS		efCS;
				EXPLOSIONLIGHTCS	elCS;
				DVector				temp;

				// Model
				VEC_COPY(emCS.vNormal, m_vNormal);
				VEC_COPY(emCS.vPos, m_vPos);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(emCS.vPos, m_vPos, temp);
				VEC_SET(emCS.vScale1, 50.0f, 50.0f, 50.0f);
				VEC_SET(emCS.vScale2, 400.0f, 400.0f, 400.0f);
				emCS.fDuration		= 0.5f;
				emCS.fAlpha			= 0.75f;
				emCS.bWaveForm		= GetRandom(0,2);
				emCS.bFadeType		= 1;
				emCS.bRandomRot		= 1;
				emCS.szModel		= 0;
				emCS.szSkin			= 0;

				psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &emCS, DFALSE, this);

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 0.5f, 0.5f, 0.0f);
				esCS.fDuration		= 1.5f;
				esCS.fAlpha			= 1.0f;
				esCS.bWaveForm		= 0;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites\\fireball2.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Smoke Ring 1
				VEC_COPY(erCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(erCS.vPos, m_vPos, temp);
				VEC_SET(erCS.vColor, 0.0f, 0.5f, 0.0f);
				erCS.fRadius		= 7500.0f;
				erCS.fPosRadius		= 10.0f;
				erCS.fVelocity		= GetRandom(100.0f,150.0f);
				erCS.fGravity		= GetRandom(0.0f,30.0f);
				erCS.nParticles		= 16;
				erCS.fDuration		= 1.5f;
				erCS.fAlpha			= 0.5f;
				erCS.fDelay			= GetRandom(0.0f,0.5f);
				erCS.bFadeType		= 1;
				erCS.bAlign			= 0;
				erCS.szParticle		= pClientDE->CreateString("SpriteTextures\\Smoke32_4.dtx");

				psfxMgr->CreateSFX(SFX_EXPLOSIONRING_ID, &erCS, DFALSE, this);
				g_pClientDE->FreeString( erCS.szParticle );
				erCS.szParticle = DNULL;

				// Light
				VEC_MULSCALAR(temp, m_vNormal, 10.0f);
				VEC_ADD(elCS.vPos, m_vPos, temp);
				VEC_SET(elCS.vColor1, 1.0f, 0.5f, 0.0f);
				VEC_SET(elCS.vColor2, 1.0f, 1.0f, 1.0f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 250.0f;
				elCS.fRadius2	= 0.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);

				// Smoke
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_MULSCALAR(temp, m_vNormal, 25.0f);
				VEC_ADD(esCS.vPos, m_vPos, temp);
				VEC_SET(esCS.vScale1, 0.5f, 0.5f, 0.0f);
				VEC_SET(esCS.vScale2, 3.0f, 3.0f, 0.0f);
				esCS.fDuration	= 15.0f;
				esCS.fAlpha		= 0.85f;
				esCS.bWaveForm	= 1;
				esCS.bFadeType	= 1;
				esCS.bAlign		= 0;
				esCS.szSprite	= pClientDE->CreateString("Sprites_ao\\greensmoke.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;
			}
			break;

			case	EXP_FLAYER_PRIMARY:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.0f, 0.0f, 0.0f);
				VEC_SET(esCS.vScale2, 0.35f, 0.35f, 0.0f);
				esCS.fDuration		= 2.0f;
				esCS.fAlpha			= 0.85f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites_ao\\greenrift.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 1.25f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 25.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLAYER_ALT:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;

				// Sprite
				VEC_SET(esCS.vNormal, 0.0f, 1.0f, 0.0f);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.0f, 0.0f, 0.0f);
				VEC_SET(esCS.vScale2, 0.45f, 0.45f, 0.0f);
				esCS.fDuration		= 22.0f;
				esCS.fAlpha			= 0.85f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 2;
				esCS.szSprite		= pClientDE->CreateString("Sprites_ao\\greenrift.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 20.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 125.0f;
				elCS.fRadius2	= 50.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLAYER_RETRACT:
			{
				EXPLOSIONSPRITECS	esCS;
				EXPLOSIONLIGHTCS	elCS;

				// Sprite
				VEC_COPY(esCS.vNormal, m_vNormal);
				VEC_COPY(esCS.vPos, m_vPos);
				VEC_SET(esCS.vScale1, 0.0f, 0.0f, 0.0f);
				VEC_SET(esCS.vScale2, 0.2f, 0.2f, 0.0f);
				esCS.fDuration		= 11.0f;
				esCS.fAlpha			= 0.85f;
				esCS.bWaveForm		= 1;
				esCS.bFadeType		= 1;
				esCS.bAlign			= 0;
				esCS.szSprite		= pClientDE->CreateString("Sprites_ao\\greenrift.spr");

				psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);

				g_pClientDE->FreeString( esCS.szSprite );
				esCS.szSprite = DNULL;

				// Light
				VEC_COPY(elCS.vPos, m_vPos);
				VEC_SET(elCS.vColor1, 0.0f, 0.75f, 0.0f);
				VEC_SET(elCS.vColor2, 0.5f, 0.5f, 0.5f);
				elCS.fDuration	= 10.0f;
				elCS.fDelay		= 0.0f;
				elCS.fRadius1	= 75.0f;
				elCS.fRadius2	= 25.0f;

				psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &elCS, DFALSE, this);
			}
			break;

			case	EXP_FLAYER_SHATTER:
			{
				EXPLOSIONFRAGCS		efCS;

				// Fragments
				VEC_COPY(efCS.vPos, m_vPos);
				VEC_COPY(efCS.vNormal, m_vNormal);
				VEC_SET(efCS.vScale, 5.0f, 5.0f, 5.0f);
				VEC_SET(efCS.vRotateMax, 1.0f, 1.0f, 1.0f);
				efCS.fBounceMod		= 0.5f;
				efCS.fDuration		= 7.0f;
				efCS.fGravity		= 0.2f;
				efCS.fFadeTime		= 2.0f;
				efCS.fInitAlpha		= 1.0f;
				efCS.szModel		= pClientDE->CreateString("Models_ao\\Ammo_ao\\chainlink.abc");
				efCS.szSkin			= pClientDE->CreateString("Skins_ao\\Ammo_ao\\chainlink.dtx");

				for(int i = 0; i < 6; i++)
				{
					efCS.fSpread		= GetRandom(2.0f, 15.0f);
					efCS.fVelocity		= GetRandom(3.0f,6.0f);

					psfxMgr->CreateSFX(SFX_EXPLOSIONFRAG_ID, &efCS, DFALSE, this);
				}
				g_pClientDE->FreeString( efCS.szModel );
				g_pClientDE->FreeString( efCS.szSkin );
			}
			break;
#endif
		}
	}
	return DTRUE;
}