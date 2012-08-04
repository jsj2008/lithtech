// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItemFX.cpp
//
// PURPOSE : PickupItem - Implementation
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PickupItemFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"

extern CGameClientShell* g_pGameClientShell;

#define PICKUPITEM_ROTVEL	0.3333f * MATH_CIRCLE

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Init
//
//	PURPOSE:	Init the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupItemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	PICKUPITEMCREATESTRUCT* pPICS = (PICKUPITEMCREATESTRUCT*)psfxCreateStruct;

	m_bRotate = pPICS->bRotate;
	m_bBounce = pPICS->bBounce;

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupItemFX::CreateObject(ILTClient *pClientDE)
{
    LTBOOL bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Update
//
//	PURPOSE:	Update the pickupitem
//
// ----------------------------------------------------------------------- //

LTBOOL CPickupItemFX::Update()
{
    if (!m_pClientDE || m_bWantRemove || !m_hServerObject) return LTFALSE;

    LTFLOAT fDeltaTime = g_pGameClientShell->GetFrameTime();

	if (m_bRotate)
	{
        LTRotation rRot;
		m_pClientDE->GetObjectRotation(m_hServerObject, &rRot);
		m_pClientDE->EulerRotateY(&rRot, PICKUPITEM_ROTVEL * fDeltaTime);
		m_pClientDE->SetObjectRotation(m_hServerObject, &rRot);
	}

	if (m_bBounce)
	{

	}


    return LTTRUE;
}