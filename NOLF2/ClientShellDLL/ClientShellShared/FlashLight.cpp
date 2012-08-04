 // ----------------------------------------------------------------------- //
//
// MODULE  : FlashLight.cpp
//
// PURPOSE : FlashLight class - Implementation
//
// CREATED : 07/21/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
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
#include "CMoveMgr.h"

VarTrack	g_cvarFLMinLightRadius;
VarTrack	g_cvarFLMaxLightRadius;
VarTrack	g_cvarFLMinLightColor;
VarTrack	g_cvarFLMaxLightColor;
VarTrack	g_cvarFLLightOffsetUp;
VarTrack	g_cvarFLLightOffsetRight;
VarTrack	g_cvarFLLightOffsetForward;

static bool NonSolidFilterFn(HOBJECT hTest, void *pUserData)
{
	if (ObjListFilterFn(hTest, pUserData))
	{
		// Ignore non-solid objects (even if ray-hit is true)...

		uint32 dwFlags;
		g_pCommonLT->GetObjectFlags(hTest, OFT_Flags, dwFlags);

		if (!(dwFlags & FLAG_SOLID))
		{
			return false;
		}
	}
    return true;
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
    m_bOn	 = LTFALSE;
    m_hLight = LTNULL;
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
        g_pLTClient->RemoveObject(m_hLight);
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

	if( m_hLight && !m_bOn )
	{
        m_bOn = LTTRUE;
		g_pCommonLT->SetObjectFlags(m_hLight, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);

		if (UpdateServer())
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
			cMsg.Writeuint8(CP_FLASHLIGHT);
			cMsg.Writeuint8(FL_ON);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
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
	if( m_hLight && m_bOn )
	{
        m_bOn = LTFALSE;
		g_pCommonLT->SetObjectFlags(m_hLight, OFT_Flags, 0, FLAG_VISIBLE);

		if (UpdateServer())
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_CLIENTMSG);
			cMsg.Writeuint8(CP_FLASHLIGHT);
			cMsg.Writeuint8(FL_OFF);
			g_pLTClient->SendToServer(cMsg.Read(), MESSAGE_GUARANTEED);
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

    g_cvarFLMinLightRadius.Init(g_pLTClient, "FLMinRadius", NULL, 75.0f);
    g_cvarFLMaxLightRadius.Init(g_pLTClient, "FLMaxtRadius", NULL, 150.0f);
    g_cvarFLMinLightColor.Init(g_pLTClient, "FLMinColor", NULL, 100.0f);
    g_cvarFLMaxLightColor.Init(g_pLTClient, "FLMaxColor", NULL, 105.0f);
    g_cvarFLLightOffsetUp.Init(g_pLTClient, "FLOffsetUp", NULL, 5.0f);
    g_cvarFLLightOffsetRight.Init(g_pLTClient, "FLOffsetRight", NULL, 0.0f);
    g_cvarFLLightOffsetForward.Init(g_pLTClient, "FLOffsetForward", NULL, 1.0f);

	HOBJECT hCamera = g_pPlayerMgr->GetCamera();
	if (!hCamera) return;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	createStruct.m_ObjectType = OT_LIGHT;
	createStruct.m_Flags	= FLAG_VISIBLE;

	//we want to make sure that this dynamically lights the world regardless of
	//performance settings
	createStruct.m_Flags2	= FLAG2_FORCEDYNAMICLIGHTWORLD;

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

    HOBJECT hFilterList[] = {hPlayerObj, g_pPlayerMgr->GetMoveMgr()->GetObject(), LTNULL};

	IntersectQuery qInfo;
	IntersectInfo iInfo;

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

	g_pLTClient->SetObjectPos(m_hLight, &vEndPos);

    LTVector vDir = vEndPos - vPos;
    LTFLOAT fDist = vDir.Length();
	vDir *= 1.0f / fDist;

    LTFLOAT fLightRadius = g_cvarFLMinLightRadius.GetFloat() +
		((g_cvarFLMaxLightRadius.GetFloat() - g_cvarFLMinLightRadius.GetFloat()) * fDist / g_cvarFLLightOffsetForward.GetFloat());

    g_pLTClient->SetLightRadius(m_hLight, fLightRadius);

    LTVector vColor; 
    vColor.y = vColor.z = vColor.x = GetRandom(g_cvarFLMinLightColor.GetFloat(), g_cvarFLMaxLightColor.GetFloat());

    LTVector vLightColor = vColor / 255.0f;

    g_pLTClient->SetLightColor(m_hLight, vLightColor.x, vLightColor.y, vLightColor.z);
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

	CMoveMgr* pMoveMgr = g_pPlayerMgr->GetMoveMgr();
	if (!pMoveMgr) return;

	LTRotation rRot;

	if (pMoveMgr->GetVehicleMgr()->IsVehiclePhysics())
	{
		if (g_pPlayerMgr->IsFirstPerson())
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
	else if (g_pPlayerMgr->IsFirstPerson())
	{
		HOBJECT hCamera = g_pPlayerMgr->GetCamera();
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
			// g_pLTClient->GetObjectRotation(hPlayerObj, &rRot);
			// g_pLTClient->GetObjectPos(hPlayerObj, &vStartPos);

			HMODELSOCKET hSocket;
			if ( LT_OK == g_pModelLT->GetSocket(hPlayerObj, "LeftHand", hSocket) )
			{
				LTransform tf;

				if ( LT_OK == g_pModelLT->GetSocketTransform(hPlayerObj, hSocket, tf, LTTRUE) )
				{
					vStartPos = tf.m_Pos;
					rRot = tf.m_Rot;
				}
			}
		}
	}

	vEndPos = vStartPos + (rRot.Forward() * g_cvarFLLightOffsetForward.GetFloat());

  	if (g_pPlayerMgr->IsFirstPerson())
  	{
  		vROffset = (rRot.Right() * g_cvarFLLightOffsetRight.GetFloat());
  		vUOffset = (rRot.Up() * g_cvarFLLightOffsetUp.GetFloat());

		// Update the Start/End position to addjust for any offset...

		vEndPos += vROffset;
		vEndPos += vUOffset;

		vStartPos += vROffset;
		vStartPos += vUOffset;
  	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight3rdPerson::CFlashLight3rdPerson()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFlashLight3rdPerson::CFlashLight3rdPerson()
{
	m_hObj = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight3rdPerson::~CFlashLight3rdPerson()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CFlashLight3rdPerson::~CFlashLight3rdPerson()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight3rdPerson::Init()
//
//	PURPOSE:	Initializes the flashlight
//
// ----------------------------------------------------------------------- //

void CFlashLight3rdPerson::Init(HOBJECT hObj)
{
	m_hObj = hObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFlashLight3rdPerson::GetLightPositions()
//
//	PURPOSE:	Get the flash light position and rotation...
//
// ----------------------------------------------------------------------- //

void CFlashLight3rdPerson::GetLightPositions(LTVector& vStartPos, LTVector& vEndPos, LTVector& vUOffset, LTVector& vROffset)
{
	if ( !m_hObj ) return;

	HMODELSOCKET hSocket;
	if ( LT_OK == g_pModelLT->GetSocket(m_hObj, "LeftHand", hSocket) )
	{
		LTransform tf;

		if ( LT_OK == g_pModelLT->GetSocketTransform(m_hObj, hSocket, tf, LTTRUE) )
		{
			LTVector vPos = tf.m_Pos;
			LTRotation rRot = tf.m_Rot;

			LTVector vRight, vUp, vForward;
			vRight = rRot.Right();
			vUp = rRot.Up();
			vForward = rRot.Forward();

			//vStartPos = vPos - vUp*4.0f + vForward*8.0f;
			//vEndPos = vPos + vForward*200.0f;
			//vUOffset = vUp;
			//vROffset = vRight;

			vStartPos = vPos;
			vEndPos = vPos + (vForward * g_cvarFLLightOffsetForward.GetFloat());

  			vROffset = (vRight * g_cvarFLLightOffsetRight.GetFloat());
  			vUOffset = (vUp * g_cvarFLLightOffsetUp.GetFloat());

			// Update the Start/End position to addjust for any offset...

			vEndPos += vROffset;
			vEndPos += vUOffset;

			vStartPos += vROffset;
			vStartPos += vUOffset;
		}
	}
}