// LTDMTestDlg.h : header file
//

#if !defined(AFX_LTDMTESTDLG_H__6E144E62_B32D_4FAE_B79B_ACBD1A057B35__INCLUDED_)
#define AFX_LTDMTESTDLG_H__6E144E62_B32D_4FAE_B79B_ACBD1A057B35__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define LT_OK 0
//#define DRESULT unsigned long
//#define LTRESULT unsigned long

#include "stdlith.h"
//#include "ltcompat.h"
#include "ltdirectmusic_impl.h"

/////////////////////////////////////////////////////////////////////////////
// CLTDMTestDlg dialog

class CLTDMTestDlg : public CDialog
{
// Construction
public:
	CLTDMTestDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CLTDMTestDlg)
	enum { IDD = IDD_LTDMTEST_DIALOG };
	CComboBox	m_comboDebugOutputLevel;
	CRichEditCtrl	m_richEditDebugOutput;
	CComboBox	m_comboIntensity;
	CComboBox	m_comboMotifName;
	CComboBox	m_comboMotifEnact;
	CComboBox	m_comboSecondaryEnact;
	CComboBox	m_comboSecondarySegment;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLTDMTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	CLTDirectMusicMgr m_LTDMMgr;
	BOOL m_bInLevel;
	BOOL m_bInitialized;
	void TermAll();

	// Generated message map functions
	//{{AFX_MSG(CLTDMTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnInitLevel();
	afx_msg void OnStopMusic();
	afx_msg void OnPlayMusic();
	afx_msg void OnExit();
	afx_msg void OnChangeIntensity();
	afx_msg void OnVolume();
	afx_msg void OnPlaySecondary();
	afx_msg void OnStopSecondary();
	afx_msg void OnPlayMotif();
	afx_msg void OnStopMotif();
	afx_msg void OnDebugOutputLevel();
	afx_msg void OnClose();
	afx_msg void OnTermLevel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LTDMTESTDLG_H__6E144E62_B32D_4FAE_B79B_ACBD1A057B35__INCLUDED_)
