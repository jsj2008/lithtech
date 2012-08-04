/****************************************************************************
;
;	MODULE:		LTRDryNotification (.H)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	11-28-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRDryNotification_H
#define LTRDryNotification_H

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

#include "ltrconout.h"

//-----------------------------------------------------------------------------
// Main class for CLTDryNotification
//-----------------------------------------------------------------------------
class CLTDryNotification : public IRMADryNotification
{
public:
	// Default constructor
	CLTDryNotification();

	// Default destructor (calls Term if it has not been called)
	~CLTDryNotification();

	// Initialize
	LTRESULT	Init();

	// Terminate
	LTRESULT	Term();

	/************************************************************************
	*  IUnknown COM Interface Methods                          ref:  pncom.h
	*/
	STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
	STDMETHOD_(UINT32, AddRef ) (THIS);
	STDMETHOD_(UINT32, Release) (THIS);

    /*
     *  IRMADryNotification methods
     */

    /************************************************************************
     *  Method:
     *      IRMADryNotification::OnDryNotification
     *  Purpose:
     *	    This function is called when it is time to write to audio device 
     *	    and there is not enough data in the audio stream. The renderer can
     *	    then decide to add more data to the audio stream. This should be 
     *	    done synchronously within the call to this function.
     *	    It is OK to not write any data. Silence will be played instead.
     */
    STDMETHOD(OnDryNotification)    (THIS_
				    UINT32 /*IN*/ ulCurrentStreamTime,
				    UINT32 /*IN*/ ulMinimumDurationRequired
				    );

protected:
    INT32				m_lRefCount;
};

// Default constructor
inline CLTDryNotification::CLTDryNotification()
{
	m_lRefCount = 0;
}

// Default destructor (calls Term if it has not been called)
inline CLTDryNotification::~CLTDryNotification()
{
	Term();
}

// Initialize
inline LTRESULT CLTDryNotification::Init()
{
	return LT_OK;
}

// Terminate
inline LTRESULT CLTDryNotification::Term()
{
	return LT_OK;
}


/*
 *  IUnknown methods
 */
//-----------------------------------------------------------------------------
inline STDMETHODIMP_(UINT32) CLTDryNotification::AddRef()
{
//	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTDryNotification::AddRef()");

//	if (!m_bInitialized)
//		return LT_NOTINITIALIZED;

    return InterlockedIncrement(&m_lRefCount);
}

//-----------------------------------------------------------------------------
inline STDMETHODIMP_(UINT32) CLTDryNotification::Release()
{
//	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTDryNotification::Release()");

//	if (!m_bInitialized)
//		return LT_NOTINITIALIZED;

    if (InterlockedDecrement(&m_lRefCount) > 0)
    {
        return m_lRefCount;
    }

    delete this;
    return LTNULL;
}

//-----------------------------------------------------------------------------
inline STDMETHODIMP CLTDryNotification::QueryInterface(REFIID riid, void** ppvObj)
{
//	LTRConsoleOutput(LTRA_CONOUT_INFO, "CLTDryNotification::QueryInterface()");

//	if (!m_bInitialized)
//		return LT_NOTINITIALIZED;

    if (IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppvObj = (IUnknown*)(IRMADryNotification*)this;
        return PNR_OK;
    }
	else if (IsEqualIID(riid, IID_IRMADryNotification))
	{
		AddRef();
		*ppvObj = (IRMADryNotification*)this;
		return PNR_OK;
	}
    *ppvObj = NULL;
    return PNR_NOINTERFACE;
};

/*
 *  IRMADryNotification methods
 */

/************************************************************************
 *  Method:
 *      IRMADryNotification::OnDryNotification
 *  Purpose:
 *	    This function is called when it is time to write to audio device 
 *	    and there is not enough data in the audio stream. The renderer can
 *	    then decide to add more data to the audio stream. This should be 
 *	    done synchronously within the call to this function.
 *	    It is OK to not write any data. Silence will be played instead.
 */
inline STDMETHODIMP CLTDryNotification::OnDryNotification (THIS_
				UINT32 /*IN*/ ulCurrentStreamTime,
				UINT32 /*IN*/ ulMinimumDurationRequired
				)
{
//	LTRConsoleOutput(LTRA_CONOUT_ERROR, "CLTDryNotification Time(%d) Min Duration(%d)", ulCurrentStreamTime, ulMinimumDurationRequired);
	return PNR_OK;
};

#endif // LTRDryNotification_H
#endif // LITHTECH_ESD
