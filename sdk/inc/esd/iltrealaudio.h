/****************************************************************************
;
;	MODULE:		ILTRealAudio (.H)
;
;	PURPOSE:	Define types and classes that need to be exposed to the game 
;				for the	Lithtech implementation of RealAudio.
;
;	HISTORY:	3-20-2000 [mds]
;
;	NOTICE:		Copyright 2000 LithTech, Inc.
;
***************************************************************************/

#ifdef LITHTECH_ESD
#ifndef ILTRealAudio_H
#define ILTRealAudio_H

#include "ltbasetypes.h"

class ILTRealAudioPlayer;


//-----------------------------------------------------------------------------
/*!
	ILTRealAudioMgr interface.  Use this manager to create and destroy
	RealAudio player objects.  To access this object, see the
	GetRealAudioMgr() member function in the ILTClient interface.
*/
class ILTRealAudioMgr
{
public:

/*!
\return an LTRESULT indicating success or failure

Initializes the RealAudio manager.  When using the ILTClient interface
to access this object, this function is called automatically.

Used for: ESD.
*/
	virtual LTRESULT	Init() = 0;

/*!
\return an LTRESULT indicating success or failure

Terminate the RealAudio manager.  When using the ILTClient interface
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
\return an the address of an ILTRealAudioPlayer object

Create RealPlayer player object.

Used for: ESD.
*/
	virtual ILTRealAudioPlayer*	CreatePlayer() = 0;

/*!
\param pPlayer The address of an ILTRealAudioPlayer to be destroyed
\return an LTRESULT indicating success or failure

Destroy RealPlayer player object.

Used for: ESD.
*/
	virtual LTRESULT	DestroyPlayer(ILTRealAudioPlayer* pPlayer) = 0;
};


//-----------------------------------------------------------------------------
/*!
	ILTRealAudioPlayer interface.  Use this object to play RealAudio audio
	files.
*/
class ILTRealAudioPlayer
{
public:

/*!
\return an LTRESULT indicating success or failure

Initializes the RealAudio player.

Used for: ESD.
*/
	virtual LTRESULT	Init() = 0;

/*!
\return an LTRESULT indicating success or failure

Terminate the RealAudio player.

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
\param pSource The URL or local file name of the audio file to open
\return an LTRESULT indicating success or failure

Open a URL or local file for playback.

Used for: ESD.
*/
	virtual LTRESULT	Open(const char* pSource) = 0;

/*!
\param bLoop Flag indicating that the audio should loop upon completion
\return an LTRESULT indicating success or failure

Play the current file.

Used for: ESD.
*/
	virtual LTRESULT	Play(bool bLoop = false) = 0;
	
/*!
\return an LTRESULT indicating success or failure

Stop playback of the current file.

Used for: ESD.
*/
	virtual LTRESULT	Stop() = 0;

/*!
\param pbIsDone bool to determine if playback has finished
\return an LTRESULT indicating success or failure

Determine if RealPlayer is finished playing.

Used for: ESD.
*/
	virtual LTRESULT	IsDone(bool* pbIsDone) = 0;

/*!
\param pbIsLooping bool to determine if playback is looping
\return an LTRESULT indicating success or failure

Determine if RealPlayer is looping.

Used for: ESD.
*/
	virtual LTRESULT	IsLooping(bool* pbIsLooping) = 0;

/*!
\return an LTRESULT indicating success or failure

Pause the current file playback.

Used for: ESD.
*/
	virtual LTRESULT	Pause() = 0;

/*!
\return an LTRESULT indicating success or failure

Resume the current file playback.

Used for: ESD.
*/
	virtual LTRESULT	Resume() = 0;

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

#endif //! ILTRealAudio_H
#endif // LITHTECH_ESD