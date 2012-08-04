// OptionsObjectBrowser.cpp: implementation of the COptionsObjectBrowser class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "optionsobjectbrowser.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionsObjectBrowser::COptionsObjectBrowser()
{
	m_bGroupByType=TRUE;
}

COptionsObjectBrowser::~COptionsObjectBrowser()
{

}

/************************************************************************/
// Load
BOOL COptionsObjectBrowser::Load()
{
	// Load the "group by type" value
	m_bGroupByType=GetBoolValue("GroupByType", TRUE);

	// Success
	return TRUE;
}

/************************************************************************/
// Save
BOOL COptionsObjectBrowser::Save()
{
	// Save the "group by type" value
	SetBoolValue("GroupByType", m_bGroupByType);

	// Success
	return TRUE;
}
