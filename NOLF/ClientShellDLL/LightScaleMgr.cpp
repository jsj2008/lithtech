#include "stdafx.h"
#include "iltclient.h"
#include "LightScaleMgr.h"

LTBOOL CLightScaleMgr::Init()
{
	m_TimeOfDayScale.Init(1, 1, 1);

    return LTTRUE;
}

void CLightScaleMgr::Term()
{
	LS_EFFECT* pEffect = m_pEffects;
    LS_EFFECT* pPrev = LTNULL;
	while (pEffect)
	{
		pPrev = pEffect;
		pEffect = pEffect->pNext;
		GetLSEffectBank()->Delete(pPrev);
	}

    m_pEffects = LTNULL;

	m_TimeOfDayScale.Init(1, 1, 1);

	SetLightScale();
}

CBankedList<LS_EFFECT> *CLightScaleMgr::GetLSEffectBank()
{
	static CBankedList<LS_EFFECT> theBank;
	return &theBank;
}

void CLightScaleMgr::SetTimeOfDayScale(LTVector &scale)
{
	m_TimeOfDayScale = scale;
	SetLightScale();
}

void CLightScaleMgr::SetLightScale (LTFLOAT nRed, LTFLOAT nGreen, LTFLOAT nBlue, LightEffectType eType)
{
	// add this to the list of light scales

	LS_EFFECT* pEffect = GetLSEffectBank()->New();
	pEffect->nRed = nRed;
	pEffect->nGreen = nGreen;
	pEffect->nBlue = nBlue;
	pEffect->eType = eType;

	pEffect->pNext = m_pEffects;
	m_pEffects = pEffect;

	// set the correct global light scale

	SetLightScale();
}

void CLightScaleMgr::ClearLightScale (LTFLOAT nRed, LTFLOAT nGreen, LTFLOAT nBlue, LightEffectType eType)
{
	LS_EFFECT* pEffect = m_pEffects;
    LS_EFFECT* pPrev = LTNULL;
	while (pEffect)
	{
		if (pEffect->nRed == nRed && pEffect->nGreen == nGreen && pEffect->nBlue == nBlue && pEffect->eType == eType)
		{
			if (pPrev)
			{
				pPrev->pNext = pEffect->pNext;
				GetLSEffectBank()->Delete(pEffect);
			}
			else
			{
				m_pEffects = pEffect->pNext;
				GetLSEffectBank()->Delete(pEffect);
			}

			break;
		}

		pPrev = pEffect;
		pEffect = pEffect->pNext;
	}

	SetLightScale();
}

void CLightScaleMgr::SetLightScale()
{
	// go though looking for the first interface type

	LS_EFFECT* pEffect = m_pEffects;
	while (pEffect)
	{
		if (pEffect->eType == LightEffectInterface)
		{
            LTVector vLightScale;
			vLightScale.x = pEffect->nRed;
			vLightScale.y = pEffect->nGreen;
			vLightScale.z = pEffect->nBlue;
            g_pLTClient->SetGlobalLightScale (&vLightScale);
			return;
		}
		pEffect = pEffect->pNext;
	}

	// go though looking for the first Damage type

	pEffect = m_pEffects;
	while (pEffect)
	{
		if (pEffect->eType == LightEffectDamage)
		{
            LTVector vLightScale;
			vLightScale.x = pEffect->nRed;
			vLightScale.y = pEffect->nGreen;
			vLightScale.z = pEffect->nBlue;
            g_pLTClient->SetGlobalLightScale (&vLightScale);
			return;
		}
		pEffect = pEffect->pNext;
	}

	// go though looking for the first powerup type

	pEffect = m_pEffects;
	while (pEffect)
	{
		if (pEffect->eType == LightEffectPowerup)
		{
            LTVector vLightScale;
			vLightScale.x = pEffect->nRed;
			vLightScale.y = pEffect->nGreen;
			vLightScale.z = pEffect->nBlue;
            g_pLTClient->SetGlobalLightScale (&vLightScale);
			return;
		}
		pEffect = pEffect->pNext;
	}

	// go through looking for the first environment type

	pEffect = m_pEffects;
	while (pEffect)
	{
		if (pEffect->eType == LightEffectEnvironment)
		{
            LTVector vLightScale;
			vLightScale.x = pEffect->nRed;
			vLightScale.y = pEffect->nGreen;
			vLightScale.z = pEffect->nBlue;

			vLightScale.x *= m_TimeOfDayScale.x;
			vLightScale.y *= m_TimeOfDayScale.y;
			vLightScale.z *= m_TimeOfDayScale.z;

            g_pLTClient->SetGlobalLightScale (&vLightScale);
			return;
		}
		pEffect = pEffect->pNext;
	}

	// go through looking for the first world type

	pEffect = m_pEffects;
	while (pEffect)
	{
		if (pEffect->eType == LightEffectWorld)
		{
            LTVector vLightScale;
			vLightScale.x = pEffect->nRed;
			vLightScale.y = pEffect->nGreen;
			vLightScale.z = pEffect->nBlue;

			vLightScale.x *= m_TimeOfDayScale.x;
			vLightScale.y *= m_TimeOfDayScale.y;
			vLightScale.z *= m_TimeOfDayScale.z;

            g_pLTClient->SetGlobalLightScale (&vLightScale);
			return;
		}
		pEffect = pEffect->pNext;
	}

	// we got all the way through without finding a suitable light effect - just set the light scale to (1,1,1)

    LTVector vec;
	VEC_SET (vec, 1.0f, 1.0f, 1.0f);

	vec.x *= m_TimeOfDayScale.x;
	vec.y *= m_TimeOfDayScale.y;
	vec.z *= m_TimeOfDayScale.z;

    g_pLTClient->SetGlobalLightScale (&vec);
}


void CLightScaleMgr::ClearAllLightScales ()
{
	while (m_pEffects)
	{
		ClearLightScale(m_pEffects->nRed,m_pEffects->nGreen,m_pEffects->nBlue,m_pEffects->eType);
	}
	Disable();

}