// ----------------------------------------------------------------------- //
//
// MODULE  : ParticleSystem.h
//
// PURPOSE : ParticleSystem - Definition
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#ifndef __PARTICLE_SYSTEM_H__
#define __PARTICLE_SYSTEM_H__

#include "ClientSFX.h"

LINKTO_MODULE( ParticleSystem );

class ParticleSystem : public CClientSFX
{
	public :

		ParticleSystem();
		~ParticleSystem();

	protected :

        uint32  EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);

		bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

	private :

        LTBOOL   m_bOn;

        uint32  m_dwFlags;
        LTFLOAT  m_fBurstWait;
        LTFLOAT  m_fBurstWaitMin;
        LTFLOAT  m_fBurstWaitMax;
        LTFLOAT  m_fParticlesPerSecond;
        LTFLOAT  m_fParticleLifetime;
        LTFLOAT  m_fRadius;
        LTFLOAT  m_fGravity;
        LTFLOAT  m_fRotationVelocity;
        LTFLOAT  m_fViewDist;
		HSTRING	m_hstrTextureName;
        LTVector m_vColor1;
        LTVector m_vColor2;
        LTVector m_vDims;
        LTVector m_vMinVel;
        LTVector m_vMaxVel;

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

        LTBOOL ReadProp(ObjectCreateStruct *pStruct);
		void InitialUpdate(int nInfo);
};

#endif // __PARTICLESYSTEM_H__