//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// AddObjectDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAddObjectDlg dialog

class CAddObjectDlg : public CDialog
{
// Construction
public:
	CAddObjectDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAddObjectDlg)
	enum { IDD = IDD_ADDOBJECTDLG };
	CTreeCtrl	m_ObjectTree;
	//}}AFX_DATA


	CString		m_SelectedTypeName;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAddObjectDlg)
	public:
	virtual int DoModal();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAddObjectDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
