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
#include "GameBase.h"

LINKTO_MODULE( ScreenShake );

class ScreenShake : public GameBase
{
	public :

		ScreenShake();
		virtual ~ScreenShake();

	protected :

        virtual uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT lData);
		virtual bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

        LTBOOL   ReadProp(ObjectCreateStruct *pData);
        LTBOOL   InitialUpdate();

		void	Update();
        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwSaveFlags);

		HSTRING		m_hstrSound;
        LTFLOAT      m_fSoundRadius;
        LTVector     m_vAmount;
		int			m_nNumShakes;
        LTFLOAT      m_fAreaOfEffect;
        LTFLOAT      m_fFrequency;
};

#endif // __SCREEN_SHAKE_H__