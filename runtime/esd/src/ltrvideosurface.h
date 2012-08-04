/****************************************************************************
;
;	MODULE:		LTRealVideoSurface (.H)
;
;	PURPOSE:	Support class for RealVideoMgr
;
;	HISTORY:	5-12-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRealVideoSurface_H
#define LTRealVideoSurface_H

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

#include "LTRVDef.h"

// Screen support
//#include "ddutils.h"

class CLTWindowlessSite;

#include "de_world.h"
#include "dtxmgr.h"

//-----------------------------------------------------------------------------
// Main class for LTRealVideoSurface
//-----------------------------------------------------------------------------
class CLTRealVideoSurface : public IRMAVideoSurface
{
	friend class CLTRealAudioMgr;
	friend class CLTRealAudioPlayer;
	friend class CLTRealVideoMgr;
	friend class CLTRealVideoPlayer;

public:
	CLTRealVideoSurface();
	~CLTRealVideoSurface();
	virtual LTRESULT Init(IUnknown* pUnknown, CLTWindowlessSite* pWindowlessSite, CLTRealVideoPlayer* pPlayer);
	virtual LTRESULT Term();

	virtual LTRESULT Update(LTRV_UPDATE_MODE updateMode);

    /*
     *	IUnknown methods
     */
    STDMETHOD (QueryInterface)	(THIS_ REFIID riid, void** ppvObj);
    STDMETHOD_(ULONG32,AddRef)	(THIS);
    STDMETHOD_(ULONG32,Release)	(THIS);

    /*
     * IRMAVideoSurface methods usually called by renderers to
     * Draw on the surface
     */
    STDMETHOD(Blt) (THIS_
					UCHAR*					/*IN*/	pImageData, 
					RMABitmapInfoHeader*	/*IN*/	pBitmapInfo, 
					REF(PNxRect)			/*IN*/	inDestRect,
					REF(PNxRect)			/*IN*/	inSrcRect);

    STDMETHOD(BeginOptimizedBlt) (THIS_ 
					RMABitmapInfoHeader*	/*IN*/	pBitmapInfo);
    
    STDMETHOD(OptimizedBlt) (THIS_
					UCHAR*					/*IN*/	pImageBits,
					REF(PNxRect)			/*IN*/	rDestRect, 
					REF(PNxRect)			/*IN*/	rSrcRect);
    
    STDMETHOD(EndOptimizedBlt) (THIS);

    STDMETHOD(GetOptimizedFormat) (THIS_
				  REF(RMA_COMPRESSION_TYPE) /*OUT*/ ulType);

    STDMETHOD(GetPreferredFormat) (THIS_
				  REF(RMA_COMPRESSION_TYPE) /*OUT*/ ulType);

protected:
    INT32					m_lRefCount;
    IUnknown*				m_pUnknown;

	CLTRealVideoPlayer*		m_pPlayer;

    RMABitmapInfoHeader		m_BitmapInfo;
	UCHAR*					m_pImageBits;
	INT32					m_lImageBufferSize;

protected: // 3D specific

	LTRESULT				CreateTexture(UINT32 uWidth, UINT32 uHeight);
	LTRESULT				DestroyTexture();

	TextureData*			m_pTextureData;
	UINT32					m_uTextureWidth;
	UINT32					m_uTextureHeight;
	SharedTexture			m_SharedTexture;
};

#endif // LTRealVideoSurface_H
#endif // LITHTECH_ESD