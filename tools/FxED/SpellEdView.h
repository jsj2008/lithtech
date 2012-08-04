// SpellEdView.h : interface of the CSpellEdView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPELLEDVIEW_H__A74036AE_6F17_11D2_8245_0060084EFFD8__INCLUDED_)
#define AFX_SPELLEDVIEW_H__A74036AE_6F17_11D2_8245_0060084EFFD8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// Includes....

#include "SpellDefTab.h"
#include "Spell.h"

class CSpellEdView : public CView
{
protected: // create from serialization only
	CSpellEdView();
	DECLARE_DYNCREATE(CSpellEdView)

	public :

		// Member Functions

		BOOL						Init(CSpell *pSpell) { m_pSpell = pSpell; return TRUE; }

		// Accessors

		CSpell*						GetSpell() { return m_pSpell; }

	private :

		// Member Variables

//		CSpellDefTab				m_spellDefTab;
		CSpell					   *m_pSpell;
		CTrackWnd					m_trackWnd;











// Attributes
public:
	CSpellEdDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellEdView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSpellEdView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSpellEdView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFileSave();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSaveAs();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SpellEdView.cpp
inline CSpellEdDoc* CSpellEdView::GetDocument()
   { return (CSpellEdDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLEDVIEW_H__A74036AE_6F17_11D2_8245_0060084EFFD8__INCLUDED_)
