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


class ParticleSystem : public CClientSFX
{
	public :

		ParticleSystem();
		~ParticleSystem();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		void	 HandleMsg(HOBJECT hSender, HSTRING hMsg);

	private :

		DBOOL	m_bOn;

		DDWORD	m_dwFlags;
		DFLOAT	m_fBurstWait;
		DFLOAT	m_fParticlesPerSecond;
		DFLOAT	m_fEmissionRadius;
		DFLOAT	m_fMinimumVelocity;
		DFLOAT	m_fMaximumVelocity;
		DFLOAT	m_fVelocityOffset;
		DFLOAT	m_fParticleLifetime;
		DFLOAT	m_fRadius;
		DFLOAT	m_fGravity;
		DFLOAT	m_fRotationVelocity;
		HSTRING	m_hstrTextureName;
		DVector	m_vColor1;
		DVector	m_vColor2;

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
		void CacheFiles();

		DBOOL ReadProp(ObjectCreateStruct *pStruct);
		void InitialUpdate(int nInfo);
};

#endif // __POLY_GRID_H__
