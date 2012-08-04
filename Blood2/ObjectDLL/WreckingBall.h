
#ifndef __WRECKINGBALL_H__
#define __WRECKINGBALL_H__



#include "cpp_engineobjects_de.h"
#include "cpp_server_de.h"
#include "B2BaseClass.h"
#include "Destructable.h"


class WreckingBall : public B2BaseClass
{
	public :

		WreckingBall();
		virtual ~WreckingBall();

	protected :

		virtual DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		void	FirstUpdate();

	private :

		DBOOL	InitialUpdate(DVector *pMovement);
		DBOOL	Update(DVector *pMovement);
		void	ComputeAttachPoint(DVector* pvAttachPoint);
		DBOOL	DrawChain(DVector* pvAttachPoint);

		CDestructable m_damage;

		DVector m_vLastAttachPoint;
		DVector m_vMomentum;

		DFLOAT	m_fDamage;
		DBOOL m_bFirstUpdate;
		HOBJECT	m_hCrane;
		HOBJECT	m_hChain;
};



#endif  // __WRECKINGBALL_H__