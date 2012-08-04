// WinLTC.h : main header file for the WINLTC application
//

#if !defined(AFX_WINLTC_H__1366B9FF_3681_45A4_A369_EF6296CCA69B__INCLUDED_)
#define AFX_WINLTC_H__1366B9FF_3681_45A4_A369_EF6296CCA69B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWinLTCApp:
// See WinLTC.cpp for the implementation of this class
//

class CWinLTCApp : public CWinApp
{
public:
	CWinLTCApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWinLTCApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWinLTCApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WINLTC_H__1366B9FF_3681_45A4_A369_EF6296CCA69B__INCLUDED_)
