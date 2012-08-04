#if !defined(AFX_SEQUENCERWND_H__2CDEAF84_9394_11D2_9B4C_0060971BDAD8__INCLUDED_)
#define AFX_SEQUENCERWND_H__2CDEAF84_9394_11D2_9B4C_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SequencerWnd.h : header file
//

// Includes....

#include "TimeBarDlg.h"
#include "PhaseInfoDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSequencerWnd window

class CSequencerWnd : public CWnd
{
// Construction
public:
	CSequencerWnd();

	public :

		// Member Functions

		BOOL							Init(CTimeBarDlg *pTimeBarDlg, CPhaseInfoDlg *pPhaseInfoDlg);

		void							DrawSequencerWnd(CDC *pDC);

		// Accessors

	private :

		// Member Variables

		CDC							   *m_pMemDC;
		CBitmap						   *m_pBitmap;
		CBitmap						   *m_pOldBitmap;

		int								m_cx;
		int								m_cy;

		CTimeBarDlg					   *m_pTimeBarDlg;
		CPhaseInfoDlg				   *m_pPhaseInfoDlg;



// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSequencerWnd)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSequencerWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSequencerWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEQUENCERWND_H__2CDEAF84_9394_11D2_9B4C_0060971BDAD8__INCLUDED_)
