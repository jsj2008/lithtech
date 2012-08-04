//------------------------------------------------------------------
//
//  FILE      : MainFrm.h
//
//  PURPOSE   :	interface of the CMainFrame class
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#pragma once



class CMainFrame : public CFrameWnd
{
protected:

	// create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

public:

	virtual ~CMainFrame();


	virtual BOOL 		PreCreateWindow(CREATESTRUCT& cs);

#ifdef _DEBUG
	virtual void 		AssertValid() const;
	virtual void 		Dump(CDumpContext& dc) const;
#endif

protected:

	// control bar embedded members
	CStatusBar  		m_wndStatusBar;
	CToolBar    		m_wndToolBar;

protected:

	afx_msg int 		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void 		OnUpdateImageInfo(CCmdUI *pCmdUI);

	DECLARE_MESSAGE_MAP()
};



#endif // __MAINFRM_H__
