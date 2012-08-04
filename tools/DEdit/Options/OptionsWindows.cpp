// OptionsWindows.cpp: implementation of the COptionsWindows class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "dedit.h"
#include "optionswindows.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionsWindows::COptionsWindows()
{

}

COptionsWindows::~COptionsWindows()
{

}

/************************************************************************/
// Load
BOOL COptionsWindows::Load()
{
	m_mainControlArray.RemoveAll();
	m_barControlArray.RemoveAll();

	// Load the project controls
	int nMainProjectControls=GetDWordValue("NumMainProjectControls", 0);

	int i;
	for (i=0; i < nMainProjectControls; i++)
	{
		// Format the registry key name
		CString sName;
		sName.Format("MainProjectControl%d", i+1);

		// Load the control type
		BOOL bSuccess;
		CMainFrame::ProjectControl controlType=(CMainFrame::ProjectControl)GetDWordValue(sName, 0, &bSuccess);

		// Add the control type if it was found successfully
		if (bSuccess)
		{
			m_mainControlArray.Add(controlType);
		}
	}

	// Load the project controls
	int nBarProjectControls=GetDWordValue("NumBarProjectControls", 0);

	for (i=0; i < nBarProjectControls; i++)
	{
		// Format the registry key name
		CString sName;
		sName.Format("BarProjectControl%d", i+1);

		// Load the control type
		BOOL bSuccess;
		CMainFrame::ProjectControl controlType=(CMainFrame::ProjectControl)GetDWordValue(sName, 0, &bSuccess);

		// Add the control type if it was found successfully
		if (bSuccess)
		{
			m_barControlArray.Add(controlType);
		}
	}

	// Search to make sure that all of the control bars are in one
	// of the arrays.  If they aren't (such as a new bar has been
	// added), then add it to the barControlArray.
	for (i=0; i < CMainFrame::CB_LAST_CONTROL_INDEX; i++)
	{
		BOOL bFound=FALSE;

		// Search the main control array
		int n;
		for (n=0; n < m_mainControlArray.GetSize(); n++)
		{
			if (m_mainControlArray[n] == i)
			{
				// The control bar was found
				bFound=TRUE;
				break;
			}
		}

		// Search the project bar control array
		for (n=0; n < m_barControlArray.GetSize(); n++)
		{
			if (m_barControlArray[n] == i)
			{
				// The control bar was found
				bFound=TRUE;
				break;
			}
		}

		// Add the control to the bar array if it was not found
		if (!bFound)
		{
			m_barControlArray.Add((CMainFrame::ProjectControl)i);
		}
	}

	return TRUE;
}

/************************************************************************/
// Save
BOOL COptionsWindows::Save()
{
	// Save the project controls
	SetDWordValue("NumMainProjectControls", m_mainControlArray.GetSize());

	int i;
	for (i=0; i < m_mainControlArray.GetSize(); i++)
	{
		CString sName;
		sName.Format("MainProjectControl%d", i+1);

		SetDWordValue(sName, (DWORD)m_mainControlArray[i]);
	}

	// Save the project controls
	SetDWordValue("NumBarProjectControls", m_barControlArray.GetSize());

	for (i=0; i < m_barControlArray.GetSize(); i++)
	{
		CString sName;
		sName.Format("BarProjectControl%d", i+1);

		SetDWordValue(sName, (DWORD)m_barControlArray[i]);
	}	

	return TRUE;
}

/************************************************************************/
// Set the control arrays
void COptionsWindows::SetMainControlArray(CMoArray<CProjectControlBarInfo *> &array)
{
	m_mainControlArray.RemoveAll();

	int i;
	for (i=0; i < array.GetSize(); i++)
	{
		m_mainControlArray.Add(array[i]->m_ControlType);
	}
}

/************************************************************************/
// Set the control arrays
void COptionsWindows::SetBarControlArray(CMoArray<CProjectControlBarInfo *> &array)
{
	m_barControlArray.RemoveAll();

	int i;
	for (i=0; i < array.GetSize(); i++)
	{
		m_barControlArray.Add(array[i]->m_ControlType);
	}
}