//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
// flattoolbar.cpp : definition of Office97 UI style toolbar
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

#include "bdefs.h"
#define _AFX_NO_OLE_SUPPORT
#include <afxpriv.h>
#include "flattoolbar.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// globals for fast drawing (shared globals)
#ifndef _MAC
static HDC hDCGlyphs = NULL;
static HDC hDCMono = NULL;
#else
#define hDCGlyphs   m_hDCGlyphs
#define hDCMono     m_hDCMono
#endif
static HBRUSH hbrDither = NULL;

/////////////////////////////////////////////////////////////////////////////
// Init / Term

#ifndef _MAC
static HBITMAP AFXAPI CreateDitherBitmap();
#else
static HBITMAP AFXAPI CreateDitherBitmap(BOOL bMonochrome);
#endif

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif



// a special struct that will cleanup automatically
struct _AFX_TOOLBAR_TERM
{
	~_AFX_TOOLBAR_TERM()
	{
#ifndef _MAC
		AfxDeleteObject((HGDIOBJ*)&hDCMono);
		AfxDeleteObject((HGDIOBJ*)&hDCGlyphs);
#endif
		AfxDeleteObject((HGDIOBJ*)&hbrDither);
	}
};

static const _AFX_TOOLBAR_TERM toolbarTerm;

/////////////////////////////////////////////////////////////////////////////

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

// TONYCL: START: OFFICE97 LOOK AND FEEL

struct CToolBarData
{
	WORD wVersion;
	WORD wWidth;
	WORD wHeight;
	WORD wItemCount;
	//WORD aItems[wItemCount]

	WORD* items()
		{ return (WORD*)(this+1); }
};

// Support loading a toolbar from a resource
BOOL CFlatToolbar::LoadToolBar(LPCTSTR lpszResourceName)
{
	ASSERT_VALID(this);
	ASSERT(lpszResourceName != NULL);

	// determine location of the bitmap in resource fork
	HINSTANCE hInst = AfxFindResourceHandle(lpszResourceName, RT_TOOLBAR);
	HRSRC hRsrc = ::FindResource(hInst, lpszResourceName, RT_TOOLBAR);
	if (hRsrc == NULL)
		return FALSE;

	HGLOBAL hGlobal = LoadResource(hInst, hRsrc);
	if (hGlobal == NULL)
		return FALSE;

	CToolBarData* pData = (CToolBarData*)LockResource(hGlobal);
	if (pData == NULL)
		return FALSE;
	ASSERT(pData->wVersion == 1);

	UINT* pItems = new UINT[pData->wItemCount];
	for (int i = 0; i < pData->wItemCount; i++)
		pItems[i] = pData->items()[i];
	BOOL bResult = SetButtons(pItems, pData->wItemCount);
	delete[] pItems;

	if (bResult)
	{
		// set new sizes of the buttons
		CSize sizeImage(pData->wWidth, pData->wHeight);
		CSize sizeButton(pData->wWidth + 7, pData->wHeight + 7);
		SetSizes(sizeButton, sizeImage);

		// load bitmap now that sizes are known by the toolbar control
		bResult = LoadBitmap(lpszResourceName);
	}

	UnlockResource(hGlobal);
	FreeResource(hGlobal);

	return bResult;
}
// TONYCL: END: OFFICE97 LOOK AND FEEL

#ifndef _MAC
static HBITMAP AFXAPI CreateDitherBitmap()
#else
static HBITMAP AFXAPI CreateDitherBitmap(BOOL bMonochrome)
#endif
{
	struct  // BITMAPINFO with 16 colors
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD      bmiColors[16];
	} bmi;
	memset(&bmi, 0, sizeof(bmi));

	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = 8;
	bmi.bmiHeader.biHeight = 8;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 1;
	bmi.bmiHeader.biCompression = BI_RGB;

	COLORREF clr = ::GetSysColor(COLOR_BTNFACE);
#ifdef _MAC
	// if monochrome or the face color is already white, use black instead to make
	// sure that we draw a dither - otherwise we'll have a white on white bitmap
	if (bMonochrome || clr == RGB(255, 255, 255))
		clr = 0;
#endif
	bmi.bmiColors[0].rgbBlue = GetBValue(clr);
	bmi.bmiColors[0].rgbGreen = GetGValue(clr);
	bmi.bmiColors[0].rgbRed = GetRValue(clr);

	clr = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	bmi.bmiColors[1].rgbBlue = GetBValue(clr);
	bmi.bmiColors[1].rgbGreen = GetGValue(clr);
	bmi.bmiColors[1].rgbRed = GetRValue(clr);

	// initialize the brushes
	long patGray[8];
	for (int i = 0; i < 8; i++)
	   patGray[i] = (i & 1) ? 0xAAAA5555L : 0x5555AAAAL;

	HDC hDC = GetDC(NULL);
	HBITMAP hbm = CreateDIBitmap(hDC, &bmi.bmiHeader, CBM_INIT,
		(LPBYTE)patGray, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
	ReleaseDC(NULL, hDC);

	return hbm;
}

// create a mono bitmap mask:
void CFlatToolbar::CreateMask(int iImage, CPoint ptOffset,
	 BOOL bHilite, BOOL bHiliteShadow)
{
	// initalize whole area with 0's
	PatBlt(hDCMono, 0, 0, m_sizeButton.cx-2, m_sizeButton.cy-2, WHITENESS);

	// create mask based on color bitmap
	// convert this to 1's
	SetBkColor(hDCGlyphs, globalData.clrBtnFace);
	BitBlt(hDCMono, ptOffset.x, ptOffset.y, m_sizeImage.cx, m_sizeImage.cy,
		hDCGlyphs, iImage * m_sizeImage.cx, 0, SRCCOPY);

	if (bHilite)
	{
		// convert this to 1's
		SetBkColor(hDCGlyphs, globalData.clrBtnHilite);

		// OR in the new 1's
		BitBlt(hDCMono, ptOffset.x, ptOffset.y, m_sizeImage.cx, m_sizeImage.cy,
			hDCGlyphs, iImage * m_sizeImage.cx, 0, SRCPAINT);

		if (bHiliteShadow)
			BitBlt(hDCMono, 1, 1, m_sizeButton.cx-3, m_sizeButton.cy-3,
				hDCMono, 0, 0, SRCAND);
	}
}

// Raster Ops
#define ROP_DSPDxax  0x00E20746L
#define ROP_PSDPxax  0x00B8074AL

