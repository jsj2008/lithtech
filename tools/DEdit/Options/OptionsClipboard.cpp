// OptionsClipboard.cpp: implementation of the COptionsClipboard class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "optionsclipboard.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionsClipboard::COptionsClipboard()
{
	m_bGenerateUniqueNames=TRUE;
	m_bUpdateRefProps=TRUE;
	m_bDisplayNameChangeReport=FALSE;
}

COptionsClipboard::~COptionsClipboard()
{

}

/************************************************************************/
// Load
BOOL COptionsClipboard::Load()
{
	m_bGenerateUniqueNames=GetBoolValue("GenerateUniqueNames", TRUE);
	m_bUpdateRefProps=GetBoolValue("UpdateReferencingProperties", TRUE);
	m_bDisplayNameChangeReport=GetBoolValue("DisplayNameChangeReport", FALSE);

	return TRUE;
}

/************************************************************************/
// Save
BOOL COptionsClipboard::Save()
{
	SetBoolValue("GenerateUniqueNames", m_bGenerateUniqueNames);
	SetBoolValue("UpdateReferencingProperties", m_bUpdateRefProps);
	SetBoolValue("DisplayNameChangeReport", m_bDisplayNameChangeReport);

	return TRUE;
}