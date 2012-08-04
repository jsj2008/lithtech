// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFXMed.cpp
//
// PURPOSE : Weapon special FX - Medium detail Implementation
//
// CREATED : 8/22/98
//
// ----------------------------------------------------------------------- //

#include "WeaponFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"
#include "WeaponFXTypes.h"
#include "RiotClientShell.h"
#include "RiotMsgIDs.h"
#include "ParticleExplosionFX.h"
#include "SpriteFX.h"
#include "ExplosionFX.h"
#include "DebrisFX.h"
#include "SparksFX.h"

extern CRiotClientShell* g_pRiotClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedJuggernautFX
//
//	PURPOSE:	Create the juggernaut fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedJuggernautFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTFLOAT fFactor = m_eSize == MS_SMALL ? 0.2f : (m_eSize == MS_LARGE ? 5.0f : 1.0f);

	CSpecialFX* pFX;

	if (!IsLiquid(m_eCode))
	{
		// Create smoke bank...

		SPRITECREATESTRUCT sc;

		LTVector vPos;
		VEC_MULSCALAR(vPos, m_vDir, 10.0f);
		VEC_ADD(vPos, vPos, m_vPos);

		VEC_COPY(sc.vPos, vPos);
		VEC_SET(sc.vVel, 0.0f, 10.0f, 0.0f);

		VEC_SET(sc.vInitialScale, 0.5f, 0.5f, 1.0f);
		VEC_MULSCALAR(sc.vInitialScale, sc.vInitialScale, fFactor);
		VEC_SET(sc.vFinalScale, 4.0f, 4.0f, 1.0f);
		VEC_MULSCALAR(sc.vFinalScale, sc.vFinalScale, fFactor);

		sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
		sc.fLifeTime		= 5.0f;
		sc.fInitialAlpha	= 1.0f;
		sc.fFinalAlpha		= 0.0f;
		sc.pFilename		= "Sprites\\SmokeTest.spr";

		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
		if (pFX) pFX->Update();

		LTFLOAT fOffset = 30.0f * fFactor;

		sc.vPos.x += GetRandom(-fOffset, fOffset);
		sc.vPos.z += GetRandom(-fOffset, fOffset);
		VEC_SET(sc.vVel, 0.0f, fOffset/2.0f, 0.0f);
		VEC_SET(sc.vInitialScale, 0.75f, 0.75f, 1.0f);
		VEC_MULSCALAR(sc.vInitialScale, sc.vInitialScale, fFactor);
		VEC_SET(sc.vFinalScale, 4.0f, 4.0f, 1.0f);
		VEC_MULSCALAR(sc.vFinalScale, sc.vFinalScale, fFactor);
		sc.fLifeTime		= 6.0f;

		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
		if (pFX) pFX->Update();
	}


	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -200.0f, 200.0f, -200.0f)
	VEC_MULSCALAR(vVelMin, vVelMin, fFactor)
	VEC_SET(vVelMax, 200.0f, 400.0f, 200.0f)
	VEC_MULSCALAR(vVelMax, vVelMax, fFactor)

	// Create a particle explosion...

	PESCREATESTRUCT pe;

	pe.bCreateDebris	= LTTRUE;
	pe.bRotateDebris	= LTTRUE;
	pe.rSurfaceRot = m_rSurfaceRot;
	VEC_COPY(pe.vPos, m_vPos);
	VEC_SET(pe.vColor1, 200.0f, 200.0f, 200.0f);
	VEC_SET(pe.vColor2, 230.0f, 230.0f, 230.0f);
	VEC_COPY(pe.vMinVel, vVelMin);
	VEC_COPY(pe.vMaxVel, vVelMax);
	pe.bSmall			= LTFALSE;
	pe.fLifeTime		= 2.5f;
	pe.fFadeTime		= 1.0f;
	pe.fOffsetTime		= 0.0f;
	pe.fRadius			= 4000.0f * fFactor;
	pe.fGravity			= 0.0f;
	pe.nNumPerPuff		= 1;
	pe.nNumEmmitters	= 4;
	pe.pFilename		= "Sprites\\SmokeTest.spr";

	if (IsLiquid(m_eCode))
	{
		GetLiquidColorRange(m_eCode, &pe.vColor1, &pe.vColor2);
		pe.pFilename = "SpecialFX\\ParticleTextures\\GreySphere_1.dtx";
	}

	pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();

	VEC_MULSCALAR(pe.vMinVel, pe.vMinVel, 1.25f);
	VEC_MULSCALAR(pe.vMaxVel, pe.vMaxVel, 1.25f);
	pe.vPos.y += 20.0f * fFactor;
	pe.fLifeTime		= 2.0f;
	pe.fFadeTime		= 0.5f;
	pe.fRadius			= 3000.0f * fFactor;

	pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();

	VEC_MULSCALAR(pe.vMinVel, pe.vMinVel, 1.2f);
	VEC_MULSCALAR(pe.vMaxVel, pe.vMaxVel, 1.2f);
	pe.vPos.y += 20.0f * fFactor;
	pe.fLifeTime		= 3.0f;
	pe.fFadeTime		= 1.5f;
	pe.fRadius			= 1500.0f;

	pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedBullgutFX
