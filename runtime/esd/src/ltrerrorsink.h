/****************************************************************************
;
;	MODULE:		LTRErrorSink (.H)
;
;	PURPOSE:	Support class for RealAudioMgr
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRErrorSink_H
#define LTRErrorSink_H

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

//-----------------------------------------------------------------------------
// Error sink class
//-----------------------------------------------------------------------------
class CLTErrorSink : public IRMAErrorSink
{
	friend class CLTRealAudioMgr;
	friend class CLTRealAudioPlayer;
	friend class CLTRealVideoMgr;
	friend class CLTRealVideoPlayer;

public:
	CLTErrorSink();
	~CLTErrorSink();
	virtual LTRESULT Init(IUnknown* pUnknown);
	virtual LTRESULT Term();

   /************************************************************************
    *  IRMAErrorSink Interface Methods                      ref:  rmaerror.h
    */
    STDMETHOD(ErrorOccurred)
	(THIS_
	 const UINT8	unSeverity,  
	 const UINT32	ulRMACode,
	 const UINT32	ulUserCode,
	 const char*	pUserString,
	 const char*	pMoreInfoURL
	);

   /************************************************************************
    *  IUnknown COM Interface Methods                          ref:  pncom.h
    */
    STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
    STDMETHOD_(UINT32, AddRef ) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

protected:
    INT32			m_lRefCount;
    IUnknown*		m_pUnknown;
};

#endif // LTRErrorSink_H
#endif // LITHTECH_ESD