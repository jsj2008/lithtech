/****************************************************************************
;
;	MODULE:		LTRealVideo_Impl (.H)
;
;	PURPOSE:	Implement RealVideo capability for LithTech engine
;
;	HISTORY:	5-12-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRealVideo_Impl_H
#define LTRealVideo_Impl_H

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

#include "LTRClientContext.h"
#include "LTRAudioDevice.h"
#include "LTRAudioHook.h"
#include "LTRAudioInfoResponse.h"
#include "LTRVDef.h"
#include "LTRVideoOverlay.h"
#include "LTReal_impl.h"

#include "de_world.h"

class CLTRealVideoPlayer;
typedef CLithBaseList<CLTRealVideoPlayer> VideoPlayerList;

//-----------------------------------------------------------------------------
// Main class for LTRealVideoMgr
//-----------------------------------------------------------------------------
class CLTRealVideoMgr : public ILTRealVideoMgr
{
public:

	////////////////////////////////
	// external functions

	// default constructor
	CLTRealVideoMgr();

	// default destructor (calls Term if it has not been called)
	~CLTRealVideoMgr();

	// Initialize the Mgr 
	virtual LTRESULT Init();

	// Terminate the Mgr
	virtual LTRESULT Term();

	virtual LTRESULT			IsRealPlayerInstalled(bool* bIsInstalled);
	virtual LTRESULT			IsRealPlayerPlugInInstalled(bool* bIsInstalled);

	virtual ILTRealVideoPlayer*	CreatePlayer();
	virtual LTRESULT			DestroyPlayer(ILTRealVideoPlayer* pPlayer);

	virtual LTRESULT			Update(LTRV_UPDATE_MODE updateMode);

	virtual LTRESULT			AppFocus(bool bHasFocus);

protected:
	LTBOOL					m_bInitialized;
	VideoPlayerList			m_VideoPlayerList;
};

//-----------------------------------------------------------------------------
class CLTSurfaceNode : public CLithBaseListItem<CLTSurfaceNode>
{
	friend class CLTRealVideoSurface;
public:
	CLTSurfaceNode() { m_pSurface = LTNULL; m_pOriginalTexture = LTNULL; m_hPoly = INVALID_HPOLY; }
	~CLTSurfaceNode();

	Surface*		m_pSurface;
	SharedTexture*	m_pOriginalTexture;
	HPOLY			m_hPoly;
};

typedef CLithBaseList<CLTSurfaceNode> SurfaceNodeList;

//-----------------------------------------------------------------------------
class CLTRealVideoSurfaceNode : public CLithBaseListItem<CLTRealVideoSurfaceNode>
{
	friend class CLTRealVideoPlayer;
public:
	CLTRealVideoSurfaceNode() { m_pVideoSurface = LTNULL; }
	~CLTRealVideoSurfaceNode();

	CLTRealVideoSurface*	m_pVideoSurface;
};

typedef CLithBaseList<CLTRealVideoSurfaceNode> RealVideoSurfaceNodeList;

//-----------------------------------------------------------------------------
class CLTRealVideoPlayer : public CLTRealPlayer, ILTRealVideoPlayer, CLithBaseListItem<CLTRealVideoPlayer>
{
	friend CLTRealVideoMgr;
	friend CLTRealVideoSurface;

public:
	// default constructor
	CLTRealVideoPlayer();

	// default destructor (calls Term if it has not been called)
	~CLTRealVideoPlayer();

	// Initialize the player
	virtual LTRESULT Init();

	// Terminate the player
	virtual LTRESULT Term();

	virtual LTRESULT	IsRealPlayerInstalled(bool* pbIsInstalled)		{ return CLTRealPlayer::IsRealPlayerInstalled(pbIsInstalled); }
	virtual LTRESULT	IsRealPlayerPlugInInstalled(bool* pbIsInstalled)	{ return CLTRealPlayer::IsRealPlayerPlugInInstalled(pbIsInstalled); }
	virtual LTRESULT	Open(const char* pSource)						{ return CLTRealPlayer::Open(pSource); }
	virtual LTRESULT	Play(bool bLoop = false)						{ return CLTRealPlayer::Play(bLoop); }
	virtual LTRESULT	Stop();
	virtual LTRESULT	IsDone(bool* pbIsDone)							{ return CLTRealPlayer::IsDone(pbIsDone); }
	virtual LTRESULT	IsLooping(bool* pbIsLooping)					{ return CLTRealPlayer::IsLooping(pbIsLooping); }
	virtual LTRESULT	Pause()											{ return CLTRealPlayer::Pause(); }
	virtual LTRESULT	Resume()										{ return CLTRealPlayer::Resume(); }
	virtual LTRESULT	SetVolume(uint32 ulVolume)						{ return CLTRealPlayer::SetVolume(ulVolume); }
	virtual LTRESULT	GetVolume(uint32* pulVolume)					{ return CLTRealPlayer::GetVolume(pulVolume); }
	virtual LTRESULT	SetPan(uint32 ulPan)							{ return CLTRealPlayer::SetPan(ulPan); }
	virtual LTRESULT	GetPan(uint32* pulPan)							{ return CLTRealPlayer::GetPan(pulPan); }
	virtual LTRESULT	SetAudioPacketPreBuffer(uint32 ulPacketCount)	{ return CLTRealPlayer::SetAudioPacketPreBuffer(ulPacketCount); }
	virtual LTRESULT	GetAudioPacketPreBuffer(uint32* pulPacketCount)	{ return CLTRealPlayer::GetAudioPacketPreBuffer(pulPacketCount); }
	virtual LTRESULT	AppFocus(bool bHasFocus)						{ return CLTRealPlayer::AppFocus(bHasFocus); }

	// Add overlay (2D playback)
	virtual LTRESULT AddOverlay(LTRect* pRect, DWORD dwFlags = 0);
	virtual LTRESULT AddOverlay(LTIntPt* pPoint, DWORD dwFlags = 0);
	virtual LTRESULT AddOverlay(DWORD dwFlags = 0);
	virtual LTRESULT RemoveOverlays();

	// Render Overlays
	virtual LTRESULT Render();

	// Add a poly handle (3D playback)
	virtual LTRESULT	AddPoly(HPOLY hPoly);
	virtual LTRESULT	RemovePolys();

	// Enable/disable rendering for 2D or 3D
	virtual LTRESULT Set2DRenderingEnabled(bool bEnabled = true)		{ m_b2DRenderingEnabled = bEnabled; return LT_OK; }
	virtual LTRESULT Set3DRenderingEnabled(bool bEnabled = true)		{ m_b3DRenderingEnabled = bEnabled; return LT_OK; }


	// Concrete functions
	virtual LTRESULT	Update(LTRV_UPDATE_MODE updateMode);

	virtual SurfaceNodeList*	GetSurfaceNodeList()	{ return &m_SurfaceNodeList; }

	virtual RealVideoSurfaceNodeList* GetRealVideoSurfaceNodeList() { return &m_RealVideoSurfaceNodeList; }

	// List of 2D video overlay objects
	virtual	RealVideoOverlayList* GetRealVideoOverlayList() { return &m_RealVideoOverlayList; }

protected:
	// List of 3D video surfaces
	SurfaceNodeList				m_SurfaceNodeList;
	RealVideoSurfaceNodeList	m_RealVideoSurfaceNodeList;

	// List of 2D video overlay objects
	RealVideoOverlayList	m_RealVideoOverlayList;

	LTBOOL					m_b2DRenderingEnabled;
	LTBOOL					m_b3DRenderingEnabled;
};
#endif // LTRealVideo_Impl_H
#endif // LITHTECH_ESD