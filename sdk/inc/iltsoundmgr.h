/*!  This interface implements the sound manager functionality.  */

#ifndef __ILTSOUNDMGR_H__
#define __ILTSOUNDMGR_H__

#ifndef __LTMODULE_H__
#include "ltmodule.h"
#endif


// uncomment one of these for filter support
// #define USE_DX8_SOFTWARE_FILTERS
#ifdef _M_IX86
#ifndef USE_EAX20_HARDWARE_FILTERS
#define USE_EAX20_HARDWARE_FILTERS
#endif
#endif

/*!  Base sound mgr interface.  (shared between client and server) */

class ILTSoundMgr : public IBase {
public:
    interface_version(ILTSoundMgr, 0);

/*!
\param  pPlaySoundInfo  Sound control structure.
\param  hResult         (return) Newly created sound handle.

\return \b LT_INVALIDPARAMS when \b pPlaySoundInfo is invalid,
\b LT_MISSINGFILE when the sound file was not found, or \b LT_ERROR when
unable to play the sound; otherwise, returns \b LT_OK.

Play a sound with full control.

Specifying the \b PLAYSOUND_GETHANDLE flag in \b pPlaySoundInfo will fill
pPlaySoundInfo->m_hSound with the handle to the client sound, and
prevent the sound from being automatically deleted when the sound is
done playing. If it is specified, you should subsequently call
KillSound().

\see KillSound()

Used for: Audio.  
*/
    virtual LTRESULT PlaySound(PlaySoundInfo *pPlaySoundInfo, 
        HLTSOUND &hResult) = 0;

/*!
\param  hSound      Handle to sound.
\param  fDuration   (return) Duration of sound.

\return \b LT_INVALIDPARAMS when \b hSound is not available;
otherwise, returns \b LT_OK.

Get total length (in seconds) of sound.

Used for: Audio.  
*/
    virtual LTRESULT GetSoundDuration(HLTSOUND hSound, 
        LTFLOAT &fDuration) = 0;

/*!
\param  hSound      Handle to client only sound.
\param  bDone       (return) Finished state of sound. 
A return value of \b TRUE means the sound is done;
a return value of \b FALSE means it is still playing.

\return \b LT_INVALIDPARAMS when \b hSound is invalid; otherwise,
returns \b LT_OK.

Check whether the sound is finished playing or if the object it was
attached to was removed.

Used for: Audio.  
*/
    virtual LTRESULT IsSoundDone(HLTSOUND hSound, bool &bDone) = 0;

/*!
\param  hSound      Handle to sound.
\return \b LT_INVALIDPARAMS when \b hSound is invalid or not playing;
otherwise, returns \b LT_OK.

Kill a sound.

Used for: Audio.
*/
    virtual LTRESULT KillSound(HLTSOUND hSound) = 0;

/*!
\param  hSound      Handle to sound.
\return \b LT_INVALIDPARAMS when \b hSound is invalid or not playing;
otherwise, returns \b LT_OK.

Kill a looping sound.  The sound will continue to play until it reaches
the end, then remove itself.

Used for: Audio.
*/
    virtual LTRESULT KillSoundLoop(HLTSOUND hSound) = 0;
};


