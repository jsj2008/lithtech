#if !defined(AFX_PHASEINFODLG_H__F2D178A6_73FD_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_PHASEINFODLG_H__F2D178A6_73FD_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PhaseInfoDlg.h : header file
//

// Includes....

#include "IntButton.h"
#include "TimeBarDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CPhaseInfoDlg dialog

class CPhaseInfoDlg : public CDialog
{
// Construction
public:
	CPhaseInfoDlg(CWnd* pParent = NULL);   // standard constructor

	public :

		// Member Functions

		void						Refresh();
		
		void						SetTimeBar(CTimeBarDlg *pDlg) { m_pTimeBar = pDlg; }
		void						ValueFn();

	private :

		// Member Variables

		CTimeBarDlg				   *m_pTimeBar;
		int							m_msTotalTime;
		int							m_nKeyRepeat;

// Dialog Data
	//{{AFX_DATA(CPhaseInfoDlg)
	enum { IDD = IDD_PHASEINFO };
	CIntButton	m_phaseLength;
	CIntButton	m_keyRepeats;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPhaseInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPhaseInfoDlg)
	afx_msg void OnValueUpdate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PHASEINFODLG_H__F2D178A6_73FD_11D2_9B4A_0060971BDAD8__INCLUDED_)
