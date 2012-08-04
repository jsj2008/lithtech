// ----------------------------------------------------------------------- //
//
// MODULE  : ObjectivesTrigger.h
//
// PURPOSE : ObjectivesTrigger - Definition
//
// CREATED : 10/4/98
//
// ----------------------------------------------------------------------- //

#ifndef __OBJECTIVESTRIGGER_H__
#define __OBJECTIVESTRIGGER_H__

#include "Trigger.h"

class ObjectivesTrigger : public Trigger
{
	public :

		ObjectivesTrigger();
		~ObjectivesTrigger();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		virtual void SendMessage(int nSlot) { SendMessages(); }
		virtual void SendMessages();

		DBOOL	ReadProp(ObjectCreateStruct *pData);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		HSTRING m_hstrTitle;		// Title of the objective
		HSTRING m_hstrText;			// Text of the objective >>> Use '|' to force a line break
		HSTRING m_hstrSound;		// Sound to play when triggered
		long	m_nResource;		// Resource number
};

#endif // __OBJECTIVESTRIGGER_H__