// ----------------------------------------------------------------------- //
//
// MODULE  : MineFX.cpp
//
// PURPOSE : Mine FX - Implementation
//
// CREATED : 2/26/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MineFX.h"
#include "WeaponModel.h"
#include "VarTrack.h"
#include "GameClientShell.h"

VarTrack	g_vtMineShit;

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMineFX::Init
//
//	PURPOSE:	Init the laser trigger
//
// ----------------------------------------------------------------------- //

LTBOOL CMineFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	MINECREATESTRUCT mine;

	mine.hServerObj = hServObj;
    mine.Read(g_pLTClient, hMessage);

	return Init(&mine);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMineFX::Init
//
//	PURPOSE:	Init the laser trigger fx
//
// ----------------------------------------------------------------------- //

LTBOOL CMineFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((MINECREATESTRUCT*)psfxCreateStruct);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMineFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CMineFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

	if (!g_vtMineShit.IsInitted())
	{
        g_vtMineShit.Init(g_pLTClient, "MineShit", NULL, 0.01f);
	}

    LTVector vDims;
    g_pLTClient->Physics()->GetObjectDims(m_hServerObject, &vDims);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMineFX::Update
//
//	PURPOSE:	Update the beam fx
//
// ----------------------------------------------------------------------- //

LTBOOL CMineFX::Update()
{
    if (!m_pClientDE || !m_hServerObject || m_bWantRemove) return LTFALSE;

    LTBOOL bShowMine = LTFALSE;

	// See if this client can see the mine...

	if (g_pInterfaceMgr->GetSunglassMode() == SUN_MINES)
	{
		bShowMine = LTTRUE;
	}

	// Show/Hide the mine fx...

	if (bShowMine)
	{
        LTVector vPos;
        g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

		HOBJECT hObj = m_RadiusModel.GetObject();
		if (!hObj)
		{
			// Create fx...

			m_scalecs.dwFlags = FLAG_VISIBLE | FLAG_NOLIGHT;

			// Set up fx flags...

			m_scalecs.pFilename			= "SFX\\Expl\\Models\\364.abc";
			m_scalecs.pSkin				= "SFX\\Beams\\Skins\\Spotlight_beam.dtx";
			m_scalecs.vPos				= vPos;
			m_scalecs.vVel.Init();
			m_scalecs.vInitialScale.Init(m_cs.fMinRadius, m_cs.fMinRadius, m_cs.fMinRadius);
			m_scalecs.vFinalScale.Init(m_cs.fMaxRadius, m_cs.fMaxRadius, m_cs.fMaxRadius);
			m_scalecs.vInitialColor.Init(1, 1, 0);
			m_scalecs.vFinalColor.Init(1, 0, 0);
            m_scalecs.bUseUserColors    = LTTRUE;
			m_scalecs.fLifeTime			= 1.0f;
			m_scalecs.fInitialAlpha		= 0.5f;
			m_scalecs.fFinalAlpha		= 0.9f;
            m_scalecs.bLoop             = LTTRUE;
			m_scalecs.fDelayTime		= 0.0f;
            m_scalecs.bAdditive         = LTTRUE;
			m_scalecs.nType				= OT_MODEL;

			m_RadiusModel.Init(&m_scalecs);
            m_RadiusModel.CreateObject(g_pLTClient);
		}
		else
		{
			// Update fx...

            uint32 dwFlags = g_pLTClient->GetObjectFlags(hObj);
            g_pLTClient->SetObjectFlags(hObj, dwFlags | FLAG_VISIBLE);
		}


		// See if it is time to scale the other way...

		if (!m_RadiusModel.Update())
		{
			if (m_scalecs.vInitialScale.x == m_cs.fMinRadius)
			{
				m_scalecs.vInitialScale.Init(m_cs.fMaxRadius, m_cs.fMaxRadius, m_cs.fMaxRadius);
				m_scalecs.vFinalScale.Init(m_cs.fMinRadius, m_cs.fMinRadius, m_cs.fMinRadius);
				m_scalecs.fInitialAlpha	= 0.9f;
				m_scalecs.fFinalAlpha	= 0.4f;
				m_scalecs.vInitialColor.Init(1, 0, 0);
				m_scalecs.vFinalColor.Init(1, 1, 1);
			}
			else
			{
				m_scalecs.vInitialScale.Init(m_cs.fMinRadius, m_cs.fMinRadius, m_cs.fMinRadius);
				m_scalecs.vFinalScale.Init(m_cs.fMaxRadius, m_cs.fMaxRadius, m_cs.fMaxRadius);
				m_scalecs.fInitialAlpha	= 0.4f;
				m_scalecs.fFinalAlpha	= 0.9f;
				m_scalecs.vInitialColor.Init(1, 1, 1);
				m_scalecs.vFinalColor.Init(1, 0, 0);
			}

			m_RadiusModel.Init(&m_scalecs);
            m_RadiusModel.CreateObject(g_pLTClient);
		}
	}
	else
	{
		// Hide fx...

		HOBJECT hObj = m_RadiusModel.GetObject();
		if (hObj)
		{
            uint32 dwFlags = g_pLTClient->GetObjectFlags(hObj);
            g_pLTClient->SetObjectFlags(hObj, dwFlags & ~FLAG_VISIBLE);
		}
	}


    return LTTRUE;
}