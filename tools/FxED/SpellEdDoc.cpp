// SpellEdDoc.cpp : implementation of the CSpellEdDoc class
//

#include "stdafx.h"
#include "SpellEd.h"

#include "SpellEdDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellEdDoc

IMPLEMENT_DYNCREATE(CSpellEdDoc, CDocument)

BEGIN_MESSAGE_MAP(CSpellEdDoc, CDocument)
	//{{AFX_MSG_MAP(CSpellEdDoc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellEdDoc construction/destruction

CSpellEdDoc::CSpellEdDoc()
{
}

CSpellEdDoc::~CSpellEdDoc()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSpellEdDoc serialization

void CSpellEdDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
	}
	else
	{
	}
}

/////////////////////////////////////////////////////////////////////////////
// CSpellEdDoc diagnostics

#ifdef _DEBUG
void CSpellEdDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CSpellEdDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSpellEdDoc commands

BOOL CSpellEdDoc::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	return CDocument::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
