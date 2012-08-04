// GameInfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "GameInfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGameInfoDlg dialog


CGameInfoDlg::CGameInfoDlg(CString sDll, CString sRez, CString sApp, CString sCmdLine, CWnd* pParent /*=NULL*/)
	: CDialog(CGameInfoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGameInfoDlg)
	m_sApp = sApp;//_T("");
	m_sRez = sRez;//_T("");
	m_sCmdLine = sCmdLine;//_T("");
	m_sDll = sDll;//_T("");
	//}}AFX_DATA_INIT
}


void CGameInfoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGameInfoDlg)
	DDX_Text(pDX, IDC_APP, m_sApp);
	DDX_Text(pDX, IDC_REZ, m_sRez);
	DDX_Text(pDX, IDC_COMMANDLINE, m_sCmdLine);
	DDX_Text(pDX, IDC_DLL, m_sDll);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGameInfoDlg, CDialog)
	//{{AFX_MSG_MAP(CGameInfoDlg)
	ON_BN_CLICKED(IDC_BROWSEAPP, OnBrowseAppPath)
	ON_BN_CLICKED(IDC_BROWSEREZ, OnBrowseProjectPath)
	ON_BN_CLICKED(IDC_BROWSEDLL, OnBrowseDllPath)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CGameInfoDlg::OnBrowseAppPath()
{
	//save the current working directory, since the file dialog messes it up
	char pszWorkingDir[MAX_PATH];

	GetCurrentDirectory(sizeof(pszWorkingDir), pszWorkingDir);

	//maybe make the default file so that it looks into the current path directory?
	CFileDialog FileDlg(	TRUE, "*.exe", NULL, OFN_FILEMUSTEXIST, 
							"LithTech Exe (*.exe)|*.exe|All Files (*.*)|*.*||");

	//prompt the user
	if(FileDlg.DoModal() == IDOK)
	{
		//they selected a file
		
		//get the string
		CString sFilePath = FileDlg.GetPathName();

		//now set this string in the edit box
		CEdit* pProjPathBox = (CEdit*)GetDlgItem(IDC_APP);
		ASSERT(pProjPathBox);
		pProjPathBox->SetWindowText(sFilePath);
	}

	SetCurrentDirectory(pszWorkingDir);    
}

void CGameInfoDlg::OnBrowseProjectPath()
{
	//save the current working directory, since the file dialog messes it up
	char pszWorkingDir[MAX_PATH];

	GetCurrentDirectory(sizeof(pszWorkingDir), pszWorkingDir);

	//maybe make the default file so that it looks into the current path directory?
	CFileDialog FileDlg(	TRUE, "*.dep", NULL, OFN_FILEMUSTEXIST, 
							"DEdit Project Files (*.dep)|*.dep|All Files (*.*)|*.*||");

	//prompt the user
	if(FileDlg.DoModal() == IDOK)
	{
		//they selected a file
		
		//get the string
		CString sFilePath = FileDlg.GetPathName();

		//now get the file name
		CString sFileName = FileDlg.GetFileName();

		//make sure that the path is larger than the filename
		if(sFileName.GetLength() <= sFilePath.GetLength())
		{
			//strip out the file name
			sFilePath = sFilePath.Left(sFilePath.GetLength() - sFileName.GetLength() - 1);
		}

		//now set this string in the edit box
		CEdit* pProjPathBox = (CEdit*)GetDlgItem(IDC_REZ);
		ASSERT(pProjPathBox);
		pProjPathBox->SetWindowText(sFilePath);
	}

	SetCurrentDirectory(pszWorkingDir);    
}

void CGameInfoDlg::OnBrowseDllPath()
{
	//save the current working directory, since the file dialog messes it up
	char pszWorkingDir[MAX_PATH];

	GetCurrentDirectory(sizeof(pszWorkingDir), pszWorkingDir);

	CFileDialog FileDlg(	TRUE, "*.fxd", NULL, OFN_FILEMUSTEXIST, 
							"ClientFX DLL (*.fxd)|*.fxd|All Files (*.*)|*.*||");

	//prompt the user
	if(FileDlg.DoModal() == IDOK)
	{
		//they selected a file
		
		//get the string
		CString sFilePath = FileDlg.GetPathName();

		//now set this string in the edit box
		CEdit* pProjPathBox = (CEdit*)GetDlgItem(IDC_DLL);
		ASSERT(pProjPathBox);
		pProjPathBox->SetWindowText(sFilePath);
	}

	SetCurrentDirectory(pszWorkingDir);    
}

/////////////////////////////////////////////////////////////////////////////
// CGameInfoDlg message handlers