struct InitSoundInfo {
/*!  
A string containing the name of the 3D provider. The sound engine will render 
all 3D sounds through this 3D provider, if it is available. To have no 3D provider,
set this parameter to an empty string. If no 3D provider is available for a sound 
or none is specified, then 3D sounds will be simulated in software with volume 
fading and panning.  The list of providers available can be obtained by calling 
\b ILTClientSoundMgr::GetSound3DproviderLists().  
*/
    char m_sz3DProvider[_MAX_PATH+1];

/*!  
The maximum number of non-3D provider voices that will be rendered at one time. 
If the number of active sounds exceeds this number, the extra sounds will still 
be tracked. If a rendered sounds stops, then any waiting sounds will begin 
rendering from the correct offset in time within the sound.  
*/
    uint16 m_nNumSWVoices;

/*!  
The maximum number of 3D provider voices that will be rendered at one time. A 
3D provider voice is a 3D sound or a sound that has certain enhancements that 
are handled by a 3D provider. A good example of an enhancement is hardware 
reverb. If this number is exceeded, the new voices will be played as non-3D 
provider voices, which means that the \b m_nNumSWVoices must also be checked.  
*/
    uint16 m_nNum3DVoices;

/*!  
The output sample rate (in hertz) for the primary sound buffer. This value can 
be 8000, 11025, 22050 or 44100.  
*/
    unsigned short m_nSampleRate;

/*!  
The output bit depth for the primary sound buffer. This value can be 8 or 16.  
*/
    unsigned short m_nBitsPerSample;

/*!  
A combination of any of the \b INITSOUNDINFOFLAG_xxx flags. 
*/
    unsigned long m_dwFlags;

/*!  
The sound engine will fill in this parameter inside the \b ILTClientSoundMgr::InitSound()
call. It can be a combination of any of the INITSOUNDINFORESULTS_xxx values. 
*/
    unsigned long m_dwResults;

/*!  
Initial output volume. This can be a number between 0 and 100. 
*/
    unsigned short m_nVolume;

/*!  
This specifies a conversion factor for the sound engine to understand what each 
game unit represents in meters. Pass a number that is units of meters/game_unit. 
For instance, if 1 game unit equals 0.5 meters, then fill this parameter with 
0.5/1.0 or just 0.5. 
*/
    float m_fDistanceFactor;

/*!
This specifies the initial Doppler factor to use
Ranges from 0 (no Doppler effect) to 10 (10 times real-world Doppler)
*/
	float m_fDopplerFactor;

/*!
Initialize the \b InitSoundInfo structure to the default values.

Used for:   Audio.
*/
    inline void Init() {
        m_sz3DProvider[0] = 0;
        m_nNumSWVoices = 32;
        m_nNum3DVoices = 0;
        m_nSampleRate = 22050;
        m_nBitsPerSample = 16;
        m_dwFlags = 0;
        m_nVolume = 100;
        m_fDistanceFactor = 1.0f;
		m_fDopplerFactor = 1.0f;
    }
};



/*!  
Client-specific sound mgr interface.  
*/

