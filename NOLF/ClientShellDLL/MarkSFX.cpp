// ----------------------------------------------------------------------- //
//
// MODULE  : MarkSFX.cpp
//
// PURPOSE : Mark special FX - Implementation
//
// CREATED : 10/13/97
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MarkSFX.h"
#include "iltclient.h"
#include "ltlink.h"
#include "GameClientShell.h"
#include "SurfaceFunctions.h"
#include "VarTrack.h"

extern CGameClientShell* g_pGameClientShell;

#define REGION_DIAMETER			100.0f  // Squared distance actually
#define MAX_MARKS_IN_REGION		10

VarTrack	g_cvarClipMarks;
VarTrack	g_cvarLightMarks;
VarTrack	g_cvarShowMarks;
VarTrack	g_cvarMarkFadeTime;
VarTrack	g_cvarMarkSolidTime;


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::Init
//
//	PURPOSE:	Create the mark
//
// ----------------------------------------------------------------------- //

LTBOOL CMarkSFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
    if (!psfxCreateStruct) return LTFALSE;

	CSpecialFX::Init(psfxCreateStruct);

	MARKCREATESTRUCT* pMark = (MARKCREATESTRUCT*)psfxCreateStruct;

    m_Rotation = pMark->m_Rotation;
	VEC_COPY(m_vPos, pMark->m_vPos);
	m_fScale		= pMark->m_fScale;
	m_nAmmoId		= pMark->nAmmoId;
	m_nSurfaceType	= pMark->nSurfaceType;

	if (!g_cvarClipMarks.IsInitted())
	{
        g_cvarClipMarks.Init(g_pLTClient, "MarksClip", NULL, 0.0f);
	}

	if (!g_cvarLightMarks.IsInitted())
	{
        g_cvarLightMarks.Init(g_pLTClient, "MarkLight", NULL, 0.0f);
	}

	if (!g_cvarShowMarks.IsInitted())
	{
        g_cvarShowMarks.Init(g_pLTClient, "MarkShow", NULL, 1.0f);
	}

	if (!g_cvarMarkFadeTime.IsInitted())
	{
        g_cvarMarkFadeTime.Init(g_pLTClient, "MarkFadeTime", NULL, 3.0f);
	}

	if (!g_cvarMarkSolidTime.IsInitted())
	{
        g_cvarMarkSolidTime.Init(g_pLTClient, "MarkSolidTime", NULL, 3.0f);
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::CreateObject
//
//	PURPOSE:	Create object associated with the mark
//
// ----------------------------------------------------------------------- //

LTBOOL CMarkSFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE) || !g_pGameClientShell) return LTFALSE;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTFALSE;

	// If we're not showing marks, don't bother...

	if (!g_cvarShowMarks.GetFloat()) return LTFALSE;


	// Before we create a new buillet hole see if there is already another
	// bullet hole close by that we could use instead...

	CSpecialFXList* pList = psfxMgr->GetFXList(SFX_MARK_ID);
    if (!pList) return LTFALSE;

	int nNumBulletHoles = pList->GetSize();

    HOBJECT hMoveObj         = LTNULL;
    HOBJECT hObj             = LTNULL;
    LTFLOAT  fClosestMarkDist = REGION_DIAMETER;
    uint8   nNumInRegion     = 0;

    LTVector vPos;

	for (int i=0; i < nNumBulletHoles; i++)
	{
		if ((*pList)[i])
		{
			hObj = (*pList)[i]->GetObject();
			if (hObj)
			{
				pClientDE->GetObjectPos(hObj, &vPos);

                LTFLOAT fDist = VEC_DISTSQR(vPos, m_vPos);
				if (fDist < REGION_DIAMETER)
				{
					if (fDist < fClosestMarkDist)
					{
						fClosestMarkDist = fDist;
						hMoveObj = hObj;
					}

					if (++nNumInRegion > MAX_MARKS_IN_REGION)
					{
						// Just move this bullet-hole to the correct pos, and
						// remove thyself...

						pClientDE->SetObjectPos(hMoveObj, &m_vPos);
                        return LTFALSE;
					}
				}
			}
		}
	}


	// Setup the mark...

	ObjectCreateStruct createStruct;
	INIT_OBJECTCREATESTRUCT(createStruct);

    LTFLOAT fScaleAdjust = 1.0f;
	if (!GetImpactSprite((SurfaceType)m_nSurfaceType, fScaleAdjust, m_nAmmoId,
		createStruct.m_Filename, ARRAY_LEN(createStruct.m_Filename)))
	{
        return LTFALSE;
	}

	createStruct.m_ObjectType = OT_SPRITE;
	createStruct.m_Flags = FLAG_VISIBLE | FLAG_ROTATEABLESPRITE;

	// Should probably force this in low detail modes...

	if (g_cvarLightMarks.GetFloat() == 0.0f)
	{
		createStruct.m_Flags |= FLAG_NOLIGHT;
	}

	VEC_COPY(createStruct.m_Pos, m_vPos);
    createStruct.m_Rotation = m_Rotation;

	m_hObject = pClientDE->CreateObject(&createStruct);

	m_fScale *= fScaleAdjust;

    LTVector vScale;
	VEC_SET(vScale, m_fScale, m_fScale, m_fScale);
	m_pClientDE->SetObjectScale(m_hObject, &vScale);

	if (g_cvarClipMarks.GetFloat() > 0)
	{
		// Clip the mark to th poly...

		IntersectQuery qInfo;
		IntersectInfo iInfo;

        LTVector vU, vR, vF;
        g_pLTClient->GetRotationVectors(&m_Rotation, &vU, &vR, &vF);

		qInfo.m_From = m_vPos + (vF * 2.0);
		qInfo.m_To   = m_vPos - (vF * 2.0);

		qInfo.m_Flags = IGNORE_NONSOLID | INTERSECT_HPOLY;

        if (g_pLTClient->IntersectSegment(&qInfo, &iInfo))
		{
            g_pLTClient->ClipSprite(m_hObject, iInfo.m_hPoly);
		}
	}

    LTFLOAT r, g, b, a;
	r = g = b = 0.5f;
	a = 1.0f;
	m_pClientDE->SetObjectColor(m_hObject, r, g, b, a);

	m_fStartTime = m_pClientDE->GetTime();

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::WantRemove
//
//	PURPOSE:	If this gets called, remove the mark (this should only get
//				called if we have a server object associated with us, and
//				that server object gets removed).
//
// ----------------------------------------------------------------------- //

