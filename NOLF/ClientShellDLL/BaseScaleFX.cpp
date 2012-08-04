// ----------------------------------------------------------------------- //
//
// MODULE  : BaseScaleFX.cpp
//
// PURPOSE : BaseScale special FX - Implementation
//
// CREATED : 5/27/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BaseScaleFX.h"
#include "iltclient.h"
#include "GameClientShell.h"
#include "VarTrack.h"
#include "winutil.h"

extern CGameClientShell* g_pGameClientShell;

static VarTrack	g_vtRotate;
static VarTrack	g_vtRotateVel;
static VarTrack	g_vtRotateLeft;
static VarTrack	g_vtFaceCamera;

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

	m_rRot				= pBaseScale->rRot;
	m_vPos				= pBaseScale->vPos;
	m_vVel				= pBaseScale->vVel;
	m_vInitialScale		= pBaseScale->vInitialScale;
	m_vFinalScale		= pBaseScale->vFinalScale;
	m_vInitialColor		= pBaseScale->vInitialColor;
	m_vFinalColor		= pBaseScale->vFinalColor;
	m_bUseUserColors	= pBaseScale->bUseUserColors;
	m_dwFlags			= pBaseScale->dwFlags;
	m_fLifeTime			= pBaseScale->fLifeTime;
	m_fDelayTime		= pBaseScale->fDelayTime;
	m_fInitialAlpha		= pBaseScale->fInitialAlpha;
	m_fFinalAlpha		= pBaseScale->fFinalAlpha;
	m_pFilename			= pBaseScale->pFilename;
	m_pSkin				= pBaseScale->pSkin;
	m_pSkin2			= pBaseScale->pSkin2;
	m_bLoop				= pBaseScale->bLoop;
	m_bAdditive			= pBaseScale->bAdditive;
	m_bMultiply			= pBaseScale->bMultiply;
	m_bChromakey		= pBaseScale->bChromakey;
	m_nType				= pBaseScale->nType;
	m_bRotate			= pBaseScale->bRotate;
	m_bFaceCamera		= pBaseScale->bFaceCamera;
	m_fRotateVel		= GetRandom(pBaseScale->fMinRotateVel, pBaseScale->fMaxRotateVel);
	m_nRotationAxis		= pBaseScale->nRotationAxis;

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

	SAFE_STRCPY(createStruct.m_Filename, m_pFilename);
	if (m_pSkin)
	{
		if (m_pSkin2)
		{
			SAFE_STRCPY(createStruct.m_SkinNames[0], m_pSkin);
			SAFE_STRCPY(createStruct.m_SkinNames[1], m_pSkin2);
		}
		else
		{
			SAFE_STRCPY(createStruct.m_SkinName, m_pSkin);
		}
	}

	// Allow create object to be called to re-init object...

	if (m_hObject)
	{
		// See if we are changing object types...

		if (pClientDE->GetObjectType(m_hObject) != m_nType)
		{
			// Shit, need to re-create object...

			pClientDE->DeleteObject(m_hObject);
            m_hObject = LTNULL;
		}
		else  // Cool, can re-use object...
		{
			pClientDE->Common()->SetObjectFilenames(m_hObject, &createStruct);
			pClientDE->SetObjectFlags(m_hObject, m_dwFlags);
			pClientDE->SetObjectPos(m_hObject, &m_vPos);
			pClientDE->SetObjectRotation(m_hObject, &m_rRot);
		}
	}


	// See if we need to create the object...

	if (!m_hObject)
	{
		createStruct.m_ObjectType	= m_nType;
		createStruct.m_Flags		= m_dwFlags;
		createStruct.m_Pos			= m_vPos;
		createStruct.m_Rotation		= m_rRot;

		m_hObject = pClientDE->CreateObject(&createStruct);
        if (!m_hObject) return LTFALSE;
	}


	// Set blend modes if applicable...

    uint32 dwFlags;
    g_pLTClient->Common()->GetObjectFlags(m_hObject, OFT_Flags2, dwFlags);

	// Clear flags...
	dwFlags &= ~FLAG2_ADDITIVE;
	dwFlags &= ~FLAG2_MULTIPLY;

    LTBOOL bFog = LTTRUE;
	if (m_bAdditive)
	{
		dwFlags |= FLAG2_ADDITIVE;
 		dwFlags &= ~FLAG2_MULTIPLY;
		bFog = LTFALSE;
	}
	else if (m_bMultiply)
	{
		dwFlags |= FLAG2_MULTIPLY;
		dwFlags &= ~FLAG2_ADDITIVE;
        bFog = LTFALSE;
	}

	if (!m_bAdditive)
	{
		dwFlags &= ~FLAG2_ADDITIVE;
	}

	if (!m_bMultiply)
	{
 		dwFlags &= ~FLAG2_MULTIPLY;
	}

	if (m_bChromakey)
	{
		dwFlags |= FLAG2_CHROMAKEY;
	}
	else
	{
		dwFlags &= ~FLAG2_CHROMAKEY;
	}

    g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags);


	// Enable/Disable fog as appropriate...

    g_pLTClient->Common()->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
	if (bFog)
	{
		dwFlags &= ~FLAG_FOGDISABLE;
	}
	else
	{
		dwFlags |= FLAG_FOGDISABLE;
	}

    g_pLTClient->Common()->SetObjectFlags(m_hObject, OFT_Flags, dwFlags);

	return Reset();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBaseScaleFX::Reset
