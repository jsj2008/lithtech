#if !defined(AFX_SPELLSDLG_H__0EFD8085_6F5A_11D2_8245_0060084EFFD8__INCLUDED_)
#define AFX_SPELLSDLG_H__0EFD8085_6F5A_11D2_8245_0060084EFFD8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SpellsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSpellsDlg dialog

class CSpellsDlg : public CDialog
{
// Construction
public:
	CSpellsDlg(CWnd* pParent = NULL);   // standard constructor
	~CSpellsDlg();

	public :

		CSpell					  **m_pSpells;
		int							m_nSpells;
















// Dialog Data
	//{{AFX_DATA(CSpellsDlg)
	enum { IDD = IDD_SPELLLIST };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellsDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpellsDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLSDLG_H__0EFD8085_6F5A_11D2_8245_0060084EFFD8__INCLUDED_)
