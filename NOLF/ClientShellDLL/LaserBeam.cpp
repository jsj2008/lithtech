 // ----------------------------------------------------------------------- //
//
// MODULE  : LaserBeam.cpp
//
// PURPOSE : LaserBeam class - Implementation
//
// CREATED : 11/16/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "LaserBeam.h"
#include "GameClientShell.h"
#include "ClientUtilities.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarLaserBeamThickness;
VarTrack	g_cvarLaserBeamAlpha;
VarTrack	g_cvarLaserBeamUOffset;
VarTrack	g_cvarLaserBeamROffset;
VarTrack	g_cvarLaserBeamFOffset;
VarTrack	g_cvarLaserBeamNumSegments;

VarTrack	g_cvarLaserBeamDebug;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeam::CLaserBeam()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CLaserBeam::CLaserBeam()
{
    m_bOn = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeam::~CLaserBeam()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CLaserBeam::~CLaserBeam()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeam::TurnOn()
//
//	PURPOSE:	Turn light on
//
// ----------------------------------------------------------------------- //

void CLaserBeam::TurnOn()
{
	Init();

    uint32 dwFlags = m_LightBeam.GetFlags();
	m_LightBeam.SetFlags(dwFlags | FLAG_VISIBLE);

    m_bOn = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeam::TurnOff()
//
//	PURPOSE:	Turn light off
//
// ----------------------------------------------------------------------- //

void CLaserBeam::TurnOff()
{
    uint32 dwFlags = m_LightBeam.GetFlags();
	m_LightBeam.SetFlags(dwFlags & ~FLAG_VISIBLE);

    m_bOn = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeam::Init()
//
//	PURPOSE:	Init stuff
//
// ----------------------------------------------------------------------- //

void CLaserBeam::Init()
{
    g_cvarLaserBeamThickness.Init(g_pLTClient, "LaserBeamThickness", NULL, 1.0f);
    g_cvarLaserBeamAlpha.Init(g_pLTClient, "LaserBeamAlpha", NULL, 0.5f);
    g_cvarLaserBeamUOffset.Init(g_pLTClient, "LaserBeamUOffset", NULL, 0.0f);
    g_cvarLaserBeamROffset.Init(g_pLTClient, "LaserBeamROffset", NULL, 0.0f);
    g_cvarLaserBeamFOffset.Init(g_pLTClient, "LaserBeamFOffset", NULL, 0.0f);
    g_cvarLaserBeamNumSegments.Init(g_pLTClient, "LaserBeamNumSegments", NULL, 2.0f);

    g_cvarLaserBeamDebug.Init(g_pLTClient, "LaserBeamDebug", NULL, 2.0f);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLaserBeam::Update()
//
//	PURPOSE:	Update the flash light
//
// ----------------------------------------------------------------------- //

void CLaserBeam::Update(LTVector vBeamStartPos, LTRotation* pRDirRot,
                        LTBOOL b3rdPerson, LTBOOL bDetect)
{
	if (!m_bOn) return;

	// Calculate beam position...

	HOBJECT hCamera = g_pGameClientShell->GetCamera();
	if (!hCamera) return;

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

    HOBJECT hFilterList[] = {hPlayerObj, g_pGameClientShell->GetMoveMgr()->GetObject(), LTNULL};

	IntersectQuery qInfo;
	IntersectInfo iInfo;

    LTVector vPos(0, 0, 0);
    LTRotation rRot;
	rRot.Init();

    LTVector vU, vR, vF;

	if (pRDirRot && b3rdPerson)
	{
		vPos = vBeamStartPos;

        g_pLTClient->GetRotationVectors(pRDirRot, &vU, &vR, &vF);
	}
	else
	{
        g_pLTClient->GetObjectRotation(hCamera, &rRot);
        g_pLTClient->GetObjectPos(hCamera, &vPos);

        g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

		if (g_cvarLaserBeamDebug.GetFloat() == 0.0f)
		{
			vBeamStartPos += vPos;
		}
		else if (g_cvarLaserBeamDebug.GetFloat() == 1.0f)
		{
			vBeamStartPos = vPos;
		}
		else
		{
            g_pLTClient->GetRotationVectors(pRDirRot, &vU, &vR, &vF);
			vBeamStartPos = vBeamStartPos;
		}

	}


    LTVector vEndPos = vPos + (vF * 10000.0f);

	qInfo.m_From = vPos;
	qInfo.m_To   = vEndPos;

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	qInfo.m_FilterFn = ObjListFilterFn;
	qInfo.m_pUserData = hFilterList;

    if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
	{
		vEndPos = iInfo.m_Point;
	}


	// Show the light beam...

    LTVector vColor = LTVector(GetRandom(235.0f, 255.0f), GetRandom(35.0f, 55.0f), GetRandom(35.0f, 55.0f));;

    LTFLOAT fAlpha = g_cvarLaserBeamAlpha.GetFloat();

	if (iInfo.m_hObject && bDetect)
	{
        uint32 dwUsrFlgs = 0;
        g_pLTClient->GetObjectUserFlags(iInfo.m_hObject, &dwUsrFlgs);

		if (dwUsrFlgs & USRFLG_CHARACTER)
		{
			fAlpha	= 0.95f;
			vColor.Init(GetRandom(35.0f, 55.0f), GetRandom(235.0f, 255.0f), GetRandom(35.0f, 55.0f));;
		}
	}

    LTFLOAT fWidth = g_cvarLaserBeamThickness.GetFloat();
	fWidth = b3rdPerson ? fWidth*2.0f : fWidth;

	vBeamStartPos += (vF * g_cvarLaserBeamFOffset.GetFloat());
	vBeamStartPos += (vR * g_cvarLaserBeamROffset.GetFloat());
	vBeamStartPos += (vU * g_cvarLaserBeamUOffset.GetFloat());

	PLFXCREATESTRUCT pls;

	if (g_cvarLaserBeamDebug.GetFloat() >= 0.0f)
	{
        // g_pLTClient->CPrint("StartPos = %.2f, %.2f, %.2f", VEC_EXPAND(vBeamStartPos));
        // g_pLTClient->CPrint("EndPos = %.2f, %.2f, %.2f", VEC_EXPAND(vEndPos));
	}

	pls.vStartPos			= vBeamStartPos;
	pls.vEndPos				= vEndPos;
	pls.vInnerColorStart	= vColor;
	pls.vInnerColorEnd		= pls.vInnerColorStart;
    pls.vOuterColorStart    = LTVector(0, 0, 0);
    pls.vOuterColorEnd      = LTVector(0, 0, 0);
	pls.fAlphaStart			= fAlpha;
	pls.fAlphaEnd			= fAlpha;
	pls.fMinWidth			= 0;
	pls.fMaxWidth			= fWidth;
	pls.fMinDistMult		= 1.0f;
	pls.fMaxDistMult		= 1.0f;
	pls.fLifeTime			= 1.0f;
	pls.fAlphaLifeTime		= 1.0f;
	pls.fPerturb			= 0.0f;
    pls.bAdditive           = LTTRUE;
    pls.bAlignFlat          = b3rdPerson ? LTFALSE : LTTRUE;
	pls.nWidthStyle			= PLWS_CONSTANT;
	pls.nNumSegments		= (int)g_cvarLaserBeamNumSegments.GetFloat();

	if (m_LightBeam.HasBeenDrawn())
	{
		// Keep the light beam in the vis list...

		m_LightBeam.SetPos(vBeamStartPos);

		// Hide the beam in portals if 1st person...Also set flag really
		// close to true...

        uint32 dwFlags2, dwFlags;

		dwFlags = m_LightBeam.GetFlags();
		dwFlags2 = m_LightBeam.GetFlags2();

		if (b3rdPerson)
		{
			dwFlags &= ~FLAG_REALLYCLOSE;
			dwFlags2 &= ~FLAG2_PORTALINVISIBLE;
		}
		else
		{
			if (g_cvarLaserBeamDebug.GetFloat() > 1.0f)
			{
				dwFlags |= FLAG_REALLYCLOSE;
                pls.bUseObjectRotation = LTTRUE;
			}
			dwFlags2 |= FLAG2_PORTALINVISIBLE;
		}

		m_LightBeam.SetFlags(dwFlags);
		m_LightBeam.SetFlags2(dwFlags2);

		m_LightBeam.ReInit(&pls);
	}
	else
	{
		m_LightBeam.Init(&pls);
        m_LightBeam.CreateObject(g_pLTClient);
	}


	m_LightBeam.Update();
}
