// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponFX.cpp
//
// PURPOSE : Weapon special FX - Implementation
//
// CREATED : 2/22/98
//
// ----------------------------------------------------------------------- //

#include "WeaponFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"
#include "WeaponFXTypes.h"
#include "RiotClientShell.h"
#include "MarkSFX.h"
#include "SparksFX.h"
#include "DynamicLightFX.h"
#include "BulletTrailFX.h"
#include "RiotMsgIDs.h"
#include "JuggernautFX.h"
#include "ShellCasingFX.h"
#include "ParticleExplosionFX.h"
#include "SpriteFX.h"
#include "ExplosionFX.h"
#include "DebrisFX.h"
#include "LineBallFX.h"
#include "CMoveMgr.h"

extern CRiotClientShell* g_pRiotClientShell;

#define	DEFAULT_WATER_IMPACT_SOUND_RADIUS	2000.0f
#define	DEFAULT_IMPACT_SOUND_RADIUS			500.0f

static uint32 s_nNumShells = 0;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::Init
//
//	PURPOSE:	Init the weapon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	WCREATESTRUCT* pCS = (WCREATESTRUCT*)psfxCreateStruct;

	uint8 nWeapon	= (pCS->nWeaponId & ~MODEL_FLAG_MASK);

	m_nWeaponId		= (RiotWeaponId)nWeapon;
	m_eSurfaceType	= (SurfaceType)pCS->nSurfaceType;
	m_nIgnoreFX		= pCS->nIgnoreFX;
	VEC_COPY(m_vFirePos, pCS->vFirePos);
	VEC_COPY(m_vPos, pCS->vPos);
	m_rRotation = pCS->rRot;

	m_eCode			= CC_NONE;
	m_eFirePosCode	= CC_NONE;
	m_nFX			= GetWeaponFX(m_nWeaponId);
	m_fDamage		= GetWeaponDamage(m_nWeaponId);
	uint8 nFlag		= (pCS->nWeaponId & MODEL_FLAG_MASK);

	m_nShooterId	= pCS->nShooterId;
	m_bLocal		= pCS->bLocal;

	// Clear all the fx we want to ignore...

	m_nFX &= ~m_nIgnoreFX;

	m_eSize = MS_NORMAL;

	if (nFlag & MODEL_SMALL_FLAG)
	{
		m_eSize = MS_SMALL;
	}
	else if (nFlag & MODEL_LARGE_FLAG)
	{
		m_eSize = MS_LARGE;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::CreateObject(ILTClient* pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !g_pRiotClientShell) return LTFALSE;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return LTFALSE;

	// Set the local client id...

	uint32 dwId;
	m_pClientDE->GetLocalClientID(&dwId);
	m_nLocalId = (uint8)dwId;

	m_nDetailLevel = pSettings->SpecialFXSetting();

	// Determine what container the sfx is in...

	HLOCALOBJ objList[1];
	uint32 dwNum = m_pClientDE->GetPointContainers(&m_vPos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
		uint32 dwUserFlags;
		m_pClientDE->Common()->GetObjectFlags(objList[0], OFT_User, dwUserFlags);

		if (dwUserFlags & USRFLG_VISIBLE)
		{
			uint16 dwCode;
			if (m_pClientDE->GetContainerCode(objList[0], &dwCode))
			{
				m_eCode = (ContainerCode)dwCode;
			}
		}
	}
			
	// Determine if the fire point is in liquid

	dwNum = m_pClientDE->GetPointContainers(&m_vFirePos, objList, 1);

	if (dwNum > 0 && objList[0])
	{
		uint32 dwUserFlags;
		m_pClientDE->Common()->GetObjectFlags(objList[0], OFT_User, dwUserFlags);

		if (dwUserFlags & USRFLG_VISIBLE)
		{
			uint16 dwCode;
			if (m_pClientDE->GetContainerCode(objList[0], &dwCode))
			{
				m_eFirePosCode = (ContainerCode)dwCode;
			}
		}
	}


	// Set up our data members...

	VEC_SUB(m_vDir, m_vPos, m_vFirePos);
	m_fFireDistance = VEC_MAG(m_vDir);
	VEC_NORM(m_vDir);

	LTVector vU, vR;
	m_pClientDE->Common()->GetRotationVectors(m_rRotation, vU, vR, m_vSurfaceNormal);
	VEC_NORM(m_vSurfaceNormal);

	m_rSurfaceRot = m_rRotation;
	m_pClientDE->Math()->AlignRotation(m_rDirRot, m_vDir, LTVector(0, 1, 0));

	// Determine type of weapon...

	switch (m_nWeaponId)
	{
		case GUN_SPIDER_ID:
		case GUN_BULLGUT_ID:
		case GUN_JUGGERNAUT_ID:
		case GUN_SHREDDER_ID:
		case GUN_REDRIOT_ID:
		case GUN_ENERGYGRENADE_ID:
		case GUN_KATOGRENADE_ID:
		case GUN_TOW_ID:
			m_bExplosionWeapon = LTTRUE;
		break;

		default :
			m_bExplosionWeapon = LTFALSE;
		break;
	}

	
	// Make a 'pushaway sphere' if it's explosive.
	static LTFLOAT radScale = 0.8f;
	static LTFLOAT startDelay = 0.2f;
	static LTFLOAT duration = 0.5f;
	static LTFLOAT strength = 600.0f;
	if(m_bExplosionWeapon && g_pRiotClientShell)
	{
		g_pRiotClientShell->GetMoveMgr()->AddPusher(
			m_vPos, // Position
			(LTFLOAT)GetWeaponDamageRadius(m_nWeaponId, m_eSize) * radScale, // Radius
			startDelay, // Start delay
			duration, // Duration
			strength // Strength (velocity add each frame based on distance to center)
		);
	}

	
	// Determine if we only want to do certain fx...

	LTBOOL bOnlyDoVectorDamageFX = LTFALSE;
	LTBOOL bDoVectorDamageFX		= LTTRUE;
	LTBOOL bOnlyDoProjDamageFX	= LTFALSE;
	LTBOOL bCreateSparks			= LTTRUE;

	if (!DetermineDamageFX(bOnlyDoVectorDamageFX, bDoVectorDamageFX, 
						   bOnlyDoProjDamageFX, bCreateSparks)) 
	{
		return LTFALSE;
	}


	// If the surface is the sky, don't create any impact related fx...

	if (m_eSurfaceType != ST_SKY)
	{
		if (bDoVectorDamageFX)
		{
			if (bCreateSparks)
			{
				if ((m_nFX & WFX_SPARKS) && (m_nDetailLevel != RS_LOW))
				{
					CreateSparks();
				}
			}

			if (bOnlyDoProjDamageFX)
			{
				return LTFALSE;
			}

			CreateWeaponSpecificFX();
		}

		// If we are only doing vector damage fx, we're done...

		if (bOnlyDoVectorDamageFX)
		{
			return LTFALSE;
		}

		if ((m_bExplosionWeapon) && m_eSize != MS_SMALL)
		{
			TintScreen();
		}
	
		if ((m_nFX & WFX_MARK) && ShowsMark(m_eSurfaceType) && (m_eSize != MS_SMALL))
		{
			CreateMark();
		}

		if ((m_nFX & WFX_SMOKE) && ShowsMark(m_eSurfaceType) && (m_nDetailLevel == RS_HIGH))
		{
			CreateSmoke();
		}

		if (m_nDetailLevel == RS_HIGH)
		{
			CreateLightFX();
		}

		PlayImpactSound();
	}
	//else
	//{
	//	g_pRiotClientShell->PrintError("Surface Type 110 - Sky Brush?");
	//}

	
	// If we are only doing certain damage fx, we're done...

	if (bOnlyDoVectorDamageFX || bOnlyDoProjDamageFX)
	{
		return LTFALSE;
	}

		
	if (IsBulletTrailWeapon(m_nWeaponId))
	{
		if (IsLiquid(m_eFirePosCode))
		{
			if (m_nDetailLevel != RS_LOW)
			{
				CreateBulletTrail(&m_vFirePos);
			}
		}
	}


	if (GetWeaponType(m_nWeaponId) == VECTOR)
	{
		// If fx is in liquid, make smoke (bubbles)...

		if (IsLiquid(m_eCode) && (m_nDetailLevel != RS_LOW))
		{
			CreateSmoke();
		}

		
		// If the surface is liquid, create bullet trails...

		if (m_eSurfaceType == ST_LIQUID)
		{
			if (m_nDetailLevel != RS_LOW)
			{
				CreateSparks();
			}

			// Some weapons don't create bullet trails in liquid...

			if (IsBulletTrailWeapon(m_nWeaponId) && (m_nDetailLevel != RS_LOW))
			{
				CreateBulletTrail(&m_vPos);
			}
		}
	}
	


	// Always create these FX...Well, as long as the detail level is high
	// enough...

	if (m_nDetailLevel != RS_LOW)
	{
		// No tracers under water...

		if ((m_nFX & WFX_TRACER) && !IsLiquid(m_eCode))
		{
			CreateTracer();
		}

		if ((m_nFX & WFX_MUZZLE) && (m_nDetailLevel != RS_MED))
		{
			CreateMuzzleFX();
		}

		if ((m_nFX & WFX_SHELL) && (m_eSize != MS_SMALL))
		{
			CreateShell();
		}

		if ((m_nFX & WFX_LIGHT) && (m_eSize != MS_SMALL))
		{
			CreateMuzzleLight();
		}
	}

	if (m_nFX & WFX_FIRESOUND)
	{
		PlayFireSound();
	}

	CreateBeamFX();

	return LTFALSE;  // Just delete me, I'm done :)
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::DetermineDamageFX
//
//	PURPOSE:	Determine the damage fx that should be created
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::DetermineDamageFX(LTBOOL & bOnlyDoVectorDamageFX, 
								   LTBOOL & bDoVectorDamageFX, 
								   LTBOOL & bOnlyDoProjDamageFX,
								   LTBOOL & bCreateSparks)
{
	// Initialize...

	bOnlyDoVectorDamageFX	= LTFALSE;
	bDoVectorDamageFX		= LTTRUE;
	bOnlyDoProjDamageFX		= LTFALSE;
	bCreateSparks			= LTTRUE;

//#define SHOW_SERVER_WEAPON_FX
#ifdef SHOW_SERVER_WEAPON_FX
	return LTTRUE;
#endif

	// Only do these checks if this is a multiplayer game...

	if (!g_pRiotClientShell->IsMultiplayerGame()) return LTTRUE;


	ProjectileType eType = GetWeaponType(m_nWeaponId);

	// Check to see if this is a local fx only.  If so we don't want to do
	// weapon specific fx for certain weapons...Else, check to see if most of
	// the fx was already done (i.e., this fx is being done on the client
	// that initiated the fx).  If so, only do necessary stuff...

	LTBOOL bHitSomebody = (m_eSurfaceType == ST_MECHA || m_eSurfaceType == ST_FLESH);
			
	if (m_bLocal)
	{
		if (eType == VECTOR || eType == MELEE)
		{
			bDoVectorDamageFX = !bHitSomebody;
		}
		else if (eType == PROJECTILE)
		{
			bCreateSparks = !bHitSomebody;
		}
	}
	else if (m_nLocalId >= 0 && m_nLocalId == m_nShooterId)
	{
		// If this fx is for a vector or melee weapon, only continue fx if 
		// we hit a character...

		if (eType == VECTOR || eType == MELEE)
		{
			if (bHitSomebody)
			{
				bOnlyDoVectorDamageFX = LTTRUE;
			}
			else
			{
				return LTFALSE;  // No need to do anything...
			}
		}
		else if (eType == CANNON)
		{
			return LTFALSE;      // No need to do anything
		}
	}


	// Check to see if we should only do the effects associated with damaging
	// somebody...

	if (!m_bLocal && m_nLocalId == m_nShooterId)
	{
		if (eType == PROJECTILE)
		{
			if (m_nWeaponId != GUN_SPIDER_ID && m_nWeaponId != GUN_KATOGRENADE_ID)
			{
				if (bHitSomebody)
				{
					bOnlyDoProjDamageFX = LTTRUE;
				}
				else
				{
					return LTFALSE;  // No need to do anything...
				}
			}
		}
	}

	return LTTRUE;
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
	if (!g_pRiotClientShell || !m_pClientDE) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	char* pMarkSprite = GetImpactSprite(m_eSurfaceType, m_nWeaponId);
	if (!pMarkSprite) return;

	LTBOOL	bCreateSmoke = LTTRUE;
	LTFLOAT  fSmokeSize   = 0.5f;
	LTFLOAT	fSmokeLife	 = (m_nDetailLevel == RS_MED) ? 0.5f : 1.0f;


	MARKCREATESTRUCT mark;

	VEC_COPY(mark.m_vPos, m_vPos);
	mark.m_Rotation = m_rSurfaceRot;
	mark.m_fScale = 0.075f;
	mark.m_hstrSprite = m_pClientDE->CreateString(pMarkSprite);

	LTFLOAT fLowScale = 0.8f, fHighScale = 1.5f;

	if (m_eSurfaceType == ST_GLASS)
	{
		fHighScale = 3.0f;
	}

	switch (m_nWeaponId)
	{
		case GUN_SHOTGUN_ID:
			mark.m_fScale = 0.05f;
			bCreateSmoke = LTFALSE;
		break;

		case GUN_LASERCANNON_ID:
			mark.m_fScale = 0.1f;
			bCreateSmoke = LTFALSE;
			fHighScale = 1.5f;
		break;

		case GUN_ASSAULTRIFLE_ID:
			mark.m_fScale = 0.1f;
		break;

		case GUN_SNIPERRIFLE_ID:
			mark.m_fScale = 0.1f;
			fSmokeSize   = 1.0f;
			fSmokeLife	 = (m_nDetailLevel == RS_MED) ? 1.0f : 2.0f;
		break;

		default : break;
	}


	// Randomly adjust the mark's scale to add a bit o spice...

	mark.m_fScale *= GetRandom(fLowScale, fHighScale);

	psfxMgr->CreateSFX(SFX_MARK_ID, &mark);

	// If low detail is set, or mark is under water, don't make smoke 
	// or bullet flash...

	if (m_nDetailLevel == RS_LOW || IsLiquid(m_eCode)) return;

	
	// Create smoke puff...

	SPRITECREATESTRUCT sc;
	CSpecialFX* pFX;

	if (bCreateSmoke)
	{
		VEC_COPY(sc.vPos, m_vPos);
		sc.vPos.y += 5.0f;
		VEC_SET(sc.vVel, 0.0f, GetRandom(10.0f, 20.0f), 0.0f);
		VEC_SET(sc.vInitialScale, 0.1f, 0.1f, 1.0f);
		VEC_SET(sc.vFinalScale, fSmokeSize, fSmokeSize, 1.0f);
		sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
		sc.fLifeTime		= fSmokeLife;
		sc.fInitialAlpha	= 1.0f;
		sc.fFinalAlpha		= 0.0f;
		sc.pFilename		= "Sprites\\SmokeTest.spr";

		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
		if (pFX) pFX->Update();
	}


	// Only create flames sometimes on medium detail setting...

	if (m_nDetailLevel == RS_MED && GetRandom(1, 3) != 1) return;

	// Don't create flame for laser cannon...

	if (m_nWeaponId == GUN_LASERCANNON_ID) return;


	// Create flame in bullet hole...

	VEC_COPY(sc.vPos, m_vPos);
	VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 0.10f, 0.10f, 1.0f);
	VEC_SET(sc.vFinalScale, 0.15f, 0.15f, 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
	sc.fLifeTime		= 0.1f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 1.0f;

	char* pImpactFiles[] = 
	{
		"Sprites\\Bulletholes\\ImpactFlame1.spr",
		"Sprites\\Bulletholes\\ImpactFlame2.spr",
		"Sprites\\Bulletholes\\ImpactFlame3.spr",
		"Sprites\\Bulletholes\\ImpactFlame4.spr",
		"Sprites\\Bulletholes\\ImpactFlame5.spr",
		"Sprites\\Bulletholes\\ImpactFlame6.spr"
	};

	sc.pFilename = pImpactFiles[GetRandom(0,5)];
	
	pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::TintScreen
//
//	PURPOSE:	Tint the screen based on the type of weapon
//
// ----------------------------------------------------------------------- //

void CWeaponFX::TintScreen()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	LTVector vTintColor;
	VEC_SET(vTintColor, 0.5f, 0.5f, 0.5f);
	LTFLOAT fRampUp = 0.2f, fRampDown = 0.6f, fTintTime = 0.1f;

	LTVector vShake;
	VEC_SET(vShake, 1.0f, 1.0f, 1.0f);

	switch (m_nWeaponId)
	{
		case GUN_SPIDER_ID:
		{	
			VEC_SET(vTintColor, 0.5f, 0.5f, 0.5f);
			fRampUp		= 0.2f;
			fRampDown	= 0.6f;
			fTintTime	= 0.1f;
		}
		break;
		case GUN_BULLGUT_ID:
		{	
			VEC_SET(vTintColor, 0.5f, 0.5f, 0.5f);
			fRampUp		= 0.2f;
			fRampDown	= 0.6f;
			fTintTime	= 0.1f;
		}
		break;
		case GUN_SHREDDER_ID:
		{	
			VEC_SET(vTintColor, 0.15f, 0.15f, 0.1f);
			fRampUp		= 0.2f;
			fRampDown	= 0.2f;
			fTintTime	= 0.1f;
		}
		break;
		case GUN_JUGGERNAUT_ID:
		{	
			VEC_SET(vTintColor, 0.7f, 0.7f, 0.5f);
			fRampUp		= 0.2f;
			fRampDown	= 0.6f;
			fTintTime	= 0.1f;
		}
		break;
		case GUN_REDRIOT_ID:
		{	
			VEC_SET(vShake, 3.0f, 3.0f, 3.0f);
			VEC_SET(vTintColor, 0.5f, 0.5f, 0.5f);
			fRampUp		= 0.2f;
			fRampDown	= 0.6f;
			fTintTime	= 0.2f;
		}
		break;
		case GUN_ENERGYGRENADE_ID:
		{	
			VEC_SET(vTintColor, 0.5f, 0.5f, 1.0f);
			fRampUp		= 0.2f;
			fRampDown	= 0.6f;
			fTintTime	= 0.1f;
		}
		break;
		case GUN_KATOGRENADE_ID:
		{	
			VEC_SET(vTintColor, 0.5f, 0.5f, 0.5f);
			fRampUp		= 0.2f;
			fRampDown	= 0.6f;
			fTintTime	= 0.1f;
		}
		break;
		case GUN_TOW_ID:
		{	
			VEC_SET(vTintColor, 0.7f, 0.7f, 0.5f);
			fRampUp		= 0.2f;
			fRampDown	= 0.6f;
			fTintTime	= 0.1f;
		}
		break;

		default : return;
	}

	LTFLOAT fRange = GetWeaponRange(m_nWeaponId) / 5.0f;
	g_pRiotClientShell->TintScreen(vTintColor, m_vPos, fRange, fRampUp, fTintTime, fRampDown);

	
	// If close enough, shake the screen...

	HLOCALOBJ hPlayerObj = m_pClientDE->GetClientObject();
	if (hPlayerObj)
	{
		LTVector vPlayerPos, vDir;
		m_pClientDE->GetObjectPos(hPlayerObj, &vPlayerPos);

		VEC_SUB(vDir, vPlayerPos, m_vPos);
		LTFLOAT fDist = VEC_MAG(vDir);

		LTFLOAT fRadius = (LTFLOAT) GetWeaponDamageRadius(m_nWeaponId, m_eSize);

		if (fDist < fRadius * 2.0f)
		{
			LTFLOAT fVal = fDist < 1.0f ? 5.0f : fRadius / fDist;
			fVal = fVal > 5.0f ? 5.0f : fVal;

			VEC_MULSCALAR(vShake, vShake, fVal);
			g_pRiotClientShell->ShakeScreen(vShake);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSparks
//
//	PURPOSE:	Create sparks
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateSparks()
{
	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTVector vColor1, vColor2;
	char* pSparkTexture = GetSparkTexture(m_nWeaponId, m_eSurfaceType, &vColor1, &vColor2);
	if (!pSparkTexture) return;

	LTVector vDir;
	VEC_MULSCALAR(vDir, m_vSurfaceNormal, 100.0f);


	uint8 nNumSparks = (m_fDamage < 50.0f) ? (uint8)m_fDamage : 50;
	nNumSparks = (m_eSurfaceType == ST_FLESH ? 50 : nNumSparks);

	SCREATESTRUCT sp;

	VEC_COPY(sp.vPos, m_vPos);
	VEC_COPY(sp.vDir, vDir);
	VEC_COPY(sp.vColor1, vColor1);
	VEC_COPY(sp.vColor2, vColor2);
	sp.hstrTexture		= m_pClientDE->CreateString(pSparkTexture);
	sp.nSparks			= nNumSparks;
	sp.fDuration		= 1.0f;
	sp.fEmissionRadius	= 0.3f;
	sp.fRadius			= (m_eSurfaceType == ST_FLESH ? 500.0f : 800.0f);
	sp.fGravity			= PSFX_DEFAULT_GRAVITY;

	if (m_eSurfaceType == ST_LIQUID)
	{
		VEC_MULSCALAR(sp.vDir, sp.vDir, 3.0f);
		sp.fEmissionRadius	= 0.2f;
		sp.fRadius			= 600.0f;
	}

	// Boost sparks up a bit for expolsion weapons...

	if (m_bExplosionWeapon)
	{
		VEC_MULSCALAR(sp.vDir, sp.vDir, 2.0f);
		sp.nSparks			= 100;
		sp.fEmissionRadius	= 1.0f;
		sp.fRadius			= (m_eSurfaceType == ST_FLESH ? 800.0f : 
							  (m_eSurfaceType == ST_LIQUID ? 1500.0f : 3000.0f));
		sp.fGravity			= 0.0f;
	}
	else if (GetWeaponType(m_nWeaponId) == MELEE)
	{
		sp.fGravity			= 0.0f;
	}
	else if (m_nWeaponId == GUN_PULSERIFLE_ID)
	{
		VEC_MULSCALAR(sp.vDir, sp.vDir, 3.0f);
		sp.fEmissionRadius	= 1.0f;
		sp.fRadius			= (m_eSurfaceType == ST_FLESH ? 800.0f : 
							  (m_eSurfaceType == ST_LIQUID ? 1500.0f : 3000.0f));
	}

	psfxMgr->CreateSFX(SFX_SPARKS_ID, &sp);

	m_pClientDE->FreeString(sp.hstrTexture);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateBulletTrail
//
//	PURPOSE:	Create a bullet trail fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateBulletTrail(LTVector *pvStartPos)
{
	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr || !pvStartPos) return;

	LTVector vColor1, vColor2;
	VEC_SET(vColor1, 200.0f, 200.0f, 200.0f);
	VEC_SET(vColor2, 255.0f, 255.0f, 255.0f);

	BTCREATESTRUCT bt;

	VEC_COPY(bt.vStartPos, *pvStartPos);
	VEC_COPY(bt.vDir, m_vDir);
	VEC_COPY(bt.vColor1, vColor1);
	VEC_COPY(bt.vColor2, vColor2);
	bt.fLifeTime		= 0.5f;
	bt.fFadeTime		= 0.3f;
	bt.fRadius			= 400.0f;
	bt.fGravity			= 0.0f;
	bt.fNumParticles	= (m_nDetailLevel == RS_MED) ? 15.0f : 30.0f;

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_BULLETTRAIL_ID, &bt);

	// Let each bullet trail do its initial update...

	if (pFX) pFX->Update();
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
	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	if (m_nDetailLevel == RS_MED && GetRandom(1, 2) == 1) return;

	TRCREATESTRUCT tr;

	tr.rRot = m_rDirRot;
	VEC_COPY(tr.vPos, m_vPos);
	VEC_COPY(tr.vVel, m_vDir);
	SetTracerValues(m_nWeaponId, &tr);

	psfxMgr->CreateSFX(SFX_TRACER_ID, &tr);
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
	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	SMCREATESTRUCT sm;

	VEC_COPY(sm.vPos, m_vPos);
	SetupSmoke(m_nWeaponId, m_eSurfaceType, &sm);

	psfxMgr->CreateSFX(SFX_SMOKE_ID, &sm);

	m_pClientDE->FreeString(sm.hstrTexture);


	// Create a blast mark to go with the smoke...

	if (m_eSurfaceType != ST_LIQUID && !IsLiquid(m_eCode))
	{
		//CreateBlastMark();
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::GetSparkTexture()
//
//	PURPOSE:	Get a spark texture associated with this weapon and surface
//
// ----------------------------------------------------------------------- //

char* CWeaponFX::GetSparkTexture(RiotWeaponId nWeaponId, SurfaceType eSurfType,
								 LTVector* pvColor1, LTVector* pvColor2)
{
	if (!pvColor1 || !pvColor2) return LTNULL;

	char* szBloodSparks[2] = { "SpecialFX\\ParticleTextures\\Blood_1.dtx", 
							   "SpecialFX\\ParticleTextures\\Blood_2.dtx" };

	char* pTexture = LTNULL;

	VEC_SET(*pvColor1, 200.0f, 200.0f, 200.0f);
	VEC_SET(*pvColor2, 255.0f, 255.0f, 255.0f);

	if (eSurfType == ST_FLESH)
	{
		CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
		if (pSettings && pSettings->Gore())
		{
			pTexture = szBloodSparks[GetRandom(0, 1)];
		}
	}
	else if (eSurfType == ST_LIQUID || IsLiquid(m_eCode))
	{
		VEC_SET(*pvColor1, 235.0f, 235.0f, 255.0f);
		VEC_SET(*pvColor2, 255.0f, 255.0f, 255.0f);
		GetLiquidColorRange(m_eCode, pvColor1, pvColor2);
		pTexture = "SpecialFX\\ParticleTextures\\GreySphere_1.dtx";
	}
	else if (GetWeaponType(nWeaponId) == MELEE)
	{
		switch (nWeaponId)
		{
			case GUN_ENERGYBATON_ID :
			{
				VEC_SET(*pvColor1, 255.0f, 127.0f, 0.0f);
				VEC_SET(*pvColor2, 255.0f, 160.0f, 0.0f);
				pTexture = "SpecialFX\\ParticleTextures\\EnergyBaton.dtx";
			}
			break;

			case GUN_ENERGYBLADE_ID :
			{
				VEC_SET(*pvColor1, 0.0f, 0.0f, 200.0f);
				VEC_SET(*pvColor2, 0.0f, 0.0f, 255.0f);
				pTexture = "SpecialFX\\ParticleTextures\\EnergyBlade.dtx";
			}
			break;

			case GUN_KATANA_ID :
			{
				VEC_SET(*pvColor1, 0.0f, 200.0f, 0.0f);
				VEC_SET(*pvColor2, 0.0f, 255.0f, 0.0f);
				pTexture = "SpecialFX\\ParticleTextures\\Katana.dtx";
			}
			break;

			case GUN_MONOKNIFE_ID :
			{
				VEC_SET(*pvColor1, 200.0f, 0.0f, 200.0f);
				VEC_SET(*pvColor2, 255.0f, 0.0f, 255.0f);
				pTexture = "SpecialFX\\ParticleTextures\\Monoknife.dtx";
			}
			break;

			case GUN_TANTO_ID :
			{
				//	VEC_SET(*pvColor1, 200.0f, 200.0f, 200.0f);
				//	VEC_SET(*pvColor2, 255.0f, 255.0f, 255.0f);
				//	pTexture = "SpecialFX\\ParticleTextures\\Tanto.dtx";
			}
			break;

			default : break;
		}
	}
	else
	{
		switch (nWeaponId)
		{
			case GUN_SHREDDER_ID:
			{
				VEC_SET(*pvColor1, 150.0f, 150.0f, 150.0f);
				VEC_SET(*pvColor2, 230.0f, 230.0f, 230.0f);
				pTexture = "Sprites\\glow.spr";
			}
			break;

			case GUN_JUGGERNAUT_ID:
			{
				VEC_SET(*pvColor1, 155.0f, 155.0f, 155.0f);
				VEC_SET(*pvColor2, 255.0f, 255.0f, 255.0f);
				pTexture = "SpecialFX\\ParticleTextures\\Particle.dtx";
			}
			break;

			case GUN_SPIDER_ID:
			{
				return LTNULL;
			}
			break;

			case GUN_REDRIOT_ID:
			case GUN_KATOGRENADE_ID:
			case GUN_TOW_ID:
			case GUN_BULLGUT_ID:
			{
				VEC_SET(*pvColor1, 255.0f, 255.0f, 255.0f);
				VEC_SET(*pvColor2, 255.0f, 255.0f, 255.0f);
				pTexture = "Sprites\\glow.spr";
			}
			break;

			case GUN_ENERGYGRENADE_ID:
			{
				VEC_SET(*pvColor1, 255.0f, 255.0f, 255.0f);
				VEC_SET(*pvColor2, 255.0f, 255.0f, 255.0f);
				pTexture = "Sprites\\grenade1.spr";
			}
			break;

			case GUN_PULSERIFLE_ID:
			{
				VEC_SET(*pvColor1, 0.0f, 255.0f, 255.0f);
				VEC_SET(*pvColor2, 255.0f, 255.0f, 255.0f);
				pTexture = "Sprites\\glow.spr";
			}
			break;

			default :
				pTexture = "SpecialFX\\ParticleTextures\\Spark_yellow_1.dtx";
			break;
		}
	}

	return (pTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::SetTracerValues()
//
//	PURPOSE:	Set tracer values associated with this weapon
//
// ----------------------------------------------------------------------- //

void CWeaponFX::SetTracerValues(RiotWeaponId nWeaponId, TRCREATESTRUCT* pTR)
{
	LTFLOAT fStartAlpha = 0.1f;
	LTFLOAT fEndAlpha   = 0.9f;

	LTVector vStartColor, vEndColor;
	VEC_SET(vStartColor, 1.0f, 0.8f, 0.0f);
	VEC_SET(vEndColor,   1.0f, 0.0f, 0.0f);

	switch (nWeaponId)
	{
		case GUN_SNIPERRIFLE_ID:
		{
			fStartAlpha = 0.1f;
			fEndAlpha   = 0.9f;
			VEC_SET(vStartColor, 1.0f, 0.8f, 0.0f);
			VEC_SET(vEndColor,   1.0f, 0.0f, 0.0f);
		}
		break;

		case GUN_MAC10_ID:
		{
			fStartAlpha = 0.1f;
			fEndAlpha   = 0.9f;
			VEC_SET(vStartColor, 1.0f, 1.0f, 0.0f);
			VEC_SET(vEndColor,   1.0f, 0.5f, 0.0f);
		}
		break;

		case GUN_ASSAULTRIFLE_ID:
		{
			fStartAlpha = 0.1f;
			fEndAlpha   = 0.9f;
			VEC_SET(vStartColor, 1.0f, 0.8f, 0.0f);
			VEC_SET(vEndColor,   1.0f, 0.0f, 0.0f);
		}
		break;


		default : break;
	}

	VEC_COPY(pTR->vStartColor, vStartColor);
	VEC_COPY(pTR->vEndColor, vEndColor);
	VEC_COPY(pTR->vStartPos, m_vFirePos);
	pTR->fStartAlpha	= fStartAlpha;
	pTR->fEndAlpha		= fEndAlpha;
	pTR->nWeaponId		= nWeaponId;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateWeaponSpecificFX()
//
//	PURPOSE:	Create weapon specific fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateWeaponSpecificFX()
{
	if (!m_pClientDE) return;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return;

	uint8 nVal = pSettings->SpecialFXSetting();

	switch (m_nWeaponId)
	{
		case GUN_REDRIOT_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowRedRiotFX();
				break;
				case RS_MED:
					CreateMedRedRiotFX();
				break;
				case RS_HIGH:
				default :
					CreateRedRiotFX();
				break;
			}
		}
		break;

		case GUN_JUGGERNAUT_ID:
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowJuggernautFX();
				break;
				case RS_MED:
					CreateMedJuggernautFX();
				break;
				case RS_HIGH:
				default :
					CreateJuggernautFX();
				break;
			}
		}
		break;

		case GUN_BULLGUT_ID:
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowBullgutFX();
				break;
				case RS_MED:
					CreateMedBullgutFX();
				break;
				case RS_HIGH:
				default :
					CreateBullgutFX();
				break;
			}
		}
		break;

		case GUN_PULSERIFLE_ID:
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowPulseRifleFX();
				break;
				case RS_MED:
					CreateMedPulseRifleFX();
				break;
				case RS_HIGH:
				default :
					CreatePulseRifleFX();
				break;
			}
		}
		break;

		case GUN_SHREDDER_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowShredderFX();
				break;
				case RS_MED:
					CreateMedShredderFX();
				break;
				case RS_HIGH:
				default :
					CreateShredderFX();
				break;
			}
		}
		break;

		case GUN_SNIPERRIFLE_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowSniperRifleFX();
				break;
				case RS_MED:
					CreateMedSniperRifleFX();
				break;
				case RS_HIGH:
				default :
					CreateSniperRifleFX();
				break;
			}
		}
		break;

		case GUN_SPIDER_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowSpiderFX();
				break;
				case RS_MED:
					CreateMedSpiderFX();
				break;
				case RS_HIGH:
				default :
					CreateSpiderFX();
				break;
			}
		}
		break;

		case GUN_COLT45_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowColt45FX();
				break;
				case RS_MED:
					CreateMedColt45FX();
				break;
				case RS_HIGH:
				default :
					CreateColt45FX();
				break;
			}
		}
		break;

		case GUN_SHOTGUN_ID	:
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowShotgunFX();
				break;
				case RS_MED:
					CreateMedShotgunFX();
				break;
				case RS_HIGH:
				default :
					CreateShotgunFX();
				break;
			}
		}
		break;

		case GUN_ASSAULTRIFLE_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowAssaultRifleFX();
				break;
				case RS_MED:
					CreateMedAssaultRifleFX();
				break;
				case RS_HIGH:
				default :
					CreateAssaultRifleFX();
				break;
			}
		}
		break;

		case GUN_MAC10_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowMac10FX();
				break;
				case RS_MED:
					CreateMedMac10FX();
				break;
				case RS_HIGH:
				default :
					CreateMac10FX();
				break;
			}
		}
		break;

		case GUN_KATOGRENADE_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowKatoGrenadeFX();
				break;
				case RS_MED:
					CreateMedKatoGrenadeFX();
				break;
				case RS_HIGH:
				default :
					CreateKatoGrenadeFX();
				break;
			}
		}
		break;

		case GUN_ENERGYGRENADE_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowEnergyGrenadeFX();
				break;
				case RS_MED:
					CreateMedEnergyGrenadeFX();
				break;
				case RS_HIGH:
				default :
					CreateEnergyGrenadeFX();
				break;
			}
		}
		break;

		case GUN_TOW_ID	:
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowTOWFX();
				break;
				case RS_MED:
					CreateMedTOWFX();
				break;
				case RS_HIGH:
				default :
					CreateTOWFX();
				break;
			}
		}
		break;

		case GUN_TANTO_ID	:
		{
			switch (nVal)
			{
				case RS_LOW:
					CreateLowTantoFX();
				break;
				case RS_MED:
					CreateMedTantoFX();
				break;
				case RS_HIGH:
				default :
					CreateTantoFX();
				break;
			}
		}
		break;


		// Currently no fx for the following...

		case GUN_ENERGYBATON_ID :
			CreateEnergyBatonFX();
		break;

		case GUN_ENERGYBLADE_ID :
			CreateEnergyBladeFX();
		break;

		case GUN_KATANA_ID :
			CreateKatanaFX();
		break;

		case GUN_MONOKNIFE_ID :
			CreateMonoKnifeFX();
		break;

		case GUN_LASERCANNON_ID :
			CreateLaserCannonFX();
		break;

		default : 
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateBeamFX()
//
//	PURPOSE:	Create beam fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateBeamFX()
{
	if (!m_pClientDE) return;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return;

	uint8 nVal = pSettings->SpecialFXSetting();

	switch (m_nWeaponId)
	{
		case GUN_REDRIOT_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
				break;

				case RS_MED:
				case RS_HIGH:
				default :
					CreateRedRiotBeam();
				break;
			}
		}
		break;

		case GUN_JUGGERNAUT_ID:
		{
			switch (nVal)
			{
				case RS_LOW:
				break;

				case RS_MED:
				case RS_HIGH:
				default :
					CreateJuggernautBeam();
				break;
			}
		}
		break;

		case GUN_SHREDDER_ID :
		{
			switch (nVal)
			{
				case RS_LOW:
				break;

				case RS_MED:
				case RS_HIGH:
				default :
					CreateShredderBeam();
				break;
			}
		}
		break;

		case GUN_LASERCANNON_ID :
		{
			switch (nVal)
			{
				// Always create the laser beam...

				case RS_LOW:
				case RS_MED:
				case RS_HIGH:
				default :
					CreateLaserCannonBeam();
				break;
			}
		}
		break;

		default : 
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLightFX()
//
//	PURPOSE:	Create light associated with this fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLightFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	LTBOOL bCreateLight = LTTRUE;
	DLCREATESTRUCT dl;

	VEC_COPY(dl.vPos, m_vPos);

	// Initialize for vector weapon...

	dl.fMinRadius    = 10.0f;
	dl.fMaxRadius	 = 30.0f;
	dl.fRampUpTime	 = 0.2f;
	dl.fMaxTime		 = 0.0f;
	dl.fMinTime		 = 0.0f;
	dl.fRampDownTime = 0.0f;
	dl.dwFlags		 = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING; // | FLAG_SOLIDLIGHT;

	VEC_SET(m_vLightColor, 0.98f, 0.98f, 0.75f);

	switch (m_nWeaponId)
	{
		case GUN_SNIPERRIFLE_ID :
		case GUN_COLT45_ID :
		case GUN_SHOTGUN_ID	:
		case GUN_ASSAULTRIFLE_ID :
		case GUN_MAC10_ID :
			if (IsLiquid(m_eCode)) return;
		break;

		case GUN_LASERCANNON_ID :
		{
			dl.fMinRadius    = 25.0f;
			dl.fMaxRadius	 = 50.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.1f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.2f;
			VEC_SET(m_vLightColor, 1.0f, 0.8f, 0.7f);
		}
		break;

		case GUN_SPIDER_ID :
		{
			dl.fMinRadius    = 50.0f;
			dl.fMaxRadius	 = 300.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.3f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.4f;
			VEC_SET(m_vLightColor, 0.98f, 0.98f, 0.1f);
		}
		break;

		case GUN_KATOGRENADE_ID :
		{
			dl.fMinRadius    = 50.0f;
			dl.fMaxRadius	 = 300.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.3f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.4f;
			VEC_SET(m_vLightColor, 0.98f, 0.5f, 0.0f);
		}
		break;

		case GUN_ENERGYGRENADE_ID :
		{
			dl.fMinRadius    = 50.0f;
			dl.fMaxRadius	 = 200.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.3f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.4f;
			VEC_SET(m_vLightColor, 0.65f, 0.98f, 0.98f);
		}
		break;

		case GUN_TOW_ID	:
		{
			dl.fMinRadius    = 50.0f;
			dl.fMaxRadius	 = 300.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.3f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.4f;
			VEC_SET(m_vLightColor, 0.98f, 0.5f, 0.0f);
		}
		break;

		case GUN_REDRIOT_ID :
		{
			dl.fMinRadius	 = 100.0f;
			dl.fMaxRadius	 = 700.0f;
			dl.fRampUpTime	 = 0.75f;
			dl.fMaxTime		 = 1.5f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 1.0f;
			VEC_SET(m_vLightColor, 0.98f, 0.5f, 0.0f);
		}
		break;

		case GUN_SHREDDER_ID :
		{
			dl.fMinRadius    = 25.0f;
			dl.fMaxRadius	 = 75.0f;
			dl.fRampUpTime	 = 0.2f;
			dl.fMaxTime		 = 0.3f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.2f;
			VEC_SET(m_vLightColor, 0.98f, 0.98f, 0.75f);
		}
		break;

		case GUN_JUGGERNAUT_ID:
		{
			dl.fMinRadius    = 50.0f;
			dl.fMaxRadius	 = 150.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.3f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.4f;
			VEC_SET(m_vLightColor, 0.98f, 0.98f, 0.75f);
		}
		break;

		case GUN_BULLGUT_ID:
		{
			dl.fMinRadius    = 25.0f;
			dl.fMaxRadius	 = 100.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.5f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.4f;
			VEC_SET(m_vLightColor, 0.98f, 0.98f, 0.75f);
		}
		break;

		case GUN_PULSERIFLE_ID:
		{
			dl.fMinRadius    = 50.0f;
			dl.fMaxRadius	 = 100.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.1f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.2f;
			VEC_SET(m_vLightColor, 0.65f, 0.98f, 0.98f);
		}
		break;

		case GUN_ENERGYBATON_ID:
		{
			dl.fMinRadius    = 25.0f;
			dl.fMaxRadius	 = 75.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.1f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.2f;
			VEC_SET(m_vLightColor, 0.98f, 0.5f, 0.0f);
		}
		break;

		case GUN_ENERGYBLADE_ID:
		{
			dl.fMinRadius    = 25.0f;
			dl.fMaxRadius	 = 75.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.1f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.2f;
			VEC_SET(m_vLightColor, 0.1f, 0.1f, 0.9f);
		}
		break;

		case GUN_KATANA_ID:
		{
			dl.fMinRadius    = 25.0f;
			dl.fMaxRadius	 = 75.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.1f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.2f;
			VEC_SET(m_vLightColor, 0.1f, 0.9f, 0.1f);
		}
		break;

		case GUN_MONOKNIFE_ID:
		{
			dl.fMinRadius    = 25.0f;
			dl.fMaxRadius	 = 75.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.1f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.2f;
			VEC_SET(m_vLightColor, 0.9f, 0.1f, 0.9f);
		}
		break;

		case GUN_TANTO_ID:
		{
			dl.fMinRadius    = 25.0f;
			dl.fMaxRadius	 = 75.0f;
			dl.fRampUpTime	 = 0.3f;
			dl.fMaxTime		 = 0.1f;
			dl.fMinTime		 = 0.0f;
			dl.fRampDownTime = 0.2f;
			VEC_SET(m_vLightColor, 0.9f, 0.9f, 0.9f);
		}
		break;

		default : 
			bCreateLight = LTFALSE;
		break;
	}

	if (bCreateLight)
	{
		CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
		if (!psfxMgr) return;

		VEC_COPY(dl.vColor, m_vLightColor);

		if (m_eSize == MS_SMALL)
		{
			dl.fMinRadius /= 5.0f;
			dl.fMaxRadius /= 5.0f;
		}
		else if (m_eSize == MS_LARGE)
		{
			dl.fMinRadius *= 5.0f;
			dl.fMaxRadius *= 5.0f;
		}

		psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);	
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMuzzleLight()
//
//	PURPOSE:	Create a muzzle light associated with this fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMuzzleLight()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	if (g_pRiotClientShell->HaveSilencer()) return;

	DLCREATESTRUCT dl;
	VEC_COPY(dl.vPos, m_vFirePos);
	dl.fMinRadius    = GetRandom(50.0f, 75.0f);
	dl.fMaxRadius	 = GetRandom(75.0f, 100.0f);
	dl.fRampUpTime	 = 0.0f;
	dl.fMaxTime		 = 0.15f;
	dl.fMinTime		 = 0.0f;
	dl.fRampDownTime = 0.0f;
	dl.dwFlags		 = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING; // | FLAG_SOLIDLIGHT;

	VEC_SET(dl.vColor, .9f, .7f, .7f);

	switch (m_nWeaponId)
	{
		case GUN_LASERCANNON_ID :
		{
			VEC_SET(dl.vColor, 0.98f, 0.2f, 0.2f);
		}
		break;

		default : 
		break;
	}

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	if (m_eSize == MS_SMALL)
	{
		return;
	}
	else if (m_eSize == MS_LARGE)
	{
		dl.fMinRadius *= 5.0f;
		dl.fMaxRadius *= 5.0f;
	}

	psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::SetupSmoke()
