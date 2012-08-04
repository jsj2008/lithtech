// LTDMTest.h : main header file for the LTDMTEST application
//

#if !defined(AFX_LTDMTEST_H__EB39A5CB_B2CC_4ABE_BBF7_0BBC1E849253__INCLUDED_)
#define AFX_LTDMTEST_H__EB39A5CB_B2CC_4ABE_BBF7_0BBC1E849253__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CLTDMTestApp:
// See LTDMTest.cpp for the implementation of this class
//

class CLTDMTestApp : public CWinApp
{
public:
	CLTDMTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLTDMTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CLTDMTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LTDMTEST_H__EB39A5CB_B2CC_4ABE_BBF7_0BBC1E849253__INCLUDED_)
