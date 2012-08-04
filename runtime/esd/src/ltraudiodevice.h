/****************************************************************************
;
;	MODULE:		LTRAudioDevice (.H)
;
;	PURPOSE:	Support class for RealAudioMgr
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRAudioDevice_H
#define LTRAudioDevice_H

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

#include "ltraudioqueue.h"

#include "iltsound.h"

//-----------------------------------------------------------------------------
// Audio device class
//-----------------------------------------------------------------------------
class CLTAudioDevice : public IRMAAudioDevice, public IRMACallback
{
	friend class CLTRealAudioMgr;
	friend class CLTRealAudioPlayer;
	friend class CLTRealVideoMgr;
	friend class CLTRealVideoPlayer;

public:
    CLTAudioDevice();
	~CLTAudioDevice();
	virtual LTRESULT Init();
	virtual LTRESULT Term();

   /************************************************************************
    *  IUnknown COM Interface Methods                          ref:  pncom.h
    */
    STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
    STDMETHOD_(UINT32, AddRef ) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

    /************************************************************************
    *	    IRMAAudioDevice methods
    */
    STDMETHOD(Open)				(THIS_
								const RMAAudioFormat*    /*IN*/ pFormat,
								IRMAAudioDeviceResponse* /*IN*/ pResponse);
    STDMETHOD(Close)			(THIS_
								const BOOL  /*IN*/ bFlush );
    STDMETHOD(Resume)			(THIS);
    STDMETHOD(Pause)			(THIS);
    STDMETHOD(Write)			(THIS_
								const RMAAudioData* /*IN*/ pAudioData);
    STDMETHOD_(BOOL,InitVolume)	(THIS_
								const UINT16	/*IN*/ uMinVolume,
								const UINT16	/*IN*/ uMaxVolume);
    STDMETHOD(SetVolume)		(THIS_
								const UINT16    /*IN*/ uVolume);
    STDMETHOD_(UINT16,GetVolume)(THIS);
    STDMETHOD(Reset)			(THIS);
    STDMETHOD(Drain)			(THIS);
    STDMETHOD(CheckFormat)		(THIS_
								const RMAAudioFormat* /*IN*/ pFormat);
    STDMETHOD(GetCurrentAudioTime)(THIS_
								REF(ULONG32) /*OUT*/ ulCurrentTime);

   /************************************************************************
    *  IRMACallback COM Interface Methods                    ref: rmaengin.h
    */

    /************************************************************************
     *	Method:
     *	    IRMACallback::Func
     *	Purpose:
     *	    This is the function that will be called when a callback is
     *	    to be executed.
     */
    STDMETHOD(Func)		(THIS);

protected:
    /****** Protected Class Variables ****************************************/
    INT32						m_lRefCount;
    IRMAAudioDeviceResponse*	m_pResponse;
    UINT16						m_uVolume;
    ULONG32						m_ulAudioTime;
};

#endif // LTRAudioDevice_H
#endif // LITHTECH_ESD