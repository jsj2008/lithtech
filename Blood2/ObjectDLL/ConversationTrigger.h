// ----------------------------------------------------------------------- //
//
// MODULE  : ConversationTrigger.h
//
// PURPOSE : ConversationTrigger - Definition
//
// CREATED : 5/26/98
//
// ----------------------------------------------------------------------- //

#ifndef __CONVERSATIONTRIGGER_H__
#define __CONVERSATIONTRIGGER_H__

#define CT_MAXMESSAGES	8

#include "Trigger.h"

class ConversationTrigger : public Trigger
{
	public :

		ConversationTrigger();
		~ConversationTrigger();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		virtual void SendMessage(int nSlot) { SendMessages(); }
		virtual void SendMessages();
		
		DBOOL	ReadProp(ObjectCreateStruct *pData);
		void	CacheFiles();
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		HSTRING m_hstrSoundFile[CT_MAXMESSAGES];	// Name of the WAV file to play
		DBYTE	m_nCharacterType;					// Which character is it? Defined in SharedDefs.h
		DBYTE	m_nNumMessages;						// Which character is it? Defined in SharedDefs.h
		DBYTE	m_nCurrentMessage;					// The current message
		DBOOL	m_bRepeatLastMessage;				// Repeat last message if continually triggered
		DBOOL	m_bLoopMessages;					// Repeat last message if continually triggered
};

#endif // __CONVERSATIONTRIGGER_H__