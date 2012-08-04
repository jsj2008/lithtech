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
#include "PlayerCamera.h"


CScreenTintMgr::CScreenTintMgr()
{
	for (int i = 0; i < NUM_TINT_EFFECTS; i++)
	{
		m_avTints[i].Init(0.0f,0.0f,0.0f);
	}
    m_bChanged = false;
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
		vTemp.x = LTMAX(vTemp.x,m_avTints[i].x);
		vTemp.y = LTMAX(vTemp.y,m_avTints[i].y);
		vTemp.z = LTMAX(vTemp.z,m_avTints[i].z);
	}

	if (vTemp.x > 1.0f)
		vTemp.x = 1.0f;
	if (vTemp.y > 1.0f)
		vTemp.y = 1.0f;
	if (vTemp.z > 1.0f)
		vTemp.z = 1.0f;
    m_bChanged = false;

    //g_pLTClient->SetCameraLightAdd( g_pPlayerMgr->GetPlayerCamera()->GetCamera(), &vTemp);

}

void CScreenTintMgr::Set(eTintEffect eEffect, LTVector *pvColor)
{
    m_bChanged = true;
	m_avTints[eEffect] = *pvColor;
}

void CScreenTintMgr::Clear(eTintEffect eEffect)
{
    m_bChanged = true;
	m_avTints[eEffect] = LTVector(0.0f,0.0f,0.0f);
}

void CScreenTintMgr::ClearAll()
{
	for (int i = 0; i < NUM_TINT_EFFECTS; i++)
	{
		Clear((eTintEffect)i);
	}
}