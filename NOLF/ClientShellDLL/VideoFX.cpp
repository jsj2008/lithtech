// ----------------------------------------------------------------------- //
//
// MODULE  : VideoFX.cpp
//
// PURPOSE : VideoFX special FX - Implementation
//
// CREATED : 5/06/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "VideoFX.h"
#include "RandomSparksFX.h"
#include "GameClientShell.h"
#include "iltcustomdraw.h"

extern CGameClientShell* g_pGameClientShell;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVideoFX::Init
//
//	PURPOSE:	Init the lightning fx
//
// ----------------------------------------------------------------------- //

LTBOOL CVideoFX::Init(HLOCALOBJ hServObj, HMESSAGEREAD hMessage)
{
    if (!CSpecialFX::Init(hServObj, hMessage)) return LTFALSE;
    if (!hMessage) return LTFALSE;

	// Read in the init info from the message...

	VIDEOCREATESTRUCT cs;
	cs.hServerObj = hServObj;
	SAFE_STRCPY(cs.szVideo, g_pLTClient->GetStringData(g_pLTClient->ReadFromMessageHString(hMessage)));

	return Init(&cs);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVideoFX::Init
//
//	PURPOSE:	Init the Video fx
//
// ----------------------------------------------------------------------- //

static HVIDEO s_hVideo;

LTBOOL CVideoFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!CSpecialFX::Init(psfxCreateStruct)) return LTFALSE;

	m_cs = *((VIDEOCREATESTRUCT*)psfxCreateStruct);

	LTRotation rRot;
	g_pLTClient->GetObjectRotation(m_hServerObject, &rRot);

	LTVector vNull, vForward;
	g_pMathLT->GetRotationVectors(rRot, vNull, vNull, vForward);

	LTVector vPos;
	g_pLTClient->GetObjectPos(m_hServerObject, &vPos);

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vPos;
	IQuery.m_To   = IQuery.m_From + (vForward * 1000.0f);

	IQuery.m_Flags = INTERSECT_HPOLY | IGNORE_NONSOLID;

	if (g_pLTClient->IntersectSegment(&IQuery, &IInfo))
	{
		if (IInfo.m_hPoly != INVALID_HPOLY)
		{
			if ( LT_OK == g_pLTClient->VideoMgr()->StartTextureVideo(m_cs.szVideo, 0, s_hVideo) )
			{
				if ( LT_OK == g_pLTClient->VideoMgr()->BindTextureVideoToPoly(s_hVideo, IInfo.m_hPoly) )
				{

				}
			}
		}
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVideoFX::CreateObject
//
//	PURPOSE:	Create the fx
//
// ----------------------------------------------------------------------- //

LTBOOL CVideoFX::CreateObject(ILTClient* pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !g_pGameClientShell) return LTFALSE;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTFALSE;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CVideoFX::Update
//
//	PURPOSE:	Update the Video Fx
//
// ----------------------------------------------------------------------- //

LTBOOL CVideoFX::Update()
{
	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr || !m_pClientDE || !m_hServerObject) return LTFALSE;

    LTFLOAT fTime = m_pClientDE->GetTime();

	// Check to see if we should go away...

	if (m_bWantRemove)
	{
        return LTFALSE;
	}

	LTRESULT hResult = g_pLTClient->VideoMgr()->GetVideoStatus(s_hVideo);
	LTBOOL bOk = hResult == LT_OK;
	LTBOOL bFinished = hResult == LT_FINISHED;

	// Hide/show the fire if necessary...

	if (m_hServerObject)
	{
        uint32 dwUserFlags;
		m_pClientDE->GetObjectUserFlags(m_hServerObject, &dwUserFlags);

		if (!(dwUserFlags & USRFLG_VISIBLE))
		{
            return LTTRUE;
		}
		else
		{
		}
	}

    return LTTRUE;
}