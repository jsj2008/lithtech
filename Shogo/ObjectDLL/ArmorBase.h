// ----------------------------------------------------------------------- //
//
// MODULE  : Armor.h
//
// PURPOSE : Riot Armor Powerup baseclass
//
// CREATED : 1/28/97
//
// ----------------------------------------------------------------------- //

#ifndef __ARMORBASE_H__
#define __ARMORBASE_H__

#include "Powerup.h"

class ArmorBase : public Powerup
{
	public :

		ArmorBase();

	protected :

		DDWORD				EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		void				ObjectTouch (HOBJECT hObject);

		DDWORD				m_nArmor;
		DDWORD				m_nModelName;
		DDWORD				m_nModelSkin;
		DDWORD				m_nSoundName;

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwFlags);
};

#endif //  __ARMORBASE_H__