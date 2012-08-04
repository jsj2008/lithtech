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
/////////////////////////////////////////////////////////////////////////////////////////
// MRCEXT.H
// Header file for MRCEXT.DLL.
// (C)1995 Micro Focus Ltd.
/////////////////////////////////////////////////////////////////////////////////////////
// $Date:   08 Aug 1996 15:29:14  $
// $Revision:   1.4  $
// $Author:   MRC  $
//
// The following macros affect the compilation of this file.
// _MRC_NOFORCE_LIBS   = if defined, .libs are not automatically included
//
// if you've not included AFXCMN.H, the sizeable toolbar controls are not available.

#ifndef __MRCEXT_H__
#define __MRCEXT_H__

#include "afxpriv.h"
#include "mrcpriv.h"

//#define _AFX_NO_OCC_SUPPORT

//#undef AFX_DATA
//#define AFX_DATA AFX_EXT_DATA

// some constants that don't always seem to be in commctrl.h
#ifndef TPM_RETURNCMD
#define TPM_RETURNCMD 0x0100L
#endif



// Window arangement positions
#define CBRS_ARRANGE_TOPLEFT	1
#define CBRS_ARRANGE_TOPRIGHT   2
#define CBRS_ARRANGE_BOTTOMLEFT 4
#define CBRS_ARRANGE_BOTTOMRIGHT 8

#define CBRS_ARRANGE_LEFT  	( CBRS_ARRANGE_TOPLEFT  | CBRS_ARRANGE_BOTTOMLEFT )
#define CBRS_ARRANGE_TOP    ( CBRS_ARRANGE_TOPRIGHT | CBRS_ARRANGE_TOPLEFT )


#define	WM_ADDCONTEXTMENUITEMS	(WM_USER + 1)	// Add to context menu message
#define WM_USER_AFTERFLOAT_MSG	(WM_USER + 2)

// forward references
class CMRCSizeControlBar;
class CSplitterRect;
class CDragDockContext;
class CMRCFrameWndSizeDock;
class CMRCMDIFrameWndSizeDock;

//////////////////////////////////////////////////////////////////////////////////////////////
// cut-down of MFC's AUX_DATA structure
struct MRC_AUX_DATA
{
	BOOL	bWin4;
    int 	cxBorder2, cyBorder2;
    COLORREF clrBtnFace, clrBtnShadow, clrBtnHilite;
    COLORREF clrBtnText, clrWindowFrame;
    HCURSOR hcurWait, hcurArrow, hcurSizeNS, hcurSizeWE;

     MRC_AUX_DATA();                 // constructor does the initialization automatically
    ~MRC_AUX_DATA();
	void MRC_AUX_DATA::UpdateSysColors();
};

#define CX_SPLIT        8               // dimensions of splitter bars
#define CY_SPLIT        8
#define CX_BORDER       1
#define CY_BORDER       1

//-------------------------------------------------------------------
inline BOOL SetWindowSize(CWnd * pWnd, CSize size)
// in-line function for setting window size, without changing Z-order, etc
//-------------------------------------------------------------------
{
	return pWnd->SetWindowPos(NULL, 0, 0, size.cx, size.cy, SWP_NOMOVE /* | SWP_NOREDRAW */ | SWP_NOZORDER);
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Registry functions
//////////////////////////////////////////////////////////////////////////////////////////////
LPVOID MRCGetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, int * pBytesRead = NULL);
BOOL MRCGetProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPVOID Buffer, DWORD nBufferSize);
BOOL MRCWriteProfileBinary(LPCTSTR lpszSection, LPCTSTR lpszEntry, LPVOID Buffer, DWORD nBufferSize);


//////////////////////////////////////////////////////////////////////////////////////////////
// Resizing all child windows within a parent - used by sizeable dialog bar
typedef void * GADGETRESIZEHANDLE;
GADGETRESIZEHANDLE CreateGadgetResizeHandle(CWnd *pWnd);
void DestroyGadgetResizeHandle(GADGETRESIZEHANDLE Handle);
void ResizeGadgetsOnWindow(GADGETRESIZEHANDLE Handle, int cx, int cy);

class CSCBButton
{
public:
    CSCBButton();

