#if !defined(AFX_TRANSLATIONDLG_H__9C3BEF01_B0DF_11D1_A7E2_006097726515__INCLUDED_)
#define AFX_TRANSLATIONDLG_H__9C3BEF01_B0DF_11D1_A7E2_006097726515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// TranslationDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TranslationDlg dialog

class TranslationDlg : public CDialog
{
// Construction
public:
	TranslationDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(TranslationDlg)
	enum { IDD = IDD_TRANSLATIONDLG };
	CString	m_xTrans;
	CString	m_yTrans;
	CString	m_zTrans;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TranslationDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(TranslationDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRANSLATIONDLG_H__9C3BEF01_B0DF_11D1_A7E2_006097726515__INCLUDED_)
