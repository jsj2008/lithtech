#include "bdefs.h"
#include "optionslighting.h"

COptionsLighting::COptionsLighting() :
	m_bLightMap(false),
	m_bVertex(false),
	m_bLambertian(true),
	m_bShadows(false),
	m_nTimeSlice(50),
	m_nMaxLMSize(32),
	m_nLMTexelSize(32),
	m_nMinLMSize(4),
	m_nLightLeakDist(16)
{
}

COptionsLighting::~COptionsLighting()
{
}

// Load/Save
BOOL COptionsLighting::Load()
{
	m_bLightMap			= GetBoolValue	("LightMap",			FALSE)  ? true : false;
	m_bVertex			= GetBoolValue	("Vertex",				FALSE) ? true : false;
	m_bLambertian		= GetBoolValue	("Lambertian",			TRUE)  ? true : false;
	m_bShadows			= GetBoolValue	("Shadows",				FALSE)  ? true : false;
	m_nTimeSlice		= GetDWordValue	("TimeSlice",			30);
	m_nMaxLMSize		= GetDWordValue	("MaxLMSize",			32);
	m_nLMTexelSize		= GetDWordValue	("TexelArea",			32);
	m_nMinLMSize		= GetDWordValue ("MinLMSize",			4);
	m_nLightLeakDist	= GetDWordValue ("LightLeakDist",		16);

	return TRUE;
}

BOOL COptionsLighting::Save()
{
	SetBoolValue("LightMap",		m_bLightMap ? TRUE : FALSE);
	SetBoolValue("Vertex",			m_bVertex ? TRUE : FALSE);
	SetBoolValue("Lambertian",		m_bLambertian ? TRUE : FALSE);
	SetBoolValue("Shadows",			m_bShadows ? TRUE : FALSE);
	SetDWordValue("TimeSlice",		m_nTimeSlice);
	SetDWordValue("MaxLMSize",		m_nMaxLMSize);
	SetDWordValue("TexelArea",		m_nLMTexelSize);
	SetDWordValue("MinLMSize",		m_nMinLMSize);
	SetDWordValue("LightLeakDist",	m_nLightLeakDist);

	return TRUE;
}

