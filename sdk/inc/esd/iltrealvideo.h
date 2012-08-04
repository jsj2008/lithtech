/****************************************************************************
;
;	MODULE:		ILTRealVideo (.H)
;
;	PURPOSE:	Define types and classes that need to be exposed to the game 
;				for the	Lithtech implementation of RealVideo.
;
;	HISTORY:	4-18-2000 [mds] File created.
;
;	NOTICE:		Copyright 2000 LithTech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef ILTRealVideo_H
#define ILTRealVideo_H

#include "ltbasetypes.h"

/*!
	RealVideo playback flags
*/
enum
{
    LTRV_FULLSCREEN = 0x00000001,
    LTRV_DOUBLESIZE = 0x00000002,
    LTRV_LEFTJUST   = 0x00000004,
    LTRV_RIGHTJUST  = 0x00000008,
    LTRV_TOPJUST    = 0x00000010,
    LTRV_BOTTOMJUST	= 0x00000020,
    LTRV_CENTERJUST	= 0x00000040
};

class ILTRealVideoPlayer;

//-----------------------------------------------------------------------------
/*!
	ILTRealVideoMgr interface.  Use this manager to create and destroy
	RealVideo player objects.  To access this object, see the
	GetRealVideoMgr() member function in the ILTClient interface.
*/
class ILTRealVideoMgr
{
public:

/*!
\return an LTRESULT indicating success or failure

Initializes the RealVideo manager.  When using the ILTClient interface
to access this object, this function is called automatically.

Used for: ESD.
*/
	virtual LTRESULT	Init() = 0;

/*!
\return an LTRESULT indicating success or failure

Terminate the RealVideo manager.  When using the ILTClient interface
to access this object, this function is called automatically.

Used for: ESD.
*/
	virtual LTRESULT	Term() = 0;

/*!
\param pbIsInstalled bool to determine if RealPlayer is installed
\return an LTRESULT indicating success or failure

Determine if RealPlayer is installed.

Used for: ESD.
*/
	virtual LTRESULT	IsRealPlayerInstalled(bool* pbIsInstalled) = 0;

/*!
\param pbIsInstalled bool to determine if the RealPlayer plugin is installed
\return an LTRESULT indicating success or failure

Determine if RealPlayer plugin is installed.

Used for: ESD.
*/
	virtual LTRESULT	IsRealPlayerPlugInInstalled(bool* pbIsInstalled) = 0;

/*!
\return an the address of an ILTRealVideoPlayer object

Create RealPlayer player object.

Used for: ESD.
*/
	virtual ILTRealVideoPlayer*	CreatePlayer() = 0;

/*!
\param pPlayer The address of an ILTRealVideoPlayer to be destroyed
\return an LTRESULT indicating success or failure

Destroy RealPlayer player object.

Used for: ESD.
*/
	virtual LTRESULT	DestroyPlayer(ILTRealVideoPlayer* pPlayer) = 0;
};