//
//	PURPOSE:	Create the bullgut fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedBullgutFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create explosion sprite...

	SPRITECREATESTRUCT sc;

	LTVector vPos;
	VEC_MULSCALAR(vPos, m_vDir, -20.0f);
	VEC_ADD(vPos, vPos, m_vPos);

	VEC_COPY(sc.vPos, vPos);
	VEC_SET(sc.vVel, 0.0f, 15.0f, 0.0f);

	VEC_SET(sc.vInitialScale, 0.5f, 0.5f, 1.0f);
	VEC_SET(sc.vFinalScale, 1.0f, 1.0f, 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
	sc.fLifeTime		= 1.5f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\weapons\\BllgtExp.spr";


	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();


	if (!IsLiquid(m_eCode))
	{
		// Create smoke...

		VEC_MULSCALAR(vPos, m_vDir, 10.0f);
		VEC_ADD(sc.vPos, sc.vPos, vPos);
		VEC_SET(sc.vInitialScale, 0.25f, 0.25f, 0.0f);
		VEC_SET(sc.vFinalScale, 3.0f, 3.0f, 0.0f);
		VEC_SET(sc.vVel, 0.0f, 10.0f, 0.0f);
		sc.fLifeTime		= 5.0f;
		sc.pFilename		= "Sprites\\SmokeTest.spr";

		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
		if (pFX) pFX->Update();
	}
	
	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -400.0f, 400.0f, -400.0f)
	VEC_SET(vVelMax, 400.0f, 700.0f, 400.0f)

	// Create a particle explosion...

	PESCREATESTRUCT pe;

	pe.bCreateDebris	= LTFALSE;
	pe.bRotateDebris	= LTFALSE;
	VEC_COPY(pe.vPos, m_vPos);
	pe.rSurfaceRot = m_rSurfaceRot;
	VEC_SET(pe.vColor1, 200.0f, 200.0f, 200.0f);
	VEC_SET(pe.vColor2, 255.0f, 255.0f, 255.0f);
	VEC_COPY(pe.vMinVel, vVelMin);
	VEC_COPY(pe.vMaxVel, vVelMax);
	pe.bSmall			= LTFALSE;
	pe.fLifeTime		= 1.75f;
	pe.fFadeTime		= 0.5f;
	pe.fOffsetTime		= 0.0f;
	pe.fRadius			= 1000.0f;
	pe.fGravity			= 0.0f;
	pe.nNumPerPuff		= 1;
	pe.nNumEmmitters	= 2;
	pe.pFilename		= "Sprites\\Fire.spr"; // "Sprites\\glow.spr";

	pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();

	pe.vPos.y			+= 20.0f;
	pe.fRadius			= 2500.0f;
	pe.pFilename	    = "Sprites\\SmokeTest.spr";

	if (IsLiquid(m_eCode))
	{
		GetLiquidColorRange(m_eCode, &pe.vColor1, &pe.vColor2);
		pe.pFilename = "SpecialFX\\ParticleTextures\\GreySphere_1.dtx";
	}

	pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CratePulseRifleFX
