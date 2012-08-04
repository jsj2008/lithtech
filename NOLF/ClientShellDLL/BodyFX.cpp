// ----------------------------------------------------------------------- //
//
// MODULE  : BodyFX.cpp
//
// PURPOSE : Body special FX - Implementation
//
// CREATED : 8/24/98
//
// (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BodyFX.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"
#include "ClientUtilities.h"
#include "SoundMgr.h"
#include "BaseScaleFX.h"
#include "SurfaceFunctions.h"

extern CGameClientShell* g_pGameClientShell;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::CBodyFX()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CBodyFX::CBodyFX()
{
	m_fFaderTime = 3.0f;
	m_fFaderTimer = 3.0f;

	m_hMarker = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::~CBodyFX()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CBodyFX::~CBodyFX()
{
	RemoveMarker();

	if ( m_hServerObject )
	{
		g_pModelLT->RemoveTracker(m_hServerObject, &m_TwitchTracker);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::Init
//
//	PURPOSE:	Init the Body fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBodyFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	BODYCREATESTRUCT bcs;

	bcs.hServerObj = hServObj;
    bcs.Read(g_pLTClient, hMessage);

	return Init(&bcs);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::Init
//
//	PURPOSE:	Init the Body fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBodyFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_bs = *((BODYCREATESTRUCT*)psfxCreateStruct);

	g_pModelLT->AddTracker(m_bs.hServerObj, &m_TwitchTracker);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::CreateObject
//
//	PURPOSE:	Create the various fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBodyFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !m_hServerObject) return LTFALSE;

    uint32 dwCFlags = m_pClientDE->GetObjectClientFlags(m_hServerObject);
	m_pClientDE->SetObjectClientFlags(m_hServerObject, dwCFlags | CF_NOTIFYMODELKEYS | CF_INSIDERADIUS);

//  uint32 dwCFlags = m_pClientDE->GetObjectClientFlags(m_hServerObject);
//	m_pClientDE->SetObjectClientFlags(m_hServerObject, dwCFlags | CF_NOTIFYMODELKEYS);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::Update
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CBodyFX::Update()
{
    if (!m_pClientDE || !m_hServerObject || m_bWantRemove) return LTFALSE;

	switch ( m_bs.eBodyState )
	{
		case eBodyStateFade:
			UpdateFade();
			break;
	}

    if (g_pGameClientShell->GetGameType() != SINGLE && m_bs.nClientId != (uint8)-1)
	{
		UpdateMarker();
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::UpdateFade
//
//	PURPOSE:	Update the fx
//
// ----------------------------------------------------------------------- //

void CBodyFX::UpdateFade()
{
	HLOCALOBJ attachList[20];
    uint32 dwListSize = 0;
    uint32 dwNumAttach = 0;

    g_pLTClient->GetAttachments(m_hServerObject, attachList, 20, &dwListSize, &dwNumAttach);
	int nNum = dwNumAttach <= dwListSize ? dwNumAttach : dwListSize;

	m_fFaderTime = Max<LTFLOAT>(0.0f, m_fFaderTime - g_pGameClientShell->GetFrameTime());

	g_pLTClient->SetObjectColor(m_hServerObject, 1, 1, 1, m_fFaderTime/m_fFaderTimer);

	for (int i=0; i < nNum; i++)
	{
		g_pLTClient->SetObjectColor(attachList[i], 1, 1, 1, m_fFaderTime/m_fFaderTimer);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::OnServerMessage
//
//	PURPOSE:	Handle any messages from our server object...
//
// ----------------------------------------------------------------------- //

LTBOOL CBodyFX::OnServerMessage(HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::OnServerMessage(hMessage)) return LTFALSE;

    uint8 nMsgId = g_pLTClient->ReadFromMessageByte(hMessage);

	switch(nMsgId)
	{
		case BFX_FADE_MSG:
		{
			m_bs.eBodyState = eBodyStateFade;
		}
		break;

	}

    return LTTRUE;
}

LTBOOL GroundFilterFn(HOBJECT hObj, void *pUserData)
{
	return ( IsMainWorld(hObj) || (OT_WORLDMODEL == g_pLTClient->GetObjectType(hObj)) );
}

void CBodyFX::OnModelKey(HLOCALOBJ hObj, ArgList *pArgs)
{
	if (!m_hServerObject || !hObj || !pArgs || !pArgs->argv || pArgs->argc == 0) return;

	char* pKey = pArgs->argv[0];
	if (!pKey) return;

	LTBOOL bSlump = !_stricmp(pKey, "NOISE");
	LTBOOL bLand = !_stricmp(pKey, "LAND");

	if ( bSlump || bLand )
	{
		LTVector vPos;
        g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		IQuery.m_From = vPos;
		IQuery.m_To = vPos - LTVector(0,96,0);
		IQuery.m_Flags = INTERSECT_OBJECTS | INTERSECT_HPOLY | IGNORE_NONSOLID;
		IQuery.m_FilterFn = GroundFilterFn;

		SurfaceType eSurface;

        if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
		{
			if (IInfo.m_hPoly && IInfo.m_hPoly != INVALID_HPOLY)
			{
				eSurface = (SurfaceType)GetSurfaceType(IInfo.m_hPoly);
			}
			else if (IInfo.m_hObject) // Get the texture flags from the object...
			{
				eSurface = (SurfaceType)GetSurfaceType(IInfo.m_hObject);
			}
			else
			{
				return;
			}
		}
		else
		{
			return;
		}

		// Play the noise

		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurface);
		_ASSERT(pSurf);
		if (!pSurf) return;

		if (bSlump && pSurf->szBodyFallSnd[0])
		{
			g_pClientSoundMgr->PlaySoundFromPos(vPos, pSurf->szBodyFallSnd, pSurf->fBodyFallSndRadius, SOUNDPRIORITY_MISC_LOW);
		}
		else if (bLand && pSurf->szBodyLedgeFallSnd[0])
		{
			g_pClientSoundMgr->PlaySoundFromPos(vPos, pSurf->szBodyLedgeFallSnd, pSurf->fBodyLedgeFallSndRadius, SOUNDPRIORITY_MISC_LOW);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::UpdateMarker
//
//	PURPOSE:	Update the marker fx
//
// ----------------------------------------------------------------------- //

void CBodyFX::UpdateMarker()
{
	if (!m_pClientDE || !m_hServerObject) return;

	CClientInfoMgr *pClientMgr = g_pInterfaceMgr->GetClientInfoMgr();
	if (!pClientMgr) return;

	CLIENT_INFO* pLocalInfo = pClientMgr->GetLocalClient();
	CLIENT_INFO* pInfo = pClientMgr->GetClientByID(m_bs.nClientId);
	if (!pInfo ||  !pLocalInfo) return;
	LTBOOL bSame = (pInfo->team == pLocalInfo->team);

	if (bSame)
	{
		if (m_hMarker)
			RemoveMarker();
		return;
	}

    uint32 dwFlags = g_pLTClient->GetObjectFlags(m_hServerObject);
	if (!(dwFlags & FLAG_VISIBLE))
	{
		RemoveMarker();
		return;
	}


    LTVector vU, vR, vF, vTemp, vDims, vPos;
    LTRotation rRot;

    ILTPhysics* pPhysics = m_pClientDE->Physics();

	m_pClientDE->GetObjectPos(m_hServerObject, &vPos);
	pPhysics->GetObjectDims(m_hServerObject, &vDims);
	vPos.y += (vDims.y + 20.0f);

	if (!m_hMarker)
	{
		CreateMarker(vPos,bSame);
	}

	if (m_hMarker)
	{
		m_pClientDE->SetObjectPos(m_hMarker, &vPos);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::CreateMarker
//
//	PURPOSE:	Create marker special fx
//
// ----------------------------------------------------------------------- //

void CBodyFX::CreateMarker(LTVector & vPos, LTBOOL bSame)
{
	if (!m_pClientDE || !g_pGameClientShell || !m_hServerObject) return;

	if (!bSame) return;


	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

	CString str = g_pClientButeMgr->GetSpecialFXAttributeString("TeamSprite");
	LTFLOAT fScaleMult = g_pClientButeMgr->GetSpecialFXAttributeFloat("TeamScale");
	LTFLOAT fAlpha = g_pClientButeMgr->GetSpecialFXAttributeFloat("TeamAlpha");

	LTVector	vScale(1.0f,1.0f,1.0f);

	VEC_MULSCALAR(vScale, vScale, fScaleMult);

	SAFE_STRCPY(createStruct.m_Filename, (char *)(LPCSTR)str);
	createStruct.m_ObjectType	= OT_SPRITE;
	createStruct.m_Flags		= FLAG_VISIBLE | FLAG_FOGDISABLE | FLAG_NOLIGHT;
	createStruct.m_Pos			= vPos;

	m_hMarker = m_pClientDE->CreateObject(&createStruct);
	if (!m_hMarker) return;

	m_pClientDE->SetObjectScale(m_hMarker, &vScale);

    LTFLOAT r, g, b, a;
	m_pClientDE->GetObjectColor(m_hServerObject, &r, &g, &b, &a);

	m_pClientDE->SetObjectColor(m_hObject, r, g, b, (fAlpha * a));

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBodyFX::RemoveMarker
//
//	PURPOSE:	Remove the marker fx
//
// ----------------------------------------------------------------------- //

void CBodyFX::RemoveMarker()
{
	if (!m_hMarker) return;

	g_pLTClient->DeleteObject(m_hMarker);
	m_hMarker = LTNULL;

}