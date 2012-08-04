// ----------------------------------------------------------------------- //
//
// MODULE  : LaserCannonFX.cpp
//
// PURPOSE : LaserCannon special FX - Implementation
//
// CREATED : 1/20/97
//
// ----------------------------------------------------------------------- //

#include "LaserCannonFX.h"
#include "clientheaders.h"
#include "ClientUtilities.h"
#include "RiotClientShell.h"
#include "WeaponModel.h"
#include "DynamicLightFX.h"
#include "RiotMsgIds.h"
#include "ltobjectcreate.h"

#define MAX_RANGE	3000.0f

extern CRiotClientShell* g_pRiotClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserCannonFX::Init
//
//	PURPOSE:	Init the laser cannon fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLaserCannonFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	LCCREATESTRUCT* pCL = (LCCREATESTRUCT*)psfxCreateStruct;
	m_dwClientID	= pCL->dwClientID;

	m_fRotPerSec	 = 4.0f;
	m_fNextLightTime = 0.0f;
	m_fLightWaitTime = 3.0f;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserCannonFX::CreateObject
//
//	PURPOSE:	Create object associated the line system.
//
// ----------------------------------------------------------------------- //

LTBOOL CLaserCannonFX::CreateObject(ILTClient *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	LTVector vPos;
	pClientDE->GetObjectPos(m_hServerObject, &vPos);


	// Setup the model...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_MODEL;
	SAFE_STRCPY(createStruct.m_Filename, "Models\\Powerups\\beam.abc");
	SAFE_STRCPY(createStruct.m_SkinName, "SpecialFX\\ParticleTextures\\LaserBeam_2.dtx");
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
	VEC_COPY(createStruct.m_Pos, vPos);

	m_hObject = m_pClientDE->CreateObject(&createStruct);


	LTRotation rRot;
	pClientDE->GetObjectRotation(m_hServerObject, &rRot);
	pClientDE->SetObjectRotation(m_hObject, &rRot);

	pClientDE->SetObjectColor(m_hObject, 0.9f, 0.9f, 0.1f, 0.9f);

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserCannonFX::Update
//
//	PURPOSE:	Update the laser cannon fx (recalculate end point)
//
// ----------------------------------------------------------------------- //

LTBOOL CLaserCannonFX::Update()
{
	if(!m_hObject || !m_pClientDE || !m_hServerObject) return LTFALSE;

	if (m_bWantRemove)
	{
		return LTFALSE;
	}

	LTFLOAT fTime = m_pClientDE->GetTime();


	// See if we should use the gun model to get the best player-view
	// effect...

	LTRotation rRot;
	LTVector vPos;

	uint32 dwId;
	if ((m_pClientDE->GetLocalClientID(&dwId) == LT_OK) && m_dwClientID == dwId)
	{
		if (!g_pRiotClientShell) return LTFALSE;

		CWeaponModel* pWeapon = g_pRiotClientShell->GetWeaponModel();
		if (!pWeapon) return LTFALSE;

		vPos = pWeapon->GetFlashPos();
		m_pClientDE->GetObjectRotation(pWeapon->GetHandle(), &rRot);
	}
	else
	{
		// Set the rotation/position based on server object...

		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	}

	m_pClientDE->SetObjectRotation(m_hObject, &rRot);


	// Cast a ray to find the end point...

	LTVector vStartPoint, vEndPoint, vU, vR, vF;
	m_pClientDE->Common()->GetRotationVectors(rRot, vU, vR, vF);
	VEC_NORM(vF);

	ClientIntersectQuery iQuery;
	ClientIntersectInfo  iInfo;


	LTFLOAT fDistance = MAX_RANGE;

	LTVector vTemp;
	VEC_MULSCALAR(vTemp, vF, fDistance);
	VEC_ADD(vEndPoint, vPos, vTemp);

	VEC_COPY(iQuery.m_From, vPos);
	VEC_COPY(iQuery.m_To, vEndPoint);

	LTBOOL bHitWorld = LTFALSE;

	if (m_pClientDE->IntersectSegment(&iQuery, &iInfo))
	{		
		VEC_SUB(vEndPoint, iInfo.m_Point, vPos);
		fDistance = VEC_MAG(vEndPoint);
		bHitWorld = LTTRUE;
	}

	if (fDistance > MAX_RANGE) fDistance = MAX_RANGE;

	// Really only want 1/2 the total distance...

	fDistance /= 2.0f;

	VEC_SET(vEndPoint, 0.0f, 0.0f, 1.0f);  // Z is straight ahead
	VEC_MULSCALAR(vEndPoint, vEndPoint, fDistance);

	VEC_SET(vStartPoint, 0.0f, 0.0f, -1.0f);  // Z is straight behind
	VEC_MULSCALAR(vStartPoint, vStartPoint, fDistance);


	// Center the model...

	VEC_MULSCALAR(vTemp, vF, fDistance);
	VEC_ADD(vPos, vPos, vTemp);
	m_pClientDE->SetObjectPos(m_hObject, &vPos);

	LTFLOAT fOffset = .2f;

	fDistance *= 2.0f;

	LTVector vScale;
	VEC_SET(vScale, 0.8f, 0.8f, fDistance);

	m_pClientDE->SetObjectScale(m_hObject, &vScale);


	// Rotate the whole thing...

	if (m_fLastRotTime > 0.0f)
	{
		LTFLOAT fDelta = fTime - m_fLastRotTime;
		
		m_fRotAmount += 360.0f/m_fRotPerSec * fDelta;

		m_pClientDE->Math()->EulerRotateZ(rRot, m_fRotAmount);
		m_pClientDE->SetObjectRotation(m_hObject, &rRot);
	}

	m_fLastRotTime = fTime;

	// Create a cool light fx if we hit the world, and enough
	// time (energy build up) has gone by...

	if (bHitWorld && m_fNextLightTime >= fTime)
	{
		CreateLightFX(&(iInfo.m_Point));

		m_fNextLightTime = fTime + m_fLightWaitTime;
	}

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserCannonFX::CreateLightFX
//
//	PURPOSE:	Create a cool light fx 
//
// ----------------------------------------------------------------------- //

void CLaserCannonFX::CreateLightFX(LTVector* pvPos)
{
	if (!m_pClientDE || !g_pRiotClientShell || !pvPos) return;

	CSFXMgr* psfxMgr = g_pRiotClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	DLCREATESTRUCT dl;

	dl.hServerObj = LTNULL;
	VEC_SET(dl.vColor, 0.6f, 0.0f, 0.0f);
	VEC_COPY(dl.vPos, *pvPos);
	dl.fMinRadius    = 5.0f;
	dl.fMaxRadius	 = 25.0f;
	dl.fRampUpTime	 = 1.0f;
	dl.fMaxTime		 = 0.5f;
	dl.fMinTime		 = 0.0f;
	dl.fRampDownTime = 2.0f;

	psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);
}

