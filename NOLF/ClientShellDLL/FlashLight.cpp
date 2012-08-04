 // ----------------------------------------------------------------------- //
//
// MODULE  : FlashLight.cpp
//
// PURPOSE : FlashLight class - Implementation
//
// CREATED : 07/21/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FlashLight.h"
#include "GameClientShell.h"
#include "ClientUtilities.h"
#include "VarTrack.h"
#include "BaseScaleFX.h"
#include "MsgIDs.h"
#include "VehicleMgr.h"

extern CGameClientShell* g_pGameClientShell;

VarTrack	g_cvarFLMinLightRadius;
VarTrack	g_cvarFLMaxLightRadius;
VarTrack	g_cvarFLMaxLightDist;
VarTrack	g_cvarFLBeamMinRadius;
VarTrack	g_cvarFLBeamRadius;
VarTrack	g_cvarFLBeamAlpha;
VarTrack	g_cvarFLBeamUOffset;
VarTrack	g_cvarFLBeamROffset;
VarTrack	g_cvarFLPolyBeam;
VarTrack	g_cvarFLNumSegments;
VarTrack	g_cvarFLMinBeamLen;
VarTrack	g_cvarFLServerUpdateTime;

static LTBOOL NonSolidFilterFn(HOBJECT hTest, void *pUserData)
{
	if (ObjListFilterFn(hTest, pUserData))
	{
		// Ignore non-solid objects (even if ray-hit is true)...

		uint32 dwFlags = g_pLTClient->GetObjectFlags(hTest);

		if (!(dwFlags & FLAG_SOLID))
		{
			return LTFALSE;
		}
	}
    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::CFlashLight()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFlashLight::CFlashLight()
{
    m_bOn               = LTFALSE;

    m_hLight            = LTNULL;

	m_fMinLightRadius	= 75.0f;
	m_fMaxLightRadius	= 500.0f;
	m_fMaxLightDist		= 10000.0f;

    m_fServerUpdateTimer = (LTFLOAT)INT_MAX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::~CFlashLight()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CFlashLight::~CFlashLight()
{
	if (m_hLight)
	{
        g_pLTClient->DeleteObject(m_hLight);
        m_hLight = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::TurnOn()
//
//	PURPOSE:	Turn light on
//
// ----------------------------------------------------------------------- //

void CFlashLight::TurnOn()
{
	CreateLight();

	if (m_hLight)
	{
        m_bOn = LTTRUE;
        uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hLight);
        g_pLTClient->SetObjectFlags(m_hLight, dwFlags | FLAG_VISIBLE);

		dwFlags = m_LightBeam.GetFlags();
		m_LightBeam.SetFlags(dwFlags | FLAG_VISIBLE);

		if ( UpdateServer() )
		{
			HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
			g_pLTClient->WriteToMessageByte(hMessage, CP_FLASHLIGHT);
			g_pLTClient->WriteToMessageByte(hMessage, FL_ON);
			g_pLTClient->EndMessage(hMessage);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::TurnOff()
//
//	PURPOSE:	Turn light off
//
// ----------------------------------------------------------------------- //

void CFlashLight::TurnOff()
{
	if (m_hLight)
	{
        m_bOn = LTFALSE;
        uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hLight);
        g_pLTClient->SetObjectFlags(m_hLight, dwFlags & ~FLAG_VISIBLE);

		dwFlags = m_LightBeam.GetFlags();
		m_LightBeam.SetFlags(dwFlags & ~FLAG_VISIBLE);

		if ( UpdateServer() )
		{
			HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
			g_pLTClient->WriteToMessageByte(hMessage, CP_FLASHLIGHT);
			g_pLTClient->WriteToMessageByte(hMessage, FL_OFF);
			g_pLTClient->EndMessage(hMessage);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::CreateLight()
//
//	PURPOSE:	Create the dynamic light
//
// ----------------------------------------------------------------------- //

void CFlashLight::CreateLight()
{
	if (m_hLight) return;

    g_cvarFLMinLightRadius.Init(g_pLTClient, "FLMinRadius", NULL, m_fMinLightRadius);
    g_cvarFLMaxLightRadius.Init(g_pLTClient, "FLMaxRadius", NULL, m_fMaxLightRadius);
    g_cvarFLMaxLightDist.Init(g_pLTClient, "FLMaxDist", NULL, m_fMaxLightDist);
    g_cvarFLBeamMinRadius.Init(g_pLTClient, "FLBeamMinRadius", NULL, 10.0f);
    g_cvarFLBeamRadius.Init(g_pLTClient, "FLBeamRadius", NULL, 150.0f);
    g_cvarFLBeamAlpha.Init(g_pLTClient, "FLBeamAlpha", NULL, 0.2f);
    g_cvarFLBeamUOffset.Init(g_pLTClient, "FLBeamUOffset", NULL, -10.0f);
    g_cvarFLBeamROffset.Init(g_pLTClient, "FLBeamROffset", NULL, 0.0f);
    g_cvarFLNumSegments.Init(g_pLTClient, "FLNumSegments", NULL, 1.0f);
    g_cvarFLPolyBeam.Init(g_pLTClient, "FLPolyBeam", NULL, 1.0f);
    g_cvarFLMinBeamLen.Init(g_pLTClient, "FLMinBeamLen", NULL, 0.0f);
    g_cvarFLServerUpdateTime.Init(g_pLTClient, "FLServerUpdateTime", LTNULL, 0.20f);


	HOBJECT hCamera = g_pGameClientShell->GetCamera();
	if (!hCamera) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_LIGHT;
	createStruct.m_Flags = FLAG_VISIBLE;
    g_pLTClient->GetObjectPos(hCamera, &(createStruct.m_Pos));

    m_hLight = g_pLTClient->CreateObject(&createStruct);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight::Update()
//
//	PURPOSE:	Update the flash light
//
// ----------------------------------------------------------------------- //

void CFlashLight::Update()
{
	if (!m_bOn || !m_hLight) return;

	// Calculate light position...

    HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
	if (!hPlayerObj) return;

    HOBJECT hFilterList[] = {hPlayerObj, g_pGameClientShell->GetMoveMgr()->GetObject(), LTNULL};

	IntersectQuery qInfo;
	IntersectInfo iInfo;

    LTRotation rRot;
	LTVector vPos, vEndPos, vUOffset, vROffset;

	GetLightPositions(vPos, vEndPos, vUOffset, vROffset);

	qInfo.m_From = vPos;
	qInfo.m_To   = vEndPos;

	qInfo.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID;
	qInfo.m_FilterFn = NonSolidFilterFn;
	qInfo.m_pUserData = hFilterList;

    if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
	{
		vEndPos = iInfo.m_Point;
	}

    m_fServerUpdateTimer += g_pGameClientShell->GetFrameTime();
	if (m_fServerUpdateTimer > g_cvarFLServerUpdateTime.GetFloat())
	{
		m_fServerUpdateTimer = 0.0f;

		if ( UpdateServer() )
		{
			HMESSAGEWRITE hMessage = g_pLTClient->StartMessage(MID_PLAYER_CLIENTMSG);
			g_pLTClient->WriteToMessageByte(hMessage, CP_FLASHLIGHT);
			g_pLTClient->WriteToMessageByte(hMessage, FL_UPDATE);
			g_pLTClient->WriteToMessageCompVector(hMessage, &vEndPos);
			g_pLTClient->EndMessage(hMessage);
		}
	}

    g_pLTClient->SetObjectPos(m_hLight, &vEndPos);

    LTVector vDir = vEndPos - vPos;
    LTFLOAT fDist = vDir.Mag();
	vDir.Norm();

    LTFLOAT fLightRadius = g_cvarFLMinLightRadius.GetFloat() +
		((g_cvarFLMaxLightRadius.GetFloat() - g_cvarFLMinLightRadius.GetFloat()) * fDist / g_cvarFLMaxLightDist.GetFloat());

    g_pLTClient->SetLightRadius(m_hLight, fLightRadius);

    LTVector vColor = LTVector(GetRandom(235.0f, 255.0f), GetRandom(235.0f, 255.0f), GetRandom(200.0f, 235.0f));;
    LTVector vLightColor = vColor / 255.0f;

    g_pLTClient->SetLightColor(m_hLight, vLightColor.x, vLightColor.y, vLightColor.z);


	// Show the light beam...

	if (g_cvarFLPolyBeam.GetFloat() > 0)
	{
		PLFXCREATESTRUCT pls;

		vPos += vUOffset;
		vPos += vROffset;

		pls.pTexture			= "sfx\\test\\fxtest42.dtx";
		pls.dwTexAddr			= LTTEXADDR_CLAMP;
		pls.vStartPos			= vPos;
		pls.vEndPos				= vEndPos;
		pls.vInnerColorStart	= vColor;
		pls.vInnerColorEnd		= vColor;
        pls.vOuterColorStart    = vColor;
        pls.vOuterColorEnd      = vColor;
		pls.fAlphaStart			= g_cvarFLBeamAlpha.GetFloat();
		pls.fAlphaEnd			= g_cvarFLBeamAlpha.GetFloat();
		pls.fMinWidth			= g_cvarFLBeamMinRadius.GetFloat();
		pls.fMaxWidth			= g_cvarFLBeamRadius.GetFloat();
		pls.fMinDistMult		= 1.0f;
		pls.fMaxDistMult		= 1.0f;
		pls.fLifeTime			= 1.0f;
		pls.fAlphaLifeTime		= 1.0f;
		pls.fPerturb			= 0.0f;
        pls.bAdditive           = LTTRUE; // LTFALSE;
        pls.bNoZ                = LTTRUE;
		pls.bAlignFlat			= LTTRUE;
		pls.nWidthStyle			= PLWS_CONSTANT;
		pls.nNumSegments		= (int)g_cvarFLNumSegments.GetFloat();

        LTBOOL bUpdateBeam = LTTRUE;

		if (m_LightBeam.HasBeenDrawn())
		{
			// Keep the light beam in the vis list...

			m_LightBeam.SetPos(vPos);

			// Hide the beam if it is too short...

            uint32 dwFlags = m_LightBeam.GetFlags();

			if (fDist < g_cvarFLMinBeamLen.GetFloat())
			{
				// Fade alpha out as beam gets shorter to help hide
				// the poly line...

				pls.fAlphaStart *= fDist / g_cvarFLMinBeamLen.GetFloat();

				if (pls.fAlphaStart < 0.01f)
				{
					dwFlags &= ~FLAG_VISIBLE;
					bUpdateBeam = LTFALSE;
				}
			}
			else
			{
				dwFlags |= FLAG_VISIBLE;
			}

			m_LightBeam.SetFlags(dwFlags);
			m_LightBeam.ReInit(&pls);
		}
		else
		{
			m_LightBeam.Init(&pls);
            m_LightBeam.CreateObject(g_pLTClient);
		}

		if (bUpdateBeam)
		{
			m_LightBeam.Update();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLightPlayer::GetLightPositions()
//
//	PURPOSE:	Get the flash light position and rotation...
//
// ----------------------------------------------------------------------- //

void CFlashLightPlayer::GetLightPositions(LTVector& vStartPos, LTVector& vEndPos, LTVector& vUOffset, LTVector& vROffset)
{
	vStartPos.Init();
	vEndPos.Init();
	vUOffset.Init();
	vROffset.Init();

	CMoveMgr* pMoveMgr = g_pGameClientShell->GetMoveMgr();
	if (!pMoveMgr) return;

	LTRotation rRot;

	if (pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
	{
		if (g_pGameClientShell->IsFirstPerson())
		{
			pMoveMgr->GetVehicleMgr()->GetVehicleLightPosRot(vStartPos, rRot);
		}
		else // 3rd person vehicle
		{
			// Get light pos on 3rd-person vehicle...

			HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
			if (hPlayerObj)
			{
				g_pLTClient->GetObjectRotation(hPlayerObj, &rRot);
				g_pLTClient->GetObjectPos(hPlayerObj, &vStartPos);
			}
		}
	}
	else if (g_pGameClientShell->IsFirstPerson())
	{
		HOBJECT hCamera = g_pGameClientShell->GetCamera();
		if (!hCamera) return;

		g_pLTClient->GetObjectRotation(hCamera, &rRot);
		g_pLTClient->GetObjectPos(hCamera, &vStartPos);
	}
	else // 3rd person
	{
		// Get light pos from 3rd-person model...

		HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (hPlayerObj)
		{
			g_pLTClient->GetObjectRotation(hPlayerObj, &rRot);
			g_pLTClient->GetObjectPos(hPlayerObj, &vStartPos);
		}
	}

	LTVector vU, vR, vF;
    g_pLTClient->GetRotationVectors(&rRot, &vU, &vR, &vF);

	vEndPos = vStartPos + (vF * g_cvarFLMaxLightDist.GetFloat());

	if (g_pGameClientShell->IsFirstPerson())
	{
		vROffset = (vR * g_cvarFLBeamROffset.GetFloat());
		vUOffset = (vU * g_cvarFLBeamUOffset.GetFloat());
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLightAI::CFlashLightAI()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFlashLightAI::CFlashLightAI()
{
	m_hAI = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLightAI::~CFlashLightAI()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CFlashLightAI::~CFlashLightAI()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLightAI::Init()
//
//	PURPOSE:	Initializes the flashlight
//
// ----------------------------------------------------------------------- //

void CFlashLightAI::Init(HOBJECT hAI)
{
	m_hAI = hAI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLightAI::Update()
//
//	PURPOSE:	Update the flash light
//
// ----------------------------------------------------------------------- //

void CFlashLightAI::Update()
{
	if ( !m_hAI ) return;

	uint32 dwUsrFlags = 0;
	if ( LT_OK == g_pLTClient->GetObjectUserFlags(m_hAI, &dwUsrFlags) && (dwUsrFlags & USRFLG_AI_FLASHLIGHT) )
	{
		TurnOn();
	}
	else
	{
		TurnOff();
	}

	CFlashLight::Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLightAI::GetLightPositions()
//
//	PURPOSE:	Get the flash light position and rotation...
//
// ----------------------------------------------------------------------- //

void CFlashLightAI::GetLightPositions(LTVector& vStartPos, LTVector& vEndPos, LTVector& vUOffset, LTVector& vROffset)
{
	if ( !m_hAI ) return;

	HMODELSOCKET hSocket;
	if ( LT_OK == g_pModelLT->GetSocket(m_hAI, "LeftHand", hSocket) )
	{
		LTransform tf;

		if ( LT_OK == g_pModelLT->GetSocketTransform(m_hAI, hSocket, tf, LTTRUE) )
		{
			LTVector vPos;
			LTRotation rRot;

			if ( LT_OK == g_pTransLT->Get(tf, vPos, rRot) )
			{
				LTVector vRight, vUp, vForward;
				if ( LT_OK == g_pMathLT->GetRotationVectors(rRot, vRight, vUp, vForward) )
				{
					vStartPos = vPos - vUp*4.0f + vForward*8.0f;
					vEndPos = vPos + vForward*200.0f;
					vUOffset = vUp;
					vROffset = vRight;
				}
			}
		}
	}
}