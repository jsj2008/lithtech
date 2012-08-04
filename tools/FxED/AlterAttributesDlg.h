#if !defined(AFX_ALTERATTRIBUTESDLG_H__13239323_7342_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_ALTERATTRIBUTESDLG_H__13239323_7342_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AlterAttributesDlg.h : header file
//

// Includes....

//#include "AlterAttributes.h"

/////////////////////////////////////////////////////////////////////////////
// CAlterAttributesDlg dialog

class CAlterAttributesDlg : public CDialog
{
// Construction
public:
	CAlterAttributesDlg(CWnd* pParent = NULL);   // standard constructor

	public :

		// Member Functions

		void						SetPtr(CAlterAttributes *pAlterAttributes) { m_pAlterAttributes = pAlterAttributes; }

	private :

		// Member Variables

		CAlterAttributes		   *m_pAlterAttributes;















// Dialog Data
	//{{AFX_DATA(CAlterAttributesDlg)
	enum { IDD = IDD_EF_ALTERATTRIBUTE };
	CComboBox	m_attr;
	CEdit	m_amount;
	CComboBox	m_affects;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAlterAttributesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAlterAttributesDlg)
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelChangeAffects();
	afx_msg void OnChangeAmount();
	afx_msg void OnSelchangeAttr();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ALTERATTRIBUTESDLG_H__13239323_7342_11D2_9B4A_0060971BDAD8__INCLUDED_)
