/****************************************************************************
;
;	MODULE:		LTClientEngineSetup (.CPP)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	12-8-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#include "ltrclientenginesetup.h"
#include "ltrconout.h"

//-----------------------------------------------------------------------------
// CLTClientEngineSetup member functions
//-----------------------------------------------------------------------------
CLTClientEngineSetup::CLTClientEngineSetup()
{
	m_bInitialized = LTFALSE;
	m_RefCount = 0;
}

//-----------------------------------------------------------------------------
CLTClientEngineSetup::~CLTClientEngineSetup()
{
	Term();
}

//-----------------------------------------------------------------------------
LTRESULT CLTClientEngineSetup::Init()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientEngineSetup::Init()");

	if (LTTRUE == m_bInitialized)
		return LT_ALREADYINITIALIZED;

	m_bInitialized = LTTRUE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
LTRESULT CLTClientEngineSetup::Term()
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientEngineSetup::Term()");

	if (LTTRUE != m_bInitialized)
		return LT_NOTINITIALIZED;

	m_bInitialized = LTFALSE;

	return LT_OK;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientEngineSetup::QueryInterface(REFIID riid, void** ppvObj)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientEngineSetup::QueryInterface()");

    if (IsEqualIID(riid, IID_IUnknown))
    {
		AddRef();
		*ppvObj = (IUnknown*)(IRMAClientEngineSetup*)this;
		return PNR_OK;
    }
    else if (IsEqualIID(riid, IID_IRMAClientEngineSetup))
    {
		AddRef();
		*ppvObj = (IRMAClientEngineSetup*) this;
		return PNR_OK;
    }

    *ppvObj = NULL;
    
	return PNR_NOINTERFACE;
}

/****************************************************************************
 *  IUnknown::AddRef                                            ref:  pncom.h
 *
 *  This routine increases the object reference count in a thread safe
 *  manner. The reference count is used to manage the lifetime of an object.
 *  This method must be explicitly called by the user whenever a new
 *  reference to an object is used.
 */
STDMETHODIMP_(UINT32)
CLTClientEngineSetup::AddRef(void)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientEngineSetup::AddRef()");

ASSERT(0);

	return InterlockedIncrement(&m_RefCount);
}


/****************************************************************************
 *  IUnknown::Release                                           ref:  pncom.h
 *
 *  This routine decreases the object reference count in a thread safe
 *  manner, and deletes the object if no more references to it exist. It must
 *  be called explicitly by the user whenever an object is no longer needed.
 */
STDMETHODIMP_(UINT32)
CLTClientEngineSetup::Release(void)
{
	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTClientEngineSetup::Release()");

	if (InterlockedDecrement(&m_RefCount) > 0)
	{
		return m_RefCount;
	}

	delete this;
	return 0;
}

//-----------------------------------------------------------------------------
STDMETHODIMP CLTClientEngineSetup::Setup(THIS_ IUnknown* pContext)
{
	return 0;
}

#endif // LITHTECH_ESD
