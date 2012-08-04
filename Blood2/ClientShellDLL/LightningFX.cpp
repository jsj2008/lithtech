// ----------------------------------------------------------------------- //
//
// MODULE  : LightningFX.cpp
//
// PURPOSE : Special FX class for lightning-like instant particle streams
//
// CREATED : 8/1/98
//
// ----------------------------------------------------------------------- //

#include "LightningFX.h"
#include "LightningSegmentFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "BloodClientShell.h"
#include "SFXMsgIds.h"
#include "ExplosionFX.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningBoltFX::Init
//
//	PURPOSE:	Init the stream
//
// ----------------------------------------------------------------------- //

DBOOL CLightningBoltFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	LIGHTNINGCREATESTRUCT	*pLS = (LIGHTNINGCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vSource, pLS->vSource);
	VEC_COPY(m_vDest, pLS->vDest);
	m_nShape			= pLS->nShape;
	m_nForm				= pLS->nForm;
	m_nType				= pLS->nType;

	SetupLightning();
	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningBoltFX::SetupLightning()
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

void CLightningBoltFX::SetupLightning()
{
	m_fMinRadius	= 300.0f;
	m_fMaxRadius	= 3000.0f;
	m_fDensity		= 2.5f;

	m_fDuration		= 0.25f;
	m_fFadeTime		= 0.15f;

	VEC_SET(m_vMinColor, 100.0f, 100.0f, 255.0f);
	VEC_SET(m_vMaxColor, 255.0f, 255.0f, 255.0f);
	m_fAlpha		= 0.75f;

	m_nNumLights	= 3;
	VEC_SET(m_vLightColor, 255.0f, 255.0f, 255.0f);
	m_fLightRadius	= 200.0f;

	m_nLightPos		= LIGHTNING_LIGHTS_SOURCE;

	m_fStartTime	= 0.0f;
	m_hstrTexture	= "SpriteTextures\\lightn32.dtx";
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningBoltFX::CreateObject()
//
//	PURPOSE:	Create the particle system
//
// ----------------------------------------------------------------------- //

DBOOL CLightningBoltFX::CreateObject(CClientDE* pClientDE)
{
	if(!pClientDE)		return DFALSE;
	m_fStartTime = pClientDE->GetTime();

	return CSpecialFX::CreateObject(pClientDE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningBoltFX::Update
//
//	PURPOSE:	Update the particles
//
// ----------------------------------------------------------------------- //

DBOOL CLightningBoltFX::Update()
{
	CreateLightning();
	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLightningBoltFX::CreateLightning()
//
//	PURPOSE:	Make the lightning segments
//
// ----------------------------------------------------------------------- //

DBOOL CLightningBoltFX::CreateLightning()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	CSFXMgr* psfxMgr = pShell->GetSFXMgr();

	if(!psfxMgr || !m_pClientDE)		return DFALSE;

	LSEGMENTCREATESTRUCT	lscs;
	DVector		vTemp;
	DVector		vU, vR, vF;
	DRotation	rRot;
	DFLOAT		fMag, fSegMag, fDirMag;
	DDWORD		m_nNumSegments;

	// Set all the consistant variables for each segment
	lscs.fAlpha = m_fAlpha;
	lscs.fDuration = m_fDuration;
	lscs.fFadeTime = m_fFadeTime;
	lscs.hstrTexture = m_pClientDE->CreateString(m_hstrTexture);

	// Calculate the magnitude
	VEC_SUB(vTemp, m_vDest, m_vSource);
	fMag = VEC_MAG(vTemp);

	// Calculate the min and max offset based on the length
	m_fMaxOffset = fMag / LIGHTNING_OFFSET_RATIO;
	m_fMinOffset = m_fMaxOffset / LIGHTNING_OFFSET_RATIO;

	// Get a rotation to follow the path of
	VEC_SET(vU, 0.0f, 1.0f, 0.0f);
	m_pClientDE->AlignRotation(&rRot, &vTemp, &vU);

	switch(m_nType)
	{
		case LIGHTNING_TYPE_LOWSEG:
			m_nNumSegments = (DDWORD)(fMag / LIGHTNING_SEGMAG_LOW);
			break;
		case LIGHTNING_TYPE_MEDSEG:
			m_nNumSegments = (DDWORD)(fMag / LIGHTNING_SEGMAG_MED);
			break;
		case LIGHTNING_TYPE_HIGHSEG:
			m_nNumSegments = (DDWORD)(fMag / LIGHTNING_SEGMAG_HIGH);
			break;
	}

	if(m_nNumSegments < 5)		m_nNumSegments = 5;
	if(m_nNumSegments > 25)		m_nNumSegments = 25;
	fSegMag = fMag / (float)m_nNumSegments;

	m_fMaxRadius = m_fMaxRadius * (m_nNumSegments / 5);

	//****************************************************************************//
	// Calculate the other variables that will be different for each segment
	DFLOAT		fOffset;
	DFLOAT		radiusRatio = (m_fMaxRadius - m_fMinRadius) / (float)m_nNumSegments;
	DFLOAT		offsetRatio = (m_fMaxOffset - m_fMinOffset) / (float)m_nNumSegments;
	DFLOAT		colorXRatio = (m_vMaxColor.x - m_vMinColor.x) / (float)m_nNumSegments;
	DFLOAT		colorYRatio = (m_vMaxColor.y - m_vMinColor.y) / (float)m_nNumSegments;
	DFLOAT		colorZRatio = (m_vMaxColor.z - m_vMinColor.z) / (float)m_nNumSegments;
	DVector		vDir;

	VEC_COPY(lscs.vOffset, m_vSource);
	VEC_COPY(lscs.vNextOffset, m_vSource);

	if(m_nShape == LIGHTNING_SHAPE_RANDOM)	m_nShape = GetRandom(LIGHTNING_SHAPE_MIN, LIGHTNING_SHAPE_MAX);
	if(m_nForm == LIGHTNING_FORM_RANDOM)	m_nType = GetRandom(LIGHTNING_FORM_MIN, LIGHTNING_FORM_MAX);

	for(DDWORD i = 0; i < m_nNumSegments; i++)
	{
		// Set the variables values for the type of lightning to use
		switch(m_nForm)
		{
			case	LIGHTNING_FORM_WIDE2THIN:
				lscs.fRadius = m_fMaxRadius - ((float)i * radiusRatio);
				fOffset = m_fMaxOffset - ((float)i * offsetRatio);

				lscs.vColor.x = m_vMaxColor.x - ((float)i * colorXRatio);
				lscs.vColor.y = m_vMaxColor.y - ((float)i * colorYRatio);
				lscs.vColor.z = m_vMaxColor.z - ((float)i * colorZRatio);
				break;

			case	LIGHTNING_FORM_THIN2WIDE:
				lscs.fRadius = m_fMinRadius + ((float)i * radiusRatio);
				fOffset = m_fMinOffset + ((float)i * offsetRatio);

				lscs.vColor.x = m_vMinColor.x + ((float)i * colorXRatio);
				lscs.vColor.y = m_vMinColor.y + ((float)i * colorYRatio);
				lscs.vColor.z = m_vMinColor.z + ((float)i * colorZRatio);
				break;

			case	LIGHTNING_FORM_THIN2THIN:
			{
				float	j;
				if(i < (m_nNumSegments >> 1))	j = (float)(i << 1);
					else						j = (float)((m_nNumSegments - 1 - i) << 1);

				lscs.fRadius = m_fMinRadius + (j * radiusRatio);
				fOffset = m_fMinOffset + (j * offsetRatio);

				lscs.vColor.x = m_vMinColor.x + (j * colorXRatio);
				lscs.vColor.y = m_vMinColor.y + (j * colorYRatio);
				lscs.vColor.z = m_vMinColor.z + (j * colorZRatio);
				break;
			}

			case	LIGHTNING_FORM_WIDE2WIDE:
			{
				float	j;
				if(i < (m_nNumSegments >> 1))	j = (float)(i << 1);
					else						j = (float)((m_nNumSegments - 1 - i) << 1);

				lscs.fRadius = m_fMaxRadius - (j * radiusRatio);
				fOffset = m_fMaxOffset - (j * offsetRatio);

				lscs.vColor.x = m_vMaxColor.x - (j * colorXRatio);
				lscs.vColor.y = m_vMaxColor.y - (j * colorYRatio);
				lscs.vColor.z = m_vMaxColor.z - (j * colorZRatio);
				break;
			}

			default:
				lscs.fRadius = m_fMaxRadius - ((float)i * radiusRatio);
				fOffset = m_fMaxOffset - ((float)i * offsetRatio);

				lscs.vColor.x = m_vMaxColor.x - ((float)i * colorXRatio);
				lscs.vColor.y = m_vMaxColor.y - ((float)i * colorYRatio);
				lscs.vColor.z = m_vMaxColor.z - ((float)i * colorZRatio);
				break;
		}

		// Set the variables values for the shape of lightning to use
		switch(m_nShape)
		{
			case	LIGHTNING_SHAPE_FLAT:
				m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
				VEC_MULSCALAR(vR, vR, GetRandom(-fOffset, fOffset));
				VEC_MULSCALAR(vF, vF, fSegMag * (float)(i + 1));
				VEC_ADD(lscs.vNextOffset, m_vSource, vF);
				VEC_ADD(lscs.vNextOffset, lscs.vNextOffset, vR);
				break;

			case	LIGHTNING_SHAPE_BOXED:
				m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
				VEC_MULSCALAR(vU, vU, GetRandom(-fOffset, fOffset));
				VEC_MULSCALAR(vR, vR, GetRandom(-fOffset, fOffset));
				VEC_MULSCALAR(vF, vF, fSegMag * (float)(i + 1));
				VEC_ADD(lscs.vNextOffset, m_vSource, vF);
				VEC_ADD(lscs.vNextOffset, lscs.vNextOffset, vR);
				VEC_ADD(lscs.vNextOffset, lscs.vNextOffset, vU);
				break;

			case	LIGHTNING_SHAPE_SPIRAL:
			{
				m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
				m_pClientDE->RotateAroundAxis(&rRot, &vF, i * (4.0f * MATH_PI / (float)(m_nNumSegments - 1)));
				m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
				VEC_MULSCALAR(vR, vR, fOffset);
				VEC_MULSCALAR(vF, vF, fSegMag * (float)(i + 1));
				VEC_ADD(lscs.vNextOffset, m_vSource, vF);
				VEC_ADD(lscs.vNextOffset, lscs.vNextOffset, vR);
				break;
			}

			case	LIGHTNING_SHAPE_STAR:
			{
				m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
				m_pClientDE->RotateAroundAxis(&rRot, &vF, (i % 5) * 4.0f * (MATH_PI / 5));
				m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
				VEC_MULSCALAR(vR, vR, fOffset);
				VEC_MULSCALAR(vF, vF, fSegMag * (float)(i + 1));
				VEC_ADD(lscs.vNextOffset, m_vSource, vF);
				VEC_ADD(lscs.vNextOffset, lscs.vNextOffset, vR);
				break;
			}

			default:
				m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);
				VEC_MULSCALAR(vU, vU, GetRandom(-fOffset, fOffset));
				VEC_MULSCALAR(vR, vR, GetRandom(-fOffset, fOffset));
				VEC_MULSCALAR(vF, vF, fSegMag * (float)(i + 1));
				VEC_ADD(lscs.vNextOffset, m_vSource, vF);
				VEC_ADD(lscs.vNextOffset, lscs.vNextOffset, vR);
				VEC_ADD(lscs.vNextOffset, lscs.vNextOffset, vU);
				break;
		}

		if(i == m_nNumSegments - 1)
			{ VEC_COPY(lscs.vNextOffset, m_vDest); }

		// Calculate the direction the particles need to travel from source to dest offsets
		VEC_SUB(vDir, lscs.vNextOffset, lscs.vOffset);
		fDirMag = VEC_MAG(vDir);

		// Determine how many particles need to be drawn to fill the gap
		lscs.nNumParticles = (DDWORD)(fDirMag / (m_fDensity * lscs.fRadius / 1000.0f));

		// Scale the direction offset based on the magnitude and particle count
		lscs.fIncrement = (fDirMag / (float)lscs.nNumParticles);

		// Create the particle segment
 		CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_LIGHTNINGSEG_ID, &lscs, DFALSE, this);

		// Place the offset at the previously calculated next offset
		VEC_COPY(lscs.vOffset, lscs.vNextOffset);
	}

	//****************************************************************************//
	// Variables to create the lights
	EXPLOSIONLIGHTCS	el;
	VEC_MULSCALAR(el.vColor1, m_vLightColor, 1.0f/255.0f);
	VEC_MULSCALAR(el.vColor2, m_vLightColor, 1.0f/255.0f);
	el.fDelay		= 0.0f;
	el.fDuration	= m_fDuration * 2.0f;
	el.fRadius1		= m_fLightRadius;
	el.fRadius2		= m_fLightRadius / 2.0f;

	// Create the lights of the lightning bolt
	for(i = 0; i < m_nNumLights; i++)
	{
		DVector		lightVec;
		DVector		vLightPos;

		switch(m_nLightPos)
		{
			case	LIGHTNING_LIGHTS_SOURCE:
				if(m_nNumLights == 1)
					{ VEC_COPY(el.vPos, m_vSource); }
				else
				{
					VEC_SUB(lightVec, m_vDest, m_vSource);
					VEC_MULSCALAR(vLightPos, lightVec, (float)i / (float)(m_nNumLights - 1));
					VEC_ADD(el.vPos, m_vSource, vLightPos);
				}
				break;

			case	LIGHTNING_LIGHTS_CENTER:
				VEC_SUB(lightVec, m_vDest, m_vSource);
				VEC_MULSCALAR(vLightPos, lightVec, (float)(i + 1) / (float)(m_nNumLights + 1));
				VEC_ADD(el.vPos, m_vSource, vLightPos);
				break;

			case	LIGHTNING_LIGHTS_DEST:
				if(m_nNumLights == 1)
					{ VEC_COPY(el.vPos, m_vDest); }
				else
				{
					VEC_SUB(lightVec, m_vSource, m_vDest);
					VEC_MULSCALAR(vLightPos, lightVec, (float)i / (float)(m_nNumLights - 1));
					VEC_ADD(el.vPos, m_vDest, vLightPos);
				}
				break;
		}

		// Create the particle segment
 		CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSIONLIGHT_ID, &el, DFALSE, this);
	}

	return DTRUE;
}