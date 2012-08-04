// Install.h : main header file for the INSTALL application
//

#if !defined(AFX_INSTALL_H__B4C83A97_5E6C_4195_9E26_B05711C11B88__INCLUDED_)
#define AFX_INSTALL_H__B4C83A97_5E6C_4195_9E26_B05711C11B88__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CInstallApp:
// See Install.cpp for the implementation of this class
//

class CInstallApp : public CWinApp
{
public:
	CInstallApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInstallApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CInstallApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSTALL_H__B4C83A97_5E6C_4195_9E26_B05711C11B88__INCLUDED_)
