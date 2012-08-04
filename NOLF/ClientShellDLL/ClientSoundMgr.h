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

#define CSNDMGR_DEFAULT_FILE		"Attributes\\ClientSnd.txt"

class CClientSoundMgr : public CGameSoundMgr
{
	public :

		CClientSoundMgr();
		~CClientSoundMgr();

        virtual LTBOOL	Init(ILTCSBase *pInterface, const char* szAttributeFile=CSNDMGR_DEFAULT_FILE);
		virtual void	Term();

        HLTSOUND	PlaySoundLocal(char *pName, SoundPriority ePriority,
            uint32 dwFlags=0, uint8 nVolume=SMGR_DEFAULT_VOLUME, float fPitchShift=1.0f);
 
		HLTSOUND	PlayInterfaceSound(char *pName, uint32 dwFlags=0);

	protected :

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