// ----------------------------------------------------------------------- //
//
// MODULE  : ScreenShake.h
//
// PURPOSE : ScreenShake class - implementation
//
// CREATED : 1/25/99
//
// ----------------------------------------------------------------------- //

#ifndef __SCREEN_SHAKE_H__
#define __SCREEN_SHAKE_H__

#include "ltengineobjects.h"

class ScreenShake : public BaseClass
{
	public :

		ScreenShake();
		virtual ~ScreenShake();

	protected :

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
        virtual uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

	private :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
        LTBOOL   InitialUpdate();

		void	Update();
        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwSaveFlags);
		void	CacheFiles();

		HSTRING		m_hstrSound;
        LTFLOAT      m_fSoundRadius;
        LTVector     m_vAmount;
		int			m_nNumShakes;
        LTFLOAT      m_fAreaOfEffect;
        LTFLOAT      m_fFrequency;
};

#endif // __SCREEN_SHAKE_H__