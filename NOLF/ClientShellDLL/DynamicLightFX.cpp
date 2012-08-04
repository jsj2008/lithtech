// ----------------------------------------------------------------------- //
//
// MODULE  : DynamicLightFX.cpp
//
// PURPOSE : Dynamic light special FX - Implementation
//
// CREATED : 2/25/98
//
// (c) 1998-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "DynamicLightFX.h"
#include "iltclient.h"
#include "VarTrack.h"

static VarTrack g_vtDLightOffsetX;
static VarTrack g_vtDLightOffsetY;
static VarTrack g_vtDLightOffsetZ;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDynamicLightFX::Init
//
//	PURPOSE:	Init the dynamic light
//
// ----------------------------------------------------------------------- //

LTBOOL CDynamicLightFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	DLCREATESTRUCT* pDL = (DLCREATESTRUCT*)psfxCreateStruct;

	m_vColor		 = pDL->vColor;
	m_vPos			 = pDL->vPos;
	m_fMinRadius     = pDL->fMinRadius;
	m_fMaxRadius	 = pDL->fMaxRadius;
	m_fRampUpTime	 = pDL->fRampUpTime;
	m_fMaxTime		 = pDL->fMaxTime;
	m_fMinTime		 = pDL->fMinTime;
	m_fRampDownTime  = pDL->fRampDownTime;
	m_dwFlags		 = pDL->dwFlags;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDynamicLightFX::CreateObject
//
//	PURPOSE:	Create object associated the dynamic light.
//
// ----------------------------------------------------------------------- //

LTBOOL CDynamicLightFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	if (!g_vtDLightOffsetX.IsInitted())
	{
		g_vtDLightOffsetX.Init(pClientDE, "DLightOffsetX", "", 0.0f);
	}

	if (!g_vtDLightOffsetY.IsInitted())
	{
		g_vtDLightOffsetY.Init(pClientDE, "DLightOffsetY", "", 0.0f);
	}

	if (!g_vtDLightOffsetZ.IsInitted())
	{
		g_vtDLightOffsetZ.Init(pClientDE, "DLightOffsetZ", "", 0.0f);
	}

	// Allow create object to be called to re-init object...

	if (m_hObject)
	{
		m_pClientDE->SetObjectFlags(m_hObject, m_dwFlags);
		m_pClientDE->SetObjectPos(m_hObject, &m_vPos);
	}
	else
	{
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType = OT_LIGHT;
		createStruct.m_Flags = m_dwFlags;
		createStruct.m_Pos = m_vPos;

		// TESTING FOR MUZZLE FLASH!!!!
		createStruct.m_Pos.x += g_vtDLightOffsetX.GetFloat();
		createStruct.m_Pos.y += g_vtDLightOffsetY.GetFloat();
		createStruct.m_Pos.z += g_vtDLightOffsetZ.GetFloat();
		// TESTING!!!!

		m_hObject = m_pClientDE->CreateObject(&createStruct);
        if (!m_hObject) return LTFALSE;
	}

	m_pClientDE->SetLightColor(m_hObject, m_vColor.x, m_vColor.y, m_vColor.z);
	m_pClientDE->SetLightRadius(m_hObject, m_fMinRadius);

	m_fStartTime = m_pClientDE->GetTime();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CDynamicLightFX::Update
//
//	PURPOSE:	Update the light
//
// ----------------------------------------------------------------------- //

LTBOOL CDynamicLightFX::Update()
{
    if(!m_hObject || !m_pClientDE) return LTFALSE;

    LTFLOAT fTime   = m_pClientDE->GetTime();
    LTFLOAT fRadius = m_fMinRadius;

	if (fTime < m_fStartTime + m_fMinTime)
	{
        return LTTRUE;
	}
	else if (fTime < m_fStartTime + m_fMinTime + m_fRampUpTime)
	{
		if (m_fRampUpTime > 0.0f)
		{
            LTFLOAT fDelta  = fTime - (m_fStartTime + m_fMinTime);
            LTFLOAT fOffset = fDelta * (m_fMaxRadius - m_fMinRadius)/m_fRampUpTime;
			fRadius += fOffset;
		}
	}
	else if (fTime < m_fStartTime + m_fMinTime + m_fRampUpTime + m_fMaxTime)
	{
		fRadius = m_fMaxRadius;
	}
	else if (fTime < m_fStartTime + m_fMinTime + m_fRampUpTime + m_fMaxTime + m_fRampDownTime)
	{
		if (m_fRampDownTime > 0.0f)
		{
            LTFLOAT fDelta  = fTime - (m_fStartTime + m_fMinTime + m_fRampUpTime + m_fMaxTime);
            LTFLOAT fOffset = fDelta * m_fMaxRadius/m_fRampDownTime;
			fRadius = m_fMaxRadius - fOffset;
		}
	}
	else
	{
        return LTFALSE;
	}

	m_pClientDE->SetLightRadius(m_hObject, fRadius);

    return LTTRUE;
}