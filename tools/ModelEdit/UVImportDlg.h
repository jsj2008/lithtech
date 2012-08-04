#if !defined(AFX_UVIMPORTDLG_H__D3D08261_C807_11D1_A7F5_006097726515__INCLUDED_)
#define AFX_UVIMPORTDLG_H__D3D08261_C807_11D1_A7F5_006097726515__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// UVImportDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// UVImportDlg dialog

class UVImportDlg : public CDialog
{
// Construction
public:
	UVImportDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(UVImportDlg)
	enum { IDD = IDD_UVIMPORT2 };
	CString	m_AnimationName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(UVImportDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(UVImportDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UVIMPORTDLG_H__D3D08261_C807_11D1_A7F5_006097726515__INCLUDED_)
