// ----------------------------------------------------------------------- //
//
// MODULE  : SoundFX.h
//
// PURPOSE : SoundFX Inventory Item
//
// CREATED : 02/03/98
//
// ----------------------------------------------------------------------- //

#ifndef __SOUND_FX_H__
#define __SOUND_FX_H__

#include "cpp_engineobjects_de.h"
#include "Destructable.h"
#include "B2BaseClass.h"


#define SNDFX_OFF		0
#define SNDFX_RAMPUP	1
#define SNDFX_ON		2
#define SNDFX_RAMPDOWN	3


class SoundFX : public B2BaseClass
{
	public :

 		SoundFX();
		~SoundFX();

	protected :

		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
        DDWORD		ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

        void		HandleTrigger( HOBJECT hSender, HMESSAGEREAD hRead );
        DBOOL		ReadProp(ObjectCreateStruct *pData);
        void		PostPropRead(ObjectCreateStruct *pStruct);
        DBOOL		InitialUpdate(DVector *pMovement);
        DBOOL		Update();

		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void		UpdateSound();
		void		CacheFiles();

	protected : 

		// Member Variables

		DBOOL		m_bStartOn;
		DBYTE		m_byState;			// State

		HSTRING		m_hstrRampUpSound;
		HSTRING		m_hstrRampDownSound;
		HSTRING		m_hstrSound;
		HSOUNDDE	m_hsndSound;
		float		m_fOuterRadius;
		float		m_fInnerRadius;
		DBYTE		m_nVolume;
		DBOOL		m_bAmbient;
		DBOOL		m_bFileStream;
		unsigned char m_nPriority;
};




#endif // __SOUND_FX_H__


