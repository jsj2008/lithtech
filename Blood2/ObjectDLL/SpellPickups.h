// ----------------------------------------------------------------------- //
//
// MODULE  : SpellPickups.h
//
// PURPOSE : Spell powerup items
//
// CREATED : 12/11/97
//
// ----------------------------------------------------------------------- //

#ifndef __SPELLPICKUPS_H__
#define __SPELLPICKUPS_H__


#include "PickupObject.h"
#include "InventoryMgr.h"


class SpellPickup : public PickupObject
{
	public:

		SpellPickup() : PickupObject() 
		{
			m_szFile = DNULL;
			m_nSpellType = SPELL_MAXSPELLTYPES;
		}

	protected :

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		void		ObjectTouch (HOBJECT hObject);
		char		*m_szFile;

		DBYTE		m_nSpellType;
};

/*
class BulletAmmoPU : public AmmoPickup
{
	public :
		BulletAmmoPU();
};

*/

#endif //  __SPELLPICKUPS_H__