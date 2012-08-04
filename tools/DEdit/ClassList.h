//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_CLASSLIST_H__52BA82F3_D327_11D2_BDF8_0060971BDC6D__INCLUDED_)
#define AFX_CLASSLIST_H__52BA82F3_D327_11D2_BDF8_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ClassList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CClassList window

class CClassList : public CListBox
{
// Construction
public:
	CClassList();

// Attributes
public:

// Operations
public:
	// Updates the contents of the listbox
	void		UpdateContents(BOOL bShowTemplates);

	// Selects a class in the listbox
	// Returns TRUE if the class could be selected
	BOOL		SelectClass(CString sClass);

	// Gets the selected class from the listbox
	CString		GetSelectedClass();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClassList)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CClassList();

	// Generated message map functions
protected:
	//{{AFX_MSG(CClassList)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLASSLIST_H__52BA82F3_D327_11D2_BDF8_0060971BDC6D__INCLUDED_)
