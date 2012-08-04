// ----------------------------------------------------------------------- //
//
// MODULE  : BaseParticleSystemFX.cpp
//
// PURPOSE : BaseParticleSystem special FX - Implementation
//
// CREATED : 10/21/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BaseParticleSystemFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::Init
//
//	PURPOSE:	Init the base particle system
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseParticleSystemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_basecs		= *((BPSCREATESTRUCT*)psfxCreateStruct);

	m_fGravity		= PSFX_DEFAULT_GRAVITY;
	m_fRadius		= PSFX_DEFAULT_RADIUS;
	m_dwFlags		= 0;
	m_pTextureName	= "SFX\\Particle\\particle.dtx";

	m_vPos.Init();
	m_vPosOffset.Init();
	m_vVel.Init();
    m_rRot.Init();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::CreateObject
//
//	PURPOSE:	Create object associated the particle system.
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseParticleSystemFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

    LTVector vPos = m_vPos;
    LTRotation rRot;

	// Use server object position if a position wasn't specified...

	if (m_hServerObject)
	{
        LTVector vZero(0, 0, 0), vServObjPos;
        if( vPos.NearlyEquals(vZero, 0.0f))
        {
			g_pLTClient->GetObjectPos(m_hServerObject, &vServObjPos);
			vPos = vServObjPos;
		}
		else
		{
            m_basecs.bClientControlsPos = LTTRUE;
		}

		// Calculate our offset from the server object...

		m_vPosOffset = vPos - vServObjPos;
	}

	// Use the specified rotation if applicable

	if (m_rRot != LTRotation(0.0f, 0.0f, 0.0f, 1.0f))
	{
		rRot = m_rRot;
	}


	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_PARTICLESYSTEM;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_UPDATEUNSEEN | FLAG_FOGDISABLE;
	createStruct.m_Pos = vPos;
	createStruct.m_Rotation = rRot;

	m_hObject = m_pClientDE->CreateObject(&createStruct);

	// Setup the ParticleSystem...

	return SetupSystem();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::SetupSystem
//
//	PURPOSE:	Setup the particle system...
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseParticleSystemFX::SetupSystem()
{
    if (!m_hObject || !m_pClientDE) return LTFALSE;

    uint32 dwWidth, dwHeight;
	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	m_pClientDE->GetSurfaceDims (hScreen, &dwWidth, &dwHeight);
    if (dwWidth < 1) return LTFALSE;

    m_fRadius /= ((LTFLOAT)dwWidth);

	m_pClientDE->SetupParticleSystem(m_hObject, m_pTextureName,
									 m_fGravity, m_dwFlags, m_fRadius);

	// Set blend modes if applicable...

    uint32 dwFlags2 = 0;

	if (m_basecs.bAdditive)
	{
		dwFlags2 |= FLAG2_ADDITIVE;
	}
	else if (m_basecs.bMultiply)
	{
		dwFlags2 |= FLAG2_MULTIPLY;
	}
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags2, dwFlags2);


	VEC_SET(m_vColorRange, m_vColor2.x - m_vColor1.x,
						   m_vColor2.y - m_vColor1.y,
						   m_vColor2.z - m_vColor1.z);

	if (m_vColorRange.x < 0.0f) m_vColorRange.x = 0.0f;
	if (m_vColorRange.y < 0.0f) m_vColorRange.y = 0.0f;
	if (m_vColorRange.z < 0.0f) m_vColorRange.z = 0.0f;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::Update
