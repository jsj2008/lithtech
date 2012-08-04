#include "bdefs.h"
#include "optionsmodels.h"

COptionsModels::COptionsModels() :
	m_bRunLowPriority(true),
	m_bLimitMemoryUse(false),
	m_nMaxMemoryUse(10),
	m_bRenderBoxAtDist(false),
	m_nRenderBoxDist(1000),
	m_nPerspectiveMode(VIEWMODEL_TEXTURED),
	m_nOrthoMode(VIEWMODEL_WIREFRAME),
	m_bAlwaysShowModels(true)
{
}

COptionsModels::~COptionsModels()
{
}

// Load/Save
BOOL COptionsModels::Load()
{
	m_bRunLowPriority	= GetBoolValue	("RunLowPriority",		TRUE) ? true : false;
	m_bLimitMemoryUse	= GetBoolValue	("LimitMemoryUse",		FALSE)	? true : false;
	m_nMaxMemoryUse		= GetDWordValue	("MaxMemoryUse",		10);
	m_bRenderBoxAtDist	= GetBoolValue	("RenderBoxAtDist",		FALSE) ? true : false;
	m_nRenderBoxDist	= GetDWordValue	("RenderBoxDist",		1000);
	m_nPerspectiveMode	= GetDWordValue	("PerspectiveViewMode", VIEWMODEL_TEXTURED);
	m_nOrthoMode		= GetDWordValue	("OrthoViewMode",		VIEWMODEL_WIREFRAME);
	m_bAlwaysShowModels = GetBoolValue  ("AlwaysShowModels",	TRUE) ? true : false;

	return TRUE;
}

BOOL COptionsModels::Save()
{
	SetBoolValue("RunLowPriority",			m_bRunLowPriority ? TRUE : FALSE);
	SetBoolValue("LimitMemoryUse",			m_bLimitMemoryUse ? TRUE : FALSE);
	SetDWordValue("MaxMemoryUse",			m_nMaxMemoryUse);
	SetBoolValue("RenderBoxAtDist",			m_bRenderBoxAtDist ? TRUE : FALSE);
	SetDWordValue("RenderBoxDist",			m_nRenderBoxDist);
	SetDWordValue("PerspectiveViewMode",	m_nPerspectiveMode);
	SetDWordValue("OrthoViewMode",			m_nOrthoMode);
	SetBoolValue("AlwaysShowModels",		m_bAlwaysShowModels ? TRUE : FALSE);

	return TRUE;
}

