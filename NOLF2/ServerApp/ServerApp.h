// ServerApp.h : main header file for the Server application
//

#if !defined(AFX_SERVERAPP_H__C6916245_FA1F_11D0_B46B_00A024805738__INCLUDED_)
#define AFX_SERVERAPP_H__C6916245_FA1F_11D0_B46B_00A024805738__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

/////////////////////////////////////////////////////////////////////////////
// CServerApp:
// See ServerApp.cpp for the implementation of this class
//

class CServerApp : public CWinApp
{
public:
	CServerApp(LPCTSTR lpszAppName = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CServerApp)
	public:
	virtual BOOL InitInstance();
	virtual int Run();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CServerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:

};

CServerApp* GetTheApp();

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SERVERAPP_H__C6916245_FA1F_11D0_B46B_00A024805738__INCLUDED_)
