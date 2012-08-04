//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_IMPORTOBJECTDLG_H__8FDB7364_E21A_11D2_BE0B_0060971BDC6D__INCLUDED_)
#define AFX_IMPORTOBJECTDLG_H__8FDB7364_E21A_11D2_BE0B_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportObjectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImportObjectDlg dialog

class CImportObjectDlg : public CDialog
{
// Construction
public:
	CImportObjectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CImportObjectDlg)
	enum { IDD = IDD_IMPORT_OBJECT };
	int		m_nUpdateRadio;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportObjectDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportObjectDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTOBJECTDLG_H__8FDB7364_E21A_11D2_BE0B_0060971BDC6D__INCLUDED_)