//
//	PURPOSE:	Create the pulse rifle fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedPulseRifleFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create impact sprite...

	SPRITECREATESTRUCT sc;

	VEC_COPY(sc.vPos, m_vPos);
	sc.rRot = m_rSurfaceRot;

	VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 0.1f, 0.1f, 1.0f);
	VEC_SET(sc.vFinalScale, 1.0f, 1.0f, 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT; 
	sc.fLifeTime		= 0.3f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\PulseImpact.spr";


	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedRedRiotFX
//
//	PURPOSE:	Create the red riot fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedRedRiotFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CSpecialFX* pFX;

	LTFLOAT	fLifeTime = 2.0f;

	// Create model explosion...

	EXCREATESTRUCT ex;
	ex.rRot = m_rDirRot;
	VEC_COPY(ex.vPos, m_vPos);

	LTFLOAT fRadius = GetWeaponDamageRadius(m_nWeaponId, m_eSize) * 2.0f;

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, 100.0f, 100.0f, 100.0f);
	VEC_SET(ex.vFinalScale, fRadius, fRadius, fRadius);
	VEC_SET(ex.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(ex.vFinalColor, 1.0f, 1.0f, 1.0f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= fLifeTime;
	ex.fInitialAlpha	= 0.8f;
	ex.fFinalAlpha		= 0.0f;
	ex.pFilename		= "Models\\PV_Weapons\\RedRiotExplosion.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\RedRiot.dtx";

	for (int i=0; i < 2; i++)
	{
		ex.fDelayTime = i*(ex.fLifeTime/2);
		pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
		if (pFX) pFX->Update();
	}
	

	// Create shockwave sprite...

	SPRITECREATESTRUCT sc;

	VEC_COPY(sc.vPos, m_vPos);
	m_pClientDE->Common()->SetupEuler(sc.rRot, MATH_HALFPI, 0.0f, 0.0f);
	VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 1.0f, 1.0f, 1.0f);
	VEC_SET(sc.vFinalScale, 10.0f, 10.0f, 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT; 
	sc.fLifeTime		= 1.0f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\Weapons\\RedRiot.spr";

	pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedShredderFX
//
//	PURPOSE:	Create the shredder fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedShredderFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create explosion sprite...

	SPRITECREATESTRUCT sc;

	VEC_COPY(sc.vPos, m_vPos);
	sc.vPos.y += 50.0f;

	VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 0.5f, 0.5f, 1.0f);
	VEC_SET(sc.vFinalScale, 1.0f, 1.0f, 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
	sc.fLifeTime		= 1.0f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\weapons\\ShrddrExp.spr";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedSpiderFX
//
//	PURPOSE:	Create the spider fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedSpiderFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create model explosion...

	EXCREATESTRUCT ex;
	VEC_COPY(ex.vPos, m_vPos);

	LTFLOAT fRadius = (LTFLOAT)GetWeaponDamageRadius(m_nWeaponId, m_eSize);

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, fRadius, fRadius, fRadius);
	VEC_SET(ex.vFinalScale, fRadius, fRadius, fRadius);
	VEC_SET(ex.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(ex.vFinalColor, 1.0f, 1.0f, 1.0f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 1.0f;
	ex.fInitialAlpha	= 1.0f;
	ex.fFinalAlpha		= 0.0f;
	ex.pFilename		= "Models\\PV_Weapons\\SpiderExplosionCore.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\SpiderCore.dtx";
	ex.bLoop			= LTTRUE;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();

	// Create shockwave sprite...

	SPRITECREATESTRUCT sc;

	VEC_COPY(sc.vPos, m_vPos);
	m_pClientDE->Common()->SetupEuler(sc.rRot, MATH_HALFPI, 0.0f, 0.0f);
	VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 0.5f, 0.5f, 1.0f);
	VEC_SET(sc.vFinalScale, 4.0f, 4.0f, 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT; 
	sc.fLifeTime		= 0.7f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\Weapons\\RedRiot.spr";

	int nNumRings = 1;
	for (int i=0; i < nNumRings; i++)
	{
		sc.fDelayTime = i*(sc.fLifeTime/nNumRings);
		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
		if (pFX) pFX->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedSniperRifleFX
//
//	PURPOSE:	Create the sniper rifle fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedSniperRifleFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	if (m_eSurfaceType == ST_LIQUID) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -100.0f, 150.0f, -100.0f)
	VEC_SET(vVelMax, 100.0f, 200.0f, 100.0f)


	if (m_eSurfaceType == ST_FLESH)
	{
		VEC_SET(vVelMin, -10.0f, 200.0f, -10.0f)
		VEC_SET(vVelMax, 10.0f, 350.0f, 10.0f)
		LTFLOAT fRange = 200.0f;

		CreateMeLTVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}


	// Create a some debris...

	DEBRISCREATESTRUCT debris;

	if (m_eSurfaceType == ST_MECHA)
	{
		debris.rRot = m_rDirRot;
	}
	else
	{
		debris.rRot = m_rSurfaceRot;
	}

	VEC_COPY(debris.vPos, m_vPos);
	VEC_COPY(debris.vMinVel, vVelMin);
	VEC_COPY(debris.vMaxVel, vVelMax);
	debris.fLifeTime		= GetRandom(1.0f, 2.0f);
	debris.fFadeTime		= 1.0f;
	debris.nNumDebris		= GetRandom(2, 5);
	debris.bRotate			= LTTRUE;
	debris.fMinScale		= 2.0f;
	debris.fMaxScale		= 5.0f;
	debris.nDebrisType		= GetVectorDebrisType(m_eSurfaceType);
	debris.bForceRemove		= LTTRUE;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_DEBRIS_ID, &debris);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedTOWFX
//
//	PURPOSE:	Create the tow fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedTOWFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTFLOAT fFactor = m_eSize == MS_SMALL ? 0.2f : (m_eSize == MS_LARGE ? 5.0f : 1.0f);

	// Create explosion sprite...

	SPRITECREATESTRUCT sc;

	VEC_COPY(sc.vPos, m_vPos);
	sc.vPos.y += 50.0f;

	VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 1.0f, 1.0f, 1.0f);
	VEC_MULSCALAR(sc.vInitialScale, sc.vInitialScale, fFactor);
	VEC_SET(sc.vFinalScale, 2.5f, 2.5f, 1.0f);
	VEC_MULSCALAR(sc.vFinalScale, sc.vFinalScale, fFactor);

	sc.vVel.y += GetRandom(10.0f, 15.0f);
	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
	sc.fLifeTime		= 0.75f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\weapons\\BllgtExp.spr";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();


	if (!IsLiquid(m_eCode))
	{
		// Create smoke...

		VEC_SET(sc.vInitialScale, 1.25f, 1.25f, 0.0f);
		VEC_MULSCALAR(sc.vInitialScale, sc.vInitialScale, fFactor);
		VEC_SET(sc.vFinalScale, 4.0f, 4.0f, 0.0f);
		VEC_MULSCALAR(sc.vFinalScale, sc.vFinalScale, fFactor);
		VEC_SET(sc.vVel, 0.0f, 15.0f, 0.0f);
		sc.fLifeTime		= 8.0f;
		sc.fInitialAlpha	= 1.0f;
		sc.fFinalAlpha		= 0.0f;
		sc.pFilename		= "Sprites\\SmokeTest.spr";

		LTVector vPos;
		sc.vVel.y += GetRandom(-10.0f, 10.0f);
		sc.vVel.x += GetRandom(-10.0f, 10.0f);
		sc.vVel.z += GetRandom(-10.0f, 10.0f);
		VEC_MULSCALAR(vPos, m_vDir, -10.0f);
		VEC_ADD(sc.vPos, sc.vPos, vPos);

		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
		if (pFX) pFX->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedKatoGrenadeFX
//
//	PURPOSE:	Create the kato grenade fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedKatoGrenadeFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create model explosion...

	EXCREATESTRUCT ex;
	VEC_COPY(ex.vPos, m_vPos);

	LTFLOAT fRadius = GetWeaponDamageRadius(m_nWeaponId, m_eSize) * 2.0f;

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, fRadius/5.0f, fRadius/5.0f, fRadius/5.0f);
	VEC_SET(ex.vFinalScale, fRadius, fRadius, fRadius);
	VEC_SET(ex.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(ex.vFinalColor, 1.0f, 1.0f, 1.0f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 1.0f;
	ex.fInitialAlpha	= 0.95f;
	ex.fFinalAlpha		= 0.0f;
	ex.pFilename		= "Models\\PV_Weapons\\KatoGrenadeExplosion.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\KatoGrenade.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();


	// Create explosion sprite...

	SPRITECREATESTRUCT sc;

	
	// Create shockwave sprite...

	VEC_COPY(sc.vPos, m_vPos);
	m_pClientDE->Common()->SetupEuler(sc.rRot, MATH_HALFPI, 0.0f, 0.0f);
	VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 0.5f, 0.5f, 1.0f);
	VEC_SET(sc.vFinalScale, 4.0f, 4.0f, 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT; 
	sc.fLifeTime		= 0.7f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\Weapons\\RedRiot.spr";

	pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedEnergyGrenadeFX
//
//	PURPOSE:	Create the energy grenade fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedEnergyGrenadeFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create model explosions...

	EXCREATESTRUCT ex;
	VEC_COPY(ex.vPos, m_vPos);

	LTFLOAT fRadius = GetWeaponDamageRadius(m_nWeaponId, m_eSize) * 2.0f;

	LTVector vUp;
	VEC_SET(vUp, 1.0f, 0.0f, 1.0f);
	m_pClientDE->Math()->AlignRotation(ex.rRot, vUp, LTVector(0, 1, 0));

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, fRadius/5.0f, fRadius/5.0f, fRadius/5.0f);
	VEC_SET(ex.vFinalScale, fRadius, fRadius, fRadius);
	VEC_SET(ex.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(ex.vFinalColor, 1.0f, 1.0f, 1.0f);
	ex.bUseUserColors = LTTRUE;
	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 0.75f;
	ex.fInitialAlpha	= 0.9f;
	ex.fFinalAlpha		= 0.2f;
	ex.pFilename		= "Models\\PV_Weapons\\EnergyGrenadeExplosionCore.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\EnergyGrenadeCore.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedTantoFX
//
//	PURPOSE:	Create the tanto fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedTantoFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	if (m_eSurfaceType == ST_LIQUID) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CSpecialFX* pFX;

	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -100.0f, 150.0f, -100.0f);
	VEC_SET(vVelMax, 100.0f, 200.0f, 100.0f);

	if (m_eSurfaceType == ST_FLESH)
	{
		VEC_SET(vVelMin, -10.0f, 200.0f, -10.0f);
		VEC_SET(vVelMax, 10.0f, 300.0f, 10.0f);
		LTFLOAT fRange = 100.0f;

		CreateMeLTVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}


	if (!IsLiquid(m_eCode))
	{
		// Create smoke puff...

		SPRITECREATESTRUCT sc;

		VEC_COPY(sc.vPos, m_vPos);
		sc.vPos.y += 3.0f;
		VEC_SET(sc.vVel, 0.0f, GetRandom(10.0f, 20.0f), 0.0f);
		VEC_SET(sc.vInitialScale, 0.1f, 0.1f, 1.0f);
		VEC_SET(sc.vFinalScale, 0.35f, 0.35f, 1.0f);
		sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
		sc.fLifeTime		= 2.0f;
		sc.fInitialAlpha	= 1.0f;
		sc.fFinalAlpha		= 0.0f;
		sc.pFilename		= "Sprites\\SmokeTest.spr";

		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
		if (pFX) pFX->Update();
	}
	
	// Create a some debris...

	DEBRISCREATESTRUCT debris;

	debris.rRot = m_rSurfaceRot;

	VEC_COPY(debris.vPos, m_vPos);
	VEC_COPY(debris.vMinVel, vVelMin);
	VEC_COPY(debris.vMaxVel, vVelMax);
	debris.fLifeTime		= GetRandom(0.5f, 1.5f);
	debris.fFadeTime		= 1.0f;
	debris.nNumDebris		= GetRandom(3, 5);
	debris.bRotate			= LTTRUE;
	debris.fMinScale		= 1.0f;
	debris.fMaxScale		= 2.0f;
	debris.nDebrisType		= GetVectorDebrisType(m_eSurfaceType);

	pFX = psfxMgr->CreateSFX(SFX_DEBRIS_ID, &debris);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedColt45FX
//
//	PURPOSE:	Create the colt 45 fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedColt45FX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	if (m_eSurfaceType == ST_LIQUID) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CSpecialFX* pFX;

	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -100.0f, 150.0f, -100.0f)
	VEC_SET(vVelMax, 100.0f, 200.0f, 100.0f)

	if (m_eSurfaceType == ST_FLESH)
	{
		VEC_SET(vVelMin, -10.0f, 200.0f, -10.0f)
		VEC_SET(vVelMax, 10.0f, 300.0f, 10.0f)
		LTFLOAT fRange = 100.0f;

		CreateMeLTVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}

	// Create a some debris...

	DEBRISCREATESTRUCT debris;

	if (m_eSurfaceType == ST_MECHA)
	{
		debris.rRot = m_rDirRot;
	}
	else
	{
		debris.rRot = m_rSurfaceRot;
	}

	VEC_COPY(debris.vPos, m_vPos);
	VEC_COPY(debris.vMinVel, vVelMin);
	VEC_COPY(debris.vMaxVel, vVelMax);
	debris.fLifeTime		= GetRandom(0.5f, 1.5f);
	debris.fFadeTime		= 1.0f;
	debris.nNumDebris		= GetRandom(1, 3);
	debris.bRotate			= LTTRUE;
	debris.fMinScale		= 1.0f;
	debris.fMaxScale		= 2.0f;
	debris.nDebrisType		= GetVectorDebrisType(m_eSurfaceType);
	debris.bForceRemove		= LTTRUE;

	pFX = psfxMgr->CreateSFX(SFX_DEBRIS_ID, &debris);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedShotgunFX
//
//	PURPOSE:	Create the shotgun fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedShotgunFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	if (m_eSurfaceType == ST_LIQUID) return;

	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -100.0f, 150.0f, -100.0f)
	VEC_SET(vVelMax, 100.0f, 200.0f, 100.0f)

	if (m_eSurfaceType == ST_FLESH)
	{
		VEC_SET(vVelMin, -10.0f, 200.0f, -10.0f)
		VEC_SET(vVelMax, 10.0f, 350.0f, 10.0f)
		LTFLOAT fRange = 75.0f;

		CreateMeLTVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}

	// Create a some debris...

	DEBRISCREATESTRUCT debris;

	if (m_eSurfaceType == ST_MECHA)
	{
		debris.rRot = m_rDirRot;
	}
	else
	{
		debris.rRot = m_rSurfaceRot;
	}

	VEC_COPY(debris.vPos, m_vPos);
	VEC_COPY(debris.vMinVel, vVelMin);
	VEC_COPY(debris.vMaxVel, vVelMax);
	debris.fLifeTime		= GetRandom(0.5f, 1.5f);
	debris.fFadeTime		= 1.0f;
	debris.nNumDebris		= 1;
	debris.bRotate			= LTTRUE;
	debris.fMinScale		= 1.0f;
	debris.fMaxScale		= 2.0f;
	debris.nDebrisType		= GetVectorDebrisType(m_eSurfaceType);
	debris.bForceRemove		= LTTRUE;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_DEBRIS_ID, &debris);
	if (pFX) pFX->Update();

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedAssaultRifleFX
//
//	PURPOSE:	Create the assault rifle fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedAssaultRifleFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	if (m_eSurfaceType == ST_LIQUID) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -50.0f, 50.0f, -50.0f)
	VEC_SET(vVelMax, 50.0f, 100.0f, 50.0f)

	if (m_eSurfaceType == ST_FLESH)
	{
		VEC_SET(vVelMin, -10.0f, 200.0f, -10.0f)
		VEC_SET(vVelMax, 10.0f, 400.0f, 10.0f)
		LTFLOAT fRange = 150.0f;

		CreateMeLTVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}

	// Create a some debris...

	DEBRISCREATESTRUCT debris;

	if (m_eSurfaceType == ST_MECHA)
	{
		debris.rRot = m_rDirRot;
	}
	else
	{
		debris.rRot = m_rSurfaceRot;
	}

	VEC_COPY(debris.vPos, m_vPos);
	VEC_COPY(debris.vMinVel, vVelMin);
	VEC_COPY(debris.vMaxVel, vVelMax);
	debris.fLifeTime		= GetRandom(0.5f, 1.5f);
	debris.fFadeTime		= 1.0f;
	debris.nNumDebris		= 1;
	debris.bRotate			= LTTRUE;
	debris.fMinScale		= 1.0f;
	debris.fMaxScale		= 4.0f;
	debris.nDebrisType		= GetVectorDebrisType(m_eSurfaceType);
	debris.bForceRemove		= LTTRUE;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_DEBRIS_ID, &debris);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMedMac10FX
