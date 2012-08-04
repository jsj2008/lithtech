#if !defined(AFX_PROPPAGEOPTIONSCLIPBOARD_H__9E2D3203_0D52_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_PROPPAGEOPTIONSCLIPBOARD_H__9E2D3203_0D52_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropPageOptionsClipboard.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropPageOptionsClipboard dialog

class CPropPageOptionsClipboard : public CPropertyPage
{
// Construction
public:
	DECLARE_DYNCREATE(CPropPageOptionsClipboard)
	CPropPageOptionsClipboard();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPropPageOptionsClipboard)
	enum { IDD = IDD_PROPPAGE_CLIPBOARD };
	BOOL	m_bDisplayReportOfChanges;
	BOOL	m_bGenerateUniqueNames;
	BOOL	m_bUpdateRefProps;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropPageOptionsClipboard)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPropPageOptionsClipboard)
	afx_msg void OnCheckGenerateUniqueNames();
	afx_msg void OnCheckUpdateReferencingProperties();
	afx_msg void OnCheckDisplayNameChangeReport();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Update the enabled status of the controls
	void		UpdateEnabledStatus();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPPAGEOPTIONSCLIPBOARD_H__9E2D3203_0D52_11D3_BE24_0060971BDC6D__INCLUDED_)
