#include "bdefs.h"
#include "optionsprefabs.h"

COptionsPrefabs::COptionsPrefabs() :
	m_eContentsView(VIEWPREFAB_CONTENTS),
	m_bShowOutline(true),
	m_bShowOrientation(true)
{
}

COptionsPrefabs::~COptionsPrefabs()
{
}

// Load/Save
BOOL COptionsPrefabs::Load()
{
	m_eContentsView		= (EViewMode)GetDWordValue("ContentsViewMode", VIEWPREFAB_CONTENTS);
	m_bShowOutline		= GetBoolValue("ShowOutline", TRUE);
	m_bShowOrientation	= GetBoolValue("ShowOrientation", TRUE);

	return TRUE;
}

BOOL COptionsPrefabs::Save()
{
	SetDWordValue("ContentsViewMode",	(DWORD)m_eContentsView);
	SetBoolValue("ShowOutline",			m_bShowOutline);
	SetBoolValue("ShowOrientation",		m_bShowOrientation);

	return TRUE;
}