//
//	PURPOSE:	Create the mac 10 fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMedMac10FX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	if (m_eSurfaceType == ST_LIQUID) return;

	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -50.0f, 50.0f, -50.0f)
	VEC_SET(vVelMax, 50.0f, 100.0f, 50.0f)

	if (m_eSurfaceType == ST_FLESH)
	{
		VEC_SET(vVelMin, -10.0f, 200.0f, -10.0f)
		VEC_SET(vVelMax, 10.0f, 350.0f, 10.0f)
		LTFLOAT fRange = 125.0f;

		CreateMeLTVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}

	// Create a some debris...

	DEBRISCREATESTRUCT debris;

	if (m_eSurfaceType == ST_MECHA)
	{
		debris.rRot = m_rDirRot;
	}
	else
	{
		debris.rRot = m_rSurfaceRot;
	}

	VEC_COPY(debris.vPos, m_vPos);
	VEC_COPY(debris.vMinVel, vVelMin);
	VEC_COPY(debris.vMaxVel, vVelMax);
	debris.fLifeTime		= GetRandom(0.5f, 1.5f);
	debris.fFadeTime		= 1.0f;
	debris.nNumDebris		= 1;
	debris.bRotate			= LTTRUE;
	debris.fMinScale		= 2.0f;
	debris.fMaxScale		= 4.0f;
	debris.nDebrisType		= GetVectorDebrisType(m_eSurfaceType);
	debris.bForceRemove		= LTTRUE;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_DEBRIS_ID, &debris);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMeLTVectorBloodFX
