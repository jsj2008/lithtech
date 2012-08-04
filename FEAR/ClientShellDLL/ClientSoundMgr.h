// ----------------------------------------------------------------------- //
//
// MODULE  : ClientSoundMgr.h
//
// PURPOSE : ClientSoundMgr definition - Controls sound on the client
//
// CREATED : 7/10/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CLIENT_SOUND_MGR_H__
#define __CLIENT_SOUND_MGR_H__

#include "SoundMgr.h"

class CClientSoundMgr;
extern CClientSoundMgr* g_pClientSoundMgr;

class CClientSoundMgr : public CGameSoundMgr
{
	public :

		CClientSoundMgr();
		~CClientSoundMgr();

        virtual bool	Init();
		virtual void	Term();

		HLTSOUND	PlayDBSoundLocal(HRECORD hSR, SoundPriority ePriority=SOUNDPRIORITY_INVALID,
			uint32 dwFlags=0, uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f, 
			SoundClass eSoundClass = DEFAULT_SOUND_CLASS, int16 nMixChannel = PLAYSOUND_MIX_DEFAULT );
		HLTSOUND	PlaySoundLocal(const char *pName, SoundPriority ePriority=SOUNDPRIORITY_MISC_LOW,
			uint32 dwFlags=0, uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f, 
			SoundClass eSoundClass = DEFAULT_SOUND_CLASS,
			int16 nMixChannel=PLAYSOUND_MIX_DEFAULT );
 
		HLTSOUND	PlayInterfaceSound(const char *pName, uint32 dwFlags=0);
		HLTSOUND	PlayInterfaceDBSound(const char *pRecordName, uint32 dwFlags=0);

		virtual	LTVector GetObjectPos(HOBJECT hObj)
		{
			LTVector vPos(0, 0, 0);
			if (hObj)
			{
				g_pLTClient->GetObjectPos(hObj, &vPos);
			}

			return vPos;
		}

		virtual	HLTSOUND PlaySound(PlaySoundInfo & playSoundInfo);

};

#endif // __CLIENT_SOUND_MGR_H__