    void Move(CPoint ptTo) {ptOrg = ptTo; };
    CRect GetRect() { return CRect(ptOrg, CSize(11, 11)); };
    void Paint(CDC* pDC);

    BOOL    bPushed;
    BOOL    bRaised;

protected:
    CPoint  ptOrg;
};

//////////////////////////////////////////////////////////////////////////////////////////////
// Sizeable control bar - derive from this
//////////////////////////////////////////////////////////////////////////////////////////////

class CMRCSizeControlBar : public CControlBar
{
// Attributes
private:
	static CObArray * m_parrAllocBars;		// array of dynamically allocated bars
public:

	CSize           m_FloatSize;           // size when docked
   	CSize			m_HorzDockSize;			// size when docked horizontal
	CSize			m_VertDockSize;			// size when docked vertical
	//  - size reflects the current docked size of the window (I do nothing clever for stretched)
	//  - height is generally shared across the rows

    CPoint			m_FloatingPosition;		// floating position
	DWORD			m_dwAllowDockingState;  // saved enable style for allow docking on/off
    int             m_Style;                // style flags
    CSize           m_PrevSize;
    BOOL            m_bPrevFloating;
	int				m_nGripperSize;			// The size of the gripper bar

#define SZBARF_DESTROY_ON_CLOSE     1       // closing the floating window closes the control bar
#define SZBARF_AUTOTIDY    			2		// keeps window in an array, so it can be deleted
#define SZBARF_STDMOUSECLICKS		4		// standard mouse handling for the menu
#define SZBARF_DLGAUTOSIZE			8		// auto-size dialog bars
#define SZBARF_ALLOW_MDI_FLOAT	   16		// allow bar to float in an MDI window
#define SZBARF_HIDE_ON_CLOSE	   32		// window hides on the "close" message


#define CBRS_MOVED_BY_USER  	 0x0001L    // set if dragged by user
// Construction
public:
	DECLARE_DYNAMIC(CMRCSizeControlBar)
    CMRCSizeControlBar(int nStyle = SZBARF_STDMOUSECLICKS);

// Operations
public:
	static void TidyUp();
	BOOL Create(CWnd * pParent, LPCTSTR lpszTitle, UINT nID = 1,
                         DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM,
                           const RECT & rect = CFrameWnd::rectDefault);
	// CWnd-style create
	virtual BOOL Create( LPCTSTR lpszClassName, LPCTSTR lpszWindowName,
							DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	void SetSizeDockStyle(DWORD dwStyle);
    void EnableDocking(DWORD dwDockStyle);

	BOOL IsProbablyFloating();

	// SHP 6/16/1999
	void SetGripperSize(int nSize)			{ m_nGripperSize=nSize; }

	BOOL IsHorzDocked();	// 6/16/1999 SHP
	BOOL IsVertDocked();	// 6/16/1999 SHP
	BOOL IsFloating();

	// command handlers for standard menus
	void OnHide();
	virtual void OnHideCurrent()	{}
	virtual void OnUnhideAll()		{}
	virtual void OnShowIcons()		{}

	// Whether to show tab icons
	bool m_bShowIcons;

	void OnToggleAllowDocking();
	void OnFloatAsMDI();
	afx_msg LONG OnAfterFloatMessage(UINT, LONG);

	// This gets called when the "hide" button is pressed
	virtual void OnButtonHide();

	// Overrides
	// ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMRCSizeControlBar)
	//}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CMRCSizeControlBar();

    virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

    // Generated message map functions

protected:
    // Overridable functions
    virtual void OnSizedOrDocked(int cx, int cy, BOOL bFloating, int flags);

	virtual BOOL OnControlBarClose();

	virtual void NcPaintGripper(CDC* pDC, CRect rcClient);

	CSCBButton m_biHide;	// The hide button

    //{{AFX_MSG(CMRCSizeControlBar)
    afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg UINT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonUp(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG
    LRESULT WindowProc(UINT nMsg, WPARAM wParam, LPARAM lParam);
	afx_msg LONG OnAddContextMenuItems(UINT wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()
};


////////////////////////////////////////////////////////////////////////
// CMRCSizeDialogBar control
// A dialog bar that is resizeable - should be 100% compaible with MFC CDialogBar

class CMRCSizeDialogBar : public CMRCSizeControlBar
{
	DECLARE_DYNAMIC(CMRCSizeDialogBar)

		// attributes:
protected:
	CSize 				m_sizeDefault;
	GADGETRESIZEHANDLE	m_GadgetResizeHandle;
	_AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
  	LPCTSTR				m_lpszTemplateName;

		// Construction
public:
	CMRCSizeDialogBar(int nStyle = SZBARF_STDMOUSECLICKS);
	BOOL Create(CWnd* pParentWnd, LPCTSTR lpszTemplateName,
			UINT nStyle, UINT nID);
	BOOL Create(CWnd* pParentWnd, UINT nIDTemplate,
			UINT nStyle, UINT nID)
		{ return Create(pParentWnd, MAKEINTRESOURCE(nIDTemplate), nStyle, nID); };

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMRCSizeDialogBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMRCSizeDialogBar();

    virtual void OnSizedOrDocked(int cx, int cy, BOOL bFloating, int flags);

	// Call this to reposition dialog controls
	virtual void RepositionControls();

	// This is called like OnInitDialog
	virtual BOOL OnInitDialogBar();

protected:
    //{{AFX_MSG(CMRCSizeDialogBar)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	DECLARE_MESSAGE_MAP()

protected:
	// support for OCX's inside dialog bars
#ifndef _AFX_NO_OCC_SUPPORT
	// data and functions necessary for OLE control containment
	//_AFX_OCC_DIALOG_INFO* m_pOccDialogInfo;
	//LPCTSTR m_lpszTemplateName;
	virtual BOOL SetOccDialogInfo(_AFX_OCC_DIALOG_INFO* pOccDialogInfo);
	afx_msg LRESULT HandleInitDialog(WPARAM, LPARAM);
#endif

};


#ifdef __AFXCMN_H__	// only define this class if afxcmn.h has been included
/////////////////////////////////////////////////////////////////////////////
// CMRCToolBarCtrl window - only really designed for use in the tool bars
// below

class CMRCToolBarCtrl : public CToolBarCtrl
{
// Construction
public:
	CMRCToolBarCtrl();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMRCToolBarCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMRCToolBarCtrl();
	void GetBoundingSize(CSize & size);
	int HitTestButtons(CPoint point);

	// Generated message map functions
protected:
	//{{AFX_MSG(CMRCToolBarCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// CMRCSizeToolBarCtrl - a variable sized toolbar for use in much the same way
// as CToolBar.
class CMRCSizeToolBar : public CMRCSizeControlBar
{
// Construction
public:
	DECLARE_DYNAMIC(CMRCSizeToolBar);

	CMRCSizeToolBar(int nStyle = 0);

// Attributes
public:
	CMRCToolBarCtrl	m_ToolCtrl;

protected:
	CSize	m_ToolCtrlButtonSize;		// size of buttons in CToolBarCtrl - not available
	CBitmap m_Bitmap;					// bitmap to be used in control

private:
	UINT * 	m_pBitmapIds;				// ID pallette in the bitmap
	int m_nBitmapButtons;				// no of buttons in the toolbar palette

// Operations
public:
	BOOL Create(CWnd * pParent, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
	 			UINT nID = AFX_IDW_TOOLBAR, LPRECT pRect = NULL);
	BOOL LoadBitmap(LPCTSTR lpszResourceName);
	BOOL LoadBitmap(UINT nID)
			{ return LoadBitmap(MAKEINTRESOURCE(nID)); };
	void SetSizes(SIZE sizeButton, SIZE sizeImage);
	BOOL SetButtons(UINT * pButtons, int nButtons);
	void SetBitmapIds(UINT * pIds, int nButtons);
	CToolBarCtrl * GetToolBarCtrl();


private:
	int FindBitmapIndex(UINT nID);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMRCSizeToolBar)
	protected:
	//}}AFX_VIRTUAL
	afx_msg void OnTBNToolTip(UINT uID, NMHDR *pNMHDR, LRESULT * pResult);
	afx_msg void OnTBNBeginAdjust(UINT uID, NMHDR *pNMHDR, LRESULT * pResult);
	afx_msg void OnTBNQueryInsert(UINT uID, NMHDR *pNMHDR, LRESULT * pResult);
	afx_msg void OnTBNQueryDelete(UINT uID, NMHDR *pNMHDR, LRESULT * pResult);
	afx_msg void OnTBNToolBarChange(UINT uID, NMHDR *pNMHDR, LRESULT * pResult);
	afx_msg void OnTBNGetButtonInfo(UINT uID, NMHDR *pNMHDR, LRESULT * pResult);

	virtual void OnSizedOrDocked(int cx, int cy, BOOL bFloating, int flags);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

// Implementation
public:
	virtual ~CMRCSizeToolBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMRCSizeToolBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif

void ArrangeWindowsInWindow (CWnd * pParentWnd, CObArray & arrWnd, DWORD dwOrient);
void MiniDockToClient(CRect & rect, BOOL bConvertToClient);

/////////////////////////////////////////////////////////////////////////////
// CMRCFrameWndSizeDock frame

class CMRCFrameWndSizeDock : public CFrameWnd
{
    DECLARE_DYNCREATE(CMRCFrameWndSizeDock)
protected:
    CMRCFrameWndSizeDock();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    void EnableDocking(DWORD dwDockStyle, CRuntimeClass *pFloatingClass=RUNTIME_CLASS(CSizableMiniDockFrameWnd));
    void TileDockedBars(DWORD dwDockStyle = CBRS_ALIGN_ANY);
	void ArrangeFloatingBars(DWORD dwOrient);
	void ArrangeWindows(CObArray & arrWnd, DWORD dwOrient);
	void GetFloatingBars(CObArray & arrWnd);

	// save/restore bar state
	void LoadSizeBarState(LPCTSTR pszProfileName);
	void SaveSizeBarState(LPCTSTR pszProfileName);
	void DestroyDynamicBars();
	void FloatControlBar(CControlBar* pBar, CPoint point, DWORD dwStyle = CBRS_ALIGN_TOP);

private:
	void SaveBarSizes(LPCTSTR pszSection, BOOL bSave);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMRCFrameWndSizeDock)
    public:
    //}}AFX_VIRTUAL

// Implementation
protected:
    virtual ~CMRCFrameWndSizeDock();

    // Generated message map functions
    //{{AFX_MSG(CMRCFrameWndSizeDock)
	afx_msg void OnSysColorChange();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

public:
};



// The CMRCMDIFrameWndSizeDock class is supplied for convenience. The major functions are simply
// accomplished by in-line casts to CMRCFrameWndSizeDock

class CMRCMDIFrameWndSizeDock : public CMDIFrameWnd
{
    DECLARE_DYNCREATE(CMRCMDIFrameWndSizeDock)

protected:
    CMRCMDIFrameWndSizeDock();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif
    void EnableDocking(DWORD dwDockStyle, CRuntimeClass *pFloatingClass=RUNTIME_CLASS(CSizableMiniDockFrameWnd))
                { ((CMRCFrameWndSizeDock *)this)->EnableDocking(dwDockStyle, pFloatingClass); };
    void TileDockedBars(DWORD dwDockStyle = CBRS_ALIGN_ANY)
                { ((CMRCFrameWndSizeDock *)this)->TileDockedBars(dwDockStyle); };

	void ArrangeFloatingBars(DWORD dwOrient);
	void ArrangeWindows(CObArray & arrWnd, DWORD dwOrient);

	void GetFloatingBars(CObArray & arrWnd)
                { ((CMRCFrameWndSizeDock *)this)->GetFloatingBars(arrWnd); };

	void LoadSizeBarState(LPCTSTR pszProfileName)
				{ ((CMRCFrameWndSizeDock *)this)->LoadSizeBarState(pszProfileName); };

	void SaveSizeBarState(LPCTSTR pszProfileName)
				{ ((CMRCFrameWndSizeDock *)this)->SaveSizeBarState(pszProfileName); };

	void FloatControlBarInMDIChild(CControlBar* pBar, CPoint point, DWORD dwStyle = CBRS_ALIGN_TOP);
	void UnFloatInMDIChild(CControlBar* pBar, CPoint point, DWORD dwStyle = CBRS_ALIGN_TOP);

	void FloatControlBar(CControlBar* pBar, CPoint point, DWORD dwStyle = CBRS_ALIGN_TOP)
				{ ((CMRCFrameWndSizeDock *)this)->FloatControlBar(pBar, point, dwStyle); };

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMRCMDIFrameWndSizeDock)
    public:
    //}}AFX_VIRTUAL

// Implementation
protected:
    virtual ~CMRCMDIFrameWndSizeDock();
	    // Generated message map functions
        //{{AFX_MSG(CMRCMDIFrameWndSizeDock)
	afx_msg void OnSysColorChange();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

public:
};



class CMRCMDIChildWndSizeDock : public CMDIChildWnd
{
    DECLARE_DYNCREATE(CMRCMDIChildWndSizeDock)

protected:
    CMRCMDIChildWndSizeDock();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
    void EnableDocking(DWORD dwDockStyle)
                { ((CMRCFrameWndSizeDock *)this)->EnableDocking(dwDockStyle); };
    void TileDockedBars(DWORD dwDockStyle = CBRS_ALIGN_ANY)
                { ((CMRCFrameWndSizeDock *)this)->TileDockedBars(dwDockStyle); };

	void GetFloatingBars(CObArray & arrWnd)
                { ((CMRCFrameWndSizeDock *)this)->GetFloatingBars(arrWnd); };

	void DestroyDynamicBars()
                { ((CMRCFrameWndSizeDock *)this)->DestroyDynamicBars(); };

	void LoadSizeBarState(LPCTSTR pszProfileName)
				{ ((CMRCFrameWndSizeDock *)this)->LoadSizeBarState(pszProfileName); };

	void SaveSizeBarState(LPCTSTR pszProfileName)
				{ ((CMRCFrameWndSizeDock *)this)->SaveSizeBarState(pszProfileName); };

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMRCMDIChildWndSizeDock)
    public:
    //}}AFX_VIRTUAL

// Implementation
protected:
    virtual ~CMRCMDIChildWndSizeDock();

    // Generated message map functions
    //{{AFX_MSG(CMRCMDIChildWndSizeDock)
            // NOTE - the ClassWizard will add and remove member functions here.
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////
// toolbar docking support



/////////////////////////////////////////////////////////////////////////////
// Subclassed CRectTracker. Used for resizing splitter windows, and CMRCSizeControlBars, too
class CMRCRectTracker : public CRectTracker
{
public:
	CRect	m_LimitRect;				// limiting rectangle - all movement must be within this.
	CRect	m_OrigRect;					// original position at start of track

	CMRCRectTracker();
	virtual void AdjustRect(int nHandle, LPRECT);
	virtual void DrawTrackerRect(LPCRECT lpRect, CWnd* pWndClipTo, CDC* pDC, CWnd* pWnd);
	BOOL TrackFromHitTest(int nHitTest, CWnd* pWnd, CPoint point,
				CWnd* pWndClipTo = NULL, BOOL bAllowInvert = FALSE);

// extra tracker styles...
#define RectTracker_OnlyMoveHorz		0x0100L  		// only move horizontally
#define RectTracker_OnlyMoveVert        0x0200L			// only move vertically
};


/////////////////////////////////////////////////////////////////////////////
// CMRCAnimateCtrl - plays AVI files directly using MCI. This means it'll
// handle all Video for Windows formats, unlike CAnimateCtrl
/*
class CMRCAnimateCtrl : public CWnd
{
// Construction
public:
	CMRCAnimateCtrl();

// Attributes
public:
	CString	m_strFilename;
	UINT 	m_MCIDeviceID;		// MCI device ID

#define ACF_REPEAT		1
#define ACF_AUTOSIZE	2		// set the window size to that of the movie
	DWORD 	m_dwPlayFlags;		// flags, eg repeat, etc.
	int  	m_nRepeatDelay;
	BOOL    m_bFileOpen;
	int     m_nFrom;		// play from start
	int		m_nTo;			// play from end


// Operations
public:
	static BOOL Init();
	Open(LPCTSTR pFilename);			// open an AVI file
	Close();							// close AVI File
	Play(DWORD dwFlags, int nFirst = 0, int nLast = -1, int nRepeatDelay = 0);
	void GetMovieRect(RECT * pRect);

	void Rewind();
	BOOL Pause();

	afx_msg LONG OnMCINotify(WPARAM wParam, LPARAM lParam);

	static BOOL InitAVI();

// Overrides
	void PostNcDestroy();
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMRCAnimateCtrl)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMRCAnimateCtrl();

	// Generated message map functions
protected:
 	static BOOL m_bInitialized;

	//{{AFX_MSG(CMRCAnimateCtrl)
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
 */


// get number of system colors
int GetNumberSystemColors();
// standard function to center one window relative to another...
void CenterWindowWithinParent(CWnd * pWnd, CWnd * pAlternate = NULL);
// border drawing functions
void DrawBorderRaisedOuter(CDC *pDC, RECT * prect);
void DrawBorderRaisedInner(CDC *pDC, RECT * prect);
void DrawBorderSunkenOuter(CDC *pDC, RECT * prect);
void DrawBorderSunkenInner(CDC *pDC, RECT * prect);
void DrawRectBorder(CDC *pDC, RECT *prect, COLORREF crTopLeft, COLORREF crBottomRighta);


////////////////////////////////////////////////////////////////////////////////
//
class CMRCDib : public CObject
//
////////////////////////////////////////////////////////////////////////////////
{
    DECLARE_DYNAMIC(CMRCDib)
public:
    CMRCDib();
    ~CMRCDib();

    BITMAPINFO *GetBitmapInfoAddress()
        {return m_pBMI;}                        // ptr to bitmap info
    BYTE *GetBitsAddress()
        {return m_pBits;}                       // ptr to the bits
    RGBQUAD *GetClrTabAddress()
        {return (LPRGBQUAD)(((BYTE *)(m_pBMI))
            + sizeof(BITMAPINFOHEADER));}       // ptr to color table
    int GetNumClrEntries();                     // number of color table entries

    virtual BOOL Create(int width, int height); // create a new DIB
    BOOL LoadFileName(LPCTSTR lpszFileName);	// load from file name
    virtual BOOL Load(CFile *fp);               // load from file
 	virtual BOOL LoadFromResource(const char * lpszResource);	// load from a resource
 	virtual BOOL LoadFromResource(UINT nID)
				{ return (LoadFromResource(MAKEINTRESOURCE(nID))); };

    virtual void Draw(CDC *pDC, int x, int y);
	virtual void Draw(CDC *pDC, int x, int y, int cx, int cy);

    int DibHeight()
        { return m_pBMI->bmiHeader.biHeight; }
    int DibWidth()
        { return m_pBMI->bmiHeader.biWidth; }

protected:
    BITMAPINFO *m_pBMI;         // pointer to BITMAPINFO struct
    BYTE *m_pBits;              // pointer to the bits
	BYTE m_cLoadedFromResource;	// flag indicating if it was loaded from a resource file

private:
	void DeleteContents();

};


////////////////////////////////////////////////////////////////////////////////
//
// CMRCDibPal class
//
////////////////////////////////////////////////////////////////////////////////

class CMRCDibPal : public CPalette
{
public:
    CMRCDibPal();
    ~CMRCDibPal();
    BOOL Create(CMRCDib *pDIB);         // create from a DIB
    int GetNumColors();                 // get the no. of colors in the pallette.
};





/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
// CMRCColorLabel window

// Newer styles
#define CLBS_CENTER				0x0001L		// center the bitmap/text
#define CLBS_BITMAP_STRETCH		0x0100L		// stretch bitmap to fit size
#define CLBS_BITMAP_FROMTITLE	0x0200L		// auto-load bitmap from title
#define CLBS_BITMAP_FROMID		0x0400L		// auto-load bitmap from resource-ID
#define CLBS_BITMAP_PALETTE		0x1000L		// use a pallette for the bitmap
#define CLBS_BITMAP_AUTOSIZE   	0x2000L		// On auto-load, set size to the bitmap
#define CLBS_BITMAP				0x8000L		//

#define	CLBS_BORDER_RAISED		0x0010L
#define CLBS_BORDER_SUNKEN    	0x0020L
#define CLBS_FILLBACKGROUND		0x0040L		// fill background with specified background color

#define CLBS_BITMAP_STYLES CLBS_CENTER | CLBS_BITMAP_STRETCH | CLBS_BITMAP_PALETTE \
						| CLBS_BITMAP_AUTOSIZE | CLBS_BITMAP \
						| CLBS_BORDER_RAISED | CLBS_BORDER_SUNKEN | CLBS_FILLBACKGROUND

class CMRCColorLabel : public CWnd
{
// Construction
public:
	static BOOL m_bRegistered;
	static BOOL Register();
	CMRCColorLabel();

// Attributes
public:
	CFont * 	m_pFont;		// font for text
	COLORREF 	m_foreground;	// text color
	COLORREF 	m_background;	// background color
	CMRCDib	* 	m_pDib;			// DIB
	CMRCDibPal * m_pDibPal;  	// DIB pallette if DIB has a pallette

// Operations
public:
	void SetColors(COLORREF foreground, COLORREF background);
	void SetFont(CFont *);

	BOOL LoadDIBFromFile(LPCTSTR lpszFileName, DWORD dwStyle = 0x70000000);
	BOOL LoadDIB(LPCTSTR lpszResource, DWORD dwStyle = 0x70000000);
	BOOL LoadDIB(UINT nID, DWORD dwStyle = 0x70000000)
				{ return LoadDIB (MAKEINTRESOURCE(nID), dwStyle); };

	BOOL CreateSplash(LPCTSTR lpszResource, DWORD dwStyle, BOOL bBorder = FALSE);
	BOOL CreateSplash(UINT nID, DWORD dwStyle, BOOL bBorder = FALSE)
		{ return CreateSplash( MAKEINTRESOURCE(nID), dwStyle, bBorder); } ;

	BOOL Create(LPCTSTR lpszWindowName, DWORD dwStyle, RECT & rect, CWnd *pParent, UINT nID);

	BOOL SetDibInfo(DWORD dwStyle);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMRCColorLabel)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMRCColorLabel();
	WNDPROC* GetSuperWndProcAddr();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMRCColorLabel)
	afx_msg void OnPaint();
	afx_msg void OnNcDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
 	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////
// CMRCBitmapButton window

class CMRCBitmapButton : public CButton
{
	DECLARE_DYNAMIC(CMRCBitmapButton)

// Construction
public:
	CMRCBitmapButton();

// Attributes
public:
	CSize 	m_sizeButton;
	CSize 	m_sizeImage;


private:
	HINSTANCE 	m_hInst;
	HRSRC 		m_hRsrc;
	HBITMAP 	m_hBitmap;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMRCBitmapButton)
	public:
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	//}}AFX_VIRTUAL
	BOOL AutoLoad(UINT nID, CWnd * pParent, BOOL bSetSize = TRUE);
	BOOL virtual LoadBitmap(LPCTSTR lpszResource);
	BOOL LoadBitmap(UINT nID)
	 	{ return LoadBitmap(MAKEINTRESOURCE(nID)); };
	BOOL AutoSize();

// Implementation
public:
	virtual ~CMRCBitmapButton();