BOOL CFlatToolbar::DrawButton(CDC* pDC, int x, int y, int iImage, UINT nStyle)
{
	ASSERT_VALID(pDC);

	int dx = m_sizeButton.cx;
	int dy = m_sizeButton.cy;
	if (!globalData.bWin4)
	{
		// make the coordinates the interior of the button
		x += 1;
		y += 1;
		dx -= 2;
		dy -= 2;

		// border around button
		pDC->FillSolidRect(x,    y-1,    dx, 1,  globalData.clrWindowFrame);
		pDC->FillSolidRect(x,    y+dy,   dx, 1,  globalData.clrWindowFrame);
		pDC->FillSolidRect(x-1,  y,  1,  dy, globalData.clrWindowFrame);
		pDC->FillSolidRect(x+dx, y,  1,  dy, globalData.clrWindowFrame);
	}

#ifdef _MAC
	if (m_bMonochrome)
		return DrawMonoButton(pDC, x, y, dx, dy, iImage, nStyle);
#endif

	// interior grey
	pDC->FillSolidRect(x, y, dx, dy, globalData.clrBtnFace);

	// determine offset of bitmap (centered within button)
	CPoint ptOffset;
	ptOffset.x = (dx - m_sizeImage.cx - 1) / 2;
	ptOffset.y = (dy - m_sizeImage.cy) / 2;

	if (nStyle & (TBBS_PRESSED | TBBS_CHECKED))
	{
		// pressed in or checked
		pDC->Draw3dRect(x, y, dx, dy,
			globalData.bWin4 ? globalData.clrWindowFrame : globalData.clrBtnShadow,
			globalData.bWin4 ? globalData.clrBtnHilite : globalData.clrBtnFace);

/* TONYCL: OFFICE97 LOOK AND FEEL
		if (globalData.bWin4)
		{
			pDC->Draw3dRect(x + 1, y + 1, dx - 2, dy - 2,
				globalData.clrBtnShadow, globalData.clrBtnFace);
		}
*/

		// for any depressed button, add one to the offsets.
		ptOffset.x += 1;
		ptOffset.y += 1;
	}
// TONYCL: START: OFFICE97 LOOK AND FEEL
	if (nStyle & TBBS_UPSTATE)
	{
		pDC->Draw3dRect(x, y, dx, dy, globalData.clrBtnHilite,
// MATTGR
// clrWindowFrame was icky
			/*globalData.bWin4 ? globalData.clrWindowFrame : */globalData.clrBtnShadow);
	}
// TONYCL: END: OFFICE97 LOOK AND FEEL

	if ((nStyle & TBBS_PRESSED) || !(nStyle & TBBS_DISABLED))
	{
		// normal image version
		BitBlt(pDC->m_hDC, x + ptOffset.x, y + ptOffset.y,
			m_sizeImage.cx, m_sizeImage.cy,
			hDCGlyphs, iImage * m_sizeImage.cx, 0, SRCCOPY);

		if (nStyle & TBBS_PRESSED)
			return TRUE;        // nothing more to do (rest of style is ignored)
	}

	if (nStyle & (TBBS_DISABLED | TBBS_INDETERMINATE))
	{
		// disabled or indeterminate version
		CreateMask(iImage, ptOffset, TRUE, FALSE);

		pDC->SetTextColor(0L);                  // 0's in mono -> 0 (for ROP)
		pDC->SetBkColor((COLORREF)0x00FFFFFFL); // 1's in mono -> 1

		if (nStyle & TBBS_DISABLED)
		{
			// disabled - draw the hilighted shadow
			HGDIOBJ hbrOld = pDC->SelectObject(globalData.hbrBtnHilite);
			if (hbrOld != NULL)
			{
				// draw hilight color where we have 0's in the mask
				BitBlt(pDC->m_hDC, x + 1, y + 1,
					m_sizeButton.cx - 2, m_sizeButton.cy - 2,
					hDCMono, 0, 0, ROP_PSDPxax);
				pDC->SelectObject(hbrOld);
			}
		}

		//BLOCK: always draw the shadow
		{
			HGDIOBJ hbrOld = pDC->SelectObject(globalData.hbrBtnShadow);
			if (hbrOld != NULL)
			{
				// draw the shadow color where we have 0's in the mask
				BitBlt(pDC->m_hDC, x, y,
					m_sizeButton.cx - 2, m_sizeButton.cy - 2,
					hDCMono, 0, 0, ROP_PSDPxax);
				pDC->SelectObject(hbrOld);
			}
		}
	}

	// if it is checked do the dither brush avoiding the glyph
	if (nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE))
	{
		HGDIOBJ hbrOld = pDC->SelectObject(hbrDither);
		if (hbrOld != NULL)
		{
			ptOffset.x -= globalData.cxBorder2;
			ptOffset.y -= globalData.cyBorder2;
			CreateMask(iImage, ptOffset, ~(nStyle & TBBS_INDETERMINATE),
					nStyle & TBBS_DISABLED);

			pDC->SetTextColor(0L);              // 0 -> 0
			pDC->SetBkColor((COLORREF)0x00FFFFFFL); // 1 -> 1

			ASSERT(globalData.cxBorder2 == globalData.cyBorder2);
			int delta = (nStyle & TBBS_INDETERMINATE) ?
				globalData.bWin4 ? globalData.cxBorder2*2 : 3 : globalData.cxBorder2*2;

			// only draw the dither brush where the mask is 1's
			BitBlt(pDC->m_hDC,
				x + globalData.cxBorder2, y + globalData.cyBorder2, dx-delta, dy-delta,
				hDCMono, 0, 0, ROP_DSPDxax);
			pDC->SelectObject(hbrOld);
		}
	}

	return TRUE;
}

#ifdef _MAC
BOOL CFlatToolbar::DrawMonoButton(CDC* pDC, int x, int y, int dx, int dy,
	int iImage, UINT nStyle)
{
	// interior is black if pressed, white if not
	if (nStyle & (TBBS_PRESSED | TBBS_CHECKED))
	{
		pDC->FillSolidRect(x, y, dx, dy, RGB(0, 0, 0));
		pDC->SetBkColor(RGB(255, 255, 255));    // bkcolor was set by PatB
	}
	else
	{
		pDC->FillSolidRect(x, y, dx, dy, RGB(0xFF, 0xFF, 0xFF));
	}

	CPoint ptOffset;
	ptOffset.x = (dx - m_sizeImage.cx - 1) / 2;
	ptOffset.y = (dy - m_sizeImage.cy) / 2;

	if ((nStyle & TBBS_PRESSED) || !(nStyle & TBBS_DISABLED))
	{
		// normal image version
		BitBlt(pDC->m_hDC, x + ptOffset.x, y + ptOffset.y, m_sizeImage.cx,
			m_sizeImage.cy, hDCGlyphs, iImage * m_sizeImage.cx, 0,
			(nStyle & (TBBS_PRESSED | TBBS_CHECKED)) ? NOTSRCCOPY : SRCCOPY);

		if (nStyle & (TBBS_PRESSED | TBBS_CHECKED))
			return TRUE;        // nothing more to do (rest of style is ignored)
	}

	if (nStyle & TBBS_DISABLED)
	{
		BitBlt(pDC->m_hDC, x + ptOffset.x, y + ptOffset.y, m_sizeImage.cx,
			m_sizeImage.cy, hDCGlyphs, iImage * m_sizeImage.cx, 0, SRCCOPY);

		int ropOld = pDC->SetROP2(R2_MASKNOTPEN);
		RECT rect;
		SetRect(&rect, 0, 0, m_sizeImage.cx, m_sizeImage.cy);
		OffsetRect(&rect, x + ptOffset.x, y + ptOffset.y);
		AfxFillRect(pDC->m_hDC, &rect, hbrDither);
		pDC->SetROP2(ropOld);

		return TRUE;
	}

	// if it is checked do the dither brush avoiding the glyph
	if (nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE))
	{
		HGDIOBJ hbrOld = pDC->SelectObject(hbrDither);
		if (hbrOld != NULL)
		{
			CreateMask(iImage, ptOffset, ~(nStyle & TBBS_INDETERMINATE),
					nStyle & TBBS_DISABLED);

			pDC->SetTextColor(0L);              // 0 -> 0
			pDC->SetBkColor((COLORREF)0x00FFFFFFL); // 1 -> 1

			int delta = (nStyle & TBBS_INDETERMINATE) ? 3 : 1;

			// only draw the dither brush where the mask is 1's
			CRect rect(0, 0, dx, dy);
			::InvertRect(hDCMono, &rect);

			BitBlt(pDC->m_hDC, x, y, dx, dy, hDCMono, 0, 0, ROP_PSDPxax);
			pDC->SelectObject(hbrOld);
		}
	}

	return TRUE;
}
#endif

