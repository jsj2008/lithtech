
#include "stdafx.h"
#include "PackerDlg.h"

BEGIN_MESSAGE_MAP(CPackerDlg, CDialog)
    //{{AFX_MSG_MAP(CPackerDlg)
	ON_BN_CLICKED(IDC_CANCELREQ, OnCancelReq)
	ON_BN_CLICKED(IDC_INPUTFILE_BROWSE1, OnInputFileBrowse1)
	ON_BN_CLICKED(IDC_INPUTFILE_BROWSE2, OnInputFileBrowse2)
	ON_BN_CLICKED(IDC_INPUTFILE_BROWSE3, OnInputFileBrowse3)
	ON_BN_CLICKED(IDC_INPUTFILE_BROWSE4, OnInputFileBrowse4)
	ON_BN_CLICKED(IDC_INPUTFILE_BROWSE5, OnInputFileBrowse5)
	ON_BN_CLICKED(IDC_INPUTFILE_BROWSE6, OnInputFileBrowse6)
	ON_BN_CLICKED(IDC_INPUTFILE_BROWSE7, OnInputFileBrowse7)
	ON_BN_CLICKED(IDC_INPUTFILE_BROWSE8, OnInputFileBrowse8)
	ON_BN_CLICKED(IDC_OUTPUTFILE_BROWSE, OnOutputFileBrowse)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_CBN_SELCHANGE(IDC_PLATFORMPICKER, OnPlatformPicker)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CPackerDlg::OnInitDialog()
{
	InitializeDialogControls();				// Grab the Dialog's controls...

	return true;
}



// Grab the Dialog's controls...
extern float g_fScale;
void CPackerDlg::InitializeDialogControls()
{
	SetWindowPos(NULL,100,200,0,0,SWP_NOOWNERZORDER|SWP_NOSIZE);

	m_pInputFilename[0]						= (CStatic*)GetDlgItem(IDC_INPUTFILENAME1);					assert(m_pInputFilename[0]);
	m_pInputFilename[1]						= (CStatic*)GetDlgItem(IDC_INPUTFILENAME2);					assert(m_pInputFilename[1]);
	m_pInputFilename[2]						= (CStatic*)GetDlgItem(IDC_INPUTFILENAME3);					assert(m_pInputFilename[2]);
	m_pInputFilename[3]						= (CStatic*)GetDlgItem(IDC_INPUTFILENAME4);					assert(m_pInputFilename[3]);
	m_pInputFilename[4]						= (CStatic*)GetDlgItem(IDC_INPUTFILENAME5);					assert(m_pInputFilename[4]);
	m_pInputFilename[5]						= (CStatic*)GetDlgItem(IDC_INPUTFILENAME6);					assert(m_pInputFilename[5]);
	m_pInputFilename[6]						= (CStatic*)GetDlgItem(IDC_INPUTFILENAME7);					assert(m_pInputFilename[6]);
	m_pInputFilename[7]						= (CStatic*)GetDlgItem(IDC_INPUTFILENAME8);					assert(m_pInputFilename[7]);
	m_pOutputFilename						= (CStatic*)GetDlgItem(IDC_OUTPUTFILENAME);					assert(m_pOutputFilename);
	m_pProgress								= (CProgressCtrl*)GetDlgItem(IDC_PROGRESS);					assert(m_pProgress);
	m_pCancelReq							= (CButton*)GetDlgItem(IDC_CANCELREQ);						assert(m_pCancelReq);
	m_pInputFilenameBrowse[0]				= (CButton*)GetDlgItem(IDC_INPUTFILE_BROWSE1);				assert(m_pInputFilenameBrowse[0]);
	m_pInputFilenameBrowse[1]				= (CButton*)GetDlgItem(IDC_INPUTFILE_BROWSE2);				assert(m_pInputFilenameBrowse[1]);
	m_pInputFilenameBrowse[2]				= (CButton*)GetDlgItem(IDC_INPUTFILE_BROWSE3);				assert(m_pInputFilenameBrowse[2]);
	m_pInputFilenameBrowse[3]				= (CButton*)GetDlgItem(IDC_INPUTFILE_BROWSE4);				assert(m_pInputFilenameBrowse[3]);
	m_pInputFilenameBrowse[4]				= (CButton*)GetDlgItem(IDC_INPUTFILE_BROWSE5);				assert(m_pInputFilenameBrowse[4]);
	m_pInputFilenameBrowse[5]				= (CButton*)GetDlgItem(IDC_INPUTFILE_BROWSE6);				assert(m_pInputFilenameBrowse[5]);
	m_pInputFilenameBrowse[6]				= (CButton*)GetDlgItem(IDC_INPUTFILE_BROWSE7);				assert(m_pInputFilenameBrowse[6]);
	m_pInputFilenameBrowse[7]				= (CButton*)GetDlgItem(IDC_INPUTFILE_BROWSE8);				assert(m_pInputFilenameBrowse[7]);
	m_pOutputFilenameBrowse					= (CButton*)GetDlgItem(IDC_OUTPUTFILE_BROWSE);				assert(m_pOutputFilenameBrowse);
	m_pStart								= (CButton*)GetDlgItem(IDC_START);							assert(m_pStart);
	m_pDontCloseWhenDone					= (CButton*)GetDlgItem(IDC_DONTCLOSEWHENDONE);
	m_pPlatformPicker						= (CComboBox*)GetDlgItem(IDC_PLATFORMPICKER);

	m_pPlatformPicker->InsertString(0, "D3D PC");
//	m_pPlatformPicker->InsertString(1, "PS2" );
	//m_pPlatformPicker->AddString("D3D PC");
	//m_pPlatformPicker->AddString("PS2");
	switch (g_PackType)
	{
		case LTB_D3D_RENDERSTYLE_FILE:	m_pPlatformPicker->SetCurSel(0); m_PlatformPicked = D3D_PC;	break;
//		case LTB_PS2_RENDERSTYLE_FILE:	m_pPlatformPicker->SetCurSel(1); m_PlatformPicked = PS2;	break;
		default:						m_pPlatformPicker->SetCurSel(0); m_PlatformPicked = D3D_PC;	break;
	}
	
	if (m_pProgress)						{ m_pProgress->SetPos(0); }
	if (m_bIGetToStartIt)					{ for (uint32 i = 0; i < 8; ++i) { m_pInputFilenameBrowse[i]->EnableWindow(); } m_pOutputFilenameBrowse->EnableWindow(); m_pStart->EnableWindow(); }
	for (uint32 i = 0; i < 8; ++i)			{ if (m_pInputFilename[i]) { m_pInputFilename[i]->SetWindowText(g_InputFile[i].c_str()); } }
	if (m_pOutputFilename)					{ m_pOutputFilename->SetWindowText(g_OutputFile.c_str()); }
}
