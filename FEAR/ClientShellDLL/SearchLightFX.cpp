// ----------------------------------------------------------------------- //
//
// MODULE  : SearchLightFX.cpp
//
// PURPOSE : SearchLight FX - Implementation
//
// CREATED : 6/8/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "SearchLightFX.h"
#include "GameClientShell.h"
#include "ClientWeaponUtils.h"
#include "ClientDB.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::Init
//
//	PURPOSE:	Init the search light
//
// ----------------------------------------------------------------------- //

bool CSearchLightFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::Init(hServObj, pMsg)) return false;
	if (!pMsg) return false;

	SEARCHLIGHTCREATESTRUCT sl;

	sl.hServerObj			= hServObj;
	sl.fBeamLength			= pMsg->Readfloat();
	sl.fBeamRadius			= pMsg->Readfloat();
	sl.fBeamAlpha			= pMsg->Readfloat();
	sl.fBeamRotTime			= pMsg->Readfloat();
	sl.fLightRadius			= pMsg->Readfloat();
	sl.bBeamAdditive		= !!pMsg->Readuint8();
	sl.vLightColor			= pMsg->ReadLTVector();

	// Should be between 0.0 and 1.0f...

	sl.vLightColor /= 255.0f;

	return Init(&sl);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::Init
//
//	PURPOSE:	Init the search light fx
//
// ----------------------------------------------------------------------- //

bool CSearchLightFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return false;

	m_cs = *((SEARCHLIGHTCREATESTRUCT*)psfxCreateStruct);

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

bool CSearchLightFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return false;

	ObjectCreateStruct createStruct;

	// Create the beam model...

	g_pLTClient->GetObjectPos(m_hServerObject, &(createStruct.m_Pos));
	createStruct.m_ObjectType = OT_MODEL;

	ClientDB& ClientDatabase = ClientDB::Instance();
	LTStrCpy(createStruct.m_Filename, ClientDatabase.GetString(ClientDatabase.GetClientSharedRecord(), CDB_SearchBeam), LTARRAYSIZE(createStruct.m_Filename) );

    if (!createStruct.m_Filename[0]) return false;

	LTStrCpy(createStruct.m_Materials[0], ClientDatabase.GetString(ClientDatabase.GetClientSharedRecord(), CDB_SearchMaterial0), LTARRAYSIZE(createStruct.m_Materials[0]) );
	LTStrCpy(createStruct.m_Materials[1], ClientDatabase.GetString(ClientDatabase.GetClientSharedRecord(), CDB_SearchMaterial1), LTARRAYSIZE(createStruct.m_Materials[1]) );


	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
	createStruct.m_Flags2 = FLAG2_FORCETRANSLUCENT;

	m_hBeam = m_pClientDE->CreateObject(&createStruct);
    if (!m_hBeam) return false;

    float r, g, b, a;
	m_pClientDE->GetObjectColor(m_hBeam, &r, &g, &b, &a);
	r = g = b = 1.0f;
	m_pClientDE->SetObjectColor(m_hBeam, r, g, b, m_cs.fBeamAlpha);


	// Create the dynamic light...

	if (m_cs.fLightRadius > 1.0f)
	{
		createStruct.m_ObjectType = OT_LIGHT;
		createStruct.m_Flags = FLAG_VISIBLE;

		m_hLight = m_pClientDE->CreateObject(&createStruct);
        if (!m_hLight) return false;

		m_pClientDE->SetObjectColor(m_hLight, m_cs.vLightColor.x, m_cs.vLightColor.y, m_cs.vLightColor.z, 1.0f);
		m_pClientDE->SetLightRadius(m_hLight, m_cs.fLightRadius);
	}

    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::Update
//
//	PURPOSE:	Update the lens flare fx
//
// ----------------------------------------------------------------------- //

bool CSearchLightFX::Update()
{
    if (!m_pClientDE || !m_hServerObject || m_bWantRemove || !m_hBeam) return false;

	// Hide/show the fx if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		g_pCommonLT->GetObjectFlags(m_hServerObject, OFT_User, dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))  // Hide fx
		{
			if (m_hBeam)
			{
				g_pCommonLT->SetObjectFlags(m_hBeam, OFT_Flags, 0, FLAG_VISIBLE);
			}
			if (m_hLight)
			{
				g_pCommonLT->SetObjectFlags(m_hLight, OFT_Flags, 0, FLAG_VISIBLE);
			}

            return true;
		}
		else  // Make all fx visible
		{
			if (m_hBeam)
			{
				g_pCommonLT->SetObjectFlags(m_hBeam, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			}
			if (m_hLight)
			{
				g_pCommonLT->SetObjectFlags(m_hLight, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
			}
		}
	}


	// Update the position/rotation of the beam...

    LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

    LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);


	// See how long to make the beam...

    LTVector vDest = vPos + (rRot.Forward() * m_cs.fBeamLength);

	IntersectInfo iInfo;
	IntersectQuery qInfo;
	qInfo.m_Flags		= INTERSECT_OBJECTS | IGNORE_NONSOLID;
	qInfo.m_FilterFn	= AttackerLiquidFilterFn;
	qInfo.m_pUserData	= m_hServerObject;
	qInfo.m_From		= vPos;
	qInfo.m_To			= vDest;

    if (g_pLTClient->IntersectSegment(qInfo, &iInfo))
	{
		vDest = iInfo.m_Point;
	}

    LTVector vDir = vDest - vPos;
    float fDistance = vDir.Mag();
	vDir.Normalize();

    LTVector vNewPos = vPos + vDir * fDistance/2.0f;
	rRot = LTRotation(vDir, LTVector(0.0f, 1.0f, 0.0f));

	if (m_cs.fBeamRotTime > 0.0f)
	{
		m_fBeamRotation += (360.0f/m_cs.fBeamRotTime * SimulationTimer::Instance().GetTimerElapsedS( ));
		m_fBeamRotation = m_fBeamRotation > 360.0f ? m_fBeamRotation - 360.0f : m_fBeamRotation;
		rRot.Rotate(vDir, DEG2RAD(m_fBeamRotation));
	}

	g_pLTClient->SetObjectTransform(m_hBeam, LTRigidTransform(vNewPos, rRot));

	//NOTE: This object needs to be rewritten as a custom render object so that it will work with non-uniform
	//scaling, or use a ClientFX object that performs this functionality
	m_pClientDE->SetObjectScale(m_hBeam, m_cs.fBeamRadius);


	// Move the dynamic light...

	if (m_hLight)
	{
		vDest -= (vDir * 5.0f);
		g_pLTClient->SetObjectPos(m_hLight, vDest);
	}

    return true;
}