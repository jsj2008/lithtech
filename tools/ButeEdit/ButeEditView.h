// ButeEditView.h : interface of the CButeEditView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUTEEDITVIEW_H__EA15072E_0EDC_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_BUTEEDITVIEW_H__EA15072E_0EDC_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ButeMgr.h"
class CButeEditView : public CListView
{
protected: // create from serialization only
	CButeEditView();
	DECLARE_DYNCREATE(CButeEditView)

// Attributes
public:
	CButeEditDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButeEditView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // called first time after construct
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CButeEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CButeEditView)
	afx_msg void OnEditModify();
	afx_msg void OnUpdateEditModify(CCmdUI* pCmdUI);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);		
	DECLARE_MESSAGE_MAP()

protected:
	// Builds the list control
	void		BuildListControl();

	// Returns a string that represents the type for a tag and key
	CString		GetTypeString(CString sTag, CString sKey);

	// Returns a string that represents the data for a tag and key
	CString		GetDataString(CString sTag, CString sKey);	

	// Call this to modify the currently selected key
	// Returns: TRUE if the key is modified
	BOOL		ModifySelectedKey();

	// Call this to modify a key at a specific index
	// Returns: TRUE if the key is modified
	BOOL		ModifyKey(int nIndex);

	// Call this to modify a specific tag/key
	// Returns: TRUE if the key is modified
	BOOL		ModifyKey(CString sTag, CString sKey);

	// Gets/Sets whether the "Replace all keys" should be enabled by default
	BOOL		GetReplaceAllKeys();
	void		SetReplaceAllKeys(BOOL bSet);

protected:
	// The callback function to retrieve the tags in the bute file
	static bool			GetKeysCallback(const char* szTag, CButeMgr::CSymTabItem *pItem, void* pAux);

	// This is called by the listbox for sorting
	static int CALLBACK	ListSortCompareFn(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

protected:
	afx_msg void OnContextMenu(CWnd*, CPoint point);
	BOOL			m_bColumnsCreated;	// Indicates if the columns have been created or not
	CStringArray	m_keyArray;			// An array of keys in the view
};

#ifndef _DEBUG  // debug version in ButeEditView.cpp
inline CButeEditDoc* CButeEditView::GetDocument()
   { return (CButeEditDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUTEEDITVIEW_H__EA15072E_0EDC_11D3_BE24_0060971BDC6D__INCLUDED_)
