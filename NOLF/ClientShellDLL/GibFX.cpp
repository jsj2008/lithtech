// ----------------------------------------------------------------------- //
//
// MODULE  : GibFX.cpp
//
// PURPOSE : Gib - Implementation
//
// CREATED : 6/15/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GibFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "SurfaceMgr.h"
#include "GameClientShell.h"
#include "ParticleSystemFX.h"
#include "SFXMsgIds.h"
#include "ParticleTrailFX.h"
#include "ParticleExplosionFX.h"
#include "ParticleShowerFX.h"
#include "BaseScaleFX.h"
#include "DebrisMgr.h"
#include "SmokeFX.h"
#include "WeaponFXTypes.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CGibFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	GIBCREATESTRUCT* pGib = (GIBCREATESTRUCT*)psfxCreateStruct;
    m_rRot = pGib->rRot;
	VEC_COPY(m_vPos, pGib->vPos);
	VEC_COPY(m_vMinVel, pGib->vMinVel);
	VEC_COPY(m_vMaxVel, pGib->vMaxVel);
	m_fLifeTime			= pGib->fLifeTime;
	m_fFadeTime			= pGib->fFadeTime;
	m_nGibFlags			= pGib->nGibFlags;
	m_bRotate			= pGib->bRotate;
	m_eCode				= (ContainerCode) pGib->nCode;
	m_eModelId			= pGib->eModelId;
	m_eModelStyle		= pGib->eModelStyle;
	m_nNumGibs			= pGib->nNumGibs;
	m_bSubGibs			= pGib->bSubGibs;
	m_bBloodSplats		= pGib->bBloodSplats;

	for (int i=0; i < m_nNumGibs; i++)
	{
		m_eGibTypes[i] = pGib->eGibTypes[i];
	}

	m_nNumRandomGibs = GetRandom(3, 6);
	m_nNumGibs += m_nNumRandomGibs;
	if (m_nNumGibs > MAX_GIB) m_nNumGibs = MAX_GIB;


	if (m_bRotate)
	{
        LTFLOAT fVal = GetRandom(MATH_CIRCLE/4.0f, MATH_CIRCLE/2.0f);
		m_fPitchVel = GetRandom(-fVal, fVal);
		m_fYawVel	= GetRandom(-fVal, fVal);
		m_fRollVel	= GetRandom(-fVal, fVal);
	}

	m_eModelType = g_pModelButeMgr->GetModelType(m_eModelId);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CGibFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	// Initialize the Gib velocity ranges based on our rotation...

    LTVector vVelMin, vVelMax, vTemp, vU, vR, vF;
	VEC_SET(vVelMin, 1.0f, 1.0f, 1.0f);
	VEC_SET(vVelMax, 1.0f, 1.0f, 1.0f);

	m_pClientDE->GetRotationVectors(&m_rRot, &vU, &vR, &vF);

	if (vF.y <= -0.95f || vF.y >= 0.95f)
	{
		vF.y = vF.y > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, 1.0f);
	}
	else if (vF.x <= -0.95f || vF.x >= 0.95f)
	{
		vF.x = vF.x > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 0.0f, 1.0f, 0.0f);
		VEC_SET(vU, 0.0f, 0.0f, 1.0f);
	}
	else if (vF.z <= -0.95f || vF.z >= 0.95f)
	{
		vF.z = vF.z > 0.0f ? 1.0f : -1.0f;
		VEC_SET(vR, 1.0f, 0.0f, 0.0f);
		VEC_SET(vU, 0.0f, 1.0f, 0.0f);
	}

	VEC_MULSCALAR(vVelMin, vF, m_vMinVel.y);
	VEC_MULSCALAR(vVelMax, vF, m_vMaxVel.y);

	VEC_MULSCALAR(vTemp, vR, m_vMinVel.x);
	VEC_ADD(vVelMin, vVelMin, vTemp);

	VEC_MULSCALAR(vTemp, vR, m_vMaxVel.x);
	VEC_ADD(vVelMax, vVelMax, vTemp);

	VEC_MULSCALAR(vTemp, vU, m_vMinVel.z);
	VEC_ADD(vVelMin, vVelMin, vTemp);

	VEC_MULSCALAR(vTemp, vU, m_vMaxVel.z);
	VEC_ADD(vVelMax, vVelMax, vTemp);


	// Initialize our emitters...

    LTVector vVel;
	for (int i=0; i < m_nNumGibs; i++)
	{
		if (i < m_nNumGibs - m_nNumRandomGibs)
		{
			m_hGib[i] = CreateGib(m_eGibTypes[i]);
		}
		else
		{
			m_hGib[i] = CreateRandomGib();
		}

		m_fGibLife[i] = GetRandom(m_fLifeTime, m_fLifeTime * 2.0f);

		m_pGibTrail[i] = CreateGibTrail(m_hGib[i]);

        m_ActiveEmitters[i] = LTTRUE;
		m_BounceCount[i]	 = GetRandom(2, 5);

		VEC_SET(vVel, GetRandom(vVelMin.x, vVelMax.x),
					  50.0f + GetRandom(vVelMin.y, vVelMax.y),
					  GetRandom(vVelMin.z, vVelMax.z));

		InitMovingObject(&(m_Emitters[i]), &m_vPos, &vVel);
		m_Emitters[i].m_dwPhysicsFlags |= m_nGibFlags;
	}


	// Create a big burst of blood...

	if ( m_eModelType == eModelTypeHuman )
	{
		//CreateBloodSpray();
	}


	// Play die sound...

	char* pSound = GetGibDieSound();
	if (pSound)
	{
		g_pClientSoundMgr->PlaySoundFromPos(m_vPos, pSound, 1000.0f,
			SOUNDPRIORITY_MISC_LOW);
	}


	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::Update
