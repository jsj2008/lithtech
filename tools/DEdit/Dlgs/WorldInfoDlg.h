//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// WorldInfoDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWorldInfoDlg dialog


class CEditRegion;


class CWorldInfoDlg : public CDialog
{
// Construction
public:
	CWorldInfoDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CWorldInfoDlg)
	enum { IDD = IDD_WORLDINFO };
	CString	m_WorldName;
	CString	m_NumPolies;
	CString	m_NumPoints;
	CString	m_NumBrushes;
	CString	m_WorldInfoString;
	//}}AFX_DATA


	CEditRegion		*m_pRegion;
	CString			m_InputWorldName;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorldInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CWorldInfoDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
