// ----------------------------------------------------------------------- //
//
// MODULE  : BaseScaleFX.cpp
//
// PURPOSE : BaseScale special FX - Implementation
//
// CREATED : 5/27/98
//
// ----------------------------------------------------------------------- //

#include "BaseScaleFX.h"
#include "clientheaders.h"
#include "ltobjectcreate.h"

static int s_nRotDir = 1;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseScaleFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!psfxCreateStruct) return LTFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	BSCREATESTRUCT* pBaseScale = (BSCREATESTRUCT*)psfxCreateStruct;

	m_rRot = pBaseScale->rRot;
	VEC_COPY(m_vPos, pBaseScale->vPos);
	VEC_COPY(m_vVel, pBaseScale->vVel);
	VEC_COPY(m_vInitialScale, pBaseScale->vInitialScale);
	VEC_COPY(m_vFinalScale, pBaseScale->vFinalScale);
	VEC_COPY(m_vInitialColor, pBaseScale->vInitialColor);
	VEC_COPY(m_vFinalColor, pBaseScale->vFinalColor);

	m_bUseUserColors	= pBaseScale->bUseUserColors;
	m_dwFlags			= pBaseScale->dwFlags;
	m_fLifeTime			= pBaseScale->fLifeTime;
	m_fDelayTime		= pBaseScale->fDelayTime;
	m_fInitialAlpha		= pBaseScale->fInitialAlpha;
	m_fFinalAlpha		= pBaseScale->fFinalAlpha;
	m_pFilename			= pBaseScale->pFilename;
	m_pSkin				= pBaseScale->pSkin;
	m_bLoop				= pBaseScale->bLoop;

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::CreateObject
//
//	PURPOSE:	Create object associated with the BaseScale
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseScaleFX::CreateObject(ILTClient *pClientDE)
{
	if (!CSpecialFX::CreateObject(pClientDE) || !m_pFilename) return LTFALSE;

	// Setup the BaseScale...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = m_nType;
	SAFE_STRCPY(createStruct.m_Filename, m_pFilename);
	if (m_pSkin) SAFE_STRCPY(createStruct.m_SkinName, m_pSkin);

	createStruct.m_Flags = m_dwFlags;  
	VEC_COPY(createStruct.m_Pos, m_vPos);
	createStruct.m_Rotation = m_rRot;

	m_hObject = pClientDE->CreateObject(&createStruct);

	LTFLOAT r, g, b, a;
	if (m_bUseUserColors)
	{
		r = m_vInitialColor.x;
		g = m_vInitialColor.y;
		b = m_vInitialColor.z;
	}
	else
	{
		m_pClientDE->GetObjectColor(m_hObject, &r, &g, &b, &a);
	}
		
	m_pClientDE->SetObjectScale(m_hObject, &m_vInitialScale);
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, m_fInitialAlpha);

	m_fStartTime = pClientDE->GetTime() + m_fDelayTime;
	m_fEndTime	 = m_fStartTime + m_fLifeTime;

	if (m_vVel.x != 0.0f || m_vVel.y != 0.0 || m_vVel.z != 0.0)
	{
		InitMovingObject(&m_movingObj, &m_vPos, &m_vVel);
		m_movingObj.m_PhysicsFlags |= MO_NOGRAVITY;
	}

	if (m_nType == OT_MODEL)
	{
		pClientDE->SetModelLooping(m_hObject, m_bLoop);
	}


	s_nRotDir *= -1;  // Change rotation direction
	m_nRotDir = s_nRotDir;
	
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::Update
//
//	PURPOSE:	Update the BaseScale
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseScaleFX::Update()
{
	if(!m_hObject || !m_pClientDE) return LTFALSE;

	LTFLOAT fTime = m_pClientDE->GetTime();

	if (fTime > m_fEndTime)
	{
		return LTFALSE;
	}
	else if (fTime < m_fStartTime)
	{
		uint32 dwFlags;
		
		m_pClientDE->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
		dwFlags &= ~FLAG_VISIBLE;
		m_pClientDE->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
		return LTTRUE;  // not yet...
	}
	else
	{
		uint32 dwFlags;
		m_pClientDE->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
		dwFlags |= FLAG_VISIBLE;
		m_pClientDE->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags, FLAGMASK_ALL);
	}

	LTFLOAT fTimeDelta = fTime - m_fStartTime;

	if (m_fFinalAlpha != m_fInitialAlpha || m_vInitialColor.x != m_vFinalColor.x ||
		m_vInitialColor.y != m_vFinalColor.y || m_vInitialColor.z != m_vFinalColor.z)
	{
		UpdateAlpha(fTimeDelta);
	}

	if (m_vInitialScale.x != m_vFinalScale.x || 
		m_vInitialScale.y != m_vFinalScale.y ||
		m_vInitialScale.z != m_vFinalScale.z)
	{
		UpdateScale(fTimeDelta);
	}

	if (m_vVel.x != 0.0f || m_vVel.y != 0.0 || m_vVel.z != 0.0)
	{
		UpdatePos(fTimeDelta);
	}

	UpdateRot(fTimeDelta);
	
	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::UpdateAlpha