	// Generated message map functions
protected:
	//{{AFX_MSG(CMRCBitmapButton)
	afx_msg void OnSysColorChange();
	//}}AFX_MSG


	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
#ifndef _MAC
#define RGB_TO_RGBQUAD(r,g,b)   (RGB(b,g,r))
#define CLR_TO_RGBQUAD(clr)     (RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)))
#else
#define RGB_TO_RGBQUAD(r,g,b)   (RGB(r,g,b) << 8)
#define CLR_TO_RGBQUAD(clr)     (clr << 8)
#endif

struct BITMAPCOLORMAP
{
	// use DWORD instead of RGBQUAD so we can compare two RGBQUADs easily
	DWORD rgbqFrom;		// use RGB_TO_RGBQUAD macro to initialize this
	int iSysColorTo;
};

HBITMAP LoadRecolorBitmap(LPCTSTR lpszResourceName, const BITMAPCOLORMAP * pColorMap = NULL, int nMaps = 0);
HBITMAP LoadRecolorBitmap(UINT nID, const BITMAPCOLORMAP * pColorMap = NULL, int nMaps = 0);
HBITMAP LoadRecolorBitmap(HINSTANCE hInst, HRSRC hRsrc, const BITMAPCOLORMAP * pColorMap = NULL, int nMaps = 0);

void ForceLayoutAdjust(CControlBar * pBar);

//#undef AFX_DATA
//#define AFX_DATA

#endif
