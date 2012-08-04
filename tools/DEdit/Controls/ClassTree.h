//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __CLASSTREE_H__
#define __CLASSTREE_H__


	// ClassTree.h : header file
	//

	/////////////////////////////////////////////////////////////////////////////
	// CClassTree window


	class CProjectMgr;
	class CProjectClass;


	#include "ltserverobj.h"


	class CClassTree : public CTreeCtrl
	{
	// Construction
	public:
		CClassTree();

	// Attributes
	public:

	// Operations
	public:

		void				UpdateContents(BOOL bIncludeTemplates=FALSE);
		void				AddItemsToTree( HTREEITEM hParent, CMoArray<CProjectClass*> &classes );		
		
		void				SelectClass( const char *pClass );
		HTREEITEM			RecurseAndFindClass( HTREEITEM hItem, const char *pName );

		// Returns the selected class
		CString				GetSelectedClass();

	// Overrides
		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CClassTree)
		protected:
		virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
		//}}AFX_VIRTUAL

	// Implementation
	public:
		virtual ~CClassTree();

		// Generated message map functions
	protected:
		//{{AFX_MSG(CClassTree)
			// NOTE - the ClassWizard will add and remove member functions here.
		//}}AFX_MSG

		DECLARE_MESSAGE_MAP()
	};

	/////////////////////////////////////////////////////////////////////////////


#endif  // __CLASSTREE_H__
