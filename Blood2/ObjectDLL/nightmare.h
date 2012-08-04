
// ----------------------------------------------------------------------- //
//
// MODULE  : Nightmare.h
//
// PURPOSE : Nightmare header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifdef _ADD_ON
#ifndef __Nightmare_AI_H__
#define __Nightmare_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

//number of each type of sounds available
#define NUM_NIT_DEATH		3
#define NUM_NIT_FEAR		2
#define NUM_NIT_IDLE		3
#define NUM_NIT_PAIN		3
#define NUM_NIT_BATTLE_CRY	3
#define NUM_NIT_SPOT		3
#define NUM_NIT_ATTACK		3

class Nightmare : public AI_Mgr
{
	public :

 		Nightmare();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	ComputeState(int nStimType = -1);

		void	MC_Fire_Stand();
		
		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();
		void	AI_STATE_Idle();

		void	CacheFiles();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;

		DFLOAT	m_fFullHealth;
		DBOOL	m_bCreateHealth;
		DBOOL	m_bShockwave;
};

#endif // __Nightmare_AI_H__
#endif // _ADD_ON