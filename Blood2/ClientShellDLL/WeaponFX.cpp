// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFX.cpp
//
// PURPOSE : Weapon special effects - adapted from Riot
//
// CREATED : 4/30/98
//
// ----------------------------------------------------------------------- //

#include "WeaponFX.h"
#include "cpp_client_de.h"
#include "ClientUtilities.h"
#include "BloodClientShell.h"
#include "MarkSFX.h"
#include "SparksFX.h"
#include "ClientServerShared.h"
#include "SFXMsgIds.h"
#include "SmokeFX.h"
#include "SmokeImpactFX.h"
#include "SurfaceFragmentFX.h"
#include "bloodsplatfx.h"
#include "ParticleExplosionFX.h"
#include "ShellCasingFX.h"
#include "ExplosionFX.h"
#include "SplashFX.h"
#include "RippleFX.h"
#include "SoundTypes.h"

extern CBloodClientShell* g_pBloodClientShell;

char* 	szSmokeParticles[6] = {"SpriteTextures\\smoke64_1.dtx",
							   "SpriteTextures\\smoke64_2.dtx",
							   "SpriteTextures\\smoke64_3.dtx",
							   "SpriteTextures\\smoke64_4.dtx",
							   "SpriteTextures\\smoke64_5.dtx",
							   "SpriteTextures\\smoke64_6.dtx",};

char* 	szSmokeSprites[6] = {"Sprites\\smokepuff1.spr",
							 "Sprites\\smokepuff2.spr",
							 "Sprites\\smokepuff3.spr",
							 "Sprites\\smokepuff4.spr",
							 "Sprites\\smokepuff5.spr",
							 "Sprites\\smokepuff6.spr",};

#define	DEFAULT_WATER_IMPACT_SOUND_RADIUS	2000.0f
#define	DEFAULT_IMPACT_SOUND_RADIUS			500.0f

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return DFALSE;

	WFXCREATESTRUCT* pCS = (WFXCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY(m_vSourcePos, pCS->vSourcePos);
	VEC_COPY(m_vDestPos, pCS->vDestPos);
	VEC_COPY(m_vForward, pCS->vForward);
	VEC_COPY(m_vNormal, pCS->vNormal);

	m_nFXFlags			= pCS->nFXFlags;
	m_nExtraData		= pCS->nExtraData;

	m_nAmmoType			= pCS->nAmmoType;
	m_eSurfaceType		= (SurfaceType)pCS->nSurfaceType;
	m_nExplosionType	= pCS->nExplosionType;

	m_fDamage			= pCS->fDamage;
	m_fDensity			= pCS->fDensity;

	VEC_COPY(m_vColor1, pCS->vColor1);
	VEC_COPY(m_vColor2, pCS->vColor2);
	VEC_COPY(m_vLightColor1, pCS->vLightColor1);
	VEC_COPY(m_vLightColor2, pCS->vLightColor2);

	m_eSourceCode	= CC_NOTHING;
	m_eDestCode		= CC_NOTHING;

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponFX::CreateObject(CClientDE* pClientDE)
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!CSpecialFX::CreateObject(pClientDE) || !pShell) return DFALSE;

	// Calculate the direction vector
	VEC_SUB(m_vDir, m_vDestPos, m_vSourcePos);

	VEC_NORM(m_vDir);
	VEC_NORM(m_vForward);
	VEC_NORM(m_vNormal);

	// Get the rotation
	m_pClientDE->AlignRotation(&m_rRotation, &m_vNormal, DNULL);

	// Determine what container the source location is in...
	HLOCALOBJ objList[1];
	DDWORD dwNum = m_pClientDE->GetPointContainers(&m_vSourcePos, objList, 1);

	if(dwNum > 0 && objList[0])
	{
		DDWORD dwUserFlags;
		m_pClientDE->GetObjectUserFlags(objList[0], &dwUserFlags);

		if(dwUserFlags & USRFLG_VISIBLE)
		{
			D_WORD dwCode;
			if (m_pClientDE->GetContainerCode(objList[0], &dwCode))
				m_eSourceCode = (ContainerCode)dwCode;
		}
	}

	// Determine what container the destination location is in...
	dwNum = m_pClientDE->GetPointContainers(&m_vDestPos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
		DDWORD dwUserFlags;
		m_pClientDE->GetObjectUserFlags(objList[0], &dwUserFlags);

		if (dwUserFlags & USRFLG_VISIBLE)
		{
			D_WORD dwCode;
			if (m_pClientDE->GetContainerCode(objList[0], &dwCode))
				m_eDestCode = (ContainerCode)dwCode;
		}
	}

	// Create the FX from the muzzle to the destination
	if(m_nFXFlags & WFX_MUZZLESMOKE)		CreateMuzzleSmoke();
	if(m_nFXFlags & WFX_MUZZLELIGHT)		CreateMuzzleLight();
	if(m_nFXFlags & WFX_EJECTSHELL)
		{ CreateShellCasing(); if(m_nAmmoType == AMMO_SHELL) CreateShellCasing(); }
	if(m_nFXFlags & WFX_TRACER)				CreateTracer();
	if(m_nFXFlags & WFX_PARTICLETRAIL)		CreateParticleTrail();

	// Create the FX at the destination
	if(m_nFXFlags & WFX_MARK)				CreateMark();
	if(m_nFXFlags & WFX_FLASH)				CreateFlash();
	if(m_nFXFlags & WFX_SPARKS)				CreateSparks();
	if(m_nFXFlags & WFX_SPLASH)				CreateSplash();
	if(m_nFXFlags & WFX_SMOKE)				CreateSmoke();
	if(m_nFXFlags & WFX_SOUND)				PlayImpactSound();
	if(m_nFXFlags & WFX_BLOODSPLAT)			CreateBloodSplat();
	if(m_nFXFlags & WFX_BLOODSPURT)			CreateBloodSpurt();
	if(m_nFXFlags & WFX_EXPLOSION)			CreateExplosion();
	if(m_nFXFlags & WFX_FRAGMENTS)			CreateFragments();
	if(m_nFXFlags & WFX_IMPACTLIGHT)		CreateImpactLight();
	if(m_nFXFlags & WFX_SCREENSHAKE)		CreateShakeNFlash();

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::Update
//
//	PURPOSE:	Update the weapon fx
//
// ----------------------------------------------------------------------- //