BOOL CFlatToolbar::PrepareDrawButton(DrawState& ds)
{
	ASSERT(m_hbmImageWell != NULL);
	ASSERT(m_sizeButton.cx > 2 && m_sizeButton.cy > 2);

	// We need to kick-start the bitmap selection process.
	ds.hbmOldGlyphs = (HBITMAP)SelectObject(hDCGlyphs, m_hbmImageWell);
	ds.hbmMono = CreateBitmap(m_sizeButton.cx-2, m_sizeButton.cy-2,
					1, 1, NULL);
	ds.hbmMonoOld = (HBITMAP)SelectObject(hDCMono, ds.hbmMono);
	if (ds.hbmOldGlyphs == NULL || ds.hbmMono == NULL || ds.hbmMonoOld == NULL)
	{
		TRACE0("Error: can't draw toolbar.\n");
		AfxDeleteObject((HGDIOBJ*)&ds.hbmMono);
		return FALSE;
	}
	return TRUE;
}

void CFlatToolbar::EndDrawButton(DrawState& ds)
{
	SelectObject(hDCMono, ds.hbmMonoOld);
	AfxDeleteObject((HGDIOBJ*)&ds.hbmMono);
	SelectObject(hDCGlyphs, ds.hbmOldGlyphs);
}

/////////////////////////////////////////////////////////////////////////////
// CFlatToolbar creation etc

struct AFX_TBBUTTON
{
	UINT nID;        // Command ID that this button sends
	UINT nStyle;    // TBBS_ styles
	int iImage;     // index into mondo bitmap of this button's picture
						// or size of this spacer
};

inline AFX_TBBUTTON* CFlatToolbar::_GetButtonPtr(int nIndex) const
{
	ASSERT(nIndex >= 0 && nIndex < m_nCount);
	ASSERT(m_pData != NULL);
	return ((AFX_TBBUTTON*)m_pData) + nIndex;
}

/*
	DIBs use RGBQUAD format:
		0xbb 0xgg 0xrr 0x00

	Reasonably efficient code to convert a COLORREF into an
	RGBQUAD is byte-order-dependent, so we need different
	code depending on the byte order we're targeting.
*/
#ifndef _MAC
#define RGB_TO_RGBQUAD(r,g,b)   (RGB(b,g,r))
#define CLR_TO_RGBQUAD(clr)     (RGB(GetBValue(clr), GetGValue(clr), GetRValue(clr)))
#else
#define RGB_TO_RGBQUAD(r,g,b)   (RGB(r,g,b) << 8)
#define CLR_TO_RGBQUAD(clr)     (clr << 8)
#endif

#ifndef _MAC
HBITMAP AFXAPI LoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc)
#else
HBITMAP AFXAPI LoadSysColorBitmap(HINSTANCE hInst, HRSRC hRsrc,
	HDC hDCGlyphs, BOOL bMonochrome)
#endif
{
	struct COLORMAP
	{
		// use DWORD instead of RGBQUAD so we can compare two RGBQUADs easily
		DWORD rgbqFrom;
		int iSysColorTo;
	};
	static const COLORMAP sysColorMap[] =
	{
		// mapping from color in DIB to system color
		{ RGB_TO_RGBQUAD(0x00, 0x00, 0x00),  COLOR_BTNTEXT },       // black
		{ RGB_TO_RGBQUAD(0x80, 0x80, 0x80),  COLOR_BTNSHADOW },     // dark grey
		{ RGB_TO_RGBQUAD(0xC0, 0xC0, 0xC0),  COLOR_BTNFACE },       // bright grey
		{ RGB_TO_RGBQUAD(0xFF, 0xFF, 0xFF),  COLOR_BTNHIGHLIGHT }   // white
	};
	const int nMaps = 4;

	HGLOBAL hglb;
	if ((hglb = ::LoadResource(hInst, hRsrc)) == NULL)
		return NULL;

	LPBITMAPINFOHEADER lpBitmap = (LPBITMAPINFOHEADER)LockResource(hglb);
	if (lpBitmap == NULL)
		return NULL;

	// make copy of BITMAPINFOHEADER so we can modify the color table
	const int nColorTableSize = 16;
	UINT nSize = lpBitmap->biSize + nColorTableSize * sizeof(RGBQUAD);
	LPBITMAPINFOHEADER lpBitmapInfo = (LPBITMAPINFOHEADER)::malloc(nSize);
	if (lpBitmapInfo == NULL)
		return NULL;
	memcpy(lpBitmapInfo, lpBitmap, nSize);

	// color table is in RGBQUAD DIB format
	DWORD* pColorTable =
		(DWORD*)(((LPBYTE)lpBitmapInfo) + (UINT)lpBitmapInfo->biSize);

	for (int iColor = 0; iColor < nColorTableSize; iColor++)
	{
		// look for matching RGBQUAD color in original
		for (int i = 0; i < nMaps; i++)
		{
			if (pColorTable[iColor] == sysColorMap[i].rgbqFrom)
			{
#ifdef _MAC
				if (bMonochrome)
				{
					// all colors except text become white
					if (sysColorMap[i].iSysColorTo != COLOR_BTNTEXT)
						pColorTable[iColor] = RGB_TO_RGBQUAD(255, 255, 255);
				}
				else
#endif
				pColorTable[iColor] =
					CLR_TO_RGBQUAD(::GetSysColor(sysColorMap[i].iSysColorTo));
				break;
			}
		}
	}

	int nWidth = (int)lpBitmapInfo->biWidth;
	int nHeight = (int)lpBitmapInfo->biHeight;
	HDC hDCScreen = ::GetDC(NULL);
	HBITMAP hbm = ::CreateCompatibleBitmap(hDCScreen, nWidth, nHeight);
	::ReleaseDC(NULL, hDCScreen);

	if (hbm != NULL)
	{
		HBITMAP hbmOld = (HBITMAP)::SelectObject(hDCGlyphs, hbm);

		LPBYTE lpBits;
		lpBits = (LPBYTE)(lpBitmap + 1);
		lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

		StretchDIBits(hDCGlyphs, 0, 0, nWidth, nHeight, 0, 0, nWidth, nHeight,
			lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
		SelectObject(hDCGlyphs, hbmOld);

#ifdef _MAC
		// We don't change this bitmap any more, so get rid of the big,
		// wasteful Macintosh port
		::SetBitmapReadOnly(hbm, BRO_READONLY);
#endif
	}

	// free copy of bitmap info struct and resource itself
	::free(lpBitmapInfo);
	::FreeResource(hglb);

	return hbm;
}

#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

CFlatToolbar::CFlatToolbar()
{
	m_hbmImageWell = NULL;
	m_hInstImageWell = NULL;
	m_hRsrcImageWell = NULL;
	m_iButtonCapture = -1;      // nothing captured

// TONYCL: START: OFFICE97 LOOK AND FEEL
	m_nUpButtonIndex = -1;		// No previous upstate button
// TONYCL: END: OFFICE97 LOOK AND FEEL

	// UISG standard sizes
	m_sizeButton.cx = 24;
	m_sizeButton.cy = 22;
	m_sizeImage.cx = 16;
	m_sizeImage.cy = 15;
// MATTGR smaller top and bottom gaps
	// 3 pixel for top/bottom gaps
	m_cyTopBorder = 2;
	m_cyBottomBorder = 2;

	// adjust sizes when running on Win4
	if (globalData.bWin4)
	{
		m_sizeButton.cx = 23;
		m_cySharedBorder = m_cxSharedBorder = 0;
		m_cxDefaultGap = 8;
	}
	else
	{
		m_cxDefaultGap = 6;
		m_cySharedBorder = m_cxSharedBorder = 1;
	}

#ifdef _MAC
	m_hDCGlyphs = CreateCompatibleDC(NULL);
	m_hDCMono = ::CreateCompatibleDC(NULL);
	if (m_hDCGlyphs == NULL || m_hDCMono == NULL)
		AfxThrowResourceException();
#endif

	// initialize the toolbar drawing engine
	static BOOL bInitialized;
	if (!bInitialized)
	{
#ifndef _MAC
		hDCGlyphs = CreateCompatibleDC(NULL);

		// Mono DC and Bitmap for disabled image
		hDCMono = ::CreateCompatibleDC(NULL);
#endif

#ifndef _MAC
		HBITMAP hbmGray = ::CreateDitherBitmap();
#else
		HBITMAP hbmGray = ::CreateDitherBitmap(m_bMonochrome);
#endif
		if (hbmGray != NULL)
		{
			ASSERT(hbrDither == NULL);
			hbrDither = ::CreatePatternBrush(hbmGray);
			AfxDeleteObject((HGDIOBJ*)&hbmGray);
		}

#ifndef _MAC
		if (hDCGlyphs == NULL || hDCMono == NULL || hbrDither == NULL)
			AfxThrowResourceException();
#else
		if (hbrDither == NULL)
			AfxThrowResourceException();
#endif
		bInitialized = TRUE;
	}
}

CFlatToolbar::~CFlatToolbar()
{
#ifdef _MAC
	ASSERT(m_hDCGlyphs != NULL);
	VERIFY(::DeleteDC(m_hDCGlyphs));

	ASSERT(m_hDCMono != NULL);
	VERIFY(::DeleteDC(m_hDCMono));
#endif

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageWell);
}

