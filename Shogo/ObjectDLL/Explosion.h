// ----------------------------------------------------------------------- //
//
// MODULE  : Explosion.h
//
// PURPOSE : Model Explosion - Definition
//
// CREATED : 11/25/97
//
// BUTCHERED : 5/28/98
// 
// ----------------------------------------------------------------------- //

#ifndef __EXPLOSION_H__
#define __EXPLOSION_H__

#include "cpp_engineobjects_de.h"
#include "Projectile.h"
#include "DamageTypes.h"

class Explosion : public BaseClass
{
	public :

 		Explosion();
		void Setup(CProjectile* pProjectile);
		void Setup(DFLOAT fDuration, DFLOAT fDamageRadius, DFLOAT fMaxDamage);

		void GoBoom();

	protected :

		DDWORD			EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD			ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
		void			HandleTrigger(HOBJECT hSender, HMESSAGEREAD hRead);

		virtual void	UpdateDamage();

		DFLOAT			m_fDamageRadius;
		DFLOAT			m_fMaxDamage;
		DFLOAT			m_fDuration;
		DFLOAT			m_fDamageScaleUDur;
		DFLOAT			m_fDamageScaleDDur;
		DFLOAT			m_fLastDamageTime;
		DBOOL			m_bFirstUpdate;
		DFLOAT			m_fStartTime;

		DamageType		m_eDamageType;
		HOBJECT			m_hFiredFrom;

	private :
	
		void Update();
		void ReadProp(ObjectCreateStruct *pData);

		void Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);
};

#endif // __EXPLOSION_H__