//-----------------------------------------------------------------------------
/*!
	ILTRealVideoPlayer interface.  Use this object to play RealVideo video
	files.
*/
class ILTRealVideoPlayer
{
public:

/*!
\return an LTRESULT indicating success or failure

Initializes the RealVideo player.

Used for: ESD.
*/
	virtual LTRESULT Init() = 0;

/*!
\return an LTRESULT indicating success or failure

Terminate the RealVideo player.

Used for: ESD.
*/
	virtual LTRESULT Term() = 0;

/*!
\param pbIsInstalled bool to determine if RealPlayer is installed
\return an LTRESULT indicating success or failure

Determine if RealPlayer is installed.

Used for: ESD.
*/
	virtual LTRESULT	IsRealPlayerInstalled(bool* pbIsInstalled) = 0;

/*!
\param pbIsInstalled bool to determine if the RealPlayer plugin is installed
\return an LTRESULT indicating success or failure

Determine if RealPlayer plugin is installed.

Used for: ESD.
*/
	virtual LTRESULT	IsRealPlayerPlugInInstalled(bool* pbIsInstalled) = 0;

/*!
\param pSource The URL or local file name of the video file to open
\return an LTRESULT indicating success or failure

Open a URL or local file for playback.

Used for: ESD.
*/
	virtual LTRESULT Open(const char* pSource) = 0;
	
/*!
\param bLoop Flag indicating that the video should loop upon completion
\return an LTRESULT indicating success or failure

Play the current file.

Used for: ESD.
*/
	virtual LTRESULT Play(bool bLoop = false) = 0;

/*!
\return an LTRESULT indicating success or failure

Stop playback of the current file.

Used for: ESD.
*/
	virtual LTRESULT Stop() = 0;

/*!
\param pbIsDone bool to determine if playback has finished
\return an LTRESULT indicating success or failure

Determine if RealPlayer is finished playing.

Used for: ESD.
*/
	virtual LTRESULT IsDone(bool* pbIsDone) = 0;

/*!
\param pbIsLooping bool to determine if playback is looping
\return an LTRESULT indicating success or failure

Determine if RealPlayer is looping.

Used for: ESD.
*/
	virtual LTRESULT IsLooping(bool* pbIsLooping) = 0;

/*!
\return an LTRESULT indicating success or failure

Pause the current file playback.

Used for: ESD.
*/
	virtual LTRESULT Pause() = 0;

/*!
\return an LTRESULT indicating success or failure

Resume the current file playback.

Used for: ESD.
*/
	virtual LTRESULT Resume() = 0;


//! 2D overlay functions...
/*!
\param pRect The address of an LTRect that defines the overlay region
\param dwFlags Playback flags for providing additional position information
\return an LTRESULT indicating success or failure

Add a two dimensional overlay to the screen.

Used for: ESD.
*/
	virtual LTRESULT AddOverlay(LTRect* pRect, DWORD dwFlags = 0) = 0;

/*!
\param pPoint The address of an LTIntPt that defines the overlay position
\param dwFlags Playback flags for providing additional position information
\return an LTRESULT indicating success or failure

Add a two dimensional overlay to the screen.

Used for: ESD.
*/
	virtual LTRESULT AddOverlay(LTIntPt* pPoint, DWORD dwFlags = 0) = 0;

/*!
\param dwFlags Playback flags for providing position information
\return an LTRESULT indicating success or failure

Add a two dimensional overlay to the screen.

Used for: ESD.
*/
	virtual LTRESULT AddOverlay(DWORD dwFlags = 0) = 0;

/*!
\return an LTRESULT indicating success or failure

Removes all two dimensional overlays from the screen.

Used for: ESD.
*/
	virtual LTRESULT RemoveOverlays() = 0;

/*!
\return an LTRESULT indicating success or failure

Renders all two dimensional overlays on the screen.  This function should be
called in the client shell update function just prior to flipping the screen.

Used for: ESD.
*/
	virtual LTRESULT Render() = 0;

//! 3D polygon functions...
/*!
\param hPoly Polygon handle used as the surface to render video
\return an LTRESULT indicating success or failure

Add a polygon handle to the player for rendering.  This function should be
called when loading a level.

Used for: ESD.
*/
	virtual LTRESULT AddPoly(HPOLY hPoly) = 0;

/*!
\return an LTRESULT indicating success or failure

Removes all polygon handles from the rendering list.  This function should be
called when unloading a level.

Used for: ESD.
*/
	virtual LTRESULT RemovePolys() = 0;

/*!
\param bEnabled bool to enable and disable two dimensional rendering
\return an LTRESULT indicating success or failure

Enables or disables two dimensional rendering.

Used for: ESD.
*/
	virtual LTRESULT Set2DRenderingEnabled(bool bEnabled = true) = 0;

/*!
\param bEnabled bool to enable and disable three dimensional rendering
\return an LTRESULT indicating success or failure

Enables or disables three dimensional rendering.

Used for: ESD.
*/
	virtual LTRESULT Set3DRenderingEnabled(bool bEnabled = true) = 0;

/*!
\return an LTRESULT indicating success or failure

Set player volume.

Used for: ESD.
*/
	virtual LTRESULT	SetVolume(uint32 ulVolume) = 0;

/*!
\return an LTRESULT indicating success or failure

Get player volume.

Used for: ESD.
*/
	virtual LTRESULT	GetVolume(uint32* pulVolume) = 0;

/*!
\return an LTRESULT indicating success or failure

Set player pan.

Used for: ESD.
*/
	virtual LTRESULT	SetPan(uint32 ulPan) = 0;

/*!
\return an LTRESULT indicating success or failure

Get player pan.

Used for: ESD.
*/
	virtual LTRESULT	GetPan(uint32* pulPan) = 0;

/*!
\return an LTRESULT indicating success or failure

Set the number of audio packets to be collected before playback begins.

Used for: ESD.
*/
	virtual LTRESULT	SetAudioPacketPreBuffer(uint32 ulPacketCount) = 0;

/*!
\return an LTRESULT indicating success or failure

Get the number of audio packets to be collected before playback begins.

Used for: ESD.
*/
	virtual LTRESULT	GetAudioPacketPreBuffer(uint32* pulPacketCount) = 0;
};

#endif // !ILTRealVideo_H
#endif // LITHTECH_ESD
