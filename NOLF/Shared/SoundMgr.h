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

        virtual LTBOOL Init(ILTCSBase *pInterface, const char* szAttributeFile);

        HLTSOUND    PlaySoundFromObject(HOBJECT hObject, char *pName, LTFLOAT fRadius,
            SoundPriority ePriority, uint32 dwFlags=0, uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f);

        HLTSOUND    PlaySoundFromPos(LTVector & vPos, char *pName, LTFLOAT fRadius,
            SoundPriority ePriority, uint32 dwFlags=0, uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f);

		CString		GetRandomSoundFilename(const char* pTag, const char* pAttributeBase);
		CString		GetSoundFilename(const char* pTag, const char* pAttribute);
        CString     GetSoundFilenameFromId(const char* pPathAttribute, uint32 dwId);
		uint32		GetSoundIdFromFilename(const char* pSoundFile);

	protected :

        CGameSoundMgr();
        virtual ~CGameSoundMgr();

		virtual	HLTSOUND PlaySound(PlaySoundInfo & playSoundInfo) = 0;
		virtual	LTVector GetObjectPos(HOBJECT hObj) = 0;

		ILTCSBase*	m_pInterface;
        LTFLOAT		m_fInnerRadiusPercent;  // Percent of outer radius used for inner radius
};

#endif // __SOUND_MGR_H__