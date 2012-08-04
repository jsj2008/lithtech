/****************************************************************************
;
;	MODULE:		LTRealVideo_Impl (.CPP)
;
;	PURPOSE:	Implement RealVideo capability for LithTech engine
;
;	HISTORY:	5-12-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrealvideo_impl.h"
#include "ltrealcore.h"
#include "soundmgr.h"
#include "de_mainworld.h"
#include "clientshell.h"
#include "clientmgr.h"

extern CClientShell *g_pClientShell;

//IWorldClientBSP holder
#include "world_client_bsp.h"
static IWorldClientBSP *world_bsp_client;
define_holder(IWorldClientBSP, world_bsp_client);

//-----------------------------------------------------------------------------
// CLTRealVideoMgr member functions
//-----------------------------------------------------------------------------
CLTRealVideoMgr::CLTRealVideoMgr()
{
	m_bInitialized = LTFALSE;
}

//-----------------------------------------------------------------------------
CLTRealVideoMgr::~CLTRealVideoMgr()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoMgr::Init()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoMgr::Init()");

	if (m_bInitialized)
		return LT_ALREADYINITIALIZED;

	CLTRealCore().Init();

	m_bInitialized = LTTRUE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoMgr::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoMgr::Term()");

	if (!m_bInitialized)
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "CLTRealAudioMgr not initialized.");

	// Remove and destroy the players...
	CLTRealVideoPlayer* pPlayer = m_VideoPlayerList.GetFirst();
	while(pPlayer)
	{
		m_VideoPlayerList.Delete(pPlayer);
		delete pPlayer;
		pPlayer = NULL;

		pPlayer = m_VideoPlayerList.GetFirst();
	}

	m_bInitialized = LTFALSE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoMgr::IsRealPlayerInstalled(bool* pbIsInstalled)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoMgr::IsRealPlayerInstalled()");

	CLTRealCore().IsRealPlayerInstalled(pbIsInstalled);

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoMgr::IsRealPlayerPlugInInstalled(bool* pbIsInstalled)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoMgr::IsRealPlayerPlugInInstalled()");

	CLTRealCore().IsRealPlayerPlugInInstalled(pbIsInstalled);

	return LT_OK;
}

//-----------------------------------------------------------------------------
ILTRealVideoPlayer*	CLTRealVideoMgr::CreatePlayer()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoMgr::CreatePlayer()");

	if (!m_bInitialized)
	{
		Init();
		if (!m_bInitialized)
			return LTNULL;
	}

	CLTRealVideoPlayer* pPlayer;
	LT_MEM_TRACK_ALLOC(pPlayer = new CLTRealVideoPlayer,LT_MEM_TYPE_MISC);

	if (LT_OK == pPlayer->Init())
	{
		// Add the player to the list
		m_VideoPlayerList.InsertLast(pPlayer);
	}
	else
	{
		// We couldn't init the player, so destroy it
		delete pPlayer;
		pPlayer = LTNULL;
	}

	return pPlayer;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoMgr::DestroyPlayer(ILTRealVideoPlayer* pPlayer)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoMgr::DestroyPlayer()");

	ASSERT(pPlayer);

	if (!m_bInitialized)
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "CLTRealAudioMgr not initialized.");

	CLTRealVideoPlayer* pCastPlayer = (CLTRealVideoPlayer*)pPlayer;
	m_VideoPlayerList.Delete(pCastPlayer);
	delete pCastPlayer;
	pPlayer = NULL;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoMgr::Update(LTRV_UPDATE_MODE updateMode)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoMgr::Update()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

	// Check the update mode
	if (!(LTRV_UPDATE_2D == updateMode || LTRV_UPDATE_3D == updateMode))
		return LT_ERROR;

	CLTRealVideoPlayer* pPlayer = m_VideoPlayerList.GetFirst();
	
	// Traverse the list and update...
	while(pPlayer)
	{
		pPlayer->Update(updateMode);
		pPlayer = pPlayer->m_pNext;
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoMgr::AppFocus(bool bHasFocus)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoMgr::AppFocus()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

	CLTRealVideoPlayer* pPlayer = m_VideoPlayerList.GetFirst();
	
	// Traverse the list and set focus...
	while(pPlayer)
	{
		pPlayer->AppFocus(bHasFocus);
		pPlayer = pPlayer->m_pNext;
	}

	return LT_OK;
}


//-----------------------------------------------------------------------------
// CLTRealVideoPlayer member functions
//-----------------------------------------------------------------------------
CLTRealVideoPlayer::CLTRealVideoPlayer()
{
	m_b2DRenderingEnabled = LTTRUE;
	m_b3DRenderingEnabled = LTTRUE;
}

//-----------------------------------------------------------------------------
CLTRealVideoPlayer::~CLTRealVideoPlayer()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::Init()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::Init()");
	
	LTRESULT LTResult = CLTRealPlayer::Init();
	if (LT_OK != LTResult)
		return LTResult;

	LTBOOL bRealPlayerInstalled = LTFALSE;
	if (LT_OK != CLTRealCore().IsRealPlayerInstalled(&bRealPlayerInstalled))
		return LT_ERROR;
	if (!bRealPlayerInstalled)
		return LT_ERROR;
	LTBOOL bRealPlayerPlugInInstalled = LTFALSE;
	if (LT_OK != CLTRealCore().IsRealPlayerPlugInInstalled(&bRealPlayerPlugInInstalled))
		return LT_ERROR;
	if (!bRealPlayerPlugInInstalled)
		return LT_ERROR;

	if (LT_OK == CLTRealCore().CreatePlayer(&m_pRMAPlayer))
	{
		ASSERT(m_pRMAPlayer);

		// Create the client context
		LT_MEM_TRACK_ALLOC(m_pClientContext = new CLTClientContext,LT_MEM_TYPE_MISC);
		ASSERT(m_pClientContext);
		m_pClientContext->Init(m_pRMAPlayer, this);
		m_pClientContext->AddRef();
		m_pRMAPlayer->SetClientContext(m_pClientContext);

		// Get an interface to the Audio Player
		if (PNR_OK != m_pRMAPlayer->QueryInterface(IID_IRMAAudioPlayer, (void**) &m_pRMAAudioPlayer))
		{
			m_pRMAAudioPlayer = LTNULL;
			return LT_ERROR;
		}
		else
		{
			// Make the hook (that the response needs)
			ASSERT(NULL == m_pAudioHook);
			LT_MEM_TRACK_ALLOC(m_pAudioHook = new CLTAudioHook,LT_MEM_TYPE_SOUND);
			if (LT_OK != m_pAudioHook->Init(this))
			{
				delete m_pAudioHook;
				m_pAudioHook = LTNULL;
				return LT_ERROR;
			}
			m_pAudioHook->AddRef();

			// Make the dry notification (that the response needs)
			ASSERT(NULL == m_pDryNotification);
			LT_MEM_TRACK_ALLOC(m_pDryNotification = new CLTDryNotification,LT_MEM_TYPE_SOUND);
			if (LT_OK != m_pDryNotification->Init())
			{
				delete m_pDryNotification;
				m_pDryNotification = LTNULL;
				return LT_ERROR;
			}
			m_pDryNotification->AddRef();

			// Make the response (that the player needs)
			ASSERT(NULL == m_pAudioInfoResponse);
			LT_MEM_TRACK_ALLOC(m_pAudioInfoResponse = new CLTAudioInfoResponse,LT_MEM_TYPE_SOUND);
			if (LT_OK != m_pAudioInfoResponse->Init(m_pAudioHook, m_pDryNotification))
			{
				delete m_pAudioInfoResponse;
				m_pAudioInfoResponse = LTNULL;
				return LT_ERROR;
			}
			m_pAudioInfoResponse->AddRef();

			// Pass off the response to the player
			m_pRMAAudioPlayer->SetStreamInfoResponse(m_pAudioInfoResponse);
		}
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::Term()");

	// Clean up client context
	if (m_pClientContext)
	{
		m_pClientContext->Release();
		m_pClientContext = LTNULL;
	}

	// Clean up the overlays
	RemoveOverlays();

	// Clean up the player
	if (m_pRMAPlayer)
	{
		m_pRMAPlayer->Stop();
		if (m_lhStream)
		{
			SoundSys()->ResetStream(m_lhStream);
			SoundSys()->ClearStreamBuffer(m_lhStream);
			SoundSys()->CloseStream(m_lhStream);
			m_lhStream = LTNULL;
		}
		CLTRealCore().DestroyPlayer(&m_pRMAPlayer);
		m_pRMAPlayer = LTNULL;
	}

	LTRESULT LTResult = CLTRealPlayer::Term();
	if (LT_OK != LTResult)
		return LTResult;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::Stop()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::Stop()");

	if (LT_OK == CLTRealPlayer::Stop())
	{
		// Tell the surfaces to stop looking for video data...
		CLTRealVideoSurfaceNode* pSurface = m_RealVideoSurfaceNodeList.GetFirst();
		while(pSurface)
		{
			ASSERT(pSurface->m_pVideoSurface);
			((CLTRealVideoSurface*)(pSurface->m_pVideoSurface))->EndOptimizedBlt();
			pSurface = pSurface->m_pNext;
		}
	}
	
	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::AddOverlay(LTRect* pRect, DWORD dwFlags)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::AddOverlay()");

	LTBOOL bRealPlayerInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerInstalled(&bRealPlayerInstalled);
	if (!bRealPlayerInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer not installed.");
		return LT_ERROR;
	}
	LTBOOL bRealPlayerPlugInInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerPlugInInstalled(&bRealPlayerPlugInInstalled);
	if (!bRealPlayerPlugInInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer plugin not installed.");
		return LT_ERROR;
	}

	if (!m_pRMAPlayer)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "RealPlayer player object does not exist.");
		return LT_ERROR;
	}

	// Valid rectangle pointer?
	if (!pRect)
		return LT_ERROR;

	// Create the overlay
	CLTRealVideoOverlay* pOverlay;
	LT_MEM_TRACK_ALLOC(pOverlay = new CLTRealVideoOverlay,LT_MEM_TYPE_MISC);
	if (LT_OK != pOverlay->Init(pRect->left, pRect->top, pRect->right, pRect->bottom, dwFlags))
	{
		delete pOverlay;
		return LT_ERROR;
	}

	// Add it to our list
	m_RealVideoOverlayList.InsertLast(pOverlay);
	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::AddOverlay(LTIntPt* pPoint, DWORD dwFlags)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::AddOverlay()");

	LTBOOL bRealPlayerInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerInstalled(&bRealPlayerInstalled);
	if (!bRealPlayerInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer not installed.");
		return LT_ERROR;
	}
	LTBOOL bRealPlayerPlugInInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerPlugInInstalled(&bRealPlayerPlugInInstalled);
	if (!bRealPlayerPlugInInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer plugin not installed.");
		return LT_ERROR;
	}

	if (!m_pRMAPlayer)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "RealPlayer player object does not exist.");
		return LT_ERROR;
	}

	// Valid point pointer?
	if (!pPoint)
		return LT_ERROR;

	LTRect rect;
	rect.Init(pPoint->x, pPoint->y, 0, 0);

	return AddOverlay(&rect, dwFlags);
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::AddOverlay(DWORD dwFlags)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::AddOverlay()");

	LTBOOL bRealPlayerInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerInstalled(&bRealPlayerInstalled);
	if (!bRealPlayerInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer not installed.");
		return LT_ERROR;
	}
	LTBOOL bRealPlayerPlugInInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerPlugInInstalled(&bRealPlayerPlugInInstalled);
	if (!bRealPlayerPlugInInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer plugin not installed.");
		return LT_ERROR;
	}

	if (!m_pRMAPlayer)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "RealPlayer player object does not exist.");
		return LT_ERROR;
	}

	LTRect rect;
	rect.Init(0, 0, 0, 0);

	return AddOverlay(&rect, dwFlags);
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::RemoveOverlays()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::RemoveOverlays()");

	LTBOOL bRealPlayerInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerInstalled(&bRealPlayerInstalled);
	if (!bRealPlayerInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer not installed.");
		return LT_ERROR;
	}
	LTBOOL bRealPlayerPlugInInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerPlugInInstalled(&bRealPlayerPlugInInstalled);
	if (!bRealPlayerPlugInInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer plugin not installed.");
		return LT_ERROR;
	}

	if (!m_pRMAPlayer)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "RealPlayer player object does not exist.");
		return LT_ERROR;
	}

	CLTRealVideoOverlay* pOverlay = m_RealVideoOverlayList.GetLast();
	while (pOverlay)
	{
		m_RealVideoOverlayList.Delete(pOverlay);
		pOverlay = m_RealVideoOverlayList.GetLast();
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::AddPoly(HPOLY hPoly)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::AddPoly()");

	LTBOOL bRealPlayerInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerInstalled(&bRealPlayerInstalled);
	if (!bRealPlayerInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer not installed.");
		return LT_ERROR;
	}
	LTBOOL bRealPlayerPlugInInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerPlugInInstalled(&bRealPlayerPlugInInstalled);
	if (!bRealPlayerPlugInInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer plugin not installed.");
		return LT_ERROR;
	}

	if (!m_pRMAPlayer)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "RealPlayer player object does not exist.");
		return LT_ERROR;
	}

	// Duplicate poly checking goes here

	// This is a new handle, so create a new surface node for it...
	WorldPoly* pWorldPoly = world_bsp_client->GetPolyFromHPoly(hPoly);
	if (pWorldPoly)
	{
		CLTSurfaceNode* pNewSurfaceNode;
		LT_MEM_TRACK_ALLOC(pNewSurfaceNode = new CLTSurfaceNode,LT_MEM_TYPE_MISC);
		pNewSurfaceNode->m_pSurface = pWorldPoly->GetSurface();
		pNewSurfaceNode->m_pOriginalTexture = LTNULL;
		pNewSurfaceNode->m_hPoly = hPoly;
		m_SurfaceNodeList.Insert(pNewSurfaceNode);
		return LT_OK;
	}

	return LT_ERROR;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::RemovePolys()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::RemovePolys()");

	LTBOOL bRealPlayerInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerInstalled(&bRealPlayerInstalled);
	if (!bRealPlayerInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer not installed.");
		return LT_ERROR;
	}
	LTBOOL bRealPlayerPlugInInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerPlugInInstalled(&bRealPlayerPlugInInstalled);
	if (!bRealPlayerPlugInInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer plugin not installed.");
		return LT_ERROR;
	}

	if (!m_pRMAPlayer)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "RealPlayer player object does not exist.");
		return LT_ERROR;
	}

	CLTSurfaceNode* pSurfaceNode = m_SurfaceNodeList.GetFirst();
	while (pSurfaceNode)
	{
		m_SurfaceNodeList.Delete(pSurfaceNode);
		pSurfaceNode = m_SurfaceNodeList.GetFirst();
	}

	return LT_ERROR;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::Render()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::Render()");

	LTBOOL bRealPlayerInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerInstalled(&bRealPlayerInstalled);
	if (!bRealPlayerInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer not installed.");
		return LT_ERROR;
	}
	LTBOOL bRealPlayerPlugInInstalled = LTFALSE;
	CLTRealCore().IsRealPlayerPlugInInstalled(&bRealPlayerPlugInInstalled);
	if (!bRealPlayerPlugInInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer plugin not installed.");
		return LT_ERROR;
	}

	if (!m_pRMAPlayer)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "RealPlayer player object does not exist.");
		return LT_ERROR;
	}

	// If 2D updates are disabled, bail...
	if (!m_b2DRenderingEnabled)
		return LT_OK;

	CLTRealVideoOverlay* pOverlay = m_RealVideoOverlayList.GetFirst();
	while (pOverlay)
	{
		pOverlay->Render();
		pOverlay = pOverlay->Next();
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealVideoPlayer::Update(LTRV_UPDATE_MODE updateMode)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealVideoPlayer::Update()");

	// Check for valid modes
	if (!(LTRV_UPDATE_2D == updateMode || LTRV_UPDATE_3D == updateMode))
		return LT_ERROR;

	// If 2D updates are disabled, bail...
	if (LTRV_UPDATE_2D == updateMode && !m_b2DRenderingEnabled)
		return LT_OK;

	// If 2D updates are disabled, bail...
	if (LTRV_UPDATE_3D == updateMode && !m_b3DRenderingEnabled)
		return LT_OK;

	// Get the first node
	CLTRealVideoSurfaceNode* pSurface = m_RealVideoSurfaceNodeList.GetFirst();
	
	// If there are no surfaces in the list, bail...
	if (!pSurface)
		return LT_OK;

	// Traverse the list and update...
	while(pSurface)
	{
		ASSERT(pSurface->m_pVideoSurface);
		((CLTRealVideoSurface*)(pSurface->m_pVideoSurface))->Update(updateMode);
		pSurface = pSurface->m_pNext;
	}

	if (m_bLooping)
	{
		LTBOOL bIsDone = LTTRUE;
		IsDone(&bIsDone);
		if (LTTRUE == bIsDone)
		{
			Play(LTTRUE);
		}
	}

	return LT_OK;
}
#endif // LITHTECH_ESD