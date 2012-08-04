// ----------------------------------------------------------------------- //
//
// MODULE  : CursorMgr.h
//
// PURPOSE : Manage all mouse cursor related functionality
//
// CREATED : 12/3/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CURSOR_MGR_H__
#define __CURSOR_MGR_H__

#include "iltcursor.h"
#include "ScreenSpriteMgr.h"


class CCursorMgr;
extern CCursorMgr* g_pCursorMgr;

class CCursorMgr
{
public:
	CCursorMgr();
	~CCursorMgr();

	LTBOOL		Init();
	void		Term();

	void		ScheduleReinit(float fDelay);
	void		CheckForReinit();
	
	void		UseHardwareCursor(LTBOOL bUseHardwareCursor, bool bForce = false);
    void		UseCursor(LTBOOL bUseCursor, LTBOOL bLockCursorToCenter = LTFALSE);
	void		Update();

	void		SetCenter(int x, int y);

	// Some functions for controlling the appearance of the cursor
	void		UseSprite(CScreenSprite * pSprite);

	void		UseSprite(char * pFile);	// can be spr or dtx, returns ID
	void		UseGlowSprite(char * pFile);
	void		UseBackgroundSprite(char * pFile);

	void		KillSprite();

	// TODO at some point in the future, we can allow multiple sprites and other FX here.

private:

	LTBOOL		m_bInitialized;
    LTBOOL      m_bUseCursor;
    LTBOOL      m_bUseHardwareCursor;

	LTIntPt		m_CursorCenter;

	// old-style cursor
	HLTCURSOR	m_hCursor;
	HSURFACE	m_hSurfCursor;

	ScreenSpriteArray m_SpriteArray;
	CScreenSprite * m_pCursorSprite;
	CScreenSprite * m_pCursorGlowSprite;
	CScreenSprite * m_pCursorBackgroundSprite;
};

#endif // __CURSOR_MGR_H__