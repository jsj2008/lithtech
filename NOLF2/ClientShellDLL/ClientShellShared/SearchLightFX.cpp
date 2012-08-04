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
#include "ClientButeMgr.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::Init
//
//	PURPOSE:	Init the search light
//
// ----------------------------------------------------------------------- //

LTBOOL CSearchLightFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
    if (!CSpecialFX::Init(hServObj, pMsg)) return LTFALSE;
    if (!pMsg) return LTFALSE;

	SEARCHLIGHTCREATESTRUCT sl;

	sl.hServerObj			= hServObj;
    sl.fBeamLength          = pMsg->Readfloat();
    sl.fBeamRadius          = pMsg->Readfloat();
    sl.fBeamAlpha           = pMsg->Readfloat();
    sl.fBeamRotTime         = pMsg->Readfloat();
    sl.fLightRadius         = pMsg->Readfloat();
    sl.bBeamAdditive        = (LTBOOL) pMsg->Readuint8();
    sl.vLightColor			= pMsg->ReadLTVector();

	sl.lens.InitFromMessage(sl.lens, pMsg);

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

LTBOOL CSearchLightFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((SEARCHLIGHTCREATESTRUCT*)psfxCreateStruct);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CSearchLightFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	// Create the beam model...

	g_pLTClient->GetObjectPos(m_hServerObject, &(createStruct.m_Pos));
	createStruct.m_ObjectType = OT_MODEL;

	g_pClientButeMgr->GetSpecialFXAttributeString("SearchBeam",createStruct.m_Filename,sizeof(createStruct.m_Filename));
    if (!createStruct.m_Filename[0]) return LTFALSE;

	g_pClientButeMgr->GetSpecialFXAttributeString("SearchSkin0",createStruct.m_SkinNames[0],sizeof(createStruct.m_SkinNames[0]));
	g_pClientButeMgr->GetSpecialFXAttributeString("SearchSkin1",createStruct.m_SkinNames[1],sizeof(createStruct.m_SkinNames[1]));
	
	g_pClientButeMgr->GetSpecialFXAttributeString("SearchRenderStyle0",createStruct.m_RenderStyleNames[0],sizeof(createStruct.m_RenderStyleNames[0]));


	createStruct.m_Flags = FLAG_VISIBLE | FLAG_NOLIGHT;
	createStruct.m_Flags2 = FLAG2_FORCETRANSLUCENT;

	m_hBeam = m_pClientDE->CreateObject(&createStruct);
    if (!m_hBeam) return LTFALSE;

    LTFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hBeam, &r, &g, &b, &a);
	r = g = b = 1.0f;
	m_pClientDE->SetObjectColor(m_hBeam, r, g, b, m_cs.fBeamAlpha);


	// Create the dynamic light...

	if (m_cs.fLightRadius > 1.0f)
	{
		createStruct.m_ObjectType = OT_LIGHT;
		createStruct.m_Flags = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;

		m_hLight = m_pClientDE->CreateObject(&createStruct);
        if (!m_hLight) return LTFALSE;

		m_pClientDE->SetLightColor(m_hLight, m_cs.vLightColor.x, m_cs.vLightColor.y, m_cs.vLightColor.z);
		m_pClientDE->SetLightRadius(m_hLight, m_cs.fLightRadius);
	}


	// Create the lens flare...

	LENSFLARECREATESTRUCT lens = m_cs.lens;

	lens.hServerObj			= m_hServerObject;
    lens.bInSkyBox          = LTFALSE;
    lens.bCreateSprite      = LTTRUE;
    lens.bSpriteOnly        = LTTRUE;
    lens.bUseObjectAngle    = LTTRUE;

	if (!m_LensFlare.Init(&lens) || !m_LensFlare.CreateObject(m_pClientDE))
	{
        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CSearchLightFX::Update
//
//	PURPOSE:	Update the lens flare fx
//
// ----------------------------------------------------------------------- //

LTBOOL CSearchLightFX::Update()
{
    if (!m_pClientDE || !m_hServerObject || m_bWantRemove || !m_hBeam) return LTFALSE;

	// Update the lens flare...

	m_LensFlare.Update();

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

            return LTTRUE;
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

    if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
	{
		vDest = iInfo.m_Point;
	}

    LTVector vDir = vDest - vPos;
    LTFLOAT fDistance = vDir.Length();
	vDir.Normalize();

    LTVector vNewPos = vPos + vDir * fDistance/2.0f;
	rRot = LTRotation(vDir, LTVector(0.0f, 1.0f, 0.0f));

	if (m_cs.fBeamRotTime > 0.0f)
	{
		m_fBeamRotation += (360.0f/m_cs.fBeamRotTime * g_pGameClientShell->GetFrameTime());
		m_fBeamRotation = m_fBeamRotation > 360.0f ? m_fBeamRotation - 360.0f : m_fBeamRotation;
		rRot.Rotate(vDir, DEG2RAD(m_fBeamRotation));
	}

	g_pLTClient->SetObjectRotation(m_hBeam, &rRot);
	g_pLTClient->SetObjectPos(m_hBeam, &vNewPos);


    LTVector vScale(m_cs.fBeamRadius, m_cs.fBeamRadius, fDistance);
	m_pClientDE->SetObjectScale(m_hBeam, &vScale);


	// Move the dynamic light...

	if (m_hLight)
	{
		vDest -= (vDir * 5.0f);
		g_pLTClient->SetObjectPos(m_hLight, &vDest);
	}

    return LTTRUE;
}