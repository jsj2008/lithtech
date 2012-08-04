/****************************************************************************
;
;	MODULE:		LTRAudioDevice (.CPP)
;
;	PURPOSE:	Support class for RealAudioMgr
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltraudiodevice.h"
#include "ltrconout.h"
#include "soundmgr.h"

//-----------------------------------------------------------------------------
// CLTAudioDevice member functions
//-----------------------------------------------------------------------------
CLTAudioDevice::CLTAudioDevice()
{
	m_lRefCount = 0;
	m_pResponse = NULL;
	m_uVolume = 100;
	m_ulAudioTime = 0;
}

//-----------------------------------------------------------------------------
CLTAudioDevice::~CLTAudioDevice()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioDevice::Init()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Init()");

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioDevice::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Term()");

	// Clean up response
	if (m_pResponse)
	{
		m_pResponse->Release();
		m_pResponse = NULL;
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::QueryInterface(REFIID riid, void**ppvObj)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::QueryInterface()");

    if(IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppvObj = (IUnknown*)(IRMAAudioDevice*)this;
        return PNR_OK;
    }
    else if(IsEqualIID(riid, IID_IRMAAudioDevice))
    {
        AddRef();
        *ppvObj = (IRMAAudioDevice*)this;
        return PNR_OK;
    }
    else if(IsEqualIID(riid, IID_IRMACallback))
    {
        AddRef();
        *ppvObj = (IRMACallback*)this;
        return PNR_OK;
    }
    
	*ppvObj = NULL;
    
	return PNR_NOINTERFACE;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTAudioDevice::AddRef()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::AddRef()");
	return InterlockedIncrement(&m_lRefCount);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTAudioDevice::Release()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Release()");
    if (InterlockedDecrement(&m_lRefCount) > 0)
        return m_lRefCount;

    delete this;
    return 0;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::Open(const RMAAudioFormat* /*IN*/ pFormat,
									IRMAAudioDeviceResponse* /*IN*/ pResponse)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Open()");

    if (!pResponse || !pFormat)
    {
		return PNR_INVALID_PARAMETER;
    }

    m_pResponse = pResponse;
    m_pResponse->AddRef();

	return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::Close(const BOOL /*IN*/ bFlush)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Close()");

    if (m_pResponse)
    {
		m_pResponse->Release();
		m_pResponse = NULL;
    }

    if (bFlush)
    {
		Drain();
    }

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::Resume()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Resume()");

	ASSERT(m_pResponse);
	m_pResponse->OnTimeSync(m_ulAudioTime);

	return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::Pause()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Pause()");

	return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::Write(const RMAAudioData* /*IN*/ pAudioData)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Write()");

//	HANDLE hThread = GetCurrentThread();
//	if (hThread)
//		SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);

	m_ulAudioTime = pAudioData->ulAudioTime;

	ASSERT(m_pResponse);
	m_pResponse->OnTimeSync(m_ulAudioTime);

	return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(BOOL) CLTAudioDevice::InitVolume(const UINT16 /*IN*/ uMinVolume,
												const UINT16 /*IN*/ uMaxVolume)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::InitVolume(), \
		Min Volume: %ul, Max Volume: %ul", uMinVolume, uMaxVolume);

    return LTTRUE; // this audio device doesn't care about the volume
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::SetVolume(const UINT16 /*IN*/ uVolume)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::SetVolume(), \
		Volume: %ul", uVolume);

    m_uVolume = uVolume;

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT16) CLTAudioDevice::GetVolume()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::GetVolume()");

    return m_uVolume;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::Reset()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Reset()");

	return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::Drain()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Drain()");

	return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::CheckFormat(const RMAAudioFormat* /*IN*/ pFormat)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::CheckFormat()");

    return PNR_OK; // we accept any format
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::GetCurrentAudioTime(REF(ULONG32) /*OUT*/ ulCurrentTime)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::GetCurrentAudioTime(), \
		CurrentTime: %ul", ulCurrentTime);

	ulCurrentTime = 0;//m_ulAudioTime;	[mds] 3/8/2001 - Lie to the core...

	return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTAudioDevice::Func()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioDevice::Func()");

	return PNR_OK;
}

#endif // LITHTECH_ESD