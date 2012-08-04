// winpacker.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "winpacker.h"
#include "winpackerDlg.h"
#include "paramlist.h"
#include "associatepacker.h"
#include "dummypacker.h"
#include "backenddialog.h"
#include "DirDialog.h"
#include "tdguard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//the maximum length that a packer can have for its name
#define MAX_PACKER_NAME_LEN				256

/////////////////////////////////////////////////////////////////////////////
// CWinpackerApp

BEGIN_MESSAGE_MAP(CWinpackerApp, CWinApp)
	//{{AFX_MSG_MAP(CWinpackerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinpackerApp construction

CWinpackerApp::CWinpackerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWinpackerApp object

CWinpackerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// BrowseForFile
//
// Brings up a file dialog for the user to find the file they wish to pack
// and stores it in the given string if the user hits ok. Returns TRUE if
// ok is selected, false otherwise
//
bool BrowseForFile(DWORD nFilterID, CString& sFile, const char* pszDlgTitle = NULL)
{
	//save the current working directory, since the file dialog messes it up
	char pszWorkingDir[MAX_PATH];

	GetCurrentDirectory(sizeof(pszWorkingDir), pszWorkingDir);


	CString sFilter;
	sFilter.LoadString(nFilterID);

	CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, sFilter);

	if(pszDlgTitle)
	{
		dlg.m_ofn.lpstrTitle = pszDlgTitle;
	}

	if(dlg.DoModal() == IDOK)
	{
		//restore the current working directory
		SetCurrentDirectory(pszWorkingDir);

		//save the file
		sFile = dlg.GetPathName();
		return true;
	}

	return false;
}

//this will try and get the file to pack. If it is unable to get the file,
//it will return false, and the app should exit. Otherwise it will return true
//and fill out sFilename with the file to open
BOOL CWinpackerApp::GetFileToPack(const CParamList& ParamList, CString& sFilename)
{
	int32 nFilePos = ParamList.FindParam("-file");
	if((nFilePos == -1) || (nFilePos >= (int32)ParamList.GetNumParams() - 1))
	{
		//let the user browse for the file
		if(!BrowseForFile(IDS_BROWSE_PACK_FILTER, sFilename, "Select File to Pack"))
			return FALSE;
	}
	else
	{
		sFilename = ParamList.GetParameter(nFilePos + 1);
	}

	//make sure that the file exists
	CFileStatus DummyStats;
	if(CFile::GetStatus(sFilename, DummyStats) == FALSE)
	{
		//there is no file, report an error and bail
		CString sMsg;
		sMsg.Format(IDS_PACK_FILE_NOT_FOUND, sFilename);

		//see if they want to bail or not
		if(MessageBox(NULL, sMsg, "Error", MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL)
			return FALSE;

		//let the user browse for the file
		if(!BrowseForFile(IDS_BROWSE_PACK_FILTER, sFilename, "Select File to Pack"))
			return FALSE;
	}

	return TRUE;
}

//this will try and get the packer to be used. It will return the success code,
//and if successful, will fill out the packer interface
BOOL CWinpackerApp::GetPacker(const CParamList& ParamList, const CString& sFilename, IPackerImpl*& pIPacker)
{
	//---------------------------------------------------------------------------------------
	//Uncomment the lines below to use the dummy packer. This is useful for debugging and
	//testing new features
	//---------------------------------------------------------------------------------------
	//static CDummyPacker DummyPacker;
	//pIPacker = &DummyPacker;
	//return TRUE;

	//now we need to get the pack directory
	const char* pszDir = NULL;
	int32 nDirPos = ParamList.FindParam("-PackerDir");
	if((nDirPos != -1) && (nDirPos < (int32)ParamList.GetNumParams() - 1))
	{
		pszDir = ParamList.GetParameter(nDirPos + 1);
	}

	//now we need to find the interface
	pIPacker = AssociatePacker(sFilename, pszDir);

	while(pIPacker == NULL)
	{
		//let the user browse for a directory and try again
		CDirDialog	DirDlg;
		DirDlg.m_strTitle.Format("Unable to find a packer for %s\r\nPlease Select Packer Directory", sFilename);

		if(!DirDlg.DoBrowse())
		{
			return FALSE;
		}

		// dlg.m_strPath has the chosen directory!
		pIPacker = AssociatePacker(sFilename, DirDlg.m_strPath);
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CWinpackerApp initialization

BOOL CWinpackerApp::InitInstance()
{
	if (!TdGuard::Aegis::GetSingleton().Init() ||
		!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.


	AfxInitRichEdit();

	//first off, the command line needs to be searched for the filename of file to be
	//packed, and the directory to search for packers
	CParamList ParamList(m_lpCmdLine);

	//get the filename
	CString sFileToPack;
	if(GetFileToPack(ParamList, sFileToPack) == FALSE)
	{
		//unable to get a filename
		return FALSE;
	}

	//now get the packer
	IPackerImpl* pPacker;
	if(GetPacker(ParamList, sFileToPack, pPacker) == FALSE)
	{
		//unable to get a packer
		return FALSE;
	}

	//get the name of the packer
	char pszPackerName[MAX_PACKER_NAME_LEN];
	if(pPacker->GetPackerName(pszPackerName, MAX_PACKER_NAME_LEN) == false)
	{
		AfxMessageBox("Could not get the name of the packer");
		return FALSE;
	}

	//setup the front end user interface
	CWinpackerDlg UIDlg(pPacker);
	UIDlg.m_sCommandLine	= m_lpCmdLine;
	UIDlg.m_sPackerName		= pszPackerName;
	UIDlg.m_sFileToPack		= sFileToPack;
	UIDlg.m_bSkipDlg		= (ParamList.FindParam("-SkipPropDlg") != -1) ? TRUE : FALSE;

	//do the front end
	int nResponse = UIDlg.DoModal();
	if (nResponse != IDOK)
	{
		//the user cancelled
		return FALSE;
	}

	//we want to save their options to the registry now for future sessions


	//the user hit ok. We need to create our dialog, and have it spawn their thread
	CBackEndDialog BackEnd(pPacker, UIDlg.GetPropList());
	BackEnd.m_sFilename			= sFileToPack;
	BackEnd.m_bPauseWhenDone	= (ParamList.FindParam("-NoPauseWhenDone") != -1) ? FALSE : TRUE;

	BackEnd.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