BOOL CFlatToolbar::Create(CWnd* pParentWnd, uint32 dwStyle, UINT nID)
{
	if (pParentWnd != NULL)
		ASSERT_VALID(pParentWnd);   // must have a parent

	// save the style
	m_dwStyle = dwStyle;
	if (nID == AFX_IDW_TOOLBAR)
		m_dwStyle |= CBRS_HIDE_INPLACE;

	// create the HWND
	CRect rect;
	rect.SetRectEmpty();
	LPCTSTR lpszClass = AfxRegisterWndClass(0, ::LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_BTNFACE+1), NULL);
	if (!CWnd::Create(lpszClass, NULL, dwStyle, rect, pParentWnd, nID))
		return FALSE;

	// Note: Parent must resize itself for control bar to be resized

	return TRUE;
}

void CFlatToolbar::SetSizes(SIZE sizeButton, SIZE sizeImage)
{
	ASSERT_VALID(this);
	ASSERT(sizeButton.cx > 0 && sizeButton.cy > 0);
	ASSERT(sizeImage.cx > 0 && sizeImage.cy > 0);

	// button must be big enough to hold image + 3 pixels on each side
	ASSERT(sizeButton.cx >= sizeImage.cx + 6);
	ASSERT(sizeButton.cy >= sizeImage.cy + 6);

	m_sizeButton = sizeButton;
	m_sizeImage = sizeImage;

	// set height
	Invalidate();   // just to be nice if called when toolbar is visible
}

void CFlatToolbar::SetHeight(int cyHeight)
{
	ASSERT_VALID(this);

	int nHeight = cyHeight;
	if (m_dwStyle & CBRS_BORDER_TOP)
		cyHeight -= globalData.cyBorder2;
	if (m_dwStyle & CBRS_BORDER_BOTTOM)
		cyHeight -= globalData.cyBorder2;
	m_cyBottomBorder = (cyHeight - m_sizeButton.cy) / 2;
	// if there is an extra pixel, m_cyTopBorder will get it
	m_cyTopBorder = cyHeight - m_sizeButton.cy - m_cyBottomBorder;
	if (m_cyTopBorder < 0)
	{
		TRACE1("Warning: CFlatToolbar::SetHeight(%d) is smaller than button.\n",
			nHeight);
		m_cyBottomBorder += m_cyTopBorder;
		m_cyTopBorder = 0;  // will clip at bottom
	}
	// bottom border will be ignored (truncate as needed)
	Invalidate();   // just to be nice if called when toolbar is visible
}

BOOL CFlatToolbar::LoadBitmap(UINT nIDBitmap)
{
	return LoadBitmap(MAKEINTRESOURCE(nIDBitmap));
}

BOOL CFlatToolbar::LoadBitmap(LPCTSTR lpszResourceName)
{
	ASSERT_VALID(this);
	ASSERT(lpszResourceName != NULL);

	AfxDeleteObject((HGDIOBJ*)&m_hbmImageWell);     // get rid of old one

	m_hInstImageWell = AfxFindResourceHandle(lpszResourceName, RT_BITMAP);
	if ((m_hRsrcImageWell = ::FindResource(m_hInstImageWell,
		lpszResourceName, RT_BITMAP)) == NULL)
		return FALSE;

#ifndef _MAC
	m_hbmImageWell = LoadSysColorBitmap(m_hInstImageWell, m_hRsrcImageWell);
#else
	m_hbmImageWell = LoadSysColorBitmap(m_hInstImageWell, m_hRsrcImageWell,
		m_hDCGlyphs, m_bMonochrome);
#endif
	return (m_hbmImageWell != NULL);
}

BOOL CFlatToolbar::SetButtons(const UINT* lpIDArray, int nIDCount)
{
	ASSERT_VALID(this);
	ASSERT(nIDCount >= 1);  // must be at least one of them
	ASSERT(lpIDArray == NULL ||
		AfxIsValidAddress(lpIDArray, sizeof(UINT) * nIDCount, FALSE));

	// first allocate array for panes and copy initial data
	if (!AllocElements(nIDCount, sizeof(AFX_TBBUTTON)))
		return FALSE;
	ASSERT(nIDCount == m_nCount);

	if (lpIDArray != NULL)
	{
		int iImage = 0;
		// go through them adding buttons
		AFX_TBBUTTON* pTBB = (AFX_TBBUTTON*)m_pData;
		for (int i = 0; i < nIDCount; i++, pTBB++)
		{
			ASSERT(pTBB != NULL);
			if ((pTBB->nID = *lpIDArray++) == 0)
			{
				// separator
				pTBB->nStyle = TBBS_SEPARATOR;
				// width of separator includes 2 pixel overlap
				pTBB->iImage = m_cxDefaultGap + m_cxSharedBorder * 2;
			}
			else
			{
				// a command button with image
				pTBB->nStyle = TBBS_BUTTON;
				pTBB->iImage = iImage++;
			}
		}
	}
	return TRUE;
}

#ifdef AFX_CORE3_SEG
#pragma code_seg(AFX_CORE3_SEG)
#endif

/////////////////////////////////////////////////////////////////////////////
// CFlatToolbar attribute access

int CFlatToolbar::CommandToIndex(UINT nIDFind) const
{
	ASSERT_VALID(this);

	AFX_TBBUTTON* pTBB = _GetButtonPtr(0);
	for (int i = 0; i < m_nCount; i++, pTBB++)
		if (pTBB->nID == nIDFind)
			return i;
	return -1;
}

UINT CFlatToolbar::GetItemID(int nIndex) const
{
	ASSERT_VALID(this);

	return _GetButtonPtr(nIndex)->nID;
}

