#if !defined(AFX_LOADLTADIALOG_H__59E81ED8_4AEF_4EAC_AE7A_1CC4CB104193__INCLUDED_)
#define AFX_LOADLTADIALOG_H__59E81ED8_4AEF_4EAC_AE7A_1CC4CB104193__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LoadLTADialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLoadLTADialog dialog

class CLoadLTADialog : public CDialog
{
// Construction
public:
	CLoadLTADialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLoadLTADialog)
	enum { IDD = IDD_LTALOADING };
	CString	m_sFileName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoadLTADialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLoadLTADialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOADLTADIALOG_H__59E81ED8_4AEF_4EAC_AE7A_1CC4CB104193__INCLUDED_)