//
//	PURPOSE:	Update the particle system
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseParticleSystemFX::Update()
{
    if (!CSpecialFX::Update() || !m_hObject || !m_pClientDE) return LTFALSE;
	
	if( g_pGameClientShell->IsServerPaused() )
		return LTTRUE;

	// See if we should rotate this bad-boy...

	if (m_vRotVel.x != 0.0f || m_vRotVel.y != 0.0f || m_vRotVel.z != 0.0f)
	{
        LTFLOAT fDelta = g_pGameClientShell->GetFrameTime();

        LTRotation rRot;
		g_pLTClient->GetObjectRotation(m_hObject, &rRot);

		m_vRotAmount += m_vRotVel * fDelta;

		if (m_vRotVel.x != 0.0f) rRot.Rotate(rRot.Right(), m_vRotAmount.x);
		if (m_vRotVel.y != 0.0f) rRot.Rotate(rRot.Up(), m_vRotAmount.y);
		if (m_vRotVel.z != 0.0f) rRot.Rotate(rRot.Forward(), m_vRotAmount.z);

		g_pLTClient->SetObjectRotation(m_hObject, &rRot);
	}


	// Update each particles scale / alpha if necessary...

    LTParticle *pCur, *pTail;

	if (m_basecs.bAdjustParticleScale || m_basecs.bAdjustParticleAlpha)
	{
		if (m_pClientDE->GetParticles(m_hObject, &pCur, &pTail))
		{
            LTFLOAT fLifetime = 0.0f, fTotalLifetime = 0.0f;
            LTFLOAT fAlphaRange = m_basecs.fEndParticleAlpha - m_basecs.fStartParticleAlpha;
            LTFLOAT fScaleRange = m_basecs.fEndParticleScale - m_basecs.fStartParticleScale;
            LTVector vColorRange = m_vColor2 - m_vColor1;

			while (pCur && pCur != pTail)
			{
				fLifetime = pCur->m_Lifetime;
				fTotalLifetime = pCur->m_TotalLifetime;

				if (fLifetime > 0.0f && fTotalLifetime > 0.0f)
				{
                    LTFLOAT fLifePercent = 1.0f - (fLifetime / fTotalLifetime);

					// Adjust scale...

					if (m_basecs.bAdjustParticleScale)
					{
						pCur->m_Size = m_fRadius * (m_basecs.fStartParticleScale + (fScaleRange * fLifePercent));
					}

					// Adjust alpha...

					if (m_basecs.bAdjustParticleAlpha)
					{
						pCur->m_Alpha = m_basecs.fStartParticleAlpha + (fAlphaRange * fLifePercent);
					}
				}

				pCur = pCur->m_pNext;
			}
		}
	}


	// Make sure we update our position relative to the server object
	// (if the server object is valid and the client isn't controling
	// the particle system pos)...

	if (m_hServerObject && !m_basecs.bClientControlsPos)
	{
        LTVector vNewPos;
		g_pLTClient->GetObjectPos(m_hServerObject, &vNewPos);
		vNewPos += m_vPosOffset;
		g_pLTClient->SetObjectPos(m_hObject, &vNewPos);
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::GetRandomColorInRange
//
//	PURPOSE:	Get a random color in our color range
//
// ----------------------------------------------------------------------- //

void CBaseParticleSystemFX::GetRandomColorInRange(LTVector & vColor)
{
    LTFLOAT fColorR = GetRandom(m_vColor1.x, m_vColor2.x);

	if (m_vColorRange.x <= 0.0f)
	{
		VEC_COPY(vColor, m_vColor1);
	}
	else
	{
		vColor.x = fColorR;
		vColor.y = (m_vColorRange.y * fColorR) / m_vColorRange.x;
		vColor.z = (m_vColorRange.z * fColorR) / m_vColorRange.x;
	}

	return;
}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::GetNumParticles
//
//	PURPOSE:	Get num particles to add based on the camera pos
//
// ----------------------------------------------------------------------- //

int CBaseParticleSystemFX::GetNumParticles(int nDefaultNum)
{
    LTFLOAT fNum = (LTFLOAT) nDefaultNum;

	if (m_basecs.bRelToCameraPos)
	{
		HLOCALOBJ hCamera = g_pPlayerMgr->GetCamera();
		if (hCamera)
		{
            LTVector vCamPos, vDist, vPos;
			g_pLTClient->GetObjectPos(m_hObject, &vPos);
			g_pLTClient->GetObjectPos(hCamera, &vCamPos);
			vDist = vCamPos - vPos;

            LTFLOAT fInnerSqr = m_basecs.fInnerCamRadius*m_basecs.fInnerCamRadius;
            LTFLOAT fOuterSqr = m_basecs.fOuterCamRadius*m_basecs.fOuterCamRadius;
            LTFLOAT fDistSqr  = vDist.MagSqr();

			if (fDistSqr > fInnerSqr)
			{
                LTFLOAT  fDistFromInnerSqr = fDistSqr - fInnerSqr;
                LTFLOAT  fRangeSqr = fOuterSqr - fInnerSqr;
				fNum *= (1.0f - (fDistFromInnerSqr / fRangeSqr));
				fNum = fNum < 1.0f ? 1.0f : fNum;
			}
		}
	}

	return (int)fNum;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseParticleSystemFX::RemoveAllParticles
//
//	PURPOSE:	Remove all the particles in system
//
// ----------------------------------------------------------------------- //

void CBaseParticleSystemFX::RemoveAllParticles()
{
	if (!m_hObject || !m_pClientDE) return;

    LTParticle *pCur, *pTail, *pNext;

	if (m_pClientDE->GetParticles(m_hObject, &pCur, &pTail))
	{
		while (pCur && pCur != pTail)
		{
			pNext = pCur->m_pNext;
			m_pClientDE->RemoveParticle(m_hObject, pCur);
			pCur = pNext;
		}
	}
}