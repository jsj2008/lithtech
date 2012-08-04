/****************************************************************************
;
;	MODULE:		LTRAudioHook (.CPP)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	10-25-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltraudiohook.h"
#include "ltrconout.h"
#include "ltreal_impl.h"
#include "soundmgr.h"
#include "console.h"

//-----------------------------------------------------------------------------
// CLTAudioHook member functions
//-----------------------------------------------------------------------------
CLTAudioHook::CLTAudioHook()
{
	m_bInitialized = LTFALSE;
	m_lRefCount = 0;
	m_pRealPlayer = LTNULL;
	memset(&m_Format, 0, sizeof(RMAAudioFormat));
	m_ulBufferWriteCount = 0;
	m_fLastOnBufferTime = 0.0f;
}

//-----------------------------------------------------------------------------
CLTAudioHook::~CLTAudioHook()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioHook::Init(CLTRealPlayer* pRealPlayer)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioHook::Init()");

	if (m_bInitialized)
		return LT_ALREADYINITIALIZED;

	ASSERT(pRealPlayer);
	m_pRealPlayer = pRealPlayer;

	m_bInitialized = LTTRUE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioHook::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioHook::Term()");

	if (!m_bInitialized)
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "CLTAudioHook terminated without initialization");

	m_bInitialized = LTFALSE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioHook::Update()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioHook::Update()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

	HANDLE hStream = m_pRealPlayer->GetStream();
	if (hStream)
	{
		if (SoundSys()->GetStreamDataQueueSize(hStream) > 0)
		{
			if (LS_PLAYING != SoundSys()->GetStreamStatus(hStream))
			{
				float fCurrentTime = time_GetTime();
				if (fCurrentTime > m_fLastOnBufferTime + 1.0f)
				{
					SoundSys()->StartStream(hStream);
					//LTRConsoleOutput(LTRA_CONOUT_ERROR, "KICK IT!");
				}
			}
		}
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTAudioHook::AddRef()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioHook::AddRef()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

    return InterlockedIncrement(&m_lRefCount);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTAudioHook::Release()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioHook::Release()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

    if (InterlockedDecrement(&m_lRefCount) > 0)
    {
        return m_lRefCount;
    }

    delete this;
    return LTNULL;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioHook::QueryInterface(REFIID riid, void** ppvObj)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioHook::QueryInterface()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppvObj = (IUnknown*)(IRMAAudioHook*)this;
        return PNR_OK;
    }
    else if (IsEqualIID(riid, IID_IRMAAudioHook))
    {
        AddRef();
        *ppvObj = (IRMAAudioHook*)this;
        return PNR_OK;
    }
    *ppvObj = NULL;
    return PNR_NOINTERFACE;
}

/*
 *  IRMAAudioHook methods
 */

/************************************************************************
 *  Method:
 *      IRMAAudioHook::OnInit
 *  Purpose:
 *      Audio Services calls OnInit() with the audio data format of the
 *	    audio data that will be provided in the OnBuffer() method.
 */
//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioHook::OnInit(THIS_ RMAAudioFormat* /*IN*/ pFormat)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioHook::OnInit()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

	m_ulBufferWriteCount = 0;

	// Check/store the audio format
	LTBOOL bFormatChanged = LTTRUE;
	if (m_Format.uMaxBlockSize == pFormat->uMaxBlockSize &&
		m_Format.uBitsPerSample == pFormat->uBitsPerSample &&
		m_Format.uChannels == pFormat->uChannels &&
		m_Format.ulSamplesPerSec == pFormat->ulSamplesPerSec)
		bFormatChanged = LTFALSE;
	else
		memcpy(&m_Format, pFormat, sizeof(RMAAudioFormat));

	// If the format hasn't changed and we already have a stream... bail...
	ASSERT(m_pRealPlayer);
	if (!bFormatChanged && LTNULL != m_pRealPlayer->GetStream())
		return PNR_OK;

	// If we have a stream, close it so we can make a new one...
	if(LTNULL != m_pRealPlayer->GetStream())
	{
		SoundSys()->CloseStream(m_pRealPlayer->GetStream());
		m_pRealPlayer->SetStream(LTNULL);
	}

	// Set up our params...
	streamBufferParams_t streamBufferParams;
	memset(&streamBufferParams, 0, sizeof(streamBufferParams_t));
	streamBufferParams.m_siParams[SBP_BUFFER_SIZE] = m_Format.uMaxBlockSize / 2;
	streamBufferParams.m_siParams[SBP_BITS_PER_CHANNEL] = m_Format.uBitsPerSample;
	streamBufferParams.m_siParams[SBP_CHANNELS_PER_SAMPLE] = m_Format.uChannels;
	streamBufferParams.m_siParams[SBP_SAMPLES_PER_SEC] = m_Format.ulSamplesPerSec;

	// Open the new stream
	m_pRealPlayer->SetStream(SoundSys()->OpenStream(&streamBufferParams));
	if (!m_pRealPlayer->GetStream())
		return PNR_FAILED;
	
	// Set volume... scale since stream volume range is 0..127
	uint32 ulVolume = 0;
	m_pRealPlayer->GetVolume(&ulVolume);
	SoundSys()->SetStreamVolume(m_pRealPlayer->GetStream(), ulVolume * 127 / 100);

	return PNR_OK;
}

/************************************************************************
 *  Method:
 *      IRMAAudioHook::OnBuffer
 *  Purpose:
 *      Audio Services calls OnBuffer() with audio data packets. The
 *	    renderer should not modify the data in the IRMABuffer part of
 *	    pAudioInData.  If the renderer wants to write a modified
 *	    version of the data back to Audio Services, then it should 
 *	    create its own IRMABuffer, modify the data and then associate 
 *	    this buffer with the pAudioOutData->pData member.
 */
//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioHook::OnBuffer(THIS_ RMAAudioData* /*IN*/ pAudioInData,
										RMAAudioData* /*OUT*/ pAudioOutData)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioHook::OnBuffer()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

	ASSERT(m_pRealPlayer);
	ASSERT(m_pRealPlayer->GetStream());

/*	if (20 < m_ulBufferWriteCount)
	{
		bool bStreamIsPlaying = false;
		if (SoundSys()->GetStreamStatus(m_pRealPlayer->GetStream()) == LS_PLAYING)
			bStreamIsPlaying = true;
		char buffer[1024];
		sprintf(buffer, "%LX %LX %s - %d  %s   %d", m_pRealPlayer, m_pRealPlayer->GetStream(), m_pRealPlayer->m_pSource, SoundSys()->GetStreamDataQueueSize(m_pRealPlayer->GetStream()), bStreamIsPlaying ? "(playing)" : "(STOPPED)", SoundSys()->GetStreamCount());
		LTRConsoleOutput(LTRA_CONOUT_ERROR, buffer);
	}*/

	bool bResult = SoundSys()->QueueStreamData(m_pRealPlayer->GetStream(), pAudioInData->pData->GetBuffer(), pAudioInData->pData->GetSize());
	uint32 ulAudioPacketPreBufferCount = 0;
	m_pRealPlayer->GetAudioPacketPreBuffer(&ulAudioPacketPreBufferCount);
	if (m_ulBufferWriteCount >= ulAudioPacketPreBufferCount && bResult)
	{
		if (LS_PLAYING != SoundSys()->GetStreamStatus(m_pRealPlayer->GetStream()))
			SoundSys()->StartStream(m_pRealPlayer->GetStream());
	}

	m_ulBufferWriteCount++;
	m_fLastOnBufferTime = time_GetTime();

	return PNR_OK;
}

#endif // LITHTECH_ESD