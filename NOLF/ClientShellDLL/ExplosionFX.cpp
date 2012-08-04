// ----------------------------------------------------------------------- //
//
// MODULE  : ExplosionFX.cpp
//
// PURPOSE : Explosion special FX - Implementation
//
// CREATED : 12/29/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ExplosionFX.h"
#include "FXButeMgr.h"
#include "ClientServerShared.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::Init
//
//	PURPOSE:	Init the particle system fx
//
// ----------------------------------------------------------------------- //

LTBOOL CExplosionFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	EXPLOSIONCREATESTRUCT cs;

	cs.hServerObj = hServObj;
    cs.Read(g_pLTClient, hMessage);

	return Init(&cs);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::Init
//
//	PURPOSE:	Init the particle system
//
// ----------------------------------------------------------------------- //

LTBOOL CExplosionFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	// Set up our creation struct...

	EXPLOSIONCREATESTRUCT* pCS = (EXPLOSIONCREATESTRUCT*)psfxCreateStruct;
	m_cs = *pCS;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CExplosionFX::CreateObject
//
//	PURPOSE:	Create object associated with the explosion
//
// ----------------------------------------------------------------------- //

LTBOOL CExplosionFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

	// Create the specified explosion...

	IMPACTFX* pImpactFX = g_pFXButeMgr->GetImpactFX(m_cs.nImpactFX);
	if (pImpactFX)
	{
		// Determine what surface we're on????

		SurfaceType eSurfaceType = ST_UNKNOWN;


		// Determine what container we're in...

		ContainerCode eCode;
		HLOCALOBJ objList[1];
        LTVector vTestPos = m_cs.vPos;
        uint32 dwNum = g_pLTClient->GetPointContainers(&vTestPos, objList, 1);

		if (dwNum > 0 && objList[0])
		{
            uint32 dwUserFlags;
            g_pLTClient->GetObjectUserFlags(objList[0], &dwUserFlags);

			if (dwUserFlags & USRFLG_VISIBLE)
			{
                uint16 dwCode;
                if (g_pLTClient->GetContainerCode(objList[0], &dwCode))
				{
					eCode = (ContainerCode)dwCode;
				}
			}
		}

		// Figure out what surface normal to use...

        LTVector vU, vR, vF;
        g_pLTClient->GetRotationVectors(&(m_cs.rRot), &vU, &vR, &vF);


		IFXCS cs;
		cs.eCode		= eCode;
		cs.eSurfType	= eSurfaceType;
		cs.rSurfRot		= m_cs.rRot;
		cs.vDir.Init(0, 0, 0);
		cs.vPos			= m_cs.vPos;
		cs.vSurfNormal	= vF;
		cs.fBlastRadius = m_cs.fDamageRadius;
		cs.fTintRange	= m_cs.fDamageRadius * 5.0f;
        cs.bPlaySound   = LTTRUE;

		g_pFXButeMgr->CreateImpactFX(pImpactFX, cs);
	}

    return LTFALSE;
}