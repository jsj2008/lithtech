#if !defined(AFX_ADDSOCKETDLG_H__11D24D73_D9A7_11D2_9EF1_00A0C9696F4D__INCLUDED_)
#define AFX_ADDSOCKETDLG_H__11D24D73_D9A7_11D2_9EF1_00A0C9696F4D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AddSocketDlg.h : header file
//

class CModelEditDlg;

/////////////////////////////////////////////////////////////////////////////
// AddSocketDlg dialog

class AddSocketDlg : public CDialog
{
// Construction
public:
	AddSocketDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(AddSocketDlg)
	enum { IDD = IDD_ADDSOCKET };
	CString	m_SocketName;
	CString	m_NodeName;
	//}}AFX_DATA


	CModelEditDlg	*m_pDlg;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AddSocketDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AddSocketDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADDSOCKETDLG_H__11D24D73_D9A7_11D2_9EF1_00A0C9696F4D__INCLUDED_)
