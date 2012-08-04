// ----------------------------------------------------------------------- //
//
// MODULE  : LeashFX.cpp
//
// PURPOSE : Leash special FX - Implementation
//
// CREATED : 4/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LeashFX.h"
#include "iltclient.h"
#include "SFXMgr.h"
#include "iltcustomdraw.h"
#include "GameClientShell.h"
#include "DynamicLightFX.h"
#include "GameButes.h"

extern CGameClientShell* g_pGameClientShell;

/*
void LeashDrawFn(CustomDrawLT *pDraw, HLOCALOBJ hObj, void *pUser)
{
	if (!hObj) return;

    if (g_pLTClient->GetObjectFlags(hObj) & FLAG_VISIBLE)
	{
		CLeashFX* pFX = (CLeashFX*)pUser;
		if (pFX)
		{
			pFX->DrawLeash(pDraw);
		}
	}
}
*/

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeashFX::Init
//
//	PURPOSE:	Init the Leash fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLeashFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CBasePolyDrawFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	// Read in the init info from the message...

	LEASHFXCREATESTRUCT lfx;

	lfx.hServerObj = hServObj;

    g_pLTClient->ReadFromMessageVector(hMessage, &(lfx.vStartPos));
    g_pLTClient->ReadFromMessageVector(hMessage, &(lfx.vEndPos));
    g_pLTClient->ReadFromMessageVector(hMessage, &(lfx.vLeashColor));

    lfx.fLeashSize      = g_pLTClient->ReadFromMessageFloat(hMessage);
    lfx.cSegments       = g_pLTClient->ReadFromMessageByte(hMessage);

	return Init(&lfx);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeashFX::Init
//
//	PURPOSE:	Init the Leash fx
//
// ----------------------------------------------------------------------- //

LTBOOL CLeashFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CBasePolyDrawFX::Init(psfxCreateStruct)) return LTFALSE;

	LEASHFXCREATESTRUCT* pLEASHFX = (LEASHFXCREATESTRUCT*)psfxCreateStruct;

	m_cs   = *pLEASHFX;
	m_vPos = m_cs.vStartPos;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeashFX::CreateObject
//
//	PURPOSE:	Create object associated the object
//
// ----------------------------------------------------------------------- //

LTBOOL CLeashFX::CreateObject(ILTClient *pClientDE)
{
    if (!CBasePolyDrawFX::CreateObject(pClientDE)) return LTFALSE;

	// Validate our init info...

    if (m_cs.cSegments < 1) return LTFALSE;

	// Set up the Leash...

	return SetupLeash();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeashFX::SetupLeash
//
//	PURPOSE:	Setup the line used to draw Leash
//
// ----------------------------------------------------------------------- //

LTBOOL CLeashFX::SetupLeash()
{
	// Allocate our verts if necessary...

	if (!m_pVerts)
	{
		m_pVerts = debug_newa(LeashVerts, m_cs.cSegments+1);
	}

    if (!m_pVerts) return LTFALSE;

//  LTVector vPos = m_cs.vStartPos;
//  LTVector vDir = (m_cs.vEndPos - m_cs.vStartPos);
//	float fDist  = vDir.Mag();
//	vDir.Norm();

	for (int i=0; i < m_cs.cSegments; i++)
	{
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeashFX::Draw
//
//	PURPOSE:	Draw the Leash
//
// ----------------------------------------------------------------------- //

LTBOOL CLeashFX::Draw(ILTCustomDraw *pDraw)
{
    if (!pDraw || !m_pVerts) return LTFALSE;

	HLOCALOBJ hCamera = g_pGameClientShell->GetCamera();
    if (!hCamera) return LTFALSE;

    LTRotation rRot;
    g_pLTClient->GetObjectRotation(hCamera, &rRot);

    LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

    LTVertex verts[4];

	for (int i=0; i < m_cs.cSegments; i++)
	{
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeashFX::HandleFirstTime
//
//	PURPOSE:	Handle the first time drawing...
//
// ----------------------------------------------------------------------- //

void CLeashFX::HandleFirstTime()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLeashFX::Update
//
//	PURPOSE:	Update the Leash
//
// ----------------------------------------------------------------------- //

LTBOOL CLeashFX::Update()
{
    LTFLOAT fTime = g_pLTClient->GetTime();

	// Hide/show Leash if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
        g_pLTClient->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
			m_Flags &= ~FLAG_VISIBLE;
            return LTTRUE;
		}
		else
		{
			m_Flags |= FLAG_VISIBLE;
		}
	}

	// Do first time stuff...

	if (m_bFirstTime)
	{
        m_bFirstTime = LTFALSE;
		HandleFirstTime();
	}

    return LTTRUE;
}