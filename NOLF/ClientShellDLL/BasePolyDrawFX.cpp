// ----------------------------------------------------------------------- //
//
// MODULE  : BasePolyDrawFX.cpp
//
// PURPOSE : BasePolyDraw (Canvas) special FX - Implementation
//
// CREATED : 4/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BasePolyDrawFX.h"

void BasePolyDrawFn(ILTCustomDraw *pDraw, HLOCALOBJ hObj, void *pUser)
{
	if (!hObj) return;

	CBasePolyDrawFX* pFX = (CBasePolyDrawFX*)pUser;
	if (pFX )
	{
		pFX->DrawAll(pDraw);
	}
}

LTBOOL CBasePolyDrawFX::s_bListInited  = LTFALSE;
HOBJECT CBasePolyDrawFX::s_hCanvasObj = LTNULL;
PolyDrawFXList CBasePolyDrawFX::s_PolyDrawFXList;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBasePolyDrawFX::CreateObject
//
//	PURPOSE:	Create object associated the object
//
// ----------------------------------------------------------------------- //

LTBOOL CBasePolyDrawFX::CreateObject(ILTClient *pClientDE)
{
    if (!CSpecialFX::CreateObject(pClientDE)) return LTFALSE;

    LTVector vPos = m_vPos;

	if (vPos.x == 0.0f && vPos.y == 0.0f && vPos.z == 0.0f)
	{
		if (m_hServerObject)
		{
			pClientDE->GetObjectPos(m_hServerObject, &vPos);
		}
	}


	// Setup the global canvas object...

	if (!s_hCanvasObj)
	{
		ObjectCreateStruct createStruct;
		INIT_OBJECTCREATESTRUCT(createStruct);

		createStruct.m_ObjectType = OT_CANVAS;
        createStruct.m_Flags = FLAG_VISIBLE | FLAG_FOGDISABLE;
		createStruct.m_Pos = vPos;

		s_hCanvasObj = m_pClientDE->CreateObject(&createStruct);
        if (!s_hCanvasObj) return LTFALSE;

		m_pClientDE->SetCanvasFn(s_hCanvasObj, BasePolyDrawFn, this);
		m_pClientDE->SetCanvasRadius(s_hCanvasObj, 10000.0f);
	}

	m_Flags = FLAG_VISIBLE;

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CBasePolyDrawFX::DrawAll
//
//	PURPOSE:	Draw all the BasePolyDrawFX objects...
//
// ----------------------------------------------------------------------- //

void CBasePolyDrawFX::DrawAll(ILTCustomDraw *pDraw)
{
	// Draw all the fx on the static list...

	CBasePolyDrawFX** pFX = s_PolyDrawFXList.GetItem(TLIT_FIRST);

	while (pFX && *pFX)
	{
		// Only draw fx if visible...

		if ((*pFX)->m_Flags & FLAG_VISIBLE)
		{
			// Check for special flags that need to be set on the static
			// canvas object for this fx...

            uint32 dwFlags2;
            g_pLTClient->Common()->GetObjectFlags(s_hCanvasObj, OFT_Flags2, dwFlags2);

			if ((*pFX)->m_Flags2 & FLAG2_PORTALINVISIBLE)
			{
                g_pLTClient->Common()->SetObjectFlags(s_hCanvasObj, OFT_Flags2,
					dwFlags2 | FLAG2_PORTALINVISIBLE);
			}

			(*pFX)->Draw(pDraw);

			// Reset old flags...

            g_pLTClient->Common()->SetObjectFlags(s_hCanvasObj, OFT_Flags2, dwFlags2);
		}

		pFX = s_PolyDrawFXList.GetItem(TLIT_NEXT);
	}
}