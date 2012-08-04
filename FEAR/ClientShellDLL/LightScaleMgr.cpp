#include "stdafx.h"
#include "iltclient.h"
#include "LightScaleMgr.h"
#include "iltrenderer.h"

//the vector that indicates that a light scale is not used
static const LTVector g_vInvalidLightScale(-1.0f, -1.0f, -1.0f);

CLightScaleMgr::CLightScaleMgr()
{
	for(uint32 nCurrType = 0; nCurrType < eNumLightScaleTypes; nCurrType++)
	{
		m_vLightScales[nCurrType] = g_vInvalidLightScale;
	}
}

CLightScaleMgr::~CLightScaleMgr()
{
}


bool CLightScaleMgr::Init()
{
	ClearAllLightScales();

    return true;
}

void CLightScaleMgr::Term()
{
	ClearAllLightScales();
}

void CLightScaleMgr::SetLightScale (const LTVector& vColor, ELightScaleType eType)
{
	assert((eType < eNumLightScaleTypes) && "Invalid light scale type");

	//setup this scale
	m_vLightScales[eType] = vColor;

	// set the correct global light scale
	SetLightScale();
}

void CLightScaleMgr::ClearLightScale (ELightScaleType eType)
{
	assert((eType < eNumLightScaleTypes) && "Invalid light scale type");
	m_vLightScales[eType] = g_vInvalidLightScale;
	
	SetLightScale();
}

void CLightScaleMgr::SetLightScale() const
{
	//look for effects in the order in which they are declared
	LTVector vTintColor(1.0f, 1.0f, 1.0f);
	
	for(uint32 nCurrType = 0; nCurrType < eNumLightScaleTypes; nCurrType++)
	{
		if(IsEnabled(static_cast<ELightScaleType>(nCurrType)))
		{
			vTintColor = m_vLightScales[nCurrType];
			break;
		}
	}

    //g_pLTClient->GetRenderer()->SetGlobalLightScale(vTintColor);
}


void CLightScaleMgr::ClearAllLightScales ()
{
	for(uint32 nCurrType = 0; nCurrType < eNumLightScaleTypes; nCurrType++)
	{
		m_vLightScales[nCurrType] = g_vInvalidLightScale;
	}

	SetLightScale();
}

bool CLightScaleMgr::IsEnabled(ELightScaleType eType) const
{
	return !m_vLightScales[eType].NearlyEquals(g_vInvalidLightScale, 0.01f);
}

void CLightScaleMgr::Enable() const
{
	//Just install our light scale
	SetLightScale();
}

void CLightScaleMgr::Disable() const
{
	//reset our light scale to off
	LTVector vTintColor(1.0f, 1.0f, 1.0f);
	//g_pLTClient->GetRenderer()->SetGlobalLightScale(vTintColor);
}