/****************************************************************************
;
;	MODULE:		LTAudioInfoResponse (.H)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	11-02-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTAudioInfoResponse_H
#define LTAudioInfoResponse_H

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

#include "ltraudiohook.h"
#include "ltrdrynotification.h"

//-----------------------------------------------------------------------------
// Main class for CLTAudioInfoResponse
//-----------------------------------------------------------------------------
class CLTAudioInfoResponse : public IRMAAudioStreamInfoResponse
{
public:
	// Default constructor
	CLTAudioInfoResponse();

	// Default destructor (calls Term if it has not been called)
	~CLTAudioInfoResponse();

	// Initialize audio info response
	LTRESULT	Init(CLTAudioHook* pAudioHook, CLTDryNotification* pDryNotification);

	// Terminate audio info response
	LTRESULT	Term();

    /*
     *  IUnknown methods
     */
    STDMETHOD(QueryInterface)       (THIS_
				                    REFIID riid,
								    void** ppvObj);

    STDMETHOD_(ULONG32,AddRef)      (THIS);

    STDMETHOD_(ULONG32,Release)     (THIS);

    /*
     *  IRMAAudioStreamInfoResponse methods
     */

    /************************************************************************
     *  Method:
     *      IRMAAudioStreamInfoResponse::OnStream
     *  Purpose:
     *	    The client implements this to get notification of streams 
     *	    associated with this player. Use 
     *	    AudioPlayer::SetStreamInfoResponse() to register your 
     *	    implementation with the AudioPlayer. Once player has been 
     *	    initialized, it will call OnStream() multiple times to pass all 
     *	    the streams. Since a stream can be added mid-presentation, 
     *	    IRMAAudioStreamInfoResponse object should be written to handle 
     *	    OnStream() in the midst of the presentation as well.
     */
    STDMETHOD(OnStream) (THIS_
			IRMAAudioStream* /*IN*/ pAudioStream);

protected:
	LTBOOL				m_bInitialized;
    INT32				m_lRefCount;
	CLTAudioHook*		m_pAudioHook;
	CLTDryNotification*	m_pDryNotification;
};

#endif // LTAudioInfoResponse_H
#endif // LITHTECH_ESD