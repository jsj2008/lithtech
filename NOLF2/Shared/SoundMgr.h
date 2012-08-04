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
#include "GameButeMgr.h"
#include "ltbasedefs.h"

// #defines...

#define SMGR_DEFAULT_VOLUME		100

class CGameSoundMgr : public CGameButeMgr
{
	public :

        virtual LTBOOL Init(const char* szAttributeFile);

        HLTSOUND    PlaySoundFromObject(HOBJECT hObject, char const* pName, LTFLOAT fORadius=-1.0f, SoundPriority ePriority=SOUNDPRIORITY_MISC_LOW,
										uint32 dwFlags=0, uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f, LTFLOAT fIRadius=-1.0f, uint8 nSoundClass = 0 );

        HLTSOUND    PlaySoundFromPos(LTVector & vPos, char const* pName, LTFLOAT fORadius=-1.0f, SoundPriority ePriority=SOUNDPRIORITY_MISC_LOW,
									uint32 dwFlags=0, uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f, LTFLOAT fIRadius=-1.0f, uint8 nSoundClass = 0 );

		void		GetRandomSoundFilename(const char* pTag, const char* pAttributeBase, char *pBuf, uint16 nBufLen);
		void		GetSoundFilename(const char* pTag, const char* pAttribute, char *pBuf, uint16 nBufLen);
        void	    GetSoundFilenameFromId(const char* pPathAttribute, uint32 dwId, char *pBuf, uint16 nBufLen);
		uint32		GetSoundIdFromFilename(const char* pSoundFile);

	protected :

        CGameSoundMgr();
        virtual ~CGameSoundMgr();

		virtual	HLTSOUND PlaySound(PlaySoundInfo & playSoundInfo) = 0;
		virtual	LTVector GetObjectPos(HOBJECT hObj) = 0;

        LTFLOAT		m_fInnerRadiusPercent;  // Percent of outer radius used for inner radius
};

#endif // __SOUND_MGR_H__