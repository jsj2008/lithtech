// ----------------------------------------------------------------------- //
//
// MODULE  : Behemoth.h
//
// PURPOSE : Behemoth header
//
// CREATED : 12/15/97
//
// ----------------------------------------------------------------------- //

#ifndef __BEHEMOTH_AI_H__
#define __BEHEMOTH_AI_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"

class Behemoth : public AI_Mgr
{
	public :

 		Behemoth();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	ComputeState(int nStimType = -1);

		void	MC_Fire_Stand();
		
		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();

		void	CacheFiles();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;
};

#endif // __BEHEMOTH_AI_H__