DBOOL CWeaponFX::Update()
{
	return DFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMark
//
//	PURPOSE:	Create a mark fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMark()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell) return;

	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return;

	char* pMarkSprite = GetMarkSprite(m_eSurfaceType);
	if (!pMarkSprite) return;

	MARKCREATESTRUCT mark;

	mark.hServerObj = DNULL;
	VEC_COPY(mark.m_Pos, m_vDestPos);
	ROT_COPY(mark.m_Rotation, m_rRotation);
	mark.m_fScale = (m_nAmmoType == AMMO_BULLET) ? 0.05f : 0.035f;
	mark.m_hstrSprite = m_pClientDE->CreateString(pMarkSprite);
	mark.m_bServerObj = DFALSE;

	psfxMgr->CreateSFX(SFX_MARK_ID, &mark, DFALSE, this);
	g_pClientDE->FreeString( mark.m_hstrSprite );
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CWeaponFX::CreateBloodSplat
// DESCRIPTION	: Create a blood splat on surface
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateBloodSplat()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell) return;

	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return;

	char* pSplatSprite = DNULL;

	switch(GetRandom(1,3))
	{
		case 1:		pSplatSprite = "sprites\\bloodsplat1.spr";	break;
		case 2:		pSplatSprite = "sprites\\bloodsplat2.spr";	break;
		case 3:		pSplatSprite = "sprites\\bloodsplat3.spr";	break;
		default:	pSplatSprite = "sprites\\bloodsplat1.spr";	break;
	}

	BSCREATESTRUCT splat;

	splat.hServerObj = DNULL;
	VEC_COPY(splat.m_Pos, m_vDestPos);
	ROT_COPY(splat.m_Rotation, m_rRotation);
	splat.m_fScale = 0.1f + GetRandom(-0.05f,0.05f);
	splat.m_hstrSprite = m_pClientDE->CreateString(pSplatSprite);
	splat.m_fGrowScale = 0;

	psfxMgr->CreateSFX(SFX_BLOODSPLAT_ID, &splat, DFALSE, this);
	g_pClientDE->FreeString( splat.m_hstrSprite );
}

// ----------------------------------------------------------------------- //
// ROUTINE		: CWeaponFX::CreateBloodSpurt
// DESCRIPTION	: Create a blood splat on surface
// RETURN TYPE	: void 
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateBloodSpurt()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell) return;
	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return;

	char* szBlood[4] = { "spritetextures\\particles\\blooddrop_1.dtx", 
						 "spritetextures\\particles\\blooddrop_2.dtx",
						 "spritetextures\\particles\\blooddrop_3.dtx",
						 "spritetextures\\particles\\blooddrop_4.dtx"};


	PESCREATESTRUCT pe;

