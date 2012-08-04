// ----------------------------------------------------------------------- //
//
// MODULE  : ServerSoundMgr.h
//
// PURPOSE : ServerSoundMgr definition - Controls sound on the server
//
// CREATED : 7/10/00
//
// (c) 2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __SERVER_SOUND_MGR_H__
#define __SERVER_SOUND_MGR_H__

#include "SoundMgr.h"

class CServerSoundMgr;
extern CServerSoundMgr* g_pServerSoundMgr;

#define SSNDMGR_DEFAULT_FILE		"Attributes\\ServerSnd.txt"

class CServerSoundMgr : public CGameSoundMgr
{
	public :

		CServerSoundMgr();
		~CServerSoundMgr();

        LTBOOL      Init(ILTCSBase *pInterface, const char* szAttributeFile=SSNDMGR_DEFAULT_FILE);
		void		Term();

		void		CacheSounds(const char* pTag, const char* pAttributeBase);
		HLTSOUND	PlaySoundDirect(PlaySoundInfo & playSoundInfo);

	protected :

		virtual	LTVector GetObjectPos(HOBJECT hObj)
		{
			LTVector vPos(0, 0, 0);
			if (hObj)
			{
				g_pLTServer->GetObjectPos(hObj, &vPos);
			}

			return vPos;
		}

		virtual	HLTSOUND PlaySound(PlaySoundInfo & playSoundInfo);

};

#endif // __SERVER_SOUND_MGR_H__