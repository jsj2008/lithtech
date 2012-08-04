//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// OptionsBase.cpp: implementation of the COptionsBase class.
//
//////////////////////////////////////////////////////////////////////

#include "bdefs.h"
#include "optionsbase.h"
#include "genregmgr.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionsBase::COptionsBase()
{
	m_pRegMgr=NULL;
}

COptionsBase::~COptionsBase()
{

}

/************************************************************************/
// Intializes the options class with a GenRegMgr and a registry root path
BOOL COptionsBase::Init(CGenRegMgr *pRegMgr, CString sRegRoot)
{
	if (pRegMgr)
	{
		m_pRegMgr=pRegMgr;
		m_sRegRoot=sRegRoot;

		return TRUE;
	}

	return FALSE;
}

/************************************************************************/
// Loads the options to the registry
BOOL COptionsBase::Load()
{
	return TRUE;
}

/************************************************************************/
// Saves the options to the registry
BOOL COptionsBase::Save()
{
	return TRUE;
}

/************************************************************************/
// Sets a string value in the registry
BOOL COptionsBase::SetStringValue (LPCSTR strValueName, LPCSTR strValue)
{
	if (m_pRegMgr)
	{
		return m_pRegMgr->SetStringValue(m_sRegRoot, strValueName, strValue);
	}
	else
	{
		return FALSE;
	}
}

/************************************************************************/
// Sets a DWORD value in the registry
BOOL COptionsBase::SetDWordValue (LPCSTR strValueName, DWORD dwValue)
{
	if (m_pRegMgr)
	{
		return m_pRegMgr->SetDwordValue(m_sRegRoot, strValueName, dwValue);
	}
	else
	{
		return FALSE;
	}
}

/************************************************************************/
// Sets a binary value in the registry
BOOL COptionsBase::SetBinaryValue (LPCSTR strValueName, void* pData, DWORD dwDataSize)
{
	if (m_pRegMgr)
	{
		return m_pRegMgr->SetBinaryValue(m_sRegRoot, strValueName, pData, dwDataSize);
	}
	else
	{
		return FALSE;
	}
}

/************************************************************************/
// Sets a bool in the registry
// (stored as a string either "True" or "False")
BOOL COptionsBase::SetBoolValue (LPCSTR strValueName, BOOL bValue)
{
	if (m_pRegMgr)
	{
		return m_pRegMgr->SetStringBoolValue(m_sRegRoot, strValueName, bValue);
	}
	else
	{
		return FALSE;
	}
}

/************************************************************************/
// Returns the size of a registry entry
DWORD COptionsBase::GetValueSize (LPCSTR strValueName)
{
	if (m_pRegMgr)
	{
		return m_pRegMgr->GetValueSize(m_sRegRoot, strValueName);
	}
	else
	{
		return 0;
	}
}

/************************************************************************/
// Gets a string value from the registry
CString	COptionsBase::GetStringValue(LPCSTR strValueName, CString sDefault, BOOL *pbSuccess)
{
	// Get the size
	DWORD dwSize=GetValueSize(strValueName);
	
	// Create the buffer
	char *pBuffer=new char[dwSize+1];
	memset(pBuffer, '\0', dwSize+1);

	// Get the data
	if (GetBinaryValue(strValueName, pBuffer, dwSize))
	{
		// Copy the buffer into a CString
		CString sReturn=pBuffer;
		delete pBuffer;
		pBuffer=NULL;

		// Set the success value
		if (pbSuccess)
		{
			*pbSuccess=TRUE;
		}

		return sReturn;
	}
	else
	{
		// Set the success value
		if (pbSuccess)
		{
			*pbSuccess=FALSE;
		}

		// Just return the default
		return sDefault;
	}
}

/************************************************************************/
// Gets a value from the registry into a buffer
BOOL COptionsBase::GetBinaryValue (LPCSTR strValueName, void* pBuffer, DWORD dwBufferSize)
{
	if (m_pRegMgr)
	{
		return m_pRegMgr->GetValue(m_sRegRoot, strValueName, pBuffer, dwBufferSize);
	}
	else
	{
		return FALSE;
	}
}

/************************************************************************/
// Retrieves a DWORD value from the registry
DWORD COptionsBase::GetDWordValue (LPCSTR strValueName, DWORD dwDefault, BOOL *pbSuccess)
{
	// The value that will be returned from this function
	DWORD dwReturnValue=0;

	// The success status for this function
	BOOL bSuccess=FALSE;

	if (m_pRegMgr)
	{	
		bSuccess=m_pRegMgr->GetDwordValue(m_sRegRoot, strValueName, (uint32 *) &dwReturnValue);		
	}	

	// Set the success value
	if (pbSuccess)
	{
		*pbSuccess=bSuccess;
	}

	// Use the default value if the registry key could not be retrieved
	if (!bSuccess)
	{
		dwReturnValue=dwDefault;
	}

	return dwReturnValue;
}

/************************************************************************/
// Returns a bool value from the registry
BOOL COptionsBase::GetBoolValue (LPCSTR strValueName, BOOL bDefault, BOOL *pbSuccess)
{
	// The value that will be returned from this function
	BOOL bReturnValue=FALSE;

	// The success status for this function
	BOOL bSuccess=FALSE;

	if (m_pRegMgr)
	{
		bSuccess=m_pRegMgr->GetStringBoolValue(m_sRegRoot, strValueName, &bReturnValue);
	}

	// Set the success value
	if (pbSuccess)
	{
		*pbSuccess=bSuccess;
	}

	// Use the default value if the registry key could not be retrieved
	if (!bSuccess)
	{
		bReturnValue=bDefault;
	}

	return bReturnValue;
}


/************************************************************************/
// Returns a string value from the DEditOptions.ini file local to the project (.dep) file

#include "edithelpers.h"	// including these up top causes odd linker errors/conflicts
#include "projectbar.h"

CString	COptionsBase::GetStringValueFromOptionsFile(LPCSTR strValueName, CString sDefault, BOOL *pbSuccess)
{
	// Sanity checks
	if (!GetMainFrame()) return("");
	if (!GetProjectBar()) return("");
	if (!GetProjectBar()->GetProject()) return("");

	// Get our base directory
	CString sBaseDir = GetProjectBar()->GetProject()->GetBaseProjectDir();

	// Build the options file name
	CString sFile = sBaseDir + "\\" + "dedit.ini";

	// Read the requested option
	char sValue[256] = { "\0" };
	GetPrivateProfileString("Options", strValueName, "", sValue, 255, sFile);
	
	// Check if we read something
	if (sValue[0] != '\0')
	{
		// Copy the buffer into a CString
		CString sReturn = sValue;

		// Set the success value
		if (pbSuccess)
		{
			*pbSuccess=TRUE;
		}

		return sReturn;
	}
	else
	{
		// Set the success value
		if (pbSuccess)
		{
			*pbSuccess=FALSE;
		}

		// Just return the default
		return sDefault;
	}
}

BOOL COptionsBase::SetStringValueToOptionsFile(LPCSTR strValueName, LPCSTR sValue)
{
	// Sanity checks
	if (!GetMainFrame()) return(FALSE);
	if (!GetProjectBar()) return(FALSE);
	if (!GetProjectBar()->GetProject()) return(FALSE);

	// Get our base directory
	CString sBaseDir = GetProjectBar()->GetProject()->GetBaseProjectDir();

	// Build the options file name
	CString sFile = sBaseDir + "\\" + "dedit.ini";

	// Read the requested option
	BOOL bRet = WritePrivateProfileString("Options", strValueName, sValue, sFile);

	// All done
	return(bRet);
}
