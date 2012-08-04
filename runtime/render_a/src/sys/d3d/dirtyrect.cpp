//	dirtyrect.cpp
//
//	Implementation of dirty-rect flip.

#include "precompile.h"
#include "common_init.h"
#include "d3d_shell.h"
#include "d3d_device.h"
#include "d3d_surface.h"
#include "d3d_init.h"
#include "dirtyrect.h"

#define	MAX_INVALID_RECTS		100

LTRect	g_invalidRect[MAX_INVALID_RECTS];
uint32	g_invalidRectCount=0;

inline bool RectangleContains(LTRect *outer, LTRect *inner)
{
	return (outer->left <= inner->left && outer->top <= inner->top && outer->right >= inner->right && outer->bottom >= inner->bottom);
}

inline LTRect RectangleCombine(LTRect *r1, LTRect *r2)
{
	LTRect r;
	r.left   = (r1->left   < r2->left  ) ? r1->left   : r2->left;
	r.top    = (r1->top    < r2->top   ) ? r1->top    : r2->top;
	r.right  = (r1->right  > r2->right ) ? r1->right  : r2->right;
	r.bottom = (r1->bottom > r2->bottom) ? r1->bottom : r2->bottom;
	return r;
}

inline uint32 RectangleArea(LTRect *r)
{
	return (r->right - r->left) * (r->bottom - r->top);
}

void InvalidateRect(LTRect *pRect)
{
	if (pRect==NULL) {						// Invalidate the whole screen...
		g_invalidRect[0].left	= 0;
		g_invalidRect[0].top	= 0;
		g_invalidRect[0].right	= g_ScreenWidth;
		g_invalidRect[0].bottom	= g_ScreenHeight;
		g_invalidRectCount		= 1;
		return; }

	if (g_invalidRectCount==0) {
		g_invalidRect[g_invalidRectCount] = *pRect;
		++g_invalidRectCount; }
	else {									// Attempt to combine this invalid area with one we already have...
		uint32 i;
		LTRect comb;
		int area1, area2, areac, areaBest;
		int bestRef = -1;

		if (g_invalidRectCount>=MAX_INVALID_RECTS-1) {
			InvalidateRect(NULL); return; }

		for (i=0; i<g_invalidRectCount; i++) {
			if (RectangleContains(&g_invalidRect[i], pRect)) return; }
		for (i=0; i<g_invalidRectCount; i++) {
			comb = RectangleCombine(&g_invalidRect[i], pRect);
			area1 = RectangleArea(&g_invalidRect[i]);
			area2 = RectangleArea(pRect);
			areac = RectangleArea(&comb);
			if (areac < area1 + area2) {
				if (bestRef < 0 || area1 + area2 - areac > areaBest) {
					areaBest = area1 + area2 - areac;
					bestRef = i; } } }
		if (bestRef >= 0)
			g_invalidRect[bestRef] = RectangleCombine(&g_invalidRect[bestRef], pRect);
		else
			g_invalidRect[g_invalidRectCount++] = *pRect; }
}

//--------------------------------------------
// This routine copies the dirty rectanges from the back buffer to the front buffer...
//--------------------------------------------
void DirtyRectSwap()
{
	if (g_invalidRectCount==0) return;	// nothing to draw! 

	RGNDATA	RegionData;
	RegionData.rdh.dwSize			= sizeof(RegionData.rdh);
	RegionData.rdh.iType			= RDH_RECTANGLES;
	RegionData.rdh.nCount			= g_invalidRectCount;
	RegionData.rdh.nRgnSize			= g_invalidRectCount * sizeof(RECT);
	RegionData.rdh.rcBound.top		= 0;								RegionData.rdh.rcBound.left		= 0;
	RegionData.rdh.rcBound.bottom	= g_Device.GetModeInfo()->Height;	RegionData.rdh.rcBound.right	= g_Device.GetModeInfo()->Width;

	HRESULT hResult = PD3DDEVICE->Present(NULL,NULL,NULL,&RegionData);
}

void ClearDirtyRects()
{
	g_invalidRectCount	= 0;
}