// ModelEdit.h : main header file for the MODELEDIT application
//

#if !defined(AFX_MODELEDIT_H__B6667225_2E9F_11D1_9462_0020AFF7CDC1__INCLUDED_)
#define AFX_MODELEDIT_H__B6667225_2E9F_11D1_9462_0020AFF7CDC1__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CModelEditApp:
// See ModelEdit.cpp for the implementation of this class
//
#define WM_STARTIDLE	(WM_USER + 565)
#define WM_EDITAPPLY	(WM_USER + 566)

class CModelEditApp : public CWinApp
{
public:
	CModelEditApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CModelEditApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CModelEditApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODELEDIT_H__B6667225_2E9F_11D1_9462_0020AFF7CDC1__INCLUDED_)
