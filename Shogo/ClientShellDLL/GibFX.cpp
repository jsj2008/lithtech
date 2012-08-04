// ----------------------------------------------------------------------- //
//
// MODULE  : GibFX.cpp
//
// PURPOSE : Gib - Implementation
//
// CREATED : 6/15/98
//
// ----------------------------------------------------------------------- //

#include "GibFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "SurfaceTypes.h"
#include "RiotClientShell.h"
#include "ParticleSystemFX.h"
#include "SFXMsgIds.h"
#include "ParticleTrailFX.h"
#include "SpriteFX.h"
#include "ParticleExplosionFX.h"
#include "SparksFX.h"
#include "ExplosionFX.h"
#include "DebrisTypes.h"
#include "SmokeFX.h"
#include "ltobjectcreate.h"

extern CRiotClientShell* g_pRiotClientShell;

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
	m_nModelId			= pGib->nModelId;
	m_eCode				= (ContainerCode)pGib->nCode;
	m_eSize				= (ModelSize)pGib->nSize;
	m_eCharacterClass	= (CharacterClass)pGib->nCharacterClass;
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

	m_eModelType = GetModelType(m_nModelId, m_eSize);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSparksFX::CreateObject
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

	m_pClientDE->Common()->GetRotationVectors(m_rRot, vU, vR, vF);

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


	// Initialize our emmitters...

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

		m_ActiveEmmitters[i] = LTTRUE;
		m_BounceCount[i]	 = GetRandom(2, 5);

		VEC_SET(vVel, GetRandom(vVelMin.x, vVelMax.x), 
					  50.0f + GetRandom(vVelMin.y, vVelMax.y), 
					  GetRandom(vVelMin.z, vVelMax.z));

		InitMovingObject(&(m_Emmitters[i]), &m_vPos, &vVel);
		m_Emmitters[i].m_PhysicsFlags |= m_nGibFlags;
	}

	
	// Create a big burst of blood...

	if (m_eModelType == MT_HUMAN)
	{
		//CreateBloodSpray();
	}


	// Play die sound...

	char* pSound = GetGibDieSound();
	if (pSound)
	{
		PlaySoundFromPos(&m_vPos, pSound, 1000.0f, SOUNDPRIORITY_MISC_LOW);
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
		for (int i=0; i < m_nNumGibs; i++)
		{
			LTFLOAT fEndTime = m_fStartTime + m_fGibLife[i];

			if (fTime > fEndTime)
			{
				if (OkToRemoveGib(m_hGib[i]))
				{
					if (m_hGib[i])
					{
						m_pClientDE->RemoveObject(m_hGib[i]);
						m_hGib[i] = LTNULL;
					}
				}
			}
		}

		// See if all the gibs have been removed or not...

		int i;
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


	// Loop over our list of emmitters, updating the position of each

	for (int i=0; i < m_nNumGibs; i++)
	{
		if (m_ActiveEmmitters[i])
		{
			LTBOOL bBounced = LTFALSE;
			if (bBounced = UpdateEmmitter(&m_Emmitters[i]))
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
//	ROUTINE:	CGibFX::UpdateEmmitter
//
//	PURPOSE:	Update emmitter position
//
// ----------------------------------------------------------------------- //

LTBOOL CGibFX::UpdateEmmitter(MovingObject* pObject)
{	
	if (!m_pClientDE || !pObject || pObject->m_PhysicsFlags & MO_RESTING) return LTFALSE;

	LTBOOL bRet = LTFALSE;

	LTVector vNewPos;
	if (UpdateMovingObject(LTNULL, pObject, &vNewPos))
	{
		bRet = BounceMovingObject(LTNULL, pObject, &vNewPos, &m_info);

		VEC_COPY(pObject->m_LastPos, pObject->m_Pos);
		VEC_COPY(pObject->m_Pos, vNewPos);

		if (m_pClientDE->Common()->GetPointStatus(&vNewPos) == LT_OUTSIDE)
		{
			pObject->m_PhysicsFlags |= MO_RESTING;
			VEC_COPY(pObject->m_Pos, pObject->m_LastPos);
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

	if (m_Emmitters[nIndex].m_PhysicsFlags & MO_RESTING)
	{
		m_ActiveEmmitters[nIndex] = LTFALSE;
		if (m_pGibTrail[nIndex])
		{
			delete m_pGibTrail[nIndex];
			m_pGibTrail[nIndex] = LTNULL;
		}

		if (m_hGib[nIndex])
		{
			if (m_bRotate)
			{
				LTRotation rRot;
				m_pClientDE->Common()->SetupEuler(rRot, 0.0f, m_fYaw, m_fRoll);
				m_pClientDE->SetObjectRotation(m_hGib[nIndex], &rRot);	
			}

			// m_pClientDE->SetObjectPos(m_hGib[nIndex], &(m_info.m_Point));
		}
	}
	else if (m_hGib[nIndex])
	{
		m_pClientDE->SetObjectPos(m_hGib[nIndex], &(m_Emmitters[nIndex].m_Pos));

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
				LTFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

				m_fPitch += m_fPitchVel * fDeltaTime;
				m_fYaw   += m_fYawVel * fDeltaTime;
				m_fRoll  += m_fRollVel * fDeltaTime;

				LTRotation rRot;
				m_pClientDE->Common()->SetupEuler(rRot, m_fPitch, m_fYaw, m_fRoll);
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
	char* pFilename = GetGibModel(m_nModelId, eType, m_eSize);
	char* pSkin		= GetSkin(m_nModelId, m_eCharacterClass, m_eSize);

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
		LTVector vScale = GetGibModelScale(m_nModelId, m_eSize);
		m_pClientDE->SetObjectScale(hObj, &vScale);

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
	DebrisType eType = DBT_GENERIC;
	switch(GetModelType(m_nModelId, m_eSize))
	{
		case MT_MECHA:
			eType = DBT_MECHA_PARTS;
		break;
		case MT_HUMAN:
			eType = DBT_HUMAN_PARTS;
		break;
		case MT_VEHICLE:
			eType = DBT_VEHICLE_PARTS;
		break;
		default : break;
	}

	LTVector vScale;
	VEC_SET(vScale, 2.0f, 2.0f, 2.0f);

	LTFLOAT fSize = (m_eSize == MS_SMALL ? 0.2f : (m_eSize == MS_LARGE ? 5.0f : 1.0f));
	VEC_MULSCALAR(vScale, vScale, fSize);

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

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return LTNULL;

	uint8 nDetailLevel = pSettings->SpecialFXSetting();
	if (nDetailLevel == RS_LOW) return LTNULL;


	PTCREATESTRUCT pt;
	pt.hServerObj = hObj;
	pt.nType      = (uint8)(m_eModelType == MT_HUMAN ? PT_BLOOD : PT_GIBSMOKE);
	pt.bSmall     = LTFALSE;

	CSpecialFX*	pSFX = new CParticleTrailFX();
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

	switch (m_eModelType)
	{
		case MT_HUMAN:
		{
			pSound = GetDebrisBounceSound(DBT_HUMAN_PARTS);
		}
		break;

		case MT_VEHICLE:
		{
			pSound = GetDebrisBounceSound(DBT_VEHICLE_PARTS);
		}
		break;

		case MT_MECHA:
		{
			pSound = GetDebrisBounceSound(DBT_MECHA_PARTS);
		}
		break;
		
		case MT_PROP_GENERIC:
		default :
		{
			pSound = GetDebrisBounceSound(DBT_GENERIC);
		}
		break;
	}

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

	switch (m_eModelType)
	{
		case MT_HUMAN:
		{
			pSound = GetDebrisExplodeSound(DBT_HUMAN_PARTS);
		}
		break;

		case MT_VEHICLE:
		{
			pSound = GetDebrisExplodeSound(DBT_VEHICLE_PARTS);
		}
		break;

		case MT_MECHA:
		{
			pSound = GetDebrisExplodeSound(DBT_MECHA_PARTS);
		}
		break;
		
		case MT_PROP_GENERIC:
		default :
		{
			pSound = GetDebrisExplodeSound(DBT_GENERIC);
		}
		break;
	}

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

	if (!(m_Emmitters[nIndex].m_PhysicsFlags & MO_LIQUID) && (m_hGib[nIndex]))
	{
		if (m_bPlayBounceSound && GetRandom(1, 4) != 1)
		{
			char* pSound = GetBounceSound();
		
			// Play appropriate sound...
		
			if (pSound)
			{
				PlaySoundFromPos(&m_Emmitters[nIndex].m_Pos, pSound, 1000.0f,
								 SOUNDPRIORITY_MISC_LOW);
			}
		}
	}


	// See if we're resting...

	m_BounceCount[nIndex]--;
	if (m_BounceCount[nIndex] <= 0)
	{
		m_Emmitters[nIndex].m_PhysicsFlags |= MO_RESTING;
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


		CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
		if (!psfxMgr) return;

		SPRITECREATESTRUCT sc;

		m_pClientDE->Math()->AlignRotation(sc.rRot, m_info.m_Plane.m_Normal, LTVector(0, 1, 0));

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

		char* pBloodFiles[] = 
		{
			"Sprites\\BloodSplat1.spr",
			"Sprites\\BloodSplat2.spr",
			"Sprites\\BloodSplat3.spr",
			"Sprites\\BloodSplat4.spr"
		};

		sc.pFilename = pBloodFiles[GetRandom(0,3)];
		

		CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
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

	switch (GetModelType(m_nModelId, m_eSize))
	{
		case MT_MECHA:
		case MT_VEHICLE:
			CreateLingeringSmoke(nIndex);
		break;

		case MT_HUMAN:
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
	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return;

	uint8 nDetailLevel = pSettings->SpecialFXSetting();
	if (nDetailLevel == RS_LOW) return;

	SMCREATESTRUCT sm;

	char* pTexture = "Sprites\\SmokeTest.spr";

	VEC_SET(sm.vColor1, 100.0f, 100.0f, 100.0f);
	VEC_SET(sm.vColor2, 125.0f, 125.0f, 125.0f);
	VEC_SET(sm.vMinDriftVel, -10.0f, 25.0f, -10.0f);
	VEC_SET(sm.vMaxDriftVel, 10.0f, 50.0f, 10.0f);

	LTFLOAT fVolumeRadius		= 10.0f;
	LTFLOAT fLifeTime			= GetRandom(m_fLifeTime * 0.75f, m_fLifeTime);
	LTFLOAT fRadius				= 1500;
	LTFLOAT fParticleCreateDelta	= 0.1f;
	LTFLOAT fMinParticleLife		= 1.0f;
	LTFLOAT fMaxParticleLife		= 5.0f;
	uint8  nNumParticles		= 3;
	LTBOOL  bIgnoreWind			= LTFALSE;

	if (IsLiquid(m_eCode))
	{
		GetLiquidColorRange(m_eCode, &sm.vColor1, &sm.vColor2);
		pTexture			= "SpecialFX\\ParticleTextures\\GreySphere_1.dtx";
		fRadius				= 750.0f;
		bIgnoreWind			= LTTRUE;
		fMinParticleLife	= 1.0f;
		fMaxParticleLife	= 1.5f;
	}

	VEC_COPY(sm.vPos, m_Emmitters[nIndex].m_Pos);
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

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	CRiotSettings* pSettings = g_pRiotClientShell->GetSettings();
	if (!pSettings) return;

	uint8 nDetailLevel = pSettings->SpecialFXSetting();
	if (nDetailLevel == RS_LOW) return;

	char* szBlood[2] = { "SpecialFX\\ParticleTextures\\Blood_1.dtx", 
					     "SpecialFX\\ParticleTextures\\Blood_2.dtx" };

	char* pTexture = szBlood[GetRandom(0,1)];

	LTVector vDir;
	VEC_SET(vDir, 0.0f, 1.0f, 0.0f);
	VEC_MULSCALAR(vDir, vDir, 100.0f);

	SCREATESTRUCT sp;

	VEC_COPY(sp.vPos, m_Emmitters[nIndex].m_Pos);
	sp.vPos.y += 30.0f;

	VEC_COPY(sp.vDir, vDir);
	VEC_SET(sp.vColor1, 200.0f, 200.0f, 200.0f);
	VEC_SET(sp.vColor2, 255.0f, 255.0f, 255.0f);
	sp.hstrTexture		= m_pClientDE->CreateString(pTexture);
	sp.nSparks			= 50;
	sp.fDuration		= 1.0f;
	sp.fEmissionRadius	= 0.3f;
	sp.fRadius			= 800.0f;
	sp.fGravity			= PSFX_DEFAULT_GRAVITY;

	if (IsLiquid(m_eCode))
	{
		VEC_MULSCALAR(sp.vDir, sp.vDir, 3.0f);
		sp.fEmissionRadius	= 0.2f;
		sp.fRadius			= 700.0f;
	}

	psfxMgr->CreateSFX(SFX_SPARKS_ID, &sp);

	m_pClientDE->FreeString(sp.hstrTexture);

	// Play appropriate sound...

	char* pSound = GetGibDieSound();

	if (pSound)
	{
		PlaySoundFromPos(&sp.vPos, pSound, 300.0f, SOUNDPRIORITY_MISC_LOW);
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
	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	SPRITECREATESTRUCT sc;

	VEC_COPY(sc.vPos, m_vPos);
	sc.vPos.y += 30.0f;
	VEC_SET(sc.vVel, 0.0f, -20.0f, 0.0f);
	VEC_SET(sc.vInitialScale, GetRandom(2.0f, 4.0f), GetRandom(2.0f, 4.0f), 1.0f);
	VEC_SET(sc.vFinalScale, GetRandom(0.5f, 0.8f), GetRandom(0.5f, 0.8f), 1.0f);

	sc.dwFlags			= FLAG_VISIBLE | FLAG_SPRITEBIAS | FLAG_NOLIGHT; 
	sc.fLifeTime		= 0.5f;
	sc.fInitialAlpha	= 1.0f;
	sc.fFinalAlpha		= 0.0f;

	char* pBloodFiles[] = 
	{
		"Sprites\\BloodSplat1.spr",
		"Sprites\\BloodSplat2.spr",
		"Sprites\\BloodSplat3.spr",
		"Sprites\\BloodSplat4.spr"
	};

	sc.pFilename = pBloodFiles[GetRandom(0,3)];
	

	CSpecialFX* pFX = psfxMgr->CreateSFX(SFX_SPRITE_ID, &sc);
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
			m_pClientDE->RemoveObject(m_hGib[i]);
			m_hGib[i] = LTNULL;
		}
		if (m_pGibTrail[i])
		{
			delete m_pGibTrail[i];
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
	if (!m_pClientDE || !g_pRiotClientShell || !hGib) return LTTRUE;


	// The only constraint is that the client isn't currently looking
	// at the model...

	HLOCALOBJ hCamera = g_pRiotClientShell->GetCamera();
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
	m_pClientDE->Common()->GetRotationVectors(rRot, vU, vR, vF);

	VEC_NORM(vDir);
	VEC_NORM(vF);
	LTFLOAT fMul = VEC_DOT(vDir, vF);
	if (fMul <= 0.0f) return LTTRUE;


	// Client is looking our way, don't remove it yet...

	return LTFALSE;
}