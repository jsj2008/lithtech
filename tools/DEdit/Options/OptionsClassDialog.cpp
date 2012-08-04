//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsClassDialog.cpp: implementation of the COptionsClassDialog class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "optionsclassdialog.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionsClassDialog::COptionsClassDialog()
{

}

COptionsClassDialog::~COptionsClassDialog()
{

}

/************************************************************************/
// Loads the class dialog options to the registry
BOOL COptionsClassDialog::Load()
{	
	// Clear the recent class array
	m_recentClasses.RemoveAll();

	// Get the number of classes to load
	DWORD dwNumRecentClasses=GetDWordValue("NumRecentClasses", 0);

	// Load each class name
	unsigned int i;
	for (i=0; i < dwNumRecentClasses; i++)
	{
		// Create the key to load
		CString sKeyName;
		sKeyName.Format("RecentClass%d", i+1);
		
		// Get the registry setting
		BOOL bSuccess;
		CString sClass=GetStringValue(sKeyName, "", &bSuccess);
		if (bSuccess)
		{
			// Add the string to the array
			m_recentClasses.Add(sClass);
		}
	}

	// Get the max number of recent classes
	m_nMaxRecentClasses=GetDWordValue("MaxNumRecentClasses", 15);	

	// Load the "show heirarchy" value
	m_bShowTree=GetBoolValue("ShowTree", TRUE);
	
	// Load the "bind individually" value
	m_bBindIndividually=GetBoolValue("BindIndividually", FALSE);	

	return TRUE;
}

/************************************************************************/
// Saves the class dialog options to the registry
BOOL COptionsClassDialog::Save()
{
	// Set the number of classes
	SetDWordValue("NumRecentClasses", m_recentClasses.GetSize());

	// Save each class name
	unsigned int i;
	for (i=0; i < m_recentClasses.GetSize(); i++)
	{
		// Create the key to save
		CString sKeyName;
		sKeyName.Format("RecentClass%d", i+1);

		// Get the registry setting
		SetStringValue(sKeyName, m_recentClasses[i]);
		
	}

	// Save the max number of recent classes
	SetDWordValue("MaxNumRecentClasses", m_nMaxRecentClasses);
	
	// Save the "show heirarchy" value	
	SetBoolValue("ShowTree", m_bShowTree);	
	
	// Set the "bind individually" value
	SetBoolValue("BindIndividually", m_bBindIndividually);	

	return TRUE;
}
