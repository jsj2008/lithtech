// ----------------------------------------------------------------------- //
//
// MODULE  : SteamFX.h
//
// PURPOSE : Steam special fx class - Definition
//
// CREATED : 10/19/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __STEAM_FX_H__
#define __STEAM_FX_H__

#include "SpecialFX.h"
#include "SmokeFX.h"
#include "SharedFXStructs.h"

class CSteamFX : public CSpecialFX
{
	public :

		CSteamFX();
		~CSteamFX()
		{
			if (m_hSound)
			{
                g_pLTClient->KillSoundLoop(m_hSound);
			}

			if (m_cs.hstrSoundName)
			{
                g_pLTClient->FreeString(m_cs.hstrSoundName);
			}

			if (m_cs.hstrParticle)
			{
                g_pLTClient->FreeString(m_cs.hstrParticle);
			}
		}

        virtual LTBOOL Init(HLOCALOBJ hServObj, HMESSAGEREAD hRead);
        virtual LTBOOL Init(SFXCREATESTRUCT* psfxCreateStruct);
        virtual LTBOOL Update();
        virtual LTBOOL CreateObject(ILTClient* pClientDE);

		virtual uint32 GetSFXID() { return SFX_STEAM_ID; }

	protected :

		// Creation data...

		STEAMCREATESTRUCT	m_cs;		// Holds all initialization data

		CSmokeFX			m_Steam;
        HLTSOUND            m_hSound;
        uint32              m_dwLastUserFlags;

		void TweakSystem();
		void StartSound();
		void StopSound();
};

#endif // __STEAM_FX_H__