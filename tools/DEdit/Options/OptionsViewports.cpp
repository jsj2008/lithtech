#include "bdefs.h"
#include "optionsviewports.h"

COptionsViewports::COptionsViewports() 
{
}

COptionsViewports::~COptionsViewports()
{
}

// Load/Save
BOOL COptionsViewports::Load()
{
	for(uint32 nView = 0; nView < NUM_VIEWPORTS; nView++)
	{
		CString sBase;
		sBase.Format("View%d", nView);

		m_bShowGrid[nView]			= !!GetBoolValue(sBase + "ShowGrid", TRUE);
		m_bShowWireframe[nView]		= !!GetBoolValue(sBase + "ShowWireframe", FALSE);
		m_bShowNormals[nView]		= !!GetBoolValue(sBase + "ShowNormals", FALSE);
		m_bSelectBackface[nView]	= !!GetBoolValue(sBase + "SelectBackface", FALSE);
		m_bShowObjects[nView]		= !!GetBoolValue(sBase + "ShowObjects", TRUE);
		m_bShowMarker[nView]		= !!GetBoolValue(sBase + "ShowMarker", TRUE);

		m_nShadeMode[nView]			= GetDWordValue(sBase + "ShadeMode", 2);
	}

	return TRUE;
}

BOOL COptionsViewports::Save()
{
	for(uint32 nView = 0; nView < NUM_VIEWPORTS; nView++)
	{
		CString sBase;
		sBase.Format("View%d", nView);

		SetBoolValue(sBase + "ShowGrid", m_bShowGrid[nView] ? TRUE : FALSE);
		SetBoolValue(sBase + "ShowNormals", m_bShowNormals[nView] ? TRUE : FALSE);
		SetBoolValue(sBase + "ShowWireframe", m_bShowWireframe[nView] ? TRUE : FALSE);
		SetBoolValue(sBase + "SelectBackface", m_bSelectBackface[nView] ? TRUE : FALSE);
		SetBoolValue(sBase + "ShowObjects", m_bShowObjects[nView] ? TRUE : FALSE);
		SetBoolValue(sBase + "ShowMarker", m_bShowMarker[nView] ? TRUE : FALSE);

		SetDWordValue(sBase + "ShadeMode", m_nShadeMode[nView]);
	}

	return TRUE;
}

