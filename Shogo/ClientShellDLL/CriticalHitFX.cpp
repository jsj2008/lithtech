// ----------------------------------------------------------------------- //
//
// MODULE  : CriticalHitFX.cpp
//
// PURPOSE : Explosion special FX - Implementation
//
// CREATED : 7/28/98
//
// ----------------------------------------------------------------------- //

#include "CriticalHitFX.h"
#include "RiotClientShell.h"
#include "clientheaders.h"
#include "ExplosionFX.h"
#include "DynamicLightFX.h"
#include "InfoDisplay.h"
#include "ClientRes.h"

extern CRiotClientShell* g_pRiotClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCriticalHitFX::Init
//
//	PURPOSE:	Init the explosion
//
// ----------------------------------------------------------------------- //

LTBOOL CCriticalHitFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	CHCREATESTRUCT* pCH = (CHCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vPos, pCH->vPos);
	m_nClientIDHitter = (int)pCH->fClientIDHitter;
	m_nClientIDHittee = (int)pCH->fClientIDHittee;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCriticalHitFX::CreateObject
//
//	PURPOSE:	Create object associated with the CCriticalHitFX
//
// ----------------------------------------------------------------------- //

LTBOOL CCriticalHitFX::CreateObject(ILTClient *pClientDE)
{
	if (!pClientDE || m_nClientIDHitter == m_nClientIDHittee) return LTFALSE;

	uint32 dwId;
	if (pClientDE->GetLocalClientID(&dwId) != LT_OK) return LTFALSE;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return LTFALSE;


	// Create model...

	EXCREATESTRUCT ex;
	VEC_COPY(ex.vPos, m_vPos);

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, 100.0f, 100.0f, 100.0f);
	VEC_SET(ex.vFinalScale, 150.0f, 150.f, 150.f);
	VEC_SET(ex.vInitialColor, 1.0f, 0.0f, 0.5f);
	VEC_SET(ex.vFinalColor, 1.0f, 0.0f, 0.5f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 0.5f;
	ex.fInitialAlpha	= 0.5f;
	ex.fFinalAlpha		= 0.5f;
	ex.pFilename		= "Models\\PV_Weapons\\Explosion.abc";
	ex.pSkin			= "SpecialFX\\ParticleTextures\\particle.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();


	// Create dynamic light...

	DLCREATESTRUCT dl;
	VEC_COPY(dl.vPos, m_vPos);

	// Initialize for vector weapon...

	dl.fMinRadius    = 50.0f;
	dl.fMaxRadius	 = 250.0f;
	dl.fRampUpTime	 = 0.15f;
	dl.fMaxTime		 = 0.0f;
	dl.fMinTime		 = 0.0f;
	dl.fRampDownTime = 0.35f;
	dl.dwFlags		 = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;

	VEC_SET(dl.vColor, 1.0f, 0.0f, 0.5f);

	psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);	


	// If my client did the critical hit, tell them so...Else if my
	// client received a critical hit, flash the screen...

	if ((int)dwId == m_nClientIDHitter)
	{
		CInfoDisplay* pInfoDisplay = g_pRiotClientShell->GetInfoDisplay();
		if (pInfoDisplay)
		{
			pInfoDisplay->AddInfo(IDS_CRITICALHIT, g_pRiotClientShell->GetMenu()->GetFont18s(), 1.5f, DI_CENTER | DI_BOTTOM);
		}
	}
	else if ((int)dwId == m_nClientIDHittee)
	{
		LTVector vTintColor;
		VEC_SET(vTintColor, 1.0f, 0.0f, 0.5f);
		LTFLOAT fRampUp = 0.2f, fRampDown = 0.6f, fTintTime = 0.1f;
	
		g_pRiotClientShell->TintScreen(vTintColor, m_vPos, 1000.0f, fRampUp, fTintTime, fRampDown);
	}

	char* pSound = "Sounds\\Weapons\\CriticalHit.wav";
	PlaySoundFromPos(&m_vPos, pSound, 2000.0f, SOUNDPRIORITY_PLAYER_HIGH);

	return LTFALSE;  // Delete me, I'm done :)
}