//
//	PURPOSE:	Update the BaseScale alpha
//
// ----------------------------------------------------------------------- //

void CBaseScaleFX::UpdateAlpha(LTFLOAT fTimeDelta)
{
	if(!m_hObject || !m_pClientDE) return;

	LTFLOAT fAlpha = m_fInitialAlpha + (fTimeDelta * (m_fFinalAlpha - m_fInitialAlpha) / m_fLifeTime);

	LTVector vColor;
	if (m_bUseUserColors)
	{
		vColor.x = m_vInitialColor.x + (fTimeDelta * (m_vFinalColor.x - m_vInitialColor.x) / m_fLifeTime);
		vColor.y = m_vInitialColor.y + (fTimeDelta * (m_vFinalColor.y - m_vInitialColor.y) / m_fLifeTime);
		vColor.z = m_vInitialColor.z + (fTimeDelta * (m_vFinalColor.z - m_vInitialColor.z) / m_fLifeTime);
	}
	else
	{
		LTFLOAT a;
		m_pClientDE->GetObjectColor(m_hObject, &(vColor.x), &(vColor.y), &(vColor.z), &a);
	}

	m_pClientDE->SetObjectColor(m_hObject, vColor.x, vColor.y, vColor.z, fAlpha);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::UpdateScale
//
//	PURPOSE:	Update the BaseScale alpha
//
// ----------------------------------------------------------------------- //

void CBaseScaleFX::UpdateScale(LTFLOAT fTimeDelta)
{
	if(!m_hObject || !m_pClientDE) return;

	LTVector vScale;
	VEC_INIT(vScale);

	vScale.x = m_vInitialScale.x + (fTimeDelta * (m_vFinalScale.x - m_vInitialScale.x) / m_fLifeTime);
	vScale.y = m_vInitialScale.y + (fTimeDelta * (m_vFinalScale.y - m_vInitialScale.y) / m_fLifeTime);
	vScale.z = m_vInitialScale.z + (fTimeDelta * (m_vFinalScale.z - m_vInitialScale.z) / m_fLifeTime);

	m_pClientDE->SetObjectScale(m_hObject, &vScale);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::UpdatePos
//
//	PURPOSE:	Update the BaseScale's pos
//
// ----------------------------------------------------------------------- //

void CBaseScaleFX::UpdatePos(LTFLOAT fTimeDelta)
{
	if(!m_hObject || !m_pClientDE) return;

	if (m_movingObj.m_PhysicsFlags & MO_RESTING) return;

	LTVector vNewPos;
	if (UpdateMovingObject(LTNULL, &m_movingObj, &vNewPos))
	{
		VEC_COPY(m_movingObj.m_LastPos, m_movingObj.m_Pos);
		VEC_COPY(m_movingObj.m_Pos, vNewPos);

		m_pClientDE->SetObjectPos(m_hObject, &vNewPos);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::UpdateRot
//
//	PURPOSE:	Update the BaseScale's rotation
//
// ----------------------------------------------------------------------- //

void CBaseScaleFX::UpdateRot(LTFLOAT fTimeDelta)
{
	if(!m_hObject || !m_pClientDE) return;

	return;

	LTVector vU, vR, vF;
	LTRotation rRot;
	m_pClientDE->GetObjectRotation(m_hObject, &rRot);
	m_pClientDE->Common()->GetRotationVectors(rRot, vU, vR, vF);
	VEC_NORM(vF);

	m_pClientDE->Math()->RotateAroundAxis(rRot, vF, m_nRotDir * 0.25f * fTimeDelta);
	m_pClientDE->SetObjectRotation(m_hObject, &rRot);
}