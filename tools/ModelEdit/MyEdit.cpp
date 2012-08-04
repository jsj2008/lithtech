// MyEdit.cpp : implementation file
//

#include "precompile.h"
#include "modeledit.h"
#include "myedit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyEdit

CMyEdit::CMyEdit()
{
}


CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	//{{AFX_MSG_MAP(CMyEdit)
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyEdit message handlers

void CMyEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if( nChar == VK_RETURN )
	{
		GetParent( )->SendMessage( WM_EDITAPPLY, WPARAM( m_hWnd ));
		return;
	}

	CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
}
