//------------------------------------------------------------------
//
//  FILE      : DtxViewView.h
//
//  PURPOSE   :	interface of the CDtxViewView class
//
//  COPYRIGHT : (c) 2003 Touchdown Entertainment, Inc. All rights reserved.
//
//------------------------------------------------------------------

#ifndef __DTXVIEWVIEW_H__
#define __DTXVIEWVIEW_H__

#pragma once


//forwards:
class CDtxViewDoc;



class CDtxViewView : public CView //CScrollView
{
protected:

	// create from serialization only
	CDtxViewView();
	DECLARE_DYNCREATE(CDtxViewView)

public:

	virtual ~CDtxViewView();

	CDtxViewDoc* 		GetDocument() const;

	virtual void 		OnDraw(CDC* pDC);
//	virtual void 		OnInitialUpdate();
//	virtual void 		OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL 		PreCreateWindow(CREATESTRUCT& cs);

#ifdef _DEBUG
	virtual void 		AssertValid() const;
	virtual void 		Dump(CDumpContext& dc) const;
#endif

protected:

	virtual BOOL 		OnPreparePrinting(CPrintInfo* pInfo);
	virtual void 		OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void 		OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

	void				UpdateScrollBars();

protected:

	afx_msg void 		OnViewAlphaChannel();
	afx_msg void 		OnViewMipmapLevels();

	DECLARE_MESSAGE_MAP()
};



#ifndef _DEBUG  // debug version in DtxViewView.cpp
inline CDtxViewDoc* CDtxViewView::GetDocument() const
{
	return reinterpret_cast<CDtxViewDoc*>(m_pDocument);
}
#endif // _DEBUG



#endif // __DTXVIEWVIEW_H__
