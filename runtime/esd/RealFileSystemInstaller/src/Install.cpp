// Install.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "Install.h"
#include "InstallDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInstallApp

BEGIN_MESSAGE_MAP(CInstallApp, CWinApp)
	//{{AFX_MSG_MAP(CInstallApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInstallApp construction

CInstallApp::CInstallApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CInstallApp object

CInstallApp theApp;

//-----------------------------------------------------------------------------
BOOL CopyFileFromRC(const char* sName, const char* sType, const char* sDest)
{
	HMODULE hMod = AfxGetResourceHandle();

	HRSRC hRes = FindResource(AfxGetResourceHandle(), sName, sType);
	if (!hRes)
	{
		DWORD error = GetLastError();
		return(FALSE);
	}

	HGLOBAL hGlob = LoadResource(hMod, hRes);
	if (!hGlob) return(FALSE);

	DWORD dwSize = SizeofResource(hMod, hRes);
	if (dwSize == 0) return(FALSE);
	if (dwSize > 999999) return(FALSE);

	BYTE* pRes = (BYTE*)LockResource(hGlob);
	if (!pRes) return(FALSE);

	BYTE* pData;
	LT_MEM_TRACK_ALLOC(pData = new BYTE [dwSize + 32],LT_MEM_TYPE_MISC);
	if (!pData) return(FALSE);

	memset(pData, 0, dwSize + 16);
	memcpy(pData, pRes, dwSize);

	CFile file;
	if (!file.Open(sDest, CFile::modeCreate | CFile::modeWrite))
	{
		delete pData;
		return(FALSE);
	}

	file.Write(pData, dwSize);
	file.Flush();
	file.Close();

	delete pData;
	pData = NULL;

	return(TRUE);
}
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// CInstallApp initialization

BOOL CInstallApp::InitInstance()
{
	// Get the location of the RMA core from the windows registry
	HKEY hKey = NULL;
	char szPathName[_MAX_PATH];
	DWORD bufSize = sizeof(szPathName) - 1;

	if (ERROR_SUCCESS == RegOpenKey(HKEY_CLASSES_ROOT,
	 "Software\\RealNetworks\\Preferences\\DT_Plugins", &hKey)) 
	{ 
		// Get the engine path
		RegQueryValue(hKey, "", szPathName, (long *)&bufSize);
		RegCloseKey(hKey);
		strcat(szPathName, "LITH3210.DLL");
	}
	else
	{
		MessageBox(NULL, "Unable to find RN plugin directory.", NULL, MB_OK);
		return FALSE;
	}

	LPCSTR pName = "#130";
	LPCSTR pType = "PLUGIN";

	if (CopyFileFromRC(pName, pType, szPathName))
		MessageBox(NULL, "LITH3210.DLL installation complete.", "Real Plug-In Installer", MB_OK);
	else
		MessageBox(NULL, "LITH3210.DLL installation failed.", NULL, MB_OK);

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.
/*
	CInstallDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
*/
	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