class ILTClientSoundMgr : public ILTSoundMgr {
public:
    interface_version_derived(ILTClientSoundMgr, ILTSoundMgr, 0);

/*!
\param pSound3DProviderList (return) The list of available 3D sound
providers.

\param bVerify Tell the engine not to report providers that aren't
completely supported on the system.  This takes longer and causes
speaker popping.

\param uiMax3DVoices Tell the engine to check to only report back
providers that can support the maximum voices required.

                                
\return \b LT_OK.

Get a list of the 3D sound providers available on the system, to be used
in the \b InitSoundInfo structure.  Games should only need to use \b bVerify
when a different provider is chosen.  EAX support can only be checked
when the provider is opened, so without the \b bVerify, the
\b SOUND3DPROVIDER_CAPS_REVERB cannot be set.  InitSound will report this
info in the \b m_dwResults flags.  

\note Be sure to release the list with ReleaseSound3DProviderList().

\see ReleaseSound3DProviderList()

Used for: Audio.  
*/
    virtual LTRESULT GetSound3DProviderLists(
        Sound3DProvider *&pSound3DProviderList, bool bVerify, uint32 uiMax3DVoices ) = 0;

/*!
\param  pSound3DProviderList    The 3D sound provider list to be released.
\return LT_OK Successful.
\return LT_INVALIDPARAMS    An invalid provider list was provided.

This releases a list of 3D sound providers generated by
GetSound3DProviderList().

\see GetSound3DProviderList()

Used for: Audio.  
*/
    virtual LTRESULT ReleaseSound3DProviderList(
        Sound3DProvider *pSound3DProviderList) = 0;

/*!
\param  pSoundInfo      The initialization parameters.
\return \b LT_INVALIDPARAMS when \b pSoundInfo is invalid, \b LT_ERROR
when unable to initialize the sound driver; otherwise, return \b
LT_OK.

Initialize the sound driver.

Used for: Audio.  
*/
    virtual LTRESULT InitSound(InitSoundInfo *pSoundInfo) = 0;

/*!
\param fDoppler		Current Doppler factor (range: 0 (off) to 10)
\return \b LT_OK

Set the global Doppler effect (only used when 3D hardware sounds are
played.  Doppler factor is a multiple of real world Doppler effect,
so 2 would twice the Doppler effect that would occur in real life.

Used for: Audio.
*/
	virtual LTRESULT SetListenerDoppler(float fVolume) = 0;

/*!
\param  nVolume     (return) Current volume level (range: 0 to 100).
\return \b LT_OK

Retrieve the global sound volume.

Used for: Audio.
*/
    virtual LTRESULT GetVolume(uint16 &nVolume) = 0;

/*!
\param  nVolume     Desired volume level (range: 0 to 100).
\return \b LT_OK

Set the global sound volume.

Used for: Audio.
*/
    virtual LTRESULT SetVolume(uint16 nVolume) = 0;

/*!
\param  
\return \b LT_OK

Tells the sound manager to update any current sounds after
volume changes are made.

Used for: Audio.
*/
    virtual LTRESULT UpdateVolumeSettings() = 0;

/*!
\param  pReverbProperties       The desired reverb properties
\return \b LT_OK

Set the reverb properties.

Used for: Audio.
*/
    virtual LTRESULT SetReverbProperties(
        ReverbProperties *pReverbProperties) = 0;

/*!
\param  pReverbProperties       (return) Current reverb properties.
\return \b LT_OK

Retrieve the current reverb properties.

Used for: Audio.
*/
    virtual LTRESULT GetReverbProperties(
        ReverbProperties *pReverbProperties) = 0;

/*!
\param  hSound      Handle to the client only sound.
\param  fLevel      Level of occlusion.
\return \b LT_OK.

Set the sound occlusion level.

Used for: Audio.
*/
    virtual LTRESULT SetSoundOcclusion(HLTSOUND hSound, 
        LTFLOAT fLevel) = 0;

/*!
\param  hSound      Handle to the client only sound.
\param  pLevel      Variable to return the level of occlusion into.
\return \b LT_OK.

Retrieve the occlusion level of a sound.

Used for: Audio.
*/
    virtual LTRESULT GetSoundOcclusion(HLTSOUND hSound, 
        LTFLOAT *pLevel) = 0;

/*!
\param  hSound      Handle to the client only sound.
\param  fLevel      Level of obstruction.
\return \b LT_OK.

Set the sound obstruction level.

Used for: Audio.
*/
    virtual LTRESULT SetSoundObstruction(HLTSOUND hSound, 
        LTFLOAT fLevel) = 0;

/*!
\param  hSound      Handle to the client only sound.
\param  pLevel      Variable to return the level of obstruction into.
\return \b LT_OK.

Retrieve the occlusion level of a sound.

Used for: Audio.
*/
    virtual LTRESULT GetSoundObstruction(HLTSOUND hSound, 
        LTFLOAT *pLevel) = 0;

#ifdef USE_DX8_SOFTWARE_FILTERS
/*!
\param  hSound      Handle to the sound.
\param  pFilter     Name of the filter.

\return \b LT_INVALIDPARAMS when \b hSound is invalid or does not support
filters, \b LT_NOTFOUND when an unknown filter is requested; otherwise,
returns \b LT_OK.

Set a sound's filter

Used for: Audio.  
*/
    virtual LTRESULT SetSoundFilter(HLTSOUND hSound, 
        const char *pFilter) = 0;

/*!
\param  hSound      Handle to the sound.
\param  pParam      Name of the parameter.
\param  fValue      Value of the parameter.
\return \b LT_ERROR when \b hSound is invalid or does not support
filters, \b LT_NOTFOUND when an unknown parameter is requested;
otherwise, returns \b LT_OK.

Set a parameter of a filter.

Used for: Audio.  
*/
    virtual LTRESULT SetSoundFilterParam(HLTSOUND hSound, 
        const char *pParam, float fValue) = 0;

#endif

#ifdef USE_EAX20_HARDWARE_FILTERS

/*!
\param  pFilter     Name of the filter.

returns \b LT_NOTFOUND if an unknown filter is requested; otherwise,
\b LT_OK.

Set a current sound engine filter

Used for: Audio.  
*/
    virtual LTRESULT SetSoundFilter(const char *pFilter) = 0;

/*!
\param  pParam      Name of the parameter.
\param  fValue      Value of the parameter.
\return \b LT_ERROR when engine does not support filters, 
\b LT_NOTFOUND when an unknown parameter is requested;
otherwise, returns \b LT_OK.

Set a parameter of a filter.

Used for: Audio.  
*/
    virtual LTRESULT SetSoundFilterParam(const char *pParam, float fValue) = 0;

/*!
\param  bEnable    Enable or disable current filter in engine
\return \b LT_ERROR when engine does not support filters, 
otherwise, returns \b LT_OK.

Turn on or off the current filter

Used for: Audio.  
*/
    virtual LTRESULT EnableSoundFilter( bool bEnable ) = 0;

#endif

/*!
\param  hSound      Handle to sound.
\param  pPos        New position of sound. Can be \b NULL.

\return \b LT_INVALIDPARAMS when there are invalid parameters, or \b
LT_ERROR when unable to find \b hSound; otherwise, returns \b LT_OK.

Set position and orientation of a sound.

Used for: Audio.  
*/
    virtual LTRESULT SetSoundPosition(HLTSOUND hSound, 
        LTVector *pPos) = 0;

/*!
\param  hSound  Handle to sound.
\param  pPos    (return) Position to retrieve.

\return \b LT_INVALIDPARAMS when there are invalid parameters, or \b
LT_ERROR when unable to find \b hSound; otherwise, returns \b LT_OK.

Retrieve current position and orientation of a sound.

Used for: Audio.  
*/
    virtual LTRESULT GetSoundPosition(HLTSOUND hSound, 
        LTVector *pPos) = 0;

/*!
\return \b LT_ERROR when unable to pause; otherwise, returns \b LT_OK.

Pause sounds.

Used for: Audio.  
*/
    virtual LTRESULT PauseSounds() = 0;

/*!
\return \b LT_ERROR when unable to resume; otherwise, returns \b LT_OK.

Resume sounds.

Used for: Audio.
*/
    virtual LTRESULT ResumeSounds() = 0;

/*!
\return \b LT_ERROR if invalid sound class or multiplier LT_OK otherwise

This function allows different sound class categories to be created.  Game can
create up to MAX_SOUND_CLASSES types.  Each sound class can have it's own multiplier
that is relative to the current maximum sound volume (if bUseGlobalVolume is set to
true).  0 is the default sound class, and results in no multiplier.

If bUseGlobalVolume is set to false, the sound class multiplier is used as the sounds
volume level allowing you to control sounds independent of the global volume setting.

Used for: Audio
*/
	virtual LTRESULT SetSoundClassMultiplier( uint8 nSoundClass, float fMult, bool bUseGlobalVolume=true ) = 0;

/*!
\return \b LT_ERROR if invalid sound type, LT_OK otherwise

Used for: Audio
*/
	virtual LTRESULT GetSoundClassMultiplier( uint8 nSoundClass, float* pfMult ) = 0;

	
/*!
\param  bListenerInClient   If \b TRUE, then \b pPos and \b pRot are ignored and 
                            can be set to \b NULL.
\param  pPos                Position of listener.
\param  pRot                Rotation of listener.
\param  bTeleport           Consider the movement a teleportation (don't 
                            apply velocity).
    
\return \b LT_OK.

Set the listener status, position, and orientation.  

Used for: Audio.
*/
 
		virtual LTRESULT	SetListener( bool bListenerInClient, 
			LTVector *pPos, LTRotation *pRot, bool bTeleport ) = 0;
		
	};



#endif  // __ILTSOUNDMGR_H__