//
//	PURPOSE:	Update the Gib
//
// ----------------------------------------------------------------------- //

LTBOOL CGibFX::Update()
{
    if (!m_pClientDE) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	if (m_bFirstUpdate)
	{
        m_bFirstUpdate = LTFALSE;
		m_fStartTime   = fTime;
		m_fLastTime	   = fTime;
	}


	// Check to see if we should start fading the Gib...

	if (fTime > m_fStartTime + m_fFadeTime)
	{
        int i;
        for (i=0; i < m_nNumGibs; i++)
		{
            LTFLOAT fEndTime = m_fStartTime + m_fGibLife[i];

			if (fTime > fEndTime)
			{
				if (OkToRemoveGib(m_hGib[i]))
				{
					if (m_hGib[i])
					{
						m_pClientDE->DeleteObject(m_hGib[i]);
                        m_hGib[i] = LTNULL;
					}
				}
			}
		}

		// See if all the gibs have been removed or not...

		for (i=0; i < m_nNumGibs; i++)
		{
			if (m_hGib[i]) break;
		}

		// All gibs have been removed so remove us...

		if (i == m_nNumGibs)
		{
			RemoveAllFX();
            return LTFALSE;
		}

// #define FADING_GIBS
#ifdef FADING_GIBS
        LTFLOAT fScale = (fEndTime - fTime) / (m_fLifeTime - m_fFadeTime);

        LTFLOAT r, g, b, a;

		for (int i=0; i < m_nNumGibs; i++)
		{
			if (m_hGib[i])
			{
				m_pClientDE->GetObjectColor(m_hGib[i], &r, &g, &b, &a);
				m_pClientDE->SetObjectColor(m_hGib[i], r, g, b, fScale);
			}
		}
#endif
	}


	// Loop over our list of emitters, updating the position of each

	for (int i=0; i < m_nNumGibs; i++)
	{
		if (m_ActiveEmitters[i])
		{
            LTBOOL bBounced = LTFALSE;
			if (bBounced = UpdateEmitter(&m_Emitters[i]))
			{
				HandleBounce(i);
			}

			UpdateGib(i, bBounced);
		}
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::UpdateEmitter
//
//	PURPOSE:	Update emitter position
//
// ----------------------------------------------------------------------- //

LTBOOL CGibFX::UpdateEmitter(MovingObject* pObject)
{
    if (!m_pClientDE || !pObject || pObject->m_dwPhysicsFlags & MO_RESTING) return LTFALSE;

    LTBOOL bRet = LTFALSE;

    LTVector vNewPos;
    if (UpdateMovingObject(LTNULL, pObject, &vNewPos))
	{
		LTBOOL bBouncedOnGround = LTFALSE;
        uint32 dwFlags = (INTERSECT_HPOLY | INTERSECT_OBJECTS | IGNORE_NONSOLID);
       
		bRet = BounceMovingObject(LTNULL, pObject, &vNewPos, &m_info, 
			dwFlags, bBouncedOnGround);

		pObject->m_vLastPos = pObject->m_vPos;
		pObject->m_vPos = vNewPos;

        if (m_pClientDE->GetPointStatus(&vNewPos) == LT_OUTSIDE)
		{
			pObject->m_dwPhysicsFlags |= MO_RESTING;
			pObject->m_vPos = pObject->m_vLastPos;
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::UpdateGib
//
//	PURPOSE:	Update gib pos/rot
//
// ----------------------------------------------------------------------- //

void CGibFX::UpdateGib(int nIndex, LTBOOL bBounced)
{
	if (nIndex < 0 || nIndex >= m_nNumGibs) return;

	if (m_Emitters[nIndex].m_dwPhysicsFlags & MO_RESTING)
	{
        m_ActiveEmitters[nIndex] = LTFALSE;
		if (m_pGibTrail[nIndex])
		{
			debug_delete(m_pGibTrail[nIndex]);
            m_pGibTrail[nIndex] = LTNULL;
		}

		if (m_hGib[nIndex])
		{
			if (m_bRotate)
			{
                LTRotation rRot;
				m_pClientDE->SetupEuler(&rRot, 0.0f, m_fYaw, m_fRoll);
				m_pClientDE->SetObjectRotation(m_hGib[nIndex], &rRot);
			}

			// m_pClientDE->SetObjectPos(m_hGib[nIndex], &(m_info.m_Point));
		}
	}
	else if (m_hGib[nIndex])
	{
		m_pClientDE->SetObjectPos(m_hGib[nIndex], &(m_Emitters[nIndex].m_vPos));

		if (m_bRotate)
		{
			if (bBounced)
			{
				// Adjust due to the bounce...

                LTFLOAT fVal = GetRandom(MATH_CIRCLE/4.0f, MATH_CIRCLE/2.0f);
				m_fPitchVel = GetRandom(-fVal, fVal);
				m_fYawVel	= GetRandom(-fVal, fVal);
				m_fRollVel	= GetRandom(-fVal, fVal);
			}

			if (m_fPitchVel != 0 || m_fYawVel != 0 || m_fRollVel)
			{
                LTFLOAT fDeltaTime = g_pGameClientShell->GetFrameTime();

				m_fPitch += m_fPitchVel * fDeltaTime;
				m_fYaw   += m_fYawVel * fDeltaTime;
				m_fRoll  += m_fRollVel * fDeltaTime;

                LTRotation rRot;
				m_pClientDE->SetupEuler(&rRot, m_fPitch, m_fYaw, m_fRoll);
				m_pClientDE->SetObjectRotation(m_hGib[nIndex], &rRot);
			}
		}
	}

	if (m_pGibTrail[nIndex])
	{
		m_pGibTrail[nIndex]->Update();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::CreateGib
//
//	PURPOSE:	Create a Gib model
//
// ----------------------------------------------------------------------- //

HLOCALOBJ CGibFX::CreateGib(GibType eType)
{
	// TODO: REIMPLEMENT GIB LOOKUP IN MODELBUTEMGR

    char* pFilename = LTNULL;//GetGibModel(m_eModel, m_eModelStyle, eType);
    char* pSkin     = LTNULL;//GetGibSkin(m_eModel, m_eModelStyle, eType);

    if (!pFilename) return LTNULL;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	SAFE_STRCPY(createStruct.m_Filename, pFilename);
	if (pSkin) SAFE_STRCPY(createStruct.m_SkinName, pSkin);
	createStruct.m_Flags = FLAG_VISIBLE; // | FLAG_NOLIGHT;
	VEC_COPY(createStruct.m_Pos, m_vPos);

	HLOCALOBJ hObj = m_pClientDE->CreateObject(&createStruct);

	if (hObj)
	{
		m_pClientDE->SetModelAnimation(hObj, m_pClientDE->GetAnimIndex(hObj, "DEAD1"));
	}

	return hObj;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::CreateRandomGib
//
//	PURPOSE:	Create a random gib model
//
// ----------------------------------------------------------------------- //

HLOCALOBJ CGibFX::CreateRandomGib()
{
    return LTNULL;
/*
	DebrisType eType = DBT_GENERIC;
	switch (g_pModelButeMgr->GetModelType(m_eModelId))
	{
		case eModelTypeHuman:
			eType = DBT_HUMAN_PARTS;
		break;
		case eModelTypeVehicle:
			eType = DBT_VEHICLE_PARTS;
		break;
		default : break;
	}

    LTVector vScale(1.0f, 1.0f, 1.0f);

	char* pFilename = GetDebrisModel(eType, vScale);
	char* pSkin     = GetDebrisSkin(eType);
    if (!pFilename) return LTNULL;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	SAFE_STRCPY(createStruct.m_Filename, pFilename);
	if (pSkin) SAFE_STRCPY(createStruct.m_SkinName, pSkin);
	createStruct.m_Flags = FLAG_VISIBLE; // | FLAG_NOLIGHT;
	VEC_COPY(createStruct.m_Pos, m_vPos);

	HLOCALOBJ hObj = m_pClientDE->CreateObject(&createStruct);

	if (hObj)
	{
		m_pClientDE->SetObjectScale(hObj, &vScale);
	}

	return hObj;
*/
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::CreateGibTrail
//
//	PURPOSE:	Create a blood/smoke gib trail fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CGibFX::CreateGibTrail(HLOCALOBJ hObj)
{
    if (!hObj || !m_pClientDE) return LTNULL;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
    if (!pSettings) return LTNULL;

    uint8 nDetailLevel = pSettings->SpecialFXSetting();
    if (nDetailLevel == RS_LOW) return LTNULL;


	PTCREATESTRUCT pt;
	pt.hServerObj = hObj;
    pt.nType      = (uint8) (m_eModelType == eModelTypeHuman ? PT_BLOOD : PT_GIBSMOKE);

	CSpecialFX*	pSFX = debug_new(CParticleTrailFX);
    if (!pSFX) return LTNULL;

	pSFX->Init(&pt);
	pSFX->CreateObject(m_pClientDE);

	return pSFX;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::GetBounceSound
//
//	PURPOSE:	Get a gib bounce sound
//
// ----------------------------------------------------------------------- //

char* CGibFX::GetBounceSound()
{
    char* pSound = LTNULL;

/*
	switch (m_eModelType)
	{
		case eModelTypeHuman:
		{
			pSound = GetDebrisBounceSound(DBT_HUMAN_PARTS);
		}
		break;

		case eModelTypeVehicle:
		{
			pSound = GetDebrisBounceSound(DBT_VEHICLE_PARTS);
		}
		break;

		case eModelTypeGenericProp:
		default :
		{
			pSound = GetDebrisBounceSound(DBT_GENERIC);
		}
		break;
	}
*/
	return pSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::GetGibDieSound
//
//	PURPOSE:	Get the sound when a gib dies
//
// ----------------------------------------------------------------------- //

char* CGibFX::GetGibDieSound()
{
    char* pSound = LTNULL;

/*
	switch (m_eModelType)
	{
		case eModelTypeHuman:
		{
			pSound = GetDebrisExplodeSound(DBT_HUMAN_PARTS);
		}
		break;

		case eModelTypeVehicle:
		{
			pSound = GetDebrisExplodeSound(DBT_VEHICLE_PARTS);
		}
		break;

		case eModelTypeGenericProp:
		default :
		{
			pSound = GetDebrisExplodeSound(DBT_GENERIC);
		}
		break;
	}
*/
	return pSound;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::HandleBounce
//
//	PURPOSE:	Handle gib bouncing
//
// ----------------------------------------------------------------------- //

void CGibFX::HandleBounce(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_nNumGibs) return;

	// Play a bounce sound if the gib isn't in liquid...

	if (!(m_Emitters[nIndex].m_dwPhysicsFlags & MO_LIQUID) && (m_hGib[nIndex]))
	{
		if (m_bPlayBounceSound && GetRandom(1, 4) != 1)
		{
			char* pSound = GetBounceSound();

			// Play appropriate sound...

			if (pSound)
			{
				g_pClientSoundMgr->PlaySoundFromPos(m_Emitters[nIndex].m_vPos,
					pSound, 1000.0f, SOUNDPRIORITY_MISC_LOW);
			}
		}
	}


	// See if we're resting...

	m_BounceCount[nIndex]--;
	if (m_BounceCount[nIndex] <= 0)
	{
		m_Emitters[nIndex].m_dwPhysicsFlags |= MO_RESTING;
		if (m_bSubGibs) HandleDoneBouncing(nIndex);
	}


	// Add a blood splat...

	if (m_bBloodSplats)
	{
		// Don't add blood splats on the sky...

        uint32 dwTextureFlags;
		m_pClientDE->GetPolyTextureFlags(m_info.m_hPoly, &dwTextureFlags);
		SurfaceType eType = (SurfaceType)dwTextureFlags;
		if (eType == ST_SKY) return;


		CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
		if (!psfxMgr) return;

		BSCREATESTRUCT sc;

        m_pClientDE->AlignRotation(&(sc.rRot), &(m_info.m_Plane.m_Normal), LTNULL    );

        LTVector vTemp;
		VEC_MULSCALAR(vTemp, m_info.m_Plane.m_Normal, 2.0f);
		VEC_ADD(sc.vPos, m_info.m_Point, vTemp);  // Off the wall/floor a bit
		VEC_SET(sc.vVel, 0.0f, 0.0f, 0.0f);
		VEC_SET(sc.vInitialScale, GetRandom(0.3f, 0.5f), GetRandom(0.3f, 0.5f), 1.0f);
		VEC_SET(sc.vFinalScale, GetRandom(0.8f, 1.0f), GetRandom(0.8f, 1.0f), 1.0f);

		sc.dwFlags			= FLAG_VISIBLE | FLAG_ROTATEABLESPRITE | FLAG_NOLIGHT;
		sc.fLifeTime		= m_fLifeTime + 10.0f;
		sc.fInitialAlpha	= 1.0f;
		sc.fFinalAlpha		= 0.0f;
		sc.nType			= OT_SPRITE;

		char* pBloodFiles[] =
		{
			"Sprites\\BloodSplat1.spr",
			"Sprites\\BloodSplat2.spr",
			"Sprites\\BloodSplat3.spr",
			"Sprites\\BloodSplat4.spr"
		};

		sc.pFilename = pBloodFiles[GetRandom(0,3)];


		CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SCALE_ID, &sc);
		if (pFX) pFX->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::HandleDoneBouncing
//
//	PURPOSE:	Handle gib done bouncing
//
// ----------------------------------------------------------------------- //

void CGibFX::HandleDoneBouncing(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_nNumGibs) return;

	switch ( g_pModelButeMgr->GetModelType(m_eModelId) )
	{
		case eModelTypeVehicle:
			CreateLingeringSmoke(nIndex);
		break;

		case eModelTypeHuman:
			CreateMiniBloodExplosion(nIndex);
		break;

		default : break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::CreateLingeringSmoke
//
//	PURPOSE:	Create a bit o smoke...
//
// ----------------------------------------------------------------------- //

void CGibFX::CreateLingeringSmoke(int nIndex)
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return;

    uint8 nDetailLevel = pSettings->SpecialFXSetting();
	if (nDetailLevel == RS_LOW) return;

	SMCREATESTRUCT sm;

	char* pTexture = "Sprites\\SmokeTest.spr";

	VEC_SET(sm.vColor1, 100.0f, 100.0f, 100.0f);
	VEC_SET(sm.vColor2, 125.0f, 125.0f, 125.0f);
	VEC_SET(sm.vMinDriftVel, -10.0f, 25.0f, -10.0f);
	VEC_SET(sm.vMaxDriftVel, 10.0f, 50.0f, 10.0f);

    LTFLOAT fVolumeRadius        = 10.0f;
    LTFLOAT fLifeTime            = GetRandom(m_fLifeTime * 0.75f, m_fLifeTime);
    LTFLOAT fRadius              = 1500;
    LTFLOAT fParticleCreateDelta = 0.1f;
    LTFLOAT fMinParticleLife     = 1.0f;
    LTFLOAT fMaxParticleLife     = 5.0f;
    uint8  nNumParticles        = 3;
    LTBOOL  bIgnoreWind          = LTFALSE;

	if (IsLiquid(m_eCode))
	{
		GetLiquidColorRange(m_eCode, &sm.vColor1, &sm.vColor2);
		pTexture			= DEFAULT_BUBBLE_TEXTURE;
		fRadius				= 750.0f;
        bIgnoreWind         = LTTRUE;
		fMinParticleLife	= 1.0f;
		fMaxParticleLife	= 1.5f;
	}

	sm.vPos					= m_Emitters[nIndex].m_vPos;
	sm.hServerObj 		    = m_hGib[nIndex];
	sm.fVolumeRadius		= fVolumeRadius;
	sm.fLifeTime			= fLifeTime;
	sm.fRadius				= fRadius;
	sm.fParticleCreateDelta	= fParticleCreateDelta;
	sm.fMinParticleLife		= fMinParticleLife;
	sm.fMaxParticleLife		= fMaxParticleLife;
	sm.nNumParticles		= nNumParticles;
	sm.bIgnoreWind			= bIgnoreWind;
	sm.hstrTexture			= m_pClientDE->CreateString(pTexture);

	psfxMgr->CreateSFX(SFX_SMOKE_ID, &sm);

	m_pClientDE->FreeString(sm.hstrTexture);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::CreateMiniBloodExplosion
//
//	PURPOSE:	Crate a mini blood explosion effect
//
// ----------------------------------------------------------------------- //

void CGibFX::CreateMiniBloodExplosion(int nIndex)
{
	// Add a mini blood explosion...

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return;

    uint8 nDetailLevel = pSettings->SpecialFXSetting();
	if (nDetailLevel == RS_LOW) return;

	char* szBlood[2] = { "SpecialFX\\ParticleTextures\\Blood_1.dtx",
					     "SpecialFX\\ParticleTextures\\Blood_2.dtx" };

	PARTICLESHOWERCREATESTRUCT ps;

	ps.vPos = m_Emitters[nIndex].m_vPos;
	ps.vPos.y += 30.0f;

	ps.vDir.Init(0, 100, 0);
	VEC_SET(ps.vColor1, 200.0f, 200.0f, 200.0f);
	VEC_SET(ps.vColor2, 255.0f, 255.0f, 255.0f);
	ps.pTexture			= szBlood[GetRandom(0,1)];;
	ps.nParticles		= 50;
	ps.fDuration		= 1.0f;
	ps.fEmissionRadius	= 0.3f;
	ps.fRadius			= 800.0f;
	ps.fGravity			= PSFX_DEFAULT_GRAVITY;

	if (IsLiquid(m_eCode))
	{
		ps.vDir	*= 3.0f;
		ps.fEmissionRadius	= 0.2f;
		ps.fRadius			= 700.0f;
	}

	psfxMgr->CreateSFX(SFX_PARTICLESHOWER_ID, &ps);

	// Play appropriate sound...

	char* pSound = GetGibDieSound();

	if (pSound)
	{
		g_pClientSoundMgr->PlaySoundFromPos(ps.vPos, pSound, 300.0f,
			SOUNDPRIORITY_MISC_LOW);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::CreateBloodSpray
//
//	PURPOSE:	Create a spray of blood
//
// ----------------------------------------------------------------------- //

void CGibFX::CreateBloodSpray()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	BSCREATESTRUCT sc;

	VEC_COPY(sc.vPos, m_vPos);
	sc.vPos.y += 30.0f;
	VEC_SET(sc.vVel, 0.0f, -20.0f, 0.0f);
	VEC_SET(sc.vInitialScale, GetRandom(2.0f, 4.0f), GetRandom(2.0f, 4.0f), 1.0f);
	VEC_SET(sc.vFinalScale, GetRandom(0.5f, 0.8f), GetRandom(0.5f, 0.8f), 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT;
	sc.fLifeTime		= 0.5f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;
	sc.nType			= OT_SPRITE;

	char* pBloodFiles[] =
	{
		"Sprites\\BloodSplat1.spr",
		"Sprites\\BloodSplat2.spr",
		"Sprites\\BloodSplat3.spr",
		"Sprites\\BloodSplat4.spr"
	};

	sc.pFilename = pBloodFiles[GetRandom(0,3)];


	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SCALE_ID, &sc);
	if (pFX) pFX->Update();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::RemoveAllFX
//
//	PURPOSE:	Remove all the fx
//
// ----------------------------------------------------------------------- //

void CGibFX::RemoveAllFX()
{
	if (!m_pClientDE) return;

	for (int i=0; i < m_nNumGibs; i++)
	{
		if (m_hGib[i])
		{
			m_pClientDE->DeleteObject(m_hGib[i]);
            m_hGib[i] = LTNULL;
		}
		if (m_pGibTrail[i])
		{
			debug_delete(m_pGibTrail[i]);
            m_pGibTrail[i] = LTNULL;
		}
	}
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::OkToRemoveGib
//
//	PURPOSE:	See if this particular model can be removed.
//
// ----------------------------------------------------------------------- //

LTBOOL CGibFX::OkToRemoveGib(HLOCALOBJ hGib)
{
    if (!m_pClientDE || !g_pGameClientShell || !hGib) return LTTRUE;


	// The only constraint is that the client isn't currently looking
	// at the model...

	HLOCALOBJ hCamera = g_pGameClientShell->GetCamera();
    if (!hCamera) return LTTRUE;

    LTVector vPos, vCamPos;
	m_pClientDE->GetObjectPos(hGib, &vPos);
	m_pClientDE->GetObjectPos(hCamera, &vCamPos);


	// Determine if the client can see us...

    LTVector vDir;
	VEC_SUB(vDir, vPos, vCamPos);

    LTRotation rRot;
    LTVector vU, vR, vF;
	m_pClientDE->GetObjectRotation(hCamera, &rRot);
	m_pClientDE->GetRotationVectors(&rRot, &vU, &vR, &vF);

	VEC_NORM(vDir);
	VEC_NORM(vF);
    LTFLOAT fMul = VEC_DOT(vDir, vF);
    if (fMul <= 0.0f) return LTTRUE;


	// Client is looking our way, don't remove it yet...

    return LTFALSE;
}