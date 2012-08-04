/****************************************************************************
;
;	MODULE:		LTRErrorSink (.CPP)
;
;	PURPOSE:	Support class for RealAudioMgr
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrerrorsink.h"
#include "ltrconout.h"

//-----------------------------------------------------------------------------
// CLTErrorSink member functions
//-----------------------------------------------------------------------------
CLTErrorSink::CLTErrorSink()
{
	m_lRefCount = 0;
	m_pUnknown = NULL;
}

//-----------------------------------------------------------------------------
CLTErrorSink::~CLTErrorSink()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTErrorSink::Init(IUnknown* pUnknown)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTErrorSink::Init()");

	if (pUnknown)
	{
		m_pUnknown = pUnknown;
		m_pUnknown->AddRef();
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTErrorSink::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTErrorSink::Term()");

	if (m_pUnknown)
	{
		m_pUnknown->Release();
		m_pUnknown = NULL;
	}

	return LT_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTErrorSink::ErrorOccurred(	const UINT8		unSeverity,  
											const ULONG32	ulRMACode,
											const ULONG32	ulUserCode,
											const char*		pUserString,
											const char*		pMoreInfoURL)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTErrorSink::ErrorOccurred()");

	IRMAErrorMessages* pErrorMessages = NULL;
    IRMABuffer* pBuffer = NULL;

	if (m_pUnknown &&
		SUCCEEDED(m_pUnknown->QueryInterface(IID_IRMAErrorMessages, (void**)&pErrorMessages)))
	{
		ASSERT(pErrorMessages);
		pBuffer = pErrorMessages->GetErrorText(ulRMACode);
	}

    LTRConsoleOutput(LTRA_CONOUT_ERROR, "Report(%d, 0x%x, \"%s\", %ld, \"%s\", \"%s\")",
				unSeverity,
				ulRMACode,
				(pUserString && *pUserString) ? pUserString : "(NULL)",
				ulUserCode,
				(pMoreInfoURL && *pMoreInfoURL) ? pMoreInfoURL : "(NULL)",
				pBuffer ? (char *)pBuffer->GetBuffer() : "(unknown error)");

    if (NULL != pBuffer)
		pBuffer->Release();

	if (NULL != pErrorMessages)
		pErrorMessages->Release();

    return PNR_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG32) CLTErrorSink::AddRef()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTErrorSink::AddRef()");

    return InterlockedIncrement(&m_lRefCount);
}

//-----------------------------------------------------------------------------
STDMETHODIMP_(ULONG32) CLTErrorSink::Release()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTErrorSink::Release()");

    if (InterlockedDecrement(&m_lRefCount) > 0)
    {
        return m_lRefCount;
    }

    delete this;

    return 0;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTErrorSink::QueryInterface(REFIID riid, void** ppvObj)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTErrorSink::QueryInterface()");

    if (IsEqualIID(riid, IID_IUnknown))
    {
		AddRef();
		*ppvObj = (IUnknown*)(IRMAErrorSink*)this;
		return PNR_OK;
    }
    else if (IsEqualIID(riid, IID_IRMAErrorSink))
    {
		AddRef();
		*ppvObj = (IRMAErrorSink*) this;
		return PNR_OK;
    }

    *ppvObj = NULL;
    
	return PNR_NOINTERFACE;
}

#endif // LITHTECH_ESD