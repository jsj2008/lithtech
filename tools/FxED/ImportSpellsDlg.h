#if !defined(AFX_IMPORTSPELLSDLG_H__735A8807_B13D_11D2_9B4E_0060971BDAD8__INCLUDED_)
#define AFX_IMPORTSPELLSDLG_H__735A8807_B13D_11D2_9B4E_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportSpellsDlg.h : header file
//

// Includes....

#include "SpellMgr.h"

/////////////////////////////////////////////////////////////////////////////
// CImportSpellsDlg dialog

class CImportSpellsDlg : public CDialog
{
// Construction
public:
	CImportSpellsDlg(CString sDicFile, CWnd* pParent = NULL);   // standard constructor

	// Member Variables

	CString							m_sDicFile;
	CSpellMgr						m_spellMgr;
	CLinkList<CSpell*>				m_selectedSpells;










// Dialog Data
	//{{AFX_DATA(CImportSpellsDlg)
	enum { IDD = IDD_IMPORTSPELLS };
	CListBox	m_spells;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportSpellsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportSpellsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTSPELLSDLG_H__735A8807_B13D_11D2_9B4E_0060971BDAD8__INCLUDED_)
