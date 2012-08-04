/****************************************************************************
;
;	MODULE:		LTRClientAdviceSink (.H)
;
;	PURPOSE:	Support class for RealAudioMgr
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRClientAdviceSink_H
#define LTRClientAdviceSink_H

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
// Client advice sink class
//-----------------------------------------------------------------------------
class CLTClientAdviceSink : public IRMAClientAdviseSink
{
	friend class CLTRealPlayer;
	friend class CLTRealAudioMgr;
	friend class CLTRealAudioPlayer;
	friend class CLTRealVideoMgr;
	friend class CLTRealVideoPlayer;

public:
    CLTClientAdviceSink();
	~CLTClientAdviceSink();
	virtual LTRESULT Init(IUnknown* /*IN*/ pUnknown);
	virtual LTRESULT Term();

   /************************************************************************
    *  IRMAClientAdviseSink Interface Methods               ref:  rmaclsnk.h
    */
    STDMETHOD (OnPosLength)  (THIS_ UINT32 ulPosition, UINT32 ulLength);
    STDMETHOD (OnPresentationOpened) (THIS);
    STDMETHOD (OnPresentationClosed) (THIS);
    STDMETHOD (OnStatisticsChanged)  (THIS);
    STDMETHOD (OnPreSeek)    (THIS_ UINT32 ulOldTime, UINT32 ulNewTime);
    STDMETHOD (OnPostSeek)   (THIS_ UINT32 ulOldTime, UINT32 ulNewTime);
    STDMETHOD (OnStop)       (THIS);
    STDMETHOD (OnPause)      (THIS_ UINT32 ulTime);
    STDMETHOD (OnBegin)	     (THIS_ UINT32 ulTime);
    STDMETHOD (OnBuffering)  (THIS_ UINT32 ulFlags, UINT16 unPercentComplete);
    STDMETHOD (OnContacting) (THIS_ const char* pHostName);

   /************************************************************************
    *  IUnknown COM Interface Methods                          ref:  pncom.h
    */
    STDMETHOD (QueryInterface ) (THIS_ REFIID ID, void** ppInterfaceObj);
    STDMETHOD_(UINT32, AddRef ) (THIS);
    STDMETHOD_(UINT32, Release) (THIS);

protected:
    //void ShowMeTheStatistics (char* /*IN*/ pszRegistryKey);

    INT32				m_lRefCount;
    IUnknown*			m_pUnknown;
    IRMAPNRegistry*		m_pRegistry;

	CLTRealPlayer*		m_pPlayer;
};

#endif // LTRClientAdviceSink_H
#endif // LITHTECH_ESD