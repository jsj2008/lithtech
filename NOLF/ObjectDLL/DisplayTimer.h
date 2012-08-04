// ----------------------------------------------------------------------- //
//
// MODULE  : DisplayTimer.h
//
// PURPOSE : DisplayTimer - Definition
//
// CREATED : 10/15/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __DISPLAY_TIMER_H__
#define __DISPLAY_TIMER_H__

#include "ltengineobjects.h"
#include "Timer.h"

class DisplayTimer : public BaseClass
{
	public :

		DisplayTimer();
		~DisplayTimer();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private :

		HSTRING	m_hstrStartCmd;		// Command to send when timer starts
		HSTRING	m_hstrEndCmd;		// Command to send when timer ends
        LTBOOL   m_bRemoveWhenDone;  // Remove the timer when done?

		CTimer	m_Timer;			// Timer

		void	ReadProp(ObjectCreateStruct *pData);
		void	InitialUpdate();
		void	Update();

		void	TriggerMsg(HOBJECT hSender, const char *pMsg);

        void    HandleStart(LTFLOAT fTime);
        void    HandleAdd(LTFLOAT fTime);
		void	HandlePause();
		void	HandleResume();
		void	HandleEnd();
		void	HandleAbort();
		void	UpdateClients();

		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
};

#endif // __DISPLAY_TIMER_H__