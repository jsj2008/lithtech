// SpellsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SpellEd.h"
#include "SpellsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellsDlg dialog


CSpellsDlg::CSpellsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpellsDlg::IDD, pParent)
{
	m_pSpells = NULL;
	m_nSpells = 0;

	//{{AFX_DATA_INIT(CSpellsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CSpellsDlg::~CSpellsDlg()
{
	delete [] m_pSpells;
}


void CSpellsDlg::DoDataExchange(CDataExchange* pDX)
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	CSpellMgr *pSpellMgr = pApp->GetSpellMgr();

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellsDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		CListBox *pListBox = (CListBox *)GetDlgItem(IDC_SPELLSLIST);

		// Fill the list box with the current spells....

		CLinkListNode<CSpell *> *pNode = pSpellMgr->GetSpells()->GetHead();

		while (pNode)
		{
			int nIndex = pListBox->AddString(pNode->m_Data->GetName());
			pListBox->SetItemData(nIndex, (DWORD)pNode->m_Data);
			
			pNode = pNode->GetNext();
		}
	}
	else
	{
		CListBox *pListBox = (CListBox *)GetDlgItem(IDC_SPELLSLIST);

		int *pSel = new int [pListBox->GetCount()];

		// Retrieve the selected items

		int nSelSpells = pListBox->GetSelItems(pListBox->GetCount(), pSel);
		
		m_pSpells = new CSpell* [nSelSpells];
		m_nSpells = nSelSpells;

		for (int i = 0; i < nSelSpells; i ++)
		{
			CSpell *pSpell = (CSpell *)pListBox->GetItemData(pSel[i]);
			m_pSpells[i] = pSpell;
		}

		// Delete the array....

		delete [] pSel;
	}
}


BEGIN_MESSAGE_MAP(CSpellsDlg, CDialog)
	//{{AFX_MSG_MAP(CSpellsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellsDlg message handlers

BOOL CSpellsDlg::DestroyWindow() 
{
	return CDialog::DestroyWindow();
}
