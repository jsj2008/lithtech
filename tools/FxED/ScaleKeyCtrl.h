#if !defined(AFX_SCALEKEYCTRL_H__0C360702_8F62_11D2_9B4A_0060971BDAD8__INCLUDED_)
#define AFX_SCALEKEYCTRL_H__0C360702_8F62_11D2_9B4A_0060971BDAD8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScaleKeyCtrl.h : header file
//

// Includes....

#include "KeyControl.h"

/////////////////////////////////////////////////////////////////////////////
// CScaleKeyCtrl window

class CScaleKeyCtrl : public CKeyControl
{
// Construction
public:
	CScaleKeyCtrl();

	// Member Functions

	CString							GetTrackValue(CLinkListNode<KEY> *pNode);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScaleKeyCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CScaleKeyCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CScaleKeyCtrl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCALEKEYCTRL_H__0C360702_8F62_11D2_9B4A_0060971BDAD8__INCLUDED_)
