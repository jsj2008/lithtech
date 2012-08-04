
// ----------------------------------------------------------------------- //
//
// MODULE  : WurmAI.h
//
// PURPOSE : WurmAI header
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#ifndef __WURM_H__
#define __WURM_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

//number of each type of sounds available
#define NUM_WRM_DEATH	3
#define NUM_WRM_FEAR	3
#define NUM_WRM_IDLE	3
#define NUM_WRM_PAIN	3
#define NUM_WRM_ATTACK	2
#define NUM_WRM_SPOT	3
#define NUM_WRM_LAUGH	3
#define NUM_WRM_ANGER	2

#define MC_FLYING		100

class WurmAI : public AI_Mgr
{
	public :

 		WurmAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	MC_Fly(DBOOL bUp);

		void	MC_Run();
		void	MC_Fire_Stand();

		void	ComputeState();

		void	AI_STATE_AttackClose();
		void	AI_STATE_Idle();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;

		DBOOL	m_bFlying;
};

#endif // __WURM_H__