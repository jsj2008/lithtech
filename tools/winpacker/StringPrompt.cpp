// StringPrompt.cpp : implementation file
//

#include "stdafx.h"
#include "winpacker.h"
#include "StringPrompt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStringPrompt dialog


CStringPrompt::CStringPrompt(CWnd* pParent /*=NULL*/)
	: CDialog(CStringPrompt::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStringPrompt)
	m_sPrompt = _T("");
	m_sString = _T("");
	//}}AFX_DATA_INIT
}


void CStringPrompt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStringPrompt)
	DDX_Text(pDX, IDC_STATIC_PROMPT, m_sPrompt);
	DDX_Text(pDX, IDC_EDIT_STRING_VAL, m_sString);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStringPrompt, CDialog)
	//{{AFX_MSG_MAP(CStringPrompt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStringPrompt message handlers

BOOL CStringPrompt::OnInitDialog() 
{
	CDialog::OnInitDialog();

	//set the title if the user specified one
	if(!m_sTitle.IsEmpty())
		SetWindowText(m_sTitle);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
