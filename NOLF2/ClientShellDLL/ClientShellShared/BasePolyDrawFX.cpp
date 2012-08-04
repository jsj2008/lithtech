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

void BasePolyDrawFn(HLOCALOBJ hObj, void *pUser)
{
	if (!hObj) return;

	CBasePolyDrawFX* pFX = (CBasePolyDrawFX*)pUser;
	if (pFX )
	{
        pFX->DrawAll(g_pLTClient->GetDrawPrim());
	}
}

HOBJECT CBasePolyDrawFX::s_hCanvasObj = LTNULL;
PolyDrawFXList* CBasePolyDrawFX::s_pPolyDrawFXList = NULL;

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
			g_pLTClient->GetObjectPos(m_hServerObject, &vPos);
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

void CBasePolyDrawFX::DrawAll(ILTDrawPrim   *pDraw)
{
	// Draw all the fx on the static list...

	CBasePolyDrawFX** pFX = s_pPolyDrawFXList->GetItem(TLIT_FIRST);

	while (pFX && *pFX)
	{
		// Only draw fx if visible...

		if ((*pFX)->m_Flags & FLAG_VISIBLE)
		{
			(*pFX)->Draw(pDraw);
		}

		pFX = s_pPolyDrawFXList->GetItem(TLIT_NEXT);
	}
}