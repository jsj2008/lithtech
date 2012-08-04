// ----------------------------------------------------------------------- //
//
// MODULE  : ExplosionFX.cpp
//
// PURPOSE : Explosion special FX - Implementation
//
// CREATED : 5/27/98
//
// ----------------------------------------------------------------------- //

#include "ExplosionFX.h"
#include "clientheaders.h"


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::Init
//
//	PURPOSE:	Init the explosion
//
// ----------------------------------------------------------------------- //

LTBOOL CExplosionFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CBaseScaleFX::Init(psfxCreateStruct)) return LTFALSE;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::CreateObject
//
//	PURPOSE:	Create object associated with the CExplosionFX
//
// ----------------------------------------------------------------------- //

LTBOOL CExplosionFX::CreateObject(ILTClient *pClientDE)
{
	if (!CBaseScaleFX::CreateObject(pClientDE)) return LTFALSE;

	return LTTRUE;
}

#ifdef EXPLOSION_SHEEYET

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::AddBlastMark()
//
//	PURPOSE:	Create blast mark
//
// ----------------------------------------------------------------------- //

void CExplosionFX::AddBlastMark()
{
	if (!m_pClientDE) return;

	// Actually, create a dynamic light for the blast mark :)

	DLCREATESTRUCT dl;

	VEC_COPY(dl.vPos, m_vPos);
	VEC_SET(dl.vColor, 0.3f, 0.3f, 0.3f);

	dl.fMinRadius    = 50.0f;
	dl.fMaxRadius	 = 100.0f;
	dl.fRampUpTime	 = 0.3f;
	dl.fMaxTime		 = 0.1f;
	dl.fMinTime		 = 0.0f;
	dl.fRampDownTime = 0.2f;

	LTVector vColor;
	VEC_SET(vColor, 1.0f, 1.0f, 1.0f);

	// FLAG_VISIBLE | FLAG_ONLYLIGHTWORLD;

	// Set light values...

	VEC_SET(pMark->m_vColor1, 70.0f, 70.0f, 70.0f);

	pMark->m_nNumRadiusCycles		= 1;
    pMark->m_fRadiusMin				= m_fMinLightRadius/4.0f;
    pMark->m_fRadiusMax				= m_fMaxLightRadius/4.0f;
	pMark->m_fRadiusMinTime			= 0.0f;
	pMark->m_fRadiusMaxTime			= 30.0f;
	pMark->m_fRadiusRampUpTime	    = m_fDuration*0.2f;
	pMark->m_fRadiusRampDownTime	= 60.0f;
	pMark->m_fLifeTime				= (pMark->m_fRadiusMinTime + 
									   pMark->m_fRadiusMaxTime + 
									   pMark->m_fRadiusRampUpTime +
									   pMark->m_fRadiusRampDownTime);
							
	pMark->Init();


	// Create center of blast mark...

	INIT_OBJECTCREATESTRUCT(theStruct);

	VEC_COPY(theStruct.m_Pos, *pvPos);
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_ONLYLIGHTWORLD;

	pMark = (LightFX*)m_pClientDE->CreateObject(hClass, &theStruct);
	if (!pMark) return;

	// Set light values...

	VEC_MULSCALAR(pMark->m_vColor1, m_vLightColor, 255.0f);

	pMark->m_nNumRadiusCycles		= 1;
    pMark->m_fRadiusMin				= m_fMinLightRadius/12.0f;
    pMark->m_fRadiusMax				= m_fMaxLightRadius/12.0f;
	pMark->m_fRadiusMinTime			= 0.0f;
	pMark->m_fRadiusMaxTime			= 1.0f;
	pMark->m_fRadiusRampUpTime	    = m_fDuration*0.2f;
	pMark->m_fRadiusRampDownTime	= 5.0f;
	pMark->m_fLifeTime				= (pMark->m_fRadiusMinTime + 
									   pMark->m_fRadiusMaxTime + 
									   pMark->m_fRadiusRampUpTime +
									   pMark->m_fRadiusRampDownTime);
							
	pMark->Init();

}
#endif
