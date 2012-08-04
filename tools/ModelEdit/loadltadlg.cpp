// loadltadlg.cpp : implementation file
//

//include stuff for thread sync
#include <afxmt.h>
#include "stdafx.h"
#include "modeledit.h"
#include "precompile.h"
#include "loadltadlg.h"


//for writing out the log file
#if _MSC_VER >= 1300
#include <fstream>
#else
#include <fstream.h>
#endif

//used to sync access to resources
CCriticalSection	g_LoadLogPipeCS;
CCriticalSection	g_ThreadDoneCS;

//cross thread resources
CString CLoadLTADlg::sm_sLoadLogPipe;
BOOL	CLoadLTADlg::sm_bLoadThreadDone = TRUE;

/////////////////////////////////////////////////////////////////////////////
// CLoadLTADlg dialog


CLoadLTADlg::CLoadLTADlg(bool bAutoConfirm, CWnd* pParent /*=NULL*/)
	: CDialog(CLoadLTADlg::IDD, pParent),
	  m_bPrevLoaderDone(TRUE), m_bAutoConfirm(bAutoConfirm)
{
	//{{AFX_DATA_INIT(CLoadLTADlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLoadLTADlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadLTADlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoadLTADlg, CDialog)
	//{{AFX_MSG_MAP(CLoadLTADlg)
	ON_BN_CLICKED(IDC_SAVELOADLOG, OnSaveLoadLog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadLTADlg message handlers

void CLoadLTADlg::OnSaveLoadLog() 
{
	//load in all the strings
	CString sDefFilename, sDefExtension, sFilter;
	sDefFilename.LoadString(IDS_DEFLTALOGFILENAME);
	sDefExtension.LoadString(IDS_DEFLTALOGEXTENSION);
	sFilter.LoadString(IDS_LTALOGFILTER);

	//create the file save dialog
	CFileDialog Dlg(FALSE, sDefExtension, sDefFilename, 
					OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, sFilter);

	//put up the dialog
	if(Dlg.DoModal() == IDOK)
	{
		//get the text from the window
		CString sLogText;
		((CEdit*)GetDlgItem(IDC_LOADLOG))->GetWindowText(sLogText);

		//open up the file
#if _MSC_VER >= 1300
		std::ofstream OutFile(Dlg.GetPathName());
#else
		ofstream OutFile(Dlg.GetPathName());
#endif

		//check to ensure that the file opened correctly
		if(OutFile.fail())
		{
			//failed to open the file, throw up a message box and bail
			CString sError;
			sError.Format("Unable to open file: %s", Dlg.GetPathName());
			MessageBox(sError, "Error", MB_ICONEXCLAMATION);
			return;
		}

		//write out and close the file
		OutFile << (const char*)(sLogText);
		OutFile.close();
	}	
}

void CLoadLTADlg::OnTimer(UINT nIDEvent) 
{
	CEdit* pEdit = ((CEdit*)GetDlgItem(IDC_LOADLOG));
	pEdit->SetSel(-1, 0, TRUE);

	if(nIDEvent == TIMER_ID)
	{
		BOOL bDone = IsDone();

		//see if we need to update the load log
		//(the check is done so that when it is running, we don't refresh it
		//continually)
		if(m_bPrevLoaderDone == FALSE)
		{
			UpdateLoadLog();
		}

		//see if the finishing state has finished at all
		if(bDone != m_bPrevLoaderDone)
		{
			((CButton*)GetDlgItem(IDC_SAVELOADLOG))->EnableWindow(bDone);
			((CButton*)GetDlgItem(IDOK))->EnableWindow(bDone);
			m_bPrevLoaderDone = bDone;
		}

		// automatically close the dialog on load completion
		if( m_bAutoConfirm && bDone && m_bPrevLoaderDone )
		{
			OnOK();
		}
	}
	
	CDialog::OnTimer(nIDEvent);
}


//Thread safe way to add text onto the end of the load log
void CLoadLTADlg::AppendLoadLog(const char* pszString)
{
	g_LoadLogPipeCS.Lock(INFINITE);

	sm_sLoadLogPipe += pszString;

	g_LoadLogPipeCS.Unlock();
}

//Thread safe way to set if the loading thread is done
void CLoadLTADlg::SetLoadThreadDone(BOOL bDone)
{
	g_ThreadDoneCS.Lock(INFINITE);

	sm_bLoadThreadDone = bDone;
	
	g_ThreadDoneCS.Unlock();
}


//Thread safe way to update the load log
void CLoadLTADlg::UpdateLoadLog()
{

	CString sTextToAdd;

	g_LoadLogPipeCS.Lock(INFINITE);


	sTextToAdd = sm_sLoadLogPipe;
	sm_sLoadLogPipe = "";

	g_LoadLogPipeCS.Unlock();

	if(!sTextToAdd.IsEmpty())
	{
		CString sLogText;
		CEdit* pEdit = ((CEdit*)GetDlgItem(IDC_LOADLOG));

		pEdit->GetWindowText(sLogText);

		sLogText += sTextToAdd;
	
		pEdit->SetWindowText(sLogText);

		pEdit->LineScroll(pEdit->GetLineCount());
	}
}

//Thread safe way to check the done flag, and update the dialog box
//appropriately
BOOL CLoadLTADlg::IsDone()
{
	BOOL bDone = FALSE;

	g_ThreadDoneCS.Lock(INFINITE);

	bDone = sm_bLoadThreadDone;

	g_ThreadDoneCS.Unlock();

	return bDone;

}

BOOL CLoadLTADlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	SetTimer(TIMER_ID, 10, NULL);

	//set the text on the dialog to reflect the filename that is being loaded
	CString sText;
	sText.Format("Loading: %s", m_sFilename);
	((CStatic*)GetDlgItem(IDC_LOADFILENAME))->SetWindowText(sText);


	//set the dialog box text to whatever is already in the string
	UpdateLoadLog();

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLoadLTADlg::OnDestroy() 
{
	KillTimer(TIMER_ID);

	CDialog::OnDestroy();
	
}

void CLoadLTADlg::OnOK() 
{
	//don't let them quit while the thread is running
	if(IsDone() == FALSE)
		return;
	
	CDialog::OnOK();
}

void CLoadLTADlg::OnCancel() 
{
	//don't let them quit while the thread is running
	if(IsDone() == FALSE)
		return;
	
	CDialog::OnCancel();
}

//Thread safe way to clear the load log
void CLoadLTADlg::ClearLoadLog()
{
	g_LoadLogPipeCS.Lock(INFINITE);

	sm_sLoadLogPipe = "";

	g_LoadLogPipeCS.Unlock();
}

