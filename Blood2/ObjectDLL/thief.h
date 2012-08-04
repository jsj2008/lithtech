// ----------------------------------------------------------------------- //
//
// MODULE  : Thief.h
//
// PURPOSE : Thief header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __THIEF_AI_H__
#define __THIEF_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


#define MAX_DAMAGE		15

class Thief : public AI_Mgr
{
	public :

 		Thief();

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

		void	CacheFiles();

		void	Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void	Load(HMESSAGEREAD hRead, DDWORD dwLoadFloats);

		DBOOL	m_bJumping;

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;
		DFLOAT m_fLastDmgTime;
		DFLOAT m_fLastDetachTime;
};

#endif // __THIEF_AI_H__