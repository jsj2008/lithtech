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
#include "ClientResShared.h"

VarTrack	g_vtCursorHack;

CCursorMgr * g_pCursorMgr = LTNULL;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr constructor and destructor
//
//	PURPOSE:	Set initial values on ctor, force a Term() on dtor
//
// ----------------------------------------------------------------------- //

CCursorMgr::CCursorMgr()
{
	g_pCursorMgr = this;

    m_bUseCursor			= LTFALSE;
    m_bUseHardwareCursor	= LTFALSE;
	m_bInitialized			= LTFALSE;

	m_pCursorSprite				= LTNULL;
	m_pCursorGlowSprite			= LTNULL;
	m_pCursorBackgroundSprite	= LTNULL;
}

CCursorMgr::~CCursorMgr()
{
	Term();

	g_pCursorMgr = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::Init
//
//	PURPOSE:	Init the cursor
//
// ----------------------------------------------------------------------- //

LTBOOL CCursorMgr::Init()
{
	if (m_bInitialized)
		return LTTRUE;

	// The following line was pulled from InterfaceMgr::Init()
    g_vtCursorHack.Init(g_pLTClient, "CursorHack", NULL, 0.0f);

	if (g_pLTClient->Cursor()->LoadCursorBitmapResource(MAKEINTRESOURCE(IDC_POINTER), m_hCursor) != LT_OK)
	{
		g_pLTClient->CPrint("can't load cursor resource.");
        return LTFALSE;
	}

    if (g_pLTClient->Cursor()->SetCursor(m_hCursor) != LT_OK)
	{
		g_pLTClient->CPrint("can't set cursor.");
        return LTFALSE;
	}

	UseHardwareCursor( GetConsoleInt("HardwareCursor",0) > 0 && GetConsoleInt("DisableHardwareCursor",0) == 0);

	if (!m_hSurfCursor)
        m_hSurfCursor = g_pLTClient->CreateSurfaceFromBitmap("interface\\cursor0.pcx");
	_ASSERT(m_hSurfCursor);

	m_pCursorSprite = LTNULL;	// stay with the default cursor until this points to something
	m_pCursorGlowSprite = LTNULL;
	m_pCursorBackgroundSprite = LTNULL;

	// Uncomment the line below if you want to see an animated cursor!
	// UseSprite("Interface\\Menu\\spr\\cursortest.spr");

	m_bInitialized = LTTRUE;

	m_CursorCenter.x = 16;
	m_CursorCenter.y = 16;

	return LTTRUE;
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

	if (m_hSurfCursor)
	{
		g_pLTClient->DeleteSurface(m_hSurfCursor);
		m_hSurfCursor = LTNULL;
	}
	// don't need to clean this up, just erase the list.  SpriteMgr will clean up.
	m_SpriteArray.clear();

	m_pCursorSprite = LTNULL;
	m_pCursorGlowSprite = LTNULL;
	m_pCursorBackgroundSprite = LTNULL;

	m_bInitialized = LTFALSE;
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
		g_vtCursorHack.SetFloat((LTFLOAT)nCursorHackFrameDelay);
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

void CCursorMgr::UseCursor(LTBOOL bUseCursor, LTBOOL bLockCursorToCenter)
{
	m_bUseCursor = bUseCursor;

	// New hardware code:
	// if the cursor is visible and being used, ONLY enable the hardware
	// cursor if no sprite has been specified
	if (m_bUseCursor && m_bUseHardwareCursor && !m_pCursorSprite)
	{
		g_pLTClient->Cursor()->SetCursorMode(CM_Hardware);
		// copied the following 4 lines from Init()
		if (g_pLTClient->Cursor()->SetCursor(m_hCursor) != LT_OK)
		{
			g_pLTClient->CPrint("can't set cursor.");
		}
	}
	else
	{
        g_pLTClient->Cursor()->SetCursorMode(CM_None);
		
		// Kill any cursor sprite
		KillSprite();
	}

	// Lock or don't lock the cursor to the center of the screen
	if(bLockCursorToCenter)
	{
		g_pLTClient->RunConsoleString("CursorCenter 1");
	}
	else
	{
		g_pLTClient->RunConsoleString("CursorCenter 0");
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::UseHardwareCursor
//
//	PURPOSE:	(De)activate the Windows cursor drawing routines
//
// ----------------------------------------------------------------------- //

void CCursorMgr::UseHardwareCursor(LTBOOL bUseHardwareCursor,bool bForce)
{
	m_bUseHardwareCursor = bUseHardwareCursor;

	if (m_bUseHardwareCursor && m_bUseCursor && !m_pCursorSprite)
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
//	PURPOSE:	Display a cursor bitmap, if required, or update the sprite coords
//
// ----------------------------------------------------------------------- //

void CCursorMgr::Update()
{
	static const HLTCOLOR kTrans	= SETRGB_T(255,0,255);
	LTBOOL bHWC = (GetConsoleInt("HardwareCursor",0) > 0 && GetConsoleInt("DisableHardwareCursor",0) == 0);

	if (bHWC != m_bUseHardwareCursor)
		UseHardwareCursor(bHWC);

	if (!m_bUseCursor)
		return;

	if (m_bUseHardwareCursor && !m_pCursorSprite)
		return;

	LTIntPt CursorPos = g_pInterfaceMgr->GetCursorPos();

	// If a software cursor is needed but none has been specified, use the default
	if (!m_pCursorSprite)
	{
		// TODO: replace with DrawPrim
		g_pLTClient->Start3D();
		g_pLTClient->StartOptimized2D();

		g_pLTClient->DrawSurfaceToSurfaceTransparent(g_pLTClient->GetScreenSurface(), m_hSurfCursor, LTNULL,
												   CursorPos.x, CursorPos.y, kTrans);

		g_pLTClient->EndOptimized2D();
		g_pLTClient->End3D(END3D_CANDRAWCONSOLE);
		return;
	}

	// Update the sprite coordinates
	m_pCursorSprite->SetPosition(CursorPos);

	// update any additional bitmaps
	if (m_pCursorGlowSprite)
		m_pCursorGlowSprite->SetPosition(CursorPos);

	if (m_pCursorBackgroundSprite)
		m_pCursorBackgroundSprite->SetPosition(CursorPos);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::SetCenter
//
//	PURPOSE:	Specify a new "center" point for any sprites used to
//				display the current cursor position
//
// ----------------------------------------------------------------------- //

void CCursorMgr::SetCenter(int x, int y)
{
	m_CursorCenter.x = x;
	m_CursorCenter.y = y;

	if (m_pCursorSprite)
		m_pCursorSprite->SetCenter(m_CursorCenter);

	if (m_pCursorGlowSprite)
		m_pCursorGlowSprite->SetCenter(m_CursorCenter);

	if (m_pCursorBackgroundSprite)
		m_pCursorBackgroundSprite->SetCenter(m_CursorCenter);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::UseSprite
//
//	PURPOSE:	Attach a sprite to the mouse cursor.  Note: only one sprite
//				may be used at a time to represent the mouse cursor.  You may
//				specify a sprite using either a resource file name or a
//				pointer to an existing sprite.
//
// ----------------------------------------------------------------------- //

void CCursorMgr::UseSprite(char * pFile)
{
	KillSprite();

	// if pFile is null, just turn the sprite off and set the pointer to null
	if (!pFile)
		return;

	// See if this sprite is in the list
	ScreenSpriteArray::iterator iter = m_SpriteArray.begin();
	while (iter != m_SpriteArray.end())
	{
		// If it is, then set it to "active"
		if (!strcmpi(pFile, (*iter)->GetName()))
		{
			m_pCursorSprite = *iter;
			m_pCursorSprite->Show(LTTRUE);
			m_pCursorSprite->SetCenter(m_CursorCenter);
			UseHardwareCursor(LTFALSE);
			return;
		}
		iter++;
	}

	// No sprite exists, so create one using this file name
	CScreenSprite * pSprite = g_pScreenSpriteMgr->CreateScreenSprite(pFile, LTFALSE, SPRITELAYER_CURSOR_FOREGROUND);
	_ASSERT(pSprite != LTNULL);

	if (!pSprite)
		return;

	// Default center coordinates
	pSprite->SetCenter(m_CursorCenter);
	pSprite->Show(LTTRUE);
	m_SpriteArray.push_back(pSprite);
	m_pCursorSprite = pSprite;
	UseHardwareCursor(LTFALSE);
}



void CCursorMgr::UseSprite(CScreenSprite * pSprite)
{
	if (m_pCursorSprite == pSprite)
		return;

	KillSprite();

	if (!pSprite)
	{
		return;
	}


	// See if this sprite is in our list.
	ScreenSpriteArray::iterator iter = m_SpriteArray.begin();
	while (iter != m_SpriteArray.end())
	{
		// If it is, then set it to "active"
		if (!strcmpi((*iter)->GetName(), pSprite->GetName()))
		{
			m_pCursorSprite = *iter;
			m_pCursorSprite->SetCenter(m_CursorCenter);
			m_pCursorSprite->Show(LTTRUE);
			UseHardwareCursor(LTFALSE);
			return;
		}
		iter++;
	}

	// This sprite has not been used before.  Add it to our local array
	// ABM 2/20/02 TWEAK add a new sprite that is a DUPLICATE of the one passed in
	CScreenSprite * pNewSprite = g_pScreenSpriteMgr->CreateScreenSprite(pSprite->GetName(), LTFALSE, SPRITELAYER_CURSOR_FOREGROUND);
	_ASSERT(pNewSprite != LTNULL);
	if (!pNewSprite)
		return;

	// Default center coordinates
	pNewSprite->SetCenter(m_CursorCenter);
	pNewSprite->Show(LTTRUE);
	m_SpriteArray.push_back(pNewSprite);
	m_pCursorSprite = pNewSprite;
	UseHardwareCursor(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::UseGlowSprite
//
//	PURPOSE:	Attach an additive glow sprite to overlay the current sprite
//				must be specified by filename, and must be used AFTER setting
//				the bitmap
//
// ----------------------------------------------------------------------- //

void CCursorMgr::UseGlowSprite(char * pSpriteFile)
{
	if (m_pCursorGlowSprite)
	{
		m_pCursorGlowSprite->Show(LTFALSE);
		m_pCursorGlowSprite = LTNULL;
	}

	// if pFile is null, just turn the sprite off and set the pointer to null
	if (!pSpriteFile)
		return;

	// See if this sprite is in the list
	ScreenSpriteArray::iterator iter = m_SpriteArray.begin();
	while (iter != m_SpriteArray.end())
	{
		// If it is, then set it to "active"
		if (!strcmpi(pSpriteFile, (*iter)->GetName()))
		{
			m_pCursorGlowSprite = *iter;
			m_pCursorGlowSprite->SetCenter(m_CursorCenter);
			m_pCursorGlowSprite->Show(LTTRUE);
			UseHardwareCursor(LTFALSE);
			return;
		}
		iter++;
	}

	// No sprite exists, so create one using this file name
	CScreenSprite * pSprite = g_pScreenSpriteMgr->CreateScreenSprite(pSpriteFile, LTFALSE, SPRITELAYER_CURSOR_ADDITIVE);
	_ASSERT(pSprite != LTNULL);

	if (!pSprite)
		return;

	// Default center coordinates
	pSprite->SetCenter(m_CursorCenter);
	pSprite->SetAdditive(LTTRUE);
	pSprite->Show(LTTRUE);
	m_SpriteArray.push_back(pSprite);
	m_pCursorGlowSprite = pSprite;
	UseHardwareCursor(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::UseBackgroundSprite
//
//	PURPOSE:	Attach a background image sprite under the current sprite
//				must be specified by filename, and must be used AFTER setting
//				the bitmap
//
// ----------------------------------------------------------------------- //

void CCursorMgr::UseBackgroundSprite(char * pSpriteFile)
{
	if (m_pCursorBackgroundSprite)
	{
		m_pCursorBackgroundSprite->Show(LTFALSE);
		m_pCursorBackgroundSprite = LTNULL;
	}

	// if pFile is null, just turn the sprite off and set the pointer to null
	if (!pSpriteFile)
		return;

	// See if this sprite is in the list
	ScreenSpriteArray::iterator iter = m_SpriteArray.begin();
	while (iter != m_SpriteArray.end())
	{
		// If it is, then set it to "active"
		if (!strcmpi(pSpriteFile, (*iter)->GetName()))
		{
			m_pCursorBackgroundSprite = *iter;
			m_pCursorBackgroundSprite->SetCenter(m_CursorCenter);
			m_pCursorBackgroundSprite->Show(LTTRUE);
			UseHardwareCursor(LTFALSE);
			return;
		}
		iter++;
	}

	// No sprite exists, so create one using this file name
	CScreenSprite * pSprite = g_pScreenSpriteMgr->CreateScreenSprite(pSpriteFile, LTFALSE, SPRITELAYER_CURSOR_BACKGROUND);
	_ASSERT(pSprite != LTNULL);

	if (!pSprite)
		return;

	// Default center coordinates
	pSprite->SetCenter(m_CursorCenter);
	pSprite->Show(LTTRUE);
	m_SpriteArray.push_back(pSprite);
	m_pCursorBackgroundSprite = pSprite;
	UseHardwareCursor(LTFALSE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCursorMgr::KillSprite
//
//	PURPOSE:	Disassociate a sprite from the mouse cursor
//
// ----------------------------------------------------------------------- //

void CCursorMgr::KillSprite()
{
	if (m_pCursorSprite)
	{
		m_pCursorSprite->Show(LTFALSE);
		m_pCursorSprite = LTNULL;
	}

	if (m_pCursorGlowSprite)
	{
		m_pCursorGlowSprite->Show(LTFALSE);
		m_pCursorGlowSprite = LTNULL;
	}

	if (m_pCursorBackgroundSprite)
	{
		m_pCursorBackgroundSprite->Show(LTFALSE);
		m_pCursorBackgroundSprite = LTNULL;
	}

	UseHardwareCursor(LTTRUE);
}