//	VEC_SET(pe.vVelMin, -50.0f, 0.0f, -50.0f)
//	VEC_SET(pe.vVelMax, 50.0f, 50.0f, 50.0f)
	VEC_MULSCALAR(pe.vMinVel, m_vNormal, 30.0f);
	VEC_MULSCALAR(pe.vMaxVel, m_vNormal, 50.0f);
	pe.vMinVel.y = 40.0f;
	pe.vMaxVel.y = 50.0f;
	pe.vMinVel.x -= 20.0f;
	pe.vMinVel.z -= 20.0f;
	pe.vMaxVel.x += 20.0f;
	pe.vMaxVel.z += 20.0f;

	VEC_COPY(pe.vPos, m_vDestPos);
	ROT_COPY(pe.rSurfaceRot, m_rRotation);
//	VEC_SET(pe.vColor1, 255.0f, 255.0f, 255.0f);
//	VEC_SET(pe.vColor2, 255.0f, 255.0f, 255.0f);
	VEC_SET(pe.vColor1, 128.0f, 128.0f, 128.0f);
	VEC_SET(pe.vColor2, 128.0f, 128.0f, 128.0f);
//	VEC_COPY(pe.vMinVel, vVelMin);
//	VEC_COPY(pe.vMaxVel, vVelMax);
	VEC_SET(pe.vMinDriftOffset, 0.0f, -10.0f, 0.0f);
	VEC_SET(pe.vMaxDriftOffset, 0.0f, -5.0f, 0.0f);
	pe.bSmall			= DFALSE;
	pe.fLifeTime		= 1.0f;
	pe.fFadeTime		= 0.5f;
	pe.fOffsetTime		= 0.0f;
	pe.fRadius			= 200.0f;
	pe.fGravity			= -100.0f;
	pe.nNumPerPuff		= 2;
	pe.nNumEmitters		= 1; //GetRandom(1,4);
	pe.nEmitterFlags	= MO_HALFGRAVITY;
	pe.bIgnoreWind		= DTRUE;
	
	int nMaxSZBlood = NRES(4) - 1;
	pe.pFilename		= szBlood[GetRandom(0,nMaxSZBlood)];
	pe.nSurfaceType		= SURFTYPE_FLESH;
	pe.nNumSteps		= 6;
	pe.bBounce			= DFALSE;


	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe, DFALSE, this);
	if (pFX) 
	{
		CBaseParticleSystemFX* pBasePS = (CBaseParticleSystemFX*)pFX;
		pBasePS->m_bSetSoftwareColor = DFALSE;

		pFX->Update();
		m_pClientDE->SetSoftwarePSColor(pFX->GetObject(), 1.0f, 0.0f, 0.0f);
	}

	// Create a pass-through spurt scaled by damage
	VEC_MULSCALAR(pe.vMinVel, m_vNormal, -2.5f * m_fDamage);
	VEC_MULSCALAR(pe.vMaxVel, m_vNormal, -3.0f * m_fDamage);
	pe.vMinVel.y += 30.0f;
	pe.vMaxVel.y += 40.0f;
	pe.vMinVel.x -= 20.0f;
	pe.vMinVel.z -= 20.0f;
	pe.vMaxVel.x += 20.0f;
	pe.vMaxVel.z += 20.0f;
	pe.fLifeTime		= 1.2f;
	pe.fFadeTime		= 0.8f;

	pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe, DFALSE, this);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSparks
//
//	PURPOSE:	Create sparks fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateSparks()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell) return;

	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return;

	SCREATESTRUCT sp;
	DBOOL	big = DFALSE;
	char	str[128];

	// General variables for the sparks
	sp.hServerObj = DNULL;
	VEC_COPY(sp.vPos, m_vDestPos);
	sp.bFadeColors = DFALSE;

	switch(m_eSurfaceType)
	{
		// Specific variable values just for metal surfaces
		case	SURFTYPE_METAL:
			big = GetRandom(0,19) ? DFALSE : DTRUE;

			VEC_MULSCALAR(sp.vDir, m_vNormal, 75.0f);
			VEC_SET(sp.vColor1, 255.0f, 255.0f, 0.0f);
			VEC_SET(sp.vColor2, 255.0f, 255.0f, 255.0f);

			sp.hstrTexture		= m_pClientDE->CreateString("spriteTextures\\particles\\particle2.dtx");
			sp.fEmissionRadius	= 0.3f;
			sp.fRadius			= 175.0f;
			sp.fGravity			= -150;

			if(big)
			{
				sp.nSparks			= (m_fDamage < 50.0f) ? (DBYTE)(m_fDamage * 5.0f): 250;
				sp.fDuration		= 2.0f;
			}
			else
			{
				sp.nSparks			= (m_fDamage < 50.0f) ? (DBYTE)(m_fDamage * 2.0f): 100;
				sp.fDuration		= 0.5f;
			}
			break;

		// Specific variable values just for flesh surfaces
		case	SURFTYPE_FLESH:
			VEC_MULSCALAR(sp.vDir, m_vNormal, 50.0f);
			VEC_SET(sp.vColor1, 192.0f, 0.0f, 0.0f);
			VEC_SET(sp.vColor2, 255.0f, 0.0f, 0.0f);

			sprintf(str, "spritetextures\\particles\\blooddrop_%d.dtx", GetRandom(1,NRES(4)));
			sp.hstrTexture = m_pClientDE->CreateString(str);
			sp.nSparks			= (m_fDamage < 50.0f) ? (DBYTE)(m_fDamage * 4.0f) : 200;
			sp.fDuration		= 2.0f;
			sp.fEmissionRadius	= 5.0f;
			sp.fRadius			= 200.0f;
			sp.fGravity			= -150.0f;
			break;
	}

	psfxMgr->CreateSFX(SFX_SPARKS_ID, &sp, DFALSE, this);
	m_pClientDE->FreeString(sp.hstrTexture);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateFragments
//
//	PURPOSE:	Creates pieces of a surface that fly off when shot
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateFragments()
{
	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if(!psfxMgr) return;

	SURFFRAGCREATESTRUCT	sfcs;
	DFLOAT		randVal;
	DVector		temp, vU, vR, vF;
	m_pClientDE->GetRotationVectors(&m_rRotation, &vU, &vR, &vF); 

	VEC_COPY(sfcs.vNormal, m_vNormal);
	VEC_COPY(sfcs.vPos, m_vDestPos);
	VEC_SET(sfcs.vDecel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sfcs.vScale, 0.05f, 0.05f, 0.0f);
	sfcs.fOffset = 3.0f;
	sfcs.fDuration = 3.0f;
	sfcs.bFade = 0;
	sfcs.bMove = 1;

	if(m_vNormal.y > 0.5)
		{	VEC_MULSCALAR(sfcs.vVel, m_vNormal, 3.0f); sfcs.fDuration = 2.0f;	}
	else
		{	VEC_COPY(sfcs.vVel, m_vNormal);	}


	switch(m_eSurfaceType)
	{
		case	SURFTYPE_WOOD:
		{
			sfcs.nType = 0;
			sfcs.bRotate = 1;

			if(IsLiquid(m_eDestCode))
				{	VEC_SET(sfcs.vGravity, 0.0f, -0.1f, 0.0f);	}
			else
				{	VEC_SET(sfcs.vGravity, 0.0f, -0.2f, 0.0f);	}

			for(char i = GetRandom(1,3); i > 0; i--)
			{
				randVal = GetRandom(-2.0f, 2.0f);
				if(randVal != 0.0f)
				{
					VEC_MULSCALAR(temp, vR, randVal);
					VEC_ADD(sfcs.vVel, sfcs.vVel, temp);
				}
				randVal = GetRandom(-2.0f, 3.0f);
				if(randVal != 0.0f)
				{
					VEC_MULSCALAR(temp, vU, randVal);
					VEC_ADD(sfcs.vVel, sfcs.vVel, temp);
				}

				psfxMgr->CreateSFX(SFX_FRAGMENTS_ID, &sfcs, DFALSE, this);
			}
			break;
		}
		case	SURFTYPE_STONE:
		{
			sfcs.nType = 1;
			sfcs.bRotate = 1;
			VEC_SET(sfcs.vScale, 0.03f, 0.03f, 0.0f);

			if(IsLiquid(m_eDestCode))
				{	VEC_SET(sfcs.vGravity, 0.0f, -0.15f, 0.0f);	}
			else
				{	VEC_SET(sfcs.vGravity, 0.0f, -0.3f, 0.0f);	}

			for(char i = GetRandom(1,3); i > 0; i--)
			{
				randVal = GetRandom(-2.0f, 2.0f);
				if(randVal != 0.0f)
				{
					VEC_MULSCALAR(temp, vR, randVal);
					VEC_ADD(sfcs.vVel, sfcs.vVel, temp);
				}
				randVal = GetRandom(-2.0f, 3.0f);
				if(randVal != 0.0f)
				{
					VEC_MULSCALAR(temp, vU, randVal);
					VEC_ADD(sfcs.vVel, sfcs.vVel, temp);
				}

				psfxMgr->CreateSFX(SFX_FRAGMENTS_ID, &sfcs, DFALSE, this);
			}
			break;
		}
		default:
			return;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateShakeNFlash
//
//	PURPOSE:	Create a screen Flash/shake effect
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateShakeNFlash()
{
	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// If we can see explosion, tint/shake screen...
	DVector vFlashColor;
	DVector vShake;
	DFLOAT fRampUp, fRampDown, fFlashTime, fRange, fShakeRange, fShakeTime;

	VEC_SET(vFlashColor, 0.7f, 0.7f, 0.7f);
	VEC_SET(vShake, 2.0f, 2.0f, 2.0f);

	fRampUp		= 0.1f;
	fRampDown	= 0.2f;
	fFlashTime	= 0.1f;
	fRange		= 500.0f;
	fShakeRange	= 1200.0f;
	fShakeTime	= 1.0f;

	DVector vCamPos;
	m_pClientDE->GetObjectPos(g_pBloodClientShell->GetCameraObj(), &vCamPos);

	// Determine if we should flash the screen...
	DVector vDir;
	VEC_SUB(vDir, m_vDestPos, vCamPos);
	DFLOAT fDist = VEC_MAG(vDir);
	if (fDist <= fRange)
	{
		// Adjust Flash for distance/angle of view

		DRotation rRot;
		DVector vU, vR, vF;
		m_pClientDE->GetObjectRotation(g_pBloodClientShell->GetCameraObj(), &rRot);
		m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

		VEC_NORM(vDir);
		VEC_NORM(vF);
		DFLOAT fMul = VEC_DOT(vDir, vF);
		if (fMul > 0.0f)
		{
			// Adjust for distance
			fMul *= (1 - fDist/fRange); 
			VEC_MULSCALAR(vFlashColor, vFlashColor, fMul);

			g_pBloodClientShell->FlashScreen(&vFlashColor, fRampUp, fFlashTime);
		}
	}

	// Determine if we should shake the screen...
	HLOCALOBJ hPlayerObj = m_pClientDE->GetClientObject();
	if (hPlayerObj)
	{
		DVector vPlayerPos;//, vPos;
		m_pClientDE->GetObjectPos(hPlayerObj, &vPlayerPos);

		VEC_SUB(vDir, vPlayerPos, m_vDestPos);
		DFLOAT fDist = VEC_MAG(vDir);

		if (fDist <= fShakeRange)
		{
			DFLOAT fMul = (1 - fDist/fShakeRange); 

			VEC_MULSCALAR(vShake, vShake, fMul);
			g_pBloodClientShell->ShakeScreen(&vShake, fShakeTime);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateExplosion
//
//	PURPOSE:	Create an explosion effect
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateExplosion()
{
	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	EXPLOSIONFXCS expCS;

	expCS.hServerObj = 0;
	VEC_COPY(expCS.vPos, m_vDestPos);
	VEC_COPY(expCS.vNormal, m_vNormal);
	expCS.nType = m_nExplosionType;

	psfxMgr->CreateSFX(SFX_EXPLOSIONFX_ID, &expCS, DFALSE, this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateImpactLight
//
//	PURPOSE:	Create an explosion effect
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateImpactLight()
{
	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if (!psfxMgr) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSplash
//
//	PURPOSE:	Create an explosion effect
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateSplash()
{
	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	SPLASHCREATESTRUCT		splcs;
	RIPPLECREATESTRUCT		ripcs;

	splcs.hServerObj = 0;
	VEC_COPY(splcs.vPos, m_vDestPos);
	VEC_COPY(splcs.vDir, m_vNormal);
	splcs.fRadius		= 100.0f;
	splcs.fPosRadius	= 1.0f;
	splcs.fHeight		= 175.0f;
	splcs.fDensity		= 4.0f;
	splcs.fSpread		= 10.0f;
	VEC_COPY(splcs.vColor1, m_vColor1);
	VEC_COPY(splcs.vColor2, m_vColor2);
	splcs.fSprayTime	= 0.3f;
	splcs.fDuration		= 0.75f;
	splcs.fGravity		= -500.0f;
	splcs.hstrTexture	= m_pClientDE->CreateString("spritetextures\\drop32_1.dtx");

	psfxMgr->CreateSFX(SFX_SPLASH_ID, &splcs, DFALSE, this);

	ripcs.hServerObj = 0;
	VEC_COPY(ripcs.vPos, m_vDestPos);
	VEC_COPY(ripcs.vNormal, m_vNormal);
	ripcs.fDuration = 1.0f;
	VEC_SET(ripcs.vMinScale, 0.1f, 0.1f, 0.0f);
	VEC_COPY(ripcs.vMaxScale, ripcs.vMinScale);
	ripcs.fInitAlpha = 0.25f;
	ripcs.bFade = 0;

	psfxMgr->CreateSFX(SFX_RIPPLE_ID, &ripcs, DFALSE, this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateFlash
//
//	PURPOSE:	Create an explosion effect
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateFlash()
{
	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if (!psfxMgr || IsLiquid(m_eDestCode))	return;

	EXPLOSIONSPRITECS	esCS;

	char	str[128];
	sprintf(str, "Sprites\\impact0%d.spr", GetRandom(1,NRES(5)));

	VEC_ADD(esCS.vPos, m_vDestPos, m_vNormal);
	VEC_COPY(esCS.vNormal, m_vNormal);
	VEC_SET(esCS.vScale1, 0.1f, 0.1f, 0.0f);
	VEC_SET(esCS.vScale2, 0.1f, 0.1f, 0.0f);
	esCS.fDuration		= 0.15f;
	esCS.fAlpha			= 1.0f;
	esCS.bWaveForm		= 0;
	esCS.bFadeType		= 0;
	esCS.bAlign			= 0;
	esCS.szSprite		= m_pClientDE->CreateString(str);

	psfxMgr->CreateSFX(SFX_EXPLOSIONSPRITE_ID, &esCS, DFALSE, this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateParticleTrail
//
//	PURPOSE:	Create an explosion effect
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateParticleTrail()
{
	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if (!psfxMgr) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMuzzleLight
//
//	PURPOSE:	Create an explosion effect
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMuzzleLight()
{
	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if (!psfxMgr) return;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateTracer
//
//	PURPOSE:	Create a tracer fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateTracer()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell) return;

	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return;

	DVector vF;

	VEC_COPY(vF, m_vDir);
	VEC_NORM(vF);
	VEC_MULSCALAR(vF, vF, 100.0f);

	TRACERCREATESTRUCT tr;

	tr.hServerObj = DNULL;
	VEC_COPY(tr.vVel, vF);

	VEC_SET(tr.vStartColor, 1.0f, 0.8f, 0.0f);
	VEC_SET(tr.vEndColor,   1.0f, 0.0f, 0.0f);
	VEC_COPY(tr.vStartPos, m_vSourcePos);
	ROT_COPY(tr.rRot, m_rRotation);
	tr.fStartAlpha	= 0.1f;
	tr.fEndAlpha	= 0.9f;

	psfxMgr->CreateSFX(SFX_TRACER_ID, &tr, DFALSE, this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSmoke
//
//	PURPOSE:	Create a smoke/water splash fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateSmoke()
{
	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if(!psfxMgr) return;

	if(IsLiquid(m_eDestCode) && m_eSurfaceType != SURFTYPE_LIQUID)
	{
		CreateBubbles(&m_vDestPos, 4);
		return;
	}

	SMOKECREATESTRUCT	sms;

	VEC_COPY(sms.vNormal, m_vNormal);
	sms.fOffset = 3.0f;
	VEC_COPY(sms.vPos, m_vDestPos);
	VEC_SET(sms.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sms.vDecel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sms.vGravity, 0.0f, GetRandom(0.05f,0.1f), 0.0f);
	sms.fDuration = 1.0f;
	sms.fDelay = 0.0f;
	sms.bUpdateScale = 1;
	VEC_SET(sms.vMinScale, 0.1f, 0.1f, 0.0f);
	VEC_SET(sms.vMaxScale, 0.35f, 0.35f, 0.0f);
	sms.fInitAlpha = 0.65f;
	sms.bFade = 1;
	sms.bRotate = 0;

	switch(m_eSurfaceType)
	{
		case	SURFTYPE_WOOD:
			VEC_SET(sms.vColor, 1.0f, 0.8f, 0.6f);
			break;
	}

	int nMax = NRES(6) - 1;
	sms.pSpriteFile = szSmokeSprites[GetRandom(0,nMax)];

	if(m_vNormal.y > -0.3f)
		sms.bMove = 1;
	else
		sms.bMove = 0;

	psfxMgr->CreateSFX(SFX_SMOKEIMPACT_ID, &sms, DFALSE, this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateBubbles
//
//	PURPOSE:	Creates a group of bubbles
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateBubbles(DVector* pvStartPos, DBYTE numBubbles)
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!m_pClientDE || !pShell) return;

	if (m_nAmmoType != AMMO_BULLET && m_nAmmoType != AMMO_SHELL && m_nAmmoType != AMMO_BMG)
		return;

	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return;

	char* pTexture = "\\SpriteTextures\\Particles\\ParticleBubble.dtx";

	SCREATESTRUCT sp;
	VEC_COPY(sp.vPos, *pvStartPos);
	VEC_MULSCALAR(sp.vDir, m_vNormal, 15.0f);
	sp.hstrTexture = m_pClientDE->CreateString(pTexture);
	sp.hServerObj  = DNULL;
	sp.nSparks = numBubbles;
	VEC_SET(sp.vColor1, 125.0f, 125.0f, 255.0f);
	VEC_SET(sp.vColor2, 180.0f, 180.0f, 255.0f);
	sp.fDuration		= 1.25f;
	sp.fEmissionRadius	= 0.05f;
	sp.fRadius			= 600.0f;
	sp.fGravity			= 50.0f;

	if (m_nAmmoType == AMMO_SHELL)
		sp.nSparks = 1;

	psfxMgr->CreateSFX(SFX_SPARKS_ID, &sp, DFALSE, this);
	m_pClientDE->FreeString(sp.hstrTexture);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::GetMarkSprite()
//
//	PURPOSE:	Get a mark sprite associated with this weapon and surface
//
// ----------------------------------------------------------------------- //

char* CWeaponFX::GetMarkSprite(SurfaceType eSurfType)
{
	char* pMark = DNULL;
/*
	Blthole.spr-metal
	Blthole2.spr-plastic and cloth
	Blthole3.spr-concrete
	Blthole4.spr-concrete
	Blthole5.spr-wood
*/
	switch(eSurfType)
	{
		case SURFTYPE_UNKNOWN:
		case SURFTYPE_FLESH:
		case SURFTYPE_SKY:
		case SURFTYPE_GLASS:
			break;

//			pMark = GetRandom(0,1) ? "sprites/glshole1.spr" : "sprites/glshole2.spr";
//			break;

		case SURFTYPE_METAL:
			pMark = "sprites/blthole.spr";
			break;

		case SURFTYPE_PLASTIC:
		case SURFTYPE_CLOTH:
			pMark = "sprites/blthole2.spr";
			break;

		case SURFTYPE_WOOD:
			pMark = "sprites/blthole5.spr";
			break;

		case SURFTYPE_STONE:
		default:
			pMark = GetRandom(0,1) ? "sprites/blthole3.spr" : "sprites/blthole4.spr";
			break;
	}

	return pMark;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::GetSparkTexture()
//
//	PURPOSE:	Get a spark texture associated with this weapon and surface
//
// ----------------------------------------------------------------------- //

char* CWeaponFX::GetSparkTexture(SurfaceType eSurfType, DVector* pvColor1, DVector* pvColor2)
{
	char* pTexture = DNULL;
	return (pTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLightFX
//
//	PURPOSE:	Create a cool light fx 
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLightFX(DVector* pvPos, DVector* pvColor)
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!m_pClientDE || !pShell || !pvPos || !pvColor) return;
/*
	CSFXMgr* psfxMgr = g_pBloodClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	DLCREATESTRUCT dl;

	dl.hServerObj = DNULL;
	VEC_COPY(dl.vColor, *pvColor);
	VEC_COPY(dl.vPos, *pvPos);
	dl.fMinRadius    = 50.0f;
	dl.fMaxRadius	 = 100.0f;
	dl.fRampUpTime	 = 0.3f;
	dl.fMaxTime		 = 0.1f;
	dl.fMinTime		 = 0.0f;
	dl.fRampDownTime = 0.2f;

	psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl, DFALSE, this);
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PlayImpactSound()
//
//	PURPOSE:	Play a surface impact sound if appropriate
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PlayImpactSound()
{
	if (!m_pClientDE) return;

//	DVector vPos;
//	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);

	char*	pImpactSound = GetImpactSound(0, m_nAmmoType, m_eSurfaceType);
	DFLOAT	fImpactSoundRadius	= DEFAULT_IMPACT_SOUND_RADIUS;

	if (pImpactSound)
		PlaySoundFromPos(&m_vDestPos, pImpactSound, fImpactSoundRadius, SOUNDPRIORITY_MISC_LOW);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMuzzleSmoke()
//
//	PURPOSE:	Create muzzle specific fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMuzzleSmoke()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!m_pClientDE || !pShell) return;

	if (m_nAmmoType != AMMO_BULLET && m_nAmmoType != AMMO_SHELL && m_nAmmoType != AMMO_BMG)
		return;

	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if(!psfxMgr) return;

	if(IsLiquid(m_eSourceCode) && m_eSurfaceType != SURFTYPE_LIQUID)
	{
		CreateBubbles(&m_vSourcePos, 4);
		return;
	}

	SMOKECREATESTRUCT	sms;

	VEC_COPY(sms.vNormal, m_vNormal);
	sms.fOffset = 0.0f;
	VEC_COPY(sms.vPos, m_vSourcePos);
	VEC_SET(sms.vGravity, GetRandom(-0.5f, 0.5f), GetRandom(-0.25f, 0.75f), GetRandom(-0.5f, 0.5f));
	sms.fDelay = 0.0f;
	sms.bUpdateScale = 1;
	sms.bFade = 1;
	sms.bRotate = 0;
	sms.bMove = 1;

	switch(GetRandom(1,NRES(6)))
	{
		case	1:		sms.pSpriteFile = "Sprites\\smokepuff1.spr";	break;
		case	2:		sms.pSpriteFile = "Sprites\\smokepuff2.spr";	break;
		case	3:		sms.pSpriteFile = "Sprites\\smokepuff3.spr";	break;
		case	4:		sms.pSpriteFile = "Sprites\\smokepuff4.spr";	break;
		case	5:		sms.pSpriteFile = "Sprites\\smokepuff5.spr";	break;
		case	6:		sms.pSpriteFile = "Sprites\\smokepuff6.spr";	break;
	}

	if(m_nAmmoType == AMMO_SHELL)
	{
		sms.fDuration = 1.0f;
		sms.fInitAlpha = 0.25f;
		VEC_SET(sms.vMinScale, 0.04f, 0.04f, 0.0f);
		VEC_SET(sms.vMaxScale, 0.25f, 0.25f, 0.0f);
		VEC_MULSCALAR(sms.vVel, m_vDir, 2.0f);
		VEC_SUB(sms.vVel, sms.vVel, m_vForward);
		VEC_NORM(sms.vVel);
		VEC_MULSCALAR(sms.vVel, sms.vVel, 3.0f);
		VEC_MULSCALAR(sms.vDecel, m_vForward, -0.07f);
	}
	else
	{
		sms.fDuration = 1.5f;
		sms.fInitAlpha = 0.5f;
		VEC_SET(sms.vMinScale, 0.05f, 0.05f, 0.0f);
		VEC_SET(sms.vMaxScale, 0.25f, 0.25f, 0.0f);
		VEC_MULSCALAR(sms.vVel, m_vForward, 2.0f);
		VEC_MULSCALAR(sms.vDecel, m_vDir, -0.035f);
	}

	psfxMgr->CreateSFX(SFX_SMOKEIMPACT_ID, &sms, DFALSE, this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateShellCasing
//
//	PURPOSE:	Eject a shell from this puppy..
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateShellCasing()
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell) return;

	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return;

	SHELLCREATESTRUCT sc;

	// Fill in the structure for the shell
	m_pClientDE->AlignRotation(&sc.rRot, &m_vDir, DNULL);
	VEC_COPY(sc.vStartPos, m_vSourcePos);
	sc.bLeftHanded	= (m_nFXFlags & WFX_LEFTHANDED) ? DTRUE : DFALSE;
	sc.nAmmoType	= m_nAmmoType;

	psfxMgr->CreateSFX(SFX_SHELLCASING_ID, &sc, DFALSE, this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::GetImpactSound()
//
//	PURPOSE:	Gets a sound to play based on weapon, ammo and surface
//
// ----------------------------------------------------------------------- //

char* CWeaponFX::GetImpactSound(DBYTE nWeaponType, DBYTE nAmmoType, SurfaceType eSurfType)
{
	char*	pSoundFile = DNULL;
	char	calc = 0;

	switch(eSurfType)
	{
		case SURFTYPE_STONE:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\stone\\impact1.wav";	break;	}
		case SURFTYPE_METAL:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\metal\\impact1.wav";	break;	}
		case SURFTYPE_WOOD:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\wood\\impact1.wav";		break;	}
		case SURFTYPE_ENERGY:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\energy\\impact1.wav";	break;	}
		case SURFTYPE_GLASS:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\glass\\impact1.wav";	break;	}
		case SURFTYPE_BUILDING:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\stone\\impact1.wav";	break;	}
		case SURFTYPE_TERRAIN:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\terrain\\impact1.wav";	break;	}
		case SURFTYPE_CLOTH:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\cloth\\impact1.wav";	break;	}
		case SURFTYPE_PLASTIC:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\plastic\\impact1.wav";	break;	}
		case SURFTYPE_FLESH:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\flesh\\impact1.wav";	break;	}
		case SURFTYPE_LIQUID:
			{	calc = 1; pSoundFile = "Sounds\\Weapons\\impacts\\water\\impact1.wav";	break;	}
		default:
			{	break;	}
	}

	if(calc)
	{
		int length = _mbstrlen(pSoundFile);
		int nMax = NRES(4) - 1;
		int random = GetRandom(0,nMax);
		pSoundFile[length - 5] = '1' + random;
	}

	return pSoundFile;
}