void CFlatToolbar::GetItemRect(int nIndex, LPRECT lpRect) const
{
	ASSERT_VALID(this);
	ASSERT(nIndex >= 0 && nIndex < m_nCount);
	ASSERT(AfxIsValidAddress(lpRect, sizeof(RECT)));

	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) ? TRUE : FALSE;
	CRect rect;
	rect.SetRectEmpty();        // only need top and left
	CalcInsideRect(rect, bHorz);
	AFX_TBBUTTON* pTBB = (AFX_TBBUTTON*)m_pData;
	for (int iButton = 0; iButton < nIndex; iButton++, pTBB++)
	{
		ASSERT(pTBB != NULL);
		// skip this button or separator
		if (bHorz)
		{
			rect.left += (pTBB->nStyle & TBBS_SEPARATOR) ?
						pTBB->iImage : m_sizeButton.cx;
			rect.left -= m_cxSharedBorder;    // go back for overlap
		}
		else
		{
			rect.top += (pTBB->nStyle & TBBS_SEPARATOR) ?
						pTBB->iImage : m_sizeButton.cy;
			rect.top -= m_cySharedBorder;    // go back for overlap
		}
	}
	ASSERT(iButton == nIndex);
	ASSERT(pTBB == _GetButtonPtr(nIndex));

	// button or image width
	if (bHorz)
	{
		// TONYCL: START: OFFICE97 LOOK AND FEEL
		// If we are not floating then shift the buttons right to allow for a gripper
		if (!IsFloating()) {
			rect.left += 3;
		}
		// TONYCL: END: OFFICE97 LOOK AND FEEL

		int cx = (pTBB->nStyle & TBBS_SEPARATOR) ? pTBB->iImage : m_sizeButton.cx;
		lpRect->right = (lpRect->left = rect.left) + cx;
		lpRect->bottom = (lpRect->top = rect.top) + m_sizeButton.cy;
	}
	else
	{
		// TONYCL: START: OFFICE97 LOOK AND FEEL
		// If we are not floating then shift the buttons down to allow for a gripper
		if (!IsFloating()) {
			rect.top += 3;
		}
		// TONYCL: END: OFFICE97 LOOK AND FEEL

		int cy = (pTBB->nStyle & TBBS_SEPARATOR) ? pTBB->iImage : m_sizeButton.cy;
		lpRect->bottom = (lpRect->top = rect.top) + cy;
		lpRect->right = (lpRect->left = rect.left) + m_sizeButton.cx;
	}
}

UINT CFlatToolbar::GetButtonStyle(int nIndex) const
{
	return _GetButtonPtr(nIndex)->nStyle;
}

void CFlatToolbar::SetButtonStyle(int nIndex, UINT nStyle)
{
	AFX_TBBUTTON* pTBB = _GetButtonPtr(nIndex);
	UINT nOldStyle = pTBB->nStyle;
	if (nOldStyle != nStyle)
	{
		// update the style and invalidate
		pTBB->nStyle = nStyle;

		// invalidate the button only if both styles not "pressed"
		if (!(nOldStyle & nStyle & TBBS_PRESSED))
			InvalidateButton(nIndex);
	}
}

CSize CFlatToolbar::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	ASSERT_VALID(this);

	CSize size = CControlBar::CalcFixedLayout(bStretch, bHorz);

	CRect rect;
	rect.SetRectEmpty();        // only need top and left
	CalcInsideRect(rect, bHorz);
	AFX_TBBUTTON* pTBB = (AFX_TBBUTTON*)m_pData;
	int nButtonDist = 0;

	if (!bStretch)
	{
		for (int iButton = 0; iButton < m_nCount; iButton++, pTBB++)
		{
			ASSERT(pTBB != NULL);
			// skip this button or separator
			nButtonDist += (pTBB->nStyle & TBBS_SEPARATOR) ?
				pTBB->iImage : (bHorz ? m_sizeButton.cx : m_sizeButton.cy);
			// go back one for overlap
			nButtonDist -= bHorz ? m_cxSharedBorder : m_cySharedBorder;
		}
		if (bHorz)
			size.cx = nButtonDist - rect.Width() + m_cxSharedBorder;
		else
			size.cy = nButtonDist - rect.Height() + m_cySharedBorder;
	}

	if (bHorz)
		size.cy = m_sizeButton.cy - rect.Height(); // rect.Height() < 0
	else
		size.cx = m_sizeButton.cx - rect.Width(); // rect.Width() < 0

	return size;
}

void CFlatToolbar::GetButtonInfo(int nIndex, UINT& nID, UINT& nStyle, int& iImage) const
{
	ASSERT_VALID(this);

	AFX_TBBUTTON* pTBB = _GetButtonPtr(nIndex);
	nID = pTBB->nID;
	nStyle = pTBB->nStyle;
	iImage = pTBB->iImage;
}

void CFlatToolbar::SetButtonInfo(int nIndex, UINT nID, UINT nStyle, int iImage)
{
	ASSERT_VALID(this);

	AFX_TBBUTTON* pTBB = _GetButtonPtr(nIndex);
	pTBB->nID = nID;
	pTBB->iImage = iImage;
	pTBB->nStyle = nStyle;
	InvalidateButton(nIndex);
}

void CFlatToolbar::DoPaint(CDC* pDC)
{
	ASSERT_VALID(this);
	ASSERT_VALID(pDC);

#ifdef _MAC
#ifdef _DEBUG
	// turn off validation to speed up button drawing
	int wdSav = WlmDebug(WD_NOVALIDATE | WD_ASSERT);
#endif
#endif

	CControlBar::DoPaint(pDC);		// draw border

	// if no toolbar loaded, don't draw any buttons
	if (m_hbmImageWell == NULL)
		return;

	BOOL bHorz = m_dwStyle & CBRS_ORIENT_HORZ ? TRUE : FALSE;
	CRect rect;
	GetClientRect(rect);
	CalcInsideRect(rect, bHorz);

	// force the full size of the button
	if (bHorz)
		rect.bottom = rect.top + m_sizeButton.cy;
	else
		rect.right = rect.left + m_sizeButton.cx;

	DrawState ds;
	if (!PrepareDrawButton(ds))
		return;     // something went wrong

	// TONYCL: START: OFFICE97 LOOK AND FEEL
	// Only draw the gripper stripes when we are docked
	if (!IsFloating()) {
		{
			CRect rect;
			GetClientRect(rect);

			// Draw the two gripper stripes
			if (bHorz) {
				// Adjust the sizes to fit into the client area properly
				rect.left += 3;
				rect.top += 3;
				rect.bottom -= 3;
				pDC->Draw3dRect(rect.left, rect.top, 3, rect.Height(), globalData.clrBtnHilite, globalData.clrBtnShadow);

				rect.left += 4;
				pDC->Draw3dRect(rect.left, rect.top, 3, rect.Height(), globalData.clrBtnHilite, globalData.clrBtnShadow);
			}
			else {
				// Adjust the sizes to fit into the client area properly
				rect.top += 3;
				rect.left += 3;
				rect.right -= 3;
				pDC->Draw3dRect(rect.left, rect.top, rect.Width(), 3, globalData.clrBtnHilite, globalData.clrBtnShadow);

				rect.top += 4;
				pDC->Draw3dRect(rect.left, rect.top, rect.Width(), 3, globalData.clrBtnHilite, globalData.clrBtnShadow);
			}
		}

		// Shift the buttons 3 pixels to allow for the grippers
		if (bHorz)
			rect.left += 3;
		else
			rect.top += 3;
	}

	// TONYCL: END: OFFICE97 LOOK AND FEEL

	AFX_TBBUTTON* pTBB = (AFX_TBBUTTON*)m_pData;
	for (int iButton = 0; iButton < m_nCount; iButton++, pTBB++)
	{
		ASSERT(pTBB != NULL);
		if (pTBB->nStyle & TBBS_SEPARATOR)
		{
			// separator
			if (bHorz)
				rect.right = rect.left + pTBB->iImage;
			else
				rect.bottom = rect.top + pTBB->iImage;

			// TONYCL: START: OFFICE97 LOOK AND FEEL
			{
				// MATTGR
				// the +1 moved the seperator too far
				// Draw a 3D seperator
				if (bHorz) {
					int nOffset;
					nOffset = rect.left /* +1 */ + ((rect.Width() - 2) / 2);
					pDC->Draw3dRect(nOffset, rect.top, 2, rect.Height(), globalData.clrBtnShadow, globalData.clrBtnHilite);
				}
				else {
					int nOffset;
					nOffset = rect.top /* +1 */ + ((rect.Height() - 2) / 2);
					pDC->Draw3dRect(rect.left, nOffset, rect.Width(), 2, globalData.clrBtnShadow, globalData.clrBtnHilite);
				}
			}
			// TONYCL: END: OFFICE97 LOOK AND FEEL
		}
		else
		{
			if (bHorz)
				rect.right = rect.left + m_sizeButton.cx;
			else
				rect.bottom = rect.top + m_sizeButton.cy;
			if (!globalData.bWin32s || pDC->RectVisible(&rect))
			{
				DrawButton(pDC, rect.left, rect.top,
					pTBB->iImage, pTBB->nStyle);
			}
		}
		// adjust for overlap
		if (bHorz)
			rect.left = rect.right - m_cxSharedBorder;
		else
			rect.top = rect.bottom - m_cySharedBorder;
	}
	EndDrawButton(ds);

#ifdef _MAC
#ifdef _DEBUG
	WlmDebug(wdSav);
#endif
#endif
}

