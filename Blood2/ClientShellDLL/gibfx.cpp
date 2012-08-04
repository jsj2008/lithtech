
// ----------------------------------------------------------------------- //
//
// MODULE  : GibFX.cpp
//
// PURPOSE : Gib special FX - Implementation
//
// CREATED : 8/3/98
//
// ----------------------------------------------------------------------- //

#include "gibfx.h"
#include "cpp_client_de.h"
#include "dlink.h"
#include "BloodClientShell.h"
#include "ClientUtilities.h"
#include "bloodsplatfx.h"
#include "ParticleExplosionFX.h"
#include "SoundTypes.h"

#define GIBS_FLESH		8
#define GIBS_WOOD		4
#define GIBS_GLASS		3
#define GIBS_METAL		5
#define GIBS_PLASTIC	5
#define GIBS_STONE		9
#define GIBS_TERRAIN	5

#define SOUNDS_FLESH	5
#define SOUNDS_WOOD		5
#define SOUNDS_GLASS	5
#define SOUNDS_METAL	5
#define SOUNDS_PLASTIC	5
#define SOUNDS_STONE	5
#define SOUNDS_TERRAIN	5




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::Init
//
//	PURPOSE:	Create the gib
//
// ----------------------------------------------------------------------- //

DBOOL CGibFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct) return DFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	GIBCREATESTRUCT* pGib = (GIBCREATESTRUCT*)psfxCreateStruct;

	VEC_COPY( m_vPos, pGib->m_Pos);
	VEC_COPY( m_vDir, pGib->m_Dir);
	VEC_COPY( m_vDims, pGib->m_Dims);
	m_dwFlags = pGib->m_dwFlags;
	m_fScale = pGib->m_fScale;

	m_nBounceCount = 1;

	DFLOAT fVel = VEC_MAG(m_vDir);

	VEC_MULSCALAR(m_vDims, m_vDims, 0.8f);
	if (m_vDims.x)
		m_vPos.x += GetRandom(-m_vDims.x, m_vDims.x);
	if (m_vDims.z)
		m_vPos.z += GetRandom(-m_vDims.z, m_vDims.z);

	m_nType = (m_dwFlags & TYPE_MASK) * 10;

	VEC_NORM(m_vDir);

	// Don't blow glass straight up quite so much
	if (m_nType == SURFTYPE_GLASS)
	{
		VEC_SET(m_vMinVel, -fVel, 0, -fVel)
		VEC_SET(m_vMaxVel, fVel, fVel*2, fVel)

		if (m_vDims.y)
			m_vPos.y += GetRandom(-m_vDims.y, m_vDims.y);
	}
	else
	{
		VEC_SET(m_vMinVel, -fVel, 0, -fVel)
		VEC_SET(m_vMaxVel, fVel, fVel*4, fVel)

		if (m_vDims.y)
			m_vPos.y += GetRandom(-m_vDims.y/2, m_vDims.y);
	}


	m_fPitchVel = GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
	m_fYawVel	= GetRandom(-MATH_CIRCLE, MATH_CIRCLE);

	if (m_dwFlags & TYPEFLAG_CUSTOM)
	{
		m_hstrModel = pGib->m_hstrModel;
		m_hstrSkin  = pGib->m_hstrSkin;
		m_hstrSound = pGib->m_hstrSound;
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::Term
//
//	PURPOSE:	Term
//
// ----------------------------------------------------------------------- //

DBOOL CGibFX::Term()
{
	m_bFade = DTRUE;

	return DFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::CreateObject
//
//	PURPOSE:	Create object associated with the mark
//
// ----------------------------------------------------------------------- //

DBOOL CGibFX::CreateObject(CClientDE *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return DFALSE;

	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell) return DFALSE;
	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return DFALSE;

	m_pClientDE->AlignRotation(&m_Rotation, &m_vDir, DNULL);

	// Determine the filenames to use
	int index;
	DBOOL bRandomizeScale = DFALSE;

	moveObj.m_PhysicsFlags = MO_HALFGRAVITY;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);
	createStruct.m_ObjectType = OT_MODEL;

	char szSound[MAX_CS_FILENAME_LEN+1];
	char szFilename[MAX_CS_FILENAME_LEN+1];
	_mbscpy((unsigned char*)szFilename, (const unsigned char*)"");

	// Determine the file to use
	if (!(m_dwFlags & TYPEFLAG_CUSTOM))
	{
		switch (m_nType)
		{
			case SURFTYPE_FLESH:
				{
					index = GetRandom(1, NRES(GIBS_FLESH));
					sprintf(szFilename, "flesh\\gib%d", index);
					bRandomizeScale = DTRUE;
					m_nBounceCount = (GetRandom(0, 4) == 4) ? 1 : 0;	// Stick the wall 20% of the time
					moveObj.m_PhysicsFlags |= MO_STICKY;

					index = GetRandom(1, NRES(SOUNDS_FLESH));
					sprintf(szSound, "sounds\\gibs\\flesh\\gib_impact%d.wav", index);
				}
				break;

			case SURFTYPE_GLASS:
				{
					index = GetRandom(1, NRES(GIBS_GLASS));
					sprintf(szFilename, "glass\\gib%d", index);
					bRandomizeScale = DTRUE;
					m_nBounceCount = 1;

					index = GetRandom(1, NRES(SOUNDS_GLASS));
					sprintf(szSound, "sounds\\gibs\\glass\\gib_impact%d.wav", index);
				}
				break;

			case SURFTYPE_WOOD:
				{
					index = GetRandom(1, NRES(GIBS_WOOD));
					sprintf(szFilename, "wood\\gib%d", index);
					bRandomizeScale = DFALSE;
					m_nBounceCount = 1;

					index = GetRandom(1, NRES(SOUNDS_WOOD));
					sprintf(szSound, "sounds\\gibs\\wood\\gib_impact%d.wav", index);
				}
				break;

			case SURFTYPE_STONE:
				{
					index = GetRandom(1, NRES(GIBS_STONE));
					sprintf(szFilename, "stone\\gib%d", index);
					bRandomizeScale = DTRUE;
					m_nBounceCount = 2;

					index = GetRandom(1, NRES(SOUNDS_STONE));
					sprintf(szSound, "sounds\\gibs\\stone\\gib_impact%d.wav", index);
				}
				break;

			case SURFTYPE_PLASTIC:
				{
					index = GetRandom(1, NRES(GIBS_PLASTIC));
					sprintf(szFilename, "plastic\\gib%d", index);
					bRandomizeScale = DTRUE;
					m_nBounceCount = 2;

					index = GetRandom(1, NRES(SOUNDS_PLASTIC));
					sprintf(szSound, "sounds\\gibs\\plastic\\gib_impact%d.wav", index);
				}
				break;

			case SURFTYPE_TERRAIN:
				{
					index = GetRandom(1, NRES(GIBS_TERRAIN));
					sprintf(szFilename, "terrain\\gib%d", index);
					bRandomizeScale = DTRUE;
					m_nBounceCount = 2;

					index = GetRandom(1, NRES(SOUNDS_TERRAIN));
					sprintf(szSound, "sounds\\gibs\\terrain\\gib_impact%d.wav", index);
				}
				break;

			case SURFTYPE_METAL:
			default:
				{
					index = GetRandom(1, NRES(GIBS_METAL));
					sprintf(szFilename, "metal\\gib%d", index);
					bRandomizeScale = DFALSE;
					m_nBounceCount = 1;

					index = GetRandom(1, NRES(SOUNDS_METAL));
					sprintf(szSound, "sounds\\gibs\\metal\\gib_impact%d.wav", index);
				}
				break;
		}

		if (!szFilename)
			return DFALSE;

		if (bRandomizeScale)
		{
			if (m_nType == SURFTYPE_FLESH)
			{
				VEC_SET(createStruct.m_Scale, GetRandom(0.8f, 1.35f), GetRandom(0.8f, 1.35f), GetRandom(0.8f, 1.35f));
			}
			else
			{
				VEC_SET(createStruct.m_Scale, GetRandom(0.5f, 1.25f), GetRandom(0.5f, 1.25f), GetRandom(0.5f, 1.25f));
			}
		}

		VEC_MULSCALAR(createStruct.m_Scale, createStruct.m_Scale, m_fScale);

		if (_mbstrlen(szSound) && !m_hstrSound)
		{
			m_hstrSound = m_pClientDE->CreateString(szSound);
		}
	}

	// Custom or normal?
	if (m_hstrModel)
		_mbscpy((unsigned char*)createStruct.m_Filename, (const unsigned char*)m_pClientDE->GetStringData(m_hstrModel));
	else
		sprintf(createStruct.m_Filename, "models\\gibs\\%s.abc", szFilename);		
	
	if (m_hstrSkin)
		_mbscpy((unsigned char*)createStruct.m_SkinName, (const unsigned char*)m_pClientDE->GetStringData(m_hstrSkin));
	else
		sprintf(createStruct.m_SkinName, "skins\\gibs\\%s.dtx", szFilename);

	createStruct.m_Flags	  = FLAG_VISIBLE;
	VEC_COPY(createStruct.m_Pos, m_vPos);

	m_hObject = pClientDE->CreateObject(&createStruct);

	// Make glass transparent
	if (m_nType == SURFTYPE_GLASS)
		m_pClientDE->SetObjectColor(m_hObject, 0.0f, 0.0f, 0.0f, 0.4f);
	
	if(m_dwFlags & TRAIL_BLOOD)
	{
		m_bBlood = DTRUE;
	}
	else if(m_dwFlags & TRAIL_SMOKE)
	{
		m_bSmoke = DTRUE;
	}

	//initialize velocity
	DVector vVelMin, vVelMax, vTemp, vU, vR, vF;
	VEC_SET(vVelMin, 1.0f, 1.0f, 1.0f);
	VEC_SET(vVelMax, 1.0f, 1.0f, 1.0f);

	m_pClientDE->GetRotationVectors(&m_Rotation, &vU, &vR, &vF);

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

	DVector vStartVel;
	VEC_SET(vStartVel, GetRandom(vVelMin.x, vVelMax.x), 
					   GetRandom(vVelMin.y, vVelMax.y), 
					   GetRandom(vVelMin.z, vVelMax.z));

	InitMovingObject(&moveObj, &m_vPos, &vStartVel);

	if (m_bBlood)
		CreateBloodSpurt(&vStartVel);

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::Update
//
//	PURPOSE:	Update the mark
//
// ----------------------------------------------------------------------- //

DBOOL CGibFX::Update()
{
	if(!m_hObject || !m_pClientDE) return DFALSE;

	//fade the object out of existence
	if(m_bFade)
	{
		if(m_fScaleCount >= 150.0f)
			return DFALSE;

		DFLOAT fAlpha = 0.0f;
		DVector vColor;
		
		m_pClientDE->GetObjectColor(m_hObject,&vColor.x, &vColor.y, &vColor.z, &fAlpha);
		m_pClientDE->SetObjectColor(m_hObject,vColor.x,vColor.y,vColor.z,1.0f - (m_fScaleCount/150.0f));
		m_fScaleCount++;

		return DTRUE;
	}

	DBOOL bBounced = DFALSE;

	ClientIntersectInfo info;

	if(moveObj.m_PhysicsFlags & MO_RESTING)
	{
		return DTRUE;
	}
	else if(bBounced = UpdateMovement(&moveObj, &info))
	{

		PlaySoundFromPos(&moveObj.m_Pos, m_pClientDE->GetStringData(m_hstrSound), 750.0f,SOUNDPRIORITY_MISC_LOW);

		if (m_bBlood)
			CreateBloodSplat(&moveObj.m_Pos, &info);

		// Don't count bounces off the ceiling
		if (info.m_Plane.m_Normal.y > 0)
		{
			if (m_nBounceCount <= 0)
			{
				moveObj.m_PhysicsFlags |= MO_RESTING;
			}
			m_nBounceCount--;
		}
		else
		{
			moveObj.m_PhysicsFlags &= ~MO_RESTING;
		}

		m_pClientDE->SetObjectPos(m_hObject, &(moveObj.m_Pos));

		// Adjust due to the bounce...

		m_fPitchVel = GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
		m_fYawVel	= GetRandom(-MATH_CIRCLE, MATH_CIRCLE);
	}
	else
	{
		//rotate the object
		if (m_fPitchVel != 0 || m_fYawVel != 0)
		{
			DFLOAT fDeltaTime = m_pClientDE->GetFrameTime();

			m_fPitch += m_fPitchVel * fDeltaTime;
			m_fYaw   += m_fYawVel * fDeltaTime;

			DRotation rRot;
			m_pClientDE->SetupEuler(&rRot, m_fPitch, m_fYaw, 0.0f);
			m_pClientDE->SetObjectRotation(m_hObject, &rRot);	
		}

		m_pClientDE->SetObjectPos(m_hObject, &(moveObj.m_Pos));

		if(m_bBlood)
		{
		}
	}

	return DTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::UpdateMovement
//
//	PURPOSE:	Update position
//
// ----------------------------------------------------------------------- //

DBOOL CGibFX::UpdateMovement(MovingObject* pObject, ClientIntersectInfo* pInfo)
{	
	if (!m_pClientDE || !pObject || pObject->m_PhysicsFlags & MO_RESTING) return DFALSE;

	DBOOL bRet = DFALSE;

	DVector vNewPos;

	if (UpdateMovingObject(DNULL, pObject, &vNewPos))
	{
//		SurfaceType eType = SURFTYPE_UNKNOWN;
//		bRet = BounceMovingObject(DNULL, pObject, &vNewPos, pInfo, &eType);
		bRet = BounceMovingObject(DNULL, pObject, &vNewPos, pInfo);

		VEC_COPY(pObject->m_LastPos, pObject->m_Pos);
		VEC_COPY(pObject->m_Pos, vNewPos);

		if (m_pClientDE->GetPointStatus(&vNewPos) != DE_INSIDE)
		{
			pObject->m_PhysicsFlags |= MO_RESTING;
		}
	}

	return bRet;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::UpdateMovement
//
//	PURPOSE:	Update position
//
// ----------------------------------------------------------------------- //

DBOOL CGibFX::CreateBloodSpurt(DVector *pvDir)
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell) return DFALSE;
	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return DFALSE;

	char* szBlood[4] = { "spritetextures\\particles\\blooddrop_1.dtx", 
						 "spritetextures\\particles\\blooddrop_2.dtx",
						 "spritetextures\\particles\\blooddrop_3.dtx",
						 "spritetextures\\particles\\blooddrop_4.dtx"};

	PESCREATESTRUCT pe;

	VEC_MULSCALAR(pe.vMinVel, *pvDir, 0.6f);
	VEC_MULSCALAR(pe.vMaxVel, *pvDir, 0.6f);

	VEC_COPY(pe.vPos, m_vPos);
//	ROT_COPY(pe.rSurfaceRot, m_rRotation);
	VEC_SET(pe.vColor1, 128.0f, 128.0f, 128.0f);
	VEC_SET(pe.vColor2, 128.0f, 128.0f, 128.0f);
	VEC_SET(pe.vMinDriftOffset, 0.0f, -10.0f, 0.0f);
	VEC_SET(pe.vMaxDriftOffset, 0.0f, -5.0f, 0.0f);
	pe.bSmall			= DFALSE;
	pe.fLifeTime		= 2.0f;
	pe.fFadeTime		= 0.5f;
	pe.fOffsetTime		= 0.0f;
	pe.fRadius			= 200.0f;
	pe.fGravity			= -100.0f;
	pe.nNumPerPuff		= 2;
	pe.nNumEmitters		= 1; //GetRandom(1,4);
	pe.nEmitterFlags	= MO_HALFGRAVITY;
	pe.bIgnoreWind		= DTRUE;
	pe.pFilename		= szBlood[GetRandom(0,2)];
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

	return DTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGibFX::CreateBloodSplat
//
//	PURPOSE:	Create a blood splat
//
// ----------------------------------------------------------------------- //

void CGibFX::CreateBloodSplat(DVector *pvPos, ClientIntersectInfo *pInfo)
{
	CBloodClientShell *pShell = (CBloodClientShell*)g_pClientDE->GetClientShell();
	if (!pShell) return;
	CSFXMgr* psfxMgr = pShell->GetSFXMgr();
	if (!psfxMgr) return;

	BSCREATESTRUCT splat;
	char* pSplatSprite = DNULL;

	switch(GetRandom(1,3))
	{
		case 1:		pSplatSprite = "sprites\\blood1.spr";	break;
		case 2:		pSplatSprite = "sprites\\blood2.spr";	break;
		case 3:		pSplatSprite = "sprites\\blood3.spr";	break;
		default:	pSplatSprite = "sprites\\blood1.spr";	break;
	}

	VEC_COPY(splat.m_Pos, *pvPos);
	m_pClientDE->AlignRotation(&splat.m_Rotation, &pInfo->m_Plane.m_Normal, &pInfo->m_Plane.m_Normal);
	splat.m_fScale = 0.14f + GetRandom(-0.1f,0.1f);
	splat.m_hstrSprite = m_pClientDE->CreateString(pSplatSprite);
	
	if(pInfo->m_Plane.m_Normal.y >= 0.5f)
		splat.m_fGrowScale = 0.01f;

	psfxMgr->CreateSFX(SFX_BLOODSPLAT_ID, &splat, DFALSE, this);
	g_pClientDE->FreeString( splat.m_hstrSprite );
}