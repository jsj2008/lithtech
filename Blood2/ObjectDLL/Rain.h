// ----------------------------------------------------------------------- //
//
// MODULE  : Rain.h
//
// PURPOSE : Rain - Definition
//
// CREATED : 10/23/97
//
// ----------------------------------------------------------------------- //

#ifndef __RAIN_H__
#define __RAIN_H__

#include "ClientSFX.h"


class Rain : public CClientSFX
{
	public :

		Rain();
		~Rain() {}

		void Setup(DFLOAT fDensity, DVector vDims, DFLOAT fLifeTime, DVector vDir, DBOOL bGravity, DFLOAT fScale, 
				   DFLOAT fSpread, DVector vColor1, DVector vColor2, DFLOAT fTimeLimit, DFLOAT fPulse);

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
	private :

		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	InitialUpdate(DVector *pMovement);
		void	SendEffectMessage();
		void	TriggerMsg(HOBJECT hSender, HSTRING hMsg);
		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		DDWORD	m_dwFlags;
		DFLOAT	m_fDensity;
		DVector m_vDims;
		DFLOAT	m_fLifetime;
		DVector m_vDirection;
		DBOOL	m_bGravity;
		DFLOAT	m_fParticleScale;
		DFLOAT	m_fSpread;
		DBOOL	m_bTriggered;
		DVector	m_vColor1;
		DVector	m_vColor2;
		DFLOAT	m_fTimeLimit;
		DFLOAT  m_fPulse;
};

#endif // __POLY_GRID_H__