//
//	PURPOSE:	Create the blood trail, splats, etc.
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMeLTVectorBloodFX(LTVector & vVelMin, LTVector & vVelMax, LTFLOAT fRange)
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings || !pSettings->Gore()) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	char* szBlood[2] = { "SpecialFX\\ParticleTextures\\Blood_1.dtx", 
						 "SpecialFX\\ParticleTextures\\Blood_2.dtx" };


	// Create blood cloud (brain cloud? ;)...

	LTVector vDir;
	VEC_MULSCALAR(vDir, m_vSurfaceNormal, 100.0f);

	uint8 nNumSparks = (m_fDamage < 50.0f) ? (uint8)m_fDamage : 50;

	SCREATESTRUCT sp;

	VEC_COPY(sp.vPos, m_vPos);
	VEC_COPY(sp.vDir, vDir);
	VEC_SET(sp.vColor1, 200.0f, 200.0f, 200.0f);
	VEC_SET(sp.vColor2, 255.0f, 255.0f, 255.0f);
	sp.hstrTexture		= m_pClientDE->CreateString(szBlood[GetRandom(0, 1)]);
	sp.nSparks			= nNumSparks;
	sp.fDuration		= 1.0f;
	sp.fEmissionRadius	= 0.3f;
	sp.fRadius			= 500.0f;
	sp.fGravity			= PSFX_DEFAULT_GRAVITY;

	psfxMgr->CreateSFX(SFX_SPARKS_ID, &sp);

	m_pClientDE->FreeString(sp.hstrTexture);
	
	
	// Create a particle trails...

	PESCREATESTRUCT pe;

	VEC_COPY(pe.vPos, m_vPos);
	pe.rSurfaceRot = m_rDirRot;
	VEC_SET(pe.vColor1, 255.0f, 255.0f, 255.0f);
	VEC_SET(pe.vColor2, 255.0f, 255.0f, 255.0f);
	VEC_COPY(pe.vMinVel, vVelMin);
	VEC_COPY(pe.vMaxVel, vVelMax);
	VEC_SET(pe.vMinDriftOffset, 0.0f, -10.0f, 0.0f);
	VEC_SET(pe.vMaxDriftOffset, 0.0f, -5.0f, 0.0f);
	pe.bSmall			= LTFALSE;
	pe.fLifeTime		= 1.0f;
	pe.fFadeTime		= 0.5f;
	pe.fOffsetTime		= 0.0f;
	pe.fRadius			= 300.0f;
	pe.fGravity			= -100.0f;
	pe.nNumPerPuff		= 1;
	pe.nNumEmmitters	= 2;
	pe.bIgnoreWind		= LTTRUE;
	pe.pFilename		= szBlood[GetRandom(0,1)];
	pe.nSurfaceType		= m_eSurfaceType;
	pe.nNumSteps		= 6;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();


	// See if we should make a blood splat...

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;

	LTVector vTemp;
	VEC_MULSCALAR(vTemp, m_vDir, fRange);
	VEC_ADD(vTemp, m_vPos, vTemp);

	VEC_COPY(iQuery.m_From, m_vPos);
	VEC_COPY(iQuery.m_To, vTemp);

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{		
		// Create a blood splat...

		SPRITECREATESTRUCT sc;

		m_pClientDE->Math()->AlignRotation(sc.rRot, iInfo.m_Plane.m_Normal, LTVector(0, 1, 0));

		VEC_MULSCALAR(vTemp, m_vDir, -2.0f);
		VEC_ADD(sc.vPos, iInfo.m_Point, vTemp);  // Off the wall a bit
		VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
		VEC_SET(sc.vInitialScale, 0.2f, 0.2f, 1.0f);
		VEC_SET(sc.vFinalScale, 0.2f, 0.2f, 1.0f);

		sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT; 
		sc.fLifeTime		= 15.0f;
		sc.fInitialAlpha	= 1.0f;
		sc.fFinalAlpha		= 0.0f;

		char* pBloodFiles[] = 
		{
			"Sprites\\BloodSplat1.spr",
			"Sprites\\BloodSplat2.spr",
			"Sprites\\BloodSplat3.spr"
		};

		sc.pFilename = pBloodFiles[GetRandom(0,2)];
		
		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
		if (pFX) pFX->Update();
	}
}