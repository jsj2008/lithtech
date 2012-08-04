// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerVehicleFX.cpp
//
// PURPOSE : Beam special FX - Implementation
//
// CREATED : 6/8/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PlayerVehicleFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "SFXMsgIds.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerVehicleFX::Init
//
//	PURPOSE:	Init the beam fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerVehicleFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	PVCREATESTRUCT cs;

	cs.hServerObj = hServObj;
    cs.Read(g_pLTClient, hMessage);

	return Init(&cs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerVehicleFX::Init
//
//	PURPOSE:	Init the player vehicle fx
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerVehicleFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	PVCREATESTRUCT* pFX = (PVCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pFX;

     return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerVehicleFX::Update
//
//	PURPOSE:	Update the tracer
//
// ----------------------------------------------------------------------- //

LTBOOL CPlayerVehicleFX::Update()
{
	if (!CSpecialFX::Update()) return LTFALSE;

	if (m_vVel.Mag() > 5.0f)
	{
		if (m_cs.ePhysicsModel == PPM_MOTORCYCLE)
		{
			// g_pLTClient->CPrint("Motorcycle is moving!");
		}
	}
	else
	{
		if (m_cs.ePhysicsModel == PPM_MOTORCYCLE)
		{
			// g_pLTClient->CPrint("Motorcycle is NOT moving!");
		}
	}

    return LTTRUE;
}