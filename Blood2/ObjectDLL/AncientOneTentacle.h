// ----------------------------------------------------------------------- //
//
// MODULE  : AncientOneTentacle.h
//
// PURPOSE : AncientOneTentacle header
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#ifndef __AncientOneTentacle_H__
#define __AncientOneTentacle_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

#define		MC_FADEIN		101
#define		MC_FADEOUT		102
#define		MC_SCALEDOWN	103
#define		MC_SCALEUP		104

#define		DEATH_DURATION	10.0f

class AncientOneTentacle : public AI_Mgr
{
	public :

 		AncientOneTentacle();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	MC_Fire_Stand();

		void	ComputeState(int nStimType = -1);

		void	AI_STATE_AttackClose();
		void	AI_STATE_Idle();
		void	AI_STATE_Special1();
		void	AI_STATE_Special2();
		void	AI_STATE_Special3();
		void	AI_STATE_Special4();

		void	MC_FadeIn();
		void	MC_FadeOut();
		void	MC_ScaleDown();
		void	MC_ScaleUp();
		void	MC_Dead();
		void	MC_Idle();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;

		DFLOAT	m_fTimeOfDeath;
		DFLOAT	m_fLastIdle;
		DBOOL	m_bHiding;
		DFLOAT	m_fScaleBy;
};

#endif // __AncientOneTentacle_H__