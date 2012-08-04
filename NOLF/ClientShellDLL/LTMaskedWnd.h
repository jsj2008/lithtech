/****************************************************************************
;
;	 MODULE:		CLTMaskedWnd (.h)
;
;	PURPOSE:		Class for a window with an irregular shape (like a bitmap)
;
;	HISTORY:		12/10/98 [kml] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions, Inc.
;
****************************************************************************/
#ifndef _LTMASKEDWND_H_
#define _LTMASKEDWND_H_

// Includes
#include "LTWnd.h"

// Classes
class CLTMaskedWnd : public CLTWnd
{
public:

	CLTMaskedWnd();
	virtual ~CLTMaskedWnd() { Term(); }
	virtual void Term();

	void EnableSurfaceTest(BOOL bEnable) { m_bSurfaceTest = bEnable; }
	virtual BOOL PtInWnd(int x, int y);  // x,y are screen coords

protected:
	BOOL m_bSurfaceTest;
};

// Inlines
inline CLTMaskedWnd::CLTMaskedWnd()
{
	m_bSurfaceTest = TRUE;
}

inline void CLTMaskedWnd::Term()
{
	CLTWnd::Term();
}

#endif