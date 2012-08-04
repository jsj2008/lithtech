// WinLTCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WinLTC.h"
#include "WinLTCDlg.h"
#include "ltamgr.h"
#include "tdguard.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinLTCDlg dialog

CWinLTCDlg::CWinLTCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWinLTCDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CWinLTCDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWinLTCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWinLTCDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWinLTCDlg, CDialog)
	//{{AFX_MSG_MAP(CWinLTCDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_COMPRESS, OnCompress)
	ON_BN_CLICKED(IDC_VIEW, OnView)
	ON_BN_CLICKED(IDC_COMPRESSTO, OnCompressTo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinLTCDlg message handlers

BOOL CWinLTCDlg::OnInitDialog()
{
	if (!TdGuard::Aegis::GetSingleton().DoWork())
	{
		ExitProcess(0);
		return FALSE;
	}

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	//init the default text
	CString sInfo;
	sInfo.LoadString(IDS_INFO_DEFAULT);
	UpdateFileInfo(sInfo);

	//disable the buttons
	EnableControlButtons(FALSE);

	//set up the button text
	UpdateButtonNames(CString(""));
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWinLTCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CWinLTCDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWinLTCDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

//gets the path name given a specified file
CString CWinLTCDlg::GetPathName(const CString& sFile)
{
	CString rv;

	//try and find the ending slash
	int nPos = sFile.ReverseFind('\\');
	
	nPos = LTMAX(nPos, sFile.ReverseFind('/'));

	//see if we found one
	if(nPos == -1)
	{
		//no match
		return rv;
	}

	//get everything to the left, including the slash
	rv = sFile.Left(nPos + 1);

	return rv;	
}

//gets the extension of the given filename
CString CWinLTCDlg::GetExtension(const CString& sFile)
{
	CString rv;

	//try and find the period
	int nPos = sFile.ReverseFind('.');

	//see if we found one
	if(nPos == -1)
	{
		//no match
		return rv;
	}

	//get everything to the right, except the period
	rv = sFile.Mid(nPos + 1);

	return rv;	
}

//modifies the string in the file window to reflect the given string
void CWinLTCDlg::UpdateFileName(const CString& sFile)
{
	//get the edit control
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_FILENAME);

	ASSERT(pEdit);

	//set the text
	pEdit->SetWindowText(sFile);
}

//updates the text in the information view
void CWinLTCDlg::UpdateFileInfo(const CString& sInfo)
{
	//get the edit control
	CStatic* pInfo = (CStatic*)GetDlgItem(IDC_FILEINFO);

	ASSERT(pInfo);

	//set the text
	pInfo->SetWindowText(sInfo);
}

//retreives the size of the specified file
DWORD CWinLTCDlg::GetFileSize(const CString& sFileName)
{
	CFile File;
	
	//open up the file
	if(File.Open(sFileName, CFile::modeRead) == FALSE)
	{
		//failed
		return 0;
	}

	//seek to the end
	DWORD nSize = (DWORD) File.GetLength();

	//close
	File.Close();

	return nSize;
}

//allows the enabling and disabling of the control buttons
void CWinLTCDlg::EnableControlButtons(BOOL bEnable)
{
	((CButton*)GetDlgItem(IDC_COMPRESS))->EnableWindow(bEnable);
	((CButton*)GetDlgItem(IDC_VIEW))->EnableWindow(bEnable);
	((CButton*)GetDlgItem(IDC_COMPRESSTO))->EnableWindow(bEnable);
}

//updates the names of the buttons to accomodate for the extension, changing
//compress to expand accordingly
void CWinLTCDlg::UpdateButtonNames(const CString& sFilename)
{
	//determine if it is compressed
	BOOL bCompressed = CLTAUtil::IsFileCompressed((const char*)sFilename) ? TRUE : FALSE;

	//update the names accordingly

	CString sExpandCompress;
	CString sExpandCompressTo;

	if(bCompressed)
	{
		sExpandCompress.LoadString(IDS_EXPANDBUTTON);
		sExpandCompressTo.LoadString(IDS_EXPANDTOBUTTON);
	}
	else
	{
		sExpandCompress.LoadString(IDS_COMPRESSBUTTON);
		sExpandCompressTo.LoadString(IDS_COMPRESSTOBUTTON);
	}

	//update the text of the buttons
	((CButton*)GetDlgItem(IDC_COMPRESS))->SetWindowText(sExpandCompress);
	((CButton*)GetDlgItem(IDC_COMPRESSTO))->SetWindowText(sExpandCompressTo);

}


void CWinLTCDlg::OnBrowse() 
{
	//the user pressed browse, this means we need to open up a file
	//dialog and update the file

	//load up the strings from the table
	CString sDefaultExtension;
	sDefaultExtension.LoadString(IDS_BROWSE_DEFAULTEXTENSION);

	CString sFilter;
	sFilter.LoadString(IDS_BROWSE_FILTER);

	//now create the dialog
	CFileDialog FileDlg(TRUE, (LPCTSTR)sDefaultExtension, NULL, 
						OFN_FILEMUSTEXIST, sFilter);

	//prompt the user
	if(FileDlg.DoModal() == IDOK)
	{
		//we need to update our member variables
		SetNewFileName(FileDlg.GetPathName());		
	}
	
}

//given a file name, it will return the name with the extension properly switched
//for the opposite type of compression
CString CWinLTCDlg::ToggleExtension(const CString& sFileName)
{
	//trim off the extension
	CString sExtension = GetExtension(sFileName);

	CString sNewFile;
	
	//see if this is an extension we recognize
	if(sExtension.CompareNoCase("lta") == 0)
	{
		sNewFile = sFileName.Left(sFileName.GetLength() - sExtension.GetLength()) + "ltc";
	}
	else if(sExtension.CompareNoCase("ltc") == 0)
	{
		sNewFile = sFileName.Left(sFileName.GetLength() - sExtension.GetLength()) + "lta";
	}
	else
	{
		//unrecognized extension, just tack on .ltc
		sNewFile = sFileName + ".ltc";
	}

	return sNewFile;
}

//determines if the designated file exists
BOOL CWinLTCDlg::DoesFileExist(const CString& sFileName)
{
	CFile InFile;
	if(InFile.Open(sFileName, CFile::modeRead) == TRUE)
	{
		InFile.Close();
		return TRUE;
	}

	return FALSE;
}

//sets up the dialog to reflect a new file name
void CWinLTCDlg::SetNewFileName(const CString& sFileName)
{
	//modify our stats
	m_nFileSize = GetFileSize(sFileName);

	//now display the file name
	UpdateFileName(sFileName);

	//update the info
	CString sInfo;
	sInfo.Format(IDS_INFO_FILESIZE, m_nFileSize / 1024);

	UpdateFileInfo(sInfo);

	//enable the control buttons
	EnableControlButtons(TRUE);

	//update the text on the buttons
	UpdateButtonNames(sFileName);

	//save this name
	m_sFileName = sFileName;
}

void CWinLTCDlg::OnCompress() 
{

	//lets first off figure out the new name
	CString sOutName = ToggleExtension(m_sFileName);

	//now lets see if it exists
	if(DoesFileExist(sOutName))
	{
		//prompt the user
		CString sTitle;
		CString sPrompt;
		
		sTitle.LoadString(IDS_OVERWRITE_TITLE);
		sPrompt.Format(IDS_OVERWRITE_PROMPT, sOutName, GetFileSize(sOutName) / 1024);

		if(MessageBox(sPrompt, sTitle, MB_ICONQUESTION | MB_YESNO) == IDNO)
		{
			return;
		}
	}

	//now let us commence with the compression/expansion
	if(ToggleCompression(	m_sFileName, sOutName, 
							CLTAUtil::IsFileCompressed(m_sFileName) ? TRUE : FALSE) == TRUE)
	{
		//we succeeded. Now we need to delete the original file
		CFile::Remove(m_sFileName);

		//save the old size
		DWORD nOldSize = m_nFileSize;

		//now we switcht the file over to the new name
		SetNewFileName(sOutName);

		//show the stats
		ShowCompressionStatistics(nOldSize, m_nFileSize);
	}	
}

void CWinLTCDlg::OnView() 
{
	//see if this file is compressed
	bool bCompressed = CLTAUtil::IsFileCompressed(m_sFileName);

	if(bCompressed)
	{
		//we need to decompress this file

		//get the temp directory
		TCHAR pszTempPath[MAX_PATH];
		GetTempPath(MAX_PATH, pszTempPath);

		//now get the temp file name
		TCHAR pszTempFile[MAX_PATH];
		GetTempFileName(pszTempPath, "ltc", 0, pszTempFile);

		//now we need to decompress to this file
		ToggleCompression(m_sFileName, CString(pszTempFile), TRUE);

		//now view it
		ShellExecute(m_hWnd, "open", "wordpad", pszTempFile, NULL, SW_SHOWDEFAULT);
	}
	else
	{
		//we can just simply view it
		ShellExecute(m_hWnd, "open", "wordpad", m_sFileName, NULL, SW_SHOWDEFAULT);
	}	
}

void CWinLTCDlg::OnCompressTo() 
{
	//see if this file is compressed or not
	bool bCompressed = CLTAUtil::IsFileCompressed(m_sFileName);

	//now load up the strings for the filters based upon if it is compressed
	CString sDefaultExtension;
	CString sFilter;

	if(bCompressed)
	{
		sDefaultExtension.LoadString(IDS_EXPANDTO_DEFAULTEXTENSION);
		sFilter.LoadString(IDS_EXPANDTO_FILTER);
	}
	else
	{
		sDefaultExtension.LoadString(IDS_COMPRESSTO_DEFAULTEXTENSION);
		sFilter.LoadString(IDS_COMPRESSTO_FILTER);
	}

	//get the default name by toggling the extension of the current one
	CString sDefaultFile = ToggleExtension(m_sFileName);


	//see where they want to compress/expand to
	CFileDialog FileDlg(FALSE, sDefaultExtension, sDefaultFile, 
						OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, sFilter);

	if(FileDlg.DoModal() == IDOK)
	{
		//now we need to expand/compress to the given destination
		if(ToggleCompression(m_sFileName, FileDlg.GetPathName(), bCompressed) == FALSE)
		{
			//very bad things
			return;
		}		
	}
	
}

//updates the information string to display compression statistics
void CWinLTCDlg::ShowCompressionStatistics(DWORD nOldSize, DWORD nNewSize)
{
	CString sStats;

	float fRatio = 0.0f;

	if(nOldSize != 0)
	{
		fRatio = ((float)nOldSize - (float)nNewSize) * 100 / (float)nOldSize;
	}
	
	sStats.Format(IDS_INFO_COMPRESSSTATS, nNewSize / 1024, nOldSize / 1024, fRatio);

	UpdateFileInfo(sStats);
}

//switches between two files. It will load in the first file, and save it to the
//other file using the opposite compression flag.
BOOL CWinLTCDlg::ToggleCompression(	const CString& sInFile, const CString sOutFile, 
									BOOL bIsInFileCompressed)
{
	//the error title, for when we need it
	CString sErrorTitle;
	sErrorTitle.LoadString(IDS_ERROR_TITLE);


	//open up the original file
	CLTAFile InFile(sInFile, true, (bIsInFileCompressed) ? true : false);
	
	if(!InFile.IsValid())
	{
		//couldn't open up the file. Inform the user and bail
		CString sError;
		sError.Format(IDS_ERROR_CANTOPENFILE, (LPCTSTR)sInFile);
		MessageBox(sError, sErrorTitle, MB_ICONEXCLAMATION | MB_OK);

		return FALSE;
	}

	//open the output file
	CLTAFile OutFile(sOutFile, false, (bIsInFileCompressed) ? false : true);

	if(!OutFile.IsValid())
	{
		//close all open files
		InFile.Close();

		//couldn't open up the file. Inform the user and bail
		CString sError;
		sError.Format(IDS_ERROR_CANTOPENFILE, (LPCTSTR)sOutFile);
		MessageBox(sError, sErrorTitle, MB_ICONEXCLAMATION | MB_OK);

		return FALSE;
	}

	//user feedback that we are working
	BeginWaitCursor();

	//now load from one while writing to the other
	uint8 nVal;
	while(InFile.ReadByte(nVal))
	{
		if(OutFile.WriteByte(nVal) == false)
		{
			//bad error
			EndWaitCursor();
			InFile.Close();
			OutFile.Close();

			CString sError;
			sError.Format(IDS_ERROR_CANTWRITETOFILE, (LPCTSTR)sOutFile);
			MessageBox(sError, sErrorTitle, MB_ICONEXCLAMATION | MB_OK);

			return FALSE;
		}
	}

	//the file has been compressed successfully
	EndWaitCursor();

	return TRUE;
}

