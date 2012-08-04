// SetFOVDlg.cpp : implementation file
//

#include "precompile.h"
#include "stdafx.h"
#include "modeledit.h"
#include "SetFOVDlg.h"
#include "RenderWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSetFOVDlg dialog


CSetFOVDlg::CSetFOVDlg(CRenderWnd *host, CWnd* pParent /*=NULL*/)
	: CDialog(CSetFOVDlg::IDD, pParent)
{
	m_baseFOV = 45 ;
	m_startVal = MIN_FOV ;
	m_endVal = MAX_FOV ;

	//{{AFX_DATA_INIT(CSetFOVDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Host = host ;
}


void CSetFOVDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetFOVDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSetFOVDlg, CDialog)
	//{{AFX_MSG_MAP(CSetFOVDlg)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_FOV_EDIT, OnEditChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSetFOVDlg message handlers

void CSetFOVDlg::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class

	// m_Host-> NotifyOfDiagDeath();
	delete this ;	
	CDialog::PostNcDestroy();
}

void CSetFOVDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	// TODO: Add your message handler code here and/or call default
	// tell host value has changed.	

	int nControl = pScrollBar->GetDlgCtrlID();

	if( nControl == IDC_FOV_SLIDER )
	{
		CSliderCtrl* pCtrl = GetSliderFOV();
		ASSERT( pCtrl != NULL );
		int nScPos = pCtrl->GetPos();
		char buf[256];
		sprintf(buf,"%d",nScPos);
		GetEditFOV()->SetWindowText( buf );
		m_Host->SetFOV( nScPos ) ;
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


BOOL CSetFOVDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	// TODO: Add extra initialization here
	CSliderCtrl *pSlider = GetSliderFOV();
	pSlider->SetRange(m_startVal, m_endVal );
	pSlider->SetPos(  m_baseFOV );

	//make it so there are 10 tics on the slider (the +9 is to round up)
	pSlider->SetTicFreq((m_endVal - m_startVal + 9) / 10);

	CString sVal;
	sVal.Format( "%d", m_baseFOV );
	GetEditFOV()->SetWindowText( sVal );

	//setup the min/max/mid strings above the scroll bar
	sVal.Format("%d", m_startVal);
	((CStatic*)GetDlgItem(IDC_MIN))->SetWindowText(sVal);

	sVal.Format("%d", m_endVal);
	((CStatic*)GetDlgItem(IDC_MAX))->SetWindowText(sVal);

	sVal.Format("%d", (m_startVal + m_endVal) / 2);
	((CStatic*)GetDlgItem(IDC_MID))->SetWindowText(sVal);

	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CEdit* CSetFOVDlg::GetEditFOV()
{
	return static_cast<CEdit*>(GetDlgItem(IDC_FOV_EDIT));
}

CSliderCtrl* CSetFOVDlg::GetSliderFOV()
{
	return static_cast<CSliderCtrl*>(GetDlgItem(IDC_FOV_SLIDER)); 
}


void CSetFOVDlg::OnEditChange()
{
	//get the string in the edit box
	CString sFOV;
	GetEditFOV()->GetWindowText(sFOV);

	//trim unneeded spaces
	sFOV.TrimLeft();
	sFOV.TrimRight();

	//convert to an integer
	int nFOV = atoi(sFOV);

	//set the slider to reflect the edit box
	GetSliderFOV()->SetPos(nFOV);
}