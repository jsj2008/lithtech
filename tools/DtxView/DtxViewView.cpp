//------------------------------------------------------------------
//
//  FILE      : DtxViewView.cpp
//
//  PURPOSE   :	implementation of the CDtxViewView class
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#include "stdafx.h"
#include "DtxView.h"

#include "DtxViewDoc.h"
#include "DtxViewView.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


IMPLEMENT_DYNCREATE(CDtxViewView, CView)

BEGIN_MESSAGE_MAP(CDtxViewView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
	ON_COMMAND(ID_VIEW_ALPHACHANNEL, OnViewAlphaChannel)
	ON_COMMAND(ID_VIEW_MIPMAPLEVELS, OnViewMipmapLevels)
END_MESSAGE_MAP()



CDtxViewView::CDtxViewView()
{
}



CDtxViewView::~CDtxViewView()
{
}



BOOL CDtxViewView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}



void CDtxViewView::OnDraw(CDC* pDC)
{
	CDtxViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->DrawTexture(pDC);
}



/*
void CDtxViewView::OnInitialUpdate()
{
	UpdateScrollBars();
    CScrollView::OnInitialUpdate();
}



void CDtxViewView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
{
	UpdateScrollBars();
    CScrollView::OnUpdate(pSender, lHint, pHint);
}



void CDtxViewView::UpdateScrollBars()
{
	const unsigned MinSize = 200;
	CSize DocSize(MinSize, MinSize);

	CDtxViewDoc* pDoc = GetDocument();
	if (NULL != pDoc)
	{
		pDoc->GetDocumentSize(&DocSize);
	}

	if (DocSize.cx < MinSize)
	{
		DocSize.cx = MinSize;
	}
	if (DocSize.cy < MinSize)
	{
		DocSize.cy = MinSize;
	}

	SetScrollSizes(MM_TEXT, DocSize);
    ResizeParentToFit();
}
*/



BOOL CDtxViewView::OnPreparePrinting(CPrintInfo* pInfo)
{
	return DoPreparePrinting(pInfo);
}



void CDtxViewView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}



void CDtxViewView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}



void CDtxViewView::OnViewAlphaChannel()
{
	CDtxViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->ToggleViewAlphaChannel();
	Invalidate(TRUE);
}



void CDtxViewView::OnViewMipmapLevels()
{
	CDtxViewDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	pDoc->ToggleViewMipmapLevels();
	Invalidate(TRUE);
}



#ifdef _DEBUG
void CDtxViewView::AssertValid() const
{
	CView::AssertValid();
}



void CDtxViewView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}



CDtxViewDoc* CDtxViewView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CDtxViewDoc)));
	return (CDtxViewDoc*)m_pDocument;
}
#endif //_DEBUG
