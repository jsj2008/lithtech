//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#if !defined(AFX_OPTIONSSHEET_H__731D64A2_43FD_11D1_A408_006097098780__INCLUDED_)
#define AFX_OPTIONSSHEET_H__731D64A2_43FD_11D1_A408_006097098780__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// OptionsSheet.h : header file
//

#include "undopage.h"
#include "proppagedisplay.h"
#include "proppageoptionsd3d.h"
#include "proppageoptionsrun.h"
#include "proppageoptionsclipboard.h"
#include "proppageoptionsmodels.h"
#include "proppageoptionscontrols.h"
#include "proppageoptionsmisc.h"
#include "proppageoptionsprefabs.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsSheet

class COptionsSheet : public CPropertySheet
{
	DECLARE_DYNAMIC(COptionsSheet)

// Construction
public:
	COptionsSheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	COptionsSheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:
	CUndoPage					m_undoPage;
	CPropPageDisplay			m_displayPage;
	CPropPageOptionsD3D			m_d3dPage;
	CPropPageOptionsRun			m_runPage;
	CPropPageOptionsClipboard	m_clipboardPage;
	CPropPageOptionsModels		m_modelsPage;
	CPropPageOptionsControls	m_controlsPage;
	CPropPageOptionsMisc		m_miscPage;
	CPropPageOptionsPrefabs		m_prefabsPage;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsSheet)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~COptionsSheet();

	// Generated message map functions
protected:
	//{{AFX_MSG(COptionsSheet)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:

	void CommonConstructor();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OPTIONSSHEET_H__731D64A2_43FD_11D1_A408_006097098780__INCLUDED_)
