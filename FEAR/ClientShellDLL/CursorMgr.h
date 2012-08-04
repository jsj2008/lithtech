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


class CCursorMgr;
extern CCursorMgr* g_pCursorMgr;

class CCursorMgr
{
public:
	CCursorMgr();
	~CCursorMgr();

	bool		Init();
	void		Term();

	void		ScheduleReinit(float fDelay);
	void		CheckForReinit();
	
	void		UseHardwareCursor(bool bUseHardwareCursor, bool bForce = false);
    void		UseCursor(bool bUseCursor, bool bLockCursorToCenter = false);
	void		Update();

	bool		SetCursor( const char* szCursorRecord );
	bool		SetCursor( HRECORD hCursorRecord );
	bool		SetDefaultCursor();

	HRECORD		GetCursor() { return m_hCurrentCursorRecord; }
	HRECORD		GetCursorRecordByName( const char* szCursorRecord );

private:

	bool		m_bInitialized;
    bool      m_bUseCursor;
    bool      m_bUseHardwareCursor;

	LTVector2n		m_CursorCenter;
	LTVector2n		m_CursorDims;

	HRECORD		m_hCurrentCursorRecord;
	HRECORD		m_hDefaultCursorRecord;

	// old-style cursor
	HLTCURSOR	m_hCursor;
	TextureReference	m_hCursorTex;

};

#endif // __CURSOR_MGR_H__