//
//	PURPOSE:	Reset the object
//
// ----------------------------------------------------------------------- //

LTBOOL CBaseScaleFX::Reset()
{
    if (!m_hObject) return LTFALSE;

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

	m_fStartTime = CWinUtil::GetTime() + m_fDelayTime;
	m_fEndTime	 = m_fStartTime + m_fLifeTime;
	m_fLastTime  = m_fStartTime;

	if (m_vVel.x != 0.0f || m_vVel.y != 0.0 || m_vVel.z != 0.0)
	{
		InitMovingObject(&m_movingObj, &m_vPos, &m_vVel);
		m_movingObj.m_dwPhysicsFlags |= MO_NOGRAVITY;
	}

	if (m_nType == OT_MODEL)
	{
		m_pClientDE->SetModelLooping(m_hObject, m_bLoop);
	}


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

    LTFLOAT fTime = CWinUtil::GetTime();

	if (fTime > m_fEndTime)
	{
        return LTFALSE;
	}
	else if (fTime < m_fStartTime)
	{
        uint32 dwFlags = m_pClientDE->GetObjectFlags(m_hObject);
		m_pClientDE->SetObjectFlags(m_hObject, dwFlags & ~FLAG_VISIBLE);
        return LTTRUE;  // not yet...
	}
	else
	{
        uint32 dwFlags = m_pClientDE->GetObjectFlags(m_hObject);
		m_pClientDE->SetObjectFlags(m_hObject, dwFlags | FLAG_VISIBLE);
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

	// Update the "previous time" value
	m_fLastTime = fTime;

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

		//m_pClientDE->CPrint("Color = (%.2f, %.2f, %.2f), Alpha = %.2f", vColor.x, vColor.y, vColor.z, fAlpha);
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
	vScale.Init();

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

	if (m_movingObj.m_dwPhysicsFlags & MO_RESTING) return;

    LTVector vNewPos;
    if (UpdateMovingObject(LTNULL, &m_movingObj, &vNewPos))
	{
		m_movingObj.m_vLastPos = m_movingObj.m_vPos;
		m_movingObj.m_vPos = vNewPos;

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
	if (!m_bRotate || !m_hObject) return;

    LTVector vU, vR, vF, vAxis;
    LTRotation rRot;
    g_pLTClient->GetObjectRotation(m_hObject, &rRot);
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	// See if this is a rotatable sprite and we want it to face the
	// camera...
	if (m_bFaceCamera)
	{
        uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hObject);
		if (dwFlags & FLAG_ROTATEABLESPRITE)
		{
			// Okay, make sure we're facing the camera...

			HOBJECT hCamera = g_pGameClientShell->GetCamera();
			if (hCamera)
			{
                LTVector vCamPos, vPos;
                g_pLTClient->GetObjectPos(hCamera, &vCamPos);
                g_pLTClient->GetObjectPos(m_hObject, &vPos);

				vF = vCamPos - vPos;
				vF.Norm();
                g_pLTClient->AlignRotation(&rRot, &vF, &vU);
			}
		}
	}

	if (m_nType == OT_MODEL && m_nRotationAxis == 1)
	{
		VEC_COPY(vAxis,vU);
	}
	else if (m_nType == OT_MODEL && m_nRotationAxis == 2)
	{
		VEC_COPY(vAxis,vR);
	}
	else
		VEC_COPY(vAxis,vF);

    g_pLTClient->RotateAroundAxis(&rRot, &vAxis, m_fRotateVel * (fTimeDelta - (m_fLastTime - m_fStartTime)));
    g_pLTClient->SetObjectRotation(m_hObject, &rRot);
}

void CBaseScaleFX::AdjustScale(LTFLOAT fScaleMultiplier)
{
	VEC_MULSCALAR(m_vInitialScale, m_vInitialScale, fScaleMultiplier);
	VEC_MULSCALAR(m_vFinalScale, m_vFinalScale, fScaleMultiplier);

	m_pClientDE->SetObjectScale(m_hObject, &m_vInitialScale);

}