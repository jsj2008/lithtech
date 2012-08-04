/****************************************************************************
;
;	MODULE:		LTRClientAdviceSink (.CPP)
;
;	PURPOSE:	Support class for RealAudioMgr
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrclientadvicesink.h"
#include "ltrconout.h"
#include "ltreal_impl.h"
#include "timemgr.h"
#include "console.h"

extern LTBOOL g_bShowRealPlayTime;

//-----------------------------------------------------------------------------
CLTClientAdviceSink::CLTClientAdviceSink()
{
	m_lRefCount = 0;
	m_pUnknown = LTNULL;
	m_pRegistry = LTNULL;

	m_pPlayer = LTNULL;
}

//-----------------------------------------------------------------------------
CLTClientAdviceSink::~CLTClientAdviceSink()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTClientAdviceSink::Init(IUnknown* /*IN*/ pUnknown)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::Init()");

	if (pUnknown)
	{
		m_pUnknown = pUnknown;
		m_pUnknown->AddRef();

		if (PNR_OK != m_pUnknown->QueryInterface(IID_IRMAPNRegistry,
													(void**)&m_pRegistry))
		    m_pRegistry = LTNULL;

		IRMAPlayer* pPlayer = LTNULL;
		if(PNR_OK == m_pUnknown->QueryInterface(IID_IRMAPlayer,
													(void**)&pPlayer))
		{
			pPlayer->AddAdviseSink(this);
			pPlayer->Release();
		}
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTClientAdviceSink::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::Term()");

    if (m_pRegistry)
    {
		m_pRegistry->Release();
		m_pRegistry = LTNULL;
    }

    if (m_pUnknown)
    {
		m_pUnknown->Release();
		m_pUnknown = LTNULL;
    }

	return LT_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnPosLength(UINT32 ulPosition,
												UINT32 ulLength)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnPosLength(), \
		Position: %ul, Length: %ul", ulPosition, ulLength);

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnPresentationOpened()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnPresentationOpened()");

//	HANDLE hThread = GetCurrentThread();
//	if (hThread)
//		SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnPresentationClosed()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnPresentationClosed()");

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnStatisticsChanged(void)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnStatisticsChanged()");

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnPreSeek(UINT32 ulOldTime,
												UINT32 ulNewTime)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnPreSeek(), \
		Old Time: %ul, New Time: %ul", ulOldTime, ulNewTime);

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnPostSeek(UINT32	ulOldTime,
												UINT32 ulNewTime)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnPostSeek(), \
		Old Time: %ul, New Time: %ul", ulOldTime, ulNewTime);

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnStop(void)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnStop()");

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnPause(UINT32 ulTime)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnPause(), \
		Time: %ul", ulTime);

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnBegin(UINT32 ulTime)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnBegin(), \
		Time: %ul", ulTime);

	if (m_pPlayer && g_bShowRealPlayTime)
	{
		m_pPlayer->m_fActualPlayTime = time_GetTime();
		char msg[256];
		sprintf(msg, "Delay %f", m_pPlayer->m_fActualPlayTime - m_pPlayer->m_fRequestedPlayTime);
		con_PrintString(CONRGB(255, 100, 255), 0, msg);
	}

	if (m_pPlayer && m_pPlayer->m_bWaitingToPlay)
		m_pPlayer->m_bWaitingToPlay = LTFALSE;

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnBuffering(UINT32 ulFlags,
												UINT16 unPercentComplete)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnBuffering(), \
		Flags: %ul, PercentComplete: %ul", ulFlags, unPercentComplete);

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::OnContacting(const char* pHostName)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::OnContacting(), \
		Hostname: %s", pHostName);

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTClientAdviceSink::AddRef()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::AddRef()");

    return InterlockedIncrement(&m_lRefCount);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTClientAdviceSink::Release()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::Release()");

    if (InterlockedDecrement(&m_lRefCount) > 0)
    {
        return m_lRefCount;
    }

    delete this;
    return 0;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientAdviceSink::QueryInterface(REFIID riid, void** ppvObj)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientAdviceSink::QueryInterface()");

    if (IsEqualIID(riid, IID_IUnknown))
    {
		AddRef();
		*ppvObj = (IUnknown*)(IRMAClientAdviseSink*)this;
		return PNR_OK;
    }
    else if (IsEqualIID(riid, IID_IRMAClientAdviseSink))
    {
		AddRef();
		*ppvObj = (IRMAClientAdviseSink*)this;
		return PNR_OK;
    }

    *ppvObj = LTNULL;
    return PNR_NOINTERFACE;
}

#endif // LITHTECH_ESD