void CFlatToolbar::InvalidateButton(int nIndex)
{
	ASSERT_VALID(this);

	CRect rect;
	GetItemRect(nIndex, &rect);
	InvalidateRect(rect, FALSE);    // don't erase background
}

int CFlatToolbar::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	ASSERT_VALID(this);

	// check child windows first by calling CControlBar
	int nHit = CControlBar::OnToolHitTest(point, pTI);
	if (nHit != -1)
		return nHit;

	// now hit test against CFlatToolbar buttons
	nHit = ((CFlatToolbar*)this)->HitTest(point);
	if (nHit != -1)
	{
		AFX_TBBUTTON* pTBB = _GetButtonPtr(nHit);
		if (pTI != NULL)
		{
			GetItemRect(nHit, &pTI->rect);
			pTI->uId = pTBB->nID;
			pTI->hwnd = m_hWnd;
			pTI->lpszText = LPSTR_TEXTCALLBACK;
		}
		nHit = pTBB->nID;
	}
	return nHit;
}

int CFlatToolbar::HitTest(CPoint point) // in window relative coords
{
	if (m_pData == NULL)
		return -1;	// no buttons

	BOOL bHorz = (m_dwStyle & CBRS_ORIENT_HORZ) ? TRUE : FALSE;
	CRect rect;
	rect.SetRectEmpty();        // only need top and left
	CalcInsideRect(rect, bHorz);
	AFX_TBBUTTON* pTBB = (AFX_TBBUTTON*)m_pData;
	ASSERT(pTBB != NULL);
	if (bHorz)
	{
		if (point.y < rect.top || point.y >= rect.top + m_sizeButton.cy)
			return -1;      // no Y hit
		for (int iButton = 0; iButton < m_nCount; iButton++, pTBB++)
		{
			if (point.x < rect.left)
				break;      // missed it
			rect.left += (pTBB->nStyle & TBBS_SEPARATOR) ?
							pTBB->iImage : m_sizeButton.cx;
			if (point.x < rect.left && !(pTBB->nStyle & TBBS_SEPARATOR))
				return iButton;     // hit !
			rect.left -= m_cxSharedBorder;    // go back for overlap
		}
	}
	else
	{
		if (point.x < rect.left || point.x >= rect.left + m_sizeButton.cx)
			return -1;      // no X hit
		for (int iButton = 0; iButton < m_nCount; iButton++, pTBB++)
		{
			if (point.y < rect.top)
				break;      // missed it
			rect.top += (pTBB->nStyle & TBBS_SEPARATOR) ?
							pTBB->iImage : m_sizeButton.cy;
			if (point.y < rect.top && !(pTBB->nStyle & TBBS_SEPARATOR))
				return iButton;     // hit !
			rect.top -= m_cySharedBorder;    // go back for overlap
		}
	}

	return -1;      // nothing hit
}

/////////////////////////////////////////////////////////////////////////////
// CFlatToolbar message handlers

BEGIN_MESSAGE_MAP(CFlatToolbar, CControlBar)
	//{{AFX_MSG_MAP(CFlatToolbar)
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_CANCELMODE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_WINDOWPOSCHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CFlatToolbar::OnLButtonDown(UINT nFlags, CPoint point)
{
	if ((m_iButtonCapture = HitTest(point)) < 0) // nothing hit
	{
		CControlBar::OnLButtonDown(nFlags, point);
		return;
	}

	AFX_TBBUTTON* pTBB = _GetButtonPtr(m_iButtonCapture);
	ASSERT(!(pTBB->nStyle & TBBS_SEPARATOR));

	// update the button before checking for disabled status
	UpdateButton(m_iButtonCapture);
	if (pTBB->nStyle & TBBS_DISABLED)
	{
		m_iButtonCapture = -1;
		return;     // don't press it
	}

	// TONYCL: START:
	// Kill the UPSTATE style so that the button is drawn in the downstate
	pTBB->nStyle &= ~TBBS_UPSTATE;
	// TONYCL: END:

	pTBB->nStyle |= TBBS_PRESSED;
	InvalidateButton(m_iButtonCapture);
	UpdateWindow(); // immediate feedback

	SetCapture();
	GetOwner()->SendMessage(WM_SETMESSAGESTRING, (WPARAM)pTBB->nID);
}

void CFlatToolbar::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (m_iButtonCapture >= 0)
	{
		AFX_TBBUTTON* pTBB = _GetButtonPtr(m_iButtonCapture);
		ASSERT(!(pTBB->nStyle & TBBS_SEPARATOR));

		UINT nNewStyle = (pTBB->nStyle & ~TBBS_PRESSED);
		int iButtonCapture = m_iButtonCapture;
		if (GetCapture() != this)
		{
			m_iButtonCapture = -1; // lost capture
			GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);
		}
		else
		{
			// should be pressed if still hitting the captured button
			if (HitTest(point) == m_iButtonCapture)
				nNewStyle |= TBBS_PRESSED;
		}
		SetButtonStyle(iButtonCapture, nNewStyle);
		UpdateWindow(); // immediate feedback
	}
// TONYCL: START: OFFICE97 LOOK AND FEEL
	else {
		// We have to be the active application
		if (GetForegroundWindow()->GetSafeHwnd() == AfxGetMainWnd()->GetSafeHwnd()) {
			// Hit test the button, and then draw the button in the up state
			int nButtonID = HitTest(point);		// Which button are we over?

			if (nButtonID == -1) {
				ReleaseCapture();
			}
			else {
				SetCapture();
			}

			// Are we over the same button as we were last time?
			if (m_nUpButtonIndex == nButtonID) {
				return;
			}
 			// Is the new button -1, and were we over a button?
			else if ((nButtonID == -1) && (m_nUpButtonIndex != -1)) {
				// Reset the button state so it's flat
				UINT nStyle = GetButtonStyle(m_nUpButtonIndex);
				nStyle &= ~TBBS_UPSTATE;
				SetButtonStyle(m_nUpButtonIndex, nStyle);
				InvalidateButton(m_nUpButtonIndex);
				m_nUpButtonIndex = -1;
				UpdateWindow(); // immediate feedback
			}
			// We are over a button
			else if (nButtonID != -1 && (!(GetButtonStyle(nButtonID) & TBBS_DISABLED))) {
				// Put the button into the upstate
				UINT nStyle = GetButtonStyle(nButtonID);
				nStyle |= TBBS_UPSTATE;
				SetButtonStyle(nButtonID, nStyle);
				InvalidateButton(nButtonID);

				// If we were over a different button to the one before, flatten it
				if (m_nUpButtonIndex != -1) {
					UINT nStyle = GetButtonStyle(m_nUpButtonIndex);
					nStyle &= ~TBBS_UPSTATE;
					SetButtonStyle(m_nUpButtonIndex, nStyle);
					InvalidateButton(m_nUpButtonIndex);
				}

				m_nUpButtonIndex = nButtonID;
				UpdateWindow(); // immediate feedback
			}
		}
	}
