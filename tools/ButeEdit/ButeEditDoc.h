// ButeEditDoc.h : interface of the CButeEditDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_BUTEEDITDOC_H__EA15072C_0EDC_11D3_BE24_0060971BDC6D__INCLUDED_)
#define AFX_BUTEEDITDOC_H__EA15072C_0EDC_11D3_BE24_0060971BDC6D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ButeMgr.h"

class CButeEditDoc : public CDocument
{
protected: // create from serialization only
	CButeEditDoc();
	DECLARE_DYNCREATE(CButeEditDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CButeEditDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CButeEditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CButeEditDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	// Access to the attribute manager
	CButeMgr		*GetButeMgr()		{ return m_pButeMgr; }

	// Access to the tags in the bute file
	int				GetNumTags()			{ return m_tagArray.GetSize(); }
	CString			GetTag(int nIndex)		{ return m_tagArray[nIndex]; }
	BOOL			AddTag(CString sTag);

	// Returns the name of the currently selected tag
	CString			GetSelectedTag()				{ return m_sSelectedTag; }

	// Sets the name of the currently selected tag
	void			SetSelectedTag(CString sTag)	{ m_sSelectedTag=sTag; }

	// Call this function to modify a specific key/value in every tag
	void			ReplaceAllKeyValues(CString sKey, CButeMgr::SymTypes type, void *pValueData);

protected:
	// The callback function to retrieve the tags in the bute file
	static bool		GetTagsCallback(const char* szTag, void* pAux);

protected:
	CButeMgr		*m_pButeMgr;		// The attribute manager object
	CStringArray	m_tagArray;			// The tags in the bute file
	CString			m_sSelectedTag;		// The currently selected tag
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUTEEDITDOC_H__EA15072C_0EDC_11D3_BE24_0060971BDC6D__INCLUDED_)
