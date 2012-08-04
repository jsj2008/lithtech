// ----------------------------------------------------------------------- //
//
// MODULE  : ExitTrigger.h
//
// PURPOSE : ExitTrigger - Definition
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __EXIT_TRIGGER_H__
#define __EXIT_TRIGGER_H__

#include "Trigger.h"

class ExitTrigger : public Trigger
{
	public :

		ExitTrigger();
		~ExitTrigger();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		virtual void ObjectTouch(HOBJECT hObj);
		virtual void Activate();
		void	Update();
		DBOOL	CanExit( );
		void	DoExit( );
		
		DBOOL ReadProp(ObjectCreateStruct *pData);
		void PostPropRead(ObjectCreateStruct *pData);
		void SendClientsExitMsg();

		HSTRING m_hstrDestinationWorld;	// Name of the destination world
		HSTRING m_hstrStartPointName;	// Name of the start point in that world
		HSTRING m_hstrBumperScreen;		// Name of the bumper screen
		DFLOAT	m_nBumperTextID;		// ID of text string to display
		DBOOL	m_bEndOfScenario;		// Is the level the end of a scenario?

		DBOOL	m_bWaitingForDialogue;	// Make sure there is no dialogue still playing or queued.
		float	m_fMaxWaitTime;			// Maximum amount of time before exit happens regardless of dialog.
};

#endif // __EXIT_TRIGGER_H__