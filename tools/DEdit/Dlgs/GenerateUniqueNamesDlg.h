#if !defined(AFX_GENERATEUNIQUENAMESDLG_H__E41795D4_0717_11D3_BE21_0060971BDC6D__INCLUDED_)
#define AFX_GENERATEUNIQUENAMESDLG_H__E41795D4_0717_11D3_BE21_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GenerateUniqueNamesDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGenerateUniqueNamesDlg dialog

class CGenerateUniqueNamesDlg : public CDialog
{
// Construction
public:
	CGenerateUniqueNamesDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGenerateUniqueNamesDlg)
	enum { IDD = IDD_GENERATE_UNIQUE_NAMES };
	BOOL	m_bUpdateRefProps;
	BOOL	m_bUpdateSelPropsOnly;
	BOOL	m_bDisplayReportOfChanges;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGenerateUniqueNamesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGenerateUniqueNamesDlg)
	afx_msg void OnCheckUpdateReferencingProperties();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENERATEUNIQUENAMESDLG_H__E41795D4_0717_11D3_BE21_0060971BDC6D__INCLUDED_)
