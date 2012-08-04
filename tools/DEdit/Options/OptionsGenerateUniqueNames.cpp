// OptionsGenerateUniqueNames.cpp: implementation of the COptionsGenerateUniqueNames class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "optionsgenerateuniquenames.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionsGenerateUniqueNames::COptionsGenerateUniqueNames()
{
	m_bUpdateRefProps=TRUE;
	m_bUpdateSelPropsOnly=FALSE;
	m_bDisplayReportOfChanges=TRUE;
}

COptionsGenerateUniqueNames::~COptionsGenerateUniqueNames()
{

}

/************************************************************************/
// Load
BOOL COptionsGenerateUniqueNames::Load()
{
	m_bUpdateRefProps=GetBoolValue("UpdateReferencingProperties", TRUE);
	m_bUpdateSelPropsOnly=GetBoolValue("UpdatedSelectedPropertiesOnly", FALSE);
	m_bDisplayReportOfChanges=GetBoolValue("DisplayReportOfChanges", TRUE);

	return TRUE;
}

/************************************************************************/
// Save
BOOL COptionsGenerateUniqueNames::Save()
{
	SetBoolValue("UpdateReferencingProperties", m_bUpdateRefProps);
	SetBoolValue("UpdatedSelectedPropertiesOnly", m_bUpdateSelPropsOnly);
	SetBoolValue("DisplayReportOfChanges", m_bDisplayReportOfChanges);

	return TRUE;
}