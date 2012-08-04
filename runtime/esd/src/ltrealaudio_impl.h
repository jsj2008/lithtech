/****************************************************************************
;
;	MODULE:		LTRealAudio_Impl (.H)
;
;	PURPOSE:	Implement RealAudio capability for LithTech engine
;
;	HISTORY:	3-20-2000 [mds] File created.
;
;	NOTICE:		Copyright (c) 2000 Lithtech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef LTRealAudio_Impl_H
#define LTRealAudio_Impl_H
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

#include "LTRClientContext.h"
#include "LTRAudioDevice.h"
#include "LTRAudioHook.h"
#include "LTRAudioInfoResponse.h"
#include "LTReal_impl.h"

class CLTRealAudioPlayer;
typedef CLithBaseList<CLTRealAudioPlayer> AudioPlayerList;

//-----------------------------------------------------------------------------
// Main class for LTRealAudioMgr
//-----------------------------------------------------------------------------
class CLTRealAudioMgr : public ILTRealAudioMgr
{
public:
	// Default constructor
	CLTRealAudioMgr();

	// Default destructor (calls Term if it has not been called)
	~CLTRealAudioMgr();

	// Initialize the Mgr 
	virtual LTRESULT Init();

	// Terminate the Mgr
	virtual LTRESULT Term();

	virtual LTRESULT			IsRealPlayerInstalled(bool* bIsInstalled);
	virtual LTRESULT			IsRealPlayerPlugInInstalled(bool* bIsInstalled);
	virtual ILTRealAudioPlayer*	CreatePlayer();
	virtual LTRESULT			DestroyPlayer(ILTRealAudioPlayer* pPlayer);

	virtual LTRESULT			Update();

	virtual LTRESULT			AppFocus(bool bHasFocus);

protected:
	LTBOOL					m_bInitialized;
	AudioPlayerList			m_AudioPlayerList;
};

//-----------------------------------------------------------------------------
// Main class for LTRealAudioPlayer
//-----------------------------------------------------------------------------
class CLTRealAudioPlayer : public CLTRealPlayer, ILTRealAudioPlayer, CLithBaseListItem<CLTRealAudioPlayer>
{
	friend CLTRealAudioMgr;
	friend CLTAudioHook;

public:
	// Default constructor
	CLTRealAudioPlayer();

	// Default destructor (calls Term if it has not been called)
	~CLTRealAudioPlayer();

	// Initialize the Mgr 
	virtual LTRESULT Init();

	// Terminate the Mgr
	virtual LTRESULT Term();

	virtual LTRESULT	IsRealPlayerInstalled(bool* pbIsInstalled)			{ return CLTRealPlayer::IsRealPlayerInstalled(pbIsInstalled); }
	virtual LTRESULT	IsRealPlayerPlugInInstalled(bool* pbIsInstalled)	{ return CLTRealPlayer::IsRealPlayerPlugInInstalled(pbIsInstalled); }
	virtual LTRESULT	Open(const char* pSource)							{ return CLTRealPlayer::Open(pSource); }
	virtual LTRESULT	Play(bool bLoop = false)							{ return CLTRealPlayer::Play(bLoop); }
	virtual LTRESULT	Stop()												{ return CLTRealPlayer::Stop(); }
	virtual LTRESULT	IsDone(bool* pbIsDone)								{ return CLTRealPlayer::IsDone(pbIsDone); }
	virtual LTRESULT	IsLooping(bool* pbIsLooping)						{ return CLTRealPlayer::IsLooping(pbIsLooping); }
	virtual LTRESULT	Pause()												{ return CLTRealPlayer::Pause(); }
	virtual LTRESULT	Resume()											{ return CLTRealPlayer::Resume(); }
	virtual LTRESULT	SetVolume(uint32 ulVolume)							{ return CLTRealPlayer::SetVolume(ulVolume); }
	virtual LTRESULT	GetVolume(uint32* pulVolume)						{ return CLTRealPlayer::GetVolume(pulVolume); }
	virtual LTRESULT	SetPan(uint32 ulPan)								{ return CLTRealPlayer::SetPan(ulPan); }
	virtual LTRESULT	GetPan(uint32* pulPan)								{ return CLTRealPlayer::GetPan(pulPan); }
	virtual LTRESULT	SetAudioPacketPreBuffer(uint32 ulPacketCount)		{ return CLTRealPlayer::SetAudioPacketPreBuffer(ulPacketCount); }
	virtual LTRESULT	GetAudioPacketPreBuffer(uint32* pulPacketCount)		{ return CLTRealPlayer::GetAudioPacketPreBuffer(pulPacketCount); }
	virtual LTRESULT	AppFocus(bool bHasFocus)							{ return CLTRealPlayer::AppFocus(bHasFocus); }

	virtual LTRESULT	Update();
};

#endif // LTRealAudio_Impl_H
#endif // LITHTECH_ESD