#include "bdefs.h"
#include "optionscontrols.h"
#include "keydefaultfactory.h"
#include "globalhotkeydb.h"

COptionsControls::COptionsControls() :
	m_bInvertMouseY(false),
	m_bZoomToCursor(false)
{
}

COptionsControls::~COptionsControls()
{
}

// Load/Save
BOOL COptionsControls::Load()
{
	m_bInvertMouseY	= GetBoolValue	("InvertMouseY",		FALSE) ? true : false;
	m_bZoomToCursor	= GetBoolValue	("ZoomToCursor",		FALSE) ? true : false;
	m_bOrbitAroundSel = GetBoolValue("OrbitAroundSel",		TRUE)  ? true : false;
	m_bAutoCaptureFocus = GetBoolValue("AutoCaptureFocus",	TRUE)  ? true : false;

	//we also need to load the control style
	CString sStyle = GetStringValue("ControlStyle", "");

	//now try and create that style
	CKeyDefaultAggregate* pAggregate = CKeyDefaultFactory::CreateDefault(sStyle);

	if(pAggregate)
	{
		CGlobalHotKeyDB::AddAggregate(pAggregate);
	}

	return TRUE;
}

BOOL COptionsControls::Save()
{
	SetBoolValue("InvertMouseY",	m_bInvertMouseY ? TRUE : FALSE);
	SetBoolValue("ZoomToCursor",	m_bZoomToCursor ? TRUE : FALSE);
	SetBoolValue("OrbitAroundSel",	m_bOrbitAroundSel ? TRUE : FALSE);
	SetBoolValue("AutoCaptureFocus", m_bAutoCaptureFocus ? TRUE : FALSE);

	return TRUE;
}