void CMarkSFX::WantRemove(LTBOOL bRemove)
{
	CSpecialFX::WantRemove(bRemove);

	if (!g_pGameClientShell) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	// Tell the special fx mgr to go ahead and remove us...

	if (m_hObject)
	{
		psfxMgr->RemoveSpecialFX(m_hObject);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMarkSFX::Update
//
//	PURPOSE:	Always return TRUE, however check to see if we should
//				hide/show the mark.
//
// ----------------------------------------------------------------------- //

LTBOOL CMarkSFX::Update()
{
	if (!g_cvarShowMarks.GetFloat())
	{
		// Remove the object...
		return LTFALSE;
	}

	LTFLOAT fTime = g_pLTClient->GetTime();

	LTFLOAT fFadeStartTime = m_fStartTime + g_cvarMarkSolidTime.GetFloat();
    LTFLOAT fFadeEndTime = fFadeStartTime + g_cvarMarkFadeTime.GetFloat();
	if (fTime > fFadeEndTime)
	{
		// Remove the object...
		return LTFALSE;
	}
	else if (fTime > fFadeStartTime)
	{
		LTFLOAT fScale = ((fFadeEndTime - fTime) / g_cvarMarkFadeTime.GetFloat());
		LTFLOAT r, g, b, a;
	
		g_pLTClient->GetObjectColor(m_hObject, &r, &g, &b, &a);
		a = a < fScale ? a : fScale;
		g_pLTClient->SetObjectColor(m_hObject, r, g, b, a);
	}

	return LTTRUE;
}