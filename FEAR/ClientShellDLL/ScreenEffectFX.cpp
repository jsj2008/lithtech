// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenEffectFX.cpp
//
// PURPOSE : PolyGrid special FX - Implementation
//
// CREATED : 10/13/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "ScreenEffectFX.h"
#include "FXDefs.h"
#include "ScreenEffectDB.h"

#include "ILTRenderer.h"
extern ILTRenderer *g_pLTRenderer;

CScreenEffectFX::CScreenEffectFX() : CSpecialFX(),
	m_hMaterial(INVALID_MATERIAL)
{
	DATABASE_CATEGORY(ScreenEffect).Init();
	m_hRecord = DATABASE_CATEGORY(ScreenEffect).GetRecordByName("Default");
	m_hParameters = DATABASE_CATEGORY(ScreenEffect).GetAttribute(m_hRecord, "Parameter");
}

CScreenEffectFX::~CScreenEffectFX()
{
	if (m_hMaterial != INVALID_MATERIAL && g_pLTRenderer)
		g_pLTRenderer->ReleaseMaterialInstance(m_hMaterial);
}

bool CScreenEffectFX::Init(HLOCALOBJ hServObj, ILTMessage_Read* pMsg)
{
	if (!CSpecialFX::Init(hServObj, pMsg))
		return false;

	// Note: the message is actually empty...
	
	// Create a local ClientFX object (the FxEdit kind)
	CLIENTFX_CREATESTRUCT cCFXCS(GetOverlayFXName(), FXFLAG_LOOP, hServObj);
	if (!g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(&m_linkClientFX, cCFXCS))
	{
		g_pLTClient->CPrint("ScreenEffect: Failed to create overlay effect %s", GetOverlayFXName());
		return false;
	}
	
	// Hide the effect so it's controlled by the server
	m_linkClientFX.GetInstance()->Hide();
	
	// Get a handle to the overlay material
	HMATERIAL hMaterial = g_pLTRenderer->CreateMaterialInstance(GetOverlayMaterialName());
	if (hMaterial == INVALID_MATERIAL)
	{
		g_pLTClient->CPrint("ScreenEffect: Failed to open material file %s", GetOverlayMaterialName());
		return false;
	}
	m_hMaterial = hMaterial;
	
	// Read out the current values
	m_aParameterValues.resize(GetNumParameters());
	for (uint32 nLoop = 0; nLoop < m_aParameterValues.size(); ++nLoop)
	{
		m_aParameterValues[nLoop] = GetParameterValue(nLoop);
	}
	
	return true;
}

bool CScreenEffectFX::Update()
{
	double fCurTime = SimulationTimer::Instance().GetTimerAccumulatedS();
	for (uint32 nCurRampInfo = 0; nCurRampInfo < m_aRampInfo.size(); ++nCurRampInfo)
	{
		SRampInfo& sRampInfo = m_aRampInfo[nCurRampInfo];
		float fValue = 0.0f;
		
		g_pLTRenderer->SetInstanceParamFloat(m_hMaterial, GetParameterName(sRampInfo.m_nParameter), sRampInfo.GetCurValue((float)fCurTime));
		
		if (sRampInfo.IsFinished((float)fCurTime))
		{
			if (nCurRampInfo != (m_aRampInfo.size() - 1))
				std::swap(m_aRampInfo[nCurRampInfo], m_aRampInfo[m_aRampInfo.size() - 1]);
			m_aRampInfo.pop_back();
			--nCurRampInfo;
		}
	}
	
	return true;
}

bool CScreenEffectFX::OnServerMessage(ILTMessage_Read* pMsg)
{
	uint32 nParameter = pMsg->Readuint8();
	float fValue = pMsg->Readfloat();
	float fRampTime = pMsg->Readfloat();
	
	double fCurTime = SimulationTimer::Instance().GetTimerAccumulatedS();
	float fOldValue = GetParameterValue(nParameter);
	
	SRampInfo* pRampInfo = NULL;
	
	for (uint32 nCurRampInfo = 0; nCurRampInfo < m_aRampInfo.size(); ++nCurRampInfo)
	{
		SRampInfo& sRampInfo = m_aRampInfo[nCurRampInfo];
		// If we're already ramping this parameter, start from the current ramp value
		if (nParameter == sRampInfo.m_nParameter)
		{
			pRampInfo = &sRampInfo;
			fOldValue = (float)sRampInfo.GetCurValue((float)fCurTime);
			break;
		}
	}
	
	// Add a new entry
	if (pRampInfo == NULL)
	{
		m_aRampInfo.push_back(SRampInfo());
		pRampInfo = &m_aRampInfo.back();
	}
	pRampInfo->m_nParameter = nParameter;
	pRampInfo->m_vTimes = LTVector2((float)fCurTime, (float)fCurTime + fRampTime);
	pRampInfo->m_vValues = LTVector2(fOldValue, fValue);
	
	return true;
}

const char* CScreenEffectFX::GetOverlayFXName()
{
	const char* pResult = DATABASE_CATEGORY(ScreenEffect).GetFXName(m_hRecord);
	return (pResult != NULL) ? pResult : "<none>";
}

const char* CScreenEffectFX::GetOverlayMaterialName()
{
	return DATABASE_CATEGORY(ScreenEffect).GetMaterial(m_hRecord);
}

const char* CScreenEffectFX::GetParameterName(uint32 nParameter)
{
	return DATABASE_CATEGORY(ScreenEffect).GetParameters(m_hRecord, nParameter);
}

float CScreenEffectFX::GetParameterValue(uint32 nParameter)
{
	float fResult;
	if (g_pLTRenderer->GetInstanceParamFloat(m_hMaterial, GetParameterName(nParameter), fResult) == LT_OK)
		return fResult;
	else
		return 0.0f;
}

uint32 CScreenEffectFX::GetNumParameters()
{
	return DATABASE_CATEGORY(ScreenEffect).GetNumValues(m_hParameters);
}


float CScreenEffectFX::SRampInfo::GetCurValue(float fTime)
{
	if (IsFinished(fTime))
		return m_vValues.y;

	float fInterpolant = (fTime - m_vTimes.x) / (m_vTimes.y - m_vTimes.x);
	return LTLERP(m_vValues.x, m_vValues.y, fInterpolant);
}

bool CScreenEffectFX::SRampInfo::IsFinished(float fTime)
{
	return (fTime >= m_vTimes.y) || (m_vTimes.x == m_vTimes.y);
}
