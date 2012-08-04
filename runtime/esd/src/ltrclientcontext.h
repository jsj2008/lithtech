/****************************************************************************
;
;	MODULE:		LTRClientContext (.H)
;
;	PURPOSE:	Support class for RealAudioMgr
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRClientContext_H
#define LTRClientContext_H

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

#include "LTRClientAdviceSink.h"
#include "LTRErrorSink.h"
#include "LTRSiteSupplier.h"

//-----------------------------------------------------------------------------
// Client context class
//-----------------------------------------------------------------------------
class CLTClientContext : public IUnknown
{
	friend class CLTRealAudioMgr;
	friend class CLTRealAudioPlayer;
	friend class CLTRealVideoMgr;
	friend class CLTRealVideoPlayer;

public:
	CLTClientContext();
	~CLTClientContext();
	virtual LTRESULT Init(IUnknown* /*IN*/ pUnknown, CLTRealVideoPlayer* pPlayer = NULL);
	virtual LTRESULT Term();

   /************************************************************************
    *  IUnknown COM Interface Methods                          ref:  pncom.h
    */
    STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
    STDMETHOD_(UINT32, AddRef ) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);


protected:
    INT32							m_lRefCount;
    CLTClientAdviceSink*			m_pClientAdviceSink;
	CLTErrorSink*					m_pErrorSink;
    //ExampleAuthenticationManager*	m_pAuthMgr;
    //ExampleSiteSupplier*			m_pSiteSupplier;
	CLTSiteSupplier*				m_pSiteSupplier;
};

#endif // CLTClientContext_H
#endif // LITHTECH_ESD