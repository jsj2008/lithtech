// ----------------------------------------------------------------------- //
//
// MODULE  : Fire.h
//
// PURPOSE : Fire - Definition
//
// CREATED : 5/6/99
//
// (c) 1999 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __FIRE_H__
#define __FIRE_H__

#include "ClientSFX.h"
#include "SFXMsgIds.h"

class Fire : public CClientSFX
{
	public :

		Fire();
		~Fire();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32  ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);
		void	HandleMsg(HOBJECT hSender, const char* szMsg);

	private :

        LTBOOL   m_bOn;
        LTBOOL   m_bSmoke;
        LTBOOL   m_bLight;
        LTBOOL   m_bSparks;
        LTBOOL   m_bSound;
        LTBOOL   m_bBlackSmoke;
        LTBOOL   m_bSmokeOnly;
        LTFLOAT  m_fRadius;
        LTFLOAT  m_fSoundRadius;
        LTFLOAT  m_fLightRadius;
        LTFLOAT  m_fLightPhase;
        LTFLOAT  m_fLightFreq;
        LTVector m_vLightColor;
        LTVector m_vLightOffset;

        void    Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void    Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

        LTBOOL   ReadProp(ObjectCreateStruct *pStruct);
		void	InitialUpdate(int nInfo);
};

#endif // __FIRE_H__