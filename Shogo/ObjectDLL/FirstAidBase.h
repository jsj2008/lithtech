// ----------------------------------------------------------------------- //
//
// MODULE  : FirstAidBase.h
//
// PURPOSE : Riot First Aid Powerup baseclass
//
// CREATED : 1/28/97
//
// ----------------------------------------------------------------------- //

#ifndef __FIRSTAIDBASE_H__
#define __FIRSTAIDBASE_H__

#include "Powerup.h"

class FirstAidBase : public Powerup
{
	public :

		FirstAidBase();

	protected :

		DDWORD				EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		void				ObjectTouch (HOBJECT hObject);

		DDWORD				m_nHealth;
		DDWORD				m_nModelName;
		DDWORD				m_nModelSkin;
		DDWORD				m_nSoundName;

	private :

		void Save(HMESSAGEWRITE hWrite, DDWORD dwFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwFlags);
};


#endif //  __FIRSTAIDBASE_H__