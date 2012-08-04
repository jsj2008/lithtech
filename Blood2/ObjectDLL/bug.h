// ----------------------------------------------------------------------- //
//
// MODULE  : Bug.h
//
// PURPOSE : Bug header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __BUG_AI_H__
#define __BUG_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

//number of each type of sounds available
#define NUM_BUG_DEATH	5
#define NUM_BUG_FEAR	1
#define NUM_BUG_IDLE	2
#define NUM_BUG_PAIN	11
#define NUM_BUG_SCREAM	3
#define NUM_BUG_SPOT	15


class BugAI : public AI_Mgr
{
	public :

 		BugAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	MC_Run();

		void	AI_STATE_Escape_RunAway();
		void	AI_STATE_Idle();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;		
};

#endif // __BUG_AI_H__