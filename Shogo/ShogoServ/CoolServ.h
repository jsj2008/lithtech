// CoolServ.h : main header file for the COOLSERV application
//

#if !defined(AFX_COOLSERV_H__C6916245_FA1F_11D0_B46B_00A024805738__INCLUDED_)
#define AFX_COOLSERV_H__C6916245_FA1F_11D0_B46B_00A024805738__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols


/////////////////////////////////////////////////////////////////////////////
// CCoolServApp:
// See CoolServ.cpp for the implementation of this class
//

class CCoolServApp : public CWinApp
{
public:
	CCoolServApp(LPCTSTR lpszAppName = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCoolServApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCoolServApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	BOOL AddResources(CStringList& collRezFiles);

};

CCoolServApp* GetTheApp();

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COOLSERV_H__C6916245_FA1F_11D0_B46B_00A024805738__INCLUDED_)
