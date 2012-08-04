// ResourceLocator.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "ResourceLocator.h"
#include "spellmgr.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceLocator dialog


CResourceLocator::CResourceLocator(CString sName, CLinkList<SPELLNAME> *pList, CWnd* pParent /*=NULL*/)
	: CDialog(CResourceLocator::IDD, pParent)
{
	//{{AFX_DATA_INIT(CResourceLocator)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pList = pList;
	m_sName = sName;
}


void CResourceLocator::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CResourceLocator)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CResourceLocator, CDialog)
	//{{AFX_MSG_MAP(CResourceLocator)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceLocator message handlers

BOOL CResourceLocator::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	CListBox *pBox = (CListBox *)GetDlgItem(IDC_RESOURCELIST);

	GetDlgItem(IDC_FOUNDRESOURCE)->SetWindowText((char *)(LPCSTR)m_sName);

	CLinkListNode<SPELLNAME> *pNode = m_pList->GetHead();

	while (pNode)
	{
		pBox->AddString(pNode->m_Data.m_sName);

		pNode = pNode->m_pNext;
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
