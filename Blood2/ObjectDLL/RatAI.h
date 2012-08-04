// ----------------------------------------------------------------------- //
//
// MODULE  : RatAI.h
//
// PURPOSE : RatAI header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __RAT_AI_H__
#define __RAT_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

//number of each type of sounds available
#define NUM_RAT_DEATH	5
#define NUM_RAT_FEAR	1
#define NUM_RAT_IDLE	2
#define NUM_RAT_PAIN	11
#define NUM_RAT_SCREAM	3
#define NUM_RAT_SPOT	15


class RatAI : public AI_Mgr
{
	public :

 		RatAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	AI_STATE_Escape_RunAway();
		void	AI_STATE_Idle();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;		
};

#endif // __RAT_AI_H__