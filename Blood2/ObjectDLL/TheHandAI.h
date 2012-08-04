// ----------------------------------------------------------------------- //
//
// MODULE  : TheHandAI.h
//
// PURPOSE : TheHandAI header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __HAND_AI_H__
#define __HAND_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

#define MAX_DAMAGE		10

class TheHandAI : public AI_Mgr
{
	public :

 		TheHandAI();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	MC_Fire_Jump();

		void	ComputeState(int nStimType = -1);

		void	AI_STATE_AttackClose();
		void	AI_STATE_EnemyAttach();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		void	CacheFiles();

		DBOOL	m_bJumping;

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;
		
		DFLOAT	m_fLastDmgTime;
		DFLOAT	m_fLastDetachTime;
};

#endif // __HAND_AI_H__