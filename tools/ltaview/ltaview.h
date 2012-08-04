// ltaview.h : main header file for the LTAVIEW application
//

#if !defined(AFX_LTAVIEW_H__E7DC485C_48DD_45E4_8468_27E61960DCEE__INCLUDED_)
#define AFX_LTAVIEW_H__E7DC485C_48DD_45E4_8468_27E61960DCEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CLTAViewApp:
// See ltaview.cpp for the implementation of this class
//

class CLTAViewApp : public CWinApp
{
public:
	CLTAViewApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLTAViewApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CLTAViewApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LTAVIEW_H__E7DC485C_48DD_45E4_8468_27E61960DCEE__INCLUDED_)
