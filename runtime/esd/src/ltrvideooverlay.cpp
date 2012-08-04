/****************************************************************************
;
;	MODULE:		LTRVideoOverlay (.CPP)
;
;	PURPOSE:	Support class for RealVideoPlayer
;
;	HISTORY:	6-22-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD

#include "LTRVideoOverlay.h"
#include "iltesd.h"
#include "interface_helpers.h"
extern int32 g_ScreenWidth, g_ScreenHeight;

//the ILTClient game interface
#include "iltclient.h"
static ILTClient *ilt_client;
define_holder(ILTClient, ilt_client);

#define ESD_MIN(a, b)	(a <= b) ? a : b;
#define ESD_MAX(a, b)	(a >= b) ? a : b;

//-----------------------------------------------------------------------------
// CLTRealVideoOverlay member functions
//-----------------------------------------------------------------------------
CLTRealVideoOverlay::CLTRealVideoOverlay()
{
	m_hSurface = LTNULL;
}

//-----------------------------------------------------------------------------
CLTRealVideoOverlay::~CLTRealVideoOverlay()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoOverlay::Init(uint32 left, uint32 top, uint32 right, uint32 bottom, DWORD dwFlags)
{
	// Valid rect?
	if (left >= right && 0 != right)
		return LT_ERROR;
	if (top >= bottom && 0 != bottom)
		return LT_ERROR;

	// Valid flags? [mds] add more checks?
	if (LTRV_LEFTJUST & dwFlags && LTRV_RIGHTJUST & dwFlags)
		return LT_ERROR;
	if (LTRV_TOPJUST & dwFlags && LTRV_BOTTOMJUST & dwFlags)
		return LT_ERROR;

	m_Rect.Init(left, top, right, bottom);
	m_dwFlags = dwFlags;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoOverlay::Term()
{
	DeleteSurface();

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoOverlay::CreateSurface(uint32 width, uint32 height)
{
	if (m_hSurface)
		return LT_ERROR;

	// Create the surface
	m_hSurface = ilt_client->CreateSurface(width, height);

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoOverlay::DeleteSurface()
{
	if (m_hSurface)
	{
		ilt_client->DeleteSurface(m_hSurface);
		m_hSurface = LTNULL;
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoOverlay::Render()
{
	if (m_hSurface)
	{
		LTRect renderedRect(m_Rect.left, m_Rect.top, m_Rect.right, m_Rect.bottom);

		LTRect screenRect(0, 0, g_ScreenWidth, g_ScreenHeight);
		uint32 uVideoWidth = 0, uVideoHeight = 0;
		ilt_client->GetSurfaceDims(m_hSurface, &uVideoWidth, &uVideoHeight);
		LTRect videoRect(0, 0, uVideoWidth, uVideoHeight);

		// Fullscreen takes priority
		if (LTRV_FULLSCREEN & m_dwFlags)
		{
			ilt_client->ScaleSurfaceToSurface((HSURFACE)&g_ScreenSurface, m_hSurface, &screenRect, &videoRect);
			return LT_OK;
		}

		// If they opted not to set the rect, we'll do it now
		if (0 == renderedRect.right)
			renderedRect.right = renderedRect.left + uVideoWidth;
		if (0 == renderedRect.bottom)
			renderedRect.bottom = renderedRect.top + uVideoHeight;

		// Double the size?
		if (LTRV_DOUBLESIZE & m_dwFlags)
		{
			int32 uWidth = renderedRect.right - renderedRect.left;
			int32 uHeight = renderedRect.bottom - renderedRect.top;
			renderedRect.right += uWidth;
			renderedRect.bottom += uHeight;
		}

		// Justify?
		if (LTRV_LEFTJUST & m_dwFlags)
		{
			int32 uShiftDistance = renderedRect.left;
			renderedRect.left -= uShiftDistance;
			renderedRect.right -= uShiftDistance;
		}
		if (LTRV_RIGHTJUST & m_dwFlags)
		{
			int32 uShiftDistance = g_ScreenWidth - renderedRect.right;
			renderedRect.left += uShiftDistance;
			renderedRect.right += uShiftDistance;
		}
		if ((LTRV_LEFTJUST & m_dwFlags || LTRV_RIGHTJUST & m_dwFlags) &&
				LTRV_CENTERJUST & m_dwFlags)
		{
			int32 uHeight = renderedRect.bottom - renderedRect.top;
			int32 uCenterScreen = g_ScreenHeight / 2;
			renderedRect.top = uCenterScreen - (uHeight / 2);
			renderedRect.bottom = uCenterScreen + (uHeight / 2);
		}
		if (LTRV_TOPJUST & m_dwFlags)
		{
			int32 uShiftDistance = renderedRect.top;
			renderedRect.top -= uShiftDistance;
			renderedRect.bottom -= uShiftDistance;
		}
		if (LTRV_BOTTOMJUST & m_dwFlags)
		{
			int32 uShiftDistance = g_ScreenHeight - renderedRect.bottom;
			renderedRect.top += uShiftDistance;
			renderedRect.bottom += uShiftDistance;
		}
		if ((LTRV_TOPJUST & m_dwFlags || LTRV_BOTTOMJUST & m_dwFlags) &&
				LTRV_CENTERJUST & m_dwFlags)
		{
			int32 uWidth = renderedRect.right - renderedRect.left;
			int32 uCenterScreen = g_ScreenWidth / 2;
			renderedRect.left = uCenterScreen - (uWidth / 2);
			renderedRect.right = uCenterScreen + (uWidth / 2);
		}
		if (!(LTRV_LEFTJUST & m_dwFlags) &&
			!(LTRV_TOPJUST & m_dwFlags) &&
			!(LTRV_RIGHTJUST & m_dwFlags) &&
			!(LTRV_BOTTOMJUST & m_dwFlags) &&
			 (LTRV_CENTERJUST & m_dwFlags))
		{
			int32 uHeight = renderedRect.bottom - renderedRect.top;
			int32 uCenterScreen = g_ScreenHeight / 2;
			renderedRect.top = uCenterScreen - (uHeight / 2);
			renderedRect.bottom = uCenterScreen + (uHeight / 2);
			int32 uWidth = renderedRect.right - renderedRect.left;
			uCenterScreen = g_ScreenWidth / 2;
			renderedRect.left = uCenterScreen - (uWidth / 2);
			renderedRect.right = uCenterScreen + (uWidth / 2);
		}

		// Clamp values (clip instead?)
		renderedRect.left = ESD_MAX(renderedRect.left, 0);
		renderedRect.top = ESD_MAX(renderedRect.top, 0);
		renderedRect.right = ESD_MIN(renderedRect.right, g_ScreenWidth);
		renderedRect.bottom = ESD_MIN(renderedRect.bottom, g_ScreenHeight);

		// Create intermediate surface
		uint32 uIntWidth = renderedRect.right - renderedRect.left;
		uint32 uIntHeight = renderedRect.bottom - renderedRect.top;
		HSURFACE hIntSurface = ilt_client->CreateSurface(uIntWidth, uIntHeight);

		// Scale original surface to intermediate surface
		ilt_client->ScaleSurfaceToSurface(hIntSurface, m_hSurface, LTNULL, &videoRect);
		
		// Draw the intermediate surface to the screen surface
		ilt_client->DrawSurfaceToSurface((HSURFACE)&g_ScreenSurface, hIntSurface, LTNULL, renderedRect.left, renderedRect.top);

		// Destroy the intermediate surface
		ilt_client->DeleteSurface(hIntSurface);
	}

	return LT_OK;
}
#endif // LITHTECH_ESD