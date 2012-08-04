// SpellEdView.cpp : implementation of the CSpellEdView class
//

#include "stdafx.h"
#include "SpellEd.h"

#include "SpellEdDoc.h"
#include "SpellEdView.h"
#include "ChildFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSpellEdView

IMPLEMENT_DYNCREATE(CSpellEdView, CView)

BEGIN_MESSAGE_MAP(CSpellEdView, CView)
	//{{AFX_MSG_MAP(CSpellEdView)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SAVE_AS, OnFileSaveAs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellEdView construction/destruction

CSpellEdView::CSpellEdView()
{
	m_pSpell = NULL;
}

CSpellEdView::~CSpellEdView()
{
}

BOOL CSpellEdView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

/////////////////////////////////////////////////////////////////////////////
// CSpellEdView drawing

void CSpellEdView::OnDraw(CDC* pDC)
{
}

/////////////////////////////////////////////////////////////////////////////
// CSpellEdView diagnostics

#ifdef _DEBUG
void CSpellEdView::AssertValid() const
{
	CView::AssertValid();
}

void CSpellEdView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CSpellEdDoc* CSpellEdView::GetDocument() // non-debug version is inline
{
	return (CSpellEdDoc *)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSpellEdView message handlers

//------------------------------------------------------------------
//
//   FUNCTION : OnInitialUpdate()
//
//   PURPOSE  : Performs initial updating of the view
//
//------------------------------------------------------------------

void CSpellEdView::OnInitialUpdate() 
{	
	CRect rcClient;
	GetClientRect(&rcClient);

	CView::OnInitialUpdate();

	// Create the spell def tab

	DWORD dwStyle = WS_CHILD | WS_VISIBLE;
	DWORD dwExStyle = 0;//WS_EX_CLIENTEDGE | WS_EX_WINDOWEDGE;
	
	m_trackWnd.SetPhase(m_pSpell->GetActivePhase());
	m_trackWnd.SetSpell(m_pSpell);
	m_trackWnd.CreateEx(dwExStyle, NULL, "Cast Track Window", dwStyle, 0, 0, rcClient.Width(), rcClient.Height(), this->GetSafeHwnd(), NULL);

//	m_spellDefTab.Create(WS_VISIBLE | WS_CHILD, rcClient, this, 0x123);

	// Add the tabs

//	m_spellDefTab.Init();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnEraseBkgnd()
//
//   PURPOSE  : Handles WM_ERASEBKGND
//
//------------------------------------------------------------------

BOOL CSpellEdView::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnSize()
//
//   PURPOSE  : Handles WM_SIZE
//
//------------------------------------------------------------------

void CSpellEdView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);
	
	if (IsWindow(m_trackWnd.GetSafeHwnd()))
	{
		CRect rcClient;
		GetClientRect(&rcClient);

		m_trackWnd.MoveWindow(0, 0, rcClient.Width(), rcClient.Height());
	}	
}

//------------------------------------------------------------------
//
//   FUNCTION : OnCreate()
//
//   PURPOSE  : Handles WM_CREATE
//
//------------------------------------------------------------------

int CSpellEdView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	CChildFrame *pFrame = (CChildFrame *)GetParent();

	if (pFrame)
	{
		pFrame->SetView(this);
	}
	
	return 0;
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFileSave()
//
//   PURPOSE  : Handles saving
//
//------------------------------------------------------------------

void CSpellEdView::OnFileSave() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	pApp->OnFileSave();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFileOpen()
//
//   PURPOSE  : Handles loading
//
//------------------------------------------------------------------

void CSpellEdView::OnFileOpen() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	pApp->OnFileOpen();
}

//------------------------------------------------------------------
//
//   FUNCTION : OnFileSaveAs()
//
//   PURPOSE  : Handles saving under a different name
//
//------------------------------------------------------------------

void CSpellEdView::OnFileSaveAs() 
{
	CSpellEdApp *pApp = (CSpellEdApp *)AfxGetApp();
	pApp->OnFileSave();
}
