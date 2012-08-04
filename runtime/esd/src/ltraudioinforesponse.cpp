/****************************************************************************
;
;	MODULE:		LTAudioInfoResponse (.CPP)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	11-02-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltraudioinforesponse.h"
#include "ltrconout.h"

//-----------------------------------------------------------------------------
// CLTAudioInfoResponse member functions
//-----------------------------------------------------------------------------
CLTAudioInfoResponse::CLTAudioInfoResponse()
{
	m_bInitialized = LTFALSE;
	m_lRefCount = 0;
	m_pAudioHook = LTNULL;
	m_pDryNotification = LTNULL;
}

//-----------------------------------------------------------------------------
CLTAudioInfoResponse::~CLTAudioInfoResponse()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioInfoResponse::Init(CLTAudioHook* pAudioHook, CLTDryNotification* pDryNotification)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioInfoResponse::Init()");

	if (m_bInitialized)
		return LT_ALREADYINITIALIZED;

	ASSERT(pAudioHook);
	ASSERT(pDryNotification);

	m_pAudioHook = pAudioHook;
	m_pDryNotification = pDryNotification;

	m_bInitialized = LTTRUE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTAudioInfoResponse::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioInfoResponse::Term()");

	if (!m_bInitialized)
		LTRConsoleOutput(LTRA_CONOUT_WARNING, "CLTAudioInfoResponse terminated without initialization");

	return LT_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTAudioInfoResponse::AddRef()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioInfoResponse::AddRef()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

    return InterlockedIncrement(&m_lRefCount);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTAudioInfoResponse::Release()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioInfoResponse::Release()");

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
STDMETHODIMP CLTAudioInfoResponse::QueryInterface(REFIID riid, void** ppvObj)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioInfoResponse::QueryInterface()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppvObj = this;
        return PNR_OK;
    }
    else if (IsEqualIID(riid, IID_IRMAAudioStreamInfoResponse))
    {
        AddRef();
        *ppvObj = (IRMAAudioStreamInfoResponse*)this;
        return PNR_OK;
    }
    *ppvObj = NULL;
    return PNR_NOINTERFACE;
}

STDMETHODIMP CLTAudioInfoResponse::OnStream(IRMAAudioStream* /*IN*/ pAudioStream)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTAudioInfoResponse::OnStream()");

	if (!m_bInitialized)
		return LT_NOTINITIALIZED;

	ASSERT(pAudioStream);
	ASSERT(m_pAudioHook);
	
	pAudioStream->AddPreMixHook(m_pAudioHook, LTFALSE);
	pAudioStream->AddDryNotification(m_pDryNotification);

    return PNR_OK;
}

#endif // LITHTECH_ESD