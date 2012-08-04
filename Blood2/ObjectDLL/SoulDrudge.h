// ----------------------------------------------------------------------- //
//
// MODULE  : SoulDrudge.h
//
// PURPOSE : SoulDrudge header
//
// CREATED : 11/20/97
//
// ----------------------------------------------------------------------- //

#ifndef __SOULDRUDGE_H__
#define __SOULDRUDGE_H__

#include "cpp_engineobjects_de.h"
#include "AI_Mgr.h"
#include "InventoryMgr.h"


class SoulDrudge : public AI_Mgr
{
	public :

 		SoulDrudge();

	protected :

		DDWORD EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);

	private :

		void	PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL	InitialUpdate(DVector *pMovement);

		void	ComputeState(int nStimType = -1);

		void	MC_Fire_Stand();

		void	CacheFiles();
		
		void	AI_STATE_AttackClose();
		void	AI_STATE_AttackFar();

		//keep only one copy per AI of animations
		static DBOOL		m_bLoadAnims;
        static CAnim_Sound	m_Anim_Sound;
};

#endif // __SOULDRUDGE_H__