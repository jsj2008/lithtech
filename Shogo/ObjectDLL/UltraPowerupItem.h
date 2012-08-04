// ----------------------------------------------------------------------- //
//
// MODULE  : UltraPowerupItem.h
//
// PURPOSE : Base class for UltraPowerupItem
//
// CREATED : 1/28/98
//
// ----------------------------------------------------------------------- //

#ifndef __ULTRAPOWERUPITEM_H__
#define __ULTRAPOWERUPITEM_H__

#include "Powerup.h"

class UltraPowerupItem : public Powerup
{
	public :

		UltraPowerupItem();
		~UltraPowerupItem();

	protected :

		DDWORD			EngineMessageFn (DDWORD messageID, void *pData, DFLOAT lData);
		void			PostPropRead(ObjectCreateStruct *pStruct);

		virtual void	ObjectTouch (HOBJECT hObject);

	protected:

		DDWORD	m_nModelName;		// id of model name string in resource dll
		DDWORD	m_nModelSkin;		// id of skin name string in resource dll
		DDWORD	m_nSoundName;		// id of sound name string in resource dll

	private:

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

#endif // __UltraPowerupItem_H__
