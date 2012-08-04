// ----------------------------------------------------------------------- //
//
// MODULE  : Steam.h
//
// PURPOSE : Steam class definition
//
// CREATED : 10/19/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __STEAM_H__
#define __STEAM_H__

#include "ltengineobjects.h"
#include "SharedFXStructs.h"
#include "Timer.h"
#include "GameBase.h"

LINKTO_MODULE( Steam );

class Steam : public GameBase
{
	public :

		Steam();
		~Steam();

        void Setup(STEAMCREATESTRUCT *pSC, LTFLOAT fLifetime=-1.0f,
            LTBOOL bStartActive=LTFALSE);

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

		void	Update();
		void	InitialUpdate();

		void	ReadProps();

        void    DoDamage(LTFLOAT fDamageAmount);

		void	CreateSFXMsg();
		void	TurnOn();
		void	TurnOff();

		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

        LTBOOL   m_bOn;          // Are we currently on?
        LTFLOAT  m_fDamage;      // How much damage (per second) the steam does

        LTFLOAT  m_fLifetime;    // How long steam stays around (-1 = forever)
        LTBOOL   m_bStartActive; // Should the steam start on?
		CTimer	 m_Timer;		 // Time until the steam turns off

		STEAMCREATESTRUCT m_SteamStruct;  // Holds all special fx steam info
};

#endif // __STEAM_H__