//
//	PURPOSE:	Setup the smoke struct based on the weapon
//
// ----------------------------------------------------------------------- //

void CWeaponFX::SetupSmoke(RiotWeaponId nWeaponId, SurfaceType eSurfType, SMCREATESTRUCT* pSM)
{
	if (!pSM) return;

	char* pTexture = "Sprites\\SmokeTest.spr";

	VEC_SET(pSM->vColor1, 100.0f, 100.0f, 100.0f);
	VEC_SET(pSM->vColor2, 150.0f, 150.0f, 150.0f);
	VEC_SET(pSM->vMinDriftVel, -10.0f, 25.0f, -10.0f);
	VEC_SET(pSM->vMaxDriftVel, 10.0f, 50.0f, 10.0f);

	LTFLOAT fVolumeRadius		= 10.0f;
	LTFLOAT fLifeTime			= GetRandom(m_fDamage/18.0f, m_fDamage/12.0f);
	LTFLOAT fRadius				= 2000 + m_fDamage*2.0f;
	LTFLOAT fParticleCreateDelta	= (m_nDetailLevel == RS_MED) ? 0.2f : 0.1f;
	LTFLOAT fMinParticleLife		= 1.0f;
	LTFLOAT fMaxParticleLife		= (m_nDetailLevel == RS_MED) ? 2.5f : 5.0f;
	uint8  nNumParticles		= (m_nDetailLevel == RS_MED) ? 2 : 3;
	LTBOOL  bIgnoreWind			= LTFALSE;

	if (nWeaponId == GUN_REDRIOT_ID || nWeaponId == GUN_KATOGRENADE_ID)
	{
		if (nWeaponId == GUN_REDRIOT_ID)
		{
			fVolumeRadius = 20.0f;
		}

		VEC_SET(pSM->vColor1, 200.0f, 150.0f, 0.0f);
		VEC_SET(pSM->vColor2, 255.0f, 200.0f, 0.0f);
	}
	
	if (nWeaponId == GUN_LASERCANNON_ID)
	{
		if (eSurfType == ST_LIQUID)
		{
			fLifeTime = 1.0f;
		}
		else
		{
			fLifeTime = 2.0f;
		}

		if (IsLiquid(m_eCode))
		{
			GetLiquidColorRange(m_eCode, &pSM->vColor1, &pSM->vColor2);
			pTexture = "SpecialFX\\ParticleTextures\\GreySphere_1.dtx";
		}

		fVolumeRadius			= 1.0f;
		fRadius					= 700;
		fParticleCreateDelta	= 0.4f;
		fMinParticleLife		= 0.5f;
		fMaxParticleLife		= 1.0f;
		nNumParticles			= 2;
	}
	else if (IsLiquid(m_eCode))
	{
		GetLiquidColorRange(m_eCode, &pSM->vColor1, &pSM->vColor2);
		pTexture = "SpecialFX\\ParticleTextures\\GreySphere_1.dtx";
		fRadius		  = 750.0f;
		bIgnoreWind	  = LTTRUE;

		if (GetWeaponType(nWeaponId) == VECTOR)
		{
			fLifeTime				= 0.5f;
			fVolumeRadius			= 1.0f;
			fRadius					= 700;
			fParticleCreateDelta	= 0.1f;
			fMinParticleLife		= 0.5f;
			fMaxParticleLife		= 1.0f;
			nNumParticles			= 1;
		}
		else
		{
			fLifeTime				= 2.5f;
			fMinParticleLife		= 1.0f;
			fMaxParticleLife		= 1.5f;
		}
	}

	pSM->fVolumeRadius			= fVolumeRadius;
	pSM->fLifeTime				= fLifeTime;
	pSM->fRadius				= fRadius;
	pSM->fParticleCreateDelta	= fParticleCreateDelta;
	pSM->fMinParticleLife		= fMinParticleLife;
	pSM->fMaxParticleLife		= fMaxParticleLife;
	pSM->nNumParticles			= nNumParticles;
	pSM->bIgnoreWind			= bIgnoreWind;
	pSM->hstrTexture			= m_pClientDE->CreateString(pTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::IsBulletTrailWeapon()
//
//	PURPOSE:	See if this weapon creates bullet trails in liquid
//
// ----------------------------------------------------------------------- //

LTBOOL CWeaponFX::IsBulletTrailWeapon(RiotWeaponId nWeaponId)
{
	LTBOOL bRet = LTTRUE;

	if (GetWeaponType(nWeaponId) != VECTOR) return LTFALSE;

	switch (nWeaponId)
	{
		case GUN_SQUEAKYTOY_ID:
		case GUN_LASERCANNON_ID:
			bRet = LTFALSE;
		break;

		default : break;
	}

	return bRet;
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

	if (GetWeaponType(m_nWeaponId) == VECTOR)
	{
		if ((m_nDetailLevel == RS_LOW) && GetRandom(1, 5) != 1) return;
		else if ((m_nDetailLevel == RS_MED) && GetRandom(1, 2) != 1) return;
	}

	if (m_eSurfaceType == ST_LIQUID && (m_nWeaponId != GUN_SQUEAKYTOY_ID))
	{
		char* pLiquidImpacts[] = { "Sounds\\Weapons\\WaterImpact1.wav",
								   "Sounds\\Weapons\\WaterImpact2.wav" };

		PlaySoundFromPos(&m_vPos, pLiquidImpacts[GetRandom(0,1)], 
						 DEFAULT_WATER_IMPACT_SOUND_RADIUS, 
						 SOUNDPRIORITY_MISC_HIGH);
	}
	else
	{
		char*	pImpactSound = GetImpactSound(m_eSurfaceType, m_nWeaponId);
		LTFLOAT	fImpactSoundRadius	= DEFAULT_IMPACT_SOUND_RADIUS;

		switch (m_nWeaponId)
		{
			case GUN_SPIDER_ID :
				fImpactSoundRadius = 1000.0f;
			break;

			case GUN_BULLGUT_ID :
				fImpactSoundRadius = 1500.0f;
			break;

			case GUN_SHREDDER_ID :
				fImpactSoundRadius = 1000.0f;
			break;

			case GUN_JUGGERNAUT_ID :
				fImpactSoundRadius = 2000.0f;
			break;

			case GUN_REDRIOT_ID :
				fImpactSoundRadius = 6000.0f;
			break;

			case GUN_ENERGYGRENADE_ID :
				fImpactSoundRadius = 2500.0f;
			break;

			case GUN_KATOGRENADE_ID :
				fImpactSoundRadius = 2500.0f;
			break;

			case GUN_TOW_ID	:
				fImpactSoundRadius = 2500.0f;
			break;

			default : 
			break;
		}

		if (pImpactSound)
		{
			uint8 nVolume = 100; // IsLiquid(m_eCode) ? 80 : 100;
			PlaySoundFromPos(&m_vPos, pImpactSound, fImpactSoundRadius, 
							 SOUNDPRIORITY_MISC_HIGH,
							 LTFALSE, LTFALSE, LTFALSE, nVolume);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateMuzzleFX()
//
//	PURPOSE:	Create muzzle specific fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMuzzleFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	char* pTexture = "Sprites\\SmokeTest.spr";

	if (IsLiquid(m_eFirePosCode))
	{
		pTexture = "SpecialFX\\ParticleTextures\\GreySphere_1.dtx";
	}

	LTVector vDir;
	VEC_MULSCALAR(vDir, m_vSurfaceNormal, 10.0f);

	SCREATESTRUCT sp;

	VEC_COPY(sp.vPos, m_vFirePos);
	VEC_COPY(sp.vDir, vDir);
	sp.hstrTexture = m_pClientDE->CreateString(pTexture);


	// Create smoke...

	sp.nSparks = 1;
	sp.fRadius = 400.0f;

	switch (m_nWeaponId)
	{
		case GUN_SHOTGUN_ID:
			sp.nSparks = 10;
			sp.fRadius = 800.0f;
		break;
	}

	VEC_SET(sp.vColor1, 100.0f, 100.0f, 100.0f);
	VEC_SET(sp.vColor2, 125.0f, 125.0f, 125.0f);
	sp.fDuration		= 1.0f;
	sp.fEmissionRadius	= 0.05f;
	sp.fRadius			= 800.0f;
	sp.fGravity			= 0.0f;

	if (IsLiquid(m_eFirePosCode))
	{
		switch (m_nWeaponId)
		{
			case GUN_SHOTGUN_ID:
				sp.nSparks = 1;
			break;
		}

		GetLiquidColorRange(m_eFirePosCode, &sp.vColor1, &sp.vColor2);
		sp.fRadius			= 600.0f;
		sp.fGravity			= 50.0f;
	}

	psfxMgr->CreateSFX(SFX_SPARKS_ID, &sp);

	m_pClientDE->FreeString(sp.hstrTexture);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateShell()
//
//	PURPOSE:	Create shell casing
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateShell()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	switch ((RiotWeaponId)m_nWeaponId)
	{
		case GUN_SHOTGUN_ID	:
		break;

		case GUN_SNIPERRIFLE_ID :
		case GUN_COLT45_ID :
		case GUN_ASSAULTRIFLE_ID :
		case GUN_MAC10_ID :
		case GUN_JUGGERNAUT_ID :
		case GUN_SHREDDER_ID :
		{
			// Only create every other shell if medium detail set...

			if (m_nDetailLevel == RS_MED && (++s_nNumShells % 2 == 0)) return;
		}
		break;

		default : return;
	}	

	SHELLCREATESTRUCT sc;
	sc.rRot = m_rDirRot;
	VEC_COPY(sc.vStartPos, m_vFirePos);
	sc.nWeaponId = m_nWeaponId;

	psfxMgr->CreateSFX(SFX_SHELLCASING_ID, &sc);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateJuggernautFX
//
//	PURPOSE:	Create the juggernaut fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateJuggernautFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTFLOAT fFactor = m_eSize == MS_SMALL ? 0.2f : (m_eSize == MS_LARGE ? 5.0f : 1.0f);

	// Create model explosion...

	EXCREATESTRUCT ex;
	VEC_COPY(ex.vPos, m_vPos);

	LTFLOAT fRadius = GetWeaponDamageRadius(m_nWeaponId, m_eSize) * 2.0f;

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, 50.0f, 50.0f, 50.0f);
	VEC_MULSCALAR(ex.vInitialScale, ex.vInitialScale, fFactor);
	VEC_SET(ex.vFinalScale, fRadius, fRadius, fRadius);
	VEC_SET(ex.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(ex.vFinalColor, 1.0f, 1.0f, 1.0f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 1.0f;
	ex.fInitialAlpha	= 0.3f;
	ex.fFinalAlpha		= 0.3f;
	ex.pFilename		= "Models\\PV_Weapons\\JuggernautExplosion.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\juggernaut.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();


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
//	ROUTINE:	CWeaponFX::CreateJuggernautBeam
//
//	PURPOSE:	Create the juggernaut beam fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateJuggernautBeam()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create model explosion...

	EXCREATESTRUCT ex;

	LTFLOAT fMaxRange = GetWeaponRange(m_nWeaponId);
	LTFLOAT fDistance = m_fFireDistance;
	if (fDistance > fMaxRange) fDistance = fMaxRange;

	LTVector vPos, vTemp;
	VEC_MULSCALAR(vTemp, m_vDir, fDistance/2.0f);
	VEC_ADD(vPos, m_vFirePos, vTemp);

	VEC_COPY(ex.vPos, vPos);
	ex.rRot = m_rDirRot;

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, 1.8f, 1.8f, fDistance);
	VEC_SET(ex.vFinalScale, 0.1f, 0.1f, fDistance);
	VEC_SET(ex.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(ex.vFinalColor, 1.0f, 1.0f, 1.0f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 1.0f;
	ex.fInitialAlpha	= 0.8f;
	ex.fFinalAlpha		= 0.0f;
	ex.pFilename		= "Models\\Powerups\\beam.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\Juggernaut.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateBullgutFX
//
//	PURPOSE:	Create the bullgut fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateBullgutFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create model explosion...

	EXCREATESTRUCT ex;
	VEC_COPY(ex.vPos, m_vPos);

	LTFLOAT fRadius = (LTFLOAT)GetWeaponDamageRadius(m_nWeaponId, m_eSize);

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, fRadius/5.0f, fRadius/5.0f, fRadius/5.0f);
	VEC_SET(ex.vFinalScale, fRadius, fRadius, fRadius);
	VEC_SET(ex.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(ex.vFinalColor, 1.0f, 1.0f, 1.0f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 1.0f;
	ex.fInitialAlpha	= 0.6f;
	ex.fFinalAlpha		= 0.0f;
	ex.pFilename		= "Models\\PV_Weapons\\RedRiotExplosion.abc";
	ex.pSkin			= "SpriteTextures\\weapons\\Explosions\\BllgCore.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();

	if (g_pRiotClientShell->IsAnime()) return;


	// Create explosion sprite...

	SPRITECREATESTRUCT sc;

	LTVector vPos;
	VEC_MULSCALAR(vPos, m_vDir, -20.0f);
	VEC_ADD(vPos, vPos, m_vPos);

	VEC_COPY(sc.vPos, vPos);
	VEC_SET(sc.vVel, 0.0f, 15.0f, 0.0f);

	//VEC_SET(sc.vInitialScale, 0.5f, 0.5f, 1.0f);
	//VEC_SET(sc.vFinalScale, 2.50f, 2.5f, 1.0f);
	VEC_SET(sc.vInitialScale, 0.5f, 0.5f, 1.0f);
	VEC_SET(sc.vFinalScale, 1.0f, 1.0f, 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
	sc.fLifeTime		= 1.5f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\weapons\\BllgtExp.spr";


	pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();


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

	
	LTVector vVelMin, vVelMax;
	VEC_SET(vVelMin, -400.0f, 400.0f, -400.0f)
	VEC_SET(vVelMax, 400.0f, 700.0f, 400.0f)

	// Create a particle explosion...

	PESCREATESTRUCT pe;

	pe.bCreateDebris	= LTTRUE;
	pe.bRotateDebris	= LTTRUE;
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
	pe.nNumEmmitters	= 3;
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

void CWeaponFX::CreatePulseRifleFX()
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


	// Rotate to be flat to direction fired...

	m_pClientDE->Math()->AlignRotation(sc.rRot, m_vDir, LTVector(0, 1, 0));

	pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateRedRiotFX
//
//	PURPOSE:	Create the red riot fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateRedRiotFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CSpecialFX* pFX;

	int		nNumRings = 2;
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

	for (int i=0; i < nNumRings; i++)
	{
		ex.fDelayTime = i*(ex.fLifeTime/nNumRings);
		pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
		if (pFX) pFX->Update();
	}
	

	// Create the line ball...

	if (g_pRiotClientShell->IsAnime())
	{
		LBCREATESTRUCT lb;

		lb.rRot = m_rDirRot;
		VEC_COPY(lb.vPos, m_vPos);
		VEC_SET(lb.vStartColor, 1.0f, 0.0f, 0.0f);
		VEC_SET(lb.vEndColor, 1.0f, 0.8f, 0.0f);
		VEC_SET(lb.vInitialScale, 0.25f, 0.25f, 0.25f);
		VEC_SET(lb.vFinalScale, 10.0f, 10.0f, 10.0f);
		lb.fSystemStartAlpha	= 0.5f;
		lb.fSystemEndAlpha		= 0.0f;
		lb.fStartAlpha			= 0.5f;
		lb.fEndAlpha			= 0.5f;
		lb.fOffset				= 20.0f;
		lb.fLifeTime			= 2.5f;
		lb.fLineLength			= 200;

		psfxMgr->CreateSFX(SFX_LINEBALL_ID, &lb);
	}

	
	// Create shockwave sprite...

	SPRITECREATESTRUCT sc;

	VEC_COPY(sc.vPos, m_vPos);
	m_pClientDE->Common()->SetupEuler(sc.rRot, MATH_HALFPI, 0.0f, 0.0f);
	VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 1.0f, 1.0f, 1.0f);
	VEC_SET(sc.vFinalScale, 10.f, 10.0f, 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT; 
	sc.fLifeTime		= 1.0f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\Weapons\\RedRiot.spr";

	for (int i=0; i < nNumRings; i++)
	{
		sc.fDelayTime = i*(fLifeTime/nNumRings);
		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
		if (pFX) pFX->Update();
	}


	// Create smoke bank...

	VEC_SET(sc.vVel, 0.0f, 10.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 2.5f, 2.0f, 1.0f);
	VEC_SET(sc.vFinalScale, 18.0f, 18.0f, 1.0f);
	VEC_SET(sc.vInitialColor, 1.0f, 0.5f, 0.0f);
	VEC_SET(sc.vFinalColor,   0.0f, 0.0f, 0.0f);
	sc.bUseUserColors = LTTRUE;
	sc.fDelayTime = 0.0f;

	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
	sc.fLifeTime		= 15.0f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\SmokeTest.spr";

	pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();

	sc.vPos.x += GetRandom(-50.0f, 50.0f);
	sc.vPos.z += GetRandom(-50.0f, 50.0f);
	VEC_SET(sc.vVel, 0.0f, 15.0f, 0.0f);
	sc.fLifeTime		= 13.0f;

	pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateRedRiotBeam
//
//	PURPOSE:	Create the red riot beam fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateRedRiotBeam()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create beam...

	LTFLOAT	fLifeTime = 2.0f;

	LTFLOAT fMaxRange = GetWeaponRange(m_nWeaponId);
	LTFLOAT fDistance = m_fFireDistance;
	if (fDistance > fMaxRange) fDistance = fMaxRange;
	
	LTVector vTemp, vPos;
	VEC_MULSCALAR(vTemp, m_vDir, fDistance/2.0f);
	VEC_ADD(vPos, m_vFirePos, vTemp);

	EXCREATESTRUCT ex;
	VEC_COPY(ex.vPos, vPos);
	ex.rRot = m_rDirRot;

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vFinalScale, 0.1f, 0.1f, fDistance);
	VEC_SET(ex.vInitialScale, 2.0f, 2.0f, fDistance);
	VEC_SET(ex.vInitialColor, 1.0f, .5f, .1f);
	VEC_SET(ex.vFinalColor, 1.0f, .5f, .1f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 0.5f;
	ex.fInitialAlpha	= 0.8f;
	ex.fFinalAlpha		= 0.0f;
	ex.pFilename		= "Models\\Powerups\\beam.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\RedRiotBeam.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateShredderFX
//
//	PURPOSE:	Create the shredder fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateShredderFX()
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


	// Create smoke bank...

	VEC_COPY(sc.vPos, m_vPos);
	VEC_SET(sc.vVel, 0.0f, 10.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 0.1f, 0.1f, 1.0f);
	VEC_SET(sc.vFinalScale, 1.0f, 1.0f, 1.0f);
	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
	sc.fLifeTime		= 4.0f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\SmokeTest.spr";

	pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateShredderBeam
//
//	PURPOSE:	Create the shredder beam fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateShredderBeam()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Create beam...

	LTVector vPos;
	LTFLOAT fMaxRange = GetWeaponRange(m_nWeaponId);
	LTFLOAT fDistance = m_fFireDistance;
	if (fDistance > fMaxRange) fDistance = fMaxRange;
	
	LTVector vTemp;
	VEC_MULSCALAR(vTemp, m_vDir, fDistance/2.0f);
	VEC_ADD(vPos, m_vFirePos, vTemp);

	EXCREATESTRUCT ex;

	VEC_COPY(ex.vPos, vPos);
	ex.rRot = m_rDirRot;

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, 1.0f, 1.0f, fDistance);
	VEC_SET(ex.vFinalScale, 0.1f, 0.1f, fDistance);
	VEC_SET(ex.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(ex.vFinalColor, 1.0f, 1.0f, 1.0f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 0.25f;
	ex.fInitialAlpha	= 0.8f;
	ex.fFinalAlpha		= 0.0f;
	ex.pFilename		= "Models\\Powerups\\beam.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\JuggernautBeam.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateLaserCannonBeam
//
//	PURPOSE:	Create the laser cannon beam fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateLaserCannonBeam()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CSpecialFX* pFX;

	// Create beam...

	LTFLOAT fMaxRange = GetWeaponRange(m_nWeaponId);

	LTFLOAT fDistance = m_fFireDistance;
	if (fDistance > fMaxRange) fDistance = fMaxRange;
	
	LTVector vTemp, vPos;
	VEC_MULSCALAR(vTemp, m_vDir, fDistance/2.0f);
	VEC_ADD(vPos, m_vFirePos, vTemp);

	EXCREATESTRUCT ex;
	VEC_COPY(ex.vPos, vPos);
	ex.rRot = m_rDirRot;

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, 0.8f, 0.8f, fDistance);
	VEC_SET(ex.vFinalScale, 0.1f, 0.1f, fDistance);
	VEC_SET(ex.vInitialColor, 1.0f, 0.1f, 0.1f);
	VEC_SET(ex.vFinalColor, 1.0f, 0.1f, 0.1f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 0.5f;
	ex.fInitialAlpha	= 0.9f;
	ex.fFinalAlpha		= 0.0f;
	ex.pFilename		= "Models\\Powerups\\beam.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\Beam.dtx";

	pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSpiderFX
//
//	PURPOSE:	Create the spider fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateSpiderFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	int nNumFireballs = 2;

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

#ifdef SHEYOT

	// Create explosion sprite...

	SPRITECREATESTRUCT sc;

	VEC_COPY(sc.vPos, m_vPos);
	VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(sc.vInitialScale, 0.5f, 0.5f, 1.0f);
	VEC_SET(sc.vFinalScale, 1.0f, 1.0f, 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
	sc.fLifeTime		= 1.0f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\weapons\\UtlltyLnchrExp.spr";


	// Create smoke...

	SPRITECREATESTRUCT sc2;

	LTVector vPos;
	VEC_SET(sc2.vInitialScale, 1.25f, 1.25f, 0.0f);
	VEC_SET(sc2.vFinalScale, 4.0f, 4.0f, 0.0f);
	VEC_SET(sc2.vVel, 0.0f, 15.0f, 0.0f);
	sc2.fLifeTime		= 8.0f;
	sc2.fInitialAlpha	= 1.0f;
	sc2.fFinalAlpha		= 0.0f;
	sc2.pFilename		= "Sprites\\SmokeTest.spr";


	for (int i=0; i < nNumFireballs; i++)
	{
		VEC_MULSCALAR(vPos, m_vDir, -50.0f*i);
		VEC_ADD(sc.vPos, ex.vPos, vPos);

#define USING_SPRITES
#ifdef USING_SPRITES
		sc.vFinalScale.x -= i*1.5f;
		sc.vFinalScale.y -= i*1.5f;

		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
		if (pFX) pFX->Update();
#endif

		sc2.vFinalScale.x -= i*.5f;
		sc2.vFinalScale.y -= i*.5f;
		sc2.vVel.y += GetRandom(-10.0f, 10.0f);
		sc2.vVel.x += GetRandom(-10.0f, 10.0f);
		sc2.vVel.z += GetRandom(-10.0f, 10.0f);
		VEC_MULSCALAR(vPos, m_vDir, -10.0f);
		VEC_ADD(sc2.vPos, sc.vPos, vPos);
		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc2);
		if (pFX) pFX->Update();
	}
#endif
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateSniperRifleFX
//
//	PURPOSE:	Create the sniper rifle fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateSniperRifleFX()
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
		LTFLOAT fRange = 200.0f;

		CreateVectorBloodFX(vVelMin, vVelMax, fRange);
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
//	ROUTINE:	CWeaponFX::CreateTOWFX
//
//	PURPOSE:	Create the tow fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateTOWFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	int nNumFireballs = 2;

	LTFLOAT fFactor = m_eSize == MS_SMALL ? 0.2f : (m_eSize == MS_LARGE ? 5.0f : 1.0f);

	// Create model explosion...

	EXCREATESTRUCT ex;
	VEC_COPY(ex.vPos, m_vPos);

	LTFLOAT fRadius = GetWeaponDamageRadius(m_nWeaponId, m_eSize) * 2.0f;

	VEC_SET(ex.vVel, 0.0f, 0.0f, 0.0f);
	VEC_SET(ex.vInitialScale, fRadius/5.0f, fRadius/5.0f, fRadius/5.0f);
	VEC_MULSCALAR(ex.vInitialScale, ex.vInitialScale,fFactor);
	VEC_SET(ex.vFinalScale, fRadius, fRadius, fRadius);
	VEC_MULSCALAR(ex.vFinalScale, ex.vFinalScale, fFactor);
	VEC_SET(ex.vInitialColor, 1.0f, 1.0f, 1.0f);
	VEC_SET(ex.vFinalColor, 1.0f, 1.0f, 1.0f);
	ex.bUseUserColors = LTTRUE;

	ex.dwFlags			= FLAG_VISIBLE | FLAG_NOLIGHT;
	ex.fLifeTime		= 1.0f;
	ex.fInitialAlpha	= 0.6f;
	ex.fFinalAlpha		= 0.0f;
	ex.pFilename		= "Models\\PV_Weapons\\RedRiotExplosion.abc";
	ex.pSkin			= "SpriteTextures\\weapons\\Explosions\\BllgCore.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();


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

	pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();


	// Create smoke...

	SPRITECREATESTRUCT sc2;

	LTVector vPos;
	VEC_SET(sc2.vInitialScale, 1.25f, 1.25f, 0.0f);
	VEC_MULSCALAR(sc2.vInitialScale, sc2.vInitialScale, fFactor);
	VEC_SET(sc2.vFinalScale, 4.0f, 4.0f, 0.0f);
	VEC_MULSCALAR(sc2.vFinalScale, sc2.vFinalScale, fFactor);
	VEC_SET(sc2.vVel, 0.0f, 15.0f, 0.0f);
	sc2.fLifeTime		= 8.0f;
	sc2.fInitialAlpha	= 1.0f;
	sc2.fFinalAlpha		= 0.0f;
	sc2.pFilename		= "Sprites\\SmokeTest.spr";


	for (int i=0; i < nNumFireballs; i++)
	{
		VEC_MULSCALAR(vPos, m_vDir, -50.0f*i);
		VEC_ADD(sc.vPos, sc.vPos, vPos);

		sc2.vFinalScale.x -= i*.5f;
		sc2.vFinalScale.y -= i*.5f;

		sc2.vVel.y += GetRandom(-10.0f, 10.0f);
		sc2.vVel.x += GetRandom(-10.0f, 10.0f);
		sc2.vVel.z += GetRandom(-10.0f, 10.0f);
		VEC_MULSCALAR(vPos, m_vDir, -10.0f);
		VEC_ADD(sc2.vPos, sc.vPos, vPos);
		sc2.vPos.y -= 100.0f;

		pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc2);
		if (pFX) pFX->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateKatoGrenadeFX
//
//	PURPOSE:	Create the kato grenade fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateKatoGrenadeFX()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	int nNumFireballs = 2;

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
	

	// Create smoke...

	LTVector vPos;
	VEC_SET(sc.vInitialScale, 1.25f, 1.25f, 0.0f);
	VEC_SET(sc.vFinalScale, 4.0f, 4.0f, 0.0f);
	VEC_SET(sc.vInitialColor, 1.0f, 0.5f, 0.0f);
	VEC_SET(sc.vFinalColor,   0.0f, 0.0f, 0.0f);
	VEC_SET(sc.vVel, 0.0f, 15.0f, 0.0f);
	sc.bUseUserColors   = LTTRUE;
	sc.fLifeTime		= 5.0f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.pFilename		= "Sprites\\SmokeTest.spr";
	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 

	sc.vVel.y += GetRandom(-10.0f, 10.0f);
	sc.vVel.x += GetRandom(-10.0f, 10.0f);
	sc.vVel.z += GetRandom(-10.0f, 10.0f);
	VEC_MULSCALAR(vPos, m_vDir, -10.0f);
	VEC_ADD(sc.vPos, sc.vPos, vPos);

	pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateEnergyGrenadeFX
//
//	PURPOSE:	Create the energy grenade fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateEnergyGrenadeFX()
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
	ex.fLifeTime		= 0.75f;
	ex.fInitialAlpha	= 0.5f;
	ex.fFinalAlpha		= 0.0f;
	ex.pFilename		= "Models\\PV_Weapons\\EnergyGrenadeExplosion.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\EnergyGrenade.dtx";

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();

	LTVector vUp;
	VEC_SET(vUp, 1.0f, 0.0f, 1.0f);
	m_pClientDE->Math()->AlignRotation(ex.rRot, vUp, LTVector(0, 1, 0));

	ex.fInitialAlpha	= 0.9f;
	ex.fFinalAlpha		= 0.2f;
	ex.pFilename		= "Models\\PV_Weapons\\EnergyGrenadeExplosionCore.abc";
	ex.pSkin			= "SpecialFX\\Explosions\\EnergyGrenadeCore.dtx";

	pFX = psfxMgr->CreateSFX(SFX_EXPLOSION_ID, &ex);
	if (pFX) pFX->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateTantoFX
//
//	PURPOSE:	Create the tanto fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateTantoFX()
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

		CreateVectorBloodFX(vVelMin, vVelMax, fRange);
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
//	ROUTINE:	CWeaponFX::CreateColt45FX
//
//	PURPOSE:	Create the colt 45 fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateColt45FX()
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

		CreateVectorBloodFX(vVelMin, vVelMax, fRange);
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
//	ROUTINE:	CWeaponFX::CreateShotgunFX
//
//	PURPOSE:	Create the shotgun fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateShotgunFX()
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

		CreateVectorBloodFX(vVelMin, vVelMax, fRange);
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
//	ROUTINE:	CWeaponFX::CreateAssaultRifleFX
//
//	PURPOSE:	Create the assault rifle fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateAssaultRifleFX()
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

		CreateVectorBloodFX(vVelMin, vVelMax, fRange);
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
//	ROUTINE:	CWeaponFX::CreateMac10FX
//
//	PURPOSE:	Create the mac 10 fx
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateMac10FX()
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

		CreateVectorBloodFX(vVelMin, vVelMax, fRange);
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
//	ROUTINE:	CWeaponFX::CreateVectorBloodFX
//
//	PURPOSE:	Create the blood trail, splats, etc.
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateVectorBloodFX(LTVector & vVelMin, LTVector & vVelMax, LTFLOAT fRange)
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

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SPARKS_ID, &sp);
	if (pFX) 
	{
		CBaseParticleSystemFX* pBasePS = (CBaseParticleSystemFX*)pFX;
		pBasePS->m_bSetSoftwareColor = LTFALSE;
	}

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
	pe.nNumPerPuff		= 2;
	pe.nNumEmmitters	= 2;
	pe.bIgnoreWind		= LTTRUE;
	pe.pFilename		= szBlood[GetRandom(0,1)];
	pe.nSurfaceType		= m_eSurfaceType;
	pe.nNumSteps		= 6;

	pFX = psfxMgr->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) 
	{
		CBaseParticleSystemFX* pBasePS = (CBaseParticleSystemFX*)pFX;
		pBasePS->m_bSetSoftwareColor = LTFALSE;

		pFX->Update();
	}

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


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::CreateBlastMark
//
//	PURPOSE:	Create a blast mark...
//
// ----------------------------------------------------------------------- //

void CWeaponFX::CreateBlastMark()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	LTFLOAT fRadius = (LTFLOAT)GetWeaponDamageRadius(m_nWeaponId, m_eSize);
	if (fRadius < 100.0f) return;

	// Create a dynamic light for the blast mark :)

	DLCREATESTRUCT dl;

	VEC_COPY(dl.vPos, m_vPos);
	VEC_SET(dl.vColor, 0.27f, 0.27f, 0.27f);

	dl.fMinRadius    = fRadius / 4.0f;
	dl.fMaxRadius	 = fRadius / 4.0f;
	dl.fRampUpTime	 = 0.0f;
	dl.fMaxTime		 = 30.0f;
	dl.fMinTime		 = 0.0f;
	dl.fRampDownTime = 60.0f;
	dl.dwFlags		 = FLAG_VISIBLE | FLAG_ONLYLIGHTWORLD | FLAG_DONTLIGHTBACKFACING;

	//psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);	

	// Create the center of the blast...

	VEC_COPY(dl.vColor, m_vLightColor);
	dl.fMinRadius    = fRadius / 6.0f;
	dl.fMaxRadius	 = fRadius / 6.0f;
	dl.fRampUpTime	 = 0.0f;
	dl.fMaxTime		 = 5.0f;
	dl.fMinTime		 = 0.0f;
	dl.fRampDownTime = 5.0f;

	psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);	
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CWeaponFX::PlayFireSound
//
//	PURPOSE:	Play the fire sound
//
// ----------------------------------------------------------------------- //

void CWeaponFX::PlayFireSound()
{
	if (!m_pClientDE || !g_pRiotClientShell) return;

	if (m_nLocalId >= 0 && m_nLocalId == m_nShooterId)
	{
		return;  // This client already heard the sound ;)
	}

	char* pFireSound = GetWeaponFireSound(m_nWeaponId);
	if (pFireSound)
	{
		PlaySoundFromPos(&m_vFirePos, pFireSound, WEAPON_SOUND_RADIUS, SOUNDPRIORITY_PLAYER_HIGH);
	}
}
