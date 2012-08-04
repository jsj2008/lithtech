// ----------------------------------------------------------------------- //
//
// MODULE  : Cothineal.h
//
// PURPOSE : Cothineal - Definition
//
// CREATED : 5/18/98
//
// ----------------------------------------------------------------------- //

#ifndef __COTHINEAL_H__
#define __COTHINEAL_H__

#include "cpp_engineobjects_de.h"
#include "MajorCharacter.h"


class Cothineal : public MajorCharacter
{
	public :

 		Cothineal();
		~Cothineal();

	protected :

		DDWORD	EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD	ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		void	ProcessDamageMsg(HMESSAGEREAD hRead);
		void	AdjustDamageAggregate();
		void	UpdateIdle();
		void	SetAnimationIndexes();
		void	UpdateAnimation();

	private :

		void	FindGabriel();
		void	CreateControlBeam();
		void	UpdateControlBeam();
		void	ToggleIdleAnis();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFlags);

		HOBJECT		m_hGabriel;
		HOBJECT		m_hBeam;
		DFLOAT		m_fToggleEyeTime;
		DFLOAT		m_fStartProtectionTime;
		DBOOL		m_bEyeOpen;
		DBOOL		m_bAniTransition;
		DBOOL		m_bOkToProcessDamage;

		HMODELANIM	m_hOpenEyeAni[3];
		HMODELANIM	m_hClosedEyeAni[3];
		HMODELANIM	m_hOpenToClose;
		HMODELANIM	m_hCloseToOpen;
};

#endif // __COTHINEAL_H__
