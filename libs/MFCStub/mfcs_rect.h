// mfcs_rect.h - MFC Stub CRect class header

#ifndef __MFCS_RECT_H__
#define __MFCS_RECT_H__

#include "..\mfcstub\mfcs_types.h"
#include "..\mfcstub\mfcs_point.h"

#ifndef _WINDOWS_
	// Make sure RECT is defined...
	typedef struct tagRECT {
		tagRECT() {}
		int32 left, top, right, bottom;
	} RECT;
#endif

class CRect : public RECT
{
public:
	CRect();
	CRect(const RECT &cRect);
	CRect(const RECT *pRect);
	CRect(int32 l, int32 t, int32 r, int32 b);
	CRect(POINT topLeft, POINT bottomRight);
	operator RECT*() { return this; }
	operator const RECT*() const { return this; }

	void SetRect(int32 l, int32 t, int32 r, int32 b);
	int32 Width() const { return right - left; }
	int32 Height() const { return bottom - top; }
	LTBOOL IntersectRect(const RECT *lpRect1, const RECT *lpRect2);
	void InflateRect(int l, int t, int r, int b) { left -= l; top -= t; right += r; bottom += b; }
	void DeflateRect(int l, int t, int r, int b) { left += l; top += t; right -= r; bottom -= b; }
	CPoint& TopLeft();
	CPoint& BottomRight();
	const CPoint& TopLeft() const;
	const CPoint& BottomRight() const;
	void NormalizeRect();

	void operator&=(const RECT& rect) { IntersectRect(*this, &rect); }
	void operator+=(POINT point) { InflateRect(-point.x, -point.y, point.x, point.y); }
	void operator+=(const RECT *lpRect) { InflateRect(lpRect->left, lpRect->top, lpRect->right, lpRect->bottom); }
	void operator-=(POINT point) { InflateRect(point.x, point.y, -point.x, -point.y); }
	void operator-=(const RECT *lpRect) { InflateRect(-lpRect->left, -lpRect->top, -lpRect->right, -lpRect->bottom); }
};

#endif // __MFCS_RECT_H__