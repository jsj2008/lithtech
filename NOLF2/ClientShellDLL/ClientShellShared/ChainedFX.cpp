//
// MODULE  : ChainedFX.cpp
//
// PURPOSE : Base class for chaining ClientFX
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "ChainedFX.h"
#include "GameClientShell.h"


void CChainedFX::Init(char* szIntroName, char* szShortIntroName, char* szLoopName)
{

	SAFE_STRCPY(m_szIntroName,szIntroName);
	SAFE_STRCPY(m_szShortIntroName,szShortIntroName);
	SAFE_STRCPY(m_szLoopName,szLoopName);

	m_iFromScreen = m_iToScreen = -1;
	m_bAllowLooping = true;
}

void CChainedFX::Start(bool bUseShortIntro)
{
	INT_FX* pFX = LTNULL;
	m_bIntroDone = false;

	if (bUseShortIntro)
	{
		if (strlen(m_szShortIntroName))
			pFX = g_pLayoutMgr->GetFX(m_szShortIntroName);
	}
	else if (strlen(m_szIntroName))
		pFX = g_pLayoutMgr->GetFX(m_szIntroName);

	if (pFX)
	{
		g_pInterfaceMgr->AddInterfaceFX(&m_IntroFX, pFX->szFXName,pFX->vPos,false);
	}
	else
	{
		m_IntroFX.ClearLink();
		m_bIntroDone = true;
	}

	m_pLoopFXStruct = g_pLayoutMgr->GetFX(m_szLoopName);
//	if (m_pLoopFXStruct && !m_IntroFX.m_pInstance)
//		g_pInterfaceMgr->AddInterfaceFX(NULL, m_pLoopFXStruct->szFXName,m_pLoopFXStruct->vPos,true);
}


void CChainedFX::Update()
{
	if (m_IntroFX.IsValid() && m_IntroFX.GetInstance()->IsDone())
	{
//		g_pLTClient->CPrint("intro fx done");
		g_pInterfaceMgr->RemoveInterfaceFX(&m_IntroFX);
		m_IntroFX.ClearLink();
		m_bIntroDone = true;
	}

	if (m_pLoopFXStruct && (!m_IntroFX.IsValid() || m_IntroFX.GetInstance()->IsFinished()))
	{
		if (m_bAllowLooping)
		{
			g_pInterfaceMgr->RemoveInterfaceFX(&m_IntroFX);
			m_IntroFX.ClearLink();
//			g_pLTClient->CPrint("intro fx finished, starting loop");
			g_pInterfaceMgr->AddInterfaceFX(&m_LoopFX, m_pLoopFXStruct->szFXName,m_pLoopFXStruct->vPos,true);
			m_pLoopFXStruct = NULL;
		}
		m_bIntroDone = true;
	}
}

void CChainedFX::End()
{
	if (m_IntroFX.IsValid())
	{
		g_pInterfaceMgr->RemoveInterfaceFX(&m_IntroFX);
		m_IntroFX.ClearLink();
	}

	if (m_LoopFX.IsValid())
	{
		g_pInterfaceMgr->RemoveInterfaceFX(&m_LoopFX);
		m_LoopFX.ClearLink();
	}
	m_bIntroDone = true;
}
