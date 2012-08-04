// ----------------------------------------------------------------------- //
//
// MODULE  : LadderFX.cpp
//
// PURPOSE : 
//
// CREATED : 06/21/04
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "LadderFX.h"


extern CGameClientShell* g_pGameClientShell;



CLadderFX::CLadderFX()
{
	m_rRot.Identity();
	m_vTop.Init();
	m_vBottom.Init();
}

CLadderFX::~CLadderFX()
{
}

bool CLadderFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::Init(hServObj,pMsg))
		return false;

	m_eSurfaceOverrideType	= (SurfaceType)pMsg->Readuint8();

	g_pLTClient->GetObjectRotation(m_hServerObject,&m_rRot);

	LTVector vDims, vPos;
	g_pPhysicsLT->GetObjectDims(m_hServerObject, &vDims);
	g_pLTClient->GetObjectPos(m_hServerObject,&vPos);

	m_vTop = m_vBottom = vPos;

	m_vTop.y += vDims.y;
	m_vBottom.y -= vDims.y;



	return true;
}