// ImportSpellsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "spelled.h"
#include "ImportSpellsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CImportSpellsDlg dialog

CImportSpellsDlg::CImportSpellsDlg(CString sDicFile, CWnd* pParent /*=NULL*/)
	: CDialog(CImportSpellsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CImportSpellsDlg)
	//}}AFX_DATA_INIT

	m_sDicFile = sDicFile;
}

//------------------------------------------------------------------
//
//   FUNCTION : DoDataExchange()
//
//   PURPOSE  : Data exchange function
//
//------------------------------------------------------------------

void CImportSpellsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CImportSpellsDlg)
	DDX_Control(pDX, IDC_SPELLS, m_spells);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate)
	{
		CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();

		// We are loading the dialog

		pApp->LoadSpellmgr(m_sDicFile.GetBuffer(m_sDicFile.GetLength()), &m_spellMgr);

		// Run through the spells and add them into the list box

		CLinkListNode<CSpell *> *pNode = m_spellMgr.GetSpells()->GetHead();

		while (pNode)
		{
			int nIndex = m_spells.AddString(pNode->GetData()->GetName());
			m_spells.SetItemData(nIndex, (DWORD)pNode->GetData());
			
			pNode = pNode->m_pNext;
		}
	}
	else
	{
		// We are unloading the dialog

		int *pSel = new int [m_spells.GetCount()];
		int nSel = m_spells.GetSelItems(m_spells.GetCount(), pSel);

		for (int i = 0; i < nSel; i ++)
		{
			CSpell *pSpell = (CSpell *)m_spells.GetItemData(pSel[i]);
			m_selectedSpells.AddTail(pSpell);
		}

		delete pSel;
	}
}


BEGIN_MESSAGE_MAP(CImportSpellsDlg, CDialog)
	//{{AFX_MSG_MAP(CImportSpellsDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CImportSpellsDlg message handlers
