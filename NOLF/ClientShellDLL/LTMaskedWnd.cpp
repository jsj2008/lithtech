/****************************************************************************
;
;	 MODULE:		LTMaskedWnd (.cpp)
;
;	PURPOSE:		Class for a window with an irregular shape (like a bitmap)
;
;	HISTORY:		12/10/98 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/

#include "StdAfx.h"
#include "LTMaskedWnd.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CLTMaskedWnd::PtInWnd
//
//	PURPOSE:	Determines if a point is in the window
//
// ----------------------------------------------------------------------- //
BOOL CLTMaskedWnd::PtInWnd(int x, int y)
{
	// Make sure that the point is in the window's rectangle
	if(!CLTWnd::PtInWnd(x,y))
		return FALSE;

	// Do the irregular surface test if necessary
	if (m_bSurfaceTest && IsFlagSet(LTWF_TRANSPARENT) && m_hSurf)
	{
        HLTCOLOR hPixelHue;
        LTRESULT dr = g_pLTClient->GetPixel(m_hSurf, x - GetWindowLeft(), y - GetWindowTop(), &hPixelHue);
		if ((dr == LT_OK) && (hPixelHue != GetTransparentColor()))
			return TRUE;

		return FALSE;
	}

	return TRUE;
}