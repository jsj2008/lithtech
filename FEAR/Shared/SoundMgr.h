// ----------------------------------------------------------------------- //
//
// MODULE  : SoundMgr.h
//
// PURPOSE : SoundMgr definition - Controls sound
//
// CREATED : 2/08/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SOUND_MGR_H__
#define __SOUND_MGR_H__

#include "SoundTypes.h"
#include "SoundDB.h"
#include "ltbasedefs.h"

// #defines...

#define SMGR_DEFAULT_VOLUME		100
#define SMGR_INVALID_VOLUME		255
#define SMGR_INVALID_RADIUS		-1.0f

class CGameSoundMgr
{
	public :

        virtual bool Init();

		HLTSOUND    PlayDBSoundFromObject(HOBJECT hObject, HRECORD hSR, float fORadius=SMGR_INVALID_RADIUS, 
											SoundPriority ePriority=SOUNDPRIORITY_INVALID, uint32 dwFlags=0, 
											uint8 nVolume=SMGR_INVALID_VOLUME, float fPitchShift=1.0f, float fIRadius=SMGR_INVALID_RADIUS, 
											SoundClass eSoundClass = DEFAULT_SOUND_CLASS, int16 nMixChannel=PLAYSOUND_MIX_DEFAULT );
		HLTSOUND    PlayDBSoundFromPos( const LTVector &vPos, HRECORD hSR, float fORadius=SMGR_INVALID_RADIUS, 
											SoundPriority ePriority=SOUNDPRIORITY_INVALID, uint32 dwFlags=0, 
											uint8 nVolume=SMGR_INVALID_VOLUME, float fPitchShift=1.0f, float fIRadius=SMGR_INVALID_RADIUS, 
											SoundClass eSoundClass = DEFAULT_SOUND_CLASS, int16 nMixChannel=PLAYSOUND_MIX_DEFAULT );


		HLTSOUND    PlaySoundFromObject(HOBJECT hObject, char const* pName, char const* pAltName, float fORadius=SMGR_INVALID_RADIUS, 
											SoundPriority ePriority=SOUNDPRIORITY_MISC_LOW,	uint32 dwFlags=0, 
											uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f, float fIRadius=SMGR_INVALID_RADIUS, 
											SoundClass eSoundClass = DEFAULT_SOUND_CLASS,
											int16 nMixChannel = PLAYSOUND_MIX_DEFAULT, float fSoundSwitchRadius=0.0f,
											float fDopplerFactor = 1.0f);

		HLTSOUND    PlaySoundFromPos( const LTVector &vPos, char const* pName, char const* pAltName, float fORadius=SMGR_INVALID_RADIUS, 
											SoundPriority ePriority=SOUNDPRIORITY_MISC_LOW,	uint32 dwFlags=0, 
											uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f, float fIRadius=SMGR_INVALID_RADIUS, 
											SoundClass eSoundClass = DEFAULT_SOUND_CLASS,
											int16 nMixChannel = PLAYSOUND_MIX_DEFAULT, float fSoundSwitchRadius=0.0f,
											float fDopplerFactor = 1.0f );

		HLTSOUND    PlayDBSoundFromPosWithPath( const LTVector &vPos, const LTVector &vPathVelocity, HRECORD hSR, float fORadius=SMGR_INVALID_RADIUS, 
											SoundPriority ePriority=SOUNDPRIORITY_INVALID, uint32 dwFlags=0, 
											uint8 nVolume=SMGR_INVALID_VOLUME, float fPitchShift=1.0f, float fIRadius=SMGR_INVALID_RADIUS, 
											SoundClass eSoundClass = DEFAULT_SOUND_CLASS, int16 nMixChannel=PLAYSOUND_MIX_DEFAULT);

		HLTSOUND    PlaySoundFromPosWithPath( const LTVector &vPos, const LTVector &vPathVelocity, char const* pName, char const* pAltName, float fORadius=SMGR_INVALID_RADIUS, 
											SoundPriority ePriority=SOUNDPRIORITY_MISC_LOW,	uint32 dwFlags=0, 
											uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f, float fIRadius=SMGR_INVALID_RADIUS, 
											SoundClass eSoundClass = DEFAULT_SOUND_CLASS,
											int16 nMixChannel = PLAYSOUND_MIX_DEFAULT, float fSoundSwitchRadius=0.0f,
											float fDopplerFactor = 1.0f );

		const char*		GetSoundFilenameFromId( const char* pPathAttribute, const char* szID );
		const char*		GetSoundIdFromFilename( const char* szSoundFile );

	protected :

        CGameSoundMgr();
        virtual ~CGameSoundMgr();

		virtual	HLTSOUND PlaySound(PlaySoundInfo & playSoundInfo) = 0;
		virtual	LTVector GetObjectPos(HOBJECT hObj) = 0;

        float		m_fInnerRadiusPercent;  // Percent of outer radius used for inner radius
};

#endif // __SOUND_MGR_H__
