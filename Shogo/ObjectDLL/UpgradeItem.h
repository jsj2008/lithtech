// ----------------------------------------------------------------------- //
//
// MODULE  : UpgradeItem.h
//
// PURPOSE : Base class for Upgrades
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#ifndef __UPGRADEITEM_H__
#define __UPGRADEITEM_H__

#include "InventoryItem.h"

class UpgradeItem : public InventoryItem
{
	public :

		UpgradeItem();
		~UpgradeItem();

	protected :

		DDWORD			EngineMessageFn (DDWORD messageID, void *pData, DFLOAT lData);
		void			PostPropRead(ObjectCreateStruct *pStruct);

		virtual void	ObjectTouch (HOBJECT hObject);

	protected:

		DDWORD	m_nModelName;		// id of model name string in resource dll
		DDWORD	m_nModelSkin;		// id of skin name string in resource dll
		DDWORD	m_nSoundName;		// id of sound name string in resource dll
		DDWORD	m_nUpgradeSubType;	// type of upgrade object (subtype for inventory purposes)

		DFLOAT	m_fCreationTime;	// time at creation
		HOBJECT m_hDroppedBy;		// user we were spawned by, if any

	private:

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

#endif // __UpgradeItem_H__
