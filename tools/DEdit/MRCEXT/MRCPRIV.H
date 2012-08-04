// MRCEXT: Micro Focus Extension DLL for MFC 2.1+
// Copyright (C)1994-5	Micro Focus Inc, 2465 East Bayshore Rd, Palo Alto, CA 94303.
// 
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation. In addition, you may also charge for any
//  application	using MRCEXT, and are under no obligation to supply source
//  code. You must accredit Micro Focus Inc in the "About Box", or banner
//  of your application. 
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should also have received a copy of the GNU General Public License with this
//  software, also indicating additional rights you have when using MRCEXT.  
//
//
// MRCPRIV.H
// $Date:   25 Jun 1996 19:43:08  $
// $Revision:   1.1  $
// $Author:   MRC  $
// MRCPRIV.H
//
// Private within extension module.

#ifndef __MRCPRIV_H__
#define __MRCPRIV_H__

#include "oldtypes.h"

//#undef AFX_DATA
//#define AFX_DATA AFX_EXT_DATA


#ifdef _DEBUG
//#define _VERBOSE_TRACE
#endif


#define REG_VERSION    		"Version"
#define REG_VERSION_NO		2		// current version of docking state


/////////////////////////////////////////////////////////////////////////////
typedef struct _ROWSIZEINFO
{
    int nFlexWidth;                 // space taken up by flexible bars
    int nFixedWidth;                // space taken up by fixed size bars
    int nMaxHeight;                 // max height taken  (both flexible and fixed height bars)
    int nMaxFixedHeight;    // max height taken by a fixed size bar.
    int nFlexBars;                  // number of sized bars
    int nTotalBars;                 // total no of bars in the row.
	int nTotalWidth;					// fixed + flex width
} ROWSIZEINFO;


/////////////////////////////////////////////////////////////////////////////
// CSplitterRect class - simple representation of splitter rectangles
class CSplitterRect : public CObject
{
public:
        CRect   m_rect;         // rectangle
        char    m_type;         // Vertical or Horizontal
        int             m_nPos;         // position at which it was inserted
                                                // ie points to pane immediately following it.

#define SPLITTER_VERT   1
#define SPLITTER_HORZ   2

	CSplitterRect(int type, const RECT & rect);
	void CSplitterRect::Draw(CDC *pDC);
#ifdef _DEBUG
	void Dump( CDumpContext& dc ) const;
#endif
};

class CMRCSizeControlBar;

/////////////////////////////////////////////////////////////////////////////
// CSizeDockBar window
class CSizeDockBar : public CDockBar
{
	friend class CDragDockContext;
	// Construction
public:
	DECLARE_DYNAMIC(CSizeDockBar)
	CSizeDockBar();		// TRUE if inside MDI frame

	// Attributes
public:
    CObArray        m_SplitArr;             // array of CSplitterRect's constructed by ReCalcLayout

    CSplitterRect * m_pSplitCapture;        // capture splitter rect (if any)
    HCURSOR         m_hcurLast;                             // last cursor type

    // following items used to detect when DockBar has changed since last time, so we
    // can re-arrange the rows if parent resizes, or bars are docked/floated/hidden
    CSize           m_LayoutSize;   // size used in previous layout
    int             m_CountBars;    // no of bars for previous layout - need to go better than this..
	CPtrArray		m_arrHiddenBars;		// array of currently invisible bars

// Operations
public:
    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
    void TileDockedBars();
	void AdjustForNewBar(CControlBar *pBar);		
	BOOL WasBarHidden(CControlBar *pBar);
	
protected:
	// Splitter rectangles...
    CSplitterRect * GetSplitter(int i)
                { return ((CSplitterRect *)(m_SplitArr[i])); };
    void AddSplitterRect(int type, int x1, int y1, int x2, int y2, int nPos);
    void DeleteSplitterRects();
    void SetSplitterSizeInRange(int start, int type, int length);
    CSplitterRect * SetHitCursor(CPoint pt);
    CSplitterRect * HitTest(CPoint pt);
    void StartTracking(CPoint pt);

        // Navigating rows
    int StartPosOfRow(int nPos);
    int StartPosOfPreviousRow(int nPos);

        // Resizing rows
    BOOL IsRowSizeable(int nPos);

    void GetRowSizeInfo(int nPos,  ROWSIZEINFO * pRZI, const CPtrArray & arrBars);
    BOOL AdjustAllRowSizes(int nNewSize);
    BOOL AdjustRowSizes(int nPos, int nNewSize, CPtrArray & arrBars);
    void TileDockedBarsRow(int nPos);

