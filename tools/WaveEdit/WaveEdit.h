// WaveEdit.h : main header file for the WAVEEDIT application
//

#if !defined(AFX_WAVEEDIT_H__0440E644_29C3_11D3_B781_444553540000__INCLUDED_)
#define AFX_WAVEEDIT_H__0440E644_29C3_11D3_B781_444553540000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWaveEditApp:
// See WaveEdit.cpp for the implementation of this class
//

class CWaveEditApp : public CWinApp
{
public:
	CWaveEditApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaveEditApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CWaveEditApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVEEDIT_H__0440E644_29C3_11D3_B781_444553540000__INCLUDED_)
