
#include "stdafx.h"
#include "UpdateDlg.h"

BEGIN_MESSAGE_MAP(CUpdateDlg, CDialog)
    //{{AFX_MSG_MAP(CUpdateDlg)
	ON_BN_CLICKED(IDC_CANCELREQ, OnCancelReq)
	ON_BN_CLICKED(IDC_INPUTFILE_BROWSE, OnInputFileBrowse)
	ON_BN_CLICKED(IDC_OUTPUTFILE_BROWSE, OnOutputFileBrowse)
	ON_EN_CHANGE(IDC_SCALEEDIT, OnScale)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_CBN_SELCHANGE(IDC_SOURCECHANNEL, OnSourceChannel)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CUpdateDlg::OnInitDialog()
{
	InitializeDialogControls();				// Grab the Dialog's controls...

	return true;
}

// Grab the Dialog's controls...
extern float g_fScale;
void CUpdateDlg::InitializeDialogControls()
{
	SetWindowPos(NULL,100,200,0,0,SWP_NOOWNERZORDER|SWP_NOSIZE);

	m_pInputFilename						= (CStatic*)GetDlgItem(IDC_INPUTFILENAME);					assert(m_pInputFilename);
	m_pOutputFilename						= (CStatic*)GetDlgItem(IDC_OUTPUTFILENAME);					assert(m_pOutputFilename);
	m_pScaleEdit							= (CEdit*)GetDlgItem(IDC_SCALEEDIT);						assert(m_pScaleEdit);
	m_pSourceChannel						= (CComboBox*)GetDlgItem(IDC_SOURCECHANNEL);				assert(m_pSourceChannel);
	m_pProgress								= (CProgressCtrl*)GetDlgItem(IDC_PROGRESS);					assert(m_pProgress);
	m_pCancelReq							= (CButton*)GetDlgItem(IDC_CANCELREQ);						assert(m_pCancelReq);
	m_pInputFilenameBrowse					= (CButton*)GetDlgItem(IDC_INPUTFILE_BROWSE);				assert(m_pInputFilenameBrowse);
	m_pOutputFilenameBrowse					= (CButton*)GetDlgItem(IDC_OUTPUTFILE_BROWSE);				assert(m_pOutputFilenameBrowse);
	m_pStart								= (CButton*)GetDlgItem(IDC_START);							assert(m_pStart);
	m_pDontCloseWhenDone					= (CButton*)GetDlgItem(IDC_DONTCLOSEONDONE);				assert(m_pDontCloseWhenDone);

	if (m_pProgress)						{ m_pProgress->SetPos(0); }
	if (m_pScaleEdit)						{ char szTmp[32]; sprintf(szTmp,"%5.2f",g_fScale); m_pScaleEdit->SetWindowText(szTmp); }
	if (m_bIGetToStartIt)					{ m_pInputFilenameBrowse->EnableWindow(); m_pOutputFilenameBrowse->EnableWindow(); m_pStart->EnableWindow(); }
	if (m_pInputFilename)					{ m_pInputFilename->SetWindowText(g_InputFile.c_str()); }
	if (m_pOutputFilename)					{ m_pOutputFilename->SetWindowText(g_OutputFile.c_str()); }
	if (m_pSourceChannel)					{ m_pSourceChannel->AddString("Alpha"); m_pSourceChannel->AddString("Red"); m_pSourceChannel->AddString("Green"); m_pSourceChannel->AddString("Blue"); m_pSourceChannel->SetCurSel((int)g_HeightSource); }
}

