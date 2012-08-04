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

LINKTO_MODULE( Fire );

class Fire : public CClientSFX
{
	public :

		Fire();
		~Fire();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

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

        void    Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void    Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

        LTBOOL   ReadProp(ObjectCreateStruct *pStruct);
		void	InitialUpdate(int nInfo);
};

#endif // __FIRE_H__