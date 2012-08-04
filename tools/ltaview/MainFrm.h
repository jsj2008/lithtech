// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__15169C01_4CCB_4D9B_A8A2_81A462F000EB__INCLUDED_)
#define AFX_MAINFRM_H__15169C01_4CCB_4D9B_A8A2_81A462F000EB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ltatreemgr.h"

class CMainFrame : public CFrameWnd
{
	
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	void OpenFileCmdLine ( LPTSTR lpCmdLine);
// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFileClose();
	afx_msg void OnFileOpen();
	afx_msg void OnUpdateFileClose(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	enum	{ID_LTATREE	= 10101};

	CLTATreeMgr		m_TreeMgr;
	HICON			m_hTitleIcon;
	BOOL			m_bDocOpen;



};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__15169C01_4CCB_4D9B_A8A2_81A462F000EB__INCLUDED_)
