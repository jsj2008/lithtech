// SpellEdDoc.h : interface of the CSpellEdDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPELLEDDOC_H__A74036AC_6F17_11D2_8245_0060084EFFD8__INCLUDED_)
#define AFX_SPELLEDDOC_H__A74036AC_6F17_11D2_8245_0060084EFFD8__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


class CSpellEdDoc : public CDocument
{
protected: // create from serialization only
	CSpellEdDoc();
	DECLARE_DYNCREATE(CSpellEdDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellEdDoc)
	public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSpellEdDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CSpellEdDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SPELLEDDOC_H__A74036AC_6F17_11D2_8245_0060084EFFD8__INCLUDED_)
