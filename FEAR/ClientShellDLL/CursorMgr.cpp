// ----------------------------------------------------------------------- //
//
// MODULE  : CursorMgr.cpp
//
// PURPOSE : Manage all mouse cursor related functionality
//
// CREATED : 12/3/01
//
// (c) 2001-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "VarTrack.h"
#include "InterfaceMgr.h"
#include "CursorMgr.h"
#include "CursorsDB.h"

VarTrack	g_vtCursorHack;

CCursorMgr * g_pCursorMgr = NULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr constructor and destructor
//
//	PURPOSE:	Set initial values on ctor, force a Term() on dtor
//
// ----------------------------------------------------------------------- //

CCursorMgr::CCursorMgr() : 
	m_hCurrentCursorRecord(NULL),
	m_hDefaultCursorRecord(NULL)
{
	g_pCursorMgr = this;

    m_bUseCursor			= false;
    m_bUseHardwareCursor	= false;
	m_bInitialized			= false;

}

CCursorMgr::~CCursorMgr()
{
	Term();

	g_pCursorMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::Init
//
//	PURPOSE:	Init the cursor
//
// ----------------------------------------------------------------------- //

bool CCursorMgr::Init()
{
	if (m_bInitialized)
		return true;

	// The following line was pulled from InterfaceMgr::Init()
    g_vtCursorHack.Init(g_pLTClient, "CursorHack", NULL, 0.0f);

	// set the default cursor
	SetDefaultCursor();
		
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::Term
//
//	PURPOSE:	Free cursor resources
//
// ----------------------------------------------------------------------- //

void CCursorMgr::Term()
{
	if (!m_bInitialized)
		return;
	m_bInitialized = false;
	m_hCurrentCursorRecord = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::ScheduleReinit(float fHack)
//
//	PURPOSE:	Set up a delayed initialization
//
// ----------------------------------------------------------------------- //

void CCursorMgr::ScheduleReinit(float fDelay)
{
	g_vtCursorHack.SetFloat(fDelay);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::CheckForReinit
//
//	PURPOSE:	Update any hack variables (reducing frame delay counter)
//
// ----------------------------------------------------------------------- //

void CCursorMgr::CheckForReinit()
{
	// because of driver bugs, we need to wait a frame after reinitializing the renderer and
	// reinitialize the cursor
	int nCursorHackFrameDelay = (int)g_vtCursorHack.GetFloat();
	if (nCursorHackFrameDelay)
	{
		nCursorHackFrameDelay--;
		g_vtCursorHack.SetFloat((float)nCursorHackFrameDelay);
		if (nCursorHackFrameDelay == 1)
			Init();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::UseCursor
//
//	PURPOSE:	Handle activation and deactivation of visible cursor
//
// ----------------------------------------------------------------------- //

void CCursorMgr::UseCursor(bool bUseCursor, bool bLockCursorToCenter)
{
	m_bUseCursor = bUseCursor;

	// New hardware code:
	// if the cursor is visible and being used
	if (m_bUseCursor && m_bUseHardwareCursor)
	{
		g_pLTClient->Cursor()->SetCursorMode(CM_Hardware);
		// copied the following 4 lines from Init()
		if (g_pLTClient->Cursor()->SetCursor(m_hCursor) != LT_OK)
		{
			DebugCPrint(1,"can't set cursor.");
		}
	}
	else
	{
        g_pLTClient->Cursor()->SetCursorMode(CM_None);
		
	}

	// Lock or don't lock the cursor to the center of the screen
	if(bLockCursorToCenter)
	{
		g_pLTClient->SetConsoleVariableFloat("CursorCenter", 1.0f);
	}
	else
	{
		g_pLTClient->SetConsoleVariableFloat("CursorCenter", 0.0f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::UseHardwareCursor
//
//	PURPOSE:	(De)activate the Windows cursor drawing routines
//
// ----------------------------------------------------------------------- //

void CCursorMgr::UseHardwareCursor(bool bUseHardwareCursor,bool bForce)
{
	m_bUseHardwareCursor = bUseHardwareCursor;

	if (m_bUseHardwareCursor && m_bUseCursor)
	{
		g_pLTClient->Cursor()->SetCursorMode(CM_Hardware,bForce);
	}
	else
	{
		g_pLTClient->Cursor()->SetCursorMode(CM_None,bForce);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::Update
//
//	PURPOSE:	Display a cursor bitmap, if required
//
// ----------------------------------------------------------------------- //

void CCursorMgr::Update()
{
	bool bHWC = (GetConsoleInt("HardwareCursor",1) > 0 && GetConsoleInt("DisableHardwareCursor",0) == 0);

	if (bHWC != m_bUseHardwareCursor)
		UseHardwareCursor(bHWC);

	if (!m_bUseCursor)
		return;
}

// sets the current cursor
bool CCursorMgr::SetCursor( HRECORD hCursorRecord )
{
	if( !hCursorRecord )
	{
		if( g_pLayoutDB )
		{
			if( !m_hDefaultCursorRecord )
				m_hDefaultCursorRecord = g_pLayoutDB->GetDefaultCursor();
			hCursorRecord = m_hDefaultCursorRecord;
		}
		if( !hCursorRecord )
			return false;
	}

	// check to see if this cursor is already set
	if( hCursorRecord == m_hCurrentCursorRecord )
		return true;
	
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)

	const char* pszHardwareCursorFileName = DATABASE_CATEGORY( Cursors ).GETRECORDATTRIB( hCursorRecord, HardwareCursor );

	if (g_pLTClient->Cursor()->LoadCursorFromFile(pszHardwareCursorFileName, m_hCursor) != LT_OK)
	{
		DebugCPrint(1,"can't load cursor resource.");
        return false;
	}

    if (g_pLTClient->Cursor()->SetCursor(m_hCursor) != LT_OK)
	{
		DebugCPrint(1,"can't set cursor.");
        return false;
	}

#endif // !PLATFORM_XENON

	UseHardwareCursor( GetConsoleInt("HardwareCursor",1) > 0 && GetConsoleInt("DisableHardwareCursor",0) == 0);

	if (!m_hCursorTex)
	{
		const char* pszSoftwareCursorFileName = DATABASE_CATEGORY( Cursors ).GETRECORDATTRIB( hCursorRecord, SoftwareCursor );

		m_hCursorTex.Load( pszSoftwareCursorFileName );

		if(m_hCursorTex)
		{
			LTVector2 vCursorSize = DATABASE_CATEGORY( Cursors ).GETRECORDATTRIB( hCursorRecord, SoftwareCursorSize );
			m_CursorDims.x = (int32)vCursorSize.x;
			m_CursorDims.y = (int32)vCursorSize.y;
		}
		else
		{
			LTERROR_PARAM1("Failed to load software cursor: %s", pszSoftwareCursorFileName);
			m_CursorDims.x = 0;
			m_CursorDims.y = 0;
		}
	}

	m_bInitialized = true;

	m_CursorCenter.x = m_CursorDims.x/2;
	m_CursorCenter.y = m_CursorDims.y/2;

	m_hCurrentCursorRecord = hCursorRecord;

	return true;
}

// sets the current cursor
bool CCursorMgr::SetCursor( const char* szCursorRecord )
{
	if( !szCursorRecord )
		return SetDefaultCursor();

	return SetCursor( DATABASE_CATEGORY( Cursors ).GetRecordByName( szCursorRecord ) );
}

// sets the default cursor
bool CCursorMgr::SetDefaultCursor()
{
	if( !g_pLayoutDB )
		return false;

	if( !m_hDefaultCursorRecord )
		m_hDefaultCursorRecord = g_pLayoutDB->GetDefaultCursor();
    
	return SetCursor( m_hDefaultCursorRecord );
}

// returns the record handle given a name
HRECORD CCursorMgr::GetCursorRecordByName( const char* szCursorRecord )
{
	return DATABASE_CATEGORY( Cursors ).GetRecordByName( szCursorRecord );
}