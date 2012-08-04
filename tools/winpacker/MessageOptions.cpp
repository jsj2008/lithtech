// MessageOptions.cpp : implementation file
//

#include "stdafx.h"
#include "winpacker.h"
#include "MessageOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMessageOptions dialog


CMessageOptions::CMessageOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CMessageOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMessageOptions)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CMessageOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMessageOptions)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessageOptions, CDialog)
	//{{AFX_MSG_MAP(CMessageOptions)
	ON_LBN_SELCHANGE(IDC_LIST_MESSAGE_SEVERITY, OnSeverityChanged)
	ON_BN_CLICKED(IDC_BUTTON_COLOR, OnButtonColor)
	ON_EN_CHANGE(IDC_EDIT_PREFIX, OnChangePrefix)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMessageOptions message handlers

void CMessageOptions::OnSeverityChanged() 
{
	//get the ID of the new selection
	CListBox* pList = ((CListBox*)GetDlgItem(IDC_LIST_MESSAGE_SEVERITY));

	int nSel = pList->GetCurSel();

	if(nSel == LB_ERR)
		return;

	//update the color and text
	m_ColorWell.m_red	= GetRValue(m_Colors[nSel]);
	m_ColorWell.m_green = GetGValue(m_Colors[nSel]);
	m_ColorWell.m_blue	= GetBValue(m_Colors[nSel]);
	m_ColorWell.Invalidate();

	((CEdit*)GetDlgItem(IDC_EDIT_PREFIX))->SetWindowText(m_sPrefixes[nSel]);	
}

void CMessageOptions::OnButtonColor() 
{
	//get the ID of the new selection
	CListBox* pList = ((CListBox*)GetDlgItem(IDC_LIST_MESSAGE_SEVERITY));

	int nSel = pList->GetCurSel();

	if(nSel == LB_ERR)
		return;

	CColorDialog Dlg(m_Colors[nSel], 0, this);

	if(Dlg.DoModal())
	{
		m_Colors[nSel] = Dlg.GetColor();

		m_ColorWell.m_red	= GetRValue(m_Colors[nSel]);
		m_ColorWell.m_green = GetGValue(m_Colors[nSel]);
		m_ColorWell.m_blue	= GetBValue(m_Colors[nSel]);
		m_ColorWell.Invalidate();
	}	
}

void CMessageOptions::OnChangePrefix() 
{
	//save the string
	CListBox* pList = ((CListBox*)GetDlgItem(IDC_LIST_MESSAGE_SEVERITY));

	int nSel = pList->GetCurSel();

	if(nSel == LB_ERR)
		return;

	((CEdit*)GetDlgItem(IDC_EDIT_PREFIX))->GetWindowText(m_sPrefixes[nSel]);	
	
}

BOOL CMessageOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//setup the list to contain all the items
	CListBox* pList = ((CListBox*)GetDlgItem(IDC_LIST_MESSAGE_SEVERITY));
	pList->InsertString(MSG_CRITICAL,	"Critical");
	pList->InsertString(MSG_ERROR,		"Error");
	pList->InsertString(MSG_WARNING,	"Warning");
	pList->InsertString(MSG_NORMAL,		"Normal");
	pList->InsertString(MSG_VERBOSE,	"Verbose");
	pList->InsertString(MSG_DEBUG,		"Debug");

	//select the first item in the list
	pList->SetCurSel(MSG_CRITICAL);
	
	//setup the data for all the properties
	m_ColorWell.m_red	= GetRValue(m_Colors[MSG_CRITICAL]);
	m_ColorWell.m_green = GetGValue(m_Colors[MSG_CRITICAL]);
	m_ColorWell.m_blue	= GetBValue(m_Colors[MSG_CRITICAL]);
	((CEdit*)GetDlgItem(IDC_EDIT_PREFIX))->SetWindowText(m_sPrefixes[MSG_CRITICAL]);

	//setup the color well
	m_ColorWell.SubclassDlgItem( IDC_BUTTON_COLOR, this );

	//setup the tooltips
	m_ToolTip.Create(this);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_LIST_MESSAGE_SEVERITY), IDS_TOOLTIP_MSGOPT_SEVLIST);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_EDIT_PREFIX), IDS_TOOLTIP_MSGOPT_PREFIX);
	m_ToolTip.AddWindowTool(GetDlgItem(IDC_BUTTON_COLOR), IDS_TOOLTIP_MSGOPT_COLOR);
	
	return TRUE;  
}
