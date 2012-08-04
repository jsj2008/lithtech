//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_PROPPAGEOPTIONSRUN_H__889A7432_F8FE_11D2_BE1C_0060971BDC6D__INCLUDED_)
#define AFX_PROPPAGEOPTIONSRUN_H__889A7432_F8FE_11D2_BE1C_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropPageOptionsRun.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsRun dialog

class CPropPageOptionsRun : public CPropertyPage
{
	DECLARE_DYNCREATE(CPropPageOptionsRun)

// Construction
public:
	CPropPageOptionsRun();
	~CPropPageOptionsRun();

// Dialog Data
	//{{AFX_DATA(CPropPageOptionsRun)
	enum { IDD = IDD_PROPPAGE_RUN };
	CString	m_sExecutable;
	CString	m_sProgramArguments;
	CString	m_sWorkingDirectory;
	//}}AFX_DATA

	// Saves/Loads the options
	BOOL		m_bLoaded;
	void		LoadOptions();
	void		SaveOptions();

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPropPageOptionsRun)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPropPageOptionsRun)
	afx_msg void OnButtonBrowse();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPPAGEOPTIONSRUN_H__889A7432_F8FE_11D2_BE1C_0060971BDC6D__INCLUDED_)
