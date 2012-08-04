// ----------------------------------------------------------------------- //
//
// MODULE  : Gabriel.h
//
// PURPOSE : Gabriel - Definition
//
// CREATED : 3/19/98
//
// ----------------------------------------------------------------------- //

#ifndef __GABRIEL_H__
#define __GABRIEL_H__

#include "cpp_engineobjects_de.h"
#include "MajorCharacter.h"


class Gabriel : public MajorCharacter
{
	public :

 		Gabriel();
		~Gabriel();

		void SetCanDamage(DBOOL bCanDamage);

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		void	ProcessDamageMsg(HMESSAGEREAD hRead);
		void	UpdateAggressive();
		void	InitializeWeapons();
		void	AdjustDamageAggregate();
		void	UpdateSenses();
		void	SetDeathAnimation();
		void	SetAnimationIndexes();
	
	private :
		
		DBOOL	SanjuroInRange();
		void	BuildTauntScript();
		void	ApproachSanjuro();
		void	CreateShield();

		void	PlayShieldSound();
		void	StopShieldSound();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		HOBJECT		m_hShield;
		HSOUNDDE	m_hShieldSound;
};

#endif // __GABRIEL_H__
