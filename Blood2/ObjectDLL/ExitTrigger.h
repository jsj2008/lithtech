// ----------------------------------------------------------------------- //
//
// MODULE  : ExitTrigger.h
//
// PURPOSE : ExitTrigger - Definition
//
// CREATED : 3/24/98
//
// ----------------------------------------------------------------------- //

#ifndef __EXITTRIGGER_H__
#define __EXITTRIGGER_H__

#include "Trigger.h"


class ExitTrigger : public Trigger
{
	public :

		ExitTrigger();
		~ExitTrigger();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		virtual void SendMessage(int nSlot) { SendMessages(); }
		virtual void SendMessages();
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		
		DBOOL ReadProp(ObjectCreateStruct *pData);

		HSTRING m_hstrNextWorld;		// Name of the destination world
		HSTRING m_hstrStartPointName;	// Name of the start point in that world
		DBYTE	m_nExitType;			// Is the level the end of a scenario?
};

#endif // __EXITTRIGGER_H__