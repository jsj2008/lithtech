//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// flattoolbar.h : definition of Office97 UI style toolbar
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1997 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.
//
// Update History:
//		TonyCl	8/Apr/1997		Office97 UI style
//

#ifndef _FLATTOOLBAR_H_
#define _FLATTOOLBAR_H_

struct AFX_TBBUTTON;        // private to implementation

#ifndef _MAC
HBITMAP AFXAPI AfxLoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc);
#else
HBITMAP AFXAPI AfxLoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc,
	HDC hDCGlyphs, BOOL bMonochrome);
#endif

// TONYCL: START: OFFICE97 LOOK AND FEEL
#define TBBS_UPSTATE		MAKELONG(0,0x80)
// TONYCL: END: OFFICE97 LOOK AND FEEL

class CFlatToolbar : public CControlBar
{
	DECLARE_DYNAMIC(CFlatToolbar)

// Construction
public:
	CFlatToolbar();
	BOOL Create(CWnd* pParentWnd,
			unsigned int dwStyle = WS_CHILD | WS_VISIBLE | CBRS_TOP,
			unsigned int  nID = AFX_IDW_TOOLBAR);

	void SetSizes(SIZE sizeButton, SIZE sizeImage);
				// button size should be bigger than image
	void SetHeight(int cyHeight);
				// call after SetSizes, height overrides bitmap size
	BOOL LoadBitmap(LPCTSTR lpszResourceName);
	BOOL LoadBitmap(UINT nIDResource);
	BOOL SetButtons(const UINT* lpIDArray, int nIDCount);
				// lpIDArray can be NULL to allocate empty buttons

	// Support loading a toolbar from a resource
	BOOL LoadToolBar(LPCTSTR lpszResourceName);
	inline BOOL LoadToolBar(UINT nIDResource) { return LoadToolBar(MAKEINTRESOURCE(nIDResource)); }


// Attributes
public: // standard control bar things
	int CommandToIndex(UINT nIDFind) const;
	UINT GetItemID(int nIndex) const;
	virtual void GetItemRect(int nIndex, LPRECT lpRect) const;
	UINT GetButtonStyle(int nIndex) const;
	void SetButtonStyle(int nIndex, UINT nStyle);

public:
	// for changing button info
	void GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const;
	void SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage);

// Implementation
public:
	int m_nUpButtonIndex;
	virtual ~CFlatToolbar();
	virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	inline AFX_TBBUTTON* _GetButtonPtr(int nIndex) const;
	void InvalidateButton(int nIndex);
	void UpdateButton(int nIndex);
	void CreateMask(int iImage, CPoint offset, 
		BOOL bHilite, BOOL bHiliteShadow);

	// for custom drawing
	struct DrawState
	{
		HBITMAP hbmMono;
		HBITMAP hbmMonoOld;
		HBITMAP hbmOldGlyphs;
	};
	BOOL PrepareDrawButton(DrawState& ds);
	BOOL DrawButton(CDC* pDC, int x, int y, int iImage, UINT nStyle);
#ifdef _MAC
	BOOL DrawMonoButton(CDC* pDC, int x, int y, int dx, int dy,
		int iImage, UINT nStyle);
#endif
	void EndDrawButton(DrawState& ds);

protected:
	CSize m_sizeButton;         // size of button
	CSize m_sizeImage;          // size of glyph
	int m_cxSharedBorder;       // shared x border between buttons
	int m_cySharedBorder;       // shared y border between buttons
	HBITMAP m_hbmImageWell;     // glyphs only
	int m_iButtonCapture;       // index of button with capture (-1 => none)
	HRSRC m_hRsrcImageWell;     // handle to loaded resource for image well
	HINSTANCE m_hInstImageWell; // instance handle to load image well from

#ifdef _MAC
	// Macintosh toolbars need per-toolbar DCs in order to
	// work correctly in multiple-monitor environments

	HDC m_hDCGlyphs;            // per-toolbar DC for glyph images
	HDC m_hDCMono;              // per-toolbar DC for mono glyph masks
#endif
	virtual void DoPaint(CDC* pDC);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual int HitTest(CPoint point);
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	//{{AFX_MSG(CFlatToolbar)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnCancelMode();
	afx_msg void OnSysColorChange();
	afx_msg void OnWindowPosChanged( WINDOWPOS* lpwndpos );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// Auxiliary System/Screen metrics

struct GLOBAL_DATA
{
	// system metrics
	int cxBorder2, cyBorder2;

	// device metrics for screen
	int cxPixelsPerInch, cyPixelsPerInch;

	// solid brushes with convenient gray colors and system colors
	HBRUSH hbrLtGray, hbrDkGray;
	HBRUSH hbrBtnHilite, hbrBtnFace, hbrBtnShadow;
	HBRUSH hbrWindowFrame;
	HPEN hpenBtnHilite, hpenBtnShadow, hpenBtnText;

	// color values of system colors used for CToolBar
	COLORREF clrBtnFace, clrBtnShadow, clrBtnHilite;
	COLORREF clrBtnText, clrWindowFrame;

	// special GDI objects allocated on demand
	HFONT   hStatusFont;
	HFONT   hToolTipsFont;

	// other system information
	UINT    nWinVer;        // Major.Minor version numbers
	BOOL	bWin32s;		// TRUE if Win32s (or Windows 95)
	BOOL    bWin4;          // TRUE if Windows 4.0
	BOOL    bNotWin4;       // TRUE if not Windows 4.0
	BOOL    bSmCaption;     // TRUE if WS_EX_SMCAPTION is supported
	BOOL	bWin31; 		// TRUE if actually Win32s on Windows 3.1

// Implementation
	GLOBAL_DATA();
	~GLOBAL_DATA();
	void UpdateSysColors();
	void UpdateSysMetrics();
};

extern GLOBAL_DATA globalData;

// Note: afxData.cxBorder and afxData.cyBorder aren't used anymore
#define CX_BORDER   1
#define CY_BORDER   1

// determine number of elements in an array (not bytes)
#define _countof(array) (sizeof(array)/sizeof(array[0]))

BOOL AFXAPI AfxCustomLogFont(UINT nIDS, LOGFONT* pLogFont);
void AFXAPI AfxDeleteObject(HGDIOBJ* pObject);

/////////////////////////////////////////////////////////////////////////////

#endif //!_FLATTOOLBAR_H_
