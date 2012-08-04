//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// CScaleSelectDlg.cpp : implementation file
//

#include "bdefs.h"
#include "dedit.h"
#include "scaleselectdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScaleSelectDlg dialog


CScaleSelectDlg::CScaleSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CScaleSelectDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CScaleSelectDlg)
	m_nScaleXAmount = 100;
	m_nScaleYAmount = 100;
	m_nScaleZAmount = 100;
	m_nScaleMode	= SCALE_PERCENT;
	m_bKeepUniform	= FALSE;
	m_bKeepTextures = TRUE;
	//}}AFX_DATA_INIT

	m_HasFocus		= -1;
	m_LastUpdated	= -1;
}

/************************************************************************/
// Updates the enabled/disabled status of the controls
void CScaleSelectDlg::UpdateEnabledStatus()
{
	UpdateData();

	//if we are keeping the values uniform, we should not allow the entry into
	//the bottom two fields
	BOOL bEnable = (m_bKeepUniform) ? FALSE : TRUE;

	GetDlgItem(IDC_SCALEYAMOUNT)->EnableWindow(bEnable);
	GetDlgItem(IDC_SCALEZAMOUNT)->EnableWindow(bEnable);

	//if we are keeping everything uniform, the X box should update the values in the
	//Y and Z boxes
	if(m_bKeepUniform)
	{
		switch(m_nScaleMode) 
		{
		case SCALE_PERCENT:
			//need to just keep the numbers the same
			m_nScaleYAmount = m_nScaleXAmount;
			m_nScaleZAmount = m_nScaleXAmount;

			break;

		case SCALE_WORLD_UNITS:
			{
				//find the percent ratio of the X change
				float fXScale = m_nScaleXAmount / (float)m_SelectionSize->x;

				//need to keep the percent scale the same
				m_nScaleYAmount = (int)(fXScale * m_SelectionSize->y);
				m_nScaleZAmount = (int)(fXScale * m_SelectionSize->z);
			}
			break;
		}

		UpdateData(FALSE);
	}

}

void CScaleSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScaleSelectDlg)
	DDX_Text (pDX, IDC_SCALEXAMOUNT, m_nScaleXAmount);
	DDX_Text (pDX, IDC_SCALEYAMOUNT, m_nScaleYAmount);
	DDX_Text (pDX, IDC_SCALEZAMOUNT, m_nScaleZAmount);
	DDX_Check(pDX, IDC_UNIFORMVALUES, m_bKeepUniform);
	DDX_Check(pDX, IDC_KEEPTEXTURES, m_bKeepTextures);
	DDX_Radio(pDX, IDC_RADIO_PERCENTAGE, m_nScaleMode);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScaleSelectDlg, CDialog)
	//{{AFX_MSG_MAP(CScaleSelectDlg)
	ON_EN_CHANGE(IDC_SCALEXAMOUNT, OnXAmount)
	ON_EN_CHANGE(IDC_SCALEYAMOUNT, OnYAmount)
	ON_EN_CHANGE(IDC_SCALEZAMOUNT, OnZAmount)
	ON_BN_CLICKED(IDC_RADIO_PERCENTAGE, OnChangeScaleMode)
	ON_BN_CLICKED(IDC_RADIO_WORLDUNITS, OnChangeScaleMode)
	ON_BN_CLICKED(IDC_UNIFORMVALUES, UpdateEnabledStatus)
	ON_BN_CLICKED(IDOK, OnOK)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScaleSelectDlg message handlers

BOOL CScaleSelectDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Update the enabled status
	UpdateEnabledStatus();

	return TRUE;  
}


void CScaleSelectDlg::OnChangeScaleMode() 
{
	int nPreviousState = m_nScaleMode;

	UpdateData();

	// don't convert the content if we didn't really change modes
	if (nPreviousState == m_nScaleMode)  
		return;  

	switch (m_nScaleMode) 
	{
	case SCALE_PERCENT:
		//need to convert from world units to percent
		m_nScaleXAmount = m_nScaleXAmount * 100 / (int)m_SelectionSize->x;
		m_nScaleYAmount = m_nScaleYAmount * 100 / (int)m_SelectionSize->y;
		m_nScaleZAmount = m_nScaleZAmount * 100 / (int)m_SelectionSize->z;

		break;

	case SCALE_WORLD_UNITS:

		//need to convert from percent to world units
		m_nScaleXAmount = m_nScaleXAmount * (int)m_SelectionSize->x / 100;
		m_nScaleYAmount = m_nScaleYAmount * (int)m_SelectionSize->y / 100;
		m_nScaleZAmount = m_nScaleZAmount * (int)m_SelectionSize->z / 100;

		break;

	}

	UpdateData(FALSE);
	UpdateEnabledStatus();
}


void CScaleSelectDlg::OnXAmount()
{
	UpdateEnabledStatus();
}

void CScaleSelectDlg::OnYAmount()
{
	UpdateEnabledStatus();
}

void CScaleSelectDlg::OnZAmount()
{
	UpdateEnabledStatus();
}


void CScaleSelectDlg::OnOK() 
{

	UpdateData();

	switch(m_nScaleMode)
	{
	case SCALE_PERCENT:
	
		m_SelectionSize->x = (m_nScaleXAmount/100.0f) * m_SelectionSize->x;
		m_SelectionSize->y = (m_nScaleYAmount/100.0f) * m_SelectionSize->y;
		m_SelectionSize->z = (m_nScaleZAmount/100.0f) * m_SelectionSize->z;
		break;

	case SCALE_WORLD_UNITS:

		m_SelectionSize->x = (float)m_nScaleXAmount;
		m_SelectionSize->y = (float)m_nScaleYAmount;
		m_SelectionSize->z = (float)m_nScaleZAmount;
		break;
	}

	CDialog::OnOK();
}
