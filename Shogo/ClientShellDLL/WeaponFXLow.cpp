// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFXLow.cpp
//
// PURPOSE : Weapon special FX - Low detail Implementation
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

extern CRiotClientShell* g_pRiotClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLowJuggernautFX
//
//	PURPOSE:	Create the juggernaut fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowJuggernautFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTFLOAT fFactor = m_eSize == MS_SMALL ? 0.2f : (m_eSize == MS_LARGE ? 5.0f : 1.0f);

	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -200.0f, 200.0f, -200.0f)
	VEC_MULSCALAR(vVelMin, vVelMin, fFactor)
	VEC_SET(vVelMax, 200.0f, 400.0f, 200.0f)
	VEC_MULSCALAR(vVelMax, vVelMax, fFactor)

	// Create a particle explosion...

	PESCREATESTRUCT pe;

	pe.bCreateDebris	= LTFALSE;
	pe.bRotateDebris	= LTFALSE;
	pe.rSurfaceRot = m_rSurfaceRot;
	VEC_COPY(pe.vPos, m_vPos);
	VEC_SET(pe.vColor1, 200.0f, 200.0f, 200.0f);
	VEC_SET(pe.vColor2, 230.0f, 230.0f, 230.0f);
	VEC_COPY(pe.vMinVel, vVelMin);
	VEC_COPY(pe.vMaxVel, vVelMax);
	pe.bSmall			= LTFALSE;
	pe.fLifeTime		= 1.5f;
	pe.fFadeTime		= 0.5f;
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

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();

	VEC_MULSCALAR(pe.vMinVel, pe.vMinVel, 1.25f);
	VEC_MULSCALAR(pe.vMaxVel, pe.vMaxVel, 1.25f);
	pe.vPos.y += 20.0f * fFactor;
	pe.fLifeTime		= 1.0f;
	pe.fFadeTime		= 0.5f;
	pe.fRadius			= 3000.0f * fFactor;

	pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();

	VEC_MULSCALAR(pe.vMinVel, pe.vMinVel, 1.2f);
	VEC_MULSCALAR(pe.vMaxVel, pe.vMaxVel, 1.2f);
	pe.vPos.y += 20.0f * fFactor;
	pe.fLifeTime		= 2.0f;
	pe.fFadeTime		= 0.5f;
	pe.fRadius			= 1500.0f;

	pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLowBullgutFX
//
//	PURPOSE:	Create the bullgut fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowBullgutFX()
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
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CratePulseRifleFX
//
//	PURPOSE:	Create the pulse rifle fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowPulseRifleFX()
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
//	ROUTINE:	CWeaponFX::CreateLowRedRiotFX
//
//	PURPOSE:	Create the red riot fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowRedRiotFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CSpecialFX* pFX;

	int		nNumRings = 1;
	LTFLOAT	fLifeTime = 1.0f;

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

	for (int i=0; i < nNumRings; i++)
	{
		ex.fDelayTime = i*(ex.fLifeTime/nNumRings);
		pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
		if (pFX) pFX->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLowShredderFX
//
//	PURPOSE:	Create the shredder fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowShredderFX()
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
//	ROUTINE:	CWeaponFX::CreateLowSpiderFX
//
//	PURPOSE:	Create the spider fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowSpiderFX()
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
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLowSniperRifleFX
//
//	PURPOSE:	Create the sniper rifle fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowSniperRifleFX()
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
		LTFLOAT fRange = 125.0f;

		CreateLowVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}

	if (GetRandom(1, 3) != 1) return;

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
	debris.fLifeTime		= GetRandom(0.5f, 1.0f);
	debris.fFadeTime		= 1.0f;
	debris.nNumDebris		= GetRandom(1, 3);
	debris.bRotate			= LTFALSE;
	debris.fMinScale		= 2.0f;
	debris.fMaxScale		= 5.0f;
	debris.nDebrisType		= GetVectorDebrisType(m_eSurfaceType);
	debris.bForceRemove		= LTTRUE;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_DEBRIS_ID, &debris);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLowTOWFX
//
//	PURPOSE:	Create the tow fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowTOWFX()
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
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLowKatoGrenadeFX
//
//	PURPOSE:	Create the kato grenade fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowKatoGrenadeFX()
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
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLowEnergyGrenadeFX
//
//	PURPOSE:	Create the energy grenade fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowEnergyGrenadeFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create model explosions...

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
	ex.fLifeTime		= 0.5f;

	LTVector vUp;
	VEC_SET(vUp, 1.0f, 0.0f, 1.0f);
	m_pClientDE->Math()->AlignRotation(ex.rRot, vUp, LTVector(0, 1, 0));

	ex.fInitialAlpha	= 0.9f;
	ex.fFinalAlpha		= 0.2f;
	ex.pFilename		= "Models\\PV_Weapons\\EnergyGrenadeExplosionCore.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\EnergyGrenadeCore.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLowTantoFX
//
//	PURPOSE:	Create the tanto fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowTantoFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	if (m_eSurfaceType == ST_LIQUID) return;

	CSpecialFX* pFX;

	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -100.0f, 150.0f, -100.0f);
	VEC_SET(vVelMax, 100.0f, 200.0f, 100.0f);

	if (m_eSurfaceType == ST_FLESH)
	{
		VEC_SET(vVelMin, -10.0f, 200.0f, -10.0f);
		VEC_SET(vVelMax, 10.0f, 300.0f, 10.0f);
		LTFLOAT fRange = 100.0f;

		CreateLowVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}

	if (GetRandom(1, 2) != 1) return;

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
//	ROUTINE:	CWeaponFX::CreateLowColt45FX
//
//	PURPOSE:	Create the colt 45 fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowColt45FX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	if (m_eSurfaceType == ST_LIQUID) return;

	CSpecialFX* pFX;

	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -100.0f, 150.0f, -100.0f)
	VEC_SET(vVelMax, 100.0f, 200.0f, 100.0f)

	if (m_eSurfaceType == ST_FLESH)
	{
		VEC_SET(vVelMin, -10.0f, 200.0f, -10.0f)
		VEC_SET(vVelMax, 10.0f, 300.0f, 10.0f)
		LTFLOAT fRange = 100.0f;

		CreateLowVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}

	if (GetRandom(1, 3) != 1) return;

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
//	ROUTINE:	CWeaponFX::CreateLowShotgunFX
//
//	PURPOSE:	Create the shotgun fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowShotgunFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	if (m_eSurfaceType == ST_LIQUID) return;

	if (GetRandom(1, 4) != 1) return;

	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -100.0f, 150.0f, -100.0f)
	VEC_SET(vVelMax, 100.0f, 200.0f, 100.0f)

	if (m_eSurfaceType == ST_FLESH)
	{
		VEC_SET(vVelMin, -10.0f, 200.0f, -10.0f)
		VEC_SET(vVelMax, 10.0f, 350.0f, 10.0f)
		LTFLOAT fRange = 75.0f;

		CreateLowVectorBloodFX(vVelMin, vVelMax, fRange);
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
//	ROUTINE:	CWeaponFX::CreateLowAssaultRifleFX
//
//	PURPOSE:	Create the assault rifle fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowAssaultRifleFX()
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
		VEC_SET(vVelMax, 10.0f, 400.0f, 10.0f)
		LTFLOAT fRange = 150.0f;

		CreateLowVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}

	if (GetRandom(1, 3) != 1) return;

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
//	ROUTINE:	CWeaponFX::CreateLowMac10FX
//
//	PURPOSE:	Create the mac 10 fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowMac10FX()
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
		VEC_SET(vVelMax, 10.0f, 350.0f, 10.0f)
		LTFLOAT fRange = 125.0f;

		CreateLowVectorBloodFX(vVelMin, vVelMax, fRange);
		return;
	}

	if (GetRandom(1, 3) != 1) return;

	// Create a some debris...

	DEBRISCREATESTRUCT debris;

	if (m_eSurfaceType == ST_MECHA)
	{
		debris.rRot = m_rDirRot;
	}
	else
	{
		debris.rRot = m_rDirRot;
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
//	ROUTINE:	CWeaponFX::CreateLowVectorBloodFX
//
//	PURPOSE:	Create the blood trail, splats, etc.
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLowVectorBloodFX(LTVector & vVelMin, LTVector & vVelMax, LTFLOAT fRange)
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings || !pSettings->Gore()) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	char* szBlood[2] = { "SpecialFX\\ParticleTextures\\Blood_1.dtx", 
						 "SpecialFX\\ParticleTextures\\Blood_2.dtx" };


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
	pe.fLifeTime		= 0.5f;
	pe.fFadeTime		= 0.25f;
	pe.fOffsetTime		= 0.0f;
	pe.fRadius			= 300.0f;
	pe.fGravity			= -100.0f;
	pe.nNumPerPuff		= 2;
	pe.nNumEmmitters	= 2;
	pe.bIgnoreWind		= LTTRUE;
	pe.pFilename		= szBlood[GetRandom(0, 1)];
	pe.nSurfaceType		= m_eSurfaceType;
	pe.nNumSteps		= 6;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();
}