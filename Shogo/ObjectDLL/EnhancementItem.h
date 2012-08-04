// ----------------------------------------------------------------------- //
//
// MODULE  : EnhancementItem.h
//
// PURPOSE : Base class for Enhancements
//
// CREATED : 2/6/98
//
// ----------------------------------------------------------------------- //

#ifndef __ENHANCEMENTITEM_H__
#define __ENHANCEMENTITEM_H__

#include "InventoryItem.h"

class EnhancementItem : public InventoryItem
{
	public :

		EnhancementItem();
		~EnhancementItem();

	protected :

		DDWORD			EngineMessageFn (DDWORD messageID, void *pData, DFLOAT lData);
		void			PostPropRead(ObjectCreateStruct *pStruct);

		virtual void	ObjectTouch (HOBJECT hObject);

	protected:

		DDWORD	m_nModelName;			// id of model name string in resource dll
		DDWORD	m_nModelSkin;			// id of skin name string in resource dll
		DDWORD	m_nSoundName;			// id of sound name string in resource dll
		DDWORD	m_nEnhancementSubType;	// type of upgrade object (subtype for inventory purposes)

	private :
		
		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

#endif // __EnhancementItem_H__
