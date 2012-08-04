/****************************************************************************
;
;	MODULE:		LTRSiteSupplier (.H)
;
;	PURPOSE:	Implement RealVideo capability for LithTech engine
;
;	HISTORY:	5-12-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRSiteSupplier_H
#define LTRSiteSupplier_H

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
#include "rmavsurf.h"

#include "ltrmap.h"
#include "ltrwindowlesssite.h"

/****************************************************************************
 *
 *  CLTSiteSupplier Class
 *
 *  Implementation for ragui's IRMASiteSupplier.
 */
class CLTSiteSupplier :  public IRMASiteSupplier
{
	friend class CLTRealAudioMgr;
	friend class CLTRealAudioPlayer;
	friend class CLTRealVideoMgr;
	friend class CLTRealVideoPlayer;

public:
	CLTSiteSupplier();
	~CLTSiteSupplier();

	virtual LTRESULT Init(IUnknown* m_pUnknown, CLTRealVideoPlayer* pPlayer);
	virtual LTRESULT Term();
    

    /************************************************************************
     *  IRMASiteSupplier Interface Methods                     ref:  rmawin.h
     */
    STDMETHOD(SitesNeeded) (THIS_ UINT32 uRequestID, IRMAValues* pSiteProps);
    STDMETHOD(SitesNotNeeded) (THIS_ UINT32 uRequestID);
    STDMETHOD(BeginChangeLayout) (THIS);
    STDMETHOD(DoneChangeLayout) (THIS);


    /************************************************************************
     *  IUnknown COM Interface Methods                          ref:  pncom.h
     */
    STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
    STDMETHOD_(UINT32, AddRef ) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);


    private:
    /****** Private Class Variables ****************************************/
    INT32					m_lRefCount;
    IRMASiteManager*		m_pSiteManager;
    IRMACommonClassFactory*	m_pCCF;
    IUnknown*				m_pUnknown;
    LTRMap					m_CreatedSites;

    // our windowless site
    CLTWindowlessSite*		m_pWindowlessSite;

	CLTRealVideoPlayer*		m_pRealVideoPlayer;
};

#endif // LTRSiteSupplier_H
#endif // LITHTECH_ESD