// TONYCL: END: OFFICE97 LOOK AND FEEL
}

void CFlatToolbar::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_iButtonCapture < 0)
	{
		CControlBar::OnLButtonUp(nFlags, point);
		return;     // not captured
	}

	AFX_TBBUTTON* pTBB = _GetButtonPtr(m_iButtonCapture);
	ASSERT(!(pTBB->nStyle & TBBS_SEPARATOR));
	UINT nIDCmd = 0;

	UINT nNewStyle = (pTBB->nStyle & ~TBBS_PRESSED);
	if (GetCapture() == this)
	{
		// we did not lose the capture
		ReleaseCapture();
		if (HitTest(point) == m_iButtonCapture)
		{
			// give button a chance to update
			UpdateButton(m_iButtonCapture);

			// then check for disabled state
			if (!(pTBB->nStyle & TBBS_DISABLED))
			{
				// pressed, will send command notification
				nIDCmd = pTBB->nID;

				if (pTBB->nStyle & TBBS_CHECKBOX)
				{
					// auto check: three state => down
					if (nNewStyle & TBBS_INDETERMINATE)
						nNewStyle &= ~TBBS_INDETERMINATE;

					nNewStyle ^= TBBS_CHECKED;
				}
			}
		}
	}

	GetOwner()->SendMessage(WM_SETMESSAGESTRING, AFX_IDS_IDLEMESSAGE);

	int iButtonCapture = m_iButtonCapture;
	m_iButtonCapture = -1;
	if (nIDCmd != 0)
		GetOwner()->SendMessage(WM_COMMAND, nIDCmd);    // send command

	SetButtonStyle(iButtonCapture, nNewStyle);
	UpdateButton(iButtonCapture);

	UpdateWindow(); // immediate feedback
}

// TONYCL: START: OFFICE97 LOOK AND FEEL
void CFlatToolbar::OnWindowPosChanged( WINDOWPOS* lpwndpos )
{
	// Force a repaint of the window to fix a repaint bug
	InvalidateRect(NULL);
	UpdateWindow();
}
// TONYCL: END: OFFICE97 LOOK AND FEEL

void CFlatToolbar::OnCancelMode()
{
	CControlBar::OnCancelMode();

	if (m_iButtonCapture >= 0)
	{
		AFX_TBBUTTON* pTBB = _GetButtonPtr(m_iButtonCapture);
		ASSERT(!(pTBB->nStyle & TBBS_SEPARATOR));
		UINT nNewStyle = (pTBB->nStyle & ~TBBS_PRESSED);
		if (GetCapture() == this)
			ReleaseCapture();
		SetButtonStyle(m_iButtonCapture, nNewStyle);
		m_iButtonCapture = -1;
		UpdateWindow();
	}
}

