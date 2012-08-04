/****************************************************************************
;
;	MODULE:		LTReal_Impl (.H)
;
;	PURPOSE:	Implement Real capability for LithTech engine
;
;	HISTORY:	11-14-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTReal_Impl_H
#define LTReal_Impl_H

#define LITHTECH_ESD_INC 1
#include "lith.h"
#include "bdefs.h"
#undef LITHTECH_ESD_INC
#include "iltesd.h"
#include "pnwintyp.h"
#include "pncom.h"
#include "rmapckts.h"
#include "rmacomm.h"
#include "rmamon.h"
#include "rmafiles.h"
#include "rmaengin.h"
#include "rmacore.h"
#include "rmaclsnk.h"
#include "rmaerror.h"
#include "rmaauth.h"
#include "rmaausvc.h"
#include "rmawin.h"

#include "soundmgr.h"
#include "ltrealcore.h"
#include "ltrconout.h"
#include "ltraudiohook.h"
#include "ltrdrynotification.h"
#include "ltraudioinforesponse.h"

#include "timemgr.h"

#include <process.h>	// _beginthread(), _endthread()

struct CLTRealPlayerSource
{
	IRMAPlayer*	m_pRMAPlayer;
	char		m_pSource[256];
};

#define MIN_PLAYER_VOLUME	0
#define MAX_PLAYER_VOLUME	100

#define PAN_PLAYER_LEFT		0
#define PAN_PLAYER_CENTER	50
#define PAN_PLAYER_RIGHT	100

#define ESD_MIN(a, b)	(a <= b) ? a : b;
#define ESD_MAX(a, b)	(a >= b) ? a : b;

//-----------------------------------------------------------------------------
// Main class for LTRealPlayer
//-----------------------------------------------------------------------------
class CLTRealPlayer
{
	friend class CLTClientAdviceSink;

public:
	// Default constructor
	CLTRealPlayer();

	// Default destructor (calls Term if it has not been called)
	~CLTRealPlayer();

	// Initialize the player
	virtual LTRESULT	Init() = 0;

	// Terminate the player
	virtual LTRESULT	Term();

	virtual LTRESULT	IsRealPlayerInstalled(bool* bIsInstalled);
	virtual LTRESULT	IsRealPlayerPlugInInstalled(bool* bIsInstalled);
	virtual LTRESULT	Open(const char* pSource);
	virtual LTRESULT	Play(bool bLoop = false);
	virtual LTRESULT	Stop();
	virtual LTRESULT	IsDone(bool* pbIsDone);
	virtual LTRESULT	IsLooping(bool* pbIsLooping);
	virtual LTRESULT	Pause();
	virtual LTRESULT	Resume();
	virtual LTRESULT	SetVolume(uint32 ulVolume);
	virtual LTRESULT	GetVolume(uint32* pulVolume);
	virtual LTRESULT	SetPan(uint32 ulPan);
	virtual LTRESULT	GetPan(uint32* pulPan);
	virtual LTRESULT	SetAudioPacketPreBuffer(uint32 ulPacketCount);
	virtual LTRESULT	GetAudioPacketPreBuffer(uint32* pulPacketCount);
	virtual LTRESULT	AppFocus(bool bHasFocus);
	
	virtual LHSTREAM	GetStream()						{ return m_lhStream; }
	virtual void		SetStream(LHSTREAM lhStream)	{ m_lhStream = lhStream; }
	virtual char*		GetSource()						{ return m_pSource; }

protected:
	virtual LTRESULT	Clear();
	virtual LTRESULT	SanityCheck();

protected:
	LHSTREAM				m_lhStream;
	LTBOOL					m_bInitialized;
	IRMAPlayer*				m_pRMAPlayer;
	CLTClientContext*		m_pClientContext;
	IRMAAudioPlayer*		m_pRMAAudioPlayer;
	CLTAudioHook*			m_pAudioHook;
	CLTDryNotification*		m_pDryNotification;
	CLTAudioInfoResponse*	m_pAudioInfoResponse;
	LTBOOL					m_bPaused;
	char					m_pSource[256];
	LTBOOL					m_bLooping;
	uint32					m_ulVolume;
	uint32					m_ulPan;
	uint32					m_ulAudioPacketPreBufferCount;

	LTBOOL					m_bWaitingToPlay;
	float					m_fStopTime;

	// Members for debugging
	float					m_fRequestedPlayTime;
	float					m_fActualPlayTime;

	HANDLE	m_hThread;
	static void ThreadOpen(void* pVoid);
	static void ThreadPlay(void* pVoid);
	CLTRealPlayerSource	mRealPlayerSource;
};

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::Clear()
{
	m_lhStream = LTNULL;
	m_bInitialized = LTFALSE;
	m_pRMAPlayer = LTNULL;
	m_pClientContext = LTNULL;
	m_pRMAAudioPlayer = LTNULL;
	m_pAudioHook = LTNULL;
	m_pDryNotification = LTNULL;
	m_pAudioInfoResponse = LTNULL;
	m_bPaused = LTFALSE;
	m_pSource[0] = '\0';
	m_bLooping = LTFALSE;
	m_ulVolume = MAX_PLAYER_VOLUME;
	m_ulPan = PAN_PLAYER_CENTER;
	m_ulAudioPacketPreBufferCount = 0;

	m_bWaitingToPlay = LTFALSE;
	m_fStopTime = 0.0f;

	m_fRequestedPlayTime = 0.0f;
	m_fActualPlayTime = 0.0f;

	m_hThread = LTNULL;

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline CLTRealPlayer::CLTRealPlayer()
{
	Clear();
}

//-----------------------------------------------------------------------------
inline CLTRealPlayer::~CLTRealPlayer()
{
	Term();
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::Init()
{
	CLTRealCore().Init();
	Clear();
	m_bInitialized = LTTRUE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::Term()
{
	if (m_lhStream)
	{
		SoundSys()->ResetStream(m_lhStream);
		SoundSys()->ClearStreamBuffer(m_lhStream);
		SoundSys()->CloseStream(m_lhStream);
		m_lhStream = LTNULL;
	}

	if (m_pAudioInfoResponse)
	{
		m_pAudioInfoResponse->Release();
		m_pAudioInfoResponse = LTNULL;
	}

	if (m_pDryNotification)
	{
		m_pDryNotification->Release();
		m_pDryNotification = LTNULL;
	}

	if (m_pAudioHook)
	{
		m_pAudioHook->Release();
		m_pAudioHook = LTNULL;
	}

	if (m_pClientContext)
	{
		m_pClientContext->Release();
		m_pClientContext = LTNULL;
	}

	Clear();

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::IsRealPlayerInstalled(bool* pbIsInstalled)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::IsRealPlayerInstalled()");

	CLTRealCore().IsRealPlayerInstalled(pbIsInstalled);

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::IsRealPlayerPlugInInstalled(bool* pbIsInstalled)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::IsRealPlayerPlugInInstalled()");

	CLTRealCore().IsRealPlayerPlugInInstalled(pbIsInstalled);

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::Open(const char* pSource)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::Open()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	if (!pSource)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealAudioMgr::Open invalid parameters.");
		return LT_INVALIDPARAMS;
	}

//LTRConsoleOutput(LTRA_CONOUT_ERROR, "CLTRealPlayer::SPAWNING.");
//	m_hThread = (HANDLE)_beginthread(ThreadOpen, 0, (void*)(&mRealPlayerSource));
//LTRConsoleOutput(LTRA_CONOUT_ERROR, "CLTRealPlayer::RETURNING.");

	if (PNR_OK == m_pRMAPlayer->OpenURL(pSource))
	{
		lstrcpy(m_pSource, pSource);
		m_bWaitingToPlay = LTTRUE;
		return LT_OK;
	}

	LTRConsoleOutput(LTRA_CONOUT_ERROR, "CLTRealPlayer::Open failed.");
	return LT_ERROR;
}

//-----------------------------------------------------------------------------
inline void CLTRealPlayer::ThreadOpen(void* pVoid)
{
	CLTRealPlayerSource* pSource = (CLTRealPlayerSource*)(pVoid);

//	LTRConsoleOutput(LTRA_CONOUT_ERROR, "CLTRealPlayer::IN THREAD.");

//	Sleep(8000);

//	LTRConsoleOutput(LTRA_CONOUT_ERROR, "CLTRealPlayer::DONE SLEEPING.");

	if (PNR_OK == pSource->m_pRMAPlayer->OpenURL(pSource->m_pSource))
	{
		pSource->m_pRMAPlayer->Begin();
	}
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::Play(bool bLoop)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::Play()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	m_fRequestedPlayTime = time_GetTime();

	m_bLooping = bLoop;

	//CLTRealCore().ReplaceAudioDevice();

	PN_RESULT result = m_pRMAPlayer->Begin();
	if (PNR_OK != result)
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "CLTRealPlayer::Play() failed.");
		return LT_ERROR;
	}
	//m_hThread = (HANDLE)_beginthread(ThreadPlay, 0, (void*)(m_pRMAPlayer));

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline void CLTRealPlayer::ThreadPlay(void* pVoid)
{
	((IRMAPlayer*)(pVoid))->Begin();
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::Stop()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::Stop()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	m_pRMAPlayer->Stop();

	m_bLooping = LTFALSE;
	m_pSource[0] = '\0';

	if (m_lhStream)
	{
		SoundSys()->ResetStream(m_lhStream);
		SoundSys()->ClearStreamBuffer(m_lhStream);
	}

	m_bWaitingToPlay = LTFALSE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::IsDone(bool* pbIsDone)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::IsDone()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	// Assume we're not done (unless all conditions are met)
	*pbIsDone = LTFALSE;

	// [mds] - Failsafe... send an update to the audio hook.  This will in turn
	// drive the hook to start flooding the stream if it's hung up by the core.
	if (m_pAudioHook)
		m_pAudioHook->Update();

	// [mds] - Failsafe... if a play is pending and too much time has passed,
	// we quit waiting.  This could result from the source lagging, or failing
	// to open at all.  In any event, the player needs to be "unlocked".
	if (m_bWaitingToPlay)
	{
		if (time_GetTime() > m_fRequestedPlayTime + 5.0f)
			m_bWaitingToPlay = LTFALSE;
	}

	// Now, if we're still waiting...
	if (m_bWaitingToPlay)
		return LT_OK;

	// Is the player done?
	if (m_pRMAPlayer->IsDone())
	{
		if (m_lhStream)
		{
			// Is the stream's data queue empty?
			if (0 == SoundSys()->GetStreamDataQueueSize(m_lhStream))
			{
				if (0.0f == m_fStopTime)
				{
					m_fStopTime = time_GetTime();
					return LT_OK;
				}

				float fCurrentTime = time_GetTime();
				if (fCurrentTime > m_fStopTime + 0.5f)
				{
					SoundSys()->ResetStream(m_lhStream);
					SoundSys()->ClearStreamBuffer(m_lhStream);
					m_fStopTime = 0.0f;
					*pbIsDone = LTTRUE;					
				}
			}
		}
		else
			*pbIsDone = LTTRUE;
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::IsLooping(bool* pbIsLooping)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::IsLooping()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	*pbIsLooping = m_bLooping;

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::Pause()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::Pause()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	LTBOOL bIsDone = LTFALSE;
	if (LT_OK != IsDone(&bIsDone))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "CLTRealPlayer::IsDone() failed.");
		return LT_ERROR;
	}

	if (!m_bPaused && !bIsDone)
	{
		m_pRMAPlayer->Pause();
		m_bPaused = LTTRUE;
	}
	
	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::Resume()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::Resume()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	LTBOOL bIsDone = LTFALSE;
	if (LT_OK != IsDone(&bIsDone))
	{
		LTRConsoleOutput(LTRA_CONOUT_ERROR, "CLTRealPlayer::IsDone() failed.");
		return LT_ERROR;
	}

	if (m_bPaused && !bIsDone)
	{
		m_pRMAPlayer->Begin();
		m_bPaused = LTFALSE;
	}
	
	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::SetVolume(uint32 ulVolume)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::SetVolume()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	// Clamp the input
	ulVolume = ESD_MIN(MIN_PLAYER_VOLUME, ulVolume);
	ulVolume = ESD_MAX(MAX_PLAYER_VOLUME, ulVolume);

	m_ulVolume = ulVolume;
	if (m_lhStream)
	{
		// Set volume... scale since stream volume range is 0..127
		SoundSys()->SetStreamVolume(m_lhStream, m_ulVolume * 127 / 100);
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::GetVolume(uint32* pulVolume)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::GetVolume()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	*pulVolume = m_ulVolume;

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::SetPan(uint32 ulPan)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::SetPan()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	// Clamp the input
	ulPan = ESD_MAX(PAN_PLAYER_LEFT, ulPan);
	ulPan = ESD_MIN(PAN_PLAYER_RIGHT, ulPan);

	m_ulPan = ulPan;
	if (m_lhStream)
	{
		// Set pan... scale since stream pan range is 0..127
		SoundSys()->SetStreamPan(m_lhStream, m_ulPan * 127 / 100);
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::GetPan(uint32* pulPan)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::GetPan()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	*pulPan = m_ulPan;

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::SetAudioPacketPreBuffer(uint32 ulPacketCount)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::SetAudioPacketPreBuffer()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	m_ulAudioPacketPreBufferCount = ulPacketCount;

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::GetAudioPacketPreBuffer(uint32* pulPacketCount)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::GetAudioPacketPreBuffer()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	*pulPacketCount = m_ulAudioPacketPreBufferCount;

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::AppFocus(bool bHasFocus)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTRealPlayer::AppFocus()");

	LTRESULT LTResult = SanityCheck();
	if (LT_OK != LTResult)
		return LTResult;

	if (bHasFocus)
	{
		if (!m_bPaused)
		{
			if (m_lhStream)
				SoundSys()->PauseStream(m_lhStream, LTFALSE);

			LTBOOL bIsDone = LTFALSE;
			IsDone(&bIsDone);
			if (!bIsDone)
				m_pRMAPlayer->Begin();
		}
	}
	else
	{
		if (m_lhStream)
		{
			SoundSys()->PauseStream(m_lhStream, LTTRUE);
			SoundSys()->ClearStreamBuffer(m_lhStream);
		}

		m_pRMAPlayer->Pause();
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
inline LTRESULT CLTRealPlayer::SanityCheck()
{
	if (!m_bInitialized)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealAudioPlayer not initialized.");
		return LT_NOTINITIALIZED;
	}

	LTBOOL bRealPlayerInstalled = LTFALSE;
	IsRealPlayerInstalled(&bRealPlayerInstalled);
	if (!bRealPlayerInstalled)
	{
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "RealPlayer not installed.");
		return LT_ERROR;
	}

	LTBOOL bRealPlayerPlugInInstalled = LTFALSE;
	IsRealPlayerPlugInInstalled(&bRealPlayerPlugInInstalled);
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

	return LT_OK;
}

#endif // LTReal_Impl_H
#endif // LITHTECH_ESD