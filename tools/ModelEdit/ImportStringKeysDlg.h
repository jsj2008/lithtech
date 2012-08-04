#if !defined(AFX_IMPORTSTRINGKEYSDLG_H__F706B3D5_A32B_4687_B522_ABE46A8C6BD8__INCLUDED_)
#define AFX_IMPORTSTRINGKEYSDLG_H__F706B3D5_A32B_4687_B522_ABE46A8C6BD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ImportStringKeysDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CImportStringKeysDlg dialog

class CImportStringKeysDlg : public CDialog
{
// Construction
public:
	CImportStringKeysDlg(CWnd* pParent = NULL);   // standard constructor

	void Clear();
	void AddMsg(const char *sMsg, BYTE r, BYTE g, BYTE b);

// Dialog Data
	//{{AFX_DATA(CImportStringKeysDlg)
	enum { IDD = IDD_IMPSTRINGKEYS };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImportStringKeysDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CImportStringKeysDlg)
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMPORTSTRINGKEYSDLG_H__F706B3D5_A32B_4687_B522_ABE46A8C6BD8__INCLUDED_)
