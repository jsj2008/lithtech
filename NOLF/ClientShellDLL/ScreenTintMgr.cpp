// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenTintMgr.cpp
//
// PURPOSE : Implementation of ScreenTintMgr class
//
// CREATED : 02/02/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "screentintmgr.h"
#include "GameClientShell.h"
extern CGameClientShell* g_pGameClientShell;


CScreenTintMgr::CScreenTintMgr()
{
	for (int i = 0; i < NUM_TINT_EFFECTS; i++)
	{
		m_avTints[i].Init(0.0f,0.0f,0.0f);
	}
    m_bChanged = LTFALSE;
}

CScreenTintMgr::~CScreenTintMgr()
{
}

void CScreenTintMgr::Update()
{
	if (!m_bChanged)
		return;

    LTVector vTemp;
	vTemp.Init(0.0f,0.0f,0.0f);
	for (int i = 0; i < NUM_TINT_EFFECTS; i++)
	{
		vTemp.x = Max(vTemp.x,m_avTints[i].x);
		vTemp.y = Max(vTemp.y,m_avTints[i].y);
		vTemp.z = Max(vTemp.z,m_avTints[i].z);
	}

	if (vTemp.x > 1.0f)
		vTemp.x = 1.0f;
	if (vTemp.y > 1.0f)
		vTemp.y = 1.0f;
	if (vTemp.z > 1.0f)
		vTemp.z = 1.0f;
    m_bChanged = LTFALSE;

	HLOCALOBJ hCamera = g_pGameClientShell->GetCamera();
    g_pLTClient->SetCameraLightAdd(hCamera, &vTemp);

}

void CScreenTintMgr::Set(eTintEffect eEffect, LTVector *pvColor)
{
    m_bChanged = LTTRUE;
	VEC_COPY(m_avTints[eEffect],*pvColor);
}

void CScreenTintMgr::Clear(eTintEffect eEffect)
{
    m_bChanged = LTTRUE;
	VEC_SET(m_avTints[eEffect],0.0f,0.0f,0.0f);
}

void CScreenTintMgr::ClearAll()
{
	for (int i = 0; i < NUM_TINT_EFFECTS; i++)
	{
		Clear((eTintEffect)i);
	}
}