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

        LTBOOL      Init(const char* szAttributeFile=SSNDMGR_DEFAULT_FILE);
		void		Term();

		HLTSOUND	PlaySoundDirect(PlaySoundInfo & playSoundInfo);

		void GetParentSoundTemplate(char* szParentTemplate, int nStrSize, const char* szSoundTemplate);

		// [KLS 6/24/02] - Added this function as a way to format the attribute
		// file correctly.  This is ONLY used during development to assist the
		// content creators in formatting the file correctly.  It takes advantage
		// of a bunch of game-specific knowledge and really only needs to be called
		// once to set up the attribute file.
#ifdef _DEBUG
		void		FormatAttributeFile();
#endif

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