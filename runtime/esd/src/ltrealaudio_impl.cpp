/****************************************************************************
;
;	MODULE:		LTRealAudio_Impl (.CPP)
;
;	PURPOSE:	Implement RealAudio capability for LithTech engine
;
;	HISTORY:	3-20-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrealaudio_impl.h"
#include "ltrealcore.h"
#include "soundmgr.h"

//-----------------------------------------------------------------------------
// CLTRealAudioMgr member functions
//-----------------------------------------------------------------------------
CLTRealAudioMgr::CLTRealAudioMgr()
{
	m_bInitialized = LTFALSE;
}

//-----------------------------------------------------------------------------
CLTRealAudioMgr::~CLTRealAudioMgr()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealAudioMgr::Init()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioMgr::Init()");

	if (m_bInitialized)
		return LT_ALREADYINITIALIZED;

	CLTRealCore().Init();

	m_bInitialized = LTTRUE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealAudioMgr::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioMgr::Term()");

	if (!m_bInitialized)
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "CLTRealAudioMgr not initialized.");

	// Remove and destroy the players...
	CLTRealAudioPlayer* pPlayer = m_AudioPlayerList.GetFirst();
	while(pPlayer)
	{
		m_AudioPlayerList.Delete(pPlayer);
		delete pPlayer;
		pPlayer = NULL;

		pPlayer = m_AudioPlayerList.GetFirst();
	}

	m_bInitialized = LTFALSE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealAudioMgr::IsRealPlayerInstalled(bool* pbIsInstalled)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioMgr::IsRealPlayerInstalled()");

	CLTRealCore().IsRealPlayerInstalled(pbIsInstalled);

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealAudioMgr::IsRealPlayerPlugInInstalled(bool* pbIsInstalled)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioMgr::IsRealPlayerPlugInInstalled()");

	CLTRealCore().IsRealPlayerPlugInInstalled(pbIsInstalled);

	return LT_OK;
}

//-----------------------------------------------------------------------------
ILTRealAudioPlayer*	CLTRealAudioMgr::CreatePlayer()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioMgr::CreatePlayer()");

	if (!m_bInitialized)
	{
		Init();
		if (!m_bInitialized)
			return LTNULL;
	}

	CLTRealAudioPlayer* pPlayer;
	LT_MEM_TRACK_ALLOC(pPlayer = new CLTRealAudioPlayer,LT_MEM_TYPE_SOUND);

	if (LT_OK == pPlayer->Init())
	{
		// Add the player to the list
		m_AudioPlayerList.InsertLast(pPlayer);
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
LTRESULT CLTRealAudioMgr::DestroyPlayer(ILTRealAudioPlayer* pPlayer)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioMgr::DestroyPlayer()");

	ASSERT(pPlayer);

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

	CLTRealAudioPlayer* pCastPlayer = (CLTRealAudioPlayer*)pPlayer;
	m_AudioPlayerList.Delete(pCastPlayer);
	delete pCastPlayer;
	pPlayer = NULL;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealAudioMgr::Update()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioMgr::Update()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

	CLTRealAudioPlayer* pPlayer = m_AudioPlayerList.GetFirst();
	
	// Traverse the list and update...
	while(pPlayer)
	{
		pPlayer->Update();
		pPlayer = pPlayer->m_pNext;
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealAudioMgr::AppFocus(bool bHasFocus)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioMgr::AppFocus()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

	CLTRealAudioPlayer* pPlayer = m_AudioPlayerList.GetFirst();
	
	// Traverse the list and set focus...
	while(pPlayer)
	{
		pPlayer->AppFocus(bHasFocus);
		pPlayer = pPlayer->m_pNext;
	}

	return LT_OK;
}


//-----------------------------------------------------------------------------
// CLTRealAudioPlayer member functions
//-----------------------------------------------------------------------------
CLTRealAudioPlayer::CLTRealAudioPlayer()
{
}

//-----------------------------------------------------------------------------
CLTRealAudioPlayer::~CLTRealAudioPlayer()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealAudioPlayer::Init()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioPlayer::Init()");

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
		m_pClientContext->Init(m_pRMAPlayer);
		m_pClientContext->AddRef();
		m_pRMAPlayer->SetClientContext(m_pClientContext);

		// Used for "showplaytime" functionality
		m_pClientContext->m_pClientAdviceSink->m_pPlayer = this;

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

//			m_pRMAAudioPlayer->AddPostMixHook(m_pAudioHook, true, true);	// [mds] This is where we would add a post mix hook, if we needed it

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
LTRESULT CLTRealAudioPlayer::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioPlayer::Term()");

	// Clean up client context
	if (m_pClientContext)
	{
		m_pClientContext->Release();
		m_pClientContext = LTNULL;
	}

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

	if (m_pAudioHook)
	{
		m_pAudioHook->Release();
		m_pAudioHook = LTNULL;
	}

	LTRESULT LTResult = CLTRealPlayer::Term();
	if (LT_OK != LTResult)
		return LTResult;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTRealAudioPlayer::Update()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealAudioPlayer::Update()");

	LTBOOL bIsDone = LTTRUE;
	IsDone(&bIsDone);

	if (bIsDone)
	{
		if (m_bLooping)
			Play(LTTRUE);
		else
			if (m_lhStream)
			{
				SoundSys()->ResetStream(m_lhStream);
				SoundSys()->ClearStreamBuffer(m_lhStream);
			}
	}

	return LTTRUE;
}

#endif // LITHTECH_ESD
