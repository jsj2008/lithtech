/****************************************************************************
;
;	MODULE:		LTRClientContext (.CPP)
;
;	PURPOSE:	Support class for RealAudioMgr
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrclientcontext.h"
#include "ltrconout.h"

//-----------------------------------------------------------------------------
// CLTClientContext member functions
//-----------------------------------------------------------------------------
CLTClientContext::CLTClientContext()
{
	m_lRefCount = 0;
	m_pClientAdviceSink = NULL;
	m_pErrorSink = NULL;
	m_pSiteSupplier = NULL;
}

//-----------------------------------------------------------------------------
CLTClientContext::~CLTClientContext()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTClientContext::Init(IUnknown* /*IN*/ pUnknown, CLTRealVideoPlayer* pPlayer)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientContext::Init()");

	LT_MEM_TRACK_ALLOC(m_pClientAdviceSink = new CLTClientAdviceSink,LT_MEM_TYPE_MISC);
	ASSERT(m_pClientAdviceSink);
	m_pClientAdviceSink->Init(pUnknown);
	m_pClientAdviceSink->AddRef();

	LT_MEM_TRACK_ALLOC(m_pErrorSink = new CLTErrorSink,LT_MEM_TYPE_MISC);
	ASSERT(m_pErrorSink);
	m_pErrorSink->Init(pUnknown);
	m_pErrorSink->AddRef();

	LT_MEM_TRACK_ALLOC(m_pSiteSupplier = new CLTSiteSupplier,LT_MEM_TYPE_MISC);
	ASSERT(m_pSiteSupplier);
	m_pSiteSupplier->Init(pUnknown, pPlayer);
	m_pSiteSupplier->AddRef();

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTClientContext::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientContext::Term()");

	if (m_pClientAdviceSink)
	{
		m_pClientAdviceSink->Release();
		m_pClientAdviceSink = NULL;
	}

	if (m_pErrorSink)
	{
		m_pErrorSink->Release();
		m_pErrorSink = NULL;
	}

	if (m_pSiteSupplier)
	{
		m_pSiteSupplier->Release();
		m_pSiteSupplier = NULL;
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientContext::QueryInterface(REFIID riid, void** ppvObj)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientContext::QueryInterface()");

    if (IsEqualIID(riid, IID_IUnknown))
    {
		AddRef();
		*ppvObj = this;
		return PNR_OK;
    }
    else if (m_pClientAdviceSink && 
	     m_pClientAdviceSink->QueryInterface(riid, ppvObj) == PNR_OK)
    {
		return PNR_OK;
    }
    else if (m_pErrorSink && 
	     m_pErrorSink->QueryInterface(riid, ppvObj) == PNR_OK)
    {
		return PNR_OK;
    }/*
    else if(m_pAuthMgr &&
	    m_pAuthMgr->QueryInterface(riid, ppvObj) == PNR_OK)
    {
	return PNR_OK;
    }*/
    else if(m_pSiteSupplier &&
	    m_pSiteSupplier->QueryInterface(riid, ppvObj) == PNR_OK)
    {
	return PNR_OK;
    }

    *ppvObj = NULL;

    return PNR_NOINTERFACE;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTClientContext::AddRef()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientContext::AddRef()");

    return InterlockedIncrement(&m_lRefCount);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(UINT32) CLTClientContext::Release()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientContext::Release()");

    if (InterlockedDecrement(&m_lRefCount) > 0)
        return m_lRefCount;

    delete this;
    return 0;
}

#endif // LITHTECH_ESD