void CFlatToolbar::OnSysColorChange()
{
#ifdef _MAC
	CControlBar::OnSysColorChange();

	ASSERT(hDCGlyphs != NULL);
	VERIFY(::DeleteDC(hDCGlyphs));
	hDCGlyphs = ::CreateCompatibleDC(NULL);

	ASSERT(hDCMono != NULL);
	VERIFY(::DeleteDC(hDCMono));
	hDCMono = ::CreateCompatibleDC(NULL);
#endif

	// re-initialize global dither brush
#ifndef _MAC
	HBITMAP hbmGray = ::CreateDitherBitmap();
#else
	HBITMAP hbmGray = ::CreateDitherBitmap(m_bMonochrome);
#endif
	if (hbmGray != NULL)
	{
		HBRUSH hbrNew = ::CreatePatternBrush(hbmGray);
		if (hbrNew != NULL)
		{
			AfxDeleteObject((HGDIOBJ*)&hbrDither);      // free old one
			hbrDither = hbrNew;
		}
		::DeleteObject(hbmGray);
	}

	// re-color bitmap for toolbar
	if (m_hbmImageWell != NULL)
	{
		HBITMAP hbmNew;
#ifndef _MAC
		hbmNew = LoadSysColorBitmap(m_hInstImageWell, m_hRsrcImageWell);
#else
		hbmNew = LoadSysColorBitmap(m_hInstImageWell, m_hRsrcImageWell,
			m_hDCGlyphs, m_bMonochrome);
#endif
		if (hbmNew != NULL)
		{
			::DeleteObject(m_hbmImageWell);     // free old one
			m_hbmImageWell = hbmNew;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFlatToolbar idle update through CToolCmdUI class

#define CToolCmdUI COldToolCmdUI

class CToolCmdUI : public CCmdUI        // class private to this file !
{
public: // re-implementations only
	virtual void Enable(BOOL bOn);
	virtual void SetCheck(int nCheck);
	virtual void SetText(LPCTSTR lpszText);
};

void CToolCmdUI::Enable(BOOL bOn)
{
	m_bEnableChanged = TRUE;
	CFlatToolbar* pToolBar = (CFlatToolbar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CFlatToolbar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pToolBar->GetButtonStyle(m_nIndex) & ~TBBS_DISABLED;
	if (!bOn)
		nNewStyle |= TBBS_DISABLED;
	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->SetButtonStyle(m_nIndex, nNewStyle);
}

void CToolCmdUI::SetCheck(int nCheck)
{
	ASSERT(nCheck >= 0 && nCheck <= 2); // 0=>off, 1=>on, 2=>indeterminate
	CFlatToolbar* pToolBar = (CFlatToolbar*)m_pOther;
	ASSERT(pToolBar != NULL);
	ASSERT_KINDOF(CFlatToolbar, pToolBar);
	ASSERT(m_nIndex < m_nIndexMax);

	UINT nNewStyle = pToolBar->GetButtonStyle(m_nIndex) &
				~(TBBS_CHECKED | TBBS_INDETERMINATE);
	if (nCheck == 1)
		nNewStyle |= TBBS_CHECKED;
	else if (nCheck == 2)
		nNewStyle |= TBBS_INDETERMINATE;
	ASSERT(!(nNewStyle & TBBS_SEPARATOR));
	pToolBar->SetButtonStyle(m_nIndex, nNewStyle | TBBS_CHECKBOX);
}

void CToolCmdUI::SetText(LPCTSTR)
{
	// ignore it
}

void CFlatToolbar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CToolCmdUI state;
	state.m_pOther = this;

	state.m_nIndexMax = (UINT)m_nCount;
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
	  state.m_nIndex++)
	{
		AFX_TBBUTTON* pTBB = _GetButtonPtr(state.m_nIndex);
		state.m_nID = pTBB->nID;

		// ignore separators
		if (!(pTBB->nStyle & TBBS_SEPARATOR))
			state.DoUpdate(pTarget, bDisableIfNoHndler);
	}

	// update the dialog controls added to the toolbar
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}

void CFlatToolbar::UpdateButton(int nIndex)
{
	// determine target of command update
	CFrameWnd* pTarget = (CFrameWnd*)GetOwner();
	if (pTarget == NULL || !pTarget->IsFrameWnd())
		pTarget = GetParentFrame();

	// send the update notification
	if (pTarget != NULL)
	{
		CToolCmdUI state;
		state.m_pOther = this;
		state.m_nIndex = nIndex;
		state.m_nIndexMax = (UINT)m_nCount;
		AFX_TBBUTTON* pTBB = _GetButtonPtr(nIndex);
		state.m_nID = pTBB->nID;
		state.DoUpdate(pTarget, pTarget->m_bAutoMenuEnable);
	}
}

/////////////////////////////////////////////////////////////////////////////
// CFlatToolbar diagnostics

#ifdef _DEBUG
void CFlatToolbar::AssertValid() const
{
	CControlBar::AssertValid();
	ASSERT(m_hbmImageWell == NULL ||
		(globalData.bWin32s || ::GetObjectType(m_hbmImageWell) == OBJ_BITMAP));

	if (m_hbmImageWell != NULL)
	{
		ASSERT(m_hRsrcImageWell != NULL);
		ASSERT(m_hInstImageWell != NULL);
	}
}

void CFlatToolbar::Dump(CDumpContext& dc) const
{
	CControlBar::Dump(dc);

	dc << "m_hbmImageWell = " << (UINT)m_hbmImageWell;
	dc << "\nm_hInstImageWell = " << (UINT)m_hInstImageWell;
	dc << "\nm_hRsrcImageWell = " << (UINT)m_hRsrcImageWell;
	dc << "\nm_iButtonCapture = " << m_iButtonCapture;
	dc << "\nm_sizeButton = " << m_sizeButton;
	dc << "\nm_sizeImage = " << m_sizeImage;

	if (dc.GetDepth() > 0)
	{
		for (int i = 0; i < m_nCount; i++)
		{
			AFX_TBBUTTON* pTBB = _GetButtonPtr(i);
			dc << "\ntoolbar button[" << i << "] = {";
			dc << "\n\tnID = " << pTBB->nID;
			dc << "\n\tnStyle = " << pTBB->nStyle;
			if (pTBB->nStyle & TBBS_SEPARATOR)
				dc << "\n\tiImage (separator width) = " << pTBB->iImage;
			else
				dc <<"\n\tiImage (bitmap image index) = " << pTBB->iImage;
			dc << "\n}";
		}
	}

	dc << "\n";
}
#endif

#undef new
#ifdef AFX_INIT_SEG
#pragma code_seg(AFX_INIT_SEG)
#endif

IMPLEMENT_DYNAMIC(CFlatToolbar, CControlBar)

/////////////////////////////////////////////////////////////////////////////
// Cached system metrics, etc

GLOBAL_DATA globalData;

// Initialization code
GLOBAL_DATA::GLOBAL_DATA()
{
	// Cache various target platform version information
	DWORD dwVersion = ::GetVersion();
	nWinVer = (LOBYTE(dwVersion) << 8) + HIBYTE(dwVersion);
	bWin32s = (dwVersion & 0x80000000) != 0;
	bWin4 = (BYTE)dwVersion >= 4;
	bNotWin4 = 1 - bWin4;   // for convenience
#ifndef _MAC
	bSmCaption = bWin4;
#else
	bSmCaption = TRUE;
#endif
	bWin31 = bWin32s && !bWin4; // Windows 95 reports Win32s

	// Cached system metrics (updated in CWnd::OnWinIniChange)
	UpdateSysMetrics();

	// Border attributes
	hbrLtGray = ::CreateSolidBrush(RGB(192, 192, 192));
	hbrDkGray = ::CreateSolidBrush(RGB(128, 128, 128));
	ASSERT(hbrLtGray != NULL);
	ASSERT(hbrDkGray != NULL);

	// Cached system values (updated in CWnd::OnSysColorChange)
	hbrBtnFace = NULL;
	hbrBtnShadow = NULL;
	hbrBtnHilite = NULL;
	hbrWindowFrame = NULL;
	hpenBtnShadow = NULL;
	hpenBtnHilite = NULL;
	hpenBtnText = NULL;
	UpdateSysColors();

	// cxBorder2 and cyBorder are 2x borders for Win4
	cxBorder2 = bWin4 ? CX_BORDER*2 : CX_BORDER;
	cyBorder2 = bWin4 ? CY_BORDER*2 : CY_BORDER;

	// allocated on demand
	hStatusFont = NULL;
	hToolTipsFont = NULL;
}

// Termination code
GLOBAL_DATA::~GLOBAL_DATA()
{
	// cleanup standard brushes
	AfxDeleteObject((HGDIOBJ*)&hbrLtGray);
	AfxDeleteObject((HGDIOBJ*)&hbrDkGray);
	AfxDeleteObject((HGDIOBJ*)&hbrBtnFace);
	AfxDeleteObject((HGDIOBJ*)&hbrBtnShadow);
	AfxDeleteObject((HGDIOBJ*)&hbrBtnHilite);
	AfxDeleteObject((HGDIOBJ*)&hbrWindowFrame);

	// cleanup standard pens
	AfxDeleteObject((HGDIOBJ*)&hpenBtnShadow);
	AfxDeleteObject((HGDIOBJ*)&hpenBtnHilite);
	AfxDeleteObject((HGDIOBJ*)&hpenBtnText);

	// clean up objects we don't actually create
	AfxDeleteObject((HGDIOBJ*)&hStatusFont);
	AfxDeleteObject((HGDIOBJ*)&hToolTipsFont);
}

void GLOBAL_DATA::UpdateSysColors()
{
	clrBtnFace = ::GetSysColor(COLOR_BTNFACE);
	clrBtnShadow = ::GetSysColor(COLOR_BTNSHADOW);
	clrBtnHilite = ::GetSysColor(COLOR_BTNHIGHLIGHT);
	clrBtnText = ::GetSysColor(COLOR_BTNTEXT);
	clrWindowFrame = ::GetSysColor(COLOR_WINDOWFRAME);

	AfxDeleteObject((HGDIOBJ*)&hbrBtnFace);
	AfxDeleteObject((HGDIOBJ*)&hbrBtnShadow);
	AfxDeleteObject((HGDIOBJ*)&hbrBtnHilite);
	AfxDeleteObject((HGDIOBJ*)&hbrWindowFrame);

	hbrBtnFace = ::CreateSolidBrush(clrBtnFace);
	ASSERT(hbrBtnFace != NULL);
	hbrBtnShadow = ::CreateSolidBrush(clrBtnShadow);
	ASSERT(hbrBtnShadow != NULL);
	hbrBtnHilite = ::CreateSolidBrush(clrBtnHilite);
	ASSERT(hbrBtnHilite != NULL);
	hbrWindowFrame = ::CreateSolidBrush(clrWindowFrame);
	ASSERT(hbrWindowFrame != NULL);

	AfxDeleteObject((HGDIOBJ*)&hpenBtnShadow);
	AfxDeleteObject((HGDIOBJ*)&hpenBtnHilite);
	AfxDeleteObject((HGDIOBJ*)&hpenBtnText);

	hpenBtnShadow = ::CreatePen(PS_SOLID, 0, clrBtnShadow);
	ASSERT(hpenBtnShadow != NULL);
	hpenBtnHilite = ::CreatePen(PS_SOLID, 0, clrBtnHilite);
	ASSERT(hpenBtnHilite != NULL);
	hpenBtnText = ::CreatePen(PS_SOLID, 0, clrBtnText);
	ASSERT(hpenBtnText != NULL);
}

void GLOBAL_DATA::UpdateSysMetrics()
{
	// Device metrics for screen
	HDC hDCScreen = GetDC(NULL);
	ASSERT(hDCScreen != NULL);
	cxPixelsPerInch = GetDeviceCaps(hDCScreen, LOGPIXELSX);
	cyPixelsPerInch = GetDeviceCaps(hDCScreen, LOGPIXELSY);
	ReleaseDC(NULL, hDCScreen);
}

/////////////////////////////////////////////////////////////////////////////
