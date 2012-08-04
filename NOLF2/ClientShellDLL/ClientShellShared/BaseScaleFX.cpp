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
#ifndef __PSX2
#include "winutil.h"
#endif

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
	m_pSkinReader		= pBaseScale->pSkinReader;
	m_pRenderStyleReader = pBaseScale->pRenderStyleReader;
	m_bLoop				= pBaseScale->bLoop;
	m_bAdditive			= pBaseScale->bAdditive;
	m_bMultiply			= pBaseScale->bMultiply;
	m_nType				= pBaseScale->nType;
	m_bRotate			= pBaseScale->bRotate;
	m_bFaceCamera		= pBaseScale->bFaceCamera;
	m_bPausable			= pBaseScale->bPausable;
	m_fRotateVel		= GetRandom(pBaseScale->fMinRotateVel, pBaseScale->fMaxRotateVel);
	m_nRotationAxis		= pBaseScale->nRotationAxis;
	m_nMenuLayer		= pBaseScale->nMenuLayer;

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

	if(m_pSkinReader)
	{
		m_pSkinReader->CopyList(0, createStruct.m_SkinNames[0], MAX_CS_FILENAME_LEN+1);
	}

	if(m_pRenderStyleReader)
	{
		m_pRenderStyleReader->CopyList(0, createStruct.m_RenderStyleNames[0], MAX_CS_FILENAME_LEN+1);
	}

	// Allow create object to be called to re-init object...

	if (m_hObject)
	{
		// See if we are changing object types...

		if (GetObjectType(m_hObject) != m_nType)
		{
			// Shit, need to re-create object...

			pClientDE->RemoveObject(m_hObject);
            m_hObject = LTNULL;
		}
		else  // Cool, can re-use object...
		{
			g_pCommonLT->SetObjectFilenames(m_hObject, &createStruct);
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags, FLAGMASK_ALL);
			g_pLTClient->SetObjectPosAndRotation(m_hObject, &m_vPos, &m_rRot);
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

    uint32 dwFlags = 0;

	// Set up the flags
    LTBOOL bFog = LTTRUE;
	if (m_bAdditive)
	{
		dwFlags |= FLAG2_ADDITIVE;
		bFog = LTFALSE;
	}
	else if (m_bMultiply)
	{
		dwFlags |= FLAG2_MULTIPLY;
        bFog = LTFALSE;
	}

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, dwFlags, FLAG2_ADDITIVE | FLAG2_MULTIPLY);


	// Enable/Disable fog as appropriate...

    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, (bFog) ? 0 : FLAG_FOGDISABLE, FLAG_FOGDISABLE);

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

	m_fStartTime	= m_fDelayTime;
	m_fEndTime		= m_fStartTime + m_fLifeTime;
	m_fElapsedTime  = 0.0f;

	if (m_vVel.x != 0.0f || m_vVel.y != 0.0 || m_vVel.z != 0.0)
	{
		InitMovingObject(&m_movingObj, m_vPos, m_vVel);
		m_movingObj.m_dwPhysicsFlags |= MO_NOGRAVITY;
	}

	if (m_nType == OT_MODEL)
	{
		m_pClientDE->SetModelLooping(m_hObject, m_bLoop != LTFALSE);
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
    if(!m_hObject || !m_pClientDE) 
		return LTFALSE;

	//handle updating the elapsed time
	float fFrameTime = m_pClientDE->GetFrameTime();

	//see if we are paused though
	if(m_bPausable && g_pGameClientShell->IsServerPaused())
		fFrameTime = 0.0f;

	m_fElapsedTime += fFrameTime;

	if (m_fElapsedTime > m_fEndTime)
	{
        return LTFALSE;
	}
	else if (m_fElapsedTime < m_fStartTime)
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE);
        return LTTRUE;  // not yet...
	}
	else
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	}

	float fElapsedFromStart = m_fElapsedTime - m_fStartTime;

	if (m_fFinalAlpha != m_fInitialAlpha || m_vInitialColor.x != m_vFinalColor.x ||
		m_vInitialColor.y != m_vFinalColor.y || m_vInitialColor.z != m_vFinalColor.z)
	{
		UpdateAlpha(fElapsedFromStart);
	}

	if (m_vInitialScale.x != m_vFinalScale.x ||
		m_vInitialScale.y != m_vFinalScale.y ||
		m_vInitialScale.z != m_vFinalScale.z)
	{
		UpdateScale(fElapsedFromStart);
	}

	if (m_vVel.x != 0.0f || m_vVel.y != 0.0 || m_vVel.z != 0.0)
	{
		UpdatePos(fElapsedFromStart);
	}

	UpdateRot(fElapsedFromStart);

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
    if (UpdateMovingObject(LTNULL, &m_movingObj, vNewPos))
	{
		m_movingObj.m_vLastPos = m_movingObj.m_vPos;
		m_movingObj.m_vPos = vNewPos;

		g_pLTClient->SetObjectPos(m_hObject, &vNewPos);
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
	vU = rRot.Up();
	vR = rRot.Right();
	vF = rRot.Forward();

	// See if this is a rotatable sprite and we want it to face the
	// camera...
	if (m_bFaceCamera)
	{
        uint32 dwFlags;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, dwFlags);
		if (dwFlags & FLAG_ROTATEABLESPRITE)
		{
			// Okay, make sure we're facing the camera...

			HOBJECT hCamera = g_pPlayerMgr->GetCamera();
			if (hCamera)
			{
                LTVector vCamPos, vPos;
				g_pLTClient->GetObjectPos(hCamera, &vCamPos);
                g_pLTClient->GetObjectPos(m_hObject, &vPos);

				vF = vCamPos - vPos;
				rRot = LTRotation(vF, vU);
			}
		}
	}

	if (m_nType == OT_MODEL && m_nRotationAxis == 1)
	{
		vAxis = vU;
	}
	else if (m_nType == OT_MODEL && m_nRotationAxis == 2)
	{
		vAxis = vR;
	}
	else
		vAxis = vF;

	rRot.Rotate(vAxis, m_fRotateVel * (fTimeDelta - (m_fElapsedTime - m_fStartTime)));
	g_pLTClient->SetObjectRotation(m_hObject, &rRot);
}

void CBaseScaleFX::AdjustScale(LTFLOAT fScaleMultiplier)
{
	VEC_MULSCALAR(m_vInitialScale, m_vInitialScale, fScaleMultiplier);
	VEC_MULSCALAR(m_vFinalScale, m_vFinalScale, fScaleMultiplier);

	m_pClientDE->SetObjectScale(m_hObject, &m_vInitialScale);

}