    int ShrinkRowToLeft(int nPos, int nAmount, BOOL bApply, int * pnFlex = NULL);
    int ShrinkRowToRight(int nPos, int nAmount, BOOL bApply, int * pnFlex = NULL);
    int CheckSumBars() const;

public:
    // Dragging
	int TestInsertPosition(CControlBar* pBarIns, CRect rect);
	int BarsOnThisRow(CControlBar *pBarIns, CRect rect);
        
    // Miscellaneous
    CMRCSizeControlBar * GetFirstControlBar ();

    BOOL IsBarHorizontal()
                { return (m_dwStyle & (CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM)); };

#define IsSizeable(pBar) (BOOL) ((CControlBar *) pBar)->IsKindOf(RUNTIME_CLASS(CMRCSizeControlBar))

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSizeDockBar)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CSizeDockBar();
#ifdef _DEBUG	
	virtual void Dump( CDumpContext& dc ) const;
#endif


    // Generated message map functions
protected:
    //{{AFX_MSG(CSizeDockBar)
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    //}}AFX_MSG
    afx_msg LRESULT OnSizeParent(WPARAM, LPARAM);
    DECLARE_MESSAGE_MAP()
};



class CDragDockContext : public CDockContext
{
// Attributes
public:
    CRect m_rectDragDock;                           // rectangle indicating where we'll dock

protected:
	CRect m_rectDragHorzAlone;		
	CRect m_rectDragVertAlone;
	CSizeDockBar * m_pTargetDockBar;
	CPoint	m_ptStart;

// Construction
public:
    CDragDockContext(CControlBar* pBar);

// Operations
    virtual void StartDrag(CPoint pt);	// only thing called externally
	virtual void ToggleDocking();		// called to toggle docking

    void Move(CPoint pt);       // called when mouse has moved
    void EndDrag();             // drop
    void CancelDrag();          // drag cancelled
    void OnKey(int nChar, BOOL bDown);

// Implementation
public:
    ~CDragDockContext();
    BOOL Track();
    void DrawFocusRect(BOOL bRemoveRect = FALSE);
    void UpdateState(BOOL* pFlag, BOOL bNewValue);
    DWORD CanDock();
    CDockBar* GetDockBar();
};


/////////////////////////////////////////////////////////////////////////////
// CSizeDockFrame window
class CSizableMiniDockFrameWnd : public CMiniDockFrameWnd
{

friend CDragDockContext; 		// access to IgnoreSysMove flag
private:
    DECLARE_DYNCREATE(CSizableMiniDockFrameWnd)

    //{{AFX_VIRTUAL(CSizableMiniDockFrameWnd)
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

// Attributes
public:
	enum ContainedBarType { Unknown, MFCBase, MRCSizeBar } ;
	enum ContainedBarType GetContainedBarType();
	
protected:
	enum ContainedBarType m_nContainedBarType;	

    
	//{{AFX_MSG(CSizableMiniDockFrameWnd)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnClose();
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint pt);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
//}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CMRCMDIFloatWnd frame - used when floating a control bar in an MDI client frame
/////////////////////////////////////////////////////////////////////////////

#define CMDIFloat_Parent CMDIChildWnd		// in case I change my mind on derivation !

class CMRCMDIFloatWnd : public CMDIFloat_Parent
{
	DECLARE_DYNCREATE(CMRCMDIFloatWnd)
protected:
	CMRCMDIFloatWnd();           // protected constructor used by dynamic creation

public:
	CDockBar	m_wndMDIDockBar;		

// Attributes
public:

// Operations
public:
	virtual BOOL Create(CWnd* pParent, DWORD dwBarStyle);

// Overrides
	void RecalcLayout(BOOL bNotify = TRUE);
	BOOL PreCreateWindow(CREATESTRUCT& cs); 
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMRCMDIFloatWnd)
	protected:
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMRCMDIFloatWnd();

	// Generated message map functions
	//{{AFX_MSG(CMRCMDIFloatWnd)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


void AdjustForBorders(CRect& rect, DWORD dwStyle);

void * AllocObjExtData(CObject * pObj, int cBytes);
void * GetObjExtDataPtr(CObject * pObj);
void DeleteObjExtData(CObject * pObj);

//#undef AFX_DATA
//#define AFX_DATA
#endif
