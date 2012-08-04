/****************************************************************************
;
;	MODULE:		LTRVideoOverlay (.H)
;
;	PURPOSE:	Support class for RealVideoPlayer
;
;	HISTORY:	6-22-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef CLTRealVideoOverlay_H
#define CLTRealVideoOverlay_H

#define LITHTECH_ESD_INC 1
#include "lith.h"
#include "bdefs.h"
#undef LITHTECH_ESD_INC

class CLTRealVideoOverlay : public CLithBaseListItem<CLTRealVideoOverlay>
{
public:
	CLTRealVideoOverlay();
	~CLTRealVideoOverlay();

	// Note: This is the desired destination rect (if right or bottom are zero, the
	//  default video width or height are used)
	LTRESULT Init(uint32 left, uint32 top, uint32 right, uint32 bottom, DWORD dwFlags);
	LTRESULT Term();

	// Note: This is the width and height of the movie that the surface is created for
	LTRESULT CreateSurface(uint32 width, uint32 height);
	LTRESULT DeleteSurface();
	HSURFACE GetSurface()	{ return m_hSurface; }

	LTRESULT Render();

protected:
	HSURFACE	m_hSurface;
	LTRect		m_Rect;
	DWORD		m_dwFlags;
};

typedef CLithBaseList<CLTRealVideoOverlay> RealVideoOverlayList;

#endif // CLTRealVideoOverlay_H
#endif // LITHTECH_ESD