/****************************************************************************
;
;	MODULE:		LTRAudioHook (.H)
;
;	PURPOSE:	Implement Real support in LithTech engine
;
;	HISTORY:	10-25-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRAudioHook_H
#define LTRAudioHook_H

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

// Forward declarations
class CLTRealPlayer;

//-----------------------------------------------------------------------------
// Main class for CLTAudioHook
//-----------------------------------------------------------------------------
class CLTAudioHook : public IRMAAudioHook
{
public:
	// Default constructor
	CLTAudioHook();

	// Default destructor (calls Term if it has not been called)
	~CLTAudioHook();

	// Initialize hook
	LTRESULT	Init(CLTRealPlayer* pRealPlayer);

	// Terminate hook
	LTRESULT	Term();

	// Update hook
	LTRESULT	Update();

	/************************************************************************
	*  IUnknown COM Interface Methods                          ref:  pncom.h
	*/
	STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
	STDMETHOD_(UINT32, AddRef ) (THIS);
	STDMETHOD_(UINT32, Release) (THIS);

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
    STDMETHOD(OnInit)		(THIS_
                    		RMAAudioFormat*	/*IN*/ pFormat);

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
    STDMETHOD(OnBuffer)		(THIS_
                    		RMAAudioData*	/*IN*/   pAudioInData,
                    		RMAAudioData*	/*OUT*/  pAudioOutData);
protected:
	LTBOOL				m_bInitialized;
    INT32				m_lRefCount;
	CLTRealPlayer*		m_pRealPlayer;
	RMAAudioFormat		m_Format;
	uint32				m_ulBufferWriteCount;
	float				m_fLastOnBufferTime;
};

#endif // LTRAudioHook_H
#endif // LITHTECH_ESD