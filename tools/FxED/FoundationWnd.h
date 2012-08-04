#if !defined(AFX_FOUNDATIONWND_H__996684C4_95CE_11D2_9B4C_0060971BDAD8__INCLUDED_)
#define AFX_FOUNDATIONWND_H__996684C4_95CE_11D2_9B4C_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FoundationWnd.h : header file
//

// Includes....

#include "TimeBarDlg.h"
#include "PhaseInfoDlg.h"
#include "SequencerWnd.h"

/////////////////////////////////////////////////////////////////////////////
// CFoundationWnd window

class CFoundationWnd : public CWnd
{
// Construction
public:
	CFoundationWnd();

	public :

		// Member Functions

		// Accessors

	private :

		// Member Variables

		CTimeBarDlg						m_timeBarDlg;
		CPhaseInfoDlg					m_phaseInfoDlg;
		CSequencerWnd					m_sequencerWnd;
















// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFoundationWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFoundationWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFoundationWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FOUNDATIONWND_H__996684C4_95CE_11D2_9B4C_0060971BDAD8__INCLUDED_)
