// ----------------------------------------------------------------------- //
//
// MODULE  : Cat.h
//
// PURPOSE : Cat object
//
// CREATED : 3/26/98
//
// ----------------------------------------------------------------------- //

#ifndef __CAT_H__
#define __CAT_H__

#include "InventoryItem.h"

class Cat : public InventoryItem
{
	public :

		Cat();
		~Cat();

	protected :

		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
		DDWORD	EngineMessageFn (DDWORD messageID, void *pData, DFLOAT lData);
		void	ReadProp(ObjectCreateStruct *pStruct);

	protected:

		HSTRING m_hstrSqueakyTarget;			// Object we trigger
		HSTRING m_hstrSqueakyMessage;			// Trigger message
		HSTRING m_hstrSqueakedAtSound;			// Sound played when squeaked at
		HSTRING m_hstrDeathSound;				// Sound played when dead
		HSTRING m_hstrDeathTriggerTarget;		// Target to trigger when dead
		HSTRING m_hstrDeathTriggerMessage;		// Message to send target when dead
		DFLOAT	m_fSoundRadius;					// Radius of sounds

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();
};

#endif // __CAT_H__
